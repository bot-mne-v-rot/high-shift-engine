#ifndef HIGH_SHIFT_STORAGE_H
#define HIGH_SHIFT_STORAGE_H

#include <concepts>
#include <type_traits>

#include "ecs/id_set.h"

namespace ecs {
    namespace detail {
        template<typename S>
        concept ConstLvalueRefToIdSetLike = requires {
            requires std::is_lvalue_reference_v<S>;
            requires std::is_const_v<std::remove_reference_t<S>>;
            requires IdSetLike<std::remove_cvref_t<S>>;
        };
    }

    /**
     * https://stackoverflow.com/questions/60449592/how-do-you-define-a-c-concept-for-the-standard-library-containers
     *
     * Storage is STL-compatible container for Components. It is not allocator-aware by default.
     *
     * Unfortunately, some of the concept features are not supported on the current CLang versions
     * so they are commented till better time.
     */
    template<class S>
    concept Storage = requires(S a, const S b) {
        //// STL Container Named Requirement:
        //            requires std::regular<S>;
        //            requires std::swappable<S>;
        //            requires std::destructible<typename S::value_type>;
        requires std::same_as<typename S::reference, typename S::value_type &>;
        requires std::same_as<typename S::const_reference, const typename S::value_type &>;
        //            requires std::forward_iterator<typename S::iterator>;
        //            requires std::forward_iterator<typename S::const_iterator>;
        //            requires std::signed_integral<typename S::difference_type>;
        requires std::same_as<typename S::difference_type, typename std::iterator_traits<typename
        S::iterator>::difference_type>;
        requires std::same_as<typename S::difference_type, typename std::iterator_traits<typename
        S::const_iterator>::difference_type>;
        { a.begin() } -> std::same_as<typename S::iterator>;
        { a.end() } -> std::same_as<typename S::iterator>;
        { b.begin() } -> std::same_as<typename S::const_iterator>;
        { b.end() } -> std::same_as<typename S::const_iterator>;
        { a.cbegin() } -> std::same_as<typename S::const_iterator>;
        { a.cend() } -> std::same_as<typename S::const_iterator>;
        { a.size() } -> std::same_as<typename S::size_type>;
        { a.empty() } -> std::same_as<bool>;
    } && requires(S a, const S b, Id id,
                  const typename S::value_type &c_lval_ref, typename S::value_type &&rval_ref) {
        //// Other requirements:
        { a[id] } -> std::same_as<typename S::value_type &>;
        { b[id] } -> std::same_as<const typename S::value_type &>;
        { a.insert(id, c_lval_ref) } -> std::same_as<void>;
        { a.insert(id, rval_ref) } -> std::same_as<void>;
        { a.erase(id) } -> std::same_as<void>;
        { b.contains(id) } -> std::same_as<bool>;
        { b.present() } -> detail::ConstLvalueRefToIdSetLike;
    };;
}

#endif //HIGH_SHIFT_STORAGE_H
