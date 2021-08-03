#ifndef HIGH_SHIFT_SPARSE_VEC_STORAGE_H
#define HIGH_SHIFT_SPARSE_VEC_STORAGE_H

#include <vector>

#include "ecs/component.h"
#include "ecs/storage.h"

namespace ecs {
    /**
     * Container to store all the components contiguously.
     * Has no particular guarantee on the order of stored
     * components.
     *
     * Suites better if the component is used not that often
     * or takes much memory.
     *
     * The component should be cheaply swapped.
     */
    template<typename T>
    class SparseVecStorage {
    public:
        using Component = T;

        SparseVecStorage() = default;
        // Preallocates memory for dense of `n` components and sparse of `ids`
        SparseVecStorage(std::size_t n, Id ids);

        std::size_t size() const;
        bool empty() const;

        // Preallocates memory for dense of `n` components and sparse of `ids`
        void reserve(std::size_t n, Id ids) noexcept;

        bool contains(Id id) const;

        T &operator[](Id index);
        const T &operator[](Id index) const;

        void insert(Id id, const T &value);
        void insert(Id id, T &&value);

        template<typename ...Args>
        void emplace(Id id, Args &&...args);

        void erase(Id id);

        void clear();

        void swap(SparseVecStorage<T> &other) noexcept;

        const IdSet &present() const;

    private:
        using Sparse = std::vector<uint32_t>;
        using Dense = std::vector<T>;
        using DenseIds = std::vector<Id>;

        IdSet mask;
        Sparse sparse; // IdSet::max_size can be stored in uint32_t
        Dense dense;
        DenseIds dense_ids;

    public:
        template<typename U, typename Fn>
        friend void foreach(const SparseVecStorage<U> &storage, Fn &&f);

        template<typename U, typename Fn>
        friend void foreach(SparseVecStorage<U> &storage, Fn &&f);

        template<typename U, typename Fn>
        friend void foreach_with_id(const SparseVecStorage<U> &storage, Fn &&f);

        template<typename U, typename Fn>
        friend void foreach_with_id(SparseVecStorage<U> &storage, Fn &&f);
    };
}

namespace std {
    template<typename T>
    void swap(ecs::SparseVecStorage<T> &a, ecs::SparseVecStorage<T> &b) {
        a.swap(b);
    }
}

#include "ecs/storages/detail/sparse_vec_storage_impl.h"

#endif //HIGH_SHIFT_SPARSE_VEC_STORAGE_H
