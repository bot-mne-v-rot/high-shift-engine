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
        std::size_t dense_sz = dense.size();
        dense.resize(dense_sz + entities_count);
        dense_ids.resize(dense_sz + entities_count);

        for (std::size_t i = 0; i < entities_count; ++i) {
            present.insert_unsafe(entities[i].id);
            dense[dense_sz + i] = starting_pos;
            dense_ids[dense_sz + i] = entities[i].id;
            ++starting_pos.index_in_chunk;
        }
    }

    void EntityChunkMapping::reserve(Id max_id, std::size_t additional_count) {
        present.reserve(max_id);
        sparse.resize(present.capacity());

        std::size_t next_capacity = 1;
        while (next_capacity < dense.size() + additional_count)
            next_capacity *= 2;
        dense.reserve(next_capacity);
        dense_ids.reserve(next_capacity);
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

    EntityPosInChunk Archetype::allocate_entity(Entity entity) {
        if (_last_chunk_free_slots == 0)
            allocate_chunk();

        EntityPosInChunk entity_pos;
        entity_pos.archetype = this;
        entity_pos.chunk_index = _chunks_count - 1;
        entity_pos.index_in_chunk = chunk_capacity() - _last_chunk_free_slots;

        auto *entities = _layout.get_entities(_chunks[_chunks_count - 1]);
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
        entity_pos.chunk_index = _chunks_count - 1;
        entity_pos.index_in_chunk = chunk_capacity() - _last_chunk_free_slots;

        Id max_id = 0;
        for (std::size_t i = 0; i < entities_count; ++i)
            max_id = std::max(max_id, entities[i].id);
        entities_mapping->reserve(max_id, entities_count);

        auto allocate_in_chunk = [&](std::size_t count, EntityPosInChunk starting_pos) {
            auto *chunk_entities = _layout.get_entities(_chunks[_chunks_count - 1]);
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
        std::size_t chunks = entities_count / chunk_capacity();
        reserve_chunks(chunks);
        starting_pos.index_in_chunk = 0;
        for (std::size_t i = 0; i < chunks; ++i) {
            allocate_chunk();
            ++starting_pos.chunk_index;
            allocate_in_chunk(chunk_capacity(), starting_pos);
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
        if (_last_chunk_free_slots == chunk_capacity())
            deallocate_last_chunk();
    }

    void Archetype::reserve_chunks(std::size_t count) {
        std::size_t target_cp = _chunks_count + count;
        if (target_cp <= _chunks_cp)
            return;

        std::size_t new_cp = 1;
        while (new_cp < target_cp)
            new_cp *= 2;

        Chunk *new_chunks = new Chunk[new_cp];
        memcpy(new_chunks, _chunks, chunk_size * _chunks_cp);
        delete[] _chunks;
        _chunks = new_chunks;
        _chunks_cp = new_cp;
    }

    Archetype::~Archetype() {
        delete[] _chunks;
    }

    void Archetype::allocate_chunk() {
        if (_chunks_count == _chunks_cp)
            reserve_chunks(1);
        ++_chunks_count;
        _last_chunk_free_slots = chunk_capacity();
    }

    void Archetype::deallocate_chunk(std::size_t chunk_index) {
        std::size_t last = _chunks_count - 1;
        std::size_t one_before_last = _chunks_count - 2;

        if (chunk_index == last) {
            --_chunks_count;
        } else if (_last_chunk_free_slots == 0 || chunk_index == one_before_last) {
            std::swap(_chunks[chunk_index], _chunks[last]);
            --_chunks_count;
        } else {
            std::swap(_chunks[chunk_index], _chunks[one_before_last]);
            std::swap(_chunks[one_before_last], _chunks[last]);
            --_chunks_count;
        }
        _last_chunk_free_slots = 0;
    }

    void Archetype::deallocate_last_chunk() {
        --_chunks_count;
        _last_chunk_free_slots = 0;
    }

    Id Archetype::swap_ent_with_last_to_remove(EntityPosInChunk entity_pos) {
        std::size_t components_num = components_count();
        Chunk &entity_chunk = _chunks[entity_pos.chunk_index];
        Chunk &last_chunk = _chunks[_chunks_count - 1];

        EntityPosInChunk last_entity_pos{
                .chunk_index = (uint32_t) (_chunks_count - 1),
                .index_in_chunk = (uint16_t) (chunk_capacity() - _last_chunk_free_slots - 1)
        };

        for (std::size_t i = 0; i < components_num; ++i) {
            auto *ent_component = (uint8_t *) get_component(entity_pos, i);
            auto *last_component = (uint8_t *) get_component(last_entity_pos, i);

            std::size_t comp_size = component_types()[i].size;
            for (std::size_t b = 0; b < comp_size; ++b)
                std::swap(ent_component[b], last_component[b]);
        }

        auto *entities = reinterpret_cast<Entity *>(last_chunk.data + entities_offset());
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