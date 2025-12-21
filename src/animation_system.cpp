#include "animation_system.h"

// Global instance
AnimationManager* g_AnimationManager = nullptr;

// AnimationController implementation
AnimationController::AnimationController() {
    currentClip = nullptr;
    currentTime = 0.0f;
    isPlaying = false;
}

AnimationController::~AnimationController() {
    Unload();
}

void AnimationController::Initialize() {
    InitializeClips();
}

void AnimationController::InitializeClips() {
    // Drink animation
    AnimationClip drink;
    drink.type = ANIM_TYPE_DRINK;
    drink.duration = 1.5f;
    drink.loop = false;
    drink.keyTimes = {0.0f, 0.5f, 1.0f, 1.5f};
    drink.keyPositions = {
        Vector3{0, 0, 0},
        Vector3{0, 0.2f, 0},
        Vector3{0, 0.3f, 0},
        Vector3{0, 0, 0}
    };
    drink.keyRotations = {
        Vector3{0, 0, 0},
        Vector3{-30, 0, 0},
        Vector3{-45, 0, 0},
        Vector3{0, 0, 0}
    };
    clips[ANIM_TYPE_DRINK] = drink;
    
    // Pistol reload animation
    AnimationClip reloadPistol;
    reloadPistol.type = ANIM_TYPE_RELOAD_PISTOL;
    reloadPistol.duration = 1.5f;
    reloadPistol.loop = false;
    reloadPistol.keyTimes = {0.0f, 0.3f, 0.7f, 1.0f, 1.5f};
    reloadPistol.keyPositions = {
        Vector3{0, 0, 0},
        Vector3{-0.1f, -0.1f, 0},
        Vector3{-0.05f, -0.05f, 0},
        Vector3{0.05f, 0.05f, 0},
        Vector3{0, 0, 0}
    };
    reloadPistol.keyRotations = {
        Vector3{0, 0, 0},
        Vector3{0, 30, 0},
        Vector3{0, 20, 0},
        Vector3{0, -10, 0},
        Vector3{0, 0, 0}
    };
    clips[ANIM_TYPE_RELOAD_PISTOL] = reloadPistol;
    
    // Rifle reload animation
    AnimationClip reloadRifle;
    reloadRifle.type = ANIM_TYPE_RELOAD_RIFLE;
    reloadRifle.duration = 2.2f;
    reloadRifle.loop = false;
    reloadRifle.keyTimes = {0.0f, 0.5f, 1.0f, 1.5f, 2.2f};
    reloadRifle.keyPositions = {
        Vector3{0, 0, 0},
        Vector3{-0.15f, -0.1f, 0},
        Vector3{-0.1f, -0.05f, 0},
        Vector3{0.05f, 0.05f, 0},
        Vector3{0, 0, 0}
    };
    reloadRifle.keyRotations = {
        Vector3{0, 0, 0},
        Vector3{0, 45, 0},
        Vector3{0, 30, 0},
        Vector3{0, -15, 0},
        Vector3{0, 0, 0}
    };
    clips[ANIM_TYPE_RELOAD_RIFLE] = reloadRifle;
    
    // Idle animation
    AnimationClip idle;
    idle.type = ANIM_TYPE_IDLE;
    idle.duration = 2.0f;
    idle.loop = true;
    idle.keyTimes = {0.0f, 1.0f, 2.0f};
    idle.keyPositions = {
        Vector3{0, 0, 0},
        Vector3{0, 0.01f, 0},
        Vector3{0, 0, 0}
    };
    idle.keyRotations = {
        Vector3{0, 0, 0},
        Vector3{0, 0, 0},
        Vector3{0, 0, 0}
    };
    clips[ANIM_TYPE_IDLE] = idle;
    
    // Draw weapon animation
    AnimationClip draw;
    draw.type = ANIM_TYPE_DRAW_WEAPON;
    draw.duration = 0.5f;
    draw.loop = false;
    draw.keyTimes = {0.0f, 0.25f, 0.5f};
    draw.keyPositions = {
        Vector3{0, -0.3f, 0},
        Vector3{0, -0.1f, 0},
        Vector3{0, 0, 0}
    };
    draw.keyRotations = {
        Vector3{0, 0, 0},
        Vector3{0, 0, 0},
        Vector3{0, 0, 0}
    };
    clips[ANIM_TYPE_DRAW_WEAPON] = draw;
}

