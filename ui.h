#pragma once
#include <U8g2lib.h>
#include <Wire.h>


#define KY040_CLK_PIN 34
#define KY040_DT_PIN 32
#define TOUCH_PIN 12
#define THRESHOLD 15
#define AVG_SAMPLES 16

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C ctx;
extern uint16_t touchSamples[AVG_SAMPLES];
extern int sampleIndex;
extern bool samplesReady;
extern float touchAverage;

extern volatile bool uiNeedsUpdate;

extern QueueHandle_t uiQueue;

#define UIStateLen 4
enum UIState
{
  BLUETOOTH,
  HOME,
  MUSIC,
  SETTINGS  
};


enum UIEvent
{
  TURN_LEFT,
  TURN_RIGHT,
  TOUCH
};

// encoder
static const int8_t encoder_transition_table[16] = {
    0, 1, -1, 0,
    -1, 0, 0, 1,
    1, 0, 0, -1,
    0, -1, 1, 0};


extern int temporary_T;

extern UIState screenState;
extern int cursor;


void uiTask(void *pvParameters);
void initTouchSamples();
void IRAM_ATTR encoderISR();
