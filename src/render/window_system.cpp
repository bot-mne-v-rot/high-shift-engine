#include "render/window_system.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "window_data.h"

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

    [[nodiscard]] static tl::expected<GLFWwindow *, std::string> create_window() {
        GLFWwindow *window = glfwCreateWindow(800, 600,
                                              "Я ебал твою тёлку. У!",
                                              nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            return tl::make_unexpected("Failed to create GLFW window");
        }
        glfwMakeContextCurrent(window);
        return window;
    }

    [[nodiscard]] static tl::expected<void, std::string> setup_glad() {
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
            return tl::make_unexpected("Failed to initialize GLAD");
        return {};
    }

    static void setup_viewport(GLFWwindow *window) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
    }

    static void setup_callbacks(GLFWwindow *window) {
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    }

    class WindowSystem::Impl {
    public:
        tl::expected<void, std::string> setup(ecs::World &world) {
            setup_glfw();

            if (auto result = create_window())
                window = result.value();
            else return tl::make_unexpected(result.error());

            window_data = {window};

            if (auto result = setup_glad()) {}
            else return result;

            setup_viewport(window);
            setup_callbacks(window);

            return {};
        }

        void update(ecs::GameLoopControl &game_loop) {
            if (glfwWindowShouldClose(window)) {
                game_loop.stop();
                return;
            }
        }

        void teardown() {
            glfwTerminate(); // must appear last
        }

        const WindowData &get_window_data() const {
            return window_data;
        }

    private:
        WindowData window_data;
        GLFWwindow *window = nullptr;
    };

    WindowSystem::WindowSystem() {
        impl = new Impl();
    }

    WindowSystem::~WindowSystem() {
        delete impl;
    }

    tl::expected<void, std::string> WindowSystem::setup(ecs::World &world) {
        return impl->setup(world);
    }

    void WindowSystem::update(ecs::GameLoopControl &game_loop) {
        impl->update(game_loop);
    }

    void WindowSystem::teardown() {
        impl->teardown();
    }

    auto WindowSystem::get_window_data() const -> const WindowData & {
        return impl->get_window_data();
    }
}