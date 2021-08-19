#ifndef HIGH_SHIFT_SOME_COMPONENTS_H
#define HIGH_SHIFT_SOME_COMPONENTS_H

namespace {
    struct alignas(64) CacheLineAligned1 {
        int a, b, c;
    };

    struct alignas(64) CacheLineAligned2 {
        int a[100];
    };

    struct Position {
        float x, y, z;
    };

    struct SomeComponent {
        int x[10];
        double g;
        float t[3];
    };
}

#endif //HIGH_SHIFT_SOME_COMPONENTS_H
