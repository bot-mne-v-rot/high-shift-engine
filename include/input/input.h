#ifndef HIGH_SHIFT_INPUT_H
#define HIGH_SHIFT_INPUT_H

#include "input/key_code.h"

#include <glm/glm.hpp>

namespace input {
    class InputSystem;

    struct KeyEvent {
        KeyCode key;
        enum struct Type {
            press,
            release
        } type;
    };

    class Input {
    public:
        /**
         * Tracks if button was pressed.
         * @return true if button is down but was up on the previous frame.
         */
        bool on_key_down(KeyCode key) const {
            return keys_pressed_state[key];
        }

        /**
         * Tracks button was released.
         * @return true if button is up but was down on the previous frame.
         */
        bool on_key_up(KeyCode key) const {
            return keys_released_state[key];
        }

        /**
         * Tracks button is held.
         * @return true if button is down.
         */
        bool is_key_down(KeyCode key) const {
            return keys_state[key];
        }

        /**
         * @return Current mouse position.
         */
        glm::vec2 get_mouse_pos() const {
            return mouse_pos;
        }

        /**
         * @return Mouse transition between frames.
         */
        glm::vec2 get_mouse_pos_delta() const {
            return mouse_pos_delta;
        }

    private:
        friend InputSystem;

        void clear_transitions() {
            memset(keys_pressed_state, false, sizeof(keys_pressed_state));
            memset(keys_released_state, false, sizeof(keys_released_state));
        }

        void process_key_event(KeyEvent event) {
            switch (event.type) {
                case KeyEvent::Type::press:
                    keys_pressed_state[event.key] = true;
                    keys_state[event.key] = true;
                    break;
                case KeyEvent::Type::release:
                    keys_released_state[event.key] = true;
                    keys_state[event.key] = false;
                    break;
            }
        }

        void set_mouse_pos(glm::vec2 new_mouse_pos) {
            if (first_mouse_update)
                first_mouse_update = false;
            else
                mouse_pos_delta = new_mouse_pos - mouse_pos;

            mouse_pos = new_mouse_pos;
        }

        bool keys_state[keys_num] = {};
        bool keys_pressed_state[keys_num] = {};
        bool keys_released_state[keys_num] = {};

        glm::vec2 mouse_pos;
        glm::vec2 mouse_pos_delta;
        bool first_mouse_update = true;
    };
}

#endif //HIGH_SHIFT_INPUT_H
