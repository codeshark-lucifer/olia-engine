#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <cstdint>
#include <algorithm>
#include <engine/ecs/component.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Scene;
class Entity : public std::enable_shared_from_this<Entity>
{
public:
    using ID = uint64_t;

private:
    inline static std::atomic<ID> s_NextID{1};
    ID id;

public:
    ID getID() const { return id; }

    Entity(const std::string &n) : name(n), id(s_NextID++)
    {
    }
    // transform
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};

    glm::vec3 forward()
    {
        glm::vec3 f = rotation * glm::vec3(0, 0, -1);
        return glm::normalize(f);
    }

    glm::vec3 right()
    {
        glm::vec3 r = rotation * glm::vec3(1, 0, 0);
        return glm::normalize(r);
    }

    glm::mat4 local() const
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::toMat4(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

        return T * R * S;
    }

    glm::mat4 matrix() const
    {
        if (auto p = parent.lock())
            return p->matrix() * local();

        return local();
    }

public:
    std::string name = "Entity";
    std::weak_ptr<Scene> scene;
    std::weak_ptr<Entity> parent;
    std::vector<std::shared_ptr<Entity>> children;
    std::vector<std::shared_ptr<Component>> components;
    // self (contruct, transform, handle(matrix, pos, rot, scale))
    // children (add, remove)
    // parent (set, clear)
    // compoents (get, add)

    void AddChild(const std::shared_ptr<Entity> &entity)
    {
        entity->parent = shared_from_this();
        children.push_back(entity);
    }

    void RemoveChild(const std::shared_ptr<Entity> &entity)
    {
        if (!entity)
            return;

        ID id = entity->getID();

        auto it = std::remove_if(children.begin(), children.end(),
                                 [id](const std::shared_ptr<Entity> &child)
                                 {
                                     return child && child->getID() == id;
                                 });

        if (it != children.end())
        {
            // clear parent link
            if (entity->parent.lock().get() == this)
                entity->parent.reset();

            children.erase(it, children.end());
        }
    }

    // components
    template <typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args &&...args)
    {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        // Prevent duplicate components of same type (Unity-style)
        if (auto existing = GetComponent<T>())
            return existing;

        auto component = std::make_shared<T>(std::forward<Args>(args)...);
        component->entity = shared_from_this(); // if Component has entity reference
        component->OnAttach();
        components.push_back(component);
        return component;
    }

    template <typename T>
    std::shared_ptr<T> GetComponent()
    {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        for (const auto &component : components)
        {
            if (auto casted = std::dynamic_pointer_cast<T>(component))
                return casted;
        }
        return nullptr;
    }
    template <typename T>
    void RemoveComponent()
    {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        components.erase(
            std::remove_if(components.begin(), components.end(),
                           [](const std::shared_ptr<Component> &c)
                           {
                               return std::dynamic_pointer_cast<T>(c) != nullptr;
                           }),
            components.end());
    }

    template <typename T>
    bool HasComponent() const
    {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        for (const auto &component : components)
        {
            if (std::dynamic_pointer_cast<T>(component))
                return true;
        }
        return false;
    }
};