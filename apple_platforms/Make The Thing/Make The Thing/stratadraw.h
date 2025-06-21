//
//  stratadraw.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/16/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef stratadraw_h
#define stratadraw_h

#include "shared_shader_types.hpp"

#include "LineRenderer.h"

//#include "memory.hpp"
#define SD_END_LAYER_DESTROY_OPT (false)

namespace sd {

typedef uint64 Render_Layer_ID;
constexpr const Render_Layer_ID Render_Layer_ID_INVALID = ULLONG_MAX;

typedef uint64 Shader_ID;
constexpr const Shader_ID Shader_ID_INVALID = ULLONG_MAX;

struct Renderer;

struct Render_Layer;

typedef uint32 Index_Type;

typedef enum RENDER_LAYER_TYPE {
    RENDER_LAYER_TYPE_STATIC,
    RENDER_LAYER_TYPE_DYNAMIC,
    RENDER_LAYER_TYPE_COUNT
} RENDER_LAYER_TYPE;

typedef enum SHAPE_MODE {
    SHAPE_MODE_NONE,
    SHAPE_MODE_POLYGON,
    SHAPE_MODE_PATH,
    SHAPE_MODE_COUNT
} SHAPE_MODE;

struct Images;


typedef int32 Texture_ID;
typedef int16 Texture_Sampler_ID;



// defines accessors for fields in Drawable_Info

#define PROPERTY( type__ , access_name__, var_name__ ) \
inline void set_##access_name__ (const type__ & var_name__ ) { \
this->instance_data . var_name__ = var_name__ ; \
} \
inline void set_ptr_##access_name__ (const type__ * var_name__ ) { \
this->instance_data . var_name__ = *var_name__ ; \
} \
inline void set_##access_name__ ( Drawable_Info* info) { \
this->instance_data . var_name__ = info->instance_data . var_name__ ; \
} \
inline type__ get_##access_name__ ( void ) { \
return this-> instance_data . var_name__ ; \
} \
inline type__ * get_ptr_ ##access_name__ ( void ) { \
return &this-> instance_data . var_name__ ; \
} \

//struct Buffer_Allocation {
//    usize offset = 0;
//    usize count = 0;
//    usize used = 0;
//};
struct Drawable_Handle {
    uint32 layer_index;
    uint32 drawable_index;
};

constexpr const sd::Texture_ID Texture_ID_INVALID = -1;
constexpr const sd::Texture_Sampler_ID Texture_Sampler_ID_INVALID = -1;
constexpr const sd::Texture_Sampler_ID Texture_Sampler_ID_CLAMP_TO_EDGE = 0;
constexpr const sd::Texture_Sampler_ID Texture_Sampler_ID_REPEAT = 1;
constexpr const sd::Texture_Sampler_ID Texture_Sampler_ID_MIRROR_REPEAT = 2;
constexpr const sd::Texture_Sampler_ID Texture_Sampler_ID_MIRROR_CLAMP_TO_EDGE = 3;
constexpr const sd::Texture_Sampler_ID Texture_Sampler_ID_CLAMP_TO_EDGE_LINEAR = 4;
constexpr const usize Texture_Sampler_ID_COUNT = 5;
constexpr const sd::Texture_Sampler_ID Texture_Sampler_ID_DEFAULT = Texture_Sampler_ID_CLAMP_TO_EDGE;

constexpr sd::Texture_ID TEXTURE_ID_BUILTIN_WHITE = 0;
constexpr sd::Texture_ID TEXTURE_ID_BUILTIN_CLEAR = 1;
constexpr sd::Texture_ID TEXTURE_ID_BUILTIN_SPECTRUM = 2;
constexpr sd::Texture_ID TEXTURE_ID_BUILTIN_BLACK = 3;
constexpr const usize BUILTIN_TEXTURE_COUNT = 4;
constexpr const usize BUILTIN_TEXTURE_BYTES_PER_COLOR = 4;

constexpr const usize DEFAULT_TEXTURE_ID = TEXTURE_ID_BUILTIN_WHITE;
constexpr const uint32 DEFAULT_TEXTURE_SAMPLER_ID = Texture_Sampler_ID_CLAMP_TO_EDGE;

struct Texture_Handle {
    void* ref = nullptr;
    Texture_ID id = Texture_ID_INVALID;
};

#ifndef SD_HIDE_DRAWABLE_INFO_DEF
struct Drawable_Info {
    SHAPE_MODE type;
    
    Instance_Data instance_data = {
        .model_matrix = SD_mat4_identity(),
        .color_factor = SD_vec4(1.0f),
        .color_addition = SD_vec4(0.0f),
        .texture_index = DEFAULT_TEXTURE_ID,
        .texture_sampler_index = Texture_Sampler_ID_DEFAULT,
        .texture_mult_index = DEFAULT_TEXTURE_ID,
        .texture_add_index = TEXTURE_ID_BUILTIN_CLEAR,
        .texture_coordinates_modifiers = SD_vec4(1.0f, 1.0f, 0.0, 0.0),
        .texture_animation_speed = SD_vec2(0.0f),
    };
    
    PROPERTY(SD_mat4, transform, model_matrix)
    PROPERTY(SD_vec4, color_factor, color_factor)
    PROPERTY(SD_vec4, color_addition, color_addition)
    PROPERTY(Texture_ID, texture_ID, texture_index)
    PROPERTY(Texture_Sampler_ID, texture_sampler_ID, texture_sampler_index)
    PROPERTY(Texture_ID, texture_mult_ID, texture_mult_index)
    PROPERTY(Texture_ID, texture_add_ID, texture_add_index)
    PROPERTY(SD_vec4, texture_coordinates_modifiers, texture_coordinates_modifiers)
    PROPERTY(SD_vec2, texture_animation_speed, texture_animation_speed)
    
    
    void init_render_data(void)
    {
        set_transform(SD_mat4_identity());
        set_color_factor(SD_vec4(1.0f));
        set_color_addition(SD_vec4(0.0f));
        set_texture_ID(DEFAULT_TEXTURE_ID);
        set_texture_sampler_ID((int16_t)0);
        set_texture_mult_ID(DEFAULT_TEXTURE_ID);
        set_texture_add_ID(TEXTURE_ID_BUILTIN_CLEAR);
        set_texture_coordinates_modifiers(SD_vec4(1.0f, 1.0f, 0.0f, 0.0f));
        set_texture_animation_speed(SD_vec2(0.0f));
    }
    
