
#pragma once
#include "globals.h"
#include <string>

struct InventorySlot;

// UI Animation states
struct UIAnimation {
    float current;
    float target;
    float speed;
    bool active;
    
    void Update(float deltaTime) {
        if (!active) return;
        if (current < target) {
            current += speed * deltaTime;
            if (current >= target) {
                current = target;
                active = false;
            }
        } else if (current > target) {
            current -= speed * deltaTime;
            if (current <= target) {
                current = target;
                active = false;
            }
        }
    }
    
    void SetTarget(float newTarget, float newSpeed = 5.0f) {
        target = newTarget;
        speed = newSpeed;
        active = true;
    }
};

// Modern HUD Manager
class HUDManager {
public:
    HUDManager();
    ~HUDManager();
    
    void Initialize();
    void Update(float deltaTime);
    void Draw(int screenW, int screenH);
    
    // HUD Elements
    void DrawPlayerStats(int screenW, int screenH, float health, float stamina, 
                        float hunger, float thirst);
    void DrawWeaponInfo(int screenW, int screenH, const InventorySlot& weapon);
    void DrawFlashlightStatus(int screenW, int screenH, float battery, bool isOn);
    void DrawCrosshair(int screenW, int screenH, bool isAiming);
    void DrawHitMarker(int screenW, int screenH);
    void DrawDamageVignette(int screenW, int screenH, float damageIntensity);
    void DrawCompass(int screenW, int screenH, float yaw);
    void DrawNotification(const std::string& message, float duration);
    
    // UI State
    void ShowDamageIndicator(float damage);
    void ShowHitMarker();
    void SetLowHealthWarning(bool enabled);
    
private:
    // Animations
    UIAnimation healthBarAnim;
    UIAnimation staminaBarAnim;
    UIAnimation hungerBarAnim;
    UIAnimation thirstBarAnim;
    UIAnimation damageVignetteAnim;
    UIAnimation hitMarkerAnim;
    
    // State
    bool lowHealthWarning;
    float lowHealthPulse;
    float notificationTimer;
    std::string currentNotification;
    
    // Drawing helpers
    void DrawModernBar(int x, int y, int width, int height, float value, 
                      Color fillColor, Color bgColor, const char* label);
    void DrawRadialBar(int centerX, int centerY, float radius, float value, 
                      Color color, float thickness);
    void DrawGlowingText(const char* text, int x, int y, int fontSize, 
                        Color color, Color glowColor);
    void DrawModernCrosshair(int centerX, int centerY, float spread, Color color);
    
    // Render textures for effects
    RenderTexture2D vignetteTexture;
};


