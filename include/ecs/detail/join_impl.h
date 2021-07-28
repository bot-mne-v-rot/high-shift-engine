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

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline auto JoinRange<Storages...>::begin() const -> iterator {
            return b;
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline auto JoinRange<Storages...>::end() const -> iterator {
            return e;
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline auto JoinRange<Storages...>::cbegin() const -> iterator {
            return b;
        }

        template<typename ...Storages>
        requires(Storage <std::remove_const_t<Storages>> &&...)
        inline auto JoinRange<Storages...>::cend() const -> iterator {
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
        inline auto JoinIterator<Storages...>::operator->() const -> pointer {
            return deref_to_ptr_storages_tuple(storages, *mask_iterator);
        }
    }

    template<typename ...Storages>
    requires(Storage <std::remove_const_t<Storages>> &&...)
    inline detail::JoinRange<Storages...> join(Storages &...storages) {
        auto joined_mask = std::make_unique<detail::JoinedMask<Storages...>>((storages.present() & ...));
        return detail::JoinRange<Storages...>(
                detail::JoinIterator<Storages...>(&storages..., joined_mask->begin()),
                detail::JoinIterator<Storages...>(&storages..., joined_mask->end()),
                std::move(joined_mask)
        );
    }

}