/*********************************************************************************************************************/
/*                                                  /===-_---~~~~~~~~~------____                                     */
/*                                                 |===-~___                _,-'                                     */
/*                  -==\\                         `//~\\   ~~~~`---.___.-~~                                          */
/*              ______-==|                         | |  \\           _-~`                                            */
/*        __--~~~  ,-/-==\\                        | |   `\        ,'                                                */
/*     _-~       /'    |  \\                      / /      \      /                                                  */
/*   .'        /       |   \\                   /' /        \   /'                                                   */
/*  /  ____  /         |    \`\.__/-~~ ~ \ _ _/'  /          \/'                                                     */
/* /-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`                                                      */
/*                   \_|      /        _)   ;  ),   __--~~                                                           */
/*                     '~~--_/      _-~/-  / \   '-~ \                                                               */
/*                    {\__--_/}    / \\_>- )<__\      \                                                              */
/*                    /'   (_/  _-~  | |__>--<__|      |                                                             */
/*                   |0  0 _/) )-~     | |__>--<__|     |                                                            */
/*                   / /~ ,_/       / /__>---<__/      |                                                             */
/*                  o o _//        /-~_>---<__-~      /                                                              */
/*                  (^(~          /~_>---<__-      _-~                                                               */
/*                 ,/|           /__>--<__/     _-~                                                                  */
/*              ,//('(          |__>--<__|     /                  .----_                                             */
/*             ( ( '))          |__>--<__|    |                 /' _---_~\                                           */
/*          `-)) )) (           |__>--<__|    |               /'  /     ~\`\                                         */
/*         ,/,'//( (             \__>--<__\    \            /'  //        ||                                         */
/*       ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'                                          */
/*     `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/                                                     */
/*   ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~                                                       */
/*    ;'( ')/ ,)(                              ~~~~~~~~~~                                                            */
/*   ' ') '( (/                                                                                                      */
/*     '   '  `                                                                                                      */
/*********************************************************************************************************************/
#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "input.h"

/* Window datatype */
typedef struct window* window;

/* Event callback function types */
typedef void(*mouse_button_fn)(window, int, int, int);
typedef void(*cursor_pos_fn)(window, double, double);
typedef void(*cursor_enter_fn)(window, int);
typedef void(*scroll_fn)(window, double, double);
typedef void(*key_fn)(window, int, int, int, int);
typedef void(*char_fn)(window, unsigned int);
typedef void(*char_mods_fn)(window, unsigned int, int);
typedef void(*fb_size_fn)(window, unsigned int, unsigned int);

/* Set of event callbacks */
typedef struct window_callbacks {
    mouse_button_fn mouse_button_cb;
    cursor_pos_fn cursor_pos_cb;
    cursor_enter_fn cursor_enter_cb;
    scroll_fn scroll_cb;
    key_fn key_cb;
    char_fn char_cb;
    char_mods_fn char_mods_cb;
    fb_size_fn fb_size_cb;
} window_callbacks;

typedef struct window_params {
    const char* title;
    int width;
    int height;
    int mode;
    struct {
        enum {
            OPENGL = 0,
            OPENGL_ES
        } type;
        struct {
            int maj;
            int min;
        } version;
        int debug;
    } ctx_params;
} window_params;

/* Creates new window */
window window_create(window_params params);

/* Closes given window */
void window_destroy(window);

/* Registers given callback functions */
void window_set_callbacks(window, window_callbacks callbacks);

/* Polls for stored events, calls the registered callbacks and updates cached state */
void window_update(window);

/* Indicates the status of VSync */
int window_vsync_enabled(window wnd);

/* Enables or disables VSync */
void window_vsync_toggle(window wnd, int state);

/* Swaps backbuffer with front buffer */
void window_swap_buffers(window wnd);

/* Sets userdata pointer to be assosiated with given window */
void window_set_userdata(window wnd, void* userdata);

/* Retrieves userdata pointer assisiated with given window */
void* window_get_userdata(window wnd);

/* Returns the current state of the given key */
enum key_action window_key_state(window wnd, enum key k);

/* Returns the current state of the given mouse button */
enum key_action window_mouse_button_state(window wnd, enum mouse_button mb);

/* Returns cursor movement difference between last two frames */
void window_get_cursor_diff(window wnd, float* x, float* y);

/* Sets cursor grub (1: grub, 0: release) */
void window_grub_cursor(window wnd, int mode);

/* Returns 1 when the cursor is being grubbed */
int window_is_cursor_grubbed(window wnd);

/* Sets the window title */
void window_set_title(window wnd, const char* title);

/* Retrieves the current window title */
const char* window_get_title(window wnd);

/* Sets a window title suffix (used for live info) */
void window_set_title_suffix(window wnd, const char* suffix);

/* Returns the default framebuffer's resolution */
void window_get_framebuffer_size(window wnd, int* width, int* height);

/* Returns the address of the specified OpenGL (ES) core or extension function */
void* window_get_proc_address(const char* proc_name);

#endif /* ! _WINDOW_H_ */
