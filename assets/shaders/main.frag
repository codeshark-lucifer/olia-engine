#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec2 fragUV;

layout(push_constant) uniform Push {
    mat4 model; 
    mat4 proj;
    mat4 view;
} push;

struct GPULight
{
    vec4 position;      // xyz = position
    vec4 color;         // rgb = color, w = intensity
};

layout(set = 0, binding = 0) readonly buffer LightBuffer
{
    GPULight lights[];
};

layout(set = 0, binding = 1) uniform CameraUBO
{
    vec4 viewPos;       // xyz = camera position
} camera;

layout(set = 0, binding = 2) uniform MaterialUBO
{
    vec4 color;
    float roughness;
    float metallic;
    float _padding;
    float padding_;
} material;

layout(set = 0, binding = 3) uniform sampler2D albedoMap;

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lights[0].color.rgb;

    vec3 norm = fragNormal;
    vec3 lightDir = normalize(lights[0].position.xyz - fragWorldPos); 

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lights[0].color.rgb;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(camera.viewPos.xyz - fragWorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);  

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lights[0].color.rgb;

    vec4 texColor = texture(albedoMap, fragUV);

    vec3 result = (ambient + diffuse + specular);
    outColor = vec4(result, 1.0) * material.color * texColor;
}