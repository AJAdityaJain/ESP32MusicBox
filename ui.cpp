#include "ui.h"

int temporary_T = 0;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C ctx(U8G2_R0, U8X8_PIN_NONE);
uint16_t touchSamples[AVG_SAMPLES];
int sampleIndex = 0;
bool samplesReady = false;
float touchAverage = 0;
UIState screenState = HOME;
int cursor = 0;


volatile bool uiNeedsUpdate = false;

QueueHandle_t uiQueue;


void initTouchSamples()
{
  for (int i = 0; i < AVG_SAMPLES; i++)
  {
    touchSamples[i] = touchRead(TOUCH_PIN);
    delay(5);
  }
  samplesReady = true;
  // calculate initial average
  float sum = 0;
  for (int i = 0; i < AVG_SAMPLES; i++)
    sum += touchSamples[i];
  touchAverage = sum / AVG_SAMPLES;
}

void processTouchInTask()
{
  uint16_t val = touchRead(TOUCH_PIN);

  if (!samplesReady)
    return;

  if (touchAverage - val >= THRESHOLD)
  {
    // activation — do not update average
    UIEvent evt = TOUCH;
    xQueueSend(uiQueue, &evt, 0);
  }
  else
  {
    // normal — update rolling average
    touchSamples[sampleIndex] = val;
    sampleIndex = (sampleIndex + 1) % AVG_SAMPLES;

    float sum = 0;
    for (int i = 0; i < AVG_SAMPLES; i++)
      sum += touchSamples[i];
    touchAverage = sum / AVG_SAMPLES;
  }
}

void IRAM_ATTR encoderISR()
{
  static int last_state = 0;
  static int8_t accumulator = 0;

  int state = (digitalRead(KY040_CLK_PIN) << 1) | digitalRead(KY040_DT_PIN);
  if (state != last_state)
  {
    int index = (last_state << 2) | state;
    accumulator += encoder_transition_table[index];
    last_state = state;

    if (accumulator >= 4)
    {
      UIEvent evt = TURN_RIGHT;
      xQueueSendFromISR(uiQueue, &evt, NULL);
      accumulator = 0;
    }
    else if (accumulator <= -4)
    {
      UIEvent evt = TURN_LEFT;
      xQueueSendFromISR(uiQueue, &evt, NULL);
      accumulator = 0;
    }
  }
}


void onScroll(bool r)
{ 
  if(screenState == HOME){
    cursor += r ? 1 : -1;
  }

  uiNeedsUpdate = true;
}

void onTouch()
{
  temporary_T++;
  uiNeedsUpdate = true;
}

void drawScreen()
{
  ctx.clearBuffer();
  // ctx.setCursor(0, 40);
  // ctx.printf("L:%d R:%d T:%d", temporary_L, temporary_R, temporary_T);
  if(screenState == HOME){
    for(int i = 0; i < UIStateLen; i++){
      if(i != HOME){
        ctx.drawStr(10, 20 + i * 10, (screenState == (UIState)i) ? ">" : " ");
      }
    }
  }
  ctx.sendBuffer();

}


// oled + all UI here
void uiTask(void *pvParameters)
{
  UIEvent evt;
  while (true)
  {
    processTouchInTask(); // poll touch every loop

    if (xQueueReceive(uiQueue, &evt, pdMS_TO_TICKS(50)))
    {
      switch (evt)
      {
      case TURN_LEFT:
        onScroll(false);
        break;
      case TURN_RIGHT:
        onScroll(true);
        break;
      case TOUCH:
        onTouch();
        break;
      }
    }

    if (uiNeedsUpdate)
    {
      drawScreen();
      uiNeedsUpdate = false;
    }
  }
}
