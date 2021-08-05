#ifndef HIGH_SHIFT_RENDER_SYSTEM_H
#define HIGH_SHIFT_RENDER_SYSTEM_H

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#include "ecs/ecs.h"
#include "render/model_loader.h"
#include "render/shader.h"
#include "render/texture_loader.h"

namespace render {
    struct Transform {
        using Storage = ecs::VecStorage<Transform>;
        glm::vec3 position;
        glm::quat rotation;
    };

    struct MeshRenderer {
        using Storage = ecs::VecStorage<MeshRenderer>;
        Mesh *mesh;
        ShaderProgram *shader_program;
    };

    struct Camera {
        using Storage = ecs::SparseVecStorage<Camera>;
        glm::mat4 projection;

        // glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        // glm::toMat4
    };

    class RenderSystem {
    public:
        RenderSystem();
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void setup(ecs::World &world);
        void update(ecs::GameLoopControl &game_loop,
                    const TextureLoader &texture_loader,
                    const MeshRenderer::Storage &renderers,
                    const Transform::Storage &transforms,
                    const Camera::Storage &cameras);
        void teardown(ecs::World &world);

    private:
        struct Impl;
        Impl *impl;
    };
}

#endif //HIGH_SHIFT_RENDER_SYSTEM_H
