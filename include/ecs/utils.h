#ifndef HIGH_SHIFT_UTILS_H
#define HIGH_SHIFT_UTILS_H

#include <tuple>

namespace ecs::detail {
    template<typename F, std::size_t I = 0, typename... Ts>
    constexpr void tuple_for_each(std::tuple<Ts...> &tup, F &&f) {
        if constexpr(I < sizeof...(Ts)) {
            f(get<I>(tup));
            tuple_for_each<F, I + 1>(tup, std::forward<F>(f));
        } else {
            return;
        }
    }

    template<typename F, std::size_t N, std::size_t I = 0, typename... Ts>
    constexpr void tuple_for_each_n(std::tuple<Ts...> &tup, F &&f) {
        if constexpr(I < N) {
            f(get<I>(tup));
            tuple_for_each<F, N, I + 1>(tup, std::forward<F>(f));
        } else {
            return;
        }
    }

    template<typename Fn, typename FnFail, std::size_t I = 0, typename... Ts>
    constexpr void tuple_for_each_branching(std::tuple<Ts...> &tup, Fn &&f, FnFail &&fail) {
        if constexpr(I < sizeof...(Ts)) {
            if (f(get<I>(tup)))
                tuple_for_each_branching<Fn, FnFail, I + 1>(
                        tup, std::forward<Fn>(f), std::forward<FnFail>(fail));
            else
                tuple_for_each_n<FnFail, I>(tup, std::forward<FnFail>(fail));
        }
    }
}

#endif //HIGH_SHIFT_UTILS_H
