#ifndef HIGH_SHIFT_SHADER_H
#define HIGH_SHIFT_SHADER_H

#include <string>
#include <vector>

#include "glm/glm.hpp"

namespace render {

    /**
     * Holds shader id from OpenGL and uses
     * RAII to control shader's lifetime.
     */
    class Shader {
    public:
        enum Type {
            vertex, // GL_VERTEX_SHADER
            fragment, // GL_FRAGMENT_SHADER
        };
        unsigned int id;

        explicit Shader(Type type);
        ~Shader();

        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;
        Shader(Shader &&other) noexcept;
        Shader &operator=(Shader &&other) noexcept;

        bool operator==(const Shader &) const = default;
        bool operator!=(const Shader &) const = default;
    };


    class ShaderProgram {
    public:
        explicit ShaderProgram();
        ~ShaderProgram();

        ShaderProgram(const ShaderProgram &) = delete;
        ShaderProgram &operator=(const ShaderProgram &) = delete;
        ShaderProgram(ShaderProgram &&other) noexcept;
        ShaderProgram &operator=(ShaderProgram &&other) noexcept;

        bool operator==(const ShaderProgram &) const = default;
        bool operator!=(const ShaderProgram &) const = default;

        void use() const;

        void set_int(std::string_view name, int value) const;
        void set_float(std::string_view name, float value) const;
        void set_bool(std::string_view name, bool value) const;
        void set_mat4(std::string_view name, const glm::mat4 &mat) const;
        void set_vec3(std::string_view name, const glm::vec3 &vec) const;
        void set_vec3(std::string_view name, float x, float y, float z) const;

        unsigned int id;
    };
}

#endif //HIGH_SHIFT_SHADER_H
