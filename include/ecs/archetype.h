#ifndef HIGH_SHIFT_ARCHETYPE_H
#define HIGH_SHIFT_ARCHETYPE_H

#include "ecs/chunk.h"
#include "ecs/component.h"
#include "ecs/entity.h"
#include "ecs/id_set.h"

#include <vector>

namespace ecs {
    class Archetype;

    struct EntityPosInChunk {
        Archetype *archetype;
        uint32_t chunk_index;
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
        bool insert(Id id, const EntityPosInChunk &entity_pos);

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
        std::vector<EntityPosInChunk> dense;
        std::vector<uint32_t> dense_ids;
        std::vector<uint32_t> sparse;
        IdSet present;
    };

    class Archetype {
    public:
        Archetype(std::vector<ComponentType> component_types,
                  EntityChunkMapping *entity_chunk_mapping)
                : _component_types(std::move(component_types)),
                  entities_mapping(entity_chunk_mapping) {
            calculate_capacity();
        }

        EntityPosInChunk allocate_entity(Entity entity);
        void deallocate_entity(EntityPosInChunk entity_pos);

        Chunk *chunks() {
            return _chunks.data();
        }

        const Chunk *chunks() const {
            return _chunks.data();
        }

        std::size_t chunks_count() const {
            return _chunks.size();
        }

        std::size_t components_count() const {
            return _component_types.size();
        }

        const std::vector<ComponentType> &component_types() const {
            return _component_types;
        }

        const std::vector<std::size_t> &component_offsets() const {
            return _component_offsets;
        }

        std::size_t chunk_capacity() const {
            return _chunk_capacity;
        }

        std::size_t entities_offset() const {
            return _entities_offset;
        }

        std::size_t entities_count() const {
            return _entities_count;
        }

        std::size_t last_chunk_free_slots() const {
            return _last_chunk_free_slots;
        }

        Entity get_entity(EntityPosInChunk pos) const {
            auto *entities = (const Entity *) (_chunks[pos.chunk_index].data + _entities_offset);
            return entities[pos.index_in_chunk];
        }

        void *get_component(EntityPosInChunk pos, std::size_t comp_index) {
            return _chunks[pos.chunk_index].data + _component_offsets[comp_index]
                   + pos.index_in_chunk * _component_types[comp_index].array_offset;
        }

    private:
        std::vector<Chunk> _chunks;
        std::vector<ComponentType> _component_types;
        std::vector<std::size_t> _component_offsets;
        EntityChunkMapping *entities_mapping;
        std::size_t _entities_offset;
        std::size_t _chunk_capacity;
        std::size_t _last_chunk_free_slots = 0;
        std::size_t _entities_count = 0;

        void calculate_capacity();
        std::size_t calculate_offsets();

        void allocate_chunk();
        void deallocate_chunk(std::size_t chunk_index);
        void deallocate_last_chunk();

        Id swap_ent_with_last_to_remove(EntityPosInChunk entity_pos);
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
        void query(Fn &&f) const {
            DynamicIdSetAnd sets_and = query_mask({
                ComponentType::create<Cs>()...
            });

            foreach(sets_and, [&](Id id) {
                f(storage[id].get());
            });
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
}

#endif //HIGH_SHIFT_ARCHETYPE_H
