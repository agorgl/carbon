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
#ifndef _GFX_H_
#define _GFX_H_

#include "sokol_gfx.h"

typedef sg_buffer                       gfx_buffer;
typedef sg_image                        gfx_image;
typedef sg_shader                       gfx_shader;
typedef sg_pipeline                     gfx_pipeline;
typedef sg_pass                         gfx_pass;
typedef sg_context                      gfx_context;

typedef sg_backend                      gfx_backend;
typedef sg_feature                      gfx_feature;
typedef sg_resource_state               gfx_resource_state;
typedef sg_usage                        gfx_usage;
typedef sg_buffer_type                  gfx_buffer_type;
typedef sg_index_type                   gfx_index_type;
typedef sg_image_type                   gfx_image_type;
typedef sg_cube_face                    gfx_cube_face;
typedef sg_shader_stage                 gfx_shader_stage;
typedef sg_pixel_format                 gfx_pixel_format;
typedef sg_primitive_type               gfx_primitive_type;
typedef sg_filter                       gfx_filter;
typedef sg_wrap                         gfx_wrap;
typedef sg_vertex_format                gfx_vertex_format;
typedef sg_vertex_step                  gfx_vertex_step;
typedef sg_uniform_type                 gfx_uniform_type;
typedef sg_cull_mode                    gfx_cull_mode;
typedef sg_face_winding                 gfx_face_winding;
typedef sg_compare_func                 gfx_compare_func;
typedef sg_stencil_op                   gfx_stencil_op;
typedef sg_blend_factor                 gfx_blend_factor;
typedef sg_blend_op                     gfx_blend_op;
typedef sg_color_mask                   gfx_color_mask;
typedef sg_action                       gfx_action;

#define GFX_BACKEND_GLCORE33                     SG_BACKEND_GLCORE33
#define GFX_BACKEND_GLES2                        SG_BACKEND_GLES2
#define GFX_BACKEND_GLES3                        SG_BACKEND_GLES3
#define GFX_BACKEND_D3D11                        SG_BACKEND_D3D11
#define GFX_BACKEND_METAL_IOS                    SG_BACKEND_METAL_IOS
#define GFX_BACKEND_METAL_MACOS                  SG_BACKEND_METAL_MACOS
#define GFX_BACKEND_METAL_SIMULATOR              SG_BACKEND_METAL_SIMULATOR
#define GFX_BACKEND_DUMMY                        SG_BACKEND_DUMMY

#define GFX_FEATURE_INSTANCING                   SG_FEATURE_INSTANCING
#define GFX_FEATURE_TEXTURE_COMPRESSION_DXT      SG_FEATURE_TEXTURE_COMPRESSION_DXT
#define GFX_FEATURE_TEXTURE_COMPRESSION_PVRTC    SG_FEATURE_TEXTURE_COMPRESSION_PVRTC
#define GFX_FEATURE_TEXTURE_COMPRESSION_ATC      SG_FEATURE_TEXTURE_COMPRESSION_ATC
#define GFX_FEATURE_TEXTURE_COMPRESSION_ETC2     SG_FEATURE_TEXTURE_COMPRESSION_ETC2
#define GFX_FEATURE_TEXTURE_FLOAT                SG_FEATURE_TEXTURE_FLOAT
#define GFX_FEATURE_TEXTURE_HALF_FLOAT           SG_FEATURE_TEXTURE_HALF_FLOAT
#define GFX_FEATURE_ORIGIN_BOTTOM_LEFT           SG_FEATURE_ORIGIN_BOTTOM_LEFT
#define GFX_FEATURE_ORIGIN_TOP_LEFT              SG_FEATURE_ORIGIN_TOP_LEFT
#define GFX_FEATURE_MSAA_RENDER_TARGETS          SG_FEATURE_MSAA_RENDER_TARGETS
#define GFX_FEATURE_PACKED_VERTEX_FORMAT_10_2    SG_FEATURE_PACKED_VERTEX_FORMAT_10_2
#define GFX_FEATURE_MULTIPLE_RENDER_TARGET       SG_FEATURE_MULTIPLE_RENDER_TARGET
#define GFX_FEATURE_IMAGETYPE_3D                 SG_FEATURE_IMAGETYPE_3D
#define GFX_FEATURE_IMAGETYPE_ARRAY              SG_FEATURE_IMAGETYPE_ARRAY

