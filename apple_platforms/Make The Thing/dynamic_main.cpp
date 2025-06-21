//
//  dynamic_main.cpp
//  dynamic_dev
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#include "dynamic_main.hpp"

#include "render_layer_labels.hpp"
#include "image.hpp"
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
#include "version.hpp"
#else
#include "drawtalk_world.hpp"
#define VERSION(name) name
#define VERSION_NUMBER 0
#endif

#define ENABLE_ALL_TESTS (0)

#define COMPUTE_TEST (0)

#define LIGHTS_TEST (0)
#define BLUR_LIGHTS (0)

#define USE_GRAYSCALE (0)

#define ATLAS_TEST (0)


#if ENABLE_ALL_TESTS

#ifdef COMPUTE_TEST
#undef COMPUTE_TEST
#endif
#define COMPUTE_TEST (1)

#ifdef LIGHTS_TEST
#undef LIGHTS_TEST
#endif
#define LIGHTS_TEST (1)

//#ifdef ATLAS_TEST
//#undef ATLAS_TEST
//#endif
//#define ATLAS_TEST (1)

#endif


#if COMPUTE_TEST
#include "shader_playground.hpp"
#endif




struct DrawTalk_World;

namespace ext {

struct Compute_Shader_Test {
    sd::Shader_ID shader_id = sd::Shader_ID_INVALID;
    sd::Shader_ID grayscale_shader_id = sd::Shader_ID_INVALID;
    sd::Buffer buffer = {};
    usize texture_width = 0;
    usize texture_height = 0;
    unsigned int buffer_size = 0;
    sd::Texture_ID texture_id = sd::Texture_ID_INVALID;
    sd::Blur blur;
    sd::Texture_Handle texture_handle;
};

struct State {
    Compute_Shader_Test compute_shader_test;

//    sd::Texture_Handle texture_handle_lights;
//    float32 texture_lights_width;
//    float32 texture_lights_height;
    sd::Render_Target render_target_lights = {};
    sd::Render_Target render_target_atlas_test = {};
    
