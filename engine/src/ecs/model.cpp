#include "core/ecs/model.h"

#include <filesystem>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

#include "components/mesh.h"
#include "components/material.h"
#include "components/mesh-renderer.h"
#include "components/skeleton.h"

#include "engine/globals.h"
#include "ufbx/ufbx.h"
#include "utils/file.h"

static glm::mat4 UfbxToGlmMat4(const ufbx_matrix &m)
{
    glm::mat4 mat(1.0f);
    mat[0] = glm::vec4((float)m.m00, (float)m.m10, (float)m.m20, 0.0f);
    mat[1] = glm::vec4((float)m.m01, (float)m.m11, (float)m.m21, 0.0f);
    mat[2] = glm::vec4((float)m.m02, (float)m.m12, (float)m.m22, 0.0f);
    mat[3] = glm::vec4((float)m.m03, (float)m.m13, (float)m.m23, 1.0f);
    return mat;
}

static glm::mat4 UfbxTransformToGlmMat4(const ufbx_transform &t)
{
    ufbx_matrix m = ufbx_transform_to_matrix(&t);
    return UfbxToGlmMat4(m);
}

std::vector<Entity> LoadModel(const char *filename)
{
    std::vector<Entity> entities;

    std::filesystem::path path = Engine::GetExecutableDir() / "assets" / filename;

    ufbx_load_opts opts{};
    opts.target_unit_meters = 1.0f;
    opts.target_axes = ufbx_axes_right_handed_y_up;
    opts.use_blender_pbr_material = true;
    ufbx_error error{};

    ufbx_scene *rawScene = ufbx_load_file(path.string().c_str(), &opts, &error);

    if (!rawScene)
    {
        std::cerr << "Failed loading FBX: " << error.description.data << "\n";
        return entities;
    }

    std::shared_ptr<ufbx_scene> scene(rawScene, ufbx_free_scene);

    std::cout << "Loaded FBX: " << filename << "\n";

    // 1. Load Materials
    std::vector<Material> materials;
    materials.reserve(scene->materials.count);

    for (size_t i = 0; i < scene->materials.count; i++)
    {
        ufbx_material *fbxMat = scene->materials.data[i];
        Material mat{};

        ufbx_vec3 col = fbxMat->pbr.base_color.has_value
                            ? fbxMat->pbr.base_color.value_vec3
                            : fbxMat->fbx.diffuse_color.value_vec3;

        float opacity = fbxMat->pbr.opacity.has_value
                            ? (float)fbxMat->pbr.opacity.value_real
                            : 1.0f;

        mat.color = glm::vec4((float)col.x, (float)col.y, (float)col.z, opacity);

        if (fbxMat->pbr.base_color.texture)
        {
            mat.albedoTexture = fbxMat->pbr.base_color.texture->filename.data;
        }

        materials.push_back(mat);
    }

    // 2. Load Meshes & Extract Skinning / Bone Data
    for (size_t i = 0; i < scene->meshes.count; i++)
    {
        ufbx_mesh *fbxMesh = scene->meshes.data[i];
        std::string meshName = fbxMesh->name.data ? fbxMesh->name.data : "";

        auto mesh = std::make_unique<Mesh>();
        Skeleton skeletonComponent{};

        // Check if mesh has skinning
        ufbx_skin_deformer *skin = (fbxMesh->skin_deformers.count > 0)
                                       ? fbxMesh->skin_deformers.data[0]
                                       : nullptr;

        if (skin)
        {
            skeletonComponent.bones.resize(skin->clusters.count);
            skeletonComponent.finalBoneMatrices.resize(skin->clusters.count, glm::mat4(1.0f));

            for (size_t c = 0; c < skin->clusters.count; c++)
            {
                ufbx_skin_cluster *cluster = skin->clusters.data[c];
                Bone &bone = skeletonComponent.bones[c];

                bone.name = cluster->name.data ? cluster->name.data : "";
                bone.node = cluster->bone_node;
                bone.inverseBindMatrix = UfbxToGlmMat4(cluster->geometry_to_bone);

                if (cluster->bone_node)
                {
                    bone.localTransform = UfbxTransformToGlmMat4(cluster->bone_node->local_transform);
                }
            }

            for (size_t c = 0; c < skin->clusters.count; c++)
            {
                Bone &bone = skeletonComponent.bones[c];
                if (bone.node && bone.node->parent)
                {
                    for (size_t p = 0; p < skin->clusters.count; p++)
                    {
                        if (skin->clusters.data[p]->bone_node == bone.node->parent)
                        {
                            bone.parentIndex = static_cast<int>(p);
                            break;
                        }
                    }
                }
            }
        }

        std::vector<uint32_t> triangles(fbxMesh->max_face_triangles * 3);

        for (size_t faceIndex = 0; faceIndex < fbxMesh->faces.count; faceIndex++)
        {
            ufbx_face face = fbxMesh->faces.data[faceIndex];

            if (face.num_indices < 3)
                continue;

            uint32_t count = ufbx_triangulate_face(
                triangles.data(),
                triangles.size(),
                fbxMesh,
                face);

            for (uint32_t t = 0; t < count * 3; t++)
            {
                uint32_t index = triangles[t];
                Engine::Vertex vertex{};

                auto pos = ufbx_get_vertex_vec3(&fbxMesh->vertex_position, index);
                vertex.position = {(float)pos.x, (float)pos.y, (float)pos.z};

                if (fbxMesh->vertex_normal.exists)
                {
                    auto normal = ufbx_get_vertex_vec3(&fbxMesh->vertex_normal, index);
                    vertex.normal = {(float)normal.x, (float)normal.y, (float)normal.z};
                }
                else
                {
                    vertex.normal = {0.0f, 1.0f, 0.0f};
                }

                if (fbxMesh->vertex_uv.exists)
                {
                    auto uv = ufbx_get_vertex_vec2(&fbxMesh->vertex_uv, index);
                    vertex.uv = {(float)uv.x, 1.0f - (float)uv.y};
                }
                else
                {
                    vertex.uv = {0.0f, 0.0f};
                }

                if (skin)
                {
                    uint32_t vtxIndex = fbxMesh->vertex_indices.data[index];
                    if (vtxIndex < skin->vertices.count)
                    {
                        ufbx_skin_vertex skinVert = skin->vertices.data[vtxIndex];
                        uint32_t numWeights = std::min((uint32_t)skinVert.num_weights, 4u);
                        for (uint32_t w = 0; w < numWeights; w++)
                        {
                            ufbx_skin_weight weight = skin->weights.data[skinVert.weight_begin + w];
                            vertex.boneIndices[w] = static_cast<int32_t>(weight.cluster_index);
                            vertex.boneWeights[w] = static_cast<float>(weight.weight);
                        }
                    }
                }

                mesh->vertices.push_back(vertex);
                mesh->indices.push_back(static_cast<uint32_t>(mesh->vertices.size() - 1));
            }
        }

        if (mesh->vertices.empty())
        {
            std::cout << "Empty mesh skipped: " << meshName << "\n";
            continue;
        }

        int materialIndex = -1;
        if (fbxMesh->materials.count > 0)
        {
            ufbx_material *targetMat = fbxMesh->materials.data[0];
            for (size_t m = 0; m < scene->materials.count; m++)
            {
                if (scene->materials.data[m] == targetMat)
                {
                    materialIndex = (int)m;
                    break;
                }
            }
        }
        else if (!materials.empty())
        {
            materialIndex = 0;
        }

        Entity entity = ecs->Create();

        auto &renderer = ecs->Add<MeshRenderer>(entity);
        renderer.SetMesh(std::move(mesh));

        auto &mat = ecs->Add<Material>(entity);
        if (materialIndex >= 0 && materialIndex < static_cast<int>(materials.size()))
        {
            mat = materials[materialIndex];
        }

        if (skin)
        {
            auto &skel = ecs->Add<Skeleton>(entity);
            skel = std::move(skeletonComponent);

            auto &animator = ecs->Add<Animator>(entity);

            if (scene->anim_stacks.count > 0)
            {
                for (size_t st = 0; st < scene->anim_stacks.count; st++)
                {
                    auto *stack = scene->anim_stacks.data[st];
                    AnimationClip clip{};
                    clip.name = stack->name.data && stack->name.length > 0 ? stack->name.data : "Default";
                    clip.scene = scene;
                    clip.anim = stack->anim;
                    clip.timeBegin = static_cast<float>(stack->time_begin);
                    clip.duration = static_cast<float>(stack->time_end - stack->time_begin);
                    if (clip.duration <= 0.0f)
                    {
                        clip.duration = static_cast<float>(stack->time_end);
                        clip.timeBegin = 0.0f;
                    }

                    animator.AddClip(clip.name, clip);
                }
            }
        }

        entities.push_back(entity);

        std::cout << "Mesh & Skeleton loaded successfully: " << meshName << "\n";
    }

    return entities;
}