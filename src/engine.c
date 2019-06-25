#include "engine.h"
#include <stdlib.h>
#include "mainloop.h"

struct engine {
    mainloop_params ml_params;
};

engine engine_create(const engine_params* params)
{
    (void) params;
    engine e = calloc(1, sizeof(*e));
    return e;
}

void engine_update(engine e, float dt)
{
    (void) e;
    (void) dt;
}

void engine_render(engine e, float dt)
{
    (void) e;
    (void) dt;
}

void engine_run(engine e)
{
    e->ml_params = (mainloop_params){
        .update_callback = (mainloop_update_fn) engine_update,
        .render_callback = (mainloop_render_fn) engine_render,
        .updates_per_sec = 60,
        .userdata = e
    };
    mainloop(&e->ml_params);
}

void engine_stop(engine e)
{
    e->ml_params.should_terminate = 1;
}

void engine_destroy(engine e)
{
    free(e);
}
