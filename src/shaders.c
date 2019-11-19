#include "shaders.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "embedded.h"

#define SHADERS_EXTN ".glsl"
#define SHADERS_PATH "shaders/"
#define SHADERS_INCD "#include"

static void path_join(char* path, const char* base, const char* uri)
{
    const char* s0 = strrchr(base, '/');
    const char* s1 = strrchr(base, '\\');
    const char* slash = s0 ? (s1 && s1 > s0 ? s1 : s0) : s1;
    if (slash) {
        size_t prefix = slash - base + 1;
        strncpy(path, base, prefix);
        strcpy(path + prefix, uri);
    } else {
        strcpy(path, base);
    }
}

static const char* shader_path_from_name(const char* name)
{
    size_t path_len = strlen(SHADERS_PATH) + 1 + strlen(name) + strlen(SHADERS_EXTN);
    char* path = calloc(1, path_len + 1);
    path_join(path, SHADERS_PATH, name);
    strncat(path, SHADERS_EXTN, path_len);
    return path;
}

static void* next_non_space(const void* s, size_t n)
{
    if (n != 0) {
        const unsigned char* p = s;
        do {
            if (!isspace(*p++))
                return ((void*)(p - 1));
        } while (--n != 0);
    }
    return 0;
}

static const char* shader_load(const char* name)
{
    /* Construct path from name */
    const char* path = shader_path_from_name(name);

    /* Load main shader file */
    char* file_data = 0; size_t file_sz = 0;
    embedded_file((void**)&file_data, &file_sz, path);
    free((void*)path);
    if (!file_data)
        return 0;

    /* Copy over to intermediate buffer */
    char* buf = calloc(1, file_sz + 1);
    size_t buf_sz = file_sz;
    memcpy(buf, file_data, file_sz);

    /* Find includes */
    for (size_t i = 0; i < buf_sz; ++i) {
        /* Current line, end of line ptrs and line size */
        const char* lin = &buf[i];
        const char* eol = memchr(lin, '\n', buf_sz - i);
        size_t line_sz  = eol - lin;

        /* Check if include directive */
        if (strncmp(SHADERS_INCD, lin, strlen(SHADERS_INCD)) == 0) {
            /* Skip directive and space */
            const char* p = lin;
            p = memchr(p, ' ', eol - p);
            p = next_non_space(p, eol - p);

            /* Find directive name boundaries */
            const char* name_start = memchr(p, '<', eol - p);
            const char* name_end = memchr(p, '>', eol - p);

            /* If valid include directive */
            if (name_start && name_end && name_end > (name_start + 1)) {
                /* Copy over name */
                char* name = calloc(1, name_end - (name_start + 1) + 1);
                memcpy(name, name_start + 1, name_end - (name_start + 1));

                /* Load included file */
                const char* incdata = shader_load(name);
                if (!incdata) {
                    fprintf(stderr, "Could not load shader \"%s\"\n", name);
                    free(name);
                    goto err;
                }
                free(name);

                /* Allocate new buffer */
                size_t incdata_sz = strlen(incdata);
                size_t added_size = incdata_sz - (eol - lin);
                buf_sz += added_size;
                void* nbuf = calloc(1, buf_sz + 1);

                /* Populate new buffer with included data */
                strncat(nbuf, buf, lin - buf);
                strncat(nbuf, incdata, incdata_sz);
                strcat(nbuf, eol);
                free((void*)incdata);
                free(buf);
                buf = nbuf;

                /* Progress current position */
                i += added_size;
            }
        }

        /* Skip to next line */
        i += line_sz;
    }
    return buf;

err:
    free(buf);
    return 0;
}

shader_desc shader_fetch(const char* name)
{
    shader_desc desc = calloc(1, sizeof(*desc));
    desc->path = shader_path_from_name(name);
    desc->source = shader_load(name);
    return desc;
}

void shader_free(shader_desc desc)
{
    free((void*)desc->source);
    free((void*)desc->path);
    free(desc);
}
