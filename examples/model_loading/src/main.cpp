#include "ecs/ecs.h"
#include "render/render_system.h"
#include "vector"
#include "glm/vec3.hpp"
#include "iostream"
#include "common/handle_manager.h"

std::filesystem::path vertex_shader_path = std::filesystem::current_path() / "shaders" / "cube.vert";
std::filesystem::path fragment_shader_path = std::filesystem::current_path() / "shaders" / "cube.frag";

int main() {
    ecs::Dispatcher dispatcher;
    if (auto result = ecs::Dispatcher::create<render::RenderSystem>()) {
        dispatcher = std::move(result.value());
    } else {
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
        {vertex_shader_path, render::Shader::Type::vertex},
        {fragment_shader_path, render::Shader::Type::fragment}
    });

    if (!result) {
        std::cerr << result.error();
    }
    auto shader_program = result.value();

    auto &model_loader = dispatcher.get_world().get<render::ModelLoader>();
    auto model = model_loader.load_model("assets/backpack/backpack.obj").value();

    entities.create(render::Transform{glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.f, glm::radians(-90.f), 0.f))},
                    render::MeshRenderer{model, shader_program});
    dispatcher.loop();
}