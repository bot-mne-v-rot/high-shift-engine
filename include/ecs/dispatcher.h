#ifndef HIGH_SHIFT_DISPATCHER_H
#define HIGH_SHIFT_DISPATCHER_H

#include "ecs/system.h"
#include "ecs/world.h"
#include "ecs/utils.h"

#include <tuple>

namespace ecs {
    namespace detail {
        template<typename R>
        void setup_resource(ecs::World &world) {
            if (!world.has<R>())
                world.emplace<R>(); // R is default-constructible
        }

        // https://stackoverflow.com/questions/42255534/c-execute-an-action-for-each-type-in-a-given-list
        template<typename Ret, System This, typename ...Args>
        void setup_resources(ecs::World &world, Ret (This::*)(Args...)) {
            int unused[] = {(setup_resource<std::remove_cvref_t<Args>>(world), 0)...};
            (void) unused; // to avoid a warning
        }

        template<typename Ret, System This, typename ...Args>
        void update_with_resources(ecs::World &world, This &system, Ret (This::*update)(Args...)) {
            (system.*update)(world.get<std::remove_cvref_t<Args>>()...);
        }
    }

    template<System ...Systems>
    class Dispatcher {
    public:
        Dispatcher() : Dispatcher(World()) {}

        explicit Dispatcher(World world_) : world(std::move(world_)) {
            detail::tuple_for_each(systems, [this]<System S>(S &system) {
                detail::setup_resources(world, &S::update);
            });
        }

        void update() {
            detail::tuple_for_each(systems, [this]<System S>(S &system) {
                detail::update_with_resources(world, system, &S::update);
            });
        }

        World &get_world() {
            return world;
        }

        const World &get_world() const {
            return world;
        }

    private:
        World world;
        std::tuple<Systems...> systems;
    };
}

#endif //HIGH_SHIFT_DISPATCHER_H
