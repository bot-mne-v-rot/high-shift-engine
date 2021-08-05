#ifndef HIGH_SHIFT_TEXTURE_LOADER_H
#define HIGH_SHIFT_TEXTURE_LOADER_H

#include "common/handle_manager.h"
#include <expected.h>
#include <string>
#include <filesystem>

namespace render {
    struct Texture2d {
        unsigned int id;
        std::string type;
    };


    class TextureLoader {
    public:
        TextureLoader();
        ~TextureLoader();

        TextureLoader(const TextureLoader &) = delete;
        TextureLoader &operator=(const TextureLoader &) = delete;
        TextureLoader(TextureLoader &&) = default;
        TextureLoader &operator=(TextureLoader &&) = default;

        tl::expected<Handle<Texture2d>, std::string> load_from_file(std::filesystem::path path);
        Texture2d *get_texture(Handle<Texture2d> handle) const;
        bool unload_texture(Handle<Texture2d> handle);


    private:
        class Impl;
        Impl *impl;
    };
}

#endif //HIGH_SHIFT_TEXTURE_LOADER_H
