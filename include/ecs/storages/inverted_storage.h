#ifndef HIGH_SHIFT_INVERTED_STORAGE_H
#define HIGH_SHIFT_INVERTED_STORAGE_H

#include "ecs/storage.h"
#include "ecs/id_set.h"

namespace ecs {
    struct EmptyComponent {};

    template<Storage S>
    class InvertedStorage {
    public:
        using IdSetType = decltype(~std::declval<S>().present());

        using Component = EmptyComponent;
        using Reference = Component &;
        using ConstReference = const Component &;

        explicit InvertedStorage(const S &storage) : storage(storage), mask(~storage.present()) {}

        std::size_t size() const;
        std::size_t capacity() const;
        bool empty() const;

        bool contains(Id id) const;

        Reference operator[](Id index);
        ConstReference operator[](Id index) const;

        const IdSetType &present() const;

    private:
        IdSetType mask;
        const S &storage;
        EmptyComponent dummy;
    };

    template<Storage S>
    auto operator!(const S &storage) {
        return InvertedStorage<S>(storage);
    }

    // Iterating over InvertedStorage makes no sense
    template<Storage S, typename Fn>
    void foreach(const InvertedStorage<S> &storage, Fn &&f);

    // Iterating over InvertedStorage makes no sense
    template<Storage S, typename Fn>
    void foreach(InvertedStorage<S> &storage, Fn &&f);

    // Iterating over InvertedStorage makes no sense
    template<Storage S, typename Fn>
    void foreach_with_id(const InvertedStorage<S> &storage, Fn &&f);

    // Iterating over InvertedStorage makes no sense
    template<Storage S, typename Fn>
    void foreach_with_id(InvertedStorage<S> &storage, Fn &&f);
}

#include "ecs/storages/detail/inverted_storage_impl.h"

#endif //HIGH_SHIFT_INVERTED_STORAGE_H
