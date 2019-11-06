#include "renderer.h"
#include <assert.h>
#include <stdlib.h>
#include "linmath.h"
#include "gfx.h"
#include "shaders.h"

typedef struct renderer {
    renderer_params params;
    gfx_image color_img;
    gfx_image depth_img;
    gfx_shader default_shd;
    gfx_pipeline default_pip;
    gfx_image fallback_tex;
}* renderer;

typedef struct {
    mat4 mvp;
} vs_params_t;

typedef struct {
    float textured;
} fs_params_t;

renderer renderer_create(renderer_params* params)
{
    /* Setup gfx wrapper */
    const gfx_desc desc = {};
    gfx_setup(&desc);
    assert(gfx_isvalid());

    /* Create render target images */
    gfx_features features = gfx_query_features();
    const int sample_count = features.msaa_render_targets ? 4 : 1;
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

    /* Load shader sources */
    shader_desc static_vs = shader_fetch("static.vs");
    shader_desc direct_fs = shader_fetch("direct.fs");

    /* Shader for the default pass */
    gfx_shader default_shd = gfx_make_shader(&(gfx_shader_desc){
        .attrs = {
            [0].name = "position",
            [1].name = "normal",
            [2].name = "uv0",
        },
        .vs.uniform_blocks[0] = {
            .size = sizeof(vs_params_t),
            .uniforms = {
                [0] = {
                    .name = "mvp",
                    .type = GFX_UNIFORMTYPE_MAT4
                }
            }
        },
        .fs.uniform_blocks[0] = {
            .size = sizeof(fs_params_t),
            .uniforms = {
                [0] = {
                    .name = "textured",
                    .type = GFX_UNIFORMTYPE_FLOAT,
                }
            }
        },
        .fs.images = {
            [0] = {
                .name = "tex",
                .type = GFX_IMAGETYPE_2D
            },
        },
        .vs.source = static_vs->source,
        .fs.source = direct_fs->source,
    });

    /* Free shader sources */
    shader_free(static_vs);
    shader_free(direct_fs);

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
            .face_winding = GFX_FACEWINDING_CCW,
            .sample_count = sample_count
        }
    });

    /* Fallback textures */
    gfx_image fallback_tex = gfx_make_image(&(gfx_image_desc){
        .width = 1,
        .height = 1,
        .content.subimage[0][0] = {
            .ptr = (unsigned char[]){255, 0, 255},
            .size = 4,
        }
    });

    renderer r = calloc(1, sizeof(*r));
    r->params         = *params;
    r->color_img      = color_img;
    r->depth_img      = depth_img;
    r->default_shd    = default_shd;
    r->default_pip    = default_pip;
    r->fallback_tex   = fallback_tex;
    return r;
}

void renderer_frame(renderer r, renderer_inputs ri)
{
    renderer_scene* rs = &ri.scene;

    /* View-projection matrix */
    mat4 proj = mat4_perspective(radians(60.0f), 0.01f, 1000.0f, (float)r->params.width/(float)r->params.height);
    mat4 view = ri.view;
    mat4 view_proj = mat4_mul_mat4(proj, view);

    /*
     * Default pass
     */

    /* Pass action for default pass, clearing to black */
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
        for (size_t j = 0; j < rm->num_primitives; ++j) {
            /* Fetch geometry bindings */
            renderer_primitive* rp = &rs->primitives[rm->first_primitive + j];
            gfx_buffer vbuf = rs->buffers[rp->vertex_buffer];
            gfx_buffer ibuf = rs->buffers[rp->index_buffer];
            /* Fetch material bindings */
            int textured = 0;
            gfx_image img = r->fallback_tex;
            if (rp->material != RENDERER_SCENE_INVALID_INDEX) {
                renderer_material* rm = &rs->materials[rp->material];
                size_t color_tex_idx = rm->data.metallic.images.base_color;
                if (color_tex_idx != RENDERER_SCENE_INVALID_INDEX) {
                    img = rs->images[color_tex_idx];
                    textured = 1;
                }
            }
            /* Apply the fetched bindings */
            gfx_apply_bindings(&(gfx_bindings){
                .vertex_buffers[0] = vbuf,
                .index_buffer      = ibuf,
                .fs_images[0]      = img,
            });
            /* Apply vertex and fragment shader uniforms */
            vs_params_t vs_params = {.mvp = mat4_mul_mat4(view_proj, rn->transform)};
            fs_params_t fs_params = {.textured = textured};
            gfx_apply_uniforms(GFX_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
            gfx_apply_uniforms(GFX_SHADERSTAGE_FS, 0, &fs_params, sizeof(fs_params));
            /* Perform the draw call */
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
