#pragma once
#include <U8g2lib.h>
#include <Wire.h>


#define KY040_CLK_PIN 34
#define KY040_DT_PIN 32
#define TOUCH_PIN 12
#define THRESHOLD 15
#define AVG_SAMPLES 16
#define TOUCH_TICKS_THRESHHOLD 15

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C ctx;
extern uint16_t touchSamples[AVG_SAMPLES];
extern int sampleIndex;
extern bool samplesReady;
extern float touchAverage;
extern int touchTicks;

extern volatile bool uiNeedsUpdate;

extern QueueHandle_t uiQueue;


#define HOME 0
#define MUSIC 1
#define BLUETOOTH 2
#define WIFI 3
#define SETTINGS 4


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


extern int screenState;
extern int cursor;
extern float cursorF;


void uiTask(void *pvParameters);
void initTouchSamples();
void IRAM_ATTR encoderISR();
