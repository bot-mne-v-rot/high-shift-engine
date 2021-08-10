//#include "render/render_system.h"
//
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//
//#include "window_data.h"
//
//#include <iostream>
//
//namespace render {
//    static void setup_GL() {
//        glEnable(GL_DEPTH_TEST);
//    }
//
//    static void render_mesh(const Mesh &mesh,
//                            const TextureLoader &texture_loader,
//                            const ShaderProgram &shader) {
//        unsigned int diffuseNr = 0;
//        unsigned int specularNr = 0;
//        for (unsigned int i = 0; i < mesh.textures.size(); i++) {
//            glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
//            Texture2d *tex = texture_loader.get_texture(mesh.textures[i]);
//
//            // retrieve texture number (the N in diffuse_textureN)
//            std::string number;
//            std::string name;
//            switch (tex->type) {
//                case Texture2d::diffuse:
//                    name = "texture_diffuse";
//                    number = std::to_string(diffuseNr++);
//                    break;
//                case Texture2d::specular:
//                    name = "texture_specular";
//                    number = std::to_string(specularNr++);
//                    break;
//                default:
//                    continue; // unsupported texture type
//            }
//
//            shader.set_int((name + number).c_str(), i);
//            glBindTexture(GL_TEXTURE_2D, tex->id);
//        }
//        glActiveTexture(GL_TEXTURE0);
//
//        // draw mesh
//        glBindVertexArray(mesh.VAO);
//        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
//        glBindVertexArray(0);
//    }
//
//    class RenderSystem::Impl {
//    public:
//        tl::expected<void, std::string> setup(ecs::World &world,
//                                              const WindowSystem &window_system) {
//            setup_GL();
//
//            window = window_system.get_window_data().window;
//
//            world.emplace<ShaderLoader>();
//            world.emplace<TextureLoader>();
//            world.emplace<ModelLoader>(world.get<TextureLoader>());
//
//            return {};
//        }
//
//        void update(const ShaderLoader &shader_loader,
//                    const TextureLoader &texture_loader,
//                    const ModelLoader &model_loader,
//                    const MeshRenderer::Storage &renderers,
//                    const Transform::Storage &transforms,
//                    const Camera::Storage &cameras) {
//            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//            ecs::joined_foreach(transforms, cameras, [&](const Transform &cam_transform, const Camera &camera) {
//                glm::mat4 projection = camera.projection;
//
//                glm::mat4 view = glm::mat4(1.0f);
//                view = glm::translate(view, -cam_transform.position);
//                view = glm::toMat4(glm::inverse(cam_transform.rotation)) * view;
//
//                ecs::joined_foreach(transforms, renderers, [&](const Transform &ent_transform,
//                                                               const MeshRenderer &renderer) {
//                    glm::mat4 local_to_world = glm::mat4(1.0f);
//                    local_to_world = glm::translate(local_to_world, ent_transform.position);
//                    local_to_world = local_to_world * glm::toMat4(ent_transform.rotation);
//
//                    auto *shader_program = shader_loader.get_shader_program(renderer.shader_program_handle);
//
//                    glm::mat4 mapping = projection * view * local_to_world;
//                    shader_program->use();
//                    shader_program->set_mat4("mapping", mapping);
//
//                    Model *model = model_loader.get_model(renderer.model_handle);
//                    if (model)
//                        for (auto &mesh : model->meshes)
//                            render_mesh(mesh, texture_loader, *shader_program);
//                });
//            });
//
//            glfwSwapBuffers(window);
//            glfwPollEvents();
//        }
//
//        void teardown(ecs::World &world) {
//            world.erase<ShaderLoader>();
//            world.erase<ModelLoader>(); // must be deleted before TextureLoader
//            world.erase<TextureLoader>();
//        }
//
//    private:
//        GLFWwindow *window = nullptr;
//    };
//
//    RenderSystem::RenderSystem() {
//        impl = new Impl();
//    }
//
//    RenderSystem::~RenderSystem() {
//        delete impl;
//    }
//
//    tl::expected<void, std::string> RenderSystem::setup(ecs::World &world,
//                                                        const WindowSystem &window_system) {
//        return impl->setup(world, window_system);
//    }
//
//    void RenderSystem::update(const ShaderLoader &shader_loader,
//                              const TextureLoader &texture_loader,
//                              const ModelLoader &model_loader,
//                              const MeshRenderer::Storage &renderers,
//                              const Transform::Storage &transforms,
//                              const Camera::Storage &cameras) {
//        impl->update(shader_loader, texture_loader, model_loader, renderers, transforms, cameras);
//    }
//
//    void RenderSystem::teardown(ecs::World &world) {
//        impl->teardown(world);
//    }
//}