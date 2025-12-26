#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <limits>
#include <algorithm>

const float EPSILON = 0.0001f;

struct OBB
{
    glm::vec3 center;
    glm::vec3 halfExtents;
    glm::mat3 orientation;

    std::vector<glm::vec3> getVertices() const {
        std::vector<glm::vec3> vertices;
        vertices.reserve(8);

        glm::vec3 x_axis = orientation[0];
        glm::vec3 y_axis = orientation[1];
        glm::vec3 z_axis = orientation[2];

        vertices.push_back(center + x_axis * halfExtents.x + y_axis * halfExtents.y + z_axis * halfExtents.z);
        vertices.push_back(center + x_axis * halfExtents.x + y_axis * halfExtents.y - z_axis * halfExtents.z);
        vertices.push_back(center + x_axis * halfExtents.x - y_axis * halfExtents.y + z_axis * halfExtents.z);
        vertices.push_back(center + x_axis * halfExtents.x - y_axis * halfExtents.y - z_axis * halfExtents.z);
        vertices.push_back(center - x_axis * halfExtents.x + y_axis * halfExtents.y + z_axis * halfExtents.z);
        vertices.push_back(center - x_axis * halfExtents.x + y_axis * halfExtents.y - z_axis * halfExtents.z);
        vertices.push_back(center - x_axis * halfExtents.x - y_axis * halfExtents.y + z_axis * halfExtents.z);
        vertices.push_back(center - x_axis * halfExtents.x - y_axis * halfExtents.y - z_axis * halfExtents.z);

        return vertices;
    }
};

struct CollisionManifold
{
    bool colliding = false;
    glm::vec3 normal;
    float penetration = 0.0f;
    std::vector<glm::vec3> contacts;
};

void projectOntoAxis(const OBB& obb, const glm::vec3& axis, float& min, float& max) {
    std::vector<glm::vec3> vertices = obb.getVertices();
    min = std::numeric_limits<float>::max();
    max = std::numeric_limits<float>::lowest();

    for (const auto& v : vertices) {
        float projection = glm::dot(v, axis);
        min = std::min(min, projection);
        max = std::max(max, projection);
    }
}

bool overlap(float min1, float max1, float min2, float max2, float& overlapAmount) {
    if (min1 > max2 + EPSILON || min2 > max1 + EPSILON) {
        overlapAmount = 0.0f;
        return false;
    }

    float o1 = max1 - min2;
    float o2 = max2 - min1;

    overlapAmount = std::min(o1, o2);
    return true;
}

CollisionManifold FindCollision(const OBB &obbA, const OBB &obbB)
{
    CollisionManifold manifold;
    manifold.colliding = true;
    manifold.penetration = std::numeric_limits<float>::max();

    std::vector<glm::vec3> axes;
    axes.reserve(15);
    axes.push_back(obbA.orientation[0]);
    axes.push_back(obbA.orientation[1]);
    axes.push_back(obbA.orientation[2]);
    axes.push_back(obbB.orientation[0]);
    axes.push_back(obbB.orientation[1]);
    axes.push_back(obbB.orientation[2]);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            glm::vec3 cross_axis = glm::cross(axes[i], axes[3 + j]);
            if (glm::length2(cross_axis) > EPSILON * EPSILON) {
                axes.push_back(glm::normalize(cross_axis));
            }
        }
    }

    for (const auto& axis : axes) {
        float min1, max1, min2, max2;
        projectOntoAxis(obbA, axis, min1, max1);
        projectOntoAxis(obbB, axis, min2, max2);

        float currentOverlap;
        if (!overlap(min1, max1, min2, max2, currentOverlap)) {
            manifold.colliding = false;
            return manifold;
        }

        if (currentOverlap < manifold.penetration) {
            manifold.penetration = currentOverlap;
            manifold.normal = axis;
        }
    }

    glm::vec3 toCenter = obbB.center - obbA.center;
    if (glm::dot(toCenter, manifold.normal) < 0)
    {
        manifold.normal = -manifold.normal;
    }

    return manifold;
}

std::vector<glm::vec3> GetContactPoints(const OBB& obb1, const OBB& obb2, const CollisionManifold& manifold) {
    std::vector<glm::vec3> contactPoints;
    // For now, a simple approximation.
    // A full implementation requires clipping.
    if (manifold.colliding) {
        // Find the vertex of obb2 that is furthest along the collision normal
        auto vertices = obb2.getVertices();
        glm::vec3 contactPoint = vertices[0];
        float maxPenetration = glm::dot(contactPoint, manifold.normal);

        for (size_t i = 1; i < vertices.size(); ++i) {
            float penetration = glm::dot(vertices[i], manifold.normal);
            if (penetration < maxPenetration) {
                maxPenetration = penetration;
                contactPoint = vertices[i];
            }
        }
        contactPoints.push_back(contactPoint);
    }
    return contactPoints;
}
