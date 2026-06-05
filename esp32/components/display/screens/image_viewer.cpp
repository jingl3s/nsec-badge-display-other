#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>

#include "lv_conf.h"
#include "lvgl/lvgl.h"

#ifdef SIMULATOR
#include "lv_utils.h"

#define IMAGE_DIR "images"
static const char *mount_point = ".";

static const char *get_mount(void) { return mount_point; }

#else
#include "rom/tjpgd.h"
#include "disk.h"
#include "lv_utils.h"

#define IMAGE_DIR "images"
static const char *get_mount(void) {
    return Disk::getInstance().getMountPoint();
}
#endif

#define MAX_IMAGES 128

struct decode_ctx {
    FILE *fp;
    uint16_t *pixels;
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
    return ext_matches(path, "bmp", "bmp");
#else
    return ext_matches(path, "jpg", "jpeg");
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

static int scan_images(void)
{
    free_images();

    char dir_path[64];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", get_mount(), IMAGE_DIR);

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

    size_t buf_size = (size_t)w * abs_h * 2;
    pixel_buffer = (uint8_t *)malloc(buf_size);
    if (!pixel_buffer) { fclose(fp); return -4; }

    uint16_t *dst = (uint16_t *)pixel_buffer;
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
            dst[y * w + x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
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
    uint16_t *dst = ctx->pixels;
    int stride = ctx->stride;
    BYTE *src = (BYTE *)bitmap;

    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            BYTE r = *src++;
            BYTE g = *src++;
            BYTE b = *src++;
            dst[y * stride + x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        }
    }
    return 1;
}

static int load_jpeg(const char *full_path)
{
    FILE *fp = fopen(full_path, "rb");
    if (!fp) return -1;

    struct decode_ctx ctx;
    ctx.fp = fp;
    ctx.pixels = NULL;
    ctx.stride = 0;

    JDEC jd;
    uint8_t pool[4096];
    JRESULT res = jd_prepare(&jd, tjpgd_in, pool, sizeof(pool), &ctx);
    if (res != JDR_OK) { fclose(fp); return -2; }

    int out_w = jd.width;
    int out_h = jd.height;

    free_pixels();

    size_t buf_size = (size_t)out_w * out_h * 2;
    pixel_buffer = (uint8_t *)malloc(buf_size);
    if (!pixel_buffer) { fclose(fp); return -3; }

    ctx.pixels = (uint16_t *)pixel_buffer;
    ctx.stride = out_w;

    res = jd_decomp(&jd, tjpgd_out, 0);
    fclose(fp);

    if (res != JDR_OK) { free_pixels(); return -4; }

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
#ifdef SIMULATOR
    return load_bmp(full_path);
#else
    return load_jpeg(full_path);
#endif
}

static void show_current_image(void)
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
    lv_obj_set_hidden(img_obj, false);
    lv_obj_set_hidden(label_status, true);

    lv_label_set_text_fmt(label_info, "%d/%d - %s", current_index + 1,
                          image_count, image_files[current_index]);
    lv_obj_set_hidden(label_info, false);
}

static void advance_to_next(void)
{
    if (image_count == 0) return;
    current_index = (current_index + 1) % image_count;
    show_current_image();
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
    lv_obj_set_style_local_image_recolor_opa(img_obj, LV_IMG_PART_MAIN,
                                             LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_event_cb(img_obj, img_click_handler);

    lv_obj_set_pos(img_obj, 0, 0);
    lv_obj_set_size(img_obj, 320, 200);
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
    lv_label_set_text(label_status, "Aucune carte SD detectee");
    lv_obj_set_style_local_text_color(label_status, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_align(label_status, NULL, LV_ALIGN_CENTER, 0, 0);

    scan_images();
    show_current_image();

    return parent;
}
