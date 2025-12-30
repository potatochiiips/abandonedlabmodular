#pragma once
#include "globals.h"

// First person hands rendering
class HandsRenderer {
public:
    HandsRenderer() {}
    ~HandsRenderer() {}

    void Initialize() {
        TraceLog(LOG_INFO, "Hands Renderer initialized");
    }

    void DrawHands(const Camera3D& camera, bool holdingWeapon);

private:
    void DrawHandMesh(Vector3 position, Vector3 forward, Vector3 right,
        Vector3 up, bool leftHand);
};

// Global instance
extern HandsRenderer* g_HandsRenderer;