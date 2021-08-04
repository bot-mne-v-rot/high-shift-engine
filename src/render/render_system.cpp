#include "render/render_system.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace render {
    static void framebuffer_size_callback(GLFWwindow *, int width, int height) {
        glViewport(0, 0, width, height);
    }

    static void setup_glfw() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }

    static GLFWwindow *create_window() {
        GLFWwindow *window = glfwCreateWindow(800, 600,
                                              "Я ебал твою тёлку. У!",
                                              nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return nullptr;
        }
        glfwMakeContextCurrent(window);
        return window;
    }

    static void setup_glad() {
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }
    }

    static void setup_viewport(GLFWwindow *window) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
    }

    static void setup_callbacks(GLFWwindow *window) {
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    }

    static void setup_GL() {
        glEnable(GL_DEPTH_TEST);
    }

    class RenderSystem::Impl {
    public:
        void setup(ecs::World &world) {
            setup_glfw();
            window = create_window();
            setup_glad();
            setup_viewport(window);
            setup_callbacks(window);
        }

        void update(ecs::GameLoopControl &game_loop) {
            if (glfwWindowShouldClose(window)) {
                game_loop.stop();
                return;
            }
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        void teardown(ecs::World &world) {
            glfwTerminate();
        }

    private:
        GLFWwindow *window = nullptr;
    };

    RenderSystem::RenderSystem() {
        impl = new Impl();
    }

    RenderSystem::~RenderSystem() {
        delete impl;
    }

    void RenderSystem::setup(ecs::World &world) {
        impl->setup(world);
    }

    void RenderSystem::update(ecs::GameLoopControl &game_loop_control) {
        impl->update(game_loop_control);
    }

    void RenderSystem::teardown(ecs::World &world) {
        impl->teardown(world);
    }
}