#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include "mainloop.h"
#include "window.h"
#include "opengl.h"
#include "renderer.h"
#include "camera.h"
#include "ecs.h"
#include "embedded.h"
#include "text.h"

#define FONT_INTERNAL "fonts/noto_mono.ttf"

struct engine {
    engine_params params;
    mainloop_params ml_params;
    mainloop_perf_data ml_perf_data;
    window wnd;
    renderer renderer;
    resmngr rmgr;
    ecs_world_t* world;
    camera cam;
    text_renderer text_renderer;
    rid font;
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
    else if (action == KEY_ACTION_RELEASE && key == KEY_RIGHT_CONTROL)
        window_grub_cursor(wnd, 0);
}

static void on_mouse_button(struct window* wnd, int button, int action, int mods)
{
    (void)mods;
    if (action == KEY_ACTION_RELEASE && button == MOUSE_BUTTON_LEFT)
        window_grub_cursor(wnd, 1);
}

engine engine_create(const engine_params* params)
{
    engine e = calloc(1, sizeof(*e));
    e->params = *params;

    /* Create window */
    window wnd = window_create((window_params){
        .title  = "Carbon",
        .width  = params->width,
        .height = params->height,
        .mode   = 0,
        .ctx_params = {
            .type    = OPENGL,
            .version = { 3, 3 },
            .debug   = 1
        }
    });
    window_set_userdata(wnd, e);
    window_set_callbacks(wnd, (window_callbacks){
        .key_cb          = on_key,
        .mouse_button_cb = on_mouse_button
    });
    e->wnd = wnd;

    /* Load OpenGL extensions */
    opengl_load_extensions(window_get_proc_address);

    /* Setup OpenGL debug handler */
    opengl_register_error_handler((opengl_error_handler_params){
        .callback = on_opengl_error,
        .userdata = e
    });

    /* Get default framebuffer size */
    int fbwidth, fbheight;
    window_get_framebuffer_size(e->wnd, &fbwidth, &fbheight);

    /* Create renderer instance */
    e->renderer = renderer_create(&(renderer_params){
        .width  = fbwidth,
        .height = fbheight
    });

    /* Create resource manager instance */
    e->rmgr = resmngr_create();

    /* Create world instance */
    e->world = ecs_init();
    ecs_setup_internal(e->world);

    /* Create text renderer instance */
    e->text_renderer = text_renderer_create();

    /* Load main font */
    void* font_data; size_t font_sz;
    embedded_file(&font_data, &font_sz, FONT_INTERNAL);
    e->font = resmngr_font_from_ttf_data(e->rmgr, font_data, font_sz);

    return e;
}

resmngr engine_resmngr(engine e)
{
    return e->rmgr;
}

ecs_world_t* engine_world(engine e)
{
    return e->world;
}

static void camera_control_update(engine e, camera* cam, float dt)
{
    /* Update camera position */
    int cam_mov_flags = 0x0;
    if (window_key_state(e->wnd, KEY_W) == KEY_ACTION_PRESS)
        cam_mov_flags |= CAMERA_MOVE_FORWARD;
    if (window_key_state(e->wnd, KEY_A) == KEY_ACTION_PRESS)
        cam_mov_flags |= CAMERA_MOVE_LEFT;
    if (window_key_state(e->wnd, KEY_S) == KEY_ACTION_PRESS)
        cam_mov_flags |= CAMERA_MOVE_BACKWARD;
    if (window_key_state(e->wnd, KEY_D) == KEY_ACTION_PRESS)
        cam_mov_flags |= CAMERA_MOVE_RIGHT;
    camera_move(cam, cam_mov_flags, dt);

    /* Update camera look */
    float cur_diff_x = 0, cur_diff_y = 0;
    window_get_cursor_diff(e->wnd, &cur_diff_x, &cur_diff_y);
    if (window_is_cursor_grubbed(e->wnd))
        camera_look(cam, cur_diff_x, cur_diff_y, dt);
}

static camera* pick_active_camera(engine e)
{
    camera* cams[ECS_MAX_CAMERAS]; size_t num_cams;
    ecs_fetch_cameras(e->world, (void*)cams, &num_cams);
    camera* camera = num_cams != 0 ? cams[0] : 0;
    return camera;
}

void engine_update(engine e, float dt)
{
    /* Process pending loading operations */
    resmngr_process(e->rmgr);

    /* Pick active camera reference */
    camera* cam = pick_active_camera(e);
    if (cam) {
        /* Update active camera and store it */
        camera_control_update(e, cam, dt);
        e->cam = *cam;
    } else {
        /* Fallback in case no camera is defined */
        camera_defaults(&e->cam);
        camera_setpos(&e->cam, vec3_new(0.0f, 1.5f, 6.0f));
        camera_setdir(&e->cam, vec3_sub(vec3_zero(), e->cam.pos));
        camera_update(&e->cam, dt);
    }

    /* Poll events, and call event callbacks */
    window_update(e->wnd);

    /* Fire ecs systems */
    ecs_progress(e->world, dt);
}

static void engine_render_perf_info(engine e)
{
    /* Alias screen width and height */
    const int width = e->params.width, height = e->params.height;

    /* Construct perf text */
    char perf_text[256];
    float msec = e->ml_perf_data.total.average;
    float updt = e->ml_perf_data.update.average;
    float rndt = e->ml_perf_data.render.average;
    snprintf(perf_text,
             sizeof(perf_text),
             "%.0f FPS %.2f|%.2f|%.2f (CPU|GPU|TOT)",
             1000.0f / msec, updt, rndt, msec);

    /* Render perf text */
    float aspect_ratio = (float)width/height;
    float padding = 0.01;
    float tscl = 1.0;
    text_draw(e->text_renderer, &(text_draw_desc){
        .text       = perf_text,
        .fnt        = resmngr_font_lookup(e->rmgr, e->font),
        .pos        = {{-1.0 + padding, 1.0 - padding * aspect_ratio, 0.0}},
        .rot        = quat_id(),
        .scl        = {{tscl, tscl, tscl}},
        .halign     = TEXT_HALIGN_LEFT,
        .valign     = TEXT_VALIGN_TOP,
        .scr_width  = width,
        .scr_height = height,
    });
}

void engine_render(engine e, float dt)
{
    (void) dt;

    /* Gather data needed by renderer from the ecs */
    renderer_inputs ri = {.view = camera_view(&e->cam)};
    ecs_prepare_renderer_inputs(e->world, &ri, e->rmgr);

    /* Render the frame */
    renderer_frame(e->renderer, ri);

    /* Free intermediate renderer input data */
    ecs_free_render_inputs(e->world, &ri);

    /* Render performance info */
    engine_render_perf_info(e);

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
    e->ml_perf_data = (mainloop_perf_data){};
    mainloop(&e->ml_params, &e->ml_perf_data);
}

void engine_stop(engine e)
{
    e->ml_params.should_terminate = 1;
}

void engine_destroy(engine e)
{
    /* Destroy text renderer instance */
    text_renderer_destroy(e->text_renderer);

    /* Destroy resource manager instance */
    resmngr_destroy(e->rmgr);

    /* Destroy world instance */
    ecs_fini(e->world);

    /* Destroy renderer instance */
    renderer_destroy(e->renderer);

    /* Close window */
    window_destroy(e->wnd);

    /* Free engine instance */
    free(e);
}
