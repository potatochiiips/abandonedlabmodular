#include "vehicle_system.h"
#include "model_manager.h"
#include "raylib.h"
#include "rlgl.h"
#include <globals.h>
#include <animation_system.h>

// Global instance
VehicleManager* g_VehicleManager = nullptr;

VehicleManager::VehicleManager() {
    currentVehicle = nullptr;
    nextVehicleId = 1;
}

VehicleManager::~VehicleManager() {
    Unload();
}

void VehicleManager::Initialize() {
    TraceLog(LOG_INFO, "Initializing Vehicle Manager...");
    InitializeVehicleStats();
    TraceLog(LOG_INFO, "Vehicle Manager initialized");
}

void VehicleManager::InitializeVehicleStats() {
    vehicleStats[VEHICLE_SEDAN] = {
        VEHICLE_SEDAN,
        "Sedan",
        30.0f,   // maxSpeed
        8.0f,    // acceleration
        12.0f,   // braking
        0.8f,    // handling
        1200.0f  // mass
    };
    
    vehicleStats[VEHICLE_SUV] = {
        VEHICLE_SUV,
        "SUV",
        25.0f,
        6.0f,
        10.0f,
        0.6f,
        1800.0f
    };
    
    vehicleStats[VEHICLE_PICKUP] = {
        VEHICLE_PICKUP,
        "Pickup",
        28.0f,
        7.0f,
        11.0f,
        0.7f,
        1500.0f
    };
    
    vehicleStats[VEHICLE_VAN] = {
        VEHICLE_VAN,
        "Van",
        22.0f,
        5.0f,
        9.0f,
        0.5f,
        2000.0f
    };
}

int VehicleManager::SpawnVehicle(VehicleType type, Vector3 position, float rotation) {
    Vehicle v;
    v.id = nextVehicleId++;
    v.position = position;
    v.velocity = Vector3{0, 0, 0};
    v.rotation = rotation;
    v.speed = 0.0f;
    v.maxSpeed = vehicleStats[type].maxSpeed;
    v.acceleration = vehicleStats[type].acceleration;
    v.braking = vehicleStats[type].braking;
    v.handling = vehicleStats[type].handling;
    v.modelId = (int)type;
    v.isPlayerInside = false;
    v.wheelRotation = 0.0f;
    v.steeringAngle = 0.0f;
    
    vehicles.push_back(v);
    return v.id;
}

void VehicleManager::Update(float deltaTime) {
    for (auto& vehicle : vehicles) {
        if (!vehicle.isPlayerInside) {
            // Decay speed when not driven
            vehicle.speed *= 0.95f;
            if (fabsf(vehicle.speed) < 0.1f) vehicle.speed = 0.0f;
        }
        
        UpdateVehiclePhysics(vehicle, deltaTime);
    }
}

void VehicleManager::UpdateVehiclePhysics(Vehicle& vehicle, float deltaTime) {
    // Update velocity based on rotation and speed
    float radians = vehicle.rotation * DEG2RAD;
    Vector3 forward = {sinf(radians), 0.0f, cosf(radians)};
    
    vehicle.velocity = Vector3Scale(forward, vehicle.speed);
    vehicle.position = Vector3Add(vehicle.position, Vector3Scale(vehicle.velocity, deltaTime));
    
    // Update wheel rotation
    vehicle.wheelRotation += vehicle.speed * deltaTime * 10.0f;
    if (vehicle.wheelRotation > 360.0f) vehicle.wheelRotation -= 360.0f;
    if (vehicle.wheelRotation < -360.0f) vehicle.wheelRotation += 360.0f;
    
    // Decay steering
    vehicle.steeringAngle *= 0.9f;
}

