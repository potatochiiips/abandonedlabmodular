#pragma once
#include "globals.h"

// Forward declarations (enums already defined in globals.h)
class UpscalingManager;

// Upscaling manager class
class UpscalingManager {
public:
    UpscalingManager();
    ~UpscalingManager();

    // Initialize upscaling system
    void Initialize(int displayWidth, int displayHeight);

    // Check hardware support
    bool IsDLSSSupported() const { return dlssSupported; }
    bool IsFSRSupported() const { return fsrSupported; }

    // Apply upscaling settings
    void ApplySettings(UpscalingMode mode, UpscalingQuality quality, float sharpness = 0.5f);

    // Get render resolution based on upscaling settings
    void GetRenderResolution(int displayWidth, int displayHeight, int* renderWidth, int* renderHeight);

    // Get quality scaling factor
    float GetQualityScale(UpscalingQuality quality);

    // Get current mode info
    UpscalingMode GetCurrentMode() const { return currentMode; }
    UpscalingQuality GetCurrentQuality() const { return currentQuality; }
    float GetSharpness() const { return sharpness; }

    // Render pipeline integration
    RenderTexture2D CreateRenderTarget(int width, int height);
    void BeginUpscaledRender();
    void EndUpscaledRender(int displayWidth, int displayHeight);

    // Cleanup
    void Unload();

private:
    bool dlssSupported;
    bool fsrSupported;
    UpscalingMode currentMode;
    UpscalingQuality currentQuality;
    float sharpness;

    RenderTexture2D renderTarget;
    Shader upscaleShader;
    bool shaderLoaded;

    int currentRenderWidth;
    int currentRenderHeight;
    int currentDisplayWidth;
    int currentDisplayHeight;

    // Check for NVIDIA GPU (for DLSS)
    bool CheckNVIDIAGPU();

    // Load FSR shader
    bool LoadFSRShader();

    // Apply FSR upscaling (software implementation)
    void ApplyFSR(int displayWidth, int displayHeight);

    // Apply basic bilinear upscaling (fallback)
    void ApplyBasicUpscale(int displayWidth, int displayHeight);
};

// Global upscaling manager instance
extern UpscalingManager* g_UpscalingManager;

// Initialize upscaling system
void InitializeUpscalingSystem(int displayWidth, int displayHeight);

// Cleanup upscaling system
void CleanupUpscalingSystem();