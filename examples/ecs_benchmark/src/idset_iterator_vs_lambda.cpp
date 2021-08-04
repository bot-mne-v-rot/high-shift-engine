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

struct AverageRes {
    uint64_t foreach_ns;
    uint64_t iter_ns;
    uint64_t set_sz;
    uint64_t set_cp;
};

AverageRes bench_average(ecs::IdSet (*gen)()) {
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

void bench(std::string_view set_name, ecs::IdSet (*gen)()) {
    auto[foreach_ns, iter_ns, set_sz, set_cp] = bench_average(gen);

    std::cout << "Benchmark \"" << set_name << "\":" << std::endl;
    std::cout << "...."
              << "average set size: " << set_sz << ", "
              << "average set capacity: " << set_cp << std::endl;

    std::cout << "...." << "Foreach:" << std::endl;
    float foreach_ns_per_elem = (float) foreach_ns / set_sz;
    std::cout << "........"
              << "overall: " << foreach_ns << " ns , "
              << "per elem: " << foreach_ns_per_elem << " ns" << std::endl;

    std::cout << "...." << "Iterator:" << std::endl;
    float iter_ns_per_elem = (float) iter_ns / set_sz;
    std::cout << "........"
              << "overall: " << iter_ns << " ns , "
              << "per elem: " << iter_ns_per_elem << " ns" << std::endl;

}

ecs::IdSet big_sparse() {
    std::size_t max_size = ecs::IdSet::max_size;
    std::size_t size = 1000000;

    ecs::IdSet set;
    set.reserve(max_size);

    srand(time(0));
    for (std::size_t i = 0; i < size; ++i)
        set.insert(rand() % size);

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
    bench("big sparse", big_sparse);
    bench("small sparse", small_sparse);
    bench("small dense", small_dense);
}