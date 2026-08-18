#pragma once
#include <U8g2lib.h>
#include <Wire.h>

#include "sd_interface.h"
#include "bt.h"

#define KY040_CLK_PIN 34
#define KY040_DT_PIN 32
#define TOUCH_PIN 12
#define THRESHOLD 15
#define AVG_SAMPLES 16
#define TOUCH_TICKS_THRESHHOLD 15
#define DOUBLECLICK_THRESHOLD 5
#define PLAYLIST_PAGE_SIZE 4
#define SENSITIVITY 3
#define BEEP

extern bool updateSecond;
extern uint8_t updateSleep;
extern QueueHandle_t uiQueue;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C ctx;

enum UIEvent
{
  TURN_LEFT,
  TURN_RIGHT,
  TOUCH
};

class TouchManager {
private:
  uint16_t touchSamples[AVG_SAMPLES] = {0};
  int sampleIndex = 0;
  bool samplesReady = false;
  float touchAverage = 0.0f;
  int touchTicks = 0;
  int doubleclick = 0;

public:
  void begin() {
    for (int i = 0; i < AVG_SAMPLES; ++i) {
      touchSamples[i] = touchRead(TOUCH_PIN);
      delay(5);
    }

    sampleIndex = 0;
    samplesReady = true;

    float sum = 0.0f;
    for (int i = 0; i < AVG_SAMPLES; ++i) {
      sum += touchSamples[i];
    }
    touchAverage = sum / AVG_SAMPLES;
  }

  void processTouchInTask() {
    if (!samplesReady) {
      return;
    }

    const uint16_t val = touchRead(TOUCH_PIN);

    if (touchAverage - val >= THRESHOLD) {
      if (touchTicks <= TOUCH_TICKS_THRESHHOLD) {
        if (updateSleep == 2) {
          ctx.setPowerSave(0);
        }
        updateSleep = 0;
        touchTicks++;
        if (doubleclick >= 0 && doubleclick < DOUBLECLICK_THRESHOLD) {
          doubleclick++;
        }
      }
      return;
    }

    if (touchTicks > 0 && touchTicks < TOUCH_TICKS_THRESHHOLD) {
      if (doubleclick > 0 && doubleclick < DOUBLECLICK_THRESHOLD) {
        const UIEvent evt = TOUCH;
        xQueueSend(uiQueue, &evt, 0);
        doubleclick = -1;
      } else {
        doubleclick = 0;
      }
    }

    touchTicks = 0;
    touchSamples[sampleIndex] = val;
    sampleIndex = (sampleIndex + 1) % AVG_SAMPLES;

    float sum = 0.0f;
    for (int i = 0; i < AVG_SAMPLES; ++i) {
      sum += touchSamples[i];
    }
    touchAverage = sum / AVG_SAMPLES;
  }
};

class BaseScreen
{
public:
  virtual ~BaseScreen() {}
  virtual void onScroll(bool right);
  virtual void onTouch() = 0;
  virtual void onRender() = 0;
protected:
  int cursor_;
  int limit_;
};

class HomeScreen : public BaseScreen
{
public:
  HomeScreen();
  void onTouch() override;
  void onRender() override;
};

class BTScreen : public BaseScreen
{
public:
  BTScreen();
  void onTouch() override;
  void onRender() override;
};

class MusicScreen : public BaseScreen
{
public:
  MusicScreen();
  void onTouch() override;
  void onRender() override;
};

class MusicPlayerScreen : public BaseScreen
{
public:
  MusicPlayerScreen();
  void onScroll(bool right) override;
  void onTouch() override;
  void onRender() override;

  File activeFile;
  volatile bool play = false;
  void next();
  void prev();
  void queue(String path);
  void queue(uint16_t index);
  void resetProgress();
  void setPlaybackDuration(uint32_t fileSize, uint32_t sampleRate, uint32_t bitrate);
  void updateProgress(uint32_t samplesDecoded, uint32_t sampleRate, uint32_t bitrate);
  void emptyQueue();

  private:
  int playlistIndex = -1;
  bool volumeMode = false;
  int volumeLevel = 80;
  String songName;
  String artistName;
  uint32_t elapsedSamples_ = 0;
  uint32_t totalSamples_ = 0;
  uint32_t sampleRate_ = 0;
  bool hasDuration_ = false;
  String formatTime(uint32_t seconds) const;
  uint16_tVec playlistItems;  
};

class ListScreen : public BaseScreen
{
public:
  
  ListScreen();
  void init(int mode);
  void update();
  void onScroll(bool right) override;
  void onTouch() override;
  void onRender() override;

private:
  int mode = 0; // 0: select playlist, 1: select artist, 2: browse, 3: browse an artist

  String artist;
  int offset_;
  int listCount_;
  String listItems_[PLAYLIST_PAGE_SIZE];
};
static const int8_t encoder_transition_table[16] = {
    0, 1, -1, 0,
    -1, 0, 0, 1,
    1, 0, 0, -1,
    0, -1, 1, 0};


void uiTask(void *pvParameters);
void uiInit();
void readOntoBuffer();
void error(const char* str);