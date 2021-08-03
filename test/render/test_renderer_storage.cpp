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
        CHECK(storage.empty());
        CHECK(!storage.contains(0));
    }

    TEST_CASE("insert and delete") {
        render::RendererStorage storage;
        render::ShaderProgram p_1{1}, p_2{2};
        render::Renderer
        r_11{p_1},
        r_12{p_1, false, true},
        r_13{p_1, true, false},
        r_14{p_1, true, true},
        r_21{p_2},
        r_22{p_2, false, true},
        r_23{p_2, true, false},
        r_24{p_2, true, true};
        std::vector<render::Renderer> renderers {r_11, r_12, r_13, r_14, r_21, r_22, r_23, r_24};
        auto rs = renderers;
        for (int i = 0; i < 8; i++) {
            storage.insert(i, std::move(renderers[i]));
        }
        CHECK(storage.size() == 8);
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

    TEST_CASE("efficient iteration") {
        render::RendererStorage storage;
        render::ShaderProgram p_1{1}, p_2{2};
        //fill with components with different programs
        for (int i = 1; i <= 100; i++) {
            storage.insert(i, render::Renderer{p_1, false, false});
            storage.insert(i+100, render::Renderer{p_1, false, true});
            storage.insert(i+200, render::Renderer{p_1, true, false});
            storage.insert(i+300, render::Renderer{p_1, true, true});
            storage.insert(i+400, render::Renderer{p_2, false, false});
            storage.insert(i+500, render::Renderer{p_2, false, true});
            storage.insert(i+600, render::Renderer{p_2, true, false});
            storage.insert(i+700, render::Renderer{p_2, true, true});
        }

        // number of expected shader changes
        int max_program_changes_expected = 8;

        auto tables = storage.get_table();
        // number of different shaders
        std::size_t max_program_changes_calculated = tables[0][0].dense.size() + tables[0][1].dense.size() + tables[1][0].dense.size() + tables[1][1].dense.size();

        CHECK(max_program_changes_expected == max_program_changes_calculated);

        std::size_t actual_program_changes = 0;
        ecs::Id prev_shader_id = 0;

        //compute actual number of times when a shader changes
        for (auto comp : storage) {
            if (comp.shader_program.id != prev_shader_id) {
                actual_program_changes++;
                prev_shader_id = comp.shader_program.id;
            }
        }

        CHECK(max_program_changes_expected == actual_program_changes);

    }
}