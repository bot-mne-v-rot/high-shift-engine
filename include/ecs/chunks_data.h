#ifndef HIGH_SHIFT_CHUNKS_DATA_H
#define HIGH_SHIFT_CHUNKS_DATA_H

#include "ecs/chunk.h"
#include "ecs/component.h"

#include <memory>
#include <vector>
#include <cassert>

namespace ecs {
    class ChunksData {
    public:
        explicit ChunksData(std::size_t shared_components_count)
                : shared_components_count(shared_components_count) {}

        ~ChunksData() {
            for (std::size_t i = 0; i < sz; ++i)
                delete chunks_ptr()[i];
//            std::free(data);
        }

        std::size_t chunks_count() const {
            return sz;
        }

        std::size_t capacity() const {
            return cp;
        }

        void reserve(std::size_t new_capacity) {
            if (new_capacity <= cp)
                return;

            std::size_t new_cp = 1;
            while (new_cp < new_capacity) new_cp *= 2;

//
            data.resize(new_cp);
            cp = new_cp;
//            std::size_t prev_chunks_ptr_size = sizeof(Chunk *) * cp;
//            std::size_t new_chunks_ptr_size = sizeof(Chunk *) * new_cp;
//            constexpr std::size_t align = alignof(Chunk *);
//
//            std::size_t new_data_size = new_chunks_ptr_size;
//            // the size must be an integral multiple of the alignment
//            new_data_size = (new_data_size + align - 1) / align * align;
//
//            auto *new_data = (uint8_t *) std::aligned_alloc(align, new_data_size);
//            if (prev_chunks_ptr_size)
//                memcpy(new_data, data, prev_chunks_ptr_size);
//            std::free(data);
//
//            data = new_data;
//            cp = new_cp;
        }

        // Chunk *chunks_ptr[cp]
        Chunk **chunks_ptr() {
            return data.data();
        }

        Chunk *const *chunks_ptr() const {
            return data.data();
        }

        std::size_t chunks_ptr_size() const {
            return sizeof(Chunk *) * cp;
        }

        void swap_chunks(std::size_t first_index, std::size_t second_index) {
            Chunk **chunks = chunks_ptr();
            std::swap(chunks[first_index]->fields.chunk_index,
                      chunks[second_index]->fields.chunk_index);
            std::swap(chunks[first_index], chunks[second_index]);
        }

        void push_chunk() {
            reserve(sz + 1);
            Chunk **chunks = chunks_ptr();

            chunks[sz] = new Chunk;
            assert(chunks[sz]);
            chunks[sz]->fields.chunk_index = sz;
            ++sz;
        }

        void pop_chunk() {
            delete chunks_ptr()[--sz];
        }

        Chunk *last_chunk() const {
            return chunks_ptr()[sz - 1];
        }

    private:
        std::size_t sz = 0;
        std::size_t cp = 0;

        std::size_t shared_components_count;

//        uint8_t *data = nullptr;
        std::vector<Chunk *> data;
    };
}

#endif //HIGH_SHIFT_CHUNKS_DATA_H
