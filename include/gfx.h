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
typedef sg_range                        gfx_range;
typedef sg_color                        gfx_color;
typedef sg_backend                      gfx_backend;
typedef sg_pixel_format                 gfx_pixel_format;
typedef sg_pixelformat_info             gfx_pixelformat_info;
typedef sg_features                     gfx_features;
typedef sg_limits                       gfx_limits;
typedef sg_resource_state               gfx_resource_state;
typedef sg_usage                        gfx_usage;
typedef sg_buffer_type                  gfx_buffer_type;
typedef sg_index_type                   gfx_index_type;
typedef sg_image_type                   gfx_image_type;
typedef sg_sampler_type                 gfx_sampler_type;
typedef sg_cube_face                    gfx_cube_face;
typedef sg_shader_stage                 gfx_shader_stage;
typedef sg_primitive_type               gfx_primitive_type;
typedef sg_filter                       gfx_filter;
typedef sg_wrap                         gfx_wrap;
typedef sg_border_color                 gfx_border_color;
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
typedef sg_color_attachment_action      gfx_color_attachment_action;
typedef sg_depth_attachment_action      gfx_depth_attachment_action;
typedef sg_stencil_attachment_action    gfx_stencil_attachment_action;
typedef sg_pass_action                  gfx_pass_action;
typedef sg_bindings                     gfx_bindings;
typedef sg_buffer_desc                  gfx_buffer_desc;
typedef sg_image_data                   gfx_image_data;
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
typedef sg_stencil_face_state           gfx_stencil_face_state;
typedef sg_stencil_state                gfx_stencil_state;
typedef sg_depth_state                  gfx_depth_state;
typedef sg_blend_state                  gfx_blend_state;
typedef sg_color_state                  gfx_color_state;
typedef sg_pipeline_desc                gfx_pipeline_desc;
typedef sg_pass_attachment_desc         gfx_pass_attachment_desc;
typedef sg_pass_desc                    gfx_pass_desc;
typedef sg_trace_hooks                  gfx_trace_hooks;
typedef sg_slot_info                    gfx_slot_info;
typedef sg_buffer_info                  gfx_buffer_info;
typedef sg_image_info                   gfx_image_info;
typedef sg_shader_info                  gfx_shader_info;
typedef sg_pipeline_info                gfx_pipeline_info;
typedef sg_pass_info                    gfx_pass_info;
typedef sg_context_desc                 gfx_context_desc;
typedef sg_desc                         gfx_desc;

#define gfx_setup                       sg_setup
#define gfx_shutdown                    sg_shutdown
#define gfx_isvalid                     sg_isvalid
#define gfx_reset_state_cache           sg_reset_state_cache
#define gfx_install_trace_hooks         sg_install_trace_hooks
#define gfx_push_debug_group            sg_push_debug_group
#define gfx_pop_debug_group             sg_pop_debug_group

#define gfx_make_buffer                 sg_make_buffer
#define gfx_make_image                  sg_make_image
#define gfx_make_shader                 sg_make_shader
#define gfx_make_pipeline               sg_make_pipeline
#define gfx_make_pass                   sg_make_pass
#define gfx_destroy_buffer              sg_destroy_buffer
#define gfx_destroy_image               sg_destroy_image
#define gfx_destroy_shader              sg_destroy_shader
#define gfx_destroy_pipeline            sg_destroy_pipeline
#define gfx_destroy_pass                sg_destroy_pass
#define gfx_update_buffer               sg_update_buffer
#define gfx_update_image                sg_update_image
#define gfx_append_buffer               sg_append_buffer
#define gfx_query_buffer_overflow       sg_query_buffer_overflow

#define gfx_begin_default_pass          sg_begin_default_pass
#define gfx_begin_default_passf         sg_begin_default_passf
#define gfx_begin_pass                  sg_begin_pass
#define gfx_apply_viewport              sg_apply_viewport
#define gfx_apply_viewportf             sg_apply_viewportf
#define gfx_apply_scissor_rect          sg_apply_scissor_rect
#define gfx_apply_scissor_rectf         sg_apply_scissor_rectf
#define gfx_apply_pipeline              sg_apply_pipeline
#define gfx_apply_bindings              sg_apply_bindings
#define gfx_apply_uniforms              sg_apply_uniforms
#define gfx_draw                        sg_draw
#define gfx_end_pass                    sg_end_pass
#define gfx_commit                      sg_commit

