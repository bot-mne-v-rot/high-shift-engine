#ifndef HIGH_SHIFT_ID_SET_H
#define HIGH_SHIFT_ID_SET_H

#include "ecs/entity.h"

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <iterator>

namespace ecs {
    template<class S>
    concept IdSetLike = requires(const S c, std::size_t lvl, std::size_t ind, Id id) {
        { c.contains(id) } -> std::same_as<bool>;
        { c.capacity() } -> std::same_as<typename S::size_type>;
        { c.level_data(lvl, ind) } -> std::same_as<uint64_t>;
        { c.level_capacity(lvl) } -> std::same_as<std::size_t>;
    };

    // Bitset that allows for fast scanning of set bits.
    class IdSet {
    public:
        using value_type = Id;
        using reference = Id &;
        using const_reference = Id;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;

        IdSet();

        class const_iterator;
        using iterator = const_iterator;

        void insert(Id id);
        void erase(Id id);
        iterator erase(const_iterator it);
        bool contains(Id id) const;

        std::size_t size() const;
        std::size_t capacity() const;
        bool empty() const;

        Id first() const;

        void reserve(std::size_t n);

        void clear();

        ~IdSet();


        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;

    public:
        static const std::size_t levels_num = 4;
        static const std::size_t bits_num = 64;
        static const std::size_t lower_bits = bits_num - 1;
        static const std::size_t shift = 6; // = log2(bits_num);
        constexpr static std::size_t shifts[levels_num] = {0, shift, shift * 2, shift * 3};

    private:
        uint64_t level3 = 0;
        uint64_t *levels[levels_num]{nullptr, nullptr, nullptr, &level3};
        std::size_t lvl_cp[levels_num]{};
        std::size_t cp = 0;
        std::size_t sz = 0;
    };

    static_assert(IdSetLike<IdSet>);

    template<IdSetLike S>
    class IdSetIterator {
    public:
        using reference = Id;
        using pointer = const Id *;
        using value_type = Id;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        IdSetIterator() = default;
        IdSetIterator(const IdSetIterator &) = default;
        IdSetIterator &operator=(const IdSetIterator &) = default;

        reference operator*() const;

        IdSetIterator &operator++();
        IdSetIterator operator++(int);

        bool operator==(const IdSetIterator &other) const;
        bool operator!=(const IdSetIterator &other) const;

        explicit IdSetIterator(const S *set, Id pos) : set(set), pos(pos) {
            if (pos == set->capacity())
                return;
            Id cur = pos;
            for (std::size_t i = 0; i < IdSet::levels_num; ++i) {
                uint32_t lower = (cur & IdSet::lower_bits);
                cur >>= IdSet::shift;
                levels_data[i] = (set->level_data(i, cur) & (~((1ull << lower) - 1)));
            }
        }

    private:
        Id pos = 0;
        const S *set = nullptr;
        uint64_t levels_data[IdSet::levels_num]{};
    };

    class IdSet::const_iterator : public IdSetIterator<IdSet> {
    public:
        const_iterator(const IdSet *set, Id pos) : IdSetIterator<IdSet>(set, pos) {}
    };

    template<IdSetLike A, IdSetLike B>
    class IdSetAnd {
    public:
        IdSetAnd(const A &a, const B &b) : a(a), b(b) {}

        bool contains(Id id) const;

        std::size_t capacity() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;

        class const_iterator;

        using iterator = const_iterator;

        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;
    private:
        const A &a;
        const B &b;
    };

    template<IdSetLike A, IdSetLike B>
    class IdSetAnd<A, B>::const_iterator : public IdSetIterator<IdSetAnd<A, B>> {
    public:
        const_iterator(const IdSetAnd<A, B> *set, Id pos) : IdSetIterator<IdSetAnd<A, B>>(set, pos) {}
    };
}

// Implementation should be visible to make inlining possible.
#include "ecs/detail/id_set_impl.h"

#endif //HIGH_SHIFT_ID_SET_H
