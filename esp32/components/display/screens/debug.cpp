#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "lv_conf.h"
#include "lvgl/lvgl.h"
#ifndef SIMULATOR
#include "lvgl_helpers.h"

#include "buzzer.h"
#include "disk.h"
#include "display_ledc_backlight.h"
#include "lv_utils.h"
#include "neopixel.h"
#include "save.h"
#include "screens/debug.h"
#include "screens/score_teams.h"
#else
#include "debug.h"
#include "score_teams.h"
#endif

#ifdef SIMULATOR
#include "lv_utils.h"
#include <stdlib.h>
#include <time.h>
extern long int random(void)
{
    return rand(); // Returns a pseudo-random integer between 0 and RAND_MAX.
}
//
#define ESP_LOGE(param, ...) printf(__VA_ARGS__);

#else 
static const char *TAG = "display";
extern long int random(void)
{
    return rand(); // Returns a pseudo-random integer between 0 and RAND_MAX.
}

#endif

static lv_obj_t *tab_view;

static lv_obj_t *led_container;
static lv_obj_t *score_controls_container;

typedef struct debug_tabs debug_tabs_t;

typedef lv_obj_t *(*tab_init_cb_t)(debug_tabs_t *tab);

struct debug_tabs {
    int id;
    const char *name;
    lv_obj_t *tab, *enable_switch;
    bool enabled;
    tab_init_cb_t init;
};
#ifdef SDCARD_ENABLED
static lv_obj_t *disk_info_container;
static lv_obj_t *tab_disk_init(debug_tabs_t *tab);
#endif
static lv_obj_t *tab_config_init(debug_tabs_t *tab);
static lv_obj_t *tab_score_init(debug_tabs_t *tab);
static lv_obj_t *tab_cup_init(debug_tabs_t *tab);
static lv_obj_t *tab_sounds_init(debug_tabs_t *tab);

static const char *TITRE_TAB_SCORE = "SCORE";
static const char *TITRE_TAB_CUP = "COUPE";
static const char *TITRE_TAB_CONFIG = "CONFIG";
static const char *TITRE_TAB_SOUNDS = "SONS";

#ifndef SIMULATOR
static TickType_t last_save_at =
    0; // used to keep sending beacons every 10 seconds

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
debug_tabs_t debug_tabs[debug_tab::count] = {
    {.name = TITRE_TAB_SCORE, .init = tab_score_init},
    {.name = TITRE_TAB_CUP, .init = tab_cup_init},
    {.name = TITRE_TAB_SOUNDS, .init = tab_sounds_init},
    {.name = TITRE_TAB_CONFIG, .init = tab_config_init},
#ifdef SDCARD_ENABLED
    {.name = "SD", .init = tab_disk_init},
#endif
};

static const char *FX_mode_names[] = {
    "Solid",
    "Blink",
    "Breathe",
    "Wipe",
    "Wipe Random",
    "Random Colors",
    "Sweep",
    "Dynamic",
    "Colorloop",
    "Rainbow",
    "Scan",
    "Scan Dual",
    "Fade",
    "Theater",
    "Theater Rainbow",
    "Running",
    "Saw",
    "Twinkle",
    "Dissolve",
    "Dissolve Rnd",
    "Sparkle",
    "Sparkle Dark",
    "Sparkle+",
    "Strobe",
    "Strobe Rainbow",
    "Strobe Mega",
    "Blink Rainbow",
    "Android",
    "Chase",
    "Chase Random",
    "Chase Rainbow",
    "Chase Flash",
    "Chase Flash Rnd",
    "Rainbow Runner",
    "Colorful",
    "Traffic Light",
    "Sweep Random",
    "Running 2",
    "Red & Blue",
    "Stream",
    "Scanner",
    "Lighthouse",
    "Fireworks",
    "Rain",
    "Merry Christmas",
    "Fire Flicker",
    "Gradient",
    "Loading",
    "Police",
    "Police All",
    "Two Dots",
    "Two Areas",
    "Circus",
    "Halloween",
    "Tri Chase",
    "Tri Wipe",
    "Tri Fade",
    "Lightning",
    "ICU",
    "Multi Comet",
    "Scanner Dual",
    "Stream 2",
    "Oscillate",
    "Pride 2015",
    "Juggle",
    "Palette",
    "Fire 2012",
    "Colorwaves",
    "Bpm",
    "Fill Noise",
    "Noise 1",
    "Noise 2",
    "Noise 3",
    "Noise 4",
    "Colortwinkles",
    "Lake",
    "Meteor",
    "Meteor Smooth",
    "Railway",
    "Ripple",
    "Twinklefox",
    "Twinklecat",
    "Halloween Eyes",
    "Solid Pattern",
    "Solid Pattern Tri",
    "Spots",
    "Spots Fade",
    "Glitter",
    "Candle",
    "Fireworks Starburst",
    "Fireworks 1D",
    "Bouncing Balls",
    "Sinelon",
    "Sinelon Dual",
    "Sinelon Rainbow",
    "Popcorn",
    "Drip",
    "Plasma",
    "Percent",
    "Ripple Rainbow",
    "Heartbeat",
    "Pacifica",
    "Candle Multi",
    "Solid Glitter",
    "Sunrise",
    "Phased",
    "Twinkleup",
    "Noise Pal",
    "Sine",
    "Phased Noise",
    "Flow",
    "Chunchun",
    "Dancing Shadows",
};

