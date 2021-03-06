#ifndef HIGH_SHIFT_WORLD_H
#define HIGH_SHIFT_WORLD_H

#include <bitset>
#include <array>
#include <vector>
#include <cassert>
#include <memory>
#include <concepts>

namespace ecs {
    namespace detail {
        inline uint32_t _get_resource_id() {
            static uint32_t id = 0;
            return ++id;
        }

        template<typename>
        inline uint32_t get_resource_id() {
            static uint32_t id = _get_resource_id();
            return id;
        }
    }

    template<typename R>
    concept Resource = requires {
        requires std::same_as<R, std::remove_cvref_t<R>>;
    };

    /**
     * Move-only container for resources. Uses type as a key.
     * Stores Resources on heap with exclusive ownership.
     */
    class World {
    public:
        World() = default;

        World(const World &) = delete;
        World &operator=(const World &) = delete;

        World(World &&) = default;
        World &operator=(World &&) = default;

        /**
         * Stores Resource by res to it.
         * If you insert the same resource twice, it is overwritten.
         * @tparam R Resource type to store.
         * @param res std::unique_ptr represents transferring of the ownership.
         */
        template<Resource R>
        void insert(std::unique_ptr<R> res) {
            static auto id = detail::get_resource_id<R>();
            presence.set(id);
            resources[id] = std::move(res);
        }

        /**
         * Creates Resource in-place.
         * If you create the same resource twice, it is overwritten.
         * @tparam R Resource type to store
         */
        template<Resource R, typename ...Args>
        void emplace(Args &&...args) {
            static auto id = detail::get_resource_id<R>();
            presence.set(id);

            auto res = std::make_unique<R>(std::forward<Args>(args)...);
            resources[id] = std::move(res);
        }

        /**
         * Destroys Resource.
         * Checks that resource exists with <cassert> which is disabled in Release build.
         * @tparam R Resource type to erase
         */
        template<Resource R>
        void erase() {
            static auto id = detail::get_resource_id<R>();
            assert(presence[id]);

            presence.reset(id);
            resources[id] = std::shared_ptr<R>();
        }

        /**
         * Obtains immutable reference to Resource.
         * Checks that resource exists with <cassert> which is disabled in Release build.
         * @tparam R Resource type to get
         */
        template<Resource R>
        const R &get() const {
            static auto id = detail::get_resource_id<R>();
            assert(presence[id]);
            return *static_cast<const R *>(resources[id].get());
        }

        /**
         * Obtains mutable reference to Resource.
         * Checks that resource exists with <cassert> which is disabled in Release build.
         * @tparam R Resource type to get
         */
        template<Resource R>
        R &get() {
            static auto id = detail::get_resource_id<R>();
            assert(presence[id]);
            return *static_cast<R *>(resources[id].get());
        }

        /**
         * Checks if Resource is present.
         * @tparam R Resource type to get
         */
        template<Resource R>
        bool has() {
            static auto id = detail::get_resource_id<R>();
            return presence[id];
        }

    private:
        static constexpr std::size_t max_resources = 1024;
        using ResourcesPresence = std::bitset<max_resources>;
        using ResourcesArray = std::array<std::shared_ptr<void>, max_resources>; // shared_ptr implements type-erasure

        ResourcesPresence presence;
        ResourcesArray resources;
    };
}

#endif //HIGH_SHIFT_WORLD_H
