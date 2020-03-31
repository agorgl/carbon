#include "renderer.h"
#include <assert.h>
#include <stdlib.h>
#include "linmath.h"
#include "gfx.h"
#include "shaders.h"
#include "exposure.h"

typedef struct renderer {
    renderer_params params;
    gfx_image color_img;
    gfx_image depth_img;
    gfx_shader default_shd;
    gfx_pipeline default_pip;
    gfx_image fallback_tex;
}* renderer;

typedef struct {
    mat4 modl;
    mat4 view;
    mat4 proj;
} vs_params_t;

typedef struct {
    vec3 view_pos;
    vec3 light_pos;
    vec4 light_col;
    vec4 bcolor_val;
    vec2 mtlrgn_val;
    float has_bcolor_map;
    float has_normal_map;
    float has_mtlrgn_map;
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
    shader_desc static_vs = shader_fetch("primitive.vs");
    shader_desc direct_fs = shader_fetch("pbr_light.fs");

    /* Shader for the default pass */
    gfx_shader default_shd = gfx_make_shader(&(gfx_shader_desc){
        .attrs = {
            [0].name = "apos",
            [1].name = "anrm",
            [2].name = "atco",
            [3].name = "atng",
        },
        .vs.uniform_blocks[0] = {
            .size = sizeof(vs_params_t),
            .uniforms = {
                [0] = { .name = "modl", .type = GFX_UNIFORMTYPE_MAT4 },
                [1] = { .name = "view", .type = GFX_UNIFORMTYPE_MAT4 },
                [2] = { .name = "proj", .type = GFX_UNIFORMTYPE_MAT4 }
            }
        },
        .fs.uniform_blocks[0] = {
            .size = sizeof(fs_params_t),
            .uniforms = {
                [0] = { .name = "view_pos",       .type = GFX_UNIFORMTYPE_FLOAT3 },
                [1] = { .name = "light_pos",      .type = GFX_UNIFORMTYPE_FLOAT3 },
                [2] = { .name = "light_col",      .type = GFX_UNIFORMTYPE_FLOAT4 },
                [3] = { .name = "bcolor_val",     .type = GFX_UNIFORMTYPE_FLOAT4 },
                [4] = { .name = "mtlrgn_val",     .type = GFX_UNIFORMTYPE_FLOAT2 },
                [5] = { .name = "has_bcolor_map", .type = GFX_UNIFORMTYPE_FLOAT },
                [6] = { .name = "has_normal_map", .type = GFX_UNIFORMTYPE_FLOAT },
                [7] = { .name = "has_mtlrgn_map", .type = GFX_UNIFORMTYPE_FLOAT },
            }
        },
        .fs.images = {
            [0] = { .name = "bcolor_map", .type = GFX_IMAGETYPE_2D },
            [1] = { .name = "normal_map", .type = GFX_IMAGETYPE_2D },
            [2] = { .name = "mtlrgn_map", .type = GFX_IMAGETYPE_2D
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
                [2] = { .format = GFX_VERTEXFORMAT_FLOAT2 }, /* texcoord */
                [3] = { .format = GFX_VERTEXFORMAT_FLOAT4 }  /* tangent  */
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

static vec3 vpos_from_matrix(mat4 view)
{
    mat4 inverse_view = mat4_inverse(view);
    vec3 view_pos = vec3_new(inverse_view.xw, inverse_view.yw, inverse_view.zw);
    return view_pos;
}

static void render_scene(renderer r, renderer_scene* rs, mat4 view, mat4 proj)
{
    /* Camera params */
    const float ev100 = 15.0;
    vec3 vpos = vpos_from_matrix(view);
    float exposure = exposure_from_ev100(ev100);

    /* Light params */
    renderer_light* rl = &rs->lights[0];  /* TODO: Support multiple lights */
    float pei = rl->intensity * exposure; /* Pre-exposed intensity */
    vec3 lpos = rl->position;
    vec4 lcol = (vec4){{rl->color.r, rl->color.g, rl->color.b, pei}};

    /* Render all primitives for all nodes in the scene */
    gfx_apply_pipeline(r->default_pip);
    for (size_t i = 0; i < rs->num_nodes; ++i) {
        renderer_node* rn = &rs->nodes[i];
        renderer_mesh* rm = &rs->meshes[rn->mesh];
        for (size_t j = 0; j < rm->num_primitives; ++j) {
            /* Fetch geometry bindings */
            renderer_primitive* rp = &rs->primitives[rm->first_primitive + j];
            gfx_buffer vbuf = rs->buffers[rp->vertex_buffer];
            gfx_buffer ibuf = rs->buffers[rp->index_buffer];
            /* Default values */
            vec4 bcolor_val = vec4_one();
            vec2 mtlrgn_val = vec2_zero();
            gfx_image bcolor_map_img = r->fallback_tex;
            gfx_image normal_map_img = r->fallback_tex;
            gfx_image mtlrgn_map_img = r->fallback_tex;
            int has_bcolor_map = 0, has_normal_map = 0, has_mtlrgn_map = 0;
            /* Fetch material bindings */
            if (rp->material != RENDERER_SCENE_INVALID_INDEX) {
                renderer_material* rm = &rs->materials[rp->material];
                bcolor_val = rm->data.metallic.params.base_color_factor;
                mtlrgn_val.x = rm->data.metallic.params.metallic_factor;
                mtlrgn_val.y = rm->data.metallic.params.roughness_factor;
                size_t color_tex_idx = rm->data.metallic.images.base_color;
                if (color_tex_idx != RENDERER_SCENE_INVALID_INDEX) {
                    bcolor_map_img = rs->images[color_tex_idx];
                    has_bcolor_map = 1;
                }
                size_t normal_tex_idx = rm->data.metallic.images.normal;
                if (normal_tex_idx != RENDERER_SCENE_INVALID_INDEX) {
                    normal_map_img = rs->images[normal_tex_idx];
                    has_normal_map = 1;
                }
                size_t metal_roughness_tex_idx = rm->data.metallic.images.metallic_roughness;
                if (metal_roughness_tex_idx != RENDERER_SCENE_INVALID_INDEX) {
                    mtlrgn_map_img = rs->images[metal_roughness_tex_idx];
                    has_mtlrgn_map = 1;
                }
            }
            /* Apply the fetched bindings */
            gfx_apply_bindings(&(gfx_bindings){
                .vertex_buffers[0] = vbuf,
                .index_buffer      = ibuf,
                .fs_images = {
                    [0] = bcolor_map_img,
                    [1] = normal_map_img,
                    [2] = mtlrgn_map_img,
                }
            });
            /* Apply vertex and fragment shader uniforms */
            vs_params_t vs_params = {
                .modl = rn->transform,
                .view = view,
                .proj = proj,
            };
            fs_params_t fs_params = {
                .view_pos       = vpos,
                .light_pos      = lpos,
                .light_col      = lcol,
                .bcolor_val     = bcolor_val,
                .mtlrgn_val     = mtlrgn_val,
                .has_bcolor_map = has_bcolor_map,
                .has_normal_map = has_normal_map,
                .has_mtlrgn_map = has_mtlrgn_map,
            };
            gfx_apply_uniforms(GFX_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
            gfx_apply_uniforms(GFX_SHADERSTAGE_FS, 0, &fs_params, sizeof(fs_params));
            /* Perform the draw call */
            gfx_draw(rp->base_element, rp->num_elements, 1);
        }
    }
}

void renderer_frame(renderer r, renderer_inputs ri)
{
    /*
     * Default pass
     */

    /* View-projection matrix */
    mat4 view = ri.view;
    mat4 proj = mat4_perspective(radians(60.0f), 0.01f, 1000.0f, (float)r->params.width/(float)r->params.height);

    /* Pass action for default pass, clearing to black */
    gfx_begin_default_pass(&(gfx_pass_action){
        .colors[0] = {
            .action = GFX_ACTION_CLEAR,
            .val = { 0.0f, 0.0f, 0.0f, 1.0f }
        }
    }, r->params.width, r->params.height);
    render_scene(r, &ri.scene, view, proj);
    gfx_end_pass();

    /* Commit everything */
    gfx_commit();
}

void renderer_destroy(renderer r)
{
    gfx_shutdown();
    free(r);
}