#define GFX_NUM_FEATURES                         SG_NUM_FEATURES
#define GFX_RESOURCESTATE_INITIAL                SG_RESOURCESTATE_INITIAL
#define GFX_RESOURCESTATE_ALLOC                  SG_RESOURCESTATE_ALLOC
#define GFX_RESOURCESTATE_VALID                  SG_RESOURCESTATE_VALID
#define GFX_RESOURCESTATE_FAILED                 SG_RESOURCESTATE_FAILED
#define GFX_RESOURCESTATE_INVALID                SG_RESOURCESTATE_INVALID

#define _GFX_USAGE_DEFAULT                       _SG_USAGE_DEFAULT
#define GFX_USAGE_IMMUTABLE                      SG_USAGE_IMMUTABLE
#define GFX_USAGE_DYNAMIC                        SG_USAGE_DYNAMIC
#define GFX_USAGE_STREAM                         SG_USAGE_STREAM
#define _GFX_USAGE_NUM                           _SG_USAGE_NUM

#define _GFX_BUFFERTYPE_DEFAULT                  _SG_BUFFERTYPE_DEFAULT
#define GFX_BUFFERTYPE_VERTEXBUFFER              SG_BUFFERTYPE_VERTEXBUFFER
#define GFX_BUFFERTYPE_INDEXBUFFER               SG_BUFFERTYPE_INDEXBUFFER
#define _GFX_BUFFERTYPE_NUM                      _SG_BUFFERTYPE_NUM

#define _GFX_INDEXTYPE_DEFAULT                   _SG_INDEXTYPE_DEFAULT
#define GFX_INDEXTYPE_NONE                       SG_INDEXTYPE_NONE
#define GFX_INDEXTYPE_UINT16                     SG_INDEXTYPE_UINT16
#define GFX_INDEXTYPE_UINT32                     SG_INDEXTYPE_UINT32
#define _GFX_INDEXTYPE_NUM                       _SG_INDEXTYPE_NUM

#define _GFX_IMAGETYPE_DEFAULT                   _SG_IMAGETYPE_DEFAULT
#define GFX_IMAGETYPE_2D                         SG_IMAGETYPE_2D
#define GFX_IMAGETYPE_CUBE                       SG_IMAGETYPE_CUBE
#define GFX_IMAGETYPE_3D                         SG_IMAGETYPE_3D
#define GFX_IMAGETYPE_ARRAY                      SG_IMAGETYPE_ARRAY
#define _GFX_IMAGETYPE_NUM                       _SG_IMAGETYPE_NUM

#define GFX_CUBEFACE_POS_X                       SG_CUBEFACE_POS_X
#define GFX_CUBEFACE_NEG_X                       SG_CUBEFACE_NEG_X
#define GFX_CUBEFACE_POS_Y                       SG_CUBEFACE_POS_Y
#define GFX_CUBEFACE_NEG_Y                       SG_CUBEFACE_NEG_Y
#define GFX_CUBEFACE_POS_Z                       SG_CUBEFACE_POS_Z
#define GFX_CUBEFACE_NEG_Z                       SG_CUBEFACE_NEG_Z
#define GFX_CUBEFACE_NUM                         SG_CUBEFACE_NUM

#define GFX_SHADERSTAGE_VS                       SG_SHADERSTAGE_VS
#define GFX_SHADERSTAGE_FS                       SG_SHADERSTAGE_FS

