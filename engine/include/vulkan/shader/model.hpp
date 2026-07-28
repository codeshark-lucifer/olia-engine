#pragma once
#include "vulkan/device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace Engine
{
    class Model
    {
    public:
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 uv;

            static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
        };

        Model(EngineDevice &device, const std::vector<Vertex> &vertices);
        ~Model();

        Model(const Model &) = delete;
        Model &operator=(const Model &) = delete;

        void BindVertexBuffer(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);

    private:
        void CreateVertexBuffers(const std::vector<Vertex> &vertices);

        EngineDevice &device;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
} // namespace Engine
