namespace ecs {
    template<typename T>
    SparseVecStorage<T>::SparseVecStorage(std::size_t n, Id ids) {
        reserve(n, ids);
    }

    template<typename T>
    T &SparseVecStorage<T>::operator[](Id index) {
        assert(mask.contains(index)); // compiled-out in Release mode
        return dense[sparse[index]];
    }

    template<typename T>
    const T &SparseVecStorage<T>::operator[](Id index) const {
        assert(mask.contains(index)); // compiled-out in Release mode
        return dense[sparse[index]];
    }

    template<typename T>
    bool SparseVecStorage<T>::contains(Id id) const {
        return mask.contains(id);
    }

    template<typename T>
    void SparseVecStorage<T>::reserve(std::size_t n, Id ids) noexcept {
        sparse.reserve(ids);
        dense.reserve(n);
        dense_ids.reserve(n);
    }

    template<typename T>
    std::size_t SparseVecStorage<T>::size() const {
        return dense.size();
    }

    template<typename T>
    bool SparseVecStorage<T>::empty() const {
        return dense.empty();
    }

    template<typename T>
    template<typename ...Args>
    void SparseVecStorage<T>::emplace(Id id, Args &&...args) {
        if (!mask.contains(id)) {
            mask.insert(id);
            sparse.resize(mask.capacity());
            sparse[id] = static_cast<uint32_t>(dense.size());
            dense.emplace_back(std::forward<Args>(args)...);
            dense_ids.push_back(id);
        } else {
            dense[sparse[id]] = T(std::forward<Args>(args)...);
        }
    }

    template<typename T>
    void SparseVecStorage<T>::erase(Id id) {
        if (mask.erase(id)) {
            std::size_t last = dense.size() - 1;
            std::swap(dense[sparse[id]], dense.back());
            std::swap(dense_ids[sparse[id]], dense_ids.back());
            sparse[dense_ids[sparse[id]]] = sparse[id];
            dense.pop_back();
            dense_ids.pop_back();
        }
    }

    template<typename T>
    void SparseVecStorage<T>::insert(Id id, const T &value) {
        emplace(id, value);
    }

    template<typename T>
    void SparseVecStorage<T>::insert(Id id, T &&value) {
        emplace(id, std::move(value));
    }

    template<typename T>
    void SparseVecStorage<T>::clear() {
        mask.clear();
        sparse.clear();
        dense.clear();
    }

    template<typename T>
    const IdSet &SparseVecStorage<T>::present() const {
        return mask;
    }

    template<typename T>
    void SparseVecStorage<T>::swap(SparseVecStorage <T> &other) noexcept {
        std::swap(mask, other.mask);
        dense.swap(other.dense);
        dense_ids.swap(other.dense_ids);
    }


    template<typename T>
    template<typename Ref, typename Ptr>
    auto SparseVecStorage<T>::iterator_template<Ref, Ptr>::operator*() const -> reference {
        return *dense_it;
    }

    template<typename T>
    template<typename Ref, typename Ptr>
    auto SparseVecStorage<T>::iterator_template<Ref, Ptr>::operator->() const -> pointer {
        return &*dense_it;
    }

    template<typename T>
    template<typename Ref, typename Ptr>
    Id SparseVecStorage<T>::iterator_template<Ref, Ptr>::id() const {
        return *dense_ids_it;
    }

    template<typename T>
    template<typename Ref, typename Ptr>
    auto SparseVecStorage<T>::iterator_template<Ref, Ptr>::operator++() -> iterator_template & {
        ++dense_it;
        ++dense_ids_it;
        return *this;
    }

    template<typename T>
    template<typename Ref, typename Ptr>
    auto SparseVecStorage<T>::iterator_template<Ref, Ptr>::operator++(int) -> iterator_template {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    template<typename T>
    auto SparseVecStorage<T>::begin() -> iterator {
        return iterator(dense.begin(), dense_ids.begin());
    }

    template<typename T>
    auto SparseVecStorage<T>::end() -> iterator {
        return iterator(dense.end(), dense_ids.end());
    }

    template<typename T>
    auto SparseVecStorage<T>::cbegin() const -> const_iterator {
        return const_iterator(dense.begin(), dense_ids.begin());
    }

    template<typename T>
    auto SparseVecStorage<T>::cend() const -> const_iterator {
        return const_iterator(dense.end(), dense_ids.end());
    }

    template<typename T>
    auto SparseVecStorage<T>::begin() const -> const_iterator {
        return cbegin();
    }

    template<typename T>
    auto SparseVecStorage<T>::end() const -> const_iterator {
        return cend();
    }

    template<typename T>
    auto SparseVecStorage<T>::with_id() -> WithIdView<iterator, const_iterator> {
        return { begin(), end() };
    }

    template<typename T>
    auto SparseVecStorage<T>::with_id() const -> WithIdView<const_iterator, const_iterator> {
        return { begin(), end() };
    }
}