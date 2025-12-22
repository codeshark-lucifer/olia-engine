#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
} vs_out;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos = worldPos.xyz;
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.FragPosLightSpace = lightSpaceMatrix * worldPos;

    gl_Position = projection * view * worldPos;
}
#shader fragment
#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
} fs_in;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.05 * (1.0 - dot(normalize(fs_in.Normal),
                                       normalize(lightPos - fs_in.FragPos))),
                     0.005);

    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

void main()
{
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    // Ambient
    vec3 ambient = 0.15 * lightColor;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Blinn-Phong)
    vec3 halfway = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfway), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);

    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    FragColor = vec4(lighting, 1.0);
}
