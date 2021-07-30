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
        for (ecs::Id id : other.mask)
            if (mask.contains(id))
                data[id] = other.data[id];
            else
                new(data + id) T(other.data[id]);

        for (ecs::Id id : mask)
            if (!other.mask.contains(id))
                std::destroy_at(data + id);

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
        for (ecs::Id id : mask)
            std::destroy_at(data + id);
        mask.clear();
        sz = 0;
    }

    template<typename T>
    void VecStorage<T>::reserve(std::size_t n) noexcept {
        if (n <= cp)
            return;

        T *new_data = static_cast<T *>(operator new(n * sizeof(T)));

        for (ecs::Id id : mask) {
            new(new_data + id) T(std::move(data[id]));
            std::destroy_at(data + id);
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
        for (ecs::Id id : mask)
            std::destroy_at(data + id);
        operator delete(data);
        sz = 0, cp = 0;
        data = nullptr;
    }

    template<typename T>
    const IdSet &VecStorage<T>::present() const {
        return mask;
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
            return storage->data[*set_iter];
        }

        pointer operator->() const {
            return storage->data + (*set_iter);
        }

        iterator_template &operator++() {
            ++set_iter;
            return *this;
        }

        iterator_template operator++(int) {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        Id id() const {
            return *set_iter;
        }

        bool operator==(const iterator_template &other) const = default;
        bool operator!=(const iterator_template &other) const = default;

        operator iterator_template<const T &, const T *>() const {
            return iterator_template<const T &, const T *>(set_iter, storage);
        }

    private:
        IdSet::iterator set_iter;

        friend VecStorage;
        VecStorage *storage;

        iterator_template(IdSet::iterator set_iter, VecStorage *storage)
                : set_iter(set_iter), storage(storage) {}
    };

    template<typename T>
    typename VecStorage<T>::iterator VecStorage<T>::begin() {
        return iterator(mask.begin(), this);
    }

    template<typename T>
    typename VecStorage<T>::iterator VecStorage<T>::end() {
        return iterator(mask.end(), this);
    }

    template<typename T>
    typename VecStorage<T>::const_iterator VecStorage<T>::cbegin() const {
        return const_iterator(mask.begin(), this);
    }

    template<typename T>
    typename VecStorage<T>::const_iterator VecStorage<T>::cend() const {
        return const_iterator(mask.end(), this);
    }

    template<typename T>
    typename VecStorage<T>::const_iterator VecStorage<T>::begin() const {
        return cbegin();
    }

    template<typename T>
    typename VecStorage<T>::const_iterator VecStorage<T>::end() const {
        return cend();
    }

    template<typename T>
    auto VecStorage<T>::with_id() -> WithIdRange<iterator, const_iterator> {
        return { begin(), end() };
    }

    template<typename T>
    auto VecStorage<T>::with_id() const -> WithIdRange<const_iterator, const_iterator> {
        return { begin(), end() };
    }
}

#endif //HIGH_SHIFT_VEC_STORAGE_IMPL_H
