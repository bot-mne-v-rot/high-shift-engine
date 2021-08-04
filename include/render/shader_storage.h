#ifndef HIGH_SHIFT_SHADER_STORAGE_H
#define HIGH_SHIFT_SHADER_STORAGE_H

#include "ecs/component.h"
#include "ecs/id_set.h"
#include "common/HashMap.h"


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
    };

    struct RendererInfo {
        bool is_static = false;
        bool is_transparent = false;
        ecs::Id shader_program_id{};
    };

    typedef HashMap<ecs::Id, Renderer> IdTable;
    typedef HashMap<ecs::Id, RendererInfo> IdInfoTable;


    struct IdTableDenseEntry {
        ecs::Id shader_id;
        IdTable table;
    };

    struct ShaderTable {
        static const std::size_t max_shaders = 1024;
        std::size_t sparse[max_shaders]{};
        std::vector<IdTableDenseEntry> dense;

        void insert(ecs::Id id, Renderer &&renderer);
        void erase(ecs::Id shader_id, ecs::Id id);
        bool contains(ecs::Id shader_id);
        void erase_shader(ecs::Id shader_id);

    };

    struct RendererStorage {
    public:
        using value_type = Renderer;
        using reference = Renderer &;
        using const_reference = const Renderer &;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;

        template<typename Ref, typename Ptr, typename ShadersIter, typename MapIter>
        class iterator_template;

        using iterator = iterator_template<reference, Renderer *, std::vector<IdTableDenseEntry>::iterator, HashMap<ecs::Id, Renderer>::iterator>;
        using const_iterator = iterator_template<const_reference, const Renderer *, std::vector<IdTableDenseEntry>::const_iterator,HashMap<ecs::Id, Renderer>::const_iterator >;

        void insert(ecs::Id id, Renderer &&renderer);
        void erase(ecs::Id id);

        std::size_t size() const;
        bool empty() const;

        bool contains(ecs::Id id) const;
        const ecs::IdSet& present() const;

        Renderer& operator[](ecs::Id id);
        const Renderer& operator[](ecs::Id id) const;

        auto& get_table();
        auto get_first_unempty_map_iterator();
        const auto get_first_unempty_map_iterator() const;

        iterator begin();
        const_iterator begin() const;
        const_iterator cbegin() const;

        iterator end();
        const_iterator end() const;
        const_iterator cend() const;

        ecs::WithIdView<iterator, const_iterator> with_id();
        ecs::WithIdView<const_iterator, const_iterator> with_id() const;

    private:
        ShaderTable shader_tables[2][2]{}; // по каждому ключу
        IdInfoTable info_table;
        ecs::IdSet mask;
    };

}

#include "detail/shader_storage_impl.h"


#endif //HIGH_SHIFT_SHADER_STORAGE_H