    std::vector<sd::Image_Atlas::Region> reserved = {};
    vec2 atlas_scale = vec2(1.0, 1.0);
};

#if COMPUTE_TEST
void compute_test_init_textures(MTT_Core* core, sd::Renderer* renderer, Compute_Shader_Test& compute_test)
{
    {
        dt::DrawTalk* dt = dt::ctx();
        dt::UI& ui = dt->ui;
        
        dt::Panel& ndom_panel = ui.margin_panels[0];
        
        float32 height_offset = ndom_panel.bounds.dimensions.y;
        
        const float32 sample_count = 1;
        auto scale = sd::get_native_display_scale();
        float32 scale_factor = 0.25 * scale;
        compute_test.texture_width  = m::max((uint64)((core->viewport.width) * sample_count * scale_factor), 1llu);
        compute_test.texture_height = m::max((uint64)((core->viewport.height - height_offset) * sample_count * scale_factor), 1llu);
        
        {
            sd::Texture_Handle tex;
            sd::Image* img = sd::Image_create_empty_w_size(sd::images(renderer), "compute", uvec2(compute_test.texture_width, compute_test.texture_height), { .usage_flags = 0 }, &tex);
            if (img == nullptr) {
                compute_test.texture_id = sd::Texture_ID_INVALID;
            } else {
                compute_test.texture_id = tex.id;
                compute_test.texture_handle = tex;
            }
            compute_test.texture_id = tex.id;
        }
    }
}
#endif

void render_target_texture_make(MTT_Core* core, sd::Renderer* renderer, State* state, const mtt::String& name, sd::Render_Target* render_target, uint64 flags, vec2 scale_v) {

    auto scale = sd::get_native_display_scale();
    float32 scale_factor = scale;
    usize texture_width  = m::max((uint64)((core->viewport.width) * scale_factor * scale_v.x), 1llu);
    usize texture_height = m::max((uint64)((core->viewport.height) * scale_factor * scale_v.y), 1llu);
    
    if (!Render_Target_make_msaa(renderer, name, uvec2(texture_width, texture_height), flags, render_target)) {
        MTT_print("Error %s %d in %s, failed to create render target\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void init_render_targets(mtt::DL_Ctx* ctx, MTT_Core* core, sd::Renderer* renderer, State* state)
{
    state->atlas_scale = vec2(1, 1);
#if LIGHTS_TEST
    {
        uint64 flags = (uint64)(sd::TEXTURE_USAGE_render_target | sd::TEXTURE_USAGE_shader_write);
#if BLUR_LIGHTS
        flags |= sd::TEXTURE_USAGE_shader_read;
#endif
        
        render_target_texture_make(core, renderer, ((State*)ctx->state), "lights", &((State*)ctx->state)->render_target_lights, flags, vec2(1.0));
    }
#endif
    
#if ATLAS_TEST
    {
        uint64 flags = (uint64)(sd::TEXTURE_USAGE_render_target | sd::TEXTURE_USAGE_shader_write);
        //flags |= sd::TEXTURE_USAGE_shader_read;
        
        render_target_texture_make(core, renderer, ((State*)ctx->state), "atlas", &((State*)ctx->state)->render_target_atlas_test, flags, ((State*)ctx->state)->atlas_scale);
    }
#endif
}

void* renderer_init(void* args)
{
    auto* ctx = (mtt::DL_Ctx*)args;
    MTT_Core* core = ctx->core;
    
    DrawTalk_World* const world = static_cast<DrawTalk_World*>(core->user_data);
    mtt::World* mtt_world = &world->mtt_world;
    
    float64 t_seconds = core->time_sim_elapsed;
    uint64 t_nanos = core->time_sim_elapsed_ns;
    sd::Renderer* renderer = core->renderer;
    
#if COMPUTE_TEST
    //MTT_print("shader=[%s]\n", sd::playground::program);
    Compute_Shader_Test& compute_test = ((State*)ctx->state)->compute_shader_test;
    if (compute_test.shader_id == sd::Shader_ID_INVALID) {
        compute_test.shader_id = sd::compute_shader_make(renderer,
                                                         sd::playground::program
                                                         , "main_entry");
    } else {
        compute_test.shader_id = sd::compute_shader_replace(renderer,
                                                            sd::playground::program
                                                            , "main_entry", compute_test.shader_id);
    }
    if (compute_test.shader_id == sd::Shader_ID_INVALID) {
        return nullptr;
    }
    
#if USE_GRAYSCALE
    if (compute_test.grayscale_shader_id == sd::Shader_ID_INVALID) {
        compute_test.grayscale_shader_id = sd::compute_shader_make(renderer,
                                                         sd::playground::program_grayscale
                                                         , "grayscale");
    } else {
        compute_test.grayscale_shader_id = sd::compute_shader_replace(renderer,
                                                            sd::playground::program_grayscale
                                                            , "grayscale", compute_test.grayscale_shader_id);
    }
    if (compute_test.grayscale_shader_id == sd::Shader_ID_INVALID) {
        return nullptr;
    }
#endif
    

    
    compute_test_init_textures(core, renderer, compute_test);
    
    compute_test.buffer_size = align_up(sizeof(sd::playground::Input_Data), 256);

    sd::Buffer_make(renderer, &compute_test.buffer, compute_test.buffer_size);
    sd::playground::Input_Data* contents = (sd::playground::Input_Data*)sd::Buffer_contents(&compute_test.buffer);
    contents->texture_index = compute_test.texture_id;
    
    contents->time_seconds = 0;
    
    
    
    compute_test.blur = sd::Blur_make(renderer, 5.0f);
#endif
    init_render_targets(ctx, core, renderer, ((State*)ctx->state));
    
    
    //    sd::Image_Load_Descriptor desc;
    //
    //    sd::images_load_async(ctx->renderer, &desc, [](const sd::Image_Load_Result& result, bool status) -> bool {
    //        return true;
    //    });
    return nullptr;
}

void* on_unload(void* args)
{
    auto* ctx = (mtt::DL_Ctx*)args;
    MTT_Core* core = ctx->core;
#if COMPUTE_TEST
    sd::Renderer* renderer = core->renderer;
    Compute_Shader_Test& compute_test = ((State*)ctx->state)->compute_shader_test;
    
    sd::Buffer_destroy(renderer, &compute_test.buffer);
    sd::Blur_destroy(renderer, &compute_test.blur);
#endif
    return nullptr;
}

void* on_before_frame(void* args)
{
    
    auto* ctx = (mtt::DL_Ctx*)args;
    MTT_Core* core = ctx->core;
    
    DrawTalk_World* const world = static_cast<DrawTalk_World*>(core->user_data);
    mtt::World* mtt_world = &world->mtt_world;
    
    float64 t_seconds = core->time_sim_elapsed;
    uint64 t_nanos = core->time_sim_elapsed_ns;
    sd::Renderer* renderer = core->renderer;
    
    auto& vp = core->viewport;
    
    auto depths = sd::depth_range_default(renderer);


    
#if COMPUTE_TEST
    Compute_Shader_Test& compute_test = ((State*)ctx->state)->compute_shader_test;
    if (compute_test.shader_id != sd::Shader_ID_INVALID && compute_test.texture_id != sd::Texture_ID_INVALID) {
        sd::Command_List cmd_list = {};
        
        sd::Command_List_begin(renderer, &cmd_list);
        
        sd::Compute_Shader_Info info;
        sd::compute_shader_info(renderer, compute_test.shader_id, &info);
        uvec3 grid_size = uvec3(compute_test.texture_width, compute_test.texture_height, 1);
        usize total_threads = info.max_total_threads_per_threadgroup;
        if (total_threads > compute_test.texture_width * compute_test.texture_height) {
            total_threads = compute_test.texture_width * compute_test.texture_height;
        }
        usize w = info.thread_execution_width;
        usize h = total_threads / w;
        uvec3 threadgroup_size = uvec3(w, h, 1);
        sd::compute_pass_begin(renderer, {
            .type = sd::COMPUTE_DISPATCH_TYPE_SERIAL,
        });
        {
            sd::compute_shader_set(renderer, compute_test.shader_id);
            auto* in_data = (sd::playground::Input_Data*)sd::Buffer_contents(&compute_test.buffer);
            in_data->time_seconds = ((float32)t_seconds);
            sd::compute_buffer_set(renderer, &compute_test.buffer, 1, 0);
            sd::dispatch_threads(renderer, grid_size, threadgroup_size);
        }
#if USE_GRAYSCALE
        {
            sd::compute_shader_set(renderer, compute_test.grayscale_shader_id);
            auto* in_data = (sd::playground::Input_Data*)sd::Buffer_contents(&compute_test.buffer);
            in_data->time_seconds = ((float32)t_seconds);
            sd::compute_buffer_set(renderer, &compute_test.buffer, 1, 0);
            sd::dispatch_threads(renderer, grid_size, threadgroup_size);
        }
#endif


        sd::compute_pass_end(renderer, {
            .state = &compute_test,
            .callback = [](void* state) {
                Compute_Shader_Test* compute_test = (Compute_Shader_Test*)state;
                
            },

            .should_wait = false,
        });

        
        //sd::compute_set_blur(renderer, &compute_test.blur, &compute_test.texture_handle, false);
        //sd::compute_set_blur(renderer, 100.0f*m::sin01(t_seconds), &compute_test.texture_handle, false);
        
        sd::Command_List_end(renderer, &cmd_list);
        sd::Command_List_submit(renderer, &cmd_list);
    }
#endif
    
#if ATLAS_TEST
    {
        sd::Image_Atlas_clear(&core->image_atlas);
        auto scale = sd::get_native_display_scale();
        float32 scale_factor = scale;
        sd::Image_Atlas_init(renderer, &core->image_atlas, sd::max_texture_dimension_2(renderer), uvec2(core->viewport.width , core->viewport.height ));
        
        sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_TEST);
        sd::rewind_layer(renderer, LAYER_LABEL_DYNAMIC_TEST);
        
        sd::begin_drawable(renderer);
        sd::begin_polygon_no_new_drawable(renderer);
        
        State* state = ((State*)ctx->state);
        
        auto& reserved = state->reserved;
        reserved.clear();

        usize count = 10;
        
        vec4 colors[] = {color::RED * .5f, color::GREEN * .5f, color::BLUE * .5f};
        usize color_idx = 0;
        
        for (usize i = 0; i < count; i += 1) {
            reserved.emplace_back();
            if (sd::Image_Atlas_image_reserve_rectangle(&core->image_atlas, vec2(core->viewport.width * .15f, core->viewport.width * .15f + (m::sin01(t_seconds / ((1 + (1*color_idx)) + (i % 2))) * .15f * core->viewport.width)), &reserved.back())) {
//                MTT_print("i: %llu image count: %llu\n", i, sd::Image_Atlas_image_count(&core->image_atlas));
                color_idx = (color_idx + 1) % (sizeof(colors) / sizeof(vec4));
            }
//                    MTT_print("RESERVE %llu: %f %f %f %f\n", i, reserved.back().rectangle.x, reserved.back().rectangle.y, reserved.back().rectangle.width, reserved.back().rectangle.height);
            sd::set_color_rgba(renderer, colors[color_idx]);
            auto& info = reserved[i];
            sd::rectangle(renderer, vec2(info.rectangle.x, info.rectangle.y), vec2(info.rectangle.width, info.rectangle.height), 999.5f);
            sd::set_color_rgba(renderer, colors[(color_idx + 1) % (sizeof(colors) / sizeof(vec4))]);
            sd::circle(renderer, 25, vec3(vec2(info.rectangle.x, info.rectangle.y) + (vec2(info.rectangle.width, info.rectangle.height)*.5f), 999.0f));
        }

        sd::end_polygon_no_new_drawable(renderer);
        sd::end_drawable(renderer);
        
        sd::Render_Target* render_target = &state->render_target_atlas_test;
        render_target->clear_color = vec4(0,0,0,0);
        
        sd::Command_List cmd_list = {};
        sd::Command_List_begin(renderer, &cmd_list);
        
        
        auto modified_viewport = (sd::Viewport){.x = 0, .y = 0, .width = core->viewport.width * state->atlas_scale.x, .height = core->viewport.height * state->atlas_scale.y};
        sd::render_pass_begin(renderer, (sd::Render_Pass_Descriptor){.id = 2, .color_targets[0] = *render_target, modified_viewport });
        
        sd::push_projection_view_transform(renderer, m::ortho(0.0, modified_viewport.width, modified_viewport.height, 0.0, -1000.0, 1000.0), mat4(1.0f));
        
        sd::push_texture_pipeline(renderer);
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DYNAMIC_TEST);
        
        sd::render_pass_end(renderer);
        
        sd::Command_List_end(renderer, &cmd_list);
        
        sd::Command_List_submit(renderer, &cmd_list);;
    }
#endif
#if LIGHTS_TEST
    {
        State* state = ((State*)ctx->state);
        
        sd::set_color_rgba(renderer, SD_vec4(1.0f, 1.0f, 0.0f, 1.0f));

        //sd::push_pipeline(renderer, sd::PIPELINE_MODE_COLOR_NO_DEPTH_STENCIL);
        sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_LIGHTS);
        sd::rewind_layer(renderer, LAYER_LABEL_DYNAMIC_LIGHTS);
        sd::begin_drawable(renderer);
        sd::begin_polygon_no_new_drawable(renderer);
        float32 sn = m::sin(t_seconds);
        float32 sn01 = m::sin01(t_seconds);
        float32 off = sn01 * vp.width * 2;
        sd::circle(renderer, 256.0f, sd::vec3(vec2(vp.width + off, vp.height) * 0.15, depths[1]), 64);
        sd::circle(renderer, 256.0f + (512.0f * sn01), sd::vec3(vec2(vp.width, vp.height + off) * 0.5, depths[1]), 64);
        sd::circle(renderer, 256.0f, sd::vec3(vec2(vp.width - off, vp.height) * 0.85, depths[1]), 64);
        
        //sd::triangle_equilateral_per_vertex_color(renderer, vec3(vp.width / 2, vp.height / 2, depths[1] - 10), -1, 200, vec4(1,0,0,1), vec4(0,1,0,1), vec4(0,0,1,1));
        sd::end_polygon_no_new_drawable(renderer);
        sd::end_drawable(renderer);
    
        sd::Render_Target* lights_render_target = &state->render_target_lights;
        lights_render_target->clear_color = vec4(0,0,0,0);
        
        sd::Command_List cmd_list = {};
        
        sd::Command_List_begin(renderer, &cmd_list);
        
        sd::render_pass_begin(renderer, (sd::Render_Pass_Descriptor){.id = 1, .color_targets[0] = *lights_render_target, core->viewport });
        
        sd::push_projection_view_transform(renderer, world->main_projection, world->cam.view_transform);
        sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_DISABLED);
        sd::push_pipeline(renderer, sd::PIPELINE_MODE_TEXTURE);
        
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DYNAMIC_LIGHTS);
        
