#include "bt.h"
bool is_connected_or_connecting = false;
bool has_paired_addr = false;
esp_bd_addr_t paired_addr = {0};
BluetoothA2DPSource a2dp_source;

SemaphoreHandle_t pcmMutex = nullptr;
int16_t pcmBuf[PCM_BUF_SIZE];
volatile int pcmHead = 0;
volatile int pcmTail = 0;
volatile bool isPlaying = false;

void onConnectionStateChange(esp_a2d_connection_state_t state, void *ptr)
{
    const char *names[] = {"DISCONNECTED", "CONNECTING", "CONNECTED", "DISCONNECTING"};
    Serial.printf("[BT] State: %s\n", names[state]);

    if (state == ESP_A2D_CONNECTION_STATE_CONNECTED)
    {
        is_connected_or_connecting = true;
    }
    else if (state == ESP_A2D_CONNECTION_STATE_CONNECTING)
    {
        is_connected_or_connecting = true;
    }
    else
    {
        is_connected_or_connecting = false;
    }
}

void onAudioStateChange(esp_a2d_audio_state_t state, void *ptr)
{
    Serial.printf("[BT] Audio : %s\n", state == ESP_A2D_AUDIO_STATE_STARTED ? "STARTED" : "STOPPED");
}
void btInit()
{
    if (pcmMutex == nullptr)
    {
        pcmMutex = xSemaphoreCreateMutex();
    }
    pcmHead = 0;
    pcmTail = 0;

    // has_paired_addr = loadBT(paired_addr);

    a2dp_source.set_ssp_enabled(true);
    a2dp_source.set_on_connection_state_changed(onConnectionStateChange);
    a2dp_source.set_on_audio_state_changed(onAudioStateChange);
    a2dp_source.set_data_callback_in_frames(getDataFrames);
    esp_bt_dev_set_device_name(BT_DEVICE_NAME);

    a2dp_source.start();
}

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
}
int32_t getDataFrames(Frame *frame, int32_t frame_count)
{
    if (!isPlaying)
    {
        memset(frame, 0, frame_count * sizeof(Frame));
        return frame_count;
    }

    if (pcmMutex == nullptr)
    {
        pcmMutex = xSemaphoreCreateMutex();
    }

    if (pcmMutex != nullptr && xSemaphoreTake(pcmMutex, portMAX_DELAY) == pdTRUE)
    {
        for (int i = 0; i < frame_count; i++)
        {
            static int16_t lastL = 0, lastR = 0;
            if (pcmHead != pcmTail)
            {
                lastL = pcmBuf[pcmHead];
                pcmHead = (pcmHead + 1) % PCM_BUF_SIZE;
            }
            if (pcmHead != pcmTail)
            {
                lastR = pcmBuf[pcmHead];
                pcmHead = (pcmHead + 1) % PCM_BUF_SIZE;
            }
            frame[i].channel1 = lastL;
            frame[i].channel2 = lastR;
        }

        xSemaphoreGive(pcmMutex);
    }
    return frame_count;
}

void btAttempt()
{
    a2dp_source.connect_to(paired_addr);
}