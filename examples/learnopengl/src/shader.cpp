#include "shader.h"

#include <fstream>

shader::shader(type t) : id(glCreateShader((GLenum) t)) {}

shader::shader(shader &&other) noexcept: id(0) {
    *this = std::move(other);
}

shader &shader::operator=(shader &&other) noexcept {
    std::swap(const_cast<GLuint &>(id),
              const_cast<GLuint &>(other.id));
    return *this;
}

tl::expected<void, std::string>
shader::load(std::string_view source) const {
    const char *c_str = source.data();
    glShaderSource(id, 1, &c_str, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(id, sizeof(info_log), nullptr, info_log);
        return tl::make_unexpected(std::string(info_log));
    }
    return {};
}

tl::expected<void, std::string>
shader::load_from_file(const std::filesystem::path &path) const {
    std::ifstream fin(path.string());
    std::string source((std::istreambuf_iterator<char>(fin)),
                       std::istreambuf_iterator<char>());
    return load(source);
}

shader::~shader() {
    glDeleteShader(id);
}

shader_program::shader_program() : id(glCreateProgram()) {}

shader_program::shader_program(shader_program &&other) noexcept: id(0) {
    *this = std::move(other);
}

shader_program &shader_program::operator=(shader_program &&other) noexcept {
    std::swap(const_cast<GLuint &>(id),
              const_cast<GLuint &>(other.id));
    return *this;
}

void shader_program::attach(const shader &sh) const {
    glAttachShader(id, sh.id);
}

tl::expected<void, std::string> shader_program::link() const {
    glLinkProgram(id);

    int success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(id, sizeof(info_log), nullptr, info_log);
        return tl::make_unexpected(std::string(info_log));
    }
    return {};
}

void shader_program::use() const {
    glUseProgram(id);
}

void shader_program::setInt(std::string_view name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.data()), value);
}

void shader_program::setFloat(std::string_view name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.data()), value);
}

void shader_program::setBool(std::string_view name, bool value) const {
    glUniform1i(glGetUniformLocation(id, name.data()), (int) value);
}

shader_program::~shader_program() {
    glDeleteProgram(id);
}
