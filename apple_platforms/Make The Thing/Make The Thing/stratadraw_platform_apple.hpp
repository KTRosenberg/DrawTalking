// MARK: IOS DECLARATION

#ifndef stratadraw_PLATFORM_APPLE_HPP
#define stratadraw_PLATFORM_APPLE_HPP

#include "ShaderTypes.h"

#include "LineRenderer.h"

#include "stratadraw.h"

#include "image.hpp"
#include "image_platform.hpp"


namespace sd {

#define SD_MAX_RENDER_COMMANDS_PER_LAYER (4096)
#define SD_MAX_DRAWABLE_INFO_PER_LAYER (4096 * 4)



struct Saved_State {
    SD_vec4 color;
    SD_vec4 prev_color;
    SD_vec4 bg_color;
    Render_Layer_ID layer_id;
    float64 active_depth;
};

struct Render_Layer {
    RENDER_LAYER_TYPE type;
    
    VERTEX_Polygon_Color* polygon_color_vertices;
    Index_Type*           polygon_color_indices;
    usize                 polygon_color_vertices_count;
    usize                 polygon_color_vertices_count_max;
    usize                 polygon_color_vertices_count_bytes;
    usize                 polygon_color_indices_count;
    usize                 polygon_color_indices_count_max;
    usize                 polygon_color_indices_count_bytes;
    Index_Type            polygon_color_next_index;
    
    usize saved_vbuffer_count;
    usize saved_ibuffer_count;
    usize saved_render_command_count;
    
     
    Drawable_Info padding;
    Drawable_Info drawable_info_list[SD_MAX_DRAWABLE_INFO_PER_LAYER];
    usize drawable_info_list_count;
    Drawable_Info* active_drawable_info;
    
    sd::LineRenderer path_renderer;
    usize layer_index;
    mem::Allocator* allocator;
    
    SHAPE_MODE saved_shape_mode;
    mtt::Dynamic_Array<Drawable_Info*> free_list;
    mtt::Dynamic_Array<Drawable_Info*> free_list_info_SD_HACK;
    
    usize saved_v_buffer_count;
    usize saved_i_buffer_count;
    usize saved_next_index;
    usize saved_i_buffer_count_used;
    
    sd::LineRenderer saved_path_renderer;
    sd::Drawable_Info saved_drawable_info;
};


struct Pen {
    usize ID;
    SD_vec4 color;
};
struct Pen_Collection {
    Pen pens[4];
};



#define SD_MAX_RENDER_LAYERS (128)

#define SAVED_LAYER (false)

struct Renderer {
    mem::Allocator cmd_allocator_backing = {};
    mem::Allocator cmd_allocator = {};
    mem::Arena cmd_mem = {};
    
    
    SD_mat4 projection = SD_mat4_identity();
    SD_mat4 view = SD_mat4_identity();
    
    Render_Layer_ID active_render_layer_id = 0;
    Render_Layer_ID saved_render_layer_id = 0;
    
    Render_Layer* render_layers = nullptr;//[(SD_MAX_RENDER_LAYERS + 1)]; // + 1 for active staging buffer
    usize render_layer_count = 0;
    usize render_layer_max_count = 0;
    SD_vec4 color = SD_vec4(1.0f);
    SD_vec4 prev_color = SD_vec4(1.0f);
    SD_vec4 background_color = SD_vec4(0.0f);
    
    SHAPE_MODE shape_mode = SHAPE_MODE_NONE;
    
    float64 active_depth = 0.0;
    
    usize saved_render_command_index = 0;
    
    Saved_State saved_state = {};

    
    void* backend = nullptr;
    
    //SD_mat4 transform_stack[(SD_MAX_RENDER_COMMANDS_PER_LAYER)];
    
    Command_List* active_cmd_list = nullptr;
    
    Command_List* cmd_lists = nullptr;
    usize cmd_list_count = 0;
    
    Images* images = nullptr;
    Image_Loader* image_loader = nullptr;
    
#if SAVED_LAYER
    Render_Layer saved_layer = {};
#endif
    usize max_buffer_size = std::numeric_limits<usize>::max();
    
    SD_vec2 depths_default;
};





void Renderer_deinit(Renderer*);
Renderer Renderer_make(void);


void setup(void);


static inline SD_Renderer_Metal_Backend* Renderer_backend(sd::Renderer* r)
{
    return (__bridge SD_Renderer_Metal_Backend*)r->backend;
}

}

#if TARGET_OS_OSX
#define Platform_Screen NSScreen

typedef NSRect Platform_Screen_Rect;
typedef CGFloat Platform_Scale;
typedef NSView Platform_View;

#else
#define Platform_Screen UIScreen

