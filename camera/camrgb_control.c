/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
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
#include <pthread.h>
#include <stdbool.h>

#include "rkisp_api.h"
#include "cam_control.h"
#include "RockchipRgaMacro.h"

static const struct rkisp_api_ctx *ctx;
static const struct rkisp_api_buf *buf;
static bool g_run;
static pthread_t g_tid;

static int g_rgb_width;
static int g_rgb_height;

//#define HAL_TRANSFORM_ROT_90 90
//#define HAL_TRANSFORM_ROT_270 270
//#define RK_FORMAT_YCbCr_420_SP 0xa

static display_callback g_display_cb = NULL;
static pthread_mutex_t g_display_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_rotation = HAL_TRANSFORM_ROT_90;
static display_callback g_process_cb = NULL;

void set_rgb_rotation(int angle)
{
    if (angle == 90)
        g_rotation = HAL_TRANSFORM_ROT_90;
    else if (angle == 270)
        g_rotation = HAL_TRANSFORM_ROT_270;
    else if (angle == 0)
        g_rotation = HAL_TRANSFORM_ROT_0;
    else if (angle == 180)
        g_rotation = HAL_TRANSFORM_ROT_180;
}

void set_rgb_display(display_callback cb)
{
    pthread_mutex_lock(&g_display_lock);
    g_display_cb = cb;
    pthread_mutex_unlock(&g_display_lock);
}

void set_rgb_process(display_callback cb)
{
    pthread_mutex_lock(&g_display_lock);
    g_process_cb = cb;
    pthread_mutex_unlock(&g_display_lock);
}

void set_rgb_param(int width, int height, display_callback cb)
{
    g_rgb_width = width;
    g_rgb_height = height;
    set_rgb_display(cb);
}

static inline void camrgb_inc_fps(void)
{
    static int fps = 0;
    static struct timeval t0;
    struct timeval t1;

    fps++;
    gettimeofday(&t1, NULL);
    if (t1.tv_sec - t0.tv_sec > 1) {
        fps = 0;
        gettimeofday(&t0, NULL);
    } else if ((t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec) > 1000000) {
        printf("ISP rgb fps: %d\n", fps);
        fps = 0;
        gettimeofday(&t0, NULL);
    }
}

static void *process(void *arg)
{
    do {
#if 0
        camrgb_inc_fps();
#endif
        buf = rkisp_get_frame(ctx, 0);
        if (buf == NULL){
            continue;
        }

        if (g_process_cb)
            g_process_cb(buf->buf, buf->fd, RK_FORMAT_YCbCr_420_SP,
                    ctx->width, ctx->height, g_rotation);

        pthread_mutex_lock(&g_display_lock);
        if (g_display_cb)
            g_display_cb(buf->buf, buf->fd, RK_FORMAT_YCbCr_420_SP,
                    ctx->width, ctx->height, g_rotation);
        pthread_mutex_unlock(&g_display_lock);

        rkisp_put_frame(ctx, buf);
    } while (g_run);

    pthread_exit(NULL);
}

int camrgb_control_init(void)
{
    int id = -1;
    char name[32];

#ifdef CAMERA_ENGINE_RKISP
//    id = get_video_id("rkisp_mainpath");
//    if (id < 0) {
//        printf("%s: get video id fail!\n", __func__);
//        return -1;
//    }
//    snprintf(name, sizeof(name), "/dev/video%d", id);
//    printf("%s: %s\n", __func__, name);
//    ctx = rkisp_open_device(name, 1);
//    ctx = rkisp_open_device2(CAM_TYPE_RKCIF);
    ctx = rkisp_open_device2(CAM_TYPE_RKISP1);
#endif
//#ifdef CAMERA_ENGINE_RKAIQ
    ctx = rkisp_open_device2(CAM_TYPE_RKISP1);
//#endif
    if (ctx == NULL) {
        printf("%s: ctx is NULL\n", __func__);
        return -1;
    }

    rkisp_set_buf(ctx, 3, NULL, 0);

//    rkisp_set_sensor_fmt(ctx, g_rgb_width, g_rgb_height, MEDIA_BUS_FMT_YUYV8_2X8);

    if(rkisp_set_fmt(ctx, g_rgb_width, g_rgb_height, V4L2_PIX_FMT_NV12))
        return -1;

    if (rkisp_start_capture(ctx))
        return -1;

#ifdef CAMERA_ENGINE_RKISP
//    rkisp_get_expo_weights(ctx, weights, sizeof(weights));
//    printf("default weights:\n");
//    for (int i = 0; i < 81; i++) {
//        printf("0x%02x ", weights[i]);
//        if ((i + 1) % 9 == 0)
//            printf("\n");
//    }
//    printf("\n");
#endif

    g_run = true;
    if (pthread_create(&g_tid, NULL, process, NULL)) {
        printf("pthread_create fail\n");
        return -1;
    }

    return 0;
}

void camrgb_control_exit(void)
{
    g_run = false;
    if (g_tid) {
        pthread_join(g_tid, NULL);
        g_tid = 0;
    }

    rkisp_stop_capture(ctx);
    rkisp_close_device(ctx);
}

bool camrgb_control_run(void) {
    return g_run;
}



