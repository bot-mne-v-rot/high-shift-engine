#include "ecs/archetype.h"

namespace ecs {
    bool EntityChunkMapping::insert(Id id, EntityPosInChunk entity_pos) {
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

    void EntityChunkMapping::insert_unsafe(Id id, EntityPosInChunk entity_pos) {
        present.insert_unsafe(id);
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
            sparse[entities[i].id] = dense_sz + i;
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

    /**
     * Allocation is divided in three steps:
     * 1. Allocate using free slots in the last chunk.
     * 2. Allocate chunks.
     * 3. Allocate the last chunk and allocate remained entities.
     *
     * It is guaranteed that entities are allocated in the same order
     * as given in the input.
     */
    void Archetype::allocate_entities(std::size_t entities_count,
                                      ComponentsData components,
                                      SharedComponentsData shared_components,
                                      const Entity *entities) {

        Id max_id = 0;
        for (std::size_t i = 0; i < entities_count; ++i)
            max_id = std::max(max_id, entities[i].id);

        entities_mapping->reserve(max_id, entities_count);

        std::vector<std::size_t> comp_indices(components.components_count);
        _layout.get_component_indices(components.components_count,
                                      components.types,
                                      comp_indices.data());

        std::vector<std::size_t> shared_comp_indices(components.components_count);
        _layout.get_shared_component_indices(shared_components.components_count,
                                             shared_components.types,
                                             shared_comp_indices.data());

        SharedComponentsVector shared_component_values;
        shared_component_values.resize(shared_components.components_count);
        for (std::size_t i = 0; i < shared_components.components_count; ++i)
            shared_component_values[shared_comp_indices[i]] = shared_components.data[i];

        Chunk *chunk = _chunks_map.try_get(shared_component_values);
        for (std::size_t ent_i = 0; ent_i < entities_count; ++ent_i) {
            if (!chunk || chunk->fields.entities_count == _layout.chunk_capacity()) {
                _chunks_data.push_chunk();
                chunk = _chunks_data.last_chunk();
                _chunks_map.insert_or_assign(shared_component_values, chunk);
            }

            uint16_t index_in_chunk = chunk->fields.entities_count;
            ++chunk->fields.entities_count;
            ++_entities_count;

            for (std::size_t comp_i = 0; comp_i < components.components_count; ++comp_i) {
                std::size_t array_offset = (components.types[comp_i].array_offset);
                std::size_t bytes_to_copy = components.types[comp_i].size;
                void *src = (uint8_t *) components.data[comp_i] + ent_i * array_offset;
                void *dst = _layout.get_component(chunk, comp_indices[comp_i], index_in_chunk);
                memcpy(dst, src, bytes_to_copy);
            }

            Entity entity = entities[ent_i];
            _layout.get_entities(chunk)[index_in_chunk] = entity;
            entities_mapping->insert_unsafe(entity.id, {
                    .archetype = this,
                    .chunk = chunk,
                    .index_in_chunk = index_in_chunk
            });
        }
    }

    void Archetype::deallocate_entity(EntityPosInChunk entity_pos) {

        ShCompVal *shared_components_values = _layout.get_shared_components_values(entity_pos.chunk);
        SharedComponentsVector key(shared_components_values,
                                   shared_components_values + shared_components_count());

        Chunk *chunk = _chunks_map.try_get(key);
        std::size_t entity_id = _layout.get_entities(chunk)[entity_pos.index_in_chunk].id;

        swap_ent_with_to_remove(entity_pos, {
                .chunk = chunk,
                .index_in_chunk = (uint16_t) (chunk->fields.entities_count - 1)
        });
        --chunk->fields.entities_count;
        entities_mapping->erase(entity_id);
        --_entities_count;

        if (chunk->fields.entities_count == 0)
            deallocate_chunk(chunk->fields.chunk_index);
    }

    void Archetype::reserve_chunks(std::size_t count) {
        _chunks_data.reserve(_chunks_data.capacity() + count);
    }

    void Archetype::deallocate_chunk(std::size_t chunk_index) {
        std::size_t last = chunks_count() - 1;
        _chunks_data.swap_chunks(chunk_index, last);
        _chunks_data.pop_chunk();
    }

    void Archetype::deallocate_last_chunk() {
        _chunks_data.pop_chunk();
    }

    Id Archetype::swap_ent_with_to_remove(EntityPosInChunk removed_pos,
                                          EntityPosInChunk last_pos) {
        std::size_t components_num = components_count();
        Chunk *entity_chunk = removed_pos.chunk;
        Chunk *last_chunk = _chunks_data.last_chunk();

        for (std::size_t i = 0; i < components_num; ++i) {
            auto *ent_component = (uint8_t *) get_component(removed_pos, i);
            auto *last_component = (uint8_t *) get_component(last_pos, i);

            std::size_t comp_size = component_types()[i].size;
            for (std::size_t b = 0; b < comp_size; ++b)
                std::swap(ent_component[b], last_component[b]);
        }

        auto *entities = _layout.get_entities(entity_chunk);
        auto *last_entities = _layout.get_entities(last_chunk);
        std::swap(entities[removed_pos.index_in_chunk],
                  last_entities[last_pos.index_in_chunk]);

        entities_mapping->insert(entities[removed_pos.index_in_chunk].id, removed_pos);

        // return entity id
        return entities[last_pos.index_in_chunk].id;
    }

    void transfer_component_data(EntityPosInChunk from, EntityPosInChunk to,
                                 const std::vector<ComponentType> &transferred_components) {
        for (ComponentType component : transferred_components) {
            std::size_t from_index = from.archetype->layout().get_component_index(component);
            std::size_t to_index = to.archetype->layout().get_component_index(component);
            memcpy(to.archetype->get_component(to, to_index),
                   from.archetype->get_component(from, from_index),
                   component.size);
        }
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

            storage.push_back(std::make_unique<Archetype>(std::move(types_vec),
                                                          std::vector<ComponentType>(),
                                                          _mapping.get()));
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