#define _GFX_PIXELFORMAT_DEFAULT                 _SG_PIXELFORMAT_DEFAULT
#define GFX_PIXELFORMAT_NONE                     SG_PIXELFORMAT_NONE
#define GFX_PIXELFORMAT_RGBA8                    SG_PIXELFORMAT_RGBA8
#define GFX_PIXELFORMAT_RGB8                     SG_PIXELFORMAT_RGB8
#define GFX_PIXELFORMAT_RGBA4                    SG_PIXELFORMAT_RGBA4
#define GFX_PIXELFORMAT_R5G6B5                   SG_PIXELFORMAT_R5G6B5
#define GFX_PIXELFORMAT_R5G5B5A1                 SG_PIXELFORMAT_R5G5B5A1
#define GFX_PIXELFORMAT_R10G10B10A2              SG_PIXELFORMAT_R10G10B10A2
#define GFX_PIXELFORMAT_RGBA32F                  SG_PIXELFORMAT_RGBA32F
#define GFX_PIXELFORMAT_RGBA16F                  SG_PIXELFORMAT_RGBA16F
#define GFX_PIXELFORMAT_R32F                     SG_PIXELFORMAT_R32F
#define GFX_PIXELFORMAT_R16F                     SG_PIXELFORMAT_R16F
#define GFX_PIXELFORMAT_L8                       SG_PIXELFORMAT_L8
#define GFX_PIXELFORMAT_DXT1                     SG_PIXELFORMAT_DXT1
#define GFX_PIXELFORMAT_DXT3                     SG_PIXELFORMAT_DXT3
#define GFX_PIXELFORMAT_DXT5                     SG_PIXELFORMAT_DXT5
#define GFX_PIXELFORMAT_DEPTH                    SG_PIXELFORMAT_DEPTH
#define GFX_PIXELFORMAT_DEPTHSTENCIL             SG_PIXELFORMAT_DEPTHSTENCIL
#define GFX_PIXELFORMAT_PVRTC2_RGB               SG_PIXELFORMAT_PVRTC2_RGB
#define GFX_PIXELFORMAT_PVRTC4_RGB               SG_PIXELFORMAT_PVRTC4_RGB
#define GFX_PIXELFORMAT_PVRTC2_RGBA              SG_PIXELFORMAT_PVRTC2_RGBA
#define GFX_PIXELFORMAT_PVRTC4_RGBA              SG_PIXELFORMAT_PVRTC4_RGBA
#define GFX_PIXELFORMAT_ETC2_RGB8                SG_PIXELFORMAT_ETC2_RGB8
#define GFX_PIXELFORMAT_ETC2_SRGB8               SG_PIXELFORMAT_ETC2_SRGB8
#define _GFX_PIXELFORMAT_NUM                     _SG_PIXELFORMAT_NUM

#define _GFX_PRIMITIVETYPE_DEFAULT               _SG_PRIMITIVETYPE_DEFAULT
#define GFX_PRIMITIVETYPE_POINTS                 SG_PRIMITIVETYPE_POINTS
#define GFX_PRIMITIVETYPE_LINES                  SG_PRIMITIVETYPE_LINES
#define GFX_PRIMITIVETYPE_LINE_STRIP             SG_PRIMITIVETYPE_LINE_STRIP
#define GFX_PRIMITIVETYPE_TRIANGLES              SG_PRIMITIVETYPE_TRIANGLES
#define GFX_PRIMITIVETYPE_TRIANGLE_STRIP         SG_PRIMITIVETYPE_TRIANGLE_STRIP
#define _GFX_PRIMITIVETYPE_NUM                   _SG_PRIMITIVETYPE_NUM

#define _GFX_FILTER_DEFAULT                      _SG_FILTER_DEFAULT
#define GFX_FILTER_NEAREST                       SG_FILTER_NEAREST
#define GFX_FILTER_LINEAR                        SG_FILTER_LINEAR
#define GFX_FILTER_NEAREST_MIPMAP_NEAREST        SG_FILTER_NEAREST_MIPMAP_NEAREST
#define GFX_FILTER_NEAREST_MIPMAP_LINEAR         SG_FILTER_NEAREST_MIPMAP_LINEAR
#define GFX_FILTER_LINEAR_MIPMAP_NEAREST         SG_FILTER_LINEAR_MIPMAP_NEAREST
#define GFX_FILTER_LINEAR_MIPMAP_LINEAR          SG_FILTER_LINEAR_MIPMAP_LINEAR
#define _GFX_FILTER_NUM                          _SG_FILTER_NUM

