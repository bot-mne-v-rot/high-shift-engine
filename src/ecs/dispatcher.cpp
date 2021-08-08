#include "ecs/dispatcher.h"

namespace ecs {
    namespace detail {
        static void topsort_systems_dfs(const ISystem &system, const ISystemsArray &systems_by_id,
                                        ISystemsVector &result, std::vector<bool> &visited) {
            if (visited[system.id()])
                return;
            visited[system.id()] = true;

            for (uint32_t system_id : system.dependencies())
                topsort_systems_dfs(systems_by_id[system_id], systems_by_id,
                                    result, visited);
            result.push_back(system);
        }

        static ISystemsVector topsort_systems(const ISystemsVector &systems,
                                              const ISystemsArray &systems_by_id) {
            ISystemsVector result;
            result.reserve(systems.size());
            std::vector<bool> visited(max_systems, false);

            for (const ISystem &system : systems)
                topsort_systems_dfs(system, systems_by_id, result, visited);

            return result;
        }
    }

    tl::expected<void, std::string> Dispatcher::setup() {
        systems = detail::topsort_systems(systems, systems_by_id);
        for (std::size_t i = 0; i < systems.size(); ++i) {
            tl::expected<void, std::string> result;
            result = systems[i].setup(*world, systems_by_id);

            if (!result) {
                for (std::size_t j = 0; j < i; ++j)
                    systems[j].teardown(*world);
            }
        }

        successfully_created = true;
        return {};
    }

    void Dispatcher::update() {
        for (auto &i_system : systems)
                i_system.update(*world);
    }

    Dispatcher::~Dispatcher() {
        if (successfully_created) {
            std::for_each(systems.rbegin(), systems.rend(), [this](auto &i_system) {
                i_system.teardown(*world);
            });
            successfully_created = false;
        }
        for (auto &system : systems)
            system.destroy_and_delete();
    }
}