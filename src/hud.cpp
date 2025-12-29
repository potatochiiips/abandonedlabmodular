#include "hud.h"
#include "items.h"
#include "rlgl.h"
#include <cmath>

HUDManager::HUDManager() {
    lowHealthWarning = false;
    lowHealthPulse = 0.0f;
    notificationTimer = 0.0f;

    // Initialize animations
    healthBarAnim = { 100.0f, 100.0f, 3.0f, false };
    staminaBarAnim = { 100.0f, 100.0f, 5.0f, false };
    hungerBarAnim = { 100.0f, 100.0f, 2.0f, false };
    thirstBarAnim = { 100.0f, 100.0f, 2.0f, false };
    damageVignetteAnim = { 0.0f, 0.0f, 5.0f, false };
    hitMarkerAnim = { 0.0f, 0.0f, 8.0f, false };
}

HUDManager::~HUDManager() {
    if (vignetteTexture.id > 0) {
        UnloadRenderTexture(vignetteTexture);
    }
}

void HUDManager::Initialize() {
    TraceLog(LOG_INFO, "Initializing Upgraded HUD Manager...");

    // Create vignette texture
    vignetteTexture = LoadRenderTexture(1920, 1080);

    TraceLog(LOG_INFO, "Upgraded HUD Manager initialized");
}

void HUDManager::Update(float deltaTime) {
    // Update animations
    healthBarAnim.Update(deltaTime);
    staminaBarAnim.Update(deltaTime);
    hungerBarAnim.Update(deltaTime);
    thirstBarAnim.Update(deltaTime);
    damageVignetteAnim.Update(deltaTime);
    hitMarkerAnim.Update(deltaTime);

    // Update low health pulse
    if (lowHealthWarning) {
        lowHealthPulse += deltaTime * 3.0f;
    }

    // Update notification timer
    if (notificationTimer > 0.0f) {
        notificationTimer -= deltaTime;
    }
}

void HUDManager::Draw(int screenW, int screenH) {
    // This is called from main render loop
}

void HUDManager::DrawPlayerStats(int screenW, int screenH, float health,
    float stamina, float hunger, float thirst) {
    // Update animation targets
    healthBarAnim.SetTarget(health);
    staminaBarAnim.SetTarget(stamina);
    hungerBarAnim.SetTarget(hunger);
    thirstBarAnim.SetTarget(thirst);

    int barWidth = 250;
    int barHeight = 8;
    int barSpacing = 18;
    int startX = 30;
    int startY = screenH - 140;

    // Modern semi-transparent background panel
    DrawRectangle(startX - 10, startY - 10, barWidth + 20, 110,
        Color{ 0, 0, 0, 100 });
    DrawRectangleLines(startX - 10, startY - 10, barWidth + 20, 110,
        Color{ 255, 255, 255, 30 });

    // Health bar (red)
    Color healthColor = health > 30 ? Color{ 255, 60, 60, 255 } : Color{ 255, 30, 30, 255 };
    if (lowHealthWarning && health < 30) {
        float pulse = (sinf(lowHealthPulse) + 1.0f) * 0.5f;
        healthColor.a = (unsigned char)(150 + pulse * 105);
    }
    DrawModernBar(startX, startY, barWidth, barHeight, healthBarAnim.current,
        healthColor, Color{ 40, 20, 20, 180 }, "HEALTH");

    // Stamina bar (yellow)
    DrawModernBar(startX, startY + barSpacing, barWidth, barHeight,
        staminaBarAnim.current, Color{ 255, 220, 60, 255 },
        Color{ 40, 35, 20, 180 }, "STAMINA");

    // Hunger bar (orange)
    DrawModernBar(startX, startY + barSpacing * 2, barWidth, barHeight,
        hungerBarAnim.current, Color{ 255, 140, 60, 255 },
        Color{ 40, 30, 20, 180 }, "HUNGER");

    // Thirst bar (blue)
    DrawModernBar(startX, startY + barSpacing * 3, barWidth, barHeight,
        thirstBarAnim.current, Color{ 60, 180, 255, 255 },
        Color{ 20, 30, 40, 180 }, "THIRST");

    // Numeric values
    int textY = startY;
    DrawGlowingText(TextFormat("%d", (int)health), startX + barWidth + 15, textY - 2,
        16, healthColor, Color{ 0, 0, 0, 100 });
    textY += barSpacing;
    DrawGlowingText(TextFormat("%d", (int)stamina), startX + barWidth + 15, textY - 2,
        16, Color{ 255, 220, 60, 255 }, Color{ 0, 0, 0, 100 });
    textY += barSpacing;
    DrawGlowingText(TextFormat("%d", (int)hunger), startX + barWidth + 15, textY - 2,
        16, Color{ 255, 140, 60, 255 }, Color{ 0, 0, 0, 100 });
    textY += barSpacing;
    DrawGlowingText(TextFormat("%d", (int)thirst), startX + barWidth + 15, textY - 2,
        16, Color{ 60, 180, 255, 255 }, Color{ 0, 0, 0, 100 });
}