#define _GFX_WRAP_DEFAULT                        _SG_WRAP_DEFAULT
#define GFX_WRAP_REPEAT                          SG_WRAP_REPEAT
#define GFX_WRAP_CLAMP_TO_EDGE                   SG_WRAP_CLAMP_TO_EDGE
#define GFX_WRAP_MIRRORED_REPEAT                 SG_WRAP_MIRRORED_REPEAT
#define _GFX_WRAP_NUM                            _SG_WRAP_NUM

#define GFX_VERTEXFORMAT_INVALID                 SG_VERTEXFORMAT_INVALID
#define GFX_VERTEXFORMAT_FLOAT                   SG_VERTEXFORMAT_FLOAT
#define GFX_VERTEXFORMAT_FLOAT2                  SG_VERTEXFORMAT_FLOAT2
#define GFX_VERTEXFORMAT_FLOAT3                  SG_VERTEXFORMAT_FLOAT3
#define GFX_VERTEXFORMAT_FLOAT4                  SG_VERTEXFORMAT_FLOAT4
#define GFX_VERTEXFORMAT_BYTE4                   SG_VERTEXFORMAT_BYTE4
#define GFX_VERTEXFORMAT_BYTE4N                  SG_VERTEXFORMAT_BYTE4N
#define GFX_VERTEXFORMAT_UBYTE4                  SG_VERTEXFORMAT_UBYTE4
#define GFX_VERTEXFORMAT_UBYTE4N                 SG_VERTEXFORMAT_UBYTE4N
#define GFX_VERTEXFORMAT_SHORT2                  SG_VERTEXFORMAT_SHORT2
#define GFX_VERTEXFORMAT_SHORT2N                 SG_VERTEXFORMAT_SHORT2N
#define GFX_VERTEXFORMAT_SHORT4                  SG_VERTEXFORMAT_SHORT4
#define GFX_VERTEXFORMAT_SHORT4N                 SG_VERTEXFORMAT_SHORT4N
#define GFX_VERTEXFORMAT_UINT10_N2               SG_VERTEXFORMAT_UINT10_N2
#define _GFX_VERTEXFORMAT_NUM                    _SG_VERTEXFORMAT_NUM

#define _GFX_VERTEXSTEP_DEFAULT                  _SG_VERTEXSTEP_DEFAULT
#define GFX_VERTEXSTEP_PER_VERTEX                SG_VERTEXSTEP_PER_VERTEX
#define GFX_VERTEXSTEP_PER_INSTANCE              SG_VERTEXSTEP_PER_INSTANCE
#define _GFX_VERTEXSTEP_NUM                      _SG_VERTEXSTEP_NUM

#define GFX_UNIFORMTYPE_INVALID                  SG_UNIFORMTYPE_INVALID
#define GFX_UNIFORMTYPE_FLOAT                    SG_UNIFORMTYPE_FLOAT
#define GFX_UNIFORMTYPE_FLOAT2                   SG_UNIFORMTYPE_FLOAT2
#define GFX_UNIFORMTYPE_FLOAT3                   SG_UNIFORMTYPE_FLOAT3
#define GFX_UNIFORMTYPE_FLOAT4                   SG_UNIFORMTYPE_FLOAT4
#define GFX_UNIFORMTYPE_MAT4                     SG_UNIFORMTYPE_MAT4
#define _GFX_UNIFORMTYPE_NUM                     _SG_UNIFORMTYPE_NUM