    usize v_buffer_offset = 0;
    usize v_buffer_count  = 0;
    usize i_buffer_offset = 0;
    usize i_buffer_count  = 0;
    
    usize i_buffer_count_used = 0;
    
    usize v_buffer_max_count = 0;
    usize i_buffer_max_count = 0;
    
    usize base_vertex = 0;
    
    //    Buffer_Allocation v_buffer = {};
    //    Buffer_Allocation i_buffer = {};
    
    inline usize index_count(void)
    {
        return i_buffer_count_used;
    }
    
    usize first_index = 0;
    usize next_index = 0;
    
    usize buffer_index = 0;
    usize layer_id = 0;
    
    uint64 flags = 0;
    
    usize reference_count = 0;
    
    uint32 is_enabled = 0;
};
#endif

void set_transform(Drawable_Info* d_info, const SD_mat4& v);
SD_mat4 get_transform(Drawable_Info* d_info);
SD_mat4* get_ptr_transform(Drawable_Info* d_info);
void set_ptr_transform(Drawable_Info* d_info, const SD_mat4* v);

#ifdef __clang__

void set_transform_intrin(Drawable_Info* d_info, const simd_float4x4& v);
simd_float4x4 get_transform_intrin(Drawable_Info* d_info);
void set_ptr_transform_intrin(Drawable_Info* d_info, const simd_float4x4* v);

#endif



// TODO: replace with proper way to proxy same memory
[[deprecated]]
Drawable_Info* Drawable_Info_make_proxy(sd::Renderer* renderer, Drawable_Info* d_info, Render_Layer_ID layer_dst);




//typedef struct
//DATA_ALIGN
//{
//    matrix_float4x4 model_matrix;
//    vector_float4 color_factor;
//    vector_float4 color_addition;
//    int   texture_index;
//    short texture_sampler_index;
//    vector_float2 texture_animation_speed;
//} Instance_Data;

typedef Drawable_Info* Drawable_Reference;

static inline bool Drawable_Info_is_tracked(Renderer* r, Drawable_Info* info)
{
    return (info->reference_count != 0);
}
static inline void Drawable_Info_ref_inc(Renderer* r, Drawable_Info* info)
{
    info->reference_count += 1;
}

void Drawable_Info_release(Renderer* r, Drawable_Info* info);



#undef PROPERTY
// TODO: ... implement handles
/*
 
 Drawable* drawable drawable(Renderer* renderer, Drawable_Handle handle);
 usize drawble_reference_count_decrement()
 usize drawble_reference_count_increment()
 */

enum LOAD_ACTION {
    LOAD_ACTION_DONT_CARE = 0,
    LOAD_ACTION_LOAD = 1,
    LOAD_ACTION_CLEAR = 2,
};
enum STORE_ACTION {
    STORE_ACTION_DONT_CARE = 0,
    STORE_ACTION_STORE = 1,
    STORE_ACTION_MULTISAMPLE_RESOLVE = 2,
    STORE_ACTION_STORE_AND_MULTISAMPLE_RESOLVE = 3,
    STORE_ACTION_UNKNOWN = 4,
    STORE_ACTION_CUSTOM_SAMPLE_DEPTH_STORE = 5,
};
/*
 typedef NS_ENUM(NSUInteger, MTLStoreAction) {
     MTLStoreActionDontCare = 0,
     MTLStoreActionStore = 1,
     MTLStoreActionMultisampleResolve = 2,
     MTLStoreActionStoreAndMultisampleResolve API_AVAILABLE(macos(10.12), ios(10.0)) = 3,
     MTLStoreActionUnknown API_AVAILABLE(macos(10.12), ios(10.0)) = 4,
     MTLStoreActionCustomSampleDepthStore API_AVAILABLE(macos(10.13), ios(11.0)) = 5,
 } API_AVAILABLE(macos(10.11), ios(8.0));

 */

struct Render_Target {
    Texture_Handle target = {};
    Texture_Handle target_depth_stencil = {};
    Texture_Handle target_msaa = {};
    Texture_Handle target_depth_stencil_msaa = {};
    uvec2 size = uvec2(0);
    uint64 flags = 0;
    
    LOAD_ACTION target_load = LOAD_ACTION_CLEAR;
    STORE_ACTION target_store = STORE_ACTION_STORE;
    LOAD_ACTION depth_load = LOAD_ACTION_CLEAR;
    STORE_ACTION depth_store = STORE_ACTION_STORE;
    LOAD_ACTION stencil_load = LOAD_ACTION_CLEAR;
    STORE_ACTION stencil_store = STORE_ACTION_STORE;
    
    vec4 clear_color = vec4(0,0,0,0);
};
static inline bool Render_Target_is_msaa(Render_Target* rt)
{
    return rt->target_msaa.ref != nullptr;
}
static inline bool Render_Target_has_depth_stencil(Render_Target* rt)
{
    return rt->target_depth_stencil.ref != nullptr;
}
static inline bool Render_Target_is_valid(Render_Target* rt)
{
    return rt->target.ref != nullptr;
}


struct Drawable_Info_List_Entry {
    sd::Drawable_Info* drawable;
    Drawable_Info_List_Entry* next;
};
struct Drawable_Info_List {
    Drawable_Info_List_Entry head;
};

struct Rectangle_F32 {
    float32 x = 0;
    float32 y = 0;
    float32 width = 0;
    float32 height = 0;
};

struct Rectangle_F64 {
    float64 x = 0;
    float64 y = 0;
    float64 width = 0;
    float64 height = 0;
};
typedef Rectangle_F64 Rectangle;


struct Rectangle_V32 {
    SD_vec2 position = SD_vec2(0.0f);
    SD_vec2 dimentions = SD_vec2(0.0f);
};
struct Rectangle_V64 {
    SD_vec2_64 position = SD_vec2_64(0.0);
    SD_vec2_64 dimentions = SD_vec2_64(0.0);
};

