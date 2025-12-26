#include <engine/scene.hpp>
#include <engine/ecs/sys/render.hpp>
#include <engine/ecs/sys/update.hpp>
#include <engine/ecs/sys/physics.hpp>
#include <iostream>
#include <glm/gtx/string_cast.hpp> // Required for glm::to_string in logging

Scene::Scene(const int &w, const int &h, const std::string &n)
{
    width = w;
    height = h;
    name = n;

    std::cout << "Scene::Scene - Initializing systems." << std::endl;
    systems.push_back(std::make_unique<PhysicsSystem>());
    systems.push_back(std::make_unique<UpdateSystem>());
    systems.push_back(std::make_unique<RenderSystem>(width, height));
    std::cout << "Scene::Scene - Systems initialized." << std::endl;
}

Scene::~Scene()
{
    std::cout << "Scene::~Scene - Destroying entities and systems." << std::endl;
    entities.clear();
    systems.clear();
}

void Scene::Begin()
{
    std::cout << "Scene::Begin - Calling Begin for all systems." << std::endl;
    for (auto &system : systems)
    {
        system->Begin(entities);
    }
    std::cout << "Scene::Begin - All systems began." << std::endl;
}

void Scene::Update(const float &deltaTime)
{
    for (auto &system : systems)
    {
        system->Update(entities, deltaTime);
    }
}

std::shared_ptr<Entity> Scene::AddEntity(const std::shared_ptr<Entity> &entity, std::shared_ptr<Scene> self)
{
    entity->scene = self;
    entities.push_back(entity);
    return entity;
}

void Scene::Destroy(const std::shared_ptr<Entity> &entity)
{
    if (!entity)
        return;

    // 1. Remove from parent if exists
    if (auto parent = entity->parent.lock())
    {
        parent->RemoveChild(entity);
    }

    // 2. Recursively destroy all children
    for (auto &child : entity->children)
    {
        Destroy(child); // recursive call
    }
    entity->children.clear();

    // 3. Clear components
    entity->components.clear();

    // 4. Remove from scene's entity list
    auto it = std::remove_if(entities.begin(), entities.end(),
                             [&entity](const std::shared_ptr<Entity> &e)
                             {
                                 return e && e->getID() == entity->getID();
                             });
    if (it != entities.end())
        entities.erase(it, entities.end());
}

void Scene::OnResize(const int &w, const int &h)
{
    width = w;
    height = h;
    for (auto &system : systems)
    {
        system->OnResize(width, height);
    }
}
