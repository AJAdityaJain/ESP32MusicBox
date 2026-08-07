#pragma once
#include "SD.h"
#include <nvs_flash.h>


#define SD_CS_PIN       5
#define PLAYLISTS_DIR   "/.playlists"
#define INDEX_BIN       "/.lira/index.bin"
#define ADDR_BIN        "/.lira/addr.bin"

bool SDInit();

int fetchPlaylistsAfter(int startIndex, String names[], int maxFiles);
int fetchArtistsAfter(int startIndex, String names[], int maxFiles);
int fetchPlaylistItems(String playlistName, uint16_t*& items);
bool fetchMp3FromIndex(File& f, uint16_t index);