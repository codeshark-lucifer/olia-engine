#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

layout(push_constant) uniform Push {
    mat4 model; 
    mat4 proj;
    mat4 view;
} push;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragWorldPos;

void main()
{
    vec4 worldPos = push.model * vec4(aPosition, 1.0);
    gl_Position = push.proj * push.view * worldPos;
    fragColor = vec3(0.3, 0.3, 1.0);
    fragNormal = normalize(mat3(push.model) * aNormal);
    fragWorldPos = worldPos.xyz;
}