#define _GFX_CULLMODE_DEFAULT                    _SG_CULLMODE_DEFAULT
#define GFX_CULLMODE_NONE                        SG_CULLMODE_NONE
#define GFX_CULLMODE_FRONT                       SG_CULLMODE_FRONT
#define GFX_CULLMODE_BACK                        SG_CULLMODE_BACK
#define _GFX_CULLMODE_NUM                        _SG_CULLMODE_NUM

#define _GFX_FACEWINDING_DEFAULT                 _SG_FACEWINDING_DEFAULT
#define GFX_FACEWINDING_CCW                      SG_FACEWINDING_CCW
#define GFX_FACEWINDING_CW                       SG_FACEWINDING_CW
#define _GFX_FACEWINDING_NUM                     _SG_FACEWINDING_NUM

#define _GFX_COMPAREFUNC_DEFAULT                 _SG_COMPAREFUNC_DEFAULT
#define GFX_COMPAREFUNC_NEVER                    SG_COMPAREFUNC_NEVER
#define GFX_COMPAREFUNC_LESS                     SG_COMPAREFUNC_LESS
#define GFX_COMPAREFUNC_EQUAL                    SG_COMPAREFUNC_EQUAL
#define GFX_COMPAREFUNC_LESS_EQUAL               SG_COMPAREFUNC_LESS_EQUAL
#define GFX_COMPAREFUNC_GREATER                  SG_COMPAREFUNC_GREATER
#define GFX_COMPAREFUNC_NOT_EQUAL                SG_COMPAREFUNC_NOT_EQUAL
#define GFX_COMPAREFUNC_GREATER_EQUAL            SG_COMPAREFUNC_GREATER_EQUAL
#define GFX_COMPAREFUNC_ALWAYS                   SG_COMPAREFUNC_ALWAYS
#define _GFX_COMPAREFUNC_NUM                     _SG_COMPAREFUNC_NUM

#define _GFX_STENCILOP_DEFAULT                   _SG_STENCILOP_DEFAULT
#define GFX_STENCILOP_KEEP                       SG_STENCILOP_KEEP
#define GFX_STENCILOP_ZERO                       SG_STENCILOP_ZERO
#define GFX_STENCILOP_REPLACE                    SG_STENCILOP_REPLACE
#define GFX_STENCILOP_INCR_CLAMP                 SG_STENCILOP_INCR_CLAMP
#define GFX_STENCILOP_DECR_CLAMP                 SG_STENCILOP_DECR_CLAMP
#define GFX_STENCILOP_INVERT                     SG_STENCILOP_INVERT
#define GFX_STENCILOP_INCR_WRAP                  SG_STENCILOP_INCR_WRAP
#define GFX_STENCILOP_DECR_WRAP                  SG_STENCILOP_DECR_WRAP
#define _GFX_STENCILOP_NUM                       _SG_STENCILOP_NUM

#define _GFX_BLENDFACTOR_DEFAULT                 _SG_BLENDFACTOR_DEFAULT
#define GFX_BLENDFACTOR_ZERO                     SG_BLENDFACTOR_ZERO
#define GFX_BLENDFACTOR_ONE                      SG_BLENDFACTOR_ONE
#define GFX_BLENDFACTOR_SRC_COLOR                SG_BLENDFACTOR_SRC_COLOR
#define GFX_BLENDFACTOR_ONE_MINUS_SRC_COLOR      SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR
#define GFX_BLENDFACTOR_SRC_ALPHA                SG_BLENDFACTOR_SRC_ALPHA
#define GFX_BLENDFACTOR_ONE_MINUS_SRC_ALPHA      SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
#define GFX_BLENDFACTOR_DST_COLOR                SG_BLENDFACTOR_DST_COLOR
#define GFX_BLENDFACTOR_ONE_MINUS_DST_COLOR      SG_BLENDFACTOR_ONE_MINUS_DST_COLOR
#define GFX_BLENDFACTOR_DST_ALPHA                SG_BLENDFACTOR_DST_ALPHA
#define GFX_BLENDFACTOR_ONE_MINUS_DST_ALPHA      SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA
#define GFX_BLENDFACTOR_SRC_ALPHA_SATURATED      SG_BLENDFACTOR_SRC_ALPHA_SATURATED
#define GFX_BLENDFACTOR_BLEND_COLOR              SG_BLENDFACTOR_BLEND_COLOR
#define GFX_BLENDFACTOR_ONE_MINUS_BLEND_COLOR    SG_BLENDFACTOR_ONE_MINUS_BLEND_COLOR
#define GFX_BLENDFACTOR_BLEND_ALPHA              SG_BLENDFACTOR_BLEND_ALPHA
#define GFX_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA    SG_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA
#define _GFX_BLENDFACTOR_NUM                     _SG_BLENDFACTOR_NUM

