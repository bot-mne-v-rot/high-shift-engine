#ifndef HIGH_SHIFT_INVERTED_STORAGE_H
#define HIGH_SHIFT_INVERTED_STORAGE_H

#include "ecs/storage.h"
#include "ecs/id_set.h"

namespace ecs {
    struct EmptyComponent {};

    template<Storage S>
    class InvertedStorage {
    public:
        using IdSetType = IdSetNot<std::remove_cvref_t<decltype(std::declval<S>().present())>>;

        using value_type = EmptyComponent;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;

        explicit InvertedStorage(const S &storage) : storage(storage), mask(~storage.present()) {}

        std::size_t size() const;
        std::size_t capacity() const;
        bool empty() const;

        bool contains(Id id) const;

        reference operator[](Id index);
        const_reference operator[](Id index) const;

        const IdSetType &present() const;

        template<typename Ref, typename Ptr>
        class iterator_template {
        public:
            using reference = Ref;
            using pointer = Ptr;
            using value_type = EmptyComponent;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            iterator_template() = default;
            iterator_template(const iterator_template &) = default;
            iterator_template &operator=(const iterator_template &) = default;

            reference operator*() const;
            pointer operator->() const;

            iterator_template &operator++();
            iterator_template operator++(int);

            bool operator==(const iterator_template &other) const = default;
            bool operator!=(const iterator_template &other) const = default;

            ecs::Id id() const;

        private:
            using MaskIterator = typename IdSetType::const_iterator;
            MaskIterator mask_iterator;
            EmptyComponent dummy;

            friend InvertedStorage;
            explicit iterator_template(MaskIterator mask_iterator);
        };

        using iterator = iterator_template<reference, pointer>;
        using const_iterator = iterator_template<const_reference, const_pointer>;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

        WithIdView<iterator, const_iterator> with_id();
        WithIdView<const_iterator, const_iterator> with_id() const;

    private:
        IdSetType mask;
        const S &storage;
        EmptyComponent dummy;
    };

    template<Storage S>
    auto operator!(const S &storage) {
        return InvertedStorage<S>(storage);
    }
}

#include "ecs/storages/detail/inverted_storage_impl.h"

#endif //HIGH_SHIFT_INVERTED_STORAGE_H