#endif
uint8_t sound_current;

bool save_to_perform; // used to refresh mood imediately when UI is interacted
                      // with
static lv_obj_t *score_label1, *score_label2, *score_team_label_pl1,
    *score_team_label_pl2;
static lv_obj_t *score_bkgrnd_pl1, *score_bkgrnd_pl2;
uint8_t score_team_pl1, score_team_pl2;

unsigned int score_pl1, score_pl2;
bool score_team_random_gen;
static uint8_t cup_selected, cups_count;
static lv_obj_t *cup_label_selected;

#define CUP_LABELS_SIZE (60)
lv_obj_t *cup_labels[CUP_LABELS_SIZE];

static void toggle_tab(debug_tabs_t *tab, bool enabled)
{
#ifndef SIMULATOR
    tab->enabled = enabled;
    if (Save::save_data.debug_feature_enabled[tab->id] != enabled) {
        Save::save_data.debug_feature_enabled[tab->id] = enabled;
        save_to_perform = true;
    }
#endif
    return;
}

#ifdef SDCARD_ENABLED
struct sd_info_table {
    const char *name;
    lv_obj_t *value;
} sd_info_table[sd_info_rows::count] = {
    {.name = "Inserted?", .value = NULL},
    {.name = "Name", .value = NULL},
    {.name = "Capacity", .value = NULL},
    {.name = "Mount point", .value = NULL},
};
/*
    Create a row container with two labels, left is the name, right is the
   value, and return the value label;
*/
static lv_obj_t *create_kv_row_labels(lv_obj_t *parent, const char *name)
{
    lv_obj_t *h = lv_cont_create(parent, NULL);
    lv_cont_set_layout(h, LV_LAYOUT_PRETTY_MID);
    lv_obj_set_drag_parent(h, true);
    lv_obj_set_auto_realign(h, true);
    lv_obj_add_style(h, LV_CONT_PART_MAIN, &style_row_container);
    lv_cont_set_fit2(h, LV_FIT_MAX, LV_FIT_TIGHT);

    lv_obj_t *left = lv_label_create(h, NULL);
    lv_obj_t *right = lv_label_create(h, NULL);
    lv_label_set_text(left, name);

    return right;
}

bool disk_info_displayed = false;

static void popup(const char *msg)
{
    static const char *btns[] = {"Close", ""};
    lv_obj_t *m = lv_msgbox_create(lv_scr_act(), NULL);
    lv_msgbox_set_text(m, msg);
    lv_msgbox_add_btns(m, btns);
    lv_obj_t *btnm = lv_msgbox_get_btnmatrix(m);
    lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECK_STATE);
}

static char disk_current_path[1024];
static lv_obj_t *disk_list, *disk_explorer, *disk_path_value;

static void disk_enable_event(lv_obj_t *sw, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
        bool enabled = lv_switch_get_state(sw);
        if (enabled == debug_tabs[debug_tab::disk].enabled) {
            break;
        }

        toggle_tab(&debug_tabs[debug_tab::disk], enabled);
        lv_obj_set_hidden(disk_info_container,
                          !debug_tabs[debug_tab::disk].enabled);
        lv_obj_set_hidden(disk_explorer, !debug_tabs[debug_tab::disk].enabled);

        if (enabled) {
            Disk::getInstance().enable();
        } else {
            Disk::getInstance().disable();
            disk_info_displayed = false;

            for (int i = 0; i < sd_info_rows::count; i++) {
                lv_label_set_text(sd_info_table[i].value, "-");
            }
        }
        break;
    }
    }

    return;
}

static void disk_refresh_files();

static void disk_list_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        const char *name = lv_list_get_btn_text(obj);
        bool dir = name[strlen(name) - 1] == '/';

        if (!strcmp("../", name)) {
            for (int i = strlen(disk_current_path) - 2;
                 i > 0 && disk_current_path[i] != '/'; i--)
                disk_current_path[i] = '\0';
            disk_refresh_files();
        } else if (dir) {
            if (strlen(disk_current_path) + strlen(name) + 1 >
                sizeof(disk_current_path)) {
                printf("can't open dir because path is too long");
                popup("Can't open dir because path is too long");
                return;
            }
            strcat(disk_current_path, name);
            disk_refresh_files();
        } else { /* open file */
            int len = strlen(disk_current_path) + strlen(name) + 1;
            char *file_path = (char *)malloc(len);

            snprintf(file_path, len, "%s%s", disk_current_path, name);

            FILE *f = fopen(file_path, "r");
            free(file_path);
            if (f == NULL) {
                ESP_LOGE(TAG, "Failed to open file for reading");
                popup("Failed to open file for reading");
                return;
            }
            char line[64];
            fgets(line, sizeof(line), f);
            fclose(f);
            // strip newline
            char *pos = strchr(line, '\n');
            if (pos) {
                *pos = '\0';
            }

            popup(line);
        }
    }
}

