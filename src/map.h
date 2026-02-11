#pragma once
#include "globals.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#ifndef MAP_CONSTANTS
#define MAP_CONSTANTS

const int menuX = 50;
const int menuY = 50;
const int menuW = 400;
const int mapX = 50;
const int mapY = 50;

// Forward Declarations
enum ModelID : int;
enum TextureID : int;
struct Player;
struct MeshRenderer;  // FIXED: Changed from 'class' to 'struct' to match rendering.h
class UpgradedMaterial;
class UpgradedMapRenderer;

#include "rendering.h"

// Enums
enum BuildingType {
    BTYPE_UNKNOWN = 0, BTYPE_LABORATORY, BTYPE_HOUSE, BTYPE_APARTMENT,
    BTYPE_OFFICE, BTYPE_WAREHOUSE, BTYPE_FACTORY, BTYPE_HOSPITAL,
    BTYPE_SCHOOL, BTYPE_POLICE_STATION, BTYPE_FIRE_STATION,
    BTYPE_GROCERY_STORE, BTYPE_GAS_STATION, BTYPE_PARKING_GARAGE
};

enum RoomType {
    ROOM_HALLWAY, ROOM_OFFICE, ROOM_BEDROOM, ROOM_BATHROOM, ROOM_KITCHEN,
    ROOM_LIVING_ROOM, ROOM_STORAGE, ROOM_LAB, ROOM_MEDICAL, ROOM_SERVER,
    ROOM_GENERATOR, ROOM_PARKING, ROOM_LOBBY, ROOM_STAIRWELL,
    ROOM_ELEVATOR_SHAFT, ROOM_TYPE_COUNT
};

enum WorldTile : int {
    WT_EMPTY = 0, WT_WATER, WT_GRASS, WT_ROAD, WT_CONCRETE,
    WT_BUILDING_FOOTPRINT, WT_SUBURB, WT_FARMLAND
};

enum InteriorTile : int {
    IT_EMPTY = 0, IT_FLOOR, IT_WALL, IT_DOOR, IT_WINDOW, IT_BED, IT_DESK,
    IT_SHELF, IT_CRATE, IT_STOVE, IT_TOILET, IT_LOCKER, IT_MEDCABINET,
    IT_ARMORRACK, IT_TABLE, IT_CHAIR, IT_CONSOLE, IT_PIPE, IT_CRYOPOD_BROKEN,
    IT_CRYOPOD_INTACT, IT_VENT, IT_SERVER_RACK, IT_FRIDGE, IT_CABINET,
    IT_BENCH, IT_BROKEN_GLASS, IT_WARNING_LIGHT, IT_COOLANT_PUDDLE,
    IT_STAIRS_UP, IT_STAIRS_DOWN, IT_ELEVATOR, IT_RUBBLE
};

// Structures
struct Room {
    RoomType type;
    int x, y, width, height;
    std::vector<Vector3> furniturePositions;
    std::vector<int> furnitureTypes;
    bool hasWindows;
    bool isDamaged;
    float debrisAmount;
};

struct Floor {
    int floorNumber;
    int width, height;
    std::vector<int> tiles;
    std::vector<Room> rooms;
    std::vector<Vector3> stairPositions;
    std::vector<Vector3> elevatorPositions;
    int playerSpawnX, playerSpawnY;
};

struct Interior {
    int width;
    int height;
    std::string id;
    std::vector<int> tiles;
    std::vector<Floor> floors;
};

struct Door {
    Vector3 position;
    bool isInteriorDoor;
    bool isLocked;
    int buildingId;
};

struct Building {
    Rectangle footprint;
    int entranceX, entranceY;
};

struct MapData {
    int width;
    int height;
    std::vector<int> tiles;
    std::vector<Building> buildings;
};

struct MapPlayerState {
    Vector3 position;
    float yaw;
    bool isInside;
    int currentBuildingId;
    int currentFloor;
    bool insideInterior;
    std::string currentInteriorId;
    int worldX;
    int worldY;
    int interiorX;
    int interiorY;

    // FIXED: Initialize all members including position
    MapPlayerState() : position({0.0f, 0.0f, 0.0f}), yaw(0), isInside(false), 
                       currentBuildingId(0), currentFloor(0),
                       insideInterior(false), currentInteriorId(""), 
                       worldX(0), worldY(0), interiorX(0), interiorY(0) {}
};

// Global State Declarations
extern std::vector<Door> doors;
extern MapData g_MapData;

// Map Functions
void GenerateMapData(MapData& m);
void InitializePlayerFromMapStart(MapData& m, MapPlayerState& p);
bool EnterInterior(MapData& m, MapPlayerState& p, int buildingId);
bool ExitInterior(MapData& m, MapPlayerState& p);
const Interior* GetInterior(const MapData& m, const std::string& id);
void GenerateMap(char map[MAP_SIZE][MAP_SIZE]);
void DrawMapMenu(int screenW, int screenH, char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw);  // FIXED: Function declaration present
void UpdateDoors(float deltaTime);
Door* GetNearestDoor(Vector3 playerPos, float maxDistance);

// The Renderer Class
class UpgradedMapRenderer {
public:
    UpgradedMapRenderer();
    ~UpgradedMapRenderer();
    void Initialize();
    void GenerateWorldGeometry(const MapData& mapData);
    void GenerateInteriorGeometry(const Interior& interior);
    void Update(float deltaTime, const Camera3D& camera);
    void Cleanup();
    std::vector<MeshRenderer*> GetActiveRenderers();
    void UpdateDoor(int doorId, float openProgress);
    void AddProp(Vector3 position, ModelID modelId, float scale = 1.0f);
    void RemoveProp(int propId);
    void GenerateEnhancedWorld();
    
    std::vector<std::unique_ptr<MeshRenderer>>& GetWorldRenderers() {
        return worldRenderers;
    }

private:
    std::vector<std::unique_ptr<MeshRenderer>> worldRenderers;
    std::vector<std::unique_ptr<MeshRenderer>> interiorRenderers;
    std::vector<std::unique_ptr<MeshRenderer>> propRenderers;
    std::vector<std::unique_ptr<MeshRenderer>> doorRenderers;
    std::shared_ptr<UpgradedMaterial> wallMaterial, floorMaterial, doorMaterial,
        concreteMaterial, grassMaterial, waterMaterial, glassMaterial;

    // Terrain and world generation helpers
    void CreateTerrainLayer();
    void CreateBiomes();
    void CreateRoadNetwork();
    void CreateBuildingsClusteredByZone();
    void CreateWaterFeatures();
    void CreateDecorativeElements();

    void CreateWall(Vector3 position, TextureID texture);
    void CreateFloor(Vector3 position, float size, TextureID texture);
    void CreateBuilding(const Building& building);
    void CreateDoor(const Door& door);
    void CreateWaterTile(Vector3 position);
    void CreateRoadTile(Vector3 position);
    void InitializeMaterials();
    std::shared_ptr<UpgradedMaterial> CreateOpaqueMaterial(TextureID texture, Color tint);
    std::shared_ptr<UpgradedMaterial> CreateTransparentMaterial(TextureID texture, Color tint);
    void UpdateVisibility(const Camera3D& camera);
    
    int nextPropId;
};

extern UpgradedMapRenderer* g_UpgradedMapRenderer;
#endif // MAP_CONSTANTS