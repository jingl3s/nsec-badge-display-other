#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>

#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "image_viewer.h"

#ifdef SIMULATOR
#include "lv_utils.h"

#define IMAGE_DIR "images"
static const char *mount_point = ".";

extern const char *TAG;
#define ESP_LOGE(param, ...) printf(__VA_ARGS__);

static const char *get_mount(void) { return mount_point; }

#else
#include "rom/tjpgd.h"
#include "disk.h"
#include "lv_utils.h"

static const char *TAG = "image_viewer";
#define IMAGE_DIR "images"
static const char *get_mount(void) {
    return Disk::getInstance().getMountPoint();
}
#endif

#define MAX_IMAGES 128

struct decode_ctx {
    FILE *fp;
    lv_color_t *pixels;
    int stride;
    int img_w;
    int img_h;
};

static lv_obj_t *img_obj;
static lv_obj_t *label_info;
static lv_obj_t *label_status;

static char **image_files;
static int image_count;
static int current_index;

static uint8_t *pixel_buffer;
static lv_img_dsc_t img_dsc;

static const char *to_lower_ext(const char *path)
{
    const char *dot = strrchr(path, '.');
    if (!dot) return "";
    return dot + 1;
}

static bool ext_matches(const char *path, const char *ext1, const char *ext2)
{
    const char *ext = to_lower_ext(path);
    size_t len = strlen(ext);
    char lower[8];
    if (len >= sizeof(lower)) return false;
    for (size_t i = 0; i <= len; i++) {
        char c = ext[i];
        lower[i] = (c >= 'A' && c <= 'Z') ? (c + 0x20) : c;
    }
    return !strcmp(lower, ext1) || !strcmp(lower, ext2);
}

static bool is_supported(const char *path)
{
#ifdef SIMULATOR
    return ext_matches(path, "bin", "bmp");
#else
    return ext_matches(path, "jpg", "jpeg") || ext_matches(path, "bin", "bin");
#endif
}

static void free_images(void)
{
    for (int i = 0; i < image_count; i++) {
        free(image_files[i]);
    }
    free(image_files);
    image_files = NULL;
    image_count = 0;
    current_index = -1;
}

int scan_images(void)
{
    free_images();

    char dir_path[64];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", get_mount(), IMAGE_DIR);

ESP_LOGE(TAG, "dir path: %s\n", dir_path);

    DIR *dir = opendir(dir_path);
    if (!dir) {
        return 0;
    }

    image_files = (char **)calloc(MAX_IMAGES, sizeof(char *));
    if (!image_files) {
        closedir(dir);
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && image_count < MAX_IMAGES) {
        if (entry->d_type == DT_DIR) continue;
        if (!is_supported(entry->d_name)) continue;

        image_files[image_count] = strdup(entry->d_name);
        if (image_files[image_count]) {
            image_count++;
        }
    }
    closedir(dir);

    if (image_count == 0) {
        free(image_files);
        image_files = NULL;
    }

    current_index = image_count > 0 ? 0 : -1;
    ESP_LOGE(TAG, "Found %d images in %s/%s\n", image_count, get_mount(), IMAGE_DIR);
    return image_count;
}

static void free_pixels(void)
{
    free(pixel_buffer);
    pixel_buffer = NULL;
    img_dsc.data = NULL;
    img_dsc.data_size = 0;
    img_dsc.header.w = 0;
    img_dsc.header.h = 0;
}

static int load_bin(const char *full_path)
{
    FILE *fp = fopen(full_path, "rb");
    if (!fp) return -1;

    lv_img_header_t hdr;
    if (fread(&hdr, sizeof(hdr), 1, fp) != 1) { fclose(fp); return -2; }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, sizeof(hdr), SEEK_SET);

    size_t data_size = (size_t)(file_size - (long)sizeof(hdr));
    free_pixels();

    pixel_buffer = (uint8_t *)malloc(data_size);
    ESP_LOGE(TAG, "BIN data_size: %d\n", data_size);
    if (!pixel_buffer) { fclose(fp); return -3; }

    if (fread(pixel_buffer, 1, data_size, fp) != data_size) {
        free_pixels();
        fclose(fp);
        return -4;
    }
    fclose(fp);

