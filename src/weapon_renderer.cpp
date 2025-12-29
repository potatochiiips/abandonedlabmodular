#include "weapon_renderer.h"
#include "model_manager.h"
#include "player.h"

WeaponRenderer::WeaponRenderer() {
    currentState = { ANIM_IDLE, 0.0f, false, 0.0f, {0,0,0}, {0,0,0}, {0,0,0} };

    basePosition = Vector3{ 0.25f, -0.2f, 0.4f };
    aimPosition = Vector3{ 0.0f, -0.15f, 0.35f };
    currentRecoil = Vector3{ 0, 0, 0 };
    viewmodelFOV = 60.0f;

    swayAmount = Vector3{ 0, 0, 0 };
    swayVelocity = Vector3{ 0, 0, 0 };
    swaySmooth = 8.0f;

    bobTimer = 0.0f;
    bobAmount = Vector3{ 0, 0, 0 };

    showMuzzleFlash = false;
    muzzleFlashTimer = 0.0f;
}

WeaponRenderer::~WeaponRenderer() {
}

void WeaponRenderer::Initialize() {
    TraceLog(LOG_INFO, "Initializing Upgraded Weapon Renderer...");

    weaponRenderer = std::make_unique<MeshRenderer>();

    // Create weapon material with special viewmodel shader
    weaponMaterial = std::make_shared<UpgradedMaterial>(g_UpgradedPipeline->CreateUnlitShader());
    weaponMaterial->SetRenderQueue(QUEUE_OVERLAY); // Render on top

    weaponRenderer->material = weaponMaterial;
    weaponRenderer->castShadows = false;
    weaponRenderer->receiveShadows = false;
    weaponRenderer->enabled = false;

    TraceLog(LOG_INFO, "Upgraded Weapon Renderer initialized");
}

void WeaponRenderer::Update(float deltaTime, const Camera3D& camera) {
    UpdateAnimations(deltaTime);
    UpdateWeaponPosition(camera);
    UpdateWeaponSway(camera);
    UpdateWeaponBob(deltaTime);

    // Update muzzle flash
    if (showMuzzleFlash) {
        muzzleFlashTimer -= deltaTime;
        if (muzzleFlashTimer <= 0.0f) {
            showMuzzleFlash = false;
        }
    }

    // Apply decay to recoil
    currentRecoil.x *= 0.85f;
    currentRecoil.y *= 0.85f;
    currentRecoil.z *= 0.85f;
}

void WeaponRenderer::UpdateAnimations(float deltaTime) {
    // Update animation timer
    if (currentState.animTimer > 0.0f) {
        currentState.animTimer -= deltaTime;
        if (currentState.animTimer <= 0.0f) {
            currentState.animState = currentState.isADS ? ANIM_ADS : ANIM_IDLE;
        }
    }

    // Smoothly transition ADS progress
    float adsTarget = currentState.isADS ? 1.0f : 0.0f;
    float adsSpeed = 8.0f;

    if (currentState.adsProgress < adsTarget) {
        currentState.adsProgress += deltaTime * adsSpeed;
        if (currentState.adsProgress > adsTarget) {
            currentState.adsProgress = adsTarget;
        }
    }
    else if (currentState.adsProgress > adsTarget) {
        currentState.adsProgress -= deltaTime * adsSpeed;
        if (currentState.adsProgress < adsTarget) {
            currentState.adsProgress = adsTarget;
        }
    }
}

void WeaponRenderer::UpdateWeaponPosition(const Camera3D& camera) {
    if (!weaponRenderer || !weaponRenderer->enabled) return;

    // Lerp between base and ADS position
    Vector3 targetPos = Vector3Lerp(basePosition, aimPosition, currentState.adsProgress);

    // Apply weapon bob
    targetPos = Vector3Add(targetPos, bobAmount);

    // Apply weapon sway
    targetPos = Vector3Add(targetPos, swayAmount);

    // Apply recoil
    targetPos = Vector3Add(targetPos, currentRecoil);

    // Apply animation offsets
    switch (currentState.animState) {
    case ANIM_RELOAD:
        targetPos.y -= 0.3f * (currentState.animTimer / 2.0f);
        targetPos.z -= 0.2f * (currentState.animTimer / 2.0f);
        break;
    case ANIM_SHOOT:
        targetPos.z -= 0.05f * (currentState.animTimer / 0.1f);
        break;
    default:
        break;
    }

    // Create transform matrix
    weaponRenderer->transform = CreateViewmodelMatrix(camera, targetPos);
}

