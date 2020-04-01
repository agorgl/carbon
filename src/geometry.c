#include "geometry.h"
#define _USE_MATH_CONSTANTS
#include <math.h>
#define pi M_PI

void generate_uv_sphere(void** vertices, size_t* num_verts, uint32_t** indices, size_t* num_indcs,
                        float radius, unsigned int rings, unsigned int sectors)
{
    /* Allocate space */
    size_t vsize = (3 /* pos */ + 3 /* nrm */ + 2 /* uv */ + 4 /* tng */) * sizeof(float);
    *num_verts = rings * sectors;
    *num_indcs = rings * sectors * 6;
    *vertices  = calloc(*num_verts, vsize);
    *indices   = calloc(*num_indcs, sizeof(*indices));

    /* Populate vertices */
    const float R = 1.0f / (float)(rings - 1);
    const float S = 1.0f / (float)(sectors - 1);
    for (unsigned int r = 0; r < rings; ++r) {
        for (unsigned int s = 0; s < sectors; ++s) {
            /* Calculate x,y,z components */
            void* vbase = *vertices + (r * sectors + s) * vsize;
            float x = cosf(2 * pi * s * S) * sinf(pi * r * R);
            float z = sinf(2 * pi * s * S) * sinf(pi * r * R);
            float y = sinf(-pi / 2 + pi * r * R);
            /* Set position */
            float* pos = vbase + (0) * sizeof(float);
            pos[0] = x * radius;
            pos[1] = y * radius;
            pos[2] = z * radius;
            /* Set normal */
            float* nrm = vbase + (3) * sizeof(float);
            nrm[0] = x;
            nrm[1] = y;
            nrm[2] = z;
            /* Set texture coordinates */
            float* uvs = vbase + (6) * sizeof(float);
            uvs[0] = s * S;
            uvs[1] = (rings - 1 - r) * R;
        }
    }

    /* Populate indices */
    uint32_t* it = *indices;
    for (unsigned int r = 0; r < rings - 1; ++r) {
        for (unsigned int s = 0; s < sectors - 1; ++s) {
            /* Calculate indices for current patch */
            unsigned int cur_row = r * sectors;
            unsigned int nxt_row = (r + 1) * sectors;
            /* First triangle */
            *it++ = (cur_row + s);
            *it++ = (nxt_row + s);
            *it++ = (nxt_row + (s + 1));
            /* Second triangle */
            *it++ = (cur_row + s);
            *it++ = (nxt_row + (s + 1));
            *it++ = (cur_row + (s + 1));
        }
    }
}
