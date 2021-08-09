#include "ecs/ecs.h"
#include "render/render_system.h"
#include "input/input_system.h"

#include <iostream>

std::filesystem::path vertex_shader_path = std::filesystem::current_path() / "shaders" / "cube.vert";
std::filesystem::path fragment_shader_path = std::filesystem::current_path() / "shaders" / "cube.frag";

class MainCameraSystem {
public:
    void setup(ecs::World &world,
               input::InputSystem &input_system) {
        create_camera(world);
        input_system.disable_cursor();
    }

    void update(render::Transform::Storage &transforms,
                const input::Input &input,
                ecs::GameLoopControl &game_loop,
                const ecs::DeltaTime &delta_time) {
        render::Transform &transform = transforms[camera_id];

        if (input.on_key_down(input::KEY_ESCAPE))
            game_loop.stop();

        update_camera_pos(input, transform, delta_time());
        update_camera_rot(input, transform);
    }

private:
    ecs::Id camera_id = 0;

    void create_camera(ecs::World &world) {
        auto &entities = world.get<ecs::Entities>();
        auto tr = render::Transform({glm::vec3(0.0f, 0.0f, 6.0f),
                                     glm::quat(glm::vec3(0.f, 0.f, 0.f))});
        auto cam = render::Camera{
            .projection = glm::perspective(glm::radians(45.0f),
                                           800.0f / 600.0f, 0.1f, 100.0f)};
        camera_id = entities.create(tr, cam);
    }

    void update_camera_rot(const input::Input &input,
                           render::Transform &transform) {
        static constexpr float sensitivity = 0.3f;

        glm::vec2 mouse_delta = input.get_mouse_pos_delta();

        yaw -= mouse_delta.x * sensitivity;
        pitch -= mouse_delta.y * sensitivity;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        transform.rotation = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.0f));
    }

    void update_camera_pos(const input::Input &input,
                           render::Transform &transform,
                           float delta_time) {
        static constexpr float speed = 3.0f;

        glm::vec3 forward = glm::rotate(transform.rotation, glm::vec3(0.0f, 0.0f, -1.0f));
        glm::vec3 right = glm::rotate(transform.rotation, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 up = glm::rotate(transform.rotation, glm::vec3(0.0f, 1.0f, 0.0f));

        if (input.is_key_down(input::KEY_W))
            transform.position += forward * delta_time * speed;
        if (input.is_key_down(input::KEY_S))
            transform.position -= forward * delta_time * speed;
        if (input.is_key_down(input::KEY_A))
            transform.position -= right * delta_time * speed;
        if (input.is_key_down(input::KEY_D))
            transform.position += right * delta_time * speed;
        if (input.is_key_down(input::KEY_SPACE))
            transform.position += up * delta_time * speed;
        if (input.is_key_down(input::KEY_LEFT_SHIFT))
            transform.position -= up * delta_time * speed;
    }

    float yaw = 0.0f;
    float pitch = 0.0f;
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