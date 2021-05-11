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
#ifndef __CAMRGB_CONTROL_H__
#define __CAMRGB_CONTROL_H__

#include <rga/rga.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*display_callback)(void *ptr, int fd, int fmt, int w, int h, int rotation);

int camrgb_control_init(void);
void camrgb_control_exit(void);
bool camrgb_control_run(void);
void set_rgb_param(int width, int height, display_callback cb);
void set_rgb_rotation(int angle);
void set_rgb_display(display_callback cb);
void set_rgb_process(display_callback cb);

int camir_control_init(void);
void camir_control_exit(void);
bool camir_control_run(void);
void set_ir_param(int width, int height, display_callback cb);
void set_ir_rotation(int angle);
void set_ir_display(display_callback cb);
void set_ir_process(display_callback cb);

#ifdef __cplusplus
}
#endif

#endif
