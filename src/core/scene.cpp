#include <engine/scene.hpp>
#include <engine/ecs/sys/render.hpp>
#include <iostream>
#include <exception>

Scene::Scene(const int &w, const int &h, const std::string &n) : width(w), height(h), name(n)
{
    std::cout << "Setting Scene\n";
    try
    {
        systems.push_back(std::make_unique<RenderSystem>(w, h));
    }
    catch (const std::exception &e)
    {
        std::cerr << "RenderSystem creation failed: " << e.what() << "\n";
    }

    std::cout << "Setup Systems\n";

    for (auto &s : systems)
    {
        s->Begin();
    }
}

Scene::~Scene() = default;

void Scene::Update(const float &deltaTime)
{
    for (auto &s : systems)
    {
        s->Update(entities, deltaTime);
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

    // 1. Detach from parent
    if (auto parent = entity->parent.lock())
    {
        parent->RemoveChild(entity);
    }

    // 2. Handle children (destroy recursively OR reparent)
    // Option A: Destroy children too (Unity-style)
    for (auto &child : entity->children)
    {
        if (child)
            Destroy(child);
    }
    entity->children.clear();

    // 3. Clear components
    entity->components.clear();

    // 4. Remove from scene list
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
                       [&](const std::shared_ptr<Entity> &e)
                       {
                           return e && e->getID() == entity->getID();
                       }),
        entities.end());

    // 5. Break scene link
    entity->scene.reset();
}