typedef Rectangle_V64 Rectangle_V;

struct Integer_Rectangle {
    uint64 x = 0;
    uint64 y = 0;
    uint64 width = 0;
    uint64 height = 0;
};

struct Viewport {
    float64 x = 0;
    float64 y = 0;
    float64 width = 0;
    float64 height = 0;
    float64 znear = 0;
    float64 zfar = 0;
};

typedef struct Integer_Rectangle Scissor_Rect;

static inline void fit_rect_centered(Rectangle* container, Rectangle* to_fit, Rectangle* out, float32 uv_center)
{
    auto OutputWidth = container->width;
    auto OutputHeight = container->height;
    auto InputWidth = to_fit->width;
    auto InputHeight = to_fit->height;
    
    // credit to Martins Mozeiko
    // https://github.com/mmozeiko/wcap/blob/main/wcap_encoder.c#L190-L214
    //
    if (OutputWidth != InputWidth || OutputHeight != InputHeight) {
        if (OutputWidth * InputHeight < OutputHeight * InputWidth)
        {
            if (OutputWidth < InputWidth)
            {
                OutputHeight = InputHeight * OutputWidth / InputWidth;
            }
            else
            {
                OutputWidth = InputWidth;
                OutputHeight = InputHeight;
            }
        }
        else
        {
            if (OutputHeight < InputHeight)
            {
                OutputWidth = InputWidth * OutputHeight / InputHeight;
            }
            else
            {
                OutputWidth = InputWidth;
                OutputHeight = InputHeight;
            }
        }
    }
    //
    SD_vec2 size = SD_vec2(OutputWidth, OutputHeight);
    SD_vec2 tl = (vec2(container->width, container->height) - size) * uv_center;
    out->x = tl.x;
    out->y = tl.y;
    out->width  = size.x;
    out->height = size.y;
}

typedef enum COMMAND_TYPE {
    COMMAND_TYPE_DRAW,
    COMMAND_TYPE_DRAW_INSTANCED,
    COMMAND_TYPE_PROJECTION_VIEW_TRANSFORM,
    COMMAND_TYPE_PIPELINE,
    COMMAND_TYPE_COLOR_PIPELINE,
    COMMAND_TYPE_COLOR_HSV_PIPELINE,
    COMMAND_TYPE_TEXTURE_PIPELINE,
    COMMAND_TYPE_SINGLE_COLOR_PIPELINE_SINGLE_SAMPLE,
    COMMAND_TYPE_TEXT_PIPELINE,
    COMMAND_TYPE_SCISSOR_RECTANGLE,
    COMMAND_TYPE_VIEWPORT,
    COMMAND_TYPE_SET_DEPTH_MODE,
    COMMAND_TYPE_CUSTOM,
    COMMAND_TYPE_SET_RENDER_PASS,
    COMMAND_TYPE_RENDER_TO_TEXTURE_BEGIN,
    COMMAND_TYPE_RENDER_TO_TEXTURE_END,
    COMMAND_TYPE_RENDER_PASS_BEGIN,
    COMMAND_TYPE_RENDER_PASS_END,
    COMMAND_TYPE_COMPUTE_PASS_BEGIN,
    COMMAND_TYPE_COMPUTE_PASS_END,
    COMMAND_TYPE_COMPUTE_DISPATCH_UNIFORM_GRID,
    COMMAND_TYPE_COMPUTE_DISPATCH_NON_UNIFORM_GRID,
    COMMAND_TYPE_COMPUTE_BUFFER_SET,
    COMMAND_TYPE_COMPUTE_SHADER_SET,
    COMMAND_TYPE_COMPUTE_SET_BLUR,
    COMMAND_TYPE_COMPUTE_SET_SOBEL,
    COMMAND_TYPE_SET_STENCIL_REFERENCE_VALUE,
    COMMAND_TYPE_VERTEX_BUFFER_SET,
    COMMAND_TYPE_FRAGMENT_BUFFER_SET,
    
    COMMAND_TYPE_COUNT,
} COMMAND_TYPE;

typedef enum PIPELINE_MODE {
    PIPELINE_MODE_COLOR,
    PIPELINE_MODE_COLOR_HSV,
    PIPELINE_MODE_TEXTURE,
    PIPELINE_MODE_COLOR_ADDITIVE,
    PIPELINE_MODE_COLOR_HSV_ADDITIVE,
    PIPELINE_MODE_COLOR_NO_DEPTH_STENCIL,
    PIPELINE_MODE_TEXT,
    PIPELINE_MODE_TEXTURE_ADDITIVE,
    
    PIPELINE_MODE_COUNT,
    
    PIPELINE_MODE_NONE,
} PIPELINE_MODE;

typedef enum DEPTH_WRITE_MODE {
    DEPTH_WRITE_MODE_DISABLED = 1 << 0,
    DEPTH_WRITE_MODE_ENABLED  = 1 << 1
} DEPTH_WRITE_MODE;

typedef enum STENCIL_MODE {
    STENCIL_MODE_DISABLED = 1 << 0,
    STENCIL_MODE_ENABLED_R = 1 << 1,
    STENCIL_MODE_ENABLED_W = 1 << 1,
} STENCIL_MODE;

struct Projection_View_Transform {
    Mat4 projection;
    Mat4 view;
};

struct Drawable_Instance_Data {
    Mat4 transform;
    Texture_ID texture_ID;
    Drawable_Instance_Data* next;
};

struct Render_Layer_Range {
    usize first_index;
    usize count;
};

struct Draw_Command {
    sd::Render_Layer_ID layer_id;
    Render_Layer_Range  layer_range;
    sd::Render_Layer_ID buffer_layer_id;
    mtt::Array_Slice<Drawable_Info> src;
};

struct Draw_Instance_Command {
    sd::Render_Layer_ID layer_id;
    Render_Layer_Range  layer_range;
    sd::Render_Layer_ID buffer_layer_id;
    
