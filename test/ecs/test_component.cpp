#include "doctest.h"

#include "ecs/component.h"
#include "some_components.h"

namespace {
    namespace test_concept {
        struct PlainComp {};
        struct PlainCompWithTag {
            using Tag = ecs::ComponentTag;
        };
        struct SharedComp {
            using Tag = ecs::SharedComponentTag;
        };
        struct ChunkComp {
            using Tag = ecs::ChunkComponentTag;
        };

        static_assert(ecs::PlainComponent<PlainComp> &&
                      !ecs::SharedComponent<PlainComp> &&
                      !ecs::ChunkComponent<PlainComp>);

        static_assert(ecs::PlainComponent<PlainCompWithTag> &&
                      !ecs::SharedComponent<PlainCompWithTag> &&
                      !ecs::ChunkComponent<PlainCompWithTag>);

        static_assert(!ecs::PlainComponent<SharedComp> &&
                      ecs::SharedComponent<SharedComp> &&
                      !ecs::ChunkComponent<SharedComp>);

        static_assert(!ecs::PlainComponent<ChunkComp> &&
                      !ecs::SharedComponent<ChunkComp> &&
                      ecs::ChunkComponent<ChunkComp>);
    }
}

TEST_SUITE("ecs::ComponentType") {
    TEST_CASE("create") {
        ecs::ComponentType type;
        std::size_t size, align;
        ecs::CmpId id;

        SUBCASE("CacheLineAligned1") {
            type = ecs::ComponentType::create<CacheLineAligned1>();
            size = sizeof(CacheLineAligned1);
            align = alignof(CacheLineAligned1);
            id = ecs::get_component_id<CacheLineAligned1>();
        }
        SUBCASE("CacheLineAligned2") {
            type = ecs::ComponentType::create<CacheLineAligned2>();
            size = sizeof(CacheLineAligned2);
            align = alignof(CacheLineAligned2);
            id = ecs::get_component_id<CacheLineAligned2>();
        }
        SUBCASE("Position") {
            type = ecs::ComponentType::create<Position>();
            size = sizeof(Position);
            align = alignof(Position);
            id = ecs::get_component_id<Position>();
        }
        SUBCASE("SomeComponent") {
            type = ecs::ComponentType::create<SomeComponent>();
            size = sizeof(SomeComponent);
            align = alignof(SomeComponent);
            id = ecs::get_component_id<SomeComponent>();
        }

        CHECK(type.id == id);
        CHECK(type.align == align);
        CHECK(type.size == size);
        CHECK(type.array_offset % align == 0);
        CHECK(type.array_offset >= size);
        CHECK(type.array_offset < size + align);
    }
}