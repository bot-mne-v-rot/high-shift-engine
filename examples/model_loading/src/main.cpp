#include "ecs/ecs.h"
#include "render/render_system.h"
#include "vector"
#include "glm/vec3.hpp"
#include "texture.h"
#include "iostream"
#include "common/handle_manager.h"

std::vector<float> raw_vert = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
};

std::vector<unsigned int> indices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                                     12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                     24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};

glm::vec3 cube_positions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
};

/*render::Shader vert_shader(render::Shader::type::vertex);
vert_shader.load_from_file("/shaders/cube.vert");
render::Shader frag_shader(render::Shader::type::fragment);
frag_shader.load_from_file("/shaders/cube.frag");
render::ShaderProgram shader_program;
shader_program.attach(vert_shader);
shader_program.attach(frag_shader);*/

/*const char* vertex_shader_path = "/shaders/cube.vert";
const char* fragment_shader_path = "/shaders/cube.frag";*/

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
    HandleManager<render::Mesh> mesh_handler;
    auto &entities = dispatcher.get_world().get<ecs::Entities>();
    auto tr = render::Transform({glm::vec3(0.0f, 0.0f, -3.0f),
                                 glm::quat(glm::vec3(0.f, 0.f, 0.f))});
    auto cam = render::Camera{glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f)};
    entities.create(tr, cam);


    auto &texture_loader = dispatcher.get_world().get<render::TextureLoader>();
    auto container = texture_loader.load_from_file("assets/container.jpg").value();
    texture_loader.get_texture(container)->type = "texture_diffuse";


    /*Texture2d awesomeface;
    awesomeface.load_texture("assets/awesomeface.png", GL_RGBA);*/



    std::vector<render::Vertex> vertices;
    for (int i = 0; i < 36; i++) {
        auto pos = glm::vec3(raw_vert[i * 5], raw_vert[i * 5 + 1], raw_vert[i * 5 + 2]);
        auto norm = glm::vec3(0.0f, 0.0f, 0.0f);
        auto tex = glm::vec2(raw_vert[i * 5 + 3], raw_vert[i * 5 + 4]);
        vertices.emplace_back(render::Vertex{pos, norm, tex});
    }

    render::Mesh cube_mesh{
            vertices,
            indices,
            {container}
    };

    setup_mesh(&cube_mesh);

    auto result = setup_shaders();
    if (!result) {
        std::cerr << result.error();
        exit(1);
    }

    for (auto pos : cube_positions)
        entities.create(render::Transform{pos, glm::quat(1.0f, 0, 0, 0)},
                        render::MeshRenderer{&cube_mesh, &result.value()});
    dispatcher.run();
}