bool disk_iter_cb(dirent *entry, void *param)
{
    char fmt[257];
    bool dir = entry->d_type == DT_DIR;

    if (dir) {
        snprintf((char *)&fmt, sizeof(fmt), "%s/", entry->d_name);
    } else {
        snprintf((char *)&fmt, sizeof(fmt), "%s", entry->d_name);
    }

    lv_obj_t *list_btn = lv_list_add_btn(
        disk_list, dir ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE, fmt);
    lv_obj_set_event_cb(list_btn, disk_list_event_handler);

    return true;
}

static void disk_refresh_files()
{
    char dir[32];
    lv_list_clean(disk_list);

    snprintf((char *)&dir, sizeof(dir), "%s/",
             Disk::getInstance().getMountPoint());

    lv_label_set_text(disk_path_value, disk_current_path);

    lv_obj_t *list_btn = lv_list_add_btn(disk_list, LV_SYMBOL_DIRECTORY, "../");
    lv_obj_set_event_cb(list_btn, disk_list_event_handler);
    if (!strcasecmp(disk_current_path, dir)) {
        lv_obj_set_click(list_btn, false);
        lv_btn_set_state(list_btn, LV_BTN_STATE_DISABLED);
    }

    Disk::getInstance().iterPath(disk_current_path,
                                 (disk_iter_cb_t)disk_iter_cb, NULL);
}

static lv_obj_t *tab_disk_init(debug_tabs_t *tab)
{
    lv_obj_t *h, *sw;
    lv_obj_t *parent = tab->tab = lv_tabview_add_tab(tab_view, tab->name);

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);

    // SD container
    h = create_container(parent);

    // Enable switch
    sw = tab->enable_switch =
        create_switch_with_label(h, "Enabled", tab->enabled);
    lv_obj_set_event_cb(sw, disk_enable_event);

    // Information container
    h = disk_info_container = create_container(parent);
    lv_obj_set_hidden(h, !tab->enabled);
    for (int i = 0; i < sd_info_rows::count; i++) {
        sd_info_table[i].value = create_kv_row_labels(h, sd_info_table[i].name);
        lv_label_set_text(sd_info_table[i].value, "-");
    }

    // File explorer container
    h = disk_explorer = create_container(parent, "Explorer");
    lv_obj_set_hidden(h, true);

    disk_path_value = create_kv_row_labels(h, "Path");

    disk_list = lv_list_create(h, NULL);
    lv_obj_add_style(disk_list, LV_CONT_PART_MAIN, &style_row_container);
    lv_obj_set_size(disk_list, 260, 160);
    lv_obj_align(disk_list, NULL, LV_ALIGN_CENTER, 0, 0);

    return parent;
}
#endif

#ifndef SIMULATOR
static void led_enable_event(lv_obj_t *sw, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
        bool enabled = lv_switch_get_state(sw);
        // bool enabled = true;
        if (enabled == debug_tabs[debug_tab::led].enabled) {
            break;
        }
        if (!NeoPixel::getInstance().getIsOn() && enabled) {
            printf("Start neopixel\n");
            NeoPixel::getInstance().init();
            NeoPixel::getInstance().start();
        }

        Save::save_data.neopixel_is_on = enabled;
        toggle_tab(&debug_tabs[debug_tab::led], enabled);
        lv_obj_set_hidden(led_container, !debug_tabs[debug_tab::led].enabled);
        save_to_perform = true;
        break;
    }
    }

    return;
}

static void led_brightness_event(lv_obj_t *slider, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
        uint8_t brightness = (uint8_t)lv_slider_get_value(slider);
        if (NeoPixel::getInstance().getBrightness() != brightness) {
            NeoPixel::getInstance().setBrightness(brightness);
            save_to_perform = true;
        }

        return;
    }
    }
}
static void backlight_brightness_event(lv_obj_t *slider, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
        uint8_t brightness = (uint8_t)lv_slider_get_value(slider);
        DisplayLedcBacklight::getInstance().setBrightness(brightness);
        save_to_perform = true;
        break;
    }
    }

    return;
}

static void led_mode_event(lv_obj_t *roller, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
        int selected = lv_roller_get_selected(roller);
        if (NeoPixel::getInstance().getMode() != selected) {
            NeoPixel::getInstance().setMode(selected);
            save_to_perform = true;
        }
        break;
    }
    }

    return;
}

static void led_color_event(lv_obj_t *cpicker, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
        lv_color_t color = lv_cpicker_get_color(cpicker);
        uint32_t rgb =
            (color.ch.red << (16 + 3)) |
            ((color.ch.green_l | (color.ch.green_h << 3)) << (8 + 2)) |
            (color.ch.blue << 3);

        NeoPixel::getInstance().setColor(rgb);
        save_to_perform = true;
        break;
    }
    }

    return;
}

