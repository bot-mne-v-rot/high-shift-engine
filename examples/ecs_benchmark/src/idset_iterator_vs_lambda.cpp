#include "ecs/id_set.h"

#include <chrono>
#include <iostream>
#include <string_view>

using Clock = std::chrono::high_resolution_clock;

void black_box() {
    volatile int h = 0;
    if (h == 1) {
        std::cout << "This should not appear\n";
    }
}

Clock::duration bench_foreach(const ecs::IdSet &set) {
    Clock::time_point start = Clock::now();

    volatile ecs::Id h = 0;
    ecs::foreach(set, [&h](auto id) {
        h = id;
    });

    Clock::time_point end = Clock::now();
    return end - start;
}

Clock::duration bench_iterator(const ecs::IdSet &set) {
    Clock::time_point start = Clock::now();

    volatile ecs::Id h = 0;
    for (auto id : set) {
        h = id;
    }

    Clock::time_point end = Clock::now();
    return end - start;
}

Clock::duration bench_foreach_and(const ecs::IdSet &a, const ecs::IdSet &b) {
    Clock::time_point start = Clock::now();

    volatile ecs::Id h = 0;
    ecs::foreach(a & b, [&h](auto id) {
        h = id;
    });

    Clock::time_point end = Clock::now();
    return end - start;
}

Clock::duration bench_iterator_and(const ecs::IdSet &a, const ecs::IdSet &b) {
    Clock::time_point start = Clock::now();

    volatile ecs::Id h = 0;
    for (auto id : a & b)
        h = id;

    Clock::time_point end = Clock::now();
    return end - start;
}

struct AverageRes {
    uint64_t foreach_ns;
    uint64_t iter_ns;
    uint64_t set_sz;
    uint64_t set_cp;
};

AverageRes bench_iter_average(ecs::IdSet (*gen)()) {
    uint64_t iters = 10;
    uint64_t foreach_sum = 0;
    uint64_t iter_sum = 0;
    uint64_t set_sz_sum = 0;
    uint64_t set_cp_sum = 0;

    for (std::size_t i = 0; i < iters; ++i) {
        ecs::IdSet set = gen();
        set_sz_sum += set.size();
        set_cp_sum += set.capacity();
        foreach_sum += bench_foreach(set).count();
        iter_sum += bench_iterator(set).count();
    }

    return {
            foreach_sum / iters,
            iter_sum / iters,
            set_sz_sum / iters,
            set_cp_sum / iters
    };
}

AverageRes bench_and_average(ecs::IdSet (*gen)()) {
    uint64_t iters = 10;
    uint64_t foreach_sum = 0;
    uint64_t iter_sum = 0;
    uint64_t set_sz_sum = 0;
    uint64_t set_cp_sum = 0;

    for (std::size_t i = 0; i < iters; ++i) {
        ecs::IdSet a = gen();
        ecs::IdSet b = gen();
        set_sz_sum += a.size();
        set_cp_sum += a.capacity();
        set_sz_sum += b.size();
        set_cp_sum += b.capacity();
        foreach_sum += bench_foreach_and(a, b).count();
        iter_sum += bench_iterator_and(a, b).count();
    }

    return {
        foreach_sum / iters,
        iter_sum / iters,
        set_sz_sum / iters / 2,
        set_cp_sum / iters / 2
    };
}

using Gen = ecs::IdSet (*)();
using Bencher = AverageRes (*)(Gen);

void bench(std::string_view set_name, Gen gen, Bencher bencher) {
    auto[foreach_ns, iter_ns, set_sz, set_cp] = bencher(gen);

    std::cout << "Benchmark \"" << set_name << "\":" << std::endl;
    std::cout << "...."
              << "average set size: " << set_sz << ", "
              << "average set capacity: " << set_cp << std::endl;

    std::cout << "...." << "Foreach:" << std::endl;
    float foreach_ns_per_elem = (float) foreach_ns / set_sz;
    std::cout << "........"
              << "overall: " << foreach_ns << " ns, "
              << "per elem: " << foreach_ns_per_elem << " ns" << std::endl;

    std::cout << "...." << "Iterator:" << std::endl;
    float iter_ns_per_elem = (float) iter_ns / set_sz;
    std::cout << "........"
              << "overall: " << iter_ns << " ns, "
              << "per elem: " << iter_ns_per_elem << " ns" << std::endl;

}

ecs::IdSet big_dense() {
    std::size_t max_size = ecs::IdSet::max_size;
    std::size_t size = 10000000;

    ecs::IdSet set;
    set.reserve(max_size);

    srand(time(0));
    for (std::size_t i = 0; i < size; ++i)
        set.insert(rand() % size);

    return set;
}

ecs::IdSet big_sparse() {
    std::size_t max_size = ecs::IdSet::max_size;
    std::size_t size = 10000;

    ecs::IdSet set;
    set.reserve(max_size);

    srand(time(0));
    for (std::size_t i = 0; i < size; ++i)
        set.insert(rand() % size);

    return set;
}

ecs::IdSet big_block() {
    std::size_t max_size = ecs::IdSet::max_size;
    std::size_t size = 1000000;

    ecs::IdSet set;
    set.reserve(max_size);

    srand(time(0));
    std::size_t shift = rand() % size;
    for (std::size_t i = 0; i < size; ++i)
        set.insert(shift + i);

    return set;
}

ecs::IdSet small_sparse() {
    std::size_t max_size = ecs::IdSet::max_size;
    std::size_t size = 1000;

    ecs::IdSet set;
    set.reserve(max_size);

    srand(time(0));
    for (std::size_t i = 0; i < size; ++i)
        set.insert(rand() % size);

    return set;
}

ecs::IdSet small_dense() {
    std::size_t max_size = 2000;
    std::size_t size = 1000;

    ecs::IdSet set;
    set.reserve(max_size);

    srand(time(0));
    for (std::size_t i = 0; i < size; ++i)
        set.insert(rand() % size);

    return set;
}

int main() {
    bench("big sparse iteration", big_sparse, bench_iter_average);
    bench("big dense iteration", big_dense, bench_iter_average);
    bench("small sparse iteration", small_sparse, bench_iter_average);
    bench("small dense iteration", small_dense, bench_iter_average);
    bench("big block iteration", big_block, bench_iter_average);

    bench("big sparse iteration over &", big_sparse, bench_and_average);
    bench("big dense iteration over &", big_dense, bench_and_average);
    bench("small sparse iteration over &", small_sparse, bench_and_average);
    bench("small dense iteration over &", small_dense, bench_and_average);
    bench("big block iteration over &", big_block, bench_and_average);
}