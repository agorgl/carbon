#include "resmngr.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "renderer.h"
#include "cgltf.h"

typedef struct resmngr {
    struct slot_map scene_map;
}* resmngr;

resmngr resmngr_create()
{
    resmngr rm = calloc(1, sizeof(*rm));
    slot_map_init(&rm->scene_map, sizeof(renderer_scene));
    return rm;
}

int resmngr_handle_valid(rid r)
{
    return slot_map_key_valid(r);
}

void resmngr_destroy(resmngr rm)
{
    for (size_t i = 0; i < rm->scene_map.size; ++i) {
        rid r = slot_map_data_to_key(&rm->scene_map, i);
        resmngr_model_delete(rm, r);
    }
    slot_map_destroy(&rm->scene_map);
    free(rm);
}

rid resmngr_model_sample(resmngr rm)
{
    rid r = slot_map_insert(&rm->scene_map, 0);
    renderer_scene* rs = slot_map_lookup(&rm->scene_map, r);
    memset(rs, 0, sizeof(*rs));

    float vertices[] = {
        /* pos               nrm                 uvs    */
        -0.5, -0.5,  0.5,    0.0,  0.0,  1.0,    6.0, 0.0,
         0.5, -0.5,  0.5,    0.0,  0.0,  1.0,    5.0, 0.0,
        -0.5,  0.5,  0.5,    0.0,  0.0,  1.0,    6.0, 1.0,
         0.5,  0.5,  0.5,    0.0,  0.0,  1.0,    5.0, 1.0,
         0.5,  0.5,  0.5,    1.0,  0.0,  0.0,    4.0, 0.0,
         0.5, -0.5,  0.5,    1.0,  0.0,  0.0,    5.0, 0.0,
         0.5,  0.5, -0.5,    1.0,  0.0,  0.0,    4.0, 1.0,
         0.5, -0.5, -0.5,    1.0,  0.0,  0.0,    5.0, 1.0,
        -0.5,  0.5,  0.5,    0.0,  1.0,  0.0,    2.0, 0.0,
         0.5,  0.5,  0.5,    0.0,  1.0,  0.0,    1.0, 0.0,
        -0.5,  0.5, -0.5,    0.0,  1.0,  0.0,    2.0, 1.0,
         0.5,  0.5, -0.5,    0.0,  1.0,  0.0,    1.0, 1.0,
         0.5, -0.5,  0.5,    0.0, -1.0,  0.0,    3.0, 0.0,
        -0.5, -0.5,  0.5,    0.0, -1.0,  0.0,    4.0, 0.0,
         0.5, -0.5, -0.5,    0.0, -1.0,  0.0,    3.0, 1.0,
        -0.5, -0.5, -0.5,    0.0, -1.0,  0.0,    4.0, 1.0,
        -0.5, -0.5,  0.5,   -1.0,  0.0,  0.0,    3.0, 0.0,
        -0.5,  0.5,  0.5,   -1.0,  0.0,  0.0,    2.0, 0.0,
        -0.5, -0.5, -0.5,   -1.0,  0.0,  0.0,    3.0, 1.0,
        -0.5,  0.5, -0.5,   -1.0,  0.0,  0.0,    2.0, 1.0,
        -0.5, -0.5, -0.5,    0.0,  0.0, -1.0,    0.0, 0.0,
        -0.5,  0.5, -0.5,    0.0,  0.0, -1.0,    0.0, 1.0,
         0.5, -0.5, -0.5,    0.0,  0.0, -1.0,    1.0, 0.0,
         0.5,  0.5, -0.5,    0.0,  0.0, -1.0,    1.0, 1.0,
    };
    gfx_buffer vbuf = gfx_make_buffer(&(gfx_buffer_desc){
        .size = sizeof(vertices),
        .content = vertices,
    });

    uint32_t indices[] = {
         0,  1,  2,
         3,  2,  1,
         4,  5,  6,
         7,  6,  5,
         8,  9, 10,
        11, 10,  9,
        12, 13, 14,
        15, 14, 13,
        16, 17, 18,
        19, 18, 17,
        20, 21, 22,
        23, 22, 21,
    };
    gfx_buffer ibuf = gfx_make_buffer(&(gfx_buffer_desc){
        .type = GFX_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indices),
        .content = indices,
    });

    *rs = (renderer_scene){
        .buffers = {
            vbuf,
            ibuf
        },
        .primitives = {
            [0] = {
                .vertex_buffer = 0,
                .index_buffer = 1,
                .base_element = 0,
                .num_elements = sizeof(indices) / sizeof(indices[0])
            }
        },
        .meshes = {
            [0] = {
                .first_primitive = 0,
                .num_primitives = 1
            }
        },
        .nodes = {
            [0] = {
                .mesh = 0,
                .transform = mat4_id()
            }
        },
        .num_buffers    = 2,
        .num_primitives = 1,
        .num_meshes     = 1,
        .num_nodes      = 1,
    };

    return r;
}

