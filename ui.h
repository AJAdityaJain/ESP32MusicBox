#pragma once
#include <U8g2lib.h>
#include <Wire.h>
#include "sd_interface.h"


#define KY040_CLK_PIN 34
#define KY040_DT_PIN 32
#define TOUCH_PIN 12
#define THRESHOLD 10
#define AVG_SAMPLES 16
#define TOUCH_TICKS_THRESHHOLD 15
#define PLAYLIST_PAGE_SIZE 4

extern volatile bool uiNeedsUpdate;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C ctx;
extern QueueHandle_t uiQueue;


#define HOME 0
#define MUSIC 1
#define BLUETOOTH 2
#define WIFI 3
#define SETTINGS 4
#define BROWSE 5
#define PLAYLISTS 6
#define ARTISTS 7
#define PLAYSCREEN 8


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
// uint16_t array pointer to hold selected playlist item list address(4 bytes) without any size
extern uint16_t* playlistItems;
extern int size;

void uiTask(void *pvParameters);
void initTouchSamples();
void IRAM_ATTR encoderISR();
