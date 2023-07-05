#include "doctest.h"

#include "ecs/chunks_data.h"
#include "some_components.h"

TEST_SUITE("ecs::ChunksData") {
    TEST_CASE("empty") {
        std::size_t shared_components_count = 5;
        ecs::ChunksData chunks_data(shared_components_count);
        CHECK(chunks_data.chunks_count() == 0);
        CHECK(chunks_data.capacity() == 0);
    }

    TEST_CASE("push chunks") {
        std::size_t shared_components_count = 5;
        ecs::ChunksData chunks_data(shared_components_count);

        for (std::size_t i = 1; i <= 10; ++i) {
            chunks_data.push_chunk();
            CHECK(chunks_data.chunks_count() == i);
            CHECK(chunks_data.capacity() >= i);

            chunks_data.chunks_ptr()[i - 1]->fields.chunk_index = i - 1;
            for (std::size_t j = 0; j < i; ++j)
                CHECK(chunks_data.chunks_ptr()[j]->fields.chunk_index == j);
        }
    }
}