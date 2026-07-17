#include "sd_interface.h"
#include "freertos/semphr.h"
#define BT_DEVICE_NAME "Bose QC35 II AJ"
#include <U8g2lib.h>
#include <Wire.h>


using namespace libhelix;

MP3DecoderHelix helix;
File mp3File;


static bool is_connected_or_connecting = false;
static bool has_paired_addr = false;
static esp_bd_addr_t paired_addr = {0};
BluetoothA2DPSource a2dp_source;



U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

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
  delay(50);
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setDisplayRotation(U8G2_R2);
  u8g2.setFont(u8g2_font_ncenB08_tr);	

  u8g2.drawLine(1,1,1,10);

  if (init_sd())
  {
    has_paired_addr = load_addr(paired_addr);
  }
  else{
    u8g2.drawStr(1,20,"memory card missing");
    u8g2.sendBuffer();
    return;
  }
  u8g2.drawLine(1,1,10,1);

  u8g2.sendBuffer();
  delay(5);

  mp3File = SD.open("/aud.mp3");
  u8g2.drawLine(126,52,126,62);

  if (!mp3File)
  {
    Serial.println("File not found");
    return;
  }
  u8g2.sendBuffer();
  delay(5);


  pcmMutex = xSemaphoreCreateMutex();
  helix.begin();
  helix.setDataCallback(pcmCallback);

  a2dp_source.set_ssp_enabled(true);
  a2dp_source.set_on_connection_state_changed(connection_state_changed);
  a2dp_source.set_on_audio_state_changed(audio_state_changed); // ← add this
  a2dp_source.set_data_callback_in_frames(get_data_frames);
  a2dp_source.start();
  u8g2.drawStr(55,45,"LIRA");
  u8g2.sendBuffer();

  delay(900);
  a2dp_source.connect_to(paired_addr);
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
  // get serial input
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == 'c')
    {
      a2dp_source.connect_to(paired_addr);
    }
  }

  delay(10);
}