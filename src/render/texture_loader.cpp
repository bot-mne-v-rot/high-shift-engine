#include "render/texture_loader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <unordered_map>
#include <utility>
#include "common/handle_manager.h"

namespace fs = std::filesystem;

namespace render {
    class TextureLoader::Impl {
    public:
        tl::expected<Handle<Texture2d>, std::string>
        load_from_file(fs::path path, Texture2d::Type type) {

            std::error_code ec;
            path = fs::canonical(path, ec);
            if (ec) return tl::make_unexpected<std::string>(ec.message());

            auto handle_it = loaded_textures.find(fs::hash_value(path));
            if (handle_it != loaded_textures.end() && handle_manager.get(handle_it->second))
                return handle_it->second;

            auto *tex = new Texture2d();
            glGenTextures(1, &tex->id);
            glBindTexture(GL_TEXTURE_2D, tex->id);
            tex->type = type;

            if (auto result = load_texture_stbi(path)) {}
            else return tl::make_unexpected(result.error());

            auto handle = handle_manager.insert(tex);
            loaded_textures[fs::hash_value(path)] = handle;
            return handle;
        }

        Texture2d *get_texture(Handle<Texture2d> handle) const {
            return handle_manager.get(handle);
        }

        bool unload_texture(Handle<Texture2d> handle) {
            if (Texture2d *tex = handle_manager.erase(handle)) {
                handle_manager.erase(handle);
                glDeleteTextures(1, &tex->id);
                delete tex;
                return true;
            }
            return true;
        }

        ~Impl() {
            foreach(handle_manager, [](Texture2d *tex) {
                glDeleteTextures(1, &tex->id);
                delete tex;
            });
        }

    private:
        static tl::expected<void, std::string> load_texture_stbi(const fs::path &path) {
            int width, height, nrChannels;
            uint8_t *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            } else {
                return tl::make_unexpected<std::string>("Failed to load image from path " + path.string());
            }
            stbi_image_free(data);
            return {};
        }

        std::unordered_map<std::size_t, Handle<Texture2d>> loaded_textures;
        HandleManager<Texture2d> handle_manager;
    };

    tl::expected<Handle<Texture2d>, std::string>
    TextureLoader::load_from_file(std::filesystem::path path, Texture2d::Type type) {
        return impl->load_from_file(std::move(path), type);
    }

    Texture2d *TextureLoader::get_texture(Handle<Texture2d> handle) const {
        return impl->get_texture(handle);
    }

    bool TextureLoader::unload_texture(Handle<Texture2d> handle) {
        return impl->unload_texture(handle);
    }

    TextureLoader::TextureLoader() {
        impl = new Impl();
    }

    TextureLoader::~TextureLoader() {
        delete impl;
    }
}