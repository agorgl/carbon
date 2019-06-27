#include "opengl.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/*-----------------------------------------------------------------
 * OpenGL Extensions
 *-----------------------------------------------------------------*/
void opengl_load_extensions(opengl_function_loader fnloader)
{
    gladLoadGL((GLADloadfunc)fnloader);
}

/*-----------------------------------------------------------------
 * OpenGL Errors
 *-----------------------------------------------------------------*/
#define REAL_GL_FUNC(f) glad_##f

/* Error handler state */
static opengl_error_handler_params err_handler;

/*
 * Checks and returns last shader compile error
 * Returned buffer must be freed if not null
 */
static const char* gl_check_last_compile_error(GLuint id)
{
    /* Check if last compile was successful */
    GLint compile_status;
    REAL_GL_FUNC(glGetShaderiv)(id, GL_COMPILE_STATUS, &compile_status);
    if (compile_status == GL_FALSE) {
        /* Gather the compile log size */
        GLint log_length;
        REAL_GL_FUNC(glGetShaderiv)(id, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length != 0) {
            /* Fetch and return log */
            GLchar* buf = calloc(1, log_length * sizeof(GLchar) + 1);
            REAL_GL_FUNC(glGetShaderInfoLog)(id, log_length, 0, buf);
            return buf;
        }
    }
    return 0;
}

/*
 * Checks and returns last shader link error
 * Returned buffer must be freed if not null
 */
static const char* gl_check_last_link_error(GLuint id)
{
    /* Check if last link was successful */
    GLint status;
    REAL_GL_FUNC(glGetProgramiv)(id, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        /* Gather the link log size */
        GLint log_length;
        REAL_GL_FUNC(glGetProgramiv)(id, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length != 0) {
            /* Fetch and return log */
            GLchar* buf = calloc(1, log_length * sizeof(GLchar) + 1);
            REAL_GL_FUNC(glGetProgramInfoLog)(id, log_length, 0, buf);
            return buf;
        }
    }
    return 0;
}

const char* gl_error_code_desc(GLenum code)
{
    switch (code) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";
        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";
        default:
            break;
    }
    return "???";
}

const char* gl_debug_source_desc(GLenum source)
{
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            return "GL_DEBUG_SOURCE_API";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "GL_DEBUG_SOURCE_SHADER_COMPILER";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "GL_DEBUG_SOURCE_THIRD_PARTY";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "GL_DEBUG_SOURCE_APPLICATION";
        case GL_DEBUG_SOURCE_OTHER:
            return "GL_DEBUG_SOURCE_OTHER";
        default:
            break;
    }
    return "???";
}

const char* gl_debug_msg_type_desc(GLenum type)
{
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            return "GL_DEBUG_TYPE_ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "GL_DEBUG_TYPE_PERFORMANCE";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "GL_DEBUG_TYPE_PORTABILITY";
        case GL_DEBUG_TYPE_MARKER:
            return "GL_DEBUG_TYPE_MARKER";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "GL_DEBUG_TYPE_PUSH_GROUP";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "GL_DEBUG_TYPE_POP_GROUP";
        case GL_DEBUG_TYPE_OTHER:
            return "GL_DEBUG_TYPE_OTHER";
        default:
            break;
    }
    return "???";
}

const char* gl_debug_msg_severity(GLenum severity)
{
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            return "GL_DEBUG_SEVERITY_HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM:
            return "GL_DEBUG_SEVERITY_MEDIUM";
        case GL_DEBUG_SEVERITY_LOW:
            return "GL_DEBUG_SEVERITY_LOW";
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return "GL_DEBUG_SEVERITY_NOTIFICATION";
        default:
            break;
    }
    return "???";
}

static const char* construct_opengl_debug_msg(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message)
{
    (void) id; (void) length;
    const char* type_str     = gl_debug_msg_type_desc(type);
    const char* source_str   = gl_debug_source_desc(source);
    const char* severity_str = gl_debug_msg_severity(severity);
    /* OpenGL error */
    const char* msg_fmt =
        "Type     : %s\n"
        "Source   : %s\n"
        "Severity : %s\n"
        "Message  : %s\n";
    size_t msg_len = snprintf(0, 0, msg_fmt, type_str, source_str, severity_str, message) + 1;
    char* msg = malloc(msg_len);
    snprintf(msg, msg_len, msg_fmt, type_str, source_str, severity_str);
    return msg;
}

static inline void opengl_post_hook(void* ret, const char* name, void(*fptr)(), int len_args, ...)
{
    (void) ret;

    /* Shader errors */
    const char* shdr_err = 0;
    if (fptr == glCompileShader || fptr == glLinkProgram) {
        GLuint s;
        va_list l;
        va_start(l, len_args);
        s = va_arg(l, GLuint);
        va_end(l);
        if (fptr == glCompileShader)
            shdr_err = gl_check_last_compile_error(s);
        else if (fptr == glLinkProgram)
            shdr_err = gl_check_last_link_error(s);
    }

    GLenum code = REAL_GL_FUNC(glGetError)();
    if (code != GL_NO_ERROR || shdr_err) {
        /* Header */
        size_t sz = 0;
        char* buf = 0;
        const char* header_fmt = "PANIC!\nOpenGL error in %s():\n";
        sz += snprintf(0, 0, header_fmt, name) + 1;
        buf = realloc(buf, sz);
        snprintf(buf, sz, header_fmt, name);

        /* Shader errors */
        if (shdr_err) {
            sz += strlen(shdr_err);
            buf = realloc(buf, sz);
            strcat(buf, shdr_err);
            free((void*)shdr_err);
            shdr_err = 0;
        }

        /* Messages */
        for (;;) {
            GLint next_log_len = 0;
            glGetIntegerv(GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH, &next_log_len);
            if (!next_log_len)
                break;
            char* msg_buf = calloc(1, next_log_len);
            GLsizei length;
            GLenum source, type, id, severity;
            glGetDebugMessageLog(1, next_log_len, &source, &type, &id, &severity, &length, (GLchar*)msg_buf);
            if (type == GL_DEBUG_TYPE_ERROR) {
                const char* msg = construct_opengl_debug_msg(source, type, id, severity, length, (const char*)msg_buf);
                sz += strlen(msg);
                buf = realloc(buf, sz);
                strcat(buf, msg);
                free((void*)msg);
            }
            free(msg_buf);
        }

        /* Show */
        err_handler.callback(err_handler.userdata, buf);
        free(buf);
    }
}

void opengl_register_error_handler(opengl_error_handler_params params)
{
    err_handler = params;
    gladSetGLPostCallback(opengl_post_hook);
}
