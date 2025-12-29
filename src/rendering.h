#pragma once
#include "globals.h"
#include <vector>
#include <map>
#include <memory>

// ============================================================================
// Upgraded-STYLE RENDERING PIPELINE
// ============================================================================

// Render Queue System (mimics Upgraded's render queue)
enum RenderQueue {
    QUEUE_BACKGROUND = 1000,    // Skybox
    QUEUE_GEOMETRY = 2000,      // Opaque objects
    QUEUE_ALPHA_TEST = 2450,    // Alpha tested geometry
    QUEUE_TRANSPARENT = 3000,   // Transparent objects
    QUEUE_OVERLAY = 4000        // UI/HUD elements
};

// Shader Pass Types (Upgraded's multi-pass rendering)
enum ShaderPassType {
    PASS_FORWARD_BASE,      // Main lighting pass
    PASS_FORWARD_ADD,       // Additional lights
    PASS_SHADOW_CASTER,     // Shadow map generation
    PASS_DEPTH_ONLY,        // Depth pre-pass
    PASS_META              // Lightmap baking (not used in runtime)
};

// Material Property Types
enum MaterialPropertyType {
    PROP_FLOAT,
    PROP_INT,
    PROP_COLOR,
    PROP_VECTOR,
    PROP_TEXTURE,
    PROP_MATRIX
};

// Material Property
struct MaterialProperty {
    std::string name;
    MaterialPropertyType type;
    union {
        float floatValue;
        int intValue;
        float vector[4];
        int textureID;
    } data;
};

// Material (Upgraded-style material system)
class UpgradedMaterial {
public:
    UpgradedMaterial(Shader shader) : shader(shader), renderQueue(QUEUE_GEOMETRY) {}
    
    void SetFloat(const std::string& name, float value) {
        MaterialProperty prop;
        prop.name = name;
        prop.type = PROP_FLOAT;
        prop.data.floatValue = value;
        properties[name] = prop;
    }
    
    void SetInt(const std::string& name, int value) {
        MaterialProperty prop;
        prop.name = name;
        prop.type = PROP_INT;
        prop.data.intValue = value;
        properties[name] = prop;
    }
    
    void SetColor(const std::string& name, Color color) {
        MaterialProperty prop;
        prop.name = name;
        prop.type = PROP_COLOR;
        prop.data.vector[0] = color.r / 255.0f;
        prop.data.vector[1] = color.g / 255.0f;
        prop.data.vector[2] = color.b / 255.0f;
        prop.data.vector[3] = color.a / 255.0f;
        properties[name] = prop;
    }
    
    void SetVector(const std::string& name, Vector4 vec) {
        MaterialProperty prop;
        prop.name = name;
        prop.type = PROP_VECTOR;
        prop.data.vector[0] = vec.x;
        prop.data.vector[1] = vec.y;
        prop.data.vector[2] = vec.z;
        prop.data.vector[3] = vec.w;
        properties[name] = prop;
    }
    
    void SetTexture(const std::string& name, Texture2D texture) {
        MaterialProperty prop;
        prop.name = name;
        prop.type = PROP_TEXTURE;
        prop.data.textureID = texture.id;
        properties[name] = prop;
    }
    
    void SetRenderQueue(RenderQueue queue) {
        renderQueue = queue;
    }
    
    RenderQueue GetRenderQueue() const { return renderQueue; }
    Shader GetShader() const { return shader; }
    
    void ApplyProperties() {
        for (const auto& pair : properties) {
            const MaterialProperty& prop = pair.second;
            int loc = GetShaderLocation(shader, prop.name.c_str());
            
            switch (prop.type) {
                case PROP_FLOAT:
                    SetShaderValue(shader, loc, &prop.data.floatValue, SHADER_UNIFORM_FLOAT);
                    break;
                case PROP_INT:
                    SetShaderValue(shader, loc, &prop.data.intValue, SHADER_UNIFORM_INT);
                    break;
                case PROP_COLOR:
                case PROP_VECTOR:
                    SetShaderValue(shader, loc, prop.data.vector, SHADER_UNIFORM_VEC4);
                    break;
                case PROP_TEXTURE:
                    // Texture binding handled separately
                    break;
            }
        }
    }
    
private:
    Shader shader;
    RenderQueue renderQueue;
    std::map<std::string, MaterialProperty> properties;
};

