#include "components/mesh.h"
using namespace Engine;

std::vector<VkVertexInputBindingDescription> Mesh::GetBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> descriptions(1);
    descriptions[0].binding = 0;
    descriptions[0].stride = sizeof(Vertex);
    descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return descriptions;
}

std::vector<VkVertexInputAttributeDescription> Mesh::GetAttributeDescriptions()
{
    // FIX: Change vector size from 3 to 5
    std::vector<VkVertexInputAttributeDescription> descriptions(5);

    // Location 0: Position
    descriptions[0].binding = 0;
    descriptions[0].location = 0;
    descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[0].offset = offsetof(Vertex, position);

    // Location 1: Normal
    descriptions[1].binding = 0;
    descriptions[1].location = 1;
    descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[1].offset = offsetof(Vertex, normal);

    // Location 2: UV
    descriptions[2].binding = 0;
    descriptions[2].location = 2;
    descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    descriptions[2].offset = offsetof(Vertex, uv);

    // Location 3: Bone Indices
    descriptions[3].binding = 0;
    descriptions[3].location = 3;
    descriptions[3].format = VK_FORMAT_R32G32B32A32_SINT;
    descriptions[3].offset = offsetof(Vertex, boneIndices);

    // Location 4: Bone Weights
    descriptions[4].binding = 0;
    descriptions[4].location = 4;
    descriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    descriptions[4].offset = offsetof(Vertex, boneWeights);

    return descriptions;
}