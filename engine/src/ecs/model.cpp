#include "core/ecs/model.h"

#include <filesystem>
#include <iostream>
#include <vector>
#include <memory>

#include "components/mesh.h"
#include "components/material.h"
#include "components/mesh-renderer.h"

#include "engine/globals.h"
#include "ufbx/ufbx.h"
#include "utils/file.h"

std::vector<Entity> LoadModel(const char *filename)
{
    std::vector<Entity> entities;

    std::filesystem::path path =
        Engine::GetExecutableDir() / "assets" / filename;

    ufbx_load_opts opts{};
    ufbx_error error{};

    ufbx_scene *scene = ufbx_load_file(
        path.string().c_str(),
        &opts,
        &error);

    if (!scene)
    {
        std::cerr
            << "Failed loading FBX: "
            << error.description.data
            << "\n";

        return entities;
    }

    std::cout
        << "Loaded FBX: "
        << filename
        << "\n";

    //
    // Load Materials
    //

    std::vector<Material> materials;

    materials.reserve(scene->materials.count);

    for (size_t i = 0; i < scene->materials.count; i++)
    {
        ufbx_material *fbxMat =
            scene->materials.data[i];

        Material mat{};

        mat.color = glm::vec4(
            (float)fbxMat->pbr.base_color.value_vec3.x,
            (float)fbxMat->pbr.base_color.value_vec3.y,
            (float)fbxMat->pbr.base_color.value_vec3.z,
            (float)fbxMat->pbr.opacity.value_real);

        if (fbxMat->pbr.base_color.texture)
        {
            mat.texturePath =
                fbxMat->pbr.base_color.texture
                    ->filename.data;
        }

        materials.push_back(mat);
    }

    //
    // Meshes
    //

    for (size_t i = 0; i < scene->meshes.count; i++)
    {

        ufbx_mesh *fbxMesh =
            scene->meshes.data[i];

        auto mesh =
            std::make_unique<Mesh>();

        std::vector<uint32_t> triangles(
            fbxMesh->max_face_triangles * 3);

        for (size_t faceIndex = 0;
             faceIndex < fbxMesh->faces.count;
             faceIndex++)
        {

            ufbx_face face =
                fbxMesh->faces.data[faceIndex];

            if (face.num_indices < 3)
                continue;

            uint32_t count =
                ufbx_triangulate_face(
                    triangles.data(),
                    triangles.size(),
                    fbxMesh,
                    face);

            for (uint32_t t = 0; t < count * 3; t++)
            {

                uint32_t index =
                    triangles[t];

                Engine::Vertex vertex{};

                //
                // Position
                //

                auto pos =
                    ufbx_get_vertex_vec3(
                        &fbxMesh->vertex_position,
                        index);

                float scale = 0.01f; // cm -> meter

                vertex.position =
                    {
                        (float)pos.x * scale,
                        (float)pos.y * scale,
                        (float)pos.z * scale};

                //
                // Normal
                //

                if (fbxMesh->vertex_normal.exists)
                {
                    auto normal =
                        ufbx_get_vertex_vec3(
                            &fbxMesh->vertex_normal,
                            index);

                    vertex.normal =
                        {
                            (float)normal.x,
                            (float)normal.y,
                            (float)normal.z};
                }
                else
                {
                    vertex.normal =
                        {
                            0, 1, 0};
                }

                //
                // UV
                //

                if (fbxMesh->vertex_uv.exists)
                {

                    auto uv =
                        ufbx_get_vertex_vec2(
                            &fbxMesh->vertex_uv,
                            index);

                    vertex.uv =
                        {
                            (float)uv.x,
                            1.0f - (float)uv.y};
                }
                else
                {
                    vertex.uv =
                        {
                            0, 0};
                }

                mesh->vertices.push_back(vertex);

                mesh->indices.push_back(
                    static_cast<uint32_t>(
                        mesh->vertices.size() - 1));
            }
        }

        if (mesh->vertices.empty())
        {
            std::cout
                << "Empty mesh skipped: "
                << fbxMesh->name.data
                << "\n";

            continue;
        }

        //
        // Find material
        //

        int materialIndex = -1;

        if (fbxMesh->materials.count)
        {

            ufbx_material *mat =
                fbxMesh->materials.data[0];

            for (size_t m = 0;
                 m < scene->materials.count;
                 m++)
            {
                if (scene->materials.data[m] == mat)
                {
                    materialIndex = (int)m;
                    break;
                }
            }
        }

        //
        // Create Entity
        //

        Entity entity =
            ecs->Create();

        auto &renderer =
            ecs->Add<MeshRenderer>(entity);

        renderer.SetMesh(
            std::move(mesh));

        if (materialIndex >= 0 &&
            materialIndex < materials.size())
        {

            auto &mat =
                ecs->Add<Material>(entity);

            mat = materials[materialIndex];
        }

        entities.push_back(entity);

        std::cout
            << "Mesh: "
            << fbxMesh->name.data
            << "\n";
    }

    ufbx_free_scene(scene);

    return entities;
}