// Mesh Renderer Component (Upgraded-style)
class MeshRenderer {
public:
    Mesh mesh;
    std::shared_ptr<UpgradedMaterial> material;
    Matrix transform;
    bool castShadows;
    bool receiveShadows;
    bool enabled;
    int layer;
    
    MeshRenderer() : castShadows(true), receiveShadows(true), enabled(true), layer(0) {
        transform = MatrixIdentity();
    }
    
    void SetMaterial(std::shared_ptr<UpgradedMaterial> mat) {
        material = mat;
    }
    
    RenderQueue GetRenderQueue() const {
        return material ? material->GetRenderQueue() : QUEUE_GEOMETRY;
    }
};

// Light Component (Upgraded-style)
enum LightType {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT
};

struct UpgradedLight {
    LightType type;
    Vector3 position;
    Vector3 direction;
    Color color;
    float intensity;
    float range;
    float spotAngle;
    bool castShadows;
    int shadowResolution;
    bool enabled;
    
    UpgradedLight() : type(LIGHT_DIRECTIONAL), intensity(1.0f), range(10.0f), 
                   spotAngle(30.0f), castShadows(true), shadowResolution(1024), enabled(true) {
        position = Vector3{0, 10, 0};
        direction = Vector3{0, -1, 0};
        color = WHITE;
    }
};

// Camera Component (Upgraded-style)
struct UpgradedCamera {
    Camera3D camera;
    RenderTexture2D targetTexture;
    Color clearColor;
    int cullingMask;
    float nearClip;
    float farClip;
    bool clearDepth;
    bool clearStencil;
    int depth; // Camera rendering order
    
    UpgradedCamera() : cullingMask(-1), nearClip(0.1f), farClip(1000.0f), 
                    clearDepth(true), clearStencil(false), depth(0) {
        clearColor = Color{135, 206, 235, 255};
        targetTexture = {0};
    }
};

// Render Command (for sorting)
struct RenderCommand {
    MeshRenderer* renderer;
    float distanceToCamera;
    RenderQueue queue;
    
    bool operator<(const RenderCommand& other) const {
        if (queue != other.queue) return queue < other.queue;
        // Opaque: front to back, Transparent: back to front
        if (queue < QUEUE_TRANSPARENT) {
            return distanceToCamera < other.distanceToCamera;
        } else {
            return distanceToCamera > other.distanceToCamera;
        }
    }
};

// Main Rendering Pipeline Manager
class UpgradedRenderPipeline {
public:
    UpgradedRenderPipeline();
    ~UpgradedRenderPipeline();
    
    void Initialize();
    void Render(const std::vector<MeshRenderer*>& renderers, 
                const std::vector<UpgradedLight*>& lights,
                const UpgradedCamera& mainCamera);
    
    // Shader creation helpers
    Shader CreateStandardShader();
    Shader CreateUnlitShader();
    Shader CreateTransparentShader();
    
    void SetAmbientLight(Color ambient, float intensity);
    void SetFog(bool enabled, Color color, float density);
    
private:
    // Shaders
    Shader standardShader;
    Shader unlitShader;
    Shader transparentShader;
    Shader shadowShader;
    
    // Render passes
    void DepthPrePass(const std::vector<RenderCommand>& commands);
    void ShadowPass(const std::vector<UpgradedLight*>& lights, 
                    const std::vector<MeshRenderer*>& renderers);
    void ForwardBasePass(const std::vector<RenderCommand>& commands, 
                        const UpgradedLight* mainLight);
    void ForwardAddPass(const std::vector<RenderCommand>& commands,
                       const std::vector<UpgradedLight*>& additionalLights);
    void TransparentPass(const std::vector<RenderCommand>& commands);
    
    // Culling
    std::vector<MeshRenderer*> FrustumCull(const std::vector<MeshRenderer*>& renderers,
                                           const UpgradedCamera& camera);
    
    // Lighting
    void SetupLighting(const UpgradedLight* light, Shader shader);
    
    // Settings
    Color ambientColor;
    float ambientIntensity;
    bool fogEnabled;
    Color fogColor;
    float fogDensity;
    
    // Shadow maps
    RenderTexture2D shadowMap;
    Matrix lightViewProjection;
};

// Global rendering pipeline
extern UpgradedRenderPipeline* g_UpgradedPipeline;

void InitializeUpgradedPipeline();
void CleanupUpgradedPipeline();
