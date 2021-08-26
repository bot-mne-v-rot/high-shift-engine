#ifndef HIGH_SHIFT_CHUNK_LAYOUT_H
#define HIGH_SHIFT_CHUNK_LAYOUT_H

#include "ecs/chunk.h"
#include "ecs/component.h"
#include "ecs/entity.h"

#include <vector>
#include <memory>

namespace ecs {
    /**
     * You can imagine that chunk data
     * is actually a struct like this
     * struct Data {
     *     ShCompVal shared_components[_shared_components_count];
     *     Entity entities[_chunk_capacity];
     *     Component_0 c0[_chunk_capacity];
     *     Component_1 c1[_chunk_capacity];
     *     // ...
     * }
     *
     * Then these values are just:
     * _shared_components_offset = offsetof(Data, shared_components);
     * _entities_offset = offsetof(Data, entities);
     * _component_offsets[i] = offsetof(Data, ci);
     *
     * Offsets are calculated with respect to the
     * C struct alignment rules.
     *
     * There is no need in additional padding
     * at the beginning or at the end of the struct
     * because chunks are aligned by the page size.
     */
    class ChunkLayout {
    public:
        explicit ChunkLayout(std::vector<ComponentType> component_types,
                             std::vector<ComponentType> shared_component_types)
                : _component_types(std::move(component_types)),
                  _shared_component_types(std::move(shared_component_types)) {
            calculate_capacity();
        }

        const std::vector<ComponentType> &component_types() const {
            return _component_types;
        }

        const std::vector<ComponentType> &shared_component_types() const {
            return _shared_component_types;
        }

        const std::vector<std::size_t> &component_offsets() const {
            return _component_offsets;
        }

        std::size_t components_count() const {
            return _component_types.size();
        }

        std::size_t shared_components_count() const {
            return _shared_component_types.size();
        }

        std::size_t chunk_capacity() const {
            return _chunk_capacity;
        }

        std::size_t entities_offset() const {
            return _entities_offset;
        }

        std::size_t shared_components_offset() const {
            return _shared_components_offset;
        }

        Entity *get_entities(Chunk *chunk) const {
            return (Entity *) (chunk->data + _entities_offset);
        }

        ShCompVal *get_shared_components_values(Chunk *chunk) const {
            return (ShCompVal *) (chunk->data + _shared_components_offset);
        }

        void *get_component_array(Chunk *chunk, std::size_t comp_index) const {
            return chunk->data + _component_offsets[comp_index];
        }

        void *get_component(Chunk *chunk, std::size_t comp_index, std::size_t index_in_chunk) const {
            return (uint8_t *) get_component_array(chunk, comp_index) +
                   index_in_chunk * _component_types[comp_index].array_offset;
        }

        template<Component T>
        T *get_component_array(Chunk *chunk, std::size_t comp_index) const {
            return static_cast<T *>(get_component_array(chunk, comp_index));
        }

        template<Component T>
        T *get_component(Chunk *chunk, std::size_t comp_index, std::size_t index_in_chunk) const {
            return get_component_array<T>(chunk, comp_index)[index_in_chunk];
        }

        std::size_t get_component_index(const ComponentType &type) const {
            return get_component_index_for(type, _component_types);
        }

        template<Component C>
        std::size_t get_component_index() const {
            return get_component_index(ComponentType::create<C>());
        }

        void get_component_indices(std::size_t types_count,
                                   const ComponentType *types,
                                   std::size_t *indices) const {
            for (std::size_t i = 0; i < types_count; ++i)
                indices[i] = get_component_index(types[i]);
        }

        std::size_t get_shared_component_index(const ComponentType &type) const {
            return get_component_index_for(type, _shared_component_types);
        }

        template<Component C>
        std::size_t get_shared_component_index() const {
            return get_shared_component_index(ComponentType::create<C>());
        }

        void get_shared_component_indices(std::size_t types_count,
                                   const ComponentType *types,
                                   std::size_t *indices) const {
            for (std::size_t i = 0; i < types_count; ++i)
                indices[i] = get_shared_component_index(types[i]);
        }

    private:
        std::vector<ComponentType> _component_types;
        std::vector<ComponentType> _shared_component_types;
        std::vector<std::size_t> _component_offsets;

        std::size_t _entities_offset;
        std::size_t _shared_components_offset;
        std::size_t _chunk_capacity;

        void calculate_capacity();
        std::size_t calculate_offsets();

        std::size_t get_component_index_for(const ComponentType &type,
                                            const std::vector<ComponentType> &types) const {
            std::size_t comps_count = components_count();
            for (std::size_t i = 0; i < comps_count; ++i)
                if (types[i] == type)
                    return i;
            return comps_count;
        }
    };
}

#endif //HIGH_SHIFT_CHUNK_LAYOUT_H
