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

        template<typename It>
        concept HasIdGetter = requires(const It it) {{ it.id() } -> std::same_as<Id>; };
    }

    /**
     * Iterator that adds syntactically convenient way to
     * obtain components with ids they belong to via
     * std::pair/std::tuple destructuring.
     */
    template<typename It, typename ConstIt>
    class WithIdIterator {
    public:
        using value_type = std::pair<ecs::Id, typename std::iterator_traits<It>::reference>;
        using reference = value_type;
        using pointer = value_type *;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        explicit WithIdIterator(It iter) : iter(std::move(iter)) {}

        WithIdIterator() = default;

        reference operator*() const {
            return {iter.id(), *iter};
        }

        WithIdIterator &operator++() {
            ++iter;
            return *this;
        }

        WithIdIterator operator++(int) {
            auto copy = *this;
            ++iter;
            return copy;
        }

        bool operator==(const WithIdIterator &) const = default;
        bool operator!=(const WithIdIterator &) const = default;

        operator WithIdIterator<ConstIt, ConstIt>() const {
            return {ConstIt(iter)};
        }

    private:
        It iter;
    };

    /**
     * Simply holds a pair of iterators to let `for(... : ...)` hook up.
     *
     * Every Storage must provide with_id() method
     * to obtain an instance of this class.
     */
    template<typename It, typename ConstIt>
    class WithIdView {
    public:
        using const_iterator = WithIdIterator<ConstIt, ConstIt>;
        using iterator = WithIdIterator<It, ConstIt>;

        WithIdView(It begin, It end)
                : b(std::move(begin)), e(std::move(end)) {}

        iterator begin() const { return b; }

        iterator end() const { return e; }

        const_iterator cbegin() const { return b; }

        const_iterator cend() const { return e; }

        operator WithIdView<ConstIt, ConstIt>() const {
            return {ConstIt(b), ConstIt(e)};
        }

    private:
        iterator b, e;
    };

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
    } && requires(S a, const S b, Id id) {
        //// Other requirements:
        { a[id] } -> std::same_as<typename S::value_type &>;
        { b[id] } -> std::same_as<const typename S::value_type &>;
        { b.contains(id) } -> std::same_as<bool>;
        { b.present() } -> detail::ConstLvalueRefToIdSetLike;

        // with_id()
        requires detail::HasIdGetter<typename S::iterator>;
        requires detail::HasIdGetter<typename S::const_iterator>;
        { a.with_id() } -> std::same_as<WithIdView<typename S::iterator, typename S::const_iterator>>;
        { b.with_id() } -> std::same_as<WithIdView<typename S::const_iterator, typename S::const_iterator>>;
    };

    /**
     * Storage that provides unified interface for
     * inserting and erasing components.
     */
    template<class S>
    concept MutStorage = requires(S a, Id id,
                                  const typename S::value_type &c_lval_ref,
                                  typename S::value_type &&rval_ref) {
        { a.insert(id, c_lval_ref) } -> std::same_as<void>;
        { a.insert(id, rval_ref) } -> std::same_as<void>;
        { a.erase(id) } -> std::same_as<void>;
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
        explicit MutStorageInterface(S &s, uint32_t resource_id)
                : storage(&s), resource_id(resource_id) {
            erase_ptr = [](void *s, Id id) {
                static_cast<S *>(s)->erase(id);
            };
        }

        void erase(Id id) {
            erase_ptr(storage, id);
        }

        const uint32_t resource_id = 0;

    private:
        void *storage = nullptr;
        void (*erase_ptr)(void *, Id id) = nullptr;
    };
}

#endif //HIGH_SHIFT_STORAGE_H
