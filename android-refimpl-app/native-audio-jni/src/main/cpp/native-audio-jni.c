/**
 * [TRIfA], Java part of Tox Reference Implementation for Android
 * Copyright (C) 2017 Zoff <zoff@zoff.cc>
 * <p>
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * <p>
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * <p>
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* This is a JNI example where we use native methods to play sounds
 * using OpenSL ES. See the corresponding Java source file located at:
 *
 *   src/com/example/nativeaudio/NativeAudio/NativeAudio.java
 */

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <jni.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/types.h>
#include <stdbool.h>

// ------------------
// com.zoffcc.applications.trifa W/libOpenSLES: Leaving BufferQueue::Enqueue (SL_RESULT_BUFFER_INSUFFICIENT)
// ------------------

// for __android_log_print(ANDROID_LOG_INFO, "YourApp", "formatted message");
#include <android/log.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"
/*
 *
 *
 * #########################################
 *
 *
 */
static const char *LOGTAG = "trifa.nativeaudio";
// #define DEBUG_NATIVE_AUDIO_DEEP 1 // define to activate full debug logging
#define WEBRTC_AEC 1
// #define WEBRTC_DEBUGGING 1
/*
 *
 *
 * #########################################
 *
 *
 */

bool filteraudio_used = false;
#ifdef WEBRTC_AEC
// webrtc lib for software AEC and NS
#include "webrtc6/webrtc/modules/audio_processing/ns/noise_suppression_x.h"
#include "webrtc6/webrtc/modules/audio_processing/aecm/echo_control_mobile.h"

int16_t *pcm_buf_resampled;
int16_t *pcm_buf_out_resampled;
#endif

int aec_active = 0;
int audio_aec_delay = 80;

/*---------------------------------------------------------------------------*/
/* Android AudioPlayer and AudioRecorder configuration                       */
/*---------------------------------------------------------------------------*/

/** Audio Performance mode.
 * Performance mode tells the framework how to configure the audio path
 * for a player or recorder according to application performance and
 * functional requirements.
 * It affects the output or input latency based on acceptable tradeoffs on
 * battery drain and use of pre or post processing effects.
 * Performance mode should be set before realizing the object and should be
 * read after realizing the object to check if the requested mode could be
 * granted or not.
 */
/** Audio Performance mode key */
#ifdef SL_ANDROID_KEY_PERFORMANCE_MODE
#undef SL_ANDROID_KEY_PERFORMANCE_MODE
#endif
#define SL_ANDROID_KEY_PERFORMANCE_MODE ((const SLchar*) "androidPerformanceMode")

/** Audio performance values */
/*      No specific performance requirement. Allows HW and SW pre/post processing. */
#ifdef SL_ANDROID_PERFORMANCE_NONE
#undef SL_ANDROID_PERFORMANCE_NONE
#endif
#define SL_ANDROID_PERFORMANCE_NONE ((SLuint32) 0x00000000)
/*      Priority given to latency. No HW or software pre/post processing.
 *      This is the default if no performance mode is specified. */
#ifdef SL_ANDROID_PERFORMANCE_LATENCY
#undef SL_ANDROID_PERFORMANCE_LATENCY
#endif
#define SL_ANDROID_PERFORMANCE_LATENCY ((SLuint32) 0x00000001)
/*      Priority given to latency while still allowing HW pre and post processing. */
#ifdef SL_ANDROID_PERFORMANCE_LATENCY_EFFECTS
#undef SL_ANDROID_PERFORMANCE_LATENCY_EFFECTS
#endif
#define SL_ANDROID_PERFORMANCE_LATENCY_EFFECTS ((SLuint32) 0x00000002)
/*      Priority given to power saving if latency is not a concern.
 *      Allows HW and SW pre/post processing. */
#ifdef SL_ANDROID_PERFORMANCE_POWER_SAVING
#undef SL_ANDROID_PERFORMANCE_POWER_SAVING
#endif
#define SL_ANDROID_PERFORMANCE_POWER_SAVING ((SLuint32) 0x00000003)

// -----------------------------
JavaVM *cachedJVM = NULL;
__attribute__((unused)) static const int audio_play_buffers_in_queue_max = 4; // BAD: !!! always keep in sync with NavitAudio.java `public static final int n_audio_in_buffer_max_count = 3;` !!!
uint8_t *audio_play_buffer[4]; // always set to array depth of `audio_play_buffers_in_queue_max` !!!
__attribute__((unused)) static const uint8_t empty_buffer[20000];
long audio_play_buffer_size[4]; // always set to array depth of `audio_play_buffers_in_queue_max` !!!
int empty_play_buffer_size =
        1920 * 2; // to be changed later, we dont know the incoming audio setup yet
int audio_play_buffers_in_queue = 0;
int audio_play_buffers_curbuf_index = 0;
#define _STOPPED 0
#define _PLAYING 1
#define _SHUTDOWN 2
int playing_state = _STOPPED;
int player_state_current = _STOPPED;
#define PLAY_BUFFERS_BETWEEN_PLAY_AND_PROCESS1 2
pthread_mutex_t play_buffer_queued_count_mutex;
int play_buffer_queued_count_mutex_valid = 0;
uint64_t t1_prev = 0;
uint64_t t2_prev = 0;

const int samples_per_frame_for_48000_40ms = 1920;

// --------- AEC ---------
#ifdef WEBRTC_AEC
void *webrtc_aecmInst = NULL;
NsxHandle *nsxInst = NULL;
// -----------------------
#define MINIAUDIO_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Watomic-implicit-seq-cst"
#pragma GCC diagnostic ignored "-Wbad-function-cast"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wvector-conversion"
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
#pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wall"
#include "miniaudio.h"
#pragma GCC diagnostic pop
ma_resampler_config miniaudio_downsample_config;
ma_resampler miniaudio_downsample_resampler;
ma_resampler_config miniaudio_upsample_config;
ma_resampler miniaudio_upsample_resampler;
#endif
// --------- AEC ---------

int num_rec_bufs = 3; // BAD: !!! always keep in sync with NavitAudio.java `public static final int n_rec_audio_in_buffer_max_count = 3;` !!!
uint8_t *audio_rec_buffer[20]; // always set to array depth of at least `num_rec_bufs` !!!
long audio_rec_buffer_size[20]; // always set to array depth of at least `num_rec_bufs` !!!
int rec_buf_pointer_start = 0;
int rec_buf_pointer_next = 0;
#define _RECORDING 3
int rec_state = _STOPPED;
#define RECORD_BUFFERS_BETWEEN_REC_AND_PROCESS 2
float wanted_mic_gain = 1.0f;
float wanted_mic_gain_lower = 1.0f;
bool wanted_mic_gain_lower_volume = false;
bool use_with_louderspeaker = true;

jclass NativeAudio_class = NULL;
jmethodID rec_buffer_ready_method = NULL;
// -----------------------------

static float audio_in_vu_value = 0.0f;
static float audio_out_vu_value = 0.0f;

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine = NULL;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay = NULL;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;
static SLEffectSendItf bqPlayerEffectSend = NULL;
static SLMuteSoloItf bqPlayerMuteSolo = NULL;
static SLVolumeItf bqPlayerVolume = NULL;
static SLmilliHertz bqPlayerSampleRate = 0;

