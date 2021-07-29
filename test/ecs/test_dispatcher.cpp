#include "doctest.h"

#include "ecs/dispatcher.h"

namespace {
    struct SomeRes {
        int a = 0;
    };

    struct OtherRes {
        int a = 0;
    };

    class SomeSystem {
    public:
        void update(const SomeRes &read_some, OtherRes &write_other) {
            if (read_some.a == 1)
                write_other.a = 2;
        }
    };

    class OtherSystem {
    public:
        void update(SomeRes &write_some, const OtherRes &read_other) {
            if (read_other.a == 0)
                write_some.a = 1;
        }
    };
}

TEST_SUITE("dispatcher") {
    TEST_CASE("init resources") {
        ecs::Dispatcher<SomeSystem, OtherSystem> dispatcher;
        ecs::World &world = dispatcher.get_world();
        CHECK(world.has<SomeRes>());
        CHECK(world.has<OtherRes>());
    }

    TEST_CASE("update resources") {
        ecs::Dispatcher<SomeSystem, OtherSystem> dispatcher;
        ecs::World &world = dispatcher.get_world();

        // Order of execution is not guaranteed.
        dispatcher.update();
        dispatcher.update();

        CHECK(world.get<SomeRes>().a == 1);
        CHECK(world.get<OtherRes>().a == 2);
    }
}