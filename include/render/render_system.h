#ifndef HIGH_SHIFT_RENDER_SYSTEM_H
#define HIGH_SHIFT_RENDER_SYSTEM_H

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#include "ecs/ecs.h"
#include "render/window_system.h"
#include "render/model_loader.h"
#include "render/shader.h"
#include "render/texture_loader.h"
#include "render/shader_loader.h"
#include "render/lights.h"

namespace render {
    struct Transform {
        glm::vec3 position;
        glm::quat rotation;
    };

    struct MeshRenderer {
        Handle<Model> model_handle;
        Handle<ShaderProgram> shader_program_handle;
    };

    struct Camera {
        glm::mat4 projection;
    };

    class RenderSystem {
    public:
        RenderSystem();
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        tl::expected<void, std::string> setup(ecs::World &world,
                                              const WindowSystem &window_system);

        void update(const ShaderLoader &shader_loader,
                    const TextureLoader &texture_loader,
                    const ModelLoader &model_loader,
                    const ecs::Entities &entities);

        void teardown(ecs::World &world);

    private:
        struct Impl;
        Impl *impl;
    };
}

#endif //HIGH_SHIFT_RENDER_SYSTEM_H