#define _GFX_BLENDOP_DEFAULT                     _SG_BLENDOP_DEFAULT
#define GFX_BLENDOP_ADD                          SG_BLENDOP_ADD
#define GFX_BLENDOP_SUBTRACT                     SG_BLENDOP_SUBTRACT
#define GFX_BLENDOP_REVERSE_SUBTRACT             SG_BLENDOP_REVERSE_SUBTRACT
#define _GFX_BLENDOP_NUM                         _SG_BLENDOP_NUM

#define _GFX_COLORMASK_DEFAULT                   _SG_COLORMASK_DEFAULT
#define GFX_COLORMASK_NONE                       SG_COLORMASK_NONE
#define GFX_COLORMASK_R                          SG_COLORMASK_R
#define GFX_COLORMASK_G                          SG_COLORMASK_G
#define GFX_COLORMASK_B                          SG_COLORMASK_B
#define GFX_COLORMASK_A                          SG_COLORMASK_A
#define GFX_COLORMASK_RGB                        SG_COLORMASK_RGB
#define GFX_COLORMASK_RGBA                       SG_COLORMASK_RGBA

#define _GFX_ACTION_DEFAULT                      _SG_ACTION_DEFAULT
#define GFX_ACTION_CLEAR                         SG_ACTION_CLEAR
#define GFX_ACTION_LOAD                          SG_ACTION_LOAD
#define GFX_ACTION_DONTCARE                      SG_ACTION_DONTCARE
#define _GFX_ACTION_NUM                          _SG_ACTION_NUM

typedef sg_color_attachment_action      gfx_color_attachment_action;
typedef sg_depth_attachment_action      gfx_depth_attachment_action;
typedef sg_stencil_attachment_action    gfx_stencil_attachment_action;
typedef sg_pass_action                  gfx_pass_action;
typedef sg_bindings                     gfx_bindings;
typedef sg_buffer_desc                  gfx_buffer_desc;
typedef sg_subimage_content             gfx_subimage_content;
typedef sg_image_content                gfx_image_content;
typedef sg_image_desc                   gfx_image_desc;
typedef sg_shader_attr_desc             gfx_shader_attr_desc;
typedef sg_shader_uniform_desc          gfx_shader_uniform_desc;
typedef sg_shader_uniform_block_desc    gfx_shader_uniform_block_desc;
typedef sg_shader_image_desc            gfx_shader_image_desc;
typedef sg_shader_stage_desc            gfx_shader_stage_desc;
typedef sg_shader_desc                  gfx_shader_desc;
typedef sg_buffer_layout_desc           gfx_buffer_layout_desc;
typedef sg_vertex_attr_desc             gfx_vertex_attr_desc;
typedef sg_layout_desc                  gfx_layout_desc;
typedef sg_stencil_state                gfx_stencil_state;
typedef sg_depth_stencil_state          gfx_depth_stencil_state;
typedef sg_blend_state                  gfx_blend_state;
typedef sg_rasterizer_state             gfx_rasterizer_state;
typedef sg_pipeline_desc                gfx_pipeline_desc;
typedef sg_attachment_desc              gfx_attachment_desc;
typedef sg_pass_desc                    gfx_pass_desc;
typedef sg_trace_hooks                  gfx_trace_hooks;
typedef sg_slot_info                    gfx_slot_info;
typedef sg_buffer_info                  gfx_buffer_info;
typedef sg_image_info                   gfx_image_info;
typedef sg_shader_info                  gfx_shader_info;
typedef sg_pipeline_info                gfx_pipeline_info;
typedef sg_pass_info                    gfx_pass_info;
typedef sg_desc                         gfx_desc;
typedef sg_draw_state                   gfx_draw_state;

