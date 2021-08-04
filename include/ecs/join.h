#ifndef HIGH_SHIFT_JOIN_H
#define HIGH_SHIFT_JOIN_H

#include <memory>
#include "ecs/storage.h"

//#include <ranges>

namespace ecs {
    namespace detail {
        template<typename S> requires(Storage<std::remove_const_t<S>>)
        using ComponentRef =
        decltype(std::declval<S>()[std::declval<Id>()]);

        template<typename ...Storages> requires(Storage<std::remove_const_t<Storages>> &&...)
        using JoinedMask =
        decltype((std::declval<Storages>().present() & ...));

        template<typename ...Storages> requires(Storage<std::remove_const_t<Storages>> &&...)
        using JoinedMaskIterator =
        typename JoinedMask<Storages...>::iterator;

        template<typename ...Storages> requires(Storage<std::remove_const_t<Storages>> &&...)
        class JoinIterator {
        public:
            using value_type = std::tuple<detail::ComponentRef<Storages>...>;
            using reference = value_type;
            using pointer = value_type *;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            JoinIterator() = default;
            JoinIterator(const JoinIterator &) = default;
            JoinIterator &operator=(const JoinIterator &) = default;

            reference operator*() const;

            JoinIterator &operator++();
            JoinIterator operator++(int);

            bool operator==(const JoinIterator &other) const = default;
            bool operator!=(const JoinIterator &other) const = default;

            ecs::Id id() const;

            explicit JoinIterator(Storages *...storage_ptrs,
                                  detail::JoinedMaskIterator<Storages...> mask_iterator)
                    : storages(storage_ptrs...), mask_iterator(std::move(mask_iterator)) {}

        protected:
            std::tuple<Storages *...> storages;
            detail::JoinedMaskIterator<Storages...> mask_iterator;
        };

        template<typename ...Storages> requires(Storage<std::remove_const_t<Storages>> &&...)
        class JoinWithIdIterator {
        public:
            using value_type = std::tuple<Id, detail::ComponentRef<Storages>...>;
            using reference = value_type;
            using pointer = value_type *;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            JoinWithIdIterator() = default;

            reference operator*() const;

            JoinWithIdIterator &operator++();
            JoinWithIdIterator operator++(int);

            bool operator==(const JoinWithIdIterator &other) const = default;
            bool operator!=(const JoinWithIdIterator &other) const = default;

            ecs::Id id() const;

            explicit JoinWithIdIterator(JoinIterator<Storages...> iter) : iter(iter) {}

        protected:
            JoinIterator<Storages...> iter;
        };

        template<typename JoinIt, typename ...KeepAlive>
        class JoinView {
        public:
            using const_iterator = JoinIt;
            using iterator = JoinIt;

            JoinView(iterator begin, iterator end, std::unique_ptr<KeepAlive> ...keep_alive)
                    : b(std::move(begin)), e(std::move(end)), keep_alive(std::move(keep_alive)...) {}

            iterator begin() const;
            iterator end() const;
            const_iterator cbegin() const;
            const_iterator cend() const;
        private:
            iterator b, e;
            [[maybe_unused]] std::tuple<std::unique_ptr<KeepAlive>...> keep_alive;
        };
    }

    template<typename ...Storages> requires(Storage<std::remove_const_t<Storages>> &&...)
    using JoinView = detail::JoinView<detail::JoinIterator<Storages...>, detail::JoinedMask<Storages...>>;

    template<typename ...Storages> requires(Storage<std::remove_const_t<Storages>> &&...)
    using JoinWithIdView = detail::JoinView<detail::JoinWithIdIterator<Storages...>, detail::JoinedMask<Storages...>>;

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
