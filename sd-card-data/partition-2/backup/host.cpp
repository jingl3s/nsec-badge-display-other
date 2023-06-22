#include <string.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_err.h>

#ifdef CONFIG_BT_BLUEDROID_ENABLED
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#endif

#ifdef CONFIG_BT_NIMBLE_ENABLED
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#endif

#include "badge/mesh/host.h"

static const char *TAG = "badge/mesh";

uint8_t _device_address[6] = {0};
bool mesh_host_initialized = false;

#ifdef CONFIG_BT_BLUEDROID_ENABLED

esp_err_t mesh_host_initialize(void)
{
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed (%s)", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed (%s)", __func__, esp_err_to_name(ret));
        goto fail;
    }
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed (%s)", __func__, esp_err_to_name(ret));
        goto fail;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluetooth failed (%s)", __func__, esp_err_to_name(ret));
        goto fail;
    }

    memcpy(&_device_address, esp_bt_dev_get_address(), ESP_BD_ADDR_LEN);
    mesh_host_initialized = true;

    return ret;
fail:
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    return ret;
}

esp_err_t mesh_host_deinit(void)
{
    esp_err_t ret;

    ret = esp_bluedroid_disable();
    if (ret) {
        ESP_LOGE(TAG, "%s esp_bluedroid_disable failed (%s)", __func__, esp_err_to_name(ret));
    }

    ret = esp_bluedroid_deinit();
    if (ret) {
        ESP_LOGE(TAG, "%s esp_bluedroid_deinit failed (%s)", __func__, esp_err_to_name(ret));
    }

    ret = esp_bt_controller_disable();
    if (ret) {
        ESP_LOGE(TAG, "%s esp_bt_controller_disable failed (%s)", __func__, esp_err_to_name(ret));
    }

    ret = esp_bt_controller_deinit();
    if (ret) {
        ESP_LOGE(TAG, "%s esp_bt_controller_deinit failed (%s)", __func__, esp_err_to_name(ret));
    }

    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        ESP_LOGE(TAG, "%s esp_bt_controller_mem_release failed (%s)", __func__, esp_err_to_name(ret));
    }

    return ESP_OK;
}

#else /* CONFIG_BT_BLUEDROID_ENABLED */

static SemaphoreHandle_t mesh_sem;
uint8_t _device_address_type;

static void mesh_on_reset(int reason)
{
    ESP_LOGI(TAG, "Resetting state; reason=%d", reason);
}

static void mesh_on_sync(void)
{
    int rc;

    ESP_LOGV(TAG, "%s: Bluetooth initialized", __func__);

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &_device_address_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "error determining address type; rc=%d", rc);
        goto cleanup;
    }

    rc = ble_hs_id_copy_addr(_device_address_type, _device_address, NULL);
	if (rc) {
		ESP_LOGE(TAG, "Failed to get own address; rc=%d", rc);
		goto cleanup;
	}

    mesh_host_initialized = true;

cleanup:
    xSemaphoreGive(mesh_sem);
}

static void mesh_host_task(void *param)
{
    ESP_LOGV(TAG, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

esp_err_t mesh_host_initialize(void)
{
    mesh_sem = xSemaphoreCreateBinary();
    if (mesh_sem == NULL) {
        ESP_LOGE(TAG, "Failed to create mesh semaphore");
        return ESP_FAIL;
    }

    nimble_port_init();
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = mesh_on_reset;
    ble_hs_cfg.sync_cb = mesh_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    nimble_port_freertos_init(mesh_host_task);

    xSemaphoreTake(mesh_sem, portMAX_DELAY);

    return ESP_OK;
}

esp_err_t mesh_host_deinit(void)
{
    nimble_port_freertos_deinit();
    nimble_port_deinit();

    mesh_host_initialized = false;

    vSemaphoreDelete(mesh_sem);
    mesh_sem = NULL;

    return ESP_OK;
}

#endif /* CONFIG_BT_BLUEDROID_ENABLED */