#endif


static lv_obj_t *tab_config_init(debug_tabs_t *tab)
{
    lv_obj_t *h, *cpicker;
#ifndef SIMULATOR
    lv_obj_t *sw;
    lv_obj_t *parent = tab->tab = lv_tabview_add_tab(tab_view, tab->name);
#else
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, TITRE_TAB_CONFIG);
#endif
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);
    lv_page_set_scrollbar_mode(parent, LV_SCRLBAR_MODE_DRAG);

#ifndef SIMULATOR
    // container
    h = create_container(parent);
    // Enable switch
    sw = tab->enable_switch =
        create_switch_with_label(h, "Couleur boitier", tab->enabled);
    lv_obj_set_event_cb(sw, led_enable_event);
#else
    h = create_container(parent, NULL, LV_LAYOUT_PRETTY_MID, false);
    create_switch_with_label(h, "Couleur boitier", true);
#endif

    lv_obj_t *led_slider_backlight = lv_slider_create(h, NULL);
    lv_obj_set_width(led_slider_backlight, 250);
    lv_obj_align(led_slider_backlight, NULL, LV_ALIGN_CENTER, 0, 0);
#ifndef SIMULATOR
    lv_obj_set_event_cb(led_slider_backlight, backlight_brightness_event);
#endif
    lv_slider_set_range(led_slider_backlight, 5, 255);
    lv_obj_set_height(led_slider_backlight, 25);

    lv_obj_t *label = lv_label_create(led_slider_backlight, NULL);
    lv_label_set_text(label, "Lumiere Ecran");
    lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);

#ifndef SIMULATOR
    lv_slider_set_value(led_slider_backlight, Save::save_data.display_backlight,
                        LV_ANIM_OFF);
    // lv_slider_set_value(slider, Save::save_data.neopixel_brightness,
    // LV_ANIM_OFF);
#endif

    // Controls container
    h = led_container =
        create_container(parent, NULL, LV_LAYOUT_PRETTY_MID, false);
#ifndef SIMULATOR
    lv_obj_set_hidden(h, !tab->enabled);
#endif
    lv_obj_t *roller = lv_roller_create(h, NULL);
#ifndef SIMULATOR
    lv_obj_add_style(roller, LV_CONT_PART_MAIN, &style_box);
#endif
    // lv_obj_set_style_local_value_str(roller, LV_CONT_PART_MAIN,
    // LV_STATE_DEFAULT, "Choose ambiant mood");
    lv_roller_set_auto_fit(roller, false);
    lv_roller_set_align(roller, LV_LABEL_ALIGN_CENTER);
    lv_roller_set_visible_row_count(roller, 4);
    lv_obj_set_width(roller, 110);
    lv_obj_set_style_local_text_font(roller, LV_LABEL_PART_MAIN,
                                     LV_STATE_DEFAULT, &lv_font_montserrat_12);
#ifndef SIMULATOR
    lv_obj_set_event_cb(roller, led_mode_event);

    char choices[1024];
    memset(choices, 0, sizeof(choices));
    int count =
        sizeof(NeoPixel::unlocked_mode) / sizeof(NeoPixel::unlocked_mode[0]);
    for (int i = 0; i < count; i++) {
        strcat((char *)&choices, FX_mode_names[NeoPixel::unlocked_mode[i]]);
        if (i != count - 1)
            strcat((char *)&choices, "\n");
    }

    lv_roller_set_options(roller, (char *)&choices, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(roller, Save::save_data.neopixel_mode, LV_ANIM_OFF);
#endif

    cpicker = lv_cpicker_create(h, NULL);
    lv_obj_set_size(cpicker, 130, 150);
    lv_obj_align(cpicker, NULL, LV_ALIGN_CENTER, 0, 0);
#ifndef SIMULATOR
    lv_obj_set_event_cb(cpicker, led_color_event);
    lv_color_t c = lv_color_hex(Save::save_data.neopixel_color);
    lv_cpicker_set_color(cpicker, c);
#endif
    lv_obj_t *slider = lv_slider_create(h, NULL);
    lv_obj_set_width(slider, 250);
    lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);
#ifndef SIMULATOR
    lv_obj_set_event_cb(slider, led_brightness_event);
    lv_slider_set_range(slider, 0, 255);
    lv_slider_set_value(slider, Save::save_data.neopixel_brightness,
                        LV_ANIM_OFF);
#endif
    return parent;
}

static void score_handler_btn1(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        char varname[10];
        score_pl1 += 1;
        sprintf(varname, "%d", score_pl1);
        lv_label_set_text(score_label1, varname);
        break;
    }
    }

    return;
}
static void score_handler_btn2(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        char varname[10];
        score_pl2 += 1;
        sprintf(varname, "%d", score_pl2);
        lv_label_set_text(score_label2, varname);

        break;
    }
    }

    return;
}

