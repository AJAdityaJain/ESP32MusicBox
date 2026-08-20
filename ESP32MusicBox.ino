#include "ui.h"

#define BUZZER_PIN 15
#define LEDC_CHANNEL 0
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_RESOLUTION 8

void buzzerTone(int freq, int duration) {
    ledcAttach(BUZZER_PIN, freq, LEDC_RESOLUTION);
    ledcWrite(BUZZER_PIN, 128);
    delay(duration);
    ledcWrite(BUZZER_PIN, 0);
    ledcDetach(BUZZER_PIN);
}

void buzzerStop() {
    ledcWrite(BUZZER_PIN, 0);
    ledcDetach(BUZZER_PIN);
}

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

uint32_t last_s = 0;
uint32_t last_note = 0;
uint32_t now = millis();
int clock_s = 0;
void loop()
{
  now = millis();
  if(now - last_s >= 1000)
  {
    last_s = now;
    clock_s++;
    if(clock_s % 15 == 0)
    {
        if(updateSleep == 1)updateSleep = 2;
        if(updateSleep == 0)updateSleep = 1;
    }
    dirty = true;
  }
  if(notesSize){
    if(now-last_note  >= notesTick){
      last_note = now;
      buzzerTone(notes[notesIndex*2+1], notes[notesIndex*2]);
      notesIndex ++;       
      if(notesIndex == notesSize){
        notesSize = 0;
        buzzerStop();
      }else{
        notesTick = notes[notesIndex*2];
      }
    }
  }

  readOntoBuffer();
  delay(1);
}