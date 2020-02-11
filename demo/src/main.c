/*********************************************************************************************************************/
/*                                                  /===-_---~~~~~~~~~------____                                     */
/*                                                 |===-~___                _,-'                                     */
/*                  -==\\                         `//~\\   ~~~~`---.___.-~~                                          */
/*              ______-==|                         | |  \\           _-~`                                            */
/*        __--~~~  ,-/-==\\                        | |   `\        ,'                                                */
/*     _-~       /'    |  \\                      / /      \      /                                                  */
/*   .'        /       |   \\                   /' /        \   /'                                                   */
/*  /  ____  /         |    \`\.__/-~~ ~ \ _ _/'  /          \/'                                                     */
/* /-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`                                                      */
/*                   \_|      /        _)   ;  ),   __--~~                                                           */
/*                     '~~--_/      _-~/-  / \   '-~ \                                                               */
/*                    {\__--_/}    / \\_>- )<__\      \                                                              */
/*                    /'   (_/  _-~  | |__>--<__|      |                                                             */
/*                   |0  0 _/) )-~     | |__>--<__|     |                                                            */
/*                   / /~ ,_/       / /__>---<__/      |                                                             */
/*                  o o _//        /-~_>---<__-~      /                                                              */
/*                  (^(~          /~_>---<__-      _-~                                                               */
/*                 ,/|           /__>--<__/     _-~                                                                  */
/*              ,//('(          |__>--<__|     /                  .----_                                             */
/*             ( ( '))          |__>--<__|    |                 /' _---_~\                                           */
/*          `-)) )) (           |__>--<__|    |               /'  /     ~\`\                                         */
/*         ,/,'//( (             \__>--<__\    \            /'  //        ||                                         */
/*       ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'                                          */
/*     `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/                                                     */
/*   ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~                                                       */
/*    ;'( ')/ ,)(                              ~~~~~~~~~~                                                            */
/*   ' ') '( (/                                                                                                      */
/*     '   '  `                                                                                                      */
/*********************************************************************************************************************/
#include <carbon.h>

typedef struct spin {
    float speed;
} spin;

static void spin_system(ecs_rows_t* rows)
{
    ECS_COLUMN(rows, transform, tarr, 1);
    ECS_COLUMN(rows, spin, sarr, 2);

    for (uint32_t i = 0; i < rows->count; ++i) {
        transform* t = &tarr[i];
        spin* s = &sarr[i];
        t->pose.rotation = quat_mul_quat(
            t->pose.rotation,
            quat_rotation_y(s->speed)
        );
        t->dirty = 1;
    }
}

static void setup_spin_system(ecs_world_t* world)
{
    ECS_COMPONENT(world, spin);
    ECS_SYSTEM(world, spin_system, EcsOnUpdate, transform, spin);
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    /* Initialize internal library structures */
    carbon_setup();

    /* Create engine instance with given params */
    engine_params params = (engine_params){
        .width  = 1280,
        .height = 720,
    };
    engine engine = engine_create(&params);

    /* Fetch the world instance */
    ecs_world_t* world = engine_world(engine);

    /* Fetch the resource manager instance */
    resmngr rmgr = engine_resmngr(engine);

    /* Create demo component and system */
    setup_spin_system(world);

    /* Declare transform type */
    ecs_entity_t ecs_entity(transform) = ecs_lookup(world, "transform");

    /* Declare spin type */
    ecs_entity_t ecs_entity(spin) = ecs_lookup(world, "spin");

    /* Declare model type */
    ecs_entity_t ecs_entity(model) = ecs_lookup(world, "model");

    /* Declare light type */
    ecs_entity_t ecs_entity(light) = ecs_lookup(world, "light");

    /* Create sample model resource */
    rid sample_model = resmngr_model_sample(rmgr);

    /* Create parent entity */
    ECS_ENTITY(world, p, transform, model);
    ecs_set(world, p, transform, {
        .pose = {
            .translation = (vec3){{0.0, 0.0, 0.0}},
            .scale = (vec3){{1.5, 1.5, 1.5}},
            .rotation = quat_id()
        },
        .dirty = 1,
    });
    ecs_set(world, p, model, { .resource = sample_model });

    /* Create first child entity */
    ECS_ENTITY(world, c1, transform, model);
    ecs_set(world, c1, transform, {
        .pose = {
            .translation = (vec3){{-2.5, 0.0, 0.0}},
            .scale = (vec3){{0.8, 0.8, 0.8}},
            .rotation = quat_id()
        },
        .dirty = 1,
    });
    ecs_set(world, c1, spin, { .speed = 0.1f, });
    ecs_set(world, c1, model, { .resource = sample_model });
    ecs_adopt(world, c1, p);

    /* Create second child entity */
    ECS_ENTITY(world, c2, transform, model);
    ecs_set(world, c2, transform, {
        .pose = {
            .translation = (vec3){{2.5, 0.0, 0.0}},
            .scale = (vec3){{0.6, 0.6, 0.6}},
            .rotation = quat_id()
        },
        .dirty = 1,
    });
    ecs_set(world, c2, spin, { .speed = 0.1f, });
    ecs_set(world, c2, model, { .resource = sample_model });
    ecs_adopt(world, c2, p);

    /* Run */
    engine_run(engine);

    /* Cleanup allocated resources */
    engine_destroy(engine);

    return 0;
}
