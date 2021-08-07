#ifndef HIGH_SHIFT_MODEL_LOADER_H
#define HIGH_SHIFT_MODEL_LOADER_H

#include <string>
#include <vector>
#include <filesystem>

#include <glm/glm.hpp>
#include <expected.h>

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

    class ModelLoader {
    public:
        explicit ModelLoader(TextureLoader &texture_loader);

        ModelLoader(const ModelLoader &) = delete;
        ModelLoader &operator=(const ModelLoader &) = delete;
        ModelLoader(ModelLoader &&) = default;
        ModelLoader &operator=(ModelLoader &&) = default;

        [[nodiscard]] tl::expected<Handle<Model>, std::string>
        load_model(const std::filesystem::path &path);

        Model *get_model(Handle<Model> handle) const; // nullptr if handle is invalid
        bool unload_model(Handle<Model> handle); // true if handle was valid
        bool unload_model_and_textures(Handle<Model> handle); // true if handle was valid

        ~ModelLoader();

    private:
        class Impl;

        Impl *impl = nullptr;
    };
}

#endif //HIGH_SHIFT_MODEL_LOADER_H
