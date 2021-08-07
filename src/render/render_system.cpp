#include "render/render_system.h"


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

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

    static void setup_GL() {
        glEnable(GL_DEPTH_TEST);
    }

    static void render_mesh(const Mesh &mesh,
                            const TextureLoader &texture_loader,
                            const ShaderProgram &shader) {
        unsigned int diffuseNr = 0;
        unsigned int specularNr = 0;
        for (unsigned int i = 0; i < mesh.textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
            Texture2d *tex = texture_loader.get_texture(mesh.textures[i]);

            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string name;
            switch (tex->type) {
                case Texture2d::diffuse:
                    name = "texture_diffuse";
                    number = std::to_string(diffuseNr++);
                    break;
                case Texture2d::specular:
                    name = "texture_specular";
                    number = std::to_string(specularNr++);
                    break;
                default:
                    continue; // unsupported texture type
            }

            shader.set_int(("material." + name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, tex->id);
        }
        glActiveTexture(GL_TEXTURE0);

        // draw mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    class RenderSystem::Impl {
    public:
        tl::expected<void, std::string> setup(ecs::World &world) {
            setup_glfw();

            if (auto result = create_window())
                window = result.value();
            else return tl::make_unexpected(result.error());

            if (auto result = setup_glad()) {}
            else return result;

            setup_viewport(window);
            setup_callbacks(window);
            setup_GL();

            world.emplace<TextureLoader>();
            world.emplace<ModelLoader>(world.get<TextureLoader>());

            return {};
        }

        void update(ecs::GameLoopControl &game_loop,
                    const TextureLoader &texture_loader,
                    const ModelLoader &model_loader,
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
                    glm::mat4 local_to_world = glm::mat4(1.0f);
                    local_to_world = glm::translate(local_to_world, ent_transform.position);
                    local_to_world = local_to_world * glm::toMat4(ent_transform.rotation);

                    glm::mat4 mapping = projection * view * local_to_world;
                    renderer.shader_program->use();
                    renderer.shader_program->set_mat4("mapping", glm::value_ptr(mapping));

                    Model *model = model_loader.get_model(renderer.model_handle);
                    if (model)
                        for (auto &mesh : model->meshes)
                            render_mesh(mesh, texture_loader, *renderer.shader_program);
                });
            });

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        void teardown(ecs::World &world) {
            world.erase<ModelLoader>(); // must be deleted before TextureLoader
            world.erase<TextureLoader>();
            glfwTerminate(); // must appear last
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

    tl::expected<void, std::string> RenderSystem::setup(ecs::World &world) {
        return impl->setup(world);
    }

    void RenderSystem::update(ecs::GameLoopControl &game_loop_control,
                              const TextureLoader &texture_loader,
                              const ModelLoader &model_loader,
                              const MeshRenderer::Storage &renderers,
                              const Transform::Storage &transforms,
                              const Camera::Storage &cameras) {
        impl->update(game_loop_control, texture_loader, model_loader, renderers, transforms, cameras);
    }

    void RenderSystem::teardown(ecs::World &world) {
        impl->teardown(world);
    }
}