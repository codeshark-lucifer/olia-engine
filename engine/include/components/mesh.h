#pragma once
#include "./structure/data.h"
#include <vulkan/vulkan.h>

struct Mesh
{
    Mesh() = default;

    // Mesh(const Mesh &) = delete;
    // Mesh &operator=(const Mesh &) = delete;

    std::vector<Engine::Vertex> vertices;
    std::vector<uint16_t> indices;

    static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};