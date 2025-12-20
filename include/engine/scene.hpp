#pragma once
#include <engine/systems/system.hpp>
#include <engine/ec/entity.hpp>
#include <engine/shader.hpp>
#include <functional>

class Scene
{
public:
    Scene(const int &w, const int &h, const std::string &n);
    void Begin();
    void Update(float dt);
    void OnResize(const int &w, const int &h);
    void AddEntity(const std::shared_ptr<Entity> &en);
    const std::vector<std::shared_ptr<Entity>> &GetEntities() const { return entities; }

private:
    int width = 0,
        height = 0;
    std::string name = "SampleScene";
    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<std::unique_ptr<System>> systems;
};