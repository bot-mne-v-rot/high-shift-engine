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
    template<IdSetLike A, IdSetLike B>
    class IdSetAnd {
    public:
        IdSetAnd(const A &a, const B &b) : a(a), b(b) {}

        bool contains(Id id) const;
        bool empty() const;
        Id first() const;

        std::size_t capacity() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;
    private:
        const A &a;
        const B &b;
    };

    template<IdSetLike A, IdSetLike B>
    inline IdSetAnd<A, B> operator&(const A &a, const B &b) {
        return IdSetAnd<A, B>(a, b);
    }

    /**
     * Virtual Id Set that represents union
     * of two Id Sets (bitwise OR of the bitmasks).
     *
     * @note It's effectively bitwise OR of each level.
     */
    template<IdSetLike A, IdSetLike B>
    class IdSetOr {
    public:
        IdSetOr(const A &a, const B &b) : a(a), b(b) {}

        bool contains(Id id) const;
        bool empty() const;
        Id first() const;

        std::size_t capacity() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;

    private:
        const A &a;
        const B &b;
    };

    template<IdSetLike A, IdSetLike B>
    inline IdSetOr<A, B> operator|(const A &a, const B &b) {
        return IdSetOr<A, B>(a, b);
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
    template<IdSetLike S>
    class IdSetNot {
    public:
        explicit IdSetNot(const S &set) : set(set) {}

        bool contains(Id id) const;
        bool empty() const;
        Id first() const;

        std::size_t capacity() const;

        uint64_t level_data(std::size_t lvl, std::size_t ind) const;
        std::size_t level_capacity(std::size_t lvl) const;

    private:
        const S &set;
    };

    template<IdSetLike S>
    inline IdSetNot<S> operator~(const S &set) {
        return IdSetNot<S>(set);
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

    template<IdSetLike Set, typename Fn>
    void foreach(const Set &set, Fn &&f);
}

namespace std {
    inline void swap(ecs::IdSet &a, ecs::IdSet &b) {
        a.swap(b);
    }
}

// Implementation should be visible to make inlining possible.
#include "ecs/detail/id_set_impl.h"

#endif //HIGH_SHIFT_ID_SET_H
