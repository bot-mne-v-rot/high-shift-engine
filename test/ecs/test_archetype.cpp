#include "doctest.h"

#include "ecs/archetype.h"
#include "some_components.h"

TEST_SUITE("ecs::EntityChunkMapping") {
    TEST_CASE("empty") {
        ecs::EntityChunkMapping mapping;

        CHECK(mapping.size() == 0);
        CHECK(mapping.empty());
    }

    TEST_CASE("insert one") {
        ecs::EntityChunkMapping mapping;

        SUBCASE("one") {
            ecs::Id id = 100;
            ecs::EntityPosInChunk entity_pos{
                    .archetype = nullptr,
                    .chunk = nullptr,
                    .index_in_chunk = 100
            };
            mapping.insert(id, entity_pos);

            CHECK(mapping.size() == 1);
            CHECK(!mapping.empty());

            CHECK(mapping[id] == entity_pos);
        }

        SUBCASE("several") {
            constexpr std::size_t n = 4;
            ecs::Id ids[n] = {100, 200, 300, 400};
            ecs::EntityPosInChunk entity_poses[n] = {
                    {.chunk = (ecs::Chunk *)(10)},
                    {.chunk = (ecs::Chunk *)(20)},
                    {.chunk = (ecs::Chunk *)(30)},
                    {.chunk = (ecs::Chunk *)(40)}
            };

            for (std::size_t i = 0; i < n; ++i)
                mapping.insert(ids[i], entity_poses[i]);

            CHECK(mapping.size() == n);
            CHECK(!mapping.empty());

            for (std::size_t i = 0; i < n; ++i)
                CHECK(mapping[ids[i]] == entity_poses[i]);
        }
    }

    TEST_CASE("erase") {
        ecs::EntityChunkMapping mapping;

        SUBCASE("insert and erase") {
            ecs::Id id = 100;
            ecs::EntityPosInChunk entity_pos{
                    .archetype = nullptr,
                    .chunk = nullptr,
                    .index_in_chunk = 100
            };
            mapping.insert(id, entity_pos);

            CHECK(mapping.size() == 1);
            CHECK(!mapping.empty());

            CHECK(mapping[id] == entity_pos);

            CHECK(mapping.erase(id));

            CHECK(mapping.size() == 0);
            CHECK(mapping.empty());

            CHECK(!mapping.erase(id));
        }
    }
}

TEST_SUITE("ecs::Archetype") {
    TEST_CASE("allocate") {
        ecs::EntityChunkMapping mapping;
        ecs::Archetype archetype(
                {
                        ecs::ComponentType::create<Position>(),
                        ecs::ComponentType::create<CacheLineAligned1>(),
                        ecs::ComponentType::create<CacheLineAligned2>(),
                        ecs::ComponentType::create<SomeComponent>()
                }, {}, &mapping);

//        SUBCASE("single entity") {
//            ecs::Id id = 20;
//            ecs::Entity entity{id, 0};
//            ecs::EntityPosInChunk pos = archetype.allocate_entity(entity);
//
//            ecs::Chunk **chunks = archetype.chunks();
//            std::size_t chunks_count = archetype.chunks_count();
//            CHECK(pos.archetype == &archetype);
//            CHECK(pos.chunk == chunks[0]);
//            CHECK(pos.index_in_chunk == 0);
//            CHECK(chunks_count == 1);
//            CHECK(archetype.entities_count() == 1);
//
//            CHECK(archetype.get_entity(pos) == entity);
//        }
    }
}

TEST_SUITE("ecs::ArchetypesStorage") {
    TEST_CASE("get_or_insert") {
        ecs::ArchetypesStorage archetypes;
        CHECK(archetypes.entities_mapping());

        std::vector<ecs::ComponentType> components{
                ecs::ComponentType::create<Position>(),
                ecs::ComponentType::create<CacheLineAligned1>(),
                ecs::ComponentType::create<CacheLineAligned2>(),
                ecs::ComponentType::create<SomeComponent>()
        };

        CHECK(archetypes.size() == 0);

        ecs::Archetype *arch = archetypes.get_or_insert(components);
        SUBCASE("insert") {
            CHECK(arch->component_types() == components);
            CHECK(archetypes.size() == 1);
            REQUIRE(arch);
        }

        SUBCASE("get") {
            ecs::Archetype *same_arch = archetypes.get_or_insert(components);
            CHECK(archetypes.size() == 1);
            CHECK(arch == same_arch);
        }

        std::vector<ecs::ComponentType> components2{
                ecs::ComponentType::create<Position>()
        };

        ecs::Archetype *arch2 = archetypes.get_or_insert(components2);
        SUBCASE("insert 2") {
            CHECK(arch2->component_types() == components2);
            CHECK(arch2 != arch);
            CHECK(archetypes.size() == 2);
        }

        SUBCASE("get 2") {
            ecs::Archetype *same_arch2 = archetypes.get_or_insert(components2);
            CHECK(archetypes.size() == 2);
            CHECK(arch2 == same_arch2);
        }
    }

    TEST_CASE("erase") {
        ecs::ArchetypesStorage archetypes;

        std::vector<ecs::ComponentType> components1{
                ecs::ComponentType::create<Position>(),
                ecs::ComponentType::create<CacheLineAligned1>(),
                ecs::ComponentType::create<CacheLineAligned2>(),
                ecs::ComponentType::create<SomeComponent>()
        };

        std::vector<ecs::ComponentType> components2{
                ecs::ComponentType::create<Position>(),
                ecs::ComponentType::create<SomeComponent>()
        };

        ecs::Archetype *arch1 = archetypes.get_or_insert(components1);
        ecs::Archetype *arch2 = archetypes.get_or_insert(components2);

        archetypes.erase(arch1);

        SUBCASE("erased") {
            CHECK(archetypes.size() == 1);
        }

        SUBCASE("other is accessible") {
            ecs::Archetype *arch = archetypes.get_or_insert(components2);
            CHECK(archetypes.size() == 1);
            CHECK(arch == arch2);
        }

        SUBCASE("create again") {
            ecs::Archetype *arch = archetypes.get_or_insert(components1);
            CHECK(archetypes.size() == 2);
        }
    }
}