static void gltf_parse_meshes(renderer_scene* rs, const cgltf_data* gltf)
{
    assert(gltf->meshes_count < RENDERER_SCENE_MAX_MESHES);

    for (size_t i = 0; i < gltf->meshes_count; ++i) {
        cgltf_mesh* gltf_mesh = &gltf->meshes[i];
        rs->meshes[rs->num_meshes++] = (renderer_mesh) {
            .first_primitive = rs->num_primitives,
            .num_primitives  = gltf_mesh->primitives_count,
        };
        assert(rs->num_primitives + gltf_mesh->primitives_count < RENDERER_SCENE_MAX_PRIMITIVES);

        /* Count vertices and indices for current mesh */
        size_t vcount = 0, icount = 0;
        for (size_t j = 0; j < gltf_mesh->primitives_count; ++j) {
            cgltf_primitive* gltf_prim = &gltf_mesh->primitives[j];
            for (size_t k = 0; k < gltf_prim->attributes_count; ++k) {
                cgltf_attribute* gltf_attr = &gltf_prim->attributes[k];
                if (gltf_attr->type == cgltf_attribute_type_position) {
                    vcount += gltf_attr->data->count;
                }
            }
            if (gltf_prim->indices) {
                icount += gltf_prim->indices->count;
            }
        }

        /* Allocate buffers */
        assert(rs->num_buffers + 2 < RENDERER_SCENE_MAX_BUFFERS);
        size_t vsize = (3 /* pos */ + 3 /* nrm */ + 2 /* uv */) * sizeof(float);
        float* vdata = calloc(vcount, vsize);
        uint32_t* idata = calloc(icount, sizeof(*idata));
        size_t nvert_buf = rs->num_buffers + 0;
        size_t nindc_buf = rs->num_buffers + 1;

        /* Populate buffers */
        size_t voffs = 0; size_t ioffs = 0;
        for (size_t j = 0; j < gltf_mesh->primitives_count; ++j) {
            /* Current primitive */
            cgltf_primitive* gltf_prim = &gltf_mesh->primitives[j];
            renderer_primitive* prim = &rs->primitives[rs->num_primitives++];
            prim->vertex_buffer = nvert_buf;
            prim->index_buffer  = nindc_buf;

            /* Count number of vertices in current primitive */
            size_t nverts = 0;
            for (size_t k = 0; k < gltf_prim->attributes_count; ++k) {
                cgltf_attribute* gltf_attr = &gltf_prim->attributes[k];
                if (gltf_attr->type == cgltf_attribute_type_position) {
                    nverts = gltf_attr->data->count;
                }
            }
            assert(nverts != 0);

            /* Copy vertex data for current primitive */
            for (size_t k = 0; k < gltf_prim->attributes_count; ++k) {
                cgltf_attribute* gltf_attr = &gltf_prim->attributes[k];
                cgltf_accessor*  gltf_accs = gltf_attr->data;
                assert(!gltf_accs->is_sparse);

                size_t elem_size = 0, attr_offs = 0;
                switch(gltf_attr->type) {
                    case cgltf_attribute_type_position:
                        elem_size = 3 * sizeof(float);
                        attr_offs = 0;
                        break;
                    case cgltf_attribute_type_normal:
                        elem_size = 3 * sizeof(float);
                        attr_offs = (3) * sizeof(float);
                        break;
                    case cgltf_attribute_type_texcoord:
                        elem_size = 2 * sizeof(float);
                        attr_offs = (3 + 3) * sizeof(float);
                        break;
                    default:
                        continue;
                }

                for (size_t l = 0; l < gltf_accs->count; ++l) {
                    size_t src_stride = gltf_accs->buffer_view->stride;
                    src_stride = src_stride != 0 ? src_stride : elem_size;
                    void* src = gltf_accs->buffer_view->buffer->data
                              + gltf_accs->buffer_view->offset
                              + gltf_accs->offset
                              + l * src_stride;
                    void* dst = (void*)vdata + (voffs + l) * vsize + attr_offs;
                    memcpy(dst, src, elem_size);
                }
            }

            /* Only indexed meshes supported for now */
            assert(gltf_prim->indices != 0);
            assert(gltf_prim->indices->type == cgltf_type_scalar);

            /* Copy index data for current primitive */
            prim->base_element = ioffs;
            prim->num_elements = gltf_prim->indices->count;
            for (size_t k = 0; k < gltf_prim->indices->count; ++k) {
                void* isrc = gltf_prim->indices->buffer_view->buffer->data
                           + gltf_prim->indices->buffer_view->offset;
                uint32_t idx = ~0u;
                switch (gltf_prim->indices->component_type) {
                    case cgltf_component_type_r_8:
                    case cgltf_component_type_r_8u:
                        idx = ((uint8_t*)isrc)[k];
                        break;
                    case cgltf_component_type_r_16:
                    case cgltf_component_type_r_16u:
                        idx = ((uint16_t*)isrc)[k];
                        break;
                    case cgltf_component_type_r_32u:
                    case cgltf_component_type_r_32f:
                        idx = ((uint32_t*)isrc)[k];
                        break;
                    case cgltf_component_type_invalid:
                    default:
                        assert(0);
                }
                idata[ioffs + k] = voffs + idx;
            }

            /* Increase offsets by number of vertices/indices */
            voffs += nverts;
            ioffs += gltf_prim->indices->count;
        }

        /* Upload and store buffer handles */
        rs->buffers[rs->num_buffers++] = gfx_make_buffer(&(gfx_buffer_desc){
            .size = vcount * vsize,
            .content = vdata,
        });
        rs->buffers[rs->num_buffers++] = gfx_make_buffer(&(gfx_buffer_desc){
            .type = GFX_BUFFERTYPE_INDEXBUFFER,
            .size = icount * sizeof(*idata),
            .content = idata,
        });

        /* Free intermediate buffers */
        free(idata);
        free(vdata);
    }
}

