#ifndef HIGH_SHIFT_INPUT_H
#define HIGH_SHIFT_INPUT_H

#include "input/key_code.h"

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
        bool on_key_down(KeyCode key) {
            return keys_pressed_state[key];
        }

        /**
         * Tracks button was released.
         * @return true if button is up but was down on the previous frame.
         */
        bool on_key_up(KeyCode key) {
            return keys_released_state[key];
        }

        /**
         * Tracks button is held.
         * @return true if button is down.
         */
        bool is_key_down(KeyCode key) {
            return keys_state[key];
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

        bool keys_state[keys_num] = {};
        bool keys_pressed_state[keys_num] = {};
        bool keys_released_state[keys_num] = {};
    };
}

#endif //HIGH_SHIFT_INPUT_H
