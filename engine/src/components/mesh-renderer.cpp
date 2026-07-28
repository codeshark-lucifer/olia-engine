#include "components/mesh-renderer.h"
#include <cstring>

void MeshRenderer::BindBuffers(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
}

void MeshRenderer::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDrawIndexed(commandBuffer, indicesCount, 1, 0, 0, 0);
}

void MeshRenderer::CreateVertexBuffers(const std::vector<Engine::Vertex> &vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

    device->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer,
        vertexBufferMemory);

    void *data;
    vkMapMemory(device->device(), vertexBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device->device(), vertexBufferMemory);
}

void MeshRenderer::CreateIndicesBuffers(const std::vector<uint16_t> &indices)
{
    indicesCount = static_cast<uint32_t>(indices.size());
    assert(indicesCount >= 3 && "Indices count must be at least 3");
    VkDeviceSize bufferSize = sizeof(indices[0]) * indicesCount;
    
    device->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT, // Fixed: Use index buffer bit
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexBuffer,
        indexBufferMemory
    );

    void *data;
    vkMapMemory(device->device(), indexBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device->device(), indexBufferMemory);
}