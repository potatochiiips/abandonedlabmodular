#include "globals.h"
#include "map.h"
#include <cstdlib>

// Stub implementations for missing functions
void GenerateMapData(MapData & m) {
    m.width = MAP_SIZE;
    m.height = MAP_SIZE;
    m.tiles.resize(MAP_SIZE * MAP_SIZE, WT_GRASS);
}

void InitializePlayerFromMapStart(MapData& m, MapPlayerState& p) {
    p.worldX = MAP_SIZE / 2;
    p.worldY = MAP_SIZE / 2;
    p.interiorX = 5;
    p.interiorY = 5;
    p.insideInterior = false;
}

bool EnterInterior(MapData& m, MapPlayerState& p, int buildingId) {
    p.insideInterior = true;
    p.currentBuildingId = buildingId;
    return true;
}

bool ExitInterior(MapData& m, MapPlayerState& p) {
    p.insideInterior = false;
    return true;
}

const Interior* GetInterior(const MapData& m, const std::string& id) {
    static Interior dummyInterior;
    dummyInterior.width = 20;
    dummyInterior.height = 20;
    dummyInterior.id = id;
    dummyInterior.tiles.resize(400, IT_FLOOR);
    return &dummyInterior;
}

void GenerateMap(char map[MAP_SIZE][MAP_SIZE]) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            map[y][x] = 1; // Floor
        }
    }
}

void DrawMapMenu(int screenW, int screenH, char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw) {
    int menuW = 800;
    int menuH = 600;
    int menuX = (screenW - menuW) / 2;
    int menuY = (screenH - menuH) / 2;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);
    DrawText("MAP", menuX + 20, menuY + 20, 24, PIPBOY_GREEN);


    DrawRectangle(mapX, mapY, mapSize, mapSize, Color{ 20, 40, 20, 255 });

    // Draw player position
    int playerMapX = mapX + (int)((playerPos.x / MAP_SIZE) * mapSize);
    int playerMapY = mapY + (int)((playerPos.z / MAP_SIZE) * mapSize);
    DrawCircle(playerMapX, playerMapY, 5, PIPBOY_GREEN);
}

void UpdateDoors(float deltaTime) {
    // Stub for door animation
}

Door* GetNearestDoor(Vector3 playerPos, float maxDistance) {
    for (auto& door : doors) {
        float dist = Vector3Distance(playerPos, door.position);
        if (dist < maxDistance) {
            return &door;
        }
    }
    return nullptr;
}