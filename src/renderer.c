#include "renderer.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "linmath.h"
#include "gfx.h"
#include "shaders.h"
#include "exposure.h"
#include "geometry.h"

#define PROBE_CUBEMAP_RESOLUTION (128)

typedef struct renderer {
    renderer_params params;
    /* Default pass */
    gfx_image color_img;
    gfx_image depth_img;
    gfx_shader default_shd;
    gfx_pipeline default_pip;
    /* Probe pass */
    gfx_image probe_color_img;
    gfx_image probe_depth_img;
    gfx_pass probe_cubemap_pass[GFX_CUBEFACE_NUM];
    /* Debug resources */
    gfx_shader probe_debug_shd;
    gfx_pipeline probe_debug_pip;
    /* Misc resources */
    gfx_image fallback_tex;
    gfx_buffer sphere_vbuf;
    gfx_buffer sphere_ibuf;
    size_t sphere_num_elem;
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
    shader_desc prbdbg_vs = shader_fetch("probe_dbg.vs");
    shader_desc prbdbg_fs = shader_fetch("probe_dbg.fs");

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
            [2] = { .name = "mtlrgn_map", .type = GFX_IMAGETYPE_2D },
        },
        .vs.source = static_vs->source,
        .fs.source = direct_fs->source,
    });

    /* Shader for the probe debug view */
    gfx_shader probe_debug_shd = gfx_make_shader(&(gfx_shader_desc){
        .attrs = {
            [0].name = "apos",
            [1].name = "anrm",
        },
        .vs.uniform_blocks[0] = {
            .size = sizeof(mat4),
            .uniforms = {
                [0] = { .name = "mvp", .type = GFX_UNIFORMTYPE_MAT4 },
            }
        },
        .fs.images = {
            [0] = { .name = "probe", .type = GFX_IMAGETYPE_CUBE },
        },
        .vs.source = prbdbg_vs->source,
        .fs.source = prbdbg_fs->source,
    });

    /* Free shader sources */
    shader_free(prbdbg_vs);
    shader_free(prbdbg_fs);
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

    /* Pipeline object for the probe debug pass */
    gfx_pipeline probe_debug_pip = gfx_make_pipeline(&(gfx_pipeline_desc){
        .layout = {
            .buffers = { [0] = { .stride = (3 + 3 + 2 + 4) * sizeof(float)} },
            .attrs = {
                [0] = { .format = GFX_VERTEXFORMAT_FLOAT3 }, /* position */
                [1] = { .format = GFX_VERTEXFORMAT_FLOAT3 }, /* normal   */
            }
        },
        .shader = probe_debug_shd,
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

    /* Probe cubemap passes render targets */
    gfx_image probe_color_img = gfx_make_image(&(gfx_image_desc){
        .type          = GFX_IMAGETYPE_CUBE,
        .render_target = 1,
        .width         = PROBE_CUBEMAP_RESOLUTION,
        .height        = PROBE_CUBEMAP_RESOLUTION,
        .min_filter    = GFX_FILTER_LINEAR,
        .mag_filter    = GFX_FILTER_LINEAR,
        .sample_count  = sample_count,
    });

    gfx_image probe_depth_img = gfx_make_image(&(gfx_image_desc){
        .type          = GFX_IMAGETYPE_CUBE,
        .render_target = 1,
        .width         = PROBE_CUBEMAP_RESOLUTION,
        .height        = PROBE_CUBEMAP_RESOLUTION,
        .pixel_format  = GFX_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count  = sample_count,
    });

    /* Probe cubemap passes */
    gfx_pass probe_cubemap_pass[GFX_CUBEFACE_NUM];
    for (int i = 0; i < GFX_CUBEFACE_NUM; ++i) {
        probe_cubemap_pass[i] = gfx_make_pass(&(gfx_pass_desc){
            .color_attachments[0] = {
                .image = probe_color_img,
                .face = i,
            },
            .depth_stencil_attachment = {
                .image = probe_depth_img,
            }
        });
    }

    /* Fallback textures */
    gfx_image fallback_tex = gfx_make_image(&(gfx_image_desc){
        .width = 1,
        .height = 1,
        .content.subimage[0][0] = {
            .ptr = (unsigned char[]){255, 0, 255},
            .size = 4,
        }
    });

    /* Internal sphere geometry */
    void* sph_verts; uint32_t* sph_indcs; size_t sph_num_verts, sph_num_indcs;
    generate_uv_sphere(&sph_verts, &sph_num_verts, &sph_indcs, &sph_num_indcs, 0.25f, 32, 32);
    gfx_buffer sphere_vbuf = gfx_make_buffer(&(gfx_buffer_desc){
        .size = (3 + 3 + 2 + 4) * sizeof(float) * sph_num_verts,
        .content = sph_verts,
    });
    gfx_buffer sphere_ibuf = gfx_make_buffer(&(gfx_buffer_desc){
        .type = GFX_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(sph_indcs) * sph_num_indcs,
        .content = sph_indcs,
    });
    free(sph_verts); free(sph_indcs);

    renderer r = calloc(1, sizeof(*r));
    r->params          = *params;
    r->color_img       = color_img;
    r->depth_img       = depth_img;
    r->default_shd     = default_shd;
    r->default_pip     = default_pip;
    r->fallback_tex    = fallback_tex;
    r->sphere_vbuf     = sphere_vbuf;
    r->sphere_ibuf     = sphere_ibuf;
    r->sphere_num_elem = sph_num_indcs;
    r->probe_color_img = probe_color_img;
    r->probe_depth_img = probe_depth_img;
    r->probe_debug_shd = probe_debug_shd;
    r->probe_debug_pip = probe_debug_pip;
    memcpy(&r->probe_cubemap_pass, probe_cubemap_pass, sizeof(r->probe_cubemap_pass));
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

static void render_probe_cubemap(renderer r, renderer_scene* rs, vec3 probe_pos)
{
    /* Render probe to temporary cubemap target */
    for (int face = 0; face < GFX_CUBEFACE_NUM; ++face) {
        gfx_begin_pass(r->probe_cubemap_pass[face], &(gfx_pass_action){
            .colors[0] = {
                .action = GFX_ACTION_CLEAR,
                .val = { 0.0f, 0.0f, 0.0f, 1.0f }
            },
        });
        vec3 center_and_up[GFX_CUBEFACE_NUM][2] = {
            /*        Center         -          Up         */
            {{{+1.0f,  0.0f,  0.0f}}, {{0.0f, -1.0f,  0.0f}}}, /* +X */
            {{{-1.0f,  0.0f,  0.0f}}, {{0.0f, -1.0f,  0.0f}}}, /* -X */
            {{{ 0.0f, +1.0f,  0.0f}}, {{0.0f,  0.0f, +1.0f}}}, /* +Y */
            {{{ 0.0f, -1.0f,  0.0f}}, {{0.0f,  0.0f, -1.0f}}}, /* -Y */
            {{{ 0.0f,  0.0f, +1.0f}}, {{0.0f, -1.0f,  0.0f}}}, /* +Z */
            {{{ 0.0f,  0.0f, -1.0f}}, {{0.0f, -1.0f,  0.0f}}}, /* -Z */
        };
        vec3 probe_tgt  = vec3_add(probe_pos, center_and_up[face][0]);
        mat4 probe_view = mat4_view_look_at(probe_pos, probe_tgt, center_and_up[face][1]);
        mat4 probe_proj = mat4_perspective(radians(90.0f), 0.01f, 1000.0f, 1.0f);
        render_scene(r, rs, probe_view, probe_proj);
        gfx_end_pass();
    }
}

static void render_probe_visualization(renderer r, vec3 probe_pos, mat4 view, mat4 proj)
{
    /* Render debug view to default framebuffer */
    gfx_begin_default_pass(&(gfx_pass_action){
        .colors[0].action = GFX_ACTION_LOAD,
        .depth.action = GFX_ACTION_LOAD
    }, r->params.width, r->params.height);
    gfx_apply_pipeline(r->probe_debug_pip);
    gfx_apply_bindings(&(gfx_bindings){
        .vertex_buffers[0] = r->sphere_vbuf,
        .index_buffer      = r->sphere_ibuf,
        .fs_images = { [0] = r->probe_color_img, }
    });
    mat4 mdl = mat4_translation(probe_pos);
    mat4 mvp = mat4_mul_mat4(mat4_mul_mat4(proj, view), mdl);
    gfx_apply_uniforms(GFX_SHADERSTAGE_VS, 0, &mvp, sizeof(mat4));
    gfx_draw(0, r->sphere_num_elem, 1);
    gfx_end_pass();
}

void renderer_frame(renderer r, renderer_inputs ri)
{
    /*
     * Default pass
     */
    renderer_scene* rs = &ri.scene;

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
    render_scene(r, rs, view, proj);
    gfx_end_pass();

    /*
     * Probes pass
     */
    vec3 probe_positions[] = {
        {{-4.0, 2.0, 0.0 }},
        {{ 0.0, 2.0, 0.0 }},
        {{ 4.0, 2.0, 0.0 }},
    };
    size_t num_probes = sizeof(probe_positions) / sizeof(probe_positions[0]);
    for (size_t i = 0; i < num_probes; ++i) {
        vec3 probe_pos = probe_positions[i];
        render_probe_cubemap(r, rs, probe_pos);
        render_probe_visualization(r, probe_pos, view, proj);
    }

    /* Commit everything */
    gfx_commit();
}

void renderer_destroy(renderer r)
{
    gfx_shutdown();
    free(r);
}
