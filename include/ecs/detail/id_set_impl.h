#include <algorithm>
#include <memory.h>
#include <cassert>
#include <limits>

namespace ecs {
    namespace detail {
        inline void reset_bit(uint64_t &mask, uint32_t pos) {
            mask &= ~(1ull << pos);
        }

        inline void set_bit(uint64_t &mask, uint32_t pos) {
            mask |= (1ull << pos);
        }

        inline void place_bit(uint64_t &mask, uint32_t pos, uint64_t bit) {
            reset_bit(mask, pos);
            mask |= (bit << pos);
        }

        inline bool check_bit(uint64_t mask, uint32_t pos) {
            return (mask >> pos) & 1;
        }

        inline void zero_out_lower(uint64_t &mask, uint32_t num) {
            mask &= (~((1 << (num)) - 1));
        }

        inline void zero_out_higher(uint64_t &mask, uint32_t num) {
            mask &= ((1 << (num)) - 1);
        }

        template<IdSetLike S>
        inline Id first(const S &set) {
            uint64_t levels_data[IdSet::levels_num];
            levels_data[IdSet::levels_num - 1] = set.level_data(IdSet::levels_num - 1, 0);

            Id pos = 0;
            for (std::size_t i = IdSet::levels_num - 1; i < IdSet::levels_num;) {
                if (levels_data[i] == 0) { // should go up
                    if (i == IdSet::levels_num - 1)
                        return set.capacity(); // nowhere to go up
                    pos >>= IdSet::shift; // move pos back
                    ++i; // go up
                    levels_data[i] &= (levels_data[i] - 1); // erase the false-positive bit
                    continue;
                }

                uint64_t trailing = __builtin_ctzll(levels_data[i]);
                pos <<= IdSet::shift;
                pos |= trailing;

                if (i == 0) break;
                --i; // go down
                levels_data[i] = set.level_data(i, pos);
            }
            return pos;
        }
    }

    ////////////////// IdSet

    inline IdSet::IdSet() = default;

    inline IdSet::IdSet(const IdSet &other) {
        *this = other;
    }

    inline IdSet &IdSet::operator=(const IdSet &other) {
        if (this == &other)
            return *this;

        reserve(other.capacity());
        level3 = other.level3;
        for (std::size_t lvl = 0; lvl + 1 < levels_num; ++lvl)
            if (other.levels[lvl])
                memcpy(levels[lvl], other.levels[lvl], lvl_cp[lvl] * sizeof(uint64_t));
        sz = other.sz;

        return *this;
    }

    inline IdSet::IdSet(IdSet &&other) {
        swap(other);
    }

    inline IdSet &IdSet::operator=(IdSet &&other) {
        swap(other);
        return *this;
    }

    inline void IdSet::swap(IdSet &other) {
        std::swap(level3, other.level3);
        for (std::size_t i = 0; i + 1 < levels_num; ++i) {
            std::swap(levels[i], other.levels[i]);
            std::swap(lvl_cp[i], other.lvl_cp[i]);
        }
        std::swap(cp, other.cp);
        std::swap(sz, other.sz);
    }

    inline void IdSet::insert(Id id) {
        reserve(id + 1);
        if (!contains(id))
            ++sz;

        for (auto &level : levels) {
            uint32_t pos = id & lower_bits;
            id >>= shift;
            detail::set_bit(level[id], pos);
        }
    }

    inline bool IdSet::erase(Id id) {
        if (!contains(id))
            return false;
        --sz;

        uint32_t pos = id & lower_bits;
        id >>= shift;
        detail::reset_bit(levels[0][id], pos);

        for (std::size_t i = 1; i < levels_num; ++i) {
            uint64_t bit = (levels[i - 1][id] != 0);

            pos = id & lower_bits;
            id >>= shift;
            detail::place_bit(levels[i][id], pos, bit);
        }
        return true;
    }

    inline IdSet::iterator IdSet::erase(const_iterator it) {
        erase(*it++);
        return it;
    }

    inline void IdSet::clear() {
        auto e = end();
        for (auto it = begin(); it != e; it = erase(it));
    }

    inline bool IdSet::contains(Id id) const {
        if (id >= cp)
            return false;
        return detail::check_bit(levels[0][id >> shift], id & lower_bits);
    }

    inline void IdSet::reserve(std::size_t n) {
        if (n < cp) return;

        std::size_t new_cp = 1;
        while (new_cp < n) new_cp <<= 1;
        cp = new_cp;

        assert(new_cp <= max_size);

        for (std::size_t i = 0; i + 1 < levels_num; ++i) {
            auto &level = levels[i];
            new_cp >>= shift;

            std::size_t new_level_cp = std::max<std::size_t>(1ull, new_cp);
            std::size_t old_level_cp = lvl_cp[i];
            lvl_cp[i] = new_level_cp;

            if (old_level_cp != new_level_cp) {
                auto new_level = new uint64_t[new_level_cp]{};
                if (level) memcpy(new_level, level, old_level_cp * sizeof(uint64_t));
                delete[] level;
                level = new_level;
            }
        }
    }

    inline std::size_t IdSet::size() const {
        return sz;
    }

