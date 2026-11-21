#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

// 【关键修改】替换旧版 ADC 驱动
#include <esp_adc/adc_oneshot.h>
#include <driver/gpio.h>

#define OTTO_VERSION_AUTO 0
#define OTTO_VERSION_CAMERA 1
#define OTTO_VERSION_NO_CAMERA 2

#ifndef OTTO_HARDWARE_VERSION
#define OTTO_HARDWARE_VERSION OTTO_VERSION_AUTO
#endif

// ... (中间结构体定义保持原样) ...

constexpr HardwareConfig NON_CAMERA_VERSION_CONFIG = {
    .power_charge_detect_pin = GPIO_NUM_21,
    .power_adc_unit = ADC_UNIT_2,
    .power_adc_channel = ADC_CHANNEL_3,
    // 【关键修改】避开 GPIO34-39 禁用区间
    .right_leg_pin = GPIO_NUM_41,  
    .right_foot_pin = GPIO_NUM_42, 
    .left_leg_pin = GPIO_NUM_17,
    .left_foot_pin = GPIO_NUM_18,
    .left_hand_pin = GPIO_NUM_8,
    .right_hand_pin = GPIO_NUM_12,
    .audio_input_sample_rate = 16000,
    .audio_output_sample_rate = 24000,
    .audio_use_simplex = true,
    .audio_i2s_gpio_ws = GPIO_NUM_NC,
    .audio_i2s_gpio_bclk = GPIO_NUM_NC,
    .audio_i2s_gpio_din = GPIO_NUM_NC,
    .audio_i2s_gpio_dout = GPIO_NUM_NC,
    .audio_i2s_mic_gpio_ws = GPIO_NUM_4,
    .audio_i2s_mic_gpio_sck = GPIO_NUM_5,
    .audio_i2s_mic_gpio_din = GPIO_NUM_6,
    .audio_i2s_spk_gpio_dout = GPIO_NUM_7,
    .audio_i2s_spk_gpio_bclk = GPIO_NUM_15,
    .audio_i2s_spk_gpio_lrck = GPIO_NUM_16,
    // 【关键修改】避开 Strapping 引脚
    .display_backlight_pin = GPIO_NUM_3,
    .display_mosi_pin = GPIO_NUM_10,
    .display_clk_pin = GPIO_NUM_9,
    .display_dc_pin = GPIO_NUM_13,  
    .display_rst_pin = GPIO_NUM_11,
    .display_cs_pin = GPIO_NUM_14,  
    .i2c_sda_pin = GPIO_NUM_NC,
    .i2c_scl_pin = GPIO_NUM_NC,
};

// ... (后面的宏定义保持原样) ...
#endif