void AnimationController::PlayAnimation(AnimationType type, bool loop) {
    if (clips.find(type) != clips.end()) {
        currentClip = &clips[type];
        currentClip->loop = loop;
        currentTime = 0.0f;
        isPlaying = true;
    }
}

void AnimationController::StopAnimation() {
    currentClip = nullptr;
    isPlaying = false;
    currentTime = 0.0f;
}

void AnimationController::Update(float deltaTime) {
    if (!isPlaying || !currentClip) return;
    
    currentTime += deltaTime;
    
    if (currentTime >= currentClip->duration) {
        if (currentClip->loop) {
            currentTime = fmodf(currentTime, currentClip->duration);
        } else {
            isPlaying = false;
            currentTime = currentClip->duration;
        }
    }
}

Vector3 AnimationController::InterpolatePosition(float time) {
    if (!currentClip || currentClip->keyTimes.empty()) return Vector3{0, 0, 0};
    
    // Find keyframes to interpolate between
    for (size_t i = 0; i < currentClip->keyTimes.size() - 1; i++) {
        if (time >= currentClip->keyTimes[i] && time <= currentClip->keyTimes[i + 1]) {
            float t = (time - currentClip->keyTimes[i]) / (currentClip->keyTimes[i + 1] - currentClip->keyTimes[i]);
            return Vector3Lerp(currentClip->keyPositions[i], currentClip->keyPositions[i + 1], t);
        }
    }
    
    return currentClip->keyPositions.back();
}

Vector3 AnimationController::InterpolateRotation(float time) {
    if (!currentClip || currentClip->keyTimes.empty()) return Vector3{0, 0, 0};
    
    for (size_t i = 0; i < currentClip->keyTimes.size() - 1; i++) {
        if (time >= currentClip->keyTimes[i] && time <= currentClip->keyTimes[i + 1]) {
            float t = (time - currentClip->keyTimes[i]) / (currentClip->keyTimes[i + 1] - currentClip->keyTimes[i]);
            return Vector3Lerp(currentClip->keyRotations[i], currentClip->keyRotations[i + 1], t);
        }
    }
    
    return currentClip->keyRotations.back();
}

void AnimationController::GetTransform(Vector3* position, Vector3* rotation) {
    if (!currentClip) {
        *position = Vector3{0, 0, 0};
        *rotation = Vector3{0, 0, 0};
        return;
    }
    
    *position = InterpolatePosition(currentTime);
    *rotation = InterpolateRotation(currentTime);
}

AnimationType AnimationController::GetCurrentAnimation() const {
    return currentClip ? currentClip->type : ANIM_TYPE_NONE;
}

void AnimationController::Unload() {
    clips.clear();
    currentClip = nullptr;
}

// AnimationManager implementation
AnimationManager::AnimationManager() {
    playerController = nullptr;
}

AnimationManager::~AnimationManager() {
    Unload();
}

void AnimationManager::Initialize() {
    playerController = CreateController();
    TraceLog(LOG_INFO, "Animation Manager initialized");
}

AnimationController* AnimationManager::CreateController() {
    AnimationController* controller = new AnimationController();
    controller->Initialize();
    controllers.push_back(controller);
    return controller;
}

void AnimationManager::UpdateAll(float deltaTime) {
    for (auto* controller : controllers) {
        controller->Update(deltaTime);
    }
}

void AnimationManager::PlayAnimation(AnimationType type, bool loop) {
    if (playerController) {
        playerController->PlayAnimation(type, loop);
    }
}

void AnimationManager::Unload() {
    for (auto* controller : controllers) {
        delete controller;
    }
    controllers.clear();
    playerController = nullptr;
}

// Global initialization
void InitializeAnimationSystem() {
    g_AnimationManager = new AnimationManager();
    g_AnimationManager->Initialize();
    TraceLog(LOG_INFO, "Animation system initialized");
}

void CleanupAnimationSystem() {
    if (g_AnimationManager) {
        delete g_AnimationManager;
        g_AnimationManager = nullptr;
    }
    TraceLog(LOG_INFO, "Animation system cleaned up");
}
