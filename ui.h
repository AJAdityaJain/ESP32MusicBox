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
#define PLAYLIST_PAGE_SIZE 4
#define SENSITIVITY 3

extern volatile bool dirty;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C ctx;
extern File activeFile;


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
  void onTouch() override;
  void onRender() override;
  uint16_t* playlistItems_;
  int playlistSize_;
};

class PlaylistScreen : public BaseScreen
{
public:
  PlaylistScreen();
  void onScroll(bool right) override;
  void onTouch() override;
  void onRender() override;

private:
  int offset_;
  int playlistCount_ =-1;
  String playlistNames_[PLAYLIST_PAGE_SIZE];
};

// encoder
static const int8_t encoder_transition_table[16] = {
    0, 1, -1, 0,
    -1, 0, 0, 1,
    1, 0, 0, -1,
    0, -1, 1, 0};


void uiTask(void *pvParameters);
void uiInit();