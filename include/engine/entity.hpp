#pragma once
#include <vector>
#include <memory>
#include <string>
#include "component.hpp"
#include "transform.hpp"

class Shader;
class MeshRenderer;

class Entity : public std::enable_shared_from_this<Entity>
{
public:
    static std::shared_ptr<Entity> Create(const std::string &name);

    std::unique_ptr<Transform> transform;
    std::weak_ptr<Entity> parent;
    std::vector<std::shared_ptr<Entity>> children;
    std::vector<std::shared_ptr<Component>> components;
    std::string name;

    void AddChild(const std::shared_ptr<Entity> &child);

    void Begin() const;
    void Update(float dt, Shader *shader = nullptr) const;
    void Render(Shader &shader) const;

    template <typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args &&...args)
    {
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

private:
    explicit Entity(const std::string &name);
};