        sd::render_pass_end(renderer);
#if BLUR_LIGHTS
        sd::compute_set_blur(renderer, 10.0f*m::sin01(t_seconds), &lights_render_target->target, false);
#endif
        
        sd::Command_List_end(renderer, &cmd_list);
        
        sd::Command_List_submit(renderer, &cmd_list);
    }
#endif
    return nullptr;
}

void* on_frame(void* args)
{
    //return nullptr;
    auto* ctx = (mtt::DL_Ctx*)args;
    MTT_Core* core = ctx->core;
    
    DrawTalk_World* const world = static_cast<DrawTalk_World*>(core->user_data);
    mtt::World* mtt_world = &world->mtt_world;
    
    float64 t_seconds = core->time_sim_elapsed;
    uint64 t_nanos = core->time_sim_elapsed_ns;
    float64 t_delta = core->time_delta_seconds;
    sd::Renderer* renderer = core->renderer;
    //    auto& selections = *(dt::Selection_Recording_State_Queue*)dt::selections();
    //    MTT_print("selections:{\n");
    //    for (const auto& selection : selections) {
    //        MTT_print("\tthing id=[%llu]\n", selection.ID);
    //    }
    //    MTT_print("}\n");
    
    auto& vp = core->viewport;
    
    auto depths = sd::depth_range_default(renderer);

    
    sd::push_pipeline(renderer, sd::PIPELINE_MODE_TEXTURE);
    sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_CANVAS);
    sd::push_projection_view_transform(renderer, world->main_projection, Mat4(1.0f));
    //sd::set_color_rgba(renderer, SD_vec4(SD_vec3(1.0f) * (float32)m::sin01(t_seconds), 1.0f));
    
    
