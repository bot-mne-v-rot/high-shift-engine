#ifndef HIGH_SHIFT_COMPONENT_H
#define HIGH_SHIFT_COMPONENT_H

#include <concepts>
#include <vector>
#include <type_traits>

#include "ecs/entity.h"
#include "ecs/storage.h"

namespace ecs {
    template<class C>
    concept Component = requires(C c) {
        typename C::Storage;
        requires Storage<typename C::Storage>;
//        requires std::same_as<decltype(c.id), Id>;
    };

    template<typename S>
    concept ConstStorageRef =
    Storage<S> &&
    Component<typename S::value_type> &&
    std::is_same_v<S, const typename S::value_type::Storage &>;

    template<typename S>
    concept MutStorageRef =
    Storage<S> &&
    Component<typename S::value_type> &&
    std::is_same_v<S, typename S::value_type::Storage &>;

    template<typename S>
    concept StorageRef = ConstStorageRef<S> || MutStorageRef<S>;
}

#endif //HIGH_SHIFT_COMPONENT_H
