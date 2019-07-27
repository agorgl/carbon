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
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <linmath.h>

typedef struct camera {
    /* Position and Orientation */
    vec3 pos;
    quat rot;
    vec3 eul;
    vec3 vel;
    vec3 angvel;
    /* Attributes */
    float pitch_lim;
    float accel;
    float max_vel;
    float ang_accel;
    float max_angvel;
    float sensitivity;
    /* Cached values */
    mat4 view_mat;
} camera;

enum camera_move_dir {
    CAMERA_MOVE_FORWARD  = 1 << 0,
    CAMERA_MOVE_LEFT     = 1 << 1,
    CAMERA_MOVE_BACKWARD = 1 << 2,
    CAMERA_MOVE_RIGHT    = 1 << 3
};

void camera_defaults(camera* cam);
void camera_move(camera* cam, int move_directions, float dt);
void camera_look(camera* cam, float offx, float offy, float dt);
void camera_update(camera* cam, float dt);
void camera_setpos(camera* cam, vec3 pos);
void camera_setdir(camera* cam, vec3 dir);
mat4 camera_view(camera* cam);

#endif /* ! _CAMERA_H_ */
