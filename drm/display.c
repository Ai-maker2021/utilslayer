/*
 * Copyright (C) 2020 Rockchip Electronics Co., Ltd.
 * author: Zhihua Wang, hogan.wang@rock-chips.com
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <rga/RgaApi.h>
#include "display.h"
#include "rkdrm_display.h"
#include "camir_control.h"
#include "cam_control.h"
#include "RockchipRgaMacro.h"
#include "draw_rect.h"

#define BUF_COUNT 3
#define USE_NV12

struct display {
    int fmt;
    int width;
    int height;
    int ir_width;
    int ir_height;
    int plane_type;
    struct drm_dev dev;
    struct drm_buf buf[BUF_COUNT];
    struct drm_buf ir_buf;
    int buf_cnt;
    int rga_fmt;
};

static struct display g_disp;

static bool ready = false;
static MRECT g_rect = {0};
static enum display_face_type g_type = DISPLAY_FACE_NORMAL;
static pthread_mutex_t g_display_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_displayRotation = 0;
static int g_hm_show = 0;
static unsigned char* g_heatmap;
static int g_hm_width = 320;
static int g_hm_height = 320;

static int drm_display_init(struct display *disp)
{
    int ret;
    if (drmInit(&disp->dev)) {
        fprintf(stderr, "drmInit: Failed\n");
        return -1;
    }

    for (int i = 0; i < disp->buf_cnt; i++) {
        ret = drmGetBuffer(disp->dev.drm_fd, disp->width, disp->height, disp->fmt, &disp->buf[i]);
        if (ret) {
            fprintf(stderr, "Alloc drm buffer failed, %d\n", i);
            return -1;
        }
    }

    ret = drmGetBuffer(disp->dev.drm_fd, disp->ir_width, disp->ir_height, disp->fmt, &disp->ir_buf);
    if (ret) {
        fprintf(stderr, "Alloc drm ir buffer failed\n");
        return -1;
    }

    return 0;
}

static int disp_init(struct display* disp, int width, int height, int count) {
#ifdef USE_NV12
    disp->fmt = DRM_FORMAT_NV12;
    disp->rga_fmt = RK_FORMAT_YCbCr_420_SP;
#endif
#ifdef USE_RGB888
    g_disp.fmt = DRM_FORMAT_BGR888;
    g_disp.rga_fmt = RK_FORMAT_RGB_888;
#endif
    disp->width = width;
    disp->height = height;
    disp->ir_width = 180;
    disp->ir_height = 320;
    disp->plane_type = DRM_PLANE_TYPE_OVERLAY;
    disp->buf_cnt = count;
    return drm_display_init(&g_disp);
}

int display_init(int width, int height, int displayRotation)
{
    g_displayRotation = displayRotation;

    int ret;
    ret = disp_init(&g_disp, width, height, BUF_COUNT);

    return ret;
}


static void drm_display_exit(struct display *disp)
{
    ready = false;
    drmDeinit(&disp->dev);
    for (int i = 0; i < disp->buf_cnt; i++)
        drmPutBuffer(disp->dev.drm_fd, &disp->buf[i]);
    drmPutBuffer(disp->dev.drm_fd, &disp->ir_buf);
    if (g_heatmap) {
        free(g_heatmap);
        g_heatmap = NULL;
    }
}

void display_exit(void)
{
    drm_display_exit(&g_disp);
}

YUV_COLOR getColor(enum display_face_type type) {
    YUV_COLOR color;
    if (type == DISPLAY_FACE_SUCCESS) {
        color.y = 179;
        color.u = 171;
        color.v = 1;
    } else if (type == DISPLAY_FACE_UNKOWN) {
        color.y = 76;
        color.u = 85;
        color.v = 255;
    } else if (type == DISPLAY_FACE_PERMISSON) {
        color.y = 197;
        color.u = 17;
        color.v = 169;
    } else {
        color.y = 252;
        color.u = 128;
        color.v = 128;
    }
    return color;
}

static inline int getDrmRotation(int rotation) {
    int drmRotation = HAL_TRANSFORM_ROT_0;
    if (rotation == 90)
        drmRotation = HAL_TRANSFORM_ROT_90;
    else if (rotation == 270)
        drmRotation = HAL_TRANSFORM_ROT_270;
    else if (rotation == 180)
        drmRotation = HAL_TRANSFORM_ROT_180;
    return drmRotation;
}

static inline int getRotation(int drmRotation) {
    int rotation = 0;
    if (drmRotation == HAL_TRANSFORM_ROT_90)
        rotation = 90;
    else if (drmRotation == HAL_TRANSFORM_ROT_270)
        rotation = 270;
    else if (drmRotation == HAL_TRANSFORM_ROT_180)
        rotation = 180;
    return rotation;
}

static void drawHeatmap(struct display *disp, int num) {
    rga_info_t src, dst;
    if (g_hm_show && g_heatmap != NULL)
    {
        memset(&src, 0, sizeof(rga_info_t));
        src.fd = -1;
        src.virAddr = g_heatmap;
        src.mmuFlag = 1;
        rga_set_rect(&src.rect, 0, 0, g_hm_width, g_hm_height, g_hm_width, g_hm_height, RK_FORMAT_BGRA_8888);
        memset(&dst, 0, sizeof(rga_info_t));
        dst.fd = -1;
        dst.virAddr = disp->buf[num].map;
        dst.mmuFlag = 1;
        rga_set_rect(&dst.rect, 20, 150, g_hm_width, g_hm_height, disp->width, disp->height, disp->rga_fmt);
        if (c_RkRgaBlit(&src, &dst, NULL)) {
            printf("%s: rga fail\n", __func__);
            return;
        }
    }
}

void drm_commit(struct display *disp, int num, void *ptr, int fd, int fmt, int w, int h, int rotation)
{
    int ret;
    rga_info_t src, dst;
    char *map = disp->buf[num].map;
    int dst_w = disp->width;
    int dst_h = disp->height;
    int dst_fmt = disp->rga_fmt;

    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.virAddr = ptr;
    src.mmuFlag = 1;
    src.rotation = getDrmRotation((getRotation(rotation) + g_displayRotation)%360);
    rga_set_rect(&src.rect, 0, 0, w, h, w, h, fmt);
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.virAddr = map;
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, dst_fmt);
    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        return;
    }

#ifdef DISPLAY_IR
    if (ready) {
        int ir_w = disp->ir_width;
        int ir_h = disp->ir_height;
        memset(&src, 0, sizeof(rga_info_t));
        src.fd = -1;
        src.virAddr = disp->ir_buf.map;
        src.mmuFlag = 1;
        rga_set_rect(&src.rect, 0, 0, ir_w, ir_h, ir_w, ir_h, dst_fmt);
        memset(&dst, 0, sizeof(rga_info_t));
        dst.fd = -1;
        dst.virAddr = map;
        dst.mmuFlag = 1;
        rga_set_rect(&dst.rect, dst_w-ir_w-20, 150, ir_w, ir_h, dst_w, dst_h, dst_fmt);
        if (c_RkRgaBlit(&src, &dst, NULL)) {
            printf("%s: rga fail\n", __func__);
            return;
        }
    }
#endif

    pthread_mutex_lock(&g_display_lock);
    drawHeatmap(disp, num);
    drawRect(map,dst_w,dst_h,getColor(g_type),g_rect);
    pthread_mutex_unlock(&g_display_lock);

    ret = drmCommit(&disp->buf[num], disp->width, disp->height, 0, 0, &disp->dev, disp->plane_type);
    if (ret) {
        fprintf(stderr, "display commit error, ret = %d\n", ret);
    }
}

void display_commit(void *ptr, int fd, int fmt, int w, int h, int rotation)
{
    static int num = 0;

    drm_commit(&g_disp, num, ptr, fd, fmt, w, h, rotation);
    num = (num + 1) % BUF_COUNT;
}

void drm_ir_commit(struct display *disp, void *ptr, int fd, int fmt, int w, int h, int rotation)
{
    rga_info_t src, dst;
    char *map = disp->ir_buf.map;
    int dst_w = disp->ir_width;
    int dst_h = disp->ir_height;
    int dst_fmt = disp->rga_fmt;

    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.virAddr = ptr;
    src.mmuFlag = 1;
    src.rotation = getDrmRotation((getRotation(rotation) + g_displayRotation)%360);
    rga_set_rect(&src.rect, 0, 0, w, h, w, h, fmt);
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.virAddr = map;
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, dst_fmt);
    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        return;
    }
    ready = true;
}

void display_ir_commit(void *ptr, int fd, int fmt, int w, int h, int rotation)
{
    drm_ir_commit(&g_disp, ptr, fd, fmt, w, h, rotation);
}

void display_switch(enum display_video_type type)
{
    set_rgb_display(NULL);
    set_ir_display(NULL);
//    set_usb_display(NULL);
    if (type == DISPLAY_VIDEO_RGB) {
        set_rgb_display(display_commit);
#ifdef DISPLAY_IR
        set_ir_display(display_ir_commit);
#endif
    }
    else if (type == DISPLAY_VIDEO_IR)
        set_ir_display(display_commit);
//    else if (type == DISPLAY_VIDEO_USB)
//        set_usb_display(display_commit);
}

void display_get_resolution(int *width, int *height)
{
    *width = g_disp.width;
    *height = g_disp.height;
}

void display_set_info(enum display_face_type type) {
    pthread_mutex_lock(&g_display_lock);
    g_type = type;
    pthread_mutex_unlock(&g_display_lock);
}

void display_set_rect(MRECT rc) {
    pthread_mutex_lock(&g_display_lock);
    g_rect = rc;
    pthread_mutex_unlock(&g_display_lock);
}

void display_set_show_heatmap(int flag) {
    pthread_mutex_lock(&g_display_lock);
    g_hm_show = flag;
    pthread_mutex_unlock(&g_display_lock);
}

void display_set_heatmap(unsigned char* heatmap, int w, int h) {
    pthread_mutex_lock(&g_display_lock);
    g_hm_width = w;
    g_hm_height = h;
    size_t size = (size_t)(w*h*4);
    if (g_heatmap == NULL) {
        g_heatmap = malloc(size);
    }
    memcpy(g_heatmap,heatmap,size);
    pthread_mutex_unlock(&g_display_lock);
}


