#pragma once
#include "globals.h"

// Vehicle types
enum VehicleType {
    VEHICLE_SEDAN,
    VEHICLE_SUV,
    VEHICLE_PICKUP,
    VEHICLE_VAN,
    VEHICLE_TYPE_COUNT
};

// Vehicle stats
struct VehicleStats {
    VehicleType type;
    const char* name;
    float maxSpeed;
    float acceleration;
    float braking;
    float handling;
    float mass;
};

// Vehicle manager class
class VehicleManager {
public:
    VehicleManager();
    ~VehicleManager();
    
    // Initialize vehicle system
    void Initialize();
    
    // Spawn a vehicle at position
    int SpawnVehicle(VehicleType type, Vector3 position, float rotation);
    
    // Update all vehicles
    void Update(float deltaTime);
    
    // Draw all vehicles
    void Draw();
    
    // Player vehicle interaction
    bool TryEnterVehicle(Vector3 playerPos);
    void ExitVehicle();
    bool IsPlayerInVehicle() const { return currentVehicle != nullptr; }
    Vehicle* GetPlayerVehicle() { return currentVehicle; }
    
    // Vehicle input handling
    void HandleVehicleInput(float deltaTime, bool useController);
    
    // Get vehicle at position (for collision)
    Vehicle* GetVehicleAt(Vector3 position, float radius);
    
    // Cleanup
    void Unload();
    
private:
    std::vector<Vehicle> vehicles;
    Vehicle* currentVehicle;
    int nextVehicleId;
    VehicleStats vehicleStats[VEHICLE_TYPE_COUNT];
    
    // Initialize vehicle stats
    void InitializeVehicleStats();
    
    // Physics simulation
    void UpdateVehiclePhysics(Vehicle& vehicle, float deltaTime);
    
    // Draw individual vehicle
    void DrawVehicle(const Vehicle& vehicle);
};

// Global vehicle manager
extern VehicleManager* g_VehicleManager;

// Initialize vehicle system
void InitializeVehicleSystem();

// Cleanup vehicle system
void CleanupVehicleSystem();
