#include "esp_bt.h"
#include "BluetoothA2DPSource.h"

#include "MP3DecoderHelix.h"
#include "buzzer.h"

#define BT_DEVICE_NAME "LIRA"
#define PCM_BUF_SIZE (4096 * 2)
// #define LOG

extern esp_bd_addr_t boseAddr;
extern esp_bd_addr_t hdAddr;
extern esp_bd_addr_t earAddr;
extern esp_bd_addr_t* pairedAddr;


int32_t getDataFrames(Frame *frame, int32_t frame_count);

void assignNext(void (*fn)());
void assignPlay(bool (*fn)());
void assignFileClose(void (*fn)());
void assignFileAvailable(bool (*fn)());
void assignFileRead(int (*fn)(uint8_t*, size_t));
void assignFileOpen(bool (*fn)());

void btInit();
void readOntoBuffer();
void onConnectionStateChange(esp_a2d_connection_state_t state, void *ptr);
void btAttempt(esp_bd_addr_t* addr);
void updateDevice(esp_bd_addr_t* addr);
void setVolume(int v);
void resetPlaybackProgress();
void updatePlaybackProgress(uint32_t samplesDecoded, uint32_t sampleRate, uint32_t bitrate);
void setPlaybackDuration(uint32_t fileSize, uint32_t sampleRate, uint32_t bitrate);