#ifndef HIGH_SHIFT_MODEL_LOADER_H
#define HIGH_SHIFT_MODEL_LOADER_H

#include <string>
#include <vector>
#include <filesystem>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <expected.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "common/handle_manager.h"
#include "texture_loader.h"

namespace render {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 tex_coords;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Handle<Texture2d>> textures;

        unsigned int VAO, VBO, EBO;
    };

    struct Model {
        std::vector<Mesh> meshes;
    };

    //const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs)

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

    class ModelLoader {
    public:
        ModelLoader();

        ModelLoader(const ModelLoader &) = delete;
        ModelLoader &operator=(const ModelLoader &) = delete;
        ModelLoader(ModelLoader &&) = default;
        ModelLoader &operator=(ModelLoader &&) = default;

        tl::expected<Handle<Model>, std::string> load_model(const std::filesystem::path &path);
        Model *get_model(Handle<Model> handle);
        bool unload_model(Handle<Model> handle); // true if handle was valid

        ~ModelLoader();

    private:
        class Impl;

        Impl *impl = nullptr;
    };
}

#endif //HIGH_SHIFT_MODEL_LOADER_H
