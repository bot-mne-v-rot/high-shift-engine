//
// Created by muldrik on 23.07.2021.
//
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <expected.h>


#ifndef HIGH_SHIFT_TEXTURE_H
#define HIGH_SHIFT_TEXTURE_H

class Texture2d {
public:
    explicit Texture2d(GLint wrap_s = GL_REPEAT,
                       GLint wrap_t = GL_REPEAT,
                       GLint texture_min_filter = GL_LINEAR,
                       GLint texture_max_filter = GL_LINEAR);

    tl::expected<void, std::string> load_texture(const char *filename, GLenum format);
    void bind();

    unsigned int id;
};

#endif //HIGH_SHIFT_TEXTURE_H
