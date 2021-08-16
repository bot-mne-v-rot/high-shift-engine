#include "render/shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>

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

    void ShaderProgram::set_vec3(std::string_view name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(id, name.data()), x, y, z);
    }

    void ShaderProgram::set_vec3(std::string_view name, const glm::vec3 &vec) const {
        glUniform3f(glGetUniformLocation(id, name.data()), vec.x, vec.y, vec.z);
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

    void ShaderProgram::set_dir_light(int index, const DirLight &dir_light) const {
        set_vec3("dir_lights[" + std::to_string(index) + "].direction", dir_light.direction);
        set_vec3("dir_lights[" + std::to_string(index) + "].ambient", dir_light.ambient);
        set_vec3("dir_lights[" + std::to_string(index) + "].diffuse", dir_light.diffuse);
        set_vec3("dir_lights[" + std::to_string(index) + "].specular", dir_light.specular);
    }

    void ShaderProgram::set_point_light(int index, const glm::vec3 &position, const PointLight &point_light) const {
        set_vec3("point_lights[" + std::to_string(index) + "].position", position);
        set_float("point_lights[" + std::to_string(index) + "].constant", point_light.constant);
        set_float("point_lights[" + std::to_string(index) + "].linear", point_light.linear);
        set_float("point_lights[" + std::to_string(index) + "].quadratic", point_light.quadratic);
        set_vec3("point_lights[" + std::to_string(index) + "].ambient", point_light.ambient);
        set_vec3("point_lights[" + std::to_string(index) + "].diffuse", point_light.diffuse);
        set_vec3("point_lights[" + std::to_string(index) + "].specular", point_light.specular);
    }

    void ShaderProgram::set_spot_light(int index, const glm::vec3 &position, const SpotLight &spot_light) const {
        set_vec3("spot_lights[" + std::to_string(index) + "].position", position);
        set_vec3("spot_lights[" + std::to_string(index) + "].direction", spot_light.direction);
        set_float("spot_lights[" + std::to_string(index) + "].cutOff", spot_light.cutOff);
        set_float("spot_lights[" + std::to_string(index) + "].outerCutOff", spot_light.outerCutOff);
        set_float("spot_lights[" + std::to_string(index) + "].constant", spot_light.constant);
        set_float("spot_lights[" + std::to_string(index) + "].linear", spot_light.linear);
        set_float("spot_lights[" + std::to_string(index) + "].quadratic", spot_light.quadratic);
        set_vec3("spot_lights[" + std::to_string(index) + "].ambient", spot_light.ambient);
        set_vec3("spot_lights[" + std::to_string(index) + "].diffuse", spot_light.diffuse);
        set_vec3("spot_lights[" + std::to_string(index) + "].specular", spot_light.specular);
    }


}