void HUDManager::DrawWeaponInfo(int screenW, int screenH, const InventorySlot& weapon) {
    if (weapon.itemId == ITEM_NONE) return;

    int panelWidth = 280;
    int panelHeight = 90;
    int panelX = screenW - panelWidth - 30;
    int panelY = screenH - panelHeight - 30;

    // Modern weapon info panel
    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Color{ 0, 0, 0, 120 });
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, Color{ 255, 255, 255, 40 });

    // Weapon name
    const char* weaponName = GetItemName(weapon.itemId);
    DrawGlowingText(weaponName, panelX + 15, panelY + 12, 22,
        Color{ 255, 255, 255, 255 }, Color{ 100, 150, 255, 100 });

    // Ammo display for weapons
    if (weapon.itemId == ITEM_PISTOL || weapon.itemId == ITEM_M16) {
        int maxAmmo = weapon.itemId == ITEM_PISTOL ? 15 : 30;

        // Large ammo counter
        const char* ammoText = TextFormat("%d", weapon.ammo);
        int ammoTextSize = 48;
        int ammoTextWidth = MeasureText(ammoText, ammoTextSize);

        Color ammoColor = weapon.ammo > maxAmmo / 3 ? Color{ 255, 255, 255, 255 } :
            weapon.ammo > 0 ? Color{ 255, 180, 60, 255 } :
            Color{ 255, 60, 60, 255 };

        DrawGlowingText(ammoText, panelX + panelWidth - ammoTextWidth - 60, panelY + 35,
            ammoTextSize, ammoColor, Color{ 0, 0, 0, 150 });

        // Magazine count
        int magId = weapon.itemId == ITEM_PISTOL ? ITEM_MAG : ITEM_M16_MAG;
        int magCount = 0;
        extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
        for (int i = 0; i < BACKPACK_SLOTS; i++) {
            if (inventory[i].itemId == magId) {
                magCount += inventory[i].quantity;
            }
        }

        DrawGlowingText(TextFormat("/ %d", magCount), panelX + panelWidth - 50, panelY + 50,
            18, Color{ 200, 200, 200, 255 }, Color{ 0, 0, 0, 100 });

        // Reload prompt
        if (weapon.ammo == 0 && magCount > 0) {
            float pulse = (sinf((float)GetTime() * 4.0f) + 1.0f) * 0.5f;
            Color promptColor = Color{ 255, 220, 60, (unsigned char)(150 + pulse * 105) };
            DrawGlowingText("PRESS R TO RELOAD", panelX + 15, panelY + 62, 14,
                promptColor, Color{ 80, 70, 20, 100 });
        }

        // Ammo bar
        float ammoPercent = (float)weapon.ammo / (float)maxAmmo;
        int barWidth = panelWidth - 30;
        int barHeight = 4;
        int barY = panelY + panelHeight - 12;

        DrawRectangle(panelX + 15, barY, barWidth, barHeight, Color{ 40, 40, 45, 180 });
        DrawRectangle(panelX + 15, barY, (int)(barWidth * ammoPercent), barHeight, ammoColor);
    }
}

void HUDManager::DrawFlashlightStatus(int screenW, int screenH, float battery, bool isOn) {
    if (!isOn && battery >= 90.0f) return; // Hide when off and full

    int iconSize = 32;
    int x = screenW - 70;
    int y = screenH - 140;

    // Icon background
    Color bgColor = isOn ? Color{ 255, 250, 220, 30 } : Color{ 60, 60, 65, 30 };
    DrawCircle(x + iconSize / 2, y + iconSize / 2, iconSize / 2 + 4, bgColor);

    // Flashlight icon (simple)
    Color iconColor = isOn ? Color{ 255, 250, 220, 255 } : Color{ 150, 150, 155, 255 };
    DrawCircle(x + iconSize / 2, y + iconSize / 2, iconSize / 3, iconColor);

    // Battery radial indicator
    Color batteryColor = battery > 30 ? Color{ 60, 255, 60, 255 } :
        battery > 10 ? Color{ 255, 180, 60, 255 } :
        Color{ 255, 60, 60, 255 };

    DrawRadialBar(x + iconSize / 2, y + iconSize / 2, iconSize / 2 + 2, battery / 100.0f,
        batteryColor, 3.0f);

    // Battery percentage
    if (battery < 50.0f || isOn) {
        DrawGlowingText(TextFormat("%d%%", (int)battery), x + 5, y + iconSize + 5, 12,
            batteryColor, Color{ 0, 0, 0, 100 });
    }
}

