#ifndef HIGH_SHIFT_SHADER_H
#define HIGH_SHIFT_SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../lib/other/expected.h"

#include <filesystem>
#include <glm/glm.hpp>

namespace render {

    class Shader {
    public:
        enum type {
            vertex = GL_VERTEX_SHADER,
            fragment = GL_FRAGMENT_SHADER
        };

        const GLuint id;

        explicit Shader(type t);

        Shader(const Shader &other) = delete;
        Shader &operator=(const Shader &other) = delete;

        Shader(Shader &&other) noexcept;
        Shader &operator=(Shader &&other) noexcept;

        [[nodiscard]] tl::expected<void, std::string> load(std::string_view source) const;
        [[nodiscard]] tl::expected<void, std::string> load_from_file(const std::filesystem::path &path) const;

        ~Shader();
    };

    class ShaderProgram {
    public:
        const GLuint id;

        ShaderProgram();

        ShaderProgram(const ShaderProgram &other) = delete;
        ShaderProgram &operator=(const Shader &other) = delete;

        ShaderProgram(ShaderProgram &&other) noexcept;
        ShaderProgram &operator=(ShaderProgram &&other) noexcept;

        void attach(const Shader &sh) const;
        [[nodiscard]] tl::expected<void, std::string> link() const;

        void use() const;

        // utility uniform functions
        void set_bool(std::string_view name, bool value) const;
        void set_int(std::string_view name, int value) const;
        void set_float(std::string_view name, float value) const;
        void set_mat4(std::string_view name, float*) const;

        ~ShaderProgram();
    };
}

#endif //HIGH_SHIFT_SHADER_H
