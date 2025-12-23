#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpace;

out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLight;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPosLight = lightSpace * vec4(FragPos, 1.0);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

#shader fragment
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLight;

uniform vec3 lightDir;
uniform vec3 viewPos;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLight)
{
    vec3 projCoords = fragPosLight.xyz / fragPosLight.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closest = texture(shadowMap, projCoords.xy).r;
    float current = projCoords.z;

    float bias = 0.005;
    return current - bias > closest ? 0.5 : 1.0;
}

void main()
{
    vec3 color = vec3(1.0, 0.7, 0.4);

    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDir), 0.0);

    float shadow = ShadowCalculation(FragPosLight);
    vec3 lighting = color * diff * shadow + color * 0.15;

    FragColor = vec4(lighting, 1.0);
}