    inline std::size_t IdSet::capacity() const {
        return cp;
    }

    inline bool IdSet::empty() const {
        return sz == 0;
    }

    inline uint64_t IdSet::level_data(std::size_t lvl, std::size_t ind) const {
        return levels[lvl][ind];
    }

    inline std::size_t IdSet::level_capacity(std::size_t lvl) const {
        return lvl_cp[lvl];
    }

    inline IdSet::~IdSet() {
        for (std::size_t i = 0; i + 1 < levels_num; ++i)
            delete[] levels[i];
    }

    inline Id IdSet::first() const {
        Id pos = 0;
        for (std::size_t i = levels_num; i--;) {
            uint64_t trailing = __builtin_ctzll(levels[i][pos]);
            pos <<= IdSet::shift;
            pos |= trailing;
        }
        return pos;
    }

    inline IdSet::const_iterator IdSet::begin() const {
        return cbegin();
    }

    inline IdSet::const_iterator IdSet::end() const {
        return cend();
    }

    inline IdSet::const_iterator IdSet::cbegin() const {
        if (empty()) return cend();
        return IdSet::const_iterator(this, first());
    }

    inline IdSet::const_iterator IdSet::cend() const {
        return IdSet::const_iterator(this, capacity());
    }

    ////////////////// IdSetAnd

    template<IdSetLike A, IdSetLike B>
    inline bool IdSetAnd<A, B>::contains(Id id) const {
        return a.contains(id) && b.contains(id);
    }

    template<IdSetLike A, IdSetLike B>
    inline uint64_t IdSetAnd<A, B>::level_data(std::size_t lvl, std::size_t ind) const {
        return a.level_data(lvl, ind) & b.level_data(lvl, ind);
    }

    template<IdSetLike A, IdSetLike B>
    inline std::size_t IdSetAnd<A, B>::level_capacity(std::size_t lvl) const {
        return std::min(a.level_capacity(lvl), b.level_capacity(lvl));
    }

    template<IdSetLike A, IdSetLike B>
    inline std::size_t IdSetAnd<A, B>::capacity() const {
        return std::min(a.capacity(), b.capacity());
    }

    template<IdSetLike A, IdSetLike B>
    inline Id IdSetAnd<A, B>::first() const {
        return detail::first(*this);
    }

    template<IdSetLike A, IdSetLike B>
    inline bool IdSetAnd<A, B>::empty() const {
        return first() == capacity();
    }

    template<IdSetLike A, IdSetLike B>
    inline auto IdSetAnd<A, B>::begin() const -> const_iterator {
        return cbegin();
    }

    template<IdSetLike A, IdSetLike B>
    inline auto IdSetAnd<A, B>::end() const -> const_iterator {
        return cend();
    }

    template<IdSetLike A, IdSetLike B>
    inline auto IdSetAnd<A, B>::cbegin() const -> const_iterator {
        return const_iterator(this, first());
    }

    template<IdSetLike A, IdSetLike B>
    inline auto IdSetAnd<A, B>::cend() const -> const_iterator {
        return const_iterator(this, capacity());
    }

    ////////////////// IdSetOr

    template<IdSetLike A, IdSetLike B>
    inline bool IdSetOr<A, B>::contains(Id id) const {
        return a.contains(id) || b.contains(id);
    }

    template<IdSetLike A, IdSetLike B>
    inline uint64_t IdSetOr<A, B>::level_data(std::size_t lvl, std::size_t ind) const {
        return a.level_data(lvl, ind) | b.level_data(lvl, ind);
    }

    template<IdSetLike A, IdSetLike B>
    inline std::size_t IdSetOr<A, B>::level_capacity(std::size_t lvl) const {
        return std::min(a.level_capacity(lvl), b.level_capacity(lvl));
    }

    template<IdSetLike A, IdSetLike B>
    inline std::size_t IdSetOr<A, B>::capacity() const {
        return std::min(a.capacity(), b.capacity());
    }

    template<IdSetLike A, IdSetLike B>
    inline Id IdSetOr<A, B>::first() const {
        return detail::first(*this);
    }

    template<IdSetLike A, IdSetLike B>
    inline bool IdSetOr<A, B>::empty() const {
        return a.empty() && b.empty();
    }

    template<IdSetLike A, IdSetLike B>
    inline auto IdSetOr<A, B>::begin() const -> const_iterator {
        return cbegin();
    }

    template<IdSetLike A, IdSetLike B>
    inline auto IdSetOr<A, B>::end() const -> const_iterator {
        return cend();
    }

    template<IdSetLike A, IdSetLike B>
    inline auto IdSetOr<A, B>::cbegin() const -> const_iterator {
        return const_iterator(this, first());
    }

    template<IdSetLike A, IdSetLike B>
    inline auto IdSetOr<A, B>::cend() const -> const_iterator {
        return const_iterator(this, capacity());
    }

    ////////////////// IdSetNot

    template<IdSetLike S>
    inline bool IdSetNot<S>::contains(Id id) const {
        return !set.contains(id);
    }

