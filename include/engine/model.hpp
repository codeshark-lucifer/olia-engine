#pragma once
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <engine/ecs/entity.hpp>
#include <engine/components/mesh.hpp>
#include <engine/components/meshfilter.hpp>
#include <engine/components/meshrenderer.hpp>
#include <engine/texture2D.hpp>
#include <engine/scene.hpp>

class Model
{
public:
    // -------------------------------
    // LOAD MODEL
    // -------------------------------
    static std::shared_ptr<Entity> Load(
        const std::string &path,
        const std::shared_ptr<Scene> &scene)
    {
        Assimp::Importer importer;
        const aiScene *aiScene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
                aiProcess_GenNormals |
                aiProcess_CalcTangentSpace |
                aiProcess_JoinIdenticalVertices |
                aiProcess_ImproveCacheLocality);

        if (!aiScene || !aiScene->mRootNode)
        {
            throw std::runtime_error("Assimp failed to load model: " + path);
        }

        const std::string directory =
            path.substr(0, path.find_last_of("/\\"));

        auto root = std::make_shared<Entity>("ModelRoot");
        root->scene = scene;

        ProcessNode(
            aiScene->mRootNode,
            aiScene,
            root,
            scene,
            directory);
        scene->AddEntity(root);

        return root;
    }

private:
    // -------------------------------
    // NODE → ENTITY
    // -------------------------------
    static void ProcessNode(
        aiNode *node,
        const aiScene *scene,
        const std::shared_ptr<Entity> &parent,
        const std::shared_ptr<Scene> &ecsScene,
        const std::string &directory)
    {
        auto entity = std::make_shared<Entity>(node->mName.C_Str());
        entity->scene = ecsScene;

        ApplyTransform(node->mTransformation, entity);
        parent->AddChild(entity);

        // Meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *aiMesh = scene->mMeshes[node->mMeshes[i]];
            auto mesh = ProcessMesh(aiMesh, scene, directory);

            entity->AddComponent<MeshFilter>(mesh);
            entity->AddComponent<MeshRenderer>();
        }

        // Children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(
                node->mChildren[i],
                scene,
                entity,
                ecsScene,
                directory);
        }
    }

    // -------------------------------
    // TRANSFORM
    // -------------------------------
    static void ApplyTransform(
        const aiMatrix4x4 &m,
        const std::shared_ptr<Entity> &entity)
    {
        aiVector3D pos, scale;
        aiQuaternion rot;
        m.Decompose(scale, rot, pos);

        entity->transform.position = {pos.x, pos.y, pos.z};
        entity->transform.scale = {scale.x, scale.y, scale.z};
        entity->transform.rotation = glm::quat(rot.w, rot.x, rot.y, rot.z);
    }

    // -------------------------------
    // MESH
    // -------------------------------
    static std::shared_ptr<Mesh> ProcessMesh(
        aiMesh *mesh,
        const aiScene *scene,
        const std::string &directory)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<std::shared_ptr<Texture2D>> textures;

        vertices.reserve(mesh->mNumVertices);

        // Vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex v{};
            v.position = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z};

            if (mesh->HasNormals())
            {
                v.normal = {
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z};
            }

            if (mesh->mTextureCoords[0])
            {
                v.uv = {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y};
            }

            vertices.push_back(v);
        }

        // Indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            const aiFace &face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Materials
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

            AppendMaterialTextures(
                textures,
                material,
                aiTextureType_DIFFUSE,
                Type::DIFFUSE,
                directory);

            AppendMaterialTextures(
                textures,
                material,
                aiTextureType_SPECULAR,
                Type::SPECULAR,
                directory);
        }

        return std::make_shared<Mesh>(vertices, indices, textures);
    }

    // -------------------------------
    // TEXTURES
    // -------------------------------
    static void AppendMaterialTextures(
        std::vector<std::shared_ptr<Texture2D>> &out,
        aiMaterial *material,
        aiTextureType type,
        Type engineType,
        const std::string &directory)
    {
        for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
        {
            aiString str;
            material->GetTexture(type, i, &str);

            out.push_back(
                std::make_shared<Texture2D>(
                    directory + "/" + str.C_Str(),
                    engineType));
        }
    }
};
