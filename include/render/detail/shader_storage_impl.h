//
// Created by muldrik on 03.08.2021.
//

#ifndef HIGH_SHIFT_SHADER_STORAGE_IMPL_H
#define HIGH_SHIFT_SHADER_STORAGE_IMPL_H


#include <render/shader_storage.h>

namespace render {

    void ShaderTable::insert(ecs::Id id, Renderer &&renderer) {
        if (!contains(renderer.shader_program.id)) {
            dense.emplace_back();
            dense.back().shader_id = renderer.shader_program.id;
            sparse[renderer.shader_program.id] = dense.size() - 1;
        }
        dense[sparse[renderer.shader_program.id]].table[id] = renderer;
    }

    void ShaderTable::erase(ecs::Id shader_id, ecs::Id id) {
        dense[sparse[shader_id]].table.erase(id);
        if (dense.empty()) {
            erase_shader(shader_id);
        }
    }

    bool ShaderTable::contains(ecs::Id shader_id) {
        return sparse[shader_id] < dense.size() && dense[sparse[shader_id]].shader_id == shader_id;
    }

    void ShaderTable::erase_shader(ecs::Id shader_id) {
        sparse[dense.back().shader_id] = sparse[shader_id];
        std::swap(dense[sparse[shader_id]], (dense.back()));
        dense.pop_back();
    }


    void RendererStorage::insert(ecs::Id id, Renderer &&renderer) {
        mask.reserve(id + 1);
        mask.insert(id);
        info_table[id] = RendererInfo{renderer.is_static, renderer.is_transparent, renderer.shader_program.id};
        shader_tables[renderer.is_static][renderer.is_transparent].insert(id, std::forward<Renderer>(renderer));
    }

    void RendererStorage::erase(ecs::Id id) {
        if (!mask.contains(id))
            return;
        mask.erase(id);
        RendererInfo info = info_table[id];
        shader_tables[info.is_static][info.is_transparent].erase(info.shader_program_id, id);
    }

    bool RendererStorage::contains(ecs::Id id) const {
        return mask.contains(id);
    }

    std::size_t RendererStorage::size() const {
        return mask.size();
    }

    bool RendererStorage::empty() const {
        return mask.empty();
    }

    const ecs::IdSet &RendererStorage::present() const {
        return mask;
    }

    Renderer &RendererStorage::operator[](ecs::Id id) {
        auto info = info_table[id];
        return shader_tables[info.is_static][info.is_transparent].dense[info.shader_program_id].table[id];
    }

    const Renderer &RendererStorage::operator[](ecs::Id id) const {
        auto info = info_table[id];
        return shader_tables[info.is_static][info.is_transparent].dense[info.shader_program_id].table[id];
    }

    auto &RendererStorage::get_table() {
        return shader_tables;
    }

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
                while (shaders_pos == storage->shader_tables[staticness_pos][opacity_pos].dense.end()) {
                    ++opacity_pos;
                    if (opacity_pos == 2) {
                        opacity_pos = 0;
                        ++staticness_pos;
                    }
                    if (staticness_pos == 2) {
                        map_pos = MapIter();
                        shaders_pos = ShadersIter();
                        return *this;
                    }
                    shaders_pos = storage->shader_tables[staticness_pos][opacity_pos].dense.begin();
                }
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


    auto RendererStorage::get_first_unempty_map_iterator() {
        if (!shader_tables[0][0].dense.empty()) return shader_tables[0][0].dense.begin();
        if (!shader_tables[0][1].dense.empty()) return shader_tables[0][1].dense.begin();
        if (!shader_tables[1][0].dense.empty()) return shader_tables[1][0].dense.begin();
        if (!shader_tables[1][1].dense.empty()) return shader_tables[1][1].dense.begin();
        assert(false);
    }

    const auto RendererStorage::get_first_unempty_map_iterator() const {
        if (!shader_tables[0][0].dense.empty()) return shader_tables[0][0].dense.cbegin();
        if (!shader_tables[0][1].dense.empty()) return shader_tables[0][1].dense.cbegin();
        if (!shader_tables[1][0].dense.empty()) return shader_tables[1][0].dense.cbegin();
        if (!shader_tables[1][1].dense.empty()) return shader_tables[1][1].dense.cbegin();
        assert(false);
    }

    RendererStorage::iterator RendererStorage::begin() {
        if (empty()) return end();
        auto shader_pos = get_first_unempty_map_iterator();
        return iterator(this, 0, 0, shader_pos, shader_pos->table.begin());
    }

    RendererStorage::const_iterator RendererStorage::begin() const {
        if (empty()) return end();
        auto shader_pos = get_first_unempty_map_iterator();
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

#endif //HIGH_SHIFT_SHADER_STORAGE_IMPL_H