    // TEMP, should really enqueue data
    // into a buffer corresponding to one canonical drawable
    Drawable_Info* instance_list;
    Drawable_Info* instance_drawable;
    MTT_List* list;
    mtt::Array_Slice<Drawable_Info> array;
};
static inline MTT_List* Draw_Instanced_Command_get_list(Draw_Instance_Command* command)
{
    return command->list;
}
static inline mtt::Array_Slice<Drawable_Info>* Draw_Instanced_Command_get_array(Draw_Instance_Command* command)
{
    return &command->array;
}

struct Render_Pass_Command {
    uint64 id;
    Render_Target color_targets[1];
    Viewport viewport;
};
struct Render_Pass_Descriptor {
    uint64 id;
    Render_Target color_targets[1];
    Viewport viewport;
};



//struct Render_Target {
//    Texture_Handle target;
//    Texture_Handle target_depth_stencil;
//    Texture_Handle target_msaa;
//    Texture_Handle target_depth_stencil_msaa;
//};

static inline Render_Target Render_Target_init_msaa(Texture_Handle* texture_handle_resolved, Texture_Handle* texture_handle_initial, Texture_Handle* texture_handle_depth_stencil_resolved, Texture_Handle* texture_handle_depth_stencil_initial)
{
    Render_Target rt = {};
    rt.target = *texture_handle_resolved;
    rt.target_msaa = *texture_handle_initial;
    rt.target_depth_stencil = *texture_handle_depth_stencil_resolved;
    rt.target_depth_stencil_msaa = *texture_handle_depth_stencil_initial;
    rt.target_store = STORE_ACTION_MULTISAMPLE_RESOLVE;
    rt.depth_store = STORE_ACTION_MULTISAMPLE_RESOLVE;
    rt.stencil_store = STORE_ACTION_MULTISAMPLE_RESOLVE;
    
    return rt;
}

static inline Render_Target Render_Target_init_msaa_no_depth_stencil(Texture_Handle* texture_handle_resolved, Texture_Handle* texture_handle_initial)
{
    Render_Target rt = {};
    rt.target = *texture_handle_resolved;
    rt.target_msaa = *texture_handle_initial;
    rt.target_store = STORE_ACTION_MULTISAMPLE_RESOLVE;
    rt.depth_store = STORE_ACTION_MULTISAMPLE_RESOLVE;
    rt.stencil_store = STORE_ACTION_MULTISAMPLE_RESOLVE;
    return rt;
}

static inline Render_Target Render_Target_init_depth_stencil(Texture_Handle* texture_handle, Texture_Handle* texture_handle_depth_stencil)
{
    Render_Target rt = {};
    rt.target = *texture_handle;
    rt.target_depth_stencil = *texture_handle_depth_stencil;
    return rt;
}

static inline Render_Target Render_Target_init_no_depth_stencil(Texture_Handle* texture_handle)
{
    Render_Target rt = {};
    rt.target = *texture_handle;
    return rt;
}

bool Render_Target_make_msaa(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out);
bool Render_Target_make_no_depth_stencil_msaa(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out);

bool Render_Target_make(sd::Renderer* renderer, const mtt::String& name, uvec2 size,  uint64 flags, Render_Target* rt_out);
bool Render_Target_make_no_depth_stencil(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out);

typedef enum COMPUTE_DISPATCH_TYPE {
    COMPUTE_DISPATCH_TYPE_CONCURRENT,
    COMPUTE_DISPATCH_TYPE_SERIAL
} COMPUTE_DISPATCH_TYPE;

struct Compute_Pass_Descriptor {
    COMPUTE_DISPATCH_TYPE type;
    bool use_own_command_buffer = false;
};

struct Buffer {
    const void* buffer;
    void* memory;
    usize alloc_bytes;
    usize size_bytes;
};

struct Render_Buffer_Set_Command {
    Buffer buffer;
    uint64 slot;
    usize byte_offset;
};

struct Blur {
    const void* handle;
    float32 radius;
};

struct Compute_Pass_Command {
    union {
        Compute_Pass_Descriptor descriptor;
        
        struct {
            Buffer buffer;
            uint64 slot;
            usize byte_offset;
        } buffer_set;
        
        struct {
            sd::Shader_ID shader_id;
        } shader_set;
        
        union {
            struct {
                uvec3 threadgroups_per_grid;
                uvec3 threads_per_threadgroup;
            } uniform_grid_args;
            struct {
                uvec3 threads_per_grid;
                uvec3 threads_per_threadgroup;
            } nonuniform_grid_args;
        } dispatch;
        
        struct {
            void* state;
            void (*callback)(void*);
            bool should_wait;
        } end_pass;
        
        struct {
            Blur blur;
            Texture_Handle texture_handle;
            bool should_wait;
        } blur_set;
        
        struct {
            Texture_Handle texture_handle_src;
            Texture_Handle texture_handle_dst;
        } sobel_set;
    };
};

// Encodes a compute command using a grid aligned to threadgroup boundaries.
void dispatch_threadgroups(sd::Renderer* r, vec3 threadgroups_per_grid, vec3 threads_per_threadgroup);

// Encodes a compute command using an arbitrarily sized grid.
void dispatch_threads(sd::Renderer* r, vec3 threads_per_grid, vec3 threads_per_threadgroup);

struct Compute_Shader_Info {
    usize thread_execution_width;
    usize max_total_threads_per_threadgroup;
};
bool compute_shader_info(sd::Renderer* r, sd::Shader_ID shader_id, Compute_Shader_Info* info);

void vertex_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset);
void vertex_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot);
void fragment_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset);
void fragment_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot);
void compute_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset);
void compute_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot);

void compute_shader_set(sd::Renderer* r, sd::Shader_ID shader_id);

MTT_NODISCARD sd::Shader_ID rasterization_shader_make_with_builtin_vertex_function(sd::Renderer* renderer, sd::Shader_ID vertex_shader_function_id, cstring fragment_function_src, cstring fragment_function_name);
sd::Shader_ID rasterization_shader_replace_with_builtin_vertex_function(sd::Renderer* renderer, sd::Shader_ID vertex_shader_function_id, cstring fragment_function_src, cstring fragment_function_name, Shader_ID shader_id);
void rasterization_shader_destroy(sd::Renderer* renderer, Shader_ID shader_id);


