#ifndef HIGH_SHIFT_SHADER_H
#define HIGH_SHIFT_SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../lib/other/expected.h"

#include <filesystem>

class shader {
public:
    enum type {
        vertex = GL_VERTEX_SHADER,
        fragment = GL_FRAGMENT_SHADER
    };

    const GLuint id;

    explicit shader(type t);

    shader(const shader &other) = delete;
    shader &operator=(const shader &other) = delete;

    shader(shader &&other) noexcept;
    shader &operator=(shader &&other) noexcept;

    [[nodiscard]] tl::expected<void, std::string> load(std::string_view source) const;
    [[nodiscard]] tl::expected<void, std::string> load_from_file(const std::filesystem::path &path) const;

    ~shader();
};

class shader_program {
public:
    const GLuint id;

    shader_program();

    shader_program(const shader_program &other) = delete;
    shader_program &operator=(const shader &other) = delete;

    shader_program(shader_program &&other) noexcept;
    shader_program &operator=(shader_program &&other) noexcept;

    void attach(const shader &sh) const;
    [[nodiscard]] tl::expected<void, std::string> link() const;

    void use() const;

    // utility uniform functions
    void setBool(std::string_view name, bool value) const;
    void setInt(std::string_view name, int value) const;
    void setFloat(std::string_view name, float value) const;

    ~shader_program();
};

#endif //HIGH_SHIFT_SHADER_H
