#pragma once
#include <vector>
#include <memory>
#include <string>

#include <engine/ecs/entity.hpp>
#include <engine/ecs/system.hpp>

#include <engine/systems/render.hpp>
#include <engine/systems/update.hpp>
#include <engine/systems/physics.hpp>

class Scene : public std::enable_shared_from_this<Scene>
{
public:
    Scene(int width, int height, const char *name)
    {
        this->width = width;
        this->height = height;

        systems.push_back(std::make_shared<PhysicsSystem>());
        systems.push_back(std::make_shared<UpdateSystem>());
        systems.push_back(std::make_shared<RenderSystem>(width, height));
    }
    Scene() = default;
    std::shared_ptr<Entity> CreateObject(const char *name)
    {
        std::cout << "* Create: " << name << std::endl;
        auto entity = std::make_shared<Entity>(name);
        entity->scene = shared_from_this();
        entities.push_back(entity);
        return entity;
    }

    std::shared_ptr<Entity> AddEntity(std::shared_ptr<Entity> entity)
    {
        std::cout << "* Add Enttiy: " << entity->name.c_str() << std::endl;
        entity->scene = shared_from_this();
        entities.push_back(entity);
        return entity;
    }

    void Begin()
    {
        for (auto &s : systems)
        {
            s->OnAttach(entities);
        }
        std::cout << "Begin Scene Loaded." << std::endl;
    }

    void Update(float deltaTime)
    {
        for (auto &s : systems)
        {
            s->Update(entities, deltaTime);
            s->Render(entities);
        }
    }

    void OnResize(int w, int h)
    {
        this->width = w;
        this->height = h;

        for (auto &s : systems)
        {
            s->OnResize(width, height, entities);
        }
    }

private:
    int width = 0, height = 0;
    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<std::shared_ptr<System>> systems;
};
