#include "display_ledc_backlight.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "save.h"
#include <driver/gpio.h>
#include <stdio.h>

static const char *TAG = "display_backlight";

void DisplayLedcBacklight::init()
{
    _ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT, // 8Bit will be sufficient for
                                             // backlight from 0 to 255
        .timer_num = LEDC_TIMER_2,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    _ledc_channel = {
        .gpio_num = GPIO_NUM_21,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_4,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_2,
        .duty = 255,
        .hpoint = 0,
        .flags =
            {
                .output_invert = 0,
            },
    };

    ledc_timer_config(&_ledc_timer);

    DisplayLedcBacklight::start();
}

void DisplayLedcBacklight::start()
{
    if (Save::save_data.display_backlight < 10 || Save::save_data.display_backlight > 255){
        setBrightness(127);

    }
    else
    {
        setBrightness(Save::save_data.display_backlight);
    }
}

void DisplayLedcBacklight::stop()
{
    ledc_stop(_ledc_channel.speed_mode, _ledc_channel.channel, 0);
    gpio_set_level((gpio_num_t)_ledc_channel.gpio_num, 127);
}

void DisplayLedcBacklight::setBrightness(uint8_t brightness)
{

    ledc_channel_config(&_ledc_channel);

    ledc_set_duty(_ledc_channel.speed_mode, _ledc_channel.channel, brightness);
    ledc_update_duty(_ledc_channel.speed_mode, _ledc_channel.channel);

    _brightness = brightness;
    ESP_LOGE(TAG, "LCD brightness: '%d'", brightness);
    if (Save::save_data.display_backlight != brightness) {
        Save::save_data.display_backlight = brightness;
    }
}

uint8_t DisplayLedcBacklight::getBrightness()
{
    return _brightness;
}

void display_backlight_set_brightness(uint8_t brightness)
{
    DisplayLedcBacklight::getInstance().setBrightness(brightness);
}
