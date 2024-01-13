#include "lv_conf.h"
#include "lvgl/lvgl.h"

lv_obj_t *create_container(lv_obj_t *parent, const char *title, lv_layout_t layout, bool max_horizontal);

void util_styles_init(void);

lv_obj_t *create_switch_with_label(lv_obj_t *parent, const char *text, bool enabled);