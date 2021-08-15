#include "doctest.h"

#include "ecs/world.h"

TEST_SUITE("ecs::World") {
    TEST_CASE("double get") {
        ecs::World world;

        int a = 5, b = 6;
        auto u = std::make_unique<int>(a);
        world.insert<int>(std::move(u));

        CHECK(world.get<int>() == a);
        world.get<int>() = b;
        CHECK(world.get<int>() == b);
    }

    TEST_CASE("insert") {
        ecs::World world;

        SUBCASE("insert and get") {
            int val = 5;
            auto u = std::make_unique<int>(val);

            world.insert<int>(std::move(u));
            CHECK(world.get<int>() == val);
        }

        SUBCASE("has inserted") {
            auto u = std::make_unique<int>(5);

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

            world.insert<int>(std::move(u1));
            world.insert<int>(std::move(u2));
            CHECK(world.has<int>());
            CHECK(world.get<int>() == val2);
        }
    }

    TEST_CASE("emplace") {
        ecs::World world;

        SUBCASE("emplace and get") {
            using T = std::pair<int, int>;
            std::pair<int, int> val(5, 6);
            world.emplace<T>(val.first, val.second);
            CHECK(world.get<T>() == val);
        }

        SUBCASE("has emplaced") {
            using T = std::pair<int, int>;
            world.emplace<T>(5, 6);
            CHECK(world.has<T>());
        }

        SUBCASE("reemplace") {
            using T = std::pair<int, int>;
            int a = 5, b = 6;

            world.emplace<T>(a, b);
            world.emplace<T>(a, b);
            CHECK(world.has<T>());
            CHECK(world.get<T>() == T(a, b));
        }
    }

    TEST_CASE("erase") {
        ecs::World world;

        SUBCASE("has not") {
            auto u = std::make_unique<int>(5);

            world.insert<int>(std::move(u));
            world.erase<int>();
            CHECK(!world.has<int>());
        }

        SUBCASE("reinsert") {
            int val1 = 5, val2 = 6;
            auto u1 = std::make_unique<int>(val1);
            auto u2 = std::make_unique<int>(val2);

            world.insert<int>(std::move(u1));
            world.erase<int>();
            world.insert<int>(std::move(u2));
            CHECK(world.has<int>());
            CHECK(world.get<int>() == val2);
        }
    }
}