namespace ecs {
    namespace detail {
        inline std::size_t get_component_index_in_archetype(const Archetype *archetype, ComponentType comp_type) {
            std::size_t comps_count = archetype->components_count();
            for (std::size_t i = 0; i < comps_count; ++i)
                if (archetype->component_types()[i] == comp_type)
                    return i;
            return comps_count;
        }

        template<Component C>
        std::size_t get_component_index_in_archetype(const Archetype *archetype) {
            auto comp_type = ComponentType::create<std::remove_cvref_t<C>>();
            return get_component_index_in_archetype(archetype, comp_type);
        }

        template<typename C>
        void create_component(C &&cmp, EntityPosInChunk entity_pos) {
            using Comp = std::remove_cvref_t<C>;
            std::size_t comp_index = get_component_index_in_archetype<Comp>(entity_pos.archetype);
            auto *comp = (Comp *) entity_pos.archetype->get_component(entity_pos, comp_index);
            new(comp) Comp(std::forward<C>(cmp));
        }

        template<typename... Cmps, typename Fn, std::size_t... Indices>
        void iterate_chunk(Fn &&f,
                           Chunk *chunk, size_t entities_count,
                           const std::size_t *offsets,
                           std::index_sequence<Indices...>) {
            uint8_t *ptrs[] = {
                    (chunk->data + offsets[Indices])...
            };

            for (size_t j = 0; j < entities_count; ++j)
                f((((Cmps *) ptrs[Indices])[j])...);
        }

        template<typename... Cmps, typename Fn, std::size_t... Indices>
        void iterate_chunk_with_entities(Fn &&f,
                                         Chunk *chunk, size_t entities_count,
                                         std::size_t entities_offset,
                                         const std::size_t *offsets,
                                         std::index_sequence<Indices...>) {
            Entity *entities = (Entity *) (chunk->data + entities_offset);
            uint8_t *ptrs[] = {
                    (chunk->data + offsets[Indices])...
            };

            for (size_t j = 0; j < entities_count; ++j)
                f(entities[j], (((Cmps *) ptrs[Indices])[j])...);
        }

        inline void transfer_entity(EntityPosInChunk from, EntityPosInChunk to,
                                    const std::vector<ComponentType> &transferred_components) {
            for (ComponentType component : transferred_components) {
                std::size_t from_index = detail::get_component_index_in_archetype(from.archetype, component);
                std::size_t to_index = detail::get_component_index_in_archetype(to.archetype, component);

                memcpy(to.archetype->get_component(to, to_index),
                       from.archetype->get_component(from, from_index),
                       component.size);
            }
        }

        inline EntityPosInChunk transfer_entity(Entity entity,
                                                EntityPosInChunk old_entity_pos,
                                                ArchetypesStorage &archetypes,
                                                const std::vector<ComponentType> &transferred_components,
                                                const std::vector<ComponentType> &new_components) {
            Archetype *new_archetype = archetypes.get_or_insert(new_components);
            EntityPosInChunk new_entity_pos = new_archetype->allocate_entity(entity);

            detail::transfer_entity(old_entity_pos, new_entity_pos, transferred_components);

            old_entity_pos.archetype->deallocate_entity(old_entity_pos);
            if (old_entity_pos.archetype->entities_count() == 1)
                archetypes.erase(old_entity_pos.archetype);

            return new_entity_pos;
        }

        template<typename C>
        C &get_component(EntityPosInChunk entity_pos) {
            using Comp = std::remove_const_t<C>;
            std::size_t comp_index = detail::get_component_index_in_archetype<Comp>(entity_pos.archetype);
            return *(C *) entity_pos.archetype->get_component(entity_pos, comp_index);
        }
    }

    template<typename... Cmps>
    Entity Entities::create(Cmps &&...cmps)
    requires(Component<std::decay_t<Cmps>> && ...) {
        ComponentType types[] {
            ComponentType::create<std::remove_cvref_t<Cmps>>()...
        };
        void *data[] {
                &cmps...
        };
        return create(sizeof...(Cmps), types, data);
    }

