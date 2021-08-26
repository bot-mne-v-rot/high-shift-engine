#include "ecs/chunk_layout.h"

namespace ecs {
    void ChunkLayout::calculate_capacity() {
        // Binary search to find the biggest capacity that fits in the chunk size
        std::size_t l = 0, r = chunk_size + 1; // answer is in [l, r)
        while (r - l > 1) {
            _chunk_capacity = (l + r) / 2;
            std::size_t expected_size = calculate_offsets();
            if (expected_size <= chunk_size)
                l = _chunk_capacity;
            else
                r = _chunk_capacity;
        }

        _chunk_capacity = l;
        calculate_offsets();
    }

    std::size_t ChunkLayout::calculate_offsets() {
        std::size_t offset = sizeof(Chunk::Fields);

        std::size_t align = alignof(ShCompVal);
        offset = (offset + align - 1) / align * align;
        _shared_components_offset = offset;
        offset += sizeof(ShCompVal) * _shared_component_types.size();

        align = alignof(Entity);
        offset = (offset + align - 1) / align * align;
        _entities_offset = offset;
        offset += sizeof(Entity) * _chunk_capacity;

        _component_offsets.resize(_component_types.size());
        for (std::size_t i = 0; i < _component_types.size(); ++i) {
            align = _component_types[i].align;
            offset = (offset + align - 1) / align * align; // complement offset to a multiple of align
            _component_offsets[i] = offset;

            offset += _component_types[i].array_offset * _chunk_capacity;
        }
        return offset; // expected size
    }
}