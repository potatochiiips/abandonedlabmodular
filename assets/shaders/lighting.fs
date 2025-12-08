#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Lighting uniforms
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 ambientColor;
uniform float ambientIntensity;

// Flashlight uniforms
uniform vec3 flashlightPos;
uniform vec3 flashlightDir;
uniform vec3 flashlightColor;
uniform float flashlightIntensity;
uniform float flashlightCutoff;
uniform float flashlightOuterCutoff;
uniform int flashlightEnabled;

// Fog uniforms
uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogStart;
uniform float fogEnd;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Sample texture
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    // Base color
    vec3 color = texelColor.rgb * colDiffuse.rgb * fragColor.rgb;
    
    // Ambient lighting
    vec3 ambient = ambientColor * ambientIntensity;
    
    // Diffuse lighting (directional light)
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = lightColor * diff * lightIntensity;
    
    // Specular lighting
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = lightColor * spec * 0.3;
    
    // Start with ambient + diffuse + specular
    vec3 result = (ambient + diffuse + specular) * color;
    
    // Flashlight (spotlight effect)
    if (flashlightEnabled == 1) {
        vec3 flashDir = normalize(flashlightPos - fragPosition);
        float theta = dot(flashDir, normalize(-flashlightDir));
        float epsilon = flashlightCutoff - flashlightOuterCutoff;
        float intensity = clamp((theta - flashlightOuterCutoff) / epsilon, 0.0, 1.0);
        
        if (theta > flashlightOuterCutoff) {
            // Calculate attenuation
            float distance = length(flashlightPos - fragPosition);
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
            
            // Flashlight diffuse
            float flashDiff = max(dot(normal, flashDir), 0.0);
            vec3 flashDiffuse = flashlightColor * flashDiff * flashlightIntensity * intensity * attenuation;
            
            // Flashlight specular
            vec3 flashReflect = reflect(-flashDir, normal);
            float flashSpec = pow(max(dot(viewDir, flashReflect), 0.0), 32.0);
            vec3 flashSpecular = flashlightColor * flashSpec * 0.5 * intensity * attenuation;
            
            result += (flashDiffuse + flashSpecular) * color;
        }
    }
    
    // Fog calculation (exponential)
    float distance = length(viewPos - fragPosition);
    float fogFactor = 0.0;
    
    if (distance > fogStart) {
        fogFactor = (distance - fogStart) / (fogEnd - fogStart);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        fogFactor = fogFactor * fogFactor; // Squared for smoother transition
    }
    
    result = mix(result, fogColor, fogFactor * fogDensity);
    
    // Output final color
    finalColor = vec4(result, texelColor.a * colDiffuse.a * fragColor.a);
}