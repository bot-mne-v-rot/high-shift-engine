#include "ecs/entities.h"

namespace ecs {
    Entity Entities::create(const std::vector<ComponentType> &types, const std::vector<void *> &data) {
        assert(types.size() == data.size());
        return create(types.size(), types.data(), data.data());
    }

    Entity Entities::create(std::size_t components_count,
                            const ComponentType *types,
                            const void *const *data) {
        if (free_list == NO_ENTRY) {
            entries.emplace_back();
            free_list = entries.size() - 1;
        }

        Entry &entry = entries[free_list];
        Entity entity{
                .id = free_list,
                .version = entry.version
        };
        free_list = entry.next;

        Archetype *arch = archetypes.get_or_insert(components_count, types);
        EntityPosInChunk entity_pos = arch->allocate_entity(entity);

        for (std::size_t i = 0; i < components_count; ++i) {
            const ComponentType &type = types[i];
            const void *bytes = data[i];

            std::size_t comp_index =
                    detail::get_component_index_in_archetype(entity_pos.archetype, type);

            memcpy(entity_pos.archetype->get_component(entity_pos, comp_index),
                   bytes, type.size);
        }

        return entity;
    }

    void Entities::create_multiple(std::size_t entities_count,
                                   std::size_t components_count,
                                   const ComponentType *types,
                                   const void *const *data,
                                   Entity *entities) {
        std::size_t created = 0;
        while (created < entities_count && free_list != NO_ENTRY) {
            Entry &entry = entries[free_list];
            entities[created] = Entity{
                    .id = free_list,
                    .version = entry.version
            };

            free_list = entry.next;
            ++created;
        }

        entries.reserve(std::max(2 * entries.size(), entries.size() + entities_count - created));
        while (created < entities_count) {
            entries.emplace_back();
            entities[created] = Entity{
                    .id = (Id) (entries.size() - 1),
                    .version = entries.back().version
            };
            ++created;
        }

        Archetype *arch = archetypes.get_or_insert(components_count, types);

        std::vector<std::size_t> comp_indices(components_count);
        for (std::size_t i = 0; i < components_count; ++i)
            comp_indices[i] = detail::get_component_index_in_archetype(arch, types[i]);

        EntityPosInChunk starting_pos = arch->allocate_entities(entities_count, entities);
        std::size_t filled = 0;

        auto fill_chunk = [&](std::size_t count) {
            for (std::size_t i = 0; i < components_count; ++i) {
                // assume count is not zero
                std::size_t bytes_to_copy = (count - 1) * (types[i].array_offset) + types[i].size;

                void *src = (uint8_t *) data[i] + filled * (types[i].array_offset);
                void *dst = arch->get_component(starting_pos, comp_indices[i]);
                memcpy(dst, src, bytes_to_copy);
            }
        };

        std::size_t chunk_capacity = arch->chunk_capacity();
        if (entities_count) {
            std::size_t in_first_chunk = std::min(entities_count, chunk_capacity - starting_pos.index_in_chunk);
            fill_chunk(in_first_chunk);

            filled += in_first_chunk;
            starting_pos.index_in_chunk = 0;
            ++starting_pos.chunk_index;
        }

        std::size_t chunks = (entities_count - filled) / chunk_capacity;
        while (chunks) {
            fill_chunk(chunk_capacity);

            ++starting_pos.chunk_index;
            filled += chunk_capacity;
            --chunks;
        }

        if (entities_count - filled) {
            std::size_t in_last_chunk = entities_count - filled;
            fill_chunk(in_last_chunk);
        }
    }

    void Entities::create_multiple(std::size_t entities_count,
                                   std::size_t components_count,
                                   const ComponentType *types,
                                   const void *const *data) {
        Entity *entities = new Entity[entities_count];
        create_multiple(entities_count, components_count, types, data, entities);
        delete[] entities;
    }

    bool Entities::destroy(Entity entity) {
        if (!is_alive(entity))
            return false;

        auto &entry = entries[entity.id];
        entry.next = free_list;
        free_list = entity.id;
        ++entry.version;

        EntityChunkMapping &mapping = *archetypes.entities_mapping();
        EntityPosInChunk entity_pos = mapping[entity.id];

        entity_pos.archetype->deallocate_entity(entity_pos);
        if (entity_pos.archetype->entities_count() == 1)
            archetypes.erase(entity_pos.archetype);

        return true;
    }
}