MTT_NODISCARD sd::Shader_ID compute_shader_make(sd::Renderer* renderer, cstring cstr, cstring main_name);

void compute_shader_destroy(sd::Renderer* renderer, Shader_ID shader_id);
sd::Shader_ID compute_shader_replace(sd::Renderer* renderer, cstring src_str, cstring main_name, sd::Shader_ID shader_id);

struct Custom_Command {
    void* state;
    void* data;
    void (*handler)(void* state, void* data);
};

struct Command {
    COMMAND_TYPE type;
    union {
        Projection_View_Transform projection_view_transform;
        sd::Draw_Command draw;
        uint64 pipeline_id;
        //Drawable_Instance_Data instance_data;
        Scissor_Rect scissor_rectangle;
        Rectangle rectangle;
        Viewport viewport;
        Custom_Command custom_command;
        
        sd::Draw_Instance_Command draw_instanced;
        
        sd::DEPTH_WRITE_MODE depth_mode;
        Render_Pass_Command render_pass;
        Render_Buffer_Set_Command render_pass_buffer_set;
        Compute_Pass_Command compute_pass;
        uint32 stencil_reference_value;
    };
    uint64 ID;
    uint64 flags;
    void (*callback)(bool, void*);
    
    Drawable_Info single_info;
    Command() {}
};
inline static uint64 ID_DEFAULT          = 0;
inline static uint64 RENDER_PASS_DEFAULT = 0;
inline static uint64 COMMAND_FLAGS_INIT = 0;

typedef enum DRAWABLE_COMMAND_FLAG {
    DRAWABLE_COMMAND_FLAG_IS_TRANSLUSCENT = (1 << 0),
    DRAWABLE_COMMAND_FLAG_IS_SHARED       = (1 << 1)
} DRAWABLE_COMMAND_FLAG;

struct Drawable_Command {
    uint64        drawable_id;
    
    uint64        flags;
    PIPELINE_MODE pipeline_mode;
    Texture_ID    texture_ID;
    Mat4          transform;
    Vec4          color_factor;
    Vec4          color_addition;
};

inline static bool flag_is_set(Drawable_Command* cmd, uint64 flag) {
    return cmd->flags & flag;
}

typedef uint64 Sort_Key[2];

struct Command_List {
    mtt::Dynamic_Array<Command> commands;
    
    inline usize size() {
        return commands.size();
    }
};

static inline void append(Command_List& cmd_list, const Command& cmd)
{
    cmd_list.commands.push_back(cmd);
}



struct Drawable_Info_Range {
    usize count;
    Drawable_Info* first;
    Drawable_Info* last;
};


using Coord = double;
using N = uint32;
using Point = std::array<Coord, 2>;
using Point2D_List = std::vector<std::vector<Point>>;

//typedef enum CLIP_OPERATION_TYPE {
//    CLIP_OPERATION_TYPE_INTERSECTION = ClipperLib::ctIntersection,
//    CLIP_OPERATION_TYPE_UNION        = ClipperLib::ctUnion,
//    CLIP_OPERATION_TYPE_DIFFERENCE   = ClipperLib::ctDifference,
//    CLIP_OPERATION_TYPE_XOR          = ClipperLib::ctXor,
//    CLIP_OPERATION_TYPE_NONE,
//    CLIP_OPERATION_COUNT
//} CLIP_TYPE;
//
//typedef enum CLIP_POLYGON_TYPE {
//    CLIP_POLYGON_TYPE_SUBJECT = ClipperLib::ptSubject,
//    CLIP_POLYGON_TYPE_CLIP    = ClipperLib::ptClip,
//    CLIP_POLYGON_TYPE_COUNT
//} CLIP_POLYGON_TYPE;
//enum class ClipType { None, Intersection, Union, Difference, Xor };
//
//enum class PathType { Subject, Clip };
typedef enum CLIP_OPERATION_TYPE {
    CLIP_OPERATION_TYPE_INTERSECTION = (unsigned)Clipper2Lib::ClipType::Intersection,
    CLIP_OPERATION_TYPE_UNION        = (unsigned)Clipper2Lib::ClipType::Union,
    CLIP_OPERATION_TYPE_DIFFERENCE   = (unsigned)Clipper2Lib::ClipType::Difference,
    CLIP_OPERATION_TYPE_XOR          = (unsigned)Clipper2Lib::ClipType::Xor,
    CLIP_OPERATION_TYPE_NONE         = (unsigned)Clipper2Lib::ClipType::None,
    CLIP_OPERATION_COUNT
} CLIP_TYPE;

typedef enum CLIP_POLYGON_TYPE {
    CLIP_POLYGON_TYPE_SUBJECT = (unsigned)Clipper2Lib::PathType::Subject,
    CLIP_POLYGON_TYPE_CLIP    = (unsigned)Clipper2Lib::PathType::Clip,
    CLIP_POLYGON_TYPE_COUNT
} CLIP_POLYGON_TYPE;

struct Boolean_Contour_Element {
    CLIP_OPERATION_TYPE clip_operation_type;
    CLIP_POLYGON_TYPE   clip_polygon_type;
    bool                is_closed = false;
    
    std::vector<std::vector<vec2>> paths = {};
};

static inline Scissor_Rect Viewport_to_Scissor_Rectangle(const Viewport& viewport)
{
    return {
        .x      = (uint64)viewport.x,
        .y      = (uint64)viewport.y,
        .width  = (uint64)viewport.width,
        .height = (uint64)viewport.height,
    };
}

static inline Viewport Scissor_Rectangle_to_Viewport(const Scissor_Rect& scissor_rect)
{
    return {
        .x      = (float64)scissor_rect.x,
        .y      = (float64)scissor_rect.y,
        .width  = (float64)scissor_rect.width,
        .height = (float64)scissor_rect.height,
        .znear = 0.0,
        .zfar = 1.0
    };
}

