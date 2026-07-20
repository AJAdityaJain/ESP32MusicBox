#include "sd_interface.h"
#include "ui.h"

#include "freertos/semphr.h"
#include "esp_bt.h"

#define SEARCH_NAME "Bose QC35 II AJ"
#define BT_DEVICE_NAME "LIRA"

using namespace libhelix;

MP3DecoderHelix helix;
File mp3File;

bool is_connected_or_connecting = false;
bool has_paired_addr = false;
esp_bd_addr_t paired_addr = {0};
BluetoothA2DPSource a2dp_source;





void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
  const char *names[] = {"DISCONNECTED", "CONNECTING", "CONNECTED", "DISCONNECTING"};
  Serial.printf("[BT] State: %s\n", names[state]);

  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED)
  {
    is_connected_or_connecting = true;
  }
  else if (state == ESP_A2D_CONNECTION_STATE_CONNECTING)
  {
    is_connected_or_connecting = true;
  }
  else
  {
    is_connected_or_connecting = false;
  }
}

void setup()
{
  delay(1000);
  Serial.begin(115200);

  ctx.begin();
  ctx.setDisplayRotation(U8G2_R2);
  ctx.setFont(u8g2_font_ncenB08_tr);


  if (!init_sd())
  {
    ctx.drawStr(1, 20, "memory card missing");
    ctx.sendBuffer();
    return;
  }
  has_paired_addr = load_addr(paired_addr);
  ctx.drawLine(1, 1, 1, 10);
  ctx.sendBuffer();

  mp3File = SD.open("/tame/aud.mp3");
  if (!mp3File)
  {
    ctx.drawStr(1, 20, "file missing");
    ctx.sendBuffer();
    return;
  }


  if (pcmMutex == nullptr)
  {
    pcmMutex = xSemaphoreCreateMutex();
  }
  pcmHead = 0;
  pcmTail = 0;
  
  helix.begin();
  helix.setDataCallback(pcmCallback);
  ctx.drawLine(126, 52, 126, 62);
  ctx.sendBuffer();


  a2dp_source.set_ssp_enabled(true);
  a2dp_source.set_on_connection_state_changed(connection_state_changed);
  a2dp_source.set_on_audio_state_changed(audio_state_changed); // ← add this
  a2dp_source.set_data_callback_in_frames(get_data_frames);
  esp_bt_dev_set_device_name(BT_DEVICE_NAME);

  a2dp_source.start();

  ctx.drawStr(55, 45, "LIRA");
  ctx.sendBuffer();



  uiQueue = xQueueCreate(20, sizeof(UIEvent));

  initTouchSamples();

  attachInterrupt(digitalPinToInterrupt(KY040_CLK_PIN), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(KY040_DT_PIN), encoderISR, CHANGE);

  xTaskCreatePinnedToCore(uiTask, "ui", 4096, NULL, 1, NULL, 1);
}

void loop()
{
  if (isPlaying && mp3File)
  {
    if (mp3File.available())
    {
      int available = (pcmTail - pcmHead + PCM_BUF_SIZE) % PCM_BUF_SIZE;
      if (available < PCM_BUF_SIZE * 3 / 4)
      {
        uint8_t chunk[512];
        int n = mp3File.read(chunk, sizeof(chunk));
        if (n > 0)
          helix.write(chunk, n);
      }
    }
    else
    {
      isPlaying = false;
      mp3File.close();
      Serial.println("Done.");
    }
  }
  delay(10);
}