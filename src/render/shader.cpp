#include "render/shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>

namespace render {
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

    void ShaderProgram::set_mat4(std::string_view name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(id, name.data()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void ShaderProgram::set_vec3(std::string_view name, const glm::vec3 &vec) const {
        glUniform3f(glGetUniformLocation(id, name.data()), vec.x, vec.y, vec.z);
    }

    void ShaderProgram::set_vec3(std::string_view name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(id, name.data()), x, y, z);
    }

    Shader::Shader(Shader::Type type) {
        switch (type) {
            case Shader::vertex:
                id = glCreateShader(GL_VERTEX_SHADER);
                break;
            case Shader::fragment:
                id = glCreateShader(GL_FRAGMENT_SHADER);
                break;
        }
    }

    Shader::Shader(Shader &&other) noexcept {
        id = other.id;
        other.id = 0;
    }

    Shader &Shader::operator=(Shader &&other) noexcept {
        std::swap(other.id, id);
        return *this;
    }

    Shader::~Shader() {
        glDeleteShader(id);
    }

    ShaderProgram::ShaderProgram() {
        id = glCreateProgram();
    }

    ShaderProgram::~ShaderProgram() {
        glDeleteProgram(id);
    }

    ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept {
        id = other.id;
        other.id = 0;
    }

    ShaderProgram &ShaderProgram::operator=(ShaderProgram &&other) noexcept {
        std::swap(id, other.id);
        return *this;
    }
}