static inline Viewport Rectangle_to_Viewport(const Rectangle& rect)
{
    return {
        .x      = (float64)rect.x,
        .y      = (float64)rect.y,
        .width  = (float64)rect.width,
        .height = (float64)rect.height,
        .znear = 0.0,
        .zfar = 1.0
    };
}


void begin_path(Renderer* renderer);
Drawable_Info* end_path(Renderer* renderer);
Drawable_Info* end_path_closed(Renderer* renderer);
void break_path(Renderer* renderer);

void begin_path_no_new_drawable(Renderer* renderer);
void end_path_no_new_drawable(Renderer* renderer);

}

namespace sd {
void contour(Renderer* renderer, std::vector<Boolean_Contour_Element> const & contour);

void begin_polygon(Renderer* renderer);
Drawable_Info* end_polygon(Renderer* renderer);

void begin_polygon_no_new_drawable(Renderer* renderer);
void end_polygon_no_new_drawable(Renderer* renderer);

void begin_drawable(Renderer* renderer);
Drawable_Info* end_drawable(Renderer* renderer);

void set_color_rgba(Renderer* renderer, float32, float32, float32, float32);
void set_color_rgba_v4(Renderer* renderer, vec4);
static inline void set_color_rgba(Renderer* renderer, vec4 c)
{
    set_color_rgba_v4(renderer, c);
}
static inline void set_color_rgb(Renderer* renderer, float32 r, float32 g, float32 b)
{
    set_color_rgba(renderer, r, g, b, 1.0f);
}
static inline void set_color_rgb(Renderer* renderer, vec3 c)
{
    set_color_rgba_v4(renderer, vec4(c, 1.0f));
}

vec4 get_color_rgba(Renderer* renderer);

void set_color_hsva(Renderer* renderer, float32, float32, float32, float32);
void set_color_hsva_v4(Renderer* renderer, vec4);


vec4 get_background_color(Renderer* renderer);
void set_background_color_rgba(Renderer* renderer, float32, float32, float32, float32);

void set_background_color_rgba_v4(Renderer* renderer, vec4);

void set_render_layer(Renderer* renderer, Render_Layer_ID render_layer);
Render_Layer_ID get_render_layer(Renderer* renderer);

void rewind_layer(Renderer* renderer, Render_Layer_ID render_layer);
void rewind_staging_layer(Renderer* renderer, Render_Layer*);
extern void (*seek_layer_to)(Renderer* renderer, Drawable_Info* rc);



void vertex(Renderer* renderer, float64 x, float64 y, float64 z);
void vertex_v2(Renderer* renderer, SD_vec2 position);
void vertex_v3(Renderer* renderer, SD_vec3 position);

void vertex(Renderer* renderer, float64 x, float64 y, float64 z, SD_vec2 uv);
void vertex_v2(Renderer* renderer, SD_vec2 position, SD_vec2 uv);
void vertex_v3(Renderer* renderer, SD_vec3 position, SD_vec2 uv);

void vertex(Renderer* renderer, float64 x, float64 y, float64 z, SD_vec2 uv, SD_vec4 color);
mtt::Array_Slice<sd::Index_Type> index_list(Renderer* renderer, sd::Drawable_Info* info, usize size);
void set_triangle_indices(sd::Renderer* renderer, mtt::Array_Slice<sd::Index_Type>& index_list, usize idx, Index_Type a, Index_Type b, Index_Type c);

void triangle_strip_to_indexed_triangles_in_place(Renderer* renderer, sd::Drawable_Info* info);

void path_vertex(Renderer* renderer, float64 x, float64 y, float64 z);
void path_vertex_v2(Renderer* renderer, SD_vec2 position);
void path_vertex_v3(Renderer* renderer, vec3 position);
void path_list(Renderer* renderer, vec3*, usize count);
void path_list_with_offset(Renderer* renderer, vec3* list, usize count, vec3 offset);
void path_list(Renderer* renderer, vec3* list, usize count, float64* radii);
void path_list_with_offset(Renderer* renderer, vec3* list, usize count, vec3 offset, float64* radii);
void path_radius(Renderer* renderer, float32 pixel_radius);
void set_path_radius(Renderer* renderer, float32 pixel_radius);
float32 get_path_radius(Renderer* renderer);
void path_arrow_head(Renderer* renderer, vec3 src_point, vec3 dst_point, float32 radius);
void path_arrow_head_with_tail(Renderer* renderer, vec3 src_point, vec3 dst_point, float32 radius);
void path_arrow_head_bidirectional(Renderer* renderer, SD_vec3 src_point, SD_vec3 dst_point, float32 radius);

void quad_color(Renderer* renderer, SD_vec2 tl, SD_vec2 bl, SD_vec2 br, SD_vec2 tr, float32 depth, vec4 ctl, vec4 cbl, vec4 cbr, vec4 ctr);
void quad_color_uv(Renderer* renderer, SD_vec2 tl, SD_vec2 bl, SD_vec2 br, SD_vec2 tr, float32 depth, SD_vec4 ctl, SD_vec4 cbl, SD_vec4 cbr, SD_vec4 ctr, SD_vec2 uv_tl, SD_vec2 uv_bl, SD_vec2 uv_br, SD_vec2 uv_tr);

void triangle(Renderer* renderer, vec3 a, vec3 b, vec3 c);
void triangle_color(Renderer* renderer, vec3 a, vec3 b, vec3 c, vec4 color);
void triangle_per_vertex_color(Renderer* renderer, vec3 a, vec3 b, vec3 c, vec4 color_a, vec4 color_b, vec4 color_c);

void triangle_equilateral(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side);
void triangle_equilateral_color(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side, vec4 color);
void triangle_equilateral_per_vertex_color(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side, vec4 color_height, vec4 color_left, vec4 color_right);

void disable_command(Renderer*, Drawable_Info*);
void enable_command(Renderer*, Drawable_Info*);
void polygon_convex_regular(Renderer*, float32, vec3, usize);
void polygon_convex_regular_v2(Renderer*, float32, SD_vec2, usize);
void polygon_arc_fraction(Renderer* renderer, float32 radius, vec3 center, usize count_sides, float32 arc_fraction);
void circle(Renderer* renderer, float32 radius, vec3 center);
void circle(Renderer* renderer, float32 radius, vec3 center, float32 poly_mode_sides);
void quad(Renderer* renderer, SD_vec2 tl, SD_vec2 bl, SD_vec2 br, SD_vec2 tr, float32 depth);
void path_quad(Renderer*, SD_vec2, SD_vec2, SD_vec2, SD_vec2, float32);
void rectangle(Renderer* renderer, SD_vec2 tl, SD_vec2 dimensions, float32 depth);
void rectangle_w_corners(Renderer* renderer, SD_vec2 tl, SD_vec2 br, float32 depth);

void rectangle_rounded(Renderer* renderer, SD_vec2 tl, SD_vec2 dimensions, float32 depth, float32 roundness, int32 segments);

void rectangle_dashed(Renderer* renderer, SD_vec2 tl, SD_vec2 dimensions, float32 depth, usize segments);


static inline Rectangle calc_rectangle_tl_from_center_radius(vec2 center, SD_vec2 radius)
{
    SD_vec2 tl = center - radius;
    SD_vec2 dimensions = radius * 2.0f;
    return Rectangle {.x = tl[0], .y = tl[1], .width = dimensions[0], .height = dimensions[1] };
}

//template <typename PROC, typename ...ARGS>
//static inline void rectangle_proc_from_center_radius(Renderer* renderer, PROC&& proc, vec3 center, vec3 radius, ARGS... args)
//{
//    SD_vec2 tl = center - radius;
//    SD_vec2 dimensions = radius * 2.0f;
//    return proc(renderer, tl, dimensions, args...);
//}

void set_depth(Renderer*, float64);

void Renderer_init(Renderer* renderer, mem::Allocator* allocator);



extern usize (*Renderer_sizeof)(void);
extern usize (*Render_Layer_sizeof)(void);
extern usize (*Render_Layer_sizeof_polygon_vertex)(void);
extern usize (*Render_Layer_sizeof_path_vertex)(void);
extern usize (*Render_Layer_sizeof_index)(void);

extern void (*Render_Layer_copy)(Renderer* renderer, usize to, Render_Layer* from);

extern void (*make_staging_layer)(
                                  Renderer* renderer,
                                  Render_Layer** location,
                                  void* vertex_memory,
                                  usize vertex_byte_count,
                                  void* index_memory,
                                  usize index_byte_count
                                  );

extern void (*make_staging_layer_with_allocator)(
                                                 Renderer* renderer,
                                                 Render_Layer** location,
                                                 usize vertex_byte_count,
                                                 usize index_byte_count,
                                                 mem::Allocator*
                                                 );

extern void (*deallocate_staging_layer_with_allocator)(
                                                       Renderer* renderer,
                                                       Render_Layer** location
                                                       );

extern Drawable_Info_Range (*append_staging_layer)(Renderer* renderer, Render_Layer_ID to_layer, Render_Layer* from_stage);
//extern Drawable_Info_Range (*append_layer)(Renderer* renderer, Render_Layer_ID to_layer, Render_Layer* from_layer);

extern void (*set_staging_layer)(Renderer* renderer, Render_Layer* stage);

Render_Layer_ID Render_Layer_make(Renderer* renderer, usize block_count);
//extern bool (*Render_Layer_reallocate)(Renderer* renderer, usize block_count, Render_Layer_ID rlid);
bool Render_Layer_reallocate_bytes(Renderer* renderer, usize block_count, Render_Layer_ID rlid);
bool Render_Layer_reallocate_vertex_bytes(Renderer* renderer, usize block_count, Render_Layer_ID rlid);
bool Render_Layer_reallocate_index_bytes(Renderer* renderer, usize block_count, Render_Layer_ID rlid);
bool Render_Layer_reserve_render_data_space(sd::Renderer* renderer, usize requested_vertex_count, usize requested_index_count);

void path_bezier(Renderer* renderer, vec3 src_point, vec3 p_mid_anchor1, vec3 p_mid_anchor2, vec3 dst_point, usize segment_count);

void path_bezier_arrow_head_with_tail(Renderer* renderer, vec3 src_point, vec3 p_mid_anchor1, vec3 p_mid_anchor2, vec3 dst_point, float32 radius, usize segment_count);


void path_cross(Renderer* renderer, const vec3 center, vec3 offset);

void continue_path(Renderer* renderer);

extern void (*resize_staging_layer)(Renderer* renderer,
                                    Render_Layer** location,
                                    void* vertex_memory,
                                    usize vertex_byte_count,
                                    void* index_memory,
                                    usize index_byte_count);


void triangulate(Renderer* renderer, sd::Point2D_List* solution);

struct Renderer_Staging_Layer {
    sd::Render_Layer* layer;
    sd::Drawable_Info_Range drawable_range;
    
