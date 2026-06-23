#include "sd_interface.h"
#include "MP3DecoderHelix.h"

#define BT_DEVICE_NAME "Bose QC35 II AJ"

static bool is_connected_or_connecting = false;
static bool has_paired_addr = false;
static esp_bd_addr_t paired_addr = {0};
BluetoothA2DPSource a2dp_source;

int32_t get_data_frames(Frame *frame, int32_t frame_count)
{
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
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
  const char *names[] = {"DISCONNECTED", "CONNECTING", "CONNECTED", "DISCONNECTING"};
  Serial.printf("[BT] State: %s\n", names[state]);

  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED)
  {
    is_connected_or_connecting = true;

    delay(1000); // Wait for a moment to ensure the connection is stable before saving the address
    esp_bd_addr_t *addr_ptr = a2dp_source.get_last_peer_address();
    memcpy(paired_addr, *addr_ptr, sizeof(esp_bd_addr_t));
    has_paired_addr = save_addr(paired_addr);
    if (has_paired_addr)
      a2dp_source.connect_to(paired_addr);
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
}

void loop()
{
  if (!is_connected_or_connecting)
  {
    Serial.printf("[BT] Connecting\n");
    if (has_paired_addr)
    {
      a2dp_source.connect_to(paired_addr);
    }
    else
    {
      a2dp_source.start(BT_DEVICE_NAME);
    }
  }

  delay(5000);
}