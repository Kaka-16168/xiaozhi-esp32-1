#include "oscillator.h"
#include <driver/ledc.h>
#include <cmath>
#include <esp_timer.h>

static unsigned long millis() {
    return esp_timer_get_time() / 1000;
}

static ledc_channel_t next_free_channel = LEDC_CHANNEL_0;

Oscillator::Oscillator(int trim) {
    trim_ = trim;
    pin_ = -1;
    rev_ = false;
    current_angle_ = 90;
    target_angle_ = 90;
    previous_millis_ = 0;
    diff_limit_ = 1; // 默认每帧移动 1 度
}

Oscillator::~Oscillator() {
    Detach();
}

void Oscillator::Attach(int pin, bool rev) {
    pin_ = pin;
    rev_ = rev;
    
    channel_ = next_free_channel;
    next_free_channel = (ledc_channel_t)((int)next_free_channel + 1);

    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 50, 
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ch_conf = {
        .gpio_num = pin_,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = channel_,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ch_conf);
    
    Write(current_angle_);
}

void Oscillator::Detach() {
    if (pin_ != -1) {
        ledc_stop(LEDC_LOW_SPEED_MODE, channel_, 0);
        pin_ = -1;
    }
}

void Oscillator::Write(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    if (rev_) {
        angle = 180 - angle;
    }

    int duty = 204 + (int)((angle / 180.0) * (1024 - 204));
    duty += trim_;

    if (duty < 0) duty = 0;
    if (duty > 8191) duty = 8191;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel_, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel_);
    
    current_angle_ = angle;
}

void Oscillator::SetTarget(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    target_angle_ = angle;
}

int Oscillator::GetCurrent() {
    return current_angle_;
}

void Oscillator::SetLimiter(int limit) {
    if (limit < 1) limit = 1;
    diff_limit_ = limit;
}

void Oscillator::DisableLimiter() {
    diff_limit_ = 180; // 允许最大跨度移动
}

bool Oscillator::NextSample() {
    unsigned long current_millis = millis();
    
    if (current_millis - previous_millis_ > 20) {
        previous_millis_ = current_millis;
        
        if (current_angle_ != target_angle_) {
            int diff = target_angle_ - current_angle_;
            if (diff > diff_limit_) diff = diff_limit_;
            if (diff < -diff_limit_) diff = -diff_limit_;
            
            current_angle_ += diff;
            Write(current_angle_);
            return true;
        }
    }
    return false;
}
