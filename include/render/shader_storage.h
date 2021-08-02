#ifndef HIGH_SHIFT_SHADER_STORAGE_H
#define HIGH_SHIFT_SHADER_STORAGE_H

#include "ecs/component.h"
#include "ecs/storage.h"
#include "ecs/id_set.h"
#include "common/HashMap.h"


#include <vector>

namespace render {
    struct RendererStorage;


    struct ShaderProgram {
        ecs::Id id{};
    };

    struct Renderer {
        using Storage = RendererStorage;
        ShaderProgram shader_program;
        bool is_static = false;
        bool is_transparent = false;
        //std::vector<Texture2d> textures;
        // ...
    };

    struct RendererInfo {
        bool is_static = false;
        bool is_transparent = false;
        ecs::Id shader_program_id{};
    };

    //typedef std::unordered_map<Id, Renderer> IdTable;
    typedef HashMap<ecs::Id, Renderer> IdTable;
    typedef HashMap<ecs::Id, RendererInfo> IdInfoTable;

    struct IdTableDenseEntry {
        std::size_t shader_id;
        IdTable table;
    };

    struct ShaderTable {
        static const std::size_t max_shaders = 1024;
        std::size_t sparse[max_shaders]{};
        std::vector<IdTableDenseEntry> dense;

        void insert(ecs::Id id, Renderer &&renderer) {
            dense.emplace_back();
            sparse[renderer.shader_program.id] = dense.size() - 1;
            dense.back().table[id] = renderer;
        }

        void erase(ecs::Id shader_id, ecs::Id id) {
            dense[sparse[shader_id]].table.erase(id);
            if (dense.empty()) {
                erase_shader(shader_id);
            }
        }

        void erase_shader(ecs::Id shader_id) {
            sparse[dense.back().shader_id] = sparse[shader_id];
            std::swap(dense[sparse[shader_id]], (dense.back()));
            dense.pop_back();
        }

    };

    struct RendererStorage {
        using value_type = Renderer;
        using reference = Renderer &;
        using const_reference = const Renderer &;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;

        template<typename Ref, typename Ptr, typename ShadersIter, typename MapIter>
        class iterator_template;

        using iterator = iterator_template<reference, Renderer *, std::vector<IdTableDenseEntry>::iterator, HashMap<ecs::Id, Renderer>::iterator>;
        using const_iterator = iterator_template<const_reference, const Renderer *, std::vector<IdTableDenseEntry>::const_iterator,HashMap<ecs::Id, Renderer>::const_iterator >;

        void insert(ecs::Id id, Renderer &&renderer) {
            mask.reserve(id + 1);
            mask.insert(id);
            shader_tables[renderer.is_static][renderer.is_transparent].insert(id, std::forward<Renderer>(renderer));
        }

        void erase(ecs::Id id) {
            if (!mask.contains(id))
                return;
            mask.erase(id);
            RendererInfo info = info_table[id];
            shader_tables[info.is_static][info.is_transparent].erase(info.shader_program_id, id);
        }

        bool contains(ecs::Id id) const {
            return mask.contains(id);
        }

        std::size_t size() const {
            return mask.size();
        }

        bool empty() const {
            return mask.empty();
        }

        const ecs::IdSet& present() const {
            return mask;
        }

        Renderer& operator[](ecs::Id id) {
            auto info = info_table[id];
            return shader_tables[info.is_static][info.is_transparent].dense[info.shader_program_id].table[id];
        }

        const Renderer& operator[](ecs::Id id) const{
            auto info = info_table[id];
            return shader_tables[info.is_static][info.is_transparent].dense[info.shader_program_id].table[id];
        }

        iterator begin();

        const_iterator begin() const;

        const_iterator cbegin() const;

        iterator end();
        const_iterator end() const;
        const_iterator cend() const;

        ecs::WithIdView<iterator, const_iterator> with_id();
        ecs::WithIdView<const_iterator, const_iterator> with_id() const;

