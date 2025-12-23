#include "zombie_system.h"
#include "rlgl.h"
#include "globals.h"
#include "map.h"
#include <cstdlib>

// Global instance
ZombieManager* g_ZombieManager = nullptr;

ZombieManager::ZombieManager() {
    nextZombieId = 1;
}

ZombieManager::~ZombieManager() {
    Unload();
}

void ZombieManager::Initialize() {
    TraceLog(LOG_INFO, "Zombie system initialized");
}

int ZombieManager::SpawnZombie(Vector3 position) {
    Zombie zombie;
    zombie.id = nextZombieId++;
    zombie.position = position;
    zombie.velocity = Vector3{0, 0, 0};
    zombie.rotation = (float)(rand() % 360);
    zombie.health = 100.0f;
    zombie.maxHealth = 100.0f;
    zombie.state = ZOMBIE_IDLE;
    zombie.stateTimer = 0.0f;
    zombie.targetPosition = position;
    zombie.detectionRange = 30.0f;
    zombie.attackRange = 2.0f;
    zombie.moveSpeed = 2.0f;
    zombie.attackDamage = 10.0f;
    zombie.attackCooldown = 0.0f;
    zombie.isAlive = true;
    
    zombies.push_back(zombie);
    
    TraceLog(LOG_INFO, "Zombie spawned at (%.1f, %.1f, %.1f)", position.x, position.y, position.z);
    return zombie.id;
}

void ZombieManager::Update(float deltaTime, Vector3 playerPos) {
    for (auto it = zombies.begin(); it != zombies.end();) {
        if (!it->isAlive || it->health <= 0.0f) {
            TraceLog(LOG_INFO, "Zombie %d destroyed", it->id);
            it = zombies.erase(it);
        } else {
            UpdateZombie(*it, deltaTime, playerPos);
            ++it;
        }
    }
}

void ZombieManager::UpdateZombie(Zombie& zombie, float deltaTime, Vector3 playerPos) {
    // Update cooldowns
    if (zombie.attackCooldown > 0.0f) {
        zombie.attackCooldown -= deltaTime;
    }
    
    // Update AI
    UpdateZombieAI(zombie, playerPos);
    
    // Update movement
    if (zombie.state == ZOMBIE_WANDER || zombie.state == ZOMBIE_CHASE) {
        Vector3 direction = Vector3Subtract(zombie.targetPosition, zombie.position);
        float distance = Vector3Length(direction);
        
        if (distance > 0.5f) {
            direction = Vector3Normalize(direction);
            zombie.velocity = Vector3Scale(direction, zombie.moveSpeed);
            
            // Update rotation to face movement direction
            zombie.rotation = atan2f(direction.x, direction.z) * RAD2DEG;
            
            // Move zombie
            zombie.position = Vector3Add(zombie.position, Vector3Scale(zombie.velocity, deltaTime));
        } else {
            zombie.velocity = Vector3{0, 0, 0};
        }
    }
    
    // Keep zombie on ground
    zombie.position.y = 1.0f; // Assuming flat ground for now
}

void ZombieManager::UpdateZombieAI(Zombie& zombie, Vector3 playerPos) {
    float distanceToPlayer = Vector3Distance(zombie.position, playerPos);
    
    zombie.stateTimer -= GetFrameTime();
    
    switch (zombie.state) {
        case ZOMBIE_IDLE:
            if (distanceToPlayer < zombie.detectionRange && HasLineOfSight(zombie.position, playerPos)) {
                zombie.state = ZOMBIE_CHASE;
                zombie.targetPosition = playerPos;
                TraceLog(LOG_INFO, "Zombie %d detected player!", zombie.id);
            } else if (zombie.stateTimer <= 0.0f) {
                // Randomly start wandering
                zombie.state = ZOMBIE_WANDER;
                zombie.stateTimer = 3.0f + (rand() % 3);
                
                // Pick random nearby position
                zombie.targetPosition = zombie.position;
                zombie.targetPosition.x += (rand() % 20 - 10);
                zombie.targetPosition.z += (rand() % 20 - 10);
            }
            break;
            
        case ZOMBIE_WANDER:
            if (distanceToPlayer < zombie.detectionRange && HasLineOfSight(zombie.position, playerPos)) {
                zombie.state = ZOMBIE_CHASE;
                zombie.targetPosition = playerPos;
            } else if (zombie.stateTimer <= 0.0f) {
                zombie.state = ZOMBIE_IDLE;
                zombie.stateTimer = 2.0f;
            }
            break;
            
        case ZOMBIE_CHASE:
            zombie.targetPosition = playerPos;
            
            if (distanceToPlayer < zombie.attackRange) {
                zombie.state = ZOMBIE_ATTACK;
                zombie.stateTimer = 1.0f;
            } else if (distanceToPlayer > zombie.detectionRange * 1.5f) {
                // Lost player
                zombie.state = ZOMBIE_IDLE;
                zombie.stateTimer = 2.0f;
            }
            break;
            
        case ZOMBIE_ATTACK:
            // Face player
            Vector3 toPlayer = Vector3Subtract(playerPos, zombie.position);
            zombie.rotation = atan2f(toPlayer.x, toPlayer.z) * RAD2DEG;
            
            // Attack if cooldown ready
            if (zombie.attackCooldown <= 0.0f && distanceToPlayer < zombie.attackRange) {
                zombie.attackCooldown = 1.5f;
                // Damage will be applied in main game loop
                TraceLog(LOG_INFO, "Zombie %d attacks! Damage: %.0f", zombie.id, zombie.attackDamage);
            }
            
            if (zombie.stateTimer <= 0.0f) {
                if (distanceToPlayer < zombie.attackRange) {
                    zombie.state = ZOMBIE_ATTACK;
                    zombie.stateTimer = 1.0f;
                } else {
                    zombie.state = ZOMBIE_CHASE;
                }
            }
            break;
    }
}

