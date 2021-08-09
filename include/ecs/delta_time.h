#ifndef HIGH_SHIFT_DELTA_TIME_H
#define HIGH_SHIFT_DELTA_TIME_H

#include <chrono>

namespace ecs {
    class Dispatcher;

    class DeltaTime {
    public:
        float operator()() const {
            return delta_time;
        }

    private:
        friend Dispatcher;
        using Clock = std::chrono::high_resolution_clock;

        float delta_time = 0.0f;
        Clock::time_point prev;
        bool first_update = true;

        void update() {
            Clock::time_point now = Clock::now();
            if (!first_update) {
                delta_time = (float) (now - prev).count() / 1e9f;
                prev = now;
            } else {
                first_update = false;
            }
        }
    };
}

#endif //HIGH_SHIFT_DELTA_TIME_H
