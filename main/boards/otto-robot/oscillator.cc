#include "oscillator.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include <cmath>

extern unsigned long IRAM_ATTR millis();

Oscillator::Oscillator(int trim) {
    trim_ = trim;
    current_millis_ = 0;
    previous_servo_command_millis_ = 0;
    current_servo_command_ = 0;
    target_servo_command_ = 0;
    next_free_channel_ = LEDC_CHANNEL_0;
}

Oscillator::~Oscillator() {
    // Detach if attached
}

bool Oscillator::NextSample() {
    long current_millis = millis();
    if (current_millis - previous_servo_command_millis_ > 20) {
        previous_servo_command_millis_ = current_millis;
        if (current_servo_command_ != target_servo_command_) {
            if (current_servo_command_ < target_servo_command_) {
                current_servo_command_++;
            } else {
                current_servo_command_--;
            }
            Write(current_servo_command_);
            return true;
        }
    }
    return false;
}

void Oscillator::Attach(int pin, bool rev) {
    pin_ = pin;
    rev_ = rev;
    
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 50,  // 50Hz for servo
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = pin,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = next_free_channel_,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
    channel_ = next_free_channel_;
    next_free_channel_ = (ledc_channel_t)((int)next_free_channel_ + 1);
    
    previous_servo_command_millis_ = millis();
}

void Oscillator::Write(int angle) {
    // Convert angle to duty cycle (13 bit, 50Hz)
    // 0.5ms - 2.5ms pulse width
    // 50Hz = 20ms period -> 20000us
    // 13 bit = 8192
    
    if (rev_) {
        angle = 180 - angle;
    }
    
    // 0.5ms (0 deg) -> 0.5/20 * 8192 = 204
    // 2.5ms (180 deg) -> 2.5/20 * 8192 = 1024
    int duty = 204 + (angle * (1024 - 204)) / 180;
    duty += trim_;
    
    if (duty < 0) duty = 0;
    if (duty > 8191) duty = 8191;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel_, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel_);
}

void Oscillator::SetTarget(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    target_servo_command_ = angle;
}

int Oscillator::GetCurrent() {
    return current_servo_command_;
}
