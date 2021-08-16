#include "doctest.h"

#include "ecs/entities.h"

#include <iostream>

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

TEST_SUITE("ecs::Entities") {
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

            SUBCASE("id reuse") {
                ecs::Entity ent2 = entities.create(ComponentC{});
                CHECK(ent2.id == entity.id);
                CHECK(ent2.version != entity.version);

                CHECK(entities.is_alive(ent2));
                CHECK(!entities.is_alive(entity));
            }
        }
    }

    TEST_CASE("foreach") {
        ecs::Entities entities;

        int n = 10000000;
        int m = 5000000;
        std::vector<bool> visited(n + m);

        std::vector<ComponentA> a_comps(n + m);
        std::vector<ComponentB> b_comps(n);
        std::vector<ComponentC> c_comps(n + m);

        for (int i = 0; i < n; ++i)
            a_comps[i].x = i, b_comps[i].y = i, c_comps[i].z = 2 * i;
        for (int i = n; i < n + m; ++i)
            a_comps[i].x = i, c_comps[i].z = 2 * i;

        ecs::ComponentType types1[] {
            ecs::ComponentType::create<ComponentA>(),
            ecs::ComponentType::create<ComponentB>(),
            ecs::ComponentType::create<ComponentC>()
        };
        void *data1[] {
            a_comps.data(), b_comps.data(), c_comps.data()
        };

        ecs::ComponentType types2[] {
            ecs::ComponentType::create<ComponentC>(),
            ecs::ComponentType::create<ComponentA>()
        };
        void *data2[] {
            c_comps.data() + n, a_comps.data() + n
        };

        entities.create_multiple(n, 3, types1, data1);
        entities.create_multiple(m, 2, types2, data2);

        SUBCASE("plain foreach") {
            entities.foreach([&](const ComponentA &a, const ComponentC &c) {
                visited[a.x] = true;
                CHECK(2 * a.x == c.z);
            });
        }

        SUBCASE("foreach with entities") {
            entities.foreach([&](ecs::Entity entity, const ComponentA &a, const ComponentC &c) {
                visited[a.x] = true;
                CHECK(entity.id == a.x);
                CHECK(2 * a.x == c.z);
            });
        }

        CHECK(std::find(visited.begin(), visited.end(), false) - visited.begin() == n + m);
    }

    TEST_CASE("add components") {
        ecs::Entities entities;
        ComponentA a{100};
        ComponentB b{200};
        ComponentC c{300};

        ecs::Entity e = entities.create(a);
        entities.add_components(e, b, c);

        bool visited = false;
        entities.foreach([&](ecs::Entity ent,
                             const ComponentA &ca, const ComponentB &cb, const ComponentC &cc) {
            CHECK(ent == e);
            CHECK(ca.x == a.x);
            CHECK(cb.y == b.y);
            CHECK(cc.z == c.z);
            visited = true;
        });
        CHECK(visited);

        int count_visited = 0;
        entities.foreach([&](ecs::Entity ent, const ComponentA &ca) {
            CHECK(ent == e);
            CHECK(ca.x == a.x);
            ++count_visited;
        });
        CHECK(count_visited == 1);
    }

    TEST_CASE("remove components") {
        ecs::Entities entities;
        ComponentA a{100};
        ComponentB b{200};
        ComponentC c{300};

        ecs::Entity e = entities.create(a, b, c);
        entities.remove_components<ComponentA, ComponentC>(e);

        bool visited = false;
        entities.foreach([&](ecs::Entity ent,
                const ComponentA &ca, const ComponentB &cb, const ComponentC &cc) {
            visited = true;
        });
        CHECK(!visited);

        int count_visited = 0;
        entities.foreach([&](ecs::Entity ent, const ComponentB &cb) {
            CHECK(ent == e);
            CHECK(cb.y == b.y);
            ++count_visited;
        });
        CHECK(count_visited == 1);
    }
}