#include "ui.h"

void setup()
{
  delay(1000);
  Serial.begin(115200);
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); 
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
  readOntoBuffer();
  delay(5);
}