        ShaderTable shader_tables[2][2]{}; // по каждому ключу
        IdInfoTable info_table;
        ecs::IdSet mask;
    };

    template<typename Ref, typename Ptr, typename ShadersIter, typename MapIter>
    class RendererStorage::iterator_template {
    public:
        using reference = Ref;
        using pointer = Ptr;
        using value_type = Renderer;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        iterator_template() = default;

        iterator_template(const iterator_template &) = default;

        iterator_template &operator=(const iterator_template &) = default;

        reference operator*() const {
            return map_pos->second;
        }

        pointer operator->() const {
            return &map_pos->second;
        }

        iterator_template &operator++() {
            ++map_pos;
            if (map_pos == shaders_pos->table.end()) {
                ++shaders_pos;
                if (shaders_pos == storage->shader_tables[staticness_pos][opacity_pos].dense.end()) {
                    ++opacity_pos;
                    if (opacity_pos == 2) {
                        opacity_pos = 0;
                        ++staticness_pos;
                        return *this;
                    }
                    if (staticness_pos == 2)
                        shaders_pos = ShadersIter();
                    else
                        shaders_pos = storage->shader_tables[staticness_pos][opacity_pos].dense.begin();
                }
                if (staticness_pos == 2)
                    map_pos = MapIter();
                else
                    map_pos = shaders_pos->table.begin();
            }
            return *this;
        }

        iterator_template operator++(int) {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        ecs::Id id() const {
            return map_pos->first;
        }

        bool operator==(const iterator_template &other) const = default;

        bool operator!=(const iterator_template &other) const = default;

        operator const_iterator() const {
            return const_iterator {storage, opacity_pos, staticness_pos, shaders_pos, map_pos};
        }

    private:

        uint8_t staticness_pos = 0;
        uint8_t opacity_pos = 0;


        RendererStorage *storage;

        /*using ShadersIter = std::vector<IdTableDenseEntry>::iterator;
        using MapIter = HashMap<ecs::Id, Renderer>::iterator;*/

        ShadersIter shaders_pos;
        MapIter map_pos;


        friend RendererStorage;
        iterator_template(RendererStorage *storage_ptr, uint8_t staticness_pos, uint8_t opacity_pos,
                          ShadersIter shaders_pos, MapIter map_pos)
                : staticness_pos(staticness_pos),
                  opacity_pos(opacity_pos),
                  storage(storage_ptr),
                  shaders_pos(shaders_pos),
                  map_pos(map_pos) {}
    };

    RendererStorage::iterator RendererStorage::begin() {
        auto shader_pos = shader_tables[0][0].dense.begin();
        return iterator(this, 0, 0, shader_pos, shader_pos->table.begin());
    }

    RendererStorage::const_iterator RendererStorage::begin() const {
        auto shader_pos = shader_tables[0][0].dense.cbegin();
        return const_iterator(const_cast<RendererStorage*>(this), 0, 0, shader_pos, shader_pos->table.cbegin());
    }

    RendererStorage::const_iterator RendererStorage::cbegin() const {
        return begin();
    }

    RendererStorage::iterator RendererStorage::end() {
        return iterator(this, 2, 0, std::vector<IdTableDenseEntry>::iterator(), HashMap<ecs::Id, Renderer>::iterator());
    }

    RendererStorage::const_iterator RendererStorage::end() const {
        return const_iterator(const_cast<RendererStorage*>(this), 2, 0, std::vector<IdTableDenseEntry>::const_iterator(), HashMap<ecs::Id, Renderer>::const_iterator());
    }

    RendererStorage::const_iterator RendererStorage::cend() const {
        return end();
    }

    auto RendererStorage::with_id() -> ecs::WithIdView<iterator, const_iterator> {
        return { begin(), end() };
    }

    auto RendererStorage::with_id() const -> ecs::WithIdView<const_iterator, const_iterator> {
        return { begin(), end() };
    }


}


#endif //HIGH_SHIFT_SHADER_STORAGE_H