#define gfx_setup                 sg_setup
#define gfx_shutdown              sg_shutdown
#define gfx_isvalid               sg_isvalid
#define gfx_query_desc            sg_query_desc
#define gfx_query_backend         sg_query_backend
#define gfx_query_feature         sg_query_feature
#define gfx_reset_state_cache     sg_reset_state_cache
#define gfx_install_trace_hooks   sg_install_trace_hooks
#define gfx_push_debug_group      sg_push_debug_group
#define gfx_pop_debug_group       sg_pop_debug_group

#define gfx_make_buffer           sg_make_buffer
#define gfx_make_image            sg_make_image
#define gfx_make_shader           sg_make_shader
#define gfx_make_pipeline         sg_make_pipeline
#define gfx_make_pass             sg_make_pass
#define gfx_destroy_buffer        sg_destroy_buffer
#define gfx_destroy_image         sg_destroy_image
#define gfx_destroy_shader        sg_destroy_shader
#define gfx_destroy_pipeline      sg_destroy_pipeline
#define gfx_destroy_pass          sg_destroy_pass
#define gfx_update_buffer         sg_update_buffer
#define gfx_update_image          sg_update_image
#define gfx_append_buffer         sg_append_buffer
#define gfx_query_buffer_overflow sg_query_buffer_overflow

#define gfx_query_buffer_state    sg_query_buffer_state
#define gfx_query_image_state     sg_query_image_state
#define gfx_query_shader_state    sg_query_shader_state
#define gfx_query_pipeline_state  sg_query_pipeline_state
#define gfx_query_pass_state      sg_query_pass_state

#define gfx_begin_default_pass    sg_begin_default_pass
#define gfx_begin_pass            sg_begin_pass
#define gfx_apply_viewport        sg_apply_viewport
#define gfx_apply_scissor_rect    sg_apply_scissor_rect
#define gfx_apply_pipeline        sg_apply_pipeline
#define gfx_apply_bindings        sg_apply_bindings
#define gfx_apply_uniforms        sg_apply_uniforms
#define gfx_draw                  sg_draw
#define gfx_end_pass              sg_end_pass
#define gfx_commit                sg_commit

#define gfx_alloc_buffer          sg_alloc_buffer
#define gfx_alloc_image           sg_alloc_image
#define gfx_alloc_shader          sg_alloc_shader
#define gfx_alloc_pipeline        sg_alloc_pipeline
#define gfx_alloc_pass            sg_alloc_pass
#define gfx_init_buffer           sg_init_buffer
#define gfx_init_image            sg_init_image
#define gfx_init_shader           sg_init_shader
#define gfx_init_pipeline         sg_init_pipeline
#define gfx_init_pass             sg_init_pass
#define gfx_fail_buffer           sg_fail_buffer
#define gfx_fail_image            sg_fail_image
#define gfx_fail_shader           sg_fail_shader
#define gfx_fail_pipeline         sg_fail_pipeline
#define gfx_fail_pass             sg_fail_pass

#define gfx_query_buffer_info     sg_query_buffer_info
#define gfx_query_image_info      sg_query_image_info
#define gfx_query_shader_info     sg_query_shader_info
#define gfx_query_pipeline_info   sg_query_pipeline_info
#define gfx_query_pass_info       sg_query_pass_info

#define gfx_setup_context         sg_setup_context
#define gfx_activate_context      sg_activate_context
#define gfx_discard_context       sg_discard_context

#endif /* ! _GFX_H_ */
