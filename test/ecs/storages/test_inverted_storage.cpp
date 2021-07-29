#include "doctest.h"

#include "ecs/storages/vec_storage.h"
#include "ecs/storages/inverted_storage.h"
#include "ecs/join.h"

namespace {
    struct SomeComponent {
        using Storage = ecs::VecStorage<SomeComponent>;
        ecs::Id id;
    };

    struct OtherComponent {
        using Storage = ecs::VecStorage<SomeComponent>;
    };

    static_assert(ecs::Component<SomeComponent>);
}

TEST_SUITE("ecs/storages/InvertedStorage") {
    TEST_CASE("basic") {
        constexpr ecs::Id ids_a[] = {10, 11, 12, 1000, 5000, 100000, 1000000, 10000000};
        constexpr ecs::Id ids_b[] = {12, 500, 1000, 1020, 5000, 100001, 1000000};
        constexpr ecs::Id ids_c[] = {10, 11, 100000, 10000000};

        constexpr std::size_t ids_n = sizeof(ids_c) / sizeof(ecs::Id);

        SomeComponent::Storage a;
        OtherComponent::Storage b;

        for (auto id : ids_a)
            a.insert(id, { id });
        for (auto id : ids_b)
            b.insert(id, {});

        std::size_t i = 0;
        auto not_b = !b; // because ecs::join expects lvalues
        for (auto[ca, _] : ecs::join(a, not_b))
            REQUIRE(ca.id == ids_c[i++]);
        CHECK(ids_n == i);

        for (auto id : ids_b)
            CHECK(!not_b.contains(id));
    }
}