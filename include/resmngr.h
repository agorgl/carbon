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
#ifndef _RESMNGR_H_
#define _RESMNGR_H_

#include <slot_map.h>

#define RID_INVALID SM_INVALID_KEY

/* Resource manager type */
typedef struct resmngr* resmngr;

/* Resource handle */
typedef sm_key rid;

/* Main interface */
resmngr resmngr_create();
int resmngr_handle_valid(rid r);
void resmngr_process(resmngr rm);
void resmngr_destroy(resmngr rm);

/* Model resources */
rid resmngr_model_sample(resmngr rm);
rid resmngr_model_from_gltf(resmngr rm, const char* fpath);
void* resmngr_model_lookup(resmngr rm, rid r);
void resmngr_model_delete(resmngr rm, rid r);

/* Font resources */
rid resmngr_font_from_ttf_file(resmngr rm, const char* fpath);
rid resmngr_font_from_ttf_data(resmngr rm, void* data, size_t sz);
void* resmngr_font_lookup(resmngr rm, rid r);
void resmngr_font_delete(resmngr rm, rid r);

#endif /* ! _RESMNGR_H_ */
