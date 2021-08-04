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

        template<typename S>
        auto returns_const_foreach_lambda() {
            return [](const typename S::Component &component) -> void {};
        }

        template<typename S>
        auto returns_foreach_const_lambda_with_id() {
            return [](Id id, const typename S::Component &component) -> void {};
        }

        template<typename S>
        auto returns_foreach_lambda() {
            return [](typename S::Component &component) -> void {};
        }

        template<typename S>
        auto returns_foreach_lambda_with_id() {
            return [](Id id, typename S::Component &component) -> void {};
        }

        template<typename S>
        using ConstForEachLambda = decltype(returns_const_foreach_lambda<S>());

        template<typename S>
        using ConstForEachLambdaWithId = decltype(returns_foreach_const_lambda_with_id<S>());

        template<typename S>
        using ForEachLambda = decltype(returns_foreach_lambda<S>());

        template<typename S>
        using ForEachLambdaWithId = decltype(returns_foreach_lambda_with_id<S>());

        struct tag {};
    }

    /**
     * Storage is a container for Components.
     * It is neither STL-compatible nor allocator-aware by default.
     *
     * Unfortunately, some of the concept features are not supported on the current CLang versions
     * so they are commented till better time.
     */
    template<class S>
    concept Storage = requires(S a, const S b, Id id) {
        typename S::Component;
        //            requires std::regular<S>;
        //            requires std::swappable<S>;
        //            requires std::destructible<typename S::Component>;
        { a.size() } -> std::same_as<std::size_t>;
        { a.empty() } -> std::same_as<bool>;
        { foreach(b, std::declval<detail::ConstForEachLambda<S>>()) } -> std::same_as<void>;
        { foreach_with_id(b, std::declval<detail::ConstForEachLambdaWithId<S>>()) } -> std::same_as<void>;
        { a[id] } -> std::same_as<typename S::Component &>;
        { b[id] } -> std::same_as<const typename S::Component &>;
        { b.contains(id) } -> std::same_as<bool>;
        { b.present() } -> detail::ConstLvalueRefToIdSetLike;
    };

    /**
     * Storage that provides unified interface for
     * inserting and erasing components.
     */
    template<class S>
    concept MutStorage = requires(S a, Id id,
                                  const typename S::Component &c_lval_ref,
                                  typename S::Component &&rval_ref,
                                  detail::ForEachLambda<S> foreach_lambda,
                                  detail::ForEachLambdaWithId<S> foreach_lambda_with_id) {
        { a.insert(id, c_lval_ref) } -> std::same_as<void>;
        { a.insert(id, rval_ref) } -> std::same_as<void>;
        { a.erase(id) } -> std::same_as<void>;
        { foreach(a, foreach_lambda) } -> std::same_as<void>;
        { foreach_with_id(a, foreach_lambda_with_id) } -> std::same_as<void>;
    } && Storage<S>;

    /**
     * Interface to Storage's erase method
     * exploiting type-erasure technique
     * without involving inheritance and
     * virtual methods.
     */
    class MutStorageInterface {
    public:
        MutStorageInterface() = default;

        template<MutStorage S>
        explicit MutStorageInterface(S &s, uint32_t res_id)
                : storage(&s), res_id(res_id) {
            erase_ptr = [](void *s, Id id) {
                static_cast<S *>(s)->erase(id);
            };
        }

        void erase(Id id) {
            erase_ptr(storage, id);
        }

        uint32_t resource_id() const {
            return res_id;
        }

    private:
        void *storage = nullptr;
        void (*erase_ptr)(void *, Id id) = nullptr;
        uint32_t res_id = 0;
    };
}

#endif //HIGH_SHIFT_STORAGE_H