    void* vertices;
    usize v_count;
    void* indices;
    usize i_count;
    
};

vec4 view_offset_for_safe_area(sd::Renderer* renderer);


void set_view(Renderer*, mat4);
Mat4 get_view(Renderer*);

void push_projection_view_transform(Renderer*, mat4, mat4);
void pop_view_transform(Renderer*);

void push_draw_command_with_layer(Renderer*, Render_Layer_ID);
void push_draw_command_with_staging_layer(Renderer*, Render_Layer*);

void push_draw_command_with_info_layer_and_buffer_layer(Renderer* renderer, Render_Layer_ID render_layer_w_drawable_info, Render_Layer_ID render_layer_w_buffer);

void push_instanced_draw_command_with_layer(Renderer* renderer, Render_Layer_ID render_layer, Drawable_Instance_Data* instance_data);
//void push_instanced_draw_command_with_drawable(Renderer* renderer, Drawable_Info* drawable, Drawable_Info* list, usize count);

void push_instanced_draw_command_with_drawable_list(Renderer* renderer, Drawable_Info* drawable, MTT_List* list);

void push_instanced_draw_command_with_drawable_list(Renderer* renderer, Drawable_Info* drawable, mtt::Array_Slice<sd::Drawable_Info> list);

//void push_instanced_draw_command_lists_with_drawable(Renderer* renderer, Drawable_Info* drawable, Drawable_Info_List* lists, usize count);


void push_draw_command_with_layer_range(Renderer* renderer, Render_Layer_ID render_layer, Render_Layer_Range layer_range);

void push_instanced_draw_command_with_staging_layer(Renderer* renderer, Render_Layer* staging_layer, Drawable_Instance_Data* instance_data);

void push_draw_command_with_drawable_info(Renderer* renderer, Drawable_Info* drawable_info);

void push_pipeline(Renderer*, uint64);
void push_color_pipeline(Renderer*);
void push_color_hsv_pipeline(Renderer* renderer);
void push_texture_pipeline(Renderer*);
void push_text_pipeline(Renderer*);

void push_scissor_rect(Renderer*, Scissor_Rect);
void push_viewport(Renderer* renderer, Viewport viewport);

void push_custom_command(Renderer*, Custom_Command custom);

void set_render_pipeline(Renderer* renderer, uint64 ID);
void render_to_texture_sync_begin(Renderer* renderer, sd::Rectangle bounds);
void render_to_texture_end(Renderer* renderer, void (*callback)(bool, void*));

void set_depth_write_mode(Renderer*, sd::DEPTH_WRITE_MODE);

void set_stencil_reference_value(Renderer* renderer, uint32 reference_value);

void render_pass_begin(Renderer* r, const Render_Pass_Descriptor& conf);
void render_pass_begin(Renderer* r);
void render_pass_end(Renderer* r);

void compute_pass_begin(Renderer* r, const Compute_Pass_Descriptor& conf);
void compute_pass_end(Renderer* r);

struct Compute_Pass_End_Descriptor {
    void* state = nullptr;
    void (*callback)(void*) = nullptr;
    bool should_wait = true;
};
void compute_pass_end(Renderer* r, const Compute_Pass_End_Descriptor& desc);

void Command_List_begin(Renderer*, Command_List*);
void Command_List_end(Renderer*, Command_List*);

void Command_List_submit(Renderer*, Command_List*);

void clear_commands(Renderer*);

//void print_commands(Renderer*);

Drawable_Info* pause_path(Renderer* renderer);


sd::Texture_ID texture_id_from_name(sd::Renderer* renderer, const mtt::String& name);

bool texture_is_valid(sd::Renderer* renderer, sd::Texture_ID texture_id);



template <typename Proc> Drawable_Info* build_drawable(Renderer* renderer, Render_Layer_ID layer_id, Proc&& proc)
{
    set_render_layer(renderer, layer_id);
    begin_drawable(renderer);
    proc();
    return end_drawable(renderer);
}

Render_Layer_ID layer_id(Render_Layer* layer);

void save(Renderer* renderer);
void restore(Renderer* renderer);

Drawable_Info* Drawable_Info_copy(Renderer* renderer, Render_Layer_ID ID, Drawable_Info* info);


Rectangle get_display_bounds(void);
Rectangle get_native_display_bounds(void);
float64 get_native_display_scale(void);

usize max_texture_dimension_2(sd::Renderer* renderer);


sd::Images* images(Renderer* renderer);

bool is_see_through_background(sd::Renderer* r);
void set_see_through_background(sd::Renderer* r, bool state);

void Drawable_Info_print(Drawable_Info* rc);


void* Buffer_contents(sd::Buffer* b);

usize Buffer_byte_size(sd::Buffer* b);

bool Buffer_is_valid(sd::Buffer* b);

bool Buffer_make(Renderer* renderer, Buffer* buffer, usize byte_count);

void Buffer_destroy(Renderer* renderer, Buffer* buffer);

SD_vec2 depth_range_default(sd::Renderer* renderer);

Blur Blur_make(sd::Renderer* renderer, float32 radius);
void Blur_destroy(sd::Renderer* renderer, Blur* blur);
void compute_set_blur(sd::Renderer* renderer, Blur* blur, Texture_Handle* texture_handle, bool should_wait);
void compute_set_blur(sd::Renderer* renderer, float32 radius, Texture_Handle* texture_handle, bool should_wait);

void compute_set_sobel(sd::Renderer* renderer, Texture_Handle* texture_handle_dst, Texture_Handle* texture_handle_src);

}



