#pragma once
#include <engine/ecs/component.hpp>
#include <engine/input.hpp>
#include <glm/glm.hpp>
#include <engine/components/physics/rigidbody.hpp>

class Player : public Component
{
public:
    Player() {}
    void Update(const float &deltaTime) override {
        auto en = entity.lock();
        auto &input = InputManager::Get();
        auto rig = en->GetComponent<Rigidbody>();
        if(!rig) return;

        if(input.IsKeyDown(SDLK_F)) {
            rig->AddForce(glm::vec3(0.0f, 10.0f, 0.0f));
        }
    }

};