void HUDManager::DrawCrosshair(int screenW, int screenH, bool isAiming) {
    int centerX = screenW / 2;
    int centerY = screenH / 2;

    float spread = isAiming ? 8.0f : 16.0f;
    Color crosshairColor = Color{ 255, 255, 255, 200 };

    DrawModernCrosshair(centerX, centerY, spread, crosshairColor);

    // ADS dot
    if (isAiming) {
        DrawCircle(centerX, centerY, 2, Color{ 255, 60, 60, 200 });
    }
}

void HUDManager::DrawHitMarker(int screenW, int screenH) {
    if (hitMarkerAnim.current <= 0.0f) return;

    int centerX = screenW / 2;
    int centerY = screenH / 2;

    float alpha = hitMarkerAnim.current * 255.0f;
    Color markerColor = Color{ 255, 255, 255, (unsigned char)alpha };

    int size = 20;
    int thickness = 3;
    int gap = 8;

    // X-shaped hit marker
    DrawLineEx(Vector2{ (float)(centerX - size), (float)(centerY - size) },
        Vector2{ (float)(centerX - gap), (float)(centerY - gap) }, thickness, markerColor);
    DrawLineEx(Vector2{ (float)(centerX + gap), (float)(centerY - gap) },
        Vector2{ (float)(centerX + size), (float)(centerY - size) }, thickness, markerColor);
    DrawLineEx(Vector2{ (float)(centerX - size), (float)(centerY + size) },
        Vector2{ (float)(centerX - gap), (float)(centerY + gap) }, thickness, markerColor);
    DrawLineEx(Vector2{ (float)(centerX + gap), (float)(centerY + gap) },
        Vector2{ (float)(centerX + size), (float)(centerY + size) }, thickness, markerColor);
}

void HUDManager::DrawDamageVignette(int screenW, int screenH, float damageIntensity) {
    if (damageVignetteAnim.current <= 0.0f) return;

    float intensity = damageVignetteAnim.current * damageIntensity;
    Color vignetteColor = Color{ 255, 0, 0, (unsigned char)(intensity * 80) };

    // Draw red vignette overlay
    int edgeWidth = screenW / 6;
    int edgeHeight = screenH / 6;

    // Top
    DrawRectangleGradientV(0, 0, screenW, edgeHeight, vignetteColor, BLANK);
    // Bottom
    DrawRectangleGradientV(0, screenH - edgeHeight, screenW, edgeHeight, BLANK, vignetteColor);
    // Left
    DrawRectangleGradientH(0, 0, edgeWidth, screenH, vignetteColor, BLANK);
    // Right
    DrawRectangleGradientH(screenW - edgeWidth, 0, edgeWidth, screenH, BLANK, vignetteColor);
}

