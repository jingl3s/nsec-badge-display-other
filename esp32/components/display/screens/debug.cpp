#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
#include "screens/image_viewer.h"
#include "screens/score_teams.h"
#else
#include "debug.h"
#include "image_viewer.h"
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

static void card_teams_set(void);

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
static lv_obj_t *disk_info_container;
static char disk_current_path[1024];
static lv_obj_t *disk_list, *disk_explorer, *disk_path_value;

static lv_obj_t *tab_config_init(debug_tabs_t *tab);
static lv_obj_t *tab_images_init(debug_tabs_t *tab);
static lv_obj_t *tab_score_init(debug_tabs_t *tab);
static lv_obj_t *tab_cup_init(debug_tabs_t *tab);
static lv_obj_t *tab_card_init(debug_tabs_t *tab);
static lv_obj_t *tab_sounds_init(debug_tabs_t *tab);
static lv_obj_t *tab_chrono_init(debug_tabs_t *tab);

static long countdown_seconds = 0;
static bool countdown_running = false;
static lv_obj_t *label_chrono;
static lv_obj_t *label_start_stop;
static lv_obj_t *chrono_parent = NULL;

static void update_chrono_label()
{
    if (label_chrono) {
        int m = countdown_seconds / 60;
        int s = countdown_seconds % 60;
        lv_label_set_text_fmt(label_chrono, "%02d:%02d", m, s);
    }
}

static void chrono_add_time_event_handler(lv_obj_t *obj, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {
        int seconds = (int)(intptr_t)lv_obj_get_user_data(obj);
        countdown_seconds += seconds;
        update_chrono_label();
    }
}

static void chrono_start_stop_event_handler(lv_obj_t *obj, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {
        countdown_running = !countdown_running;
        lv_label_set_text(label_start_stop,
                          countdown_running ? "STOP" : "START");
        // if (countdown_running) {
        //     NeoPixel::getInstance().setColor(0x000000); // Red
        // }
    }
}

static void chrono_reset_event_handler(lv_obj_t *obj, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {
        countdown_running = false;
        countdown_seconds = 0;
        lv_label_set_text(label_start_stop, "START");
        update_chrono_label();
        // Reset screen color if it was changed
        if (chrono_parent) {
            lv_obj_set_style_local_bg_color(chrono_parent, LV_PAGE_PART_BG,
                                            LV_STATE_DEFAULT, LV_COLOR_WHITE);
        }
    }
}

static const char *TITRE_TAB_SCORE = "SCORE";
static const char *TITRE_TAB_CUP = "Co.";
static const char *TITRE_TAB_CARD = "Cart.";
static const char *TITRE_TAB_CONFIG = "CFG";
static const char *TITRE_TAB_SOUNDS = "SO.";
static const char *TITRE_TAB_CHRONO = "Ch.";
static const char *TITRE_TAB_IMAGES = "IMG";

#ifndef SIMULATOR
static TickType_t last_save_at =
    0; // used to keep sending beacons every 10 seconds
