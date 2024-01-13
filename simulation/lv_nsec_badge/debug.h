// #pragma once

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
#ifndef SIMULATOR
namespace debug_tab {
    enum {
        score=0,
        led,
        // disk,

        count // keep last
    };
}

namespace sd_info_rows {
enum {
    inserted = 0,
    name,
    capacity,
    mount,

    count // keep last
};
}
#endif

void screen_debug_init(void);
void screen_debug_loop(void);

#ifdef __cplusplus
}
#endif
