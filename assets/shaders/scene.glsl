#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightViewProjection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    FragPosLightSpace = lightViewProjection * vec4(FragPos, 1.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

#shader fragment
#version 330 core

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D diffuse_texture1;
uniform sampler2D shadowMap;

struct Light {
    int type;           // 0 = Dir, 1 = Point, 2 = Spot
    vec3 direction;
    vec3 position;
    vec3 color;
    float intensity;
    float range;
};

uniform Light lights[16];
uniform int lightCount;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);

void main()
{
    vec3 albedo = texture(diffuse_texture1, TexCoord).rgb;
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = 0.1 * albedo; // ambient once

    for (int i = 0; i < lightCount; i++)
    {
        vec3 lightDir;
        float attenuation = 1.0;
        float shadow = 0.0;

        if (lights[i].type == 0) // Directional
        {
            lightDir = normalize(-lights[i].direction);
            shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
        }
        else if (lights[i].type == 1) // Point
        {
            vec3 toLight = lights[i].position - FragPos;
            float dist = length(toLight);
            if (dist > lights[i].range) continue;

            lightDir = normalize(toLight);
            attenuation = 1.0 / (dist * dist);
        }
        else if (lights[i].type == 2) // Spot
        {
            vec3 toLight = lights[i].position - FragPos;
            float dist = length(toLight);
            if (dist > lights[i].range) continue;

            lightDir = normalize(toLight);
            float theta = dot(lightDir, normalize(-lights[i].direction));
            if (theta < 0.85) continue;

            attenuation = 1.0 / (dist * dist);
            shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
        }

        float diff = max(dot(normal, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

        vec3 diffuse  = diff * albedo * lights[i].color;
        vec3 specular = spec * lights[i].color;
        
        result += (1.0 - shadow) * 
            (diffuse + specular) * 
            lights[i].intensity * 
            attenuation;

    }


    FragColor = vec4(result, 1.0);
}


float ShadowCalculation(vec4 fragPosLightSpace, vec3 norm, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    float shadow = 0.0;
    // float bias = 0.005; // Old fixed bias
    float bias = max(0.0025 * (1.0 - dot(norm, -lightDir)), 0.00025);
    int samples = 8;
    float texelSize = 1.0 / 4096.0;

    for(int x = -samples; x <= samples; ++x)
    {
        for(int y = -samples; y <= samples; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            if(projCoords.z - bias > pcfDepth)
                shadow += 1.0;
        }
    }
    shadow /= pow((samples * 2 + 1), 2);

    return shadow;
}
