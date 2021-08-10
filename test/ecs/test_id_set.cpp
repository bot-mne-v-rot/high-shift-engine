#include "doctest.h"

#include "ecs/id_set.h"

#include <climits>

TEST_SUITE("ecs/storages/IdSet") {
    TEST_CASE("empty") {
        ecs::IdSet set;
        CHECK(set.size() == 0);
        CHECK(set.empty());
        CHECK(!set.contains(0));
        CHECK(!set.contains(45345231));
    }

    TEST_CASE("insert") {
        ecs::Id id = 100;

        ecs::IdSet set;
        CHECK(!set.contains(id));

        set.insert(id);

        SUBCASE("has inserted") {
            CHECK(set.contains(id));
        }

        SUBCASE("enough space") {
            CHECK(set.capacity() > id);
            CHECK(set.size() == 1);
        }

        ecs::Id id1 = 10, id2 = 200, id3 = 30000;

        set.insert(id2);
        set.insert(id1);
        set.insert(id3);

        SUBCASE("has inserted more") {
            CHECK(set.contains(id1));
            CHECK(set.contains(id2));
            CHECK(set.contains(id3));
        }

        SUBCASE("enough space again") {
            CHECK(set.capacity() > id3);
            CHECK(set.size() == 4);
        }

        SUBCASE("insert again") {
            set.insert(id2);
            CHECK(set.contains(id2));
            CHECK(set.size() == 4);
        }
    }

    TEST_CASE("erase") {
        ecs::IdSet set;
        ecs::Id id1 = 100, id2 = 2000, id3 = 30000, nid = 5;

        set.insert(id2);
        set.insert(id1);
        set.insert(id3);

        CHECK(set.contains(id1));
        CHECK(set.contains(id2));
        CHECK(set.contains(id3));

        SUBCASE("erase existed") {
            set.erase(id1);
            CHECK(!set.contains(id1));
            CHECK(set.contains(id2));
            CHECK(set.contains(id3));
            CHECK(set.size() == 2);
        }

        SUBCASE("erase existed twice") {
            set.erase(id1);
            CHECK(!set.contains(id1));
            set.erase(id1);
            CHECK(!set.contains(id1));
            CHECK(set.size() == 2);
        }

        SUBCASE("erase not existed") {
            set.erase(nid);
            CHECK(set.contains(id1));
            CHECK(set.contains(id2));
            CHECK(set.contains(id3));
            CHECK(set.size() == 3);
        }

        SUBCASE("erase out of bounds") {
            set.erase(UINT32_MAX);
            CHECK(set.contains(id1));
            CHECK(set.contains(id2));
            CHECK(set.contains(id3));
            CHECK(set.size() == 3);
        }
    }

    TEST_CASE("clear") {
        ecs::IdSet set;
        constexpr ecs::Id ids[] = {10, 11, 12, 500, 1000, 1020, 5000, 100000, 100001, 1000000, 10000000};
        constexpr std::size_t ids_n = sizeof(ids) / sizeof(ecs::Id);

        for (ecs::Id id : ids)
            set.insert(id);
        CHECK(set.size() == ids_n);

        set.clear();
        CHECK(set.size() == 0);
        CHECK(set.empty());
    }

    TEST_CASE("iterator") {
        ecs::IdSet set;
        constexpr ecs::Id ids[] = {10, 11, 12, 500, 1000, 1020, 5000, 100000, 100001, 1000000, 10000000};
        constexpr std::size_t ids_n = sizeof(ids) / sizeof(ecs::Id);

        for (ecs::Id id : ids)
            set.insert(id);

        SUBCASE("go through") {
            std::size_t i = 0;
            ecs::foreach(set, [&](ecs::Id id) {
                CHECK(ids[i++] == id);
            });
            CHECK(i == ids_n);
        }

        SUBCASE("go through after erase") {
            std::size_t removed_idx = 1;
            set.erase(ids[removed_idx]);

            std::size_t i = 0;
            ecs::foreach(set, [&](ecs::Id id) {
                if (i < removed_idx)
                    CHECK(ids[i] == id);
                else
                    CHECK(ids[i + 1] == id);
                ++i;
            });
            CHECK(i == ids_n - 1);

            set.insert(ids[removed_idx]);
            SUBCASE("insert again") {
                i = 0;
                ecs::foreach(set, [&](ecs::Id id) {
                    CHECK(ids[i++] == id);
                });
                CHECK(i == ids_n);
            }
        }
    }

    TEST_CASE("bitwise and") {
        ecs::IdSet a, b;
        constexpr ecs::Id ids_a[] = {10, 11, 12, 1000, 5000, 100000, 1000000, 10000000};
        constexpr ecs::Id ids_b[] = {11, 12, 500, 1000, 1020, 5000, 100001, 1000000};
        constexpr ecs::Id ids_c[] = {11, 12, 1000, 5000, 1000000};
        constexpr std::size_t ids_n = sizeof(ids_c) / sizeof(ecs::Id);

        for (ecs::Id id : ids_a)
            a.insert(id);
        for (ecs::Id id : ids_b)
            b.insert(id);

        SUBCASE("emptiness") {
            CHECK(!(a & b).empty());
        }

        SUBCASE("first") {
            CHECK((a & b).first() == 11);
        }

        SUBCASE("iteration") {
            std::size_t i = 0;
            ecs::foreach(a & b, [&](ecs::Id id) {
                CHECK(ids_c[i++] == id);
            });
            CHECK(i == ids_n);
        }
    }

    TEST_CASE("bitwise or") {
        ecs::IdSet a, b;
        constexpr ecs::Id ids_a[] = {10, 11, 12, 1000, 5000, 100000, 1000000, 10000000};
        constexpr ecs::Id ids_b[] = {11, 12, 500, 1000, 1020, 5000, 100001, 1000000};
        constexpr ecs::Id ids_c[] = {10, 11, 12, 500, 1000, 1020, 5000, 100000, 100001, 1000000, 10000000};
        constexpr std::size_t ids_n = sizeof(ids_c) / sizeof(ecs::Id);

        for (ecs::Id id : ids_a)
            a.insert(id);
        for (ecs::Id id : ids_b)
            b.insert(id);

        auto c = a | b;

        SUBCASE("emptiness") {
            CHECK(!c.empty());
        }

        SUBCASE("first") {
            CHECK(c.first() == ids_c[0]);
        }

        SUBCASE("iteration") {
            std::size_t i = 0;
            ecs::foreach(c, [&](ecs::Id id) {
                CHECK(ids_c[i++] == id);
            });
            CHECK(i == ids_n);
        }
    }

    TEST_CASE("copy/move") {
        ecs::IdSet set;
        constexpr ecs::Id ids[] = {10, 11, 12, 500, 1000, 1020, 5000, 100000, 100001, 1000000, 10000000};
        constexpr ecs::Id o_ids[] = {200, 300, 400};
        constexpr std::size_t ids_n = sizeof(ids) / sizeof(ecs::Id);

        for (ecs::Id id : ids)
            set.insert(id);

        SUBCASE("copy constructor") {
            ecs::IdSet other(set);

            std::size_t i = 0;
            ecs::foreach(other, [&](ecs::Id id) {
                CHECK(ids[i++] == id);
            });
            CHECK(i == ids_n);

            CHECK(set.capacity() == other.capacity());
            CHECK(set.size() == other.size());
        }

        SUBCASE("move constructor") {
            ecs::IdSet other(std::move(set));

            std::size_t i = 0;
            ecs::foreach(other, [&](ecs::Id id) {
                CHECK(ids[i++] == id);
            });
            CHECK(i == ids_n);
        }

        SUBCASE("assignment") {
            ecs::IdSet other;
            for (ecs::Id id : o_ids)
                other.insert(id);

            SUBCASE("copy") {
                other = set;
                CHECK(set.capacity() == other.capacity());
                CHECK(set.size() == other.size());
            }

            SUBCASE("move") {
                other = std::move(set);
            }

            std::size_t i = 0;
            ecs::foreach(other, [&](ecs::Id id) {
                CHECK(ids[i++] == id);
            });
            CHECK(i == ids_n);
        }
    }

    TEST_CASE("foreach") {
        ecs::IdSet set;
        constexpr ecs::Id ids[] = {10, 11, 12, 500, 1000, 1020, 5000, 100000, 100001, 1000000, 10000000};
        constexpr std::size_t ids_n = sizeof(ids) / sizeof(ecs::Id);

        for (ecs::Id id : ids)
            set.insert(id);

        SUBCASE("go through") {
            std::size_t i = 0;
            ecs::foreach(set, [&i, &ids](auto id) {
                CHECK(ids[i++] == id);
            });
            CHECK(i == ids_n);
        }
    }
}