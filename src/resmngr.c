#include "resmngr.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "renderer.h"
#include "cgltf.h"
#include "stb_image.h"
#include "text.h"
#include "list.h"
#include "threads.h"
#include "thread_pool.h"

#define RES_TYPE_TEXTURE 1

typedef struct resmngr {
    struct slot_map scene_map;
    struct slot_map font_map;
    struct list_head loaded_queue;
    mtx_t loaded_queue_mtx;
    threadpool_t* worker_pool;
}* resmngr;

typedef struct load_params {
    union {
        const char* fpath;
        struct {
            void* data;
            size_t size;
        };
    };
    enum {
        LOAD_FILE,
        LOAD_MEMORY
    } type;
} load_params;

typedef struct loaded_data {
    int type;
    void* data;
    struct list_head list;
}* loaded_data;

typedef struct loaded_data_texture {
    gfx_image im;
    gfx_image_desc* im_desc;
}* loaded_data_texture;

resmngr resmngr_create()
{
    resmngr rm = calloc(1, sizeof(*rm));
    slot_map_init(&rm->scene_map, sizeof(renderer_scene));
    slot_map_init(&rm->font_map, sizeof(font));
    mtx_init(&rm->loaded_queue_mtx, mtx_plain);
    rm->worker_pool = threadpool_create(8, THREAD_POOL_MAX_QUEUE);
    INIT_LIST_HEAD(&rm->loaded_queue);
    return rm;
}

int resmngr_handle_valid(rid r)
{
    return slot_map_key_valid(r);
}

static void resmngr_loaded_queue_put(resmngr rm, loaded_data ldata)
{
    mtx_lock(&rm->loaded_queue_mtx);
    list_add(&ldata->list, &rm->loaded_queue);
    mtx_unlock(&rm->loaded_queue_mtx);
}

static loaded_data resmngr_loaded_queue_get(resmngr rm)
{
    mtx_lock(&rm->loaded_queue_mtx);
    loaded_data ldata = list_first_entry_or_null(&rm->loaded_queue, struct loaded_data, list);
    if (ldata) {
        list_del(&ldata->list);
    }
    mtx_unlock(&rm->loaded_queue_mtx);
    return ldata;
}

static void upload_image_resource(gfx_image im, gfx_image_desc* desc);

void resmngr_process(resmngr rm)
{
    /* Fetch next loaded resource */
    loaded_data ldata = resmngr_loaded_queue_get(rm);
    if (ldata) {
        switch (ldata->type) {
            case RES_TYPE_TEXTURE: {
                loaded_data_texture tdata = ldata->data;
                upload_image_resource(tdata->im, tdata->im_desc);
                break;
            }
            default:
                break;
        }
        free(ldata->data);
        free(ldata);
    }
}

