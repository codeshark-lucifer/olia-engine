#pragma once
#include <memory>
#include <engine/ecs/entity.hpp>
#include <engine/ecs/system.hpp>

class Scene : public std::enable_shared_from_this<Scene>
{
public:
    Scene(const int &w, const int &h, const std::string &n);
    ~Scene();

    void Update(const float &deltaTime);
    std::shared_ptr<Entity> AddEntity(const std::shared_ptr<Entity> &entity, std::shared_ptr<Scene> self);
    void Destroy(const std::shared_ptr<Entity> &entity);
    void OnResize(const int &w, const int &h);

private:
    std::string name = "Scene";
    int width = 0, height = 0;
    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<std::unique_ptr<System>> systems;
};