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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <unistd.h>

#include <list>
#include <string>

#include "play_wav.h"

#define USE_APLAY 0

#define SOUND_NAME "hw:0,0"
#define NUM_CHANNELS 2
#define SAMPLE_RATE 44100
//#define NUM_CHANNELS 1
//#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE 16

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

static std::list<std::list<std::string>> pathList;

static snd_pcm_t *g_handle = nullptr;
static char *g_buffer = nullptr;
static size_t g_size;

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
static pthread_t g_tid;
static bool g_run;

static char alsa_mix_dev[] = "default";
static char alsa_mix_ctrl[] = "Master";

static FILE* wav_file_check(char *name)
{
    FILE *fp;
    struct riff_wave_header wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt;
    unsigned int more_chunks = 1;

    fp = fopen64(name, "rb");
    if (!fp) {
        fprintf(stderr, "failed to open '%s'\n", name);
        return NULL;
    }
    if (fread(&wave_header, sizeof(wave_header), 1, fp) != 1) {
        fprintf(stderr, "error: '%s' does not contain a riff/wave header\n", name);
        goto exit;
    }
    if ((wave_header.riff_id != ID_RIFF) || (wave_header.wave_id != ID_WAVE)) {
        fprintf(stderr, "error: '%s' is not a riff/wave file\n", name);
        goto exit;
    }
    do {
        if (fread(&chunk_header, sizeof(chunk_header), 1, fp) != 1) {
            fprintf(stderr, "error: '%s' does not contain a data chunk\n", name);
            goto exit;
        }
        switch (chunk_header.id) {
        case ID_FMT:
            if (fread(&chunk_fmt, sizeof(chunk_fmt), 1, fp) != 1) {\
                fprintf(stderr, "error: '%s' has incomplete format chunk\n", name);
                goto exit;
            }
            if (chunk_header.sz > sizeof(chunk_fmt))
                fseek(fp, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
            break;
        case ID_DATA:
            more_chunks = 0;
            break;
        default:
            fseek(fp, chunk_header.sz, SEEK_CUR);
            break;
        }
    } while (more_chunks);

//    printf("%d,%d,%d,%d\n",chunk_fmt.num_channels,chunk_fmt.sample_rate,chunk_fmt.bits_per_sample,chunk_fmt.block_align);

    if (/*chunk_fmt.num_channels != NUM_CHANNELS ||*/
        chunk_fmt.sample_rate != SAMPLE_RATE ||
        chunk_fmt.bits_per_sample != BITS_PER_SAMPLE) {
        fprintf(stderr, "%s is not %d num_channels, %d sample_rate, %d bits_per_sample\n",
                name, NUM_CHANNELS, SAMPLE_RATE, BITS_PER_SAMPLE);
        goto exit;
    }

    return fp;

exit:
    fclose(fp);
    return NULL;
}

static int play_wav_init(void)
{
    int rc;
    int dir = 0;
    unsigned int val;
    snd_pcm_uframes_t frames;
    snd_pcm_hw_params_t *params;

    rc = snd_pcm_open(&g_handle, alsa_mix_dev, SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        return -1;
    }

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(g_handle, params);
    snd_pcm_hw_params_set_access(g_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(g_handle, params, SND_PCM_FORMAT_S16);
    snd_pcm_hw_params_set_channels(g_handle, params, NUM_CHANNELS);

    val = SAMPLE_RATE;
    snd_pcm_hw_params_set_rate(g_handle, params, val, dir);

    frames = 32 * NUM_CHANNELS;
    snd_pcm_hw_params_set_period_size_near(g_handle, params, &frames, &dir);

    rc = snd_pcm_hw_params(g_handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        snd_pcm_close(g_handle);
        g_handle = nullptr;
        return -1;
    }

    snd_pcm_hw_params_get_period_size(params, &frames, &dir);

    g_size = frames * 2 * NUM_CHANNELS;
    g_buffer = static_cast<char*>(malloc(g_size));
    if (!g_buffer) {
        fprintf(stderr, "Not enough Memory!\n");
        snd_pcm_close(g_handle);
        g_handle = nullptr;
        return -1;
    }

    return 0;
}

static void play_wav_exit()
{
    if (g_handle != nullptr) {
//        snd_pcm_drop(g_handle);
        snd_pcm_drain(g_handle);
        snd_pcm_close(g_handle);
        g_handle = nullptr;
    }

    if (g_buffer != nullptr) {
        free(g_buffer);
        g_buffer = nullptr;
    }
}

static void play_wav(char *name)
{
    if (!g_handle || !g_buffer) {
        return;
    }

    int rc;
    size_t num_read;
    FILE* fp = wav_file_check(name);

    if (!fp)
        return;

    do {
        num_read = fread(g_buffer, 1, g_size, fp);
        if (num_read > 0) {
            rc = snd_pcm_writei(g_handle, g_buffer, snd_pcm_bytes_to_frames(g_handle, num_read));
            if (rc == -EPIPE) {
//                fprintf(stderr, "underrun occurred\n");
                snd_pcm_prepare(g_handle);
            } else if (rc < 0) {
                fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
                break;
            }
        }
    } while (num_read > 0);

    fclose(fp);
}

static void play_wav_wait(void)
{
    pthread_mutex_lock(&g_mutex);
    pthread_cond_wait(&g_cond, &g_mutex);
    pthread_mutex_unlock(&g_mutex);
}

void play_wav_signal(std::list<std::string>* wavList)
{
    pthread_mutex_lock(&g_mutex);
    pathList.clear();
    if (wavList && wavList->size()) {
        pathList.push_back(*wavList);
    }
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);
}

static void *play_wav_thread(void *arg)
{
    while (g_run) {
        std::list<std::string> list;
        pthread_mutex_lock(&g_mutex);
        if (pathList.size()) {
            list = pathList.front();
            pathList.pop_front();
        }
        pthread_mutex_unlock(&g_mutex);

        if (list.size()){
#if !USE_APLAY
            if (play_wav_init() == 0){
                for (auto item : list) {
                    play_wav((char*)item.c_str());
                }
                play_wav_exit();
            }
            //            sleep(list.size()>1?2:1);
#else
            char cmd[512] = {'\0'};
            snprintf(cmd, sizeof(cmd), "aplay %s", path.c_str());
            system(cmd);
            usleep(100000);
#endif
        } else {
            play_wav_wait();
        }
    }

    pthread_exit(NULL);
}

static int volume_init(char *alsa_mix_ctrl, snd_mixer_t **alsa_mix_handle_p, snd_mixer_elem_t **alsa_mix_elem_p)
{
    int alsa_mix_index = 0;
    int rc = -1;
    snd_mixer_selem_id_t *alsa_mix_sid = NULL;

    snd_mixer_selem_id_alloca(&alsa_mix_sid);
    if (alsa_mix_sid != nullptr) {
        snd_mixer_selem_id_set_index(alsa_mix_sid, alsa_mix_index);
        snd_mixer_selem_id_set_name(alsa_mix_sid, alsa_mix_ctrl);

        if ((rc = snd_mixer_open(alsa_mix_handle_p, 0)) < 0){
            fprintf(stderr, "error from mixer_open: %s\n", snd_strerror(rc));
            return rc;
        }
        if ((rc = snd_mixer_attach(*alsa_mix_handle_p, alsa_mix_dev)) < 0) {
            fprintf(stderr, "error from mixer_attach: %s\n", snd_strerror(rc));
            return rc;
        }
        if ((rc = snd_mixer_selem_register(*alsa_mix_handle_p, NULL, NULL)) < 0) {
            fprintf(stderr, "error from selem_register: %s\n", snd_strerror(rc));
            return rc;
        }
        if ((rc = snd_mixer_load(*alsa_mix_handle_p)) < 0) {
            fprintf(stderr, "error from mixer_load: %s\n", snd_strerror(rc));
            return rc;
        }
        *alsa_mix_elem_p = snd_mixer_find_selem(*alsa_mix_handle_p, alsa_mix_sid);
        if (!*alsa_mix_elem_p) {
            rc = -1;
            fprintf(stderr, "error from find_selem\n");
        }
    }

    return rc;
}

static void volume_uninit(snd_mixer_t *alsa_mix_handle)
{
    if(alsa_mix_handle){
        snd_mixer_close(alsa_mix_handle);
    }
}

int volume_get(void)
{
    long ll, lr, vol = -1;
    int rc;
    //Digital channel
    snd_mixer_t *alsa_mix_handle = NULL;
    snd_mixer_elem_t *alsa_mix_elem = NULL;
    rc = volume_init(alsa_mix_ctrl, &alsa_mix_handle, &alsa_mix_elem);
    if (rc == 0) {
        snd_mixer_handle_events(alsa_mix_handle);
        snd_mixer_selem_get_playback_volume(alsa_mix_elem, SND_MIXER_SCHN_FRONT_LEFT, &ll);
        snd_mixer_selem_get_playback_volume(alsa_mix_elem, SND_MIXER_SCHN_FRONT_RIGHT, &lr);

        vol = (int)((ll + lr) >> 1);
    }
    volume_uninit(alsa_mix_handle);

    return vol;
}

int volume_set(int vol)
{
    int rc;
    snd_mixer_t *alsa_mix_handle = NULL;
    snd_mixer_elem_t *alsa_mix_elem = NULL;

    rc = volume_init(alsa_mix_ctrl, &alsa_mix_handle, &alsa_mix_elem);
    if (rc == 0) {
        long min,max;
        if ((rc = snd_mixer_selem_get_playback_volume_range(alsa_mix_elem,&min,&max)) == 0){
            if(vol > max)
                vol = max;
            if(vol < min)
                vol = min;
        } else {
            fprintf(stderr, "error from get_playback_volume_range: %s\n", snd_strerror(rc));
        }

        snd_mixer_selem_set_playback_volume(alsa_mix_elem, SND_MIXER_SCHN_FRONT_LEFT, vol);
        snd_mixer_selem_set_playback_volume(alsa_mix_elem, SND_MIXER_SCHN_FRONT_RIGHT, vol);
    }

    volume_uninit(alsa_mix_handle);
    return 0;
}

int play_wav_thread_init(void)
{
//#if !USE_APLAY
////    system("amixer sset 'Playback Path' SPK");
////    system("i2cset -f -y 0 0x20 0x31 0x10");
////    system("i2cset -f -y 0 0x20 0x32 0x10");

//    if (play_wav_init())
//        return -1;

//#endif
    g_run = true;
    if (pthread_create(&g_tid, NULL, play_wav_thread, NULL)) {
        printf("%s create thread failed!\n", __func__);
        return -1;
    }
    return 0;
}

void play_wav_thread_exit(void)
{
    g_run = false;
    play_wav_signal(NULL);
    if (g_tid) {
        pthread_join(g_tid, NULL);
        g_tid = 0;
    }
//#if !USE_APLAY
//    play_wav_exit();
//#endif
}
