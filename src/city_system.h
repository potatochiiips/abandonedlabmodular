#pragma once
#include "globals.h"

// City district types
enum DistrictType {
    DISTRICT_DOWNTOWN,      // Tall buildings, skyscrapers
    DISTRICT_RESIDENTIAL,   // Houses, apartments
    DISTRICT_INDUSTRIAL,    // Warehouses, factories
    DISTRICT_COMMERCIAL,    // Shops, offices
    DISTRICT_SUBURBAN,      // Spread out houses
    DISTRICT_TYPE_COUNT
};

// City zone definition
struct CityZone {
    int x, z;              // Grid position
    int width, height;     // Size in grid cells
    DistrictType type;
    float destructionLevel; // 0.0 to 1.0
    std::vector<int> buildingIds;
};

// Detailed building with full interior
struct DetailedBuilding {
    int id;
    Vector3 position;
    float rotation;
    int width, depth, floors;
    BuildingDamageLevel damageLevel;
    int exteriorModelId;
    
    // Interior data
    bool hasInterior;
    std::vector<std::vector<int>> floorLayouts; // [floor][roomIndex]
    std::vector<Vector3> stairPositions;
    std::vector<Vector3> elevatorPositions;
    std::vector<DebrisInstance> interiorDebris;
    
    // Exterior details
    bool hasWindows;
    bool hasFire;
    bool hasGraffiti;
    std::vector<Vector3> windowPositions;
    std::vector<DebrisInstance> exteriorDebris;
};

// City manager
class CityManager {
public:
    CityManager();
    ~CityManager();
    
    // Initialize city system
    void Initialize(unsigned int seed = 54321);
    
    // Generate destroyed cities
    void GenerateCities(TerrainManager* terrain);
    
    // Update visible buildings
    void Update(const Camera3D& camera);
    
    // Draw visible city elements
    void Draw(const Camera3D& camera);
    
    // Get building at position (for entry)
    DetailedBuilding* GetBuildingAt(Vector3 position, float radius);
    
    // Check if position is inside building interior
    bool IsInsideBuilding(Vector3 position, DetailedBuilding** outBuilding);
    
    // Cleanup
    void Unload();
    
private:
    std::vector<CityZone> zones;
    std::vector<DetailedBuilding> buildings;
    std::vector<DebrisInstance> streetDebris;
    unsigned int citySeed;
    
    // Generate city zone
    void GenerateZone(const CityZone& zone, TerrainManager* terrain);
    
    // Generate single building with interior
    DetailedBuilding GenerateBuilding(Vector3 position, DistrictType district, float destruction);
    
    // Generate building interior
    void GenerateBuildingInterior(DetailedBuilding& building);
    
    // Generate floor layout
    std::vector<int> GenerateFloorLayout(int width, int depth, int floor, bool isTopFloor);
    
    // Place debris around destroyed areas
    void PlaceDebris(const CityZone& zone);
    
    // Draw building exterior
    void DrawBuildingExterior(const DetailedBuilding& building);
    
    // Draw building interior (if player inside)
    void DrawBuildingInterior(const DetailedBuilding& building, int currentFloor);
    
    // Draw debris
    void DrawDebris(const DebrisInstance& debris);
};

// Global city manager
extern CityManager* g_CityManager;

// Initialize city system
void InitializeCitySystem(unsigned int seed = 54321);

// Cleanup city system
void CleanupCitySystem();
