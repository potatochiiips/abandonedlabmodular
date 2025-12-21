#pragma once
#include "globals.h"

void DrawHUD(int screenW, int screenH, float health, float stamina, float hunger, float thirst, float fov, float flashlightBattery, bool isFlashlightOn, InventorySlot* inventory);

// Draw round minimap with circular mask
void DrawRoundMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw, int centerX, int centerY, int radius, int viewRange);