static void score_update_teams()
{
    ESP_LOGE(TAG, "pl1 %d, pl2 %d\n", score_team_pl1, score_team_pl2);
    lv_label_set_text(score_team_label_pl1, tournaments[cup_selected]->teams[score_team_pl1]);
    lv_label_set_text(score_team_label_pl2, tournaments[cup_selected]->teams[score_team_pl2]);


    uint8_t index_team_color_pl1,index_team_color_pl2;
    index_team_color_pl1 = score_team_pl1 % teams_color_size;
    index_team_color_pl2 = score_team_pl2 % teams_color_size;
    lv_obj_set_style_local_bg_color(score_bkgrnd_pl1, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, teams_color[index_team_color_pl1]);
    lv_obj_set_style_local_bg_color(score_bkgrnd_pl2, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, teams_color[index_team_color_pl2]);

}

static void score_change_team_pl1(lv_obj_t *btn, lv_event_t event)
{

    switch (event) {
    case LV_EVENT_CLICKED: {
        score_team_pl1 = (score_team_pl1 + 1) % tournaments[cup_selected]->number_teams;
        score_update_teams();
        break;
    }
    }

    return;
}
static void score_change_team_pl2(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        score_team_pl2 = (score_team_pl2 + 1) % tournaments[cup_selected]->number_teams;
        score_update_teams();
        break;
    }
    }
}

static void score_replace_teams()
{
    if (score_team_random_gen == false) {
        srand(time(NULL)); // Initialization, should only be called once.
        score_team_random_gen = true;
    }

    score_team_pl1 = random() % tournaments[cup_selected]->number_teams;
    score_team_pl2 = random() % tournaments[cup_selected]->number_teams;
    while (score_team_pl1 == score_team_pl2) {
        score_team_pl2 = random() % tournaments[cup_selected]->number_teams;
    }
    score_update_teams();
}

static void score_reset(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        score_pl2 = score_pl1 = 0;
        lv_label_set_text(score_label1, "0");
        lv_label_set_text(score_label2, "0");
        score_replace_teams();
        break;
    }
    }

    return;
}
static void score_fin(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
#ifndef SIMULATOR
        Buzzer::getInstance().buzz(2500, 200);
        vTaskDelay(pdMS_TO_TICKS(300));
        Buzzer::getInstance().buzz(2500, 200);
        vTaskDelay(pdMS_TO_TICKS(300));
        Buzzer::getInstance().buzz(2500, 200);
#endif
        break;
    }
    }

    return;
}
static void score_debut(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
#ifndef SIMULATOR
        Buzzer::getInstance().buzz(2500, 300);
#endif
        break;
    }
    }

    return;
}


