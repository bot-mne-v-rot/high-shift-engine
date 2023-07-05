#ifndef HIGH_SHIFT_ARCHETYPE_H
#define HIGH_SHIFT_ARCHETYPE_H

#include "ecs/chunk.h"
#include "ecs/component.h"
#include "ecs/entity.h"
#include "ecs/id_set.h"
#include "ecs/chunk_layout.h"
#include "ecs/chunks_data.h"
#include "ecs/chunks_map.h"

#include <vector>

namespace ecs {
    class Archetype;

    struct EntityPosInChunk {
        Archetype *archetype;
        Chunk *chunk;
        uint16_t index_in_chunk;

        bool operator==(const EntityPosInChunk &) const = default;
        bool operator!=(const EntityPosInChunk &) const = default;
    };

    class EntityChunkMapping {
    public:
        const EntityPosInChunk &operator[](Id id) const {
            return dense[sparse[id]];
        }

        EntityPosInChunk &operator[](Id id) {
            return dense[sparse[id]];
        }

        /**
         * @return true if id wasn't present in the table
         */
        bool insert(Id id, EntityPosInChunk entity_pos);

        /**
         * Same as insert but does not perform checking if id is
         * present. Does not perform auto memory allocations.
         */
        void insert_unsafe(Id id, EntityPosInChunk entity_pos);

        /**
         * Same as insert but for multiple entities.
         * Does not perform version checking.
         * Does not perform auto memory allocations.
         *
         * Position for the next entity is just an increment of the previous index_in_chunk.
         * chunk_index is not incremented.
         */
        void insert_multiple(std::size_t entities_count, const Entity *entities,
                             EntityPosInChunk starting_pos);

        void reserve(Id max_id, std::size_t additional_count);

        /**
         * @return true if id was present in the table
         */
        bool erase(Id id);

        bool contains(Id id) const {
            return present.contains(id);
        }

        std::size_t size() const {
            return present.size();
        }

        bool empty() const {
            return present.empty();
        }

    private:
        // plain list of EntityPosInChunk
        std::vector<EntityPosInChunk> dense;

        // inverse of `sparse`: mapping from positions in `dense` to entity ids
        std::vector<uint32_t> dense_ids;

        // mapping from entity idd to positions in `dense`
        std::vector<uint32_t> sparse;

        IdSet present;
    };

    struct ComponentsData {
        std::size_t components_count;
        const ComponentType *types;
        const void *const *data;
    };

    struct SharedComponentsData {
        std::size_t components_count;
        const ComponentType *types;
        const ShCompVal *data;
    };

    class Archetype {
    public:
        Archetype(std::vector<ComponentType> component_types,
                  std::vector<ComponentType> shared_component_types,
                  EntityChunkMapping *entity_chunk_mapping)
                : _layout(std::move(component_types),
                          std::move(shared_component_types)),
                  entities_mapping(entity_chunk_mapping),
                  _chunks_data(0) {}

        EntityPosInChunk allocate_entity(Entity entity);
        void deallocate_entity(EntityPosInChunk entity_pos);

        /**
         * Efficiently allocates entities in a row.
         */
        void allocate_entities(std::size_t entities_count,
                               ComponentsData components,
                               SharedComponentsData shared_components,
                               const Entity *entities);

        Chunk **chunks() {
            return _chunks_data.chunks_ptr();
        }

        const Chunk *const *chunks() const {
            return _chunks_data.chunks_ptr();
        }

        std::size_t chunks_count() const {
            return _chunks_data.chunks_count();
        }

        std::size_t components_count() const {
            return _layout.components_count();
        }

        const std::vector<ComponentType> &component_types() const {
            return _layout.component_types();
        }

        std::size_t shared_components_count() const {
            return _layout.shared_components_count();
        }

        const std::vector<ComponentType> &shared_component_types() const {
            return _layout.shared_component_types();
        }

        std::size_t entities_count() const {
            return _entities_count;
        }

        const ChunkLayout &layout() const {
            return _layout;
        }

        void *get_component(EntityPosInChunk pos, std::size_t comp_index) {
            return _layout.get_component(pos.chunk, comp_index, pos.index_in_chunk);
        }

        Entity get_entity(EntityPosInChunk pos) const {
            return _layout.get_entities(pos.chunk)[pos.index_in_chunk];
        }

        friend void transfer_component_data(EntityPosInChunk from, EntityPosInChunk to,
                                            const std::vector<ComponentType> &transferred_components);

    private:
        ChunksData _chunks_data;

        ChunkLayout _layout;
        ChunksMap _chunks_map;
        EntityChunkMapping *entities_mapping;

        std::size_t _entities_count = 0;

        void reserve_chunks(std::size_t count);
        void deallocate_chunk(std::size_t chunk_index);
        void deallocate_last_chunk();

        Id swap_ent_with_to_remove(EntityPosInChunk removed_pos,
                                   EntityPosInChunk last_pos);
    };

    class ArchetypesStorage {
    public:
        using Storage = std::vector<std::unique_ptr<Archetype>>;

        Archetype *get_or_insert(const std::vector<ComponentType> &types);
        Archetype *get_or_insert(std::size_t components_count,
                                 const ComponentType *types);
        void erase(Archetype *archetype);

        static constexpr std::size_t max_components = 1024;
        IdSet component_masks[max_components];

        template<Component... Cs, typename Fn>
        void foreach(Fn &&f) const {
            ComponentType types[]{
                    ComponentType::create<Cs>()...
            };
            DynamicIdSetAnd sets_and = query_mask(sizeof...(Cs), types);

            ecs::foreach(sets_and, [&](Id id) {
                f(storage[id].get());
            });
        }

        template<Component... Cs, typename Fn>
        void foreach(std::tuple<Cs...>, Fn &&f) const {
            foreach<Cs...>(std::forward<Fn>(f));
        }

        EntityChunkMapping *entities_mapping() const {
            return _mapping.get();
        }

        std::size_t size() const {
            return storage.size();
        }

    private:
        Storage storage;
        std::unique_ptr<EntityChunkMapping> _mapping = std::make_unique<EntityChunkMapping>();

        Storage::iterator find(std::size_t components_count,
                               const ComponentType *types);
        DynamicIdSetAnd query_mask(std::size_t components_count,
                                   const ComponentType *types) const;
    };

    void transfer_component_data(EntityPosInChunk from, EntityPosInChunk to,
                                 const std::vector<ComponentType> &transferred_components);
}

#endif //HIGH_SHIFT_ARCHETYPE_H
