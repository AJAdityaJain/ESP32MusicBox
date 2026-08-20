#include "bt.h"
using namespace libhelix;

MP3DecoderHelix helix;
esp_bd_addr_t boseAddr = {0x28,0x11,0xA5,0x42,0x6D,0x88};
esp_bd_addr_t hdAddr = {0x00,0x1B,0x66,0xD2,0x16,0x48};
esp_bd_addr_t earAddr = {0x02,0x1B,0x6f,0xf6,0x07,0xEB};
esp_bd_addr_t* pairedAddr = nullptr;



static BluetoothA2DPSource a2dp_source;

static SemaphoreHandle_t pcmMutex = nullptr;
static int16_t pcmBuf[PCM_BUF_SIZE];
static volatile int pcmHead = 0;
static volatile int pcmTail = 0;

static void (*playNext)();
static bool (*available)();
static void (*fileClose)();
static bool (*isPlaying)();
static int (*readFile)(uint8_t*, size_t);
static bool (*fileOpen)();

void assignNext(void (*fn)()){
    playNext = fn;
}
void assignPlay(bool (*fn)()){
    isPlaying = fn;
}
void assignFileClose(void (*fn)()){
    fileClose = fn;
}
void assignFileAvailable(bool (*fn)()){
    available = fn;
}
void assignFileRead(int (*fn)(uint8_t*, size_t)){
    readFile = fn;
}
void assignFileOpen(bool (*fn)()){
    fileOpen = fn;
}


void pcmCallback(MP3FrameInfo &info, int16_t *data, size_t len, void *)
{
    static int16_t dcOffset = 0;
    if (pcmMutex == nullptr)
    {
        pcmMutex = xSemaphoreCreateMutex();
    }

    if (pcmMutex != nullptr && xSemaphoreTake(pcmMutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
        for (size_t i = 0; i < len; i++)
        {
            int next = (pcmTail + 1) % PCM_BUF_SIZE;
            while (next == pcmHead) {
                xSemaphoreGive(pcmMutex);
                vTaskDelay(pdMS_TO_TICKS(1));
                xSemaphoreTake(pcmMutex, pdMS_TO_TICKS(5));
                next = (pcmTail + 1) % PCM_BUF_SIZE;
            }

            pcmBuf[pcmTail] = data[i];
            pcmTail = next;
        }

        xSemaphoreGive(pcmMutex);
    }

    if (info.samprate > 0 && info.bitrate > 0)
    {
        #ifdef LOG
        Serial.println(((pcmTail - pcmHead + PCM_BUF_SIZE) % PCM_BUF_SIZE) / (PCM_BUF_SIZE * 1.0f));
        #endif
        updatePlaybackProgress(info.outputSamps, info.samprate, info.bitrate);
    }
}

int32_t getDataFrames(Frame *frame, int32_t frame_count)
{
    if (!isPlaying())
    {
        #ifdef BEEP
            static float phase = 0.0f;
            const float increment = 2.0f * M_PI * 440.0f / 44100.0f;
            for (int i = 0; i < frame_count; i++)
            {
                int16_t s = (int16_t)(5000 * sinf(phase));
                frame[i].channel1 = s;
                frame[i].channel2 = s;
                phase += increment;
                if (phase > 2.0f * M_PI)
                phase -= 2.0f * M_PI;
            }
            return frame_count;
        #else
            memset(frame, 0, frame_count * sizeof(Frame));
            return frame_count;
        #endif        
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

void readOntoBuffer(){
    if (!isPlaying() || !fileOpen())return;

    if (!available())
    {
        fileClose();
        playNext();
        return;
    }

    int bufferedSamples = (pcmTail - pcmHead + PCM_BUF_SIZE) % PCM_BUF_SIZE;
    if (bufferedSamples < PCM_BUF_SIZE * 3 / 4)
    {
        uint8_t chunk[512];
        int n = readFile(chunk, sizeof(chunk));
        if (n > 0)
        {
            helix.write(chunk, n);
        }
    }
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


    a2dp_source.set_local_name(BT_DEVICE_NAME);
    esp_bt_cod_t cod;
    cod.major = ESP_BT_COD_MAJOR_DEV_AV;
    cod.minor = 0x04; // portable audio
    cod.service = ESP_BT_COD_SRVC_AUDIO | ESP_BT_COD_SRVC_RENDERING;
    esp_bt_gap_set_cod(cod, ESP_BT_INIT_COD);
    esp_bt_dev_set_device_name("iPhone"); // yes literally

    a2dp_source.set_volume(80);
    a2dp_source.set_ssp_enabled(true);
    a2dp_source.set_on_connection_state_changed(onConnectionStateChange);
    a2dp_source.set_data_callback_in_frames(getDataFrames);

    updateDevice(&hdAddr);
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