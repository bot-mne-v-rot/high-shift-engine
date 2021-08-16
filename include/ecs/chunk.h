#ifndef HIGH_SHIFT_CHUNK_H
#define HIGH_SHIFT_CHUNK_H

#include <cstdint>
#include <cstddef>

namespace ecs {
    static const std::size_t page_size = 4 * 1024; // 4 KiB
    static const std::size_t chunk_size = 16 * 1024; // 16 KiB
    struct alignas(page_size) Chunk {
        uint8_t data[chunk_size];
    };

    static_assert(sizeof(Chunk) == chunk_size);
    static_assert(alignof(Chunk) == page_size);
}

#endif //HIGH_SHIFT_CHUNK_H
