#include "ui.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C ctx(U8G2_R0, U8X8_PIN_NONE);
bool updateSecond = false;
uint8_t updateSleep = 0;

inline void drawPlay(int x, int y,bool fwd){
  if(fwd){
    ctx.drawBox(x,y,1,11);
    ctx.drawBox(x+1,y+1,2,9);
    ctx.drawBox(x+3,y+2,2,7);
    ctx.drawBox(x+5,y+3,2,5);
    ctx.drawBox(x+7,y+4,2,3);
    ctx.drawBox(x+9,y+5,1,1);
  }else{
    ctx.drawBox(x,y,1,11);
    ctx.drawBox(x-2,y+1,2,9);
    ctx.drawBox(x-4,y+2,2,7);
    ctx.drawBox(x-6,y+3,2,5);
    ctx.drawBox(x-8,y+4,2,3);
    ctx.drawBox(x-9,y+5,1,1);

  }
}

inline void drawHome(int x, int y){
  ctx.drawBox(x,y+4,9,1);
  ctx.drawBox(x+1,y+3,7,1);
  ctx.drawBox(x+2,y+2,5,1);
  ctx.drawBox(x+3,y+1,3,1);
  ctx.drawBox(x+4,y,1,1);
  ctx.drawBox(x+4,y,1,1);
  ctx.drawFrame(x+1,y+4,7,7);
}


QueueHandle_t uiQueue;
TouchManager touchManager;

HomeScreen homeScreen_;
BTScreen btScreen_;
MusicScreen musicScreen_;
ListScreen playlistScreen_;
MusicPlayerScreen playScreen_;

BaseScreen *activeScreen_ = &homeScreen_;

void resetPlaybackProgress()
{
  playScreen_.resetProgress();
}

void updatePlaybackProgress(uint32_t samplesDecoded, uint32_t sampleRate, uint32_t bitrate)
{
  playScreen_.updateProgress(samplesDecoded, sampleRate, bitrate);
}

void setPlaybackDuration(uint32_t fileSize, uint32_t sampleRate, uint32_t bitrate)
{
  playScreen_.setPlaybackDuration(fileSize, sampleRate, bitrate);
}

void BaseScreen::onScroll(bool right)
{
  cursor_ = right ? (cursor_ + 1)
                  : (cursor_ - 1);
  if(cursor_ < 0) cursor_ = limit_-1;
  if(cursor_ >= limit_) cursor_ = 0;
}

HomeScreen::HomeScreen() { cursor_ = 0; limit_ = SENSITIVITY*5;}
void HomeScreen::onTouch() {
    int option = (cursor_/SENSITIVITY);
    if(option == 0) {activeScreen_ = &musicScreen_;}
    if(option == 1){activeScreen_ = &btScreen_;}
    if(option == 3){activeScreen_ = &playScreen_;}
    
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

    // music
    if(option == 3)ctx.drawRFrame(52, 8, 20, 20, 2);
    drawPlay(58, 13, true);

    if(option == 0)ctx.drawRFrame(77, 8, 20, 20, 2);

    //rect

    ctx.drawBox(81, 11, 12, 2);
    ctx.drawBox(81, 13, 2, 10);
    ctx.drawBox(91, 13, 2, 10);
    ctx.drawBox(82, 21, 3, 3);
    ctx.drawBox(92, 21, 3, 3);
    ctx.drawLine(85,22,85,23);
    ctx.drawLine(95,22,95,23);

    //Cross
  if(option == 4)ctx.drawRFrame(102-50,25+ 8, 20, 20, 2);

  ctx.drawLine(103+3-50, 9+3+25,103+18-6-50,9+18-6+25);
  ctx.drawLine(103+18-6-50, 9+3+25,103+3-50,9+18-6+25);

};

BTScreen::BTScreen() { cursor_ = 0; limit_ = SENSITIVITY*4;}
void BTScreen::onTouch(){
  int option = (cursor_/SENSITIVITY);
  if(option == 0) activeScreen_=&homeScreen_;
  else{btAttempt(option ==1?&hdAddr:option == 2?&boseAddr:option == 3?&earAddr:nullptr);}

  if(option == 3){digitalWrite(4, HIGH);}
  else{digitalWrite(4, LOW);}
}
void BTScreen::onRender(){
  if(connectionState == 0){ctx.drawStr(1, 58,"X");}
  else if (connectionState == 1){ctx.drawStr(1, 58,"...");}
  else{ctx.drawStr(1, 58,"Connected");}


    int option = (cursor_/SENSITIVITY);
    if(option == 0)ctx.drawRFrame(0, 0, 128, 15, 2);
    ctx.drawStr(1, 13, "Home");

    if(option != 0)ctx.drawRFrame(0, 15, 128, 15, 2);
    ctx.drawStr(1, 28, option == 1? "HD 458BT":option == 2? "Bose QC35":option == 3? "Jack":"???");
}

