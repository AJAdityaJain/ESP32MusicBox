#include "ui.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C ctx(U8G2_R0, U8X8_PIN_NONE);
uint16_t touchSamples[AVG_SAMPLES];
int sampleIndex = 0;
bool samplesReady = false;
float touchAverage = 0;
int touchTicks = 0;

uint16_t *playlistItems = nullptr;
int playlistSize = 0;

volatile bool dirty = false;

QueueHandle_t uiQueue;

HomeScreen homeScreen_;
MusicScreen musicScreen_;
PlaylistScreen playlistScreen_;

BaseScreen *activeScreen_ = &homeScreen_;

void BaseScreen::onScroll(bool right)
{
  // cursorF += r ? 0.3 : -0.3;
  cursor_ = right ? (cursor_ + 1)
                  : (cursor_ - 1);
  if(cursor_ < 0) cursor_ = limit_-1;
  if(cursor_ > limit_) cursor_ = 0;
}

HomeScreen::HomeScreen() { cursor_ = 0; limit_ = SENSITIVITY*4;}
void HomeScreen::onTouch() {
    int option = (cursor_/SENSITIVITY);
    if(option == 0) {activeScreen_ = &musicScreen_;}
};
void HomeScreen::onRender() {
    // BT
    int option = (cursor_/SENSITIVITY);

    if(option == 1)ctx.drawRFrame(2, 8, 20, 20, 2);
    ctx.drawLine(12, 10, 12, 26);
    ctx.drawLine(11, 10, 11, 26);
    ctx.drawLine(6, 14, 18, 22);
    ctx.drawLine(6, 15, 18, 23);
    ctx.drawLine(6, 22, 18, 14);
    ctx.drawLine(6, 23, 18, 15);
    ctx.drawLine(12, 10, 18, 14);
    ctx.drawLine(12, 9, 18, 13);
    ctx.drawLine(12, 26, 18, 22);
    ctx.drawLine(12, 25, 18, 21);

    // Music
    if(option == 2)ctx.drawRFrame(27, 8, 20, 20, 2);

    ctx.drawLine(29,25,29,18);
    ctx.drawLine(30,25,30,19);
    ctx.drawLine(29,17,36,10);
    ctx.drawLine(30,18,37,11);
    ctx.drawLine(38,10,44,10);
    ctx.drawLine(39,11,44,11);

    ctx.drawLine(33,25,33,20);
    ctx.drawLine(34,25,34,21);
    ctx.drawLine(33,19,38,14);
    ctx.drawLine(34,20,39,15);
    ctx.drawLine(39,14,44,14);
    ctx.drawLine(40,15,44,15);

    ctx.drawLine(37,25,37,22);
    ctx.drawLine(38,25,38,23);
    ctx.drawLine(37,21,40,18);
    ctx.drawLine(38,22,41,19);
    ctx.drawLine(41,18,44,18);
    ctx.drawLine(42,19,44,19);

    ctx.drawLine(41,23,41,25);
    ctx.drawLine(42,22,42,26);
    ctx.drawLine(43,22,43,26);
    ctx.drawLine(44,22,44,26);
    ctx.drawLine(45,23,45,25);

    // Settings
    if(option == 3)ctx.drawRFrame(52, 8, 20, 20, 2);
    ctx.drawRFrame(55, 10, 13, 16, 5);

    if(option == 0)ctx.drawRFrame(77, 8, 20, 20, 2);

    //rect

    ctx.drawBox(81, 11, 12, 2);
    ctx.drawBox(81, 13, 2, 10);
    ctx.drawBox(91, 13, 2, 10);
    ctx.drawBox(82, 21, 3, 3);
    ctx.drawBox(92, 21, 3, 3);
    ctx.drawLine(85,22,85,23);
    ctx.drawLine(95,22,95,23);

};

MusicScreen::MusicScreen() { cursor_ = PLAYLISTS; limit_=SENSITIVITY*4; }
void MusicScreen::onTouch() {
  int option = (cursor_/SENSITIVITY);
  if(option == 0) activeScreen_ = &homeScreen_;
  if(option == 2) activeScreen_ = &playlistScreen_;
};
void MusicScreen::onRender() {
    int option = (cursor_/SENSITIVITY);
    if(option == 0)ctx.drawRFrame(0, 0, 128, 15, 2);
    ctx.drawStr(1, 13, "Home");
    if(option == 1)ctx.drawRFrame(0, 15, 128, 15, 2);
    ctx.drawStr(1, 28, "| Browse");
    if(option == 2)ctx.drawRFrame(0, 30, 128, 15, 2);
    ctx.drawStr(1, 43, "| Playlists");
    if(option == 3)ctx.drawRFrame(0, 45, 128, 15, 2);
    ctx.drawStr(1, 58, "| Artists");

};

