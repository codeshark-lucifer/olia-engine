#include "vulkan/shader/model.hpp"
#include <vulkan/vulkan.h>

#include <cassert>
#include <cstring>

namespace Engine
{
    std::vector<VkVertexInputBindingDescription> Model::Vertex::GetBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> descriptions(1);
        descriptions[0].binding = 0;
        descriptions[0].stride = sizeof(Vertex);
        descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return descriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescriptions()
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

    Model::Model(EngineDevice &device, const std::vector<Vertex> &vertices)
        : device(device)
    {
        CreateVertexBuffers(vertices);
    }

    Model::~Model()
    {
        vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
        vkFreeMemory(device.device(), vertexBufferMemory, nullptr);
    }

    void Model::CreateVertexBuffers(const std::vector<Vertex> &vertices)
    {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer,
            vertexBufferMemory);

        void *data;
        vkMapMemory(device.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
        std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(device.device(), vertexBufferMemory);
    }

    void Model::BindVertexBuffer(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    void Model::Draw(VkCommandBuffer commandBuffer)
    {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }
}