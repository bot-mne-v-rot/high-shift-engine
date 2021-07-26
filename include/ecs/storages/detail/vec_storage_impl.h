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
    VecStorage<T> &VecStorage<T>::operator=(const VecStorage &other) {
        if (this == &other)
            return *this;

        reserve(other.capacity());
        for (std::size_t i = 0; i < other.mask.size(); ++i)
            if (other.mask[i] && mask[i])
                data[i] = other.data[i];
            else if (other.mask[i] && !mask[i])
                new(data + i) T(other.data[i]);
            else if (!other.mask[i] && mask[i])
                std::destroy_at(data + i);

        for (std::size_t i = other.mask.size(); i < mask.size(); ++i)
            if (mask[i])
                std::destroy_at(data + i);

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
        return mask.size() > id && mask[id];
    }

    template<typename T>
    template<typename ...Args>
    void VecStorage<T>::emplace(Id id, Args &&...args) {
        if (mask.size() <= id) mask.resize(id + 1);
        reserve(mask.capacity());

        if (!mask[id]) ++sz;
        mask[id] = true;
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
        if (mask.size() <= id || !mask[id])
            return;
        std::destroy_at(data + id);
        mask[id] = false;
        --sz;
    }

    template<typename T>
    void VecStorage<T>::clear() {
        for (std::size_t i = 0; i < mask.size(); ++i)
            sz = 0;
    }

    template<typename T>
    void VecStorage<T>::reserve(std::size_t n) noexcept {
        if (n < cp)
            return;

        T *new_data = static_cast<T *>(operator new(n * sizeof(T)));

        for (std::size_t i = 0; i < mask.size(); ++i)
            if (mask[i]) {
                new(new_data + i) T(std::move(data[i]));
                std::destroy_at(data + i);
            }

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
        for (std::size_t i = 0; i < mask.size(); ++i)
            if (mask[i])
                std::destroy_at(data + i);
        operator delete(data);
        sz = 0, cp = 0;
        data = nullptr;
    }

    template<typename T>
    template<typename Ref, typename Ptr>
    class VecStorage<T>::iterator_template {
    public:
        using reference = Ref;
        using pointer = Ptr;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        iterator_template() = default;
        iterator_template(const iterator_template &) = default;
        iterator_template &operator=(const iterator_template &) = default;

        reference operator*() const {
            return *data;
        }

        pointer operator->() const {
            return data;
        }

        iterator_template &operator++() {
            Id id = static_cast<Id>(data - storage->data) + 1;
            while (id < storage->mask.size() && !storage->mask[id])
                ++id;
            data = storage->data + id;
            return *this;
        }

        iterator_template operator++(int) {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        bool operator==(const iterator_template &other) const {
            return data == other.data && storage == other.storage;
        }

        bool operator!=(const iterator_template &other) const {
            return data != other.data || storage != other.storage;
        }

        operator iterator_template<const T &, const T *>() const {
            return iterator_template<const T &, const T *>(data, storage);
        }

    private:
        T *data = nullptr;

        friend VecStorage;
        VecStorage *storage;
        iterator_template(T *data, VecStorage *storage)
                : data(data), storage(storage) {}
    };

    template<typename T>
    typename VecStorage<T>::iterator VecStorage<T>::begin() {
        std::size_t id = 0;
        while (id < mask.size() && !mask[id]) ++id;
        return iterator(data + id, this);
    }

    template<typename T>
    typename VecStorage<T>::iterator VecStorage<T>::end() {
        return iterator(data + mask.size(), this);
    }

    template<typename T>
    typename VecStorage<T>::const_iterator VecStorage<T>::cbegin() const {
        std::size_t id = 0;
        while (id < mask.size() && !mask[id]) ++id;
        return const_iterator(data, this);
    }

    template<typename T>
    typename VecStorage<T>::const_iterator VecStorage<T>::cend() const {
        return const_iterator(data + mask.size(), this);
    }

    template<typename T>
    typename VecStorage<T>::const_iterator VecStorage<T>::begin() const {
        return cbegin();
    }

    template<typename T>
    typename VecStorage<T>::const_iterator VecStorage<T>::end() const {
        return cend();
    }
}

#endif //HIGH_SHIFT_VEC_STORAGE_IMPL_H
