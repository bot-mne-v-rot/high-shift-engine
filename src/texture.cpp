//
// Created by muldrik on 24.07.2021.
//

//
// Created by muldrik on 23.07.2021.
//
#include "texture.h"
#include <stb_image.h>

Texture2d::Texture2d(GLint wrap_s, GLint wrap_t, GLint texture_min_filter, GLint texture_max_filter) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_max_filter);
    }

tl::expected<void, std::string> Texture2d::load_texture(const char *filename, GLenum format) {
    int width, height, nrChannels;
    uint8_t *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        return tl::make_unexpected<std::string>("Failed to load image from path " + std::string(filename));
    }
    stbi_image_free(data);

    return {};
}

void Texture2d::bind() {
    glBindTexture(GL_TEXTURE_2D, id);
}
