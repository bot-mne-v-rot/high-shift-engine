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

    namespace detail {
        template<typename>
        struct is_system_setup_signature : public std::false_type {
        };

        template<typename Ret, typename This, typename ...Args>
        struct is_system_setup_signature<Ret (This::*)(ecs::World &, Args...)>
                : public std::bool_constant<(std::is_lvalue_reference_v<Args> && ...) &&
                (System<std::remove_cvref_t<Args>> && ...) &&
                (std::is_same_v<Ret, tl::expected<void, std::string>> ||
                std::is_same_v<Ret, void>)> {
                };

        template<typename F>
        constexpr bool is_system_setup_signature_v =
                is_system_setup_signature<F>::value;

        template<typename>
        struct return_type_is_tl_unexpected : public std::false_type {
        };

        template<typename Ret, typename This, typename ...Args>
        struct return_type_is_tl_unexpected<Ret (This::*)(Args...)>
                : public std::is_same<Ret, tl::expected<void, std::string>> {
                };

        template<typename F>
        constexpr bool return_type_is_tl_unexpected_v =
                return_type_is_tl_unexpected<F>::value;
    }

    /**
     * Optionally System can provide setup method that
     * accepts reference to World.
     *
     * It would be called by Dispatcher during setup routine
     * and before automatic resources' creation.
     *
     * Signature must be Ret setup(ecs::World &, S1 &, S2 &, ...)
     * S1, S2, ... is either ecs::System or const ecs::System.
     * Number of systems is arbitrary.
     *
     * Ret is either void or tl::expected<void, std::string>.
     */
    template<class S>
    concept SystemHasSetup = requires {
        &S::setup;
        requires detail::is_system_setup_signature_v<decltype(&S::setup)>;
    };

    template<class S>
    concept SystemSetupMayFail = requires {
        requires detail::return_type_is_tl_unexpected_v<decltype(&S::setup)>;
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
