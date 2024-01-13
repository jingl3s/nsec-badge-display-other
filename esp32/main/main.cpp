#include "buzzer.h"
#include "console.h"
#ifdef SDCARD_ENABLED
#include "disk.h"
#endif
#include "display.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "neopixel.h"
#include "nvs_flash.h"
#include "save.h"
#include "sdkconfig.h"
#include "display_ledc_backlight.h"
#include <inttypes.h>
#include <stdio.h>

#define TAG "main"

#ifdef SDCARD_ENABLED

static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

esp_err_t initialize_storage()
{
    ESP_LOGI(TAG, "Mounting /flags filesystem");
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 1,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(
        "/flags", "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}
#endif

static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

extern "C" void app_main(void)
{

    initialize_nvs();
    fflush(stdout);

    Save::load_save();
    Save::load_and_set_log_levels();
#ifdef SDCARD_ENABLED
    initialize_storage();
#endif    
    if (Save::save_data.neopixel_is_on) {
        NeoPixel::getInstance().init();
    }
    // NeoPixel::getInstance().stop();
    DisplayLedcBacklight::getInstance().init();

    Buzzer::getInstance().init();
    // Buzzer::getInstance().play(Buzzer::Sounds::Mode1);
    // Buzzer::getInstance().play(Buzzer::Sounds::Mode2);
    // Buzzer::getInstance().play(Buzzer::Sounds::Mode3);
#ifdef SDCARD_ENABLED
    Disk::getInstance().init();
#endif
    Display::getInstance().init();

    console_create_task();
}