//    {
//        dt::DrawTalk* dt = dt::ctx();
//        dt::UI& ui = dt->ui;
//        
//        dt::Panel& panel = ui.margin_panels[0];
//        dt::Text_Panel& text_panel = panel.text;
//        
//        vec2 scroll_top = panel.bounds.tl + vec2(text_panel.offset.x * 0.25, text_panel.offset.y);
//        vec2 dim = vec2(text_panel.offset.x * 0.25, panel.bounds.dimensions.y - text_panel.offset.y * 2);
//        
//        sd::set_color_rgba(renderer, SD_vec4(SD_vec3(0.0f, 0.0f, 0.0f), 0.1f));
//        sd::begin_drawable(renderer);
//        sd::begin_polygon_no_new_drawable(renderer);
//        sd::rectangle(renderer, scroll_top, dim, depths[1]);
//        sd::end_polygon_no_new_drawable(renderer);
//        sd::end_drawable(renderer);
//    }
    sd::set_color_rgba(renderer, SD_vec4(SD_vec3(1.0f), 1.0f));
    sd::begin_drawable(renderer);
    sd::begin_polygon_no_new_drawable(renderer);
    
    dt::DrawTalk* dt = dt::ctx();
    dt::UI& ui = dt->ui;
    
    dt::Panel& ndom_panel = ui.margin_panels[0];
    
    float32 height_offset = ndom_panel.bounds.dimensions.y;
    
    
