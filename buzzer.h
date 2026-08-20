#pragma once
#include <math.h>
#include "SPI.h"

extern int* notes;
extern uint32_t notesTick;
extern uint32_t notesIndex;
extern uint32_t notesSize;

void playConnected();

void playConnecting();

void playDisconnected();
