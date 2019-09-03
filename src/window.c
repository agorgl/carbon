#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <GLFW/glfw3.h>

struct window
{
    GLFWwindow* wnd_handle;
    struct window_callbacks callbacks;
    void* userdata;
    char* title, *title_suffix;
    int vsync;
    /* Misc state */
    float cursor_pos[2], cursor_prev_pos[2];
};

static void glfw_err_cb(int code, const char* desc)
{
    fprintf(stderr, "Error %d, \t%s\n", code, desc);
    exit(1);
}

static void glfw_mouse_button_cb(GLFWwindow* wnd_handle, int button, int action, int mods)
{
    window wnd = glfwGetWindowUserPointer(wnd_handle);
    if (wnd->callbacks.mouse_button_cb)
        wnd->callbacks.mouse_button_cb(wnd, button, action, mods);
}

static void glfw_cursor_pos_cb(GLFWwindow* wnd_handle, double xpos, double ypos)
{
    window wnd = glfwGetWindowUserPointer(wnd_handle);
    if (wnd->callbacks.cursor_pos_cb)
        wnd->callbacks.cursor_pos_cb(wnd, xpos, ypos);
}

static void glfw_cursor_enter_cb(GLFWwindow* wnd_handle, int entered)
{
    window wnd = glfwGetWindowUserPointer(wnd_handle);
    if (wnd->callbacks.cursor_enter_cb)
        wnd->callbacks.cursor_enter_cb(wnd, entered == GL_TRUE ? 1 : 0);
}

static void glfw_scroll_cb(GLFWwindow* wnd_handle, double xoff, double yoff)
{
    window wnd = glfwGetWindowUserPointer(wnd_handle);
    if (wnd->callbacks.scroll_cb)
        wnd->callbacks.scroll_cb(wnd, xoff, yoff);
}

static void glfw_key_cb(GLFWwindow* wnd_handle, int key, int scancode, int action, int mods)
{
    window wnd = glfwGetWindowUserPointer(wnd_handle);
    int act = -1;
    switch (action) {
        case GLFW_PRESS:
            act = KEY_ACTION_PRESS;
            break;
        case GLFW_RELEASE:
            act = KEY_ACTION_RELEASE;
            break;
        case GLFW_REPEAT:
            act = KEY_ACTION_REPEAT;
            break;
    }
    if (wnd->callbacks.key_cb)
        wnd->callbacks.key_cb(wnd, key, scancode, act, mods);
}

static void glfw_char_cb(GLFWwindow* wnd_handle, unsigned int codepoint)
{
    window wnd = glfwGetWindowUserPointer(wnd_handle);
    if (wnd->callbacks.char_cb)
        wnd->callbacks.char_cb(wnd, codepoint);
}

static void glfw_char_mods_cb(GLFWwindow* wnd_handle, unsigned int codepoint, int mods)
{
    window wnd = glfwGetWindowUserPointer(wnd_handle);
    if (wnd->callbacks.char_mods_cb)
        wnd->callbacks.char_mods_cb(wnd, codepoint, mods);
}

static void glfw_fb_size_cb(GLFWwindow* wnd_handle, int w, int h)
{
    window wnd = glfwGetWindowUserPointer(wnd_handle);
    if (wnd->callbacks.fb_size_cb)
        wnd->callbacks.fb_size_cb(wnd, w, h);
}

window window_create(window_params params)
{
    /* Initialize glfw context */
    glfwSetErrorCallback(glfw_err_cb);
    glfwInit();

    window wnd = calloc(1, sizeof(*wnd));

    /* Pass appropriate context hints */
    if (params.ctx_params.type == OPENGL) {
        glfwWindowHint(GLFW_SAMPLES, 4);
        if (params.ctx_params.version.maj > 3
        || (params.ctx_params.version.maj == 3 && params.ctx_params.version.min >= 2)) {
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    } else if (params.ctx_params.type == OPENGL_ES) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, params.ctx_params.version.maj);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, params.ctx_params.version.min);

    /* Debug context hint */
    if (params.ctx_params.debug)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    /* Window hints */
    glfwWindowHint(GLFW_FLOATING, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    /* Get monitor value according to given window mode */
    GLFWmonitor* mon = 0;
    switch (params.mode) {
        case 0: {
            mon = 0;
            break;
        }
        case 1: {
            mon = glfwGetPrimaryMonitor();
            const GLFWvidmode* videoMode = glfwGetVideoMode(mon);
            glfwWindowHint(GLFW_RED_BITS,     videoMode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS,   videoMode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS,    videoMode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);
            break;
        }
        case 2: {
            mon = glfwGetPrimaryMonitor();
            break;
        }
    }

    /* Create the window */
    GLFWwindow* window = glfwCreateWindow(params.width, params.height, params.title, mon, 0);
    if (!window)
        return 0;
    wnd->wnd_handle = window;

    /* Set it as the current opengl context */
    glfwMakeContextCurrent(wnd->wnd_handle);

    /* Set window user pointer to this in order to be able to use our callbacks */
    glfwSetWindowUserPointer(wnd->wnd_handle, wnd);

    /* Initialize glfw callbacks to internal functions */
    glfwSetMouseButtonCallback(wnd->wnd_handle, glfw_mouse_button_cb);
    glfwSetCursorPosCallback(wnd->wnd_handle, glfw_cursor_pos_cb);
    glfwSetCursorEnterCallback(wnd->wnd_handle, glfw_cursor_enter_cb);
    glfwSetScrollCallback(wnd->wnd_handle, glfw_scroll_cb);
    glfwSetKeyCallback(wnd->wnd_handle, glfw_key_cb);
    glfwSetCharCallback(wnd->wnd_handle, glfw_char_cb);
    glfwSetCharModsCallback(wnd->wnd_handle, glfw_char_mods_cb);
    glfwSetFramebufferSizeCallback(wnd->wnd_handle, glfw_fb_size_cb);

    /* Enable VSync by default */
    window_vsync_toggle(wnd, 1);

    /* Save window title */
    size_t title_len = strlen(params.title);
    wnd->title = calloc(1, title_len + 1);
    strncpy(wnd->title, params.title, title_len);

    return wnd;
}

