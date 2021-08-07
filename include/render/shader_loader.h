#ifndef HIGH_SHIFT_SHADER_LOADER_H
#define HIGH_SHIFT_SHADER_LOADER_H

#include "expected.h"
#include "filesystem"
#include "common/handle_manager.h"

namespace render {

    struct Shader {

    };


    class ShaderProgram {

    };

    class ShaderLoader {
    public:
        ShaderLoader();
        ~ShaderLoader();

        ShaderLoader(const ShaderLoader &) = delete;
        ShaderLoader &operator=(const ShaderLoader &) = delete;
        ShaderLoader(ShaderLoader &&) = default;
        ShaderLoader &operator=(ShaderLoader &&) = default;

        [[nodiscard]] tl::expected<Handle<Shader>, std::string>
        load_shader_from_file(std::filesystem::path path);

        ShaderProgram *get_shader(Handle<ShaderProgram> handle) const;
        bool unload_shader(Handle<ShaderProgram> handle);

    private:
        class Impl;
        Impl *impl;
    };
}

#endif //HIGH_SHIFT_SHADER_LOADER_H