static lv_obj_t *tab_score_init(debug_tabs_t *tab)
{
    lv_obj_t *h;
#ifdef SIMULATOR
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, TITRE_TAB_SCORE);
#else
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, tab->name);
#endif
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);
    // Controls container
    lv_page_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_page_set_scroll_propagation(parent, false);

    // Align content from middle or top for the 2nd line
    h = score_controls_container =
        create_container(parent, NULL, LV_LAYOUT_PRETTY_MID, true);
    //  h = score_controls_container = create_container(parent, NULL,     LV_LAYOUT_GRID, true);

    // Reduire l'espace entre les composants au minimum
    lv_obj_set_style_local_pad_inner(h, LV_OBJ_PART_MAIN,
                                   LV_STATE_DEFAULT, 2);


    lv_coord_t object_width = 156;
    // object_width = 100;
    object_width = 140;
    static lv_coord_t small_btn_width = 50;
    static lv_coord_t small_btn_height = 80;

    lv_obj_t *btn1 = lv_btn_create(h, NULL);
    lv_obj_set_event_cb(btn1, score_handler_btn1);
    lv_obj_set_width(btn1, object_width);
    lv_obj_set_height(btn1, 60);

    score_label1 = lv_label_create(btn1, NULL);
    lv_label_set_text(score_label1, "0");
    lv_label_set_align(score_label1, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(score_label1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_text_font(score_label1, LV_LABEL_PART_MAIN,
                                     LV_STATE_DEFAULT, &lv_font_montserrat_48);

    lv_obj_t *btn2 = lv_btn_create(h, NULL);
    lv_obj_set_event_cb(btn2, score_handler_btn2);
    lv_obj_set_width(btn2, object_width);
    lv_obj_set_height(btn2, 60);

    score_label2 = lv_label_create(btn2, NULL);
    lv_label_set_text(score_label2, "0");
    // lv_label_set_align(score_label2, LV_LABEL_ALIGN_CENTER);
    // lv_obj_align(score_label2, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_text_font(score_label2, LV_LABEL_PART_MAIN,
                                     LV_STATE_DEFAULT, &lv_font_montserrat_48);

    // Create a background object to set a label color
    score_bkgrnd_pl1 = lv_obj_create(h, NULL);
    lv_obj_set_style_local_bg_color(score_bkgrnd_pl1, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_set_width(score_bkgrnd_pl1, object_width);
    lv_obj_set_style_local_border_width(score_bkgrnd_pl1, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);


    score_team_label_pl1 = lv_label_create(score_bkgrnd_pl1, NULL);
    lv_obj_set_style_local_pad_top(score_team_label_pl1, LV_OBJ_PART_MAIN,
                                   LV_STATE_DEFAULT, 13);

    lv_label_set_align(score_team_label_pl1, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(score_team_label_pl1, LV_LABEL_LONG_SROLL_CIRC);
    lv_label_set_text(score_team_label_pl1, "-");
    lv_obj_set_width(score_team_label_pl1, object_width);
    lv_obj_set_style_local_text_color(score_team_label_pl1, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, LV_COLOR_BLACK);

    // Create a background object to set a label color 
    score_bkgrnd_pl2 = lv_obj_create(h, NULL);
    lv_obj_set_style_local_bg_color(score_bkgrnd_pl2, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_obj_set_width(score_bkgrnd_pl2, object_width);
    lv_obj_set_style_local_border_width(score_bkgrnd_pl2, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);

    score_team_label_pl2 = lv_label_create(score_bkgrnd_pl2, NULL);
    lv_label_set_align(score_team_label_pl2, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(score_team_label_pl2, LV_LABEL_LONG_SROLL_CIRC);
    lv_label_set_text(score_team_label_pl2, "-");
    // Ne fonctionne pas pour utiliser un alignement verticale
    // lv_obj_align(score_team_label_pl2, NULL, LV_ALIGN_CENTER, -0, 0);
    lv_obj_set_height(score_team_label_pl2, 50);
    lv_obj_set_style_local_pad_top(score_team_label_pl2, LV_OBJ_PART_MAIN,
                                   LV_STATE_DEFAULT, 13);
    lv_obj_set_width(score_team_label_pl2, object_width);
    lv_obj_set_style_local_text_color(score_team_label_pl2, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, LV_COLOR_BLACK);


    lv_obj_t *btn_debut = lv_btn_create(h, NULL);
    lv_obj_set_event_cb(btn_debut, score_debut);
    lv_obj_set_height(btn_debut, small_btn_height);
    lv_obj_set_width(btn_debut, small_btn_width + 10);
    lv_obj_t *label_local_debut = lv_label_create(btn_debut, NULL);
    lv_label_set_text(label_local_debut, "DEBUT");

    lv_obj_t *btn_fin = lv_btn_create(h, NULL);
    lv_obj_set_event_cb(btn_fin, score_fin);
    lv_obj_set_height(btn_fin, small_btn_height);
    lv_obj_set_width(btn_fin, small_btn_width - 10);
    lv_obj_t *label_local2 = lv_label_create(btn_fin, NULL);
    lv_label_set_text(label_local2, "FIN");

    lv_obj_t *btn_reset = lv_btn_create(h, NULL);
    lv_obj_set_event_cb(btn_reset, score_reset);
    lv_obj_set_height(btn_reset, small_btn_height);
    lv_obj_set_width(btn_reset, small_btn_width);

    lv_obj_t *label_local = lv_label_create(btn_reset, NULL);
    lv_label_set_text(label_local, "0-0");

    lv_obj_t *btn_sel_team_pl1 = lv_btn_create(h, NULL);
    lv_obj_set_event_cb(btn_sel_team_pl1, score_change_team_pl1);
    lv_obj_set_height(btn_sel_team_pl1, small_btn_height);
    lv_obj_set_width(btn_sel_team_pl1, small_btn_width);
    lv_obj_t *label_sel_team_pl1 = lv_label_create(btn_sel_team_pl1, NULL);
    lv_label_set_text(label_sel_team_pl1, "E1");

    lv_obj_t *btn_sel_team_pl2 = lv_btn_create(h, NULL);
    lv_obj_set_event_cb(btn_sel_team_pl2, score_change_team_pl2);
    lv_obj_set_height(btn_sel_team_pl2, small_btn_height);
    lv_obj_set_width(btn_sel_team_pl2, small_btn_width);

    lv_obj_t *label_sel_team_pl2 = lv_label_create(btn_sel_team_pl2, NULL);
    lv_label_set_text(label_sel_team_pl2, "E2");

    // bouton position absolue
    // lv_obj_t *btn_debut2 = lv_btn_create(lv_scr_act(), NULL);
    // lv_obj_set_x(btn_debut2, 270);
    // lv_obj_set_y(btn_debut2,200);

    // score_replace_teams();
    score_team_pl1 = 0;
    score_team_pl2 = 0;

    return parent;
}


static void cup_update_displayed_teams(void){
    for (int i = 0; i < CUP_LABELS_SIZE-1; i++) {
        if (i < tournaments[cup_selected]->number_teams){
            lv_label_set_text(cup_labels[i], tournaments[cup_selected]->teams[i]);
        }
        else {
            lv_label_set_text(cup_labels[i], "");
        }
    }
}

static void cup_handler(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        cup_selected = (cup_selected+1)%cups_count;
        printf("Cup selected:%d\n",cup_selected);
        lv_label_set_text(cup_label_selected, tournaments[cup_selected]->title);
        printf("Teams size:%u\n",tournaments[cup_selected]->number_teams);
        score_team_pl1 = 0;
        score_team_pl2 = 0;
        lv_label_set_text(score_team_label_pl1, "-");
        lv_label_set_text(score_team_label_pl2, "-");

        cup_update_displayed_teams();
#ifndef SIMULATOR
        Save::save_data.cup = cup_selected;
#endif
        save_to_perform = true;
        break;
    }

    }
    return;
}




static lv_obj_t *tab_cup_init(debug_tabs_t *tab)
{
    const uint8_t obj_height = 40;
#ifdef SIMULATOR
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, TITRE_TAB_CUP);
#else
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, tab->name);
#endif
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);

    lv_obj_t *cup_label_title = lv_label_create(parent, NULL);
    lv_label_set_text(cup_label_title, "Coupe");
    lv_label_set_align(cup_label_title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(cup_label_title, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *cup_button = lv_btn_create(parent, NULL);
    // bouton position absolue si besoin maus sans set_scrl_layout
    // lv_obj_set_x(cup_button, 160);
    // lv_obj_set_y(cup_button, 0);
    lv_obj_set_width(cup_button, 200);
    lv_obj_set_style_local_radius(cup_button,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_height(cup_button, obj_height);
    lv_obj_set_event_cb(cup_button, cup_handler);

    cup_label_selected = lv_label_create(cup_button, NULL);
    lv_label_set_text(cup_label_selected, "0");
    lv_label_set_align(cup_label_selected, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(cup_label_selected, NULL, LV_ALIGN_CENTER, 0, 0);

#ifndef SIMULATOR
    cup_selected = Save::save_data.cup;
#endif
    cups_count = sizeof(tournaments) / sizeof(tournaments[0]);
    lv_label_set_text(cup_label_selected,
                              tournaments[cup_selected]->title);

    for (int i = 0; i < CUP_LABELS_SIZE; i++) {
        cup_labels[i] = lv_label_create(parent, NULL);
        lv_label_set_text(cup_labels[i], "");
        lv_obj_set_style_local_text_font(cup_labels[i], LV_LABEL_PART_MAIN,
                                     LV_STATE_DEFAULT, &lv_font_montserrat_12);

    }
    cup_update_displayed_teams();

    return parent;
}



static void cup_sounds_handler(lv_obj_t *btn, lv_event_t event, uint8_t snd_index)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        ESP_LOGE(TAG, "Now playing %u\n", snd_index);
#ifndef SIMULATOR
        Buzzer::getInstance().play(static_cast<Buzzer::Sounds>(snd_index));
#endif
        break;
    }
    }
    return;
}

