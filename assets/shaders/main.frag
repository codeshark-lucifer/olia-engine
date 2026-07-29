#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragWorldPos;

layout(set = 0, binding = 0) buffer LightBuffer
{
    vec4 position; // xyz = position, w=?
    vec4 color;    // rgb = color, w = intensity
} lights[];

layout(set = 0, binding = 1) uniform CameraUBO
{
    vec4 viewPos;
} camera;

void main()
{
    float ambientStrength = 0.1;
    float specularStrength = 0.3;
    vec3 ambient = ambientStrength * lights[0].color.rgb;

    vec3 lightDir = normalize(lights[0].position.xyz - fragWorldPos);  
    
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 diffuse = diff * lights[0].color.rgb * lights[0].color.w;

    vec3 viewDir = normalize(camera.viewPos.xyz - fragWorldPos);
    vec3 reflectDir = reflect(-lightDir, fragNormal);  

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lights[0].color.rgb;  

    vec3 result = (ambient + diffuse + specular) * fragColor;
    outColor = vec4(result, 1.0);
}