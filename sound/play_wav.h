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
#ifndef __PLAY_WAV_H__
#define __PLAY_WAV_H__

#include <string>
#include <list>

#ifdef __cplusplus
extern "C" {
#endif

#define WELCOME_WAV "/etc/welcome.wav"
#define RECOGNITION_SUCCESS_WAV "recognition_success.wav"
#define RECOGNITION_SUCCESS_2 "door_opened.wav"
#define RECOGNITION_SUCCESS_3 "welcome.wav"
#define RECOGNITION_SUCCESS_4 "door_opened_welcome.wav"

#define RECOGNITION_FAILED_WAV "failed_default.wav"
#define RECOGNITION_FAILED_2 "recognition_failed.wav"
#define RECOGNITION_FAILED_3 "verification_failed.wav"

#define RECOGNITION_PERMISSION "no_permission.wav"

void play_wav_signal(std::list<std::string>* pWavList);
int play_wav_thread_init(void);
void play_wav_thread_exit(void);
int volume_set(int vol);
int volume_get(void);

#ifdef __cplusplus
}
#endif

#endif
