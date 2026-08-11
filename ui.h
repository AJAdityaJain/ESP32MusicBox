#pragma once
#include <U8g2lib.h>
#include <Wire.h>

#include "sd_interface.h"
#include "bt.h"

#define KY040_CLK_PIN 34
#define KY040_DT_PIN 32
#define TOUCH_PIN 12
#define THRESHOLD 10
#define AVG_SAMPLES 16
#define TOUCH_TICKS_THRESHHOLD 15
#define DOUBLECLICK_THRESHOLD 10
#define PLAYLIST_PAGE_SIZE 4
#define SENSITIVITY 3
#define BEEP


enum UIEvent
{
  TURN_LEFT,
  TURN_RIGHT,
  TOUCH
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
  uint16_t* playlistItems = nullptr;
  volatile bool play = false;
  int playlistSize = 0;
  int playlistIndex = -1;
  bool volumeMode = false;
  int volumeLevel = 80;
  File activeFile;
  String songName;
  String artistName;
  uint32_t elapsedSamples_ = 0;
  uint32_t totalSamples_ = 0;
  uint32_t sampleRate_ = 0;
  bool hasDuration_ = false;
  void init(String name);
  void next();
  void prev();
  void resetProgress();
  void setPlaybackDuration(uint32_t fileSize, uint32_t sampleRate, uint32_t bitrate);
  void updateProgress(uint32_t samplesDecoded, uint32_t sampleRate, uint32_t bitrate);
  String formatTime(uint32_t seconds) const;
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
  int mode = 0; // 0: select playlist, 1: select artist
  int offset_;
  int listCount_;
  String listItems_[PLAYLIST_PAGE_SIZE];
};

// encoder
static const int8_t encoder_transition_table[16] = {
    0, 1, -1, 0,
    -1, 0, 0, 1,
    1, 0, 0, -1,
    0, -1, 1, 0};

extern bool updateSecond;
extern uint8_t updateSleep;
extern uint32_t clock_s;

void uiTask(void *pvParameters);
void uiInit();
void readOntoBuffer();
void error(const char* str);