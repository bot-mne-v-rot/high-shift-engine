#ifndef HIGH_SHIFT_ID_SET_H
#define HIGH_SHIFT_ID_SET_H

#include "ecs/entity.h"

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <vector>

namespace ecs {
    template<class S>
    concept IdSetLike = requires(const S c, std::size_t lvl, std::size_t ind, Id id) {
        { c.contains(id) } -> std::same_as<bool>;
        { c.capacity() } -> std::same_as<std::size_t>;
        { c.level_data(lvl, ind) } -> std::same_as<uint64_t>;
        { c.level_capacity(lvl) } -> std::same_as<std::size_t>;
        { c.first() } -> std::same_as<Id>;
        { c.empty() } -> std::same_as<bool>;
    };

    /**
     * Bitset that allows for fast scanning of set bits.
     *
     * Layout example, with an imaginary 4-bit uint64_t:
     *
     * Layer 3: 1------------------------------------------------ ...
     * Layer 2: 1------------------ 1------------------ 0-------- ...
     * Layer 1: 1--- 0--- 0--- 0--- 1--- 0--- 1--- 0--- 0--- 0--- ...
     * Layer 0: 0010 0000 0000 0000 0011 0000 1111 0000 0000 0000 ...
     */
    class IdSet {
    public:
        IdSet();

        IdSet(const IdSet &);
        IdSet &operator=(const IdSet &);

        IdSet(IdSet &&);
        IdSet &operator=(IdSet &&);

        void insert(Id id);
        bool erase(Id id); // true if element was present
        bool contains(Id id) const;

        /**
         * Unlike usual insert does not perform
         * memory allocation and boundaries check.
         * Does not check if id is present.
         */
        void insert_unsafe(Id id);

        std::size_t size() const;
        std::size_t capacity() const;
        bool empty() const;

        Id first() const;

        void reserve(std::size_t n);

        void clear();

        void swap(IdSet &other);

        ~IdSet();

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;

    public:
        static const std::size_t levels_num = 4;
        static const std::size_t bits_num = 64;
        static const std::size_t lower_bits = bits_num - 1;
        static const std::size_t shift = 6; // = log2(bits_num);
        constexpr static std::size_t shifts[levels_num] = {0, shift, shift * 2, shift * 3};

        static const std::size_t max_size = (1 << (shift * levels_num));
        // pow(bits_num, levels_num) = pow(2, log2(bits_num) * levels_num);

    private:
        uint64_t level3 = 0; // level3 is preallocated and hardcoded to never grow
        uint64_t *levels[levels_num]{nullptr, nullptr, nullptr, &level3};
        std::size_t lvl_cp[levels_num]{0, 0, 0, 1};
        std::size_t cp = 0;
        std::size_t sz = 0;
    };

    static_assert(IdSetLike<IdSet>);

    /**
     * Virtual Id Set that represents intersection
     * of two Id Sets (bitwise AND of the bitmasks).
     *
     * @note It's effectively bitwise AND of each level.
     * Therefore set bit on the upper level does not
     * guarantee that any bit on the lower level is set.
     * That requires more complex traversal algorithm and
     * results in lower performance in some cases.
     *
     * Consider following example:
     * A:                      B:
     * Layer 1: 1--- 1--- ...  Layer 1: 1--- 1--- ...
     * Layer 0: 1010 1010 ...  Layer 0: 0101 0101 ...
     *
     * A && B:
     * Layer 1: 1--- 1--- ...
     * Layer 0: 0000 0000 ...
     *
     */
    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    class IdSetAnd {
    public:
        // Type deduction of the universal references is performed in the & operator
        IdSetAnd(A &&a, B &&b) : a(std::forward<A>(a)), b(std::forward<B>(b)) {}

        bool contains(Id id) const;
        bool empty() const;
        Id first() const;

        std::size_t capacity() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;

    private:
        A a;
        B b;
    };

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline IdSetAnd<A, B> operator&(A &&a, B &&b) {
        return IdSetAnd<A, B>(std::forward<A>(a), std::forward<B>(b));
    }

    /**
     * Virtual Id Set that represents intersection
     * of two Id Sets (bitwise AND of the bitmasks).
     *
     * Unlike IdSetAnd its' type is not modified
     * by &'ed sets.
     */
    class DynamicIdSetAnd {
    public:
        explicit DynamicIdSetAnd(std::vector<const IdSet *> sets) : sets(std::move(sets)) {}

        bool contains(Id id) const;
        bool empty() const;
        Id first() const;

        std::size_t capacity() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;

    private:
        std::vector<const IdSet *> sets;
    };

    /**
     * Virtual Id Set that represents union
     * of two Id Sets (bitwise OR of the bitmasks).
     *
     * @note It's effectively bitwise OR of each level.
     */
    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    class IdSetOr {
    public:
        // Type deduction of the universal references is performed in the | operator
        IdSetOr(A &&a, B &&b) : a(std::forward<A>(a)), b(std::forward<B>(b)) {}

        bool contains(Id id) const;
        bool empty() const;
        Id first() const;

        std::size_t capacity() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;

    private:
        A a;
        B b;
    };

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline IdSetOr<A, B> operator|(A &&a, B &&b) {
        return IdSetOr<A, B>(std::forward<A>(a), std::forward<B>(b));
    }

    /**
     * Virtual Id Set that represents Id Set
     * complement (bitwise NOT of the bitmask).
     *
     * As soon as any IdSet is finite (except FullIdSet)
     * it's complement is infinite. The implementation
     * follows the idea of infinite capacity.
     *
     * @note It's effectively bitwise NOT of the bottom level
     * while others are constant 1s.
     */
    template<typename S>
    requires IdSetLike<std::remove_cvref_t<S>>
    class IdSetNot {
    public:
        // Type deduction of the universal reference is performed in the | operator
        explicit IdSetNot(S &&set) : set(std::forward<S>(set)) {}

        bool contains(Id id) const;
        bool empty() const;
        Id first() const;

        std::size_t capacity() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;

    private:
        S set;
    };

    template<typename S>
    requires IdSetLike<std::remove_cvref_t<S>>
    inline IdSetNot<S> operator~(S &&set) {
        return IdSetNot<S>(std::forward<S>(set));
    }

    /**
     * Virtual Id Set that represents full Id Set
     * (all Ids are present).
     *
     * @note It just returns 1s.
     * @note It has infinite capacity.
     */
    class FullIdSet {
    public:
        bool contains(Id id) const;
        bool empty() const;
        Id first() const;

        std::size_t capacity() const;
        std::size_t size() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;
    };

    static_assert(IdSetLike<FullIdSet>);

    /**
     * Virtual Id Set that represents empty Id Set
     * (no Id is present).
     *
     * @note It just returns 0s.
     */
    class EmptyIdSet {
    public:
        bool contains(Id id) const;
        bool empty() const;
        Id first() const;

        std::size_t capacity() const;
        std::size_t size() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;
    };

    static_assert(IdSetLike<EmptyIdSet>);

    /**
     * Iterates over set and calls f with each id.
     */
    template<IdSetLike Set, typename Fn>
    void foreach(const Set &set, Fn &&f);

    /**
     * @param f predicate.
     * @return the first matched id if any.
     * @return IdSet::max_size, otherwise.
     */
    template<IdSetLike Set, typename Fn>
    Id find_if(const Set &set, Fn &&f);
}

namespace std {
    inline void swap(ecs::IdSet &a, ecs::IdSet &b) {
        a.swap(b);
    }
}

// Implementation should be visible to make inlining possible.
#include "ecs/detail/id_set_impl.h"

#endif //HIGH_SHIFT_ID_SET_H
