#ifndef HIGH_SHIFT_JOIN_H
#define HIGH_SHIFT_JOIN_H

#include <memory>
#include "ecs/storage.h"

//#include <ranges>

namespace ecs {
    namespace detail {
        template<typename S>
        requires(Storage<std::remove_const_t<S>>)
        using ComponentRef =
                decltype(std::declval<S>()[std::declval<Id>()]);

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> &&...)
        using JoinedMask =
                decltype((std::declval<Storages>().present() & ...));
    }

    // Umm, well, workflow with non-terminating variadic templates is kinda tough...

    template<typename A, typename B, typename Fn>
    inline void joined_foreach(A &a, B &b, Fn &&f);

    template<typename A, typename B, typename C, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, Fn &&f);

    template<typename A, typename B, typename C, typename D, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, D &d, Fn &&f);

    template<typename A, typename B, typename C, typename D, typename E, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, D &d, E &e, Fn &&f);

    template<typename A, typename B, typename C, typename D, typename E, typename F, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, D &d, E &e, F &f, Fn &&fn);

    template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, D &d, E &e, F &f, G &g, Fn &&fn);

    template<typename A, typename B, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, Fn &&f);

    template<typename A, typename B, typename C, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, Fn &&f);

    template<typename A, typename B, typename C, typename D, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, D &d, Fn &&f);

    template<typename A, typename B, typename C, typename D, typename E, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, D &d, E &e, Fn &&f);

    template<typename A, typename B, typename C, typename D, typename E, typename F, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, D &d, E &e, F &f, Fn &&fn);

    template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, D &d, E &e, F &f, G &g, Fn &&fn);
}

#include "ecs/detail/join_impl.h"

#endif //HIGH_SHIFT_JOIN_H