#if LV_COLOR_DEPTH == 32
    /* If bin was generated for 16-bit LVGL, convert RGB565 → lv_color_t (32-bit) */
    size_t expected_16 = (size_t)hdr.w * hdr.h * 2;
    if (data_size == expected_16 && hdr.cf == LV_IMG_CF_TRUE_COLOR) {
        size_t expected_32 = (size_t)hdr.w * hdr.h * sizeof(lv_color_t);
        uint8_t *dst32 = (uint8_t *)malloc(expected_32);
        if (!dst32) { free_pixels(); return -5; }

        uint16_t *in = (uint16_t *)pixel_buffer;
        lv_color_t *out = (lv_color_t *)dst32;
        size_t npix = (size_t)hdr.w * hdr.h;
        for (size_t i = 0; i < npix; i++) {
            uint16_t px = in[i];
            uint8_t r5 = (px >> 11) & 0x1F;
            uint8_t g6 = (px >> 5)  & 0x3F;
            uint8_t b5 =  px        & 0x1F;
            out[i] = lv_color_make((r5 << 3) | (r5 >> 2),
                                   (g6 << 2) | (g6 >> 4),
                                   (b5 << 3) | (b5 >> 2));
        }
        free(pixel_buffer);
        pixel_buffer = dst32;
        data_size = expected_32;
    }
#endif

    img_dsc.header = hdr;
    img_dsc.data_size = data_size;
    img_dsc.data = pixel_buffer;

    return 0;
}

#ifdef SIMULATOR


static int load_bmp(const char *full_path)
{
    FILE *fp = fopen(full_path, "rb");
    if (!fp) return -1;

    uint8_t hdr[54];
    if (fread(hdr, 1, 54, fp) != 54) { fclose(fp); return -2; }

    if (hdr[0] != 'B' || hdr[1] != 'M') { fclose(fp); return -2; }

    int w = *(int32_t *)&hdr[18];
    int h = *(int32_t *)&hdr[22];
    uint16_t bpp = *(uint16_t *)&hdr[28];
    uint32_t compression = *(uint32_t *)&hdr[30];
    uint32_t data_off = *(uint32_t *)&hdr[10];

    if (bpp != 24 || compression != 0) {
        fclose(fp);
        return -3;
    }

    int abs_h = h < 0 ? -h : h;
    int bottom_up = h > 0;

    free_pixels();

    size_t buf_size = (size_t)w * abs_h * sizeof(lv_color_t);
    ESP_LOGE(TAG, "BMP buf_size: %d\n", buf_size);
    pixel_buffer = (uint8_t *)malloc(buf_size);
    if (!pixel_buffer) { fclose(fp); return -4; }

    lv_color_t *dst = (lv_color_t *)pixel_buffer;
    int row_size = ((w * 24 + 31) / 32) * 4;
    uint8_t *row = (uint8_t *)malloc(row_size);
    if (!row) { free_pixels(); fclose(fp); return -4; }

    for (int y = 0; y < abs_h; y++) {
        int src_y = bottom_up ? (abs_h - 1 - y) : y;
        fseek(fp, data_off + src_y * row_size, SEEK_SET);
        fread(row, 1, row_size, fp);

        for (int x = 0; x < w; x++) {
            uint8_t b = row[x * 3 + 0];
            uint8_t g = row[x * 3 + 1];
            uint8_t r = row[x * 3 + 2];
            dst[y * w + x] = lv_color_make(r, g, b);
        }
    }

    free(row);
    fclose(fp);

    img_dsc.header.always_zero = 0;
    img_dsc.header.w = w;
    img_dsc.header.h = abs_h;
    img_dsc.data_size = buf_size;
    img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    img_dsc.data = pixel_buffer;

    return 0;
}

#else /* !SIMULATOR - use ESP32 ROM TJpgDec */

static UINT tjpgd_in(JDEC *jd, BYTE *buf, UINT len)
{
    struct decode_ctx *ctx = (struct decode_ctx *)jd->device;
    if (buf) {
        return (UINT)fread(buf, 1, len, ctx->fp);
    }
    if (fseek(ctx->fp, len, SEEK_CUR) == 0) {
        return len;
    }
    return 0;
}

