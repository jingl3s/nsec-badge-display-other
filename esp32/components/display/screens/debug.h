#pragma once

#ifndef SIMULATOR
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

namespace debug_tab
{
enum {
    score = 0,
    cup,
    cards,
    sounds,
    led,
#ifdef SDCARD_ENABLED
    disk,
#endif
    count // keep last
};
} // namespace debug_tab
#ifdef SDCARD_ENABLED
namespace sd_info_rows
{
enum {
    inserted = 0,
    name,
    capacity,
    mount,

    count // keep last
};
}
#endif

void screen_debug_init();
void screen_debug_loop();

#ifdef __cplusplus
}
#endif
