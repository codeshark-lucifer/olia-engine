#pragma once

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include "built-in.h"

namespace Olia
{

// =====================================================
// Entity
// =====================================================

using Entity = uint32_t;

// =====================================================
// Component Pool Base
// =====================================================

class IPool
{
public:

    virtual ~IPool() = default;

    virtual void Remove(Entity entity) = 0;
};


// =====================================================
// Component Storage
// =====================================================

template<typename T>
class Pool : public IPool
{
public:

    std::unordered_map<Entity, T> components;


    void Add(Entity entity, const T& component)
    {
        components[entity] = component;
    }


    bool Has(Entity entity)
    {
        return components.find(entity) != components.end();
    }


    T& Get(Entity entity)
    {
        return components.at(entity);
    }


    void Remove(Entity entity) override
    {
        components.erase(entity);
    }
};



// =====================================================
// ECS World
// =====================================================

class ECS
{

private:

    Entity nextEntity = 0;


    std::vector<Entity> entities;


    std::unordered_map<
        std::type_index,
        std::unique_ptr<IPool>
    > pools;

public:
    // Create Entity

    Entity Create()
    {
        Entity entity = nextEntity++;

        entities.push_back(entity);

        return entity;
    }

    // Clear all entities and component pools
    void Clear()
    {
        entities.clear();
        pools.clear();
        nextEntity = 0;
    }



    // Add Component

    template<typename T>
    void Add(Entity entity, T component)
    {

        auto type = std::type_index(typeid(T));


        if(pools.find(type) == pools.end())
        {
            pools[type] =
                std::make_unique<Pool<T>>();
        }


        auto* pool =
            static_cast<Pool<T>*>(pools[type].get());


        pool->Add(entity, component);
    }



    // Get Component

    template<typename T>
    T& Get(Entity entity)
    {

        auto type =
            std::type_index(typeid(T));


        auto* pool =
            static_cast<Pool<T>*>(pools[type].get());


        return pool->Get(entity);
    }

    // Check Component

    template<typename T>
    bool Has(Entity entity)
    {

        auto type =
            std::type_index(typeid(T));


        if(pools.find(type) == pools.end())
            return false;


        auto* pool =
            static_cast<Pool<T>*>(pools[type].get());


        return pool->Has(entity);
    }

    template<typename... Components>
    std::vector<Entity> Query()
    {
        std::vector<Entity> result;


        for(Entity entity : entities)
        {
            bool matches = true;


            // Check every required component
            (
                [&]()
                {
                    if(!Has<Components>(entity))
                        matches = false;

                }(),
                ...
            );


            if(matches)
                result.push_back(entity);
        }


        return result;
    }
};

} // namespace Olia