#endif /* stratadraw_h */

#ifdef SD_IMPLEMENTATION
#undef SD_IMPLEMENTATION

namespace sd {

void (*seek_layer_to)(Renderer* renderer, Drawable_Info* rc);


usize (*Renderer_sizeof)(void);
usize (*Render_Layer_sizeof)(void);
usize (*Render_Layer_sizeof_polygon_vertex)(void);
usize (*Render_Layer_sizeof_path_vertex)(void);
usize (*Render_Layer_sizeof_index)(void);

void (*Render_Layer_copy)(Renderer* renderer, usize to, Render_Layer* from);

void (*make_staging_layer)(
                           Renderer* renderer,
                           Render_Layer** location,
                           void* vertex_memory,
                           usize vertex_byte_count,
                           void* index_memory,
                           usize index_byte_count
                           );

void (*make_staging_layer_with_allocator)(
                                          Renderer* renderer,
                                          Render_Layer** location,
                                          usize vertex_byte_count,
                                          usize index_byte_count,
                                          mem::Allocator*
                                          );


Drawable_Info_Range (*append_staging_layer)(Renderer* renderer, Render_Layer_ID to_layer, Render_Layer* from_stage);
//Drawable_Info_Range (*append_layer)(Renderer* renderer, Render_Layer_ID to_layer, Render_Layer* from_layer);

void (*set_staging_layer)(Renderer* renderer, Render_Layer* stage);

void save_staging_layer(Renderer* renderer, Render_Layer* stage);

//bool (*Render_Layer_reallocate)(Renderer* renderer, usize block_count, Render_Layer_ID rlid);

void (*resize_staging_layer)(Renderer* renderer,
                             Render_Layer** location,
                             void* vertex_memory,
                             usize vertex_byte_count,
                             void* index_memory,
                             usize index_byte_count);

void (*deallocate_staging_layer_with_allocator)(
                                                Renderer* renderer,
                                                Render_Layer** location
                                                );

}

#endif


