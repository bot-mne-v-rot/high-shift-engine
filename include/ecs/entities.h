#ifndef HIGH_SHIFT_ENTITIES_H
#define HIGH_SHIFT_ENTITIES_H

#include "ecs/world.h"
#include "ecs/archetype.h"
#include "ecs/component.h"
#include "ecs/id_set.h"

namespace ecs {

    /**
     * Class to manage entities.
     *
     * Entity is not stored directly anywhere.
     * It is represented by an id and its components.
     * But we still need a way to track used and free ids.
     *
     * This resource is automatically injected into the world
     * by Dispatcher. So you can easily access it within your
     * systems.
     */
    class Entities {
    public:
        explicit Entities() {}

        /**
         * Create an entity listing all the initial components.
         * Automatically creates archetype.
         *
         * @return created entity.
         */
        template<typename ...Cmps>
        Entity create(Cmps &&...cmps) {
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

            std::vector<ComponentType> components{
                    ComponentType::create<std::remove_cvref_t<Cmps>>()...
            };

            Archetype *arch = archetypes.get_or_insert(components);
            EntityPosInChunk entity_pos = arch->allocate_entity(entity.id);

            (create_component(std::forward<Cmps>(cmps), entity_pos), ...);

            return entity;
        }

        /**
         * Erase all the components assigned to the entity
         * and mark the entity.
         *
         * Due to versioning, consecutive access by removed
         * entity will fail even if id is in use.
         *
         * @return true if entity existed.
         */
        bool destroy(Entity entity) {
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

        template<typename... Cmps, typename Fn>
        requires(Component<std::remove_const_t<Cmps>> &&...)
        void foreach(Fn &&f) const {
            foreach_impl<Cmps...>(std::forward<Fn>(f), std::make_index_sequence<sizeof...(Cmps)>());
        }

        /**
         * Checks if entity with such id is tracked as alive.
         */
        bool is_alive(Entity entity) const {
            return entity.id < entries.size() &&
                   entity.version == entries[entity.id].version;
        }

    private:
        ArchetypesStorage archetypes;

        struct Entry {
            uint32_t next = NO_ENTRY;
            uint32_t version = 0;
        };

        World *world = nullptr;
        std::vector<Entry> entries;
        uint32_t free_list = NO_ENTRY;
        static constexpr uint32_t NO_ENTRY = UINT32_MAX;

        template<Component C>
        static std::size_t get_component_index_in_archetype(const Archetype *archetype) {
            auto comp_type = ComponentType::create<std::remove_cvref_t<C>>();
            std::size_t comps_count = archetype->components_count();
            for (std::size_t i = 0; i < comps_count; ++i)
                if (archetype->component_types()[i] == comp_type)
                    return i;
            return comps_count;
        }

        template<typename C>
        void create_component(C &&cmp, EntityPosInChunk entity_pos) {
            using Comp = std::remove_cvref_t<C>;
            std::size_t comp_index = get_component_index_in_archetype<Comp>(entity_pos.archetype);
            auto *comp = (Comp *) entity_pos.archetype->get_component(entity_pos, comp_index);
            new(comp) Comp(std::forward<C>(cmp));
        }

        template<typename... Cmps, typename Fn, std::size_t... Indices>
        requires(Component<std::remove_const_t<Cmps>> &&...)
        void foreach_impl(Fn &&f, std::index_sequence<Indices...>) const {
            archetypes.query<std::remove_const_t<Cmps>...>([&](Archetype *arch) {
                std::size_t comp_indices[] = {
                        get_component_index_in_archetype<std::remove_const_t<Cmps>>(arch)...
                };
                std::size_t offsets[] = {
                        arch->component_offsets()[comp_indices[Indices]]...
                };
                std::size_t array_offsets[] = {
                        arch->component_types()[comp_indices[Indices]].array_offset...
                };

                std::size_t chunks_count = arch->chunks_count();
                Chunk *chunk = arch->chunks();

                std::size_t chunk_capacity = arch->chunk_capacity();
                for (std::size_t i = 0; i + 1 < chunks_count; ++i) {
                    uint8_t *ptrs[] = {
                            (chunk->data + offsets[Indices])...
                    };

                    for (std::size_t j = 0; j < chunk_capacity; ++j)
                        f((((Cmps *) ptrs[Indices])[j])...);
                    ++chunk;
                }

                std::size_t last_chunk_entities_count = chunk_capacity - arch->last_chunk_free_slots();
                uint8_t *ptrs[] = {
                        (chunk->data + offsets[Indices])...
                };

                for (std::size_t j = 0; j < last_chunk_entities_count; ++j) {
                    f((*(Cmps *) ptrs[Indices])...);

                    ((ptrs[Indices] += array_offsets[Indices]), ...);
                }
            });
        }
    };
}

#endif //HIGH_SHIFT_ENTITIES_H