// recorder interfaces
static SLObjectItf recorderObject = NULL;
static SLRecordItf recorderRecord = NULL;
static SLAndroidSimpleBufferQueueItf recorderBufferQueue = NULL;


// ----- function defs ------

void
Java_com_zoffcc_applications_nativeaudio_NativeAudio_set_1JNI_1audio_1buffer(JNIEnv *env,
                                                                             jclass clazz,
                                                                             jobject buffer,
                                                                             jlong buffer_size_in_bytes,
                                                                             jint num);

jint Java_com_zoffcc_applications_nativeaudio_NativeAudio_PlayPCM16(JNIEnv *env, jclass clazz,
                                                                    jint bufnum);

jint Java_com_zoffcc_applications_nativeaudio_NativeAudio_isPlaying(JNIEnv *env, jclass clazz);

jboolean Java_com_zoffcc_applications_nativeaudio_NativeAudio_StopPCM16(JNIEnv *env, jclass clazz);

jint Java_com_zoffcc_applications_nativeaudio_NativeAudio_isRecording(JNIEnv *env, jclass clazz);

jboolean Java_com_zoffcc_applications_nativeaudio_NativeAudio_StopREC(JNIEnv *env, jclass clazz);

float audio_vu(const int16_t *pcm_data, uint32_t sample_count);

void set_aec_active(int active);

static int32_t upsample_16000_to_48000_basic(int16_t *in, int16_t *out, int32_t sample_count);

static int32_t downsample_48000_to_16000_basic(int16_t *in, int16_t *out, int32_t sample_count);

// ----- function defs ------




JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    JNIEnv *env_this;
    cachedJVM = jvm;

    if ((*jvm)->GetEnv(jvm, (void **) &env_this, JNI_VERSION_1_6))
    {
        // dbg(0,"Could not get JVM");
        return JNI_ERR;
    }

    // dbg(0,"++ Found JVM ++");
    return JNI_VERSION_1_6;
}


JNIEnv *jni_getenv(void)
{
    JNIEnv *env_this;
    (*cachedJVM)->GetEnv(cachedJVM, (void **) &env_this, JNI_VERSION_1_6);
    return env_this;
}


int android_find_class_global(char *name, jclass *ret)
{
    JNIEnv *jnienv2;
    jnienv2 = jni_getenv();
    *ret = (*jnienv2)->FindClass(jnienv2, name);

    if (!*ret)
    {
        return 0;
    }

    *ret = (*jnienv2)->NewGlobalRef(jnienv2, *ret);
    return 1;
}

// --------------------------


// gives a counter value that increaes every millisecond
#ifdef DEBUG_NATIVE_AUDIO_DEEP
static uint64_t current_time_monotonic_default(void);

static uint64_t current_time_monotonic_default()
{
    uint64_t time = 0;
    struct timespec clock_mono;
    clock_gettime(CLOCK_MONOTONIC, &clock_mono);
    time = 1000ULL * clock_mono.tv_sec + (clock_mono.tv_nsec / 1000000ULL);
    return time;
}
#endif
// --------------------------

// this callback handler is called every time a buffer finishes recording
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
#ifdef DEBUG_NATIVE_AUDIO_DEEP
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "bqRecorderCallback:A003:STATE = %d",
                        (int) rec_state);
#endif
    if (rec_state != _RECORDING)
    {
#ifdef DEBUG_NATIVE_AUDIO_DEEP
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "bqRecorderCallback:A003:RETURN");
#endif
        return;
    }

    int nextSize = 0;
    int8_t *nextBuffer = NULL;

    nextBuffer = (int8_t *) audio_rec_buffer[rec_buf_pointer_next];
    nextSize = audio_rec_buffer_size[rec_buf_pointer_next];

    if ((nextSize > 0) && (nextBuffer))
    {
        if (bq == NULL)
        {
            __android_log_print(ANDROID_LOG_INFO, LOGTAG, "bqRecorderCallback:A003:ERR:bq == NULL");
            return;
        }

#ifdef DEBUG_NATIVE_AUDIO_DEEP
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "bqRecorderCallback:A005:Enqueue -> %d, nextSize=%d",
                            rec_buf_pointer_next, (int) nextSize);
