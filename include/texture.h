//
// Created by muldrik on 23.07.2021.
//
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifndef HIGH_SHIFT_TEXTURE_H
#define HIGH_SHIFT_TEXTURE_H

class Texture2d {
public:
    explicit Texture2d(GLint wrap_s = GL_REPEAT,
              GLint wrap_t = GL_REPEAT,
              GLint texture_min_filter = GL_LINEAR,
              GLint texture_max_filter = GL_LINEAR) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_max_filter);
    }

    tl::expected<void, std::string> load_texture(const char *filename, GLenum format) {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
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

    void bind() {
        glBindTexture(GL_TEXTURE_2D, id);
    }

    unsigned int id;
};

#endif //HIGH_SHIFT_TEXTURE_H
