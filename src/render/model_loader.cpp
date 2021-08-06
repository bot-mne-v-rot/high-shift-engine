#include "render/model_loader.h"
#include "iostream"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace render {
    static void setup_mesh(Mesh *mesh) {
        glGenVertexArrays(1, &mesh->VAO);
        glGenBuffers(1, &mesh->VBO);
        glGenBuffers(1, &mesh->EBO);

        glBindVertexArray(mesh->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

        glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int),
                     &mesh->indices[0], GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, tex_coords));

        glBindVertexArray(0);
    }

    class ModelLoader::Impl {
    public:
        explicit Impl(TextureLoader &texture_loader)
                : texture_loader(texture_loader) {}

        ~Impl() {
            foreach(handle_manager, [](Model *model) {
                unload_model_raw(model);
                delete model;
            });
        }

        tl::expected<Handle<Model>, std::string> load_model(const std::filesystem::path &path) {
            Assimp::Importer import;
            const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
                return tl::make_unexpected(import.GetErrorString());

            std::filesystem::path directory = path.parent_path();
            Model *model = new Model();
            process_node(model, directory, scene->mRootNode, scene);

            return handle_manager.insert(model);
        }

        Model *get_model(Handle<Model> handle) {
            return handle_manager.get(handle);
        }

        bool unload_model(Handle<Model> handle) {
            if (Model *model = handle_manager.get(handle)) {
                unload_model_raw(model);
                handle_manager.erase(handle);
                return true;
            }
            return false;
        }

        bool unload_model_and_textures(Handle<Model> handle) {
            if (Model *model = handle_manager.get(handle)) {
                for (auto& mesh : model->meshes)
                    for (auto& texture : mesh.textures)
                        texture_loader.unload_texture(texture);
                unload_model_raw(model);
                handle_manager.erase(handle);
                return true;
            }
            return false;
        }

    private:
        Assimp::Importer importer;
        HandleManager<Model> handle_manager;
        TextureLoader &texture_loader;

        Mesh process_mesh(aiMesh *mesh, const aiScene *scene,
                          const std::filesystem::path &directory) {
            Mesh result;

            result.vertices.reserve(mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                Vertex vertex;

                vertex.position.x = mesh->mVertices[i].x;
                vertex.position.y = mesh->mVertices[i].y;
                vertex.position.z = mesh->mVertices[i].z;

                vertex.normal.x = mesh->mNormals[i].x;
                vertex.normal.y = mesh->mNormals[i].y;
                vertex.normal.z = mesh->mNormals[i].z;

                if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
                    vertex.tex_coords.x = mesh->mTextureCoords[0][i].x;
                    vertex.tex_coords.y = mesh->mTextureCoords[0][i].y;
                } else {
                    vertex.tex_coords = glm::vec2(0.0f, 0.0f);
                }

                result.vertices.push_back(vertex);
            }
            // process indices
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    result.indices.push_back(face.mIndices[j]);
            }
            // process material
            if (mesh->mMaterialIndex >= 0) {
                aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                load_material_textures(result.textures, directory, material, aiTextureType_DIFFUSE, "texture_diffuse");
                load_material_textures(result.textures, directory, material, aiTextureType_SPECULAR, "texture_specular");
            }

            setup_mesh(&result);

            return result;
        }

        void process_node(Model *model, const std::filesystem::path &directory,
                          aiNode *node, const aiScene *scene) {
            // process all the node's meshes (if any)
            for (unsigned int i = 0; i < node->mNumMeshes; i++) {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                model->meshes.push_back(process_mesh(mesh, scene, directory));
            }
            // then do the same for each of its children
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                process_node(model, directory, node->mChildren[i], scene);
            }
        }

        void load_material_textures(std::vector<Handle<Texture2d>> &textures,
                                    const std::filesystem::path &directory,
                                    aiMaterial *mat, aiTextureType type, std::string type_name) {
            for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
                aiString str;
                mat->GetTexture(type, i, &str);
                textures.push_back(
                        texture_loader.load_from_file(directory / str.C_Str(), type_name).value()
                );
                std::cout << "process_texture " << i << " / " << mat->GetTextureCount(type) << std::endl;
            }
        }

        static void unload_model_raw(Model *model) {
            for (auto& mesh : model->meshes) {
                glDeleteVertexArrays(1, &mesh.VAO);
                glDeleteBuffers(1, &mesh.VBO);
                glDeleteBuffers(1, &mesh.EBO);
            }
            delete model;
        }
    };

    ModelLoader::ModelLoader(TextureLoader &texture_loader) {
        impl = new Impl(texture_loader);
    }

    ModelLoader::~ModelLoader() {
        delete impl;
    }

    tl::expected<Handle<Model>, std::string> ModelLoader::load_model(const std::filesystem::path &path) {
        return impl->load_model(path);
    }

    Model *ModelLoader::get_model(Handle<Model> handle) const {
        return impl->get_model(handle);
    }

    bool ModelLoader::unload_model(Handle<Model> handle) {
        return impl->unload_model(handle);
    }

    bool ModelLoader::unload_model_and_textures(Handle<Model> handle) {
        return impl->unload_model_and_textures(handle);
    }
}