static UINT tjpgd_out(JDEC *jd, void *bitmap, JRECT *rect)
{
    struct decode_ctx *ctx = (struct decode_ctx *)jd->device;
    lv_color_t *dst = ctx->pixels;
    int stride = ctx->stride;
    BYTE *src = (BYTE *)bitmap;

    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            BYTE r = *src++;
            BYTE g = *src++;
            BYTE b = *src++;
            dst[y * stride + x] = lv_color_make(r, g, b);
        }
    }
    return 1;
}

/* TJpgDec work area — static to avoid blowing the LVGL task stack.
 * Minimum is 3092 bytes; 8 KB gives headroom for complex Huffman tables. */
static uint8_t s_jpeg_pool[8192];

#define DISPLAY_W 320
#define DISPLAY_H 160  /* 320x160x2 = 102400 bytes, fits in ~110KB heap */

static int load_jpeg(const char *full_path)
{
    FILE *fp = fopen(full_path, "rb");
    if (!fp) return -1;

    struct decode_ctx ctx;
    ctx.fp = fp;
    ctx.pixels = NULL;
    ctx.stride = 0;

    JDEC jd;
    JRESULT res = jd_prepare(&jd, tjpgd_in, s_jpeg_pool, sizeof(s_jpeg_pool), &ctx);
    if (res != JDR_OK) {
        const char *reason = (res == 6) ? "format error" :
                             (res == 7) ? "progressive JPEG not supported (use baseline)" :
                             (res == 8) ? "unsupported sampling factor" :
                             (res == 3) ? "pool too small" : "unknown";
        ESP_LOGE(TAG, "jd_prepare failed: %d (%s)\n", res, reason);
        fclose(fp);
        return -2;
    }

    /* Choose scale so decoded pixels fit in available heap and display width.
     * Leave 4 KB margin; DISPLAY_H is a soft target only — if the image fits
     * in memory at native height, skip the scale even if height > DISPLAY_H. */
    size_t avail_bytes = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    size_t max_pixels  = avail_bytes / sizeof(lv_color_t);

    uint8_t scale = 0;
    while (scale < 3) {
        size_t out_w = (size_t)(jd.width  >> scale);
        size_t out_h = (size_t)(jd.height >> scale);
        if (out_w <= DISPLAY_W && out_w * out_h <= max_pixels)
            break;
        scale++;
    }

    int out_w = jd.width >> scale;
    int out_h = jd.height >> scale;
    ESP_LOGE(TAG, "JPEG %dx%d scale 1/%d → %dx%d\n",
             jd.width, jd.height, (1 << scale), out_w, out_h);

    /* Guard against corrupt headers reporting absurd dimensions */
    if (out_w <= 0 || out_h <= 0 || out_w > 2048 || out_h > 2048) {
        fclose(fp);
        return -5;
    }

    size_t buf_size = (size_t)out_w * out_h * sizeof(lv_color_t);

    // size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    // ESP_LOGE(TAG, "Heap largest free block: %u bytes, need: %u bytes\n",
    //          (unsigned)largest, (unsigned)buf_size);

    pixel_buffer = (uint8_t *)malloc(buf_size);
    if (!pixel_buffer) {
        ESP_LOGE(TAG, "malloc %u bytes failed\n", (unsigned)buf_size);
        fclose(fp);
        return -3;
    }
    ESP_LOGE(TAG, "malloc %u bytes OK\n", (unsigned)buf_size);

    ctx.pixels = (lv_color_t *)pixel_buffer;
    ctx.stride = out_w;

    res = jd_decomp(&jd, tjpgd_out, scale);
    fclose(fp);

    if (res != JDR_OK) {
        ESP_LOGE(TAG, "jd_decomp failed: %d\n", res);
        free_pixels();
        return -4;
    }

    img_dsc.header.always_zero = 0;
    img_dsc.header.w = out_w;
    img_dsc.header.h = out_h;
    img_dsc.data_size = buf_size;
    img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    img_dsc.data = pixel_buffer;

    return 0;
}

#endif /* SIMULATOR */

static int decode_image(const char *full_path)
{
    if (ext_matches(full_path, "bin", "bin")) return load_bin(full_path);
#ifdef SIMULATOR
    return load_bmp(full_path);
#else
    return load_jpeg(full_path);
#endif
}

