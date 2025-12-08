#pragma once
#include "globals.h"

// Building interior structure
struct BuildingInterior {
    int floors;
    char layout[5][20][20]; // Up to 5 floors, 20x20 grid each
    Vector3 worldPos; // Position in world coordinates
    int width;
    int depth;
};

// Door structure
struct Door {
    Vector3 position;
    bool isOpen;
    float openAmount; // 0.0 to 1.0 for animation
    int targetFloor; // -1 for exterior doors, floor number for interior stairs
    Vector3 targetPosition; // Where player teleports when entering
    bool isStairs; // True if this is a staircase door
};

// Global building and door management
extern std::vector<BuildingInterior> buildingInteriors;
extern std::vector<Door> doors;
extern int currentFloor; // -1 for outside, 0+ for inside building
extern int currentBuildingIndex; // -1 for outside

void GenerateMap(char map[MAP_SIZE][MAP_SIZE]);
void DrawMapMenu(int screenW, int screenH, char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw);
void DrawMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw, int scale, int minimapX, int minimapY, int screenW, bool largeMap, int screenH);
void DrawMapGeometry(char map[MAP_SIZE][MAP_SIZE]);

// New functions for enhanced world
void GenerateBuildingInterior(BuildingInterior& building, int floors);
void DrawBuildingExterior(const BuildingInterior& building);
void DrawBuildingInterior(const BuildingInterior& building, int floor);
void DrawDoor(const Door& door);
void UpdateDoors(float deltaTime);
Door* GetNearestDoor(Vector3 playerPos, float maxDistance);
void DrawVegetation(int x, int z, char tileType);

// Frustum culling helper
struct AABB {
    Vector3 min;
    Vector3 max;
};

bool IsAABBInFrustum(const Camera3D& camera, const AABB& box);