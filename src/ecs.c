#include "ecs.h"
#include <components.h>
#include <string.h>

static void transform_add_system(ecs_rows_t* rows)
{
    ECS_COLUMN_COMPONENT(rows, transform, 1);

    for (uint32_t i = 0; i < rows->count; ++i) {
        ecs_set(rows->world, rows->entities[i], transform, {
            .pose = {
                .scale       = (vec3){{1.0, 1.0, 1.0}},
                .rotation    = (quat){{0.0, 0.0, 0.0, 1.0}},
                .translation = (vec3){{0.0, 0.0, 0.0}}
            },
            .dirty = 1
        });
    }
}

static void model_add_system(ecs_rows_t* rows)
{
    ECS_COLUMN_COMPONENT(rows, model, 1);

    for (uint32_t i = 0; i < rows->count; ++i) {
        ecs_set(rows->world, rows->entities[i], model, {.resource = RID_INVALID});
    }
}

static void propagate_dirty_flag_system(ecs_rows_t* rows)
{
    transform* tarr = ecs_column(rows, transform, 1);
    for (uint32_t i = 0; i < rows->count;++i) {
        transform* t = &tarr[i];
        t->dirty = 1;
    }
}

static void transform_system(ecs_rows_t* rows)
{
    ECS_COLUMN(rows, transform, tarr, 1);
    ECS_SHARED_TEST(rows, transform, parent, 2);
    ECS_COLUMN_ENTITY(rows, pdflag, 3);

    /* Parent matrix */
    mat4 pm = parent ? parent->local_mat : mat4_id();

    for (uint32_t i = 0; i < rows->count; ++i) {
        transform* t = &tarr[i];
        if (t->dirty) {
            /* Get current entity */
            ecs_entity_t e = rows->entities[i];

            /* Make world matrix */
            mat4 m = mat4_world(
                    t->pose.translation,
                    t->pose.scale,
                    t->pose.rotation);
            t->local_mat = m;
            t->world_mat = mat4_mul_mat4(pm, m);
            t->dirty = 0;

            /* Propagate dirty flag bellow */
            ECS_TYPE_VAR(ft) = ecs_type_from_entity(rows->world, e);
            ecs_run_w_filter(rows->world,
                             pdflag,
                             rows->delta_time,
                             0, 0, ft, 0);
        }
    }
}

