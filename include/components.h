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
#ifndef _COMPONENTS_H_
#define _COMPONENTS_H_

#include <flecs.h>
#include "linmath.h"
#include "resmngr.h"
#include "camera.h"

typedef struct transform {
    /* Individual components */
    struct {
        vec3 scale;
        quat rotation;
        vec3 translation;
    } pose;
    /* Indicates that cached matrices should be refreshed */
    int dirty;
    /* Cached combined matrices */
    mat4 local_mat;
    mat4 world_mat;
} transform;

typedef struct model {
    /* Associated resource handle */
    rid resource;
} model;

typedef struct light {
    /* Normalized color value */
    vec3 color;
    /* Intensity value, later multiplied with color */
    float intensity;
} light;

#define IMPORT_COMPONENT(world, component) \
    ecs_entity_t ecs_entity(component) = ecs_lookup(world, #component); \
    (void)ecs_entity(component);

#define IMPORT_COMPONENTS(world)       \
    IMPORT_COMPONENT(world, transform) \
    IMPORT_COMPONENT(world, model)     \
    IMPORT_COMPONENT(world, light)     \
    IMPORT_COMPONENT(world, camera)

#endif /* ! _COMPONENTS_H_ */
