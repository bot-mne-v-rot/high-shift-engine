#include "input/input_system.h"

#include "../render/window_data.h"

#include <unordered_map>
#include <iostream>

namespace input {
    std::unordered_map<int, KeyCode> key_map;

    static void setup_key_map() {
        key_map[GLFW_KEY_SPACE] = KeyCode::KEY_SPACE;
        key_map[GLFW_KEY_APOSTROPHE] = KeyCode::KEY_APOSTROPHE;
        key_map[GLFW_KEY_COMMA] = KeyCode::KEY_COMMA;
        key_map[GLFW_KEY_MINUS] = KeyCode::KEY_MINUS;
        key_map[GLFW_KEY_PERIOD] = KeyCode::KEY_PERIOD;
        key_map[GLFW_KEY_SLASH] = KeyCode::KEY_SLASH;
        key_map[GLFW_KEY_0] = KeyCode::KEY_0;
        key_map[GLFW_KEY_1] = KeyCode::KEY_1;
        key_map[GLFW_KEY_2] = KeyCode::KEY_2;
        key_map[GLFW_KEY_3] = KeyCode::KEY_3;
        key_map[GLFW_KEY_4] = KeyCode::KEY_4;
        key_map[GLFW_KEY_5] = KeyCode::KEY_5;
        key_map[GLFW_KEY_6] = KeyCode::KEY_6;
        key_map[GLFW_KEY_7] = KeyCode::KEY_7;
        key_map[GLFW_KEY_8] = KeyCode::KEY_8;
        key_map[GLFW_KEY_9] = KeyCode::KEY_9;
        key_map[GLFW_KEY_SEMICOLON] = KeyCode::KEY_SEMICOLON;
        key_map[GLFW_KEY_EQUAL] = KeyCode::KEY_EQUAL;
        key_map[GLFW_KEY_A] = KeyCode::KEY_A;
        key_map[GLFW_KEY_B] = KeyCode::KEY_B;
        key_map[GLFW_KEY_C] = KeyCode::KEY_C;
        key_map[GLFW_KEY_D] = KeyCode::KEY_D;
        key_map[GLFW_KEY_E] = KeyCode::KEY_E;
        key_map[GLFW_KEY_F] = KeyCode::KEY_F;
        key_map[GLFW_KEY_G] = KeyCode::KEY_G;
        key_map[GLFW_KEY_H] = KeyCode::KEY_H;
        key_map[GLFW_KEY_I] = KeyCode::KEY_I;
        key_map[GLFW_KEY_J] = KeyCode::KEY_J;
        key_map[GLFW_KEY_K] = KeyCode::KEY_K;
        key_map[GLFW_KEY_L] = KeyCode::KEY_L;
        key_map[GLFW_KEY_M] = KeyCode::KEY_M;
        key_map[GLFW_KEY_N] = KeyCode::KEY_N;
        key_map[GLFW_KEY_O] = KeyCode::KEY_O;
        key_map[GLFW_KEY_P] = KeyCode::KEY_P;
        key_map[GLFW_KEY_Q] = KeyCode::KEY_Q;
        key_map[GLFW_KEY_R] = KeyCode::KEY_R;
        key_map[GLFW_KEY_S] = KeyCode::KEY_S;
        key_map[GLFW_KEY_T] = KeyCode::KEY_T;
        key_map[GLFW_KEY_U] = KeyCode::KEY_U;
        key_map[GLFW_KEY_V] = KeyCode::KEY_V;
        key_map[GLFW_KEY_W] = KeyCode::KEY_W;
        key_map[GLFW_KEY_X] = KeyCode::KEY_X;
        key_map[GLFW_KEY_Y] = KeyCode::KEY_Y;
        key_map[GLFW_KEY_Z] = KeyCode::KEY_Z;
        key_map[GLFW_KEY_LEFT_BRACKET] = KeyCode::KEY_LEFT_BRACKET;
        key_map[GLFW_KEY_BACKSLASH] = KeyCode::KEY_BACKSLASH;
        key_map[GLFW_KEY_RIGHT_BRACKET] = KeyCode::KEY_RIGHT_BRACKET;
        key_map[GLFW_KEY_GRAVE_ACCENT] = KeyCode::KEY_GRAVE_ACCENT;
        key_map[GLFW_KEY_WORLD_1] = KeyCode::KEY_WORLD_1;
        key_map[GLFW_KEY_WORLD_2] = KeyCode::KEY_WORLD_2;
        key_map[GLFW_KEY_ESCAPE] = KeyCode::KEY_ESCAPE;
        key_map[GLFW_KEY_ENTER] = KeyCode::KEY_ENTER;
        key_map[GLFW_KEY_TAB] = KeyCode::KEY_TAB;
        key_map[GLFW_KEY_BACKSPACE] = KeyCode::KEY_BACKSPACE;
        key_map[GLFW_KEY_INSERT] = KeyCode::KEY_INSERT;
        key_map[GLFW_KEY_DELETE] = KeyCode::KEY_DELETE;
        key_map[GLFW_KEY_RIGHT] = KeyCode::KEY_RIGHT;
        key_map[GLFW_KEY_LEFT] = KeyCode::KEY_LEFT;
        key_map[GLFW_KEY_DOWN] = KeyCode::KEY_DOWN;
        key_map[GLFW_KEY_UP] = KeyCode::KEY_UP;
        key_map[GLFW_KEY_PAGE_UP] = KeyCode::KEY_PAGE_UP;
        key_map[GLFW_KEY_PAGE_DOWN] = KeyCode::KEY_PAGE_DOWN;
        key_map[GLFW_KEY_HOME] = KeyCode::KEY_HOME;
        key_map[GLFW_KEY_END] = KeyCode::KEY_END;
        key_map[GLFW_KEY_CAPS_LOCK] = KeyCode::KEY_CAPS_LOCK;
        key_map[GLFW_KEY_SCROLL_LOCK] = KeyCode::KEY_SCROLL_LOCK;
        key_map[GLFW_KEY_NUM_LOCK] = KeyCode::KEY_NUM_LOCK;
        key_map[GLFW_KEY_PRINT_SCREEN] = KeyCode::KEY_PRINT_SCREEN;
        key_map[GLFW_KEY_PAUSE] = KeyCode::KEY_PAUSE;
        key_map[GLFW_KEY_F1] = KeyCode::KEY_F1;
        key_map[GLFW_KEY_F2] = KeyCode::KEY_F2;
        key_map[GLFW_KEY_F3] = KeyCode::KEY_F3;
        key_map[GLFW_KEY_F4] = KeyCode::KEY_F4;
        key_map[GLFW_KEY_F5] = KeyCode::KEY_F5;
        key_map[GLFW_KEY_F6] = KeyCode::KEY_F6;
        key_map[GLFW_KEY_F7] = KeyCode::KEY_F7;
        key_map[GLFW_KEY_F8] = KeyCode::KEY_F8;
        key_map[GLFW_KEY_F9] = KeyCode::KEY_F9;
        key_map[GLFW_KEY_F10] = KeyCode::KEY_F10;
        key_map[GLFW_KEY_F11] = KeyCode::KEY_F11;
        key_map[GLFW_KEY_F12] = KeyCode::KEY_F12;
        key_map[GLFW_KEY_F13] = KeyCode::KEY_F13;
        key_map[GLFW_KEY_F14] = KeyCode::KEY_F14;
        key_map[GLFW_KEY_F15] = KeyCode::KEY_F15;
        key_map[GLFW_KEY_F16] = KeyCode::KEY_F16;
        key_map[GLFW_KEY_F17] = KeyCode::KEY_F17;
        key_map[GLFW_KEY_F18] = KeyCode::KEY_F18;
        key_map[GLFW_KEY_F19] = KeyCode::KEY_F19;
        key_map[GLFW_KEY_F20] = KeyCode::KEY_F20;
        key_map[GLFW_KEY_F21] = KeyCode::KEY_F21;
        key_map[GLFW_KEY_F22] = KeyCode::KEY_F22;
        key_map[GLFW_KEY_F23] = KeyCode::KEY_F23;
        key_map[GLFW_KEY_F24] = KeyCode::KEY_F24;
        key_map[GLFW_KEY_F25] = KeyCode::KEY_F25;
        key_map[GLFW_KEY_KP_0] = KeyCode::KEY_KP_0;
        key_map[GLFW_KEY_KP_1] = KeyCode::KEY_KP_1;
        key_map[GLFW_KEY_KP_2] = KeyCode::KEY_KP_2;
        key_map[GLFW_KEY_KP_3] = KeyCode::KEY_KP_3;
        key_map[GLFW_KEY_KP_4] = KeyCode::KEY_KP_4;
        key_map[GLFW_KEY_KP_5] = KeyCode::KEY_KP_5;
        key_map[GLFW_KEY_KP_6] = KeyCode::KEY_KP_6;
        key_map[GLFW_KEY_KP_7] = KeyCode::KEY_KP_7;
        key_map[GLFW_KEY_KP_8] = KeyCode::KEY_KP_8;
        key_map[GLFW_KEY_KP_9] = KeyCode::KEY_KP_9;
        key_map[GLFW_KEY_KP_DECIMAL] = KeyCode::KEY_KP_DECIMAL;
        key_map[GLFW_KEY_KP_DIVIDE] = KeyCode::KEY_KP_DIVIDE;
        key_map[GLFW_KEY_KP_MULTIPLY] = KeyCode::KEY_KP_MULTIPLY;
        key_map[GLFW_KEY_KP_SUBTRACT] = KeyCode::KEY_KP_SUBTRACT;
        key_map[GLFW_KEY_KP_ADD] = KeyCode::KEY_KP_ADD;
        key_map[GLFW_KEY_KP_ENTER] = KeyCode::KEY_KP_ENTER;
        key_map[GLFW_KEY_KP_EQUAL] = KeyCode::KEY_KP_EQUAL;
        key_map[GLFW_KEY_LEFT_SHIFT] = KeyCode::KEY_LEFT_SHIFT;
        key_map[GLFW_KEY_LEFT_CONTROL] = KeyCode::KEY_LEFT_CONTROL;
        key_map[GLFW_KEY_LEFT_ALT] = KeyCode::KEY_LEFT_ALT;
        key_map[GLFW_KEY_LEFT_SUPER] = KeyCode::KEY_LEFT_SUPER;
        key_map[GLFW_KEY_RIGHT_SHIFT] = KeyCode::KEY_RIGHT_SHIFT;
        key_map[GLFW_KEY_RIGHT_CONTROL] = KeyCode::KEY_RIGHT_CONTROL;
        key_map[GLFW_KEY_RIGHT_ALT] = KeyCode::KEY_RIGHT_ALT;
        key_map[GLFW_KEY_RIGHT_SUPER] = KeyCode::KEY_RIGHT_SUPER;
        key_map[GLFW_KEY_MENU] = KeyCode::KEY_MENU;
    }

