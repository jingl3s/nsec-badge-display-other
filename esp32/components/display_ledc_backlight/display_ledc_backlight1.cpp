/* LEDC (LED Controller) fade example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <stdio.h>

#define LEDC_TEST_DUTY (255)
#define LEDC_TEST_FADE_TIME (3000)

void app_main2(void)
{
    // int ch;

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT, // 8Bit will be sufficient for backlight from 0 to 255
        .timer_num = LEDC_TIMER_2,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_channel_config_t ledc_channel = {
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

    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);

    // Initialize fade service.
    // ledc_fade_func_install(0);

    // long length = 5;
    // pause during sound
    // vTaskDelay(pdMS_TO_TICKS(length));
    // ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel,
    //                         LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
    // ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel,
    //                 LEDC_FADE_NO_WAIT);
    // vTaskDelay(1000 / portTICK_PERIOD_MS);

    // ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 80);
    // ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    // gpio_set_level(LEDC_HS_CH0_GPIO, 0);


    // while (1) {
        // printf("1. LEDC fade up to duty = %d\n", LEDC_TEST_DUTY);
        // ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel,
        //                         LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
        // ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel,
        //                 LEDC_FADE_NO_WAIT);
        // vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);

        // printf("2. LEDC fade down to duty = 0\n");
        // ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel,
        //                         0, LEDC_TEST_FADE_TIME);
        // ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel,
        //                 LEDC_FADE_NO_WAIT);
        // vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);

        printf("3. LEDC set duty = %d without fade\n", 255);
        ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel,
                      255);
        ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("3. LEDC set duty = %d without fade\n", 127);
        ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel,
                      127);
        ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // printf("3. LEDC set duty = %d without fade\n", 1);
        // ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel,
        //               1);
        // ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     printf("4. LEDC set duty = 0 without fade\n");
    //     ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
    //     ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    // gpio_set_level(LEDC_HS_CH0_GPIO, 0);
}