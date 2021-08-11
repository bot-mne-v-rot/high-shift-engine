#include "doctest.h"

#include "ecs/archetype.h"

TEST_SUITE("ecs/EntityChunkMapping") {
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
                    .chunk_index = 20,
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
                    {.chunk_index = 10},
                    {.chunk_index = 20},
                    {.chunk_index = 30},
                    {.chunk_index = 40}
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
                    .chunk_index = 20,
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

namespace {
    struct alignas(64) CacheLineAligned1 {
        int a, b, c;
    };

    struct alignas(64) CacheLineAligned2 {
        int a[100];
    };

    struct Position {
        float x, y, z;
    };

    struct SomeComponent {
        int x[10];
        double g;
        float t[3];
    };
}

TEST_SUITE("ecs/ComponentType") {
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

TEST_SUITE("ecs/Archetype") {
    TEST_CASE("offsets") {
        ecs::EntityChunkMapping mapping;
        ecs::Archetype *archetype;

        SUBCASE("simple") {
            archetype = new ecs::Archetype(
                    {
                            ecs::ComponentType::create<Position>()
                    }, &mapping);
            CHECK(archetype->chunk_capacity() <= ecs::chunk_size / sizeof(Position));
        }

        SUBCASE("cache lines") {
            archetype = new ecs::Archetype(
                    {
                            ecs::ComponentType::create<CacheLineAligned1>(),
                            ecs::ComponentType::create<CacheLineAligned2>(),
                    }, &mapping);
        }
        SUBCASE("complex") {
            archetype = new ecs::Archetype(
                    {
                            ecs::ComponentType::create<Position>(),
                            ecs::ComponentType::create<CacheLineAligned1>(),
                            ecs::ComponentType::create<CacheLineAligned2>(),
                            ecs::ComponentType::create<SomeComponent>()
                    }, &mapping);
        }

        auto &component_types = archetype->component_types();
        auto &component_offsets = archetype->component_offsets();
        auto components_count = archetype->components_count();
        auto chunk_capacity = archetype->chunk_capacity();

        std::size_t size_sum = 0;
        for (auto type : component_types)
            size_sum += type.size;
        std::size_t capacity_upper_bound = ecs::chunk_size / size_sum;
        CHECK(chunk_capacity <= capacity_upper_bound);

        for (std::size_t i = 0; i < components_count; ++i) {
            CHECK(component_offsets[i] % component_types[i].align == 0);
            if (i > 0)
                CHECK(component_offsets[i] >= component_offsets[i - 1]);
        }

        delete archetype;
    }

    TEST_CASE("allocate") {
        ecs::EntityChunkMapping mapping;
        ecs::Archetype archetype(
                {
                        ecs::ComponentType::create<Position>(),
                        ecs::ComponentType::create<CacheLineAligned1>(),
                        ecs::ComponentType::create<CacheLineAligned2>(),
                        ecs::ComponentType::create<SomeComponent>()
                }, &mapping);

        SUBCASE("single entity") {
            ecs::Id id = 20;
            ecs::EntityPosInChunk pos = archetype.allocate_entity(id);

            ecs::Chunk *chunks = archetype.chunks();
            std::size_t chunks_count = archetype.chunks_count();
            CHECK(pos.archetype == &archetype);
            CHECK(pos.chunk_index == 0);
            CHECK(pos.index_in_chunk == 0);
            CHECK(chunks_count == 1);
            CHECK(archetype.entities_count() == 1);
            CHECK(archetype.last_chunk_free_slots() == archetype.chunk_capacity() - 1);

            void *id_ptr =
                    chunks[pos.chunk_index].data + archetype.entity_ids_offset() +
                    pos.index_in_chunk * sizeof(ecs::Id);

            CHECK(*reinterpret_cast<ecs::Id *>(id_ptr) == id);
            CHECK(archetype.get_entity_id(pos) == id);
        }
    }
}

TEST_SUITE("ecs/ArchetypesStorage") {
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