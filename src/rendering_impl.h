#include "Upgraded_rendering.h"
#include "rlgl.h"
#include <algorithm>

UpgradedRenderPipeline* g_UpgradedPipeline = nullptr;

UpgradedRenderPipeline::UpgradedRenderPipeline() {
    ambientColor = Color{54, 58, 66, 255};
    ambientIntensity = 0.3f;
    fogEnabled = false;
    fogColor = Color{128, 128, 128, 255};
    fogDensity = 0.05f;
    shadowMap = {0};
}

UpgradedRenderPipeline::~UpgradedRenderPipeline() {
    if (standardShader.id > 0) UnloadShader(standardShader);
    if (unlitShader.id > 0) UnloadShader(unlitShader);
    if (transparentShader.id > 0) UnloadShader(transparentShader);
    if (shadowShader.id > 0) UnloadShader(shadowShader);
    if (shadowMap.id > 0) UnloadRenderTexture(shadowMap);
}

void UpgradedRenderPipeline::Initialize() {
    TraceLog(LOG_INFO, "Initializing Upgraded-style Render Pipeline...");
    
    // Create standard shaders
    standardShader = CreateStandardShader();
    unlitShader = CreateUnlitShader();
    transparentShader = CreateTransparentShader();
    
    // Create shadow map
    shadowMap = LoadRenderTexture(2048, 2048);
    
    TraceLog(LOG_INFO, "Upgraded Render Pipeline initialized");
}

