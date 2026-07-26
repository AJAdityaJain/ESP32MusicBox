#include "sd_interface.h"

bool init_sd()
{
    return SD.begin(SD_CS_PIN);
}


    // esp_bd_addr_t *addr_ptr = a2dp_source.get_last_peer_address();
    // memcpy(paired_addr, *addr_ptr, sizeof(esp_bd_addr_t));
    // has_paired_addr = save_addr(paired_addr);
    // if (has_paired_addr)
    // a2dp_source.connect_to(paired_addr);

bool save_addr(const esp_bd_addr_t addr)
{
    if (!(addr[0] && addr[1] && addr[2] && addr[3] && addr[4] && addr[5]))
        return false;

    File f = SD.open(ADDR_FILE, FILE_WRITE);
    if (!f)
    {
        Serial.println("[SD] Failed to open for write");
        return false;
    }

    for (int i = 0; i < 6; i++)
    {
        f.printf("%02X", addr[i]);
        if (i < 5)
            f.print(":");
    }
    f.close();
    Serial.printf("[SD] Saved addr: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    return true;
}

bool load_addr(esp_bd_addr_t addr)
{
    if (!SD.exists(ADDR_FILE))
        return false;

    File f = SD.open(ADDR_FILE, FILE_READ);
    if (!f)
        return false;
    String line = f.readStringUntil('\n');
    f.close();

    line.trim();
    if (line.length() < 17)
        return false;

    for (int i = 0; i < 6; i++)
    {
        addr[i] = (uint8_t)strtol(line.substring(i * 3, i * 3 + 2).c_str(), NULL, 16);
    }

    Serial.printf("[SD] Loaded addr: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    if (!(addr[0] && addr[1] && addr[2] && addr[3] && addr[4] && addr[5]))
        return false;

    return true;
}

int get_playlists_after(int startIndex, String names[], int maxFiles)
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


int get_playlist_items(String playlistName, uint16_t*& items){
    //The file path is the PLAYLISTS_DIR + "/" + playlistName and it contains a list of uint16_t values, it will have no seperators.

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