void resmngr_destroy(resmngr rm)
{
    threadpool_destroy(rm->worker_pool, 0);
    mtx_destroy(&rm->loaded_queue_mtx);
    for (size_t i = 0; i < rm->scene_map.size; ++i) {
        rid r = slot_map_data_to_key(&rm->scene_map, i);
        resmngr_model_delete(rm, r);
    }
    for (size_t i = 0; i < rm->font_map.size; ++i) {
        rid r = slot_map_data_to_key(&rm->font_map, i);
        resmngr_font_delete(rm, r);
    }
    slot_map_destroy(&rm->scene_map);
    slot_map_destroy(&rm->font_map);
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
            prim->material      = gltf_prim->material - gltf->materials;

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

static size_t gltf_texture_index(const cgltf_data* gltf, cgltf_texture* texture)
{
    return texture
        ? (size_t)(texture - gltf->textures)
        : RENDERER_SCENE_INVALID_INDEX;
}

static void gltf_parse_materials(renderer_scene* rs, const cgltf_data* gltf)
{
    assert(gltf->materials_count < RENDERER_SCENE_MAX_MATERIALS);

    for (size_t i = 0; i < gltf->materials_count; ++i) {
        cgltf_material* gltf_mat = &gltf->materials[i];
        renderer_material* rmat = &rs->materials[rs->num_materials++];
        if (gltf_mat->has_pbr_metallic_roughness) {
            cgltf_pbr_metallic_roughness* pbr_mr = &gltf_mat->pbr_metallic_roughness;
            *rmat = (renderer_material) {
                .type = RENDERER_MATERIAL_TYPE_METALLIC,
                .data = {
                    .metallic = {
                        .params = {
                            .base_color_factor = (*(vec4*)pbr_mr->base_color_factor),
                            .emissive_factor   = (*(vec3*)gltf_mat->emissive_factor),
                            .metallic_factor   = pbr_mr->metallic_factor,
                            .roughness_factor  = pbr_mr->roughness_factor,
                        },
                        .images = {
                            .base_color         = gltf_texture_index(gltf, pbr_mr->base_color_texture.texture),
                            .metallic_roughness = gltf_texture_index(gltf, pbr_mr->metallic_roughness_texture.texture),
                            .normal             = gltf_texture_index(gltf, gltf_mat->normal_texture.texture),
                            .occlusion          = gltf_texture_index(gltf, gltf_mat->occlusion_texture.texture),
                            .emissive           = gltf_texture_index(gltf, gltf_mat->emissive_texture.texture),
                        }
                    }
                }
            };
        } else if (gltf_mat->has_pbr_specular_glossiness) {
            assert(0 && "Unimplemented");
        } else if (gltf_mat->unlit) {
            assert(0 && "Unimplemented");
        } else {
            assert(0 && "Unimplemented");
        }
    }
}

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

static void image_mipmaps_populate(gfx_image_desc* desc)
{
    assert(desc->pixel_format == GFX_PIXELFORMAT_RGBA8
        || desc->pixel_format == GFX_PIXELFORMAT_BGRA8
        || desc->pixel_format == GFX_PIXELFORMAT_R8);

    unsigned int pixel_size = desc->pixel_format == GFX_PIXELFORMAT_R8 ? 1 : 4; /* TODO: gfx_pixelformat_bytesize(desc.pixel_format); */
    for (int cube_face = 0; cube_face < GFX_CUBEFACE_NUM; ++cube_face) {
        unsigned int target_width = desc->width, target_height = desc->height;
        for (int level = 1; level < GFX_MAX_MIPMAPS; ++level) {
            unsigned img_size = target_width * target_height * pixel_size;
            unsigned char* source = (unsigned char*)desc->content.subimage[cube_face][level - 1].ptr;
            if (!source)
                break;

            unsigned int source_width = target_width;
            target_width /= 2; target_height /= 2;
            if (target_width < 1 && target_height < 1)
                break;

            unsigned char* target = (unsigned char*)malloc(img_size);
            if (target_width < 1)
                target_width = 1;
            if (target_height < 1)
                target_height = 1;

            for (unsigned int x = 0; x < target_width; ++x) {
                for (unsigned int y = 0; y < target_height; ++y) {
                    for (unsigned int channel = 0; channel < pixel_size; ++channel) {
                        uint32_t color = 0;
                        int sx = x * 2, sy = y * 2;
                        color += source[source_width * pixel_size * sx + sy * pixel_size + channel];
                        color += source[source_width * pixel_size * (sx + 1) + sy * pixel_size + channel];
                        color += source[source_width * pixel_size * (sx + 1) + (sy + 1) * pixel_size + channel];
                        color += source[source_width * pixel_size * sx + (sy + 1) * pixel_size + channel];
                        color /= 4;
                        target[target_width * pixel_size * x + y * pixel_size + channel] = (uint8_t)color;
                    }
                }
            }

            desc->content.subimage[cube_face][level].ptr = target;
            desc->content.subimage[cube_face][level].size = img_size;
            if (desc->num_mipmaps <= level)
                desc->num_mipmaps = level + 1;
        }
    }
}

static void image_mipmaps_free(gfx_image_desc* desc)
{
    for (int cube_face = 0; cube_face < GFX_CUBEFACE_NUM; ++cube_face) {
        for (int i = 1; i < GFX_MAX_MIPMAPS; ++i) {
            gfx_subimage_content* sic = &desc->content.subimage[cube_face][i];
            free((void*)sic->ptr);
        }
    }
}

typedef struct {
    resmngr rm;
    gfx_image im;
    const char* path;
} imgres_thrd_data;

static void image_resource_load(void* data)
{
    imgres_thrd_data* td = data;
    resmngr rm           = td->rm;
    gfx_image im         = td->im;
    const char* path     = td->path;

    /* Load and decode texture data */
    int width, height, channels;
    void* pixels = stbi_load(path, &width, &height, &channels, 4);
    free((void*)path);
    if (!pixels)
        goto cleanup;

    /* Create description */
    gfx_image_desc* im_desc = calloc(1, sizeof(*im_desc));
    *im_desc = (gfx_image_desc){
        .width        = width,
        .height       = height,
        .min_filter   = GFX_FILTER_LINEAR_MIPMAP_LINEAR,
        .mag_filter   = GFX_FILTER_LINEAR,
        .pixel_format = GFX_PIXELFORMAT_RGBA8,
        .content.subimage[0][0] = {
            .ptr = pixels,
            .size = width * height * channels
        }
    };

    /* Populate mipmaps */
    image_mipmaps_populate(im_desc);

    /* Push to loaded queue */
    loaded_data_texture tdata = calloc(1, sizeof(*tdata));
    *tdata = (struct loaded_data_texture) {
        .im_desc = im_desc,
        .im      = im,
    };
    loaded_data ldata = calloc(1, sizeof(*ldata));
    *ldata = (struct loaded_data) {
        .type = RES_TYPE_TEXTURE,
        .data = tdata,
    };
    resmngr_loaded_queue_put(rm, ldata);

cleanup:
    free(td);
}

static void upload_image_resource(gfx_image im, gfx_image_desc* desc)
{
    /* Upload image */
    gfx_init_image(im, desc);

    /* Free texture data from host memory */
    for (int cube_face = 0; cube_face < GFX_CUBEFACE_NUM; ++cube_face) {
        gfx_subimage_content* sic = &desc->content.subimage[cube_face][0];
        free((void*)sic->ptr);
    }
    image_mipmaps_free(desc);
    free(desc);
}

static int gltf_load_textures(renderer_scene* rs, const char* gltf_path, const cgltf_data* gltf, resmngr rm)
{
    assert(gltf->textures_count < RENDERER_SCENE_MAX_IMAGES);
    for (size_t i = 0; i < gltf->textures_count; ++i) {
        /* Get texture location */
        cgltf_texture* gltf_tex = &gltf->textures[i];
        cgltf_image* gltf_img = gltf_tex->image;
        assert(gltf_img->uri);

        /* Construct path to image */
        char* path = calloc(1, strlen(gltf_path) + strlen(gltf_img->uri) + 1);
        path_join(path, gltf_path, gltf_img->uri);

        /* Allocate image handle */
        gfx_image im = gfx_alloc_image();
        rs->images[rs->num_images++] = im;

        /* Prepare thread data */
        imgres_thrd_data* tdata = calloc(1, sizeof(*tdata));
        tdata->rm   = rm;
        tdata->im   = im;
        tdata->path = path;

        /* Launch loader thread */
        threadpool_add(rm->worker_pool, image_resource_load, tdata);
    }
    return 1;
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
    gltf_parse_materials(rs, data);

    if (!gltf_load_textures(rs, fpath, data, rm))
        return RID_INVALID;

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

static rid resmngr_font_from_ttf(resmngr rm, load_params lparams)
{
    rid r = slot_map_insert(&rm->font_map, 0);
    font* fnt = slot_map_lookup(&rm->font_map, r);
    memset(fnt, 0, sizeof(*fnt));

    /* Load font */
    const int atlas_sz = 512;
    const int font_psz = 48;
    fnt->atlas = texture_atlas_new(atlas_sz, atlas_sz, 1);
    switch (lparams.type) {
        case LOAD_FILE:
            fnt->tfont = texture_font_new_from_file(fnt->atlas, font_psz, lparams.fpath);
            break;
        case LOAD_MEMORY:
            fnt->tfont = texture_font_new_from_memory(fnt->atlas, font_psz, lparams.data, lparams.size);
            break;
        default:
            assert(0);
    }
    fnt->tfont->rendermode = GLYPH_RENDER_SIGNED_DISTANCE_FIELD;

    /* Preload glyphs */
    const char* cache = " !\"#$%&'()*+,-./0123456789:;<=>?"
                        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                        "`abcdefghijklmnopqrstuvwxyz{|}~";
    texture_font_load_glyphs(fnt->tfont, cache);

    /* Create atlas image in GPU */
    gfx_image_desc im_desc = (gfx_image_desc){
        .width        = atlas_sz,
        .height       = atlas_sz,
        .min_filter   = GFX_FILTER_LINEAR,
        .mag_filter   = GFX_FILTER_LINEAR,
        .wrap_u       = GFX_WRAP_CLAMP_TO_EDGE,
        .wrap_v       = GFX_WRAP_CLAMP_TO_EDGE,
        .pixel_format = GFX_PIXELFORMAT_R8,
        .content.subimage[0][0] = {
            .ptr  = fnt->atlas->data,
            .size = atlas_sz * atlas_sz * 1
        }
    };
    image_mipmaps_populate(&im_desc);
    fnt->atlas_img = gfx_make_image(&im_desc);
    image_mipmaps_free(&im_desc);

    return r;
}

rid resmngr_font_from_ttf_file(resmngr rm, const char* fpath)
{
    return resmngr_font_from_ttf(rm, (load_params){
        .type  = LOAD_FILE,
        .fpath = fpath
    });
}

rid resmngr_font_from_ttf_data(resmngr rm, void* data, size_t sz)
{
    return resmngr_font_from_ttf(rm, (load_params){
        .type = LOAD_MEMORY,
        .data = data,
        .size = sz
    });
}

void* resmngr_font_lookup(resmngr rm, rid r)
{
    return slot_map_lookup(&rm->font_map, r);
}

void resmngr_font_delete(resmngr rm, rid r)
{
    struct slot_map* sm = &rm->font_map;
    font* fnt = slot_map_lookup(sm, r);
    gfx_destroy_image(fnt->atlas_img);
    texture_font_delete(fnt->tfont);
    texture_atlas_delete(fnt->atlas);
    slot_map_remove(sm, r);
}
