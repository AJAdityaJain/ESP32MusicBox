#include "sd_interface.h"

bool SDInit()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    return SD.begin(SD_CS_PIN);
}


int fetchPlaylistsAfter(int startIndex, String names[], int maxFiles)
{
    if (!SD.exists(PLAYLISTS_DIR))
        return 0;

    File dir = SD.open(PLAYLISTS_DIR);
    if (!dir || !dir.isDirectory())
        return 0;

    int index = 0;
    int found = 0;
    File entry = dir.openNextFile();
    while (entry && found < maxFiles)
    {
        if (!entry.isDirectory())
        {
            if (index >= startIndex)
            {
                names[found++] = String(entry.name());
            }
            index++;
        }
        entry.close();
        entry = dir.openNextFile();
    }

    dir.close();
    return found;
}


int fetchPlaylistItems(String playlistName, uint16_t*& items){

    String path = String(PLAYLISTS_DIR) + "/" + playlistName;
    File f = SD.open(path, FILE_READ);
    if (!f)
        return 0;

    int count = f.size() / sizeof(uint16_t);
    items = (uint16_t*)malloc(count * sizeof(uint16_t));
    if (!items)
    {
        f.close();
        return 0;
    }

    for (int i = 0; i < count; i++)
    {
        items[i] = f.read();
    }
    f.close();
    return count;
}

bool fetchMp3FromIndex(File& f, uint16_t index)
{
    if (f)
    {
        f.close();
    }

    File indexFile = SD.open(INDEX_BIN, FILE_READ);
    if (!indexFile)
        return false;

    indexFile.seek(index * 8);

    uint8_t record[8] = {0};
    if (indexFile.read(record, sizeof(record)) != sizeof(record))
    {
        indexFile.close();
        return false;
    }
    indexFile.close();

    uint32_t addrOffset = (uint32_t)record[0] |
                          ((uint32_t)record[1] << 8) |
                          ((uint32_t)record[2] << 16) |
                          ((uint32_t)record[3] << 24);

    File addrFile = SD.open(ADDR_BIN, FILE_READ);
    if (!addrFile)
        return false;

    if (!addrFile.seek(addrOffset))
    {
        addrFile.close();
        return false;
    }

    String line;
    int slashCount = 0;
    while (addrFile.available())
    {
        char c = addrFile.read();
        if (c == '/')
        {
            slashCount++;
            if (slashCount == 3)
                break;
        }
        line += c;
    }
    addrFile.close();

    if (line.length() == 0)
        return false;

    if (!line.startsWith("/"))
        return false;

    f = SD.open(line + ".mp3", FILE_READ);
    return (bool)f;
}