void HUDManager::DrawCompass(int screenW, int screenH, float yaw) {
    int compassWidth = 300;
    int compassHeight = 40;
    int compassX = screenW / 2 - compassWidth / 2;
    int compassY = 20;

    // Background
    DrawRectangle(compassX, compassY, compassWidth, compassHeight, Color{ 0, 0, 0, 100 });
    DrawRectangleLines(compassX, compassY, compassWidth, compassHeight,
        Color{ 255, 255, 255, 30 });

    // Cardinal directions
    const char* directions[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
    float angles[] = { 0, 45, 90, 135, 180, 225, 270, 315 };

    for (int i = 0; i < 8; i++) {
        float angle = angles[i] - yaw;
        while (angle < 0) angle += 360;
        while (angle >= 360) angle -= 360;

        // Map to compass position (-150 to 150 degrees visible)
        if (angle > 180) angle -= 360;

        if (fabs(angle) < 75) {
            int xPos = compassX + compassWidth / 2 + (int)((angle / 75.0f) * (compassWidth / 2));
            int yPos = compassY + compassHeight / 2;

            Color dirColor = (i % 2 == 0) ? Color{ 255, 255, 255, 255 } : Color{ 200, 200, 200, 200 };
            if (i == 0) dirColor = Color{ 255, 60, 60, 255 }; // North is red

            DrawGlowingText(directions[i], xPos - MeasureText(directions[i], 16) / 2,
                yPos - 8, 16, dirColor, Color{ 0, 0, 0, 100 });
        }
    }

    // Center indicator
    DrawLine(compassX + compassWidth / 2, compassY + 5,
        compassX + compassWidth / 2, compassY + compassHeight - 5,
        Color{ 255, 60, 60, 255 });
}

void HUDManager::DrawNotification(const std::string& message, float duration) {
    currentNotification = message;
    notificationTimer = duration;
}

void HUDManager::ShowDamageIndicator(float damage) {
    damageVignetteAnim.SetTarget(1.0f, 10.0f);
    damageVignetteAnim.SetTarget(0.0f, 3.0f);
}

void HUDManager::ShowHitMarker() {
    hitMarkerAnim.current = 1.0f;
    hitMarkerAnim.SetTarget(0.0f, 8.0f);
}

void HUDManager::SetLowHealthWarning(bool enabled) {
    lowHealthWarning = enabled;
    if (!enabled) {
        lowHealthPulse = 0.0f;
    }
}

// Helper drawing functions
void HUDManager::DrawModernBar(int x, int y, int width, int height, float value,
    Color fillColor, Color bgColor, const char* label) {
    // Background
    DrawRectangle(x, y, width, height, bgColor);

    // Fill
    int fillWidth = (int)((value / 100.0f) * width);
    DrawRectangle(x, y, fillWidth, height, fillColor);

    // Shine effect
    int shineHeight = height / 3;
    DrawRectangleGradientV(x, y, fillWidth, shineHeight,
        Color{ 255, 255, 255, 40 }, BLANK);

    // Border
    DrawRectangleLines(x, y, width, height, Color{ 255, 255, 255, 60 });

    // Label
    DrawText(label, x, y - 14, 11, Color{ 200, 200, 200, 255 });
}

void HUDManager::DrawRadialBar(int centerX, int centerY, float radius, float value,
    Color color, float thickness) {
    int segments = 32;
    float angleStep = 360.0f / segments;
    float fillAngle = value * 360.0f;

    for (int i = 0; i < segments; i++) {
        float angle1 = (i * angleStep - 90) * DEG2RAD;
        float angle2 = ((i + 1) * angleStep - 90) * DEG2RAD;

        if (i * angleStep < fillAngle) {
            Vector2 p1 = { centerX + cosf(angle1) * radius, centerY + sinf(angle1) * radius };
            Vector2 p2 = { centerX + cosf(angle2) * radius, centerY + sinf(angle2) * radius };
            DrawLineEx(p1, p2, thickness, color);
        }
    }
}

void HUDManager::DrawGlowingText(const char* text, int x, int y, int fontSize,
    Color color, Color glowColor) {
    // Glow
    for (int ox = -1; ox <= 1; ox++) {
        for (int oy = -1; oy <= 1; oy++) {
            if (ox != 0 || oy != 0) {
                DrawText(text, x + ox, y + oy, fontSize, glowColor);
            }
        }
    }
    // Main text
    DrawText(text, x, y, fontSize, color);
}

void HUDManager::DrawModernCrosshair(int centerX, int centerY, float spread, Color color) {
    int lineLength = 12;
    int thickness = 2;
    int gap = (int)spread;

    // Top
    DrawLineEx(Vector2{ (float)centerX, (float)(centerY - gap) },
        Vector2{ (float)centerX, (float)(centerY - gap - lineLength) }, thickness, color);
    // Bottom
    DrawLineEx(Vector2{ (float)centerX, (float)(centerY + gap) },
        Vector2{ (float)centerX, (float)(centerY + gap + lineLength) }, thickness, color);
    // Left
    DrawLineEx(Vector2{ (float)(centerX - gap), (float)centerY },
        Vector2{ (float)(centerX - gap - lineLength), (float)centerY }, thickness, color);
    // Right
    DrawLineEx(Vector2{ (float)(centerX + gap), (float)centerY },
        Vector2{ (float)(centerX + gap + lineLength), (float)centerY }, thickness, color);
}

// Global instance
extern HUDManager* g_UpgradedHUD;