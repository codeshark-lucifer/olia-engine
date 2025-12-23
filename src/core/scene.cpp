#include <engine/scene.hpp>
#include <engine/ecs/sys/render.hpp>

Scene::Scene(const int &w, const int &h, const std::string &n)
{
    width = w;
    height = h;
    name = n;
    systems.push_back(std::make_unique<RenderSystem>(width, height));
}

Scene::~Scene()
{
    entities.clear();
    systems.clear();
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
    // TODO: Implement
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
