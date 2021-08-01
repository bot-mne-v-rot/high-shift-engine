#ifndef HIGH_SHIFT_VEC_STORAGE_H
#define HIGH_SHIFT_VEC_STORAGE_H

#include "ecs/component.h"
#include "ecs/storage.h"
#include "ecs/id_set.h"

namespace ecs {
    /**
     * Container to store all the components contiguously.
     * Id and index in the internal container are equal.
     * Provides better cache utilization if the component
     * is used on most components.
     */
    template<typename T>
    class VecStorage {
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

        VecStorage();
        // Preallocates memory for n components
        explicit VecStorage(std::size_t n);

        VecStorage(const VecStorage &other);
        VecStorage(VecStorage &&other) noexcept;

        VecStorage &operator=(const VecStorage &other);
        VecStorage &operator=(VecStorage &&other) noexcept;

        std::size_t size() const;
        std::size_t capacity() const;
        bool empty() const;

        void reserve(std::size_t n) noexcept;

        bool contains(Id id) const;

        T &operator[](Id index);
        const T &operator[](Id index) const;

        void insert(Id id, const T &value);
        void insert(Id id, T &&value);

        template<typename ...Args>
        void emplace(Id id, Args &&...args);

        void erase(Id id);

        void clear();

        void swap(VecStorage<T> &other) noexcept;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

        WithIdView<iterator, const_iterator> with_id();
        WithIdView<const_iterator, const_iterator> with_id() const;

        const IdSet &present() const;

        ~VecStorage() noexcept;

    private:
        IdSet mask;
        T *data = nullptr;
        std::size_t sz = 0, cp = 0;
    };
}

namespace std {
    template<typename T>
    void swap(ecs::VecStorage<T> &a, ecs::VecStorage<T> &b) {
        a.swap(b);
    }
}

#include "detail/vec_storage_impl.h"

#endif //HIGH_SHIFT_VEC_STORAGE_H
