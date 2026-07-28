#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdint>
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
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f}; // radians
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};

    glm::mat4 GetMatrix(Engine::ECS *ecs = nullptr) const
    {
        glm::mat4 model(1.0f);

        // Translation
        model = glm::translate(
            model,
            position);

        // Rotation
        model = glm::rotate(
            model,
            rotation.x,
            glm::vec3(1, 0, 0));

        model = glm::rotate(
            model,
            rotation.y,
            glm::vec3(0, 1, 0));

        model = glm::rotate(
            model,
            rotation.z,
            glm::vec3(0, 0, 1));

        // Scale
        model = glm::scale(
            model,
            scale);

        return model;
    }

    glm::vec3 GetForward() const
    {
        glm::mat4 rot(1.0f);
        rot = glm::rotate(rot, rotation.x, glm::vec3(1, 0, 0));
        rot = glm::rotate(rot, rotation.y, glm::vec3(0, 1, 0));
        rot = glm::rotate(rot, rotation.z, glm::vec3(0, 0, 1));
        return glm::normalize(glm::vec3(rot * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    }

    glm::vec3 GetRight() const
    {
        glm::mat4 rot(1.0f);
        rot = glm::rotate(rot, rotation.x, glm::vec3(1, 0, 0));
        rot = glm::rotate(rot, rotation.y, glm::vec3(0, 1, 0));
        rot = glm::rotate(rot, rotation.z, glm::vec3(0, 0, 1));
        return glm::normalize(glm::vec3(rot * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
    }

    glm::vec3 GetUp() const
    {
        glm::mat4 rot(1.0f);
        rot = glm::rotate(rot, rotation.x, glm::vec3(1, 0, 0));
        rot = glm::rotate(rot, rotation.y, glm::vec3(0, 1, 0));
        rot = glm::rotate(rot, rotation.z, glm::vec3(0, 0, 1));
        return glm::normalize(glm::vec3(rot * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
    }
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

    float fov = 45.0f;
    float aspect = 16.0f / 9.0f;

    float orthoSize = 10.0f;
    float zoom = 1.0f;

    glm::mat4 GetProjection() const
    {
        if (projection == ProjectionType::Perspective)
        {
            glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
            // proj[1][1] *= -1;         // flip Y
            // proj[2][2] = -proj[2][2]; // adjust depth mapping
            // proj[3][2] = -proj[3][2];
            return proj;
        }
        float safeZoom = (zoom <= 0.0001f) ? 0.0001f : zoom;
        float halfHeight = orthoSize / safeZoom;
        float halfWidth = halfHeight * aspect;

        return glm::ortho(
            -halfWidth,
            halfWidth,
            -halfHeight,
            halfHeight,
            nearPlane,
            farPlane);
    }

    glm::mat4 GetView(const Transform &transform) const
    {
        glm::mat4 view(1.0f);

        view = glm::rotate(view, -transform.rotation.z, glm::vec3(0, 0, 1));
        view = glm::rotate(view, -transform.rotation.y, glm::vec3(0, 1, 0));
        view = glm::rotate(view, -transform.rotation.x, glm::vec3(1, 0, 0));

        view = glm::translate(view, -transform.position);
        return view;
    }

    glm::mat4 GetViewProjection(
        const Transform &transform) const
    {
        return GetProjection() * GetView(transform);
    }
};