#include <cstdlib>
#include <glad/gl.h>
#include <olia/olia.h>
#include <stdio.h>

#include <stdexcept>
#include <string>
#include <unordered_set>

namespace Olia
{
  GLContext context{};
  double g_ScrollYDelta = 0.0;

  std::function<void(float)> onPhysicsUpdate = nullptr;
  std::function<void(float)> onAppUpdate = nullptr;

  void framebuffer_size_callback(GLFWwindow* window, int width, int height)
  {
      glViewport(0, 0, width, height);
  }

  void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
  {
      g_ScrollYDelta = yoffset;
  }

  void DrawQuad(glm::vec2 pos, glm::vec2 size,
                glm::vec4 color,
                Texture* texture)
  {
    Entity sprite = context.ecs->Create();

    Transform transform;
    transform.position = glm::vec3(pos.x, pos.y, 0.0f);

    SpriteRenderer renderer;

    renderer.size = {size.x, size.y};

    renderer.color = color;
    renderer.texture = texture;

    context.ecs->Add(sprite, transform);

    context.ecs->Add(sprite, renderer);
  }

  bool Init(int width, int height)
  {
    Properties props;
    if (glfwInit() == GLFW_FALSE)
    {
      fprintf(stderr, "Failed to initialize GLFW.\n");
      return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window =
        glfwCreateWindow(width, height, props.app_name, nullptr, nullptr);
    if (!window)
    {
      glfwTerminate();
      fprintf(stderr, "Failed to create GLFW window.\n");
      return false;
    }

    glfwMakeContextCurrent(window);
    context.window = window;
    context.virtualWidth = width;
    context.virtualHeight = height;

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
      glfwDestroyWindow(window);
      glfwTerminate();
      fprintf(stderr, "Failed to initialize Glad.\n");
      return false;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint) {
        UISystem::CharCallback(codepoint);
    });

    glfwSetScrollCallback(window, scroll_callback);

    glViewport(0, 0, width, height);

    context.shader = new Shader();

    context.ecs = new ECS();
    context.renderer = new Renderer2D();
    context.renderer->Init();
    context.input = new InputManager();
    context.input->Initialize(context.window);

    const char *vertex = R"(

#version 330 core

layout(location=0) in vec3 a_Position;
layout(location=1) in vec4 a_Color;
layout(location=2) in vec2 a_TexCoords;
layout(location=3) in float a_TexIndex;

out vec4 v_Color;
out vec2 v_TexCoords;
flat out float v_TexIndex;

uniform mat4 u_Transform;

void main()
{
    v_Color = a_Color;
    v_TexCoords = a_TexCoords;
    v_TexIndex = a_TexIndex;
    gl_Position = u_Transform * vec4(a_Position, 1.0);
}

)";
    const char *fragment = R"(

#version 330 core

in vec4 v_Color;
in vec2 v_TexCoords;
flat in float v_TexIndex;

out vec4 FragColor;

uniform sampler2D u_Textures[8];

