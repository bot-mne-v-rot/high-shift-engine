#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

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


float vertices[] = {
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

std::filesystem::path vertex_shader_path = std::filesystem::current_path() / "shaders" / "cube.vert";
std::filesystem::path fragment_shader_path = std::filesystem::current_path() / "shaders" / "cube.frag";

tl::expected<shader_program, std::string> setup_shaders() {
    shader vertex_shader(shader::type::vertex);
    if (auto result = vertex_shader.load_from_file(vertex_shader_path)) {}
    else return tl::make_unexpected("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + result.error());

    shader fragment_shader(shader::type::fragment);
    if (auto result = fragment_shader.load_from_file(fragment_shader_path)) {}
    else return tl::make_unexpected("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" + result.error());

    shader_program program;
    program.attach(vertex_shader);
    program.attach(fragment_shader);

    if (auto result = program.link()) {}
    else return tl::make_unexpected("ERROR::SHADER::LINKING_FAILED\n" + result.error());

    return program;
}

struct cube {
    GLuint VAO = 0;
    GLuint texture0 = 0;
    GLuint texture1 = 0;
    shader_program program;
};

tl::expected<GLuint, std::string> create_texture(const char *filename, GLenum format) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    uint8_t *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        return tl::make_unexpected<std::string>("Failed to load image");
    }
    stbi_image_free(data);

    return texture;
}

tl::expected<cube, std::string> setup_cube() {
    // VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

//    // EBO
//    GLuint EBO;
//    glGenBuffers(1, &EBO);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // VBO
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

    GLuint texture0;
    if (auto result = create_texture("assets/container.jpg", GL_RGB))
        texture0 = result.value();
    else return tl::make_unexpected(std::move(result.error()));

    GLuint texture1;
    if (auto result = create_texture("assets/awesomeface.png", GL_RGBA))
        texture1 = result.value();
    else return tl::make_unexpected(std::move(result.error()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
    trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));

    // shaders
    return setup_shaders()
            .map([&](shader_program program) {
                program.use();
                program.setInt("texture0", 0);
                program.setInt("texture1", 1);

                return cube{
                        VAO, texture0, texture1, std::move(program)
                };
            });
}

void render_cube(const cube &tri, glm::vec3 pos) {
    glBindVertexArray(tri.VAO);
    tri.program.use();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
//    model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

    glm::mat4 view;
    view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glm::mat4 mapping = projection * view * model;

    int modelLoc = glGetUniformLocation(tri.program.id, "mapping");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mapping));

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

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

    cube tri;
    if (auto result = setup_cube()) {
        tri = std::move(result.value());
    } else {
        std::cerr << result.error();
        return 1;
    }

    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto &cube_position : cube_positions)
            render_cube(tri, cube_position);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}