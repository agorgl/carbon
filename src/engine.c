#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include "mainloop.h"
#include "window.h"
#include "opengl.h"
#include "renderer.h"
#include "ecs.h"

struct engine {
    mainloop_params ml_params;
    window wnd;
    renderer renderer;
    ecs_world_t* world;
};

static void on_opengl_error(void* userdata, const char* msg)
{
    engine e = userdata;
    fprintf(stderr, "%s\n", msg);
    engine_stop(e);
}

static void on_key(window wnd, int key, int scancode, int action, int mods)
{
    (void)scancode; (void)mods;
    engine e = window_get_userdata(wnd);
    if (action == KEY_ACTION_RELEASE && key == KEY_ESCAPE)
        engine_stop(e);
}

engine engine_create(const engine_params* params)
{
    (void) params;
    engine e = calloc(1, sizeof(*e));
    const int width = 1280, height = 720;

    /* Create window */
    window wnd = window_create((window_params){
        .title  = "Carbon",
        .width  = width,
        .height = height,
        .mode   = 0,
        .ctx_params = {
            .type    = OPENGL,
            .version = { 3, 3 },
            .debug   = 1
        }
    });
    window_set_userdata(wnd, e);
    window_set_callbacks(wnd, (window_callbacks){
        .key_cb = on_key
    });
    e->wnd = wnd;

    /* Load OpenGL extensions */
    opengl_load_extensions(window_get_proc_address);

    /* Setup OpenGL debug handler */
    opengl_register_error_handler((opengl_error_handler_params){
        .callback = on_opengl_error,
        .userdata = e
    });

    /* Create renderer instance */
    e->renderer = renderer_create(&(renderer_params){
        .width  = width,
        .height = height
    });

    /* Create world instance */
    e->world = ecs_init();
    ecs_setup_internal(e->world);

    return e;
}

ecs_world_t* engine_world(engine e)
{
    return e->world;
}

void engine_update(engine e, float dt)
{
    /* Poll events, and call event callbacks */
    window_update(e->wnd);

    /* Fire ecs systems */
    ecs_progress(e->world, dt);
}

void engine_render(engine e, float dt)
{
    (void) dt;

    /* Gather data needed by renderer from the ecs */
    renderer_inputs ri = {};
    ecs_prepare_renderer_inputs(e->world, &ri);

    /* Render the frame */
    renderer_frame(e->renderer, ri);

    /* Free intermediate renderer input data */
    ecs_free_render_inputs(e->world, &ri);

    /* Show backbuffer */
    window_swap_buffers(e->wnd);
}

void engine_run(engine e)
{
    /* Run main loop */
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
    /* Destroy world instance */
    ecs_fini(e->world);

    /* Destroy renderer instance */
    renderer_destroy(e->renderer);

    /* Close window */
    window_destroy(e->wnd);

    /* Free engine instance */
    free(e);
}
