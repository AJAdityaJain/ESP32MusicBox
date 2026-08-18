#pragma once
#include "SD.h"
#include <nvs_flash.h>


#define SD_CS_PIN       5
#define PLAYLISTS_DIR   "/.playlists"
#define INDEX_BIN       "/.lira/index.bin"
#define ADDR_BIN        "/.lira/addr.bin"

bool SDInit();
class uint16_tVec {
private:
    uint16_t* data_;
    size_t size_;
    size_t capacity_;

public:
    uint16_tVec() : data_(nullptr), size_(0), capacity_(0) {}
    ~uint16_tVec() { delete[] data_; }
    
    void reserve(size_t newCapacity) {
        Serial.printf("Reserving capacity: %zu\n", newCapacity);
        if (newCapacity <= capacity_) return;
        uint16_t* newData = new uint16_t[newCapacity];
        if (data_) memcpy(newData, data_, size_ * sizeof(uint16_t));
        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }

    void push(uint16_t value) {
        if (size_ >= capacity_) {
            reserve((capacity_ == 0) ? 16 : capacity_ * 2);
        }
        data_[size_++] = value;
        Serial.printf("Pushed value: %u, new size: %zu\n", value, size_);
    }

    void clear() {
        Serial.println("Clearing vector");
        size_ = 0;
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }

    uint16_t& operator[](size_t index) { return data_[index]; }
    const uint16_t& operator[](size_t index) const { return data_[index]; }
};
int fetchPlaylistsAfter(int startIndex, String names[], int maxFiles);
int fetchArtistsAfter(int startIndex, String names[], int maxFiles);
int fetchSongsAfter(String artistName, int startIndex, String names[], int maxFiles);
void queueItems(String path, uint16_tVec& items);
String getMp3AddrFromIndex(uint16_t index);
bool fetchMp3FromIndex(File& f, uint16_t index);
uint16_t getIndexFrom(String artistName, int songIndex);