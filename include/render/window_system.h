#ifndef HIGH_SHIFT_WINDOW_SYSTEM_H
#define HIGH_SHIFT_WINDOW_SYSTEM_H

#include "ecs/ecs.h"

namespace render {
    class WindowSystem {
    public:
        WindowSystem();
        ~WindowSystem();

        tl::expected<void, std::string> setup(ecs::World &world);
        void update(ecs::GameLoopControl &game_loop);
        void teardown();

        struct WindowData;
        const WindowData &get_window_data() const;

    private:
        class Impl;

        Impl *impl = nullptr;
    };
}

#endif //HIGH_SHIFT_WINDOW_SYSTEM_H
