#ifndef HIGH_SHIFT_GAME_LOOP_CONTROL_H
#define HIGH_SHIFT_GAME_LOOP_CONTROL_H

#include <string>

namespace ecs {
    /**
     * Special resource to control game loop.
     */
    struct GameLoopControl {
    public:
        void stop() {
            _stopped = true;
        }

        bool stopped() const {
            return _stopped;
        }

    private:
        bool _stopped = false;
    };
}

#endif //HIGH_SHIFT_GAME_LOOP_CONTROL_H
