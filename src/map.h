#pragma once
#include "globals.h"
#include "rendering.h"
#include <vector>
#include <memory>
#include <unordered_map>

class UpgradedMapRenderer {
public:
    UpgradedMapRenderer();
    ~UpgradedMapRenderer();
    
    void Initialize();
    void GenerateWorldGeometry(const MapData& mapData);
    void GenerateInteriorGeometry(const Interior& interior);
    void Update(float deltaTime, const Camera3D& camera);
    void Cleanup();
    
    // Get all renderers for the pipeline
    std::vector<MeshRenderer*> GetActiveRenderers();
    
    // Dynamic updates
    void UpdateDoor(int doorId, float openProgress);
    void AddProp(Vector3 position, ModelID modelId, float scale = 1.0f);
    void RemoveProp(int propId);
    
private:
    // Render components
    std::vector<std::unique_ptr<MeshRenderer>> worldRenderers;
    std::vector<std::unique_ptr<MeshRenderer>> interiorRenderers;
    std::vector<std::unique_ptr<MeshRenderer>> propRenderers;
    std::vector<std::unique_ptr<MeshRenderer>> doorRenderers;
    
    // Materials cache
    std::shared_ptr<UpgradedMaterial> wallMaterial;
    std::shared_ptr<UpgradedMaterial> floorMaterial;
    std::shared_ptr<UpgradedMaterial> doorMaterial;
    std::shared_ptr<UpgradedMaterial> concreteMaterial;
    std::shared_ptr<UpgradedMaterial> grassMaterial;
    std::shared_ptr<UpgradedMaterial> waterMaterial;
    std::shared_ptr<UpgradedMaterial> glassMaterial;
    
    // Geometry generation
    void CreateWall(Vector3 position, TextureID texture);
    void CreateFloor(Vector3 position, float size, TextureID texture);
    void CreateBuilding(const Building& building);
    void CreateDoor(const Door& door);
    void CreateWaterTile(Vector3 position);
    void CreateRoadTile(Vector3 position);
    
    // Material creation
    void InitializeMaterials();
    std::shared_ptr<UpgradedMaterial> CreateOpaqueMaterial(TextureID texture, Color tint);
    std::shared_ptr<UpgradedMaterial> CreateTransparentMaterial(TextureID texture, Color tint);
    
    // Culling and LOD
    void UpdateVisibility(const Camera3D& camera);
    
    int nextPropId;
};
// Forward declaration
struct Player;

// Define MAP_WIDTH/HEIGHT for map.cpp
#define MAP_WIDTH MAP_SIZE
#define MAP_HEIGHT MAP_SIZE

// Wall heights
#define WALL_HEIGHT 3.0f
#define DOOR_HEIGHT 2.5f
#define CEILING_HEIGHT 3.0f
#define FLOOR_HEIGHT 4.0f  // Distance between floors

// Building Type enum
enum BuildingType {
    BTYPE_UNKNOWN = 0,
    BTYPE_LABORATORY,
    BTYPE_HOUSE,
    BTYPE_APARTMENT,
    BTYPE_OFFICE,
    BTYPE_WAREHOUSE,
    BTYPE_FACTORY,
    BTYPE_HOSPITAL,
    BTYPE_SCHOOL,
    BTYPE_POLICE_STATION,
    BTYPE_FIRE_STATION,
    BTYPE_GROCERY_STORE,
    BTYPE_GAS_STATION,
    BTYPE_PARKING_GARAGE
};

// Room types for interiors
enum RoomType {
    ROOM_HALLWAY,
    ROOM_OFFICE,
    ROOM_BEDROOM,
    ROOM_BATHROOM,
    ROOM_KITCHEN,
    ROOM_LIVING_ROOM,
    ROOM_STORAGE,
    ROOM_LAB,
    ROOM_MEDICAL,
    ROOM_SERVER,
    ROOM_GENERATOR,
    ROOM_PARKING,
    ROOM_LOBBY,
    ROOM_STAIRWELL,
    ROOM_ELEVATOR_SHAFT,
    ROOM_TYPE_COUNT
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
    IT_COOLANT_PUDDLE,
    IT_STAIRS_UP,
    IT_STAIRS_DOWN,
    IT_ELEVATOR,
    IT_RUBBLE
};
// Room definition for building interiors
struct Room {
    RoomType type;
    int x, y, width, height;
    std::vector<Vector3> furniturePositions;
    std::vector<int> furnitureTypes;
    bool hasWindows;
    bool isDamaged;
    float debrisAmount; // 0.0 to 1.0
};

// Floor definition
struct Floor {
    int floorNumber;
    int width, height;
    std::vector<int> tiles;
    std::vector<Room> rooms;
    std::vector<Vector3> stairPositions;
    std::vector<Vector3> elevatorPositions;
    int playerSpawnX, playerSpawnY;
};