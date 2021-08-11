#ifndef HIGH_SHIFT_COMPONENT_H
#define HIGH_SHIFT_COMPONENT_H

#include <concepts>
#include <type_traits>

namespace ecs {
    using CmpId = uint16_t;

    namespace detail {
        inline CmpId _get_component_id() {
            static CmpId id = 0;
            return ++id;
        }
    }

    template<typename>
    inline CmpId get_component_id() {
        static CmpId id = detail::_get_component_id();
        return id;
    }

    template<typename C>
    concept Component = requires {
        requires std::is_trivially_destructible_v<C>;
    };

    struct ComponentType {
        std::size_t size;
        std::size_t align;
        std::size_t array_offset;
        CmpId id;

        template<Component C>
        static ComponentType create() {
            std::size_t size = sizeof(C);
            std::size_t align = alignof(C);
            // array_offset should be a multiple of align and not less that size
            std::size_t array_offset = (size + align - 1) / align * align;
            return {
                .size = size,
                .align = align,
                .array_offset = array_offset,
                .id = get_component_id<C>()
            };
        }
    };
}

#endif //HIGH_SHIFT_COMPONENT_H