#define gfx_query_desc                  sg_query_desc
#define gfx_query_backend               sg_query_backend
#define gfx_query_features              sg_query_features
#define gfx_query_limits                sg_query_limits
#define gfx_query_pixelformat           sg_query_pixelformat
#define gfx_query_buffer_state          sg_query_buffer_state
#define gfx_query_image_state           sg_query_image_state
#define gfx_query_shader_state          sg_query_shader_state
#define gfx_query_pipeline_state        sg_query_pipeline_state
#define gfx_query_pass_state            sg_query_pass_state
#define gfx_query_buffer_info           sg_query_buffer_info
#define gfx_query_image_info            sg_query_image_info
#define gfx_query_shader_info           sg_query_shader_info
#define gfx_query_pipeline_info         sg_query_pipeline_info
#define gfx_query_pass_info             sg_query_pass_info
#define gfx_query_buffer_defaults       sg_query_buffer_defaults
#define gfx_query_image_defaults        sg_query_image_defaults
#define gfx_query_shader_defaults       sg_query_shader_defaults
#define gfx_query_pipeline_defaults     sg_query_pipeline_defaults
#define gfx_query_pass_defaults         sg_query_pass_defaults

#define gfx_alloc_buffer                sg_alloc_buffer
#define gfx_alloc_image                 sg_alloc_image
#define gfx_alloc_shader                sg_alloc_shader
#define gfx_alloc_pipeline              sg_alloc_pipeline
#define gfx_alloc_pass                  sg_alloc_pass
#define gfx_dealloc_buffer              sg_dealloc_buffer
#define gfx_dealloc_image               sg_dealloc_image
#define gfx_dealloc_shader              sg_dealloc_shader
#define gfx_dealloc_pipeline            sg_dealloc_pipeline
#define gfx_dealloc_pass                sg_dealloc_pass

#define gfx_init_buffer                 sg_init_buffer
#define gfx_init_image                  sg_init_image
#define gfx_init_shader                 sg_init_shader
#define gfx_init_pipeline               sg_init_pipeline
#define gfx_init_pass                   sg_init_pass
#define gfx_uninit_buffer               sg_uninit_buffer
#define gfx_uninit_image                sg_uninit_image
#define gfx_uninit_shader               sg_uninit_shader
#define gfx_uninit_pipeline             sg_uninit_pipeline
#define gfx_uninit_pass                 sg_uninit_pass

#define gfx_fail_buffer                 sg_fail_buffer
#define gfx_fail_image                  sg_fail_image
#define gfx_fail_shader                 sg_fail_shader
#define gfx_fail_pipeline               sg_fail_pipeline
#define gfx_fail_pass                   sg_fail_pass

#define gfx_setup_context               sg_setup_context
#define gfx_activate_context            sg_activate_context
#define gfx_discard_context             sg_discard_context

