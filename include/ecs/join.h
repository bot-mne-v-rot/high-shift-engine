#ifndef HIGH_SHIFT_JOIN_H
#define HIGH_SHIFT_JOIN_H

#include "ecs/storage.h"

//#include <ranges>

namespace ecs {
    namespace detail {
        template<typename S>
        requires(Storage<std::remove_const_t<S>>)
        using ComponentRef =
        typename std::iterator_traits<decltype(std::declval<S>().begin())>::reference;

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        using JoinedMask =
        decltype((std::declval<Storages>().present() & ...));

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        using JoinedMaskIterator =
        typename JoinedMask<Storages...>::iterator;

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
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
            pointer operator->() const;

            JoinIterator &operator++();
            JoinIterator operator++(int);

            bool operator==(const JoinIterator &other) const = default;
            bool operator!=(const JoinIterator &other) const = default;

            explicit JoinIterator(Storages *...storage_ptrs,
                                  detail::JoinedMaskIterator<Storages...> mask_iterator)
                    : storages(storage_ptrs...), mask_iterator(std::move(mask_iterator)) {}

        private:
            std::tuple<Storages *...> storages;
            detail::JoinedMaskIterator<Storages...> mask_iterator;
        };

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        class JoinRange {
        public:
            using const_iterator = JoinIterator<Storages...>;
            using iterator = JoinIterator<Storages...>;
            using Mask = JoinedMask<Storages...>;

            JoinRange(iterator begin, iterator end, std::unique_ptr<Mask> keep_alive)
                    : b(begin), e(end), keep_alive(std::move(keep_alive)) {}

            iterator begin() const;
            iterator end() const;
            const_iterator cbegin() const;
            const_iterator cend() const;
        private:
            iterator b, e;
            [[maybe_unused]] std::unique_ptr<Mask> keep_alive; // Iterator are invalidated if Mask is relocated
        };
    }

    template<typename ...Storages>
    requires(Storage<std::remove_const_t<Storages>> && ...)
    inline detail::JoinRange<Storages...> join(Storages &...storages);
}

#include "ecs/detail/join_impl.h"

#endif //HIGH_SHIFT_JOIN_H
