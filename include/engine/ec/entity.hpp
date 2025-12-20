#pragma once
#include <engine/ec/component.hpp>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

class Entity; // forward declaration

struct Transform
{
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    std::weak_ptr<Entity> entity;

    Transform() = default;
    explicit Transform(const std::shared_ptr<Entity> &en) : entity(en) {}

    glm::mat4 GetLocalMatrix() const
    {
        glm::mat4 mat(1.0f);
        mat = glm::translate(mat, position);
        mat *= glm::toMat4(glm::quat(rotation));
        mat = glm::scale(mat, scale);
        return mat;
    }

    glm::mat4 GetWorldMatrix() const;
};

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
        en->transform = std::make_unique<Transform>(en); // now Transform is known
        return en;
    }

    void SetParent(const std::shared_ptr<Entity> &newParent)
    {
        auto self = shared_from_this();

        if (auto currentParent = parent.lock())
            currentParent->RemoveChild(self);

        if (newParent)
        {
            parent = newParent;
            newParent->children.push_back(self);
        }
        else
            parent.reset();
    }

    void RemoveChild(const std::shared_ptr<Entity> &child)
    {
        if (!child) return;
        auto it = std::remove_if(children.begin(), children.end(),
                                 [&](const std::shared_ptr<Entity> &c) { return c == child; });
        if (it != children.end())
        {
            child->parent.reset();
            children.erase(it, children.end());
        }
    }

    std::shared_ptr<Entity> GetParent() { return parent.lock(); }

    void AddChild(const std::shared_ptr<Entity> &child)
    {
        if (!child) return;
        child->SetParent(shared_from_this());
    }

    template <typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args &&...args)
    {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
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
            if (auto casted = std::dynamic_pointer_cast<T>(comp))
                result.push_back(casted);
        return result;
    }

    template <typename T>
    std::shared_ptr<T> GetComponent()
    {
        for (auto &comp : components)
            if (auto casted = std::dynamic_pointer_cast<T>(comp))
                return casted;
        return nullptr;
    }

    template <typename T>
    std::vector<std::shared_ptr<T>> GetComponentsInChildren() const
    {
        std::vector<std::shared_ptr<T>> result;
        for (auto &child : children)
            for (auto &comp : child->components)
                if (auto casted = std::dynamic_pointer_cast<T>(comp))
                    result.push_back(casted);
        return result;
    }

    explicit Entity(const std::string &n) : name(n)
    {
        // For safety, assign transform later using Create()
        transform = std::make_unique<Transform>();
    }
};

// Now implement Transform::GetWorldMatrix after Entity is fully known
inline glm::mat4 Transform::GetWorldMatrix() const
{
    auto e = entity.lock();
    if (!e || !e->parent.lock())
        return GetLocalMatrix();

    return e->parent.lock()->transform->GetWorldMatrix() * GetLocalMatrix();
}
