#include "ecs.h"
#include <components.h>
#include <string.h>

static struct {
    ecs_query_t* prm_query;
    ecs_query_t* prl_query;
    ecs_query_t* prc_query;
} ecs_internal;

static void transform_add_system(ecs_iter_t* it)
{
    ECS_COLUMN_COMPONENT(it, transform, 1);

    for (int32_t i = 0; i < it->count; ++i) {
        ecs_set(it->world, it->entities[i], transform, {
            .pose = {
                .scale       = (vec3){{1.0, 1.0, 1.0}},
                .rotation    = (quat){{0.0, 0.0, 0.0, 1.0}},
                .translation = (vec3){{0.0, 0.0, 0.0}}
            },
            .dirty = 1
        });
    }
}

static void model_add_system(ecs_iter_t* it)
{
    ECS_COLUMN_COMPONENT(it, model, 1);

    for (int32_t i = 0; i < it->count; ++i) {
        ecs_set(it->world, it->entities[i], model, {.resource = RID_INVALID});
    }
}

static void camera_add_system(ecs_iter_t* it)
{
    ECS_COLUMN_COMPONENT(it, camera, 1);

    for (int32_t i = 0; i < it->count; ++i) {
        camera cam;
        camera_defaults(&cam);
        ecs_set_ptr(it->world, it->entities[i], camera, &cam);
    }
}

static void transform_update_subtree(ecs_world_t* world, ecs_entity_t ecs_entity(transform), ecs_entity_t e, transform* t, transform* tpar)
{
    /* Parent matrix */
    mat4 pm = tpar ? tpar->world_mat : mat4_id();

    /* Update entity */
    if (t->dirty) {
        t->local_mat = mat4_world(
            t->pose.translation,
            t->pose.scale,
            t->pose.rotation
        );
    }
    t->world_mat = mat4_mul_mat4(pm, t->local_mat);
    t->dirty = 0;

    /* Update children */
    ecs_iter_t it = ecs_scope_iter(world, e);
    while (ecs_scope_next(&it)) {
        int32_t t_index = ecs_table_component_index(&it, ecs_entity(transform));
        transform* tarr = ecs_table_column(&it, t_index);

        /* Recursively update subtrees */
        for (int32_t i = 0; i < it.count; i++) {
            transform* tc = &tarr[i];
            transform_update_subtree(world, ecs_entity(transform), it.entities[i], tc, t);
        }
    }
}

static void transform_system(ecs_iter_t* it)
{
    ecs_entity_t ecs_entity(transform) = ecs_column_entity(it, 1);
    transform* tarr = ecs_column(it, transform, 1);
    transform* parent = ecs_column(it, transform, 2);

    for (int32_t i = 0; i < it->count; ++i) {
        transform* t = &tarr[i];
        if (t->dirty) {
            ecs_entity_t e = it->entities[i];
            transform_update_subtree(it->world, ecs_entity(transform), e, t, parent);
        }
    }
}

static void camera_system(ecs_iter_t* it)
{
    camera* carr = ecs_column(it, camera, 1);

    for (int32_t i = 0; i < it->count; ++i) {
        camera* c = &carr[i];
        camera_update(c, it->delta_time);
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
    size_t offs_lights     = tgt_scn->num_lights;

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

    assert(offs_lights + src_scn->num_lights < RENDERER_SCENE_MAX_LIGHTS);
    for (size_t i = 0; i < src_scn->num_lights; ++i) {
        tgt_scn->lights[offs_lights + i] = src_scn->lights[i];
        ++tgt_scn->num_lights;
    }
}

struct pri_params {
    renderer_inputs* ri;
    resmngr rm;
};

static void prepare_renderer_model_inputs_system(ecs_iter_t* it)
{
    ECS_COLUMN(it, transform, tarr, 1);
    ECS_COLUMN(it, model, marr, 2);
    struct pri_params* pp = it->param;

    /* Copy over data */
    for (int32_t i = 0; i < it->count; ++i) {
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

static void prepare_renderer_light_inputs_system(ecs_iter_t* it)
{
    ECS_COLUMN(it, transform, tarr, 1);
    ECS_COLUMN(it, light, larr, 2);
    struct pri_params* pp = it->param;

    /* Copy over data */
    for (int32_t i = 0; i < it->count; ++i) {
        /* Get component ptrs for current entity */
        transform* t = &tarr[i];
        light* l = &larr[i];
        /* Create empty scene with current light */
        renderer_scene lscn = (renderer_scene){
            .lights = {
                [0] = (renderer_light){
                    .color     = l->color,
                    .intensity = l->intensity,
                    .position  = t->pose.translation,
                },
            },
            .num_lights = 1,
        };
        /* Merge given scene into render input scene */
        merge_renderer_scenes(&pp->ri->scene, &lscn, t->world_mat);
    }
}

void ecs_prepare_renderer_inputs(ecs_world_t* world, renderer_inputs* ri, resmngr rm)
{
    (void)world;

    ecs_iter_t prm_it = ecs_query_iter(ecs_internal.prm_query);
    prm_it.param = &(struct pri_params){ .ri = ri, .rm = rm };
    while (ecs_query_next(&prm_it)) {
        prepare_renderer_model_inputs_system(&prm_it);
    }

    ecs_iter_t prl_it = ecs_query_iter(ecs_internal.prl_query);
    prl_it.param = &(struct pri_params){ .ri = ri, .rm = rm };
    while (ecs_query_next(&prl_it)) {
        prepare_renderer_light_inputs_system(&prl_it);
    }
}

void ecs_free_render_inputs(ecs_world_t* world, renderer_inputs* ri)
{
    (void) world;
    (void) ri;
}

void ecs_fetch_cameras(ecs_world_t* world, void* cameras[ECS_MAX_CAMERAS], size_t* num_cameras)
{
    (void)world;

    *num_cameras = 0;
    ecs_iter_t it = ecs_query_iter(ecs_internal.prc_query);
    while (ecs_query_next(&it)) {
        camera* carr = ecs_column(&it, camera, 1);
        for (int32_t i = 0; i < it.count; ++i) {
            camera* c = &carr[i];
            cameras[(*num_cameras)++] = c;
            if (*num_cameras >= ECS_MAX_CAMERAS)
                break;
        }
    }
}

void ecs_setup_internal(ecs_world_t* world)
{
    /* Register internal component types */
    ECS_COMPONENT(world, transform);
    ECS_COMPONENT(world, model);
    ECS_COMPONENT(world, light);
    ECS_COMPONENT(world, camera);

    /* Register internal triggers */
    ECS_TRIGGER(world, transform_add_system, EcsOnAdd, transform);
    ECS_TRIGGER(world, model_add_system, EcsOnAdd, model);
    ECS_TRIGGER(world, camera_add_system, EcsOnAdd, camera);

    /* Register internal systems */
    ECS_SYSTEM(world, transform_system, EcsOnUpdate, transform, CASCADE:transform);
    ECS_SYSTEM(world, camera_system, EcsOnUpdate, camera);

    /* Create queries */
    ecs_query_t* prm_query = ecs_query_new(world, "transform, model");
    ecs_query_t* prl_query = ecs_query_new(world, "transform, light");
    ecs_query_t* prc_query = ecs_query_new(world, "camera");
    ecs_internal.prm_query = prm_query;
    ecs_internal.prl_query = prl_query;
    ecs_internal.prc_query = prc_query;
}
