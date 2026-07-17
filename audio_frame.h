#pragma once
#include <math.h>
#include "SD.h"
#include "SPI.h"

#include "BluetoothA2DPSource.h"
#include "MP3DecoderHelix.h"

#define PCM_BUF_SIZE (4096 * 2)

static SemaphoreHandle_t pcmMutex;
static int16_t pcmBuf[PCM_BUF_SIZE];
static volatile int pcmHead = 0, pcmTail = 0;
static volatile bool isPlaying = false;


void audio_state_changed(esp_a2d_audio_state_t state, void *ptr);
void pcmCallback(MP3FrameInfo &info, int16_t *data, size_t len, void *);
int32_t get_data_frames(Frame *frame, int32_t frame_count);

// int32_t get_data_frames(Frame *frame, int32_t frame_count);