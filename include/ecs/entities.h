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
        Entity create(Cmps &&...cmps);

        /**
         * Erase all the components assigned to the entity
         * and mark the entity as removed.
         *
         * If the archetype of the entity becomes empty,
         * it will be automatically removed.
         *
         * Due to versioning, consecutive access by removed
         * entity will fail even if id is in use.
         *
         * @return true if entity existed.
         */
        bool destroy(Entity entity);

        /**
         * Adds listed components to the entity.
         *
         * New archetype is implicitly created and
         * the entity is transferred.
         *
         * @example
         * Entities.add_components(entity, ComponentA{}, ComponentB{});
         */
        template<typename... Cmps>
        void add_components(Entity entity, Cmps &&...cmps);

        /**
         * Removes listed components from the entity.
         *
         * New archetype is implicitly created and
         * the entity is transferred.
         *
         * @example
         * Entities.remove_components<ComponentA, ComponentB>(entity);
         */
        template<Component... Cmps>
        void remove_components(Entity entity);

        /**
         * Iterates over each entity that has listed components.
         *
         * Deduces types from the function signature.
         * @example
         * Entities.foreach([](const ComponentA &a, ComponentB &b) {
         *     // ...
         * });
         *
         * To get the entity list it as a first argument.
         * @example
         * Entities.foreach([](Entity entity, const ComponentA &a, ComponentB &b) {
         *     // ...
         * });
         */
        template<typename Fn>
        void foreach(Fn &&fn) const {
            foreach_functor(fn, &std::remove_cvref_t<Fn>::operator());
        }

        template<typename... Cmps>
        std::tuple<Cmps &...> get_components(Entity entity) const
        requires(Component<std::remove_const_t<Cmps>> && ...);

        template<typename Ret, typename... Args>
        requires(Component<std::remove_cvref_t<Args>> &&...)
        void foreach(Ret(*f)(Args...)) const {
            foreach_impl<std::remove_reference_t<Args>...>(f, std::make_index_sequence<sizeof...(Args)>());
        }

        template<typename Ret, typename... Args>
        requires(Component<std::remove_cvref_t<Args>> &&...)
        void foreach(Ret(*f)(Entity, Args...)) const {
            foreach_with_entities_impl<std::remove_reference_t<Args>...>(f, std::make_index_sequence<sizeof...(Args)>());
        }

        /**
         * Checks if entity with such id is tracked as alive.
         */
        bool is_alive(Entity entity) const {
            return entity.id < entries.size() &&
                   entity.version == entries[entity.id].version;
        }

        ArchetypesStorage archetypes;

    private:
        struct Entry {
            uint32_t next = NO_ENTRY;
            uint32_t version = 0;
        };

        World *world = nullptr;
        std::vector<Entry> entries;
        uint32_t free_list = NO_ENTRY;
        static constexpr uint32_t NO_ENTRY = UINT32_MAX;

        template<typename Fn, typename This, typename Ret, typename... Args>
        requires(Component<std::remove_cvref_t<Args>> &&...)
        void foreach_functor(Fn &&functor, Ret (This::*)(Args...)) const {
            foreach_impl<std::remove_reference_t<Args>...>(std::forward<Fn>(functor),
                                                           std::make_index_sequence<sizeof...(Args)>());
        }

        template<typename Fn, typename This, typename Ret, typename... Args>
        requires(Component<std::remove_cvref_t<Args>> &&...)
        void foreach_functor(Fn &&functor, Ret (This::*)(Args...) const) const {
            foreach_impl<std::remove_reference_t<Args>...>(std::forward<Fn>(functor),
                                                           std::make_index_sequence<sizeof...(Args)>());
        }

        template<typename Fn, typename This, typename Ret, typename... Args>
        requires(Component<std::remove_cvref_t<Args>> &&...)
        void foreach_functor(Fn &&functor, Ret (This::*)(Entity, Args...)) const {
            foreach_with_entities_impl<std::remove_reference_t<Args>...>(std::forward<Fn>(functor),
                                                                         std::make_index_sequence<sizeof...(Args)>());
        }

        template<typename Fn, typename This, typename Ret, typename... Args>
        requires(Component<std::remove_cvref_t<Args>> &&...)
        void foreach_functor(Fn &&functor, Ret (This::*)(Entity, Args...) const) const {
            foreach_with_entities_impl<std::remove_reference_t<Args>...>(std::forward<Fn>(functor),
                                                                         std::make_index_sequence<sizeof...(Args)>());
        }

        template<typename... Cmps, typename Fn, std::size_t... Indices>
        void foreach_impl(Fn &&f, std::index_sequence<Indices...> indices) const;

        template<typename... Cmps, typename Fn, std::size_t... Indices>
        void foreach_with_entities_impl(Fn &&f, std::index_sequence<Indices...> indices) const;
    };
}

#include "ecs/detail/entities_impl.h"

#endif //HIGH_SHIFT_ENTITIES_H
