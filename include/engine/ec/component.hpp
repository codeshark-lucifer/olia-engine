#pragma once
#include <memory>
#include <imgui.h> // Include ImGui header

class Entity;
class Component
{
public:
    std::weak_ptr<Entity> entity;
    virtual ~Component() = default;

    virtual void Awake() {}
    virtual void Start() {}

    virtual void FixedUpdate(float dt) {}
    virtual void Update(float dt) {}
    virtual void LateUpdate(float dt) {}

    // ImGui controls for inspector
    virtual void DrawImGuiControls() {}
};