#if COMPUTE_TEST
    {
        Compute_Shader_Test& compute_test = ((State*)ctx->state)->compute_shader_test;
        
        if (compute_test.texture_id != sd::Texture_ID_INVALID && compute_test.shader_id != sd::Shader_ID_INVALID) {
            sd::rectangle(renderer, SD_vec2(0.0f, height_offset), SD_vec2(core->viewport.width, core->viewport.height - height_offset), depths[1]-1);
        } else {
            sd::set_color_rgba(renderer, SD_vec4(SD_vec3(0.5f, 0.0f, 0.0f), 1.0f));
            sd::rectangle(renderer, SD_vec2(0.0f, height_offset), SD_vec2(core->viewport.width, core->viewport.height - height_offset), depths[1]-1);
        }
    }
#else
//    sd::set_color_rgba(renderer, SD_vec4(SD_vec3(0.5f, 1.0f * m::sin01((float32)t_seconds), 0.0f), 1.0f));
//    sd::rectangle(renderer, SD_vec2(0.0f, height_offset), SD_vec2(core->viewport.width * .5, (core->viewport.height - height_offset) * .5 * m::cos01((float32)t_seconds * 0.0f)), nextafter(depths[0], depths[1]));
#endif
    
    
    //
    sd::set_color_rgba(renderer, SD_vec4(1.0f, 0.0f, 0.0f, 1.0f));
    //sd::rectangle(renderer, SD_vec2(vp.width, vp.height) * 0.25f, SD_vec2(vp.width, vp.height) * 0.25f, 999.9f);
    
  
    
