#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "texture.h"
#include "ecs/ecs.h"

#include <iostream>
#include <cmath>

void framebuffer_size_callback(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
}

glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camera_rot = glm::vec3(-90.0f, 0.0f, 0.0f); // yaw, pitch, roll
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

float delta_time = 0.0f;    // Time between current frame and last frame
float last_frame = 0.0f; // Time of last frame

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    auto w = (float) (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    auto a = (float) (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    auto s = (float) (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    auto d = (float) (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    auto space = (float) (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    auto shift = (float) (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

    const float camera_speed = 3.0f; // adjust accordingly
    float up = space - shift, right = d - a, front = w - s;

    auto &yaw = camera_rot.x;
    auto &pitch = camera_rot.y;
    auto &roll = camera_rot.z;

    glm::vec3 look_dir;
    look_dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    look_dir.y = sin(glm::radians(pitch));
    look_dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    camera_front = look_dir;

    glm::mat3 basis;
    basis[0] = camera_up;
    basis[1] = glm::normalize(glm::cross(camera_front, camera_up));
    basis[2] = camera_front;

    glm::vec3 local_move_dir(up, right, front);
    if (local_move_dir != glm::vec3(0, 0, 0))
        local_move_dir = glm::normalize(local_move_dir);

    glm::vec3 move_dir = basis * local_move_dir;

    float current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    camera_pos += move_dir * camera_speed * delta_time;
}


void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    static float lastX, lastY;
    static bool first_mouse = true;
    if (first_mouse) {
        lastX = xpos;
        lastY = ypos;
        first_mouse = false;
    }

    auto &yaw = camera_rot.x;
    auto &pitch = camera_rot.y;
    auto &roll = camera_rot.z;

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 look_dir;
    look_dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    look_dir.y = sin(glm::radians(pitch));
    look_dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    camera_front = look_dir;
}

std::filesystem::path vertex_shader_path = std::filesystem::current_path() / "shaders" / "cube.vert";
std::filesystem::path fragment_shader_path = std::filesystem::current_path() / "shaders" / "cube.frag";

tl::expected<std::shared_ptr<ShaderProgram>, std::string> setup_shaders() {
    Shader vertex_shader(Shader::type::vertex);
    if (auto result = vertex_shader.load_from_file(vertex_shader_path)) {}
    else return tl::make_unexpected("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + result.error());

    Shader fragment_shader(Shader::type::fragment);
    if (auto result = fragment_shader.load_from_file(fragment_shader_path)) {}
    else return tl::make_unexpected("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" + result.error());

    ShaderProgram* program = new ShaderProgram;
    program->attach(vertex_shader);
    program->attach(fragment_shader);

    if (auto result = program->link()) {}
    else return tl::make_unexpected("ERROR::SHADER::LINKING_FAILED\n" + result.error());

    return std::shared_ptr<ShaderProgram>(program);
}


inline glm::mat4 get_view() {
    return glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
}

inline glm::mat4 get_projectioon() {
    return glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
}

struct CubeData {
    CubeData() {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // loading vertices
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);
        // texture attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glActiveTexture(GL_TEXTURE0);
        texture0.bind();
        texture0.load_texture("assets/container.jpg", GL_RGB);
        glActiveTexture(GL_TEXTURE1);
        texture1.bind();
        texture1.load_texture("assets/awesomeface.png", GL_RGBA);

    }

    GLuint VAO = 0;
    Texture2d texture0, texture1;
    ShaderProgram shader_program;
    float vertices[180] {
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
};

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

struct TransformComponent {
    using Storage = ecs::VecStorage<TransformComponent>;
    glm::vec3 position;
};


struct ShaderComponent {
    using Storage = ecs::VecStorage<ShaderComponent>;
    std::shared_ptr<ShaderProgram> program;
};

class RenderSystem {
public:
    void update(const TransformComponent::Storage & transform_s, const ShaderComponent::Storage & shader_s, CubeData & cube_data) {
        for (auto[transform_cmp, shader_cmp] : ecs::join(transform_s, shader_s)) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), transform_cmp.position);
            glm::mat4 mapping = get_projectioon() * get_view() * model;

            shader_cmp.program->use();
            int modelLoc = glGetUniformLocation(shader_cmp.program->id, "mapping");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mapping));

            glBindVertexArray(cube_data.VAO);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
};



int main(int argc, char *argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Я ебал твою тёлку. У!", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);

    ecs::Dispatcher<RenderSystem> render_dispatcher;
    render_dispatcher.get_world().emplace<TransformComponent::Storage>();
    render_dispatcher.get_world().emplace<ShaderComponent::Storage>();

    int i = 0;
    auto result_shader_program = setup_shaders();
    if (!result_shader_program) std::cerr << result_shader_program.error();
    auto shader_program = std::shared_ptr(result_shader_program.value());
    shader_program->use();
    shader_program->setInt("texture0", 0);
    shader_program->setInt("texture1", 1);

    for (auto pos : cube_positions) {
        ecs::Resource auto& t_s = render_dispatcher.get_world().get<TransformComponent::Storage>();
        t_s.insert(i, TransformComponent{pos});
        ecs::Resource auto& s_s = render_dispatcher.get_world().get<ShaderComponent::Storage>();
        s_s.insert(i++, ShaderComponent{shader_program});
    }

    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_dispatcher.update();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}