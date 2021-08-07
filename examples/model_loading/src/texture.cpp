//
// Created by muldrik on 23.07.2021.
//
#include "texture.h"
#include <stb_image.h>

Texture2d::Texture2d() {
    glGenTextures(1, &id);
}

tl::expected<void, std::string> Texture2d::load_texture(const char *filename, GLenum format) {
    glBindTexture(GL_TEXTURE_2D, id);
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