#endif

        SLAndroidSimpleBufferQueueState state;
        (*bq)->GetState(bq, &state);

        if (state.count < 1)
        {
            (*bq)->Enqueue(bq, nextBuffer,
                                    (SLuint32) nextSize);
            __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                                "bqRecorderCallback:Enqueue:more_buffers");

            (*bq)->GetState(bq, &state);

            __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                                "bqRecorderCallback:EB:real_buffer_count=%d", state.count);

            rec_buf_pointer_next++;
            if (rec_buf_pointer_next >= num_rec_bufs)
            {
                rec_buf_pointer_next = 0;
            }

            rec_buf_pointer_start++;
            if (rec_buf_pointer_start >= num_rec_bufs)
            {
                rec_buf_pointer_start = 0;
            }

            nextBuffer = (int8_t *) audio_rec_buffer[rec_buf_pointer_next];
            nextSize = audio_rec_buffer_size[rec_buf_pointer_next];

        }

        // enque the next buffer
        SLresult result = (*bq)->Enqueue(bq, nextBuffer, (SLuint32) nextSize);
        if (result != SL_RESULT_SUCCESS)
        {
            __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                                "bqRecorderCallback:A006:Enqueue:ERR:result=%d", (int) result);
        }
        (void) result;

        rec_buf_pointer_next++;
        if (rec_buf_pointer_next >= num_rec_bufs)
        {
            rec_buf_pointer_next = 0;
        }

        // __android_log_print(ANDROID_LOG_INFO, LOGTAG, "StartREC:A003:ENQU-CB:max_num_bufs=%d,bs=%d,bn=%d",
        //                    (int)num_rec_bufs, (int)rec_buf_pointer_start, (int)rec_buf_pointer_next);

        // signal Java code that a new record data is available in buffer #cur_rec_buf
        if ((NativeAudio_class) && (rec_buffer_ready_method) && (rec_state == _RECORDING))
        {
            if (filteraudio_used)
            {
#ifdef WEBRTC_AEC
                if (aec_active == 1)
                {
                    if (audio_rec_buffer_size[rec_buf_pointer_start] ==
                        (samples_per_frame_for_48000_40ms * 2))
                    {
                        const int num_samples_record =
                                audio_rec_buffer_size[rec_buf_pointer_start] / 2;

                        int16_t *pcm_buf_rec = (int16_t *) audio_rec_buffer[rec_buf_pointer_start];
                        int16_t *pcm_buf_record_resampled = (int16_t *) calloc(1,
                                                                               sizeof(int16_t) *
                                                                               (num_samples_record /
                                                                                3));
                        int16_t *pcm_buf_fltrd_resampled = (int16_t *) calloc(1,
                                                                              sizeof(int16_t) *
                                                                              (num_samples_record /
                                                                               3));
                        // downsample to 16khz
                        downsample_48000_to_16000_basic(pcm_buf_rec, pcm_buf_record_resampled,
                                                        num_samples_record);
                        const int split_factor = 4;
                        const int sample_count_split = (num_samples_record / split_factor);
                        const int sample_count_split_downsampled = sample_count_split / 3;
#ifdef WEBRTC_DEBUGGING
                        printf("WebRtcAecm_Process:samples=%d split_factor=%d sample_count_split=%d sample_count_split_downsampled=%d\n",
                               (int32_t) num_samples_record, split_factor, sample_count_split,
                               sample_count_split_downsampled);
#endif
                        for (int x = 0; x < split_factor; x++)
                        {
                            short const *const tmp1[] = {
                                    pcm_buf_record_resampled + (x * sample_count_split_downsampled),
                                    0};
                            short *const tmp2[] = {
                                    pcm_buf_fltrd_resampled + (x * sample_count_split_downsampled),
                                    0};
                            WebRtcNsx_Process(nsxInst,
                                              tmp1,
                                              1,
                                              tmp2);
                            int32_t res = WebRtcAecm_Process(
                                    webrtc_aecmInst,
                                    pcm_buf_record_resampled + (x * sample_count_split_downsampled),
                                    pcm_buf_fltrd_resampled + (x * sample_count_split_downsampled),
                                    pcm_buf_out_resampled + (x * sample_count_split_downsampled),
                                    sample_count_split_downsampled,
                                    (int16_t)audio_aec_delay
                            );
                            // suppress unused var
                            (void) res;
#ifdef WEBRTC_DEBUGGING
                            printf("WebRtcAecm_Process:res=%d\n", res);
#endif
                        }
                        // upsample back to 48khz
                        upsample_16000_to_48000_basic(pcm_buf_out_resampled,
                                                      pcm_buf_rec,
                                                      num_samples_record / 3);
                        free(pcm_buf_record_resampled);
                        free(pcm_buf_fltrd_resampled);
                    }
                }
#endif
            }

            int16_t *this_buffer_pcm16 = (int16_t *) audio_rec_buffer[rec_buf_pointer_start];
            int this_buffer_size_pcm16 = audio_rec_buffer_size[rec_buf_pointer_start] / 2;

            // TODO: make this better? faster?
            // --------------------------------------------------
            // increase GAIN manually and rather slow:
            if ((!wanted_mic_gain_lower_volume) && (wanted_mic_gain == 1.0f))
            {
                // gain == 1.0, so do nothing
            }
            else
            {
                int loop = 0;
                int32_t temp = 0;
                for (loop = 0; loop < this_buffer_size_pcm16; loop++)
                {
                    if (wanted_mic_gain_lower_volume)
                    {
                        temp = (int16_t) ((float) (*this_buffer_pcm16) * wanted_mic_gain_lower);
                    }
                    else
                    {
                        temp = (int16_t) ((int32_t)(*this_buffer_pcm16) * (int32_t)wanted_mic_gain);
                    }
                    if (temp > INT16_MAX)
                    {
                        temp = INT16_MAX;
                    }
                    else if (temp < INT16_MIN)
                    {
                        temp = INT16_MIN;
                    }

                    // __android_log_print(ANDROID_LOG_INFO, LOGTAG, "gain:old=%d new=%d",
                    //                     (*this_buffer_pcm16), (int16_t) temp);
                    *this_buffer_pcm16 = (int16_t) temp;
                    this_buffer_pcm16++;
                }
            }
            // --------------------------------------------------

            this_buffer_pcm16 = (int16_t *) audio_rec_buffer[rec_buf_pointer_start];
            audio_in_vu_value = audio_vu(this_buffer_pcm16,
                                         (uint32_t) (this_buffer_size_pcm16 / 2));

            // TODO: rewerite this, so that it does not need to call "AttachCurrentThread" and "DetachCurrentThread"
            //       every time!
            if (rec_state == _RECORDING)
            {
                JNIEnv *jnienv2;
                jnienv2 = jni_getenv();
                if (jnienv2 == NULL)
                {
                    JavaVMAttachArgs args;
                    args.version = JNI_VERSION_1_6; // choose your JNI version
                    args.name = NULL; // you might want to give the java thread a name
                    args.group = NULL; // you might want to assign the java thread to a ThreadGroup
                    if (cachedJVM)
                    {
                        (*cachedJVM)->AttachCurrentThread(cachedJVM, (JNIEnv **) &jnienv2, &args);
                    }
                }

                if (jnienv2 != NULL)
                {
                    (*jnienv2)->CallStaticVoidMethod(jnienv2, NativeAudio_class,
                                                     rec_buffer_ready_method,
                                                     rec_buf_pointer_start);
                }

                if (cachedJVM)
                {
                    (*cachedJVM)->DetachCurrentThread(cachedJVM);
                }
            }
        }

        rec_buf_pointer_start++;
        if (rec_buf_pointer_start >= num_rec_bufs)
        {
            rec_buf_pointer_start = 0;
        }

        // __android_log_print(ANDROID_LOG_INFO, LOGTAG, "StartREC:A003:ENQU-CB_PR:max_num_bufs=%d,bs=%d,bn=%d",
        //                    (int)num_rec_bufs, (int)rec_buf_pointer_start, (int)rec_buf_pointer_next);

    }
}

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
#ifdef DEBUG_NATIVE_AUDIO_DEEP
    uint64_t t2 = current_time_monotonic_default();
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "bqPlayerCallback:delta=%d", (int)(t2-t2_prev));
    t2_prev = t2;
#endif

    if (bq == NULL)
    {
        return;
    }
    if (bqPlayerBufferQueue == NULL)
    {
        return;
    }
    SLAndroidSimpleBufferQueueState state5;
    (*bq)->GetState(bq, &state5);
    if (state5.count < 1)
    {
       // (*bq)->Enqueue(bq, empty_buffer, (SLuint32) empty_play_buffer_size);
       // __android_log_print(ANDROID_LOG_INFO, LOGTAG,
       //                     "bqPlayerCallback:Enqueue:bq:%d idx=%d empty_play_buffer_size=%d",
       //                     (int)state5.count, (int)state5.index, empty_play_buffer_size);
    }
}

// create the engine and output mix objects
void Java_com_zoffcc_applications_nativeaudio_NativeAudio_createEngine(JNIEnv *env, jclass clazz,
                                                                       jint num_bufs)
{
    audio_in_vu_value = 0.0f;
    audio_out_vu_value = 0.0f;

    SLresult result;

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createEngine");

    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createEngine:pthread_mutex_init");
    pthread_mutex_init(&play_buffer_queued_count_mutex, NULL);
    play_buffer_queued_count_mutex_valid = 1;

    // find java methods ------------
    NativeAudio_class = NULL;
    android_find_class_global("com/zoffcc/applications/nativeaudio/NativeAudio",
                              &NativeAudio_class);
    rec_buffer_ready_method = (*env)->GetStaticMethodID(env, NativeAudio_class, "rec_buffer_ready",
                                                        "(I)V");

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createEngine:class=%p", NativeAudio_class);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createEngine:method=%p",
                        (void *) rec_buffer_ready_method);
    // find java methods ------------


    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // create output mix
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

}


