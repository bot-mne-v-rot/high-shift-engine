#ifndef HIGH_SHIFT_UTILS_H
#define HIGH_SHIFT_UTILS_H

#include <tuple>
#include <type_traits>

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

    template<class M, class Key>
    typename M::mapped_type &
    get_else_update(M &m, Key const &k, typename M::mapped_type const &v) {
        return m.insert(typename M::value_type(k, v)).first->second;
    }

    template <typename, typename> struct Cons;

    template <typename  T, typename... Args>
    struct Cons<T, std::tuple<Args...>> {
        using type = std::tuple<T, Args...>;
    };

    template <template<typename> class Predicate, typename...> struct filter;
    template <template<typename> class Predicate> struct filter<Predicate> { using type = std::tuple<>; };

    template <template<typename> class Predicate, typename Head, typename... Tail>
    struct filter<Predicate, Head, Tail...> {
        using type = typename std::conditional<Predicate<Head>::value,
            typename Cons<Head, typename filter<Predicate, Tail...>::type>::type,
            typename filter<Predicate, Tail...>::type
        >::type;
    };
}

#endif //HIGH_SHIFT_UTILS_H
