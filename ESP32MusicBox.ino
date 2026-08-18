#include "ui.h"

void setup()
{
  Serial.begin(115200);
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); 
  uiInit();

  esp_reset_reason_t reason = esp_reset_reason();
  if (reason == ESP_RST_PANIC) {
      error("Turn this on and off again. #Crash");
      while(true)delay(1000);
      return;
  }


  if (!SDInit())
  {
    error("memory card missing");
      while(true)delay(1000);
    return;
  }

  btInit();

  xTaskCreatePinnedToCore(uiTask, "ui", 4096, NULL, 1, NULL, 0);
}

uint32_t lastTick = 0;
uint32_t now = millis();
int clock_s = 0;
void loop()
{
  now = millis();
  if(now - lastTick >= 1000)
  {
    lastTick = now;
    clock_s++;
    if(clock_s % 15 == 0)
    {
        if(updateSleep == 1)updateSleep = 2;
        if(updateSleep == 0)updateSleep = 1;
    }
    updateSecond = true;
  }
  readOntoBuffer();
  delay(1);
}