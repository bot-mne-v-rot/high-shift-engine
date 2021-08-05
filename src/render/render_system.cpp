#include "render/render_system.h"


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

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

    static void render_mesh(const Mesh &mesh, const ShaderProgram &shader) {
        unsigned int diffuseNr = 0;
        unsigned int specularNr = 0;
        for (unsigned int i = 0; i < mesh.textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string name = mesh.textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);

            shader.set_int((name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        }
        glActiveTexture(GL_TEXTURE0);

        // draw mesh
        glBindVertexArray(mesh.VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    class RenderSystem::Impl {
    public:
        void setup(ecs::World &world) {
            setup_glfw();
            window = create_window();
            setup_glad();
            setup_viewport(window);
            setup_callbacks(window);
            setup_GL();
        }

        void update(ecs::GameLoopControl &game_loop,
                    const MeshRenderer::Storage &renderers,
                    const Transform::Storage &transforms,
                    const Camera::Storage &cameras) {
            if (glfwWindowShouldClose(window)) {
                game_loop.stop();
                return;
            }
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ecs::joined_foreach(transforms, cameras, [&](const Transform &cam_transform, const Camera &camera) {
                glm::mat4 projection = camera.projection;



                glm::mat4 view = glm::mat4(1.0f);
                view = glm::translate(view, cam_transform.position);
                view *= glm::toMat4(cam_transform.rotation);

                ecs::joined_foreach(transforms, renderers, [&](const Transform &ent_transform,
                                                              const MeshRenderer &renderer) {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, ent_transform.position);
                    model = model * glm::toMat4(ent_transform.rotation);
                    glm::mat4 mapping = projection * view * model;
                    renderer.shader_program->use();
                    renderer.shader_program->set_mat4("mapping", glm::value_ptr(mapping));
                    render_mesh(*renderer.mesh, *renderer.shader_program);
                });
            });

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

    void RenderSystem::update(ecs::GameLoopControl &game_loop_control,
                              const MeshRenderer::Storage &renderers,
                              const Transform::Storage &transforms,
                              const Camera::Storage &cameras) {
        impl->update(game_loop_control, renderers, transforms, cameras);
    }

    void RenderSystem::teardown(ecs::World &world) {
        impl->teardown(world);
    }
}