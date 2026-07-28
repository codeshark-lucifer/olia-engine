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
    std::vector<VkVertexInputAttributeDescription> descriptions(3);
    descriptions[0].binding = 0;
    descriptions[0].location = 0;
    descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[0].offset = offsetof(Vertex, position);

    descriptions[1].binding = 0;
    descriptions[1].location = 1;
    descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[1].offset = offsetof(Vertex, normal);

    descriptions[2].binding = 0;
    descriptions[2].location = 2;
    descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    descriptions[2].offset = offsetof(Vertex, uv);
    return descriptions;
}

