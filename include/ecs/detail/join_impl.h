namespace ecs {
    namespace detail {
        template<typename ...Args, std::size_t... Ns>
        inline auto deref_storages_tuple_impl(const std::tuple<Args...> &tuple, Id id, std::index_sequence<Ns...>) {
            return std::tie((*std::get<Ns>(tuple))[id]...);
        }
        template<typename ...Args>
        inline auto deref_storages_tuple(const std::tuple<Args...> &tuple, Id id) {
            return deref_storages_tuple_impl(tuple, id, std::make_index_sequence<sizeof...(Args)>());
        }

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        auto JoinRange<Storages...>::begin() const -> iterator {
            return b;
        }

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        auto JoinRange<Storages...>::end() const -> iterator {
            return e;
        }

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        auto JoinRange<Storages...>::cbegin() const -> iterator {
            return b;
        }

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        auto JoinRange<Storages...>::cend() const -> iterator {
            return e;
        }

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        JoinIterator<Storages...> & JoinIterator<Storages...>::operator++() {
            ++mask_iterator;
            return *this;
        }

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        JoinIterator<Storages...> JoinIterator<Storages...>::operator++(int) {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        template<typename ...Storages>
        requires(Storage<std::remove_const_t<Storages>> && ...)
        auto JoinIterator<Storages...>::operator*() const -> reference {
            return deref_storages_tuple(storages, *mask_iterator);
        }
    }

    template<typename ...Storages>
    requires(Storage<std::remove_const_t<Storages>> && ...)
    inline detail::JoinRange<Storages...> join(Storages &...storages) {
        auto joined_mask = (storages.present() && ...);
        return detail::JoinRange<Storages...>(
                detail::JoinIterator<Storages...>(&storages..., joined_mask.begin()),
                detail::JoinIterator<Storages...>(&storages..., joined_mask.end()),
                joined_mask
        );
    }

}