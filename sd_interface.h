#pragma once
#include "audio_frame.h"


#define ADDR_FILE       "/.lira/bt_paired.txt"
#define SD_CS_PIN       5
#define PLAYLISTS_DIR   "/.playlists"

bool init_sd();
bool save_addr(const esp_bd_addr_t addr);
bool load_addr(esp_bd_addr_t addr);
int get_playlists_after(int startIndex, String names[], int maxFiles);
int get_playlist_items(String playlistName, uint16_t*& items);