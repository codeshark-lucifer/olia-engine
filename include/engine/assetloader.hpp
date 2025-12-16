#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <engine/meshrenderer.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <engine/utils/entity.hpp>
#include <engine/mesh.hpp>
#include <engine/meshrenderer.hpp>

namespace asset
{
    class Model
    {
    public:
        std::shared_ptr<Entity> root;

    public:
        Model(const std::string &path) { Load(path); }
        ~Model() = default;

        void Load(const std::string &path)
        {
            Assimp::Importer import;
            const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                throw std::runtime_error("Error Assimp" + (std::string)import.GetErrorString());
                return;
            }
            directory = path.substr(0, path.find_last_of('/'));

            root = ProcessNode(scene->mRootNode, scene);
        }

        glm::mat4 AiMatrix4x4ToGlm(const aiMatrix4x4 &from)
        {
            glm::mat4 to;
            //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
            to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
            to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
            to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
            to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
            return to;
        }

        std::shared_ptr<Entity> ProcessNode(aiNode *node, const aiScene *scene)
        {
            std::shared_ptr<Entity> go = Entity::Create(node->mName.C_Str());

            aiVector3D scaling;
            aiQuaternion rotation;
            aiVector3D position;
            node->mTransformation.Decompose(scaling, rotation, position);

            go->transform->position = glm::vec3(position.x, position.y, position.z);
            go->transform->scale = glm::vec3(scaling.x, scaling.y, scaling.z);
            go->transform->rotation = glm::eulerAngles(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));


            for (unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                go->AddComponent<MeshFilter>(ProcessMesh(mesh, scene));
                go->AddComponent<MeshRenderer>();
            }
            // then do the same for each of its children
            for (unsigned int i = 0; i < node->mNumChildren; i++)
            {
                go->AddChild(ProcessNode(node->mChildren[i], scene));
            }
            return go;
        }

        std::shared_ptr<Mesh> ProcessMesh(aiMesh *mesh, const aiScene *scene)
        {
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            std::vector<std::shared_ptr<Texture2D>> textures;

            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;

                glm::vec3 vector;
                vector.x = mesh->mVertices[i].x;
                vector.y = mesh->mVertices[i].y;
                vector.z = mesh->mVertices[i].z;
                vertex.Position = vector;

                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;

                if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
                {
                    glm::vec2 vec;
                    vec.x = mesh->mTextureCoords[0][i].x;
                    vec.y = mesh->mTextureCoords[0][i].y;
                    vertex.TexCoord = vec;
                }
                else
                    vertex.TexCoord = glm::vec2(0.0f, 0.0f);

                vertices.push_back(vertex);
            }

            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }

            if (mesh->mMaterialIndex >= 0)
            {
                aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                std::vector<std::shared_ptr<Texture2D>> diffuseMaps = LoadMaterialTextures(material,
                                                                                           aiTextureType_DIFFUSE);
                textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
                std::vector<std::shared_ptr<Texture2D>> specularMaps = LoadMaterialTextures(material,
                                                                                            aiTextureType_SPECULAR);
                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            }

            return std::make_shared<Mesh>(vertices, indices, textures);
        }

        std::vector<std::shared_ptr<Texture2D>> LoadMaterialTextures(aiMaterial *mat, aiTextureType type)
        {
            std::vector<std::shared_ptr<Texture2D>> textures;
            for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
            {
                aiString str;
                mat->GetTexture(type, i, &str);
                Type ttype = Type::DIFFUSE;
                if(type == aiTextureType_DIFFUSE) {
                    ttype = Type::DIFFUSE;
                } else if(type == aiTextureType_SPECULAR) {
                    ttype = Type::SPECULAR;
                }

                textures.push_back(std::make_shared<Texture2D>(str.C_Str(), ttype, true));
            }
            return textures;
        }

    private:
        std::string directory = "";
        std::vector<std::shared_ptr<Texture2D>> textures_loaded;
    };

} // namespace asset