//     mtt::Thing* t = mtt::get_thing_ptr_most_recently_selected_with_touch(mtt_world);
//     if (t != nullptr) {
//     mtt::Thing_set_position(t, vec3(vp.width * 0.5f, vp.height * 0.5f, 999.0f));
//     } else {
//     }
//
//
//
//     float32 off = m::sin01(t_seconds) * vp.height;
//     float32 off2 = m::sin01(t_seconds * 8.0f) * vp.height;
//     std::vector<sd::Boolean_Contour_Element> els = {
//     {
//     .is_closed = true,
//     .clip_operation_type = sd::CLIP_OPERATION_TYPE_NONE,
//     .clip_polygon_type = sd::CLIP_POLYGON_TYPE_SUBJECT,
//     .paths = {
//     {
//     SD_vec2(vp.width + off2, vp.height) * 0.25f,
//     SD_vec2(vp.width + off2, vp.height) * SD_vec2(0.25f, 0.75f),
//     SD_vec2(vp.width + off2, vp.height) * SD_vec2(0.75f, 0.75f),
//     SD_vec2(vp.width + off2, vp.height) * SD_vec2(0.75f, 0.25f)
//     }
//     }
//     },
//     {
//     .is_closed = true,
//     .clip_operation_type = sd::CLIP_OPERATION_TYPE_DIFFERENCE,
//     .clip_polygon_type = sd::CLIP_POLYGON_TYPE_CLIP,
//     .paths = {
//     {
//     SD_vec2(0.0f, off2) + SD_vec2(0.5f*vp.width, 400.0f),
//     SD_vec2(0.0f, off2) + SD_vec2(0.5f*vp.width, 600.0f),
//     SD_vec2(0.0f, off2) + SD_vec2(vp.width + 100.0f, 400.0f)
//     }
//     }
//     },
//     {
//     .is_closed = true,
//     .clip_operation_type = sd::CLIP_OPERATION_TYPE_DIFFERENCE,
//     .clip_polygon_type = sd::CLIP_POLYGON_TYPE_CLIP,
//     .paths = {
//     {
//     SD_vec2(-off, 0.0f) + SD_vec2(vp.width, 400.0f),
//     SD_vec2(-off, 0.0f) + SD_vec2(vp.width, 600.0f),
//     SD_vec2(-off, 0.0f) + SD_vec2(vp.width + 100.0f, 400.0f)
//     }
//     }
//     }
//     };
//     sd::contour(renderer, els);
    
    
    sd::end_polygon_no_new_drawable(renderer);
    auto* drawable = sd::end_drawable(renderer);
    sd::Viewport vp_adjusted = vp;
    {
        vp_adjusted.height -= height_offset;
        vp_adjusted.x = 0;
        vp_adjusted.y = height_offset;
    }
    vec2 t_scale = {ctx->vars["scale"].Vector2};
    for (usize32 i = 0; i < 2; i += 1) {
        if (t_scale[i] == 0.0f) {
            t_scale[i] = 1.0f;
        }
    }
    
    drawable->set_transform(m::translate(SD_mat4(1.0f), vec3(vp_adjusted.width*0.5, vp_adjusted.height*0.75, 0.0)) * m::rotate(SD_mat4(1.0f), ctx->vars["rspeed"].Float * (float32)t_seconds, SD_vec3(0.0f, 0.0f, 1.0f)) * m::scale(mat4(1.0f), vec3(t_scale.x, t_scale.y, 1.0f)) * m::translate(SD_mat4(1.0f), -vec3(vp_adjusted.width*0.5, vp_adjusted.height*0.75, 0.0)));
    drawable->is_enabled = true;
//    sd::save(renderer);
//    sd::begin_path(renderer);
//    {
//        sd::path_radius(renderer, 50.0f);
//
//        float32 y_inc = 0.0f;
//        float32 radius = 50.0f;
//
//        vec3 pos = {0.0f, 0.0f, depths[1]};
//        pos += vec3(100.0f, 600.0f, 0.0f);
//        usize bound = 500;
//        float32 radius_inc = 20.0f / bound;
//        float32 r_inc_factor = 1;
//        float32 x_inc = (vp_adjusted.width * 0.75) / bound;
//        for (usize i = 0; i < bound; i += 1) {
//            float32 H = ((float32)i / bound) * 359;
//
//            vec4 rgb = HSVtoRGBinplace(H, 1.0, 1.0, 1.0);
//            sd::set_color_rgba_v4(renderer, {rgb[0], rgb[1], rgb[2], 1.0f});
//            pos += vec3(x_inc, y_inc, 0.0f);
//            sd::path_radius(renderer, radius);
//            sd::path_vertex_v3(renderer, pos);
//            radius += radius_inc;
//            r_inc_factor += 1;
//            radius_inc = (20.0f / bound) * r_inc_factor / 40;
//        }
//        sd::path_radius(renderer, 100.0f);
//        sd::path_vertex_v3(renderer, pos + vec3(0.0f, 200.0f, 0.0f));
//    }
//    sd::end_path(renderer);
//    sd::restore(renderer);
#if COMPUTE_TEST
    {
        Compute_Shader_Test& compute_test = ((State*)ctx->state)->compute_shader_test;
        if (compute_test.texture_id != sd::Texture_ID_INVALID && compute_test.shader_id != sd::Shader_ID_INVALID) {
            drawable->set_texture_ID(compute_test.texture_id);
            drawable->set_texture_sampler_ID(sd::Texture_Sampler_ID_CLAMP_TO_EDGE_LINEAR);
        }
    }
