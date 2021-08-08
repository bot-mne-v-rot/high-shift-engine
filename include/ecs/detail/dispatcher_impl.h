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

        inline uint32_t _get_system_id() {
            static uint32_t id = 0;
            return ++id;
        }

        template<System>
        inline uint32_t get_system_id() {
            static uint32_t id = _get_system_id();
            return id;
        }

        template<typename Ret, System This, typename ...Args>
        Ret setup_system(ecs::World &world, const ISystemsArray &systems_by_id,
                          This &system, Ret (This::*setup)(ecs::World &, Args...)) {
            return (system.*setup)(world, systems_by_id[get_system_id<std::remove_cvref_t<Args>>()]...);
        }

        template<typename Ret, System This, typename ...Args>
        void gather_dependencies(Ret (This::*setup)(ecs::World &, Args...),
                                 std::vector<uint32_t> &result) {
            uint32_t deps[] = {
                    get_system_id<std::remove_cvref_t<Args>>()...
            };
            for (uint32_t dep : deps)
                result.push_back(dep);
        }

        template<System S>
        detail::ISystem::ISystem(S *system_ptr) : system_ptr(system_ptr), id_(get_system_id<S>()) {
            f_setup = [](void *sys_ptr, ecs::World &world, const ISystemsArray &systems_by_id) {
                tl::expected<void, std::string> result;

                S &sys = *static_cast<S *>(sys_ptr);
                if constexpr(SystemHasSetup<S>)
                    result = detail::setup_system(world, systems_by_id, sys, &S::setup);
                if (result)
                    detail::setup_resources(world, &S::update);

                return result;
            };

            f_update = [](void *sys_ptr, ecs::World &world) {
                S &sys = *static_cast<S *>(sys_ptr);
                detail::update_with_resources(world, sys, &S::update);
            };

            f_teardown = [](void *sys_ptr, ecs::World &world) {
                S &sys = *static_cast<S *>(sys_ptr);
                if constexpr(SystemHasTeardown<S>)
                    sys.teardown(world);
            };

            f_delete = [](void *sys_ptr) {
                delete static_cast<S *>(sys_ptr);
            };

            if constexpr(SystemHasSetup<S>)
                gather_dependencies(&S::setup, deps);
        }
    }

    template<System... Systems>
    tl::expected<Dispatcher, std::string> Dispatcher::create() {
        Dispatcher dispatcher;
        dispatcher.add<Systems...>();

        if (auto result = dispatcher.setup())
            return dispatcher;
        else
            return tl::make_unexpected(result.error());
    }

    template<System... Systems>
    void Dispatcher::add() {
        int unused[] = {
                (_add<Systems>(), 0)...
        };
        (void) unused; // to suppress warning
    }

    template<System S>
    void Dispatcher::_add() {
        S *system_ptr = new S();
        detail::ISystem i_system(system_ptr);

        systems.push_back(i_system);
        systems_by_id[i_system.id()] = i_system;
    }
}