// create buffer queue audio player
void Java_com_zoffcc_applications_nativeaudio_NativeAudio_createBufferQueueAudioPlayer(JNIEnv *env,
                                                                                       jclass clazz,
                                                                                       jint sampleRate,
                                                                                       jint channels,
                                                                                       jint num_bufs,
                                                                                       jint eac_delay_ms)
{
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:start:engineEngine");

    SLresult result;
    if (sampleRate >= 0)
    {
        bqPlayerSampleRate = sampleRate * 1000;
    }

    pthread_mutex_lock(&play_buffer_queued_count_mutex);
    audio_play_buffers_in_queue = 0;
    pthread_mutex_unlock(&play_buffer_queued_count_mutex);

    SLuint32 _speakers = SL_SPEAKER_FRONT_CENTER;

    if (channels == 2)
    {
        _speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "createBufferQueueAudioPlayer:channels=%d",
                            (int) channels);
    }

    // configure audio source
    const int queue_buffers_max = 4;
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                       (SLuint32) queue_buffers_max};
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:start:buffers for queue=%d",
                        queue_buffers_max);

    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, (SLuint32) channels, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   _speakers, SL_BYTEORDER_LITTLEENDIAN};

    /*
     * Enable Fast Audio when possible:  once we set the same rate to be the native, fast audio path
     * will be triggered
     */
    if (bqPlayerSampleRate)
    {
        format_pcm.samplesPerSec = bqPlayerSampleRate;       //sample rate in milli seconds
    }


    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:start:samplerate=%d",
                        (int) format_pcm.samplesPerSec);

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};


    /*
     * create audio player:
     *     fast audio does not support SL_IID_EFFECTSEND
     */
#define  num_params  3
    const SLInterfaceID ids[num_params] = {SL_IID_BUFFERQUEUE,
                                           SL_IID_VOLUME,
                                           SL_IID_ANDROIDCONFIGURATION,
            /*SL_IID_EFFECTSEND,*/
            /*SL_IID_MUTESOLO,*/};
    const SLboolean req[num_params] = {SL_BOOLEAN_TRUE,
                                       SL_BOOLEAN_TRUE,
                                       SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/
            /*SL_BOOLEAN_TRUE,*/};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                num_params, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // ----------------------------------------------------------
    SLAndroidConfigurationItf playerConfig;
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_ANDROIDCONFIGURATION,
                                             &playerConfig);

    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:res_001=%d SL_RESULT_SUCCESS=%d",
                        (int) result, (int) SL_RESULT_SUCCESS);

    if (SL_RESULT_SUCCESS == result)
    {
        // SLint32 streamType = SL_ANDROID_STREAM_MEDIA;
        SLint32 streamType = SL_ANDROID_STREAM_VOICE;
        result = (*playerConfig)->SetConfiguration(playerConfig, SL_ANDROID_KEY_STREAM_TYPE,
                                                   &streamType,
                                                   sizeof(SLint32));
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createBufferQueueAudioPlayer:res_002=%d",
                            (int) result);


        SLuint32 presetValue2 = SL_ANDROID_PERFORMANCE_LATENCY;
        (*playerConfig)->SetConfiguration(playerConfig,
                                          SL_ANDROID_KEY_PERFORMANCE_MODE,
                                          &presetValue2,
                                          sizeof(SLuint32));

        // -- read what values where actually set --
        SLuint32 presetRetrieved = SL_ANDROID_RECORDING_PRESET_NONE;
        SLuint32 presetSize = 2 * sizeof(SLuint32); // intentionally too big
        (*playerConfig)->GetConfiguration(playerConfig,
                                          SL_ANDROID_KEY_PERFORMANCE_MODE,
                                          &presetSize, (void *) &presetRetrieved);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "createBufferQueueAudioPlayer:performance_mode=%u",
                            presetRetrieved);

        // -- read what values where actually set --

    }
    // ----------------------------------------------------------

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:res_Realize=%d SL_RESULT_SUCCESS=%d",
                        (int) result, (int) SL_RESULT_SUCCESS);
    (void) result;

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:res_003=%d SL_RESULT_SUCCESS=%d",
                        (int) result, (int) SL_RESULT_SUCCESS);
    (void) result;


    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:res_005=%d SL_RESULT_SUCCESS=%d",
                        (int) result, (int) SL_RESULT_SUCCESS);
    (void) result;

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:res_005a=%d SL_RESULT_SUCCESS=%d",
                        (int) result, (int) SL_RESULT_SUCCESS);
    (void) result;

    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:res_006=%d SL_RESULT_SUCCESS=%d",
                        (int) result, (int) SL_RESULT_SUCCESS);
    (void) result;

    SLmillibel curVolume;
    (*bqPlayerVolume)->GetVolumeLevel(bqPlayerVolume, &curVolume);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:GetVolumeLevel=%d",
                        (int) curVolume);

    SLmillibel maxVolume;
    (*bqPlayerVolume)->GetMaxVolumeLevel(bqPlayerVolume, &maxVolume);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:GetMaxVolumeLevel=%d",
                        (int) maxVolume);

    // set max volume
    (*bqPlayerVolume)->SetVolumeLevel(bqPlayerVolume, maxVolume);

    // set the player's state
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:res_007=%d SL_RESULT_SUCCESS=%d PAUSED",
                        (int) result, (int) SL_RESULT_SUCCESS);
    (void) result;

    (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, empty_buffer,
                                    (SLuint32) empty_play_buffer_size);
    (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, empty_buffer,
                                    (SLuint32) empty_play_buffer_size);

    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createBufferQueueAudioPlayer:Enqueue:emtpy buffers");

    if ((channels == 1) && (sampleRate == 48000))
    {
#ifdef WEBRTC_AEC

        // ----------------------------------------------------------
        // Audio Resampling
        //
        miniaudio_downsample_config = ma_resampler_config_init(
            ma_format_s16,
            1,
            48000,
            16000,
            ma_resample_algorithm_linear);

        ma_result result1 = ma_resampler_init(&miniaudio_downsample_config, NULL, &miniaudio_downsample_resampler);
        if (result1 != MA_SUCCESS) {
            __android_log_print(ANDROID_LOG_INFO, LOGTAG, "ma_resampler_init downsample -----> ERROR");
        }
        ma_resampler_set_rate(&miniaudio_downsample_resampler, 48000, 16000);

        miniaudio_upsample_config = ma_resampler_config_init(
            ma_format_s16,
            1,
            16000,
            48000,
            ma_resample_algorithm_linear);

        ma_result result2 = ma_resampler_init(&miniaudio_upsample_config, NULL, &miniaudio_upsample_resampler);
        if (result2 != MA_SUCCESS) {
            __android_log_print(ANDROID_LOG_INFO, LOGTAG, "ma_resampler_init upsample -----> ERROR");
        }
        ma_resampler_set_rate(&miniaudio_upsample_resampler, 16000, 48000);
        //
        // ----------------------------------------------------------

        pcm_buf_resampled = calloc(1, sizeof(int16_t) * samples_per_frame_for_48000_40ms);
        pcm_buf_out_resampled = calloc(1, sizeof(int16_t) * samples_per_frame_for_48000_40ms);

        // ----------------------------------------------------------
        // Noise Suppression
        //
        nsxInst = WebRtcNsx_Create();
        int res1 = WebRtcNsx_Init(nsxInst, sampleRate); // only at 16kHz MONO
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "WebRtcNsx_Init:res=%d",
                            res1);
        // ----------------------------------------------------------
        // mode          : 0: Mild, 1: Medium , 2: Aggressive
        // ----------------------------------------------------------
        int res3 = WebRtcNsx_set_policy(nsxInst, 2);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "WebRtcNsx_set_policy:res=%d",
                            res3);
        //
        // ----------------------------------------------------------


        // ----------------------------------------------------------
        // Acoustic Echo Cancellation
        //
        webrtc_aecmInst = WebRtcAecm_Create();
        int32_t res2 = WebRtcAecm_Init(webrtc_aecmInst, sampleRate / 3); // only at 16kHz MONO
        // ----------------------------------------------------------
        // typedef struct {
        //     int16_t cngMode;            // AecmFalse, AecmTrue (default)
        //     int16_t echoMode;           // 0, 1, 2, 3 (default), 4
        // } AecmConfig;
        // ----------------------------------------------------------
        AecmConfig config;
        config.echoMode = AecmTrue;
        config.cngMode = 2;
        WebRtcAecm_set_config(webrtc_aecmInst, config);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "WebRtcAecm_Init:res=%d sampleRate=%d",
                            res2, sampleRate);
        //
        // ----------------------------------------------------------
