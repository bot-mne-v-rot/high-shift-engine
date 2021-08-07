#ifndef HIGH_SHIFT_SHADER_LOADER_H
#define HIGH_SHIFT_SHADER_LOADER_H

#include "expected.h"
#include "filesystem"
#include "common/handle_manager.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "render/shader.h"

namespace fs = std::filesystem;


namespace render {

    struct ShaderPath {
        fs::path path;
        Shader::Type texture_type;
    };

    class ShaderLoader {
    public:
        ShaderLoader();
        ~ShaderLoader();

        ShaderLoader(const ShaderLoader &) = delete;
        ShaderLoader &operator=(const ShaderLoader &) = delete;
        ShaderLoader(ShaderLoader &&) = default;
        ShaderLoader &operator=(ShaderLoader &&) = default;

        [[nodiscard]] tl::expected<Handle<ShaderProgram>, std::string>
        create_program(const std::vector<ShaderPath>&);

        [[nodiscard]] ShaderProgram *get_shader_program(Handle<ShaderProgram> handle) const;
        bool unload_shader_program(Handle<ShaderProgram> handle);

    private:
        class Impl;
        Impl *impl;
    };
}

#endif //HIGH_SHIFT_SHADER_LOADER_H
