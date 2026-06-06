#pragma once

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t *tab_images_init_real(lv_obj_t *tab_view, const char *tab_name);

int scan_images(void);

void show_current_image(void);

#ifdef __cplusplus
}
#endif
