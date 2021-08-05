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
    explicit Texture2d();

    tl::expected<void, std::string> load_texture(const char *filename, GLenum format);
    void bind();

    unsigned int id;
};

#endif //HIGH_SHIFT_TEXTURE_H