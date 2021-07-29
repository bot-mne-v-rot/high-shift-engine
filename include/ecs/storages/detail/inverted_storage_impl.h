namespace ecs {
    template<Storage S>
    std::size_t InvertedStorage<S>::size() const {
        return mask.capacity() - storage.size();
    }

    template<Storage S>
    std::size_t InvertedStorage<S>::capacity() const {
        return mask.capacity();
    }

    template<Storage S>
    bool InvertedStorage<S>::empty() const {
        return size() == 0;
    }

    template<Storage S>
    bool InvertedStorage<S>::contains(Id id) const {
        return !storage.contains(id);
    }

    template<Storage S>
    auto InvertedStorage<S>::operator[](Id) -> reference {
        return dummy;
    }

    template<Storage S>
    auto InvertedStorage<S>::operator[](Id) const -> const_reference {
        return dummy;
    }

    template<Storage S>
    auto InvertedStorage<S>::present() const -> const IdSetType & {
        return mask;
    }

    template<Storage S>
    template<typename Ref, typename Ptr>
    InvertedStorage<S>::iterator_template<Ref, Ptr>::iterator_template(MaskIterator mask_iterator)
            : mask_iterator(mask_iterator) {}

    template<Storage S>
    template<typename Ref, typename Ptr>
    auto InvertedStorage<S>::iterator_template<Ref, Ptr>::operator*() const -> reference {
        return dummy;
    }

    template<Storage S>
    template<typename Ref, typename Ptr>
    auto InvertedStorage<S>::iterator_template<Ref, Ptr>::operator->() const -> pointer {
        return &dummy;
    }

    template<Storage S>
    template<typename Ref, typename Ptr>
    auto InvertedStorage<S>::iterator_template<Ref, Ptr>::operator++() -> iterator_template & {
        ++mask_iterator;
        return *this;
    }

    template<Storage S>
    template<typename Ref, typename Ptr>
    auto InvertedStorage<S>::iterator_template<Ref, Ptr>::operator++(int) -> iterator_template {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    template<Storage S>
    auto InvertedStorage<S>::begin() -> iterator {
        return { mask.begin() };
    }

    template<Storage S>
    auto InvertedStorage<S>::end() -> iterator {
        return { mask.end() };
    }

    template<Storage S>
    auto InvertedStorage<S>::cbegin() const -> const_iterator {
        return { mask.cbegin() };
    }

    template<Storage S>
    auto InvertedStorage<S>::cend() const -> const_iterator {
        return { mask.cend() };
    }

    template<Storage S>
    auto InvertedStorage<S>::begin() const -> const_iterator {
        return cbegin();
    }

    template<Storage S>
    auto InvertedStorage<S>::end() const -> const_iterator {
        return cend();
    }
}