typedef CGRect Platform_Screen_Rect;
typedef CGFloat Platform_Scale;
typedef UIView Platform_View;

#endif

extern Platform_View* backing_view;

Platform_Screen* get_main_screen(void);
Platform_Screen_Rect get_bounds(Platform_Screen* screen);
Platform_Screen_Rect get_bounds();
Platform_Screen_Rect get_native_bounds(Platform_Screen* screen);
Platform_Screen_Rect get_native_bounds();
Platform_Scale get_native_scale(Platform_Screen* screen);
Platform_Scale get_native_scale();


namespace mapbox {
namespace util {

//template <>
//struct nth<0, ClipperLib::IntPoint> {
//    inline static auto get(const ClipperLib::IntPoint &t) {
//        return t.X;
//    };
//};
//template <>
//struct nth<1, ClipperLib::IntPoint> {
//    inline static auto get(const ClipperLib::IntPoint &t) {
//        return t.Y;
//    };
//};

template <>
struct nth<0, Clipper2Lib::PointD> {
    inline static auto get(const Clipper2Lib::PointD &t) {
        return t.x;
    };
};
template <>
struct nth<1, Clipper2Lib::PointD> {
    inline static auto get(const Clipper2Lib::PointD &t) {
        return t.y;
    };
};

template <>
struct nth<0, SD_vec2> {
    inline static auto get(const SD_vec2 &t) {
        return t.x;
    };
};
template <>
struct nth<1, SD_vec2> {
    inline static auto get(const SD_vec2 &t) {
        return t.y;
    };
};




} // namespace util
} // namespace mapbox

#endif

// MARK: IOS IMPLEMENTATION

#ifdef SD_IMPLEMENTATION_IOS
#undef SD_IMPLEMENTATION_IOS

Platform_View* backing_view;

#if TARGET_OS_OSX

Platform_Screen* get_main_screen(void)
{
    return Platform_Screen.mainScreen;
}

Platform_Screen_Rect get_bounds(Platform_Screen* screen)
{
    return backing_view.bounds;
}
Platform_Screen_Rect get_bounds()
{
    return get_bounds(get_main_screen());
}

Platform_Screen_Rect get_native_bounds(Platform_Screen* screen)
{
    return backing_view.bounds;
}
Platform_Screen_Rect get_native_bounds()
{
    return get_native_bounds(get_main_screen());
}


Platform_Scale get_native_scale(Platform_Screen* screen)
{
    //auto x = [backing_view convertSizeToBacking:NSMakeSize(1, 1)];
    return screen.backingScaleFactor;
}
Platform_Scale get_native_scale()
{
    return get_native_scale(get_main_screen());
}

namespace sd {

Rectangle get_display_bounds(void)
{
    auto bounds = get_bounds();
    return (Rectangle) {
        .x = bounds.origin.x,
        .y = bounds.origin.y,
        .width = bounds.size.width,
        .height = bounds.size.height
    };
}
Rectangle get_native_display_bounds(void)
{
    auto bounds = get_native_bounds();
    return (Rectangle) {
        .x = bounds.origin.x,
        .y = bounds.origin.y,
        .width = bounds.size.width,
        .height = bounds.size.height
    };
}
float64 get_native_display_scale(void)
{
    return get_native_scale();
}

}

#else

Platform_Screen* get_main_screen(void)
{
    return Platform_Screen.mainScreen;
}

Platform_Screen_Rect get_bounds(Platform_Screen* screen)
{
    return screen.bounds;
}
Platform_Screen_Rect get_bounds()
{
    return get_bounds(get_main_screen());
}

Platform_Screen_Rect get_native_bounds(Platform_Screen* screen)
{
    return screen.nativeBounds;
}
Platform_Screen_Rect get_native_bounds()
{
    return get_native_bounds(get_main_screen());
}

Platform_Scale get_native_scale(Platform_Screen* screen)
{
    return screen.nativeScale;
}
Platform_Scale get_native_scale()
{
    return get_native_scale(get_main_screen());
}

namespace sd {

Rectangle get_display_bounds(void)
{
    auto bounds = get_bounds();
    return (Rectangle) {
        .x = bounds.origin.x,
        .y = bounds.origin.y,
        .width = bounds.size.width,
        .height = bounds.size.height
    };
}
Rectangle get_native_display_bounds(void)
{
    auto bounds = get_native_bounds();
    return (Rectangle) {
        .x = bounds.origin.x,
        .y = bounds.origin.y,
        .width = bounds.size.height,
        .height = bounds.size.width
    };
}
float64 get_native_display_scale(void)
{
    return get_native_scale();
}



}

#endif




#endif
