#pragma once
#include "globals.h"
#include <vector>

// Zombie AI states
enum ZombieState {
    ZOMBIE_IDLE,
    ZOMBIE_WANDER,
    ZOMBIE_CHASE,
    ZOMBIE_ATTACK
};

// Zombie entity
struct Zombie {
    int id;
    Vector3 position;
    Vector3 velocity;
    float rotation;
    float health;
    float maxHealth;
    ZombieState state;
    float stateTimer;
    Vector3 targetPosition;
    float detectionRange;
    float attackRange;
    float moveSpeed;
    float attackDamage;
    float attackCooldown;
    bool isAlive;
};

// Zombie manager
class ZombieManager {
public:
    ZombieManager();
    ~ZombieManager();
    
    // Initialize system
    void Initialize();
    
    // Spawn zombie at position
    int SpawnZombie(Vector3 position);
    
    // Update all zombies
    void Update(float deltaTime, Vector3 playerPos);
    
    // Draw all zombies
    void Draw(const Camera3D& camera);
    
    // Damage zombie
    void DamageZombie(int zombieId, float damage);
    
    // Get zombie at position (for shooting)
    Zombie* GetZombieAt(Vector3 position, float radius);
    
    // Get all zombies
    const std::vector<Zombie>& GetZombies() const { return zombies; }
    
    // Clear all zombies
    void ClearAll();
    
    // Cleanup
    void Unload();
    
private:
    std::vector<Zombie> zombies;
    int nextZombieId;
    
    // Update individual zombie
    void UpdateZombie(Zombie& zombie, float deltaTime, Vector3 playerPos);
    
    // Update zombie AI
    void UpdateZombieAI(Zombie& zombie, Vector3 playerPos);
    
    // Draw individual zombie
    void DrawZombie(const Zombie& zombie);
    
    // Check line of sight
    bool HasLineOfSight(Vector3 from, Vector3 to);
};

// Global zombie manager
extern ZombieManager* g_ZombieManager;

// Initialize zombie system
void InitializeZombieSystem();

// Cleanup zombie system
void CleanupZombieSystem();
