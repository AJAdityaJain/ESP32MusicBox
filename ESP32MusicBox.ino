#include "ui.h"

using namespace libhelix;

MP3DecoderHelix helix;

void setup()
{
  delay(1000);
  Serial.begin(115200);

  ctx.begin();
  ctx.setDisplayRotation(U8G2_R2);
  ctx.setFont(u8g2_font_ncenB08_tr);


  if (!SDInit())
  {
    ctx.drawStr(1, 20, "memory card missing");
    ctx.sendBuffer();
    return;
  }
  ctx.drawLine(1, 1, 1, 10);
  ctx.sendBuffer();

  
  helix.begin();
  helix.setDataCallback(pcmCallback);
  ctx.drawLine(126, 52, 126, 62);
  ctx.sendBuffer();

  btInit();

  ctx.drawStr(55, 45, "LIRA");
  ctx.sendBuffer();



  uiInit();

  xTaskCreatePinnedToCore(uiTask, "ui", 4096, NULL, 1, NULL, 1);
}

void loop()
{
  if (isPlaying && activeFile)
  {
    if (activeFile.available())
    {
      int available = (pcmTail - pcmHead + PCM_BUF_SIZE) % PCM_BUF_SIZE;
      if (available < PCM_BUF_SIZE * 3 / 4)
      {
        uint8_t chunk[512];
        int n = activeFile.read(chunk, sizeof(chunk));
        if (n > 0)
          helix.write(chunk, n);
      }
    }
    else
    {
      isPlaying = false;
      activeFile.close();
      Serial.println("Done.");
    }
  }
  delay(10);
}