    std::vector<KeyEvent> key_events;

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS)
            key_events.push_back({.key = key_map[key], .type = KeyEvent::Type::press});
        else if (action == GLFW_RELEASE)
            key_events.push_back({.key = key_map[key], .type = KeyEvent::Type::release});
    }

    class InputSystem::Impl {
    public:
        void setup(ecs::World &, const render::WindowSystem &window_system) {
            window = window_system.get_window_data().window;
            glfwSetKeyCallback(window, key_callback);
            setup_key_map();
        }

        void update(Input &input) {
            update_keyboard(input);
            update_mouse(input);
        }

        void disable_cursor() {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        void enable_cursor() {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

    private:
        void update_keyboard(Input &input) {
            input.clear_transitions();
            for (KeyEvent event : key_events)
                input.process_key_event(event);
            key_events.clear();
        }

        void update_mouse(Input &input) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            input.set_mouse_pos(glm::vec2(xpos, ypos));
        }

        GLFWwindow *window = nullptr;
        bool fps_mouse = false;
    };

    InputSystem::InputSystem() {
        impl = new Impl();
    }

    InputSystem::~InputSystem() {
        delete impl;
    }

    void InputSystem::setup(ecs::World &world, const render::WindowSystem &window_system) {
        impl->setup(world, window_system);
    }

    void InputSystem::update(Input &input) {
        impl->update(input);
    }

    void InputSystem::disable_cursor() {
        impl->disable_cursor();
    }

    void InputSystem::enable_cursor() {
        impl->enable_cursor();
    }
}