#pragma once
#include <vector>
#include <engine/entity.hpp>
#include <engine/component.hpp>
#include <engine/shader.hpp>
#include <engine/meshrenderer.hpp>
#include <engine/camera.hpp>
#include <engine/buffers/fbo.hpp>
#include <engine/buffers/quad.hpp>
#include <engine/buffers/sbo.hpp>

class Scene
{
public:
    Scene(const std::string &n, int msaaSamples = 1);
    ~Scene() = default;

    void Begin() const;
    void Update(float dt);
    void OnResize(int w, int h) const;

    void AddEntity(std::shared_ptr<Entity> en);
    std::shared_ptr<Entity> CreateEntity(const std::string &n);

private:
    glm::mat4 lightSpaceMatrix = glm::mat4(0.0f);
    std::string name = "";
    std::vector<std::shared_ptr<Entity>> entities;
    std::shared_ptr<Shader> defaultShader = nullptr;
    std::shared_ptr<Camera> defaultCamera = nullptr;
    std::shared_ptr<Shader> defaultFrameBufferShader = nullptr;
    std::shared_ptr<FBO> defaultFrameBuffer = nullptr;
    std::shared_ptr<FBO> intermediateFrameBuffer = nullptr;
    std::shared_ptr<Quad> screen = nullptr;
    int msaaSamples = 1;

    std::shared_ptr<ShadowBuffer> defaultShadowBuffer = nullptr;
    std::shared_ptr<Shader> defaultShadowShader = nullptr;
};