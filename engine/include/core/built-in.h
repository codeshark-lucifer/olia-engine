#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>

namespace Olia
{

    struct Transform
    {
        glm::vec3 position{0.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 scale{1.0f};
    };

    struct Texture
    {
        GLuint id;
        int width;
        int height;
    };

    struct SpriteRenderer
    {
        Texture *texture = nullptr;
        glm::vec4 color{1.0f};
        glm::vec2 size{64.0f, 64.0f};
        bool useTexCoords = false;
        glm::vec2 texCoords[4] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };
    };

    struct TextComponent
    {
        std::string text;
        glm::vec2 position{0.0f};
        float scale = 1.0f;
        glm::vec4 color{1.0f};
    };

    struct Camera2D
    {
        glm::vec2 position{0.0f};

        float zoom = 1.0f;

        float width = 800.0f;
        float height = 600.0f;
    };

}