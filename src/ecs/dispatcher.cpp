#include "ecs/dispatcher.h"

namespace ecs {
    namespace detail {
        enum DfsNodeColor {
            black,
            gray,
            white,
        };

        static Dispatcher::DependencyError unsatisfied_dependency_error(
                std::string dependent_name, std::string dependency_name) {
            std::string message = dependent_name + " depends on " + dependency_name +
                                  " but such system is not present.";

            return Dispatcher::DependencyError{
                    std::move(dependent_name),
                    std::move(dependency_name),
                    std::move(message),
                    Dispatcher::DependencyError::unsatisfied
            };
        }

        static Dispatcher::DependencyError circular_dependency_error(
                std::string dependent_name, std::string dependency_name) {
            std::string message = dependent_name + " and " + dependency_name +
                                  " form circular dependency.";

            return Dispatcher::DependencyError{
                    std::move(dependent_name),
                    std::move(dependency_name),
                    std::move(message),
                    Dispatcher::DependencyError::circular
            };
        }

        static tl::expected<void, Dispatcher::DependencyError>
        topsort_systems_dfs(const ISystem &system, const ISystemsArray &systems_by_id,
                            ISystemsVector &topsort, std::vector<DfsNodeColor> &visited,
                            ISystemsBitset &systems_presence) {
            visited[system.id()] = gray;

            std::size_t deps_size = system.dependencies().size();
            for (std::size_t i = 0; i < deps_size; ++i) {
                uint32_t system_id = system.dependencies()[i];
                if (!systems_presence[system_id])
                    return tl::make_unexpected(unsatisfied_dependency_error(
                            system.name(),
                            system.dependencies_names()[i]));

                if (visited[system_id] == white) {
                    if (auto result = topsort_systems_dfs(systems_by_id[system_id], systems_by_id,
                                                          topsort, visited, systems_presence)) {}
                    else return tl::make_unexpected(std::move(result.error()));
                } else if (visited[system_id] == gray) {
                    return tl::make_unexpected(circular_dependency_error(
                            system.name(),
                            system.dependencies_names()[i]));
                }
            }
            topsort.push_back(system);

            visited[system.id()] = black;
            return {};
        }

        static tl::expected<ISystemsVector, Dispatcher::DependencyError>
        topsort_systems(const ISystemsVector &systems,
                        const ISystemsArray &systems_by_id,
                        ISystemsBitset &systems_presence) {
            ISystemsVector topsort;
            topsort.reserve(systems.size());
            std::vector<DfsNodeColor> visited(max_systems, white);

            for (const ISystem &system : systems)
                if (visited[system.id()] == white) {
                    if (auto result = topsort_systems_dfs(system, systems_by_id, topsort,
                                                          visited, systems_presence)) {}
                    else return tl::make_unexpected(std::move(result.error()));
                }

            return topsort;
        }
    }

    auto Dispatcher::setup() -> tl::expected<void, DispatcherError> {
        if (auto result = detail::topsort_systems(systems, systems_by_id, systems_presence))
            systems = std::move(result.value());
        else return tl::make_unexpected(result.error());

        for (std::size_t i = 0; i < systems.size(); ++i) {
            tl::expected<void, std::string> result;
            result = systems[i].setup(*world, systems_by_id);

            if (!result) {
                for (std::size_t j = 0; j < i; ++j)
                    systems[j].teardown(*world);
                return tl::make_unexpected(SystemError{
                    .failed_system = systems[i].name(),
                    .message = std::move(result.error())
                });
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

    World &Dispatcher::get_world() {
        return *world;
    }

    const World &Dispatcher::get_world() const {
        return *world;
    }

    tl::expected<Dispatcher, Dispatcher::DispatcherError> DispatcherBuilder::build() {
        if (auto result = dispatcher.setup())
            return std::move(dispatcher);
        else
            return tl::make_unexpected(result.error());
    }
}