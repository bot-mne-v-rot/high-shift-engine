namespace ecs {
    namespace detail {
        template<typename ...Storages, typename Fn>
        requires(Storage<std::remove_const_t<Storages>> &&...)
        inline void joined_foreach_impl(Storages &...storages, Fn &&f) {
            auto joined_mask = (storages.present() & ...);
            foreach(joined_mask, [&](Id id) {
                f(storages[id]...);
            });
        }

        template<typename ...Storages, typename Fn>
        requires(Storage<std::remove_const_t<Storages>> &&...)
        inline void joined_foreach_with_id_impl(Storages &...storages, Fn &&f) {
            auto joined_mask = (storages.present() & ...);
            foreach(joined_mask, [&](Id id) {
                f(id, storages[id]...);
            });
        }
    }

    template<typename A, typename B, typename Fn>
    inline void joined_foreach(A &a, B &b, Fn &&f) {
        detail::joined_foreach_impl<A, B>(a, b, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, Fn &&f) {
        detail::joined_foreach_impl<A, B, C>(a, b, c, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename D, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, D &d, Fn &&f) {
        detail::joined_foreach_impl<A, B, C, D>(a, b, c, d, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename D, typename E, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, D &d, E &e, Fn &&f) {
        detail::joined_foreach_impl<A, B, C, D, E>(a, b, c, d, e, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename D, typename E, typename F, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, D &d, E &e, F &f, Fn &&fn) {
        detail::joined_foreach_impl<A, B, C, D, E, F>(a, b, c, d, e, f, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename Fn>
    inline void joined_foreach(A &a, B &b, C &c, D &d, E &e, F &f, G &g, Fn &&fn) {
        detail::joined_foreach_impl<A, B, C, D, E, F, G>(a, b, c, d, e, f, g, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, Fn &&f) {
        detail::joined_foreach_with_id_impl<A, B>(a, b, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, Fn &&f) {
        detail::joined_foreach_with_id_impl<A, B, C>(a, b, c, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename D, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, D &d, Fn &&f) {
        detail::joined_foreach_with_id_impl<A, B, C, D>(a, b, c, d, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename D, typename E, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, D &d, E &e, Fn &&f) {
        detail::joined_foreach_with_id_impl<A, B, C, D, E>(a, b, c, d, e, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename D, typename E, typename F, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, D &d, E &e, F &f, Fn &&fn) {
        detail::joined_foreach_with_id_impl<A, B, C, D, E, F>(a, b, c, d, e, f, std::forward<Fn>(f));
    }

    template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename Fn>
    inline void joined_foreach_with_id(A &a, B &b, C &c, D &d, E &e, F &f, G &g, Fn &&fn) {
        detail::joined_foreach_with_id_impl<A, B, C, D, E, F, G>(a, b, c, d, e, f, g, std::forward<Fn>(f));
    }
}