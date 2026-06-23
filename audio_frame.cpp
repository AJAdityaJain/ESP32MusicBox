#include "audio_frame.h"


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