#endif
        filteraudio_used = true;
    }
    else
    {
        filteraudio_used = false;
    }

    player_state_current = _STOPPED;
    playing_state = _STOPPED;
}


// create audio recorder: recorder is not in fast path
void
Java_com_zoffcc_applications_nativeaudio_NativeAudio_createAudioRecorder(JNIEnv *env, jclass clazz,
                                                                         jint sampleRate,
                                                                         jint num_bufs)
{
    SLresult result;

    SLuint32 channels = 1; // always record mono
    num_rec_bufs = num_bufs;

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:start");

    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};

    SLDataSource audioSrc = {&loc_dev, NULL};

    SLuint32 rec_samplerate = SL_SAMPLINGRATE_16;
    if ((int) sampleRate == 48000)
    {
        rec_samplerate = SL_SAMPLINGRATE_48;
    }
    else if ((int) sampleRate == 8000)
    {
        rec_samplerate = SL_SAMPLINGRATE_8;
    }
    else if ((int) sampleRate == 32000)
    {
        rec_samplerate = SL_SAMPLINGRATE_32;
    }
    else if ((int) sampleRate == 16000)
    {
        rec_samplerate = SL_SAMPLINGRATE_16;
    }

    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "createAudioRecorder:start:samplerate=%d", (int) rec_samplerate);


    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                     (SLuint32) num_rec_bufs};

    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, (SLuint32) channels,
                                   (SLuint32) rec_samplerate,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};

    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[2] = {
            SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
            SL_IID_ANDROIDCONFIGURATION
    };
    const SLboolean req[2] = {
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE
    };
    result = (*engineEngine)->CreateAudioRecorder(engineEngine, &recorderObject, &audioSrc,
                                                  &audioSnk, (2), id, req);

    if (SL_RESULT_SUCCESS != result)
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:ERR:01");
        return;
    }

    // Configure the voice recognition preset which has no
    // signal processing for lower latency.
    SLAndroidConfigurationItf inputConfig;
    result = (*recorderObject)->GetInterface(recorderObject,
                                             SL_IID_ANDROIDCONFIGURATION,
                                             &inputConfig);

    if (SL_RESULT_SUCCESS == result)
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "createAudioRecorder:SL_IID_ANDROIDCONFIGURATION...");

        SLuint32 presetValue = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
        if (use_with_louderspeaker)
        {
            presetValue = SL_ANDROID_RECORDING_PRESET_VOICE_COMMUNICATION;
        }
        // SL_ANDROID_RECORDING_PRESET_UNPROCESSED <--- ??
        // SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION <-- low latency
        // SL_ANDROID_RECORDING_PRESET_VOICE_COMMUNICATION <-- high latency
        (*inputConfig)->SetConfiguration(inputConfig,
                                         SL_ANDROID_KEY_RECORDING_PRESET,
                                         &presetValue,
                                         sizeof(SLuint32));

        SLuint32 presetValue2 = SL_ANDROID_PERFORMANCE_LATENCY;
        // SL_ANDROID_PERFORMANCE_NONE
        // SL_ANDROID_PERFORMANCE_LATENCY <--- ??
        // SL_ANDROID_PERFORMANCE_LATENCY_EFFECTS
        (*inputConfig)->SetConfiguration(inputConfig,
                                         SL_ANDROID_KEY_PERFORMANCE_MODE,
                                         &presetValue2,
                                         sizeof(SLuint32));

        // -- read what values where actually set --
        SLuint32 presetRetrieved = SL_ANDROID_RECORDING_PRESET_NONE;
        SLuint32 presetSize = 2 * sizeof(SLuint32); // intentionally too big
        (*inputConfig)->GetConfiguration(inputConfig,
                                         SL_ANDROID_KEY_RECORDING_PRESET,
                                         &presetSize, (void *) &presetRetrieved);

        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "createAudioRecorder:SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION=3");
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "createAudioRecorder:SL_ANDROID_RECORDING_PRESET_VOICE_COMMUNICATION=4");
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "createAudioRecorder:SL_ANDROID_RECORDING_PRESET_UNPROCESSED=5");

        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:record_preset=%u",
                            presetRetrieved);

        presetRetrieved = SL_ANDROID_PERFORMANCE_NONE;
        presetSize = 2 * sizeof(SLuint32); // intentionally too big
        (*inputConfig)->GetConfiguration(inputConfig,
                                         SL_ANDROID_KEY_PERFORMANCE_MODE,
                                         &presetSize, (void *) &presetRetrieved);

        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "createAudioRecorder:SL_ANDROID_PERFORMANCE_NONE=0");
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "createAudioRecorder:SL_ANDROID_PERFORMANCE_LATENCY=1");
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "createAudioRecorder:SL_ANDROID_PERFORMANCE_LATENCY_EFFECTS=2");

        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:performance_mode=%u",
                            presetRetrieved);

        // -- read what values where actually set --

    }

    // realize the audio recorder
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result)
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:ERR:02");
        return;
    }

    // get the record interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorderRecord);
    assert(SL_RESULT_SUCCESS == result);

#if 0
    /* Enable AEC if requested */
    SLAndroidAcousticEchoCancellationItf aecItf;
    result = (*recorderObject)->GetInterface(
        recorderObject, SL_IID_ANDROIDACOUSTICECHOCANCELLATION, (void *) &aecItf);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:AEC is %savailable",
                        SL_RESULT_SUCCESS == result ? "" : "not ");
    if (SL_RESULT_SUCCESS == result)
    {
        result = (*aecItf)->SetEnabled(aecItf, true);
        SLboolean enabled;
        result = (*aecItf)->IsEnabled(aecItf, &enabled);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:AEC is %s",
                            enabled ? "enabled" : "not enabled");
    }