static void merge_renderer_scenes(renderer_scene* tgt_scn, renderer_scene* src_scn, mat4 ptransform)
{
    size_t offs_buffers    = tgt_scn->num_buffers;
    size_t offs_images     = tgt_scn->num_images;
    size_t offs_materials  = tgt_scn->num_materials;
    size_t offs_primitives = tgt_scn->num_primitives;
    size_t offs_meshes     = tgt_scn->num_meshes;
    size_t offs_nodes      = tgt_scn->num_nodes;

    assert(offs_buffers + src_scn->num_buffers < RENDERER_SCENE_MAX_BUFFERS);
    for (size_t i = 0; i < src_scn->num_buffers; ++i) {
        tgt_scn->buffers[offs_buffers + i] = src_scn->buffers[i];
        ++tgt_scn->num_buffers;
    }

    assert(offs_images + src_scn->num_images < RENDERER_SCENE_MAX_IMAGES);
    for (size_t i = 0; i < src_scn->num_images; ++i) {
        tgt_scn->images[offs_images + i] = src_scn->images[i];
        ++tgt_scn->num_images;
    }

    assert(offs_materials + src_scn->num_materials < RENDERER_SCENE_MAX_MATERIALS);
    for (size_t i = 0; i < src_scn->num_materials; ++i) {
        renderer_material* rmat = &tgt_scn->materials[offs_materials + i];
        *rmat = src_scn->materials[i];

        /* Update texture indices */
        if (rmat->type == RENDERER_MATERIAL_TYPE_METALLIC) {
            size_t* index_ptrs[] = {
                &rmat->data.metallic.images.base_color,
                &rmat->data.metallic.images.metallic_roughness,
                &rmat->data.metallic.images.normal,
                &rmat->data.metallic.images.occlusion,
                &rmat->data.metallic.images.emissive,
            };

            size_t num_index_ptrs = sizeof(index_ptrs) / sizeof(index_ptrs[0]);
            for (size_t k = 0; k < num_index_ptrs; ++k) {
                size_t* idxp = index_ptrs[k];
                if (*idxp != RENDERER_SCENE_INVALID_INDEX)
                    *idxp += offs_images;
            }
        } else if (rmat->type == RENDERER_MATERIAL_TYPE_SPECULAR) {
            size_t* index_ptrs[] = {
                &rmat->data.specular.images.diffuse,
                &rmat->data.specular.images.specular_glossiness,
                &rmat->data.specular.images.normal,
                &rmat->data.specular.images.occlusion,
                &rmat->data.specular.images.emissive,
            };

            size_t num_index_ptrs = sizeof(index_ptrs) / sizeof(index_ptrs[0]);
            for (size_t k = 0; k < num_index_ptrs; ++k) {
                size_t* idxp = index_ptrs[k];
                if (*idxp != RENDERER_SCENE_INVALID_INDEX)
                    *idxp += offs_images;
            }
        }
        ++tgt_scn->num_materials;
    }

    assert(offs_primitives + src_scn->num_primitives < RENDERER_SCENE_MAX_PRIMITIVES);
    for (size_t i = 0; i < src_scn->num_primitives; ++i) {
        renderer_primitive* p = &tgt_scn->primitives[offs_primitives + i];
        *p = src_scn->primitives[i];
        if (p->material != RENDERER_SCENE_INVALID_INDEX)
            p->material += offs_materials;
        if (p->vertex_buffer != RENDERER_SCENE_INVALID_INDEX)
            p->vertex_buffer += offs_buffers;
        if (p->index_buffer != RENDERER_SCENE_INVALID_INDEX)
            p->index_buffer += offs_buffers;
        ++tgt_scn->num_primitives;
    }

    assert(offs_meshes + src_scn->num_meshes < RENDERER_SCENE_MAX_MESHES);
    for (size_t i = 0; i < src_scn->num_meshes; ++i) {
        renderer_mesh* m = &tgt_scn->meshes[offs_meshes + i];
        *m = src_scn->meshes[i];
        if (m->first_primitive != RENDERER_SCENE_INVALID_INDEX)
            m->first_primitive += offs_primitives;
        ++tgt_scn->num_meshes;
    }

    assert(offs_nodes + src_scn->num_nodes < RENDERER_SCENE_MAX_NODES);
    for (size_t i = 0; i < src_scn->num_nodes; ++i) {
        renderer_node* n = &tgt_scn->nodes[offs_nodes + i];
        *n = src_scn->nodes[i];
        if (n->mesh != RENDERER_SCENE_INVALID_INDEX)
            n->mesh += offs_meshes;
        n->transform = mat4_mul_mat4(ptransform, n->transform);
        ++tgt_scn->num_nodes;
    }
}

struct pri_params {
    renderer_inputs* ri;
    resmngr rm;
};

static void prepare_renderer_inputs_system(ecs_rows_t* rows)
{
    ECS_COLUMN(rows, transform, tarr, 1);
    ECS_COLUMN(rows, model, marr, 2);
    struct pri_params* pp = rows->param;

    /* Copy over data */
    for (uint32_t i = 0; i < rows->count; ++i) {
        /* Get component ptrs for current entity */
        transform* t = &tarr[i];
        model* m = &marr[i];
        if (resmngr_handle_valid(m->resource)) {
            /* Fetch scene for above model handle */
            renderer_scene* entity_scn = resmngr_model_lookup(pp->rm, m->resource);
            /* Merge given scene into render input scene */
            merge_renderer_scenes(&pp->ri->scene, entity_scn, t->world_mat);
        }
    }
}

void ecs_prepare_renderer_inputs(ecs_world_t* world, renderer_inputs* ri, resmngr rm)
{
    ecs_entity_t prisys = ecs_lookup(world, "prepare_renderer_inputs_system");
    ecs_run(world, prisys, 0, &(struct pri_params){
        .ri = ri,
        .rm = rm
    });
}

void ecs_free_render_inputs(ecs_world_t* world, renderer_inputs* ri)
{
    (void) world;
    (void) ri;
}

void ecs_setup_internal(ecs_world_t* world)
{
    /* Register internal component types */
    ECS_COMPONENT(world, transform);
    ECS_COMPONENT(world, model);

    /* Register internal systems */
    ECS_SYSTEM(world, propagate_dirty_flag_system, EcsManual, transform, CONTAINER.EcsContainer);
    ECS_SYSTEM(world, transform_add_system, EcsOnAdd, transform);
    ECS_SYSTEM(world, transform_system, EcsOnUpdate, transform, CASCADE.transform, ID.propagate_dirty_flag_system);
    ECS_SYSTEM(world, prepare_renderer_inputs_system, EcsManual, transform, model);
    ECS_SYSTEM(world, model_add_system, EcsOnAdd, model);
}
