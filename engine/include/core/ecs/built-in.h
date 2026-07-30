#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdint>
#include <cmath>
#include "core/ecs/entity.h"

enum class ProjectionType
{
    Orthographic,
    Perspective
};

namespace Engine
{
    class ECS;
}

struct Transform
{
    // Parent Entity ID (NULL_ENTITY if root object)
    Entity parent = NULL_ENTITY;

    // Local Space Properties
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f}; // radians (Euler X, Y, Z)
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};

    //--------------------------------------------------------------------------
    // Local Transforms
    //--------------------------------------------------------------------------
    glm::mat4 GetLocalMatrix() const
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, scale);
        return model;
    }

    // Direct aliases for local transform getters/setters
    glm::vec3 GetLocalPosition() const { return position; }
    glm::vec3 GetLocalRotation() const { return rotation; }
    glm::vec3 GetLocalScale() const { return scale; }

    void SetLocalPosition(const glm::vec3 &pos) { position = pos; }
    void SetLocalRotation(const glm::vec3 &rot) { rotation = rot; }
    void SetLocalScale(const glm::vec3 &s) { scale = s; }

    //--------------------------------------------------------------------------
    // World / Global Transforms
    //--------------------------------------------------------------------------
    // Returns full combined hierarchy matrix (World Matrix)
    glm::mat4 GetWorldMatrix(Engine::ECS *ecs = nullptr) const;
    glm::mat4 GetGlobalMatrix(Engine::ECS *ecs = nullptr) const { return GetWorldMatrix(ecs); }
    glm::mat4 GetMatrix(Engine::ECS *ecs = nullptr) const { return GetWorldMatrix(ecs); }

    // Global Position
    glm::vec3 GetWorldPosition(Engine::ECS *ecs = nullptr) const
    {
        return glm::vec3(GetWorldMatrix(ecs)[3]);
    }
    glm::vec3 GetGlobalPosition(Engine::ECS *ecs = nullptr) const { return GetWorldPosition(ecs); }

    // Global Scale
    glm::vec3 GetWorldScale(Engine::ECS *ecs = nullptr) const
    {
        glm::mat4 m = GetWorldMatrix(ecs);
        return glm::vec3(
            glm::length(glm::vec3(m[0])),
            glm::length(glm::vec3(m[1])),
            glm::length(glm::vec3(m[2]))
        );
    }
    glm::vec3 GetGlobalScale(Engine::ECS *ecs = nullptr) const { return GetWorldScale(ecs); }

    // Global Rotation (Euler Angles in Radians)
    glm::vec3 GetWorldRotation(Engine::ECS *ecs = nullptr) const
    {
        glm::mat4 m = GetWorldMatrix(ecs);
        glm::vec3 s = GetWorldScale(ecs);

        glm::mat3 rotMat;
        rotMat[0] = glm::vec3(m[0]) / (s.x > 0.00001f ? s.x : 1.0f);
        rotMat[1] = glm::vec3(m[1]) / (s.y > 0.00001f ? s.y : 1.0f);
        rotMat[2] = glm::vec3(m[2]) / (s.z > 0.00001f ? s.z : 1.0f);

        float pitch = std::atan2(rotMat[1][2], rotMat[2][2]);
        float yaw   = std::atan2(-rotMat[0][2], std::sqrt(rotMat[1][2] * rotMat[1][2] + rotMat[2][2] * rotMat[2][2]));
        float roll  = std::atan2(rotMat[0][1], rotMat[0][0]);
        return glm::vec3(pitch, yaw, roll);
    }
    glm::vec3 GetGlobalRotation(Engine::ECS *ecs = nullptr) const { return GetWorldRotation(ecs); }

    //--------------------------------------------------------------------------
    // Direction Vectors (World Space if ECS provided)
    //--------------------------------------------------------------------------
    glm::vec3 GetForward(Engine::ECS *ecs = nullptr) const
    {
        glm::mat4 m = GetWorldMatrix(ecs);
        return glm::normalize(glm::vec3(-m[2])); // -Z is forward
    }

    glm::vec3 GetRight(Engine::ECS *ecs = nullptr) const
    {
        glm::mat4 m = GetWorldMatrix(ecs);
        return glm::normalize(glm::vec3(m[0])); // +X is right
    }

    glm::vec3 GetUp(Engine::ECS *ecs = nullptr) const
    {
        glm::mat4 m = GetWorldMatrix(ecs);
        return glm::normalize(glm::vec3(m[1])); // +Y is up
    }

    void SetParent(Entity newParent) { parent = newParent; }
};

struct GameObject
{
    Entity id = 0;
    Transform *transform = nullptr;
};

struct Camera
{
    ProjectionType projection = ProjectionType::Perspective;

    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    float fov = 60.0f;
    float aspect = 16.0f / 9.0f;

    float orthoSize = 10.0f;
    float zoom = 1.0f;

    glm::mat4 GetProjection() const
    {
        if (projection == ProjectionType::Perspective)
        {
            glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
            proj[1][1] *= -1; // flip Y for Vulkan coordinate system
            return proj;
        }
        float safeZoom = (zoom <= 0.0001f) ? 0.0001f : zoom;
        float halfHeight = orthoSize / safeZoom;
        float halfWidth = halfHeight * aspect;

        glm::mat4 proj = glm::ortho(
            -halfWidth,
            halfWidth,
            -halfHeight,
            halfHeight,
            nearPlane,
            farPlane);
        proj[1][1] *= -1;
        return proj;
    }

    glm::mat4 GetView(const Transform &transform, Engine::ECS *ecs = nullptr) const
    {
        // View matrix is the inverse of the camera's world matrix
        glm::mat4 worldMat = transform.GetWorldMatrix(ecs);
        return glm::inverse(worldMat);
    }

    glm::mat4 GetViewProjection(const Transform &transform, Engine::ECS *ecs = nullptr) const
    {
        return GetProjection() * GetView(transform, ecs);
    }
};