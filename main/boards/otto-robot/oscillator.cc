#include "oscillator.h"
#include <driver/ledc.h>
#include <cmath>
#include <esp_timer.h>

// 获取毫秒级的系统时间，替代 Arduino 的 millis()
static unsigned long millis() {
    return esp_timer_get_time() / 1000;
}

// 静态变量，用于全局分配不同的 LEDC 通道
static ledc_channel_t next_free_channel = LEDC_CHANNEL_0;

Oscillator::Oscillator(int trim) {
    trim_ = trim;
    pin_ = -1;
    rev_ = false;
    current_angle_ = 90; // 默认居中
    target_angle_ = 90;
    previous_millis_ = 0;
}

Oscillator::~Oscillator() {
    if (pin_ != -1) {
        ledc_stop(LEDC_LOW_SPEED_MODE, channel_, 0);
    }
}

void Oscillator::Attach(int pin, bool rev) {
    pin_ = pin;
    rev_ = rev;
    
    // 【关键修改】枚举类型不能直接使用 ++，必须先转 int 加 1，再转回枚举
    channel_ = next_free_channel;
    next_free_channel = (ledc_channel_t)((int)next_free_channel + 1);
    
    timer_ = LEDC_TIMER_0;

    // 配置 50Hz 舵机 PWM 定时器
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = timer_,
        .freq_hz = 50, 
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    // 配置通道
    ledc_channel_config_t ch_conf = {
        .gpio_num = pin_,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = channel_,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = timer_,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ch_conf);
    
    // 初始赋值
    Write(current_angle_);
}

void Oscillator::Write(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    if (rev_) {
        angle = 180 - angle;
    }

    // 角度转 PWM 占空比：0.5ms ~ 2.5ms (对应 204 ~ 1024)
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

bool Oscillator::NextSample() {
    unsigned long current_millis = millis();
    
    // 每 20ms 步进一度，实现平滑动画
    if (current_millis - previous_millis_ > 20) {
        previous_millis_ = current_millis;
        
        if (current_angle_ != target_angle_) {
            if (current_angle_ < target_angle_) {
                current_angle_++;
            } else {
                current_angle_--;
            }
            Write(current_angle_);
            return true;
        }
    }
    return false;
}
