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

static void spin_system(ecs_iter_t* it)
{
    transform* tarr = ecs_column(it, transform, 1);
    spin* sarr = ecs_column(it, spin, 2);

    for (int32_t i = 0; i < it->count; ++i) {
        transform* t = &tarr[i];
        spin* s = &sarr[i];
        t->pose.rotation = quat_mul_quat(
            t->pose.rotation,
            quat_rotation_y(s->speed)
        );
        t->dirty = 1;
    }
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

    /* Import internal component handles */
    IMPORT_COMPONENTS(world);

    /* Create demo component and system */
    ECS_COMPONENT(world, spin);
    ECS_SYSTEM(world, spin_system, EcsOnUpdate, transform, spin);

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
    ecs_add_entity(world, c1, ECS_CHILDOF | p);

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
    ecs_add_entity(world, c2, ECS_CHILDOF | p);

    /* Create light entity */
    ECS_ENTITY(world, l, transform, light);
    ecs_set(world, l, transform, {
        .pose = {
            .translation = (vec3){{0.2, 1.0, 0.2}},
            .scale = (vec3){{0.0, 0.0, 0.0}},
            .rotation = quat_id()
        },
        .dirty = 1
    });
    ecs_set(world, l, light, {
        .color = cct(5500),
        .intensity = 120000,
    });

    /* Create camera entity */
    ECS_ENTITY(world, c, camera);
    camera* cam = ecs_get_mut(world, c, camera, 0);
    camera_setpos(cam, vec3_new(0.0f, 1.5f, 6.0f));
    camera_setdir(cam, vec3_sub(vec3_zero(), cam->pos));

    /* Run */
    engine_run(engine);

    /* Cleanup allocated resources */
    engine_destroy(engine);

    return 0;
}
