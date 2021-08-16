#include "ecs/archetype.h"

namespace ecs {
    bool EntityChunkMapping::insert(Id id, EntityPosInChunk entity_pos) {
        if (!present.contains(id)) {
            insert_unsafe(id, entity_pos);
            return true;
        } else {
            dense[sparse[id]] = entity_pos;
            return false;
        }
    }

    void EntityChunkMapping::insert_unsafe(Id id, EntityPosInChunk entity_pos) {
        present.insert(id);
        sparse.resize(present.capacity());

        sparse[id] = dense.size();
        dense.push_back(entity_pos);
        dense_ids.push_back(id);
    }

    void EntityChunkMapping::insert_multiple(std::size_t entities_count, const Entity *entities,
                                             EntityPosInChunk starting_pos) {
        Id max_id = 0;
        for (std::size_t i = 0; i < entities_count; ++i)
            max_id = entities[i].id;

        present.reserve(max_id);
        sparse.resize(present.capacity());

        std::size_t next_capacity = 1;
        while (next_capacity < dense.size() + entities_count)
            next_capacity *= 2;
        dense.reserve(next_capacity);
        dense_ids.reserve(next_capacity);

        for (std::size_t i = 0; i < entities_count; ++i) {
            present.insert(entities[i].id);
            dense.push_back(starting_pos);
            dense_ids.push_back(entities[i].id);
            ++starting_pos.index_in_chunk;
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
     *     Entity entities[_chunk_capacity];
     *     Component_0 c0[_chunk_capacity];
     *     Component_1 c1[_chunk_capacity];
     *     // ...
     * }
     *
     * Then these values are just:
     * _entities_offset = offsetof(Data, entities);
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
        _entities_offset = 0;
        std::size_t offset = sizeof(Entity) * _chunk_capacity;

        _component_offsets.resize(_component_types.size());
        for (std::size_t i = 0; i < _component_types.size(); ++i) {
            std::size_t align = _component_types[i].align;
            offset += (align - offset % align) % align; // complement offset to a multiple of align
            _component_offsets[i] = offset;

            offset += _component_types[i].array_offset * _chunk_capacity;
        }
        return offset; // expected size
    }

    EntityPosInChunk Archetype::allocate_entity(Entity entity) {
        if (_last_chunk_free_slots == 0)
            allocate_chunk();

        EntityPosInChunk entity_pos;
        entity_pos.archetype = this;
        entity_pos.chunk_index = _chunks.size() - 1;
        entity_pos.index_in_chunk = _chunk_capacity - _last_chunk_free_slots;

        auto *entities = reinterpret_cast<Entity *>(_chunks.back().data + _entities_offset);
        entities[entity_pos.index_in_chunk] = entity;

        entities_mapping->insert(entity.id, entity_pos);
        --_last_chunk_free_slots;
        ++_entities_count;

        return entity_pos;
    }

    /**
     * Allocation is divided in three steps:
     * 1. Allocate using free slots in the last chunk.
     * 2. Allocate chunks.
     * 3. Allocate the last chunk and allocate remained entities.
     *
     * It is guaranteed that entities are allocated in the same order
     * as given in the input.
     */
    EntityPosInChunk Archetype::allocate_entities(std::size_t entities_count, const Entity *entities) {
        if (_last_chunk_free_slots == 0)
            allocate_chunk();

        _entities_count += entities_count;

        EntityPosInChunk entity_pos;
        entity_pos.archetype = this;
        entity_pos.chunk_index = _chunks.size() - 1;
        entity_pos.index_in_chunk = _chunk_capacity - _last_chunk_free_slots;

        auto allocate_in_chunk = [&](std::size_t count, EntityPosInChunk starting_pos) {
            auto *chunk_entities = reinterpret_cast<Entity *>(_chunks.back().data + _entities_offset);
            memcpy(chunk_entities + entity_pos.index_in_chunk, entities, count * sizeof(Entity));

            entities_mapping->insert_multiple(count, entities, entity_pos);

            entities_count -= count;
            entities += count;
            _last_chunk_free_slots -= count;
        };

        // first chunk
        EntityPosInChunk starting_pos = entity_pos;
        std::size_t in_first_chunk = std::min(entities_count, _last_chunk_free_slots);
        allocate_in_chunk(in_first_chunk, starting_pos);

        // intermediate chunks
        std::size_t chunks = entities_count / _chunk_capacity;
        reserve_chunks(chunks);
        starting_pos.index_in_chunk = 0;
        for (std::size_t i = 0; i < chunks; ++i) {
            allocate_chunk();
            ++starting_pos.chunk_index;
            allocate_in_chunk(_chunk_capacity, starting_pos);
        }

        // last chunk
        if (entities_count) {
            allocate_chunk();
            ++starting_pos.chunk_index;
            std::size_t in_last_chunk = entities_count;
            allocate_in_chunk(in_last_chunk, starting_pos);
        }

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

    void Archetype::reserve_chunks(std::size_t count) {
        std::size_t target_cp = _chunks.size() + count;
        std::size_t new_cp = 1;
        while (new_cp < target_cp)
            new_cp *= 2;
        _chunks.reserve(new_cp);
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

        auto *entities = reinterpret_cast<Entity *>(_chunks.back().data + _entities_offset);
        std::swap(entities[entity_pos.index_in_chunk],
                  entities[last_entity_pos.index_in_chunk]);

        entities_mapping->insert(entities[entity_pos.index_in_chunk].id, entity_pos);

        // return entity id
        return entities[last_entity_pos.index_in_chunk].id;
    }

    auto ArchetypesStorage::find(std::size_t components_count,
                                 const ComponentType *types)
    -> Storage::iterator {
        DynamicIdSetAnd sets_and = query_mask(components_count,
                                              types);

        Id id = find_if(sets_and, [&](Id id) {
            return storage[id]->components_count() == components_count;
        });

        if (id != IdSet::max_size)
            return storage.begin() + id;
        else
            return storage.end();
    }

    DynamicIdSetAnd ArchetypesStorage::query_mask(std::size_t components_count,
                                                  const ComponentType *types) const {
        std::vector<const IdSet *> sets;
        sets.reserve(components_count);
        for (std::size_t i = 0; i < components_count; ++i)
            sets.push_back(&component_masks[types[i].id]);

        return DynamicIdSetAnd(std::move(sets));
    }

    Archetype *ArchetypesStorage::get_or_insert(const std::vector<ComponentType> &types) {
        return get_or_insert(types.size(), types.data());
    }

    Archetype *ArchetypesStorage::get_or_insert(std::size_t components_count,
                                                const ComponentType *types) {
        auto it = find(components_count, types);

        if (it == storage.end()) {
            std::vector<ComponentType> types_vec(types, types + components_count);
            for (auto &type : types_vec)
                component_masks[type.id].insert(storage.size());

            storage.push_back(std::make_unique<Archetype>(std::move(types_vec), _mapping.get()));
            return storage.back().get();
        } else {
            return it->get();
        }
    }

    void ArchetypesStorage::erase(Archetype *archetype) {
        auto it = find(archetype->component_types().size(),
                       archetype->component_types().data());

        if (it != storage.end()) {
            std::size_t found = it - storage.begin();
            std::size_t last = storage.size() - 1;

            for (auto &type : archetype->component_types())
                component_masks[type.id].erase(found);

            for (auto &type : storage[last]->component_types())
                component_masks[type.id].erase(last);
            for (auto &type : storage[last]->component_types())
                component_masks[type.id].insert(found);

            std::swap(*it, storage[last]);

            storage.pop_back();
        }
    }
}