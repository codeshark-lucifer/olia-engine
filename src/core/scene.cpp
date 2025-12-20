#include <engine/scene.hpp>
#include <engine/config.h>

#include <engine/systems/update.hpp>
#include <engine/systems/physics.hpp>
#include <engine/systems/render.hpp>

Scene::Scene(const int &w, const int &h, const std::string &n) : width(w), height(h), name(n)
{
    systems.push_back(std::make_unique<PhysicsSystem>());
    systems.push_back(std::make_unique<UpdateSystem>());
    systems.push_back(std::make_unique<RenderSystem>(w, h));
}

void Scene::Begin()
{
    // Recursive lambda using std::function
    std::function<void(const std::shared_ptr<Entity> &)> beginEntity;

    beginEntity = [&](const std::shared_ptr<Entity> &en)
    {
        // Update this entity's components
        for (auto &comp : en->components)
        {
            comp->Awake();
            comp->Start();
        }

        // Update children
        for (auto &child : en->children)
        {
            beginEntity(child);
        }
    };

    // Start from root entities
    for (auto &en : entities)
    {
        beginEntity(en);
    }
}

void Scene::Update(float dt)
{
    for (auto &s : systems)
    {
        s->Update(entities, dt);
    }
}

void Scene::OnResize(const int &w, const int &h)
{
    this->width = w;
    this->height = h;
    for (auto &s : systems)
    {
        s->OnResize(w, h);
    }
}

void Scene::AddEntity(const std::shared_ptr<Entity> &en)
{
    entities.push_back(en);
}