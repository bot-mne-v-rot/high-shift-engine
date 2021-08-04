#ifndef HIGH_SHIFT_RENDER_SYSTEM_H
#define HIGH_SHIFT_RENDER_SYSTEM_H

#include "ecs/ecs.h"

namespace render {
    class RenderSystem {
    public:
        RenderSystem();
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void setup(ecs::World &world);
        void update(ecs::GameLoopControl &game_loop);
        void teardown(ecs::World &world);

    private:
        struct Impl;
        Impl *impl;
    };
}

#endif //HIGH_SHIFT_RENDER_SYSTEM_H
