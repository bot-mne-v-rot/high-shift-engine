#include "doctest.h"

#include "ecs/storages/vec_storage.h"

namespace {
    struct SomeComponent {
        using Storage = ecs::VecStorage<SomeComponent>;
        int a = 0;

        bool operator==(const SomeComponent &) const = default;
        bool operator!=(const SomeComponent &) const = default;
    };

    struct IdComponent {
        using Storage = ecs::VecStorage<IdComponent>;
        ecs::Id id;

        bool operator==(const IdComponent &) const = default;
        bool operator!=(const IdComponent &) const = default;
    };

    static_assert(ecs::Component<SomeComponent>);
    static_assert(ecs::Component<IdComponent>);
}

TEST_SUITE("ecs/storages/VecStorage") {
    TEST_CASE("empty") {
        ecs::VecStorage<SomeComponent> storage;
        CHECK(storage.size() == 0);
        CHECK(storage.empty());
        CHECK(!storage.contains(0));
        CHECK(!storage.contains(45345231));
    }

    TEST_CASE("insert") {
        SomeComponent component{3};
        ecs::Id id = 100;

        ecs::VecStorage<SomeComponent> storage;
        CHECK(!storage.contains(id));

        storage.insert(id, component);

        SUBCASE("has inserted") {
            CHECK(storage.contains(id));
            CHECK(storage[id] == component);
        }

        SUBCASE("enough space") {
            CHECK(storage.capacity() > id);
            CHECK(storage.size() == 1);
        }

        SomeComponent c1{3}, c2{4}, c3{5};
        ecs::Id id1 = 10, id2 = 20, id3 = 30;

        storage.insert(id2, c2);
        storage.insert(id1, c1);
        storage.insert(id3, c3);

        SUBCASE("has inserted more") {
            CHECK(storage.contains(id1));
            CHECK(storage.contains(id2));
            CHECK(storage.contains(id3));
            CHECK(storage[id1] == c1);
            CHECK(storage[id2] == c2);
            CHECK(storage[id3] == c3);
        }

        SUBCASE("enough space again") {
            CHECK(storage.capacity() > id3);
            CHECK(storage.size() == 4);
        }

        SUBCASE("insert again") {
            storage.insert(id2, component);
            CHECK(storage.contains(id2));
            CHECK(storage[id2] == component);
            CHECK(storage.size() == 4);
        }
    }

    TEST_CASE("iterator") {
        SomeComponent c1{3}, c2{4}, c3{5};
        ecs::Id id1 = 10, id2 = 20, id3 = 30;

        ecs::VecStorage<SomeComponent> storage;
        storage.insert(id2, c2);
        storage.insert(id1, c1);
        storage.insert(id3, c3);

        SUBCASE("go through") {
            auto begin = storage.begin();
            auto end = storage.end();

            CHECK(*begin == c1);
            ++begin;
            CHECK(*begin == c2);
            ++begin;
            CHECK(*begin == c3);
            ++begin;
            CHECK(begin == end);
        }

        storage.erase(id2);

        SUBCASE("go through again") {
            auto begin = storage.begin();
            auto end = storage.end();

            CHECK(*begin++ == c1);
            CHECK(*begin++ == c3);
            CHECK(begin == end);
        }
    }

    TEST_CASE("with id") {
        ecs::Id ids[] = {1, 2, 50, 100};

        ecs::VecStorage<IdComponent> storage;
        for (auto id : ids)
            storage.insert(id, { id });

        for (auto[id, comp] : storage.with_id()) {
            CHECK(id == comp.id);
            static_assert(std::is_same_v<decltype(comp), IdComponent &>);
        }
    }
}