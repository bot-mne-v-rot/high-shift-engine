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
        using value_type = T;
        using reference = T &;
        using const_reference = const T &;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;

        template<typename Ref, typename Ptr>
        class iterator_template;

        using iterator = iterator_template<reference, T *>;
        using const_iterator = iterator_template<const_reference, const T *>;

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

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

        WithIdView<iterator, const_iterator> with_id();
        WithIdView<const_iterator, const_iterator> with_id() const;

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
        template<typename Ref, typename Ptr>
        class iterator_template {
            using DenseIter = typename Dense::iterator;
            using DenseIdsIter = typename DenseIds::iterator;
        public:
            using reference = Ref;
            using pointer = Ptr;
            using value_type = T;
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
            DenseIter dense_it;
            DenseIdsIter dense_ids_it;

            friend SparseVecStorage;

            iterator_template(DenseIter dense_it, DenseIdsIter dense_ids_it)
                    : dense_it(std::move(dense_it)),
                      dense_ids_it(std::move(dense_ids_it)) {}
        };
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
