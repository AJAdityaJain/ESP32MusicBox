#include "sd_interface.h"
#include "freertos/semphr.h"

#define BT_DEVICE_NAME "Bose QC35 II AJ"
using namespace libhelix;

MP3DecoderHelix mp3;

File mp3File;

volatile bool isPlaying = false;

static SemaphoreHandle_t pcmMutex;
static int16_t pcmBuf[PCM_BUF_SIZE];
static volatile int pcmHead = 0, pcmTail = 0;

static bool is_connected_or_connecting = false;
static bool has_paired_addr = false;
static esp_bd_addr_t paired_addr = {0};
BluetoothA2DPSource a2dp_source;

void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
  if (state == ESP_A2D_AUDIO_STATE_STARTED)
  {
    Serial.println("[BT] Audio started");
    isPlaying = true;
  }
  else
  {
    Serial.println("Sus");
    isPlaying = false;
  }
}
void pcmCallback(MP3FrameInfo &info, int16_t *data, size_t len, void *)
{

  xSemaphoreTake(pcmMutex, portMAX_DELAY);
  for (size_t i = 0; i < len; i++)
  {
    int next = (pcmTail + 1) % PCM_BUF_SIZE;
    if (next != pcmHead)
      pcmBuf[pcmTail] = data[i], pcmTail = next;
  }

  xSemaphoreGive(pcmMutex);
}
int32_t get_data_frames(Frame *frame, int32_t frame_count)
{
  if (!isPlaying)
  {
    memset(frame, 0, frame_count * sizeof(Frame));
    return frame_count;
  }
  xSemaphoreTake(pcmMutex, portMAX_DELAY);
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
  return frame_count;
}
void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
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

void setup()
{
  Serial.begin(115200);
  delay(1000);

  if (init_sd())
  {
    has_paired_addr = load_addr(paired_addr);
  }

  mp3File = SD.open("/aud.mp3");
  if (!mp3File)
  {
    Serial.println("File not found");
    return;
  }

  pcmMutex = xSemaphoreCreateMutex();
  mp3.begin();
  mp3.setDataCallback(pcmCallback);

  a2dp_source.set_ssp_enabled(true);
  a2dp_source.set_on_connection_state_changed(connection_state_changed);
  a2dp_source.set_on_audio_state_changed(audio_state_changed); // ← add this
  a2dp_source.set_data_callback_in_frames(get_data_frames);
  a2dp_source.start();
  delay(1000);
  a2dp_source.connect_to(paired_addr);
}

void loop()
{
  if (isPlaying && mp3File)
  {
    if (mp3File.available())
    {
      int available = (pcmTail - pcmHead + PCM_BUF_SIZE) % PCM_BUF_SIZE;
      if (available < PCM_BUF_SIZE * 3 / 4)
      {
        uint8_t chunk[512];
        int n = mp3File.read(chunk, sizeof(chunk));
        if (n > 0)
          mp3.write(chunk, n);
      }
    }
    else
    {
      isPlaying = false;
      mp3File.close();
      Serial.println("Done.");
    }
  }
  // get serial input
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == 'c')
    {
      a2dp_source.connect_to(paired_addr);
    }
  }

  delay(10);
}