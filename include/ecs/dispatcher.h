#ifndef HIGH_SHIFT_DISPATCHER_H
#define HIGH_SHIFT_DISPATCHER_H

#include "ecs/system.h"
#include "ecs/entities.h"
#include "ecs/utils.h"
#include "ecs/game_loop_control.h"

#include <tuple>

namespace ecs {
    namespace detail {
        class ISystem;

        static const std::size_t max_systems = 128;
        using ISystemsArray = std::array<ISystem, max_systems>;
        using ISystemsVector = std::vector<detail::ISystem>;

        class ISystem {
        public:
            ISystem() = default;

            template<System S>
            explicit ISystem(S *system_ptr);

            [[nodiscard]] tl::expected<void, std::string>
            setup(ecs::World &world, const ISystemsArray &systems_by_id) {
                return f_setup(system_ptr, world, systems_by_id);
            }

            void update(ecs::World &world) {
                f_update(system_ptr, world);
            }

            void teardown(ecs::World &world) {
                f_teardown(system_ptr, world);
            }

            void destroy_and_delete() {
                f_delete(system_ptr);
            }

            const std::vector<uint32_t> &dependencies() const {
                return deps;
            }

            uint32_t id() const {
                return id_;
            }

        private:
            using FnSetup = tl::expected<void, std::string> (*)(void *, ecs::World &, const ISystemsArray &);
            FnSetup f_setup = nullptr;

            using FnUpdate = void (*)(void *, ecs::World &);
            FnUpdate f_update = nullptr;

            using FnTeardown = void (*)(void *, ecs::World &);
            FnTeardown f_teardown = nullptr;

            using FnDelete = void (*)(void *);
            FnDelete f_delete = nullptr;

            uint32_t id_ = 0;
            void *system_ptr = nullptr;
            std::vector<uint32_t> deps;
        };
    }

    /**
     * Heart of the ECS. Statically dispatches all the
     * resources used by the specified systems.
     * @tparam Systems
     */
    class Dispatcher {
    public:
        explicit Dispatcher() : world(std::make_unique<World>()) {
            world->emplace<Entities>(world.get());
            world->emplace<GameLoopControl>();
        };

        Dispatcher(Dispatcher &&) = default;
        Dispatcher &operator=(Dispatcher &&) = default;

        template<System... Systems>
        static tl::expected<Dispatcher, std::string> create();

        /**
         * Takes all the resources used by the systems and injects them
         * when update method of each system is called.
         */
        void update();

        void loop() {
            auto &game_loop_control = world->get<GameLoopControl>();
            while (successfully_created && !game_loop_control.stopped())
                update();
        }

        World &get_world() {
            return *world;
        }

        const World &get_world() const {
            return *world;
        }

        /**
         * Calls all the teardown methods for systems that provide id.
         * and calls all the systems' destructors.
         */
        ~Dispatcher();

    private:
        template<System... Systems>
        void add();

        template<System S>
        void _add();

        /**
         * Calls all the setup methods for systems that provide id.
         * Automatically creates all the resources used by the systems.
         *
         * If one of the Systems failed to setup,
         * calls all the teardowns of the systems that have been already set up.
         */
        [[nodiscard]] tl::expected<void, std::string> setup();

        detail::ISystemsArray systems_by_id;
        detail::ISystemsVector systems;

        bool successfully_created = false;
        std::unique_ptr<World> world; // unique_ptr to be easily relocated
    };
}

#include "ecs/detail/dispatcher_impl.h"

#endif //HIGH_SHIFT_DISPATCHER_H
