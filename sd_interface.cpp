#include "sd_interface.h"

bool SDInit()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    return SD.begin(SD_CS_PIN, SPI, 25000000);  // 25MHz
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

int fetchArtistsAfter(int startIndex, String names[], int maxFiles){

    if (!SD.exists("/"))
        return 0;
    


    File dir = SD.open("/");
    if (!dir || !dir.isDirectory())
        return 0;

    int index = 0;
    int found = 0;
    File entry = dir.openNextFile();
    while (entry && found < maxFiles)
    {
        if (entry.isDirectory())
        {
            String name = String(entry.name());
            if (name.startsWith(".") || name.startsWith("Sys")){
                entry.close();
                entry = dir.openNextFile();
                continue;
            }

            if (index >= startIndex)
            {
                names[found++] = name;
            }
            index++;
        }
        entry.close();
        entry = dir.openNextFile();
    }

    dir.close();
    return found;
}

int fetchSongsAfter(String artistName, int startIndex, String names[], int maxFiles)
{
    String indexPath = "/" + artistName + "/index.bin";
    if (!SD.exists(indexPath))
        return 0;

    File f = SD.open(indexPath, FILE_READ);
    if (!f)        return 0;
    
    f.seek(startIndex * 4);
    int found = 0;
    while (found < maxFiles && f.available())
    {
        uint8_t bytes[4] = {0};
        if (f.read(bytes, sizeof(bytes)) != sizeof(bytes))
        {
            f.close();
            return found;
        }

        uint32_t value = ((uint32_t)bytes[0]) |
                         ((uint32_t)bytes[1] << 8) |
                         ((uint32_t)bytes[2] << 16) |
                         ((uint32_t)bytes[3] << 24);

        String songPath = getMp3AddrFromIndex((uint16_t)value);
        if (!songPath.isEmpty())
        {
            int slashIndex = songPath.lastIndexOf('/');
            String fileName = songPath.substring(slashIndex + 1);
            names[found++] = fileName;
        }
    }

    
    f.close();
    return found;
}

void queueItems(String path, uint16_tVec& items){

    File f = SD.open(path, FILE_READ);
    if (!f)return;

    size_t fileSize = f.size();
    if (fileSize % 4 != 0)
    {
        f.close();
        return;
    }

    int count = fileSize / 4;
    items.reserve(count);

    for (int i = 0; i < count; i++)
    {
        uint8_t bytes[4] = {0};
        if (f.read(bytes, sizeof(bytes)) != sizeof(bytes))
        {
            items.clear();
            f.close();
            return;
        }

        uint32_t value = ((uint32_t)bytes[0]) |
                         ((uint32_t)bytes[1] << 8) |
                         ((uint32_t)bytes[2] << 16) |
                         ((uint32_t)bytes[3] << 24);
        items.push((uint16_t)value);
        // items[i] = (uint16_t)value;
    }
    f.close();
}

String getMp3AddrFromIndex(uint16_t index)
{

    File indexFile = SD.open(INDEX_BIN, FILE_READ);
    if (!indexFile)return String();

    indexFile.seek(index * 8);

    uint8_t record[8] = {0};
    if (indexFile.read(record, sizeof(record)) != sizeof(record))
    {
        indexFile.close();
        return String();
    }
    indexFile.close();

    uint32_t addrOffset = (uint32_t)record[0] |
                          ((uint32_t)record[1] << 8) |
                          ((uint32_t)record[2] << 16) |
                          ((uint32_t)record[3] << 24);

    File addrFile = SD.open(ADDR_BIN, FILE_READ);
    if (!addrFile)
        return String();

    if (!addrFile.seek(addrOffset))
    {
        addrFile.close();
        return String();
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


    if (!line.startsWith("/"))
        return String();
    return line;
}

uint16_t getIndexFrom(String artistName, int songIndex)
{
    String indexPath = "/" + artistName + "/index.bin";
    if (!SD.exists(indexPath))
        return 0;

    File f = SD.open(indexPath, FILE_READ);
    if (!f)        return 0;

    f.seek(songIndex * 4);
    uint8_t bytes[4] = {0};
    if (f.read(bytes, sizeof(bytes)) != sizeof(bytes))
    {
        f.close();
        return 0;
    }

    uint32_t value = ((uint32_t)bytes[0]) |
                     ((uint32_t)bytes[1] << 8) |
                     ((uint32_t)bytes[2] << 16) |
                     ((uint32_t)bytes[3] << 24);

    f.close();
    return (uint16_t)value;
}

bool fetchMp3FromIndex(File& f, uint16_t index)
{
    if (f)f.close();
    f = SD.open((getMp3AddrFromIndex(index) + ".mp3"), FILE_READ);
    return (bool)f;
}