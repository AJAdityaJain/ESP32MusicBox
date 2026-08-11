#include "bt.h"

MP3DecoderHelix helix;
volatile bool dirty = true;
esp_bd_addr_t boseAddr = {0x28,0x11,0xA5,0x42,0x6D,0x88};
esp_bd_addr_t hdAddr = {0x00,0x1B,0x66,0xD2,0x16,0x48};
esp_bd_addr_t earAddr = {0x02,0x1B,0x6f,0xf6,0x07,0xEB};
esp_bd_addr_t* pairedAddr = nullptr;


uint8_t connectionState = 0;

BluetoothA2DPSource a2dp_source;

SemaphoreHandle_t pcmMutex = nullptr;
int16_t pcmBuf[PCM_BUF_SIZE];
volatile int pcmHead = 0;
volatile int pcmTail = 0;

void pcmCallback(MP3FrameInfo &info, int16_t *data, size_t len, void *)
{
    if (pcmMutex == nullptr)
    {
        pcmMutex = xSemaphoreCreateMutex();
    }

    if (pcmMutex != nullptr && xSemaphoreTake(pcmMutex, portMAX_DELAY) == pdTRUE)
    {
        for (size_t i = 0; i < len; i++)
        {
            int next = (pcmTail + 1) % PCM_BUF_SIZE;
            if (next != pcmHead)
                pcmBuf[pcmTail] = data[i], pcmTail = next;
        }

        xSemaphoreGive(pcmMutex);
    }

    if (info.samprate > 0 && info.bitrate > 0)
    {
        updatePlaybackProgress(info.outputSamps, info.samprate, info.bitrate);
    }
}

void onConnectionStateChange(esp_a2d_connection_state_t state, void *ptr)
{
    const char *names[] = {"DISCONNECTED", "CONNECTING", "CONNECTED", "DISCONNECTING"};
    Serial.printf("[BT] State: %s\n", names[state]);

    if (state == ESP_A2D_CONNECTION_STATE_CONNECTING)
    {
        connectionState = 1;
    }
    else if(state == ESP_A2D_CONNECTION_STATE_CONNECTED){
        connectionState = 2;
    }
    else{
        connectionState = 0;
    }

    dirty = true;
}

void onAudioStateChange(esp_a2d_audio_state_t state, void *ptr)
{
    Serial.printf("[BT] Audio : %s\n", state == ESP_A2D_AUDIO_STATE_STARTED ? "STARTED" : "STOPPED");
}

void btInit()
{
    helix.begin();
    helix.setDataCallback(pcmCallback);

    if (pcmMutex == nullptr)
    {
        pcmMutex = xSemaphoreCreateMutex();
    }
    pcmHead = 0;
    pcmTail = 0;


    a2dp_source.set_volume(80);
    a2dp_source.set_ssp_enabled(true);
    a2dp_source.set_on_connection_state_changed(onConnectionStateChange);
    a2dp_source.set_on_audio_state_changed(onAudioStateChange);
    a2dp_source.set_data_callback_in_frames(getDataFrames);
    esp_bt_dev_set_device_name(BT_DEVICE_NAME);

    updateDevice(&boseAddr);
    a2dp_source.start();
}

void setVolume(int v){
    a2dp_source.set_volume(v);
}

void updateDevice(esp_bd_addr_t* addr){
    pairedAddr = addr;
    a2dp_source.set_auto_reconnect(*pairedAddr, 5);
}
void btAttempt(esp_bd_addr_t* addr)
{
    setVolume(127);
    if(addr == nullptr)
    {
        return;
    }
    if(pairedAddr != addr){
        updateDevice(addr);
    }
    a2dp_source.connect_to(*pairedAddr);
}