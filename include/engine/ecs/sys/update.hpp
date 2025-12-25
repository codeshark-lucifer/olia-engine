#pragma once
#include <engine/ecs/system.hpp>
#include <stack>
#include <vector>
#include <memory>
#include <algorithm>
#include <execution> // For parallel execution (C++17)
#include <iostream>

class UpdateSystem : public System
{
public:
    UpdateSystem() : System("UpdateSystem") {}

    // Called once at the start
    void Begin(const std::vector<std::shared_ptr<Entity>>& entities) override
    {
        for (auto& e : entities)
        {
            TraverseEntities(e, [](const std::shared_ptr<Entity>& entity) {
                for (auto& comp : entity->components)
                    comp->Begin();
            });
        }
    }

    // Called every frame
    void Update(const std::vector<std::shared_ptr<Entity>>& entities, const float& deltaTime) override
    {
        for (auto& e : entities)
        {
            TraverseEntities(e, [deltaTime](const std::shared_ptr<Entity>& entity) {
                for (auto& comp : entity->components)
                    comp->Update(deltaTime);
            });
        }

        // Optional: parallel version if components are thread-safe
        /*
        std::for_each(std::execution::par, entities.begin(), entities.end(),
            [deltaTime](const std::shared_ptr<Entity>& e) {
                TraverseEntities(e, [deltaTime](const std::shared_ptr<Entity>& entity) {
                    for (auto& comp : entity->components)
                        comp->Update(deltaTime);
                });
            });
        */
    }

private:
    // Iterative traversal using stack
    template<typename Func>
    void TraverseEntities(const std::shared_ptr<Entity>& root, Func&& func)
    {
        std::stack<std::shared_ptr<Entity>> stack;
        stack.push(root);

        while (!stack.empty())
        {
            auto entity = stack.top();
            stack.pop();

            func(entity);

            for (auto& child : entity->children)
                stack.push(child);
        }
    }
};
