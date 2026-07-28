#pragma once

#include "./entity.h"

#include <map>
#include <utility>
#include <stdexcept>

namespace Engine
{

    class IPool
    {
    public:
        virtual ~IPool() = default;

        virtual void Remove(Entity entity) = 0;
    };

    template <typename T>
    class Pool : public IPool
    {

    public:
        std::map<Entity, T> components;
        template <typename... Args>
        T &Add(
            Entity entity,
            Args &&...args)
        {
            auto [it, inserted] =
                components.try_emplace(
                    entity,
                    std::forward<Args>(args)...);

            return it->second;
        }

        bool Has(
            Entity entity) const
        {
            return components.find(entity) != components.end();
        }

        T &Get(
            Entity entity)
        {
            auto it = components.find(entity);
            if (it == components.end())
            {
                throw std::out_of_range("Entity does not have requested component");
            }
            return it->second;
        }

        void Remove(
            Entity entity) override
        {
            components.erase(entity);
        }
    };

}