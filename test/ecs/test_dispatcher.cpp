#include "doctest.h"

#include "ecs/dispatcher.h"

// Not used but https://github.com/onqtam/doctest/issues/126
#include <iostream>

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

    class CircularDepSystemB;

    class CircularDepSystemA {
    public:
        void setup(ecs::World &, const CircularDepSystemB &) {}

        void update() {}
    };

    class CircularDepSystemB {
    public:
        void setup(ecs::World &, const CircularDepSystemA &) {}

        void update() {}
    };

    class FailingSystem {
    public:
        tl::expected<void, std::string> setup(ecs::World &) {
            return tl::make_unexpected("I failed.");
        }

        void update() {}
    };

    static_assert(ecs::System<SomeSystem>);
    static_assert(ecs::System<OtherSystem>);
    static_assert(ecs::System<ThirdSystem>);
    static_assert(ecs::System<TwoDepsSystem>);
    static_assert(ecs::System<CircularDepSystemA>);
    static_assert(ecs::System<CircularDepSystemB>);
    static_assert(ecs::System<FailingSystem>);

    static_assert(ecs::SystemHasSetup<SomeSystem>);
    static_assert(ecs::SystemHasSetup<OtherSystem>);
    static_assert(ecs::SystemHasSetup<ThirdSystem>);
    static_assert(ecs::SystemHasSetup<TwoDepsSystem>);
    static_assert(ecs::SystemHasSetup<CircularDepSystemA>);
    static_assert(ecs::SystemHasSetup<CircularDepSystemB>);
    static_assert(ecs::SystemHasSetup<FailingSystem>);
}

TEST_SUITE("ecs::Dispatcher") {
    TEST_CASE("init resources") {
        auto dispatcher_result = ecs::Dispatcher::create<SomeSystem, OtherSystem>();
        REQUIRE(dispatcher_result);

        auto dispatcher = std::move(dispatcher_result.value());
        ecs::World &world = dispatcher.get_world();

        CHECK(world.has<SomeRes>());
        CHECK(world.has<OtherRes>());
    }

    TEST_CASE("order of creation") {
        SUBCASE("one depends on another") {
            auto dispatcher_result = ecs::Dispatcher::create<OtherSystem, SomeSystem>();
            REQUIRE(dispatcher_result);

            auto dispatcher = std::move(dispatcher_result.value());
            REQUIRE(dispatcher.has_system<SomeSystem>());
            REQUIRE(dispatcher.has_system<OtherSystem>());

            auto &some_system = dispatcher.get_system<SomeSystem>();
            auto &other_system = dispatcher.get_system<OtherSystem>();

            CHECK(some_system.was_setup);
            CHECK(other_system.was_setup);
            CHECK(other_system.dep_was_setup);
        }

        SUBCASE("transitive dependency") {
            auto dispatcher_result = ecs::Dispatcher::create<OtherSystem, ThirdSystem, SomeSystem>();
            REQUIRE(dispatcher_result);

            auto dispatcher = std::move(dispatcher_result.value());
            REQUIRE(dispatcher.has_system<SomeSystem>());
            REQUIRE(dispatcher.has_system<OtherSystem>());
            REQUIRE(dispatcher.has_system<ThirdSystem>());

            auto &some_system = dispatcher.get_system<SomeSystem>();
            auto &other_system = dispatcher.get_system<OtherSystem>();
            auto &third_system = dispatcher.get_system<ThirdSystem>();

            CHECK(some_system.was_setup);
            CHECK(other_system.was_setup);
            CHECK(other_system.dep_was_setup);
            CHECK(third_system.was_setup);
            CHECK(third_system.dep_was_setup);
        }

        SUBCASE("two dependencies") {
            auto dispatcher_result = ecs::Dispatcher::create<OtherSystem, TwoDepsSystem, SomeSystem>();
            REQUIRE(dispatcher_result);

            auto dispatcher = std::move(dispatcher_result.value());
            REQUIRE(dispatcher.has_system<SomeSystem>());
            REQUIRE(dispatcher.has_system<OtherSystem>());
            REQUIRE(dispatcher.has_system<TwoDepsSystem>());

            auto &some_system = dispatcher.get_system<SomeSystem>();
            auto &other_system = dispatcher.get_system<OtherSystem>();
            auto &two_deps_system = dispatcher.get_system<TwoDepsSystem>();

            CHECK(some_system.was_setup);
            CHECK(other_system.was_setup);
            CHECK(other_system.dep_was_setup);
            CHECK(two_deps_system.was_setup);
            CHECK(two_deps_system.dep1_was_setup);
            CHECK(two_deps_system.dep2_was_setup);
        }
    }

    TEST_CASE("dependency not satisfied") {
        auto dispatcher_result = ecs::Dispatcher::create<OtherSystem>();
        REQUIRE(!dispatcher_result);

        auto err = std::move(dispatcher_result.error());
        REQUIRE(std::holds_alternative<ecs::Dispatcher::DependencyError>(err));

        auto dep_err = std::move(std::get<ecs::Dispatcher::DependencyError>(err));

        CHECK(dep_err.type == ecs::Dispatcher::DependencyError::Type::unsatisfied);
        CHECK(dep_err.dependent_name == demangled_type_name<OtherSystem>());
        CHECK(dep_err.dependency_name == demangled_type_name<SomeSystem>());
    }

    TEST_CASE("circular dependency") {
        auto dispatcher_result = ecs::Dispatcher::create<CircularDepSystemA, CircularDepSystemB>();
        REQUIRE(!dispatcher_result);

        auto err = std::move(dispatcher_result.error());
        REQUIRE(std::holds_alternative<ecs::Dispatcher::DependencyError>(err));

        auto dep_err = std::move(std::get<ecs::Dispatcher::DependencyError>(err));

        CHECK(dep_err.type == ecs::Dispatcher::DependencyError::Type::circular);

        std::string demangled_a = demangled_type_name<CircularDepSystemA>();
        std::string demangled_b = demangled_type_name<CircularDepSystemB>();

        CHECK(((dep_err.dependent_name == demangled_a && dep_err.dependency_name == demangled_b) ||
               (dep_err.dependent_name == demangled_b && dep_err.dependency_name == demangled_a)));
    }

    TEST_CASE("failing system") {
        auto dispatcher_result = ecs::Dispatcher::create<FailingSystem>();
        REQUIRE(!dispatcher_result);

        auto err = std::move(dispatcher_result.error());
        REQUIRE(std::holds_alternative<ecs::Dispatcher::SystemError>(err));

        auto sys_err = std::move(std::get<ecs::Dispatcher::SystemError>(err));
        CHECK(sys_err.failed_system == demangled_type_name<FailingSystem>());
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