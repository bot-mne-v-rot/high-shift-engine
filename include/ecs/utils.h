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
}

#endif //HIGH_SHIFT_UTILS_H
