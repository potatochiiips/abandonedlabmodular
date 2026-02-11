#pragma once
#include "globals.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

// Forward declarations
enum ModelID;
class UpgradedMaterial;

// Light types for rendering pipeline
enum LightType {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT
};

// Render queue values for material rendering order
enum RenderQueue {
    QUEUE_BACKGROUND = 0,
    QUEUE_GEOMETRY = 1000,
    QUEUE_TRANSPARENT = 2000,
    QUEUE_OVERLAY = 3000
};

// Material structure for advanced rendering
class UpgradedMaterial {
public:
    UpgradedMaterial() : renderQueue(0) {}
    explicit UpgradedMaterial(Shader shader) : renderQueue(0), shader_(shader) {}
    
    void SetRenderQueue(int queue) { renderQueue = queue; }
    void SetTexture(const std::string& uniformName, Texture2D texture) { textures_[uniformName] = texture; }
    void SetColor(const std::string& uniformName, Color color) { colors_[uniformName] = color; }
    
    int GetRenderQueue() const { return renderQueue; }
    Texture2D GetTexture(const std::string& uniformName) const {
        auto it = textures_.find(uniformName);
        return it != textures_.end() ? it->second : Texture2D{0};
    }
    Color GetColor(const std::string& uniformName) const {
        auto it = colors_.find(uniformName);
        return it != colors_.end() ? it->second : WHITE;
    }
    Shader GetShader() const { return shader_; }
    
private:
    int renderQueue;
    Shader shader_;
    std::unordered_map<std::string, Texture2D> textures_;
    std::unordered_map<std::string, Color> colors_;
};

// Light structure for enhanced lighting
struct UpgradedLight {
    LightType type;
    Vector3 position;
    Vector3 direction;
    Color color;
    float intensity;
    float range;           // For point/spot lights
    float spotAngle;       // For spot lights
    bool castShadows;
    int shadowResolution;
};

// Simple mesh renderer structure - consistent declaration as struct
struct MeshRenderer {
    Mesh mesh = { 0 };
    Matrix transform = MatrixIdentity();
    bool enabled = true;
    bool castShadows = true;
    bool receiveShadows = true;
    Color tint = WHITE;
    std::shared_ptr<UpgradedMaterial> material = nullptr;
};

// Model cache with reference counting
struct CachedModel {
    Model model = { 0 };
    int referenceCount = 0;
    bool isLoaded = false;
};

// Advanced rendering pipeline
class UpgradedPipeline {
public:
    UpgradedPipeline();
    ~UpgradedPipeline();

    void Initialize();
    void Cleanup();

    // Lighting
    void SetAmbientLight(Color color, float intensity);
    void SetFog(bool enabled, Color color, float density);
    void AddLight(const UpgradedLight& light);
    void ClearLights();

    // Shader creation
    Shader CreateUnlitShader() const;
    Shader CreateLitShader() const;

    // Rendering
    void BeginFrame();
    void EndFrame();
    void RenderMesh(const MeshRenderer& renderer, Color tint = WHITE);
    void RenderModel(const Vector3& position, int modelId, float scale, Color tint = WHITE);

private:
    Color ambientColor;
    float ambientIntensity;
    bool fogEnabled;
    Color fogColor;
    float fogDensity;
    std::vector<UpgradedLight> lights;
};

// Efficient rendering pipeline (legacy)
class RenderingPipeline {
public:
    RenderingPipeline();
    ~RenderingPipeline();

    void Initialize();
    void Cleanup();

    // Model management
    Model LoadModel(int modelId);
    void UnloadModel(int modelId);
    bool IsModelLoaded(int modelId) const;
    Model GetModel(int modelId);

    // Rendering
    void BeginFrame();
    void EndFrame();
    void RenderMesh(const MeshRenderer& renderer, Color tint = WHITE);
    void RenderModel(const Vector3& position, int modelId, float scale, Color tint = WHITE);

    // Cache management
    void ClearUnusedModels();
    int GetCachedModelCount() const;

private:
    std::unordered_map<int, CachedModel> modelCache;
    Model fallbackModel;

    void CreateFallbackModel();
};

// Global instances
extern RenderingPipeline* g_RenderingPipeline;
extern UpgradedPipeline* g_UpgradedPipeline;

void InitializeRenderingPipeline();
void CleanupRenderingPipeline();