#endif

#if 0
    /* Enable AGC if requested */
    if (agc)
    {
        SLAndroidAutomaticGainControlItf agcItf;
        result = (*recorderObject)->GetInterface(
            recorderObject, SL_IID_ANDROIDAUTOMATICGAINCONTROL, (void *) &agcItf);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:AGC is %savailable",
                            SL_RESULT_SUCCESS == result ? "" : "not ");
        if (SL_RESULT_SUCCESS == result)
        {
            result = (*agcItf)->SetEnabled(agcItf, true);
            SLboolean enabled;
            result = (*agcItf)->IsEnabled(agcItf, &enabled);
            __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:AGC is %s",
                                enabled ? "enabled" : "not enabled");
        }
    }
    /* Enable NS if requested */
    if (ns)
    {
        SLAndroidNoiseSuppressionItf nsItf;
        result = (*recorderObject)->GetInterface(
            recorderObject, SL_IID_ANDROIDNOISESUPPRESSION, (void *) &nsItf);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:NS is %savailable",
                            SL_RESULT_SUCCESS == result ? "" : "not ");
        if (SL_RESULT_SUCCESS == result)
        {
            result = (*nsItf)->SetEnabled(nsItf, true);
            SLboolean enabled;
            result = (*nsItf)->IsEnabled(nsItf, &enabled);
            __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:NS is %s",
                                enabled ? "enabled" : "not enabled");
        }
    }
#endif

    // get the buffer queue interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                             &recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, bqRecorderCallback,
                                                      NULL);
    assert(SL_RESULT_SUCCESS == result);

    rec_buf_pointer_start = 0;
    rec_buf_pointer_next = 0;
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "rec_state:SET:001:_STOPPED");
    rec_state = _STOPPED;

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "createAudioRecorder:end");
}


void Java_com_zoffcc_applications_nativeaudio_NativeAudio_set_1JNI_1audio_1buffer(JNIEnv *env,
                                                                                  jclass clazz,
                                                                                  jobject buffer,
                                                                                  jlong buffer_size_in_bytes,
                                                                                  jint num)
{
    JNIEnv *jnienv2;
    jnienv2 = jni_getenv();

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "set_JNI_audio_buffer:num=%d, len=%d",
                        (int) num,
                        (int) buffer_size_in_bytes);

    audio_play_buffer[num] = (uint8_t *) (*jnienv2)->GetDirectBufferAddress(jnienv2, buffer);
    jlong capacity = buffer_size_in_bytes; // (*jnienv2)->GetDirectBufferCapacity(jnienv2, buffer);
    audio_play_buffer_size[num] = (long) capacity;
}

jint Java_com_zoffcc_applications_nativeaudio_NativeAudio_PlayPCM16(JNIEnv *env, jclass clazz, jint bufnum)
{
    if (playing_state == _SHUTDOWN)
    {
        audio_out_vu_value = 0.0f;
        return -1;
    }

    if (bqPlayerPlay == NULL)
    {
        return -2;
    }

    if (bqPlayerBufferQueue == NULL)
    {
        audio_out_vu_value = 0.0f;
        return -2;
    }

    int8_t *nextBuffer = (int8_t *) audio_play_buffer[bufnum];
    int nextSize = audio_play_buffer_size[bufnum];
    empty_play_buffer_size = nextSize;

#ifdef DEBUG_NATIVE_AUDIO_DEEP
    uint64_t t1 = current_time_monotonic_default();
    __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                        "PlayPCM16:delta=%d bufsize=%d", (int)(t1-t1_prev), nextSize);
    t1_prev = t1;
#endif

    if (nextSize > 0)
    {
        if (player_state_current == _PLAYING)
        {
            audio_out_vu_value = audio_vu((int16_t *) nextBuffer, (uint32_t) (nextSize / 2));
        }
        playing_state = _PLAYING;

        if (player_state_current != _PLAYING)
        {
            (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer,
                                            (SLuint32) nextSize);
            (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer,
                                            (SLuint32) nextSize);
            __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                                "PlayPCM16:SetPlayState:A:SL_PLAYSTATE_PLAYING:Enqueue:2 frames");
            (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
            player_state_current = _PLAYING;

            if (filteraudio_used)
            {
                //pass_audio_output(filteraudio, (const int16_t *) nextBuffer,
                //                  (unsigned int) (nextSize / 2));
#ifdef WEBRTC_AEC
                if (aec_active == 1)
                {
                    if (nextSize == (samples_per_frame_for_48000_40ms * 2))
                    {
                        const int num_samples_play = nextSize / 2;
                        int16_t *pcm_buf_play = (int16_t *) nextBuffer;
                        int16_t *pcm_buf_play_resampled = (int16_t *) calloc(1,
                                                                             sizeof(int16_t) *
                                                                             (num_samples_play /
                                                                              3));
                        // downsample to 16khz
                        downsample_48000_to_16000_basic(pcm_buf_play, pcm_buf_play_resampled,
                                                        num_samples_play);

                        const int split_factor = 4;
                        const int sample_count_split = (num_samples_play / split_factor);
                        const int sample_count_split_downsampled = sample_count_split / 3;
#ifdef WEBRTC_DEBUGGING
                        printf("WebRtcAecm_BufferFarend:samples=%d split_factor=%d sample_count_split=%d sample_count_split_downsampled=%d\n",
                           (int32_t) num_samples_play, split_factor, sample_count_split,
                           sample_count_split_downsampled);
#endif
                        for (int x = 0; x < split_factor; x++)
                        {
                            int32_t res = WebRtcAecm_BufferFarend(
                                    webrtc_aecmInst,
                                    (int16_t *) pcm_buf_play_resampled +
                                    (x * sample_count_split_downsampled),
                                    sample_count_split_downsampled);
                            // suppress unused var
                            (void) res;
#ifdef WEBRTC_DEBUGGING
                            printf("WebRtcAecm_BufferFarend:res=%d\n", res);
#endif
                        }
                        free(pcm_buf_play_resampled);
                    }
                }
#endif
            }
        }
        else
        {
            (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer,
                                            (SLuint32) nextSize);
            SLuint32 pState;
            (*bqPlayerPlay)->GetPlayState(bqPlayerPlay, &pState);
            if (pState != SL_PLAYSTATE_PLAYING)
            {
                (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
                player_state_current = _PLAYING;
                __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                                    "PlayPCM16:SetPlayState:B:SL_PLAYSTATE_PLAYING");
            }
        }
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,"PlayPCM16:nextSize==0");
    }
    return 0;
}

void
Java_com_zoffcc_applications_nativeaudio_NativeAudio_set_1JNI_1audio_1rec_1buffer(JNIEnv *env,
                                                                                  jclass clazz,
                                                                                  jobject buffer,
                                                                                  jlong buffer_size_in_bytes,
                                                                                  jint num)
{
    JNIEnv *jnienv2;
    jnienv2 = jni_getenv();

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "set_JNI_audio_rec_buffer:num=%d, len=%d",
                        (int) num,
                        (int) buffer_size_in_bytes);

    audio_rec_buffer[num] = (uint8_t *) (*jnienv2)->GetDirectBufferAddress(jnienv2, buffer);
    jlong capacity = buffer_size_in_bytes; // (*jnienv2)->GetDirectBufferCapacity(jnienv2, buffer);
    audio_rec_buffer_size[num] = (long) capacity;
}

