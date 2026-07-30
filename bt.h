#include "esp_bt.h"
#include <math.h>
#include "SPI.h"
#include "BluetoothA2DPSource.h"

#include "MP3DecoderHelix.h"

#define BT_DEVICE_NAME "LIRA"
#define PCM_BUF_SIZE (4096 * 2)

using namespace libhelix;

extern MP3DecoderHelix helix;
extern uint8_t connectionState;
extern volatile bool dirty;

extern SemaphoreHandle_t pcmMutex;
extern int16_t pcmBuf[PCM_BUF_SIZE];
extern volatile int pcmHead;
extern volatile int pcmTail;

int32_t getDataFrames(Frame *frame, int32_t frame_count);
void btInit();
void pcmCallback(MP3FrameInfo &info, int16_t *data, size_t len, void *);
void btAttempt(int i);