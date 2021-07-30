namespace ecs {
    namespace detail {
        template<typename ...Args, std::size_t... Ns>
        inline auto deref_storages_tuple_impl(const std::tuple<Args...> &tuple, Id id,
                                              std::index_sequence<Ns...>) {
            return std::tie((*std::get<Ns>(tuple))[id]...);
        }

        template<typename ...Args>
        inline auto deref_storages_tuple(const std::tuple<Args...> &tuple, Id id) {
            return deref_storages_tuple_impl(tuple, id, std::make_index_sequence<sizeof...(Args)>());
        }

        template<typename ...Args, std::size_t... Ns>
        inline auto deref_to_ptr_storages_tuple_impl(const std::tuple<Args...> &tuple, Id id,
                                                     std::index_sequence<Ns...>) {
            return std::tie(&(*std::get<Ns>(tuple))[id]...);
        }

        template<typename ...Args>
        inline auto deref_to_ptr_storages_tuple(const std::tuple<Args...> &tuple, Id id) {
            return deref_to_ptr_storages_tuple_impl(tuple, id, std::make_index_sequence<sizeof...(Args)>());
        }

        template<typename JoinIt, typename ...KeepAlive>
        inline auto JoinView<JoinIt, KeepAlive...>::begin() const -> iterator {
            return b;
        }

        template<typename JoinIt, typename ...KeepAlive>
        inline auto JoinView<JoinIt, KeepAlive...>::end() const -> iterator {
            return e;
        }

        template<typename JoinIt, typename ...KeepAlive>
        inline auto JoinView<JoinIt, KeepAlive...>::cbegin() const -> iterator {
            return b;
        }

        template<typename JoinIt, typename ...KeepAlive>
        inline auto JoinView<JoinIt, KeepAlive...>::cend() const -> iterator {
            return e;
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline JoinIterator<Storages...> &JoinIterator<Storages...>::operator++() {
            ++mask_iterator;
            return *this;
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline JoinIterator<Storages...> JoinIterator<Storages...>::operator++(int) {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline auto JoinIterator<Storages...>::operator*() const -> reference {
            return deref_storages_tuple(storages, *mask_iterator);
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline ecs::Id JoinIterator<Storages...>::id() const {
            return *mask_iterator;
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline auto JoinWithIdIterator<Storages...>::operator++() -> JoinWithIdIterator & {
            ++iter;
            return *this;
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline auto JoinWithIdIterator<Storages...>::operator++(int) -> JoinWithIdIterator {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline auto JoinWithIdIterator<Storages...>::operator*() const -> reference {
            return std::tuple_cat(std::make_tuple<Id>(id()), *iter);
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline ecs::Id JoinWithIdIterator<Storages...>::id() const {
            return iter.id();
        }
    }

    template<typename ...Storages>
    inline JoinView<Storages...> join(Storages &...storages) {
        auto joined_mask = std::make_unique<detail::JoinedMask<Storages...>>((storages.present() & ...));
        auto beg = detail::JoinIterator<Storages...>(&storages..., joined_mask->begin());
        auto end = detail::JoinIterator<Storages...>(&storages..., joined_mask->end());
        return {beg, end, std::move(joined_mask)};
    }

    template<typename ...Storages>
    requires(Storage <std::remove_const_t<Storages>> &&...)
    inline JoinWithIdView<Storages...> join_with_id(Storages &...storages) {
        auto joined_mask = std::make_unique<detail::JoinedMask<Storages...>>((storages.present() & ...));
        auto beg = detail::JoinWithIdIterator(
                detail::JoinIterator<Storages...>(&storages..., joined_mask->begin()));
        auto end = detail::JoinWithIdIterator(
                detail::JoinIterator<Storages...>(&storages..., joined_mask->end()));
        return {beg, end, std::move(joined_mask)};
    }

}