#pragma once
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <cstdint>
#include <algorithm>

#include "component.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <engine/ecs/physics.component.hpp> // PhysicsComponent
#include <Bullet3/btBulletDynamicsCommon.h> // btBoxShape
struct Transform
{
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};

    glm::mat4 local() const
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::toMat4(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

        return T * R * S;
    }

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
};
class Scene;
class Entity : public std::enable_shared_from_this<Entity>
{
private:
    using ID = uint64_t;
    inline static std::atomic<ID> s_NextID{1};
    ID id;

public:
    ID getID() const { return id; }

public:
    explicit Entity(const std::string &name)
        : name(name) {}

    /* =========================
       Transform
       ========================= */

    glm::mat4 WorldMatrix() const
    {
        if (auto p = parent.lock())
            return p->WorldMatrix() * transform.local();

        return transform.local();
    }

    /* =========================
       Hierarchy
       ========================= */

    bool HasParent() const { return !parent.expired(); }
    bool HasChildren() const { return !children.empty(); }

    std::shared_ptr<Entity> GetParent() const
    {
        return parent.lock();
    }

    bool IsChildOf(const std::shared_ptr<Entity> &entity) const
    {
        auto p = parent.lock();
        while (p)
        {
            if (p == entity)
                return true;
            p = p->parent.lock();
        }
        return false;
    }

    void AddChild(const std::shared_ptr<Entity> &child)
    {
        if (!child || child.get() == this)
            return;

        if (IsChildOf(child))
            return; // prevent cycles

        child->RemoveFromParent();
        child->parent = shared_from_this();
        children.push_back(child);
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

    void RemoveFromParent()
    {
        if (auto p = parent.lock())
            p->RemoveChild(shared_from_this());
    }

    void ClearChildren()
    {
        for (auto &child : children)
            child->parent.reset();
        children.clear();
    }

    /* =========================
       Components
       ========================= */

    template <typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args &&...args)
    {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        auto comp = std::make_shared<T>(std::forward<Args>(args)...);
        comp->entity = shared_from_this();
        comp->OnAttach();
        components.push_back(std::static_pointer_cast<Component>(comp));



        return comp;
    }

    template <typename T>
    std::shared_ptr<T> GetComponent()
    {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        for (auto &comp : components)
        {
            if (auto casted = std::dynamic_pointer_cast<T>(comp))
                return casted;
        }
        return nullptr;
    }

    template <typename T>
    std::shared_ptr<const T> GetComponent() const
    {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        for (const auto &comp : components)
        {
            if (auto casted = std::dynamic_pointer_cast<const T>(comp))
                return casted;
        }
        return nullptr;
    }

    /* =========================
       Traversal
       ========================= */

    template <typename Func>
    void Traverse(Func func)
    {
        func(shared_from_this());
        for (auto &child : children)
            child->Traverse(func);
    }

    std::vector<std::shared_ptr<Entity>> GetChildren() { return children; }
    std::vector<std::shared_ptr<Component>> GetAllComponents() { return components; }

public:
    std::string name;
    Transform transform;
    std::weak_ptr<Scene> scene;

private:
    std::weak_ptr<Entity> parent;
    std::vector<std::shared_ptr<Entity>> children;
    std::vector<std::shared_ptr<Component>> components;
};
