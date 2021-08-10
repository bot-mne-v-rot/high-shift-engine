#include "doctest.h"

#include "ecs/system.h"

namespace {
    struct SomeRes {
    };

    struct OtherRes {
    };

    class ExampleSystem {
    public:
        void update(const SomeRes &, OtherRes &) {}
    };

    static_assert(ecs::System<ExampleSystem>);
}