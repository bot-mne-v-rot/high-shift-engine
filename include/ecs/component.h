#ifndef HIGH_SHIFT_COMPONENT_H
#define HIGH_SHIFT_COMPONENT_H

#include <concepts>
#include <vector>
#include <type_traits>

#include "entity.h"

namespace ecs {
    namespace detail {
        // https://stackoverflow.com/questions/60449592/how-do-you-define-a-c-concept-for-the-standard-library-containers
        // Unfortunately, some of the concept features are not supported on the current CLang versions
        template<class ContainerType>
        concept Container = requires(ContainerType a, const ContainerType b)
        {
//        requires std::regular<ContainerType>;
//        requires std::swappable<ContainerType>;
//        requires std::destructible<typename ContainerType::value_type>;
            requires std::same_as<typename ContainerType::reference, typename ContainerType::value_type &>;
            requires std::same_as<typename ContainerType::const_reference, const typename ContainerType::value_type &>;
//        requires std::forward_iterator<typename ContainerType::iterator>;
//        requires std::forward_iterator<typename ContainerType::const_iterator>;
//        requires std::signed_integral<typename ContainerType::difference_type>;
            requires std::same_as<typename ContainerType::difference_type, typename std::iterator_traits<typename
            ContainerType::iterator>::difference_type>;
            requires std::same_as<typename ContainerType::difference_type, typename std::iterator_traits<typename
            ContainerType::const_iterator>::difference_type>;
            { a.begin() } -> std::same_as<typename ContainerType::iterator>;
            { a.end() } -> std::same_as<typename ContainerType::iterator>;
            { b.begin() } -> std::same_as<typename ContainerType::const_iterator>;
            { b.end() } -> std::same_as<typename ContainerType::const_iterator>;
            { a.cbegin() } -> std::same_as<typename ContainerType::const_iterator>;
            { a.cend() } -> std::same_as<typename ContainerType::const_iterator>;
            { a.size() } -> std::same_as<typename ContainerType::size_type>;
            { a.empty() } -> std::same_as<bool>;
        };
    }

    template<class C>
    concept Component = requires(C c) {
        typename C::Storage;
        requires detail::Container<typename C::Storage>;
        { c.id } -> std::same_as<Id>;
    };

    template<typename S>
    concept ConstStorageRef =
    detail::Container<S> &&
    Component<typename S::value_type> &&
    std::is_same_v<S, const typename S::value_type::Storage &>;

    template<typename S>
    concept MutStorageRef =
    detail::Container<S> &&
    Component<typename S::value_type> &&
    std::is_same_v<S, typename S::value_type::Storage &>;

    template<typename S>
    concept StorageRef = ConstStorageRef<S> || MutStorageRef<S>;
}

#endif //HIGH_SHIFT_COMPONENT_H
