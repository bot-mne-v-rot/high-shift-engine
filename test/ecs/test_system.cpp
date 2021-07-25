#include "doctest.h"

#include "ecs/system.h"

#include <vector>

namespace {
    struct SomeComponent {
        using Storage = std::vector<SomeComponent>;
        ecs::Id id;
    };

    struct OtherComponent {
        using Storage = std::vector<OtherComponent>;
        ecs::Id id;
    };

    class ExampleSystem {
    public:
        void update(const SomeComponent::Storage &, OtherComponent::Storage &) {}
    };

    static_assert(ecs::Component<SomeComponent>);
    static_assert(ecs::Component<OtherComponent>);
    static_assert(ecs::System<ExampleSystem>);
}