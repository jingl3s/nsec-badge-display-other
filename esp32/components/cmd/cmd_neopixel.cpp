#include "cmd_neopixel.h"

#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "esp_log.h"
#include "neopixel.h"
#include "save.h"

static struct {
    struct arg_int *mode;
    struct arg_int *brightness;
    struct arg_int *color;
    struct arg_end *end;
} neopixel_args;

static const char *TAG = "cmd";

static int configure_neopixel(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&neopixel_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, neopixel_args.end, argv[0]);
        return 1;
    }

    Save::save_data.neopixel_is_on = true;
    NeoPixel::getInstance().setBrightness(neopixel_args.brightness->ival[0]);
    NeoPixel::getInstance().setColor(neopixel_args.color->ival[0]);
    NeoPixel::getInstance().setMode(neopixel_args.mode->ival[0]);
    ESP_LOGE(TAG, "CMD brightness %d, color %d, mode %d", neopixel_args.brightness->ival[0], neopixel_args.color->ival[0], neopixel_args.mode->ival[0]);
    Save::write_save();
    return 0;
}

void register_neopixel(void)
{
    neopixel_args.mode =
        arg_int1(NULL, NULL, "<pattern>", "pattern to be displayed");
    neopixel_args.brightness =
        arg_int1(NULL, NULL, "<brightness>", "neopixels brightness [0-255]");
    neopixel_args.color = arg_int1(NULL, NULL, "<color>",
                                   "neopixels brightness [0x000000-0xffffff]");
    neopixel_args.end = arg_end(3);

    const esp_console_cmd_t neopixel_cmd = {
        .command = "neopixel",
        .help = "Configure neopixels patterns and brightness",
        .hint = NULL,
        .func = &configure_neopixel,
        .argtable = &neopixel_args};

    ESP_ERROR_CHECK(esp_console_cmd_register(&neopixel_cmd));
}
