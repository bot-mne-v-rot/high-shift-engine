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

    struct Texture {
        unsigned int id;
        std::string type;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        unsigned int VAO, VBO, EBO;
    };

    class ModelLoader {
    public:
        void setup_mesh(Mesh *mesh);
        void load_model(const char* path);
    };
}

#endif //HIGH_SHIFT_MODEL_LOADER_H