void VehicleManager::HandleVehicleInput(float deltaTime, bool useController) {
    if (!currentVehicle) return;

    float throttle = 0.0f;
    float brake = 0.0f;
    float steer = 0.0f;

    // Keyboard/controller input
    if (useController && IsGamepadAvailable(0)) {
        throttle = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER);
        brake = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_TRIGGER);
        steer = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
    }
    else {
        if (IsKeyDown(KEY_W)) throttle = 1.0f;
        if (IsKeyDown(KEY_S)) brake = 1.0f;
        if (IsKeyDown(KEY_A)) steer = -1.0f;
        if (IsKeyDown(KEY_D)) steer = 1.0f;
    }

    // FIXED: Better acceleration with deltaTime
    if (throttle > 0.1f) {
        currentVehicle->speed += currentVehicle->acceleration * throttle * deltaTime * 10.0f; // Scale up
        currentVehicle->speed = fminf(currentVehicle->speed, currentVehicle->maxSpeed);
    }

    // FIXED: Better braking
    if (brake > 0.1f) {
        float brakePower = currentVehicle->braking * brake * deltaTime * 10.0f; // Scale up
        if (currentVehicle->speed > 0) {
            currentVehicle->speed -= brakePower;
            currentVehicle->speed = fmaxf(currentVehicle->speed, 0.0f);
        }
        else {
            // Reverse
            currentVehicle->speed += brakePower * 0.5f;
            currentVehicle->speed = fminf(currentVehicle->speed, 0.0f);
        }
    }

    // FIXED: Better friction when coasting
    if (throttle < 0.1f && brake < 0.1f) {
        currentVehicle->speed *= 0.95f; // Friction
        if (fabsf(currentVehicle->speed) < 0.1f) {
            currentVehicle->speed = 0.0f;
        }
    }

    // FIXED: Better steering
    if (fabsf(steer) > 0.1f && fabsf(currentVehicle->speed) > 0.5f) {
        currentVehicle->steeringAngle = steer * 30.0f;
        float turnRate = currentVehicle->handling * steer * deltaTime * 100.0f; // Scale up
        float speedFactor = fabsf(currentVehicle->speed) / currentVehicle->maxSpeed;
        currentVehicle->rotation += turnRate * speedFactor;
    }
    else {
        // Decay steering
        currentVehicle->steeringAngle *= 0.8f;
    }
}

bool VehicleManager::TryEnterVehicle(Vector3 playerPos) {
    Vehicle* nearest = GetVehicleAt(playerPos, 3.0f);
    if (nearest && !nearest->isPlayerInside) {
        nearest->isPlayerInside = true;
        currentVehicle = nearest;
        TraceLog(LOG_INFO, "Entered vehicle");
        return true;
    }
    return false;
}

void VehicleManager::ExitVehicle() {
    if (currentVehicle) {
        currentVehicle->isPlayerInside = false;
        currentVehicle = nullptr;
        TraceLog(LOG_INFO, "Exited vehicle");
    }
}

Vehicle* VehicleManager::GetVehicleAt(Vector3 position, float radius) {
    Vehicle* nearest = nullptr;
    float minDist = radius;
    
    for (auto& vehicle : vehicles) {
        float dist = Vector3Distance(position, vehicle.position);
        if (dist < minDist) {
            minDist = dist;
            nearest = &vehicle;
        }
    }
    
    return nearest;
}

void VehicleManager::Draw() {
    for (const auto& vehicle : vehicles) {
        DrawVehicle(vehicle);
    }
}

void VehicleManager::DrawVehicle(const Vehicle& vehicle) {
    // Draw simple vehicle body
    Vector3 bodySize = {2.0f, 1.5f, 4.0f};
    
    // Apply rotation
    rlPushMatrix();
    rlTranslatef(vehicle.position.x, vehicle.position.y + 1.0f, vehicle.position.z);
    rlRotatef(vehicle.rotation, 0, 1, 0);
    
    // Body
    DrawCube(Vector3{0, 0, 0}, bodySize.x, bodySize.y, bodySize.z, Color{100, 100, 120, 255});
    
    // Cabin
    DrawCube(Vector3{0, 0.75f, -0.5f}, 1.8f, 1.0f, 2.0f, Color{80, 80, 100, 255});
    
    // Wheels
    float wheelRadius = 0.4f;
    float wheelWidth = 0.3f;
    Vector3 wheelPositions[] = {
        {-1.0f, -0.5f, 1.5f},  // Front left
        {1.0f, -0.5f, 1.5f},   // Front right
        {-1.0f, -0.5f, -1.5f}, // Rear left
        {1.0f, -0.5f, -1.5f}   // Rear right
    };
    
    for (int i = 0; i < 4; i++) {
        rlPushMatrix();
        rlTranslatef(wheelPositions[i].x, wheelPositions[i].y, wheelPositions[i].z);
        
        // Rotate front wheels for steering
        if (i < 2) {
            rlRotatef(vehicle.steeringAngle, 0, 1, 0);
        }
        
        rlRotatef(vehicle.wheelRotation, 1, 0, 0);
        DrawCylinder(Vector3{0, 0, 0}, wheelRadius, wheelRadius, wheelWidth, 16, Color{40, 40, 45, 255});
        rlPopMatrix();
    }
    
    rlPopMatrix();
}

void VehicleManager::Unload() {
    vehicles.clear();
    currentVehicle = nullptr;
}

// Global initialization
void InitializeVehicleSystem() {
    g_VehicleManager = new VehicleManager();
    g_VehicleManager->Initialize();
    TraceLog(LOG_INFO, "Vehicle system initialized");
}

void CleanupVehicleSystem() {
    if (g_VehicleManager) {
        delete g_VehicleManager;
        g_VehicleManager = nullptr;
    }
    TraceLog(LOG_INFO, "Vehicle system cleaned up");
}
