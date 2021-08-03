#ifndef HIGH_SHIFT_VEC_STORAGE_H
#define HIGH_SHIFT_VEC_STORAGE_H

#include "ecs/component.h"
#include "ecs/storage.h"

namespace ecs {
    /**
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

        const IdSet &present() const;

        ~VecStorage() noexcept;

        template<typename U, typename Fn>
        friend void foreach(const VecStorage<U> &storage, Fn &&f);

        template<typename U, typename Fn>
        friend void foreach(VecStorage<U> &storage, Fn &&f);

        template<typename U, typename Fn>
        friend void foreach_with_id(const VecStorage<U> &storage, Fn &&f);

        template<typename U, typename Fn>
        friend void foreach_with_id(VecStorage<U> &storage, Fn &&f);

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
