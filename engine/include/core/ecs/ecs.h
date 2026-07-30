#pragma once

#include "./pool.h"
#include "./built-in.h"

#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <utility>

namespace Engine
{
    class ECS
    {
    private:
        Entity nextEntity = 0;
        std::vector<Entity> entities;
        std::unordered_map<std::type_index, std::unique_ptr<IPool>> pools;

    public:
        Entity Create()
        {
            Entity entity = nextEntity++;
            entities.push_back(entity);

            auto &trans = Add<Transform>(entity);
            auto &obj = Add<GameObject>(entity);

            obj.transform = &trans;
            obj.id = entity;
            return entity;
        }

        void Clear()
        {
            entities.clear();
            pools.clear();
            nextEntity = 0;
        }

        template <typename T, typename... Args>
        T &Add(Entity entity, Args &&...args)
        {
            if constexpr (std::is_same_v<T, Transform> || std::is_same_v<T, GameObject>)
            {
                if (Has<T>(entity))
                {
                    throw std::logic_error("Transform/GameObject already exists on this entity. Use Get<T>() instead.");
                }
            }

            auto type = std::type_index(typeid(T));
            if (pools.find(type) == pools.end())
            {
                pools[type] = std::make_unique<Pool<T>>();
            }

            auto *pool = static_cast<Pool<T> *>(pools[type].get());
            return pool->Add(entity, std::forward<Args>(args)...);
        }

        template <typename T>
        T &Get(Entity entity)
        {
            auto type = std::type_index(typeid(T));
            auto it = pools.find(type);
            if (it == pools.end())
            {
                throw std::out_of_range("Component pool does not exist");
            }
            auto *pool = static_cast<Pool<T> *>(it->second.get());
            return pool->Get(entity);
        }

        template <typename T>
        bool Has(Entity entity)
        {
            auto type = std::type_index(typeid(T));
            auto it = pools.find(type);
            if (it == pools.end())
                return false;

            auto *pool = static_cast<Pool<T> *>(it->second.get());
            return pool->Has(entity);
        }

        template <typename T>
        void Remove(Entity entity)
        {
            auto type = std::type_index(typeid(T));
            auto it = pools.find(type);
            if (it == pools.end())
                return;

            auto *pool = static_cast<Pool<T> *>(it->second.get());
            pool->Remove(entity);
        }

        template <typename... Components>
        std::vector<Entity> Query()
        {
            std::vector<Entity> result;
            for (Entity entity : entities)
            {
                bool match = true;
                ([&]() {
                    if (!Has<Components>(entity))
                        match = false;
                }(), ...);

                if (match)
                    result.push_back(entity);
            }
            return result;
        }
    };
}

// Inline implementation of Transform::GetWorldMatrix requiring full ECS definition
inline glm::mat4 Transform::GetWorldMatrix(Engine::ECS *ecs) const
{
    glm::mat4 local = GetLocalMatrix();
    if (ecs != nullptr && parent != NULL_ENTITY && ecs->Has<Transform>(parent))
    {
        return ecs->Get<Transform>(parent).GetWorldMatrix(ecs) * local;
    }
    return local;
}