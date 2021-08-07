#include "ecs/ecs.h"
#include "render/render_system.h"
#include "vector"
#include "glm/vec3.hpp"
#include "iostream"
#include "common/handle_manager.h"

std::filesystem::path cube_vertex_shader_path = std::filesystem::current_path() / "shaders" / "cube.vert";
std::filesystem::path cube_fragment_shader_path = std::filesystem::current_path() / "shaders" / "cube.frag";
std::filesystem::path lighting_vertex_shader_path = std::filesystem::current_path() / "shaders" / "lighting.vert";
std::filesystem::path lighting_fragment_shader_path = std::filesystem::current_path() / "shaders" / "lighting.frag";

int main() {
    ecs::Dispatcher<render::RenderSystem> dispatcher;
    if (auto result = dispatcher.setup()) {}
    else {
        std::cerr << result.error() << std::endl;
        return 1;
    }

    auto &entities = dispatcher.get_world().get<ecs::Entities>();
    auto tr = render::Transform({glm::vec3(0.0f, 0.0f, -6.0f),
                                 glm::quat(glm::vec3(0.f, 0.f, 0.f))});
    auto cam = render::Camera{
        glm::perspective(glm::radians(45.0f),
                         800.0f / 600.0f, 0.1f, 100.0f)};
    entities.create(tr, cam);

    /*auto result = setup_shaders();
    if (!result) {
        std::cerr << result.error();
        exit(1);
    }*/
    auto &shader_loader = dispatcher.get_world().get<render::ShaderLoader>();
    auto result = shader_loader.create_program({
        {cube_vertex_shader_path, render::Shader::Type::vertex},
        {cube_fragment_shader_path, render::Shader::Type::fragment}
    });

    if (!result) {
        std::cerr << result.error() << std::endl;
        return 1;
    }

    auto light_res = shader_loader.create_program({
        {lighting_vertex_shader_path, render::Shader::Type::vertex},
        {lighting_fragment_shader_path, render::Shader::Type::fragment}
    });

    if (!light_res) {
        std::cerr << result.error() << std::endl;
        return 1;
    }

    auto cube_program = result.value();
    auto lighting_shader = light_res.value();


    auto &model_loader = dispatcher.get_world().get<render::ModelLoader>();
    auto result2 = model_loader.load_model("assets/cube/untitled.obj");
    if (!result2) {
        std::cerr << result2.error() << std::endl;
        return 1;
    }
    auto model = result2.value();

    shader_loader.get_shader_program(lighting_shader)->set_vec3("objectColor", 1.0f, 0.5f, 0.31f);
    shader_loader.get_shader_program(lighting_shader)->set_vec3("lightColor", 1.0f, 1.0f, 1.0f);

    entities.create(render::Transform{glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.f, glm::radians(-30.f), 0.f))},
                    render::Light{glm::vec3(1.0f, 1.0f, 1.0f)},
                    render::MeshRenderer{model, lighting_shader});

    entities.create(render::Transform{glm::vec3(4.0f, 0.0f, -3.0f),
                                      glm::quat(glm::vec3(0.f, glm::radians(20.f), 0.f))},
                    render::MeshRenderer{model, cube_program});
    dispatcher.run();
}