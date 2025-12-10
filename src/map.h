#pragma once
#include "globals.h"
#include <unordered_map>

// Forward declaration
struct Player;

// New map system constants
#define WORLD_TILE_PIXELS 8
#define INTERIOR_TILE_PIXELS 4

// Define MAP_WIDTH/HEIGHT for map.cpp legacy functions
#define MAP_WIDTH MAP_SIZE
#define MAP_HEIGHT MAP_SIZE

// Wall heights
#define WALL_HEIGHT 3.0f
#define DOOR_HEIGHT 2.5f
#define CEILING_HEIGHT 3.0f

// Building Type enum
enum BuildingType {
    BTYPE_UNKNOWN = 0,
    BTYPE_LABORATORY,
    BTYPE_HOUSE
};

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
    IT_COOLANT_PUDDLE
};

// Item spawn structure
struct ItemSpawn {
    int x;
    int y;
    std::string itemType;
};

// Interior structure
struct Interior {
    int width;
    int height;
    std::string id;
    std::vector<int> tiles;
    std::vector<ItemSpawn> spawns;
    int playerSpawnX;
    int playerSpawnY;
    int doorX; // Door position in interior coordinates
    int doorY;

    Interior() : width(0), height(0), playerSpawnX(-1), playerSpawnY(-1), doorX(-1), doorY(-1) {}
};

// Building footprint rect
struct BuildingRect {
    int x, y, w, h;
};

// Door structure
struct Door {
    Vector3 position;
    Vector3 normal; // Direction door faces
    bool isOpen;
    float openProgress;
    bool isLocked;
    int requiredKeyId;
    int buildingId; // Which building this door belongs to (-1 for none)
    bool isInteriorDoor; // True if this is the exit door inside a building

    Door() : isOpen(false), openProgress(0.0f), isLocked(false), requiredKeyId(0), buildingId(-1), isInteriorDoor(false) {
        position = Vector3{ 0, 0, 0 };
        normal = Vector3{ 0, 0, 1 };
    }
};

// Building structure
struct Building {
    BuildingRect footprint;
    BuildingType type;
    std::string interiorId;
    int id;
    int entranceX;
    int entranceY;

    // For compatibility with main.cpp
    Vector3 position;
    int floor;

    Building() : id(0), entranceX(0), entranceY(0), floor(0) {
        position = Vector3{ 0, 0, 0 };
    }
};

// Map Data structure
struct MapData {
    int width;
    int height;
    std::vector<int> tiles;
    Texture2D tileset;
    std::unordered_map<std::string, Interior> interiors;
    std::vector<Building> buildings;
    bool startInsideInterior;
    std::string startInteriorId;

    MapData() : width(0), height(0), startInsideInterior(false) {
        tileset = { 0 };
    }
};

// Player state in map system
struct MapPlayerState {
    bool insideInterior;
    std::string currentInteriorId;
    int currentBuildingId;
    int worldX;
    int worldY;
    int interiorX;
    int interiorY;

    MapPlayerState() : insideInterior(false), currentBuildingId(0),
        worldX(0), worldY(0), interiorX(0), interiorY(0) {
    }
};

// Global map data
extern MapData g_MapData;
extern MapPlayerState g_MapPlayer;

// Global building and door management
extern std::vector<Door> doors;
extern std::vector<Building> buildings;
extern int currentFloor;
extern int currentBuildingIndex;

// Map generation functions
void GenerateMapData(MapData& m);
void InitializePlayerFromMapStart(MapData& m, MapPlayerState& p);
bool EnterInterior(MapData& m, MapPlayerState& p, int buildingId);
bool ExitInterior(MapData& m, MapPlayerState& p);
const Interior* GetInterior(const MapData& m, const std::string& id);

// Legacy compatibility functions
void GenerateMap(char map[MAP_SIZE][MAP_SIZE]);
void DrawMapMenu(int screenW, int screenH, char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw);
void DrawMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw, int minimapX, int minimapY, int minimapW, int minimapH, bool largeMap, int screenH);
void DrawMapGeometry(char map[MAP_SIZE][MAP_SIZE]);

// Enhanced world functions - NEW 3D DRAWING
void Draw3DWorld(const MapData& mapData, const MapPlayerState& playerState);
void Draw3DInterior(const Interior& interior);
void DrawDoor(const Door& door);
void UpdateDoors(float deltaTime);
Door* GetNearestDoor(Vector3 playerPos, float maxDistance);

// Collision detection
bool CheckWallCollision(Vector3 position, float radius, const MapData& mapData, const MapPlayerState& playerState);

// Frustum culling helper
struct AABB {
    Vector3 min;
    Vector3 max;
};

bool IsAABBInFrustum(const Camera3D& camera, const AABB& box);