#include "doctest.h"

#include "ecs/entities.h"

namespace {
    struct ComponentA {
        int x;
    };

    struct ComponentB {
        int y;
    };

    struct ComponentC {
        int z;
    };

    static_assert(ecs::Component<ComponentA>);
    static_assert(ecs::Component<ComponentB>);
    static_assert(ecs::Component<ComponentC>);
}

TEST_SUITE("ecs/entities") {
    TEST_CASE("basic") {
        ecs::Entities entities;

        ecs::Entity entity = entities.create(ComponentA{},
                                             ComponentB{});

        SUBCASE("creation") {
            CHECK(entities.is_alive(entity));
        }

        SUBCASE("destruction") {
            CHECK(entities.destroy(entity));
            CHECK(!entities.is_alive(entity));
        }
    }

    TEST_CASE("foreach") {
        ecs::Entities entities;

        int n = 10000;
        int m = 5000;
        std::vector<bool> visited(n + m);

        for (int i = 0; i < n; ++i)
            entities.create(ComponentA{i}, ComponentB{i}, ComponentC{i});
        for (int i = n; i < n + m; ++i)
            entities.create(ComponentA{i}, ComponentC{i});

        entities.foreach<ComponentA, ComponentC>([&](auto &a, auto &c) {
            visited[a.x] = true;
            a.x = c.z;
        });

        CHECK(std::find(visited.begin(), visited.end(), false) - visited.begin() == n + m);
    }
}