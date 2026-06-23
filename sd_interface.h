#pragma once
#include "SD.h"
#include "SPI.h"
#include "audio_frame.h"


#define ADDR_FILE       "/bt_paired.txt"
#define SD_CS_PIN       5

bool init_sd();
bool save_addr(const esp_bd_addr_t addr);
bool load_addr(esp_bd_addr_t addr);