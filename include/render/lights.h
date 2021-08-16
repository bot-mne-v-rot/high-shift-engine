#ifndef HIGH_SHIFT_LIGHTS_H
#define HIGH_SHIFT_LIGHTS_H

#include "ecs/storages/sparse_vec_storage.h"

namespace render {

    struct DirLight {
        using Storage = ecs::SparseVecStorage<DirLight>;
        glm::vec3 direction{0.0f, -1.0f, 0.0f};

        glm::vec3 ambient{0.2f, 0.2f, 0.2f};
        glm::vec3 diffuse{0.5f, 0.5f, 0.5f};
        glm::vec3 specular{1.0f, 1.0f, 1.0f};
    };

    struct PointLight {
        using Storage = ecs::SparseVecStorage<PointLight>;
        float constant;
        float linear;
        float quadratic;

        glm::vec3 ambient{0.2f, 0.2f, 0.2f};
        glm::vec3 diffuse{0.5f, 0.5f, 0.5f};
        glm::vec3 specular{1.0f, 1.0f, 1.0f};
    };

    struct SpotLight {
        using Storage = ecs::SparseVecStorage<SpotLight>;
        glm::vec3 direction;
        float cutOff;
        float outerCutOff;

        float constant = 1.0f;
        float linear = 0.09f;
        float quadratic = 0.032;

        glm::vec3 ambient{0.2f, 0.2f, 0.2f};
        glm::vec3 diffuse{0.5f, 0.5f, 0.5f};
        glm::vec3 specular{1.0f, 1.0f, 1.0f};
    };
}


#endif //HIGH_SHIFT_LIGHTS_H
