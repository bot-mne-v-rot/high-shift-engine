#ifndef HIGH_SHIFT_SYSTEM_H
#define HIGH_SHIFT_SYSTEM_H

#include "ecs/component.h"
#include "ecs/world.h"

#include <concepts>

namespace ecs {
    namespace detail {
        template<typename>
        struct all_params_are_lvalue_refs : public std::false_type {
        };

        template<typename Ret, typename This, typename ...Args>
        struct all_params_are_lvalue_refs<Ret (This::*)(Args...)>
                : public std::bool_constant<(... && std::is_lvalue_reference_v<Args>)> {
        };

        template<typename F>
        constexpr bool all_params_are_lvalue_refs_v = all_params_are_lvalue_refs<F>::value;
    }

    template<class S>
    concept System = requires(S sys) {
        requires std::is_default_constructible_v<S>;
        &S::update; // has update method
        requires detail::all_params_are_lvalue_refs_v<decltype(&S::update)>;
    };
}

#endif //HIGH_SHIFT_SYSTEM_H
