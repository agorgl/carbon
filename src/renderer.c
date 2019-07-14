#include "renderer.h"
#include <assert.h>
#include <stdlib.h>
#include "linmath.h"
#include "gfx.h"

typedef struct renderer {
    renderer_params params;
    gfx_image color_img;
    gfx_image depth_img;
    gfx_pass offscreen_pass;
    gfx_buffer vbuf;
    gfx_buffer ibuf;
    gfx_shader offscreen_shd;
    gfx_shader default_shd;
    gfx_pipeline offscreen_pip;
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

    /* Create offscreen pass onto the above render targets */
    gfx_pass offscreen_pass = gfx_make_pass(&(gfx_pass_desc){
        .color_attachments[0].image = color_img,
        .depth_stencil_attachment.image = depth_img
    });

    /* Cube vertex buffer with positions, colors and tex coords */
    float vertices[] = {
        /* pos                  color                       uvs */
        -1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 1.0f,

        -1.0f, -1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     0.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     1.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     0.0f, 1.0f,

         1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 1.0f,

        -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 1.0f,
         1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 1.0f
    };
    gfx_buffer vbuf = gfx_make_buffer(&(gfx_buffer_desc){
        .size = sizeof(vertices),
        .content = vertices,
    });

    /* An index buffer for the cube */
    uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };
    gfx_buffer ibuf = gfx_make_buffer(&(gfx_buffer_desc){
        .type = GFX_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indices),
        .content = indices,
    });

    /* Shader for the non-textured cube, rendered in the offscreen pass */
    gfx_shader offscreen_shd = gfx_make_shader(&(gfx_shader_desc){
        .vs.uniform_blocks[0] = {
            .size = sizeof(params_t),
            .uniforms = {
                [0] = { .name="mvp", .type=GFX_UNIFORMTYPE_MAT4 }
            }
        },
        .vs.source =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec4 position;\n"
            "in vec4 color0;\n"
            "out vec4 color;\n"
            "void main() {\n"
            "  gl_Position = mvp * position;\n"
            "  color = color0;\n"
            "}\n",
        .fs.source =
            "#version 330\n"
            "in vec4 color;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = color;\n"
            "}\n"
    });

    /* ...and a second shader for rendering a textured cube in the default pass */
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
        .fs.images[0] = { .name="tex", .type=GFX_IMAGETYPE_2D },
        .vs.source =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec4 position;\n"
            "in vec4 color0;\n"
            "in vec2 texcoord0;\n"
            "out vec4 color;\n"
            "out vec2 uv;\n"
            "void main() {\n"
            "  gl_Position = mvp * position;\n"
            "  color = color0;\n"
            "  uv = texcoord0;\n"
            "}\n",
        .fs.source =
            "#version 330\n"
            "uniform sampler2D tex;\n"
            "in vec4 color;\n"
            "in vec2 uv;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = texture(tex, uv) + color * 0.5;\n"
            "}\n"
    });

    /* Pipeline object for offscreen rendering, don't need texcoords here */
    gfx_pipeline offscreen_pip = gfx_make_pipeline(&(gfx_pipeline_desc){
        .layout = {
            /* Need to provide stride, because the buffer's texcoord is skipped */
            .buffers[0].stride = 36,
            /* But don't need to provide attr offsets, because pos and color are continuous */
            .attrs = {
                [0] = { .format = GFX_VERTEXFORMAT_FLOAT3 },
                [1] = { .format = GFX_VERTEXFORMAT_FLOAT4 }
            }
        },
        .shader = offscreen_shd,
        .index_type = GFX_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = GFX_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true
        },
        .blend.depth_format = GFX_PIXELFORMAT_DEPTH,
        .rasterizer = {
            .cull_mode = GFX_CULLMODE_BACK,
            .sample_count = sample_count
        }
    });

    /* And another pipeline object for the default pass */
    gfx_pipeline default_pip = gfx_make_pipeline(&(gfx_pipeline_desc){
        .layout = {
            /* Don't need to provide buffer stride or attr offsets, no gaps here */
            .attrs = {
                [0] = { .format = GFX_VERTEXFORMAT_FLOAT3 },
                [1] = { .format = GFX_VERTEXFORMAT_FLOAT4 },
                [2] = { .format = GFX_VERTEXFORMAT_FLOAT2 }
            }
        },
        .shader = default_shd,
        .index_type = GFX_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = GFX_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true,
        },
        .rasterizer.cull_mode = GFX_CULLMODE_BACK
    });

    renderer r = calloc(1, sizeof(*r));
    r->params         = *params;
    r->color_img      = color_img;
    r->depth_img      = depth_img;
    r->offscreen_pass = offscreen_pass;
    r->vbuf           = vbuf;
    r->ibuf           = ibuf;
    r->offscreen_shd  = offscreen_shd;
    r->default_shd    = default_shd;
    r->offscreen_pip  = offscreen_pip;
    r->default_pip    = default_pip;
    return r;
}

