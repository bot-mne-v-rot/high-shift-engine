#include "ecs/ecs.h"
#include "render/render_system.h"
#include "input/input_system.h"

#include <iostream>

std::filesystem::path vertex_shader_path = std::filesystem::current_path() / "shaders" / "cube.vert";
std::filesystem::path fragment_shader_path = std::filesystem::current_path() / "shaders" / "cube.frag";

class MainCameraSystem {
public:
    void setup(ecs::World &world) {
        auto &entities = world.get<ecs::Entities>();
        auto tr = render::Transform({glm::vec3(0.0f, 0.0f, 6.0f),
                                     glm::quat(glm::vec3(0.f, 0.f, 0.f))});
        auto cam = render::Camera{
                .projection = glm::perspective(glm::radians(45.0f),
                                               800.0f / 600.0f, 0.1f, 100.0f)};
        camera_id = entities.create(tr, cam);
    }

    void update(render::Transform::Storage &transforms,
                input::Input &input) {
        render::Transform &transform = transforms[camera_id];

        glm::vec3 forward = glm::rotate(glm::inverse(transform.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
        glm::vec3 right = glm::rotate(glm::inverse(transform.rotation), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 up = glm::rotate(glm::inverse(transform.rotation), glm::vec3(0.0f, 1.0f, 0.0f));

        if (input.is_key_down(input::KEY_W))
            transform.position += forward * 0.01f;
        if (input.is_key_down(input::KEY_S))
            transform.position -= forward * 0.01f;
        if (input.is_key_down(input::KEY_A))
            transform.position -= right * 0.01f;
        if (input.is_key_down(input::KEY_D))
            transform.position += right * 0.01f;
        if (input.is_key_down(input::KEY_SPACE))
            transform.position += up * 0.01f;
        if (input.is_key_down(input::KEY_LEFT_SHIFT))
            transform.position -= up * 0.01f;
    }

private:
    ecs::Id camera_id;
};

int main() {
    ecs::Dispatcher dispatcher;

    auto dispatcher_result = ecs::DispatcherBuilder()
            .add_system<render::WindowSystem>(render::WindowArgs{
                    .width = 800, .height = 600, .title = "Model loading example"
            })
            .add_system<render::RenderSystem>()
            .add_system<input::InputSystem>()
            .add_system<MainCameraSystem>()
            .build();

    if (dispatcher_result) {
        dispatcher = std::move(dispatcher_result.value());
    } else {
        std::visit([](auto &&err) {
            std::cerr << err.message << std::endl;
        }, dispatcher_result.error());
        return 1;
    }

    auto &shader_loader = dispatcher.get_world().get<render::ShaderLoader>();
    auto result = shader_loader
            .create_program({{vertex_shader_path,   render::Shader::Type::vertex},
                             {fragment_shader_path, render::Shader::Type::fragment}});

    if (!result) {
        std::cerr << result.error();
    }
    auto shader_program = result.value();

    auto &model_loader = dispatcher.get_world().get<render::ModelLoader>();
    auto model = model_loader.load_model("assets/backpack/backpack.obj").value();

    auto &entities = dispatcher.get_world().get<ecs::Entities>();
    entities.create(render::Transform{glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.f, glm::radians(-90.f), 0.f))},
                    render::MeshRenderer{model, shader_program});
    dispatcher.loop();
}