    template<IdSetLike S>
    inline uint64_t IdSetNot<S>::level_data(std::size_t lvl, std::size_t ind) const {
        if (lvl) return ~0ull; // upper levels are ignored
        return ind < set.level_capacity(0) ? ~set.level_data(0, ind) : ~0ull;
    }

    template<IdSetLike S>
    inline std::size_t IdSetNot<S>::level_capacity(std::size_t) const {
        return IdSet::max_size;
    }

    template<IdSetLike S>
    inline std::size_t IdSetNot<S>::capacity() const {
        return IdSet::max_size;
    }

    template<IdSetLike S>
    inline Id IdSetNot<S>::first() const {
        return detail::first(*this);
    }

    template<IdSetLike S>
    inline bool IdSetNot<S>::empty() const {
        return !set.empty();
    }

    template<IdSetLike S>
    inline auto IdSetNot<S>::begin() const -> const_iterator {
        return cbegin();
    }

    template<IdSetLike S>
    inline auto IdSetNot<S>::end() const -> const_iterator {
        return cend();
    }

    template<IdSetLike S>
    inline auto IdSetNot<S>::cbegin() const -> const_iterator {
        return const_iterator(this, first());
    }

    template<IdSetLike S>
    inline auto IdSetNot<S>::cend() const -> const_iterator {
        return const_iterator(this, capacity());
    }

    ////////////////// FullIdSet

    inline bool FullIdSet::contains([[maybe_unused]] Id id) const {
        return true;
    }

    inline uint64_t FullIdSet::level_data([[maybe_unused]] std::size_t lvl,
                                          [[maybe_unused]] std::size_t ind) const {
        return ~0ull;
    }

    inline std::size_t FullIdSet::level_capacity([[maybe_unused]] std::size_t lvl) const {
        return IdSet::max_size;
    }

    inline std::size_t FullIdSet::capacity() const {
        return IdSet::max_size;
    }

    inline std::size_t FullIdSet::size() const {
        return IdSet::max_size;
    }

    inline Id FullIdSet::first() const {
        return 0;
    }

    inline bool FullIdSet::empty() const {
        return false;
    }

    inline auto FullIdSet::begin() const -> const_iterator {
        return cbegin();
    }

    inline auto FullIdSet::end() const -> const_iterator {
        return cend();
    }

    inline auto FullIdSet::cbegin() const -> const_iterator {
        return {this, first()};
    }

    inline auto FullIdSet::cend() const -> const_iterator {
        return {this, static_cast<Id>(capacity())};
    }

    ////////////////// EmptyIdSet

    inline bool EmptyIdSet::contains([[maybe_unused]]Id id) const {
        return false;
    }

    inline uint64_t EmptyIdSet::level_data([[maybe_unused]] std::size_t lvl,
                                           [[maybe_unused]] std::size_t ind) const {
        return 0ull;
    }

    inline std::size_t EmptyIdSet::level_capacity([[maybe_unused]] std::size_t lvl) const {
        return 0;
    }

    inline std::size_t EmptyIdSet::capacity() const {
        return 0;
    }

    inline std::size_t EmptyIdSet::size() const {
        return 0;
    }

    inline Id EmptyIdSet::first() const {
        return 0;
    }

    inline bool EmptyIdSet::empty() const {
        return true;
    }

    inline auto EmptyIdSet::begin() const -> const_iterator {
        return cbegin();
    }

    inline auto EmptyIdSet::end() const -> const_iterator {
        return cend();
    }

    inline auto EmptyIdSet::cbegin() const -> const_iterator {
        return {this, first()};
    }

    inline auto EmptyIdSet::cend() const -> const_iterator {
        return {this, static_cast<Id>(capacity())};
    }

    ////////////////// IdSetIterator

    template<IdSetLike S>
    inline Id IdSetIterator<S>::operator*() const {
        return pos;
    }

    template<IdSetLike S>
    inline IdSetIterator <S> &IdSetIterator<S>::operator++() {
        levels_data[0] &= levels_data[0] - 1; // zero out current bit

        pos >>= IdSet::shift; // index of block at level 0
        for (std::size_t i = 0;;) {
            if (levels_data[i] == 0) { // should go up
                if (i == IdSet::levels_num - 1) {
                    pos = set->capacity(); // nowhere to go up
                    break;
                }
                pos >>= IdSet::shift; // move pos back
                ++i; // go up
                levels_data[i] &= (levels_data[i] - 1); // erase the false-positive bit
                continue;
            }

            uint64_t trailing = __builtin_ctzll(levels_data[i]);
            pos <<= IdSet::shift;
            pos |= trailing;

            if (i == 0) break;
            --i; // go down
            levels_data[i] = set->level_data(i, pos);
        }
        return *this;
    }

    template<IdSetLike S>
    inline IdSetIterator <S> IdSetIterator<S>::operator++(int) {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    template<IdSetLike S>
    bool IdSetIterator<S>::operator==(const IdSetIterator <S> &other) const {
        return set == other.set && pos == other.pos;
    }

    template<IdSetLike S>
    bool IdSetIterator<S>::operator!=(const IdSetIterator <S> &other) const {
        return set != other.set || pos != other.pos;
    }
}
