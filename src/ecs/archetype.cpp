#include "ecs/archetype.h"

namespace ecs {
    bool EntityChunkMapping::insert(Id id, const EntityPosInChunk &entity_pos) {
        if (!present.contains(id)) {
            present.insert(id);
            sparse.resize(present.capacity());

            sparse[id] = dense.size();
            dense.push_back(entity_pos);
            dense_ids.push_back(id);
            return true;
        } else {
            dense[sparse[id]] = entity_pos;
            return false;
        }
    }

    bool EntityChunkMapping::erase(Id id) {
        if (present.erase(id)) {
            std::swap(dense.back(), dense[sparse[id]]);
            std::swap(dense_ids.back(), dense_ids[sparse[id]]);
            dense.pop_back();
            dense_ids.pop_back();

            sparse[dense_ids[sparse[id]]] = sparse[id];
            return true;
        }
        return false;
    }

    void Archetype::calculate_capacity() {
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

    /**
     * You can imagine that chunk data
     * is actually a struct like this
     * struct Data {
     *     Id entity_ids[_chunk_capacity];
     *     Component_0 c0[_chunk_capacity];
     *     Component_1 c1[_chunk_capacity];
     *     // ...
     * }
     *
     * Then these values are just:
     * _entities_ids_offset = offsetof(Data, entity_ids);
     * _component_offsets[i] = offsetof(Data, ci);
     *
     * Offsets are calculated with respect to the
     * data alignment rules.
     *
     * There is no need in additional padding
     * at the beginning or at the end of the struct
     * because chunks are aligned by the page size.
     */
    std::size_t Archetype::calculate_offsets() {
        _entity_ids_offset = 0;
        std::size_t offset = sizeof(Id) * _chunk_capacity;

        _component_offsets.resize(_component_types.size());
        for (std::size_t i = 0; i < _component_types.size(); ++i) {
            std::size_t align = _component_types[i].align;
            offset += (align - offset % align) % align; // complement offset to a multiple of align
            _component_offsets[i] = offset;

            offset += _component_types[i].array_offset * _chunk_capacity;
        }
        return offset; // expected size
    }

    EntityPosInChunk Archetype::allocate_entity(Id id) {
        if (_last_chunk_free_slots == 0)
            allocate_chunk();

        EntityPosInChunk entity_pos;
        entity_pos.archetype = this;
        entity_pos.chunk_index = _chunks.size() - 1;
        entity_pos.index_in_chunk = _chunk_capacity - _last_chunk_free_slots;

        auto *entities = reinterpret_cast<Id *>(_chunks.back().data + _entity_ids_offset);
        entities[entity_pos.index_in_chunk] = id;

        entities_mapping->insert(id, entity_pos);
        --_last_chunk_free_slots;
        ++_entities_count;

        return entity_pos;
    }

    void Archetype::deallocate_entity(EntityPosInChunk entity_pos) {
        Id entity_id = swap_ent_with_last_to_remove(entity_pos);

        entities_mapping->erase(entity_id);
        ++_last_chunk_free_slots;
        --_entities_count;
        if (_last_chunk_free_slots == _chunk_capacity)
            deallocate_last_chunk();
    }

    void Archetype::allocate_chunk() {
        _chunks.emplace_back();
        _last_chunk_free_slots = _chunk_capacity;
    }

    void Archetype::deallocate_chunk(std::size_t chunk_index) {
        std::size_t last = _chunks.size() - 1;
        std::size_t one_before_last = _chunks.size() - 2;

        if (chunk_index == last) {
            _chunks.pop_back();
        } else if (_last_chunk_free_slots == 0) {
            std::swap(_chunks[chunk_index], _chunks[last]);
            _chunks.pop_back();
        } else if (chunk_index == one_before_last) {
            _chunks.erase(_chunks.begin() + (std::ptrdiff_t) chunk_index);
        } else {
            std::swap(_chunks[chunk_index], _chunks[one_before_last]);
            _chunks.erase(_chunks.begin() + (std::ptrdiff_t) chunk_index);
        }
        _last_chunk_free_slots = 0;
    }

    void Archetype::deallocate_last_chunk() {
        _chunks.pop_back();
        _last_chunk_free_slots = 0;
    }

    Id Archetype::swap_ent_with_last_to_remove(EntityPosInChunk entity_pos) {
        std::size_t components_num = _component_types.size();
        Chunk &entity_chunk = _chunks[entity_pos.chunk_index];
        Chunk &last_chunk = _chunks.back();

        EntityPosInChunk last_entity_pos{
                .chunk_index = (uint32_t) (_chunks.size() - 1),
                .index_in_chunk = (uint16_t) (_chunk_capacity - _last_chunk_free_slots - 1)
        };

        for (std::size_t i = 0; i < components_num; ++i) {
            auto *ent_component = (uint8_t *) get_component(entity_pos, i);
            auto *last_component = (uint8_t *) get_component(last_entity_pos, i);

            std::size_t comp_size = _component_types[i].size;
            for (std::size_t b = 0; b < comp_size; ++b)
                std::swap(ent_component[b], last_component[b]);
        }

        auto *entities = reinterpret_cast<Id *>(_chunks.back().data + _entity_ids_offset);
        std::swap(entities[entity_pos.index_in_chunk],
                  entities[last_entity_pos.index_in_chunk]);

        entities_mapping->insert(entities[entity_pos.index_in_chunk], entity_pos);

        // return entity id
        return entities[last_entity_pos.index_in_chunk];
    }
}