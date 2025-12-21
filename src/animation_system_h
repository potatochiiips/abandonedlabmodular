#pragma once
#include "globals.h"

// Animation clip structure
struct AnimationClip {
    AnimationType type;
    float duration;
    bool loop;
    std::vector<Vector3> keyPositions;
    std::vector<Vector3> keyRotations;
    std::vector<float> keyTimes;
};

// Animation controller
class AnimationController {
public:
    AnimationController();
    ~AnimationController();
    
    // Initialize animation system
    void Initialize();
    
    // Play an animation
    void PlayAnimation(AnimationType type, bool loop = false);
    
    // Stop current animation
    void StopAnimation();
    
    // Update animation state
    void Update(float deltaTime);
    
    // Get current animation transform
    void GetTransform(Vector3* position, Vector3* rotation);
    
    // Check if animation is playing
    bool IsPlaying() const { return currentClip != nullptr; }
    
    // Get current animation type
    AnimationType GetCurrentAnimation() const;
    
    // Cleanup
    void Unload();
    
private:
    std::map<AnimationType, AnimationClip> clips;
    AnimationClip* currentClip;
    float currentTime;
    bool isPlaying;
    
    // Initialize animation clips
    void InitializeClips();
    
    // Interpolate between keyframes
    Vector3 InterpolatePosition(float time);
    Vector3 InterpolateRotation(float time);
};

// Animation manager class
class AnimationManager {
public:
    AnimationManager();
    ~AnimationManager();
    
    // Initialize animation system
    void Initialize();
    
    // Create animation controller for entity
    AnimationController* CreateController();
    
    // Update all active animations
    void UpdateAll(float deltaTime);
    
    // Cleanup
    void Unload();
    
    // Play animation (convenience method)
    void PlayAnimation(AnimationType type, bool loop = false);
    
private:
    std::vector<AnimationController*> controllers;
    AnimationController* playerController;
};

// Global animation manager
extern AnimationManager* g_AnimationManager;

// Initialize animation system
void InitializeAnimationSystem();

// Cleanup animation system
void CleanupAnimationSystem();
