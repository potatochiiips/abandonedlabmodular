#pragma once
#include "globals.h"
#include <unordered_map>
#include <player.h>

// New map system constants
#define WORLD_TILE_PIXELS 8
#define INTERIOR_TILE_PIXELS 4

// Define MAP_WIDTH/HEIGHT for map.cpp legacy functions
#define MAP_WIDTH MAP_SIZE
#define MAP_HEIGHT MAP_SIZE

// Building Type enum (Needed for map.cpp)
enum BuildingType {
    BTYPE_UNKNOWN = 0,
    BTYPE_LABORATORY, // ADDED: Used in map.cpp
    BTYPE_HOUSE       // ADDED: Used in map.cpp
};

// World tile enums
enum WorldTile : int {
    WT_EMPTY = 0,
    WT_WATER,
    WT_GRASS,
    WT_ROAD, // ADDED: Used in map.cpp
    WT_CONCRETE,
    WT_BUILDING_FOOTPRINT, // ADDED: Used in map.cpp
    WT_SUBURB,
    WT_FARMLAND
};

// Interior tile enums
enum InteriorTile : int {
    IT_EMPTY = 0,
    IT_FLOOR, // ADDED: Used in map.cpp
    IT_WALL,  // ADDED: Used in map.cpp
    IT_DOOR,  // ADDED: Used in map.cpp
    IT_WINDOW,
    IT_BED,   // ADDED: Used in map.cpp
    IT_DESK,
    IT_SHELF,
    IT_CRATE,
    IT_STOVE,
    IT_TOILET,
    IT_LOCKER,
    IT_MEDCABINET,
    IT_ARMORRACK,
    IT_TABLE,
    IT_CHAIR,
    IT_CONSOLE, // ADDED: Used in map.cpp
    IT_PIPE,
    IT_CRYOPOD_BROKEN, // ADDED: Used in map.cpp
    IT_CRYOPOD_INTACT,
    IT_VENT,
    IT_SERVER_RACK,
    IT_FRIDGE,
    IT_CABINET,
    IT_BENCH, // ADDED: Used in map.cpp
    IT_BROKEN_GLASS, // ADDED: Used in map.cpp
    IT_WARNING_LIGHT,
    IT_COOLANT_PUDDLE // ADDED: Used in map.cpp
};

// Interior structure
struct Interior {
    // ADDED MISSING MEMBERS: Fixes errors like 'width' is not a member of 'Interior'
    int width;
    int height;
    std::string id;
    std::vector<int> tiles;
    std::vector<Vector3> spawns;
    float playerSpawnX;
    float playerSpawnY;
};

// Building structure for main.cpp logic
struct Building {
    Vector3 position; // Entrance position (used for Vector3Distance)
    int floor;        // Ground floor = 0
    std::string interiorId;
    // ADDED MISSING MEMBERS: Fixes errors like 'footprint' is not a member of 'Building'
    std::vector<Vector3> footprint;
    BuildingType type;
    int id;
    int entranceX;
    int entranceY;
};

// Map Data structure
struct MapData {
    int width;
    int height;
    std::vector<int> tiles;
    Texture2D tileset;
    std::unordered_map<std::string, Interior> interiors;
    bool insideInterior = false;
    std::string currentInteriorId;
    int currentBuildingId = 0;
    // ADDED MISSING MEMBERS: Fixes errors like 'startInsideInterior' is not a member of 'MapData'
    bool startInsideInterior = false;
    std::string startInteriorId = "";
    // ADDED MISSING MEMBERS: Fixes errors like 'buildings' is not a member of 'MapData'
    std::vector<Building> buildings;
};
struct MapPlayerState {
    Vector3 worldPos;
    Vector3 interiorPos;
    int currentFloor;
    bool insideInterior = false;
    std::string currentInteriorId;
    int currentBuildingId = 0;
};

// Global map data
extern MapData g_MapData;
extern MapPlayerState g_MapPlayer;
// Door structure
struct Door {
    // ... (rest of Door struct) ...
};

// Global building and door management
extern std::vector<Door> doors;
extern std::vector<Building> buildings; // Now declared in map.h
extern int currentFloor;
extern int currentBuildingIndex;

// Map generation functions
void GenerateMapData(MapData& m);
// FIX: Replaced Player& p with the concrete struct name now that it's defined
void InitializePlayerFromMapStart(MapData& m, Player& p);
// FIX: Replaced Player& p with the concrete struct name now that it's defined
bool EnterInterior(MapData& m, Player& p, int buildingId);
// FIX: Replaced Player& p with the concrete struct name now that it's defined
bool ExitInterior(MapData& m, Player& p);
const Interior* GetInterior(const MapData& m, const std::string& id);


// Legacy compatibility functions
void GenerateMap(char map[MAP_SIZE][MAP_SIZE]);
void DrawMapMenu(int screenW, int screenH, char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw);
void DrawMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw, int minimapX, int minimapY, int minimapW, int minimapH, bool largeMap, int screenH);
void DrawMapGeometry(char map[MAP_SIZE][MAP_SIZE]);

// Enhanced world functions
void DrawDoor(const Door& door);
void UpdateDoors(float deltaTime);
Door* GetNearestDoor(Vector3 playerPos, float maxDistance);

// Frustum culling helper
struct AABB {
    Vector3 min;
    Vector3 max;
};

bool IsAABBInFrustum(const Camera3D& camera, const AABB& box);