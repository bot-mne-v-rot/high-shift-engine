#include "ecs/entities.h"

#include <immintrin.h>

namespace ecs {

    Entity Entities::create(const std::vector <ComponentType> &types, const std::vector<void *> &data) {
        assert(types.size() == data.size());
        return create(types.size(), types.data(), data.data());
    }

    Entity Entities::create(std::size_t components_count,
                            const ComponentType *types,
                            const void *const *data) {
        Entity entity;
        create_multiple(1, components_count, types, data, &entity);
        return entity;
    }

    void Entities::create_multiple(std::size_t entities_count,
                                   std::size_t components_count,
                                   const ComponentType *types,
                                   const void *const *data,
                                   Entity *entities) {
        get_free_entities(entities_count, entities);

        Archetype *arch = archetypes.get_or_insert(components_count, types);
        ComponentsData components {
            components_count, types, data
        };

        SharedComponentsData shared_components {
            .components_count = 0
        };

        arch->allocate_entities(entities_count, components, shared_components, entities);
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

        push_free_list(entity);

        EntityChunkMapping &mapping = *archetypes.entities_mapping();
        EntityPosInChunk entity_pos = mapping[entity.id];

        entity_pos.archetype->deallocate_entity(entity_pos);
        if (entity_pos.archetype->entities_count() == 1)
            archetypes.erase(entity_pos.archetype);

        return true;
    }

    void Entities::get_free_entities(std::size_t entities_count, Entity *entities) {
        std::size_t created = 0;
        while (created < entities_count && free_list != NO_ENTRY) {
            entities[created] = pop_free_list();
            ++created;
        }

        entries.reserve(entries.size() + entities_count - created);
        while (created < entities_count) {
            entries.emplace_back();
            entities[created] = Entity{
                    .id = (Id)(entries.size() - 1),
                    .version = 0
            };
            ++created;
        }
    }

    Entity Entities::get_free_entity() {
        if (free_list == NO_ENTRY) {
            entries.emplace_back();
            free_list = entries.size() - 1;
        }

        return pop_free_list();
    }

    Entity Entities::pop_free_list() {
        Entry &entry = entries[free_list];
        Entity entity{
                .id = free_list,
                .version = entry.version
        };
        free_list = entry.next;
        return entity;
    }

    void Entities::push_free_list(Entity entity) {
        auto &entry = entries[entity.id];
        entry.next = free_list;
        free_list = entity.id;
        ++entry.version;
    }
}