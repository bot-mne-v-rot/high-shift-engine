#include "ecs/ecs.h"
#include "render/render_system.h"
#include "vector"
#include "glm/vec3.hpp"
#include "texture.h"
#include "iostream"
#include "common/handle_manager.h"

std::filesystem::path vertex_shader_path = std::filesystem::current_path() / "shaders" / "cube.vert";
std::filesystem::path fragment_shader_path = std::filesystem::current_path() / "shaders" / "cube.frag";

tl::expected<render::ShaderProgram, std::string> setup_shaders() {
    render::Shader vertex_shader(render::Shader::type::vertex);
    if (auto result = vertex_shader.load_from_file(vertex_shader_path)) {}
    else return tl::make_unexpected("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + result.error());

    render::Shader fragment_shader(render::Shader::type::fragment);
    if (auto result = fragment_shader.load_from_file(fragment_shader_path)) {}
    else return tl::make_unexpected("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" + result.error());

    render::ShaderProgram program;
    program.attach(vertex_shader);
    program.attach(fragment_shader);

    if (auto result = program.link()) {}
    else return tl::make_unexpected("ERROR::SHADER::LINKING_FAILED\n" + result.error());

    return program;
}


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

    auto result = setup_shaders();
    if (!result) {
        std::cerr << result.error();
        exit(1);
    }

    auto &model_loader = dispatcher.get_world().get<render::ModelLoader>();
    auto model = model_loader.load_model("assets/backpack/backpack.obj").value();

    entities.create(render::Transform{glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.f, glm::radians(-90.f), 0.f))},
                    render::MeshRenderer{model, &result.value()});
    dispatcher.run();
}