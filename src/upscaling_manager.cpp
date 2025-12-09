#include "upscaling_manager.h"
#include "rlgl.h"

// Global instance
UpscalingManager* g_UpscalingManager = nullptr;

// Quality scale factors
static const float QUALITY_SCALES[UPSCALE_QUALITY_COUNT] = {
    0.50f,  // Performance
    0.67f,  // Balanced
    0.75f,  // Quality
    0.85f   // Ultra Quality
};

UpscalingManager::UpscalingManager() {
    dlssSupported = false;
    fsrSupported = true; // FSR is software-based, always supported
    currentMode = UPSCALING_NONE;
    currentQuality = UPSCALE_QUALITY_QUALITY;
    sharpness = 0.5f;
    shaderLoaded = false;
    currentRenderWidth = 0;
    currentRenderHeight = 0;
    currentDisplayWidth = 0;
    currentDisplayHeight = 0;
    renderTarget = { 0 };
    upscaleShader = { 0 };
}

UpscalingManager::~UpscalingManager() {
    Unload();
}

void UpscalingManager::Initialize(int displayWidth, int displayHeight) {
    TraceLog(LOG_INFO, "Initializing Upscaling Manager...");
    
    currentDisplayWidth = displayWidth;
    currentDisplayHeight = displayHeight;
    
    // Check for NVIDIA GPU (DLSS support)
    dlssSupported = CheckNVIDIAGPU();
    
    if (dlssSupported) {
        TraceLog(LOG_INFO, "NVIDIA GPU detected - DLSS available");
    } else {
        TraceLog(LOG_INFO, "NVIDIA GPU not detected - DLSS unavailable");
    }
    
    // FSR is always available (software implementation)
    TraceLog(LOG_INFO, "AMD FSR available (software implementation)");
    
    // Load FSR shader if available
    if (LoadFSRShader()) {
        TraceLog(LOG_INFO, "FSR shader loaded successfully");
    } else {
        TraceLog(LOG_WARNING, "FSR shader not found, using fallback upscaling");
    }
    
    TraceLog(LOG_INFO, "Upscaling Manager initialized");
}

bool UpscalingManager::CheckNVIDIAGPU() {
    // Simple heuristic: check OpenGL vendor string
    // In a real implementation, you'd check for NVIDIA DLSS SDK support
    const char* vendor = (const char*)rlGetVersion();
    
    // For now, we'll conservatively return false
    // Real DLSS requires the NVIDIA DLSS SDK which isn't included in raylib
    return false;
}

bool UpscalingManager::LoadFSRShader() {
    // Try to load FSR shader from file
    if (FileExists("assets/shaders/fsr.vs") && FileExists("assets/shaders/fsr.fs")) {
        upscaleShader = LoadShader("assets/shaders/fsr.vs", "assets/shaders/fsr.fs");
        if (upscaleShader.id > 0) {
            shaderLoaded = true;
            return true;
        }
    }
    
    shaderLoaded = false;
    return false;
}

void UpscalingManager::ApplySettings(UpscalingMode mode, UpscalingQuality quality, float sharpnessValue) {
    currentMode = mode;
    currentQuality = quality;
    sharpness = Clamp(sharpnessValue, 0.0f, 1.0f);
    
    // Calculate render resolution
    GetRenderResolution(currentDisplayWidth, currentDisplayHeight, 
                        &currentRenderWidth, &currentRenderHeight);
    
    // Recreate render target if needed
    if (currentMode != UPSCALING_NONE) {
        if (renderTarget.id > 0) {
            UnloadRenderTexture(renderTarget);
        }
        renderTarget = CreateRenderTarget(currentRenderWidth, currentRenderHeight);
    }
    
    const char* modeNames[] = { "None", "FSR", "DLSS" };
    const char* qualityNames[] = { "Performance", "Balanced", "Quality", "Ultra Quality" };
    
    TraceLog(LOG_INFO, "Upscaling settings applied: Mode=%s, Quality=%s, Render=%dx%d, Display=%dx%d",
             modeNames[mode], qualityNames[quality],
             currentRenderWidth, currentRenderHeight,
             currentDisplayWidth, currentDisplayHeight);
}

void UpscalingManager::GetRenderResolution(int displayWidth, int displayHeight, int* renderWidth, int* renderHeight) {
    if (currentMode == UPSCALING_NONE) {
        *renderWidth = displayWidth;
        *renderHeight = displayHeight;
    } else {
        float scale = GetQualityScale(currentQuality);
        *renderWidth = (int)(displayWidth * scale);
        *renderHeight = (int)(displayHeight * scale);
        
        // Ensure even dimensions (required by some upscaling algorithms)
        *renderWidth = (*renderWidth / 2) * 2;
        *renderHeight = (*renderHeight / 2) * 2;
    }
}

