#include "doctest.h"

#include "ecs/world.h"

TEST_SUITE("ecs/world") {
    TEST_CASE("insert") {
        SUBCASE("insert and get") {
            int val = 5;
            auto u = std::make_unique<int>(val);

            ecs::World world;
            world.insert<int>(std::move(u));
            CHECK(world.get<int>() == val);
        }

        SUBCASE("has inserted") {
            auto u = std::make_unique<int>(5);

            ecs::World world;
            world.insert<int>(std::move(u));
            CHECK(world.has<int>());
        }

        SUBCASE("insert different resources") {
            using T1 = int;
            using T2 = double;
            T1 val1 = 5;
            T2 val2 = 5.0;
            auto u1 = std::make_unique<T1>(val1);
            auto u2 = std::make_unique<T2>(val2);

            ecs::World world;
            world.insert<T1>(std::move(u1));
            world.insert<T2>(std::move(u2));
            CHECK(world.get<T1>() == val1);
            CHECK(world.get<T2>() == val2);
            CHECK(world.has<T1>());
            CHECK(world.has<T2>());
        }

        SUBCASE("reinsert") {
            int val1 = 5, val2 = 6;
            auto u1 = std::make_unique<int>(val1);
            auto u2 = std::make_unique<int>(val2);

            ecs::World world;
            world.insert<int>(std::move(u1));
            world.insert<int>(std::move(u2));
            CHECK(world.has<int>());
            CHECK(world.get<int>() == val2);
        }
    }

    TEST_CASE("emplace") {
        SUBCASE("emplace and get") {
            using T = std::pair<int, int>;
            std::pair<int, int> val(5, 6);
            ecs::World world;
            world.emplace<T>(val.first, val.second);
            CHECK(world.get<T>() == val);
        }

        SUBCASE("has emplaced") {
            using T = std::pair<int, int>;
            ecs::World world;
            world.emplace<T>(5, 6);
            CHECK(world.has<T>());
        }

        SUBCASE("reemplace") {
            using T = std::pair<int, int>;
            int a = 5, b = 6;

            ecs::World world;
            world.emplace<T>(a, b);
            world.emplace<T>(a, b);
            CHECK(world.has<T>());
            CHECK(world.get<T>() == T(a, b));
        }
    }

    TEST_CASE("erase") {
        SUBCASE("has not") {
            auto u = std::make_unique<int>(5);

            ecs::World world;
            world.insert<int>(std::move(u));
            world.erase<int>();
            CHECK(!world.has<int>());
        }

        SUBCASE("reinsert") {
            int val1 = 5, val2 = 6;
            auto u1 = std::make_unique<int>(val1);
            auto u2 = std::make_unique<int>(val2);

            ecs::World world;
            world.insert<int>(std::move(u1));
            world.erase<int>();
            world.insert<int>(std::move(u2));
            CHECK(world.has<int>());
            CHECK(world.get<int>() == val2);
        }
    }
}