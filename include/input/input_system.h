#ifndef HIGH_SHIFT_INPUT_SYSTEM_H
#define HIGH_SHIFT_INPUT_SYSTEM_H

#include "ecs/ecs.h"
#include "render/window_system.h"
#include "input/input.h"

namespace input {
    class InputSystem {
    public:
        InputSystem();
        ~InputSystem();

        void setup(ecs::World &world, const render::WindowSystem &window_system);
        void update(Input &input);

    private:
        class Impl;

        Impl *impl = nullptr;
    };

    static_assert(ecs::System<InputSystem>);
}

#endif //HIGH_SHIFT_INPUT_SYSTEM_H