#endif

#if (LIGHTS_TEST || ATLAS_TEST)
    sd::rewind_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD_2);
    sd::rewind_layer(renderer, LAYER_LABEL_PER_FRAME_SCREEN_2);

    //sd::save(renderer);
    sd::push_texture_pipeline(renderer);
    
    
    
    
    

    sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_ENABLED);
    
    
    //sd::set_color_rgba_v4(renderer, vec4(1.0f, 0.0f, 0.0f, 1.0f));
    //sd::circle(renderer, 256.0f, sd::vec3(vp.width / 2.0, vp.height / 2, depths[1]));

    {
        auto& scissor_bounds = dt::ctx()->ui.top_panel.base.bounds;
        sd::push_scissor_rect(renderer, (sd::Scissor_Rect) {
            .x = (uint64)0,
            .y = (uint64)(scissor_bounds.dimensions.y),
            .width = (uint64)core->viewport.width,
            .height = (uint64)(core->viewport.height - scissor_bounds.dimensions.y),
        });
    }
    
    
    sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_ENABLED);
    
    sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD_2);
    sd::push_projection_view_transform(renderer, world->main_projection, world->cam.view_transform);
//    if (false) {
//        sd::begin_polygon(renderer);
//        sd::set_color_rgba_v4(renderer, vec4(0.0f, 0.0f, 1.0f, 1.0f));
//        sd::rectangle(renderer, vec2(0.0f), vec2(vp.width, vp.height) * .5, depths[1]);
//        State* state = ((State*)ctx->state);
//
//        drawable = sd::end_polygon(renderer);
//
//        drawable->is_enabled = true;
//    }
    
    if (true) {
        sd::begin_polygon(renderer);
        sd::set_color_rgba_v4(renderer, vec4(0.0f, 0.0f, 1.0f, 1.0f));
        sd::rectangle(renderer, vec2(0.0f), vec2(vp.width * 2, vp.height * 2), 0.0);
        State* state = ((State*)ctx->state);
        sd::Texture_Handle* lights_texture_id = &state->render_target_lights.target;
        drawable = sd::end_polygon(renderer);
        //drawable->set_texture_ID(lights_texture_id->id);
        drawable->set_texture_mult_ID(lights_texture_id->id);
        //drawable->set_texture_add_ID(sd::TEXTURE_ID_BUILTIN_SPECTRUM);
        drawable->set_transform(m::translate(mat4(1.0f), vec3(vec2(vp.width, vp.height)*vec2((float32)m::sin(t_seconds), (float32)m::cos(t_seconds)),-1)));
        drawable->is_enabled = true;
    }
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD_2);
    
    if (true) {
        sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_SCREEN_2);
        sd::push_projection_view_transform(renderer, world->main_projection, Mat4(1.0f));
        sd::begin_polygon(renderer);
        sd::set_color_rgba_v4(renderer, vec4(1.0f, 1.0f, 0.0f, 0.4f));
        sd::rectangle(renderer, vec2(0.0f), vec2(vp.width, vp.height), 0);
        
        State* state = ((State*)ctx->state);
        sd::Texture_Handle* lights_texture_id = &state->render_target_lights.target;
        drawable = sd::end_polygon(renderer);
        drawable->set_texture_ID(lights_texture_id->id);
        //drawable->set_texture_mult_ID(lights_texture_id->id);
        drawable->is_enabled = true;
        
        