static mat4 build_transform_for_gltf_node(const cgltf_data* gltf, const cgltf_node* node)
{
    mat4 parent_tform = mat4_id();
    if (node->parent) {
        parent_tform = build_transform_for_gltf_node(gltf, node->parent);
    }

    mat4 tform = mat4_id();
    if (node->has_matrix) {
        /* NOTE: Needs testing, not sure if the element order is correct */
        tform = *(mat4*)node->matrix;
    } else {
        mat4 translate = mat4_id();
        if (node->has_translation)
            translate = mat4_translation((vec3)vec3_new(
                    node->translation[0],
                    node->translation[1],
                    node->translation[2]));

        mat4 rotate = mat4_id();
        if (node->has_rotation)
            rotate = mat4_rotation_quat(quat_new(
                    node->rotation[0],
                    node->rotation[1],
                    node->rotation[2],
                    node->rotation[3]));

        mat4 scale = mat4_id();
        if (node->has_scale)
            scale = mat4_scale(vec3_new(
                    node->scale[0],
                    node->scale[1],
                    node->scale[2]));

        /* NOTE: not sure if the multiplication order is correct */
        tform = mat4_mul_mat4(
                    parent_tform,
                    mat4_mul_mat4(
                        mat4_mul_mat4(scale, rotate),
                        translate
                    )
                );
    }
    return tform;
}

static void gltf_parse_nodes(renderer_scene* rs, const cgltf_data* gltf)
{
    assert(gltf->nodes_count < RENDERER_SCENE_MAX_NODES);

    for (size_t i = 0; i < gltf->nodes_count; ++i) {
        cgltf_node* gltf_node = &gltf->nodes[i];
        /* Ignore nodes without mesh, those are not relevant since we
           bake the transform hierarchy into per-node world space transforms */
        if (gltf_node->mesh) {
            rs->nodes[rs->num_nodes++] = (renderer_node) {
                .mesh = gltf_node->mesh - gltf->meshes,
                .transform = build_transform_for_gltf_node(gltf, gltf_node),
            };
        }
    }
}

rid resmngr_model_from_gltf(resmngr rm, const char* fpath)
{
    rid r = slot_map_insert(&rm->scene_map, 0);
    renderer_scene* rs = slot_map_lookup(&rm->scene_map, r);
    memset(rs, 0, sizeof(*rs));

    cgltf_data* data = 0;
    cgltf_options options = {};
    cgltf_result result;

    result = cgltf_parse_file(&options, fpath, &data);
    if (result != cgltf_result_success)
        return RID_INVALID;

    result = cgltf_load_buffers(&options, data, fpath);
    if (result != cgltf_result_success)
        return RID_INVALID;

    gltf_parse_meshes(rs, data);
    gltf_parse_nodes(rs, data);

    cgltf_free(data);
    return r;
}

void* resmngr_model_lookup(resmngr rm, rid r)
{
    return slot_map_lookup(&rm->scene_map, r);
}

void resmngr_model_delete(resmngr rm, rid r)
{
    struct slot_map* sm = &rm->scene_map;
    renderer_scene* rs = slot_map_lookup(sm, r);

    for (size_t i = 0; i < rs->num_buffers; ++i) {
        gfx_buffer buf = rs->buffers[i];
        gfx_destroy_buffer(buf);
    }

    for (size_t i = 0; i< rs->num_images; ++i) {
        gfx_image img = rs->images[i];
        gfx_destroy_image(img);
    }

    slot_map_remove(sm, r);
}
