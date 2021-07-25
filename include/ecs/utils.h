#ifndef HIGH_SHIFT_UTILS_H
#define HIGH_SHIFT_UTILS_H

#include <tuple>

namespace ecs::detail {
    template<typename F, std::size_t I = 0, typename... Ts>
    constexpr void tuple_for_each(std::tuple<Ts...> tup, F f) {
        if constexpr(I < sizeof...(Ts)) {
            f(get<I>(tup));
            tuple_for_each<F, I + 1>(tup, std::move(f));
        } else {
            return;
        }
    }

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

#endif //HIGH_SHIFT_UTILS_H