PlaylistScreen::PlaylistScreen() { cursor_ = 0; playlistCount_ == -1; offset_= 0;}
void PlaylistScreen::onTouch() {
    if(cursor_ < 0 || cursor_-offset_*PLAYLIST_PAGE_SIZE > playlistCount_-1) return;
    Serial.printf("Selected playlist: %s\n", playlistNames_[cursor_-offset_*PLAYLIST_PAGE_SIZE].c_str());
    // get selected playlist items
    if(playlistItems != nullptr){
      free(playlistItems);
      playlistItems = nullptr;
    }
    playlistSize = get_playlist_items(playlistNames_[cursor_-offset_*PLAYLIST_PAGE_SIZE].c_str(),playlistItems);
    if(playlistSize == 0){
      Serial.println("Failed to load playlist items");
    }
    //display all items in playlistItems on serial
    for(int i=0;i<playlistSize;i++){
      Serial.printf("Item %d: %d\n", i, playlistItems[i]);
    }

};
void PlaylistScreen::onRender() {
      if (playlistCount_ == -1){
      playlistCount_ = get_playlists_after(offset_*PLAYLIST_PAGE_SIZE, playlistNames_, PLAYLIST_PAGE_SIZE);
    }
    ctx.drawRFrame(0, 15*(cursor_-(offset_*PLAYLIST_PAGE_SIZE)), 128, 15, 2);
    for(int i=0;i<playlistCount_;i++){
      if(playlistNames_[i].length() > 0)ctx.drawStr(1, (i*15)+13, playlistNames_[i].c_str());
    }

};
void PlaylistScreen::onScroll(bool right)
{
  cursor_ += right ? 1 : -1;
  if (cursor_ - (offset_ * PLAYLIST_PAGE_SIZE) > PLAYLIST_PAGE_SIZE - 1)
  {
    offset_++;
    playlistCount_ = -1;
  };
  if (cursor_ - (offset_ * PLAYLIST_PAGE_SIZE) < 0)
  {
    if (offset_ > 0)
    {
      offset_--;
      playlistCount_ = -1;
    }
    else
    {
      cursor_ = 0;
    }
  }
}

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

  // Serial.println(touchTicks);

  if (touchAverage - val >= THRESHOLD)
  {
    if (touchTicks <= TOUCH_TICKS_THRESHHOLD)
    {
      touchTicks++;
    }
  }
  else
  {
    if (touchTicks > 0 && touchTicks < TOUCH_TICKS_THRESHHOLD)
    {
      UIEvent evt = TOUCH;
      xQueueSend(uiQueue, &evt, 0);
    }
    touchTicks = 0;
    // normal — update rolling average
    touchSamples[sampleIndex] = val;
    sampleIndex = (sampleIndex + 1) % AVG_SAMPLES;

    float sum = 0;
    for (int i = 0; i < AVG_SAMPLES; i++)
      sum += touchSamples[i];
    touchAverage = sum / AVG_SAMPLES;
  }

  // UIEvent evt = TOUCH;
  // xQueueSend(uiQueue, &evt, 0);
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
  activeScreen_->onScroll(r);
  dirty = true;
}

void onTouch()
{
  activeScreen_->onTouch();
  dirty = true;
  return;
  // else if(screenState == PLAYSCREEN){
  //   isPlaying = !isPlaying;
  // }
}


void drawScreen()
{
  ctx.clearBuffer();
  activeScreen_->onRender();
  ctx.sendBuffer();
  return;
  // else if(screenState == PLAYSCREEN){
  //   if(isPlaying){
  //     ctx.drawStr(1, 13, "Playing");
  //   }
  //   else{
  //     ctx.drawStr(1, 13, "Paused");
  //   }
  // }

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

    if (dirty)
    {
      drawScreen();
      dirty = false;
    }
  }
}