void main()
{
    int index = int(v_TexIndex);
    if (index == 0)
    {
        FragColor = v_Color;
    }
    else
    {
        vec4 texColor = v_Color;
        if (index == 1) texColor = texture(u_Textures[0], v_TexCoords) * v_Color;
        else if (index == 2) texColor = texture(u_Textures[1], v_TexCoords) * v_Color;
        else if (index == 3) texColor = texture(u_Textures[2], v_TexCoords) * v_Color;
        else if (index == 4) texColor = texture(u_Textures[3], v_TexCoords) * v_Color;
        else if (index == 5) texColor = texture(u_Textures[4], v_TexCoords) * v_Color;
        else if (index == 6) texColor = texture(u_Textures[5], v_TexCoords) * v_Color;
        else if (index == 7) texColor = texture(u_Textures[6], v_TexCoords) * v_Color;
        else if (index == 8) texColor = texture(u_Textures[7], v_TexCoords) * v_Color;
        FragColor = texColor;
    }
}

)";

    context.shader->Create(vertex, fragment);

    // Bind texture samplers
    context.shader->Bind();
    for (int i = 0; i < 8; ++i)
    {
        context.shader->SetInt("u_Textures[" + std::to_string(i) + "]", i);
    }

    {
      // Default OBJ

      // setup start all ecs
      Entity camera = context.ecs->Create();
      Camera2D c_cam;
      c_cam.width = static_cast<float>(width);
      c_cam.height = static_cast<float>(height);

      context.ecs->Add(camera, c_cam);
    }

    return true;
  }

  void SetUp() {}

  void handle_input();
  void handle_physics(float dt);
  void handle_update(float dt);
  void handle_render();

  void Loop()
  {
    SetUp();

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(context.window))
    {
      glfwPollEvents();

      double currentTime = glfwGetTime();
      float deltaTime = static_cast<float>(currentTime - lastTime);
      lastTime = currentTime;

      // If deltaTime is too large (e.g. from window dragging, focus loss, or lag),
      // snap it to a standard single-frame step (1/60s) to prevent physics glitches.
      if (deltaTime > 0.1f)
      {
          deltaTime = 1.0f / 60.0f;
      }

      glClearColor(context.backgroundColor.x, context.backgroundColor.y,
                   context.backgroundColor.z, context.backgroundColor.w);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // ecs loop here (input, physics, update, render)
      handle_input();
      handle_physics(deltaTime);
      handle_update(deltaTime);
      handle_render();

      glfwSwapBuffers(context.window);
      g_ScrollYDelta = 0.0;
    }

    Clear();
    glfwTerminate();
  }

  bool InitText(const std::string& fontPath, unsigned int fontSize)
  {
      if (!context.textRenderer)
      {
          context.textRenderer = new TextRenderer();
      }
      return context.textRenderer->LoadFont(fontPath, fontSize);
  }

  void RenderText(const std::string& text, float x, float y, float scale, const glm::vec4& color)
  {
      if (context.textRenderer)
      {
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

          context.textRenderer->RenderText(text, x, y, scale, color);

          glDisable(GL_BLEND);
      }
  }

  void DrawText(const std::string& text, glm::vec2 pos, float scale, glm::vec4 color)
  {
      Entity entity = context.ecs->Create();
      TextComponent textComp;
      textComp.text = text;
      textComp.position = pos;
      textComp.scale = scale;
      textComp.color = color;
      context.ecs->Add(entity, textComp);
  }

  float GetTextWidth(const std::string& text, float scale)
  {
      if (context.textRenderer)
      {
          return context.textRenderer->GetTextWidth(text, scale);
      }
      return 0.0f;
  }

  void Clear()
  {
    if (context.shader)
      delete context.shader;
      
    if (context.input)
      delete context.input;
    if (context.ecs)
      delete context.ecs;
    if (context.textRenderer)
      delete context.textRenderer;

    if (context.window)
    {
      glfwDestroyWindow(context.window);
      context.window = nullptr;
    }
  }

  void handle_input()
  {
    // InputManager
    context.input->Update();
  }

  void handle_physics(float dt)
  {
    if (onPhysicsUpdate)
    {
      onPhysicsUpdate(dt);
    }
  }

  void handle_update(float dt)
  {
    UISystem::Update();
    if (onAppUpdate)
    {
      onAppUpdate(dt);
    }
  }

  void handle_render()
  {
    auto cameras = context.ecs->Query<Camera2D>();
    if (cameras.empty())
      return;

    auto &camera = context.ecs->Get<Camera2D>(cameras.front());

    context.renderer->BeginScene(camera);

    auto sprites = context.ecs->Query<SpriteRenderer>();

    // Collect all scroll view children to skip rendering them here (they are rendered in UIScrollView's scissor block)
    std::unordered_set<Entity> scrollChildren;
    auto scrollViews = context.ecs->Query<UIScrollViewComponent>();
    for (Entity entity : scrollViews)
    {
      auto& sv = context.ecs->Get<UIScrollViewComponent>(entity);
      for (Entity child : sv.children)
      {
        scrollChildren.insert(child);
      }
    }

    for (Entity entity : sprites)
    {
      if (scrollChildren.find(entity) != scrollChildren.end())
        continue;

      auto &transform = context.ecs->Get<Transform>(entity);
      auto &sprite = context.ecs->Get<SpriteRenderer>(entity);

      context.renderer->DrawSprite(transform, sprite);
    }

    context.renderer->EndScene();

    // Render ECS text components
    auto texts = context.ecs->Query<TextComponent>();
    for (Entity entity : texts)
    {
      auto &textComp = context.ecs->Get<TextComponent>(entity);
      RenderText(textComp.text, textComp.position.x, textComp.position.y, textComp.scale, textComp.color);
    }

    // Render UI overlays
    UISystem::Render();
  }

} // namespace Olia