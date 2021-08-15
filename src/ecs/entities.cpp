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