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
#ifndef _TEXTURE_ATLAS_H_
#define _TEXTURE_ATLAS_H_

#include <stdlib.h>

/**
 * Texture atlas
 *
 * A texture atlas is used to pack several small regions into a single texture.
 *
 * The actual implementation is based on the article by Jukka Jylänki :
 * "A Thousand Ways to Pack the Bin - A Practical Approach to Two-Dimensional
 * Rectangle Bin Packing", February 27, 2010.
 *
 * More precisely, this is an implementation of the Skyline Bottom-Left
 * algorithm based on C++ sources provided by Jukka Jylänki at:
 * http://clb.demon.fi/files/RectangleBinPack/
 *
 *
 * Example Usage:
 *
 * ...
 *
 * // Creates a new atlas of 512x512 with a depth of 1
 * texture_atlas* atlas = texture_atlas_new(512, 512, 1);
 *
 * // Allocates a region of 20x20
 * ivec4 region = texture_atlas_get_region(atlas, 20, 20);
 *
 * // Fill region with some data
 * texture_atlas_set_region(atlas, region.x, region.y, region.width, region.height, data, stride)
 * ...
 */

/* Tuple of 4 ints */
typedef union {
    int data[4];
    struct { int x, y, z, w; };
    struct { int x_, y_, width, height; };
    struct { int r, g, b, a; };
} ivec4;

/* Tuple of 3 ints */
typedef union {
    int data[3];
    struct { int x, y, z; };
    struct { int r, g, b; };
} ivec3;

/* A texture atlas is used to pack several small regions into a single texture */
typedef struct texture_atlas {
    /* Allocated nodes */
    struct vector* nodes;
    /* Width (in pixels) of the underlying texture */
    size_t width;
    /* Height (in pixels) of the underlying texture */
    size_t height;
    /* Depth (in bytes) of the underlying texture */
    size_t depth;
    /* Allocated surface size */
    size_t used;
    /* Atlas data */
    unsigned char* data;
} texture_atlas;

/* Creates a new empty texture atlas */
texture_atlas* texture_atlas_new(const size_t width, const size_t height, const size_t depth);

/* Deletes a texture atlas */
void texture_atlas_delete(texture_atlas* self);

/* Allocate a new region in the atlas
 * Takes width and height of the region to allocate
 * Returns coordinates of the allocated region */
ivec4 texture_atlas_get_region(texture_atlas* self, const size_t width, const size_t height);

/* Upload data to the specified atlas region */
void texture_atlas_set_region(texture_atlas* self,
                              const size_t x, const size_t y,
                              const size_t width, const size_t height,
                              const unsigned char* data, const size_t stride);

/* Remove all allocated regions from the atlas */
void texture_atlas_clear(texture_atlas* self);

#endif /* ! _TEXTURE_ATLAS_H_ */
