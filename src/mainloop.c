#include "mainloop.h"
#include "ptime.h"

void mainloop(const mainloop_params* params)
{
    const float msecs_per_update = 1000.0f / params->updates_per_sec;
    const uint64_t ticks_per_update = time_ticks(msecs_per_update);

    float interpolation;
    uint64_t previous, current, elapsed, lag = 0;

    previous = time_now();
    while (!params->should_terminate) {
        current = time_now();
        elapsed = time_diff(current, previous);
        previous = current;
        lag += elapsed;

        while (lag > ticks_per_update) {
            params->update_callback(params->userdata, 1.0f / params->updates_per_sec);
            lag -= ticks_per_update;
        }

        interpolation = (float)lag / ticks_per_update;
        params->render_callback(params->userdata, interpolation);
    }
}
