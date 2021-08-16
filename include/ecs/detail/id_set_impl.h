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

    inline void IdSet::clear() {
        ecs::foreach(*this, [this](Id id) {
            erase(id);
        });
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

    ////////////////// IdSetAnd

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline bool IdSetAnd<A, B>::contains(Id id) const {
        return a.contains(id) && b.contains(id);
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline uint64_t IdSetAnd<A, B>::level_data(std::size_t lvl, std::size_t ind) const {
        return a.level_data(lvl, ind) & b.level_data(lvl, ind);
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline std::size_t IdSetAnd<A, B>::level_capacity(std::size_t lvl) const {
        return std::min(a.level_capacity(lvl), b.level_capacity(lvl));
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline std::size_t IdSetAnd<A, B>::capacity() const {
        return std::min(a.capacity(), b.capacity());
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline Id IdSetAnd<A, B>::first() const {
        return detail::first(*this);
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline bool IdSetAnd<A, B>::empty() const {
        return first() == capacity();
    }

    ////////////////// DynamicIdSet

    inline bool DynamicIdSetAnd::contains(Id id) const {
        bool answer = false;
        for (auto &set : sets)
            answer = (answer && set->contains(id));
        return answer;
    }

    inline std::size_t DynamicIdSetAnd::capacity() const {
        std::size_t cp = IdSet::max_size;
        for (auto &set : sets)
            cp = std::min(cp, set->capacity());
        return cp;
    }

    inline Id DynamicIdSetAnd::first() const {
        return detail::first(*this);
    }

    inline bool DynamicIdSetAnd::empty() const {
        return first() == capacity();
    }

    inline std::size_t DynamicIdSetAnd::level_capacity(std::size_t lvl) const {
        std::size_t cp = IdSet::max_size;
        for (auto &set : sets)
            cp = std::min(cp, set->level_capacity(lvl));
        return cp;
    }

    inline uint64_t DynamicIdSetAnd::level_data(std::size_t lvl, std::size_t ind) const {
        uint64_t data = UINT64_MAX;
        for (auto &set : sets)
            data &= set->level_data(lvl, ind);
        return data;
    }

    ////////////////// IdSetOr

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline bool IdSetOr<A, B>::contains(Id id) const {
        return a.contains(id) || b.contains(id);
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline uint64_t IdSetOr<A, B>::level_data(std::size_t lvl, std::size_t ind) const {
        return (ind < a.level_capacity(lvl) ? a.level_data(lvl, ind) : 0) |
               (ind < b.level_capacity(lvl) ? b.level_data(lvl, ind) : 0);
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline std::size_t IdSetOr<A, B>::level_capacity(std::size_t lvl) const {
        return std::max(a.level_capacity(lvl), b.level_capacity(lvl));
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline std::size_t IdSetOr<A, B>::capacity() const {
        return std::max(a.capacity(), b.capacity());
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline Id IdSetOr<A, B>::first() const {
        return detail::first(*this);
    }

    template<typename A, typename B>
    requires IdSetLike<std::remove_cvref_t<A>> && IdSetLike<std::remove_cvref_t<B>>
    inline bool IdSetOr<A, B>::empty() const {
        return a.empty() && b.empty();
    }

    ////////////////// IdSetNot

    template<typename S>
    requires IdSetLike<std::remove_cvref_t<S>>
    inline bool IdSetNot<S>::contains(Id id) const {
        return !set.contains(id);
    }

    template<typename S>
    requires IdSetLike<std::remove_cvref_t<S>>
    inline uint64_t IdSetNot<S>::level_data(std::size_t lvl, std::size_t ind) const {
        if (lvl) return ~0ull; // upper levels are ignored
        return ind < set.level_capacity(0) ? ~set.level_data(0, ind) : ~0ull;
    }

    template<typename S>
    requires IdSetLike<std::remove_cvref_t<S>>
    inline std::size_t IdSetNot<S>::level_capacity(std::size_t) const {
        return IdSet::max_size;
    }

    template<typename S>
    requires IdSetLike<std::remove_cvref_t<S>>
    inline std::size_t IdSetNot<S>::capacity() const {
        return IdSet::max_size;
    }

    template<typename S>
    requires IdSetLike<std::remove_cvref_t<S>>
    inline Id IdSetNot<S>::first() const {
        return detail::first(*this);
    }

    template<typename S>
    requires IdSetLike<std::remove_cvref_t<S>>
    inline bool IdSetNot<S>::empty() const {
        return !set.empty();
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

    ////////////////// foreach

    template<IdSetLike Set, typename Fn>
    void foreach(const Set &set, Fn &&f) {
        std::size_t lvl3_cp = set.level_capacity(3);
        std::size_t lvl2_cp = set.level_capacity(2);
        std::size_t lvl1_cp = set.level_capacity(1);
        std::size_t lvl0_cp = set.level_capacity(0);

        if (!lvl3_cp)
            return;

        uint64_t lvl3_data = set.level_data(3, 0);
        while (lvl3_data) {
            std::size_t lvl2 = __builtin_ctzll(lvl3_data); // get right-most bit
            if (lvl2 >= lvl2_cp)
                return;

            uint64_t lvl2_data = set.level_data(2, lvl2);
            while (lvl2_data) {
                std::size_t lvl1 = (lvl2 << IdSet::shift) |
                                   __builtin_ctzll(lvl2_data); // get right-most bit
                if (lvl1 >= lvl1_cp)
                    return;

                uint64_t lvl1_data = set.level_data(1, lvl1);
                while (lvl1_data) {
                    std::size_t lvl0 = (lvl1 << IdSet::shift) |
                                       __builtin_ctzll(lvl1_data); // get right-most bit
                    if (lvl0 >= lvl0_cp)
                        return;

                    uint64_t lvl0_data = set.level_data(0, lvl0);

                    while (lvl0_data) {
                        uint64_t pos = (lvl0 << IdSet::shift) |
                                       __builtin_ctzll(lvl0_data); // get right-most bit
                        f(pos);
                        lvl0_data &= (lvl0_data - 1); // clear right-most bit
                    }
                    lvl1_data &= (lvl1_data - 1); // clear right-most bit
                }
                lvl2_data &= (lvl2_data - 1); // clear right-most bit
            }
            lvl3_data &= (lvl3_data - 1); // clear right-most bit
        }
    }

    template<IdSetLike Set, typename Fn>
    Id find_if(const Set &set, Fn &&f) {
        std::size_t lvl3_cp = set.level_capacity(3);
        std::size_t lvl2_cp = set.level_capacity(2);
        std::size_t lvl1_cp = set.level_capacity(1);
        std::size_t lvl0_cp = set.level_capacity(0);

        if (!lvl3_cp)
            return IdSet::max_size;

        uint64_t lvl3_data = set.level_data(3, 0);
        while (lvl3_data) {
            std::size_t lvl2 = __builtin_ctzll(lvl3_data); // get right-most bit
            if (lvl2 >= lvl2_cp)
                return IdSet::max_size;

            uint64_t lvl2_data = set.level_data(2, lvl2);
            while (lvl2_data) {
                std::size_t lvl1 = (lvl2 << IdSet::shift) |
                                   __builtin_ctzll(lvl2_data); // get right-most bit
                if (lvl1 >= lvl1_cp)
                    return IdSet::max_size;

                uint64_t lvl1_data = set.level_data(1, lvl1);
                while (lvl1_data) {
                    std::size_t lvl0 = (lvl1 << IdSet::shift) |
                                       __builtin_ctzll(lvl1_data); // get right-most bit
                    if (lvl0 >= lvl0_cp)
                        return IdSet::max_size;

                    uint64_t lvl0_data = set.level_data(0, lvl0);

                    while (lvl0_data) {
                        uint64_t pos = (lvl0 << IdSet::shift) |
                                       __builtin_ctzll(lvl0_data); // get right-most bit
                        if (f(pos))
                            return pos;
                        lvl0_data &= (lvl0_data - 1); // clear right-most bit
                    }
                    lvl1_data &= (lvl1_data - 1); // clear right-most bit
                }
                lvl2_data &= (lvl2_data - 1); // clear right-most bit
            }
            lvl3_data &= (lvl3_data - 1); // clear right-most bit
        }
        return IdSet::max_size;
    }
}
