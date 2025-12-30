#pragma once
#include <engine/components/physics/collider/collider.hpp>
#include <engine/components/meshfilter.hpp>

class MeshCollider3D : public Collider
{
public:
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;

public:
    MeshCollider3D() { type = CollisionType::CAPSULE; }

    void OnAttach() override
    {
        auto en = entity.lock();
        if (en)
            return;
        // setup indeces, vertices
        if (auto meshfilter = en->GetComponent<MeshFilter>())
        {
            auto mesh = meshfilter->mesh;
            vertices = mesh->GetPoints();
            indices = mesh->GetIndices();
        }
        else
        {
            throw std::runtime_error("Mesh is null Collider require meshfilter!.");
        }
    }

    btCollisionShape *CreateShape() const override
    {
        btTriangleMesh *triangleMesh = new btTriangleMesh();

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            const glm::vec3 &v0 = vertices[indices[i]];
            const glm::vec3 &v1 = vertices[indices[i + 1]];
            const glm::vec3 &v2 = vertices[indices[i + 2]];

            triangleMesh->addTriangle(
                btVector3(v0.x, v0.y, v0.z),
                btVector3(v1.x, v1.y, v1.z),
                btVector3(v2.x, v2.y, v2.z));
        }

        return new btBvhTriangleMeshShape(triangleMesh, true);
    }
};
