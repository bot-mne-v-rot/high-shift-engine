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

        inline EntityPosInChunk transfer_entity(Entity entity,
                                                EntityPosInChunk old_entity_pos,
                                                ArchetypesStorage &archetypes,
                                                const std::vector<ComponentType> &transferred_components,
                                                const std::vector<ComponentType> &new_components) {
            Archetype *new_archetype = archetypes.get_or_insert(new_components);
            EntityPosInChunk new_entity_pos = new_archetype->allocate_entity(entity);

            transfer_component_data(old_entity_pos, new_entity_pos, transferred_components);

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

        template<typename, typename>
        struct ComponentExtractor;

        template<PlainComponent Comp, typename Ref>
        struct ComponentExtractor<Comp, Ref> {
            explicit ComponentExtractor(const Archetype *archetype) {
                std::size_t comp_index = archetype->layout().get_component_index<Comp>();
                offset = archetype->layout().component_offsets()[comp_index];
            }

            void update(Chunk *chunk) {
                array = (Comp * )(chunk->data + offset);
            }

            Ref get(std::size_t i) {
                return array[i];
            }

            std::size_t offset;
            Comp *array;
        };

        template<typename Ref>
        struct ComponentExtractor<Entity, Ref> {
            static_assert(!std::is_reference_v<Ref> || std::is_const_v<std::remove_reference_t<Ref>>,
                          "You can't mutate entity within foreach.");

            explicit ComponentExtractor(const Archetype *archetype) {
                offset = archetype->layout().entities_offset();
            }

            void update(Chunk *chunk) {
                entities = (Entity *) (chunk->data + offset);
            }

            Ref get(std::size_t i) {
                return entities[i];
            }

            std::size_t offset;
            Entity *entities;
        };

//        template<SharedComponent Comp, typename Ref>
//        struct ComponentExtractor<Comp, Ref> {
//            explicit ComponentExtractor(const Archetype *archetype) {
//                std::size_t comp_index = archetype->layout().get_shared_component_index<Comp>();
//                archetype->layout().shared_components_offset()
//                offset = archetype->layout().shared_components_offset()[comp_index];
//            }
//
//            void update(Chunk *chunk) {
//                comp = (Comp *) (chunk->data + offset);
//            }
//
//            Ref get(std::size_t) {
//                return array[i];
//            }
//
//            std::size_t offset;
//            Comp *comp;
//        };

        template<typename... Extractors, typename Fn, std::size_t... Indices>
        void iterate_chunk(Fn &&f,
                           Chunk *chunk,
                           std::tuple<Extractors...> extractors,
                           std::index_sequence<Indices...>) {
            (std::get<Indices>(extractors).update(chunk), ...);

            for (size_t j = 0; j < chunk->fields.entities_count; ++j)
                f(std::get<Indices>(extractors).get(j)...);
        }

        template<PlainComponent...>
        struct PlainComponentsArgsPack {};

        template<SharedComponent...>
        struct SharedComponentsArgsPack {};

        template<typename>
        struct ComponentPredicate : public std::false_type {};
        template<Component C>
        struct ComponentPredicate<C> : public std::true_type {};

        template<typename>
        struct PlainComponentPredicate : public std::false_type {};
        template<PlainComponent C>
        struct PlainComponentPredicate<C> : public std::true_type {};

        template<typename>
        struct SharedComponentPredicate : public std::false_type {};
        template<SharedComponent C>
        struct SharedComponentPredicate<C> : public std::true_type {};

        template<typename... Args>
        using FilterComponents = detail::filter<ComponentPredicate, Args...>;

        template<typename... Args>
        using FilterPlainComponents = detail::filter<PlainComponentPredicate, Args...>;

        template<typename... Args>
        using FilterSharedComponents = detail::filter<SharedComponentPredicate, Args...>;
    }

    template<typename... Cmps>
    Entity Entities::create(Cmps &&...cmps)
    requires(Component <std::decay_t<Cmps>> &&...) {
        ComponentType types[]{
                ComponentType::create<std::remove_cvref_t<Cmps>>()...
        };
        void *data[]{
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
    std::tuple<Cmps &...>
    Entities::get_components(Entity entity)
    const requires(Component <std::remove_const_t<Cmps>> &&...) {
        EntityChunkMapping &mapping = *archetypes.entities_mapping();
        EntityPosInChunk entity_pos = mapping[entity.id];
        return std::forward_as_tuple(
                detail::get_component<Cmps>(entity_pos)...
        );
    }

    template<typename... Cmps, typename Fn>
    void Entities::foreach_impl(Fn &&f) const {
        auto components = typename detail::FilterComponents<std::remove_cvref_t<Cmps>...>::type();
        archetypes.foreach(components, [&](Archetype *arch) {
            auto extractors = std::make_tuple(
                    detail::ComponentExtractor<std::remove_cvref_t<Cmps>, Cmps>(arch)...
            );

            auto indices = std::make_index_sequence<sizeof...(Cmps)>();
            std::size_t chunks_count = arch->chunks_count();
            Chunk **chunks = arch->chunks();

            for (std::size_t i = 0; i < chunks_count; ++i)
                detail::iterate_chunk(std::forward<Fn>(f), chunks[i],
                                      extractors, indices);
        });
    }

}