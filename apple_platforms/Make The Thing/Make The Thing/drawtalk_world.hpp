//
//  drawtalk_world.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/7/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_world_hpp
#define drawtalk_world_hpp

#include "stratadraw.h"

#include "camera.hpp"

#include "color.hpp"
#include "curve.hpp"
#include "collision.hpp"
#include "collision_defaults.hpp"
#include "thing.hpp"
#include "memory.hpp"

#include "drawtalk.hpp"

#include "input_multimodal.hpp"



#include "web_browser_support.hpp"

#include "image.hpp"

//#include "nanovg.h"
//#include "nanovg_test_drawing.hpp"

#include "render_layer_labels.hpp"

#include "word_info.hpp"

struct Render_Info {
    sd::Render_Layer_ID render_layers[LAYER_LABEL_COUNT];
    usize render_layers_count;
    
    sd::Renderer_Staging_Layer grid_layer;
    bool debug_grid_is_on;
    
    sd::Image* remote_screen = nullptr;
    sd::Texture_ID bg_texture_id = sd::Texture_ID_INVALID;
    vec2 bg_texture_dimensions = vec2(0.0f);
//    Pool_Allocation_init(&this->drawable_pool_alloc, this->allocator, 1024, sizeof(sd::Drawable_Info), 16);
//    MTT_List_init_with_allocator(&this->list, &this->list_pool_alloc.allocator);
    
    
};

struct Touch_Sequence {
    std::vector<usize> order_ids;
    usize sequence_offset;
};
struct Input_Info {
    //usize next_sequence_number;
    Touch_Sequence sequence;
    usize pen_tool_id;
    usize touch_tool_id;
};

struct alignas(16) DrawTalk_World {
    Render_Info render_info = {};
    
    //mtt::Collision_System collision_system;
    
    Input_Info input_info = {};
    
    dt::Control_System controls = {};
    
    mtt::World mtt_world;
    mtt::External_World ext_worlds[1];
    
    dt::DrawTalk dt = {};
    mtt::Camera cam = {};
    usize cam_stack_idx = 0;
#define DT_CAM_STACK_MAX (2)
    mtt::Camera* cam_stack[DT_CAM_STACK_MAX];
    
    
    //mtt::Camera* active_cam = nullptr;
    Mat4        main_projection = {};
    //Mat4        projection;
    sd::Viewport scaled_viewport = {};
    
    mtt::Thing_ID system_thing_canvas = mtt::Thing_ID_INVALID;
    mtt::Thing_ID system_thing_I = mtt::Thing_ID_INVALID;
    mtt::Thing_ID system_thing_View = mtt::Thing_ID_INVALID;
};
inline static void cam_stack_init(DrawTalk_World* dt, mtt::Camera* cam)
{
    dt->cam_stack[dt->cam_stack_idx] = cam;
    //dt->cam = *dt->cam_stack[dt->cam_stack_idx];
    //dt->active_cam = cam;
}
inline static mtt::Camera* cam_stack_top(DrawTalk_World* dt)
{
    return dt->cam_stack[dt->cam_stack_idx];
}

inline static mtt::Camera* cam_stack_push(DrawTalk_World* dt, mtt::Camera* cam)
{
    ASSERT_MSG(dt->cam_stack_idx < (DT_CAM_STACK_MAX - 1), "cam stack pushed too many times!\n");
    dt->cam_stack_idx += 1;
    dt->cam_stack[dt->cam_stack_idx] = cam;
    return cam;
}
inline static mtt::Camera* cam_stack_pop(DrawTalk_World* dt)
{
    ASSERT_MSG(dt->cam_stack_idx > 0, "cam stack popped too many times!\n");
    
    dt->cam_stack_idx -= 1;
    return dt->cam_stack[dt->cam_stack_idx];
}
inline static DrawTalk_World* DrawTalk_World_ctx(void)
{
    return static_cast<DrawTalk_World*>(mtt_core_ctx()->user_data);
}
inline static DrawTalk_World* DrawTalk_World_ctx(MTT_Core* core)
{
    return static_cast<DrawTalk_World*>(core->user_data);
}

inline static bool Camera_is_drawable(mtt::Camera* camera)
{
    return (camera == &(static_cast<DrawTalk_World*>(mtt_core_ctx()->user_data)->cam));
}

#define DT_THING_PROXY_SCENE_IDX_SEMANTIC_DIAGRAM_VIEW 0
#define DT_THING_PROXY_SCENE_IDX_CONTEXT_PANEL_VIEW 1
#define DT_THING_PROXY_SCENE_COUNT (2)

#endif /* drawtalk_world_hpp */
