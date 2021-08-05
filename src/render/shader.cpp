#include "render/shader.h"

#include <fstream>

namespace render {
    Shader::Shader(type t) : id(glCreateShader((GLenum) t)) {}

    Shader::Shader(Shader &&other) noexcept: id(0) {
        *this = std::move(other);
    }

    Shader &Shader::operator=(Shader &&other) noexcept {
        std::swap(const_cast<GLuint &>(id),
                  const_cast<GLuint &>(other.id));
        return *this;
    }

    tl::expected<void, std::string>
    Shader::load(std::string_view source) const {
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
    Shader::load_from_file(const std::filesystem::path &path) const {
        std::ifstream fin(path.string());
        std::string source((std::istreambuf_iterator<char>(fin)),
                           std::istreambuf_iterator<char>());
        return load(source);
    }

    Shader::~Shader() {
        glDeleteShader(id);
    }

    ShaderProgram::ShaderProgram() : id(glCreateProgram()) {}

    ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept: id(0) {
        *this = std::move(other);
    }

    ShaderProgram &ShaderProgram::operator=(ShaderProgram &&other) noexcept {
        std::swap(const_cast<GLuint &>(id),
                  const_cast<GLuint &>(other.id));
        return *this;
    }

    void ShaderProgram::attach(const Shader &sh) const {
        glAttachShader(id, sh.id);
    }

    tl::expected<void, std::string> ShaderProgram::link() const {
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

    void ShaderProgram::use() const {
        glUseProgram(id);
    }

    void ShaderProgram::set_int(std::string_view name, int value) const {
        glUniform1i(glGetUniformLocation(id, name.data()), value);
    }

    void ShaderProgram::set_float(std::string_view name, float value) const {
        glUniform1f(glGetUniformLocation(id, name.data()), value);
    }

    void ShaderProgram::set_bool(std::string_view name, bool value) const {
        glUniform1i(glGetUniformLocation(id, name.data()), (int) value);
    }

    ShaderProgram::~ShaderProgram() {
        glDeleteProgram(id);
    }

    void ShaderProgram::set_mat4(std::string_view name, float* value) const {
        glUniformMatrix4fv(glGetUniformLocation(id, name.data()), 1, GL_FALSE, value);
    }
}