#if ATLAS_TEST
        {
            State* state = ((State*)ctx->state);
//            sd::begin_polygon(renderer);
//            sd::set_color_rgba_v4(renderer, vec4(1.0f, 1.0f, 1.0f, 0.2f));
//            sd::rectangle(renderer, vec2(0.0f), vec2(vp.width, vp.height), 999.8f);
//            drawable = sd::end_polygon(renderer);
            sd::Texture_Handle* texture_id = &state->render_target_atlas_test.target;
//            drawable->set_texture_ID(texture_id->id);
//            drawable->is_enabled = true;
            
            sd::set_color_rgba_v4(renderer, vec4(0.0f, 0.0f, 0.0f, 1.0f));
            for (usize i = 0; i < state->reserved.size(); i += 1) {
                auto& info = state->reserved[i];
                sd::begin_polygon(renderer);
                sd::rectangle(renderer, vec2(info.rectangle.x, info.rectangle.y), vec2(info.rectangle.width, info.rectangle.height), 0);
                auto* d_info = sd::end_polygon(renderer);
                d_info->set_texture_ID(texture_id->id);
                d_info->set_texture_mult_ID(lights_texture_id->id);
                
                d_info->set_texture_coordinates_modifiers(sd::Texture_compute_coordinate_modifiers(vec2(state->render_target_atlas_test.size / sd::get_native_display_scale()), vec2(info.rectangle.x, info.rectangle.y), vec2(info.rectangle.width, info.rectangle.height)));
                d_info->is_enabled = true;
            }
        }
#endif
    }
    

    
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_PER_FRAME_SCREEN_2);

    
    sd::push_texture_pipeline(renderer);
    
    //sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_ENABLED);
    
    {
        sd::push_scissor_rect(renderer, sd::Viewport_to_Scissor_Rectangle(core->viewport));
    }


    //sd::restore(renderer);
#endif
    
    sd::push_projection_view_transform(renderer, world->main_projection, Mat4(1.0f));
    
    return nullptr;
}

void* on_after_frame(void* args)
{
    return nullptr;
}

void* on_resize(void* args)
{
    mtt::DL_Ctx* ctx = (mtt::DL_Ctx*)args;
    vec2 new_size = *(vec2*)ctx->params;
    
    MTT_Core* core = ctx->core;
    
    
    float64 t_seconds = core->time_sim_elapsed;
    uint64 t_nanos = core->time_sim_elapsed_ns;
    float64 t_delta = core->time_delta_seconds;
    sd::Renderer* renderer = core->renderer;
    
#if COMPUTE_TEST
    
    if (ctx->state == nullptr) {
        return  nullptr;
    }
    
    Compute_Shader_Test& compute_test = ((State*)ctx->state)->compute_shader_test;
    compute_test_init_textures(core, renderer, compute_test);
    
#endif

    init_render_targets(ctx, core, renderer, ((State*)ctx->state));
    
    return nullptr;
}

void* on_load(void* args)
{
    
    mtt::DL_Ctx* ctx = (mtt::DL_Ctx*)args;
    
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    
    MTT_Dynamic_Library* dylib = (MTT_Dynamic_Library*)(&ctx->core->library);
    
    
    struct Init_Data { cstring name; PROC_MTT_Dynamic_Library proc; };
    Init_Data init_data[] = {
        {"on_load", on_load},
        {"renderer_init", renderer_init},
        {"on_frame", on_frame},
        {"on_before_frame", on_before_frame},
        {"on_after_frame", on_after_frame},
        {"on_unload", on_unload},
        {"on_resize", on_resize},
    };
    for (usize i = 0; i < sizeof(init_data) / sizeof(Init_Data); i += 1) {
        MTT_Dynamic_Library_register_proc(dylib, init_data[i].name, init_data[i].proc);
    }
    
    mtt::MTT_API_set(&ctx->mtt_api);
    sd::Renderer_API_set(&ctx->sd_api);
    
#endif
    
    if (ctx->state == nullptr) {
        ctx->state = mem::alloc_init<State>(&ctx->core->allocator);
    }
    
    MTT_print("%s\n", "loaded");

    return nullptr;
}



}

//
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP

#ifdef __cplusplus
extern "C" {
#endif

#else

namespace ext {

#endif
//

    MTT_DYNAMIC_EXPORT void* VERSION(setup)(void* args)
    {        
        ext::on_load(args);
        
        MTT_print("In %s version: %d\n", __PRETTY_FUNCTION__, VERSION_NUMBER);

        
        
        ext::renderer_init(args);
        
        mtt::DL_Ctx* ctx = (mtt::DL_Ctx*)args;
        
        
        
        
        return nullptr;
    }
  
//
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP

#ifdef __cplusplus
}
#endif

#else
}
#endif
//
