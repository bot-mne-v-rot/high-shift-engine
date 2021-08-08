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
        void setup(ecs::World &) {
            was_setup = true;
        }

        void update(SomeRes &write_some, const OtherRes &read_other) {
            if (read_other.a == 0)
                write_some.a = 1;
        }

        bool was_setup = false;
    };

    class OtherSystem {
    public:
        void setup(ecs::World &, const SomeSystem &dep) {
            dep_was_setup = dep.was_setup;
            was_setup = true;
        }

        void update(const SomeRes &read_some, OtherRes &write_other) {
            if (read_some.a == 1)
                write_other.a = 2;
        }

        bool dep_was_setup = false;
        bool was_setup = false;
    };

    class ThirdSystem {
    public:
        void setup(ecs::World &, const OtherSystem &dep) {
            dep_was_setup = dep.was_setup;
            was_setup = true;
        }

        void update() {}

        bool dep_was_setup = false;
        bool was_setup = false;
    };

    class TwoDepsSystem {
    public:
        void setup(ecs::World &, const SomeSystem &dep1, const OtherSystem &dep2) {
            dep1_was_setup = dep1.was_setup;
            dep2_was_setup = dep2.was_setup;
            was_setup = true;
        }

        void update() {}

        bool dep1_was_setup = false;
        bool dep2_was_setup = false;
        bool was_setup = false;
    };

    static_assert(ecs::System<SomeSystem>);
    static_assert(ecs::System<OtherSystem>);
    static_assert(ecs::System<ThirdSystem>);
    static_assert(ecs::System<TwoDepsSystem>);
}

TEST_SUITE("dispatcher") {
    TEST_CASE("init resources") {
        auto dispatcher_result = ecs::Dispatcher::create<SomeSystem, OtherSystem>();
        REQUIRE(dispatcher_result);

        auto dispatcher = std::move(dispatcher_result.value());
        ecs::World &world = dispatcher.get_world();

        CHECK(world.has<SomeRes>());
        CHECK(world.has<OtherRes>());
    }

    TEST_CASE("update resources") {
        auto dispatcher_result = ecs::Dispatcher::create<OtherSystem, SomeSystem>();
        REQUIRE(dispatcher_result);

        auto dispatcher = std::move(dispatcher_result.value());
        ecs::World &world = dispatcher.get_world();

        // A system is executed after its dependencies
        dispatcher.update();

        CHECK(world.get<SomeRes>().a == 1);
        CHECK(world.get<OtherRes>().a == 2);
    }
}