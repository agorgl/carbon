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
#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <stdlib.h>
#include <linmath.h>
#include <gfx.h>

#define RENDERER_SCENE_INVALID_INDEX (-1)
#define RENDERER_SCENE_MAX_BUFFERS (16)
#define RENDERER_SCENE_MAX_IMAGES (16)
#define RENDERER_SCENE_MAX_MATERIALS (16)
#define RENDERER_SCENE_MAX_PIPELINES (16)
#define RENDERER_SCENE_MAX_PRIMITIVES (16)
#define RENDERER_SCENE_MAX_MESHES (16)
#define RENDERER_SCENE_MAX_NODES (16)

/* Renderer type */
typedef struct renderer* renderer;

/* Renderer initialization parameters */
typedef struct renderer_params {
    int width;
    int height;
} renderer_params;

/* All materials grouped */
typedef struct renderer_material {
    union {
        /* Metallic material data */
        struct {
            /* Metallic material params */
            struct {
                vec4 base_color_factor;
                vec3 emissive_factor;
                float metallic_factor;
                float roughness_factor;
            } params;
            /* Metallic material images */
            struct {
                size_t base_color;
                size_t metallic_roughness;
                size_t normal;
                size_t occlusion;
                size_t emissive;
            } images;
        } metallic;
        /* Specular material data */
        struct {
            /* Specular material params */
            struct {
                vec4 diffuse_factor;
                vec3 specular_factor;
                vec3 emissive_factor;
                float glossiness_factor;
            } params;
            /* Specular material images */
            struct {
                size_t diffuse;
                size_t specular_glossiness;
                size_t normal;
                size_t occlusion;
                size_t emissive;
            } images;
        } specular;
    } data;
    enum {
        RENDERER_MATERIAL_TYPE_METALLIC,
        RENDERER_MATERIAL_TYPE_SPECULAR,
    } type;
} renderer_material;

/* A 'primitive' (aka submesh) contains everything needed to issue a draw call */
typedef struct renderer_primitive {
    size_t pipeline;      /* Index into scene.pipelines array */
    size_t material;      /* Index into scene.materials array */
    size_t vertex_buffer; /* Index into scene.buffers array for vertex buffer */
    size_t index_buffer;  /* Index into scene.buffers array for index buffer, or RENDERER_SCENE_INVALID_INDEX */
    size_t base_element;  /* Index of first index or vertex to draw */
    size_t num_elements;  /* Number of vertices or indices to draw */
} renderer_primitive;

/* A mesh is just a group of primitives (aka submeshes) */
typedef struct renderer_mesh {
    int first_primitive; /* Index into scene.primitives */
    int num_primitives;
} renderer_mesh;

/* A node associates a transform with an mesh */
typedef struct renderer_node {
    size_t mesh; /* Index into scene.meshes */
    mat4 transform;
} renderer_node;

/* The complete scene */
typedef struct renderer_scene {
    sg_buffer          buffers[RENDERER_SCENE_MAX_BUFFERS];
    sg_image           images[RENDERER_SCENE_MAX_IMAGES];
    sg_pipeline        pipelines[RENDERER_SCENE_MAX_PIPELINES];
    renderer_material  materials[RENDERER_SCENE_MAX_MATERIALS];
    renderer_primitive primitives[RENDERER_SCENE_MAX_PRIMITIVES];
    renderer_mesh      meshes[RENDERER_SCENE_MAX_MESHES];
    renderer_node      nodes[RENDERER_SCENE_MAX_NODES];
    size_t num_buffers;
    size_t num_images;
    size_t num_pipelines;
    size_t num_materials;
    size_t num_primitives;
    size_t num_meshes;
    size_t num_nodes;
} renderer_scene;

typedef struct renderer_inputs {
    renderer_scene scene;
} renderer_inputs;

renderer renderer_create(renderer_params* params);
void renderer_frame(renderer r, renderer_inputs ri);
void renderer_destroy(renderer r);

#endif /* ! _RENDERER_H_ */