Matrix WeaponRenderer::CreateViewmodelMatrix(const Camera3D& camera, Vector3 offset) {
    // Get camera vectors
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = Vector3Normalize(camera.up);

    // Calculate weapon position in world space
    Vector3 position = camera.position;
    position = Vector3Add(position, Vector3Scale(forward, offset.z));
    position = Vector3Add(position, Vector3Scale(right, offset.x));
    position = Vector3Add(position, Vector3Scale(up, offset.y));

    // Create orientation matrix
    Matrix orientation = {
        right.x, right.y, right.z, 0.0f,
        up.x, up.y, up.z, 0.0f,
        forward.x, forward.y, forward.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Scale
    Matrix scale = MatrixScale(1.0f, 1.0f, 1.0f);

    // Translation
    Matrix translation = MatrixTranslate(position.x, position.y, position.z);

    // Combine transformations
    return MatrixMultiply(MatrixMultiply(scale, orientation), translation);
}

void WeaponRenderer::UpdateWeaponSway(const Camera3D& camera) {
    // Get mouse delta or look input
    Vector2 mouseDelta = GetMouseDelta();

    // Calculate target sway
    Vector3 targetSway;
    targetSway.x = -mouseDelta.x * 0.002f;
    targetSway.y = -mouseDelta.y * 0.002f;
    targetSway.z = 0.0f;

    // Reduce sway when aiming
    float swayMultiplier = 1.0f - (currentState.adsProgress * 0.7f);
    targetSway.x *= swayMultiplier;
    targetSway.y *= swayMultiplier;

    // Smooth damp to target
    swayAmount.x = Lerp(swayAmount.x, targetSway.x, GetFrameTime() * swaySmooth);
    swayAmount.y = Lerp(swayAmount.y, targetSway.y, GetFrameTime() * swaySmooth);

    // Clamp sway
    swayAmount.x = Clamp(swayAmount.x, -0.05f, 0.05f);
    swayAmount.y = Clamp(swayAmount.y, -0.05f, 0.05f);
}

void WeaponRenderer::UpdateWeaponBob(float deltaTime) {
    extern Vector3 playerVelocity;
    float speed = Vector3Length(Vector3{ playerVelocity.x, 0, playerVelocity.z });

    if (speed > 0.1f && currentState.adsProgress < 0.5f) {
        bobTimer += deltaTime * 8.0f;

        float bobIntensity = 0.02f * (1.0f - currentState.adsProgress);

        bobAmount.x = sinf(bobTimer) * bobIntensity;
        bobAmount.y = sinf(bobTimer * 2.0f) * bobIntensity * 0.5f;
    }
    else {
        // Decay bob when not moving
        bobAmount.x *= 0.9f;
        bobAmount.y *= 0.9f;

        if (fabs(bobAmount.x) < 0.001f) bobAmount.x = 0.0f;
        if (fabs(bobAmount.y) < 0.001f) bobAmount.y = 0.0f;
    }
}

void WeaponRenderer::SetEquippedWeapon(int itemId) {
    if (itemId == ITEM_NONE) {
        weaponRenderer->enabled = false;
        return;
    }

    // Get weapon model
    ModelID modelId = GetModelIDFromItem(itemId);

    if (g_ModelManager && g_ModelManager->IsLoaded(modelId)) {
        Model model = g_ModelManager->GetModel(modelId);

        if (model.meshCount > 0) {
            weaponRenderer->mesh = model.meshes[0];

            // Apply weapon texture if available
            const ModelData* modelData = g_ModelManager->GetModelData(modelId);
            if (modelData) {
                // Use model's texture
            }

            weaponRenderer->enabled = true;

            TraceLog(LOG_INFO, "Equipped weapon: %d", itemId);
        }
    }
}

void WeaponRenderer::PlayShootAnimation() {
    currentState.animState = ANIM_SHOOT;
    currentState.animTimer = 0.2f;

    // Recoil impulse
    currentRecoil.y = -0.08f;
    currentRecoil.z = -0.05f;

    // Muzzle flash
    showMuzzleFlash = true;
    muzzleFlashTimer = 0.1f;
}

void WeaponRenderer::PlayReloadAnimation() {
    currentState.animState = ANIM_RELOAD;
    currentState.animTimer = 2.0f;
}

void WeaponRenderer::SetAimingDownSights(bool aiming) {
    currentState.isADS = aiming;
}

void WeaponRenderer::ApplyRecoil(float pitch, float yaw) {
    currentRecoil.y -= pitch * 0.01f;
    currentRecoil.x += yaw * 0.005f;
}

MeshRenderer* WeaponRenderer::GetWeaponRenderer() {
    if (weaponRenderer && weaponRenderer->enabled) {
        return weaponRenderer.get();
    }
    return nullptr;
}

void WeaponRenderer::RenderItemInHand(const Camera3D& camera, int itemId) {
    if (itemId == ITEM_NONE) return;

    // For non-weapon items (flashlight, consumables, etc.)
    ModelID modelId = GetModelIDFromItem(itemId);

    if (g_ModelManager && g_ModelManager->IsLoaded(modelId)) {
        // Calculate hand position
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        Vector3 up = Vector3Normalize(camera.up);

        Vector3 itemPos = camera.position;
        itemPos = Vector3Add(itemPos, Vector3Scale(forward, 0.5f));
        itemPos = Vector3Add(itemPos, Vector3Scale(right, 0.2f));
        itemPos = Vector3Add(itemPos, Vector3Scale(up, -0.3f));

        // Draw using model manager
        g_ModelManager->DrawModel(modelId, itemPos, forward, right, up, WHITE);

        // Special effects for flashlight
        if (itemId == ITEM_FLASHLIGHT) {
            extern bool isFlashlightOn;
            if (isFlashlightOn) {
                Vector3 glowPos = Vector3Add(itemPos, Vector3Scale(forward, 0.1f));
                DrawSphere(glowPos, 0.04f, Color{ 255, 255, 220, 100 });
            }
        }
    }
}

void WeaponRenderer::DrawMuzzleFlash(const Camera3D& camera) {
    if (!showMuzzleFlash) return;

    // Calculate muzzle position
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = Vector3Normalize(camera.up);

    Vector3 muzzlePos = camera.position;
    muzzlePos = Vector3Add(muzzlePos, Vector3Scale(forward, 0.6f));
    muzzlePos = Vector3Add(muzzlePos, Vector3Scale(right, 0.15f));
    muzzlePos = Vector3Add(muzzlePos, Vector3Scale(up, -0.15f));

    // Draw muzzle flash sprite
    float flashSize = 0.15f;
    Color flashColor = Color{ 255, 230, 150, (unsigned char)(muzzleFlashTimer / 0.1f * 255) };

    // Multiple layers for flash effect
    DrawSphere(muzzlePos, flashSize, flashColor);
    DrawSphere(muzzlePos, flashSize * 0.7f, Color{ 255, 255, 200, flashColor.a });
    DrawSphere(muzzlePos, flashSize * 0.4f, Color{ 255, 255, 255, flashColor.a });
}

// ============================================================================
// FIRST PERSON HANDS RENDERING
// ============================================================================

class HandsRenderer {
public:
    void DrawHands(const Camera3D& camera, bool holdingWeapon) {
        if (holdingWeapon) return; // Weapon handles its own hands

        // Draw idle hands animation
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        Vector3 up = Vector3Normalize(camera.up);

        float time = (float)GetTime();
        float breatheBob = sinf(time * 1.5f) * 0.01f;

        // Right hand
        Vector3 rightHandPos = camera.position;
        rightHandPos = Vector3Add(rightHandPos, Vector3Scale(forward, 0.4f));
        rightHandPos = Vector3Add(rightHandPos, Vector3Scale(right, 0.2f));
        rightHandPos = Vector3Add(rightHandPos, Vector3Scale(up, -0.25f + breatheBob));

        DrawHandMesh(rightHandPos, forward, right, up, false);

        // Left hand
        Vector3 leftHandPos = camera.position;
        leftHandPos = Vector3Add(leftHandPos, Vector3Scale(forward, 0.45f));
        leftHandPos = Vector3Add(leftHandPos, Vector3Scale(right, -0.25f));
        leftHandPos = Vector3Add(leftHandPos, Vector3Scale(up, -0.28f + breatheBob * 0.8f));

        DrawHandMesh(leftHandPos, forward, right, up, true);
    }

private:
    void DrawHandMesh(Vector3 position, Vector3 forward, Vector3 right,
        Vector3 up, bool leftHand) {
        Color skinColor = Color{ 210, 180, 140, 255 };

        // Palm
        Vector3 palmSize = { 0.04f, 0.06f, 0.08f };
        DrawCubeV(position, palmSize, skinColor);

        // Fingers
        float fingerSpacing = leftHand ? -0.01f : 0.01f;
        for (int i = 0; i < 4; i++) {
            Vector3 fingerPos = position;
            fingerPos = Vector3Add(fingerPos, Vector3Scale(forward, 0.05f));
            fingerPos = Vector3Add(fingerPos, Vector3Scale(right, fingerSpacing * (i - 1.5f)));

            DrawCubeV(fingerPos, Vector3{ 0.008f, 0.008f, 0.03f }, skinColor);
        }

        // Thumb
        Vector3 thumbPos = position;
        thumbPos = Vector3Add(thumbPos, Vector3Scale(right, leftHand ? 0.025f : -0.025f));
        thumbPos = Vector3Add(thumbPos, Vector3Scale(forward, 0.02f));
        DrawCubeV(thumbPos, Vector3{ 0.01f, 0.01f, 0.025f }, skinColor);
    }
};

// Global instances
extern WeaponRenderer* g_WeaponRenderer;
extern HandsRenderer* g_HandsRenderer;
