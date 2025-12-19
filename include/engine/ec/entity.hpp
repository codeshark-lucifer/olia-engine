#pragma once
#include <engine/ec/component.hpp>
#include <engine/components/transform.hpp>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

class Entity : public std::enable_shared_from_this<Entity>
{
public:
    std::unique_ptr<Transform> transform;
    std::weak_ptr<Entity> parent;
    std::vector<std::shared_ptr<Entity>> children;
    std::vector<std::shared_ptr<Component>> components;
    std::string name = "New Entity";

    static std::shared_ptr<Entity> Create(const std::string &name)
    {
        auto en = std::make_shared<Entity>(name);
        en->transform->entity = en;
        return en;
    }

    void SetParent(const std::shared_ptr<Entity> &newParent)
    {
        auto self = shared_from_this();

        // Remove from current parent
        if (auto currentParent = parent.lock())
        {
            currentParent->RemoveChild(self);
        }

        // Set new parent
        parent.reset();
        if (newParent)
        {
            parent = newParent;
            newParent->children.push_back(self);
        }
    }

    void RemoveChild(const std::shared_ptr<Entity> &child)
    {
        if (!child)
            return;

        auto it = std::remove_if(children.begin(), children.end(),
                                 [&](const std::shared_ptr<Entity> &c)
                                 {
                                     return c == child;
                                 });

        if (it != children.end())
        {
            // Clear child's parent
            child->parent.reset();
            children.erase(it, children.end());
        }
    }

    std::shared_ptr<Entity> GetParent()
    {
        return parent.lock();
    }

    void AddChild(const std::shared_ptr<Entity> &child)
    {
        if (!child)
            return;
        child->parent = shared_from_this();
        children.push_back(child);
    }

    template <typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args &&...args)
    {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        auto comp = std::make_shared<T>(std::forward<Args>(args)...);
        comp->entity = shared_from_this();
        components.push_back(comp);
        return comp;
    }

    template <typename T>
    std::vector<std::shared_ptr<T>> GetComponents() const
    {
        std::vector<std::shared_ptr<T>> result;
        for (auto &comp : components)
        {
            if (auto casted = std::dynamic_pointer_cast<T>(comp))
                result.push_back(casted);
        }
        return result;
    }

    template <typename T>
    std::shared_ptr<T> GetComponent()
    {
        for (auto &comp : components)
        {
            if (auto casted = std::dynamic_pointer_cast<T>(comp))
                return casted;
        }
        return nullptr;
    }

    template <typename T>
    std::vector<std::shared_ptr<T>> GetComponentsInchild() const
    {
        std::vector<std::shared_ptr<T>> result;
        for (auto &child : children)
        {
            for (auto &comp : child->components)
            {
                if (auto casted = std::dynamic_pointer_cast<T>(comp))
                    result.push_back(casted);
            }
        }
        return result;
    }

    Entity(const std::string &n) : name(n)
    {
        transform = std::make_unique<Transform>();
    }
};