void window_destroy(window wnd)
{
    glfwDestroyWindow(wnd->wnd_handle);
    if (wnd->title_suffix)
        free(wnd->title_suffix);
    if (wnd->title)
        free(wnd->title);
    free(wnd);

    /* Close glfw context */
    glfwTerminate();
}

void window_set_callbacks(window wnd, window_callbacks callbacks)
{
    wnd->callbacks = callbacks;
}

int window_vsync_enabled(window wnd)
{
    return wnd->vsync;
}

void window_vsync_toggle(window wnd, int state)
{
    wnd->vsync = state;
    glfwSwapInterval(!!wnd->vsync);
}

void window_update(window wnd)
{
    /* Poll events and fire callbacks */
    glfwPollEvents();
    /* Update cached state */
    wnd->cursor_prev_pos[0] = wnd->cursor_pos[0];
    wnd->cursor_prev_pos[1] = wnd->cursor_pos[1];
    double xpos, ypos;
    glfwGetCursorPos(wnd->wnd_handle, &xpos, &ypos);
    wnd->cursor_pos[0] = xpos;
    wnd->cursor_pos[1] = ypos;
}

void window_swap_buffers(window wnd)
{
    glfwSwapBuffers(wnd->wnd_handle);
}

void window_set_userdata(window wnd, void* userdata)
{
    wnd->userdata = userdata;
}

void* window_get_userdata(window wnd)
{
    return wnd->userdata;
}

enum key_action window_key_state(window wnd, enum key k)
{
    int glfw_state = glfwGetKey(wnd->wnd_handle, k);
    switch (glfw_state) {
        case GLFW_PRESS:
            return KEY_ACTION_PRESS;
        case GLFW_RELEASE:
            return KEY_ACTION_RELEASE;
        case GLFW_REPEAT:
            return KEY_ACTION_REPEAT;
        default:
            assert(0);
    }
}

enum key_action window_mouse_button_state(window wnd, enum mouse_button mb)
{
    int glfw_state = glfwGetMouseButton(wnd->wnd_handle, mb);
    switch (glfw_state) {
        case GLFW_PRESS:
            return KEY_ACTION_PRESS;
        case GLFW_RELEASE:
            return KEY_ACTION_RELEASE;
        default:
            assert(0);
    }
}

void window_get_cursor_diff(window wnd, float* x, float* y)
{
    if (x)
        *x = wnd->cursor_pos[0] - wnd->cursor_prev_pos[0];
    if (y)
        *y = wnd->cursor_pos[1] - wnd->cursor_prev_pos[1];
}

void window_grub_cursor(window wnd, int mode)
{
    int glfw_mode = mode > 0 ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
    glfwSetInputMode(wnd->wnd_handle, GLFW_CURSOR, glfw_mode);
}

int window_is_cursor_grubbed(window wnd)
{
    return glfwGetInputMode(wnd->wnd_handle, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

static void glfw_update_title(GLFWwindow* wnd_handle, const char* title, const char* suffix)
{
    /* full_title = "title" + " " + "suffix" + '\0' */
    size_t full_title_len = 0;
    full_title_len += title ? strlen(title) : 0;
    ++full_title_len;
    full_title_len += suffix ? strlen(suffix) : 0;
    ++full_title_len;

    /* Construct full title as title + " " + suffix */
    char* full_title = calloc(1, full_title_len);
    if (title)
        strcat(full_title, title);
    if (title && suffix)
        strcat(full_title, " ");
    if (suffix)
        strcat(full_title, suffix);

    glfwSetWindowTitle(wnd_handle, full_title);
    free(full_title);
}

void window_set_title(window wnd, const char* title)
{
    if (wnd->title) {
        free(wnd->title);
        wnd->title = 0;
    }

    if (title) {
        size_t title_len = strlen(title);
        wnd->title = calloc(1, title_len + 1);
        strncpy(wnd->title, title, title_len);
    }
    glfw_update_title(wnd->wnd_handle, wnd->title, wnd->title_suffix);
}

const char* window_get_title(window wnd)
{
    return wnd->title;
}

void window_set_title_suffix(window wnd, const char* suffix)
{
    if (wnd->title_suffix) {
        free(wnd->title_suffix);
        wnd->title_suffix = 0;
    }

    if (suffix) {
        size_t suffix_len = strlen(suffix);
        wnd->title_suffix = calloc(1, suffix_len + 1);
        strncpy(wnd->title_suffix, suffix, suffix_len);
    }
    glfw_update_title(wnd->wnd_handle, wnd->title, wnd->title_suffix);
}

void window_get_framebuffer_size(window wnd, int* width, int* height)
{
    glfwGetFramebufferSize(wnd->wnd_handle, width, height);
}

void* window_get_proc_address(const char* proc_name)
{
    return glfwGetProcAddress(proc_name);
}
