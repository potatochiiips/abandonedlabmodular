#pragma once
#include "globals.h"

void GenerateMap(char map[MAP_SIZE][MAP_SIZE]);
void DrawMapMenu(int screenW, int screenH, char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw);
void DrawMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw, int scale, int minimapX, int minimapY, int screenW, bool largeMap, int screenH);

// Helper function assumed to be used in main.cpp for drawing the 3D map
void DrawMapGeometry(char map[MAP_SIZE][MAP_SIZE]);