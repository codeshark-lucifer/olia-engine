#include "components/mesh-renderer.h"
#include <cstring>
#include <cassert>

void MeshRenderer::Setup(Engine::EngineDevice* devicePtr)
{
    device = devicePtr;
    CreateVertexBuffers(mesh->vertices);
    CreateIndicesBuffers(mesh->indices);
}

void MeshRenderer::BindBuffers(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = { vertexBuffer->getBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);
}

void MeshRenderer::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDrawIndexed(commandBuffer, indicesCount, 1, 0, 0, 0);
}

void MeshRenderer::CreateVertexBuffers(const std::vector<Engine::Vertex>& vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");

    // Create a buffer with one "instance" per vertex (or use 1 instance with total size).
    // Using instanceCount = vertexCount and instanceSize = sizeof(Vertex) keeps alignment simple.
    vertexBuffer = std::make_unique<Engine::EngineBuffer>(
        *device,                                     // dereference pointer
        sizeof(Engine::Vertex),                     // instance size
        vertexCount,                                // instance count
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        1                                           // minOffsetAlignment (1 is fine for vertex data)
    );

    // Map the whole buffer, copy vertex data, then unmap.
    VkResult result = vertexBuffer->map();
    assert(result == VK_SUCCESS);
    vertexBuffer->writeToBuffer(
        const_cast<void*>(static_cast<const void*>(vertices.data())),
        sizeof(Engine::Vertex) * vertexCount,
        0
    );
    vertexBuffer->unmap();
}

void MeshRenderer::CreateIndicesBuffers(const std::vector<uint16_t>& indices)
{
    indicesCount = static_cast<uint32_t>(indices.size());
    assert(indicesCount >= 3 && "Indices count must be at least 3");

    indexBuffer = std::make_unique<Engine::EngineBuffer>(
        *device,
        sizeof(uint16_t),                          // each index is one instance
        indicesCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        1
    );

    VkResult result = indexBuffer->map();
    assert(result == VK_SUCCESS);
    indexBuffer->writeToBuffer(
        const_cast<void*>(static_cast<const void*>(indices.data())),
        sizeof(uint16_t) * indicesCount,
        0
    );
    indexBuffer->unmap();
}