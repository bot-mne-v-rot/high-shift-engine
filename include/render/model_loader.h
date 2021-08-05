#ifndef HIGH_SHIFT_MODEL_LOADER_H
#define HIGH_SHIFT_MODEL_LOADER_H

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
