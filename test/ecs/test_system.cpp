#include "doctest.h"

#include "ecs/system.h"
#include "ecs/storages/vec_storage.h"

namespace {
    struct SomeComponent {
        using Storage = ecs::VecStorage<SomeComponent>;
        ecs::Id id;
    };

    struct OtherComponent {
        using Storage = ecs::VecStorage<OtherComponent>;
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