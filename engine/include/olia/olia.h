#pragma once
#include <functional>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <olia/utils/mathf.h>

#include <core/ecs.h>
#include <core/input.h>

#include <renderer/batch.h>
#include <renderer/renderer2D.h>
#include <olia/utils/shader.h>
#include <olia/utils/filesystem.h>
#include <olia/utils/text_renderer.h>
#include <olia/ui/ui.h>

namespace Olia
{
    struct Properties
    {
        const char *app_name = "olia - engine";
    };

    struct GLContext
    {
        GLFWwindow *window = nullptr;
        InputManager *input = nullptr;
        ECS *ecs = nullptr;
        Shader *shader = nullptr;

        Renderer2D *renderer = nullptr;
        TextRenderer *textRenderer = nullptr;

        glm::vec4 backgroundColor{0.1f};

        int virtualWidth = 800;
        int virtualHeight = 600;
    };

    bool Init(int width, int height);
    void SetUp();
    void Loop();

    void Clear();

    void DrawQuad(glm::vec2 pos, glm::vec2 size,
                  glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
                  Texture *texture = nullptr);

    bool InitText(const std::string& fontPath, unsigned int fontSize);
    void RenderText(const std::string& text, float x, float y, float scale = 1.0f, const glm::vec4& color = glm::vec4(1.0f));
    void DrawText(const std::string& text, glm::vec2 pos, float scale = 1.0f, glm::vec4 color = glm::vec4(1.0f));
    float GetTextWidth(const std::string& text, float scale = 1.0f);

    extern GLContext context;
    extern double g_ScrollYDelta;

    extern std::function<void(float)> onPhysicsUpdate;
    extern std::function<void(float)> onAppUpdate;
}