bool ZombieManager::HasLineOfSight(Vector3 from, Vector3 to) {
    // Simple line of sight - check if path is clear
    // In full implementation, would check against walls
    float distance = Vector3Distance(from, to);
    return distance < 50.0f; // Simple distance check for now
}

void ZombieManager::Draw(const Camera3D& camera) {
    for (const auto& zombie : zombies) {
        if (zombie.isAlive) {
            DrawZombie(zombie);
        }
    }
}

void ZombieManager::DrawZombie(const Zombie& zombie) {
    // Draw zombie body
    rlPushMatrix();
    rlTranslatef(zombie.position.x, zombie.position.y, zombie.position.z);
    rlRotatef(zombie.rotation, 0, 1, 0);
    
    // Body
    DrawCube(Vector3{0, 0.9f, 0}, 0.6f, 1.8f, 0.4f, Color{80, 100, 80, 255});
    
    // Head
    DrawCube(Vector3{0, 1.9f, 0}, 0.4f, 0.4f, 0.4f, Color{90, 110, 90, 255});
    
    // Eyes (glowing red)
    DrawCube(Vector3{-0.1f, 2.0f, 0.2f}, 0.08f, 0.08f, 0.02f, RED);
    DrawCube(Vector3{0.1f, 2.0f, 0.2f}, 0.08f, 0.08f, 0.02f, RED);
    
    // Arms
    DrawCube(Vector3{-0.4f, 0.9f, 0}, 0.2f, 1.0f, 0.2f, Color{80, 100, 80, 255});
    DrawCube(Vector3{0.4f, 0.9f, 0}, 0.2f, 1.0f, 0.2f, Color{80, 100, 80, 255});
    
    // Legs
    DrawCube(Vector3{-0.15f, 0.4f, 0}, 0.2f, 0.8f, 0.2f, Color{70, 90, 70, 255});
    DrawCube(Vector3{0.15f, 0.4f, 0}, 0.2f, 0.8f, 0.2f, Color{70, 90, 70, 255});
    
    rlPopMatrix();
    
    // Draw health bar above zombie
    Vector3 healthBarPos = zombie.position;
    healthBarPos.y += 2.5f;
    
    float healthPercent = zombie.health / zombie.maxHealth;
    float barWidth = 0.6f;
    float barHeight = 0.1f;
    
    // Background (red)
    DrawCube(healthBarPos, barWidth, barHeight, 0.02f, RED);
    
    // Health (green)
    Vector3 healthPos = healthBarPos;
    healthPos.x -= barWidth / 2.0f * (1.0f - healthPercent);
    DrawCube(healthPos, barWidth * healthPercent, barHeight, 0.03f, GREEN);
}

void ZombieManager::DamageZombie(int zombieId, float damage) {
    for (auto& zombie : zombies) {
        if (zombie.id == zombieId && zombie.isAlive) {
            zombie.health -= damage;
            TraceLog(LOG_INFO, "Zombie %d took %.0f damage (%.0f/%.0f HP)", 
                     zombieId, damage, zombie.health, zombie.maxHealth);
            
            if (zombie.health <= 0.0f) {
                zombie.isAlive = false;
                TraceLog(LOG_INFO, "Zombie %d killed!", zombieId);
            }
            break;
        }
    }
}

Zombie* ZombieManager::GetZombieAt(Vector3 position, float radius) {
    Zombie* nearest = nullptr;
    float minDist = radius;
    
    for (auto& zombie : zombies) {
        if (!zombie.isAlive) continue;
        
        float dist = Vector3Distance(position, zombie.position);
        if (dist < minDist) {
            minDist = dist;
            nearest = &zombie;
        }
    }
    
    return nearest;
}

void ZombieManager::ClearAll() {
    zombies.clear();
    TraceLog(LOG_INFO, "All zombies cleared");
}

void ZombieManager::Unload() {
    ClearAll();
}

// Global initialization
void InitializeZombieSystem() {
    g_ZombieManager = new ZombieManager();
    g_ZombieManager->Initialize();
    TraceLog(LOG_INFO, "Zombie system initialized");
}

void CleanupZombieSystem() {
    if (g_ZombieManager) {
        delete g_ZombieManager;
        g_ZombieManager = nullptr;
    }
    TraceLog(LOG_INFO, "Zombie system cleaned up");
}