static TickType_t match_start_tick = 0;

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
debug_tabs_t debug_tabs[debug_tab::count] = {
    {.name = TITRE_TAB_SCORE, .init = tab_score_init},
    {.name = TITRE_TAB_CUP, .init = tab_cup_init},
    {.name = TITRE_TAB_CARD, .init = tab_card_init},
    {.name = TITRE_TAB_CHRONO, .init = tab_chrono_init},
    {.name = TITRE_TAB_SOUNDS, .init = tab_sounds_init},
    {.name = TITRE_TAB_CONFIG, .init = tab_config_init},
#ifdef SDCARD_ENABLED
    {.name = "---", .init = NULL},
    {.name = TITRE_TAB_IMAGES, .init = tab_images_init},
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
static lv_obj_t *score_team_bkgrnd_pl1, *score_team_bkgrnd_pl2;
static lv_obj_t *score_label_cart_yellow_team_pl1,
    *score_label_cart_yellow_team_pl2;
static lv_obj_t *score_label_cart_red_team_pl1, *score_label_cart_red_team_pl2;
static lv_obj_t *score_label_cart_red_team_bkgrnd_pl1,
    *score_label_cart_red_team_bkgrnd_pl2;
static lv_obj_t *score_label_cart_yellow_team_bkgrnd_pl1,
    *score_label_cart_yellow_team_bkgrnd_pl2;

uint8_t score_team_pl1, score_team_pl2;
uint8_t index_team_color_pl1, index_team_color_pl2;

unsigned int score_pl1, score_pl2;
bool score_team_random_gen;
static uint8_t cup_selected, cup_selected_2, cups_count;
static lv_obj_t *cup_label_selected, *cup_label_selected_2;

#define CUP_LABELS_SIZE (60)
lv_obj_t *cup_labels[CUP_LABELS_SIZE];

static lv_obj_t *card_team_bkgrnd, *card_team_label,
    *card_player_number_text_area;
static lv_obj_t *card_team1_bkgrnd, *card_team1_label, *card_team2_bkgrnd,
    *card_team2_label;
#define CARD_YELLOW_PLAYERS_SIZE 30
static lv_obj_t *card_yellow_team1_textarea, *card_yellow_team2_textarea;
char *card_yellow_players[2][CARD_YELLOW_PLAYERS_SIZE];
char *card_yellow_players_regrouped[2];
uint8_t card_yellow_players_counter[2];
uint8_t card_index_team;
#define CARD_RED_PLAYERS_SIZE 6
static lv_obj_t *card_red_team1_textarea, *card_red_team2_textarea;
char *card_red_players[2][CARD_RED_PLAYERS_SIZE];
char *card_red_players_regrouped[2];
uint8_t card_red_players_counter[2];

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

static void popup(const char *msg)
{
    static const char *btns[] = {"Close", ""};
    lv_obj_t *m = lv_msgbox_create(lv_scr_act(), NULL);
    lv_msgbox_set_text(m, msg);
    lv_msgbox_add_btns(m, btns);
    lv_obj_t *btnm = lv_msgbox_get_btnmatrix(m);
    lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECK_STATE);
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

// One-shot reload of the latest saved match at boot. The SD card is mounted
// asynchronously, so the reload is deferred to screen_debug_loop() and runs
// on the first card detection.
static bool match_boot_load_pending = true;

static void disk_enable_event(lv_obj_t *sw, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
        bool enabled = lv_switch_get_state(sw);
        if (enabled) {
            Disk::getInstance().enable();
        } else {
            Disk::getInstance().disable();
            disk_info_displayed = false;

            for (int i = 0; i < sd_info_rows::count; i++) {
                lv_label_set_text(sd_info_table[i].value, "-");
            }
        }

        // Persist the SD switch state; screen_debug_init() restores it and
        // re-enables Disk from sd_enabled on next start.
        Save::save_data.sd_enabled = enabled;
        save_to_perform = true;

        // lv_obj_set_hidden(disk_info_container,
        //                   !debug_tabs[debug_tab::disk].enabled);
        // lv_obj_set_hidden(disk_explorer,
        // !debug_tabs[debug_tab::disk].enabled);

        // if (enabled) {
        // Disk::getInstance().enable();
        // } else {
        //     Disk::getInstance().disable();
        //     disk_info_displayed = false;

        //     for (int i = 0; i < sd_info_rows::count; i++) {
        //         lv_label_set_text(sd_info_table[i].value, "-");
        //     }
        // }
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

static void save_match_to_sd()
{
    // A match is being recorded: a late SD card insertion must not clobber
    // the current state with the previously saved match.
    match_boot_load_pending = false;

    if (Disk::getInstance().getCardState() != Disk::CardState::Present) {
        return;
    }

    // The SD subsystem must be enabled for the FAT mount to be valid
    if (!Disk::getInstance().isEnabled()) {
        return;
    }

    char file_path[80];
    snprintf(file_path, sizeof(file_path), "%s/matches.tsv",
             Disk::getInstance().getMountPoint());

    bool write_header = false;
    FILE *check = fopen(file_path, "r");
    if (check == NULL) {
        write_header = true;
    } else {
        fclose(check);
    }

    FILE *f = fopen(file_path, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open %s for writing", file_path);
        popup("Erreur: impossible d'ecrire sur la carte SD");
        return;
    }

    if (write_header) {
        fprintf(f, "cup1\tcup2\tteam1\tteam2\tscore1\tscore2\tduration\tyellow_"
                   "team1\tyellow_team2\tred_team1\tred_team2\n");
    }

    const char *team1_name = tournaments[cup_selected]->teams[score_team_pl1];
    const char *team2_name = tournaments[cup_selected_2]->teams[score_team_pl2];

    char duration_str[16] = "";
    if (match_start_tick != 0) {
        uint32_t elapsed_s =
            (uint32_t)(((xTaskGetTickCount() - match_start_tick) * 1000ULL) /
                       configTICK_RATE_HZ) /
            1000;
        snprintf(duration_str, sizeof(duration_str), "%02u:%02u",
                 (unsigned int)(elapsed_s / 60),
                 (unsigned int)(elapsed_s % 60));
    }

    fprintf(f, "%s\t%s\t%s\t%s\t%u\t%u\t%s\t", tournaments[cup_selected]->title,
            tournaments[cup_selected_2]->title, team1_name, team2_name,
            score_pl1, score_pl2, duration_str);

    /* yellow cards team 1 */
    for (uint8_t i = 0; i < card_yellow_players_counter[0]; i++) {
        if (i > 0)
            fprintf(f, ",");
        fprintf(f, "#%s", card_yellow_players[0][i]);
    }
    fprintf(f, "\t");

    /* yellow cards team 2 */
    for (uint8_t i = 0; i < card_yellow_players_counter[1]; i++) {
        if (i > 0)
            fprintf(f, ",");
        fprintf(f, "#%s", card_yellow_players[1][i]);
    }
    fprintf(f, "\t");

    /* red cards team 1 */
    for (uint8_t i = 0; i < card_red_players_counter[0]; i++) {
        if (i > 0)
            fprintf(f, ",");
        fprintf(f, "#%s", card_red_players[0][i]);
    }
    fprintf(f, "\t");

    /* red cards team 2 */
    for (uint8_t i = 0; i < card_red_players_counter[1]; i++) {
        if (i > 0)
            fprintf(f, ",");
        fprintf(f, "#%s", card_red_players[1][i]);
    }
    fprintf(f, "\n");

    fclose(f);
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

#ifndef SIMULATOR
static void bip_enable_event(lv_obj_t *sw, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
        Save::save_data.bip_enabled = lv_switch_get_state(sw);
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

#ifndef SIMULATOR
    // Bip tactile switch
    lv_obj_t *bip_sw =
        create_switch_with_label(h, "Bip tactile", Save::save_data.bip_enabled);
    lv_obj_set_event_cb(bip_sw, bip_enable_event);
#else
    create_switch_with_label(h, "Bip tactile", true);
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

    // ---- SD section (merged from former tab_disk_init) ----
#ifdef SDCARD_ENABLED
    // debug_tabs_t *sd = &debug_tabs[debug_tab::disk];
    lv_obj_t *sw_label;
    // SD enable switch
    h = create_container(parent);
    sw_label = create_switch_with_label(h, "Sauve carte SD", Save::save_data.sd_enabled);
    lv_obj_set_event_cb(sw_label, disk_enable_event);

    // SD info container
    // h = disk_info_container = create_container(parent);
    disk_info_container = h;
    // lv_obj_set_hidden(h, !sd->enabled);
    for (int i = 0; i < sd_info_rows::count; i++) {
        sd_info_table[i].value = create_kv_row_labels(h, sd_info_table[i].name);
        lv_label_set_text(sd_info_table[i].value, "-");
    }

    // File explorer container
    h = disk_explorer = create_container(parent);
    // h = disk_explorer = h;
    // lv_obj_set_hidden(h, !sd->enabled);
    disk_path_value = create_kv_row_labels(h, "Path");

    disk_list = lv_list_create(h, NULL);
    lv_obj_add_style(disk_list, LV_CONT_PART_MAIN, &style_row_container);
    lv_obj_set_size(disk_list, 260, 160);
    lv_obj_align(disk_list, NULL, LV_ALIGN_CENTER, 0, 0);
#elif defined(SIMULATOR)
    // SD widgets in the simulator are static (no Disk backend).
    h = create_container(parent, NULL, LV_LAYOUT_PRETTY_MID, false);
    create_switch_with_label(h, "SD", true);

    h = disk_explorer =
        create_container(parent, "Explorer", LV_LAYOUT_PRETTY_MID, false);
    disk_list = lv_list_create(h, NULL);
    lv_obj_set_size(disk_list, 260, 160);
    lv_obj_align(disk_list, NULL, LV_ALIGN_CENTER, 0, 0);
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
#ifndef SIMULATOR
#ifdef SDCARD_ENABLED
        save_match_to_sd();
#endif
#endif
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
#ifndef SIMULATOR
#ifdef SDCARD_ENABLED
        save_match_to_sd();
#endif
#endif
        break;
    }
    }
    return;
}

static void score_update_teams_pl1()
{
    ESP_LOGE(TAG, "pl1 %d\n", score_team_pl1);
    lv_label_set_text(score_team_label_pl1,
                      tournaments[cup_selected]->teams[score_team_pl1]);
    lv_label_set_text(card_team_label,
                      tournaments[cup_selected]->teams[score_team_pl1]);

    index_team_color_pl1 = score_team_pl1 % teams_color_size;
    lv_obj_set_style_local_bg_color(score_team_bkgrnd_pl1, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    teams_color[index_team_color_pl1]);
    lv_obj_set_style_local_bg_color(card_team_bkgrnd, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    teams_color[index_team_color_pl1]);
    card_teams_set();
}

static void score_update_teams_pl2()
{
    ESP_LOGE(TAG, "pl2 %d\n", score_team_pl2);
    lv_label_set_text(score_team_label_pl2,
                      tournaments[cup_selected_2]->teams[score_team_pl2]);

    index_team_color_pl2 = score_team_pl2 % teams_color_size;
    lv_obj_set_style_local_bg_color(score_team_bkgrnd_pl2, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    teams_color[index_team_color_pl2]);
    card_teams_set();
}

static void score_change_team_pl1(lv_obj_t *btn, lv_event_t event)
{

    switch (event) {
    case LV_EVENT_CLICKED: {
        score_team_pl1 =
            (score_team_pl1 + 1) % tournaments[cup_selected]->number_teams;
        score_update_teams_pl1();
        break;
    }
    }

    return;
}
static void score_change_team_pl2(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        score_team_pl2 =
            (score_team_pl2 + 1) % tournaments[cup_selected_2]->number_teams;
        score_update_teams_pl2();
        break;
    }
    }
}

static void score_card_update_event_handler()
{
    char varname[7];

    if (card_red_players_counter[0] == 0) {
        lv_label_set_text(score_label_cart_red_team_pl1, "");
        lv_obj_set_style_local_bg_color(score_label_cart_red_team_bkgrnd_pl1,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        LV_COLOR_WHITE);
    } else {
        snprintf(varname, sizeof(varname), "%d", card_red_players_counter[0]);
        lv_label_set_text(score_label_cart_red_team_pl1, varname);
        lv_obj_set_style_local_bg_color(score_label_cart_red_team_bkgrnd_pl1,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        LV_COLOR_RED);
    }

    if (card_red_players_counter[1] == 0) {
        lv_label_set_text(score_label_cart_red_team_pl2, "");
        lv_obj_set_style_local_bg_color(score_label_cart_red_team_bkgrnd_pl2,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        LV_COLOR_WHITE);
    } else {
        snprintf(varname, sizeof(varname), "%d", card_red_players_counter[1]);
        lv_label_set_text(score_label_cart_red_team_pl2, varname);
        lv_obj_set_style_local_bg_color(score_label_cart_red_team_bkgrnd_pl2,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        LV_COLOR_RED);
    }

    if (card_yellow_players_counter[0] == 0) {
        lv_label_set_text(score_label_cart_yellow_team_pl1, "");
        lv_obj_set_style_local_bg_color(score_label_cart_yellow_team_bkgrnd_pl1,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        LV_COLOR_WHITE);
    } else {
        snprintf(varname, sizeof(varname), "%d",
                 card_yellow_players_counter[0]);
        lv_label_set_text(score_label_cart_yellow_team_pl1, varname);
        lv_obj_set_style_local_bg_color(score_label_cart_yellow_team_bkgrnd_pl1,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        LV_COLOR_YELLOW);
    }

    if (card_yellow_players_counter[1] == 0) {
        lv_label_set_text(score_label_cart_yellow_team_pl2, "");
        lv_obj_set_style_local_bg_color(score_label_cart_yellow_team_bkgrnd_pl2,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        LV_COLOR_WHITE);
    } else {
        snprintf(varname, sizeof(varname), "%d",
                 card_yellow_players_counter[1]);
        lv_label_set_text(score_label_cart_yellow_team_pl2, varname);
        lv_obj_set_style_local_bg_color(score_label_cart_yellow_team_bkgrnd_pl2,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        LV_COLOR_YELLOW);
    }
}

static void score_replace_teams()
{
    if (score_team_random_gen == false) {
        srand(time(NULL)); // Initialization, should only be called once.
        score_team_random_gen = true;
    }

    score_team_pl1 = random() % tournaments[cup_selected]->number_teams;
    score_team_pl2 = random() % tournaments[cup_selected_2]->number_teams;
    while (score_team_pl1 == score_team_pl2) {
        score_team_pl2 = random() % tournaments[cup_selected]->number_teams;
    }
    score_update_teams_pl1();
    score_update_teams_pl2();
}