void
Java_com_zoffcc_applications_nativeaudio_NativeAudio_setMicGainToggle(JNIEnv *env, jclass clazz,
                                                                      jboolean lower_volume)
{
    wanted_mic_gain_lower_volume = (bool)lower_volume;
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "setMicGainToggle:%d (true==%d)",
                        (int) wanted_mic_gain_lower_volume, (int)true);
}

void
Java_com_zoffcc_applications_nativeaudio_NativeAudio_setMicGainFactor(JNIEnv *env, jclass clazz,
                                                                      jfloat gain_factor)
{
    if (((float) gain_factor >= 1.0f) && ((float) gain_factor <= 15.0f))
    {
        wanted_mic_gain = (float) gain_factor;
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "setMicGainFactor:%f",
                            (double) wanted_mic_gain);

        if (gain_factor == 1.0f)
        {
            wanted_mic_gain_lower = 0.9f;
        }
        else if (gain_factor == 2.0f)
        {
            wanted_mic_gain_lower = 0.8f;
        }
        else if (gain_factor == 3.0f)
        {
            wanted_mic_gain_lower = 0.6f;
        }
        else if (gain_factor == 4.0f)
        {
            wanted_mic_gain_lower = 0.5f;
        }
        else if (gain_factor == 5.0f)
        {
            wanted_mic_gain_lower = 0.4f;
        }
        else
        {
            wanted_mic_gain_lower = 0.35f;
        }

        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "setMicGainFactor:lower:%f",
                            (double) wanted_mic_gain_lower);
    }
}

jint Java_com_zoffcc_applications_nativeaudio_NativeAudio_isRecording(JNIEnv *env, jclass clazz)
{
    if (rec_state == _RECORDING)
    {
        return (jint) 1;
    }
    else
    {
        return (jint) 0;
    }
}


jboolean Java_com_zoffcc_applications_nativeaudio_NativeAudio_StopREC(JNIEnv *env, jclass clazz)
{
    rec_buf_pointer_start = 0;
    rec_buf_pointer_next = 0;

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "rec_state:SET:002:_STOPPED");
    rec_state = _STOPPED;

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "StopREC, state=%d", (int) rec_state);

#if 0
    if (recorderRecord != NULL)
    {
        SLuint32 curState;
        result = (*recorderRecord)->GetRecordState(recorderRecord, &curState);
        if (curState == SL_RECORDSTATE_STOPPED)
        {
            return JNI_TRUE;
        }
    }
#endif

    // stop recording and clear buffer queue
    if (recorderRecord != NULL)
    {
        (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
    }

    if (recorderBufferQueue != NULL)
    {
        (*recorderBufferQueue)->Clear(recorderBufferQueue);
    }

    // also reset buffer pointers
    rec_buf_pointer_start = 0;
    rec_buf_pointer_next = 0;

    return JNI_TRUE;
}


jint Java_com_zoffcc_applications_nativeaudio_NativeAudio_StartREC(JNIEnv *env, jclass clazz)
{
    if (rec_state == _SHUTDOWN)
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "StartREC:_SHUTDOWN --> RET");
        return -1;
    }

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "StartREC");

    rec_buf_pointer_start = 0;
    rec_buf_pointer_next = 0;
    int nextSize = 0;
    short *nextBuffer = NULL;
    nextBuffer = (short *) audio_rec_buffer[rec_buf_pointer_next];
    nextSize = audio_rec_buffer_size[rec_buf_pointer_next];

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "StartREC:A001:max_num_bufs=%d,bs=%d,bn=%d",
                        (int) num_rec_bufs, (int) rec_buf_pointer_start,
                        (int) rec_buf_pointer_next);

    if (nextSize > 0)
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "StartREC:A002x");

        if (recorderBufferQueue == NULL)
        {
            __android_log_print(ANDROID_LOG_INFO, LOGTAG, "StartREC:ERR:01");
            return -2;
        }

        // in case already recording, stop recording and clear buffer queue
        SLresult result;
        result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
        result = (*recorderBufferQueue)->Clear(recorderBufferQueue);

        // enqueue buffers ----------------
        SLAndroidSimpleBufferQueueState state;
        result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, nextBuffer,
                                                 (SLuint32) nextSize);

        (*recorderBufferQueue)->GetState(recorderBufferQueue, &state);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "bqRecorderCallback:Enqueue:more_buffers:%d", state.count);

        rec_buf_pointer_next++;
        if (rec_buf_pointer_next >= num_rec_bufs)
        {
            rec_buf_pointer_next = 0;
        }

        rec_buf_pointer_start++;
        if (rec_buf_pointer_start >= num_rec_bufs)
        {
            rec_buf_pointer_start = 0;
        }
        // enqueue buffers ----------------

        // enqueue buffers ----------------
        result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, nextBuffer,
                                                 (SLuint32) nextSize);
        (*recorderBufferQueue)->GetState(recorderBufferQueue, &state);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "bqRecorderCallback:Enqueue:more_buffers:%d", state.count);

        rec_buf_pointer_next++;
        if (rec_buf_pointer_next >= num_rec_bufs)
        {
            rec_buf_pointer_next = 0;
        }

        rec_buf_pointer_start++;
        if (rec_buf_pointer_start >= num_rec_bufs)
        {
            rec_buf_pointer_start = 0;
        }
        // enqueue buffers ----------------

        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "StartREC:A003:ENQU-01:max_num_bufs=%d,bs=%d,bn=%d",
                            (int) num_rec_bufs, (int) rec_buf_pointer_start,
                            (int) rec_buf_pointer_next);


        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "rec_state:SET:002:_RECORDING");
        rec_state = _RECORDING;

        // start recording
        result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);

        if (SL_RESULT_SUCCESS != result)
        {
            __android_log_print(ANDROID_LOG_INFO, LOGTAG, "StartREC:ERR:03");
            return -2;
        }

        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "rec_state:088");
    }

    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "rec_state:099");

    return 0;
}


jint Java_com_zoffcc_applications_nativeaudio_NativeAudio_isPlaying(JNIEnv *env, jclass clazz)
{
    if (playing_state == _PLAYING)
    {
        return (jint) 1;
    }
    else
    {
        return (jint) 0;
    }
}

jboolean Java_com_zoffcc_applications_nativeaudio_NativeAudio_StopPCM16(JNIEnv *env, jclass clazz)
{
    playing_state = _STOPPED;

    audio_out_vu_value = 0.0f;

    // set the player's state
    SLresult result;
    if (bqPlayerPlay != NULL)
    {
        result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "player_state:res_007=%d SL_RESULT_SUCCESS=%d STOPPED",
                            (int) result, (int) SL_RESULT_SUCCESS);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "player_state:res_008:bqPlayerPlay == NULL");
    }
    (void) result;

    player_state_current = _STOPPED;

    if (bqPlayerBufferQueue != NULL)
    {
        (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
    }

    pthread_mutex_lock(&play_buffer_queued_count_mutex);
    audio_play_buffers_in_queue = 0;
    audio_play_buffers_curbuf_index = 0;
    pthread_mutex_unlock(&play_buffer_queued_count_mutex);

    return JNI_TRUE;
}