float UpscalingManager::GetQualityScale(UpscalingQuality quality) {
    if (quality >= 0 && quality < UPSCALE_QUALITY_COUNT) {
        return QUALITY_SCALES[quality];
    }
    return 1.0f;
}

RenderTexture2D UpscalingManager::CreateRenderTarget(int width, int height) {
    RenderTexture2D target = LoadRenderTexture(width, height);
    
    // Set texture filtering for better quality
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    
    return target;
}

void UpscalingManager::BeginUpscaledRender() {
    if (currentMode != UPSCALING_NONE && renderTarget.id > 0) {
        BeginTextureMode(renderTarget);
    }
}

void UpscalingManager::EndUpscaledRender(int displayWidth, int displayHeight) {
    if (currentMode != UPSCALING_NONE && renderTarget.id > 0) {
        EndTextureMode();
        
        // Now upscale the render target to display resolution
        BeginDrawing();
        
        switch (currentMode) {
            case UPSCALING_FSR:
                ApplyFSR(displayWidth, displayHeight);
                break;
                
            case UPSCALING_DLSS:
                // DLSS would require NVIDIA SDK integration
                // For now, fall back to basic upscaling
                TraceLog(LOG_WARNING, "DLSS not implemented, using fallback");
                ApplyBasicUpscale(displayWidth, displayHeight);
                break;
                
            default:
                ApplyBasicUpscale(displayWidth, displayHeight);
                break;
        }
    }
}

void UpscalingManager::ApplyFSR(int displayWidth, int displayHeight) {
    if (shaderLoaded && upscaleShader.id > 0) {
        // Use FSR shader with sharpening
        BeginShaderMode(upscaleShader);
        
        // Set shader uniforms
        int sharpnessLoc = GetShaderLocation(upscaleShader, "sharpness");
        SetShaderValue(upscaleShader, sharpnessLoc, &sharpness, SHADER_UNIFORM_FLOAT);
        
        int inputSizeLoc = GetShaderLocation(upscaleShader, "inputSize");
        float inputSize[2] = { (float)currentRenderWidth, (float)currentRenderHeight };
        SetShaderValue(upscaleShader, inputSizeLoc, inputSize, SHADER_UNIFORM_VEC2);
        
        int outputSizeLoc = GetShaderLocation(upscaleShader, "outputSize");
        float outputSize[2] = { (float)displayWidth, (float)displayHeight };
        SetShaderValue(upscaleShader, outputSizeLoc, outputSize, SHADER_UNIFORM_VEC2);
        
        // Draw upscaled texture
        DrawTexturePro(
            renderTarget.texture,
            Rectangle{ 0, 0, (float)currentRenderWidth, (float)-currentRenderHeight },
            Rectangle{ 0, 0, (float)displayWidth, (float)displayHeight },
            Vector2{ 0, 0 },
            0.0f,
            WHITE
        );
        
        EndShaderMode();
    } else {
        // Fallback to basic upscaling with manual sharpening
        ApplyBasicUpscale(displayWidth, displayHeight);
    }
}

void UpscalingManager::ApplyBasicUpscale(int displayWidth, int displayHeight) {
    // Simple bilinear upscaling
    DrawTexturePro(
        renderTarget.texture,
        Rectangle{ 0, 0, (float)currentRenderWidth, (float)-currentRenderHeight },
        Rectangle{ 0, 0, (float)displayWidth, (float)displayHeight },
        Vector2{ 0, 0 },
        0.0f,
        WHITE
    );
}

void UpscalingManager::Unload() {
    if (renderTarget.id > 0) {
        UnloadRenderTexture(renderTarget);
        renderTarget = { 0 };
    }
    
    if (shaderLoaded && upscaleShader.id > 0) {
        UnloadShader(upscaleShader);
        upscaleShader = { 0 };
        shaderLoaded = false;
    }
}

// Global initialization
void InitializeUpscalingSystem(int displayWidth, int displayHeight) {
    g_UpscalingManager = new UpscalingManager();
    g_UpscalingManager->Initialize(displayWidth, displayHeight);
    TraceLog(LOG_INFO, "Upscaling system initialized");
}

void CleanupUpscalingSystem() {
    if (g_UpscalingManager) {
        delete g_UpscalingManager;
        g_UpscalingManager = nullptr;
    }
    TraceLog(LOG_INFO, "Upscaling system cleaned up");
}