MusicScreen::MusicScreen() { cursor_ = 0; limit_=SENSITIVITY*4; }
void MusicScreen::onTouch() {
  int option = (cursor_/SENSITIVITY);
  if(option == 0) activeScreen_ = &homeScreen_;
  if(option == 1) {activeScreen_ = &playlistScreen_;playlistScreen_.init(2);}
  if(option == 2) {activeScreen_ = &playlistScreen_;playlistScreen_.init(0);}
  if(option == 3) {activeScreen_ = &playlistScreen_;playlistScreen_.init(1);}
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

ListScreen::ListScreen() { cursor_ = 0; listCount_ = -1; offset_= 0;}
void ListScreen::onTouch() {
  if(cursor_ == -1) {activeScreen_ = &homeScreen_;return;}
  if(cursor_ < 0 || cursor_-offset_*PLAYLIST_PAGE_SIZE > listCount_-1) return;
  if(mode == 0 || mode == 1){
    activeScreen_ = &playScreen_;
    MusicPlayerScreen* casted = (MusicPlayerScreen*)activeScreen_;
    String selectedItem = listItems_[cursor_-offset_*PLAYLIST_PAGE_SIZE];
    if(mode == 0) selectedItem = "/.playlists/"+selectedItem;
    else if(mode == 1) selectedItem = "/"+selectedItem + "/index.bin";
    casted->queue(selectedItem.c_str());
  }
  else if(mode == 2){
    mode = 3;
    artist = listItems_[cursor_-offset_*PLAYLIST_PAGE_SIZE];
    listCount_ = -1;
    offset_ = 0;
    cursor_ = 0;
  }
  else if(mode == 3){
    activeScreen_ = &playScreen_;
    MusicPlayerScreen* casted = (MusicPlayerScreen*)activeScreen_;
    uint16_t index = getIndexFrom(artist, cursor_-offset_*PLAYLIST_PAGE_SIZE);
    casted->queue(index);
  }
};

void ListScreen::init(int m) {
  mode = m;
  offset_ = 0;
  update();
}
void ListScreen::update() {
  if (mode == 0) {
    listCount_ = fetchPlaylistsAfter(offset_*PLAYLIST_PAGE_SIZE, listItems_, PLAYLIST_PAGE_SIZE);
  } else if (mode == 1 || mode == 2) {
    listCount_ = fetchArtistsAfter(offset_*PLAYLIST_PAGE_SIZE, listItems_, PLAYLIST_PAGE_SIZE);
  }
  else if (mode == 3) {
    listCount_ = fetchSongsAfter(artist, offset_*PLAYLIST_PAGE_SIZE, listItems_, PLAYLIST_PAGE_SIZE);
  }

}


void ListScreen::onRender() {
    if (listCount_ == -1){
      update();
    }
    ctx.drawRFrame(0, 15*(cursor_-(offset_*PLAYLIST_PAGE_SIZE)), 128, 15, 2);
    for(int i=0;i<listCount_;i++){
      if(listItems_[i].length() > 0)ctx.drawStr(1, (i*15)+13, listItems_[i].c_str());
    }
    if(offset_ == 0){
      if(cursor_ == -1){
        ctx.drawRFrame(117, 0, 12, 12, 2);
      }; 
      drawHome(118, 1);
    }

};
void ListScreen::onScroll(bool right)
{
  cursor_ += right ? 1 : -1;
  if (cursor_ - (offset_ * PLAYLIST_PAGE_SIZE) > PLAYLIST_PAGE_SIZE - 1)
  {
    offset_++;
    listCount_ = -1;
  };
  if (cursor_ - (offset_ * PLAYLIST_PAGE_SIZE) < -1)
  {
    if (offset_ > 0)
    {
      offset_--;
      listCount_ = -1;
    }
    else
    {
      cursor_ = -1;
    }
  }
}

MusicPlayerScreen::MusicPlayerScreen()
{
  cursor_ = 0;
  limit_ = 6;
  playlistIndex = -1;
  play = false;
  resetProgress();
}

void MusicPlayerScreen::onScroll (bool right)
{
  if(!volumeMode){
    BaseScreen::onScroll(right);
  }
  else{
    volumeLevel += right ? 5 : -5;
    if(volumeLevel < 0) volumeLevel = 0;
    if(volumeLevel > 127) volumeLevel = 127;
    setVolume(volumeLevel);
  }
}

void MusicPlayerScreen::resetProgress(){
  elapsedSamples_ = 0;
  totalSamples_ = 0;
  sampleRate_ = 0;
  hasDuration_ = false;
}
void MusicPlayerScreen::setPlaybackDuration(uint32_t fileSize, uint32_t sampleRate, uint32_t bitrate){
  if (fileSize == 0 || sampleRate == 0 || bitrate == 0)
  {
    totalSamples_ = 0;
    sampleRate_ = sampleRate;
    hasDuration_ = false;
    return;
  }

  sampleRate_ = sampleRate;
  totalSamples_ = ((uint64_t)fileSize * 8ULL * sampleRate) / bitrate;
  hasDuration_ = totalSamples_ > 0;
}

void MusicPlayerScreen::updateProgress(uint32_t samplesDecoded, uint32_t sampleRate, uint32_t bitrate){
  elapsedSamples_ += samplesDecoded;
  if (sampleRate > 0)
  {
    sampleRate_ = sampleRate;
  }

  if (activeFile.size() > 0 && sampleRate_ > 0 && bitrate > 0 && !hasDuration_)
  {
    setPlaybackDuration(activeFile.size(), sampleRate_, bitrate);
  }
  if(updateSecond)
  {
    updateSecond = false;
    dirty = true;
  }
}
String MusicPlayerScreen::formatTime(uint32_t seconds) const
{
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", seconds / 60, seconds % 60);
  return String(buffer);
}
void MusicPlayerScreen::onRender(){
  //if option make box around 
    int option = (cursor_/1);

  if(option == 0) ctx.drawRFrame(1,0,13,13,2);
  drawHome(3,1);

  if(option == 1) ctx.drawRFrame(41,0,13,13,2);
  drawPlay(51,1,false);
  ctx.drawBox(42,1,1,11);

  if(option == 2) ctx.drawRFrame(57,0,13,13,2);
  if (play)
  {
    ctx.drawBox(59,1,3,11);
    ctx.drawBox(65,1,3,11);
  }
  else drawPlay(59,1,true);

  if(option == 3) ctx.drawRFrame(74,0,13,13,2);
  ctx.drawBox(84,1,1,11);
  drawPlay(75,1,true);

  if(option == 4)ctx.drawRFrame(101,0,13,13,2);

  ctx.drawBox(103,4,2,5);
  ctx.drawBox(105,3,1,7);
  ctx.drawBox(106,2,1,9);
  ctx.drawBox(107,1,2,11);
  ctx.drawBox(111,3,1,6);
  ctx.drawBox(111,3,1,6);
  ctx.drawBox(110,2,1,1);
  ctx.drawBox(110,9,1,1);


  if(option == 5) ctx.drawRFrame(113,0,13,13,2);
  ctx.drawRFrame(114,1,11,11,2);
  ctx.drawBox(117,8,1,1);
  ctx.drawBox(117,8,1,1);
  ctx.drawBox(119,6,1,1);
  ctx.drawBox(121,4,1,1);

  ctx.drawStr(4, 36, songName.c_str());
  ctx.drawStr(4, 47, artistName.c_str());
  if(volumeMode){
    ctx.drawBox(0,0, volumeLevel, 1);
  }

  ctx.setCursor(90, 48);
  ctx.printf("[%d/%d]", playlistIndex + 1, playlistItems.size());

  if (hasDuration_ && totalSamples_ > 0)
  {
    int e = elapsedSamples_ / 2;//double channel
    ctx.drawFrame(28, 54, 72, 6);
    int fillWidth = (e * 70) / totalSamples_;
    if (fillWidth > 70) fillWidth = 70;
    if (fillWidth > 0)
    {
      ctx.drawBox(29, 55, fillWidth, 4);
    }

    String elapsedText = formatTime(e / sampleRate_);
    String totalText = formatTime(totalSamples_ / sampleRate_);
    ctx.drawStr(4, 60, elapsedText.c_str());
    ctx.drawStr(96, 60, totalText.c_str());
  }
}
void MusicPlayerScreen::onTouch(){
  int option = (cursor_/1);

  if(option == 0){
    activeScreen_ = &homeScreen_;

  }
  else if(option == 1){
    prev();
  }
  else if(option == 2){
    play =!play;
  }
  else if(option == 3){
    next();
  }
  else if(option == 4){
    volumeMode = !volumeMode;
  }
  else if(option == 5){
    playlistItems.shuffle();
    resetProgress();    
  }
}

void MusicPlayerScreen::queue(uint16_t index){
  playlistItems.push(index);
  if(playlistIndex < 0){
    next();
  }
}

void MusicPlayerScreen::emptyQueue(){
  resetProgress();
  playlistItems.clear();
  playlistIndex = -1;

}
void MusicPlayerScreen::queue(String path){
  queueItems(path, playlistItems);

  if(playlistIndex < 0){
    next();
  }
}

void MusicPlayerScreen::next(){
  if(playlistItems.size() == playlistIndex+1)return;
  playlistIndex++;
  resetProgress();
  if(fetchMp3FromIndex(activeFile,playlistItems[playlistIndex])){
    String s = activeFile.path();
    String path = s.substring(1); 
    int slashIndex = path.indexOf('/');
    String fileWithExt = path.substring(slashIndex + 1); 
    
    artistName = path.substring(0, slashIndex);
    songName = fileWithExt.substring(0, fileWithExt.lastIndexOf('.'));
  }
  dirty = true;
}
void MusicPlayerScreen::prev(){
  if(0 == playlistIndex)return;
  playlistIndex--;
  resetProgress();
  if(fetchMp3FromIndex(activeFile,playlistItems[playlistIndex])){
    String s = activeFile.path();
    String path = s.substring(1); 
    int slashIndex = path.indexOf('/');
    String fileWithExt = path.substring(slashIndex + 1); 
    
    artistName = path.substring(0, slashIndex);
    songName = fileWithExt.substring(0, fileWithExt.lastIndexOf('.'));
  }
  dirty = true;
}

int32_t getDataFrames(Frame *frame, int32_t frame_count)
{
    if (!playScreen_.play)
    {
        #ifdef BEEP
            static float phase = 0.0f;
            const float increment = 2.0f * M_PI * 440.0f / 44100.0f;
            for (int i = 0; i < frame_count; i++)
            {
                int16_t s = (int16_t)(5000 * sinf(phase));
                frame[i].channel1 = s;
                frame[i].channel2 = s;
                phase += increment;
                if (phase > 2.0f * M_PI)
                phase -= 2.0f * M_PI;
            }
            return frame_count;
        #else
            memset(frame, 0, frame_count * sizeof(Frame));
            return frame_count;
        #endif        
    }

    if (pcmMutex == nullptr)
    {
        pcmMutex = xSemaphoreCreateMutex();
    }

    if (pcmMutex != nullptr && xSemaphoreTake(pcmMutex, portMAX_DELAY) == pdTRUE)
    {
        for (int i = 0; i < frame_count; i++)
        {
            static int16_t lastL = 0, lastR = 0;
            if (pcmHead != pcmTail)
            {
                lastL = pcmBuf[pcmHead];
                pcmHead = (pcmHead + 1) % PCM_BUF_SIZE;
            }
            if (pcmHead != pcmTail)
            {
                lastR = pcmBuf[pcmHead];
                pcmHead = (pcmHead + 1) % PCM_BUF_SIZE;
            }
            frame[i].channel1 = lastL;
            frame[i].channel2 = lastR;
        }

        xSemaphoreGive(pcmMutex);
    }
    return frame_count;
}

void readOntoBuffer(){
  if (playScreen_.play && playScreen_.activeFile)
  {
    if (playScreen_.activeFile.available())
    {
      int available = (pcmTail - pcmHead + PCM_BUF_SIZE) % PCM_BUF_SIZE;
      if (available < PCM_BUF_SIZE * 3 / 4)
      {
        uint8_t chunk[512];
        int n = playScreen_.activeFile.read(chunk, sizeof(chunk));
        if (n > 0)
          helix.write(chunk, n);
      }
    }
    else
    {
      playScreen_.activeFile.close();
      playScreen_.next();
    }
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

void uiInit()
{
  ctx.begin();
  ctx.setContrast(1);
  ctx.setDisplayRotation(U8G2_R2);
  ctx.setFont(u8g2_font_ncenB08_tr);
  ctx.drawStr(55, 45, "LIRA");
  ctx.sendBuffer();


  uiQueue = xQueueCreate(20, sizeof(UIEvent));
  touchManager.begin();

  attachInterrupt(digitalPinToInterrupt(KY040_CLK_PIN), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(KY040_DT_PIN), encoderISR, CHANGE);
}


void uiTask(void *pvParameters)
{
  UIEvent evt;
  while (true)
  {
    touchManager.processTouchInTask();

    if (xQueueReceive(uiQueue, &evt, pdMS_TO_TICKS(50)))
    {
      switch (evt) {
        case TURN_LEFT:  activeScreen_->onScroll(false); break;
        case TURN_RIGHT: activeScreen_->onScroll(true);  break;
        case TOUCH:      activeScreen_->onTouch();       break;
      }
      dirty = true;
    }

    if (dirty)
    {
      dirty = false;
      if(updateSleep == 2){        
        ctx.setPowerSave(1);
      } else {
        ctx.clearBuffer();
        activeScreen_->onRender();
        ctx.sendBuffer();
      }
    }
  }
}
void error(const char* str){
  ctx.drawStr(1, 20, str);
  ctx.sendBuffer();
}