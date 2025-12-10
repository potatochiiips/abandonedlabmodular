#pragma once
#include "globals.h"

// New map system constants
#define WORLD_TILE_PIXELS 8
#define INTERIOR_TILE_PIXELS 4

// World tile enums
enum WorldTile : int {
    WT_EMPTY = 0,
    WT_WATER,
    WT_GRASS,
    WT_ROAD,
    WT_CONCRETE,
    WT_BUILDING_FOOTPRINT,
    WT_SUBURB,
    WT_FARMLAND
};

// Interior tile enums
enum InteriorTile : int {
    IT_EMPTY = 0,
    IT_FLOOR,
    IT_WALL,
    IT_DOOR,
    IT_WINDOW,
    IT_BED,
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
    IT_CONSOLE,
    IT_PIPE,
    IT_CRYOPOD_BROKEN,
    IT_CRYOPOD_INTACT,
    IT_VENT,
    IT_SERVER_RACK,
    IT_FRIDGE,
    IT_CABINET,
    IT_BENCH,
    IT_BROKEN_GLASS,
    IT_WARNING_LIGHT,
    IT_FLOOR_DRAIN,
    IT_COOLANT_PUDDLE,
    IT_SYRINGE,
    IT_VIAL,
    IT_CANISTER
};

// Building types
enum BuildingType {
    BTYPE_HOUSE,
    BTYPE_STORE,
    BTYPE_OFFICE,
    BTYPE_HOSPITAL,
    BTYPE_MILITARY_BASE,
    BTYPE_FARMHOUSE,
    BTYPE_LABORATORY
};

// Basic structures
struct Rect { int x, y, w, h; };

struct ItemSpawn {
    int x, y;
    std::string item;
};

struct Interior {
    int width;
    int height;
    std::vector<int> tiles; // InteriorTile values
    std::vector<ItemSpawn> spawns;
    std::string id;
    int playerSpawnX = -1;
    int playerSpawnY = -1;
};

struct Building {
    Rect footprint;
    BuildingType type;
    std::string interiorId;
    int entranceX, entranceY;
    int id;
    bool locked = false;
};

struct MapData {
    int width;
    int height;
    std::vector<int> tiles; // WorldTile values
    std::vector<Building> buildings;
    std::unordered_map<std::string, Interior> interiors;
    bool startInsideInterior = false;
    std::string startInteriorId;
};

struct Player {
    int worldX = 0;
    int worldY = 0;
    int interiorX = 0;
    int interiorY = 0;
    bool insideInterior = false;
    std::string currentInteriorId;
    int currentBuildingId = 0;
};

// Global map data
extern MapData g_MapData;
extern Player g_MapPlayer;

// Door structure
struct Door {
    Vector3 position;
    bool isOpen;
    float openAmount;
    int targetFloor;
    Vector3 targetPosition;
    bool isStairs;
};

// Global building and door management
extern std::vector<Door> doors;
extern int currentFloor;
extern int currentBuildingIndex;

// Map generation functions
void GenerateMapData(MapData& m);
void InitializePlayerFromMapStart(MapData& m, Player& p);
bool EnterInterior(MapData& m, Player& p, int buildingId);
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