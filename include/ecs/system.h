#ifndef HIGH_SHIFT_SYSTEM_H
#define HIGH_SHIFT_SYSTEM_H

#include "ecs/component.h"
#include "ecs/world.h"
#include "ecs/utils.h"

#include <concepts>

namespace ecs {
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
        requires detail::all_params_are_lvalue_refs_v<decltype(&S::update)>;
    };
}

#endif //HIGH_SHIFT_SYSTEM_H
