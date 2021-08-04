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
    auto InvertedStorage<S>::operator[](Id) -> Reference {
        return dummy;
    }

    template<Storage S>
    auto InvertedStorage<S>::operator[](Id) const -> ConstReference {
        return dummy;
    }

    template<Storage S>
    auto InvertedStorage<S>::present() const -> const IdSetType & {
        return mask;
    }
}