#define GFX_INVALID_ID                           SG_INVALID_ID
#define GFX_NUM_SHADER_STAGES                    SG_NUM_SHADER_STAGES
#define GFX_NUM_INFLIGHT_FRAMES                  SG_NUM_INFLIGHT_FRAMES
#define GFX_MAX_COLOR_ATTACHMENTS                SG_MAX_COLOR_ATTACHMENTS
#define GFX_MAX_SHADERSTAGE_BUFFERS              SG_MAX_SHADERSTAGE_BUFFERS
#define GFX_MAX_SHADERSTAGE_IMAGES               SG_MAX_SHADERSTAGE_IMAGES
#define GFX_MAX_SHADERSTAGE_UBS                  SG_MAX_SHADERSTAGE_UBS
#define GFX_MAX_UB_MEMBERS                       SG_MAX_UB_MEMBERS
#define GFX_MAX_VERTEX_ATTRIBUTES                SG_MAX_VERTEX_ATTRIBUTES
#define GFX_MAX_MIPMAPS                          SG_MAX_MIPMAPS
#define GFX_MAX_TEXTUREARRAY_LAYERS              SG_MAX_TEXTUREARRAY_LAYERS
#define GFX_BACKEND_GLCORE33                     SG_BACKEND_GLCORE33
#define GFX_BACKEND_GLES2                        SG_BACKEND_GLES2
#define GFX_BACKEND_GLES3                        SG_BACKEND_GLES3
#define GFX_BACKEND_D3D11                        SG_BACKEND_D3D11
#define GFX_BACKEND_METAL_IOS                    SG_BACKEND_METAL_IOS
#define GFX_BACKEND_METAL_MACOS                  SG_BACKEND_METAL_MACOS
#define GFX_BACKEND_METAL_SIMULATOR              SG_BACKEND_METAL_SIMULATOR
#define GFX_BACKEND_WGPU                         SG_BACKEND_WGPU
#define GFX_BACKEND_DUMMY                        SG_BACKEND_DUMMY
#define GFX_PIXELFORMAT_NONE                     SG_PIXELFORMAT_NONE
#define GFX_PIXELFORMAT_R8                       SG_PIXELFORMAT_R8
#define GFX_PIXELFORMAT_R8SN                     SG_PIXELFORMAT_R8SN
#define GFX_PIXELFORMAT_R8UI                     SG_PIXELFORMAT_R8UI
#define GFX_PIXELFORMAT_R8SI                     SG_PIXELFORMAT_R8SI
#define GFX_PIXELFORMAT_R16                      SG_PIXELFORMAT_R16
#define GFX_PIXELFORMAT_R16SN                    SG_PIXELFORMAT_R16SN
#define GFX_PIXELFORMAT_R16UI                    SG_PIXELFORMAT_R16UI
#define GFX_PIXELFORMAT_R16SI                    SG_PIXELFORMAT_R16SI
#define GFX_PIXELFORMAT_R16F                     SG_PIXELFORMAT_R16F
#define GFX_PIXELFORMAT_RG8                      SG_PIXELFORMAT_RG8
#define GFX_PIXELFORMAT_RG8SN                    SG_PIXELFORMAT_RG8SN
#define GFX_PIXELFORMAT_RG8UI                    SG_PIXELFORMAT_RG8UI
#define GFX_PIXELFORMAT_RG8SI                    SG_PIXELFORMAT_RG8SI
#define GFX_PIXELFORMAT_R32UI                    SG_PIXELFORMAT_R32UI
#define GFX_PIXELFORMAT_R32SI                    SG_PIXELFORMAT_R32SI
#define GFX_PIXELFORMAT_R32F                     SG_PIXELFORMAT_R32F
#define GFX_PIXELFORMAT_RG16                     SG_PIXELFORMAT_RG16
#define GFX_PIXELFORMAT_RG16SN                   SG_PIXELFORMAT_RG16SN
#define GFX_PIXELFORMAT_RG16UI                   SG_PIXELFORMAT_RG16UI
#define GFX_PIXELFORMAT_RG16SI                   SG_PIXELFORMAT_RG16SI
#define GFX_PIXELFORMAT_RG16F                    SG_PIXELFORMAT_RG16F
#define GFX_PIXELFORMAT_RGBA8                    SG_PIXELFORMAT_RGBA8
#define GFX_PIXELFORMAT_RGBA8SN                  SG_PIXELFORMAT_RGBA8SN
#define GFX_PIXELFORMAT_RGBA8UI                  SG_PIXELFORMAT_RGBA8UI
#define GFX_PIXELFORMAT_RGBA8SI                  SG_PIXELFORMAT_RGBA8SI
#define GFX_PIXELFORMAT_BGRA8                    SG_PIXELFORMAT_BGRA8
#define GFX_PIXELFORMAT_RGB10A2                  SG_PIXELFORMAT_RGB10A2
#define GFX_PIXELFORMAT_RG11B10F                 SG_PIXELFORMAT_RG11B10F
#define GFX_PIXELFORMAT_RG32UI                   SG_PIXELFORMAT_RG32UI
#define GFX_PIXELFORMAT_RG32SI                   SG_PIXELFORMAT_RG32SI
#define GFX_PIXELFORMAT_RG32F                    SG_PIXELFORMAT_RG32F
#define GFX_PIXELFORMAT_RGBA16                   SG_PIXELFORMAT_RGBA16
#define GFX_PIXELFORMAT_RGBA16SN                 SG_PIXELFORMAT_RGBA16SN
#define GFX_PIXELFORMAT_RGBA16UI                 SG_PIXELFORMAT_RGBA16UI
#define GFX_PIXELFORMAT_RGBA16SI                 SG_PIXELFORMAT_RGBA16SI
#define GFX_PIXELFORMAT_RGBA16F                  SG_PIXELFORMAT_RGBA16F
#define GFX_PIXELFORMAT_RGBA32UI                 SG_PIXELFORMAT_RGBA32UI
#define GFX_PIXELFORMAT_RGBA32SI                 SG_PIXELFORMAT_RGBA32SI
#define GFX_PIXELFORMAT_RGBA32F                  SG_PIXELFORMAT_RGBA32F
#define GFX_PIXELFORMAT_DEPTH                    SG_PIXELFORMAT_DEPTH
#define GFX_PIXELFORMAT_DEPTH_STENCIL            SG_PIXELFORMAT_DEPTH_STENCIL
#define GFX_PIXELFORMAT_BC1_RGBA                 SG_PIXELFORMAT_BC1_RGBA
#define GFX_PIXELFORMAT_BC2_RGBA                 SG_PIXELFORMAT_BC2_RGBA
#define GFX_PIXELFORMAT_BC3_RGBA                 SG_PIXELFORMAT_BC3_RGBA
#define GFX_PIXELFORMAT_BC4_R                    SG_PIXELFORMAT_BC4_R
#define GFX_PIXELFORMAT_BC4_RSN                  SG_PIXELFORMAT_BC4_RSN
#define GFX_PIXELFORMAT_BC5_RG                   SG_PIXELFORMAT_BC5_RG
#define GFX_PIXELFORMAT_BC5_RGSN                 SG_PIXELFORMAT_BC5_RGSN
#define GFX_PIXELFORMAT_BC6H_RGBF                SG_PIXELFORMAT_BC6H_RGBF
#define GFX_PIXELFORMAT_BC6H_RGBUF               SG_PIXELFORMAT_BC6H_RGBUF
#define GFX_PIXELFORMAT_BC7_RGBA                 SG_PIXELFORMAT_BC7_RGBA
#define GFX_PIXELFORMAT_PVRTC_RGB_2BPP           SG_PIXELFORMAT_PVRTC_RGB_2BPP
#define GFX_PIXELFORMAT_PVRTC_RGB_4BPP           SG_PIXELFORMAT_PVRTC_RGB_4BPP
#define GFX_PIXELFORMAT_PVRTC_RGBA_2BPP          SG_PIXELFORMAT_PVRTC_RGBA_2BPP
#define GFX_PIXELFORMAT_PVRTC_RGBA_4BPP          SG_PIXELFORMAT_PVRTC_RGBA_4BPP
#define GFX_PIXELFORMAT_ETC2_RGB8                SG_PIXELFORMAT_ETC2_RGB8
#define GFX_PIXELFORMAT_ETC2_RGB8A1              SG_PIXELFORMAT_ETC2_RGB8A1
#define GFX_PIXELFORMAT_ETC2_RGBA8               SG_PIXELFORMAT_ETC2_RGBA8
#define GFX_PIXELFORMAT_ETC2_RG11                SG_PIXELFORMAT_ETC2_RG11
#define GFX_PIXELFORMAT_ETC2_RG11SN              SG_PIXELFORMAT_ETC2_RG11SN
#define GFX_RESOURCESTATE_INITIAL                SG_RESOURCESTATE_INITIAL
#define GFX_RESOURCESTATE_ALLOC                  SG_RESOURCESTATE_ALLOC
#define GFX_RESOURCESTATE_VALID                  SG_RESOURCESTATE_VALID
#define GFX_RESOURCESTATE_FAILED                 SG_RESOURCESTATE_FAILED
#define GFX_RESOURCESTATE_INVALID                SG_RESOURCESTATE_INVALID
#define GFX_USAGE_IMMUTABLE                      SG_USAGE_IMMUTABLE
#define GFX_USAGE_DYNAMIC                        SG_USAGE_DYNAMIC
#define GFX_USAGE_STREAM                         SG_USAGE_STREAM
#define GFX_BUFFERTYPE_VERTEXBUFFER              SG_BUFFERTYPE_VERTEXBUFFER
#define GFX_BUFFERTYPE_INDEXBUFFER               SG_BUFFERTYPE_INDEXBUFFER
#define GFX_INDEXTYPE_NONE                       SG_INDEXTYPE_NONE
#define GFX_INDEXTYPE_UINT16                     SG_INDEXTYPE_UINT16
#define GFX_INDEXTYPE_UINT32                     SG_INDEXTYPE_UINT32
#define GFX_IMAGETYPE_2D                         SG_IMAGETYPE_2D
#define GFX_IMAGETYPE_CUBE                       SG_IMAGETYPE_CUBE
#define GFX_IMAGETYPE_3D                         SG_IMAGETYPE_3D
#define GFX_IMAGETYPE_ARRAY                      SG_IMAGETYPE_ARRAY
#define GFX_SAMPLERTYPE_FLOAT                    SG_SAMPLERTYPE_FLOAT
#define GFX_SAMPLERTYPE_SINT                     SG_SAMPLERTYPE_SINT
#define GFX_SAMPLERTYPE_UINT                     SG_SAMPLERTYPE_UINT
#define GFX_CUBEFACE_POS_X                       SG_CUBEFACE_POS_X
#define GFX_CUBEFACE_NEG_X                       SG_CUBEFACE_NEG_X
#define GFX_CUBEFACE_POS_Y                       SG_CUBEFACE_POS_Y
#define GFX_CUBEFACE_NEG_Y                       SG_CUBEFACE_NEG_Y
#define GFX_CUBEFACE_POS_Z                       SG_CUBEFACE_POS_Z
#define GFX_CUBEFACE_NEG_Z                       SG_CUBEFACE_NEG_Z
#define GFX_CUBEFACE_NUM                         SG_CUBEFACE_NUM
#define GFX_SHADERSTAGE_VS                       SG_SHADERSTAGE_VS
#define GFX_SHADERSTAGE_FS                       SG_SHADERSTAGE_FS
#define GFX_PRIMITIVETYPE_POINTS                 SG_PRIMITIVETYPE_POINTS
#define GFX_PRIMITIVETYPE_LINES                  SG_PRIMITIVETYPE_LINES
#define GFX_PRIMITIVETYPE_LINE_STRIP             SG_PRIMITIVETYPE_LINE_STRIP
#define GFX_PRIMITIVETYPE_TRIANGLES              SG_PRIMITIVETYPE_TRIANGLES
#define GFX_PRIMITIVETYPE_TRIANGLE_STRIP         SG_PRIMITIVETYPE_TRIANGLE_STRIP
#define GFX_FILTER_NEAREST                       SG_FILTER_NEAREST
#define GFX_FILTER_LINEAR                        SG_FILTER_LINEAR
#define GFX_FILTER_NEAREST_MIPMAP_NEAREST        SG_FILTER_NEAREST_MIPMAP_NEAREST
#define GFX_FILTER_NEAREST_MIPMAP_LINEAR         SG_FILTER_NEAREST_MIPMAP_LINEAR
#define GFX_FILTER_LINEAR_MIPMAP_NEAREST         SG_FILTER_LINEAR_MIPMAP_NEAREST
#define GFX_FILTER_LINEAR_MIPMAP_LINEAR          SG_FILTER_LINEAR_MIPMAP_LINEAR
#define GFX_WRAP_REPEAT                          SG_WRAP_REPEAT
#define GFX_WRAP_CLAMP_TO_EDGE                   SG_WRAP_CLAMP_TO_EDGE
#define GFX_WRAP_CLAMP_TO_BORDER                 SG_WRAP_CLAMP_TO_BORDER
#define GFX_WRAP_MIRRORED_REPEAT                 SG_WRAP_MIRRORED_REPEAT
#define GFX_BORDERCOLOR_TRANSPARENT_BLACK        SG_BORDERCOLOR_TRANSPARENT_BLACK
#define GFX_BORDERCOLOR_OPAQUE_BLACK             SG_BORDERCOLOR_OPAQUE_BLACK
#define GFX_BORDERCOLOR_OPAQUE_WHITE             SG_BORDERCOLOR_OPAQUE_WHITE
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
#define GFX_VERTEXFORMAT_USHORT2N                SG_VERTEXFORMAT_USHORT2N
#define GFX_VERTEXFORMAT_SHORT4                  SG_VERTEXFORMAT_SHORT4
#define GFX_VERTEXFORMAT_SHORT4N                 SG_VERTEXFORMAT_SHORT4N
#define GFX_VERTEXFORMAT_USHORT4N                SG_VERTEXFORMAT_USHORT4N
#define GFX_VERTEXFORMAT_UINT10_N2               SG_VERTEXFORMAT_UINT10_N2
#define GFX_VERTEXSTEP_PER_VERTEX                SG_VERTEXSTEP_PER_VERTEX
#define GFX_VERTEXSTEP_PER_INSTANCE              SG_VERTEXSTEP_PER_INSTANCE
#define GFX_UNIFORMTYPE_INVALID                  SG_UNIFORMTYPE_INVALID
#define GFX_UNIFORMTYPE_FLOAT                    SG_UNIFORMTYPE_FLOAT
#define GFX_UNIFORMTYPE_FLOAT2                   SG_UNIFORMTYPE_FLOAT2
#define GFX_UNIFORMTYPE_FLOAT3                   SG_UNIFORMTYPE_FLOAT3
#define GFX_UNIFORMTYPE_FLOAT4                   SG_UNIFORMTYPE_FLOAT4
#define GFX_UNIFORMTYPE_MAT4                     SG_UNIFORMTYPE_MAT4
#define GFX_CULLMODE_NONE                        SG_CULLMODE_NONE
#define GFX_CULLMODE_FRONT                       SG_CULLMODE_FRONT
#define GFX_CULLMODE_BACK                        SG_CULLMODE_BACK
#define GFX_FACEWINDING_CCW                      SG_FACEWINDING_CCW
#define GFX_FACEWINDING_CW                       SG_FACEWINDING_CW
#define GFX_COMPAREFUNC_NEVER                    SG_COMPAREFUNC_NEVER
#define GFX_COMPAREFUNC_LESS                     SG_COMPAREFUNC_LESS
#define GFX_COMPAREFUNC_EQUAL                    SG_COMPAREFUNC_EQUAL
#define GFX_COMPAREFUNC_LESS_EQUAL               SG_COMPAREFUNC_LESS_EQUAL
#define GFX_COMPAREFUNC_GREATER                  SG_COMPAREFUNC_GREATER
#define GFX_COMPAREFUNC_NOT_EQUAL                SG_COMPAREFUNC_NOT_EQUAL
#define GFX_COMPAREFUNC_GREATER_EQUAL            SG_COMPAREFUNC_GREATER_EQUAL
#define GFX_COMPAREFUNC_ALWAYS                   SG_COMPAREFUNC_ALWAYS
#define GFX_STENCILOP_KEEP                       SG_STENCILOP_KEEP
#define GFX_STENCILOP_ZERO                       SG_STENCILOP_ZERO
#define GFX_STENCILOP_REPLACE                    SG_STENCILOP_REPLACE
#define GFX_STENCILOP_INCR_CLAMP                 SG_STENCILOP_INCR_CLAMP
#define GFX_STENCILOP_DECR_CLAMP                 SG_STENCILOP_DECR_CLAMP
#define GFX_STENCILOP_INVERT                     SG_STENCILOP_INVERT
#define GFX_STENCILOP_INCR_WRAP                  SG_STENCILOP_INCR_WRAP
#define GFX_STENCILOP_DECR_WRAP                  SG_STENCILOP_DECR_WRAP
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
#define GFX_BLENDOP_ADD                          SG_BLENDOP_ADD
#define GFX_BLENDOP_SUBTRACT                     SG_BLENDOP_SUBTRACT
#define GFX_BLENDOP_REVERSE_SUBTRACT             SG_BLENDOP_REVERSE_SUBTRACT
#define GFX_COLORMASK_NONE                       SG_COLORMASK_NONE
#define GFX_COLORMASK_R                          SG_COLORMASK_R
#define GFX_COLORMASK_G                          SG_COLORMASK_G
#define GFX_COLORMASK_RG                         SG_COLORMASK_RG
#define GFX_COLORMASK_B                          SG_COLORMASK_B
#define GFX_COLORMASK_RB                         SG_COLORMASK_RB
#define GFX_COLORMASK_GB                         SG_COLORMASK_GB
#define GFX_COLORMASK_RGB                        SG_COLORMASK_RGB
#define GFX_COLORMASK_A                          SG_COLORMASK_A
#define GFX_COLORMASK_RA                         SG_COLORMASK_RA
#define GFX_COLORMASK_GA                         SG_COLORMASK_GA
#define GFX_COLORMASK_RGA                        SG_COLORMASK_RGA
#define GFX_COLORMASK_BA                         SG_COLORMASK_BA
#define GFX_COLORMASK_RBA                        SG_COLORMASK_RBA
#define GFX_COLORMASK_GBA                        SG_COLORMASK_GBA
#define GFX_COLORMASK_RGBA                       SG_COLORMASK_RGBA
#define GFX_ACTION_CLEAR                         SG_ACTION_CLEAR
#define GFX_ACTION_LOAD                          SG_ACTION_LOAD
#define GFX_ACTION_DONTCARE                      SG_ACTION_DONTCARE

#endif /* ! _GFX_H_ */
