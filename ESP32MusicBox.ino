#include "ui.h"

#define BEEP
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

void setup()
{
  delay(1000);
  Serial.begin(115200);

  uiInit();
  if (!SDInit())
  {
    error("memory card missing");
    return;
  }

  btInit();

  xTaskCreatePinnedToCore(uiTask, "ui", 4096, NULL, 1, NULL, 0);
}

uint32_t lastTick = 0;
uint32_t now = millis();
void loop()
{
  now = millis();
  if(now - lastTick >= 1000)
  {
    clock_s++;
    if(clock_s % 60 == 0)
    {
        if(updateSleep == 1){updateSleep = 2;}
        if(updateSleep == 0)updateSleep = 1;
      dirty = true;
    }
    lastTick = now;
    updateSecond = true;
  }
  tick();
  delay(10);
}