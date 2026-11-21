#ifndef _OSCILLATOR_H_
#define _OSCILLATOR_H_

#include <driver/ledc.h>

class Oscillator {
public:
    Oscillator(int trim = 0);
    ~Oscillator();
    
    void Attach(int pin, bool rev = false);
    void Detach();
    void Write(int angle);
    void SetTarget(int angle);
    int GetCurrent();
    bool NextSample();
    
    // 补全缺失的接口
    void SetLimiter(int limit);
    void DisableLimiter();

private:
    int pin_;
    int trim_;
    bool rev_;
    
    ledc_channel_t channel_;
    
    int current_angle_;
    int target_angle_;
    unsigned long previous_millis_;
    
    // 补全缺失的限幅变量
    int diff_limit_; 
};

#endif // _OSCILLATOR_H_
