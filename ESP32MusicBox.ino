#include "sd_interface.h"
#include "MP3DecoderHelix.h"

#define BT_DEVICE_NAME "Bose QC35 II AJ"

static bool is_connected_or_connecting = false;
static bool has_paired_addr = false;
static esp_bd_addr_t paired_addr = {0};
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
  Serial.begin(115200);
  delay(1000);

  if (init_sd())
  {
    has_paired_addr = load_addr(paired_addr);
  }

  a2dp_source.set_ssp_enabled(true);
  a2dp_source.set_on_connection_state_changed(connection_state_changed);
  a2dp_source.set_data_callback_in_frames(get_data_frames);
  a2dp_source.start(BT_DEVICE_NAME);
  delay(1000);
  a2dp_source.connect_to(paired_addr);
}

void loop()
{
  //get serial input 
  while(Serial.available())
  {
    char c = Serial.read();
    if (c == 'c')
    {
      a2dp_source.connect_to(paired_addr);
    }
  }

  delay(10);
}