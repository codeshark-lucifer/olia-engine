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

float ShadowCalculation(vec4 fragPosLightSpace, vec3 norm, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    float shadow = 0.0;
    // float bias = 0.005; // Old fixed bias
    float bias = max(0.0025 * (1.0 - dot(norm, -lightDir)), 0.00025);
    int samples = 3;
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

void main()
{
    vec3 color = vec3(1.0, 1.0, 1.0);
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDir), 0.0);

    float shadow = ShadowCalculation(FragPosLight, norm, lightDir);
    vec3 lighting = (0.3 + (1.0 - shadow) * diff) * color;

    FragColor = vec4(lighting, 1.0);
}
