#ifndef HIGH_SHIFT_VEC_STORAGE_IMPL_H
#define HIGH_SHIFT_VEC_STORAGE_IMPL_H

namespace ecs {
    template<typename T>
    VecStorage<T>::VecStorage() = default;

    template<typename T>
    std::size_t VecStorage<T>::size() const {
        return sz;
    }

    template<typename T>
    std::size_t VecStorage<T>::capacity() const {
        return cp;
    }

    template<typename T>
    bool VecStorage<T>::empty() const {
        return sz == 0;
    }

    template<typename T>
    VecStorage<T>::VecStorage(std::size_t n) {
        reserve(n);
    }

    template<typename T>
    VecStorage<T>::VecStorage(const VecStorage &other) {
        *this = other;
    }

    template<typename T>
    VecStorage <T> &VecStorage<T>::operator=(const VecStorage &other) {
        if (this == &other)
            return *this;

        reserve(other.capacity());
        foreach(other.mask, [&, this](Id id) {
            if (mask.contains(id))
                data[id] = other.data[id];
            else
                new(data + id) T(other.data[id]);
        });

        foreach(mask, [&, this](Id id) {
            if (!other.mask.contains(id))
                std::destroy_at(data + id);
        });

        mask = other.mask;

        sz = other.sz;
        return *this;
    }

    template<typename T>
    VecStorage<T>::VecStorage(VecStorage &&other) noexcept {
        *this = std::move(other);
    }

    template<typename T>
    VecStorage <T> &VecStorage<T>::operator=(VecStorage &&other) noexcept {
        swap(other);
        return *this;
    }

    template<typename T>
    T &VecStorage<T>::operator[](Id index) {
        return data[index];
    }

    template<typename T>
    const T &VecStorage<T>::operator[](Id index) const {
        return data[index];
    }

    template<typename T>
    bool VecStorage<T>::contains(Id id) const {
        return mask.contains(id);
    }

    template<typename T>
    template<typename ...Args>
    void VecStorage<T>::emplace(Id id, Args &&...args) {
        mask.reserve(id + 1);
        reserve(mask.capacity());
        if (!mask.contains(id)) ++sz;
        mask.insert(id);
        new(data + id) T(std::forward<Args>(args)...);
    }

    template<typename T>
    void VecStorage<T>::insert(Id id, const T &value) {
        emplace(id, value);
    }

    template<typename T>
    void VecStorage<T>::insert(Id id, T &&value) {
        emplace(id, std::move(value));
    }

    template<typename T>
    void VecStorage<T>::erase(Id id) {
        if (!mask.contains(id))
            return;
        std::destroy_at(data + id);
        mask.erase(id);
        --sz;
    }

    template<typename T>
    void VecStorage<T>::clear() {
        foreach(mask, [this](Id id) {
            std::destroy_at(data + id);
        });
        mask.clear();
        sz = 0;
    }

    template<typename T>
    void VecStorage<T>::reserve(std::size_t n) noexcept {
        if (n <= cp)
            return;

        T *new_data = static_cast<T *>(operator new(n * sizeof(T)));

        foreach(mask, [&, this](Id id) {
            new(new_data + id) T(std::move(data[id]));
            std::destroy_at(data + id);
        });

        operator delete(data);

        data = new_data;
        cp = n;
    }

    template<typename T>
    void VecStorage<T>::swap(VecStorage &other) noexcept {
        std::swap(data, other.data);
        std::swap(mask, other.mask);
        std::swap(sz, other.sz);
        std::swap(cp, other.cp);
    }

    template<typename T>
    VecStorage<T>::~VecStorage() noexcept {
        foreach(mask, [this](Id id) {
            std::destroy_at(data + id);
        });
        operator delete(data);
        sz = 0, cp = 0;
        data = nullptr;
    }

    template<typename T>
    const IdSet &VecStorage<T>::present() const {
        return mask;
    }

    template<typename T, typename Fn>
    void foreach(const VecStorage<T> &storage, Fn &&f) {
        ecs::foreach(storage.present(), [&](auto id) {
            f(storage[id]);
        });
    }

    template<typename T, typename Fn>
    void foreach(VecStorage<T> &storage, Fn &&f) {
        ecs::foreach(storage.present(), [&](auto id) {
            f(storage[id]);
        });
    }

    template<typename T, typename Fn>
    void foreach_with_id(const VecStorage<T> &storage, Fn &&f) {
        ecs::foreach(storage.mask, [&](auto id) {
            f(id, storage[id]);
        });
    }

    template<typename T, typename Fn>
    void foreach_with_id(VecStorage<T> &storage, Fn &&f) {
        ecs::foreach(storage.mask, [&](auto id) {
            f(id, storage[id]);
        });
    }
}

#endif //HIGH_SHIFT_VEC_STORAGE_IMPL_H
