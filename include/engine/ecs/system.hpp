#pragma once
#include <engine/ecs/entity.hpp>

class System
{
public:
    System(const char *name = "System") : name(name) {}
    virtual ~System() = default;
    virtual void Update(std::vector<std::shared_ptr<Entity>>& entities, float deltaTime) {}
    virtual void Render(std::vector<std::shared_ptr<Entity>>& entities) {}
    virtual void OnAttach(std::vector<std::shared_ptr<Entity>>& entities) {}
    virtual void OnResize(int w, int h, std::vector<std::shared_ptr<Entity>>& entities) {}

private:
    const char *name = "";
};