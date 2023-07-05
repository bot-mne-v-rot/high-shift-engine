#ifndef HIGH_SHIFT_COMPONENT_H
#define HIGH_SHIFT_COMPONENT_H

#include "ecs/entity.h"

#include <concepts>
#include <type_traits>

namespace ecs {
    struct ComponentTag {};
    struct SharedComponentTag {};
    struct ChunkComponentTag {};
    struct MarkerComponentTag {};

    template<typename T>
    struct ComponentTraits {
    private:
        template<typename>
        static ComponentTag tag_impl(...);

        template<typename Q>
        static typename Q::Tag tag_impl(int);

    public:
        using Tag = decltype(tag_impl<T>(0));
    };

    template<typename C>
    concept Component = requires {
        requires !std::is_fundamental_v<C>;
        requires std::is_trivially_destructible_v<C>;
        requires !std::is_same_v<Entity, C>;
        requires std::is_same_v<C, std::remove_cvref_t<C>>;
    };

    template<typename C>
    concept PlainComponent = requires {
        requires Component<C>;
        requires std::is_same_v<typename ComponentTraits<C>::Tag, ComponentTag>;
    };

    template<typename C>
    concept SharedComponent = requires {
        requires Component<C>;
        requires std::is_same_v<typename ComponentTraits<C>::Tag, SharedComponentTag>;
    };

    template<typename C>
    concept ChunkComponent = requires {
        requires Component<C>;
        requires std::is_same_v<typename ComponentTraits<C>::Tag, ChunkComponentTag>;
    };

    using CompId = uint16_t;
    using ShCompVal = uint32_t;

    namespace detail {
        inline CompId _get_component_id() {
            static CompId id = 0;
            return ++id;
        }
    }

    template<typename>
    inline CompId get_component_id() {
        static CompId id = detail::_get_component_id();
        return id;
    }

    struct ComponentType {
        std::size_t size;
        std::size_t align;
        std::size_t array_offset;
        CompId id;

        template<Component C>
        static ComponentType create() {
            std::size_t size = sizeof(C);
            std::size_t align = alignof(C);
            // array_offset should be a multiple of align and not less than size
            std::size_t array_offset = (size + align - 1) / align * align;
            return {
                    .size = size,
                    .align = align,
                    .array_offset = array_offset,
                    .id = get_component_id<C>()
            };
        }

        bool operator==(const ComponentType &other) const {
            return id == other.id;
        }

        bool operator!=(const ComponentType &other) const {
            return id != other.id;
        }
    };
}

#endif //HIGH_SHIFT_COMPONENT_H
