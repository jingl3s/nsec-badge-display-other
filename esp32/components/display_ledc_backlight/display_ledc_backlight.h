#pragma once

#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <stdio.h>

class DisplayLedcBacklight
{
  public:
    static DisplayLedcBacklight &getInstance()
    {
        static DisplayLedcBacklight instance;
        return instance;
    }

  private:
    DisplayLedcBacklight()
    {
    }

    uint8_t _brightness;
    ledc_timer_config_t _ledc_timer;
    ledc_channel_config_t _ledc_channel;

  public:
    DisplayLedcBacklight(DisplayLedcBacklight const &) = delete;
    void operator=(DisplayLedcBacklight const &) = delete;

    void init();
    void stop();
    void start();
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness();
};

void display_backlight_set_brightness(uint8_t brightness);