#include "ecs.h"
#include <components.h>
#include <string.h>

static void propagate_dirty_flag_system(ecs_rows_t* rows)
{
    transform* tarr = ecs_column(rows, transform, 1);
    for (uint32_t i = 0; i < rows->count;++i) {
        transform* t = &tarr[i];
        t->dirty = 1;
    }
}

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

static void prepare_renderer_inputs_system(ecs_rows_t* rows)
{
    ECS_COLUMN(rows, transform, tarr, 1);
    renderer_inputs* ri = rows->param;

    /* Extend objects array */
    size_t offset = ri->num_objects;
    ri->num_objects += rows->count;
    ri->objects = realloc(ri->objects, ri->num_objects * sizeof(*ri->objects));
    memset(ri->objects + offset, 0, rows->count * sizeof(*ri->objects));

    /* Copy over data */
    for (uint32_t i = 0; i < rows->count; ++i) {
        transform* t = &tarr[i];
        renderer_object* ro = &ri->objects[offset + i];
        ro->wrld_mat = t->world_mat;
    }
}

void ecs_prepare_renderer_inputs(ecs_world_t* world, renderer_inputs* ri)
{
    ecs_entity_t prisys = ecs_lookup(world, "prepare_renderer_inputs_system");
    ecs_run(world, prisys, 0, ri);
}

void ecs_free_render_inputs(ecs_world_t* world, renderer_inputs* ri)
{
    (void) world;
    free(ri->objects);
}

void ecs_setup_internal(ecs_world_t* world)
{
    /* Register internal component types */
    ECS_COMPONENT(world, transform);

    /* Register internal systems */
    ECS_SYSTEM(world, propagate_dirty_flag_system, EcsManual, transform, CONTAINER.EcsContainer);
    ECS_SYSTEM(world, transform_add_system, EcsOnAdd, transform);
    ECS_SYSTEM(world, transform_system, EcsOnUpdate, transform, CASCADE.transform, ID.propagate_dirty_flag_system);
    ECS_SYSTEM(world, prepare_renderer_inputs_system, EcsManual, transform);
}
