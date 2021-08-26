#ifndef HIGH_SHIFT_CHUNKS_MAP_H
#define HIGH_SHIFT_CHUNKS_MAP_H

#include "ecs/component.h"
#include "ecs/utils.h"

#include <vector>
#include <unordered_map>

namespace ecs {
    using SharedComponentsVector = std::vector<ShCompVal>;
}

namespace std {
    template<>
    struct hash<ecs::SharedComponentsVector> {
        std::size_t operator()(const ecs::SharedComponentsVector &values) const {
            constexpr std::size_t base = 12345;
            std::size_t hash = 0;
            for (auto val : values)
                hash = base * hash + val;
            return hash;
        }
    };
}

namespace ecs {
    // Polynomial hash. Mod is 2^64.
    inline std::size_t hash_shared_components_values(std::size_t shared_components_count,
                                                     std::size_t index_in_arrays,
                                                     const ShCompVal **values) {
        constexpr std::size_t base = 12345;
        std::size_t hash = 0;
        for (std::size_t i = 0; i < shared_components_count; ++i)
            hash = base * hash + values[i][index_in_arrays];
        return hash;
    }

    class ChunksMap {
    public:
//        Chunk *try_get(std::size_t shared_components_count,
//                       std::size_t index_in_arrays,
//                       const ShCompVal *const *values) {
//
//            auto vec = from_values(shared_components_count,
//                                   index_in_arrays, values);
//
//            return detail::get_else_update(map, vec, nullptr);
//        }
//
//        void insert_or_assign(std::size_t shared_components_count,
//                              std::size_t index_in_arrays,
//                              const ShCompVal *const *values,
//                              Chunk *chunk) {
//            auto vec = from_values(shared_components_count,
//                                   index_in_arrays, values);
//            map[vec] = chunk;
//        }

        Chunk *try_get(const SharedComponentsVector &key) {
            return detail::get_else_update(map, key, nullptr);
        }

        void insert_or_assign(const SharedComponentsVector &key,
                              Chunk *chunk) {
            map[key] = chunk;
        }

    private:
//        static SharedComponentsVector from_values(std::size_t shared_components_count,
//                                                  std::size_t index_in_arrays,
//                                                  const ShCompVal *const *values) {
//            SharedComponentsVector vector(shared_components_count);
//            for (std::size_t i = 0; i < shared_components_count; ++i)
//                vector[i] = values[i][index_in_arrays];
//            return vector;
//        }

        std::unordered_map<SharedComponentsVector, Chunk *> map{};
    };
}


#endif //HIGH_SHIFT_CHUNKS_MAP_H
