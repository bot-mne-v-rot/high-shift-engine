#ifndef HIGH_SHIFT_WINDOW_SYSTEM_H
#define HIGH_SHIFT_WINDOW_SYSTEM_H

#include "ecs/ecs.h"

namespace render {
    struct WindowArgs {
        std::size_t width;
        std::size_t height;
        std::string title;
    };

    class WindowSystem {
    public:
        explicit WindowSystem(WindowArgs args);
        ~WindowSystem();

        WindowSystem(const WindowSystem &) = delete;
        WindowSystem &operator=(const WindowSystem &) = delete;

        tl::expected<void, std::string> setup(ecs::World &world);
        void update(ecs::GameLoopControl &game_loop);
        void teardown();

        struct WindowData;
        const WindowData &get_window_data() const;

    private:
        class Impl;

        Impl *impl = nullptr;
    };

    static_assert(ecs::System<WindowSystem>);
}

#endif //HIGH_SHIFT_WINDOW_SYSTEM_H
