#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
layout(location = 3) in ivec4 aBoneIndices;
layout(location = 4) in vec4 aBoneWeights;

layout(push_constant) uniform Push {
    mat4 model; 
    mat4 proj;
    mat4 view;
} push;

// Storage Buffer or Uniform Buffer for Final Bone Matrices
layout(set = 0, binding = 4) readonly buffer BoneBuffer
{
    mat4 finalBoneMatrices[];
};

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec2 fragUV;

void main()
{
    // Compute Skinning Matrix
    mat4 boneTransform = mat4(0.0);

    float totalWeight = aBoneWeights.x + aBoneWeights.y + aBoneWeights.z + aBoneWeights.w;

    if (totalWeight > 0.0)
    {
        boneTransform += finalBoneMatrices[aBoneIndices.x] * aBoneWeights.x;
        boneTransform += finalBoneMatrices[aBoneIndices.y] * aBoneWeights.y;
        boneTransform += finalBoneMatrices[aBoneIndices.z] * aBoneWeights.z;
        boneTransform += finalBoneMatrices[aBoneIndices.w] * aBoneWeights.w;
    }
    else
    {
        boneTransform = mat4(1.0); // Static mesh fallback
    }

    vec4 localPosition = boneTransform * vec4(aPosition, 1.0);
    vec4 worldPos = push.model * localPosition;

    gl_Position = push.proj * push.view * worldPos;

    mat3 normalMatrix = transpose(inverse(mat3(push.model * boneTransform)));
    fragNormal = normalize(normalMatrix * aNormal);
    fragWorldPos = worldPos.xyz;
    fragUV = aUV;
}