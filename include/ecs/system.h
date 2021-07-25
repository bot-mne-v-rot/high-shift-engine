#ifndef HIGH_SHIFT_SYSTEM_H
#define HIGH_SHIFT_SYSTEM_H

#include "ecs/component.h"
#include "ecs/world.h"
#include "ecs/utils.h"

#include <concepts>

namespace ecs {
    template<class S>
    concept System = requires(S sys) {
        requires std::is_default_constructible_v<S>;
        &S::update; // has update method
        requires detail::all_params_are_lvalue_refs_v<decltype(&S::update)>;
    };
}

#endif //HIGH_SHIFT_SYSTEM_H