Shader UpgradedRenderPipeline::CreateStandardShader() {
    const char* vsCode = R"(
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec4 fragColor;

void main() {
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

    const char* fsCode = R"(
#version 330
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Lighting
uniform vec3 lightPos;
uniform vec3 lightDir;
uniform vec4 lightColor;
uniform float lightIntensity;
uniform int lightType; // 0=directional, 1=point, 2=spot

// Ambient
uniform vec4 ambient;
uniform float ambientIntensity;

// Camera
uniform vec3 viewPos;

// Fog
uniform bool fogEnabled;
uniform vec4 fogColor;
uniform float fogDensity;

out vec4 finalColor;

void main() {
    // Sample texture
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 baseColor = texelColor * colDiffuse * fragColor;
    
    // Ambient lighting
    vec3 ambientLight = ambient.rgb * ambientIntensity;
    
    // Calculate lighting direction
    vec3 lightDirection;
    float attenuation = 1.0;
    
    if (lightType == 0) { // Directional
        lightDirection = normalize(-lightDir);
    } else if (lightType == 1) { // Point
        vec3 lightVec = lightPos - fragPosition;
        float distance = length(lightVec);
        lightDirection = normalize(lightVec);
        attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    } else { // Spot
        vec3 lightVec = lightPos - fragPosition;
        float distance = length(lightVec);
        lightDirection = normalize(lightVec);
        attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    }
    
    // Diffuse lighting
    float diff = max(dot(fragNormal, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor.rgb * lightIntensity * attenuation;
    
    // Specular lighting (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 halfwayDir = normalize(lightDirection + viewDir);
    float spec = pow(max(dot(fragNormal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor.rgb * 0.5 * attenuation;
    
    // Combine lighting
    vec3 result = (ambientLight + diffuse + specular) * baseColor.rgb;
    
    // Apply fog
    if (fogEnabled) {
        float dist = length(viewPos - fragPosition);
        float fogFactor = exp(-fogDensity * dist);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        result = mix(fogColor.rgb, result, fogFactor);
    }
    
    finalColor = vec4(result, baseColor.a);
}
)";

    Shader shader = LoadShaderFromMemory(vsCode, fsCode);
    
    // Set default values
    int ambientLoc = GetShaderLocation(shader, "ambient");
    int ambientIntLoc = GetShaderLocation(shader, "ambientIntensity");
    Vector4 amb = {ambientColor.r/255.0f, ambientColor.g/255.0f, ambientColor.b/255.0f, 1.0f};
    SetShaderValue(shader, ambientLoc, &amb, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, ambientIntLoc, &ambientIntensity, SHADER_UNIFORM_FLOAT);
    
    return shader;
}

Shader UpgradedRenderPipeline::CreateUnlitShader() {
    const char* vsCode = R"(
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;

out vec2 fragTexCoord;
out vec4 fragColor;

void main() {
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

    const char* fsCode = R"(
#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    finalColor = texelColor * colDiffuse * fragColor;
}
)";

    return LoadShaderFromMemory(vsCode, fsCode);
}

Shader UpgradedRenderPipeline::CreateTransparentShader() {
    // Similar to standard but with alpha blending considerations
    return CreateStandardShader();
}

void UpgradedRenderPipeline::Render(const std::vector<MeshRenderer*>& renderers,
                                 const std::vector<UpgradedLight*>& lights,
                                 const UpgradedCamera& mainCamera) {
    // 1. Frustum culling
    std::vector<MeshRenderer*> visibleRenderers = FrustumCull(renderers, mainCamera);
    
    // 2. Build render commands and sort
    std::vector<RenderCommand> commands;
    Vector3 camPos = mainCamera.camera.position;
    
    for (MeshRenderer* renderer : visibleRenderers) {
        if (!renderer->enabled) continue;
        
        RenderCommand cmd;
        cmd.renderer = renderer;
        
        // Calculate distance to camera for sorting
        Vector3 objPos = {renderer->transform.m12, renderer->transform.m13, renderer->transform.m14};
        cmd.distanceToCamera = Vector3Distance(camPos, objPos);
        cmd.queue = renderer->GetRenderQueue();
        
        commands.push_back(cmd);
    }
    
    std::sort(commands.begin(), commands.end());
    
    // 3. Begin rendering to target
    if (mainCamera.targetTexture.id > 0) {
        BeginTextureMode(mainCamera.targetTexture);
    }
    
    BeginMode3D(mainCamera.camera);
    
    // Clear
    ClearBackground(mainCamera.clearColor);
    if (mainCamera.clearDepth) {
        rlClearScreenBuffers();
    }
    
    // 4. Shadow pass (if needed)
    std::vector<UpgradedLight*> shadowCasters;
    for (UpgradedLight* light : lights) {
        if (light->enabled && light->castShadows) {
            shadowCasters.push_back(light);
        }
    }
    if (!shadowCasters.empty()) {
        ShadowPass(shadowCasters, visibleRenderers);
    }
    
    // 5. Depth pre-pass for opaque objects (optional optimization)
    // DepthPrePass(commands);
    
    // 6. Forward rendering
    UpgradedLight* mainLight = nullptr;
    std::vector<UpgradedLight*> additionalLights;
    
    for (UpgradedLight* light : lights) {
        if (!light->enabled) continue;
        
        if (light->type == LIGHT_DIRECTIONAL && !mainLight) {
            mainLight = light;
        } else {
            additionalLights.push_back(light);
        }
    }
    
    // Main lighting pass
    ForwardBasePass(commands, mainLight);
    
    // Additional lights pass
    if (!additionalLights.empty()) {
        ForwardAddPass(commands, additionalLights);
    }
    
    EndMode3D();
    
    if (mainCamera.targetTexture.id > 0) {
        EndTextureMode();
    }
}

void UpgradedRenderPipeline::ForwardBasePass(const std::vector<RenderCommand>& commands,
                                          const UpgradedLight* mainLight) {
    // Render opaque and alpha-tested geometry
    for (const RenderCommand& cmd : commands) {
        if (cmd.queue >= QUEUE_TRANSPARENT) continue;
        
        MeshRenderer* renderer = cmd.renderer;
        if (!renderer->material) continue;
        
        Shader shader = renderer->material->GetShader();
        BeginShaderMode(shader);
        
        // Apply material properties
        renderer->material->ApplyProperties();
        
        // Setup lighting
        if (mainLight) {
            SetupLighting(mainLight, shader);
        }
        
        // Draw mesh with transform
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(renderer->transform));
        DrawMesh(renderer->mesh, LoadMaterialDefault(), MatrixIdentity());
        rlPopMatrix();
        
        EndShaderMode();
    }
}

void UpgradedRenderPipeline::TransparentPass(const std::vector<RenderCommand>& commands) {
    // Enable blending
    rlEnableColorBlend();
    rlSetBlendMode(BLEND_ALPHA);
    
    // Render transparent objects (back to front)
    for (const RenderCommand& cmd : commands) {
        if (cmd.queue < QUEUE_TRANSPARENT) continue;
        
        MeshRenderer* renderer = cmd.renderer;
        if (!renderer->material) continue;
        
        Shader shader = renderer->material->GetShader();
        BeginShaderMode(shader);
        
        renderer->material->ApplyProperties();
        
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(renderer->transform));
        DrawMesh(renderer->mesh, LoadMaterialDefault(), MatrixIdentity());
        rlPopMatrix();
        
        EndShaderMode();
    }
    
    rlDisableColorBlend();
}

void UpgradedRenderPipeline::SetupLighting(const UpgradedLight* light, Shader shader) {
    int typeLoc = GetShaderLocation(shader, "lightType");
    int posLoc = GetShaderLocation(shader, "lightPos");
    int dirLoc = GetShaderLocation(shader, "lightDir");
    int colorLoc = GetShaderLocation(shader, "lightColor");
    int intensityLoc = GetShaderLocation(shader, "lightIntensity");
    
    int type = (int)light->type;
    SetShaderValue(shader, typeLoc, &type, SHADER_UNIFORM_INT);
    SetShaderValue(shader, posLoc, &light->position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, dirLoc, &light->direction, SHADER_UNIFORM_VEC3);
    
    Vector4 color = {light->color.r/255.0f, light->color.g/255.0f, 
                     light->color.b/255.0f, light->color.a/255.0f};
    SetShaderValue(shader, colorLoc, &color, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, intensityLoc, &light->intensity, SHADER_UNIFORM_FLOAT);
}

std::vector<MeshRenderer*> UpgradedRenderPipeline::FrustumCull(
    const std::vector<MeshRenderer*>& renderers,
    const UpgradedCamera& camera) {
    // Simple distance culling for now
    std::vector<MeshRenderer*> visible;
    Vector3 camPos = camera.camera.position;
    
    for (MeshRenderer* renderer : renderers) {
        Vector3 objPos = {renderer->transform.m12, renderer->transform.m13, renderer->transform.m14};
        float distance = Vector3Distance(camPos, objPos);
        
        if (distance < camera.farClip) {
            visible.push_back(renderer);
        }
    }
    
    return visible;
}

void UpgradedRenderPipeline::ShadowPass(const std::vector<UpgradedLight*>& lights,
                                     const std::vector<MeshRenderer*>& renderers) {
    // Shadow rendering implementation would go here
    // This is a simplified version
}

void UpgradedRenderPipeline::ForwardAddPass(const std::vector<RenderCommand>& commands,
                                         const std::vector<UpgradedLight*>& additionalLights) {
    // Additional lights with additive blending
    rlSetBlendMode(BLEND_ADDITIVE);
    
    for (const UpgradedLight* light : additionalLights) {
        for (const RenderCommand& cmd : commands) {
            if (cmd.queue >= QUEUE_TRANSPARENT) continue;
            
            MeshRenderer* renderer = cmd.renderer;
            if (!renderer->material) continue;
            
            Shader shader = renderer->material->GetShader();
            BeginShaderMode(shader);
            
            SetupLighting(light, shader);
            
            rlPushMatrix();
            rlMultMatrixf(MatrixToFloat(renderer->transform));
            DrawMesh(renderer->mesh, LoadMaterialDefault(), MatrixIdentity());
            rlPopMatrix();
            
            EndShaderMode();
        }
    }
    
    rlSetBlendMode(BLEND_ALPHA);
}

void UpgradedRenderPipeline::DepthPrePass(const std::vector<RenderCommand>& commands) {
    // Depth pre-pass for early-z culling
    // Would render all opaque geometry with depth-only shader
}

void UpgradedRenderPipeline::SetAmbientLight(Color ambient, float intensity) {
    ambientColor = ambient;
    ambientIntensity = intensity;
}

void UpgradedRenderPipeline::SetFog(bool enabled, Color color, float density) {
    fogEnabled = enabled;
    fogColor = color;
    fogDensity = density;
}

void InitializeUpgradedPipeline() {
    g_UpgradedPipeline = new UpgradedRenderPipeline();
    g_UpgradedPipeline->Initialize();
    TraceLog(LOG_INFO, "Upgraded-style render pipeline initialized");
}

void CleanupUpgradedPipeline() {
    if (g_UpgradedPipeline) {
        delete g_UpgradedPipeline;
        g_UpgradedPipeline = nullptr;
    }
    TraceLog(LOG_INFO, "Upgraded-style render pipeline cleaned up");
}
