#include "ecs/ecs.h"
#include "render/render_system.h"
#include "input/input_system.h"
#include "render/lights.h"

#include <iostream>

std::filesystem::path cube_vertex_path = std::filesystem::current_path() / "shaders" / "cube.vert";
std::filesystem::path cube_fragment_path = std::filesystem::current_path() / "shaders" / "cube.frag";
std::filesystem::path light_vertex_path = std::filesystem::current_path() / "shaders" / "lighting.vert";
std::filesystem::path light_fragment_path = std::filesystem::current_path() / "shaders" / "lighting.frag";

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
        auto tr = render::Transform({glm::vec3(0.f, 0.f, 6.0f),
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

glm::vec3 govno_lampa_positions[] = {
        glm::vec3( 0.7f,  -10.0f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(-6.0f,  2.0f, 0.0f),
};

render::DirLight sun {
    .direction = {0.2f, -1.0f, -0.3f},
    .ambient = {0.05f, 0.05f, 0.05f},
    .diffuse = {0.2f, 0.2f, 0.2f},
    .specular = {0.5f, 0.5f, 0.5f}
};


render::PointLight govno_lampa {
    .constant = 1.0f,
    .linear = 0.09f,
    .quadratic = 0.032f,
    .ambient = {0.10f, 0.10f, 0.10f},
    .diffuse = {0.6f, 0.6f, 0.6f},
    .specular = {0.8f, 0.8f, 0.8f},
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
    auto result1 = shader_loader
            .create_program({{cube_vertex_path,   render::Shader::Type::vertex},
                             {cube_fragment_path, render::Shader::Type::fragment}});

    if (!result1) {
        std::cerr << result1.error();
        return 1;
    }

    auto shader_program = result1.value();


    auto light_cube_program = shader_loader
            .create_program({{light_vertex_path, render::Shader::Type::vertex},
                              {light_fragment_path, render::Shader::Type::fragment}})\
                              .value();

    auto &model_loader = dispatcher.get_world().get<render::ModelLoader>();
    auto backpack = model_loader.load_model("assets/backpack/backpack.obj").value();
    auto cube = model_loader.load_model("assets/cube/untitled.obj").value();

    auto &entities = dispatcher.get_world().get<ecs::Entities>();
    entities.create(render::Transform{glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.f, glm::radians(-90.f), 0.f))},
                    render::MeshRenderer{backpack, shader_program});

    entities.create(render::Transform{glm::vec3(5.0f, 0.0f, 3.0f),
                                      glm::quat(glm::vec3(0.0f, glm::radians(0.f), 0.f))},
                    render::MeshRenderer{backpack, shader_program});

    entities.create(render::Transform{glm::vec3(5.0f, 0.0f, 3.0f),
                                      glm::quat(glm::vec3(0.1f, glm::radians(0.f), 0.f))},
                    render::MeshRenderer{backpack, shader_program});


    entities.create(sun);
    for (auto pos : govno_lampa_positions) {
        entities.create(render::Transform{pos}, govno_lampa, render::MeshRenderer{cube, light_cube_program});
    }



    dispatcher.loop();
}