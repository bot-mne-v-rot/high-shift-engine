#include "doctest.h"

#include "ecs/chunk_layout.h"
#include "some_components.h"

TEST_SUITE("ecs::ChunkLayout") {
    TEST_CASE("offsets") {
        ecs::ChunkLayout *layout;

        SUBCASE("simple") {
            layout = new ecs::ChunkLayout(
                    {
                            ecs::ComponentType::create<Position>()
                    });
            CHECK(layout->chunk_capacity() <= ecs::chunk_size / sizeof(Position));
        }
        SUBCASE("cache lines") {
            layout = new ecs::ChunkLayout(
                    {
                            ecs::ComponentType::create<CacheLineAligned1>(),
                            ecs::ComponentType::create<CacheLineAligned2>(),
                    });
        }
        SUBCASE("complex") {
            layout = new ecs::ChunkLayout(
                    {
                            ecs::ComponentType::create<Position>(),
                            ecs::ComponentType::create<CacheLineAligned1>(),
                            ecs::ComponentType::create<CacheLineAligned2>(),
                            ecs::ComponentType::create<SomeComponent>()
                    });
        }

        auto &component_types = layout->component_types();
        auto &component_offsets = layout->component_offsets();
        auto components_count = layout->components_count();
        auto chunk_capacity = layout->chunk_capacity();

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

        delete layout;
    }
}