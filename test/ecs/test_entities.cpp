//#include "doctest.h"
//
//#include "ecs/entities.h"
//#include "ecs/storages/vec_storage.h"
//
//namespace {
//    struct ComponentA {
//        using Storage = ecs::VecStorage<ComponentA>;
//    };
//
//    struct ComponentB {
//        using Storage = ecs::VecStorage<ComponentB>;
//    };
//
//    struct ComponentC {
//        using Storage = ecs::VecStorage<ComponentC>;
//    };
//
//    static_assert(ecs::Component<ComponentA>);
//    static_assert(ecs::Component<ComponentB>);
//    static_assert(ecs::Component<ComponentC>);
//}
//
//TEST_SUITE("ecs/entities") {
//    TEST_CASE("basic") {
//        ecs::World world;
//        world.emplace<ComponentA::Storage>();
//        world.emplace<ComponentB::Storage>();
//        world.emplace<ComponentC::Storage>();
//
//        auto &storage_a = world.get<ComponentA::Storage>();
//        auto &storage_b = world.get<ComponentB::Storage>();
//        auto &storage_c = world.get<ComponentC::Storage>();
//
//        ecs::Entities entities(&world);
//
//        auto id = entities.create(ComponentA{},
//                                  ComponentB{});
//
//        SUBCASE("creation") {
//            CHECK(storage_a.contains(id));
//            CHECK(storage_b.contains(id));
//
//            CHECK(entities.is_alive(id));
//        }
//
//        SUBCASE("destruction") {
//            storage_c.emplace(id); // adding additional
//
//            entities.destroy(id);
//
//            CHECK(!storage_a.contains(id));
//            CHECK(!storage_b.contains(id));
//            CHECK(!storage_c.contains(id));
//
//            CHECK(!entities.is_alive(id));
//        }
//
//        SUBCASE("storage erasure") {
//            world.erase<ComponentA::Storage>();
//            CHECK(!world.has<ComponentA::Storage>());
//        }
//
//    }
//}