#ifndef HIGH_SHIFT_ENTITY_H
#define HIGH_SHIFT_ENTITY_H

#include <cstdint>

namespace ecs {
    using Id = uint32_t;
    struct Entity {
        Id id;
        uint32_t version;
    };
}

#endif //HIGH_SHIFT_ENTITY_H
