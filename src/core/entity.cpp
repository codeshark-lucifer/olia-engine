#include <engine/utils/entity.hpp>
#include <engine/shader.hpp>
#include <engine/meshrenderer.hpp>

#include <iostream>

std::shared_ptr<Entity> Entity::Create(const std::string &name)
{
    auto e = std::shared_ptr<Entity>(new Entity(name));
    e->transform->entity = e;
    return e;
}

Entity::Entity(const std::string &name)
    : name(name)
{
    transform = std::make_unique<Transform>();
}

void Entity::AddChild(const std::shared_ptr<Entity> &child)
{
    if (!child)
        return;
    child->parent = shared_from_this();
    children.push_back(child);
}

void Entity::Begin() const
{
    for (auto &c : components)
    {
        c->Awake();
        c->Start();
    }
}