#pragma GCC diagnostic ignored "-Wunused-function"
static void cup_sounds_handler_00(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 0);
}

static void cup_sounds_handler_01(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 1);
}

static void cup_sounds_handler_02(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 2); 
}

static void cup_sounds_handler_03(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 3);
}

static void cup_sounds_handler_04(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 4);
}

static void cup_sounds_handler_05(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 5);
}

static void cup_sounds_handler_06(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 6);
}

static void cup_sounds_handler_07(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 7);
}

static void cup_sounds_handler_08(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 8);
}

static void cup_sounds_handler_09(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 9);
}

static void cup_sounds_handler_10(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 10);
}

static void cup_sounds_handler_11(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 11);
}

static void cup_sounds_handler_12(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 12);
}

static void cup_sounds_handler_13(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 13);
}

static void cup_sounds_handler_14(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 14);
}

static void cup_sounds_handler_15(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 15);
}

static void cup_sounds_handler_16(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 16);
}

static void cup_sounds_handler_17(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 17);
}

static void cup_sounds_handler_18(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 18);
}

static void cup_sounds_handler_19(lv_obj_t *btn, lv_event_t event)
{
    cup_sounds_handler(btn, event, 19);
}

const char *cup_sounds_name[] = {"Beep","Buzz","Connection","Disconnection","ButtonPushed","Mode1","Mode2","Mode3","Surprise","OhOoh","Cuddly","Sleeping","Happy","SuperHappy","HappyShort","Sad","ImportantNotice","LevelUp","LevelDown"};

static lv_event_cb_t cup_sounds_handlers[20] = {
    cup_sounds_handler_01, cup_sounds_handler_02, cup_sounds_handler_03,
    cup_sounds_handler_04, cup_sounds_handler_05, cup_sounds_handler_06, cup_sounds_handler_07,
    cup_sounds_handler_08, cup_sounds_handler_09, cup_sounds_handler_10, cup_sounds_handler_11,
    cup_sounds_handler_12, cup_sounds_handler_13, cup_sounds_handler_14, cup_sounds_handler_15,
    cup_sounds_handler_16, cup_sounds_handler_17, cup_sounds_handler_18, cup_sounds_handler_19
};

