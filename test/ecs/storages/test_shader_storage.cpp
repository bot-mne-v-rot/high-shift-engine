//
// Created by muldrik on 02.08.2021.
//
#include "doctest.h"

#include "render/shader_storage.h"

TEST_SUITE("ecs/storages/ShaderStorage") {
    static_assert(ecs::Storage<render::RendererStorage>);
    static_assert(ecs::Component<render::Renderer>);

    TEST_CASE("empty") {
        render::RendererStorage storage;
        render::Renderer renderer;
        storage.insert(0, std::move(renderer));
        CHECK(storage.contains(0));
        CHECK(!storage.contains(1));
    }

    TEST_CASE("insert and delete") {
        render::RendererStorage storage;
        render::ShaderProgram p_1{1}, p_2{2}, p_3{3}, p_4{4};
        render::Renderer
             r_11{p_1},
             r_12{p_1, false, true},
             r_13{p_1, true, false},
             r_14{p_1, true, true},
             r_21{p_2},
             r_22{p_2, false, true},
             r_23{p_2, true, false},
             r_24{p_1, true, true};
        std::vector<render::Renderer> renderers {r_11, r_12, r_13, r_14, r_21, r_22, r_23, r_24};
        auto rs = renderers;
        for (int i = 0; i < 8; i++) {
            storage.insert(i, std::move(renderers[i]));
        }
        for (int i = 0; i < 8; i++) {
            CHECK(storage.contains(i));
        }
        for (int i = 0; i < 8; i++) {
            storage.erase(i);
        }
        for (int i = 0; i < 8; i++) {
            CHECK(!storage.contains(i));
        }
    }
}