void renderer_frame(renderer r, renderer_inputs ri)
{
    static float rx = 0.0f, ry = 0.0f;

    /* View-projection matrix */
    mat4 proj = mat4_perspective(radians(60.0f), 0.01f, 1000.0f, (float)r->params.width/(float)r->params.height);
    mat4 view = mat4_view_look_at(vec3_new(0.0f, 1.5f, 6.0f), vec3_new(0.0f, 0.0f, 0.0f), vec3_new(0.0f, 1.0f, 0.0f));
    mat4 view_proj = mat4_mul_mat4(proj, view);

    /* Prepare the uniform block with the model-view-projection matrix,
     * we just use the same matrix for the offscreen- and default-pass */
    rx += 1.0f; ry += 2.0f;
    mat4 model = mat4_mul_mat4(
        mat4_rotation_x(radians(rx)),
        mat4_rotation_y(radians(ry)));

    /*
     * Offscreen pass, this renders a rotating,
     * untextured cube to the offscreen render target
     */

    /* Pass action for offscreen pass, clearing to black */
    gfx_begin_pass(r->offscreen_pass, &(gfx_pass_action){
        .colors[0] = { .action = GFX_ACTION_CLEAR, .val = { 0.0f, 0.0f, 0.0f, 1.0f } }
    });
    gfx_apply_pipeline(r->offscreen_pip);
    for (size_t i = 0; i < ri.scene.num_nodes; ++i) {
        /* Resource bindings for offscreen rendering */
        gfx_apply_bindings(&(gfx_bindings){
            .vertex_buffers[0] = r->vbuf,
            .index_buffer = r->ibuf
        });
        /* Uniforms for offscreen rendering */
        params_t vs_params = {.mvp = mat4_mul_mat4(view_proj, model)};
        gfx_apply_uniforms(GFX_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
        /* Draw */
        gfx_draw(0, 36, 1);
    }
    gfx_end_pass();

    /*
     * Default pass, this renders a textured cube, using the
     * offscreen render target as texture image
     */

    /* Pass action for default pass, clearing to blue-ish */
    gfx_begin_default_pass(&(gfx_pass_action){
        .colors[0] = {
            .action = GFX_ACTION_CLEAR,
            .val = { 0.0f, 0.25f, 1.0f, 1.0f }
        }
    }, r->params.width, r->params.height);
    gfx_apply_pipeline(r->default_pip);
    for (size_t i = 0; i < ri.scene.num_nodes; ++i) {
        mat4 modl = mat4_mul_mat4(ri.scene.nodes[i].transform, model);
        /* And the resource bindings for the default pass where a textured cube will
         * rendered, note how the render-target image is used as texture here */
        gfx_apply_bindings(&(gfx_bindings){
            .vertex_buffers[0] = r->vbuf,
            .index_buffer      = r->ibuf,
            .fs_images[0]      = r->color_img
        });
        params_t vs_params = {.mvp = mat4_mul_mat4(view_proj, modl)};
        gfx_apply_uniforms(GFX_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
        gfx_draw(0, 36, 1);
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