void show_current_image(void)
{
    if (current_index < 0 || current_index >= image_count) {
        lv_obj_set_hidden(img_obj, true);
        lv_obj_set_hidden(label_info, true);
        lv_label_set_text(label_status, "Aucune image trouvee");
        lv_obj_set_hidden(label_status, false);
        return;
    }

    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s/%s/%s",
             get_mount(), IMAGE_DIR, image_files[current_index]);
    ESP_LOGE(TAG, "Displaying image: %s\n", full_path);

    /* Detach LVGL from the old buffer before freeing it */
    lv_obj_set_hidden(img_obj, true);
    lv_img_set_src(img_obj, NULL);
    free_pixels();

    int err = decode_image(full_path);
    if (err != 0) {
        lv_obj_set_hidden(img_obj, true);
        lv_obj_set_hidden(label_info, true);
        lv_obj_set_hidden(label_status, false);
        lv_label_set_text_fmt(label_status, "Erreur %d: %s", err,
                              image_files[current_index]);
        return;
    }

    lv_img_set_src(img_obj, &img_dsc);
    lv_obj_align(img_obj, lv_obj_get_parent(img_obj), LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_set_hidden(img_obj, false);
    lv_obj_set_hidden(label_status, true);

    lv_label_set_text_fmt(label_info, "%d/%d - %s", current_index + 1,
                          image_count, image_files[current_index]);
    lv_obj_set_hidden(label_info, false);
}

static void advance_to_next(void)
/*Affiche l'image et prepare la suivante*/
{
    if (image_count == 0) return;
    show_current_image();
    current_index = (current_index + 1) % image_count;
}

static void img_click_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        advance_to_next();
    }
}

lv_obj_t *tab_images_init_real(lv_obj_t *tab_view, const char *tab_name)
{
    lv_obj_t *parent = lv_tabview_add_tab(tab_view, tab_name);
    lv_page_set_scrl_layout(parent, LV_LAYOUT_OFF);
    lv_page_set_scrollbar_mode(parent, LV_SCRLBAR_MODE_OFF);

    lv_obj_set_style_local_bg_color(parent, LV_PAGE_PART_BG, LV_STATE_DEFAULT,
                                    LV_COLOR_BLACK);
    lv_obj_set_style_local_text_color(parent, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, LV_COLOR_WHITE);

    img_obj = lv_img_create(parent, NULL);
    lv_obj_set_event_cb(img_obj, img_click_handler);
    lv_obj_set_hidden(img_obj, true);

    label_info = lv_label_create(parent, NULL);
    lv_obj_set_style_local_text_color(label_info, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(label_info, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa(label_info, LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT, LV_OPA_50);
    lv_obj_set_pos(label_info, 0, 220);
    lv_obj_set_size(label_info, 320, 20);
    lv_obj_set_hidden(label_info, true);

    label_status = lv_label_create(parent, NULL);
    lv_label_set_text(label_status, "Aucune image");
    lv_obj_set_style_local_text_color(label_status, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_align(label_status, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *cup_button_load = lv_btn_create(parent, NULL);
    lv_obj_set_width(cup_button_load, 50);
    lv_obj_set_height(cup_button_load, 50);
    lv_obj_set_pos(cup_button_load, 270, 150);
    lv_obj_set_style_local_radius(cup_button_load, LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(cup_button_load, LV_OBJ_PART_MAIN,
                                        LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(cup_button_load, LV_OBJ_PART_MAIN,
                                        LV_STATE_PRESSED, 0);
    lv_obj_set_style_local_border_width(cup_button_load, LV_OBJ_PART_MAIN,
                                        LV_STATE_FOCUSED, 0);
    lv_obj_set_style_local_bg_opa(cup_button_load, LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(cup_button_load, LV_OBJ_PART_MAIN,
                                  LV_STATE_PRESSED, LV_OPA_TRANSP);
    lv_obj_set_event_cb(cup_button_load, img_click_handler);
    lv_obj_t *cup_label_load = lv_label_create(cup_button_load, NULL);
    lv_label_set_text(cup_label_load, ">");

#ifdef SIMULATOR
    scan_images();
#endif
    return parent;
}
