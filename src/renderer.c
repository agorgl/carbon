#include "renderer.h"
#include <assert.h>
#include <stdlib.h>
#include "linmath.h"
#include "gfx.h"

typedef struct renderer {
    renderer_params params;
    gfx_image color_img;
    gfx_image depth_img;
    gfx_shader default_shd;
    gfx_pipeline default_pip;
}* renderer;

typedef struct {
    mat4 mvp;
} params_t;

renderer renderer_create(renderer_params* params)
{
    /* Setup gfx wrapper */
    const gfx_desc desc = {};
    gfx_setup(&desc);
    assert(gfx_isvalid());

    /* Create render target images */
    const int sample_count = gfx_query_feature(GFX_FEATURE_MSAA_RENDER_TARGETS) ? 4 : 1;
    gfx_image_desc img_desc = {
        .render_target = 1,
        .width = params->width,
        .height = params->height,
        .min_filter = GFX_FILTER_LINEAR,
        .mag_filter = GFX_FILTER_LINEAR,
        .sample_count = sample_count
    };
    gfx_image color_img = gfx_make_image(&img_desc);
    img_desc.pixel_format = GFX_PIXELFORMAT_DEPTH;
    gfx_image depth_img = gfx_make_image(&img_desc);

    /* Shader for the default pass */
    gfx_shader default_shd = gfx_make_shader(&(gfx_shader_desc){
        .vs.uniform_blocks[0] = {
            .size = sizeof(params_t),
            .uniforms = {
                [0] = {
                    .name = "mvp",
                    .type = GFX_UNIFORMTYPE_MAT4
                }
            }
        },
        .vs.source =
            "#version 330\n"
            "in vec3 position;\n"
            "in vec3 normal;\n"
            "in vec2 uv0;\n"
            "uniform mat4 mvp;\n"
            "out vec3 color;\n"
            "out vec2 uv;\n"
            "void main() {\n"
            "  color = position;\n"
            "  uv = uv0;\n"
            "  gl_Position = mvp * vec4(position, 1.0);\n"
            "}\n",
        .fs.source =
            "#version 330\n"
            "out vec4 fcolor;\n"
            "in vec3 color;\n"
            "in vec2 uv;\n"
            "void main() {\n"
            "  fcolor = vec4(color, 1.0);\n"
            "}\n"
    });

    /* Pipeline object for the default pass */
    gfx_pipeline default_pip = gfx_make_pipeline(&(gfx_pipeline_desc){
        .layout = {
            /* Don't need to provide buffer stride or attr offsets, no gaps here */
            .attrs = {
                [0] = { .format = GFX_VERTEXFORMAT_FLOAT3 }, /* position */
                [1] = { .format = GFX_VERTEXFORMAT_FLOAT3 }, /* normal   */
                [2] = { .format = GFX_VERTEXFORMAT_FLOAT2 }  /* texcoord */
            }
        },
        .shader = default_shd,
        .index_type = GFX_INDEXTYPE_UINT32,
        .depth_stencil = {
            .depth_compare_func = GFX_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true,
        },
        .rasterizer = {
            .cull_mode = GFX_CULLMODE_BACK,
            .face_winding = SG_FACEWINDING_CCW,
            .sample_count = sample_count
        }
    });

    renderer r = calloc(1, sizeof(*r));
    r->params         = *params;
    r->color_img      = color_img;
    r->depth_img      = depth_img;
    r->default_shd    = default_shd;
    r->default_pip    = default_pip;
    return r;
}

void renderer_frame(renderer r, renderer_inputs ri)
{
    renderer_scene* rs = &ri.scene;
    static float rx = 0.0f, ry = 0.0f;

    /* View-projection matrix */
    mat4 proj = mat4_perspective(radians(60.0f), 0.01f, 1000.0f, (float)r->params.width/(float)r->params.height);
    mat4 view = ri.view;
    mat4 view_proj = mat4_mul_mat4(proj, view);

    /* Prepare the uniform block with the model-view-projection matrix,
     * we just use the same matrix for the offscreen- and default-pass */
    ry += 1.0f;
    mat4 model = mat4_mul_mat4(
        mat4_rotation_x(radians(rx)),
        mat4_rotation_y(radians(ry)));

    /*
     * Default pass
     */

    /* Pass action for default pass, clearing to blue-ish */
    gfx_begin_default_pass(&(gfx_pass_action){
        .colors[0] = {
            .action = GFX_ACTION_CLEAR,
            .val = { 0.0f, 0.0f, 0.0f, 1.0f }
        }
    }, r->params.width, r->params.height);
    gfx_apply_pipeline(r->default_pip);
    for (size_t i = 0; i < rs->num_nodes; ++i) {
        renderer_node* rn = &rs->nodes[i];
        renderer_mesh* rm = &rs->meshes[rn->mesh];
        mat4 modl = mat4_mul_mat4(rn->transform, model);
        for (size_t j = 0; j < rm->num_primitives; ++j) {
            /* Resource bindings for the default pass */
            renderer_primitive* rp = &rs->primitives[rm->first_primitive + j];
            gfx_buffer vbuf = rs->buffers[rp->vertex_buffer];
            gfx_buffer ibuf = rs->buffers[rp->index_buffer];
            gfx_apply_bindings(&(gfx_bindings){
                .vertex_buffers[0] = vbuf,
                .index_buffer      = ibuf,
            });
            params_t vs_params = {.mvp = mat4_mul_mat4(view_proj, modl)};
            gfx_apply_uniforms(GFX_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
            gfx_draw(rp->base_element, rp->num_elements, 1);
        }
    }
    gfx_end_pass();

    /* Commit everything */
    gfx_commit();
}

void renderer_destroy(renderer r)
{
    gfx_shutdown();
    free(r);
}
