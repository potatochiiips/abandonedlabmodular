#pragma once
#include "globals.h"

// File I/O Prototypes
bool SaveFileExists(int slotIndex);
void SaveGame(int slotIndex, Vector3 pos, float yaw, float pitch, float hp, float stam, float hung, float thirst, InventorySlot* inv, float batt, bool lightOn, char map[MAP_SIZE][MAP_SIZE], float fov);
bool LoadGame(int slotIndex, Vector3* pos, float* yaw, float* pitch, float* hp, float* stam, float* hung, float* thirst, InventorySlot* inv, float* batt, bool* lightOn, char map[MAP_SIZE][MAP_SIZE], float* fov);