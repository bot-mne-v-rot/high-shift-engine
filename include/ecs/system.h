#ifndef HIGH_SHIFT_SYSTEM_H
#define HIGH_SHIFT_SYSTEM_H

#include "ecs/component.h"
#include "ecs/world.h"
#include "ecs/utils.h"

#include <expected.h>
#include <concepts>

namespace ecs {
    namespace detail {
        template<typename>
        struct all_params_are_lvalue_refs_to_resources : public std::false_type {
        };

        template<typename Ret, typename This, typename ...Args>
        struct all_params_are_lvalue_refs_to_resources<Ret (This::*)(Args...)>
                : public std::bool_constant<(std::is_lvalue_reference_v<Args> && ...) &&
                                            (Resource<std::remove_cvref_t<Args>> && ...)> {
        };

        template<typename F>
        constexpr bool all_params_are_lvalue_refs_to_resources_v =
                all_params_are_lvalue_refs_to_resources<F>::value;
    }
    /**
     * The only thing system should provide is `update` method.
     * You can list any resources you wish to access by either
     * mutable or immutable references (lvalue ref or const lvalue ref).
     *
     * @example ```
     * class ExampleSystem {
     * public:
     *     void update(const SomeComponent::Storage &read_some, OtherComponent::Storage &write_other,
     *                 OtherResource &write_resource, const SomeResource &read_resource) {
     *         ...
     *     }
     * };
     * ```
     *
     * Check test/test_system.cpp for the full example.
     */
    template<class S>
    concept System = requires(S sys) {
        requires std::is_default_constructible_v<S>;
        &S::update; // has update method
        requires detail::all_params_are_lvalue_refs_to_resources_v<decltype(&S::update)>;
    };

    /**
     * Optionally System can provide setup method that
     * accepts reference to World.
     *
     * It would be called by Dispatcher during setup routine
     * and before automatic resources' creation.
     *
     * The setup may fail and return tl::unexpected.
     */
    template<class S>
    concept SystemHasSetup = requires(S sys, World &world) {
        { sys.setup(world) } -> std::same_as<tl::expected<void, std::string>>;
    };

    /**
     * Optionally System can provide teardown method that
     * accepts reference to World.
     *
     * It would be called by Dispatcher during teardown routine
     * and before System's destructor.
     */
    template<class S>
    concept SystemHasTeardown = requires(S sys, World &world) {
        sys.teardown(world);
    };
}

#endif //HIGH_SHIFT_SYSTEM_H
