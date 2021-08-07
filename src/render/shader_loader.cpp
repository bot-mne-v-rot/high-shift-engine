#include "render/shader_loader.h"
#include "fstream"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utility>

namespace fs = std::filesystem;


namespace render {
    class ShaderLoader::Impl {
    public:

        ~Impl() {
            foreach(program_handle_manager, [](ShaderProgram *program) {
                delete program;
            });
        }

        [[nodiscard]] tl::expected<Shader, std::string>
        load_shader_from_file(fs::path &path, Shader::Type type) {

            Shader shader(type);

            // Воняет, не умею работать с ifstream нормально
            std::ifstream fin(path.string());
            if (!fin) {
                return tl::make_unexpected("Failed to open shader path: " + path.string());
            }
            std::string source((std::istreambuf_iterator<char>(fin)),
                               std::istreambuf_iterator<char>());
            source.data();
            const char *c_str = source.data();
            glShaderSource(shader.id, 1, &c_str, nullptr);
            glCompileShader(shader.id);

            int success;
            glGetShaderiv(shader.id, GL_COMPILE_STATUS, &success);
            if (!success) {
                char info_log[512];
                glGetShaderInfoLog(shader.id, sizeof(info_log), nullptr, info_log);
                return tl::make_unexpected(std::string(info_log));
            }
            return shader;
        }

        [[nodiscard]] tl::expected<Handle<ShaderProgram>, std::string>
        create_program(const std::vector<ShaderPath> &paths) {
            auto shader_program = std::make_unique<ShaderProgram>();

            std::vector<Shader> shaders;
            for (auto path : paths) {
                if (auto result = load_shader_from_file(path.path, path.texture_type)) {
                    Shader &shader = result.value();
                    glAttachShader(shader_program->id, shader.id);
                    shaders.push_back(std::move(shader));
                }
                else return tl::make_unexpected(result.error());
            }

            glLinkProgram(shader_program->id);
            int success;
            glGetProgramiv(shader_program->id, GL_LINK_STATUS, &success);
            if (!success) {
                char info_log[512];
                glGetProgramInfoLog(shader_program->id, sizeof(info_log), nullptr, info_log);
                return tl::make_unexpected(std::string(info_log));
            }

            auto handle = program_handle_manager.insert(shader_program.release());
            return handle;
        }


        ShaderProgram *get_shader_program(Handle<ShaderProgram> handle) const {
            return program_handle_manager.get(handle);
        }

        bool unload_shader_program(Handle<ShaderProgram> handle) {
            if (auto *program = program_handle_manager.erase(handle)) {
                delete program;
                return true;
            }
            return false;
        }

    private:
        HandleManager<ShaderProgram> program_handle_manager;
    };

    ShaderProgram *ShaderLoader::get_shader_program(Handle<ShaderProgram> handle) const {
        return impl->get_shader_program(handle);
    }

    bool ShaderLoader::unload_shader_program(Handle<ShaderProgram> handle) {
        return impl->unload_shader_program(handle);
    }

    tl::expected<Handle<ShaderProgram>, std::string>
    ShaderLoader::create_program(const std::vector<ShaderPath>& paths) {
        return impl->create_program(paths);
    }

    ShaderLoader::ShaderLoader() {
        impl = new Impl();
    }

    ShaderLoader::~ShaderLoader() {
        delete impl;
    }


}