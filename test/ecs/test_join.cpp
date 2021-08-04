#include "doctest.h"

#include "ecs/join.h"
#include "ecs/storage.h"
#include "ecs/system.h"
#include "ecs/storages/vec_storage.h"


namespace {
    struct SomeComponent {
        using Storage = ecs::VecStorage<SomeComponent>;
        std::size_t i = 0;
    };

    struct OtherComponent {
        using Storage = ecs::VecStorage<OtherComponent>;
        std::size_t i = 0;
    };

    class ExampleSystem {
    public:
        void update(const SomeComponent::Storage &, OtherComponent::Storage &) {}
    };

    static_assert(ecs::Component<SomeComponent>);
    static_assert(ecs::Component<OtherComponent>);
    static_assert(ecs::System<ExampleSystem>);
}

TEST_SUITE("ecs/join") {
    TEST_CASE("compiles") {
        SomeComponent::Storage storage_a;
        OtherComponent::Storage storage_b;

        const SomeComponent::Storage &a = storage_a;
        OtherComponent::Storage &b = storage_b;
        for (auto[some, other] : ecs::join(a, b)) {
            static_assert(std::is_same_v<decltype(some), const SomeComponent &>);
            static_assert(std::is_same_v<decltype(other), OtherComponent &>);
        }
    }

    TEST_CASE("basic") {
        SomeComponent::Storage storage_a;
        OtherComponent::Storage storage_b;

        constexpr ecs::Id ids_a[] = {10, 11, 12, 1000, 5000};
        constexpr ecs::Id ids_b[] = {11, 12, 500, 1000, 1020, 5000};
        constexpr ecs::Id ids_c[] = {11, 12, 1000, 5000};
        constexpr std::size_t ids_n = sizeof(ids_c) / sizeof(ecs::Id);

        for (ecs::Id id : ids_a)
            storage_a.insert(id, SomeComponent{id});
        for (ecs::Id id : ids_b)
            storage_b.insert(id, OtherComponent{id});

        std::size_t i = 0;
        const SomeComponent::Storage &a = storage_a;
        OtherComponent::Storage &b = storage_b;

        SUBCASE("iterator") {
            SUBCASE("without id") {
                for (auto[some, other] : ecs::join(a, b)) {
                    CHECK(some.i == ids_c[i]);
                    CHECK(other.i == ids_c[i]);
                    ++i;
                }
                CHECK(i == ids_n);
            }

            SUBCASE("with id") {
                for (auto[id, some, other] : ecs::join_with_id(a, b)) {
                    CHECK(some.i == ids_c[i]);
                    CHECK(other.i == ids_c[i]);
                    CHECK(id == ids_c[i]);
                    ++i;
                }
                CHECK(i == ids_n);
            }
        }

        SUBCASE("foreach") {
            SUBCASE("without id") {
                ecs::joined_foreach<const SomeComponent::Storage,
                        OtherComponent::Storage>(a, b, [&](auto &some, auto &other) {
                    CHECK(some.i == ids_c[i]);
                    CHECK(other.i == ids_c[i]);
                    ++i;
                });
                CHECK(i == ids_n);
            }
        }
    }

}

