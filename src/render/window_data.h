#ifndef HIGH_SHIFT_WINDOW_DATA_H
#define HIGH_SHIFT_WINDOW_DATA_H

#include "render/window_system.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace render {
    struct WindowSystem::WindowData {
        GLFWwindow *window;
    };
}

#endif //HIGH_SHIFT_WINDOW_DATA_H