static lv_obj_t *tab_sounds_init(debug_tabs_t *tab)
{
#ifdef SIMULATOR
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, TITRE_TAB_SOUNDS);
#else
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, tab->name);
#endif
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);

    // Create buttons with counter in names and different event handlers
    for (int i = 0; i < 19; i++) {
        lv_obj_t *btn = lv_btn_create(parent, NULL);
        lv_obj_set_style_local_radius(btn,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
        lv_obj_set_width(btn, 60);
        
        // Set the event handler using the function pointer array
        lv_obj_set_event_cb(btn, cup_sounds_handlers[i]);

        lv_obj_t *label = lv_label_create(btn, NULL);
        lv_label_set_text(label, cup_sounds_name[i]);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL);
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    }

    return parent;
}

void screen_debug_init()
{
    score_pl1 = 0; 
    score_pl2 = 0;

    score_team_random_gen = false;

    tab_view = lv_tabview_create(lv_scr_act(), NULL);
    // // Reduce size of Tabs titles
    lv_obj_set_style_local_pad_top(tab_view, LV_TABVIEW_PART_TAB_BG,
                                   LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_bottom(tab_view, LV_TABVIEW_PART_TAB_BG,
                                      LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_left(tab_view, LV_TABVIEW_PART_TAB_BG,
                                    LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_right(tab_view, LV_TABVIEW_PART_TAB_BG,
                                     LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_margin_bottom(tab_view, LV_TABVIEW_PART_TAB_BG,
                                         LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_margin_top(tab_view, LV_TABVIEW_PART_TAB_BG,
                                      LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_margin_left(tab_view, LV_TABVIEW_PART_BG,
                                       LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_margin_right(tab_view, LV_TABVIEW_PART_BG,
                                        LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_pad_top(tab_view, LV_TABVIEW_PART_TAB_BTN,
                                   LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_pad_bottom(tab_view, LV_TABVIEW_PART_TAB_BTN,
                                      LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_margin_top(tab_view, LV_TABVIEW_PART_TAB_BTN,
                                      LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_margin_bottom(tab_view, LV_TABVIEW_PART_TAB_BTN,
                                         LV_STATE_DEFAULT, 0);

#ifndef SIMULATOR
    for (int i = 0; i < debug_tab::count; i++) {
        debug_tabs[i].id = i;
        debug_tabs[i].enabled = Save::save_data.debug_feature_enabled[i];
    }

    for (int i = 0; i < debug_tab::count; i++) {
        debug_tabs[i].init(&debug_tabs[i]);
    }
#ifdef SDCARD_ENABLED
    if (debug_tabs[debug_tab::disk].enabled) {
        Disk::getInstance().enable();
    }
#endif

    DisplayLedcBacklight::getInstance().start();
#else
    util_styles_init();
    tab_score_init(NULL);
    tab_cup_init(NULL);
    tab_sounds_init(NULL);
    tab_config_init(NULL);

#endif

    return;
}

#ifndef SIMULATOR

void screen_debug_loop()
{
    if (save_to_perform) {

        TickType_t now = xTaskGetTickCount();
        TickType_t elapsed_ticks = now - last_save_at;
        uint32_t elapsed_time_s =
            (uint32_t)((elapsed_ticks * 1000) / configTICK_RATE_HZ) / 1000;

        if (elapsed_time_s > (10)) {
            printf("Save settings\n");
            Save::write_save();
            save_to_perform = false;
            last_save_at = now;
        }
    }

#ifdef SDCARD_ENABLED

    if (debug_tabs[debug_tab::disk].enabled) {
        if (!disk_info_displayed &&
            Disk::getInstance().getCardState() == Disk::CardState::Present) {
            sdmmc_card_t *card = Disk::getInstance().getCardInfo();
            char name[10];

            snprintf((char *)&name, sizeof(name), card->cid.name);

            lv_label_set_text(sd_info_table[sd_info_rows::inserted].value,
                              "Yes");
            lv_label_set_text(sd_info_table[sd_info_rows::name].value, name);
            lv_label_set_text_fmt(sd_info_table[sd_info_rows::capacity].value,
                                  "%lluMB",
                                  ((uint64_t)card->csd.capacity) *
                                      card->csd.sector_size / (1024 * 1024));

            snprintf((char *)&disk_current_path, sizeof(disk_current_path),
                     "%s/", Disk::getInstance().getMountPoint());
            lv_label_set_text(sd_info_table[sd_info_rows::mount].value,
                              Disk::getInstance().getMountPoint());

            disk_refresh_files();
            lv_obj_set_hidden(disk_explorer, false);

            disk_info_displayed = true;
        }

        if (disk_info_displayed &&
            Disk::getInstance().getCardState() != Disk::CardState::Present) {
            lv_label_set_text(sd_info_table[sd_info_rows::inserted].value,
                              "No");
            lv_label_set_text(sd_info_table[sd_info_rows::mount].value, "-");

            lv_obj_set_hidden(disk_explorer, true);

            disk_info_displayed = false;
        }
    }
#endif
    return;
}
#endif
