#include "mainloop.h"
#include "ptime.h"

#define timeit(var) \
    for (uint64_t _break = 1, tp = time_now();   \
         _break || ((var = time_now() - tp), 0); \
         _break = 0)

void mainloop(const mainloop_params* params, mainloop_perf_data* perf_data)
{
    const float msecs_per_update = 1000.0f / params->updates_per_sec;
    const uint64_t ticks_per_update = time_ticks(msecs_per_update);

    float interpolation;
    uint64_t previous, current, elapsed, lag = ticks_per_update;
    uint64_t update_time, render_time;

    previous = time_now();
    while (!params->should_terminate) {
        current = time_now();
        elapsed = time_diff(current, previous);
        previous = current;
        lag += elapsed;

        timeit(update_time) {
            while (lag >= ticks_per_update) {
                params->update_callback(params->userdata, 1.0f / params->updates_per_sec);
                lag -= ticks_per_update;
            }
        }

        interpolation = (float)lag / ticks_per_update;
        timeit(render_time)
            params->render_callback(params->userdata, interpolation);

        if (perf_data) {
            struct {
                struct mainloop_perf_counter* perfc;
                float sample;
            } pu[] = {
                {.perfc = &perf_data->update, .sample = update_time / 1e6},
                {.perfc = &perf_data->render, .sample = render_time / 1e6},
                {.perfc = &perf_data->total,  .sample = elapsed / 1e6},
            };
            for (size_t i = 0; i < sizeof(pu) / sizeof(pu[0]); ++i) {
                struct mainloop_perf_counter* perfc = pu[i].perfc;
                perfc->samples[perfc->cur_sample++] = pu[i].sample;
                if (perfc->cur_sample >= MAX_PERF_SAMPLES)
                    perfc->cur_sample = 0;
                if (perfc->num_samples < MAX_PERF_SAMPLES)
                    ++perfc->num_samples;

                float sum = 0.0f;
                for (size_t j = 0; j < perfc->num_samples; ++j)
                    sum += perfc->samples[j];
                perfc->average = sum / perfc->num_samples;
            }
        }
    }
}
