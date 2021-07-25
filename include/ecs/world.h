#ifndef HIGH_SHIFT_WORLD_H
#define HIGH_SHIFT_WORLD_H

#include <bitset>
#include <array>
#include <cassert>
#include <memory>

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

    /**
     * Container for resources.
     */
    class World {
    public:
        World() = default;

        World(const World &) = delete;
        World &operator=(const World &) = delete;

        World(World &&) = default;
        World &operator=(World &&) = default;

        template<typename Res>
        void insert(std::unique_ptr<Res> ptr) {
            static auto id = detail::get_resource_id<Res>();
            presence.set(id);
            resources[id] = std::move(ptr);
        }

        template<typename Res, typename ...Args>
        void emplace(Args &&...args) {
            static auto id = detail::get_resource_id<Res>();
            presence.set(id);
            resources[id] = std::make_unique<Res>(std::forward<Args>(args)...);
        }

        template<typename Res>
        void erase() {
            static auto id = detail::get_resource_id<Res>();
            assert(presence[id]);

            presence.reset(id);
            resources[id] = std::shared_ptr<Res>();
        }

        template<class Res>
        const Res &get() const {
            static auto id = detail::get_resource_id<Res>();
            assert(presence[id]);
            return *static_cast<const Res *>(resources[id].get());
        }

        template<class Res>
        Res &get() {
            static auto id = detail::get_resource_id<Res>();
            assert(presence[id]);
            return *static_cast<Res *>(resources[id].get());
        }

        template<class Res>
        bool has() {
            static auto id = detail::get_resource_id<Res>();
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
