#include "globals.h"
#include "map.h"
#include "enhanced_map_system.h"
#include "model_manager.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

MapPlayerState g_MapPlayer;
MapData g_MapData;
std::vector<Door> doors;

void GenerateMapData(MapData& m) {
    m.width = 256;
    m.height = 256;
    m.tiles.resize(m.width * m.height, WT_GRASS);
    m.buildings.clear();
    
    TraceLog(LOG_INFO, "Map data cleared and resized");
}

void InitializePlayerFromMapStart(MapData& m, MapPlayerState& p) {
    p.position = { 128.0f, 1.8f, 128.0f };
    p.yaw = 0.0f;
    p.isInside = false;
    p.currentBuildingId = 0;
    p.currentFloor = 0;
    p.insideInterior = false;
    p.currentInteriorId = "";
    p.worldX = 128;
    p.worldY = 128;
    p.interiorX = 0;
    p.interiorY = 0;
    
    TraceLog(LOG_INFO, "Player initialized at world spawn");
}

bool EnterInterior(MapData& m, MapPlayerState& p, int buildingId) {
    if (g_EnhancedMapSystem) {
        const Interior* interior = g_EnhancedMapSystem->GetLabInterior();
        if (interior) {
            p.insideInterior = true;
            p.currentInteriorId = interior->id;
            p.currentBuildingId = buildingId;
            p.interiorX = 10;
            p.interiorY = 15;
            p.currentFloor = 0;
            TraceLog(LOG_INFO, "Entered interior: %s", interior->id.c_str());
            return true;
        }
    }
    return false;
}

bool ExitInterior(MapData& m, MapPlayerState& p) {
    if (p.insideInterior) {
        p.insideInterior = false;
        p.currentInteriorId = "";
        p.position = { (float)p.worldX, 1.8f, (float)p.worldY };
        TraceLog(LOG_INFO, "Exited to exterior at (%.1f, %.1f)", p.position.x, p.position.z);
        return true;
    }
    return false;
}

const Interior* GetInterior(const MapData& m, const std::string& id) {
    if (g_EnhancedMapSystem) {
        return g_EnhancedMapSystem->GetLabInterior();
    }
    return nullptr;
}

void GenerateMap(char map[MAP_SIZE][MAP_SIZE]) {
    // Initialize entire map as empty
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            map[y][x] = WT_GRASS;
        }
    }
    TraceLog(LOG_INFO, "Base map generated");
}

void UpdateDoors(float deltaTime) {
    // Door animation updates would go here
}

Door* GetNearestDoor(Vector3 playerPos, float maxDistance) {
    Door* nearest = nullptr;
    float nearestDist = maxDistance;
    
    for (auto& door : doors) {
        float dist = Vector3Distance(playerPos, door.position);
        if (dist < nearestDist) {
            nearestDist = dist;
            nearest = &door;
        }
    }
    
    return nearest;
}