    template<typename... Cmps>
    void Entities::add_components(Entity entity, Cmps &&...cmps) {
        if (!is_alive(entity))
            return;

        EntityChunkMapping &mapping = *archetypes.entities_mapping();
        EntityPosInChunk old_entity_pos = mapping[entity.id];

        const auto &transferred_components = old_entity_pos.archetype->component_types();

        auto new_components = transferred_components;
        (new_components.push_back(ComponentType::create<std::remove_cvref_t<Cmps>>()), ...);

        EntityPosInChunk new_entity_pos = detail::transfer_entity(entity, old_entity_pos, archetypes,
                                                                  transferred_components, new_components);
        (detail::create_component(std::forward<Cmps>(cmps), new_entity_pos), ...);
    }

    template<Component... Cmps>
    void Entities::remove_components(Entity entity) {
        if (!is_alive(entity))
            return;

        EntityChunkMapping &mapping = *archetypes.entities_mapping();
        EntityPosInChunk old_entity_pos = mapping[entity.id];

        auto new_components = old_entity_pos.archetype->component_types();
        (std::erase(new_components, ComponentType::create<Cmps>()), ...);

        EntityPosInChunk new_entity_pos = detail::transfer_entity(entity, old_entity_pos, archetypes,
                                                                  new_components, new_components);
    }

    template<typename... Cmps>
    std::tuple<Cmps &...> Entities::get_components(Entity entity) const
    requires(Component <std::remove_const_t<Cmps>> &&...) {
        EntityChunkMapping &mapping = *archetypes.entities_mapping();
        EntityPosInChunk entity_pos = mapping[entity.id];
        return std::forward_as_tuple(
                detail::get_component<Cmps>(entity_pos)...
        );
    }

    template<typename... Cmps, typename Fn, std::size_t... Indices>
    void Entities::foreach_impl(Fn &&f, std::index_sequence<Indices...> indices) const {
        archetypes.foreach<std::remove_const_t<Cmps>...>([&](Archetype *arch) {
            std::size_t comp_indices[] = {
                    detail::get_component_index_in_archetype<std::remove_const_t<Cmps>>(arch)...
            };
            std::size_t offsets[] = {
                    arch->component_offsets()[comp_indices[Indices]]...
            };

            std::size_t chunks_count = arch->chunks_count();
            Chunk *chunk = arch->chunks();

            std::size_t chunk_capacity = arch->chunk_capacity();
            for (std::size_t i = 0; i + 1 < chunks_count; ++i) {
                detail::iterate_chunk<Cmps...>(std::forward<Fn>(f),
                                               chunk, chunk_capacity,
                                               offsets, indices);
                ++chunk;
            }
            if (chunks_count) {
                std::size_t last_chunk_entities_count = chunk_capacity - arch->last_chunk_free_slots();
                detail::iterate_chunk<Cmps...>(std::forward<Fn>(f),
                                               chunk, last_chunk_entities_count,
                                               offsets, indices);
            }
        });
    }

    template<typename... Cmps, typename Fn, std::size_t... Indices>
    void Entities::foreach_with_entities_impl(Fn &&f, std::index_sequence<Indices...> indices) const {
        archetypes.foreach<std::remove_const_t<Cmps>...>([&](Archetype *arch) {
            std::size_t comp_indices[] = {
                    detail::get_component_index_in_archetype<std::remove_const_t<Cmps>>(arch)...
            };
            std::size_t offsets[] = {
                    arch->component_offsets()[comp_indices[Indices]]...
            };
            std::size_t entities_offset = arch->entities_offset();

            std::size_t chunks_count = arch->chunks_count();
            Chunk *chunk = arch->chunks();

            std::size_t chunk_capacity = arch->chunk_capacity();
            for (std::size_t i = 0; i + 1 < chunks_count; ++i) {
                detail::iterate_chunk_with_entities<Cmps...>(std::forward<Fn>(f),
                                                             chunk, chunk_capacity,
                                                             entities_offset, offsets,
                                                             indices);
                ++chunk;
            }
            if (chunks_count) {
                std::size_t last_chunk_entities_count = chunk_capacity - arch->last_chunk_free_slots();
                detail::iterate_chunk_with_entities<Cmps...>(std::forward<Fn>(f),
                                                             chunk, last_chunk_entities_count,
                                                             entities_offset, offsets,
                                                             indices);
            }
        });
    }

}