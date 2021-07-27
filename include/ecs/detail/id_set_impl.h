#include <algorithm>
#include <memory.h>
#include <cassert>

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
    }

    inline IdSet::IdSet() = default;

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

    inline void IdSet::erase(Id id) {
        if (cp <= id)
            return;

        if (contains(id))
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

        assert(new_cp <= (1 << (shift * levels_num)));
        // pow(bits_num, levels_num) = pow(2, log2(bits_num) * levels_num);

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
        uint64_t levels_data[IdSet::levels_num];
        levels_data[IdSet::levels_num - 1] = level_data(IdSet::levels_num - 1, 0);

        Id pos = 0;
        for (std::size_t i = IdSet::levels_num - 1; i < IdSet::levels_num; ) {
            if (levels_data[i] == 0) { // should go up
                if (i == IdSet::levels_num - 1)
                    return capacity(); // nowhere to go up
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
            levels_data[i] = level_data(i, pos);
        }
        return pos;
    }

    template<IdSetLike A, IdSetLike B>
    inline bool IdSetAnd<A, B>::empty() const {
        return first() == capacity();
    }

    template<IdSetLike A, IdSetLike B>
    inline typename IdSetAnd<A, B>::const_iterator IdSetAnd<A, B>::begin() const {
        return cbegin();
    }

    template<IdSetLike A, IdSetLike B>
    inline typename IdSetAnd<A, B>::const_iterator IdSetAnd<A, B>::end() const {
        return cend();
    }

    template<IdSetLike A, IdSetLike B>
    inline typename IdSetAnd<A, B>::const_iterator IdSetAnd<A, B>::cbegin() const {
//        if (empty()) return cend();
        return IdSetAnd<A, B>::const_iterator(this, first());
    }

    template<IdSetLike A, IdSetLike B>
    inline typename IdSetAnd<A, B>::const_iterator IdSetAnd<A, B>::cend() const {
        return IdSetAnd<A, B>::const_iterator(this, capacity());
    }

    template<IdSetLike S>
    inline Id IdSetIterator<S>::operator*() const {
        return pos;
    }

    template<IdSetLike S>
    inline IdSetIterator<S> &IdSetIterator<S>::operator++() {
        levels_data[0] &= levels_data[0] - 1; // zero out current bit

        pos >>= IdSet::shift; // index of block at level 0
        for (std::size_t i = 0; i < IdSet::levels_num; ) {
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
