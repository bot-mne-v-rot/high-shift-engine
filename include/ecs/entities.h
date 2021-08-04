#ifndef HIGH_SHIFT_ENTITIES_H
#define HIGH_SHIFT_ENTITIES_H

#include "ecs/world.h"
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
        explicit Entities(World *world) : world(world) {}

        /**
         * Create an entity listing all the initial components.
         * Alternatively, you can call this method with no parameters
         * and insert each component to the storage directly.
         *
         * @note Inserting components directly for non-existing
         * entities can lead to those components being overridden
         * if its id is allocated by Entities class.
         *
         * @return id of just created entity.
         */
        template<typename ...Cmps>
        Id create(Cmps &&...cmps) {
            Id id = (~alive).first();
            alive.insert(id);
            int unused[] = {
                    (create_component(id, std::forward<Cmps>(cmps)), 0)...
            };
            (void) unused;
            return id;
        }

        /**
         * Erase all the components assigned to the id
         * and mark the id as unused.
         *
         * This method also deletes components
         * even if they weren't initially
         * assigned to the id by the `create` method.
         *
         * @note This method is kinda slow because
         * it iterates over all storages and
         * indirectly calling `erase` method
         * via type-erasure.
         *
         * @return true if entity existed.
         */
        bool destroy(Id id) {
            if (!alive.contains(id))
                return false;
            alive.erase(id);

            for (auto storage : world->storages())
                storage.erase(id);
            return true;
        }

        /**
         * Checks if entity with such id is tracked as alive.
         */
        bool is_alive(Id id) const {
            return alive.contains(id);
        }

    private:
        World *world = nullptr;
        IdSet alive;

        template<typename Cmp>
        requires Component<std::remove_cvref_t<Cmp>>
        void create_component(Id id, Cmp &&cmp) {
            world->get<typename std::remove_cvref_t<Cmp>::Storage>()
                    .insert(id, std::forward<Cmp>(cmp));
        }
    };
}

#endif //HIGH_SHIFT_ENTITIES_H
