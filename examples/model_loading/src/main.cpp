#include "ecs/ecs.h"
#include "render/render_system.h"

int main() {
    ecs::Dispatcher<render::RenderSystem> dispatcher;

    dispatcher.run();
}