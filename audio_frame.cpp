#include "audio_frame.h"

SemaphoreHandle_t pcmMutex = nullptr;
int16_t pcmBuf[PCM_BUF_SIZE];
volatile int pcmHead = 0;
volatile int pcmTail = 0;
volatile bool isPlaying = false;

void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
  isPlaying = (state == ESP_A2D_AUDIO_STATE_STARTED);
  Serial.printf("[BT] Audio : %s\n", isPlaying ? "STARTED" : "STOPPED");
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
int32_t get_data_frames(Frame *frame, int32_t frame_count)
{
  if (!isPlaying)
  {
    Serial.println("Not playing, returning silence");
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