// shut down the native audio system
void Java_com_zoffcc_applications_nativeaudio_NativeAudio_shutdownEngine(JNIEnv *env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "shutdownEngine");

    if (filteraudio_used)
    {
        // filteraudio
        filteraudio_used = false;
        // webrtc
#ifdef WEBRTC_AEC
        WebRtcAecm_Free(webrtc_aecmInst);
        webrtc_aecmInst = NULL;
        WebRtcNsx_Free(nsxInst);
        nsxInst = NULL;
        free(pcm_buf_resampled);
        pcm_buf_resampled = NULL;
        free(pcm_buf_out_resampled);
        pcm_buf_out_resampled = NULL;
        ma_resampler_uninit(&miniaudio_downsample_resampler, NULL);
        ma_resampler_uninit(&miniaudio_upsample_resampler, NULL);
#endif
    }

    playing_state = _SHUTDOWN;
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "rec_state:SET:002:_SHUTDOWN");
    rec_state = _SHUTDOWN;

    // set the player's state
    if (bqPlayerPlay != NULL)
    {
        SLresult result;
        result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        (void) result;
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "player_state:res_009=%d SL_RESULT_SUCCESS=%d STOPPED",
                            (int) result, (int) SL_RESULT_SUCCESS);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "player_state:bqPlayerPlay==NULL");
    }

    player_state_current = _STOPPED;

    if (bqPlayerBufferQueue != NULL)
    {
        SLAndroidSimpleBufferQueueState state;
        (*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &state);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "player_state:real_buffer_count(before clear)=%d",
                            state.count);

        (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
    }

    if (play_buffer_queued_count_mutex_valid == 1)
    {
        pthread_mutex_lock(&play_buffer_queued_count_mutex);
    }
    audio_play_buffers_in_queue = 0;
    audio_play_buffers_curbuf_index = 0;
    if (play_buffer_queued_count_mutex_valid == 1)
    {
        pthread_mutex_unlock(&play_buffer_queued_count_mutex);
    }

    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL)
    {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerEffectSend = NULL;
        bqPlayerMuteSolo = NULL;
        bqPlayerVolume = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL)
    {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        // outputMixEnvironmentalReverb = NULL;
    }

    // destroy audio recorder object, and invalidate all associated interfaces
    if (recorderObject != NULL)
    {
        (*recorderObject)->Destroy(recorderObject);
        recorderObject = NULL;
        recorderRecord = NULL;
        recorderBufferQueue = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL)
    {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    if (play_buffer_queued_count_mutex_valid == 1)
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "shutdownEngine:pthread_mutex_destroy");
        pthread_mutex_destroy(&play_buffer_queued_count_mutex);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG,
                            "shutdownEngine:mutex already destroyed");
    }
    play_buffer_queued_count_mutex_valid = 0;
}

float audio_vu(const int16_t *pcm_data, uint32_t sample_count)
{
    float sum = 0.0f;

    for (uint32_t i = 0; i < sample_count; i++)
    {
        sum += abs(pcm_data[i]) / 32767.0;
    }

    float vu = 20.0f * logf(sum);
    return vu;
}

void
Java_com_zoffcc_applications_nativeaudio_NativeAudio_set_1rec_1preset(JNIEnv *env, jclass clazz,
                                                                      jboolean with_loud_speaker)
{
    use_with_louderspeaker = with_loud_speaker;
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "set_rec_preset:use_with_louderspeaker=%d", use_with_louderspeaker);
}

void
Java_com_zoffcc_applications_nativeaudio_NativeAudio_set_1aec_1active(JNIEnv *env, jclass clazz,
                                                                      jint active)
{
    if (filteraudio_used == 1)
    {
        aec_active = (int) active;
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "set_aec_active:aec_active=%d", aec_active);
    }
}

jint
Java_com_zoffcc_applications_nativeaudio_NativeAudio_get_1aec_1active(JNIEnv *env, jclass clazz)
{
    return aec_active;
}

void
Java_com_zoffcc_applications_nativeaudio_NativeAudio_set_1audio_1aec_1delay(JNIEnv *env,
                                                                            jclass clazz,
                                                                            jint delay)
{
    audio_aec_delay = (int) delay;
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "set_audio_aec_delay:audio_aec_delay=%d",
                        audio_aec_delay);
}

jint
Java_com_zoffcc_applications_nativeaudio_NativeAudio_get_1audio_1aec_1delay(JNIEnv *env,
                                                                            jclass clazz)
{
    return audio_aec_delay;
}

jfloat Java_com_zoffcc_applications_nativeaudio_NativeAudio_get_1vu_1in(JNIEnv *env, jclass clazz)
{
    return (jfloat) audio_in_vu_value;
}

jfloat Java_com_zoffcc_applications_nativeaudio_NativeAudio_get_1vu_1out(JNIEnv *env, jclass clazz)
{
    return (jfloat) audio_out_vu_value;
}

static int32_t upsample_16000_to_48000_basic(int16_t *in, int16_t *out, int32_t sample_count)
{
    if (sample_count < 1)
    {
        return -1;
    }

    ma_uint64 frameCountIn  = sample_count;
    ma_uint64 frameCountOut = sample_count * 3;
    ma_result result = ma_resampler_process_pcm_frames(&miniaudio_upsample_resampler, in, &frameCountIn, out, &frameCountOut);
    if (result != MA_SUCCESS) {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "upsample_16000_to_48000_basic::error!!");
    }

#if 0
    int16_t tmp1;
    int16_t tmp2;
    int16_t v1;
    int16_t v2;
    for (int i = 0; i < sample_count; i++)
    {
        tmp1 = *(in);
        tmp2 = *(in + 1);
        v1 = (int16_t) ((float) (tmp2 - tmp1) / 3.0f);
        v2 = (int16_t) (((float) (tmp2 - tmp1) * 2.0f) / 3.0f);
        *out = tmp1;
        out++;
        *out = tmp1 + v1;
        out++;
        *out = tmp1 + v2;
        out++;
        in++;
    }
#endif

    return 0;
}

static int32_t downsample_48000_to_16000_basic(int16_t *in, int16_t *out, int32_t sample_count)
{
    if (sample_count < 3)
    {
        return -1;
    }

    ma_uint64 frameCountIn  = sample_count;
    ma_uint64 frameCountOut = sample_count / 3;
    ma_result result = ma_resampler_process_pcm_frames(&miniaudio_downsample_resampler, in, &frameCountIn, out, &frameCountOut);
    if (result != MA_SUCCESS) {
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "downsample_48000_to_16000_basic::error!!");
    }

#if 0
    for (int i = 0; i < (sample_count / 3); i++)
    {
        *out = *(in + 0);
        out++;
        in = in + 3;
    }
#endif

    return 0;
}

#pragma clang diagnostic pop