static void score_reset(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        score_pl2 = score_pl1 = 0;
        lv_label_set_text(score_label1, "0");
        lv_label_set_text(score_label2, "0");
        score_replace_teams();
        score_card_update_event_handler();
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
#ifdef SDCARD_ENABLED
        save_match_to_sd();
#endif
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
        match_start_tick = xTaskGetTickCount();
#endif
        break;
    }
    }

    return;
}

static void score_bkgrnd_pl1_color_event_handler(lv_obj_t *_kb, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {
        index_team_color_pl1 = (index_team_color_pl1 + 1) % teams_color_size;
        ESP_LOGE(TAG, "Selected color index pl1 %d\n", index_team_color_pl1);
        lv_obj_set_style_local_bg_color(score_team_bkgrnd_pl1, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT,
                                        teams_color[index_team_color_pl1]);
    }
}

static void score_bkgrnd_pl2_color_event_handler(lv_obj_t *_kb, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {
        index_team_color_pl2 = (index_team_color_pl2 + 1) % teams_color_size;
        ESP_LOGE(TAG, "Selected color index pl2 %d\n", index_team_color_pl2);
        lv_obj_set_style_local_bg_color(score_team_bkgrnd_pl2, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT,
                                        teams_color[index_team_color_pl2]);
    }
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
    //  h = score_controls_container = create_container(parent, NULL,
    //  LV_LAYOUT_GRID, true);

    // Reduire l'espace entre les composants au minimum
    lv_obj_set_style_local_pad_inner(h, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);

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

    // Create a background object to set a label color
    score_label_cart_red_team_bkgrnd_pl1 = lv_obj_create(h, NULL);
    lv_obj_set_style_local_bg_color(score_label_cart_red_team_bkgrnd_pl1,
                                    LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                    LV_COLOR_RED);
    lv_obj_set_style_local_border_width(score_label_cart_red_team_bkgrnd_pl1,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_width(score_label_cart_red_team_bkgrnd_pl1, 15);
    lv_obj_set_height(score_label_cart_red_team_bkgrnd_pl1, 20);
    lv_obj_set_style_local_radius(score_label_cart_red_team_bkgrnd_pl1,
                                  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    score_label_cart_red_team_pl1 =
        lv_label_create(score_label_cart_red_team_bkgrnd_pl1, NULL);
    lv_label_set_text(score_label_cart_red_team_pl1, "");
    lv_obj_set_style_local_border_width(score_label_cart_red_team_pl1,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_left(score_label_cart_red_team_pl1,
                                    LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_pad_top(score_label_cart_red_team_pl1,
                                   LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_text_color(score_label_cart_red_team_pl1,
                                      LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,
                                      LV_COLOR_WHITE);

    // Create a background object to set a label color
    score_label_cart_red_team_bkgrnd_pl2 = lv_obj_create(h, NULL);
    lv_obj_set_style_local_bg_color(score_label_cart_red_team_bkgrnd_pl2,
                                    LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                    LV_COLOR_RED);
    lv_obj_set_style_local_border_width(score_label_cart_red_team_bkgrnd_pl2,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_width(score_label_cart_red_team_bkgrnd_pl2, 15);
    lv_obj_set_height(score_label_cart_red_team_bkgrnd_pl2, 20);
    lv_obj_set_style_local_radius(score_label_cart_red_team_bkgrnd_pl2,
                                  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    score_label_cart_red_team_pl2 =
        lv_label_create(score_label_cart_red_team_bkgrnd_pl2, NULL);
    lv_label_set_text(score_label_cart_red_team_pl2, "");
    lv_obj_set_style_local_border_width(score_label_cart_red_team_pl2,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_left(score_label_cart_red_team_pl2,
                                    LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_pad_top(score_label_cart_red_team_pl2,
                                   LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_text_color(score_label_cart_red_team_pl2,
                                      LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,
                                      LV_COLOR_WHITE);

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
    score_team_bkgrnd_pl1 = lv_obj_create(h, NULL);
    lv_obj_set_style_local_bg_color(score_team_bkgrnd_pl1, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_set_width(score_team_bkgrnd_pl1, object_width);
    lv_obj_set_style_local_border_width(score_team_bkgrnd_pl1, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);

    lv_obj_set_event_cb(score_team_bkgrnd_pl1,
                        score_bkgrnd_pl1_color_event_handler);

    score_team_label_pl1 = lv_label_create(score_team_bkgrnd_pl1, NULL);
    lv_obj_set_style_local_pad_top(score_team_label_pl1, LV_OBJ_PART_MAIN,
                                   LV_STATE_DEFAULT, 13);

    lv_label_set_align(score_team_label_pl1, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(score_team_label_pl1, LV_LABEL_LONG_SROLL_CIRC);
    lv_label_set_text(score_team_label_pl1, "-");
    lv_obj_set_width(score_team_label_pl1, object_width);
    lv_obj_set_style_local_text_color(score_team_label_pl1, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, LV_COLOR_BLACK);

    // Create a background object to set a label color
    score_label_cart_yellow_team_bkgrnd_pl1 = lv_obj_create(h, NULL);
    lv_obj_set_style_local_bg_color(score_label_cart_yellow_team_bkgrnd_pl1,
                                    LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                    LV_COLOR_YELLOW);
    lv_obj_set_style_local_border_width(score_label_cart_yellow_team_bkgrnd_pl1,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_set_width(score_label_cart_yellow_team_bkgrnd_pl1, 15);
    lv_obj_set_height(score_label_cart_yellow_team_bkgrnd_pl1, 20);
    lv_obj_set_style_local_radius(score_label_cart_yellow_team_bkgrnd_pl1,
                                  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    score_label_cart_yellow_team_pl1 =
        lv_label_create(score_label_cart_yellow_team_bkgrnd_pl1, NULL);
    lv_label_set_text(score_label_cart_yellow_team_pl1, "");
    lv_obj_set_style_local_border_width(score_label_cart_yellow_team_pl1,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_left(score_label_cart_yellow_team_pl1,
                                    LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_pad_top(score_label_cart_yellow_team_pl1,
                                   LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);

    // Create a background object to set a label color
    score_label_cart_yellow_team_bkgrnd_pl2 = lv_obj_create(h, NULL);
    lv_obj_set_style_local_bg_color(score_label_cart_yellow_team_bkgrnd_pl2,
                                    LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                    LV_COLOR_YELLOW);
    lv_obj_set_style_local_border_width(score_label_cart_yellow_team_bkgrnd_pl2,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_set_width(score_label_cart_yellow_team_bkgrnd_pl2, 15);
    lv_obj_set_height(score_label_cart_yellow_team_bkgrnd_pl2, 20);
    lv_obj_set_style_local_radius(score_label_cart_yellow_team_bkgrnd_pl2,
                                  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    score_label_cart_yellow_team_pl2 =
        lv_label_create(score_label_cart_yellow_team_bkgrnd_pl2, NULL);
    lv_label_set_text(score_label_cart_yellow_team_pl2, "");
    lv_obj_set_style_local_border_width(score_label_cart_yellow_team_pl2,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_left(score_label_cart_yellow_team_pl2,
                                    LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_pad_top(score_label_cart_yellow_team_pl2,
                                   LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);

    // Create a background object to set a label color
    score_team_bkgrnd_pl2 = lv_obj_create(h, NULL);
    lv_obj_set_style_local_bg_color(score_team_bkgrnd_pl2, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_obj_set_width(score_team_bkgrnd_pl2, object_width);
    lv_obj_set_style_local_border_width(score_team_bkgrnd_pl2, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);
    lv_obj_set_event_cb(score_team_bkgrnd_pl2,
                        score_bkgrnd_pl2_color_event_handler);

    score_team_label_pl2 = lv_label_create(score_team_bkgrnd_pl2, NULL);
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
    score_card_update_event_handler();

    return parent;
}

static void cup_update_displayed_teams(void)
{
    for (int i = 0; i < CUP_LABELS_SIZE - 1; i++) {
        if (i < tournaments[cup_selected]->number_teams) {
            lv_label_set_text(cup_labels[i],
                              tournaments[cup_selected]->teams[i]);
        } else {
            lv_label_set_text(cup_labels[i], "");
        }
    }
}

static void cup_handler(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        cup_selected = (cup_selected + 1) % cups_count;
        printf("Cup selected:%d\n", cup_selected);
        lv_label_set_text(cup_label_selected, tournaments[cup_selected]->title);
        printf("Teams size:%u\n", tournaments[cup_selected]->number_teams);
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

static void cup_2_handler(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        cup_selected_2 = (cup_selected_2 + 1) % cups_count;
        printf("Cup selected:%d\n", cup_selected_2);
        lv_label_set_text(cup_label_selected_2,
                          tournaments[cup_selected_2]->title);
        printf("Teams size:%u\n", tournaments[cup_selected_2]->number_teams);
        score_team_pl1 = 0;
        score_team_pl2 = 0;
        lv_label_set_text(score_team_label_pl1, "-");
        lv_label_set_text(score_team_label_pl2, "-");

        cup_update_displayed_teams();
#ifndef SIMULATOR
        Save::save_data.cup_2 = cup_selected_2;
#endif
        save_to_perform = true;
        break;
    }
    }
    return;
}

static void cup_handler_copy(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        cup_selected_2 = cup_selected;
        printf("Cup selected:%d\n", cup_selected_2);
        lv_label_set_text(cup_label_selected_2,
                          tournaments[cup_selected_2]->title);
        printf("Teams size:%u\n", tournaments[cup_selected_2]->number_teams);
        score_team_pl1 = 0;
        score_team_pl2 = 0;
        lv_label_set_text(score_team_label_pl1, "-");
        lv_label_set_text(score_team_label_pl2, "-");

        cup_update_displayed_teams();
#ifndef SIMULATOR
        Save::save_data.cup_2 = cup_selected_2;
#endif
        save_to_perform = true;
        break;
    }
    }
    return;
}

static int split_tsv_line(char *line, char **fields, int max_fields)
{
    int count = 0;
    char *p = line;
    while (p != NULL && count < max_fields) {
        fields[count++] = p;
        char *tab = strchr(p, '\t');
        if (tab != NULL) {
            *tab = '\0';
            p = tab + 1;
        } else {
            p = NULL;
        }
    }
    return count;
}

static void parse_card_players(char *card_str, char *players_list[], uint8_t *counter, char *regrouped_str, int limit)
{
    *counter = 0;
    if (card_str == NULL || strlen(card_str) == 0) {
        return;
    }
    char *p = card_str;
    while (p != NULL && *counter < limit) {
        char *comma = strchr(p, ',');
        if (comma != NULL) {
            *comma = '\0';
        }

        char *token = p;
        if (token[0] == '#') {
            token++;
        }

        if (strlen(token) > 0 && strlen(token) <= 3) {
            char *player_number = (char *)malloc(strlen(token) + 1);
            if (player_number != NULL) {
                strcpy(player_number, token);
                players_list[*counter] = player_number;

                if (regrouped_str != NULL) {
                    if (*counter > 0) {
                        strcat(regrouped_str, " ");
                    }
                    strcat(regrouped_str, player_number);
                }
                (*counter)++;
            }
        }

        if (comma != NULL) {
            p = comma + 1;
        } else {
            p = NULL;
        }
    }
}

// show_popups: user feedback for the manual button; silent for the automatic
// boot-time reload (a missing or empty file is normal on first start).
static void load_latest_match_from_sd(bool show_popups)
{
    char file_path[80];
#ifndef SIMULATOR
#ifdef SDCARD_ENABLED
    if (Disk::getInstance().getCardState() != Disk::CardState::Present) {
        if (show_popups) {
            popup("Erreur: carte SD non detectee");
        }
        return;
    }
    snprintf(file_path, sizeof(file_path), "%s/matches.tsv",
             Disk::getInstance().getMountPoint());
#else
    snprintf(file_path, sizeof(file_path), "matches.tsv");
#endif
#else
    snprintf(file_path, sizeof(file_path), "matches.tsv");
#endif

    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open %s for reading", file_path);
        if (show_popups) {
            popup("Erreur: impossible de lire matches.tsv");
        }
        return;
    }

    // Static: the GUI task stack is only 4096 bytes (display.cpp); 2 KB of
    // locals here overflow it and reboot the ESP32. Safe because this is only
    // ever called from the LVGL task.
    static char line[1024];
    static char last_line[1024];
    last_line[0] = '\0';
    bool found_line = false;

    // Read header line
    if (fgets(line, sizeof(line), f) == NULL) {
        fclose(f);
        if (show_popups) {
            popup("Erreur: fichier vide");
        }
        return;
    }

    // Read lines until EOF
    while (fgets(line, sizeof(line), f) != NULL) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }
        if (len > 0) {
            strcpy(last_line, line);
            found_line = true;
        }
    }
    fclose(f);

    if (!found_line) {
        if (show_popups) {
            popup("Erreur: aucune donnee de match");
        }
        return;
    }

    // Split TSV line
    char *fields[12];
    memset(fields, 0, sizeof(fields));
    int count = split_tsv_line(last_line, fields, 11);
    if (count < 6) {
        if (show_popups) {
            popup("Erreur: colonnes manquantes");
        }
        return;
    }

    char *cup1 = fields[0];
    char *cup2 = fields[1];
    char *team1 = fields[2];
    char *team2 = fields[3];
    char *score1_str = fields[4];
    char *score2_str = fields[5];
#ifndef SIMULATOR
    char *duration_str = (count > 6) ? fields[6] : NULL;
#endif
    char *yellow1_str = (count > 7) ? fields[7] : NULL;
    char *yellow2_str = (count > 8) ? fields[8] : NULL;
    char *red1_str = (count > 9) ? fields[9] : NULL;
    char *red2_str = (count > 10) ? fields[10] : NULL;

    // Match tournaments
    int cup1_idx = -1;
    int cup2_idx = -1;
    int tournaments_count = sizeof(tournaments) / sizeof(tournaments[0]);
    for (int i = 0; i < tournaments_count; i++) {
        if (strcmp(tournaments[i]->title, cup1) == 0) {
            cup1_idx = i;
        }
        if (strcmp(tournaments[i]->title, cup2) == 0) {
            cup2_idx = i;
        }
    }

    if (cup1_idx == -1 || cup2_idx == -1) {
        if (show_popups) {
            popup("Erreur: tournois non trouves");
        }
        return;
    }

    // Match teams
    int team1_idx = -1;
    for (int i = 0; i < tournaments[cup1_idx]->number_teams; i++) {
        if (strcmp(tournaments[cup1_idx]->teams[i], team1) == 0) {
            team1_idx = i;
            break;
        }
    }

    int team2_idx = -1;
    for (int i = 0; i < tournaments[cup2_idx]->number_teams; i++) {
        if (strcmp(tournaments[cup2_idx]->teams[i], team2) == 0) {
            team2_idx = i;
            break;
        }
    }

    if (team1_idx == -1 || team2_idx == -1) {
        if (show_popups) {
            popup("Erreur: equipes non trouvees");
        }
        return;
    }

    // Free existing card malloc'd arrays first to avoid leaks
    for (size_t team = 0; team < 2; team++) {
        for (int i = 0; i < card_yellow_players_counter[team]; i++) {
            if (card_yellow_players[team][i] != NULL) {
                free(card_yellow_players[team][i]);
                card_yellow_players[team][i] = NULL;
            }
        }
        card_yellow_players_counter[team] = 0;

        for (int i = 0; i < card_red_players_counter[team]; i++) {
            if (card_red_players[team][i] != NULL) {
                free(card_red_players[team][i]);
                card_red_players[team][i] = NULL;
            }
        }
        card_red_players_counter[team] = 0;
    }

    // Parse cards
    parse_card_players(yellow1_str, card_yellow_players[0], &card_yellow_players_counter[0], card_yellow_players_regrouped[0], CARD_YELLOW_PLAYERS_SIZE);
    parse_card_players(yellow2_str, card_yellow_players[1], &card_yellow_players_counter[1], card_yellow_players_regrouped[1], CARD_YELLOW_PLAYERS_SIZE);
    parse_card_players(red1_str, card_red_players[0], &card_red_players_counter[0], card_red_players_regrouped[0], CARD_RED_PLAYERS_SIZE);
    parse_card_players(red2_str, card_red_players[1], &card_red_players_counter[1], card_red_players_regrouped[1], CARD_RED_PLAYERS_SIZE);

    // Update variables
    cup_selected = cup1_idx;
    cup_selected_2 = cup2_idx;
    score_team_pl1 = team1_idx;
    score_team_pl2 = team2_idx;
    score_pl1 = atoi(score1_str);
    score_pl2 = atoi(score2_str);

#ifndef SIMULATOR
    if (duration_str && strlen(duration_str) > 0) {
        unsigned int min = 0, sec = 0;
        if (sscanf(duration_str, "%u:%u", &min, &sec) == 2) {
            uint32_t seconds = min * 60 + sec;
            match_start_tick = xTaskGetTickCount() - pdMS_TO_TICKS(seconds * 1000);
        }
    } else {
        match_start_tick = 0;
    }
#endif

    // Update cup tab UI
    if (cup_label_selected != NULL) {
        lv_label_set_text(cup_label_selected, tournaments[cup_selected]->title);
    }
    if (cup_label_selected_2 != NULL) {
        lv_label_set_text(cup_label_selected_2, tournaments[cup_selected_2]->title);
    }
    cup_update_displayed_teams();

    // Update score UI
    char temp_score[16];
    if (score_label1 != NULL) {
        sprintf(temp_score, "%u", score_pl1);
        lv_label_set_text(score_label1, temp_score);
    }
    if (score_label2 != NULL) {
        sprintf(temp_score, "%u", score_pl2);
        lv_label_set_text(score_label2, temp_score);
    }
    if (score_team_label_pl1 != NULL) {
        lv_label_set_text(score_team_label_pl1, tournaments[cup_selected]->teams[score_team_pl1]);
    }
    if (score_team_label_pl2 != NULL) {
        lv_label_set_text(score_team_label_pl2, tournaments[cup_selected_2]->teams[score_team_pl2]);
    }

    index_team_color_pl1 = score_team_pl1 % teams_color_size;
    if (score_team_bkgrnd_pl1 != NULL) {
        lv_obj_set_style_local_bg_color(score_team_bkgrnd_pl1, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT,
                                        teams_color[index_team_color_pl1]);
    }
    index_team_color_pl2 = score_team_pl2 % teams_color_size;
    if (score_team_bkgrnd_pl2 != NULL) {
        lv_obj_set_style_local_bg_color(score_team_bkgrnd_pl2, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT,
                                        teams_color[index_team_color_pl2]);
    }

    // Update card UI labels
    if (card_team1_label != NULL) {
        lv_label_set_text(card_team1_label, tournaments[cup_selected]->teams[score_team_pl1]);
    }
    if (card_team1_bkgrnd != NULL) {
        lv_obj_set_style_local_bg_color(card_team1_bkgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        teams_color[score_team_pl1 % teams_color_size]);
    }
    if (card_team2_label != NULL) {
        lv_label_set_text(card_team2_label, tournaments[cup_selected_2]->teams[score_team_pl2]);
    }
    if (card_team2_bkgrnd != NULL) {
        lv_obj_set_style_local_bg_color(card_team2_bkgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        teams_color[score_team_pl2 % teams_color_size]);
    }

    card_index_team = 0;
    if (card_team_label != NULL) {
        lv_label_set_text(card_team_label, tournaments[cup_selected]->teams[score_team_pl1]);
    }
    if (card_team_bkgrnd != NULL) {
        lv_obj_set_style_local_bg_color(card_team_bkgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                        teams_color[score_team_pl1 % teams_color_size]);
    }
    if (card_player_number_text_area != NULL) {
        lv_textarea_set_text(card_player_number_text_area, "");
    }

    if (card_yellow_team1_textarea != NULL) {
        lv_textarea_set_text(card_yellow_team1_textarea, card_yellow_players_regrouped[0]);
    }
    if (card_yellow_team2_textarea != NULL) {
        lv_textarea_set_text(card_yellow_team2_textarea, card_yellow_players_regrouped[1]);
    }
    if (card_red_team1_textarea != NULL) {
        lv_textarea_set_text(card_red_team1_textarea, card_red_players_regrouped[0]);
    }
    if (card_red_team2_textarea != NULL) {
        lv_textarea_set_text(card_red_team2_textarea, card_red_players_regrouped[1]);
    }

    score_card_update_event_handler();

#ifndef SIMULATOR
    Save::save_data.cup = cup_selected;
    Save::save_data.cup_2 = cup_selected_2;
    Save::write_save();
#endif
}

static void cup_handler_load(lv_obj_t *btn, lv_event_t event)
{
    switch (event) {
    case LV_EVENT_CLICKED: {
        load_latest_match_from_sd(true);
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
    lv_obj_set_style_local_radius(cup_button, LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT, 10);
    lv_obj_set_height(cup_button, obj_height);
    lv_obj_set_event_cb(cup_button, cup_handler);

    cup_label_selected = lv_label_create(cup_button, NULL);
    lv_label_set_text(cup_label_selected, "0");
    lv_label_set_align(cup_label_selected, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(cup_label_selected, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *cup_button_copy = lv_btn_create(parent, NULL);
    lv_obj_set_width(cup_button_copy, 50);
    lv_obj_set_style_local_radius(cup_button_copy, LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT, 10);
    lv_obj_set_height(cup_button_copy, obj_height);
    lv_obj_set_event_cb(cup_button_copy, cup_handler_copy);

    lv_obj_t *cup_label_copy = lv_label_create(cup_button_copy, NULL);
    lv_label_set_text(cup_label_copy, "Copy");
    lv_label_set_align(cup_label_copy, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(cup_label_copy, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *cup_button_2 = lv_btn_create(parent, NULL);
    // bouton position absolue si besoin mais sans set_scrl_layout
    // lv_obj_set_x(cup_button, 160);
    // lv_obj_set_y(cup_button, 0);
    lv_obj_set_width(cup_button_2, 200);
    lv_obj_set_style_local_radius(cup_button_2, LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT, 10);
    lv_obj_set_height(cup_button_2, obj_height);
    lv_obj_set_event_cb(cup_button_2, cup_2_handler);

    cup_label_selected_2 = lv_label_create(cup_button_2, NULL);
    lv_label_set_text(cup_label_selected_2, "0");
    lv_label_set_align(cup_label_selected_2, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(cup_label_selected_2, NULL, LV_ALIGN_CENTER, 0, 0);

    // Laissé pour le futur chargement depuis SD, mais pas de besoin immédiat et ça prend de la place dans l'UI
    // lv_obj_t *cup_button_load = lv_btn_create(parent, NULL);
    // lv_obj_set_width(cup_button_load, 200);
    // lv_obj_set_style_local_radius(cup_button_load, LV_OBJ_PART_MAIN,
    //                               LV_STATE_DEFAULT, 10);
    // lv_obj_set_height(cup_button_load, obj_height);
    // lv_obj_set_event_cb(cup_button_load, cup_handler_load);

    // lv_obj_t *cup_label_load = lv_label_create(cup_button_load, NULL);
    // lv_label_set_text(cup_label_load, "Charger depuis SD");
    // lv_label_set_align(cup_label_load, LV_LABEL_ALIGN_CENTER);
    // lv_obj_align(cup_label_load, NULL, LV_ALIGN_CENTER, 0, 0);

#ifndef SIMULATOR
    cup_selected = Save::save_data.cup;
    cup_selected_2 = Save::save_data.cup_2;
#endif
    cups_count = sizeof(tournaments) / sizeof(tournaments[0]);
    lv_label_set_text(cup_label_selected, tournaments[cup_selected]->title);
    lv_label_set_text(cup_label_selected_2, tournaments[cup_selected_2]->title);

    for (int i = 0; i < CUP_LABELS_SIZE; i++) {
        cup_labels[i] = lv_label_create(parent, NULL);
        lv_label_set_text(cup_labels[i], "");
        lv_obj_set_style_local_text_font(cup_labels[i], LV_LABEL_PART_MAIN,
                                         LV_STATE_DEFAULT,
                                         &lv_font_montserrat_12);
    }
    cup_update_displayed_teams();

    return parent;
}

static lv_obj_t *card_keyboard;

static void card_keyboard_event_cb(lv_obj_t *_kb, lv_event_t e)
{
    lv_keyboard_def_event_cb(card_keyboard, e);

    // if(e == LV_EVENT_CANCEL || e == LV_EVENT_DELETE) {
    if (e == LV_EVENT_CANCEL) {
        if (card_keyboard) {
            lv_obj_del(card_keyboard);
            card_keyboard = NULL;
        }
    }

    if (e == LV_EVENT_APPLY || e == LV_EVENT_CANCEL) {
        if (card_keyboard) {
            lv_group_focus_obj(lv_keyboard_get_textarea(card_keyboard));
            lv_obj_del(card_keyboard);
            card_keyboard = NULL;
        }
    }
}

static void card_team_label_event_handler(lv_obj_t *_kb, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {
        if (card_index_team == 0) {

            lv_label_set_text(
                card_team_label,
                tournaments[cup_selected_2]->teams[score_team_pl2]);
            lv_obj_set_style_local_bg_color(
                card_team_bkgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                teams_color[score_team_pl2 % teams_color_size]);
            card_index_team = 1;
        } else {
            lv_label_set_text(card_team_label,
                              tournaments[cup_selected]->teams[score_team_pl1]);
            lv_obj_set_style_local_bg_color(
                card_team_bkgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                teams_color[score_team_pl1 % teams_color_size]);
            card_index_team = 0;
        }
    }
}

static void card_teams_set()
{
    // # Team 1
    lv_label_set_text(card_team1_label,
                      tournaments[cup_selected]->teams[score_team_pl1]);
    lv_obj_set_style_local_bg_color(
        card_team1_bkgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
        teams_color[score_team_pl1 % teams_color_size]);

    // # Team 2
    lv_label_set_text(card_team2_label,
                      tournaments[cup_selected_2]->teams[score_team_pl2]);
    lv_obj_set_style_local_bg_color(
        card_team2_bkgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
        teams_color[score_team_pl2 % teams_color_size]);

    for (size_t index = 0; index < 2; index++) {
        // card_yellow_players_regrouped[index] = "";
        memset(card_yellow_players_regrouped[index], 0,
               CARD_YELLOW_PLAYERS_SIZE * 3);
        memset(card_red_players_regrouped[index], 0, CARD_RED_PLAYERS_SIZE * 3);
    }

    card_index_team = 0;
    lv_label_set_text(card_team_label,
                      tournaments[cup_selected]->teams[score_team_pl1]);
    lv_obj_set_style_local_bg_color(
        card_team_bkgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
        teams_color[score_team_pl1 % teams_color_size]);
    lv_textarea_set_text(card_player_number_text_area, "");

    lv_textarea_set_text(card_yellow_team1_textarea, "");
    lv_textarea_set_text(card_yellow_team2_textarea, "");
    card_yellow_players_counter[0] = 0;
    card_yellow_players_counter[1] = 0;
    lv_textarea_set_text(card_red_team1_textarea, "");
    lv_textarea_set_text(card_red_team2_textarea, "");
    card_red_players_counter[0] = 0;
    card_red_players_counter[1] = 0;
}

static void card_red_event(lv_obj_t *_kb, lv_event_t e);

static void card_yellow_event(lv_obj_t *_kb, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {

        char *player_number;
        const char *player_number_read =
            lv_textarea_get_text(card_player_number_text_area);
        // +1 for null terminator
        player_number = (char *)malloc(strlen(player_number_read) + 1);
        strcpy(player_number, player_number_read);

        if (strlen(player_number) == 0) {
            return;
        }

        if (strlen(player_number) > 3) {
            return;
        }

        // Check if player already with yellow card
        for (int i = 0; i < card_yellow_players_counter[card_index_team]; i++) {
            if (strcmp(card_yellow_players[card_index_team][i],
                       player_number) == 0) {
                card_red_event(NULL, LV_EVENT_CLICKED);
                return;
            };
        }

        if (card_yellow_players_counter[card_index_team] != 0) {
            strcat(card_yellow_players_regrouped[card_index_team], " ");
        }
        card_yellow_players[card_index_team]
                           [card_yellow_players_counter[card_index_team]] =
                               player_number;
        card_yellow_players_counter[card_index_team] += 1;
        if (card_yellow_players_counter[card_index_team] >
            CARD_YELLOW_PLAYERS_SIZE) {
            return;
        }
        strcat(card_yellow_players_regrouped[card_index_team], player_number);
        // printf("\n");
        // ESP_LOGE(tag, card_yellow_players_regrouped[card_index_team]);
        // printf("\n");
        // for (int i = 0; i < card_yellow_players_counter[card_index_team];
        // i++) {
        //     printf("%s ", card_yellow_players[card_index_team][i]);
        // }
        // printf("\n");

        lv_textarea_clear_selection(card_player_number_text_area);
        lv_textarea_set_text(card_player_number_text_area, "");
        if (card_index_team == 0) {
            lv_textarea_set_text(
                card_yellow_team1_textarea,
                card_yellow_players_regrouped[card_index_team]);
        } else {
            lv_textarea_set_text(
                card_yellow_team2_textarea,
                card_yellow_players_regrouped[card_index_team]);
        }
        score_card_update_event_handler();
#ifndef SIMULATOR
#ifdef SDCARD_ENABLED
        save_match_to_sd();
#endif
#endif
    }
}

static void card_red_event(lv_obj_t *_kb, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {

        char *player_number;
        const char *player_number_read =
            lv_textarea_get_text(card_player_number_text_area);
        // +1 for null terminator
        player_number = (char *)malloc(strlen(player_number_read) + 1);
        strcpy(player_number, player_number_read);

        if (strlen(player_number) == 0) {
            return;
        }

        if (strlen(player_number) > 3) {
            return;
        }

        // Check if player already with red card
        for (int i = 0; i < card_red_players_counter[card_index_team]; i++) {
            if (strcmp(card_red_players[card_index_team][i], player_number) ==
                0) {
                return;
            };
        }

        if (card_red_players_counter[card_index_team] != 0) {
            strcat(card_red_players_regrouped[card_index_team], " ");
        }
        card_red_players[card_index_team]
                        [card_red_players_counter[card_index_team]] =
                            player_number;
        card_red_players_counter[card_index_team] += 1;
        if (card_red_players_counter[card_index_team] > CARD_RED_PLAYERS_SIZE) {
            return;
        }
        strcat(card_red_players_regrouped[card_index_team], player_number);
        // printf("\n");
        // ESP_LOGE(tag, card_red_players_regrouped[card_index_team]);
        // printf("\n");
        // for (int i = 0; i < card_red_players_counter[card_index_team];
        // i++) {
        //     printf("%s ", card_red_players[card_index_team][i]);
        // }
        // printf("\n");

        lv_textarea_clear_selection(card_player_number_text_area);
        lv_textarea_set_text(card_player_number_text_area, "");
        if (card_index_team == 0) {
            lv_textarea_set_text(card_red_team1_textarea,
                                 card_red_players_regrouped[card_index_team]);
        } else {
            lv_textarea_set_text(card_red_team2_textarea,
                                 card_red_players_regrouped[card_index_team]);
        }
        score_card_update_event_handler();
#ifndef SIMULATOR
#ifdef SDCARD_ENABLED
        save_match_to_sd();
#endif
#endif
    }
}

static void card_textarea_event_cb(lv_obj_t *ta, lv_event_t e)
{
    if (e == LV_EVENT_FOCUSED) {
        if (card_keyboard == NULL) {
            card_keyboard = lv_keyboard_create(lv_scr_act(), NULL);
            lv_keyboard_set_mode(card_keyboard, LV_KEYBOARD_MODE_NUM);
            lv_obj_set_event_cb(card_keyboard, card_keyboard_event_cb);
        }
        lv_textarea_set_cursor_hidden(ta, false);
        lv_keyboard_set_textarea(card_keyboard, ta);
    } else if (e == LV_EVENT_DEFOCUSED) {
        lv_textarea_set_cursor_hidden(ta, true);
    }
}

// POSITION TWEAK as simulator and RSP32 do not display in same way with
// layout_off
#define CARD_OBJECT_WIDTH_HALF_SCREEN 160
#define CARD_OBJECT_SIZE_WIDTH_BIG 159
#define CARD_OBJECT_HEIGHT 30
#define CARD_BUTTONS_WIDTH 79
#define CARD_PLAYER_NUMBER_WIDTH 40
#define CARD_LEFT_POS 0
#define CARD_YELLOW_PLAYERS_HEIGHT 80
#define CARD_RED_PLAYERS_HEIGHT 35

#ifdef SIMULATOR
#define CARD_UPPER_VERTICAL_POS 0
#define CARD_LEFT_POS_ELEMENTS 0
#define CARD_LEFT_POS_SEPARATOR CARD_PLAYER_NUMBER_WIDTH
#define CARD_RIGHT_PANEL_LEFT_POS CARD_OBJECT_WIDTH_HALF_SCREEN + 1
#else
#define CARD_UPPER_VERTICAL_POS 12
#define CARD_LEFT_POS_ELEMENTS 13
#define CARD_LEFT_POS_SEPARATOR CARD_PLAYER_NUMBER_WIDTH + 13
#define CARD_RIGHT_PANEL_LEFT_POS CARD_OBJECT_WIDTH_HALF_SCREEN + 13
#endif
#define CARD_YELLOW_PLAYERS_VERTICAL_POS                                       \
    CARD_OBJECT_HEIGHT + CARD_OBJECT_HEIGHT + 2 + CARD_UPPER_VERTICAL_POS
#define CARD_TEAMS_NAME_VERTICAL_POS                                           \
    CARD_OBJECT_HEIGHT + CARD_UPPER_VERTICAL_POS + 1
#define CARD_RED_CARD_H_POS CARD_RIGHT_PANEL_LEFT_POS + CARD_BUTTONS_WIDTH
#define CARD_RED_PLAYERS_VERTICAL_POS                                          \
    CARD_YELLOW_PLAYERS_VERTICAL_POS + CARD_YELLOW_PLAYERS_HEIGHT + 1

static lv_obj_t *tab_card_init(debug_tabs_t *tab)
{
#ifdef SIMULATOR
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, TITRE_TAB_CARD);
#else
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, tab->name);
#endif
    lv_page_set_scrollbar_mode(parent, LV_SCRLBAR_MODE_OFF);
    lv_page_set_scrl_layout(parent, LV_LAYOUT_OFF);

    card_player_number_text_area = lv_textarea_create(parent, NULL);
    lv_textarea_set_text(card_player_number_text_area, "");
    lv_textarea_set_placeholder_text(card_player_number_text_area, "00");
    lv_textarea_set_one_line(card_player_number_text_area, true);
    lv_textarea_set_cursor_hidden(card_player_number_text_area, true);
    lv_textarea_set_text_align(card_player_number_text_area,
                               LV_LABEL_ALIGN_CENTER);
    lv_textarea_set_max_length(card_player_number_text_area, 2);

    lv_obj_set_pos(card_player_number_text_area, CARD_LEFT_POS, 0);
    lv_obj_set_size(card_player_number_text_area, CARD_PLAYER_NUMBER_WIDTH,
                    CARD_OBJECT_HEIGHT);

    lv_obj_set_style_local_radius(card_player_number_text_area,
                                  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_border_width(card_player_number_text_area,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_set_event_cb(card_player_number_text_area, card_textarea_event_cb);

    // Team label background
    card_team_bkgrnd = lv_obj_create(parent, NULL);
    lv_obj_set_style_local_bg_color(card_team_bkgrnd, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_set_pos(card_team_bkgrnd, CARD_LEFT_POS_SEPARATOR,
                   CARD_UPPER_VERTICAL_POS);
    lv_obj_set_size(card_team_bkgrnd, 119, CARD_OBJECT_HEIGHT);
    lv_obj_set_style_local_border_width(card_team_bkgrnd, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(card_team_bkgrnd, LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT, 1);

    lv_obj_set_event_cb(card_team_bkgrnd, card_team_label_event_handler);

    // Team label
    card_team_label = lv_label_create(card_team_bkgrnd, NULL);
    lv_label_set_align(card_team_label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(card_team_label, LV_LABEL_LONG_SROLL_CIRC);
    lv_label_set_text(card_team_label, "-");

    lv_obj_set_size(card_team_label, 119, CARD_OBJECT_HEIGHT);

    lv_obj_set_style_local_pad_top(card_team_label, LV_OBJ_PART_MAIN,
                                   LV_STATE_DEFAULT, 7);
    lv_obj_set_style_local_text_color(card_team_label, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_border_width(card_team_label, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);

    // Card Yellow label background
    lv_obj_t *card_yellow_bkgrnd = lv_obj_create(parent, NULL);
    lv_obj_set_style_local_bg_color(card_yellow_bkgrnd, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_obj_set_style_local_border_width(card_yellow_bkgrnd, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_radius(card_yellow_bkgrnd, LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT, 1);

    lv_obj_set_pos(card_yellow_bkgrnd, CARD_RIGHT_PANEL_LEFT_POS,
                   CARD_UPPER_VERTICAL_POS);
    lv_obj_set_size(card_yellow_bkgrnd, CARD_BUTTONS_WIDTH, CARD_OBJECT_HEIGHT);

    lv_obj_set_event_cb(card_yellow_bkgrnd, card_yellow_event);

    // Card Yellow label
    lv_obj_t *card_yellow_label = lv_label_create(card_yellow_bkgrnd, NULL);
    lv_label_set_align(card_yellow_label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(card_yellow_label, "JAUNE");

    lv_obj_set_style_local_pad_top(card_yellow_label, LV_OBJ_PART_MAIN,
                                   LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_pad_left(card_yellow_label, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, 15);
    lv_obj_set_style_local_text_color(card_yellow_label, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_border_width(card_yellow_label, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);

    // Card Red label background
    lv_obj_t *card_red_bkgrnd = lv_obj_create(parent, card_yellow_bkgrnd);
    lv_obj_set_style_local_bg_color(card_red_bkgrnd, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_obj_set_pos(card_red_bkgrnd, CARD_RED_CARD_H_POS,
                   CARD_UPPER_VERTICAL_POS);

    lv_obj_set_event_cb(card_red_bkgrnd, card_red_event);

    // Card Red label
    lv_obj_t *card_red_label =
        lv_label_create(card_red_bkgrnd, card_yellow_label);
    lv_label_set_text(card_red_label, "ROUGE");

    // Team1 cards
    card_team1_bkgrnd = lv_obj_create(parent, card_yellow_bkgrnd);
    lv_obj_set_style_local_bg_color(card_team1_bkgrnd, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_set_pos(card_team1_bkgrnd, CARD_LEFT_POS_ELEMENTS,
                   CARD_TEAMS_NAME_VERTICAL_POS);
    lv_obj_set_size(card_team1_bkgrnd, CARD_OBJECT_SIZE_WIDTH_BIG,
                    CARD_OBJECT_HEIGHT);

    card_team1_label = lv_label_create(card_team1_bkgrnd, card_yellow_label);
    lv_label_set_align(card_team1_label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(card_team1_label, LV_LABEL_LONG_SROLL_CIRC);
    lv_label_set_text(card_team1_label, "-");
    lv_obj_set_style_local_pad_left(card_team1_label, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, 0);

    lv_obj_set_size(card_team1_label, CARD_OBJECT_SIZE_WIDTH_BIG,
                    CARD_OBJECT_HEIGHT);

    card_yellow_team1_textarea = lv_textarea_create(parent, NULL);
    lv_textarea_set_text(card_yellow_team1_textarea, "");
    lv_textarea_set_placeholder_text(card_yellow_team1_textarea, "");
    lv_textarea_set_one_line(card_yellow_team1_textarea, false);
    lv_textarea_set_cursor_hidden(card_yellow_team1_textarea, true);
    lv_obj_set_style_local_bg_color(card_yellow_team1_textarea,
                                    LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                    LV_COLOR_MAKE(0x88, 0x88, 0x00));

    lv_obj_set_pos(card_yellow_team1_textarea, CARD_LEFT_POS_ELEMENTS,
                   CARD_YELLOW_PLAYERS_VERTICAL_POS);
    lv_obj_set_size(card_yellow_team1_textarea, CARD_OBJECT_SIZE_WIDTH_BIG,
                    CARD_YELLOW_PLAYERS_HEIGHT);
    lv_obj_set_style_local_radius(card_yellow_team1_textarea, LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_border_width(card_yellow_team1_textarea,
                                        LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    card_red_team1_textarea =
        lv_textarea_create(parent, card_yellow_team1_textarea);
    lv_obj_set_style_local_bg_color(card_red_team1_textarea, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    LV_COLOR_MAKE(0xFF, 0x33, 0x33));

    lv_obj_set_pos(card_red_team1_textarea, CARD_LEFT_POS_ELEMENTS,
                   CARD_RED_PLAYERS_VERTICAL_POS);
    lv_obj_set_size(card_red_team1_textarea, CARD_OBJECT_SIZE_WIDTH_BIG,
                    CARD_RED_PLAYERS_HEIGHT);

    // team2 cards copied from team1
    card_team2_bkgrnd = lv_obj_create(parent, card_team1_bkgrnd);
    lv_obj_set_style_local_bg_color(card_team2_bkgrnd, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_set_pos(card_team2_bkgrnd, CARD_RIGHT_PANEL_LEFT_POS,
                   CARD_TEAMS_NAME_VERTICAL_POS);

    card_team2_label = lv_label_create(card_team2_bkgrnd, card_team1_label);

    card_yellow_team2_textarea =
        lv_textarea_create(parent, card_yellow_team1_textarea);
    lv_obj_set_pos(card_yellow_team2_textarea, CARD_RIGHT_PANEL_LEFT_POS,
                   CARD_YELLOW_PLAYERS_VERTICAL_POS);

    card_red_team2_textarea =
        lv_textarea_create(parent, card_red_team1_textarea);
    lv_obj_set_pos(card_red_team2_textarea, CARD_RIGHT_PANEL_LEFT_POS,
                   CARD_RED_PLAYERS_VERTICAL_POS);

    card_yellow_players_counter[0] = 0;
    card_yellow_players_counter[1] = 0;
    card_red_players_counter[0] = 0;
    card_red_players_counter[1] = 0;
    for (size_t index = 0; index < 2; index++) {
        card_yellow_players_regrouped[index] =
            (char *)malloc(CARD_YELLOW_PLAYERS_SIZE * 3);
        if (card_yellow_players_regrouped[index] != NULL) {
            memset(card_yellow_players_regrouped[index], 0,
                   CARD_YELLOW_PLAYERS_SIZE * 3);
        }
        card_red_players_regrouped[index] =
            (char *)malloc(CARD_RED_PLAYERS_SIZE * 3);
        if (card_red_players_regrouped[index] != NULL) {
            memset(card_red_players_regrouped[index], 0,
                   CARD_RED_PLAYERS_SIZE * 3);
        }
    }
    card_index_team = 0;

    return parent;
}

static void cup_sounds_handler(lv_obj_t *btn, lv_event_t event,
                               uint8_t snd_index)
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

const char *cup_sounds_name[] = {"Beep",
                                 "Buzz",
                                 "Connection",
                                 "Disconnection",
                                 "ButtonPushed",
                                 "Mode1",
                                 "Mode2",
                                 "Mode3",
                                 "Surprise",
                                 "OhOoh",
                                 "Cuddly",
                                 "Sleeping",
                                 "Happy",
                                 "SuperHappy",
                                 "HappyShort",
                                 "Sad",
                                 "ImportantNotice",
                                 "LevelUp",
                                 "LevelDown"};

static lv_event_cb_t cup_sounds_handlers[20] = {
    cup_sounds_handler_01, cup_sounds_handler_02, cup_sounds_handler_03,
    cup_sounds_handler_04, cup_sounds_handler_05, cup_sounds_handler_06,
    cup_sounds_handler_07, cup_sounds_handler_08, cup_sounds_handler_09,
    cup_sounds_handler_10, cup_sounds_handler_11, cup_sounds_handler_12,
    cup_sounds_handler_13, cup_sounds_handler_14, cup_sounds_handler_15,
    cup_sounds_handler_16, cup_sounds_handler_17, cup_sounds_handler_18,
    cup_sounds_handler_19};

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
        lv_obj_set_style_local_radius(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                      10);
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

static lv_obj_t *tab_chrono_init(debug_tabs_t *tab)
{
    static lv_style_t style_chrono_large;
    static bool style_init = false;
    if (!style_init) {
        lv_style_init(&style_chrono_large);
        lv_style_set_text_font(&style_chrono_large, LV_STATE_DEFAULT,
                               &lv_font_montserrat_48);
        style_init = true;
    }

#ifdef SIMULATOR
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, TITRE_TAB_CHRONO);
#else
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, tab->name);
#endif
    chrono_parent = parent;
    lv_page_set_scrl_layout(parent, LV_LAYOUT_COLUMN_MID);

    // lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);
    // Controls container
    // lv_page_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    // lv_page_set_scroll_propagation(parent, false);

    // Align content from middle or top for the 2nd line
    // h = create_container(parent, NULL, LV_LAYOUT_PRETTY_MID, true);
    //  h = score_controls_container = create_container(parent, NULL,
    //  LV_LAYOUT_GRID, true);

    // Reduire l'espace entre les composants au minimum

    // Top container for time and main controls
    lv_obj_t *top_cont = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_pad_inner(top_cont, LV_OBJ_PART_MAIN,
                                     LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_pad_top(top_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                   0);
    lv_obj_set_style_local_pad_bottom(top_cont, LV_OBJ_PART_MAIN,
                                      LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_left(top_cont, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_right(top_cont, LV_OBJ_PART_MAIN,
                                     LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_margin_bottom(top_cont, LV_OBJ_PART_MAIN,
                                         LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_margin_top(top_cont, LV_OBJ_PART_MAIN,
                                      LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_margin_left(top_cont, LV_OBJ_PART_MAIN,
                                       LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_margin_right(top_cont, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_pad_top(top_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                   10);
    lv_obj_set_style_local_pad_bottom(top_cont, LV_OBJ_PART_MAIN,
                                      LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_margin_top(top_cont, LV_OBJ_PART_MAIN,
                                      LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_margin_bottom(top_cont, LV_OBJ_PART_MAIN,
                                         LV_STATE_DEFAULT, 0);

    lv_cont_set_layout(top_cont, LV_LAYOUT_PRETTY_MID);
    lv_cont_set_fit2(top_cont, LV_FIT_MAX, LV_FIT_TIGHT);
    // lv_obj_set_style_local_pad_all(top_cont, LV_CONT_PART_MAIN,
    // LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(top_cont, LV_CONT_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);

    // Timer display
    label_chrono = lv_label_create(top_cont, NULL);
    lv_obj_add_style(label_chrono, LV_LABEL_PART_MAIN, &style_chrono_large);
    update_chrono_label();

    // Start/Stop and Reset buttons container (stacked to the right)
    lv_obj_t *cont2 = lv_cont_create(top_cont, NULL);
    lv_cont_set_layout(cont2, LV_LAYOUT_COLUMN_MID);
    lv_cont_set_fit(cont2, LV_FIT_TIGHT);
    // lv_obj_set_style_local_pad_all(cont2, LV_CONT_PART_MAIN,
    // LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_inner(cont2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                     0);
    lv_obj_set_style_local_border_width(cont2, LV_CONT_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_opa(cont2, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,
                                  LV_OPA_TRANSP);

    lv_obj_t *btn_ss = lv_btn_create(cont2, NULL);
    lv_obj_set_width(btn_ss, 80);
    lv_obj_set_event_cb(btn_ss, chrono_start_stop_event_handler);
    label_start_stop = lv_label_create(btn_ss, NULL);
    lv_label_set_text(label_start_stop, countdown_running ? "STOP" : "START");

    lv_obj_t *btn_reset = lv_btn_create(cont2, NULL);
    lv_obj_set_width(btn_reset, 80);
    lv_obj_set_event_cb(btn_reset, chrono_reset_event_handler);
    lv_obj_t *lbl_reset = lv_label_create(btn_reset, NULL);
    lv_label_set_text(lbl_reset, "RESET");

    // Buttons container for adding time (bottom row)
    lv_obj_t *cont = lv_cont_create(top_cont, NULL);
    lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);
    lv_cont_set_fit2(cont, LV_FIT_MAX, LV_FIT_TIGHT);
    lv_obj_set_style_local_pad_inner(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                     0);
    // lv_obj_set_style_local_pad_all(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,
    // 0);
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);

    const struct {
        const char *name;
        int seconds;
    } time_btns[] = {
        {"+10s", 10},
        {"+30s", 30},
        {"+1m", 60},
    };

    for (int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_btn_create(cont, NULL);
        lv_obj_set_width(btn, 70);
        lv_obj_set_user_data(btn, (void *)(intptr_t)time_btns[i].seconds);
        lv_obj_set_event_cb(btn, chrono_add_time_event_handler);
        lv_obj_t *lbl = lv_label_create(btn, NULL);
        lv_label_set_text(lbl, time_btns[i].name);
    }

    // Remove padding from the tab page itself
    // lv_obj_set_style_local_pad_all(parent, LV_PAGE_PART_BG, LV_STATE_DEFAULT,
    // 0); lv_obj_set_style_local_pad_all(parent, LV_PAGE_PART_SCROLLABLE,
    // LV_STATE_DEFAULT, 0);

    return parent;
}

static lv_obj_t *tab_images_init(debug_tabs_t *tab)
{
#ifdef SDCARD_ENABLED
    return tab_images_init_real(tab_view, tab->name);
#else
    return NULL;
#endif
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
        if (debug_tabs[i].init) {
            debug_tabs[i].init(&debug_tabs[i]);
        }
    }
#ifdef SDCARD_ENABLED
    if (Save::save_data.sd_enabled) {
        Disk::getInstance().enable();
    }
#endif

    DisplayLedcBacklight::getInstance().start();
#else
    util_styles_init();
    tab_score_init(NULL);
    tab_cup_init(NULL);
    tab_card_init(NULL);
    tab_chrono_init(NULL);
    tab_sounds_init(NULL);
    tab_config_init(NULL);
    tab_images_init_real(tab_view, TITRE_TAB_IMAGES);
    load_latest_match_from_sd(false);
#endif

    return;
}

#ifndef SIMULATOR

void screen_debug_loop()
{
    static uint32_t last_chrono_tick = 0;
    uint32_t now_ms = lv_tick_get();
    if (countdown_running && now_ms - last_chrono_tick >= 1000) {
        last_chrono_tick = now_ms;
        if (countdown_seconds > 0) {
            countdown_seconds--;
            update_chrono_label();
            if (countdown_seconds == 0) {
                countdown_running = false;
                if (label_start_stop) {
                    lv_label_set_text(label_start_stop, "START");
                }
#ifndef SIMULATOR
                Buzzer::getInstance().play(Buzzer::Sounds::LevelUp);
                // NeoPixel::getInstance().setColor(0xFF0000); // Red
#endif
                if (chrono_parent) {
                    lv_obj_set_style_local_bg_color(
                        chrono_parent, LV_PAGE_PART_BG, LV_STATE_DEFAULT,
                        LV_COLOR_RED);
                }
            }
        }
    }

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

    // Boot-time reload of the latest saved match, deferred until the SD card
    // is actually mounted (detection is asynchronous). Silent: a missing or
    // empty matches.tsv is normal on first start.
    if (match_boot_load_pending &&
        Disk::getInstance().getCardState() == Disk::CardState::Present) {
        match_boot_load_pending = false;
        load_latest_match_from_sd(false);
    }

    if (Save::save_data.sd_enabled) {
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
