#ifndef HIGH_SHIFT_DISPATCHER_H
#define HIGH_SHIFT_DISPATCHER_H

#include "ecs/system.h"
#include "ecs/entities.h"
#include "ecs/utils.h"
#include "ecs/game_loop_control.h"

#include <tuple>

namespace ecs {
    namespace detail {
        template<typename R>
        void setup_resource(ecs::World &world) {
            if constexpr(std::is_default_constructible_v<R>)
                if (!world.has<R>())
                    world.emplace<R>();
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

    /**
     * Heart of the ECS. Statically dispatches all the
     * resources used by the specified systems.
     * @tparam Systems
     */
    template<System ...Systems>
    class Dispatcher {
    public:
        Dispatcher() : Dispatcher(World()) {}

        explicit Dispatcher(World world_) : world(std::move(world_)) {
            world.emplace<Entities>(&world);
            world.emplace<GameLoopControl>();
        };

        /**
         * Calls all the setup methods for systems that provide id.
         * Automatically creates all the resources used by the systems.
         *
         * If one of the Systems failed to create,
         * destroys all of the created systems.
         */
        tl::expected<void, std::string> setup() {
            tl::expected<void, std::string> result;
            detail::tuple_for_each_branching(
                    systems,
                    [&, this]<System S>(S &system) {
                        if constexpr(SystemHasSetup<S>)
                            result = system.setup(world);
                        detail::setup_resources(world, &S::update);
                        return (bool) result;
                    },
                    [this]<System S>(S &system) {
                        if constexpr(SystemHasTeardown<S>)
                            system.teardown(world);
                    }
            );
            if (result)
                successfully_created = true;
            return result;
        }

        /**
         * Takes all the resources used by the systems and injects them
         * when update method of each system is called.
         */
        void update() {
            detail::tuple_for_each(systems, [this]<System S>(S &system) {
                detail::update_with_resources(world, system, &S::update);
            });
        }

        void run() {
            auto &game_loop_control = world.get<GameLoopControl>();
            while (successfully_created && !game_loop_control.stopped())
                update();
        }

        World &get_world() {
            return world;
        }

        const World &get_world() const {
            return world;
        }

        /**
         * Calls all the teardown methods for systems that provide id.
         */
        ~Dispatcher() {
            if (successfully_created) {
                detail::tuple_for_each(systems, [this]<System S>(S &system) {
                    if constexpr(SystemHasTeardown<S>)
                    system.teardown(world);
                });
            }
        }

    private:
        bool successfully_created = false;
        World world;
        std::tuple<Systems...> systems;
    };
}

#endif //HIGH_SHIFT_DISPATCHER_H
