//
//  make_the_thing.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/11/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

constexpr const static bool ROTATE_TEST_ON = false;
constexpr const static bool LOOK_AT_TEST_ON = false;
#define NUMBER_DATA_FLOW_TEST (0)
#define TANG_INFO_TEST (0)

#include "make_the_thing.h"

#include "image.hpp"
#include "file_system.hpp"
#include "scripting.hpp"
#include "clipboard.hpp"

#include "video_recording.h"

#define SD_IMPLEMENTATION
#include "drawtalk_world.hpp"


#include "nanovg_test_drawing.hpp"
#define DO_DEBUG_DRAW (1)

#define OLD_STAGING_LAYER_ON_DRAW (false)

#include "input.hpp"
#include "erase_tool.hpp"

#include "MicroDollar.h"

//#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
//#include "LibQuickJS/LibQuickJS.h"
//#include "LibQuickJS/quickjs-libc.h"
//
//int QJS_main(int argc, char **argv);
//#endif



#include "dynamic_library_shared.hpp"
#include "misc.hpp"

#include "regular_expr.hpp"

#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
#include "dynamic_main.hpp"
#endif

#include "user_testing.h"

//#define USE_DEBUG_GRID_BG

bool new_points_arrived = false;
std::vector<std::vector<vec3>> points;



struct WEE {
    int x;
    MTT_ALIGN(16) float bla;
};


constexpr const bool USE_OLD_ADD_TO_DRAWING_METHOD = false;

WEE wee;

namespace mtt {

bool enable_video = false;

constexpr const bool TEST_PARTICLES_AND_NODE_GRAPH = false;
constexpr const bool enable_particles_in_test = false;
constexpr const bool enable_old_way_of_overriding_text_panel = false;

struct Test_Particles {
    sd::Drawable_Info* particle_mesh;
    mtt::Thing* particle_system;
    mtt::Thing* node_graph;
    mtt::Thing* node_graph_copy;
} test_particles = {};

sd::Drawable_Info* grid = nullptr;
vec3 grid_translation = vec3(0.0f);
isize grid_construction_width;
isize grid_construction_height;

// MARK: - Setup

struct Font_Settings {
    float lineh;
} font_settings;

struct Transform_Hierarchy_Record {
    mat4 parent_transform;
    mtt::Thing* thing;
    mtt::Rep* parent_rep;
};
std::vector<Transform_Hierarchy_Record> transform_stack;

//struct World_Visualization {
//    bool is_on = false;
//    bool has_started = false;
//    float64 t_start = 0.0;
//    float64 interval = 0.5;
//} world_vis;









std::vector<Collider*> debug_colliders;

mtt::Thing* __testthing__ = nullptr;
mtt::Thing* tanginfo__ = nullptr;
mtt::Array<mtt::Thing*, 7> nodes = {};
mtt::Array<mtt::Thing_ID, 7> nodes_as_ids = {};

struct Hierarchy_Test {
    mtt::Thing* root;
    mtt::Thing* l;
    mtt::Thing* r;
    mtt::Thing* ll;
    mtt::Thing* lr;
    mtt::Thing* rl;
    mtt::Thing* rr;
} htest = {};


struct Arrows {
    LAYER_LABEL render_layer = LAYER_LABEL_PER_FRAME_WORLD;
    using Curve = std::vector<vec3>;
    Curve curve;
    bool is_active = false;
    bool arrow_enabled = true;
    vec4 color = vec4(1.0f);
    usize radius = 1.0f;
    uintptr input_key = 0;
    
    vec2 canvas_pos = vec2(0.0f);
    
    struct Deferred_Op {
        mtt::Thing_ID first = mtt::Thing_ID_INVALID;
        mtt::Thing_ID second = mtt::Thing_ID_INVALID;
    };
    
    std::vector<Deferred_Op> deferred;
    
    mtt::Thing_ID prev_in_chain = mtt::Thing_ID_INVALID;
    
    void draw_arrows(sd::Renderer* r, usize scale)
    {
        //        if (curve.size() >= 4) {
        //            vec3 prev = vec3(0.0f);
        //
        //            sd::set_render_layer(r, this->render_layer);
        //
        //            auto old_color = sd::get_color_rgba(r);
        //            auto old_radius = sd::get_path_pixel_radius(r);
        //            sd::set_color_rgba_v4(r, color);
        //            sd::path_pixel_radius(r, m::max(4.0f, (float32)(radius * scale)));
        //            sd::begin_path(r);
        //
        //            static const usize split_count = 15;
        //            sd::path_vertex_v3(r, curve[0]);
        //            for (usize i = 1; i < split_count; i += 1) {
        //                vec3 point = m::catmullRom(curve[0], curve[1], curve[2], curve[3], (float32)i / split_count);
        //                sd::path_vertex_v3(r, point);
        //                prev = point;
        //            }
        //            sd::path_vertex_v3(r, curve[curve.size() - 1]);
        //            sd::break_path(r);
        //            sd::path_arrow_head(r, prev, curve[curve.size() - 1], m::max(12.0f, 12.0f * scale));
        //            sd::end_path(r);
        //
        //
        
        //        }
        
        CatmullRom spline;
        spline.init(this->curve, 1, false);
        
        sd::set_render_layer(r, this->render_layer);
        
        auto old_color = sd::get_color_rgba(r);
        auto old_radius = sd::get_path_radius(r);
        sd::set_color_rgba_v4(r, this->color);
        sd::path_radius(r, m::max(4.0f, 4 * (float32)(this->radius * scale)));
        sd::begin_path(r);
        if (this->arrow_enabled) {
            spline.DrawSplineWithArrow(r, 12.0f, scale);
        } else {
            spline.DrawSpline(r);
        }
        sd::end_path(r);
        
        sd::set_color_rgba_v4(r, old_color);
        sd::set_path_radius(r, old_radius);
    }
    
    void clear(void)
    {
        this->curve.clear();
        this->prev_in_chain = mtt::Thing_ID_INVALID;
    }
    void push(vec3 point)
    {
        this->curve.push_back(point);
    }
    
    ARROW_LINK_FLAGS arrow_type = ARROW_LINK_FLAGS_DIRECTED;
    
    ARROW_LINK_FLAGS arrow_flags_visible_with_type()
    {
        ARROW_LINK_FLAGS result_flags = (ARROW_LINK_FLAGS)(ARROW_LINK_FLAGS_VISIBLE | arrow_type);
        return result_flags;
    }
    
} thing_arrows;


struct Instanced_Indicators {
    struct Indicators_Render_Info {
        //LAYER_LABEL layer = LAYER_LABEL_STATIC_INSTANCING_CANVAS;
        sd::Drawable_Info* drawable_source = nullptr;
        std::vector<sd::Drawable_Info> instances = {};
        
        mtt::Set_Stable<mtt::Thing_ID> things = {};
        vec2 corner_multiplier = vec2(0.0f);
    };
    Indicators_Render_Info render_info_list[1];
    usize count = 1;
} instanced_indicators;




void Application_Configuration_reapply(Application_Configuration* config, void* core_ptr)
{
    MTT_Core* core = (MTT_Core*)core_ptr;
    
    for (auto& panel : dt::DrawTalk::ctx()->ui.margin_panels) {
        panel.text.color = config->text_panel_color;
    }
    
    
    mtt::set_color_scheme((mtt::Color_Scheme_ID)config->color_scheme);
    sd::set_background_color_rgba_v4(core->renderer, dt::bg_color_default());
    
    {
        auto* dt_ctx = dt::DrawTalk::ctx();
        auto* world = dt_ctx->mtt;
        auto& dock = dt_ctx->ui.dock;
        auto* button  = dock.label_to_button["color"];
        auto& label_to_config = dock.label_to_config;
        auto* conf = &label_to_config[button->label];
        mtt::Thing* thing = world->Thing_get(button->thing);
        mtt::Rep* rep = mtt::rep(thing);
        
        switch (mtt::color_scheme()) {
            default: {
                MTT_FALLTHROUGH;
            }
            case mtt::COLOR_SCHEME_LIGHT_MODE: {
                rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_DARK_MODE]->is_enabled = false;
                rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_LIGHT_MODE]->is_enabled = true;
                break;
            }
            case mtt::COLOR_SCHEME_DARK_MODE: {
                rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_DARK_MODE]->is_enabled = true;
                rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_LIGHT_MODE]->is_enabled = false;
                break;
            }
        }
    }
    
    MTT_follow_system_window(config->follow_system_window);
}

void init_with_config(bool* status_out = nullptr, const mtt::String& path = "config/init");
void init_with_config(bool* status_out, const mtt::String& path)
{
    mtt::Text_File_Load_Descriptor desc = {};
    desc.paths.push_back({});
    desc.paths.back().path = path;
    desc.paths.back().extension = "json";
    mtt::Text_File_load_async(&desc, [](mtt::Text_File_Load_Result result, bool status) -> bool {
        if (status == false) {
            return false;
        }
        
        auto* mtt_core = mtt_core_ctx();
        mtt::Application_Configuration_init(&mtt_core->application_configuration, (void*)&result);
        
        mtt::Application_Configuration_reapply(&mtt_core->application_configuration, (void*)mtt_core);
        
        
        return true;
    });
}

#define STRESS_TEST (true)



auto init_dynamic_load = [](MTT_Core* core, mtt::String& version_number) {
    sd::Renderer* renderer = core->renderer;
    // dynamic loading for development
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
#ifdef NDEBUG
    return;
#endif
    
    struct CTX {
        char path[4096];
        usize length;
    } ctx;
    mtt::File_System_get_main_directory_path_cstring( [](cstring str, usize length, void* ctx) {
        strcpy(((CTX*)ctx)->path, str);
        ((CTX*)ctx)->length = length;
    }, &ctx);
    
    
#define USE_SUFFIX (1)
    
    //    if (core->library.version_number > 0) {
    //        const std::string cmd_call = "mv \"" + core->library.name + "\" " + "\"" + cpp_str + "\"";
    //        MTT_print("system call:[%s]\n", cmd_call.c_str());
    //        system(cmd_call.c_str());
    //        system(mtt::String("install_name_tool -id \"" + cpp_str + " " + cpp_str + "\"").c_str());
    //        system(mtt::String("chmod +x \"" + cpp_str + "\"").c_str());
    //    }
    
    //    core->send_message_drawtalk_server(core, "dyld", "{ \"version\": " + std::to_string(core->library.version_number) + ", \"path\": \"" + cpp_str_original + "\"}");
    
    
#if USE_SUFFIX
    const mtt::String lib = "libdynamic_dev" + version_number + ".dylib";
#else
    mtt::String lib = "libdynamic_dev.dylib";
#endif
    
    usize retry_count = 5;
    bool succeeded = false;
    static const mtt::String USER_str = getenv("USER");
    do {
        if (MTT_Dynamic_Library_init(&core->library, lib) || MTT_Dynamic_Library_init(&core->library, "/Users/" + USER_str + "/" + core->application_configuration.system_directory +"/Make-the-Thing/apple_platforms/Make The Thing/build/Debug/" + lib)) {
            core->library.name = lib;
            core->library.version_number = version_number;
            MTT_log_info("loaded: %s version=[%s]\n", lib.c_str(), core->library.version_number.c_str());
            
            mtt::String sym = "setup_" + version_number;
            
            PROC_MTT_Dynamic_Library dy_setup_proc;
            if (MTT_Dynamic_Library_load_and_register_proc(&core->library,
                                                           sym,
                                                           &dy_setup_proc)) {
                core->library(sym,
                              &core->dl_ctx);
                retry_count = 0;
                succeeded = true;
            } else if (MTT_Dynamic_Library_load_and_register_proc(&core->library,
                                                                  "setup",
                                                                  &dy_setup_proc)) {
                core->library("setup",
                              &core->dl_ctx);
                retry_count = 0;
                succeeded = true;
            } else {
                retry_count -= 1;
            }
            
        } else {
            retry_count -= 1;
        }
    } while (retry_count > 0);
    if (!succeeded) {
        send_message_drawtalk_server_quick_command(core, "dyld", "");
    }
#endif
};

vec2 color_current_tl;
vec2 color_current_dim;
vec2 color_current_br;

const float32 height_off = 30.0f;
const float32 color_current_height_off = height_off;

bool color_changed_this_frame = false;
void setup(MTT_Core* core)
{
    if constexpr (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP || MTT_PLATFORM == MTT_PLATFORM_TYPE_IOS) {
        if constexpr (MTT_PLATFORM == MTT_PLATFORM_TYPE_IOS) {
            MTT_print("%s\n", "PLATFORM_IOS");
        } else if constexpr (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP) {
            MTT_print("%s\n", "PLATFORM_MACOS_DESKTOP");
            
            core->input.users[0].show_cursor = false;
        }
        
        mtt::init(&core->history, nullptr, &core->allocator, 256);
        
        text_in_buf = "";
        
        //ecs_log_set_level(2);
        ecs_log_enable_colors(false);
        
        //        if constexpr ((false)) {
        //            // test 1: default heap allocator
        //            mem::Allocator alloc = {};
        //            alloc.data = NULL;
        //            alloc.allocate = [](void* alloc, unsigned long long byte_count) -> void* {
        //                return aligned_alloc(16, align_up(byte_count, 16));
        //            };
        //            alloc.deallocate = [](void* alloc, void* mem, unsigned long long count) -> void {
        //
        //                free(mem);
        //            };
        //            {
        //                // allocate, use, and deallocate a std::vector
        //                void* mem = alloc.allocate((void*)&alloc, sizeof(std::vector<unsigned long long>));
        //
        //                std::vector<unsigned long long>* my_vector = new (mem) std::vector<unsigned long long>();
        //                for (unsigned long long i = 0; i < 1025; i += 1) {
        //                    my_vector->push_back(i);
        //                }
        //                ((std::vector<unsigned long long>*)mem)->~vector<unsigned long long>();
        //                free(mem);
        //            }
        //
        //            // test 2 dynamic arena (fixed-size) allocator
        //
        //            {
        //                mem::Memory_Pool_Fixed pool = {};
        //                mem::Memory_Pool_Fixed_init(&pool, alloc, 5, sizeof(std::vector<unsigned long long>), 16);
        //                mem::Allocator pool_allocator = mem::Memory_Pool_Fixed_Allocator(&pool);
        //
        //                void* mem = pool_allocator.allocate((void*)&pool_allocator, sizeof(std::vector<unsigned long long>));
        //
        //                std::vector<unsigned long long>* my_vector = new (mem) std::vector<unsigned long long>();
        //                for (unsigned long long i = 0; i < 1025; i += 1) {
        //                    my_vector->push_back(i);
        //                }
        //
        //                my_vector->~vector<unsigned long long>();
        //                pool_allocator.deallocate((void*)&pool_allocator, mem, 1);
        //
        //            }
        //
        //            // test 3 using allocate/deallocate wrappers
        //            {
        //                mem::Memory_Pool_Fixed pool = {};
        //                mem::Memory_Pool_Fixed_init(&pool, alloc, 5, sizeof(std::vector<unsigned long long>), 16);
        //                mem::Allocator pool_allocator = mem::Memory_Pool_Fixed_Allocator(&pool);
        //
        //                std::vector<unsigned long long>* my_vector = mem::allocate<std::vector<unsigned long long>>(&pool_allocator);
        //
        //                for (unsigned long long i = 0; i < 1025; i += 1) {
        //                    my_vector->push_back(i);
        //                }
        //
        //                mem::deallocate(&pool_allocator, my_vector);
        //            }
        //
        //
        //
        //
        //
        //        }
        
        core->user_data = static_cast<void*>(mem::alloc_init<DrawTalk_World>(&core->allocator));
        
        DrawTalk_World* const world = static_cast<DrawTalk_World*>(core->user_data);
        mtt::Camera_init(&world->cam);
        cam_stack_init(world, &world->cam);
        
        //world->input_info.next_sequence_number     = 0;
        //world->input_info.sequence.sequence_offset = 0;
        
        // file tests
        MTT_Date_Time date_time_start;
        mtt_date_time_current(&date_time_start);
        MTT_log_info("Date-Time: year=[%llu] month=[%llu] day=[%llu] hour=[%llu] minute=[%llu] second=[%llu]\n",
                     date_time_start.year,
                     date_time_start.month,
                     date_time_start.day,
                     date_time_start.hour,
                     date_time_start.minute,
                     date_time_start.second);
        
        mtt::File_System_add_path("scripts");
        mtt::File_System_add_path("testing");
        
        
        mtt::String PATH_;
        mtt::File_System_get_main_directory_path_cstring([](cstring str, usize length, void* ctx) {
            *((mtt::String*)ctx) = str;
        }, &PATH_);
        MTT_user_test_log_init((PATH_ + "/testing/" +
                                std::to_string(date_time_start.year) + "_yr_" +
                                std::to_string(date_time_start.month) + "_mon_" +
                                std::to_string(date_time_start.day) + "_day_" +
                                std::to_string(date_time_start.hour) + "_hour_" +
                                std::to_string(date_time_start.minute) + "_min_" +
                                std::to_string(date_time_start.second) + "_sec_" + ".json"));
        

        
        
        if constexpr ((false)) {
            cstring test_data = "Hi, how are you today?\n\n";
            
            mtt::File_System_create_file("testing/test.txt", (mtt::File_Args){
                .data = (void*)test_data,
                .byte_size = strlen(test_data)
            });
            mtt::Text_File_Write_Descriptor desc;
            desc.path.path = "scripts/json_bla";
            desc.path.extension = "json";
            desc.to_write = R"(
                              {
                "value" : "This is a testtttt\n"
            })";
            desc.flags = mtt::WRITE_FLAG_APPEND;
            mtt::Text_File_write(&desc, [](bool) {} );
            
            mtt::File_System_delete_file("testing/test.txt");
            mtt::File_System_remove_path("testing");
            
            
            {
                {
                    cstring file_extension = "json";
                    std::vector<std::string> paths;
                    mtt::File_System_find_file_paths_with_extension("scripts", &file_extension, 1, paths);
                    
                    mtt::Text_File_Load_Descriptor desc_text;
                    for (usize i = 0; i < paths.size(); i += 1) {
                        desc_text.paths.push_back((mtt::Path_Info){paths[i], "json"});
                    }
                    if (mtt::Text_File_load_async(&desc_text, [](mtt::Text_File_Load_Result result, bool status) -> bool {
                        if (status == false) {
                            
                        } else {
                            
                            for (usize i = 0; i < result.text.size(); i += 1) {
                                MTT_print("%s\n", result.text[i].c_str());
                                
                            }
                            
                        }
                        return true;
                    })) {
                        
                    } else {
                        
                    }
                }
                
                std::vector<RAW_SCRIPT*> scripts;
                mtt::from_JSON_files(&world->mtt_world, scripts);
            }
            
        }
        
        // MARK: Initialize controls
        
        dt::Control_System_init(&world->controls,
                                "pen+touch",
                                &world->mtt_world,
                                (void*)core);
        
        
        // MARK: Initialize collision detection system
        
        Collision_System_init(&world->mtt_world.collision_system, 0, core->allocator, &world->mtt_world);
        Collision_System_layer_make(&world->mtt_world.collision_system, 1 << 31);
        world->mtt_world.collision_system.collision_handler_AABB_default = mtt::collision_handler_AABB_default;
        world->mtt_world.collision_system.collision_handler_Circle_default = mtt::collision_handler_Circle_default;
        world->mtt_world.collision_system.label = "world";
        
        Collision_System_init(&world->mtt_world.collision_system_canvas, 0, core->allocator, &world->mtt_world);
        Collision_System_layer_make(&world->mtt_world.collision_system_canvas, 1 << 31);
        world->mtt_world.collision_system.collision_handler_AABB_default = mtt::collision_handler_AABB_default;
        world->mtt_world.collision_system_canvas.label = "canvas";
        
        // MARK: Initialize renderer
        sd::Renderer* const renderer = core->renderer;
        {
            world->render_info.render_layers_count = 0;
            for (usize i = 0; i < LAYER_LABEL_COUNT; i += 1) {
                world->render_info.render_layers[world->render_info.render_layers_count] = sd::Render_Layer_make(renderer, layer_label_sizes[i]);
                world->render_info.render_layers_count += 1;
            }
            
            
            
            //                MTT_List_init_with_allocator(&this->list, &this->list_pool_alloc.allocator);
        }
    
         
//#define STAGING_LAYER_TEST
#ifdef STAGING_LAYER_TEST
        sd::Renderer_Staging_Layer* stage0 = &world->render_info.grid_layer;
        stage0->layer = (sd::Render_Layer*)core->allocator.allocate(&core->allocator, sd::Render_Layer_sizeof());
        stage0->vertices = core->allocator.allocate(&core->allocator, 512);
        stage0->v_count  = 512 * 8;
        stage0->indices  = core->allocator.allocate(&core->allocator, 512);
        stage0->i_count  = 512 * 8;

        sd::make_staging_layer(renderer, (&world->render_info.grid_layer.layer), stage0->vertices, 512, stage0->indices, 512);
        
        //        sd::set_background_color_rgba_v4(renderer, {0.0, 0.0, 0.0, 0.0});

        sd::set_render_layer(renderer, LAYER_LABEL_DEBUG_STATIC);
        
        sd::begin_polygon(renderer);
        sd::quad_color(renderer, {0.0f, 0.0f}, {0.0f, core->viewport.height}, {core->viewport.width, core->viewport.height}, {core->viewport.width, 0.0f}, -1000.0,
                       {255.0f/255.0f, 129.0f/255.0f, 0.0f, 0.7f},
                       {255.0f/255.0f, 129.0f/255.0f, 0.0f, 0.5f},
                       {255.0f/255.0f, 129.0f/255.0f, 0.0f, 0.0f},
                       {255.0f/255.0f, 129.0f/255.0f, 0.0f, 0.5f});
        sd::end_polygon(renderer);
        
        sd::set_staging_layer(renderer, stage0->layer);
        sd::begin_polygon(renderer);
        sd::quad_color(renderer, {core->viewport.width / 2.0f, 0.0f}, {core->viewport.width / 2.0f, core->viewport.height / 2.0}, {core->viewport.width, core->viewport.height / 2.0}, {core->viewport.width, 0.0f}, -1000.0,
                       vec4(1.0f),
                       vec4(1.0f),
                       vec4(1.0f),
                       vec4(1.0f));
        sd::end_polygon(renderer);
        
        sd::begin_polygon(renderer);
        sd::quad_color(renderer, {core->viewport.width / 2.0f, core->viewport.height / 2.0}, {core->viewport.width / 2.0f, core->viewport.height}, {core->viewport.width, core->viewport.height}, {core->viewport.width, core->viewport.height / 2.0}, -1000.0,
                       vec4(0.5f),
                       vec4(0.5f),
                       vec4(0.5f),
                       vec4(0.5f));
        sd::end_polygon(renderer);
        
        world->render_info.grid_layer.drawable_range = sd::append_staging_layer(renderer, LAYER_LABEL_DYNAMIC, stage0->layer);
#endif
        sd::set_render_layer(renderer, LAYER_LABEL_DEBUG_DYNAMIC);
        //#define USE_DEBUG_GRID_BG
#ifdef USE_DEBUG_GRID_BG
        sd::path_pixel_radius(renderer, 1);
        
        
        sd::set_depth(renderer, -100.0);
        sd::set_color_rgba_v4(renderer, {0.1f, 0.1f, 0.1f, 1.0});
        sd::begin_path(renderer);
        {
            grid_construction_width = world->sk.collision_system.layers[0].grid_dimensions * m::ceil(core->viewport.width / world->sk.collision_system.layers[0].grid_dimensions);
            
            grid_construction_height = world->sk.collision_system.layers[0].grid_dimensions * m::ceil(core->viewport.height / world->sk.collision_system.layers[0].grid_dimensions);
            
            
            for (isize row = -1; row < 2; row += 1) {
                for (isize col = -1; col < 2; col += 1) {
                    
                    vec2 offset = {col * grid_construction_width, row * grid_construction_height};
                    
                    for (isize i = 0; i < grid_construction_height; i += world->sk.collision_system.layers[0].grid_dimensions) {
                        sd::path_vertex(renderer, 0 + offset.x, i + offset.y, 0);
                        sd::path_vertex(renderer, grid_construction_width + offset.x, i + offset.y, -100.0);
                        
                        sd::break_path(renderer);
                    }
                    for (isize i = 0; i < grid_construction_width; i += world->sk.collision_system.layers[0].grid_dimensions) {
                        sd::path_vertex(renderer, i + offset.x, 0 + offset.y, 0);
                        sd::path_vertex(renderer, i + offset.x, grid_construction_height + offset.y, -100.0);
                        
                        sd::break_path(renderer);
                    }
                }
            }
            grid = sd::end_path(renderer);
            grid->is_enabled = false;
        }
#endif
        
        sd::set_depth(renderer, 0.0);
        
        
//        mtt::Dynamic_Array<mtt::Dynamic_Array<mtt::Dynamic_Array<int>>> da = {};
//        for (usize i = 0; i < 2; i += 1) {
//            da.push_back((mtt::Dynamic_Array<mtt::Dynamic_Array<int>>){});
//            for (usize j = 0; j < da.size(); j += 1) {
//                da[j].push_back((mtt::Dynamic_Array<int>){});
//                for (usize k = 0; k < da[j].size(); k += 1) {
//                    da[j][k].push_back((int)k);
//                }
//            }
//        }
//        mtt::Dynamic_Array<mtt::Dynamic_Array<mtt::Dynamic_Array<int>>> da_copy = {};
//        
//        da.clone_to(&da_copy);
//        da.deinit();
//        da_copy.deinit();
        
        mtt::set_color_scheme((mtt::Color_Scheme_ID)core->application_configuration.color_scheme);
        
        MTT_follow_system_window(core->application_configuration.follow_system_window);
        
        mtt::World_init(&world->mtt_world, (const mtt::World_Descriptor){
            .renderer  = renderer,
            .allocator = &core->allocator
        });
        mtt::Thing_Proxy_Scene_make_count(&world->mtt_world, DT_THING_PROXY_SCENE_COUNT);
        mtt::Thing_Proxy_Scene_for_idx(&world->mtt_world, DT_THING_PROXY_SCENE_IDX_CONTEXT_PANEL_VIEW)->on_clear = [](mtt::Thing_Proxy_Scene* scene) {
            scene->thing_to_proxy_map.clear();
            scene->proxy_aggregate.clear();
        };
        
        world->mtt_world.time_seconds = core->time_sim_elapsed;
        world->mtt_world.time_ns = core->time_sim_elapsed_ns;
        world->mtt_world.timestep = MTT_TIMESTEP;
        world->mtt_world.timestep_ns = MTT_TIMESTEP_NS;
        world->dt.pen.color = vec4(1.0, .325, .286, 1.0);
        world->dt.ui.changed = true;
        world->dt.ui.recording_indicator.color = color::RED;
        world->dt.ui.recording_indicator.is_on = false;
        
        
        world->dt.mtt = &world->mtt_world;
        
        
        
        dt::DrawTalk_init(core, &world->mtt_world, &world->dt);
        
        
        
        world->cam.cam_transform = Mat4(1.0f);
        mtt::calc_view_matrix(&world->cam);
        
        
        core->user_data = (void*)world;
        
        {
            {
                mtt::Thing* system = mtt::Thing_make(&world->mtt_world, ARCHETYPE_SYSTEM_ELEMENT);
                auto* thing_noun = dt::noun_lookup("thing");
                ASSERT_MSG(thing_noun != nullptr, "should exist!\n");
                //dt::vis_word_derive_from(system, thing_noun);
                dt::vis_word_derive_from(system, dt::noun_add("system"));
                system->do_evaluation = false;
                system->is_user_movable = false;
                system->is_user_drawable = false;
                system->is_visible = false;
                system->is_user_destructible = false;
                system->is_reserved = true;
                Thing_set_label(system, "MTT_CANVAS");
                world->system_thing_canvas = mtt::thing_id(system);
            }
            
            
            {
                mtt::Thing* I = mtt::Thing_make(&world->mtt_world, ARCHETYPE_SYSTEM_ELEMENT);
                auto* thing_noun = dt::noun_lookup("thing");
                ASSERT_MSG(thing_noun != nullptr, "should exist!\n");
                //dt::vis_word_derive_from(I, thing_noun);
                dt::vis_word_derive_from(I, dt::noun_add("I"));
                I->do_evaluation = false;
                I->is_user_movable = false;
                I->is_user_drawable = false;
                I->is_visible = false;
                I->is_user_destructible = false;
                I->is_reserved = true;
                Thing_set_label(I, "MTT_USER_MAIN");
                
                world->system_thing_I = mtt::thing_id(I);
            }
            
            {
                mtt::Thing* View = mtt::Thing_make(&world->mtt_world, ARCHETYPE_SYSTEM_ELEMENT);
                auto* thing_noun = dt::noun_lookup("thing");
                ASSERT_MSG(thing_noun != nullptr, "should exist!\n");
                //dt::vis_word_derive_from(View, thing_noun);
                dt::vis_word_derive_from(View, dt::noun_add("view"));
                View->do_evaluation = true;
                View->is_user_movable = false;
                View->is_user_drawable = false;
                View->is_visible = false;
                View->is_user_destructible = false;
                View->is_reserved = true;
                Thing_set_label(View, "MTT_VIEW_MAIN");
                
                world->system_thing_View = mtt::thing_id(View);
                
                View->logic.proc =  [](LOGIC_PROCEDURE_PARAM_LIST) -> LOGIC_PROC_RETURN_TYPE {
                    
                    mtt::Thing* self_thing = thing;
                    mtt::Thing_ID self_thing_id = mtt::thing_id(self_thing);
                    auto* word_dict = dt::word_dict();
                    auto& map = word_dict->source_to_target_relation;
                    auto find_it = map.find(self_thing_id);
                    if (find_it == map.end()) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    }
                    
//                    auto new_entry = (Word_Dictionary::Active_Relation){
//                        .source = src->id,
//                        .target = target->id,
//                        .dict_entry = rel
//                    };
                    
                    /*
                     typedef mtt::Map_Stable<Word_Dictionary_Entry*, dt::Dynamic_Array<Active_Relation>> Active_Relation_Map;
                     
                     std::unordered_map<mtt::Thing_ID, Active_Relation_Map> source_to_target_relation;
                     std::unordered_map<mtt::Thing_ID, Active_Relation_Map> target_to_source_relation;
                     */
                    
                    auto* dt = dt::DrawTalk::ctx();
                    if (mtt::Thing_ID_is_valid(dt->scn_ctx.thing_selected_with_pen)) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    }
                    
                    auto& rel_map = find_it->second;
                    auto find_follow = rel_map.find(dt::verb_add("follow"));
                    if (find_follow == rel_map.end()) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    }
                    
                    mtt::Camera* cam = &DrawTalk_World_ctx()->cam;
                    MTT_Core* core = mtt_core_ctx();
                    auto* view_position = &core->view_position;
                    auto* cam_transform = &cam->cam_transform;
                    
                    usize count_to_follow = 0;
                    
                    vec3 min_pos = vec3(POSITIVE_INFINITY);
                    vec3 max_pos = vec3(NEGATIVE_INFINITY);
                    auto& rel_list = find_follow->second;
                    for (const auto& rel : rel_list) {
                        mtt::Thing_ID tgt = rel.target;
                        mtt::Thing* tgt_thing;
                        if (mtt::Thing_try_get(world, tgt, &tgt_thing) && !dt::selection_is_recorded(tgt_thing)) {
                            vec3* pos_to_follow = mtt::access<vec3>(tgt_thing, "position");
                            if (pos_to_follow) {
                                vec3 pos_to_follow_loc = *pos_to_follow;
                                
                                mtt::Rep* rep = mtt::rep(tgt_thing);
                                for (usize i = 0; i < rep->colliders.size(); i += 1) {
                                    mtt::Collider* c = rep->colliders[i];
                                    auto* box = &c->aabb.saved_box;
                                    min_pos.x = m::min(min_pos.x, box->tl.x);
                                    min_pos.y = m::min(min_pos.y, box->tl.y);
                                    max_pos.x = m::max(max_pos.x, box->br.x);
                                    max_pos.y = m::max(max_pos.y, box->br.y);
                                    count_to_follow += 1;
                                }
                            }
                        }
                    }
                    
                    if (count_to_follow == 0) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    }
                    
                    vec2 mid = ((min_pos + max_pos) * 0.5f);
                    vec2 view_dim = vec3(core->viewport.width, core->viewport.height, 0.0);
                    vec3 translation =  vec3(mid - (view_dim * 0.5f), 0.0f);
                    vec2 thing_sep = (max_pos - min_pos);
                    vec3 scale = vec3((thing_sep / (view_dim * ((count_to_follow == 1) ? 0.1875f : 1.0f) )), 1.0f);
                    float32 s_val = (count_to_follow == 1) ? 1.0f : m::max(scale.x, scale.y);
                    
                    
                    *view_position = translation;
                    
                    *cam_transform = m::translate(Mat4(1.0f), translation) * m::translate(Mat4(1.0f), 0.5f*vec3(view_dim, 0)) * m::scale(Mat4(1.0f), vec3(s_val, s_val, 1.0f)) * m::translate(Mat4(1.0f), -0.5f*vec3(view_dim,0));
                    
                    mtt::calc_view_matrix(cam);
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
            }
        }
        
        
        //        {
        //            core->get_debug_visualization = [](MTT_Core* core) -> char* const {
        //
        //                mtt::World* world = &(((DrawTalk_World*)(core->user_data))->mtt_world);
        //
        //                mtt::String output = "digraph H {\n splines=ortho;\n";
        //
        //
        //
        //                for (auto it = mtt::iterator(world); mtt::next(&it);) {
        //                    Thing* thing = it.value;
        //
        //                    // build node
        //                    output += "_" + std::to_string(thing->id) + "[\nshape=plaintext\nlabel=<";
        //                    output += "<table border='1' cellborder='1'>\n";
        //                    output += "<tr><td bgcolor='pink'>label=[" + std::string(MTT_string_ref_to_cstring(thing->label)) + "]:name=[" + thing->ecs_entity.name().c_str() + "]\n";
        //                    output += "</td></tr>\n";
        //
        //                    output += "<tr>";
        //                    output += "<td align='left'>[is_active] ";
        //                    output += (mtt::is_active(thing) ? "true" : "false");
        //                    output += "</td></tr>";
        //                    if (thing->archetype_id == mtt::ARCHETYPE_GROUP) {
        //                        output += "<tr>";
        //                        output += "<td align='left'>[group_is_active] ";
        //                        output += (mtt::thing_group_is_active(thing) ? "true" : "false");
        //                        output += "</td></tr>";
        //                    }
        //
        //                    constexpr const bool show_fields_and_edges = true;
        //
        //
        //                    if constexpr ((show_fields_and_edges)) {
        //
        //                        // fields
        //                        if (thing->field_descriptor.fields.size() != 0) {
        //                            for (auto it_fields = thing->field_descriptor.fields.begin(); it_fields != thing->field_descriptor.fields.end(); ++it_fields) {
        //                                output += "<tr>";
        //
        //                                output += "<td align='left'>[field] " + (*thing->field_descriptor.idx_to_name)[it_fields->index] + "    -    " + mtt::meta[it_fields->type].name;
        //
        //                                output += "</td></tr>";
        //                            }
        //                        }
        //
        //                        // in ports
        //                        if (thing->ports.in_ports.size() != 0) {
        //
        //                            // output += "<tr>";
        //
        //                            for (auto it_ports = thing->ports.in_ports.begin(); it_ports != thing->ports.in_ports.end(); ++it_ports) {
        //
        //                                output += "<tr>";
        //
        //                                output += "<td align='left' port='_" + std::to_string(it_ports->index) + "_in'>";
        //
        //                                output += "[in-" + std::to_string(it_ports->index) + "] " + std::string(mtt::string_get(it_ports->tag)) + "    -    " + mtt::meta[it_ports->type].name;
        //
        //                                output += "</td></tr>";
        //                            }
        //
        //                        }
        //                        // out ports
        //                        if (thing->ports.out_ports.size() != 0) {
        //
        //                            for (auto it_ports = thing->ports.out_ports.begin(); it_ports != thing->ports.out_ports.end(); ++it_ports) {
        //
        //                                output += "<tr>";
        //
        //                                output += "<td align='left' port='_" + std::to_string(it_ports->index) + "_out'>";
        //
        //                                output += "[out-" + std::to_string(it_ports->index) + "] " + std::string(mtt::string_get(it_ports->tag)) + "    -    " + mtt::meta[it_ports->type].name;
        //
        //                                output += "</td></tr>";
        //                            }
        //                        }
        //
        //
        //                    }
        //                    output += "</table>>];\n";
        //
        //                    if constexpr ((show_fields_and_edges)) {
        //
        //                        for (auto it_c = curr_graph(world)->incoming.begin(); it_c != world->graph.incoming.end(); ++ it_c) {
        //                            auto& c_list = it_c->second;
        //                            for (usize i = 0; i < c_list.size(); i += 1) {
        //                                mtt::Connection* c = &c_list[i];
        //                                if (!c->is_valid()) {
        //                                    continue;
        //                                }
        //
        //                                mtt::Thing_ID parent = c->src_thingref;
        //                                mtt::Thing_ID child = c->dst_thingref;
        //
        //                                uint64 source_port = c->header.src_port_ref;
        //                                uint64 destination_port = c->header.dst_port_ref;
        //
        //                                output += "_" + std::to_string(parent) + ":_" +
        //                                std::to_string(source_port) + "_out";
        //                                output += " -> ";
        //                                output += "_" + std::to_string(child) + ":_" +
        //                                std::to_string(destination_port) + "_in";
        //                                output += ";\n";
        //
        //                            }
        //
        //                        }
        //
        //                    }
        //                }
        //
        //                for (auto it = mtt::iterator(world); mtt::next(&it);) {
        //                    Thing* thing = it.value;
        //
        //                    if (!mtt::is_root(thing) || thing->child_id_set.size() == 0) {
        //                        continue;
        //                    }
        //
        //
        //                    mtt::String hierarchy = "subgraph cluster_" + std::to_string(thing->id) + " { style=filled; color=beige;";
        //
        //                    mtt::Thing_apply_self_and_children(world, thing, [](mtt::World* world, mtt::Thing* thing, void* args) {
        //                        if (mtt::is_root(thing)) {
        //                            return;
        //                        }
        //
        //                        mtt::String* hierarchy = (mtt::String*)args;
        //
        //
        //                        *hierarchy += "_" + std::to_string(thing->parent_thing_id) + " -> _" + std::to_string(thing->id) + " [style=dashed, color=violet];";
        //
        //                    }, &hierarchy);
        //                    mtt::Things_mark_unvisited(world);
        //
        //
        //                    hierarchy += "}";
        //
        //                    output += hierarchy;
        //
        //                }
        //
        //                output += "}\n";
        //
        //                char* const out = (char* const)core->allocator.allocate((void*)&core->allocator, sizeof(char) * (output.size() + 1));
        //                out[output.size()] = '\0';
        //
        //                memcpy(out, output.c_str(), output.length() + 1);
        //
        //                return out;
        //            };
        //        }
        
        
        //world->ct.ui.hsv_slider = core->viewport.height;
        
        world->dt.pen.color = dt::pen_color_default();
        sd::set_background_color_rgba_v4(renderer, dt::bg_color_default());
        
        
        
        
        
        
        //        ecs_rule_t* test_fire = mtt_ecs_rule_new(world->mtt_world.ecs_world.c_ptr(), "collide(FLAMMABLE, EXPLOSIVE), type.flammable(FLAMMABLE), type.explosive(EXPLOSIVE)");
        //        ASSERT_MSG(test_fire != NULL, "should exist\n");
        //
        //        auto* r = mtt::Rule_make(&world->mtt_world);
        //        auto* trigger = mtt::Rule_add_trigger(r);
        //        trigger->relation_kind = "collide";
        //        trigger->ctx = test_fire;
        //        {
        //            mtt::append(&trigger->symbols, dt::Symbol());
        //            const mtt::String sym_key = "FLAMMABLE";
        //            trigger->name_to_symbol_idx[sym_key] = trigger->symbols.count - 1;
        //            dt::Symbol* sym = trigger->symbols.last_ptr();
        //            sym->key = sym_key;
        //            sym->label = "Source";
        //            trigger->role_to_symbol_idx[sym->label].push_back(trigger->symbols.count - 1);
        //            sym->constraints.push_back((dt::Constraint){
        //                .kind = dt::CONSTRAINT_KIND_TYPE,
        //                .word = dt::noun_lookup("tree"),
        //            });
        //            ASSERT_MSG(dt::noun_lookup("tree") != nullptr, "Should not be null!\n");
        //            r->symbol_name_to_trigger[sym_key].push_back(trigger);
        //        }
        //        {
        //            mtt::append(&trigger->symbols, dt::Symbol());
        //            const mtt::String sym_key = "EXPLOSIVE";
        //            trigger->name_to_symbol_idx[sym_key] = trigger->symbols.count - 1;
        //            dt::Symbol* sym = trigger->symbols.last_ptr();
        //            sym->key = sym_key;
        //            sym->label = "Destination";
        //            trigger->role_to_symbol_idx[sym->label].push_back(trigger->symbols.count - 1);
        //            sym->constraints.push_back((dt::Constraint){
        //                .kind = dt::CONSTRAINT_KIND_TYPE,
        //                .word = dt::noun_lookup("explosion"),
        //            });
        //            ASSERT_MSG(dt::noun_lookup("explosion") != nullptr, "Should not be null!\n");
        //            r->symbol_name_to_trigger[sym_key].push_back(trigger);
        //        }
        //        {
        //            auto* response = mtt::Rule_add_response(r);
        //            response->key = "catch_fire";
        //            r->response.is_bidirectional = false;
        //            mtt::append(&response->symbols, dt::Symbol());
        //            const mtt::String sym_key = "FLAMMABLE";
        //            response->name_to_symbol_idx[sym_key] = response->symbols.count - 1;
        //            dt::Symbol* sym = response->symbols.last_ptr();
        //            sym->key = sym_key;
        //            sym->label = "Source";
        //
        //            response->role_to_symbol_idx[sym->label].push_back(response->symbols.count - 1);
        //
        //            dt::Constraint c = (dt::Constraint){
        //                .kind = dt::CONSTRAINT_KIND_TYPE,
        //                .word = dt::noun_lookup("tree"),
        //            };
        //            sym->constraints.push_back(c);
        //
        //            r->symbol_name_to_response[sym_key].push_back(&r->response);
        //
        //            for (usize s = 0; s < trigger->symbols.count; s+= 1) {
        //                dt::Symbol* sym = &trigger->symbols[s];
        //                sym->rule_symbol_id = ecs_rule_find_variable(trigger->ctx, sym->key.c_str());
        //            }
        //        }
        ////        {
        ////            dt::Word_Dictionary_Entry* arr[] = {dt::noun_lookup("particle"), dt::noun_lookup("antiparticle")};
        ////            for (usize i = 0; i < 700; i += 1) {
        ////                mtt::Rep* rep;
        ////                mtt::Thing* thing = mtt::Thing_make_with_unit_collider(&world->mtt_world, mtt::ARCHETYPE_FREEHAND_SKETCH, vec2(10.0f), &rep, mtt::COLLIDER_TYPE_AABB, vec3(i * 40.0f, 0.0f, 200.0f));
        ////                sd::begin_polygon(core->renderer);
        ////                {
        ////                    sd::rectangle(core->renderer, vec2(-0.5f), vec2(1.0f), 0.0f);
        ////                }
        ////                auto* d = sd::end_polygon(core->renderer);
        ////                rep->render_data.drawable_info_list.push_back(d);
        ////                rep->pose_scale = vec3(15.0f,15.0f, 1.0f);
        ////            }
        ////
        ////        }
        
        
        //        mtt::add_connection(&world->mtt_world, Thing_ID src, const String& src_tag, Thing_ID dst, const String& dst_tag, Thing** src_thing_out, Thing** dst_thing_out)
        
        
        mtt::Augmented_Reality_load(&core->ar_ctx);
        
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        {
            mtt::Video_Recording_Context_Args args = {
                .image = world->render_info.remote_screen = sd::Image_create_empty(sd::images(renderer), "remote_screen", { .usage_flags = 0 }),
                .images = sd::images(renderer),
                .user_data = (void*)world,
                .renderer = (void*)renderer,
                .on_data = [](Video_Recording_Context_Info* info) {
                    // auto* world = static_cast<DrawTalk_World*>(info->user_data);
                    
                    //                    MTT_Core* core = static_cast<MTT_Core*>(user_data);
                    //                    sd::Images* imgs = static_cast<sd::Images*>(images);
                }
            };
            core->screen_recording = mtt::Video_Recording_Context_make(&args);
        }
#endif
        
        
        if constexpr ((false)) {
            
            mtt::Thing* a = mtt::Thing_make(&world->mtt_world, mtt::ARCHETYPE_NUMBER);
            //mtt::Thing* b = mtt::Thing_make(&world->mtt_world, mtt::ARCHETYPE_NUMBER);
            //mtt::Thing* c = mtt::Thing_make(&world->mtt_world, mtt::ARCHETYPE_NUMBER);
            mtt::Thing* dist = mtt::Thing_make(&world->mtt_world, mtt::ARCHETYPE_DISTANCE);
            
            mtt::add_connection(&world->mtt_world, a, "position", dist, "source");
            //mtt::add_connection(&world->mtt_world, b, "position", dist, "destination");
            //mtt::add_connection(&world->mtt_world, dist, "distance", c, "value");
            
            (*mtt::access<vec3>(dist, "scale")) = vec3(0.0f, 1.0f, 0.0f);
        }
        
        //m::rotate(Mat4(1.0f), (float32)time, vec3(0.0f, 0.0f, 1.0f));
        //m::rotate(v, angle)
        
        
        //        dt::rules::Condition_Builder cb;
        //        cb.set_var("DOG",  "AGENT");
        //        cb.set_var("BED",  "DIRECT_OBJECT");
        //        cb.set_var("BIRD", "AGENT");
        //        cb.set_var("SKY",  "OBJECT");
        //
        //        cb.add_clause({
        //            {.name = "jump", .is_variable = false, .is_negated = false},
        //            {.name = "DOG",  .is_variable = true},
        //            {.name = "BED",  .is_variable = true}
        //        });
        //
        //        cb.add_clause({
        //            {.name = "fly",  .is_variable = false, .is_negated = false},
        //            {.name = "BIRD", .is_variable = true},
        //            {.name = "SKY",  .is_variable = true}
        //        });
        //
        ////        cb.add_clause({
        ////            {.name = "fly",  .is_variable = false, .is_negated = false},
        ////            {.name = "DOG",  .is_variable = true},
        ////            {.name = "SKY",  .is_variable = true}
        ////        });
        //
        //        cb.print();
        //        int step1 = 1;
        //
        //        cb.build();
        //
        //        cb.print();
        //        int step2 = 2;
        
        
        
        if constexpr ((false)) {
            auto* mtt = &world->mtt_world;
            
            auto* src = mtt::Thing_make(mtt, mtt::ARCHETYPE_NUMBER);
            auto* dst = mtt::Thing_make(mtt, mtt::ARCHETYPE_NUMBER);
            
            
            auto* dist  = mtt::Thing_make(mtt, mtt::ARCHETYPE_DISTANCE);
            auto* diff = mtt::Thing_make(mtt, mtt::ARCHETYPE_DIFFERENCE);
            
            
            
            auto* diff_display = mtt::Thing_make(mtt, mtt::ARCHETYPE_NUMBER);
            auto* dist_display = mtt::Thing_make(mtt, mtt::ARCHETYPE_NUMBER);
            
            auto* sign = mtt::Thing_make(mtt, mtt::ARCHETYPE_SIGN);
            auto* sign_display = mtt::Thing_make(mtt, mtt::ARCHETYPE_NUMBER);
            
            mtt::add_connection(mtt, src, "position", dist, "source");
            mtt::add_connection(&world->mtt_world, dst, "position", dist, "destination");
            mtt::add_connection(&world->mtt_world, dist, "distance", dist_display, "value");
            
            mtt::add_connection(&world->mtt_world, dist, "distance", diff, "value");
            mtt::add_connection(&world->mtt_world, diff, "value", diff_display, "value");
            
            mtt::add_connection(&world->mtt_world, dist, "distance", diff, "value");
            mtt::add_connection(&world->mtt_world, diff, "value", sign, "value");
            
            mtt::add_connection(&world->mtt_world, sign, "value", sign_display, "value");
            
            auto* rotator = mtt::Thing_make(mtt, mtt::ARCHETYPE_ROTATOR);
            *mtt::access<mtt::Thing_Ref>(rotator, "thing") = mtt::Thing_Ref(src);
            
            
            mtt::add_connection(mtt, sign, "value", rotator, "direction");
            mtt::add_connection(mtt, dist, "distance", rotator, "speed");
            
            
        }
#if NUMBER_DATA_FLOW_TEST
        {
            MTT_print("SIZE OF ANY: %lu\n", sizeof(mtt::Any));
            auto* mtt = &world->mtt_world;
            auto* time_value = mtt::Thing_make(mtt, mtt::ARCHETYPE_TIME);
            auto* sin01 = mtt::Thing_make(mtt, mtt::ARCHETYPE_SINE01);
            mtt::add_connection(mtt, time_value, 0, sin01, 0);
            
            
            
            auto* value_to_multiply = mtt::Thing_make(mtt, mtt::ARCHETYPE_FLOAT32_VALUE);
            mtt::access_write<float32>(value_to_multiply, "value", [](mtt::Thing* thing, float32* f) {
                *f = MTT_PI32;
            });
            auto* multiplier = mtt::Thing_make(mtt, mtt::ARCHETYPE_MULTIPLY);
            
            mtt::add_connection(mtt, sin01, 0, multiplier, 0);
            mtt::add_connection(mtt, value_to_multiply, 0, multiplier, 1);
            
            auto* display = mtt::Thing_make(mtt, mtt::ARCHETYPE_NUMBER);
            add_connection(mtt, time_value, 0, display, 0);
            
            __testthing__ = multiplier;
            
            auto* modified_display = mtt::Thing_make(mtt, mtt::ARCHETYPE_NUMBER);
            mtt::add_connection(mtt, sin01, 0, modified_display, 0);
            
            auto* multiplied_modified_display = mtt::Thing_make(mtt, mtt::ARCHETYPE_NUMBER);
            mtt::add_connection(mtt, multiplier, 0, multiplied_modified_display, 0);
        }
#endif
#if TANG_INFO_TEST
        {
            auto* mtt = &world->mtt_world;
            tanginfo__ = mtt::Thing_make(mtt, mtt::ARCHETYPE_TEXT);
            tanginfo__->is_user_destructible = false;
        }
#endif
        
        
#define QUEUE_MISC_TEST (0)
#if QUEUE_MISC_TEST
        
        
        MTT_print("%lu\n", sizeof(mtt::Any));
        MTT_print("%lu\n", sizeof(mtt::Dynamic_Array<int>));

        
        mem::Buckets_Allocation buckets;
        mem::Buckets_Allocation_init(&buckets, core->allocator, 4);
        
        mtt::Dynamic_Array<uint32> _ = mtt::Dynamic_Array<uint32>::make(buckets.allocator, 0, 2);
        
        {
            usize32 i = 0;
            for (usize32 j = 0; j < 2; i += 1, j += 1) {
                _.push_back(i);
            }
            for (usize32 j = 0; j < 2; i += 1, j += 1) {
                _.push_back(i);
            }
            ASSERT_MSG(_.size() == 4, "Size incorrect!\n");
            MTT_print("{\n");
            for (usize32 i = 0; i < _.size(); i += 1) {
                MTT_print("    %u,\n", _[i]);
            }
            MTT_print("}\n");
        }
        
        {
            usize i = 0;
            mtt::Message_Queue q;
            q.init(4);
            ASSERT_MSG(q.count == 0,        "incorrect count\n");
            ASSERT_MSG(q.messages.cap == 4, "incorrect cap\n");
            
            for (i = 0; i < 4; i += 1) {
                Message msg;
                msg.length = i;
                q.enqueue(&msg);
            }
            ASSERT_MSG(q.count == 4,        "incorrect count\n");
            ASSERT_MSG(q.messages.cap == 4, "incorrect cap\n");
            
            
            Message_Queue_print(&q);
            
            {
                Message msg;
                msg.length = i++;
                q.enqueue(&msg);
                ASSERT_MSG(q.count == 5,        "incorrect count\n");
                ASSERT_MSG(q.messages.cap == 8, "incorrect cap\n");
            }
            
            {
                q.dequeue_discard();
                
                ASSERT_MSG(q.count == 4,        "incorrect count\n");
                ASSERT_MSG(q.messages.cap == 8, "incorrect cap\n");
                
                Message_Queue_print(&q);
            }
            
            {
                for (; !q.is_full(); i += 1) {
                    Message msg;
                    msg.length = i;
                    q.enqueue(&msg);
                    
                    Message_Queue_print(&q);
                }
                
                
            }
            
            {
                Message msg;
                msg.length = i++;
                q.enqueue(&msg);
                ASSERT_MSG(q.count == 9,        "incorrect count\n");
                ASSERT_MSG(q.messages.cap == 16, "incorrect cap\n");
                
                Message_Queue_print(&q);
            }
            
            q.deinit();
            q.init(2);
            
            ASSERT_MSG(q.count == 0 && q.cap() == 2, "state incorrect\n");
            
            for (i = 0; i < 2; i += 1) {
                Message msg;
                msg.length = i;
                q.enqueue(&msg);
            }
            ASSERT_MSG(q.is_full(), "should be full\n");
            Message_Queue_print(&q);
        }
#endif
        
        sd::Image_Load_Descriptor desc;
        desc.paths.push_back((sd::Path_Info){"images/KTR_Insignia", "png", "KTR_Insignia"});
        
        
        if (sd::images_load_async(renderer, &desc, [](const sd::Image_Load_Result& result, bool status) -> bool {
            if (status == false) {
                
            } else {
                
                sd::Image* image;
                if (sd::Images_lookup(result.image_storage, "KTR_Insignia", &image)) {
                    
                }
                
            }
            return true;
        })) {
            
        } else {
            
        }
        
        
        mtt::Text_File_Load_Descriptor desc_text;
        desc_text.paths.push_back((mtt::Path_Info){"WEE", "txt"});
        if (mtt::Text_File_load_async(&desc_text, [](mtt::Text_File_Load_Result result, bool status) -> bool {
            if (status == false) {
                
            } else {
                
                for (usize i = 0; i < result.text.size(); i += 1) {
                    MTT_print("%s\n", result.text[i].c_str());
                    
                }
                
            }
            return true;
        })) {
            
        } else {
            
        }
        
        
        
        
        
        //        sd::build_drawable(renderer, LAYER_LABEL_STATIC, [&]() {
        //            vec3 cursor = {core->viewport.width / 2.0f, core->viewport.height / 2.0f, 500.0f};
        //            vec3 vp_half = cursor;
        //            vp_half.z = 0.0f;
        //
        //            vec3 off = {16.0f, 16.0f, 0.0f};
        //            vec3 increment = off;
        //            // move to top left of square
        //            cursor -= off;
        //
        //            sd::set_color_rgba_v4(renderer, {1.0f, 0.0f, 1.0f, 1.0f});
        //            sd::path_pixel_radius(renderer, 1);
        //            sd::begin_path(renderer);
        //
        //            for (usize i = 0; i < 4; i += 1) {
        //                // top-left
        //                sd::path_vertex_v3(renderer, cursor);
        //                // top-right
        //                cursor += (vec3){off.x * 2, 0.0f, 0.0f};
        //                sd::path_vertex_v3(renderer, cursor);
        //                // bottom-right
        //                cursor += (vec3){0, off.y * 2, 0.0f};
        //                sd::path_vertex_v3(renderer, cursor);
        //                // bottom-left
        //                cursor += (vec3){-increment.x -off.x * 2, 0, 0.0f};
        //                sd::path_vertex_v3(renderer, cursor);
        //
        //                cursor += (vec3){0.0f, -increment.y -off.y * 2, 0.0f};
        //
        //                // enlarge offset
        //                off += increment;
        //            }
        //            sd::path_vertex_v3(renderer, cursor);
        //
        //            sd::end_path(renderer);
        //        });
        
        //test_particles.particle_mesh->is_enabled = false;
        
        {
            auto& wait_indicator_info = instanced_indicators.render_info_list[0];
            wait_indicator_info.corner_multiplier = vec2(1.0f, -1.0f);
            wait_indicator_info.drawable_source = sd::build_drawable(renderer, LAYER_LABEL_STATIC_INSTANCING_CANVAS, [&](){
                sd::save(renderer);
                
                
                sd::begin_polygon_no_new_drawable(renderer);
                //auto center = vec2(core->viewport.width, core->viewport.height) * 0.5f;
                //auto radius = vec2(core->viewport.width, core->viewport.height) * (float32)(1.0/128.0);
                //auto r = sd::calc_rectangle_tl_from_center_radius(center, radius);
                //sd::rectangle(renderer, vec2(r.x, r.y), vec2(r.width, r.height), 995.0f);
                //sd::set_color_rgba_v4(renderer, color::WHITE);
                
                
                sd::set_color_rgba_v4(renderer, vec4(vec3(color::WHITE) * 0.5f, 1.0f));
                sd::circle(renderer, 0.5f, vec3(0.0f, 0.0f, 0.0f));
                sd::set_color_rgba_v4(renderer, color::WHITE);
                sd::polygon_arc_fraction(renderer, 0.5f, vec3(0.0f, 0.0f, 0.0f), 24, .125);

                
                sd::end_polygon_no_new_drawable(renderer);
                sd::restore(renderer);
            });
            
//            auto radius = core->viewport.width * (float32)(1.0/128.0);
//            auto it_count = core->viewport.width / (2 * radius);
//            for (usize i = 0; i < it_count; i += 1) {
//                for (usize j = 0; j < it_count; j += 1) {
//                    wait_indicator_info.instances.emplace_back();
//                    auto& d_info = wait_indicator_info.instances.back();
//                    d_info.set_transform(m::translate(mat4(1.0f), vec3(i * radius * 2, j * radius * 2, 999.9f)) * m::scale(Mat4(1.0f), vec3(radius, radius, 1.0f)));
//                }
//            }
        }
        
        
        if constexpr ((TEST_PARTICLES_AND_NODE_GRAPH))
        {
            test_particles.particle_system = mtt::Thing_make(mtt::ctx(), mtt::ARCHETYPE_PARTICLE_SYSTEM);
            test_particles.node_graph = mtt::Thing_make(mtt::ctx(), mtt::ARCHETYPE_NODE_GRAPH);
            
            mtt::Thing_set_label(test_particles.node_graph, "node_graph_test");
            
            //        mtt::Thing* ref = mtt::Thing_make(&world->mtt_world, mtt::ARCHETYPE_REFERENCE);
            //        auto* refs = mtt::access<Thing_List>(ref, "references");
            //        refs->push_back(mtt::Thing_Ref(particle_thing));
            
            
            float32 node_scale_factor = 16;
            float32 particle_scale_factor = 4;
            if constexpr ((enable_particles_in_test)) {
                mtt::access_array<Particle_System_State>( test_particles.particle_system, "particle_state_list", [&](auto& array) {
                    
                    auto* mesh_src = sd::build_drawable(renderer, LAYER_LABEL_STATIC_INSTANCING, [&]() {
                        const usize scale_factor = particle_scale_factor;
                        float32 extent = core->viewport.height * scale_factor * 4;
                        const usize particle_count = 1024 * 4;
                        
                        vec3 center = vec3(0.0f); //vec3(core->viewport.width / 2.0f, core->viewport.height / 2.0f,    0.0f);
                        float32 velocity_factor = 256.0f;
                        float32 velocity_max = 24;
                        
                        auto& dr_list = **mtt::access_pointer_to_pointer<mtt::Dynamic_Array<sd::Drawable_Info>*>(test_particles.particle_system, "particle_drawable_list");
                        mtt::reserve(&dr_list, particle_count);
                        dr_list.set_size(particle_count);
                        //                for (usize i = 0; i < dr_list.size(); i += 1) {
                        //                    dr_list[i] = sd::Drawable_Info();
                        //                }
                        
                        for (usize i = 0; i < particle_count; i += 1) {
                            Particle_System_State p = {};
                            // particle.position = v3(25, 25, 0) + v3(random_range(-5, 5), random_range(-5, 5), 0);
                            
                            float64 r;
                            float64 a_rad;
                            MTT_Random_Float64_value_within_circle(extent, &r, &a_rad);
                            
//                            float64 c = m::cos(a_rad);
//                            float64 s = m::sin(a_rad);
                            float64 c;
                            float64 s;
                            m::sincos(a_rad, &s, &c);
                            float64 x = ((float64)r)*c;
                            float64 y = ((float64)r)*s;
                            float32 z_offset = m::max(-999.0f, -1.0f - (0.001f * (i + 1)));
                            p.position = center + vec3((float32)x, (float32)y, z_offset);
                            p.velocity = vec3(0.0f);//m::normalize(p.position - center) * (velocity_factor * (1.0f + (float32)MTT_Random_Float64_value_scaled(velocity_max)));
                            p.max_velocity_magnitude = (2.0f * m::min(25.0f * 25.0f * 4, (float32)core->viewport.width)) / 500.0f;
                            p.angular_velocity = MTT_Random_range(5, 32);
                            p.angle    = MTT_Random_range(0, 360) * (MTT_PI / 180.0f);
                            p.scale    = MTT_Random_range(4, 16) * scale_factor;
                            p.color =  vec4(vec3(0.5f, 0.5f, 1.0f), 1.0f * MTT_Random_range(50, 101) / 100.0f);
                            dr_list[i].set_color_factor(vec4(0.0f));
                            dr_list[i].set_color_addition(p.color);
                            
                            array.push_back(p);
                            array[i].print();
                        }
                        
                        
                        
                        sd::begin_polygon_no_new_drawable(renderer);
                        {
                            sd::set_color_rgba_v4(renderer, vec4(1.0f));
                            sd::polygon_convex_regular(renderer, 1.0f, vec3(0.0f), 5);
                        }
                        sd::end_polygon_no_new_drawable(renderer);
                    });
                    
                    {
                        mtt::Rep* rep = mtt::rep(test_particles.particle_system);
                        rep->render_data.drawable_info_list.push_back(mesh_src);
                        rep->render_data.is_shared = true;
                    }
                });
            }
            {
                mtt::Thing* nav_nodes = test_particles.node_graph;
                
                
                
                
                
                
                for (usize i = 0; i < nodes.cap(); i += 1) {
                    mtt::Rep* rep;
                    
                    nodes[i] = Thing_make_with_aabb_corners(&world->mtt_world, mtt::ARCHETYPE_FREEHAND_SKETCH, vec2(1.0f), &rep, (mtt::Box){.tl = vec2(-1, -1), .br = vec2(1, 1)}, 0.0, true);
                    mtt::Thing_set_label(nodes[i], "node graph test " + std::to_string(i));
                    nodes[i]->is_user_movable = true;
                    nodes[i]->lock_to_canvas  = false;
                    nodes[i]->is_user_drawable = true;
                    //                nodes[i]->is_user_destructible = false;
                    
                    {
                        auto* noun = dt::noun_lookup("thing");
                        ASSERT_MSG(noun != nullptr, "should exist!\n");
                        dt::vis_word_derive_from(nodes[i], noun);
                    }
                    
                    
                    nodes_as_ids[i] = nodes[i]->id;
                };
                nodes.count = nodes.cap();
                nodes_as_ids.count = nodes.count;
                
                sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC);
                sd::set_color_rgba_v4(renderer, vec4(1.0f, 1.0f, 1.0f, 1.0f));
                for (usize i = 0; i  < nodes.size(); i += 1) {
                    sd::begin_polygon(renderer);
                    {
                        //sd::polygon_convex_regular(renderer, 1.0f, vec3(0.0f), 21);
                        sd::rectangle(renderer, vec2(-1.0f, -1.0f), vec2(2.0f, 2.0f), 0.0f);
                    }
                    auto* d = sd::end_polygon(renderer);
                    {
                        mtt::Rep* rep = mtt::rep(nodes[i]);
                        
                        mtt::set_pose_transform(nodes[i], m::scale(Mat4(1.0f), {node_scale_factor*4.0f, node_scale_factor*4.0f, 1.0f}));
                        rep->render_data.drawable_info_list.push_back(d);
                        rep->append_new(REPRESENTATION_TYPE_IMAGE);
                        rep->points.back().push_back(vec3(-1.0f, -1.0f, 0.0f));
                        rep->points.back().push_back(vec3(-1.0f, 1.0f, 0.0f));
                        rep->points.back().push_back(vec3(1.0f, 1.0f, 0.0f));
                        rep->points.back().push_back(vec3(1.0f, -1.0f, 0.0f));
                        rep->points.back().push_back(vec3(-1.0f, -1.0f, 0.0f));
                        
                        rep->render_data.set_on_copy([](void* ctx, Render_Data& dst, Render_Data& src, void* args) {
                            int64 layer_id = (int64)args;
                            
                            mtt::Thing* thing = static_cast<mtt::Thing*>(ctx);
                            mtt::World* mtt_world = mtt::world(thing);
                            
                            sd::Renderer* renderer = mtt_world->renderer;
                            
                            sd::save(renderer);
                            
                            mtt::Rep& rep = *mtt::rep(thing);
                            
                            if (layer_id == -1) {
                                for (usize i = 0; i < src.drawable_info_list.size(); i += 1) {
                                    auto* info = sd::Drawable_Info_copy(renderer, src.drawable_info_list[i]->layer_id, src.drawable_info_list[i]);
                                    sd::Drawable_Info_ref_inc(renderer, info);
                                    
                                    sd::set_transform(info, rep.hierarchy_model_transform);
                                    dst.drawable_info_list.emplace_back(info);
                                    dst.drawable_info_list.back()->is_enabled = true;
                                }
                            } else {
                                for (usize i = 0; i < src.drawable_info_list.size(); i += 1) {
                                    auto* info = sd::Drawable_Info_copy(renderer, layer_id, src.drawable_info_list[i]);
                                    sd::Drawable_Info_ref_inc(renderer, info);
                                    
                                    sd::set_transform(info, rep.hierarchy_model_transform);
                                    dst.drawable_info_list.emplace_back(info);
                                    dst.drawable_info_list.back()->is_enabled = true;
                                }
                            }
                            
                            
                            //ASSERT_MSG(dst.drawable_info_list.size() = src.drawable_info_list.size(), "WRONG SIZE???");
                            
                            sd::restore(renderer);
                            
                        });
                    }
                }
                
                {
                    vec3 loc_scale = vec3(node_scale_factor * 4 * 16);
                    loc_scale.z = 1.0f;
                    vec3 offset = {25 - 0.5, 25 - 0.5, 0};
                    
                    vec3 inline_offset = {-2, -2, 0};
                    
                    auto& ng_state = *mtt::access<Node_Graph_State>(nav_nodes, "state");
                    
                    
                    vec3 pos[] = {
                        { 0,  0,  -1.1f},
                        { 0,  5,  -1.1f},
                        { 1,  1,  -1.1f},
                        {-1,  1,  -1.1f},
                        { 1, -1,  -1.1f},
                        {-1, -1,  -1.1f},
                        { 0, -1,  -1.1f},
                    };
                    
                    for (usize i = 0; i < nodes.size(); i += 1) {
                        mtt::Thing_set_position(nodes[i], (offset + ((pos[i] + inline_offset) * loc_scale)));
                    }
                    
                    
                    //            particle_nodes[0].position = v3(25, 25, 0) + v3(0, 0, 0);   particle_nodes[0].nexts.allocator = wb_default_allocator(); array_add(*particle_nodes[0].nexts, *particle_nodes[1]);
                    
                    //            particle_nodes[1].position = v3(25, 25, 0) + v3(0, 5, 0);   particle_nodes[1].nexts.allocator = wb_default_allocator(); array_add(*particle_nodes[1].nexts, *particle_nodes[2]); array_add(*particle_nodes[1].nexts, *particle_nodes[3]);
                    
                    //            particle_nodes[2].position = v3(25, 25, 0) + v3(5, 5, 0);   particle_nodes[2].nexts.allocator = wb_default_allocator(); array_add(*particle_nodes[2].nexts, *particle_nodes[4]);
                    
                    //            particle_nodes[3].position = v3(25, 25, 0) + v3(-5, 5, 0);  particle_nodes[3].nexts.allocator = wb_default_allocator(); array_add(*particle_nodes[3].nexts, *particle_nodes[5]);
                    
                    //            particle_nodes[4].position = v3(25, 25, 0) + v3(5, -5, 0);  particle_nodes[4].nexts.allocator = wb_default_allocator(); array_add(*particle_nodes[4].nexts, *particle_nodes[6]);
                    
                    //            particle_nodes[5].position = v3(25, 25, 0) + v3(-5, -5, 0); particle_nodes[5].nexts.allocator = wb_default_allocator(); array_add(*particle_nodes[5].nexts, *particle_nodes[6]);
                    
                    //            particle_nodes[6].position = v3(25, 25, 0) + v3(0, -5, 0);  particle_nodes[6].nexts.allocator = wb_default_allocator(); array_add(*particle_nodes[6].nexts, *particle_nodes[0]);
                    
                    node_graph_set_graph_edge_list(&ng_state, nodes[0]->id, {
                        {nodes[1]->id}
                    });
                    node_graph_set_graph_edge_list(&ng_state, nodes[1]->id, {
                        {nodes[2]->id, THING_EDGE_FLAG_TELEPORT_TO_DST},
                        {nodes[3]->id, THING_EDGE_FLAG_TELEPORT_TO_DST}
                    });
                    node_graph_set_graph_edge_list(&ng_state, nodes[2]->id, {
                        {nodes[4]->id}
                    });
                    node_graph_set_graph_edge_list(&ng_state, nodes[3]->id, {
                        {nodes[5]->id}
                    });
                    node_graph_set_graph_edge_list(&ng_state, nodes[4]->id, {
                        {nodes[6]->id}
                    });
                    node_graph_set_graph_edge_list(&ng_state, nodes[5]->id, {
                        {nodes[6]->id}
                    });
                    node_graph_set_graph_edge_list(&ng_state, nodes[6]->id, {
                        {nodes[0]->id}
                    });
                    for (usize i = 0; i < nodes.size(); i += 1) {
                        ng_state.nodes.push_back(nodes[i]->id);
                    }
                    
                    node_graph_print(&ng_state);
                    
                    if constexpr ((enable_particles_in_test)) {
                        node_graph_add_follower(&ng_state,
                                                test_particles.particle_system,
                                                [&](auto& record) {
                            
                            mtt::access_array<Particle_System_State>( test_particles.particle_system, "particle_state_list", [&](auto& array) {
                                for (usize i = 0; i < array.size(); i += 1) {
                                    const usize which_idx = MTT_Random_range(0, nodes.size());
                                    array[i].target = nodes[which_idx]->id;
                                }
                            });
                            
                        });
                    }
                    
#define TEST_REMOTE_IMAGE_LOAD (false)
                    if constexpr ( (!TEST_REMOTE_IMAGE_LOAD ))
                    {
                        
                        sd::Image_Load_Descriptor desc;
                        desc.paths.push_back((sd::Path_Info){"images/dog0", "png", "dog0"});
                        desc.custom_data = (void*)&nodes_as_ids;
                        
                        if (sd::images_load_async(renderer, &desc, [](const sd::Image_Load_Result& result, bool status) -> bool {
                            if (status == false) {
                                
                            } else {
                                sd::Image* image;
                                if (sd::Images_lookup(result.image_storage, "dog0", &image)) {
                                    auto& n_list = *((mtt::Array<mtt::Thing_ID, 7>*)(result.custom_data));
                                    for (usize i = 0; i < nodes.size(); i += 1) {
                                        mtt::Thing* thing = mtt::ctx()->Thing_try_get(n_list[i]);
                                        ASSERT_MSG(thing != nullptr, "thing should exist!");
                                        mtt::Rep* rep = mtt::rep(thing);
                                        for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
#define TEST_SCROLLING (1)
#if TEST_SCROLLING
                                            
                                            (*it)->set_texture_ID(2);
                                            (*it)->set_texture_sampler_ID(sd::Texture_Sampler_ID_REPEAT);
                                            
                                            (*it)->set_texture_animation_speed(SD_vec2(2, 0.0f));
                                            
#else
                                            (*it)->set_texture_ID(image->texture_ID);
                                            (*it)->set_texture_sampler_ID(sd::Texture_Sampler_ID_CLAMP_TO_EDGE);
#endif
                                        }
                                    }
                                } else {
                                }
                            }
                            return true;
                        })) {
                        } else {
                        }
                        
                        
                    } else
                    {
                        {
                            
                            
                            sd::Image_Load_Descriptor desc2;
                            
                            desc2.paths.push_back((sd::Path_Info){"<IMAGE URL>/<NAME OF IMAGE>.png"});
                            desc2.custom_data = (void*)&nodes_as_ids;
                            desc2.persist_data = true;
                            sd::images_load_remote_async(renderer, &desc2, [](const sd::Image_Load_Result& result, bool status) -> bool {
                                if (status == false) {
                                    MTT_error("%s", "failed to load\n");
                                } else {
                                    
                                    sd::Image* image;
                                    if (sd::Images_lookup(result.image_storage, "<NAME OF IMAGE>", &image)) {
                                        auto& n_list = *((mtt::Array<mtt::Thing_ID, 7>*)(result.custom_data));
                                        for (usize i = 0; i < nodes.size(); i += 1) {
                                            mtt::Thing* thing = mtt::ctx()->Thing_try_get(n_list[i]);
                                            ASSERT_MSG(thing != nullptr, "thing should exist!");
                                            mtt::Rep* rep = mtt::rep(thing);
                                            mtt::set_pose_transform(thing, m::scale(Mat4(1.0f), rep->pose_transform_values.scale * vec3(image->size.x / image->size.y, 1.0, 1.0f)));
                                            for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
                                                (*it)->set_texture_ID(image->texture_ID);
                                            }
                                        }
                                    } else {
                                    }
                                }
                                
                                return true;
                            });
                        }
                        
                    }
                    
                    
                    
                    
                }
                
            }
        }
        
        
        if constexpr ((false)) {
            core->remote_system.net = &core->server_ext;
            Extension_Server_init(&core->server_ext);
            // directly to Toio
            
//            if constexpr ((false)) {
//                core->server_ext.on_read = [](cstring msg, usize length) {
//                    mtt::String str_in = mtt::String("Toio status: [") + msg + mtt::String("]");
//                    mtt::text_update_value(tanginfo__, str_in);
//                };
//            } else {
//                core->server_ext.on_read = [](cstring msg, usize length) {
//                    mtt::String str_in = mtt::String("Toio status: [") + msg + mtt::String("]");
//                    MTT_print("message received: %s\n", str_in.c_str());
//                };
//            }
            core->server_ext.on_start = [](void) {
                MTT_print("%s", "(Extension Server)started\n");
            };
            Extension_Server_start_with_port(&core->server_ext, 90);
            core->server_ext.on_connect = [](void) {
                MTT_print("%s", "(Extension Server)a client connected!\n");
            };
            core->server_ext.on_read = [](cstring msg, usize length) {
                //MTT_print("(Extension Server)read\n");
                auto* mtt_world = mtt::ctx();
                
                MTT_log_debug("%s : %s\n", msg, msg + 4);
                
                Extension_Server_write(&mtt_core_ctx()->server_ext,
                                       msg + 4, length -  4);
                
                
                typedef enum MTT_MESSAGE_TYPE {
                    MTT_MESSAGE_TYPE_REQUEST_ID = 1,
                    MTT_MESSAGE_TYPE_PING,
                    MTT_MESSAGE_TYPE_RESET,
                    MTT_MESSAGE_TYPE_MTT_INIT,
                } MTT_MESSAGE_TYPE;
                static mtt::Map<mtt::String, MTT_MESSAGE_TYPE> lookup = {
                    {"MTT_REQUEST_ID", MTT_MESSAGE_TYPE_REQUEST_ID},
                    {"__PING__", MTT_MESSAGE_TYPE_PING},
                    {"MTT_RESET", MTT_MESSAGE_TYPE_RESET},
                    {"MTT_INIT", MTT_MESSAGE_TYPE_MTT_INIT},
                };
                
                usize size = 4096 * 2;
                ArduinoJson::DynamicJsonDocument doc(size);
                DeserializationError error = deserializeJson(doc, msg);
                while (error) {
                    if (strcmp(error.c_str(), "NoMemory") != 0) {
                        MTT_error("%s", "JSON deserialization memory issue\n");
                        break;
                    } else {
                        doc.clear();
                    }
                    
                    size = size * 2;
                    doc = ArduinoJson::DynamicJsonDocument(size);
                    error = deserializeJson(doc, msg);
                }
                
                const MTT_MESSAGE_TYPE type = lookup[(doc["type"])];
                switch (type) {
                    case MTT_MESSAGE_TYPE_MTT_INIT: {
                        auto* core = mtt_core_ctx();
                        
                        {
                            mtt::String msg = "{\"type\":\"MTT_INIT\",\"w\":" + std::to_string(core->viewport.width) + ",\"h\":" + std::to_string(core->viewport.height) + "}\r\n";
                            cstring as_cstr = msg.c_str();
                            
                            //MTT_print("SENDING BACK REQUEST ID: %s\n", as_cstr);
                            
                            mtt::External_World_reset(mtt::curr_external_world(mtt_world));
                            Extension_Server_write(&core->server_ext,
                                                   as_cstr, msg.size());
                            break;
                        }
                        break;
                    }
                    case MTT_MESSAGE_TYPE_REQUEST_ID: {
                        auto& dt_ctx = *dt::DrawTalk::ctx();
                        
                        for (auto& selection : dt_ctx.recorder.selection_recording.selections) {
                            auto thing_id = selection.ID;
                            Thing* thing = mtt::Thing_try_get(mtt_world, thing_id);
                            if (thing == nullptr || !(thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH || thing->archetype_id == mtt::ARCHETYPE_NUMBER || thing->archetype_id == mtt::ARCHETYPE_TEXT)) {
                                continue;
                            }
                            
                            
                            
                            mtt::External_Thing* ext_thing = mtt::External_Thing_map(mtt::curr_external_world(mtt::world(thing)), thing);
                            MTT_UNUSED(ext_thing);
                            //                            if (mtt_world->externally_linked_things.size() == prev_count) {
                            //                                break;
                            //                            }
                            {
                                mtt::String msg = "{\"type\":\"MTT_REQUEST_ID\",\"id\":" + std::to_string(thing->id) + ",\"reqID\":" + std::to_string(doc["reqID"].as<uint32>()) + "}\r\n";
                                cstring as_cstr = msg.c_str();
                                
                                //MTT_print("SENDING BACK REQUEST ID: %s\n", as_cstr);
                                
                                Extension_Server_write(&mtt_core_ctx()->server_ext,
                                                       as_cstr, msg.size());
                                break;
                            }
                        }
                        break;
                    }
                    case MTT_MESSAGE_TYPE_PING: {
                        break;
                    }
                    case MTT_MESSAGE_TYPE_RESET: {
                        mtt::External_World_reset(mtt::curr_external_world(mtt_world));
                        MTT_print("%s", "MTT_MESSAGE_TYPE_RESET received\n");
                        break;
                    }
                    default: {
                        MTT_print("%s", "ERROR: unknown MTT_MESSAGE_TYPE\n");
                        break;
                    }
                }
                
                
                doc.clear();
            };
        }
        
        //mtt::Thing_destroy(particle_thing);
        //mtt::Thing_destroy(ref);
        //        dt::Speech_Property* R = new dt::Speech_Property();
        //        for (usize i = 0; i < 3; i += 1) {
        //            auto* sp = R->push_prop("WEE");
        //            sp->label = std::to_string(i);
        //            if (i == 0 || i == 2) {
        //                sp->parent_ref_disabled = true;
        //            }
        //        }
        //
        //        for (auto sp: R->get_active("WEE")) {
        //            sp->print();
        //        }
        //        MTT_BP();
        
#define TEST_THING_TYPE_INIT (0)
        
#if TEST_THING_TYPE_INIT && !defined(NDEBUG)
        for (usize i = 0; i < mtt::THING_TYPE_COUNT; i += 1) {
            auto* t = mtt::Thing_make(&(world->mtt_world), (mtt::THING_TYPE)i);
            auto* tc = mtt::Thing_copy(t);
            auto* tc2 = mtt::Thing_copy(tc);
            mtt::Thing_destroy(t);
            mtt::Thing_destroy(tc);
            mtt::Thing_destroy(tc2);
        }
#endif
        
        core->dl_ctx.core = core;
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
            mtt::MTT_API_init(&core->dl_ctx.mtt_api);
            sd::Renderer_API_init(&core->dl_ctx.sd_api);
#else
        ext::setup(&core->dl_ctx);
#endif

#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        subscribe_message("dyl", [](void* ctx, void* data) {
            MTT_Core* core = (MTT_Core*)ctx;
            mtt::String* version = (mtt::String*)data;
            init_dynamic_load(core, *version);
        }, core);
#endif
        
        set_net_message_handler([](void* msg, usize length, void* ctx) {
//            mtt::String str = "EXTO" + *((mtt::String*)msg);
//            const char* as_c_str = str.c_str();
            
            mtt::String str = R"(EXTO{type:msg, msg:"hi!"})";
            cstring as_c_str = str.c_str();
            
            MTT_log_debug("%s\n", as_c_str);
            
            net_write((void*)as_c_str, str.size());
        }, core);
        
        core->on_application_focus = [](MTT_Core* core) {
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
//            {
//                core->library("on_unload", &core->dl_ctx);
//            }
//            MTT_Dynamic_Library_close(&core->library);
#endif
        };

        
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        send_message_drawtalk_server_quick_command(core, "dyld", "");
#endif

        
    } else if (MTT_PLATFORM == MTT_PLATFORM_TYPE_WASM) {
        MTT_print("%s\n", "PLATFORM_WASM");
    }
    
    
    
    
    MTT_Core_resume(core);
}

sd::Drawable_Info* rewind_cmd_test = nullptr;




// MARK: - Frame update


vec3 integrate(float32 timestep, vec3* velocity, vec3* acceleration);
vec3 integrate(float32 timestep, vec3* velocity, vec3* acceleration)
{
    *velocity += (*acceleration) * timestep * mtt::MOVEMENT_SCALE;
    vec3 translation = (*velocity) * timestep * mtt::MOVEMENT_SCALE;
    return translation;
}

vec3 integrate(float32 timestep, vec3 velocity, vec3 acceleration);
vec3 integrate(float32 timestep, vec3 velocity, vec3 acceleration)
{
    return integrate(timestep, &velocity, &acceleration);
}

std::vector<char> buf = {};
bool run_animation = true;

DEFINE_MTT_DLL_CALL

bool cam_info_choose(DrawTalk_World* world, vec2 canvas_pos, mtt::Camera** camera_out)
{
    auto* mtt_world = &world->mtt_world;
    auto* dt = &world->dt;
    
    bool map_view_selected = dt::map_view_selected(mtt_world, &world->dt, canvas_pos);
    
    if (map_view_selected) {
        *camera_out = &dt->ui.top_panel.cam;
    } else {
        *camera_out = &world->cam;
    }
    
    return map_view_selected;
}

bool collision_system_info_choose(DrawTalk_World* world, vec2 canvas_pos, mtt::Collision_System** canvas_system_out, mtt::Collision_System** world_system_out, mtt::Camera** camera_out)
{
    auto* mtt_world = &world->mtt_world;
    auto* dt = &world->dt;
    
    bool map_view_selected = dt::map_view_selected(mtt_world, &world->dt, canvas_pos);
    
    Collision_System_Group_World_Canvas* view_collision_sys_group;
    if (map_view_selected) {
        MTT_print("map_view_selected=[%s]\n", bool_str(map_view_selected));
        
        
        view_collision_sys_group = dt::map_view_collision_system_group(dt);
        *camera_out = &dt->ui.top_panel.cam;
    } else {
        view_collision_sys_group = &mtt_world->collision_system_group;
        *camera_out = &world->cam;
    }
    
    *world_system_out = view_collision_sys_group->world;
    *canvas_system_out = view_collision_sys_group->canvas;
    
    return map_view_selected;
}

float32 max_x_bottom_left_panel = 0.0f;
struct Pause_Trigger {
    vec2 side_length = vec2(32.0f);
    vec2 corner_offset = vec2(8.0f);
    
    vec2 tl = {};
    vec2 br = {};
    inline vec4 corners(vec2 bottom_corner)
    {
        vec2 br = bottom_corner - corner_offset;
        vec2 tl = br - side_length;
        this->tl = tl;
        this->br = br;
        
        return vec4(tl, br);
    }
} pause_trigger = {};
#define PAUSE_TRIGGER_IN_CORNER (0)

#ifndef NDEBUG
std::vector<dt::Rule_Test> continuous_rule_tests = {};
bool continuous_rule_tests_enabled = false;
#endif

void on_frame(MTT_Core* core)
{
#ifndef NDEBUG
    MTT_set_default_active_logger();
#endif
    //#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    //    dt::drew_this_frame = false;
    //#endif
    MTT_user_test_log_write_enqueued();
    MTT_user_test_log_flush();
    
    if (!run_animation) {
        return;
    }
    
    //usize count = 0;
    
    DrawTalk_World* world = static_cast<DrawTalk_World*>(core->user_data);
    
    world->render_info.debug_grid_is_on = true;
    
    const float64 time = core->time_seconds;
    Input* const input = &core->input;
    
    core->time_sim_elapsed_ns = m::min(mtt_s_to_ns(1), core->time_sim_elapsed_ns);
    
    world->dt.scn_ctx.long_press_this_frame = false;

    //    {
    //        auto screen_tex_id = world->render_info.remote_screen->texture_ID;
    //    }
    //vec2 ROT = m::rotate(vec2(1.0f, 0.0f), (float32)time);
    
    //    std::cout << "ROT:" << m::to_string(ROT) << std::endl;
    //    std::cout << "AS MAT:" << m::to_string(m::rotate(Mat4(1.0f), (float32)time, vec3(0.0f, 0.0f, 1.0f))) << std::endl;
    //
    
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    
    if (enable_video && mtt::Video_Recording_is_init() &&
        !mtt::Video_Recording_has_device(core->screen_recording)) {
        mtt::Video_Recording_discover_devices(core->screen_recording);
        if (mtt::Video_Recording_has_device(core->screen_recording)) {
            mtt::Video_Recording_start(core->screen_recording);
        }
    }
#endif
    
    
    //logic();
    //Speech_Timestamp_print(core->speech_system.timestamp);
    
    sd::Renderer* renderer = core->renderer;
    
    //world->mtt_world.time_seconds = core->time_seconds;
    
    world->main_projection = mtt::orthographic_projection_matrix(0.0, core->viewport.width, core->viewport.height, 0.0, -1000.0, 1000.0);
    //    world->projection = mtt::orthographic_projection_matrix_scale(0.0, core->viewport.width, core->viewport.height, 0.0, -1000.0, 1000.0, 1.0, &world->scaled_viewport);
    
    
    
    sd::rewind_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD);
    sd::rewind_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD_2);
    sd::rewind_layer(renderer, LAYER_LABEL_PER_FRAME);
    sd::rewind_layer(renderer, LAYER_LABEL_CANVAS_PER_FRAME);
    sd::rewind_layer(renderer, LAYER_LABEL_BACKGROUND_CANVAS);
    sd::rewind_layer(renderer, LAYER_LABEL_DYNAMIC_CANVAS);
    sd::rewind_layer(renderer, LAYER_LABEL_DRAWTALK_DYNAMIC_PER_FRAME);
    sd::rewind_layer(renderer, LAYER_LABEL_DRAWTALK_DYNAMIC_PER_FRAME_UI);
    sd::rewind_layer(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER_PER_FRAME);
    
    mtt::World* mtt_world = &world->mtt_world;
    mtt::frame_begin(mtt_world, true);
    
    vec2 off = {10.0f, -5.0f};
    color_current_tl = vec2(-0.4f) + off + vec2(0.0, core->viewport.height - color_current_height_off);
    color_current_dim = (vec2){25.0f, 25.0f} + vec2(0.8f);
    color_current_br = color_current_tl + color_current_dim;
    color_changed_this_frame = false;
    
    
    
    if (sd::texture_is_valid(renderer, world->render_info.bg_texture_id)) {
        sd::set_render_layer(renderer, LAYER_LABEL_BACKGROUND_CANVAS);
        sd::save(renderer);
        sd::set_color_rgba_v4(renderer, color::WHITE);
        sd::begin_polygon(renderer);
        
        sd::Rectangle container;
        container.x      = core->viewport.x;
        container.y      = core->viewport.y;
        container.width  = core->viewport.width;
        container.height = core->viewport.height;
        
        sd::Rectangle to_fit;
        to_fit.x = 0;
        to_fit.y = 0;
        to_fit.width = world->render_info.bg_texture_dimensions.x * .6f;
        to_fit.height = world->render_info.bg_texture_dimensions.y * .6f;
        
        sd::Rectangle result;
        sd::fit_rect_centered(&container, &to_fit, &result, 0.5f);
        
        sd::rectangle(renderer, vec2(result.x, result.y), vec2(result.width, result.height), -999);
        auto info = sd::end_polygon(renderer);
        info->set_texture_ID(world->render_info.bg_texture_id);
        //info->set_transform(m::scale(Mat4(1.0f), vec3(0.5f, 0.5, 1.0f)));
        sd::restore(renderer);
    }
    vec2 most_recent_canvas_pos_something__ = vec2(0.0f);
    bool something_touchdown__ = false;
    
    // MARK: - input
    static mtt::Set_Stable<mtt::Thing_ID> preview_selected_things = {};
    {
        dt::Control_System* control = &world->controls;
        UI_Event event;
        Input_Record* u_input = &input->users[0];
        new_points_arrived = false;
        UI_Touch* touch = nullptr;
        
        
        
#define TRANSFORM(point) (point + core->view_position)
        
#define PR(...) MTT_print(__VA_ARGS__ "\n")
        while (Input_poll_event(u_input, &event)) {
            //PR("EVENT");
            /*while (1) */ {
                switch (event.input_type) {
                        // MARK: handle the stylus
                    case UI_TOUCH_TYPE_POINTER: {
                        //PR("PENCIL");
                        float32 pixel_radius = 2;
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                        switch (event.state_type) {
                            case UI_HOVER_STATE_BEGAN: {
                                //MTT_print("hover began: %f\n", event.z_position);
                                auto& hover = u_input->hover;
                                hover.state = event.state_type;
                                hover.azimuth = event.pointer_info.azimuth;
                                hover.altitude_angle_radians = event.pointer_info.altitude_angle_radians;
                                hover.azimuth_unit_vector = event.pointer_info.azimuth_unit_vector;
                                hover.pos = event.position;
                                hover.z_pos = event.z_position;
                                continue;
                                //break;
                            }
                            case UI_HOVER_STATE_CHANGED: {
                                //MTT_print("hover changed: %f\n", event.z_position);
                                auto& hover = u_input->hover;
                                hover.state = event.state_type;
                                hover.azimuth = event.pointer_info.azimuth;
                                hover.altitude_angle_radians = event.pointer_info.altitude_angle_radians;
                                hover.azimuth_unit_vector = event.pointer_info.azimuth_unit_vector;
                                hover.pos = event.position;
                                hover.z_pos = event.z_position;
                                continue;
                                //break;
                            }
                            case UI_HOVER_STATE_CANCELLED: {
                                //MTT_print("hover changed: %f\n", event.z_position);
                                auto& hover = u_input->hover;
                                hover.state = event.state_type;
                                hover.azimuth = event.pointer_info.azimuth;
                                hover.altitude_angle_radians = event.pointer_info.altitude_angle_radians;
                                hover.azimuth_unit_vector = event.pointer_info.azimuth_unit_vector;
                                hover.pos = event.position;
                                hover.z_pos = event.z_position;
                                continue;
                            }
                            case UI_HOVER_STATE_ENDED: {
                                //MTT_print("hover changed: %f\n", event.z_position);
                                auto& hover = u_input->hover;
                                hover.state = event.state_type;
                                hover.azimuth = event.pointer_info.azimuth;
                                hover.altitude_angle_radians = event.pointer_info.altitude_angle_radians;
                                hover.azimuth_unit_vector = event.pointer_info.azimuth_unit_vector;
                                hover.pos = event.position;
                                hover.z_pos = event.z_position;
                                
                                //MTT_print("hover ended: %f\n", event.z_position);
                                continue;
                                //break;
                            }
                            default: {
                                break;
                            }
                        }
#endif
                        
//                        auto find_it = u_input->pointer_map.find(event.key);
//                        if (find_it != u_input->pointer_map.end()) {
//                            touch = &find_it->second;
//                        }
                        
                        UI_Touch_Pointer* pointer = nullptr;
                        auto result = u_input->pointer_map.find(event.key);
                        if (result != u_input->pointer_map.end()) {
                            touch = &result->second;
                            pointer = &touch->pointer;
                        }
                        
                        
                        switch (control->stylus.mode) {
                            case dt::SP_STYLUS_MODE_NONE: {
                                control->stylus.mode = dt::SP_STYLUS_MODE_DRAW;
                                control->stylus.base.active_state = dt::SP_CONTROL_ACTIVE_STATE_ON;
                                
                                
                                
                                
                                
                                // fallthrough
                            }
                            case dt::SP_STYLUS_MODE_DRAW: {
                                //PR("\tMODE_DRAW");
                                world->dt.pen.pixel_radius = pixel_radius;// * m::max(0.5, pointer->force) * 2.0f;
                                sd::path_radius(renderer, world->dt.pen.pixel_radius);
                                
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
#define ADJUSTED_RADIUS(base, radius, multiplier) (base)
#else
#define ADJUSTED_RADIUS(base, radius, multiplier) m::clamp(((float64)base * m::max((float64)0.125, (float64)radius) * (float64)2.0) * (float64)multiplier, 0.4, 4.0)
#endif
                                
                                switch (event.state_type) {
                                    case UI_TOUCH_STATE_BEGAN: {
                                        //touch->user_data = (void*)world->input_info.next_sequence_number;
                                        //world->input_info.next_sequence_number += 1;
                                        
                                        pointer_op_set_in_progress(u_input, pointer_op_current(u_input));
                                        thing_arrows.is_active = false;
                                        
                                        
                                        
//                                        touch->user_data_2 = (void*)cam;
//                                        touch->user_data_3 = nullptr; // collision system
                                        
                                        
                                        
                                        
                                        
                                        static Collider_List out;
                                        out.clear();
                                        static Collider_List out_canvas;
                                        out_canvas.clear();
                                        
                                        bool was_hit = false;
                                        
                                        mtt::Camera* cam = nullptr;
                                        mtt::Collision_System* collision_system_canvas = nullptr;
                                        mtt::Collision_System* collision_system_world = nullptr;
                                        
                                        vec2 canvas_pos = pointer->positions[pointer->count - 1];
                                        most_recent_canvas_pos_something__ = canvas_pos;
                                        something_touchdown__ = true;

                                        if (pointer_op_in_progress(u_input) == UI_POINTER_OP_DRAW) {
                                            if ( !(canvas_pos.x < color_current_tl.x || canvas_pos.x > color_current_br.x || canvas_pos.y < color_current_tl.y || canvas_pos.y > color_current_br.y )) {
                                                
                                                color_changed_this_frame = true;
                                                
                                                auto color_thing = [&](mtt::Thing* thing_to_color) {
                                                    auto* r = mtt::rep(thing_to_color);
                                                    auto& dr_list = r->render_data.drawable_info_list;
                                                    for (usize d_i = 0; d_i < dr_list.size(); d_i += 1) {
                                                        auto* dr_info = dr_list[d_i];
                                                        dr_info->set_color_factor(SD_vec4(0.0f, 0.0f, 0.0f, 0.0f));
                                                        dr_info->set_color_addition(world->dt.pen.color);
                                                    }
                                                };
                                                
                                                for (auto it = world->dt.scn_ctx.selected_things.begin(); it != world->dt.scn_ctx.selected_things.end(); ++it) {
                                                    
                                                    mtt::Thing* thing_to_color = nullptr;
                                                    if (!world->mtt_world.Thing_try_get(*it, &thing_to_color)) {
                                                        break;
                                                    }
                                                    
                                                    color_thing(thing_to_color);
                                                }
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
                                                for (auto it = world->dt.recorder.selection_recording.selections.begin(); it != world->dt.recorder.selection_recording.selections.end(); ++it) {
                                                    mtt::Thing* thing_to_color = nullptr;
                                                    if (!world->mtt_world.Thing_try_get(it->ID, &thing_to_color)) {
                                                        break;
                                                    }
                                                    
                                                    color_thing(thing_to_color);
                                                }
#endif
                                                if (mtt::Thing_ID_is_valid(world->dt.scn_ctx.thing_selected_drawable)) {
                                                    auto* t = mtt::Thing_try_get(&world->mtt_world, world->dt.scn_ctx.thing_selected_drawable);
                                                    if (mtt::Thing_is_valid(t)) {
                                                        color_thing(t);
                                                    }
                                                }
                                                MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                          STRLN("{")
                                                                          STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                          STRIDTLN(STRMK("kind") ":" STRMK("stylus") ",")
                                                                          STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                          STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                          STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])))
                                                                          );
                                                break;
                                            }
                                        }
                                        
                                        collision_system_info_choose(world, canvas_pos, &collision_system_canvas, &collision_system_world, &cam);
                                        
                                        Input_set_user_data_for_touch(u_input, touch, cam);
                                        
                                        vec2 transformed_pos__ = mtt::transform_point(cam, canvas_pos);
                                        
                                        
                                        
                                        vec2 input_pos = transformed_pos__;
                                        
                                        MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                  STRLN("{")
                                                                  STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                  STRIDTLN(STRMK("kind") ":" STRMK("stylus") ",")
                                                                  STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                  STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                  STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])) + ",")
                                                                  STRIDTLN(STRMKPAIR("canvas_pos", STRV2(canvas_pos)) + ",")
                                                                  STRIDTLN(STRMKPAIR("world_pos", STRV2(input_pos)))
                                                                  STRLN("},")
                                                                  );
                                        
                                        
                                        bool broad_hit_canvas = point_query(collision_system_canvas, 0, canvas_pos, &out_canvas);
                                        
                                        if (broad_hit_canvas) {
                                            Point point;
                                            point.coord = canvas_pos;
                                            
                                            mtt::Thing_ID thing_ID = mtt::Thing_ID_INVALID;
                                            mtt::Thing* thing = nullptr;
                                            
                                            Collider* c_hit = nullptr;
                                            
                                            bool narrow_hit = false;
                                            Hit hit;
                                            
                                            float32 min_area = POSITIVE_INFINITY;
                                            
                                            for (auto it = out_canvas.cbegin(); it != out_canvas.cend(); ++it) {
                                                mtt::Collider* c = *it;
                                                if (!mtt::priority_layer_is_equal(c->priority_layer, world->mtt_world.priority_layer)) {
                                                    continue;
                                                }
                                                switch ((*it)->type) {
                                                    case mtt::COLLIDER_TYPE_AABB: {
                                                        AABB mod = c->aabb;
                                                        
                                                        
                                                        
                                                        mod.tl = mod.saved_box.tl;
                                                        mod.br = mod.saved_box.br;
                                                        //if (!mtt::AABB_Point_intersection(&mod, &point, &hit)) {
                                                        if (!mtt::Point_Quad_intersection(&point, &c->aabb.saved_quad)) {
                                                            break;
                                                        }
                                                        
                                                        float32 area = calc_aabb_area(&mod);
                                                        if (area >= min_area) {
                                                            break;
                                                        }
                                                        min_area = area;
                                                        
                                                        thing_ID = (Thing_ID)c->user_data;
                                                        thing = mtt::Thing_from_ID(&world->mtt_world, thing_ID);
                                                        
                                                        if (thing == nullptr || !mtt::is_active(thing)) {
                                                            MTT_error("WARNING: this collider should no longer exist... this should be impossible %s %d\n", __FILE__, __LINE__);
                                                            continue;
                                                        }
                                                        
                                                        
                                                        
                                                        narrow_hit = true;
                                                        c_hit = c;
                                                        
                                                        break;
                                                    }
                                                    default: {
                                                        break;
                                                    }
                                                }
                                            }
                                            was_hit = narrow_hit;
                                            
                                            
                                            if (was_hit && ((thing->archetype_id != mtt::ARCHETYPE_FREEHAND_SKETCH) || mtt::Thing_is_locked(thing) || !mtt::is_user_drawable(thing) || !mtt::is_active(thing))) {
                                                thing->input_handlers.do_on_pen_input_began(thing, vec3(transformed_pos__, 0.0f), canvas_pos, &event, touch, 0);
                                                
                                                world->dt.scn_ctx.thing_selected_with_pen = thing_ID;
                                                break;
                                                
                                            }
                                            was_hit = false;
                                            
                                        }
                                        
                                        bool broad_hit = point_query(collision_system_world, 0, input_pos, &out);
                                        
                                        Collider* c_hit = nullptr;
                                        
                                        Thing_ID thing_ID = mtt::Thing_ID_INVALID;
                                        mtt::Thing* thing = nullptr;
                                        
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                                        auto handle_hierarchy_op = [](mtt::Thing* thing) {
                                        };
#else
                                        auto handle_hierarchy_op = [](mtt::Thing* thing) {
                                            mtt::Thing* parent_to_connect = thing;
                                            if (mtt::Thing_is_proxy(parent_to_connect)) {
                                                parent_to_connect = mtt::Thing_mapped_from_proxy(parent_to_connect);
                                            }
                                            //mtt::Thing* child_to_connect = nullptr;
                                            auto& dt_ctx = *dt::DrawTalk::ctx();
                                            if (!dt_ctx.scn_ctx.selected_things.empty()) {
                                                for (const auto thing_id : dt_ctx.scn_ctx.selected_things) {
                                                    //for (auto it = dt_ctx.scn_ctx.selected_things.begin(); it != dt_ctx.scn_ctx.selected_things.end(); ++it) {
                                                    mtt::Thing* sel_thing = thing->world()->Thing_try_get(thing_id);
                                                    if (!dt::can_label_thing(sel_thing) && sel_thing->archetype_id != mtt::ARCHETYPE_TEXT) {
                                                        continue;
                                                    }
                                                    
                                                    mtt::Thing* child = sel_thing;
                                                    if (mtt::Thing_is_proxy(child)) {
                                                        child = mtt::Thing_mapped_from_proxy(child);
                                                    }
                                                    if (child->archetype_id == mtt::ARCHETYPE_NUMBER && parent_to_connect->archetype_id == mtt::ARCHETYPE_TEXT) {
                                                        continue;
                                                    }
                                                    
                                                    if ((parent_to_connect->archetype_id == mtt::ARCHETYPE_TEXT || parent_to_connect->archetype_id == mtt::ARCHETYPE_NUMBER) &&
                                                        (child->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH || child->archetype_id == mtt::ARCHETYPE_PARTICLE_SYSTEM || child->archetype_id == mtt::ARCHETYPE_NODE_GRAPH)) {
                                                        continue;
                                                    }
                                                    
                                                    if (child == thing) {
                                                        if (dt_ctx.scn_ctx.selected_things.size() == 1) {
                                                            mtt::disconnect_child_from_parent(child->world(), child);
                                                            break;
                                                        } else {
                                                            continue;
                                                        }
                                                    }
                                                    
//                                                    if (mtt::Thing_is_proxy(child) != mtt::Thing_is_proxy(parent_to_connect)) {
//                                                        continue;
//                                                    }
                                                    
                                                    // FIXME: probably does not handle all cases
                                                    if (mtt::get_parent_ID(child) != mtt::Thing_ID_INVALID) {
                                                        if (mtt::get_parent_ID(child) == parent_to_connect->id) {
                                                            mtt::disconnect_child_from_parent(child->world(), child);
                                                            break;
                                                        } else {
                                                            mtt::disconnect_child_from_parent(child->world(), child);
                                                        }
                                                    } else {
                                                        if (mtt::is_ancestor_of(child, parent_to_connect)) {
                                                            mtt::disconnect_parent_from_children(child->world(), child);
                                                        }
                                                    }
                                                    
                                                    
                                                    mtt::connect_parent_to_child(child->world(), parent_to_connect, child);
                                                    
                                                    break;
                                                }
                                            }
                                        };
#endif
                                        
                                        if (broad_hit && !USE_OLD_ADD_TO_DRAWING_METHOD) {
                                            
                                            Point point;
                                            point.coord = input_pos;
                                            
                                            bool narrow_hit = false;
                                            Hit hit;
                                            
                                            float32 min_area = POSITIVE_INFINITY;
                                            
                                            for (auto it = out.cbegin(); it != out.cend(); ++it) {
                                                mtt::Collider* c = *it;
                                                if (!mtt::priority_layer_is_equal(c->priority_layer, world->mtt_world.priority_layer)) {
                                                    continue;
                                                }
                                                switch ((*it)->type) {
                                                    case mtt::COLLIDER_TYPE_AABB: {
                                                        AABB mod = c->aabb;
                                                        mod.tl = mod.saved_box.tl;
                                                        mod.br = mod.saved_box.br;
                                                        
                                                        
                                                        //if (!mtt::AABB_Point_intersection(&mod, &point, &hit)) {
                                                        if (!mtt::Point_Quad_intersection(&point, &c->aabb.saved_quad)) {
                                                            break;
                                                        }
                                                        
                                                        float32 area = calc_aabb_area(&mod);
                                                        if (area >= min_area) {
                                                            break;
                                                        }
                                                        
                                                        
                                                        thing_ID = (Thing_ID)c->user_data;
                                                        thing = mtt::Thing_from_ID(&world->mtt_world, thing_ID);
                                                        
                                                        if (world->dt.scn_ctx.thing_selected_drawable) {
                                                            if (thing->is_user_drawable) {
                                                                break;
                                                            }
                                                        }
                                                        
                                                        
                                                        min_area = area;
                                                        
                                                        
                                                        if (thing == nullptr) {
                                                            MTT_error("WARNING: this collider should no longer exist... this should be impossible %s %d\n", __FILE__, __LINE__);
                                                            continue;
                                                        }
                                                        
                                                        //                                        if (mtt::Thing_is_locked(thing) || !thing->is_user_drawable || !thing->is_active) {
                                                        //                                            break;
                                                        //                                        }
                                                        
                                                        narrow_hit = true;
                                                        c_hit = c;
                                                        
                                                        break;
                                                    }
                                                    default: { break; }
                                                }
                                            }
                                            was_hit = narrow_hit;
                                            
                                            if (was_hit && ((thing->archetype_id != mtt::ARCHETYPE_FREEHAND_SKETCH) || mtt::Thing_is_locked(thing) || !mtt::is_user_drawable(thing) || !mtt::is_active(thing))) {
                                                
                                                
                                                
                                                if (event.flags == UI_EVENT_FLAG_CONTEXT_MODE && was_hit && ((mtt::is_actor(thing)))) {
                                                    handle_hierarchy_op(thing);
                                                }
                                                thing->input_handlers.do_on_pen_input_began(thing, vec3(transformed_pos__, 0.0f), canvas_pos, &event, touch, 0);
                                                
                                                world->dt.scn_ctx.thing_selected_with_pen = thing_ID;
                                                
                                                if (mtt::flag_is_set(thing, mtt::THING_FLAG_is_user_erasable)) {
                                                    switch (pointer_op_in_progress(u_input)) {
                                                        case UI_POINTER_OP_ERASE: {
                                                            usize select_count = selection_count(thing);
                                                            if (select_count == 0) {
                                                                if (mtt::Thing_is_proxy(thing)) {
                                                                    mtt::Thing_destroy(mtt_world, thing->mapped_thing_id);
                                                                } else {
                                                                    mtt::Thing_destroy(thing);
                                                                }
                                                            }
                                                            
                                                            //                                                Erase_Tool_Args args;
                                                            //                                                args.renderer = renderer;
                                                            //                                                mtt::erase_tool_began(u_input, thing, vec3(transformed_pos__, 0.0f), canvas_pos__, &event, touch, 0, (void*)&args);
                                                            break;
                                                        }
                                                    }
                                                    
                                                }
                                                break;
                                                
                                            }
                                        }
                                        
                                        if ((!was_hit && world->dt.scn_ctx.thing_most_recently_selected_with_touch != mtt::Thing_ID_INVALID) || ((was_hit && thing_ID != world->dt.scn_ctx.thing_most_recently_selected_with_touch) && (world->dt.scn_ctx.thing_most_recently_selected_with_touch != mtt::Thing_ID_INVALID))) {
                                            
                                            thing_ID = world->dt.scn_ctx.thing_most_recently_selected_with_touch;
                                            thing = world->mtt_world.Thing_try_get(thing_ID);
                                            if (thing == nullptr || thing->archetype_id != mtt::ARCHETYPE_FREEHAND_SKETCH) {
                                                
                                                break;
                                            }
                                            
                                            was_hit = true;
                                            
                                            
                                            if (mtt::rep(thing)->colliders.empty()) {
                                                break;
                                            }
                                            
                                            c_hit = mtt::rep(thing)->colliders.front();
                                            
                                            
                                        }
                                        
                                        
                                        
                                        
                                        if (event.flags == UI_EVENT_FLAG_CONTEXT_MODE) {
                                            if (thing == nullptr) {
                                                break;
                                            }
                                            handle_hierarchy_op(thing);
                                            break;
                                        }
                                        
                                        {
                                            const float32 bottom_panel_height = dt::bottom_panel_height;
                                            if (canvas_pos.y > (core->viewport.height - (bottom_panel_height))) {
                                                auto* dt_ctx = dt::DrawTalk::ctx();
                                                auto& dock = dt_ctx->ui.dock;
                                                auto* button  = dock.label_to_button["color"];
                                                //auto& label_to_config = dock.label_to_config;
                                                //auto* conf = &label_to_config[button->label];
                                                mtt::Thing* thing = dt_ctx->mtt->Thing_get(button->thing);
                                                mtt::Rep* rep = mtt::rep(thing);
                                                auto& bound = rep->colliders.back()->aabb.saved_box;
                                                if (canvas_pos.x > m::max(max_x_bottom_left_panel, bound.br.x)) {
                                                    {
                                                        
                                                        // tapped background
                                                        //
//                                                                dt::Selection_Recorder_clear_selections(&world->dt.recorder.selection_recording);
//
//                                                                control->most_recent_bg_touch_canvas      = vec3(canvas_pos, 0.0f);
//                                                                control->most_recent_bg_touch_transformed = vec3(transformed_pos, 0.0f);
//
//                                                                auto* dt_ctx = dt::DrawTalk::ctx();
//                                                                if ((true) || (dt::get_confirm_phase() != dt::CONFIRM_PHASE_ANIMATE && !dt_ctx->is_waiting_on_results)) {
//                                                                    dt::record_location_selection(dt_ctx, vec3(transformed_pos, 0.0f), canvas_pos, core->time_milliseconds, event.timestamp);
//                                                                }
//
//                                                                world->dt.scn_ctx.thing_selected_drawable = mtt::Thing_ID_INVALID;
                                                        break;
                                                    }
                                                } else if (canvas_pos.x < bound.tl.x || canvas_pos.y < bound.tl.y
                                                        || canvas_pos.x > bound.br.x || canvas_pos.y > bound.br.y) {
                                                    break;
                                                }
                                            }
                                        }
                                        
                                        bool USE_AUTO_SELECT_WITH_PEN = (pointer_op_in_progress(u_input) != UI_POINTER_OP_DRAW);
                                        bool thing_selected_drawable_is_valid = mtt::Thing_ID_is_valid(world->dt.scn_ctx.thing_selected_drawable);
                                        if (
                                            (thing_selected_drawable_is_valid && !was_hit) ||
                                            (USE_AUTO_SELECT_WITH_PEN && (thing_selected_drawable_is_valid || was_hit)) ||
                                            (USE_OLD_ADD_TO_DRAWING_METHOD && was_hit)
                                            ) {
                                                
                                                Collider* collider = c_hit;
                                                
                                                auto prev_selected_with_pen = world->dt.scn_ctx.thing_selected_with_pen;
                                                auto prev_selected_drawable = world->dt.scn_ctx.thing_selected_drawable;
                                                world->dt.scn_ctx.thing_selected_with_pen = thing_ID;
                                                
                                                if (!USE_OLD_ADD_TO_DRAWING_METHOD && thing_selected_drawable_is_valid) {
                                                    thing = world->mtt_world.Thing_try_get(world->dt.scn_ctx.thing_selected_drawable);
                                                    
                                                    collider = mtt::rep(thing)->colliders.front();
                                                    world->dt.scn_ctx.thing_selected_with_pen = thing->id;
                                                    
                                                } else {
                                                    world->dt.scn_ctx.thing_selected_drawable = thing_ID;
                                                }
                                                
                                                
                                                thing->saved_parent_thing_id = thing->parent_thing_id;
                                                if (thing->saved_parent_thing_id != mtt::Thing_ID_INVALID) {
                                                    mtt::disconnect_child_from_parent(thing->world(), thing);
                                                }
                                                
                                                if (pointer_op_in_progress(u_input) != UI_POINTER_OP_DRAW) {
                                                    switch (pointer_op_in_progress(u_input)) {
                                                        case UI_POINTER_OP_ERASE: {
                                                            if (!mtt::flag_is_set(thing, THING_FLAG_is_user_erasable) && thing->is_user_destructible) {
                                                                break;
                                                            }
                                                            
                                                            usize select_count = selection_count(thing);
                                                            //auto selected_drawable = world->dt.scn_ctx.thing_selected_drawable;
                                                            //auto prev_selected_with_pend = prev_selected_with_pen;
                                                            if (prev_selected_drawable != thing->id && select_count == 0) {
                                                                mtt::Thing_destroy(thing);
                                                                break;
                                                            }
                                                            
                                                            Erase_Tool_Args args;
                                                            args.renderer = renderer;
                                                            mtt::erase_tool_began(u_input, thing, vec3(transformed_pos__, 0.0f), canvas_pos, &event, touch, 0, (void*)&args);
                                                            break;
                                                        }
                                                        case UI_POINTER_OP_ARROW_CREATE: {
                                                            if (!Camera_is_drawable(cam) || mtt::Thing_is_proxy(thing)) {
                                                                break;
                                                            }
                                                            
                                                            thing_arrows.is_active = true;
                                                            thing_arrows.clear();
                                                            thing_arrows.color = vec4(0.8f, 0.8f, 0.8f, 1.0f);
                                                            thing_arrows.radius = 1.0f;
                                                            thing_arrows.push(vec3(transformed_pos__, 0.0f));
                                                            thing_arrows.deferred.clear();
                                                            thing_arrows.prev_in_chain = thing->id;
                                                            thing_arrows.input_key = event.key;
                                                            thing_arrows.canvas_pos = canvas_pos;
                                                            break;
                                                        }
                                                    }
                                                } else {
                                                    
                                                    if (!Camera_is_drawable(cam) || !mtt::is_user_drawable(thing)) {
//                                                        Input_set_user_data_for_touch(u_input, touch, &world->cam);
//                                                        collision_system_world = &world->mtt_world.collision_system;
//                                                        collision_system_canvas = &world->mtt_world.collision_system_canvas;
//                                                        cam = &world->cam;
//                                                        transformed_pos__ = mtt::transform_point(cam, canvas_pos);
                                                        world->dt.scn_ctx.thing_selected_drawable = mtt::Thing_ID_INVALID;
                                                        world->dt.scn_ctx.thing_selected_with_pen = mtt::Thing_ID_INVALID;
                                                        if (thing->saved_parent_thing_id != mtt::Thing_ID_INVALID) {
                                                            mtt::Thing* parent = nullptr;
                                                            if (thing->world()->Thing_try_get(thing->saved_parent_thing_id, &parent)) {
                                                                mtt::connect_parent_to_child(thing->world(), parent, thing);
                                                            }
                                                        }
                                                        
                                                        break;
                                                    }
                                                    
                                                    mtt::Representation& rep = *mtt::rep(thing);
                                                    
                                                    
                                                    std::vector<std::vector<vec3>>* points = &rep.points;
                                                    
                                                    //                                for (usize i = 0; i < points->size(); i += 1) {
                                                    //                                    for (usize j = 0; j < (*points)[i].size(); j += 1) {
                                                    //                                        (*points)[i][j] += rep.transform.position;
                                                    //                                    }
                                                    //                                }
                                                    points->emplace_back();
                                                    auto& radii = rep.radii;
                                                    radii.emplace_back();
                                                    rep.representation_types.push_back(REPRESENTATION_TYPE_CURVE);
                                                    
                                                    auto& color_ranges = rep.colors;
                                                    color_ranges.emplace_back();
                                                    color_ranges.back().emplace_back();
                                                    color_ranges.back().back().color = world->dt.pen.color;
                                                    color_ranges.back().back().i_begin = 0;
                                                    color_ranges.back().back().count = pointer->count;
                                                    
                                                    //#define xform() world_point = m::inverse(rep.pose_transform) * rep.model_transform_inverse * vec4(world_point, 1.0)
                                                    
                                                    
                                                    
                                                    vec2 tl = collider->aabb.tl;
                                                    vec2 br = collider->aabb.br;
                                                    mat4 pose_inv = m::inverse(rep.pose_transform);
                                                    mat4 model_inv = m::inverse(rep.model_transform);
                                                    vec3 scale;
                                                    quat orientation;
                                                    vec3 translation;
                                                    vec3 skew;
                                                    vec4 perspective;
                                                    
                                                    m::decompose(rep.model_transform, scale, orientation, translation, skew, perspective);
                                                    
                                                    pixel_radius /= m::max(rep.pose_transform_values.scale.x, rep.pose_transform_values.scale.y);
                                                    sd::path_radius(renderer, pixel_radius);
                                                    
                                                    for (usize i = 0; i < pointer->count; i += 1) {
                                                        vec3 world_point = transform_point(cam, vec3(pointer->positions[i], 0.0f));
                                                        
                                                        
                                                        vec3 world_point_inv = pose_inv * model_inv * vec4(world_point, 1.0f);
                                                        world_point_inv += translation;
                                                        
                                                        tl[0] = m::min(tl[0], world_point_inv[0]);
                                                        tl[1] = m::min(tl[1], world_point_inv[1]);
                                                        br[0] = m::max(br[0], world_point_inv[0]);
                                                        br[1] = m::max(br[1], world_point_inv[1]);
                                                        
                                                        //xform();
                                                        xform(world_point, rep);
                                                        world_point.z = 0;
                                                        
                                                        points->back().emplace_back(world_point);
                                                        radii.back().emplace_back(ADJUSTED_RADIUS(pixel_radius, pointer->forces[i], pointer->altitude_angle_radians));
                                                    }
                                                    
                                                    
                                                    
                                                    Collider_remove(collider->system, 0, collider);
                                                    
                                                    auto adjust_orientation = (rep.forward_dir.x < 0) ? m::inverse(m::toMat4(orientation)) : m::toMat4(orientation);
                                                    
                                                    
                                                    collider->aabb.tl = tl;
                                                    collider->aabb.br = br;
                                                    collider->aabb.half_extent = (br - tl) / 2.0f;
                                                    //collider->center_anchor = vec3((collider->aabb.tl + collider->aabb.br), 0.0f) * 0.5f;
                                                    collider->transform = rep.pose_transform * adjust_orientation;
                                                    
                                                    push_AABBs(collider->system, collider, 1, 0);
                                                    
                                                    
                                                    sd::set_render_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                    sd::rewind_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                    sd::begin_path(renderer);
                                                    {
                                                        //                                        usize i = 0;
                                                        //                                        for (; i < points->size() - 1; i += 1) {
                                                        //                                        auto& curve = (*points)[i];
                                                        //                                        auto& colors = color_ranges[i];
                                                        //                                        for (usize c = 0; c < colors.size(); c += 1) {
                                                        //                                            GFX_Attrib_Range& color_range = colors[c];
                                                        //                                            sd::set_color_rgba_v4(renderer, color_range.color);
                                                        //
                                                        //                                            sd::path_list(renderer,
                                                        //                                                          curve.data() + colors[c].i_begin, colors[c].count
                                                        //                                                          );
                                                        //
                                                        //                                        }
                                                        //                                        sd::break_path(renderer);
                                                        //                                        }
                                                        usize i = points->size() - 1;
                                                        auto& curve = (*points)[i];
                                                        auto& colors = color_ranges[i];
                                                        
                                                        for (usize c = 0; c < colors.size(); c += 1) {
                                                            GFX_Attrib_Range& color_range = colors[c];
                                                            sd::set_color_rgba_v4(renderer, color_range.color);
                                                            
                                                            sd::path_list_with_offset(renderer,
                                                                                      curve.data() + colors[c].i_begin, colors[c].count, -rep.offset,
                                                                                      radii.back().data() + colors[c].i_begin
                                                                                      );
                                                            
                                                        }
                                                        //sd::break_path(renderer);
                                                    }
                                                    auto* info = sd::pause_path(renderer);
                                                    sd::set_transform(info, rep.model_transform * rep.pose_transform * m::translate(rep.offset));
                                                    //rep.render_data.drawable_info_list[0]->is_enabled = false;
                                                    rep.render_data.drawable_info_list.emplace_back(info);
                                                    rep.render_data.drawable_info_list.back()->is_enabled = true;
                                                }
                                                
                                                
                                                
                                                dt::record_selection(&world->dt, thing_ID, core->time_milliseconds, event.timestamp, &event);
                                                
                                                thing->input_handlers.do_on_pen_input_began(thing, vec3(transformed_pos__, 0.0f), canvas_pos, &event, touch, 0);
                                                
                                            } else {
                                                
                                                if (!mtt::priority_layer_is_equal(world->mtt_world.priority_layer, mtt::Priority_Layer_default())) {
                                                    break;
                                                }
                                                
                                                
                                                
                                                if (pointer_op_in_progress(u_input) != UI_POINTER_OP_DRAW) {
                                                    
                                                    
                                                    switch (pointer_op_in_progress(u_input)) {
                                                        case UI_POINTER_OP_ERASE: {
                                                            //                                            sd::set_render_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                            //                                            sd::rewind_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                            //                                            sd::begin_polygon(renderer);
                                                            //                                            {
                                                            //                                                sd::circle(renderer, 5, vec3(transformed_pos__, 990.0f));
                                                            //                                            }
                                                            //                                            sd::end_polygon(renderer);
                                                            break;
                                                        }
                                                    }
                                                    
                                                    
                                                    break;
                                                }
                                                
                                                if (!Camera_is_drawable(cam)) {
//                                                    Input_set_user_data_for_touch(u_input, touch, &world->cam);
//                                                    collision_system_world = &world->mtt_world.collision_system;
//                                                    collision_system_canvas = &world->mtt_world.collision_system_canvas;
//                                                    cam = &world->cam;
//                                                    transformed_pos__ = mtt::transform_point(cam, canvas_pos);
                                                    break;
                                                }
                                                
                                                mtt::Thing* thing = mtt::Thing_make(&world->mtt_world, mtt::ARCHETYPE_FREEHAND_SKETCH);
                                                
                                                
                                                auto* noun = dt::noun_lookup("thing");
                                                ASSERT_MSG(noun != nullptr, "should exist!\n");
                                                dt::vis_word_derive_from(thing, noun);
                                                
                                                
                                                Thing_ID new_thing_ID = thing->id;
                                                
                                                world->dt.scn_ctx.thing_selected_with_pen = new_thing_ID;
                                                world->dt.scn_ctx.thing_selected_drawable = new_thing_ID;
                                                
                                                thing->is_visible = true;
                                                mtt::set_is_active(thing);
                                                thing->is_root    = true;
                                                
                                                
                                                mtt::Representation& rep = *mtt::rep(thing);
                                                
                                                rep.model_transform         = mat4(1.0f);
                                                rep.model_transform_inverse = mat4(1.0f);
                                                
                                                
                                                //                                mat4 T = mat4(1.0f);
                                                //                                T = m::translate(T, vec3(5.0f, 0.257f, 0.0f));
                                                //                                vec3 scale;
                                                //                                quat orientation;
                                                //                                vec3 translation;
                                                //                                vec3 skew;
                                                //                                vec4 perspective;
                                                //
                                                //                                m::decompose(T, scale, orientation, translation, skew, perspective);
                                                //
                                                //                                std::cout << "scale: " << m::to_string(scale) <<
                                                //                                            " orientation: " << m::to_string(orientation) <<
                                                //                                            " translation: " << m::to_string(translation) <<
                                                //                                            " skew: " << m::to_string(skew) <<
                                                //                                            " perspective: " << m::to_string(perspective) <<
                                                //                                std::endl;
                                                
                                                
                                                
                                                
                                                // push points
                                                std::vector<std::vector<vec3>>* points = &rep.points;
                                                points->emplace_back(std::vector<vec3>());
                                                auto& radii = rep.radii;
                                                radii.emplace_back();
                                                rep.representation_types.push_back(REPRESENTATION_TYPE_CURVE);
                                                
                                                std::vector<std::vector<GFX_Attrib_Range>>& color_ranges = rep.colors;
                                                color_ranges.emplace_back(std::vector<GFX_Attrib_Range>());
                                                color_ranges.back().emplace_back(GFX_Attrib_Range());
                                                color_ranges.back().back().color = world->dt.pen.color;
                                                color_ranges.back().back().i_begin = 0;
                                                color_ranges.back().back().count = pointer->count;
                                                
                                                pixel_radius /= m::max(rep.pose_transform_values.scale.x, rep.pose_transform_values.scale.y);
                                                sd::path_radius(renderer, pixel_radius);
                                                
                                                vec2 tl = {POSITIVE_INFINITY, POSITIVE_INFINITY};
                                                vec2 br = {NEGATIVE_INFINITY, NEGATIVE_INFINITY};
                                                for (usize i = 0; i < pointer->count; i += 1) {
                                                    vec3 world_point = transform_point(cam, vec3(pointer->positions[i], 0.0f));
                                                    
                                                    tl = m::min<vec2>(tl, world_point);
                                                    br = m::max<vec2>(br, world_point);
                                                    
                                                    
                                                    points->back().emplace_back(world_point);
                                                    radii.back().emplace_back(ADJUSTED_RADIUS(pixel_radius, pointer->forces[i], pointer->altitude_angle_radians));
                                                }
                                                
                                                
                                                
                                                
                                                
                                                rep.colliders.emplace_back(mtt::Collider_make(&world->mtt_world.collision_system, 0));                                Collider* collider = rep.colliders.back();
                                                collider->type = mtt::COLLIDER_TYPE_AABB;
                                                collider->handler = collision_handler_AABB_default;
                                                collider->priority = 0;
                                                //                                collider->ID = new_thing_ID;
                                                collider->aabb.tl = tl;
                                                collider->aabb.br = br;
                                                collider->aabb.half_extent = (br - tl) / 2.0f;
                                                //collider->center_anchor = vec3((collider->aabb.tl + collider->aabb.br), 0.0f) * 0.5f;
                                                collider->transform = rep.pose_transform;
                                                
                                                collider->pivot_anchor_offset = vec3(0.0f);
                                                collider->user_data = (void*)new_thing_ID;
                                                
                                                
                                                
                                                push_AABBs(collider->system, collider, 1, 0);
                                                
                                                
                                                sd::rewind_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                sd::set_render_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                                                sd::begin_path(renderer);
                                                {
                                                    for (usize i = 0; i < points->size(); i += 1) {
                                                        sd::path_list(renderer,
                                                                      ((*points)[i]).data(), ((*points)[i]).size(),
                                                                      radii.back().data()
                                                                      );
                                                        //sd::break_path(renderer);
                                                    }
                                                }
                                                auto* info = sd::pause_path(renderer);
                                                info->is_enabled = true;
                                                sd::set_transform(info, rep.model_transform * rep.pose_transform * m::translate(rep.offset));
                                                rep.render_data.drawable_info_list.emplace_back(info);
                                                
                                                dt::record_selection(&world->dt, new_thing_ID, core->time_milliseconds, event.timestamp, &event);
                                                
                                                {
                                                    rep.render_data.set_on_copy([](void* ctx, Render_Data& dst, Render_Data& src, void* args) {
                                                        int64 layer_id = (int64)args;
                                                        
                                                        mtt::Thing* thing = static_cast<mtt::Thing*>(ctx);
                                                        mtt::World* mtt_world = thing->world();
                                                        
                                                        sd::Renderer* renderer = mtt_world->renderer;
                                                        
                                                        sd::save(renderer);
                                                        
                                                        mtt::Rep& rep = *mtt::rep(thing);
                                                        auto* points       = &(rep.points);
                                                        auto& color_ranges = rep.colors;
                                                        
                                                        dst.drawable_info_list.clear();
                                                        
                                                        //sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC);
                                                        
                                                        constexpr const bool new_way_of_copying = true;
                                                        if constexpr ((new_way_of_copying)) {
                                                            
                                                            if (layer_id == -1) {
                                                                for (usize i = 0; i < src.drawable_info_list.size(); i += 1) {
                                                                    auto* info = sd::Drawable_Info_copy(renderer, src.drawable_info_list[i]->layer_id, src.drawable_info_list[i]);
                                                                    sd::Drawable_Info_ref_inc(renderer, info);
                                                                    
                                                                    sd::set_transform(info,rep.hierarchy_model_transform);
                                                                    dst.drawable_info_list.emplace_back(info);
                                                                    dst.drawable_info_list.back()->is_enabled = true;
                                                                }
                                                            } else {
                                                                for (usize i = 0; i < src.drawable_info_list.size(); i += 1) {
                                                                    auto* info = sd::Drawable_Info_copy(renderer, layer_id, src.drawable_info_list[i]);
                                                                    sd::Drawable_Info_ref_inc(renderer, info);
                                                                    
                                                                    sd::set_transform(info,rep.hierarchy_model_transform);
                                                                    dst.drawable_info_list.emplace_back(info);
                                                                    dst.drawable_info_list.back()->is_enabled = true;
                                                                }
                                                            }
                                                        } else {
                                                            
                                                            //                                            {
                                                            //                                                usize i = 0;
                                                            //                                                for (; i < points->size(); i += 1) {
                                                            //                                                    sd::begin_path(renderer);
                                                            //                                                    {
                                                            //                                                        auto& curve = (*points)[i];
                                                            //                                                        auto& colors = color_ranges[i];
                                                            //                                                        for (usize c = 0; c < colors.size() - 1; c += 1) {
                                                            //                                                            GFX_Attrib_Range& color_range = colors[c];
                                                            //                                                            sd::set_color_rgba_v4(renderer, color_range.color);
                                                            //
                                                            //                                                            sd::path_list(renderer,
                                                            //                                                                          curve.data() + colors[c].i_begin, colors[c].count
                                                            //                                                                          );
                                                            //
                                                            //                                                            sd::break_path(renderer);
                                                            //                                                        }
                                                            //                                                        {
                                                            //                                                            GFX_Attrib_Range& color_range = colors[colors.size() - 1];
                                                            //                                                            sd::set_color_rgba_v4(renderer, color_range.color);
                                                            //
                                                            //                                                            sd::path_list(renderer,
                                                            //                                                                          curve.data() + colors[colors.size() - 1].i_begin, colors[colors.size() - 1].count
                                                            //                                                                          );
                                                            //
                                                            //                                                            sd::break_path(renderer);
                                                            //                                                        }
                                                            //                                                    }
                                                            //                                                    auto* info = sd::end_path(renderer);
                                                            //                                                    info->transform = rep.model_transform * rep.pose_transform * m::translate(rep.offset);
                                                            //                                                    dst.drawable_info_list.emplace_back(info);
                                                            //                                                    dst.drawable_info_list.back()->is_enabled = true;
                                                            //                                                }
                                                            //                                            }
                                                            
                                                        }
                                                        
                                                        //ASSERT_MSG(dst.drawable_info_list.size() = src.drawable_info_list.size(), "WRONG SIZE???");
                                                        
                                                        sd::restore(renderer);
                                                        
                                                    });
                                                    
                                                }
                                                
                                                thing->input_handlers.do_on_pen_input_began(thing, vec3(transformed_pos__, 0.0f), canvas_pos, &event, touch, mtt::INPUT_EVENT_FLAG_CREATED_NEW);
                                                
                                            }
                                        
                                        break;
                                    }
                                    case UI_TOUCH_STATE_MOVED: {
                                        
                                        if (world->dt.scn_ctx.thing_selected_with_pen == mtt::Thing_ID_INVALID) {
                                            MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT_MOVE,
                                                                      STRLN("{")
                                                                      STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                      STRIDTLN(STRMK("kind") ":" STRMK("stylus") ",")
                                                                      STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                      STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                      STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])))
                                                                      STRLN("},")
                                                                      );
                                            
                                            break;
                                        }
                                        
                                        mtt::Thing* thing = mtt::Thing_from_ID(&world->mtt_world, world->dt.scn_ctx.thing_selected_with_pen);
                                        if (thing == nullptr) {
                                            world->dt.scn_ctx.thing_selected_with_pen = mtt::Thing_ID_INVALID;
                                            MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT_MOVE,
                                                                      STRLN("{")
                                                                      STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                      STRIDTLN(STRMK("kind") ":" STRMK("stylus") ",")
                                                                      STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                      STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                      STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])))
                                                                      STRLN("},")
                                                                      );
                                            break;
                                        }
                                        
                                        auto* cam = (mtt::Camera*)Input_get_user_data_for_touch(u_input, touch);
                                        if (cam == nullptr) {
                                            MTT_error("%s\n", "camera is null for some reason");
                                            cam = &world->cam;
                                        }
                                        
                                        vec2 canvas_pos__ = pointer->positions[pointer->count - 1];
                                        vec2 transformed_pos__ = mtt::transform_point(cam, canvas_pos__);
                                        
                                        MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT_MOVE,
                                                                  STRLN("{")
                                                                  STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                  STRIDTLN(STRMK("kind") ":" STRMK("stylus") ",")
                                                                  STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                  STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                  STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])) + ",")
                                                                  STRIDTLN(STRMKPAIR("canvas_pos", STRV2(canvas_pos__)) + ",")
                                                                  STRIDTLN(STRMKPAIR("world_pos", STRV2(transformed_pos__)))
                                                                  STRLN("},")
                                                                  );
                                        
                                        
                                        if (thing->archetype_id != mtt::ARCHETYPE_FREEHAND_SKETCH || !mtt::is_user_drawable(thing)) {
                                            thing->input_handlers.do_on_pen_input_moved(thing, vec3(transformed_pos__, 0.0f), canvas_pos__, &event, touch, 0);
                                            break;
                                        }
                                        
                                        
                                        if (pointer_op_in_progress(u_input) != UI_POINTER_OP_DRAW) {
                                            switch (pointer_op_current(u_input)) {
                                                case UI_POINTER_OP_ERASE: {
                                                    //                                        sd::set_render_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                    //                                        //sd::rewind_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                    //                                        sd::begin_polygon(renderer);
                                                    //                                        {
                                                    //                                            sd::circle(renderer, 5, vec3(transformed_pos__, 990.0f));
                                                    //                                        }
                                                    //                                        sd::end_polygon(renderer);
                                                    
                                                    Erase_Tool_Args args;
                                                    args.renderer = renderer;
                                                    mtt::erase_tool_moved(u_input, thing, vec3(transformed_pos__, 0.0f), canvas_pos__, &event, touch, 0, &args);
                                                    break;
                                                }
                                                case UI_POINTER_OP_ARROW_CREATE: {
                                                    if (thing_arrows.is_active) {
                                                        if (event.key != thing_arrows.input_key) {
                                                            break;
                                                        }
                                                        thing_arrows.push(vec3(transformed_pos__, 0.0f));
                                                        {
                                                            if (mtt::Thing_is_proxy(thing)) {
                                                                break;
                                                            }
                                                            

                                                            
                                                            bool was_hit = false;
                                                            
                                                            auto* collision_system = &mtt::world(thing)->collision_system;
                                                            
                                                            {
                                                                mtt::Collider* c_hit = nullptr;
                                                                
                                                                float32 min_area = POSITIVE_INFINITY;
                                                                
                                                                auto& dt_ctx = *dt::DrawTalk::ctx();
                                                                
                                                                mtt::Hit hit;
                                                                was_hit = mtt::point_query_narrow_including_selections(collision_system, transformed_pos__, &c_hit, hit, &min_area, [](mtt::Collider* c, mtt::Thing* thing) {
                                                                    return mtt::priority_layer_is_equal(c->priority_layer, mtt::world(thing)->priority_layer);
                                                                });
                                                                
                                                                if (was_hit) {
                                                                    mtt::Thing_ID other_id = (mtt::Thing_ID)c_hit->user_data;
                                                                    mtt::Thing* other = thing->world()->Thing_try_get(other_id);
                                                                    if (other != nullptr && mtt::is_actor(other)) {
                                                                        if (thing_arrows.deferred.size() > 0) {
                                                                            mtt::Thing_ID prev_first = thing_arrows.deferred.back().first;
                                                                            mtt::Thing_ID prev_second = thing_arrows.deferred.back().second;
                                                                            if (prev_first != thing_arrows.prev_in_chain || prev_second != other_id) {
                                                                                if (thing_arrows.prev_in_chain != other_id) {
                                                                                    thing_arrows.deferred.push_back({thing_arrows.prev_in_chain, other_id});
                                                                                    thing_arrows.prev_in_chain = other_id;
                                                                                }
                                                                            }
                                                                        } else {
                                                                            mtt::Thing_ID prev_in_chain = thing_arrows.prev_in_chain;
                                                                            if (prev_in_chain != other_id) {
                                                                                thing_arrows.deferred.push_back({prev_in_chain, other_id});
                                                                                thing_arrows.prev_in_chain = other_id;
                                                                            }
                                                                        }
                                                                        
                                                                    }
                                                                } else {
                                                                    
                                                                    
                                                                }
                                                                //#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
                                                                
                                                                thing_arrows.canvas_pos = canvas_pos__;
                                                                //#endif
                                                            }
                                                        }
                                                        
                                                    }
                                                }
                                                break;
                                            }
                                        } else {
                                            mtt::Representation& rep = *mtt::rep(thing);
                                            
                                            
                                            std::vector<std::vector<vec3>>* points = &rep.points;
                                            auto& radii = rep.radii;
                                            
                                            Collider* collider = rep.colliders.back();
                                            vec2 tl = collider->aabb.tl;
                                            vec2 br = collider->aabb.br;
                                            
                                            vec3 scale;
                                            quat orientation;
                                            vec3 translation;
                                            vec3 skew;
                                            vec4 perspective;
                                            
                                            m::decompose(rep.model_transform, scale, orientation, translation, skew, perspective);
                                            
                                            pixel_radius /= m::max(rep.pose_transform_values.scale.x, rep.pose_transform_values.scale.y);
                                            sd::path_radius(renderer, pixel_radius);
                                            
                                            if (mtt::is_user_drawable(thing)) {
                                                
                                                //rep.model_transform_inverse = m::inverse(rep.model_transform);
                                                mat4 pose_inv = m::inverse(rep.pose_transform);
                                                mat4 model_inv = m::inverse(rep.model_transform);
                                                for (usize i = 0; i < pointer->count; i += 1) {
                                                    vec3 world_point = transform_point(cam, vec3(pointer->positions[i], 0.0f));
                                                    
                                                    vec3 world_point_inv = pose_inv * model_inv * vec4(world_point, 1.0f);
                                                    world_point_inv += translation;
                                                    
                                                    tl = m::min<vec2>(tl, world_point_inv);
                                                    br = m::max<vec2>(br, world_point_inv);
                                                    
                                                    xform(world_point, rep);
                                                    world_point.z = 0;
                                                    
                                                    points->back().emplace_back(world_point);
                                                    radii.back().emplace_back(ADJUSTED_RADIUS(pixel_radius, pointer->forces[i], pointer->altitude_angle_radians));
                                                }
                                                
                                                
                                                
                                                Collider_remove(collider->system, 0, collider);
                                                
                                                auto adjust_orientation = (rep.forward_dir.x < 0) ? m::inverse(m::toMat4(orientation)) : m::toMat4(orientation);
                                                
                                                collider->aabb.tl = tl;
                                                collider->aabb.br = br;
                                                collider->aabb.half_extent = (br - tl) / 2.0f;
                                                //collider->center_anchor = vec3((collider->aabb.tl + collider->aabb.br), 0.0f) * 0.5f;
                                                collider->transform = rep.pose_transform * adjust_orientation;
                                                push_AABBs(collider->system, collider, 1, 0);
                                                
                                                std::vector<std::vector<GFX_Attrib_Range>>& color_ranges = rep.colors;
                                                if (world->dt.pen.color == color_ranges.back().back().color) {
                                                    color_ranges.back().back().color = world->dt.pen.color;
                                                    
                                                    color_ranges.back().back().count += pointer->count;
                                                } else {
                                                    usize offset = color_ranges.back().back().i_begin + color_ranges.back().back().count;
                                                    color_ranges.back().emplace_back(GFX_Attrib_Range());
                                                    color_ranges.back().back().color = world->dt.pen.color;
                                                    color_ranges.back().back().i_begin = offset;
                                                    color_ranges.back().back().count = pointer->count;
                                                }
                                                
                                                
                                                sd::set_render_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                //sd::rewind_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                //sd::continue_path(renderer);
                                                sd::continue_path(renderer);
                                                {
                                                    sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                                                    if (pointer->count > 0) {
                                                        sd::path_list_with_offset(renderer,
                                                                                  points->back().data() + points->back().size() - pointer->count , pointer->count, - rep.offset,
                                                                                  radii.back().data() + points->back().size() - pointer->count
                                                                                  );
                                                    }
                                                }
                                                auto* info = sd::pause_path(renderer);
                                                sd::set_transform(info, rep.model_transform * rep.pose_transform * m::translate(rep.offset));
                                                rep.render_data.drawable_info_list.back()->is_enabled = false;
                                                rep.render_data.drawable_info_list[rep.render_data.drawable_info_list.size() - 1] = info;
                                                rep.render_data.drawable_info_list.back()->is_enabled = true;
                                                
                                            }
                                            
                                        }
                                        
                                        thing->input_handlers.do_on_pen_input_moved(thing, vec3(transformed_pos__, 0.0f), canvas_pos__, &event, touch, 0);
                                        
                                        break;
                                    }
                                    case UI_TOUCH_STATE_CANCELLED:
                                        // fallthrough
                                    case UI_TOUCH_STATE_ENDED: {
                                        
                                        //PR("\t\tENDED");
                                        if (!mtt::Thing_ID_is_valid(world->dt.scn_ctx.thing_selected_with_pen)) {
                                            control->stylus.mode = dt::SP_STYLUS_MODE_NONE;
                                            
                                            control->stylus.base.active_state = dt::SP_CONTROL_ACTIVE_STATE_OFF;
                                            pointer_op_set_in_progress(u_input, UI_POINTER_OP_NONE);
                                            break;
                                        }
                                        
                                        mtt::Thing* thing = mtt::Thing_from_ID(&world->mtt_world, world->dt.scn_ctx.thing_selected_with_pen);
                                        if (thing == nullptr) {
                                            control->stylus.mode = dt::SP_STYLUS_MODE_NONE;
                                            
                                            control->stylus.base.active_state = dt::SP_CONTROL_ACTIVE_STATE_OFF;
                                            world->dt.scn_ctx.thing_selected_with_pen = mtt::Thing_ID_INVALID;
                                            pointer_op_set_in_progress(u_input, UI_POINTER_OP_NONE);
                                            MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                      STRLN("{")
                                                                      STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                      STRIDTLN(STRMK("kind") ":" STRMK("stylus") ",")
                                                                      STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                      STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                      STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])) )
                                                                      STRLN("},")
                                                                      );
                                            break;
                                        }
                                        
                                        
                                        vec2 canvas_pos__ = pointer->positions[pointer->count - 1];
                                        
                                        auto* cam = (mtt::Camera*)Input_get_user_data_for_touch(u_input, touch);
                                        if (cam == nullptr) {
                                            control->stylus.mode = dt::SP_STYLUS_MODE_NONE;
                                            
                                            control->stylus.base.active_state = dt::SP_CONTROL_ACTIVE_STATE_OFF;
                                            world->dt.scn_ctx.thing_selected_with_pen = mtt::Thing_ID_INVALID;
                                            pointer_op_set_in_progress(u_input, UI_POINTER_OP_NONE);
                                            
                                            MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                      STRLN("{")
                                                                      STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                      STRIDTLN(STRMK("kind") ":" STRMK("stylus") ",")
                                                                      STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                      STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                      STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])) )
                                                                      STRLN("},")
                                                                      );
                                            
                                            break;
                                        }
                                        
                                        
                                        vec2 transformed_pos__ = mtt::transform_point(cam, canvas_pos__);
                                        
                                        MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                  STRLN("{")
                                                                  STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                  STRIDTLN(STRMK("kind") ":" STRMK("stylus") ",")
                                                                  STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                  STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                  STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])) + ",")
                                                                  STRIDTLN(STRMKPAIR("canvas_pos", STRV2(canvas_pos__)) + ",")
                                                                  STRIDTLN(STRMKPAIR("world_pos", STRV2(transformed_pos__)))
                                                                  STRLN("},")
                                                                  );
                                        
                                        control->stylus.mode = dt::SP_STYLUS_MODE_NONE;
                                        
                                        control->stylus.base.active_state = dt::SP_CONTROL_ACTIVE_STATE_OFF;
                                        
                                        {
                                            mtt::Thing* thing = mtt::Thing_from_ID(&world->mtt_world, world->dt.scn_ctx.thing_selected_with_pen);
                                            
                                            if (thing->archetype_id != mtt::ARCHETYPE_FREEHAND_SKETCH || !mtt::is_user_drawable(thing)) {
                                                thing->input_handlers.do_on_pen_input_ended(thing, vec3(transformed_pos__, 0.0f), canvas_pos__, &event, touch, 0);
                                                
                                                if (thing->id == world->dt.scn_ctx.thing_selected_with_pen) {
                                                    world->dt.scn_ctx.thing_selected_with_pen = mtt::Thing_ID_INVALID;
                                                }
                                                pointer_op_set_in_progress(u_input, UI_POINTER_OP_NONE);
                                                break;
                                            }
                                            
                                            
                                            thing->world()->saved_children = thing->child_id_set;
                                            mtt::disconnect_parent_from_children(thing->world(), thing);
                                            
                                            if (pointer_op_in_progress(u_input) != UI_POINTER_OP_DRAW) {
                                                switch (pointer_op_in_progress(u_input)) {
                                                    case UI_POINTER_OP_ERASE: {
                                                        //                                            sd::set_render_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                        //                                            sd::rewind_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                        ////                                            sd::begin_polygon(renderer);
                                                        //                                            {
                                                        //                                                sd::circle(renderer, 5, vec3(transformed_pos__, 990.0f));
                                                        //                                            }
                                                        //                                            sd::end_polygon(renderer);
                                                        
                                                        Erase_Tool_Args args;
                                                        args.renderer = renderer;
                                                        mtt::erase_tool_ended(u_input, thing, vec3(transformed_pos__, 0.0f), canvas_pos__, &event, touch, 0, &args);
                                                        break;
                                                    }
                                                    case UI_POINTER_OP_ARROW_CREATE: {
                                                        if (thing_arrows.is_active && !mtt::Thing_is_proxy(thing)) {
                                                            if (thing_arrows.deferred.size() == 1) {
                                                                const usize i_deferred = 0;
                                                                mtt::Thing_ID src = thing_arrows.deferred[i_deferred].first;
                                                                mtt::Thing_ID dst = thing_arrows.deferred[i_deferred].second;
                                                                
                                                                arrow_edge_add_or_if_exists_remove(&world->mtt_world, src, dst, "", thing_arrows.arrow_flags_visible_with_type());
                                                            } else {
                                                                if (thing_arrows.deferred.empty()) {
                                                                    mtt::arrows_reverse(thing);
                                                                } else if (thing_arrows.deferred.size() > 1 && key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL))) {
                                                                    mtt::Thing_ID dst = thing_arrows.deferred[thing_arrows.deferred.size() - 1].second;
                                                                    mtt::Thing* dst_thing = world->mtt_world.Thing_try_get(dst);
                                                                    if (dst_thing != nullptr) {

                                                                        
                                                                        for (usize i_deferred = 0; i_deferred < thing_arrows.deferred.size(); i_deferred += 1) {
                                                                            mtt::Thing_ID src = thing_arrows.deferred[i_deferred].first;
                                                                            
                                                                            if (src == dst) {
                                                                                continue;
                                                                            }
                                                                            
                                                                            mtt::Thing* src_thing = world->mtt_world.Thing_try_get(src);
                                                                            if (src_thing == nullptr) {
                                                                                continue;
                                                                            }
                                                                            
                                                                            
                                                                            mtt::arrow_edge_add_or_if_exists_remove(&world->mtt_world, src, dst, "", thing_arrows.arrow_flags_visible_with_type());
                                                                        }
                                                                    }
                                                                } else if (thing_arrows.deferred.size() > 1 && key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_COMMAND))) {
                                                                    mtt::Thing_ID dst = thing_arrows.deferred[thing_arrows.deferred.size() - 1].second;
                                                                    mtt::Thing* dst_thing = world->mtt_world.Thing_try_get(dst);
                                                                    if (dst_thing != nullptr) {

                                                                        
                                                                        for (usize i_deferred = 0; i_deferred < thing_arrows.deferred.size(); i_deferred += 1) {
                                                                            mtt::Thing_ID src = thing_arrows.deferred[i_deferred].first;
                                                                            
                                                                            if (src == dst) {
                                                                                continue;
                                                                            }
                                                                            
                                                                            mtt::Thing* src_thing = world->mtt_world.Thing_try_get(src);
                                                                            if (src_thing == nullptr) {
                                                                                continue;
                                                                            }
                                                                            
                                                                            
                                                                            mtt::arrow_edge_add_or_if_exists_remove(&world->mtt_world, dst, src, "", thing_arrows.arrow_flags_visible_with_type());
                                                                        }
                                                                    }
                                                                } else {
                                                                    static mtt::String label = "";
                                                                    label.clear();
                                                                    Arrows::Deferred_Op* prev_deferred = nullptr;
                                                                    mtt::Thing* prev_src = nullptr;
                                                                    for (usize i_deferred = 0; i_deferred < thing_arrows.deferred.size(); i_deferred += 1) {
                                                                        mtt::Thing_ID src = thing_arrows.deferred[i_deferred].first;
                                                                        mtt::Thing_ID dst = thing_arrows.deferred[i_deferred].second;
                                                                        
                                                                        
                                                                        mtt::Thing* src_thing = world->mtt_world.Thing_try_get(src);
                                                                        mtt::Thing* dst_thing = world->mtt_world.Thing_try_get(dst);
                                                                        if (src_thing == nullptr || dst_thing == nullptr) {
                                                                            continue;
                                                                        }
                                                                        
                                                                        if (src == dst) {
                                                                            continue;
                                                                        }
                                                                        
                                                                        bool do_add_edge = true;
                                                                        if ((src_thing->archetype_id == mtt::ARCHETYPE_TEXT && dst_thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH)){
                                                                            if (prev_src != nullptr && prev_src->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH) {
                                                                                
                                                                                cstring str = mtt::text_get_value(src_thing);
                                                                                if (strcmp(str, mtt::text_thing_empty_string) != 0) {
                                                                                    auto* rel_entry = dt::attribute_add(str);
        #define MTT_ARROW_BYPASS
        #ifdef MTT_ARROW_BYPASS
                                                                                    if (!dt::Thing_has_relation(prev_src, rel_entry, dst_thing)) {
                                                                                        dt::Thing_add_relation(prev_src, rel_entry, dst_thing);
                                                                                        //mtt::arrow_edge_add(&world->mtt_world, src, dst, str);
                                                                                        
                                                                                        
                                                                                        
                                                                                        mtt::arrow_edge_add(&world->mtt_world, prev_deferred->first, dst, str, thing_arrows.arrow_flags_visible_with_type());
                                                                                        do_add_edge = false;
                                                                                        
                                                                                        {
                                                                                            mtt::arrow_edge_remove(&world->mtt_world,  prev_deferred->first, src);
                                                                                        }
                                                                                    } else {
                                                                                        dt::Thing_remove_relation(prev_src, rel_entry, dst_thing);
                                                                                        
                                                                                        
                                                                                        mtt::arrow_edge_remove(&world->mtt_world, prev_deferred->first, prev_deferred->second);
                                                                                        mtt::arrow_edge_remove(&world->mtt_world, src, dst, str);
                                                                                        mtt::arrow_edge_remove(&world->mtt_world, prev_deferred->first, dst, str);
                                                                                        
                                                                                        do_add_edge = false;
                                                                                    }
        #else
                                                                                    if (!dt::Thing_has_relation(prev_src, rel_entry, dst_thing)) {
                                                                                        dt::Thing_add_relation(prev_src, rel_entry, dst_thing);
                                                                                        mtt::arrow_edge_add(&world->mtt_world, src, dst, str);
                                                                                        
                                                                                        do_add_edge = false;
                                                                                        
                                                                                        
                                                                                    } else {
                                                                                        dt::Thing_remove_relation(prev_src, rel_entry, dst_thing);
                                                                                        
                                                                                        mtt::arrow_edge_remove(&world->mtt_world, src, dst, str);
                                                                                        
                                                                                        do_add_edge = false;
                                                                                    }
        #endif
        #undef MTT_ARROW_BYPASS
                                                                                }
                                                                            }
                                                                        }
                                                                        
                                                                        if (do_add_edge) {
                                                                            mtt::arrow_edge_add(&world->mtt_world, src, dst, "", thing_arrows.arrow_flags_visible_with_type());
                                                                        }
                                                                        {

                                                                        }
                                                                        
                                                                        prev_deferred = &thing_arrows.deferred[i_deferred];
                                                                        prev_src = src_thing;
                                                                    }
                                                                }
                                                            }
                                                            thing_arrows.prev_in_chain = mtt::Thing_ID_INVALID;
                                                            thing_arrows.is_active = false;
                                                            thing_arrows.clear();
                                                        }
                                                        break;
                                                    }
                                                }
                                                
                                                if (thing->saved_parent_thing_id != mtt::Thing_ID_INVALID) {
                                                    mtt::Thing* parent = nullptr;
                                                    if (thing->world()->Thing_try_get(thing->saved_parent_thing_id, &parent)) {
                                                        mtt::connect_parent_to_child(thing->world(), parent, thing);
                                                    }
                                                }
                                                for (auto it = thing->world()->saved_children.begin(); it != thing->world()->saved_children.end(); ++it) {
                                                    mtt::Thing* child = nullptr;
                                                    if (thing->world()->Thing_try_get(*it, &child)) {
                                                        mtt::connect_parent_to_child(thing->world(), thing, child);
                                                    }
                                                }
                                                
                                            } else {

                                                mtt::Representation& rep = *mtt::rep(thing);
                                                
                                                std::vector<std::vector<vec3>>* points = &rep.points;
                                                auto& radii = rep.radii;
                                                std::vector<std::vector<GFX_Attrib_Range>>& color_ranges = rep.colors;
                                                sd::Drawable_Info* info;
                                                
                                                if (mtt::is_user_drawable(thing)) {
                                                    if (world->dt.pen.color == color_ranges.back().back().color) {
                                                        color_ranges.back().back().color = world->dt.pen.color;
                                                        color_ranges.back().back().count += pointer->count;
                                                    } else {
                                                        usize offset = color_ranges.back().back().i_begin + color_ranges.back().back().count;
                                                        color_ranges.back().emplace_back(GFX_Attrib_Range());
                                                        color_ranges.back().back().color = world->dt.pen.color;
                                                        color_ranges.back().back().i_begin = offset;
                                                        color_ranges.back().back().count = pointer->count;
                                                    }
                                                    
                                                    Collider* collider = rep.colliders.back();
                                                    vec2 tl = collider->aabb.tl;
                                                    vec2 br = collider->aabb.br;
                                                    
                                                    vec3 scale;
                                                    quat orientation;
                                                    vec3 translation;
                                                    vec3 skew;
                                                    vec4 perspective;
                                                    
                                                    pixel_radius /= m::max(rep.pose_transform_values.scale.x, rep.pose_transform_values.scale.y);
                                                    sd::path_radius(renderer, pixel_radius);
                                                    
                                                    m::decompose(rep.model_transform, scale, orientation, translation, skew, perspective);
                                                    
                                                    mat4 pose_inv = m::inverse(rep.pose_transform);
                                                    mat4 model_inv = m::inverse(rep.model_transform);
                                                    
                                                    auto prev = radii.back().back();
                                                    for (usize i = 0; i < pointer->count; i += 1) {
                                                        vec3 world_point = transform_point(cam, vec3(pointer->positions[i], 0.0f));
                                                        
                                                        
                                                        
                                                        vec3 world_point_inv = pose_inv * model_inv * vec4(world_point, 1.0f);
                                                        world_point_inv += translation;
                                                        
                                                        
                                                        tl = m::min<vec2>(tl, world_point_inv);
                                                        br = m::max<vec2>(br, world_point_inv);
                                                        
                                                        xform(world_point, rep);
                                                        world_point.z = 0;
                                                        
                                                        points->back().emplace_back(world_point);
                                                        //radii.back().emplace_back(ADJUSTED_RADIUS(pixel_radius, pointer->forces[i]));
                                                        radii.back().emplace_back(prev);
                                                    }
                                                    
                                                    
                                                    //                                if (br[1] - tl[1] < 20) {
                                                    //                                    usize diff = (20 - (br[1] - tl[1])) / 2;
                                                    //                                    br[1] += diff;
                                                    //                                    tl[1] -= diff;
                                                    //                                }
                                                    //                                if (br[0] - tl[0] < 20) {
                                                    //                                    usize diff = (20 - (br[0] - tl[0])) / 2;
                                                    //                                    br[0] += diff;
                                                    //                                    tl[0] -= diff;
                                                    //                                }
                                                    
                                                    sd::set_render_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                                    
#define PRESERVE_IN_PROCESS_LOOK (false)
                                                    //
#if !PRESERVE_IN_PROCESS_LOOK
                                                    sd::rewind_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
#else
                                                    sd::continue_path(renderer);
                                                    {
                                                        sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                                                        if (pointer->count > 0) {
                                                            sd::path_list_with_offset(renderer,
                                                                                      points->back().data() + points->back().size() - pointer->count , pointer->count, - rep.offset,
                                                                                      radii.back().data() + points->back().size() - pointer->count
                                                                                      );
                                                        }
                                                    }
                                                    
                                                    sd::Drawable_Info* info = sd::end_path(renderer);
#endif
                                                    
                                                    
                                                    Collider_remove(collider->system, 0, collider);
                                                    
                                                    
                                                    
                                                    collider->aabb.tl = tl;
                                                    collider->aabb.br = br;
                                                    collider->aabb.half_extent = (br - tl) / 2.0f;
                                                    
                                                    
                                                    auto adjust_orientation = (rep.forward_dir.x < 0) ? m::inverse(m::toMat4(orientation)) : m::toMat4(orientation);
                                                    
                                                    collider->transform = rep.pose_transform * adjust_orientation * m::scale(Mat4(1.0f), scale);
                                                    
                                                    
                                                    Box box = calc_transformed_aabb(collider);
                                                    vec3 new_center = vec3((box.tl + box.br) / 2.0, 0.0);
                                                    vec3 offset = new_center - translation;
                                                    vec2 old_center = collider->center_anchor;
                                                    collider->center_anchor = new_center;
                                                    
                                                    push_AABBs(collider->system, collider, 1, 0);
                                                    
                                                    vec3 OFF = pose_inv * m::inverse(m::toMat4(orientation)) * vec4((new_center - vec3(old_center, 0.0f)), 1.0f);
                                                    
                                                    rep.model_transform = m::translate(Mat4(1.0f), new_center) * m::toMat4(orientation);
                                                    rep.model_transform_inverse = m::inverse(rep.model_transform);
                                                    rep.hierarchy_model_transform = rep.model_transform;
                                                    //rep.world_transform_inverse = rep.model_transform_inverse;
                                                    
                                                    // latest curve
                                                    //std::cout << "WEE{" << std::endl;
                                                    
                                                    //vec3 prev_OFF = rep.offset;
                                                    
                                                    
                                                    
                                                    for (usize i = 0; i < points->back().size(); i += 1) {
                                                        points->back()[i] -= rep.offset;
                                                    }
                                                    rep.offset -= OFF;
                                                    
                                                    
                                                    //
                                                    
                                                    
                                                    //sd::path_pixel_radius(renderer, world->ct.pen.pixel_radius * m::max(0.01f, d_scale.x));
                                                    
                                                    /*for (usize i = 0; i < points->size(); i += 1) */
#if !PRESERVE_IN_PROCESS_LOOK
                                                    sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                                                    //sd::Drawable_Info* info;
                                                    {
                                                        sd::begin_path(renderer);
                                                        auto& curve = points->back();
                                                        auto& colors = color_ranges.back();
                                                        for (usize c = 0; c < colors.size(); c += 1) {
                                                            GFX_Attrib_Range& color_range = colors[c];
                                                            sd::set_color_rgba_v4(renderer, color_range.color);
                                                            
                                                            sd::path_list(renderer,
                                                                          curve.data() + colors[c].i_begin, colors[c].count,
                                                                          radii.back().data() + colors[c].i_begin
                                                                          );
                                                            
                                                            //sd::pause_path(renderer);
                                                            
                                                            
                                                        }
                                                        info = sd::end_path(renderer);
                                                    }
#endif
                                                    
                                                    //                                    Drawable_Info_print(info);
                                                    sd::set_transform(info, rep.model_transform * rep.pose_transform * m::translate(rep.offset));
                                                    info->is_enabled = true;
                                                    
                                                    info->set_texture_ID(sd::DEFAULT_TEXTURE_ID);
                                                    info->set_texture_sampler_ID(sd::DEFAULT_TEXTURE_SAMPLER_ID);
                                                    
                                                    auto* final_info = sd::Drawable_Info_copy(renderer, LAYER_LABEL_DYNAMIC, info);
                                                    sd::Drawable_Info_ref_inc(renderer, final_info);
                                                    
                                                    
                                                    
                                                    rep.render_data.drawable_info_list.back()->is_enabled = false;
                                                    rep.render_data.drawable_info_list.pop_back();
                                                    rep.render_data.drawable_info_list.push_back(final_info);
                                                    final_info->is_enabled = true;
                                                    
                                                    
                                                    
                                                    //                                MTT_print("drawable count: %lu\n", rep.render_data.drawable_info_list.size());
                                                    //                                MTT_print("point count = %lu, color ranges size = %lu", rep.points.size(), rep.colors.size());
                                                    //                                MTT_print("{\n");
                                                    //                                for (auto CIT = color_ranges.begin(); CIT != color_ranges.end(); ++CIT) {
                                                    //                                    MTT_print("\t{\n");
                                                    //                                    for (auto CIT_SUB = (*CIT).begin(); CIT_SUB != (*CIT).end(); ++CIT_SUB) {
                                                    //                                        MTT_print("\t\t%s,\n", m::to_string((*CIT_SUB).color).c_str());
                                                    //                                    }
                                                    //                                    MTT_print("\t}\n");
                                                    //                                }
                                                    //                                MTT_print("}\n");
                                                    //
                                                    //                                int BP = 0;
                                                }
                                                
                                                if (thing->saved_parent_thing_id != mtt::Thing_ID_INVALID) {
                                                    mtt::Thing* parent = nullptr;
                                                    if (thing->world()->Thing_try_get(thing->saved_parent_thing_id, &parent)) {
                                                        mtt::connect_parent_to_child(thing->world(), parent, thing);
                                                    }
                                                }
                                                for (auto it = thing->world()->saved_children.begin(); it != thing->world()->saved_children.end(); ++it) {
                                                    mtt::Thing* child = nullptr;
                                                    if (thing->world()->Thing_try_get(*it, &child)) {
                                                        mtt::connect_parent_to_child(thing->world(), thing, child);
                                                    }
                                                }
                                                thing->world()->saved_children.clear();
                                                
                                                //rep.transform.position = vec3(0.0, 0.0, 0.0);
                                                
                                                if (dt::can_label_thing(thing)) {
                                                    try_labeling_via_deixis_match(dt::DrawTalk::ctx());
                                                }
                                                
                                                if (!mtt::Thing_is_proxy(thing) && mtt::is_user_drawable(thing)) {
                                                    auto* proxies = mtt::Thing_proxies_try_get(thing);
                                                    if (proxies != nullptr) {
                                                        for (auto proxy_id : *proxies) {
                                                            mtt::Thing* proxy = mtt::Thing_try_get(mtt_world, proxy_id);
                                                            if (proxy == nullptr) {
                                                                continue;
                                                            }
                                                            
                                                            proxy->world()->saved_children = proxy->child_id_set;
                                                            mtt::disconnect_parent_from_children(proxy->world(), proxy);
                                                            
                                                            {
                                                                auto& proxy_rep = *mtt::rep(proxy);
                                                                
                                                                Collider* c_src = rep.colliders.back();
                                                                {
                                                                    for (usize i = 0; i < proxy_rep.colliders.size(); i += 1) {
                                                                        mtt::Collider* collider = proxy_rep.colliders[i];
                                                                        
                                                                        mtt::Collider_remove(collider->system, collider->layer, collider);
                                                                        
                                                                        Collider_copy_into(collider, rep.colliders[i], (void*)proxy->id);
                                                                    }
                                                                    
                                                                }
                                                                
                                                                // cloned data for proxy
                                                                {
                                                                    //                                                    c_cpy->aabb = c_src->aabb;
                                                                    //
                                                                    //                                                    Box box = calc_transformed_aabb(c_cpy);
                                                                    
                                                                    //push_AABBs(c_cpy->system, c_cpy, 1, 0);
                                                                    //proxy_rep.colliders.push_back(c_cpy);
                                                                    
                                                                    
                                                                    if constexpr ((false)) {
                                                                        proxy_rep.colors.push_back(color_ranges.back());
                                                                        proxy_rep.radii.push_back(radii.back());
                                                                        proxy_rep.points.push_back(points->back());
                                                                        proxy_rep.representation_types.push_back(rep.representation_types.back());
                                                                    }
                                                                    
                                                                    proxy_rep.offset = rep.offset;
                                                                    
                                                                    
                                                                    //                                                    proxy_rep.model_transform = rep.model_transform;
                                                                    //                                                    proxy_rep.model_transform_inverse = rep.model_transform_inverse;
                                                                    //                                                    proxy_rep.hierarchy_model_transform = rep.hierarchy_model_transform;
                                                                    
                                                                    //                                                    sd::set_transform(info, proxy_rep.model_transform * proxy_rep.pose_transform * m::translate(proxy_rep.offset));
                                                                    ASSERT_MSG(rep.colliders.size() == proxy_rep.colliders.size(), "should have same collider count");
                                                                    if constexpr ((false)) {
                                                                        auto* final_info_proxy = sd::Drawable_Info_copy(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER, info);
                                                                        sd::Drawable_Info_ref_inc(renderer, final_info_proxy);
                                                                        
                                                                        
                                                                        //                                                    proxy_rep.render_data.drawable_info_list.back()->is_enabled = false;
                                                                        //                                                    proxy_rep.render_data.drawable_info_list.pop_back();
                                                                        proxy_rep.render_data.drawable_info_list.push_back(final_info_proxy);
                                                                        final_info_proxy->is_enabled = true;
                                                                    } else {
                                                                        auto* final_info_proxy = mem::alloc_init<sd::Drawable_Info>(&mtt_world->drawable_pool.allocator);
                                                                        *final_info_proxy = *info;
                                                                        final_info_proxy->reference_count = 0;
                                                                        
                                                                        proxy_rep.render_data.drawable_info_list.push_back(final_info_proxy);
                                                                        final_info_proxy->is_enabled = true;
                                                                    }
                                                                    //
                                                                    //sd::set_transform(final_info, rep.hierarchy_model_transform);
                                                                }
                                                            }
                                                            
                                                            if (proxy->saved_parent_thing_id != mtt::Thing_ID_INVALID) {
                                                                mtt::Thing* parent = nullptr;
                                                                if (proxy->world()->Thing_try_get(proxy->saved_parent_thing_id, &parent)) {
                                                                    mtt::connect_parent_to_child(proxy->world(), parent, proxy);
                                                                }
                                                            }
                                                            for (auto it = proxy->world()->saved_children.begin(); it != proxy->world()->saved_children.end(); ++it) {
                                                                mtt::Thing* child = nullptr;
                                                                if (proxy->world()->Thing_try_get(*it, &child)) {
                                                                    mtt::connect_parent_to_child(proxy->world(), proxy, child);
                                                                }
                                                            }
                                                            proxy->world()->saved_children.clear();
                                                            
                                                        }
                                                    }
                                                } else {
                                                    
                                                }
                                                
                                                sd::rewind_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
                                            }
                                            
                                            thing->input_handlers.do_on_pen_input_ended(thing, vec3(transformed_pos__, 0.0f), canvas_pos__, &event, touch, 0);
                                            
                                            
                                            if (thing->id == world->dt.scn_ctx.thing_selected_with_pen) {
                                                world->dt.scn_ctx.thing_selected_with_pen = mtt::Thing_ID_INVALID;
                                            }
                                            
                                            pointer_op_set_in_progress(u_input, UI_POINTER_OP_NONE);
                                            
                                            
                                            //                                mtt::Thing* test_rotator = mtt::Thing_make(thing->world, mtt::ARCHETYPE_ROTATOR);
                                            //                                float32* dir = mtt::access<float32>(test_rotator, "direction");
                                            //                                *dir = 4.0f;
                                            //
                                            //
                                            //                                mtt::Thing* rotator = mtt::get_first_child_with_archetype(thing, mtt::ARCHETYPE_ROTATOR);
                                            //                                if (rotator == nullptr) {
                                            //                                    mtt::connect_parent_to_child(thing->world, thing, test_rotator);
                                            //                                }
                                            //mtt::add_connection(thing->world(), __testthing__, "result" , thing, "rotation");
                                            
                                        }
                                        
                                        break;
                                    }
                                    case UI_TOUCH_STATE_DOUBLE_TAP_PREFERRED: {
                                        //MTT_print("%s", "double tap preferred\n");
                                        
                                        
                                        switch (pointer_op_current(u_input)) {
                                            case UI_POINTER_OP_DRAW: {
                                                pointer_op_set_current(u_input, UI_POINTER_OP_ERASE);
                                                break;
                                            }
                                            case UI_POINTER_OP_ERASE: {
                                                pointer_op_set_current(u_input, UI_POINTER_OP_DRAW);
                                                break;
                                            }
                                            case UI_POINTER_OP_ARROW_CREATE: {
                                                //pointer_op_set_current(u_input, UI_POINTER_OP_DRAW);
                                                if (thing_arrows.arrow_type == ARROW_LINK_FLAGS_DIRECTED) {
                                                    thing_arrows.arrow_type = (ARROW_LINK_FLAGS)~ARROW_LINK_FLAGS_DIRECTED;
                                                } else {
                                                    thing_arrows.arrow_type = ARROW_LINK_FLAGS_DIRECTED;
                                                }
                                                break;
                                            }
                                        }
                                        
                                        
                                        break;
                                    }
                                    case UI_TOUCH_STATE_PRESSED_BUTTON_BEGAN: {
                                        auto& ui = dt::ctx()->ui;
                                        mtt::Thing* ui_button_thing = world->mtt_world.Thing_get(ui.dock.buttons[ui.dock.label_to_index.find(DT_SPEECH_COMMAND_BUTTON_NAME)->second]->thing);
                                        handle_speech_phase_change(*dt::DrawTalk::ctx(), ui_button_thing);
                                        break;
                                    }
                                    case UI_TOUCH_STATE_PRESSED_BUTTON_CHANGED: {
                                        break;
                                    }
                                    case UI_TOUCH_STATE_PRESSED_BUTTON_CANCELLED: {
                                        break;
                                    }
                                    case UI_TOUCH_STATE_PRESSED_BUTTON_ENDED: {
                                        break;
                                    }
                                        
                                    default: { break; }
                                }
                                break;
                            }
                        }
                        
                        break;
                    }
                    case UI_TOUCH_TYPE_DIRECT: {
                        UI_Touch_Direct* direct = nullptr;
                        {
                            //MTT_print("KEY: %lu\n", event.key);
                            auto result = u_input->direct_map.find(event.key);
                            if (result != u_input->direct_map.end()) {
                                touch = &result->second;
                                direct = &touch->direct;
                                
                            }
                            
                        }
                        switch (control->touch.mode) {
                            case dt::SP_TOUCH_MODE_NONE: {
                                
                                switch (control->stylus.mode) {
                                        
                                    default: {
                                        
                                        switch (event.state_type) {
                                            case UI_TOUCH_STATE_BEGAN: {
                                                control->touch.count_active += 1;
                                                //                                                MTT_print("{\n\tUI_Event t=[%f]",
                                                //                                                          direct->timestamps[direct->count - 1]);
                                                if (world->dt.selection_map.find(event.key) != world->dt.selection_map.end()) {
                                                    MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                              STRLN("{")
                                                                              STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                              STRIDTLN(STRMK("kind") ":" STRMK("direct") ",")
                                                                              STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                              STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                              STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])))
                                                                              STRLN("},")
                                                                              );
                                                    break;
                                                }
                                                
                                                
                                                
//                                                touch->user_data_2 = (void*)cam;
//                                                touch->user_data_3 = nullptr; // collision system
                                                static Collider_List out_world;
                                                out_world.clear();
                                                static Collider_List out_canvas;
                                                out_canvas.clear();
                                                
                                                bool was_hit = false;
                                                
                                                mtt::Camera* cam = nullptr;
                                                mtt::Collision_System* collision_system_canvas = nullptr;
                                                mtt::Collision_System* collision_system_world = nullptr;
                                                
                                                vec2 canvas_pos = direct->positions[direct->count - 1];
                                                
                                                something_touchdown__ = true;
                                                most_recent_canvas_pos_something__ = canvas_pos;
                                                
                                                collision_system_info_choose(world, canvas_pos, &collision_system_canvas, &collision_system_world, &cam);
                                                
                                                Input_set_user_data_for_touch(u_input, touch, cam);

                                                
                                                
                                                vec2 transformed_pos = mtt::transform_point(cam, canvas_pos);
                                                
                                                MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                          STRLN("{")
                                                                          STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                          STRIDTLN(STRMK("kind") ":" STRMK("direct") ",")
                                                                          STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                          STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                          STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])) + ",")
                                                                          STRIDTLN(STRMKPAIR("canvas_pos", STRV2(canvas_pos)) + ",")
                                                                          STRIDTLN(STRMKPAIR("world_pos", STRV2(transformed_pos)))
                                                                          STRLN("},")
                                                                          );
                                                
                                                {
                                                    const float32 bottom_panel_height = dt::bottom_panel_height;
                                                    if (canvas_pos.y > (core->viewport.height - (bottom_panel_height))) {
                                                        auto* dt_ctx = dt::DrawTalk::ctx();
                                                        auto& dock = dt_ctx->ui.dock;
                                                        auto* button  = dock.label_to_button["color"];
                                                        //auto& label_to_config = dock.label_to_config;
                                                        //auto* conf = &label_to_config[button->label];
                                                        mtt::Thing* thing = dt_ctx->mtt->Thing_get(button->thing);
                                                        mtt::Rep* rep = mtt::rep(thing);
                                                        auto& bound = rep->colliders.back()->aabb.saved_box;
                                                        if (canvas_pos.x > m::max(max_x_bottom_left_panel, bound.br.x)) {
                                                            {
                                                                
                                                                // tapped background
                                                                //
//                                                                dt::Selection_Recorder_clear_selections(&world->dt.recorder.selection_recording);
//
//                                                                control->most_recent_bg_touch_canvas      = vec3(canvas_pos, 0.0f);
//                                                                control->most_recent_bg_touch_transformed = vec3(transformed_pos, 0.0f);
//
//                                                                auto* dt_ctx = dt::DrawTalk::ctx();
//                                                                if ((true) || (dt::get_confirm_phase() != dt::CONFIRM_PHASE_ANIMATE && !dt_ctx->is_waiting_on_results)) {
//                                                                    dt::record_location_selection(dt_ctx, vec3(transformed_pos, 0.0f), canvas_pos, core->time_milliseconds, event.timestamp);
//                                                                }
//
//                                                                world->dt.scn_ctx.thing_selected_drawable = mtt::Thing_ID_INVALID;
#if PAUSE_TRIGGER_IN_CORNER
                                                                float32 box[4] = {
                                                                    pause_trigger.tl.x,
                                                                    pause_trigger.tl.y,
                                                                    pause_trigger.br.x,
                                                                    pause_trigger.br.y,
                                                                };
                                                                if (mtt::Box_point_check(box, canvas_pos)) {
                                                                    MTT_Core_toggle_pause(core);
                                                                }
#endif
                                                                    
                                                                break;
                                                            }
                                                        } else if (canvas_pos.x < bound.tl.x || canvas_pos.y < bound.tl.y
                                                                || canvas_pos.x > bound.br.x || canvas_pos.y > bound.br.y) {
                                                            break;
                                                        }
                                                    }
                                                }

                                                
                                                vec2 pos = {};
                                                
                                                bool broad_hit_canvas = point_query(collision_system_canvas, 0, canvas_pos, &out_canvas);
                                                bool broad_hit = point_query(collision_system_world, 0, transformed_pos, &out_world);
                                                
                                                
                                                Collider* c_hit = nullptr;
                                                
                                                float32 min_area = POSITIVE_INFINITY;
                                                
                                                bool col_layers[]                 = {broad_hit_canvas, broad_hit};
                                                vec2 points_to_check[]            = {canvas_pos, transformed_pos};
                                                Collision_System* systems[] = {collision_system_canvas, collision_system_world};
                                                
                                                for (usize check_idx = 0; check_idx < 2 && !was_hit; check_idx += 1) {
                                                    if (!col_layers[check_idx]) {
                                                        continue;
                                                    }
                                                    
                                                    Hit hit;
                                                    was_hit = mtt::point_query_narrow_including_selections(systems[check_idx], points_to_check[check_idx], &c_hit, hit, &min_area, [](mtt::Collider* c, mtt::Thing* thing) {
                                                        return mtt::priority_layer_is_equal(c->priority_layer, mtt::world(thing)->priority_layer);
                                                    });
                                                    
                                                    pos = points_to_check[check_idx];
                                                }
                                                if (was_hit) {
                                                    Thing_ID thing_ID = (Thing_ID)c_hit->user_data;
                                                    if (thing_ID == world->dt.scn_ctx.thing_selected_with_pen) {
                                                        break;
                                                    }
                                                    
                                                    Thing* thing = nullptr;
                                                    if (world->mtt_world.Thing_try_get(thing_ID, &thing)) {
                                                        if (thing->is_locked) {
                                                            break;
                                                        }
                                                    } else {
                                                        break;
                                                    }
                                                    
                                                    mtt::Thing_print(&world->mtt_world, thing);
                                                    
                                                    
                                                    {
                                                        world->dt.scn_ctx.selected_things.insert(thing_ID);
                                                        
                                                        
                                                        if (mtt::Thing_marked_for_moving(thing, event.key)){
                                                            auto find_it = world->dt.selection_map.find(event.key);
                                                            if (find_it != world->dt.selection_map.end()) {
                                                                dt::Selection* selection = &find_it->second;
                                                                selection->thing = thing_ID;
                                                                //selection->offset = pos;
                                                                //selection->count += 1;
                                                                //selection->time = event.timestamp;
                                                                break;
                                                            } else {
                                                                dt::Selection selection;
                                                                selection.thing = thing_ID;
                                                                selection.offset = pos;
                                                                //selection.count = 1;
                                                                selection.time = event.timestamp;
                                                                world->dt.selection_map[event.key] = selection;
                                                                
                                                            }
                                                            
                                                            
                                                            mtt::increment_selection_count(thing);
                                                            
                                                        } else {
                                                            break;
                                                        }
                                                        
                                                        
                                                        
                                                        
                                                        world->dt.ui.recording_indicator.is_on = true;
                                                        world->dt.ui.changed = true;
                                                        
                                                        
                                                        mtt::Thing_Archetype* arch = nullptr;
                                                        world->mtt_world.Thing_Archetype_get(thing->archetype_id, &arch);
                                                        // recording begin
                                                        bool should_forward_input_to_root = false;
                                                        if (!arch->ignore_selection && (mtt::is_user_movable(thing) || mtt::Thing_is_proxy(thing))) {
                                                            
                                                            mtt::set_thing_most_recently_selected_with_touch(&world->mtt_world, thing_ID, __LINE__, __FILE__);
                                                            
                                                        } else if (thing->forward_input_to_root) {
                                                            mtt::Thing* root = mtt::get_root(thing);
                                                            if (root != thing && root->is_user_movable) {
                                                                root->input_handlers.do_on_touch_input_began(root, vec3(transformed_pos, 0.0f), canvas_pos, &event, touch, 0);
                                                                
                                                                should_forward_input_to_root = true;
                                                            }
                                                        }
                                                        
                                                        if (thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH || thing->archetype_id == mtt::ARCHETYPE_NUMBER || thing->archetype_id == mtt::ARCHETYPE_TEXT) {
                                                            
                                                            if (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND))) {
                                                                dt::erase_selection(&world->dt, thing->id);
                                                            } else {
                                                                dt::record_selection(&world->dt, thing->id, core->time_milliseconds, event.timestamp, &event);
                                                            }
                                                            
                                                            
                                                        }
                                                        if (mtt::is_actor(thing)) {
                                                            if (!mtt::Thing_ID_is_valid(world->dt.scn_ctx.thing_selected_drawable) && !mtt::Thing_is_proxy(thing)) {
                                                                world->dt.scn_ctx.thing_selected_drawable = thing->id;
                                                            }
                                                            
//                                                            {
//                                                                bool arrows_ui_selected = false;
//                                                                {
//                                                                    auto& mp = dt::DrawTalk::ctx()->ui.dock.label_to_button;
//                                                                    auto button_find = mp.find("draw arrows");
//                                                                    if (button_find != mp.end()) {
//                                                                        auto* button = button_find->second;
//                                                                        mtt::Thing* arrows_thing = mtt::Thing_try_get(&world->mtt_world, button->thing);
//                                                                        if (arrows_thing != nullptr) {
//                                                                            usize selected_count = mtt::selection_count(arrows_thing);
//                                                                            arrows_ui_selected = (selected_count > 0);
//                                                                        }
//                                                                    }
//                                                                }
//                                                                
//                                                                if (!mtt::Thing_is_proxy(thing) && (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_ALTERNATE)) || arrows_ui_selected)) {
//                                                                    //MTT_print("ALT DOWN~!\n");
//                                                                    thing_arrows.is_active = true;
//                                                                    thing_arrows.clear();
//                                                                    thing_arrows.color = vec4(0.8f, 0.8f, 0.8f, 1.0f);
//                                                                    thing_arrows.radius = 1.0f;
//                                                                    thing_arrows.push(vec3(transformed_pos, 0.0f));
//                                                                    thing_arrows.deferred.clear();
//                                                                    thing_arrows.prev_in_chain = thing->id;
//                                                                    thing_arrows.input_key = event.key;
//                                                                    thing_arrows.canvas_pos = canvas_pos;
//                                                                    
//                                                                    
//                                                                } else {
//                                                                    //MTT_print("ALT UP~!\n");
//                                                                }
//                                                            }
                                                        }
                                                        
                                                        if (dt::can_label_thing(thing)) {
                                                            try_labeling_via_deixis_match(dt::DrawTalk::ctx());
                                                        }
                                                        
                                                        if (!should_forward_input_to_root) {
                                                            thing->input_handlers.do_on_touch_input_began(thing, vec3(transformed_pos, 0.0f), canvas_pos, &event, touch, 0);
                                                        }
                                                        
                                                        
                                                        
                                                        
                                                    }
                                                } else {
                                                
                                                    // tapped background
                                                    //
                                                    
                                                    
                                                    
                                                    auto* dt_ctx = dt::DrawTalk::ctx();
                                                    
                                                    dt::Selection_Recorder_clear_selections(&dt_ctx->recorder.selection_recording);
                                                    
                                                    control->most_recent_bg_touch_canvas      = vec3(canvas_pos, 0.0f);
                                                    control->most_recent_bg_touch_transformed = vec3(transformed_pos, 0.0f);
                                                    
                                                    
                                                    if ((true) || (dt::get_confirm_phase() != dt::CONFIRM_PHASE_ANIMATE && !dt_ctx->is_waiting_on_results)) {
                                                        dt::record_location_selection(dt_ctx, vec3(transformed_pos, 0.0f), canvas_pos, core->time_milliseconds, event.timestamp);
                                                    }
                                                    
                                                    dt_ctx->scn_ctx.thing_selected_drawable = mtt::Thing_ID_INVALID;
                                                    
                                                    //Context_View_disable(dt_ctx);
                                                    
                                                    
                                                    
                                                    //dt::DrawTalk::ctx()->lang_ctx.deixis_reset();
                                                }
                                                break;
                                            }
                                            case UI_TOUCH_STATE_MOVED: {
                                                auto result = world->dt.selection_map.find(event.key);
                                                if (result == world->dt.selection_map.end()) {
                                                    MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT_MOVE,
                                                                              STRLN("{")
                                                                              STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                              STRIDTLN(STRMK("kind") ":" STRMK("direct") ",")
                                                                              STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                              STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                              STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])))
                                                                              STRLN("},")
                                                                              );
                                                    break;
                                                }
                                                
                                                dt::Selection* selection = &result->second;
                                                //MTT_print("moving the selected object\n");
                                                
                                                auto* cam = (mtt::Camera*)Input_get_user_data_for_touch(u_input, touch);
                                                if (cam == nullptr) {
                                                    MTT_error("%s\n", "camera is null for some reason");
                                                    cam = &world->cam;
                                                }
                                                
                                                vec2 canvas_pos = direct->positions[direct->count - 1];
                                                vec2 transformed_pos = mtt::transform_point(cam, canvas_pos);
                                                
                                                MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT_MOVE,
                                                                          STRLN("{")
                                                                          STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                          STRIDTLN(STRMK("kind") ":" STRMK("direct") ",")
                                                                          STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                          STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                          STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])) + ",")
                                                                          STRIDTLN(STRMKPAIR("canvas_pos", STRV2(canvas_pos)) + ",")
                                                                          STRIDTLN(STRMKPAIR("world_pos", STRV2(transformed_pos)))
                                                                          STRLN("},")
                                                                          );
                                                
                                                if (thing_arrows.is_active) {
                                                    if (event.key != thing_arrows.input_key) {
                                                        break;
                                                    }
                                                    thing_arrows.push(vec3(transformed_pos, 0.0f));
                                                    {
                                                        mtt::Thing* thing = nullptr;
                                                        if (!world->mtt_world.Thing_try_get(selection->thing, &thing) || mtt::Thing_is_proxy(thing)) {
                                                            break;
                                                        }
                                                        
                                                        static Collider_List out_canvas;
                                                        out_canvas.clear();
                                                        
                                                        bool was_hit = false;
                                                        
                                                        auto* collision_system = &mtt::world(thing)->collision_system;
                                                        
                                                        {
                                                            mtt::Collider* c_hit = nullptr;
                                                            
                                                            float32 min_area = POSITIVE_INFINITY;
                                                            
                                                            //auto& dt_ctx = *dt::DrawTalk::ctx();
                                                            
                                                            mtt::Hit hit;
                                                            was_hit = mtt::point_query_narrow_including_selections(collision_system, transformed_pos, &c_hit, hit, &min_area, [](mtt::Collider* c, mtt::Thing* thing) {
                                                                return mtt::priority_layer_is_equal(c->priority_layer, mtt::world(thing)->priority_layer);
                                                            });
                                                            
                                                            if (was_hit) {
                                                                mtt::Thing_ID other_id = (mtt::Thing_ID)c_hit->user_data;
                                                                mtt::Thing* other = thing->world()->Thing_try_get(other_id);
                                                                if (other != nullptr && mtt::is_actor(other)) {
                                                                    if (thing_arrows.deferred.size() > 0) {
                                                                        mtt::Thing_ID prev_first = thing_arrows.deferred.back().first;
                                                                        mtt::Thing_ID prev_second = thing_arrows.deferred.back().second;
                                                                        if (prev_first != thing_arrows.prev_in_chain || prev_second != other_id) {
                                                                            if (thing_arrows.prev_in_chain != other_id) {
                                                                                thing_arrows.deferred.push_back({thing_arrows.prev_in_chain, other_id});
                                                                                thing_arrows.prev_in_chain = other_id;
                                                                            }
                                                                        }
                                                                    } else {
                                                                        mtt::Thing_ID prev_in_chain = thing_arrows.prev_in_chain;
                                                                        if (prev_in_chain != other_id) {
                                                                            thing_arrows.deferred.push_back({prev_in_chain, other_id});
                                                                            thing_arrows.prev_in_chain = other_id;
                                                                        }
                                                                    }
                                                                    
                                                                }
                                                            } else {
                                                                
                                                                
                                                            }
                                                            //#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
                                                            
                                                            thing_arrows.canvas_pos = canvas_pos;
                                                            //#endif
                                                        }
                                                    }
                                                    break;
                                                }
                                                
                                                mtt::Thing* thing = nullptr;
                                                if (!world->mtt_world.Thing_try_get(selection->thing, &thing)) {
                                                    break;
                                                }
                                                mtt::Thing_ID thing_ID = thing->id;
                                                
                                                mtt::Thing_Archetype* arch = nullptr;
                                                world->mtt_world.Thing_Archetype_get(thing->archetype_id, &arch);
                                                
                                                if (mtt::Thing_marked_for_moving(thing, event.key)) {
                                                    
                                                    if (world->dt.scn_ctx.thing_selected_with_pen != thing->id && mtt::is_user_movable(thing) && !arch->ignore_selection ) {
                                                        
                                                        mtt::Representation& rep = *mtt::rep(thing);
                                                        
                                                        Collider* collider = rep.colliders.back();
                                                        
                                                        vec2 translation;
                                                        vec2 pos = (thing->lock_to_canvas) ? canvas_pos : transformed_pos;
                                                        {
                                                            translation = pos - selection->offset;
                                                            selection->offset = pos;
                                                            
                                                            Collider_remove(collider->system, 0, collider);
                                                            
                                                            //std::cout << m::to_string(translation) << std::endl;
                                                            collider->aabb.tl += translation;
                                                            collider->aabb.br += translation;
                                                            
                                                            collider->center_anchor = vec3((collider->aabb.tl + collider->aabb.br), 0.0f) * 0.5f;
                                                            
                                                            vec3 d_scale;
                                                            quat d_orientation;
                                                            vec3 d_translation;
                                                            vec3 d_skew;
                                                            vec4 d_perspective;
                                                            
                                                            m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                                            collider->transform = rep.pose_transform * m::toMat4(d_orientation);
                                                            
                                                            push_AABBs(collider->system, collider, 1, 0);
                                                            
                                                            
                                                            
                                                            rep.transform.translation += vec3(translation, 0.0);
                                                            
                                                            translation = adjust_translation_in_hierarchy(translation, thing);
                                                            
                                                            rep.model_transform =  m::translate(mat4(1.0f), vec3(translation, 0.0f)) * rep.model_transform;
                                                            rep.model_transform_inverse = m::inverse(rep.model_transform);
                                                            
                                                        }
                                                        {
                                                            auto* position = mtt::access<vec3>(thing, "position");
                                                            
                                                            if (position != nullptr) {
                                                                *position = collider->center_anchor;
                                                            }
                                                        }
                                                        
                                                    } else if (thing->forward_input_to_root) {
                                                        
                                                        mtt::Thing* root = mtt::get_root(thing);
                                                        if (root != thing && root->is_user_movable ) {
                                                            
                                                            mtt::Representation& rep = *mtt::rep(root);
                                                            
                                                            
                                                            
                                                            vec2 translation;
                                                            vec2 pos = (root->lock_to_canvas) ? canvas_pos : transformed_pos;
                                                            Collider* collider = nullptr;
                                                            {
                                                                translation = pos - selection->offset;
                                                                selection->offset = pos;
                                                                
                                                                collider = (rep.colliders.empty()) ? nullptr : rep.colliders.front();
                                                                if (collider != nullptr) {
                                                                    Collider_remove(collider->system, 0, collider);
                                                                    
                                                                    collider->aabb.tl += translation;
                                                                    collider->aabb.br += translation;
                                                                    
                                                                    collider->center_anchor = vec3((collider->aabb.tl + collider->aabb.br), 0.0f) * 0.5f;
                                                                    
                                                                    vec3 d_scale;
                                                                    quat d_orientation;
                                                                    vec3 d_translation;
                                                                    vec3 d_skew;
                                                                    vec4 d_perspective;
                                                                    
                                                                    m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                                                    collider->transform = rep.pose_transform * m::toMat4(d_orientation);
                                                                    
                                                                    push_AABBs(collider->system, collider, 1, 0);
                                                                }
                                                                
                                                                rep.transform.translation += vec3(translation, 0.0);
                                                                
                                                                translation = adjust_translation_in_hierarchy(translation, thing);
                                                                
                                                                rep.model_transform = m::translate(vec3(translation, 0.0f)) * rep.model_transform;   rep.model_transform_inverse = m::inverse(rep.model_transform);
                                                            }
                                                            {
                                                                auto* position = mtt::access<vec3>(root, "position");
                                                                
                                                                if (position != nullptr) {
                                                                    *position = (collider == nullptr) ? rep.transform.translation : collider->center_anchor;
                                                                }
                                                            }
                                                            
                                                            root->input_handlers.do_on_touch_input_moved(root, vec3(transformed_pos, 0.0f), canvas_pos, &event, touch, 0);
                                                        }
                                                    }
                                                    
                                                    
                                                    thing->input_handlers.do_on_touch_input_moved(thing, vec3(transformed_pos, 0.0f), canvas_pos, &event, touch, 0);
                                                    
                                                }
                                                
                                                //
                                                break;
                                            }
                                            case UI_TOUCH_STATE_CANCELLED:
                                                // fallthrough
                                            case UI_TOUCH_STATE_ENDED: {
                                                
                                                if (control->touch.count_active > 0) {
                                                    control->touch.count_active -= 1;
                                                }
                                                
                                                auto result = world->dt.selection_map.find(event.key);
                                                if (result == world->dt.selection_map.end()) {
                                                    MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                              STRLN("{")
                                                                              STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                              STRIDTLN(STRMK("kind") ":" STRMK("direct") ",")
                                                                              STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                              STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                              STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])))
                                                                              STRLN("},")
                                                                              );
                                                    break;
                                                }
                                                
                                                world->dt.ui.recording_indicator.is_on = false;
                                                world->dt.ui.changed = true;
                                                
                                                
                                                dt::Selection selection = result->second;
                                                world->dt.selection_map.erase(result);
                                                
                                                auto select_find = world->dt.scn_ctx.selected_things.find(selection.thing);
                                                if (select_find != world->dt.scn_ctx.selected_things.end()) {
                                                    world->dt.scn_ctx.selected_things.erase(selection.thing);
                                                }
                                                
                                                
                                                mtt::Thing* thing = nullptr;
                                                if (!world->mtt_world.Thing_try_get(selection.thing, &thing)) {
                                                    MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                              STRLN("{")
                                                                              STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                              STRIDTLN(STRMK("kind") ":" STRMK("direct") ",")
                                                                              STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                              STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                              STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])))
                                                                              STRLN("},")
                                                                              );
                                                    break;
                                                }
                                                
                                                mtt::decrement_selection_count(thing);
                                                
                                                auto* cam = (mtt::Camera*)Input_get_user_data_for_touch(u_input, touch);
                                                
                                                if (cam == nullptr) {
                                                    MTT_error("%s\n", "camera is null for some reason");
                                                    cam = &world->cam;
                                                }
                                                
                                                vec2 canvas_pos = direct->positions[direct->count - 1];
                                                vec2 transformed_pos = mtt::transform_point(cam, canvas_pos);
                                                
                                                MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                                                          STRLN("{")
                                                                          STRIDTLN(STRMK("type") ":" STRMK("input") ",")
                                                                          STRIDTLN(STRMK("kind") ":" STRMK("direct") ",")
                                                                          STRIDTLN(STRMK("t") ":" + mtt::to_str(event.timestamp) + ",")
                                                                          STRIDTLN(STRMKPAIR("event_mode", STRVAR( dt::SP_STYLUS_MODE_AS_STRING[control->stylus.mode])) + ",")
                                                                          STRIDTLN(STRMKPAIR("state", STRVAR(ui_state_type_to_string[event.state_type])) + ",")
                                                                          STRIDTLN(STRMKPAIR("canvas_pos", STRV2(canvas_pos)) + ",")
                                                                          STRIDTLN(STRMKPAIR("world_pos", STRV2(transformed_pos)))
                                                                          STRLN("},")
                                                                          );
                                                
                                                mtt::Representation& rep = *mtt::rep(thing);
                                                Collider* collider = rep.colliders.back();
                                                if (rep.colliders.empty() || collider == nullptr) {
                                                    break;
                                                }
                                                
                                                vec2 pos = (thing->lock_to_canvas) ? canvas_pos : transformed_pos;
                                                
                                                mtt::Thing_ID thing_ID = (mtt::Thing_ID)collider->user_data;
                                                
                                                mtt::Thing_Archetype* arch = nullptr;
                                                world->mtt_world.Thing_Archetype_get(thing->archetype_id, &arch);
                                                
                                                if (mtt::Thing_marked_for_moving(thing, event.key)) {
                                                    
                                                    // check if the movement should be ignored
                                                    if (event.flags != UI_EVENT_FLAG_CONTEXT_MODE) {
                                                        if (mtt::is_user_movable(thing) && !arch->ignore_selection && thing->id != world->dt.scn_ctx.thing_selected_with_pen && !thing_arrows.is_active ) {
                                                            vec2 translation = pos - selection.offset;
                                                            selection.offset = pos;
                                                            
                                                            Collider_remove(collider->system, 0, collider);
                                                            
                                                            collider->aabb.tl += translation;
                                                            collider->aabb.br += translation;
                                                            
                                                            collider->center_anchor = vec3((collider->aabb.tl + collider->aabb.br), 0.0f) * 0.5f;
                                                            
                                                            vec3 d_scale;
                                                            quat d_orientation;
                                                            vec3 d_translation;
                                                            vec3 d_skew;
                                                            vec4 d_perspective;
                                                            
                                                            m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                                            collider->transform = rep.pose_transform * m::toMat4(d_orientation);
                                                            
                                                            push_AABBs(collider->system, collider, 1, 0);
                                                            
                                                            rep.transform.translation += vec3(translation, 0.0);
                                                            
                                                            translation = adjust_translation_in_hierarchy(translation, thing);
                                                            
                                                            rep.model_transform = rep.model_transform = m::translate(vec3(translation, 0.0f)) * rep.model_transform;
                                                            rep.model_transform_inverse = m::inverse(rep.model_transform);
                                                            
                                                            {
                                                                auto* position = mtt::access<vec3>(thing, "position");
                                                                
                                                                if (position != nullptr) {
                                                                    *position = collider->center_anchor;
                                                                }
                                                            }
                                                            
                                                        } else if (thing->forward_input_to_root) {
                                                            
                                                            mtt::Thing* root = mtt::get_root(thing);
                                                            if (root != thing) {
                                                                
                                                                if (!thing_arrows.is_active ) {
                                                                    vec2 translation = pos - selection.offset;
                                                                    selection.offset = pos;
                                                                    
                                                                    mtt::Rep& rep = *mtt::rep(root);
                                                                    
                                                                    mtt::Collider* collider = (rep.colliders.empty()) ? nullptr : rep.colliders.front();
                                                                    if (collider != nullptr) {
                                                                        Collider_remove(collider->system, 0, collider);
                                                                        
                                                                        collider->aabb.tl += translation;
                                                                        collider->aabb.br += translation;
                                                                        
                                                                        
                                                                        collider->center_anchor = vec3((collider->aabb.tl + collider->aabb.br), 0.0f) * 0.5f;
                                                                        
                                                                        vec3 d_scale;
                                                                        quat d_orientation;
                                                                        vec3 d_translation;
                                                                        vec3 d_skew;
                                                                        vec4 d_perspective;
                                                                        
                                                                        m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                                                        collider->transform = rep.pose_transform * m::toMat4(d_orientation);
                                                                        
                                                                        push_AABBs(collider->system, collider, 1, 0);
                                                                    }
                                                                    
                                                                    rep.transform.translation += vec3(translation, 0.0);
                                                                    
                                                                    translation = adjust_translation_in_hierarchy(translation, thing);
                                                                    
                                                                    rep.model_transform = rep.model_transform = m::translate(vec3(translation, 0.0f)) * rep.model_transform;
                                                                    rep.model_transform_inverse = m::inverse(rep.model_transform);
                                                                    
                                                                    {
                                                                        auto* position = mtt::access<vec3>(root, "position");
                                                                        
                                                                        if (position != nullptr) {
                                                                            *position = (collider == nullptr) ? rep.transform.translation : collider->center_anchor;
                                                                        }
                                                                    }
                                                                }
                                                                
                                                                root->input_handlers.do_on_touch_input_ended(root, vec3(transformed_pos, 0.0f), canvas_pos, &event, touch, 0);
                                                            }
                                                        }
                                                    } else if (thing->forward_input_to_root) {
                                                        mtt::Thing* root = mtt::get_root(thing);
                                                        if (root != thing) {
                                                            root->input_handlers.do_on_touch_input_ended(root, vec3(transformed_pos, 0.0f), canvas_pos, &event, touch, 0);
                                                        }
                                                    }
                                                    
                                                    thing->input_handlers.do_on_touch_input_ended(thing, vec3(transformed_pos, 0.0f), canvas_pos, &event, touch, 0);
                                                    
                                                    
                                                }
                                                
                                                mtt::Thing_unmark_for_moving(thing, event.key);

                                                
                                                
                                                if (mtt::get_thing_most_recently_selected_with_touch(&world->mtt_world) == thing->id) {
                                                    mtt::set_thing_most_recently_selected_with_touch(&world->mtt_world, mtt::Thing_ID_INVALID, __LINE__, __FILE__);
                                                }
                                                
                                                break;
                                            }
#define MTT_Input_should_allow_pan() ((u_input->direct_map.size() == 2) || event.count == (~0))
                                            case UI_TOUCH_STATE_PAN_GESTURE_BEGAN:
                                                if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    break;
                                                }
                                                
                                                if (MTT_Input_should_allow_pan()) {
                                                    UI_Touch* touch_input = nullptr;
                                                    mtt::Camera* cam = nullptr;
                                                    cam_info_choose(world, event.position, &cam);
                                                    
                                                    Input_set_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PAN, &event, &touch_input, cam);
                                                    
                                                    if (mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_TRANSLATION)) {
                                                        
                                                        core->view_position -= vec3(event.translation, 0.0f);
                                                        cam->cam_transform = cam->cam_transform * m::translate(Mat4(1.0f), -vec3(event.translation, 0.0f));
                                                        mtt::calc_view_matrix(cam);
                                                    }
                                                } else
                                                {
                                                    
                                                    //                                    world->ct.ui.hsv_slider -= event.translation.y;
                                                    //                                    world->ct.ui.hsv_slider = m::clamp((float64)world->ct.ui.hsv_slider, -core->viewport.height, core->viewport.height);
                                                    //
                                                    //
                                                    //                                    world->ct.pen.color = (m::abs(world->ct.ui.hsv_slider) == core->viewport.height) ? vec4(1.0f) : HSVtoRGBinplace((uint32)(m::abs(world->ct.ui.hsv_slider / core->viewport.height) * 359), 1.0, 1.0, 1.0f);
                                                    //
                                                    //
                                                    //                                    world->ct.ui.changed = true;
                                                }
                                                //
                                                break;
                                            case UI_TOUCH_STATE_PAN_GESTURE_CHANGED: {
                                                if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    break;
                                                }
                                                
                                                if (MTT_Input_should_allow_pan()) {
                                                    //auto* cam = &world->cam;
                                                    
                                                    UI_Touch* touch_input = nullptr;
                                                    auto* cam = (mtt::Camera*)Input_get_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PAN, &event, &touch_input);
                                                    if (cam == nullptr) {
                                                        cam_info_choose(world, event.position, &cam);
                                                        Input_set_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PAN, &event, &touch_input, cam);
                                                    }
                                                    if (mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_TRANSLATION)) {
                                                        
                                                        core->view_position -= vec3(event.translation, 0.0f);
                                                        cam->cam_transform = cam->cam_transform * m::translate(Mat4(1.0f), -vec3(event.translation, 0.0f));
                                                        mtt::calc_view_matrix(cam);
                                                    }
                                                    
                                                } else
                                                {
                                                    
                                                    //                                    world->ct.ui.hsv_slider -= event.translation.y;
                                                    //                                    world->ct.ui.hsv_slider = m::clamp((float64)world->ct.ui.hsv_slider, -core->viewport.height, core->viewport.height);
                                                    //
                                                    //
                                                    //                                    world->ct.pen.color = (m::abs(world->ct.ui.hsv_slider) == core->viewport.height) ? vec4(1.0f) : HSVtoRGBinplace((uint32)(m::abs(world->ct.ui.hsv_slider / core->viewport.height) * 359), 1.0, 1.0, 1.0f);
                                                    //
                                                    //
                                                    //                                    world->ct.ui.changed = true;
                                                }
                                                break;
                                            }
                                            case UI_TOUCH_STATE_PAN_GESTURE_CANCELLED:
                                                // fallthrough
                                            case UI_TOUCH_STATE_PAN_GESTURE_ENDED: {
                                                if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    break;
                                                }
                                                
                                                if (MTT_Input_should_allow_pan()) {
                                                    
                                                    
                                                    //auto* cam = &world->cam;
                                                    
                                                    UI_Touch* touch_input = nullptr;
                                                    auto* cam = (mtt::Camera*)Input_get_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PAN, &event, &touch_input);
                                                    if (cam == nullptr) {
                                                        cam_info_choose(world, event.position, &cam);
                                                        Input_set_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PAN, &event, &touch_input, cam);
                                                    }
                                                    if (cam != nullptr) {
                                                        if (mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_TRANSLATION)) {
                                                            cam->cam_transform = cam->cam_transform * m::translate(Mat4(1.0f), -vec3(event.translation, 0.0f));
                                                            mtt::calc_view_matrix(cam);
                                                        }
                                                    } else {
                                                        
                                                        MTT_error("%s\n", "camera is null!");
                                                    }
                                                    
                                                    
                                                } else
                                                {
                                                    //                                    world->ct.ui.hsv_slider -= event.translation.y;
                                                    //                                    world->ct.ui.hsv_slider = m::clamp((float64)world->ct.ui.hsv_slider, -core->viewport.height, core->viewport.height);
                                                    //
                                                    //
                                                    //                                    world->ct.pen.color = (m::abs(world->ct.ui.hsv_slider) == core->viewport.height) ? vec4(1.0f) : HSVtoRGBinplace((uint32)(m::abs(world->ct.ui.hsv_slider / core->viewport.height) * 359), 1.0, 1.0, 1.0f);
                                                    //
                                                    //                                    world->ct.ui.changed = true;
                                                }
                                                break;
                                            }
                                            case UI_TOUCH_STATE_PINCH_GESTURE_BEGAN: {
                                                mtt::Camera* cam = nullptr;
                                                cam_info_choose(world, event.position, &cam);
                                                UI_Touch* touch_input = nullptr;
                                                Input_set_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input, cam);
#define MTT_ENABLE_PINCH_TO_SCALE_THING (true)
                                                if (thing_arrows.is_active) {
                                                    break;
                                                } else if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    if (!MTT_ENABLE_PINCH_TO_SCALE_THING || !mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_SCALE)) {
                                                        break;
                                                    }
                                                    
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                                                    {
                                                        
                                                        auto& ui = dt::ctx()->ui;
                                                        mtt::Thing* ui_button = world->mtt_world.Thing_get(ui.dock.buttons[ui.dock.label_to_index.find("scale")->second]->thing);
                                                        
                                                        dt::UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<dt::UI_Button*>(ui_button, "ctx_ptr");
                                                        if (ctx_ptr->flags == 0) {
                                                            Input_set_flags_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input, 0);
                                                            break;
                                                        } else {
                                                            Input_set_flags_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input, 1);
                                                        }
//                                                        auto* const state = &ui_button->input_handlers.state[mtt::INPUT_MODALITY_FLAG_TOUCH];
//                                                        if (state->state == UI_TOUCH_STATE_NONE) {
//                                                            MTT_print("%s\n", "disabled");
//                                                            Input_set_flags_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input, 0);
//                                                            break;
//                                                        } else {
//                                                            MTT_print("%s\n", "enabled");
//                                                            Input_set_flags_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input, 1);
//                                                        }
                                                        
                                                    }
#endif
                                                    
                                                    for (auto it = world->dt.scn_ctx.selected_things.begin(); it != world->dt.scn_ctx.selected_things.end(); ++it) {
                                                                                                                
                                                        mtt::Thing* thing = nullptr;
                                                        if (!world->mtt_world.Thing_try_get(*it, &thing)) {
                                                            break;
                                                        }
                                                        
                                                        if (thing->lock_to_canvas) {
                                                            break;
                                                        }
                                                        
                                                        mtt::Rep* repre = mtt::rep(thing);
                                                        
                                                        Mat4 M = m::scale(Mat4(1.0f), vec3(event.scale, 1.0f));
                                                        
                                                        set_pose_transform(thing, M * repre->pose_transform);
                                                        
                                                        for (usize i = 0; i < repre->colliders.size(); i += 1) {
                                                            mtt::Collider_remove(repre->colliders[i]->system, 0, repre->colliders[i]);
                                                            repre->colliders[i]->transform = repre->pose_transform;
                                                            mtt::push_AABBs(repre->colliders[i]->system, repre->colliders[i], 1, 0);
                                                        }
                                                    }
                                                    
                                                    break;
                                                } else {
                                                    
                                                    if (event.scale.x == 0.0f) {
                                                        MTT_error("%s", "ERROR: zoom should not be 0");
                                                        break;
                                                    }
                                                    
                                                    
                                                    Mat4 M = m::translate(Mat4(1.0f), vec3(event.position, 0.0f)) *
                                                    m::scale(Mat4(1.0f), vec3(1.0f / event.scale, 1.0f)) *
                                                    m::translate(Mat4(1.0f), -vec3(event.position, 0.0f));
                                                    
                                                    if (mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_SCALE)) {
                                                        cam->cam_transform = cam->cam_transform * M;
                                                        mtt::calc_view_matrix(cam);
                                                    }
                                                    
                                                    break;
                                                }
                                            }
                                            case UI_TOUCH_STATE_PINCH_GESTURE_CHANGED: {
                                                
                                                UI_Touch* touch_input = nullptr;
                                                auto* cam = (mtt::Camera*)Input_get_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input);
                                                if (cam == nullptr) {
                                                    cam_info_choose(world, event.position, &cam);
                                                    Input_set_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input, cam);
                                                }
                                                if (thing_arrows.is_active) {
                                                    break;
                                                } else if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    
                                                    if (!MTT_ENABLE_PINCH_TO_SCALE_THING || !mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_SCALE)) {
                                                        break;
                                                    }
                                                    
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                                                    {
                                                        
//                                                        auto& ui = dt::ctx()->ui;
//                                                        mtt::Thing* ui_button = world->mtt_world.Thing_get(ui.dock.buttons[ui.dock.label_to_index.find("scale")->second]->thing);
//                                                        
//                                                        auto* const state = &ui_button->input_handlers.state[mtt::INPUT_MODALITY_FLAG_TOUCH];
//                                                        if (state->state == UI_TOUCH_STATE_NONE) {
//                                                            break;
//                                                        }
                                                        if (Input_get_flags_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input) == 0) {
                                                            break;
                                                        }
                                                        
                                                    }
#endif
                                                    
                                                    for (auto it = world->dt.scn_ctx.selected_things.begin(); it != world->dt.scn_ctx.selected_things.end(); ++it) {
                                                                                                                
                                                        mtt::Thing* thing = nullptr;
                                                        if (!world->mtt_world.Thing_try_get(*it, &thing)) {
                                                            break;
                                                        }
                                                        
                                                        if (thing->lock_to_canvas) {
                                                            break;
                                                        }
                                                        
                                                        mtt::Rep* repre = mtt::rep(thing);
                                                        
                                                        Mat4 M = m::scale(Mat4(1.0f), vec3(event.scale, 1.0f));
                                                        
                                                        set_pose_transform(thing, M * repre->pose_transform);
                                                        
                                                        for (usize i = 0; i < repre->colliders.size(); i += 1) {
                                                            mtt::Collider_remove(repre->colliders[i]->system, 0, repre->colliders[i]);
                                                            repre->colliders[i]->transform = repre->pose_transform;
                                                            mtt::push_AABBs(repre->colliders[i]->system, repre->colliders[i], 1, 0);
                                                        }
                                                    }
                                                    
                                                    break;
                                                } else {
                                                    
                                                    if (event.scale.x == 0.0f) {
                                                        MTT_error("%s", "ERROR: zoom should not be 0");
                                                        break;
                                                    }
                                                    
                                                    
                                                    Mat4 M = m::translate(Mat4(1.0f), vec3(event.position, 0.0f)) *
                                                    m::scale(Mat4(1.0f), vec3(1.0f / event.scale, 1.0f)) *
                                                    m::translate(Mat4(1.0f), -vec3(event.position, 0.0f));
                                                    
                                                    
                                                    
                                                    if (mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_SCALE)) {
                                                        cam->cam_transform = cam->cam_transform * M;
                                                        mtt::calc_view_matrix(cam);
                                                    }
                                                    
                                                    break;
                                                }
                                            }
                                            case UI_TOUCH_STATE_PINCH_GESTURE_CANCELLED:
                                                // fallthrough
                                            case UI_TOUCH_STATE_PINCH_GESTURE_ENDED: {
                                                UI_Touch* touch_input = nullptr;
                                                auto* cam = (mtt::Camera*)Input_get_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input);
                                                if (cam == nullptr) {
                                                    cam_info_choose(world, event.position, &cam);
                                                    Input_set_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input, cam);
                                                }
                                                if (thing_arrows.is_active) {
                                                    break;
                                                } else if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    if (!MTT_ENABLE_PINCH_TO_SCALE_THING || !mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_SCALE)) {
                                                        break;
                                                    }
                                                    
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                                                    {
                                                        
//                                                        auto& ui = dt::ctx()->ui;
//                                                        mtt::Thing* ui_button = world->mtt_world.Thing_get(ui.dock.buttons[ui.dock.label_to_index.find("scale")->second]->thing);
//                                                        
//                                                        auto* const state = &ui_button->input_handlers.state[mtt::INPUT_MODALITY_FLAG_TOUCH];
//                                                        if (state->state == UI_TOUCH_STATE_NONE) {
//                                                            break;
//                                                        }
                                                        if (Input_get_flags_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_PINCH, &event, &touch_input) == 0) {
                                                            break;
                                                        }
                                                        
                                                    }
#endif
                                                    
                                                    for (auto it = world->dt.scn_ctx.selected_things.begin(); it != world->dt.scn_ctx.selected_things.end(); ++it) {
                                                                                                                
                                                        mtt::Thing* thing = nullptr;
                                                        if (!world->mtt_world.Thing_try_get(*it, &thing)) {
                                                            break;
                                                        }
                                                        
                                                        if (thing->lock_to_canvas) {
                                                            break;
                                                        }
                                                        
                                                        mtt::Rep* repre = mtt::rep(thing);
                                                        
                                                        Mat4 M = m::scale(Mat4(1.0f), vec3(event.scale, 1.0f));
                                                        
                                                        set_pose_transform(thing, M * repre->pose_transform);
                                                        
                                                        for (usize i = 0; i < repre->colliders.size(); i += 1) {
                                                            mtt::Collider_remove(repre->colliders[i]->system, 0, repre->colliders[i]);
                                                            repre->colliders[i]->transform = repre->pose_transform;
                                                            mtt::push_AABBs(repre->colliders[i]->system, repre->colliders[i], 1, 0);
                                                        }
                                                    }
                                                    
                                                    break;
                                                } else {
                                                    
                                                    if (event.scale.x == 0.0f) {
                                                        MTT_error("%s", "ERROR: zoom should not be 0");
                                                        break;
                                                    }
                                                    
                                                    
                                                    Mat4 M = m::translate(Mat4(1.0f), vec3(event.position, 0.0f)) *
                                                    m::scale(Mat4(1.0f), vec3(1.0f / event.scale, 1.0f)) *
                                                    m::translate(Mat4(1.0f), -vec3(event.position, 0.0f));
                                                        
                                                    
                                                    if (mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_SCALE)) {
                                                        cam->cam_transform = cam->cam_transform * M;
                                                        mtt::calc_view_matrix(cam);
                                                    }
                                                    
                                                    break;
                                                }
                                            }
                                            case UI_TOUCH_STATE_ROTATION_GESTURE_BEGAN: {
                                                mtt::Camera* cam = nullptr;
                                                cam_info_choose(world, event.position, &cam);
                                                UI_Touch* touch_input = nullptr;
                                                Input_set_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_ROTATION, &event, &touch_input, cam);
                                                
                                                if (thing_arrows.is_active) {
                                                    break;
                                                }
                                                
                                                // FIXME: need to rotate the objects instead of the world, canceling gestures instead for now
                                                if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    if (!mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_ROTATION)) {
                                                        break;
                                                    }
                                                    
                                                    if (world->dt.recorder.selection_recording.selections.size() > 1) {
                                                        auto it = world->dt.recorder.selection_recording.selections.end() - 1;
                                                        for (;it != world->dt.recorder.selection_recording.selections.begin(); --it) {
                                                            
                                                            mtt::Thing* thing = nullptr;
                                                            if (!world->mtt_world.Thing_try_get(it->ID, &thing)) {
                                                                continue;
                                                            }
                                                            
                                                            if (thing->lock_to_canvas) {
                                                                continue;
                                                            }
                                                            
                                                            
                                                            mtt::Rep* repre = mtt::rep(thing);
                                                            
                                                            
                                                            Mat4 M = m::rotate(Mat4(1.0f), event.rotation, vec3(0.0f, 0.0f, 1.0f));
                                                            
                                                            set_pose_transform(thing, M * repre->pose_transform);
                                                            
                                                            for (usize i = 0; i < repre->colliders.size(); i += 1) {
                                                                mtt::Collider_remove(repre->colliders[i]->system, 0, repre->colliders[i]);
                                                                repre->colliders[i]->transform = repre->pose_transform;
                                                                mtt::push_AABBs(repre->colliders[i]->system, repre->colliders[i], 1, 0);
                                                            }
                                                        }
                                                    } else {
                                                        auto& it = *world->dt.scn_ctx.selected_things.begin();
                                                        
                                                        mtt::Thing* thing = nullptr;
                                                        if (!world->mtt_world.Thing_try_get(it, &thing)) {
                                                            break;
                                                        }
                                                        
                                                        if (thing->lock_to_canvas) {
                                                            break;
                                                        }
                                                        
                                                        
                                                        mtt::Rep* repre = mtt::rep(thing);
                                                        
                                                        
                                                        Mat4 M = m::rotate(Mat4(1.0f), event.rotation, vec3(0.0f, 0.0f, 1.0f));
                                                        
                                                        set_pose_transform(thing, M * repre->pose_transform);
                                                        
                                                        for (usize i = 0; i < repre->colliders.size(); i += 1) {
                                                            mtt::Collider_remove(repre->colliders[i]->system, 0, repre->colliders[i]);
                                                            repre->colliders[i]->transform = repre->pose_transform;
                                                            mtt::push_AABBs(repre->colliders[i]->system, repre->colliders[i], 1, 0);
                                                        }
                                                    }
                                                    
                                                    break;
                                                }
                                                
                                                Mat4 M = m::translate(Mat4(1.0f), vec3(event.position, 0.0f)) * m::rotate(Mat4(1.0f), -event.rotation, vec3(0.0f, 0.0f, 1.0f)) *
                                                m::translate(Mat4(1.0f), vec3(-event.position, 0.0f));
                                                
                                                if (mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_ROTATION)) {
                                                    cam->cam_transform = cam->cam_transform * M;
                                                    mtt::calc_view_matrix(cam);
                                                }
                                                //
                                                break;
                                            }
                                            case UI_TOUCH_STATE_ROTATION_GESTURE_CHANGED: {
                                                
                                                UI_Touch* touch_input = nullptr;
                                                auto* cam = (mtt::Camera*)Input_get_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_ROTATION, &event, &touch_input);
                                                if (cam == nullptr) {
                                                    cam_info_choose(world, event.position, &cam);
                                                    Input_set_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_ROTATION, &event, &touch_input, cam);
                                                }
                                                if (thing_arrows.is_active) {
                                                    break;
                                                }
                                                if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    if (!mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_ROTATION)) {
                                                        break;
                                                    }
                                                    
                                                    if (world->dt.recorder.selection_recording.selections.size() > 1) {
                                                        auto it = world->dt.recorder.selection_recording.selections.end() - 1;
                                                        for (;it != world->dt.recorder.selection_recording.selections.begin(); --it) {
                                                            
                                                            mtt::Thing* thing = nullptr;
                                                            if (!world->mtt_world.Thing_try_get(it->ID, &thing)) {
                                                                continue;
                                                            }
                                                            
                                                            if (thing->lock_to_canvas) {
                                                                continue;
                                                            }
                                                            
                                                            
                                                            mtt::Rep* repre = mtt::rep(thing);
                                                            
                                                            
                                                            Mat4 M = m::rotate(Mat4(1.0f), event.rotation, vec3(0.0f, 0.0f, 1.0f));
                                                            
                                                            set_pose_transform(thing, M * repre->pose_transform);
                                                            
                                                            for (usize i = 0; i < repre->colliders.size(); i += 1) {
                                                                mtt::Collider_remove(repre->colliders[i]->system, 0, repre->colliders[i]);
                                                                repre->colliders[i]->transform = repre->pose_transform;
                                                                mtt::push_AABBs(repre->colliders[i]->system, repre->colliders[i], 1, 0);
                                                            }
                                                        }
                                                    } else {
                                                        auto& it = *world->dt.scn_ctx.selected_things.begin();
                                                        
                                                        mtt::Thing* thing = nullptr;
                                                        if (!world->mtt_world.Thing_try_get(it, &thing)) {
                                                            break;
                                                        }
                                                        
                                                        if (thing->lock_to_canvas) {
                                                            break;
                                                        }
                                                        
                                                        
                                                        mtt::Rep* repre = mtt::rep(thing);
                                                        
                                                        
                                                        Mat4 M = m::rotate(Mat4(1.0f), event.rotation, vec3(0.0f, 0.0f, 1.0f));
                                                        
                                                        set_pose_transform(thing, M * repre->pose_transform);
                                                        
                                                        for (usize i = 0; i < repre->colliders.size(); i += 1) {
                                                            mtt::Collider_remove(repre->colliders[i]->system, 0, repre->colliders[i]);
                                                            repre->colliders[i]->transform = repre->pose_transform;
                                                            mtt::push_AABBs(repre->colliders[i]->system, repre->colliders[i], 1, 0);
                                                        }
                                                    }
                                                    
                                                    break;
                                                }
                                                
                                                Mat4 M = m::translate(Mat4(1.0f), vec3(event.position, 0.0f)) * m::rotate(Mat4(1.0f), -event.rotation, vec3(0.0f, 0.0f, 1.0f)) *
                                                m::translate(Mat4(1.0f), vec3(-event.position, 0.0f));
                                                

                                                if (mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_ROTATION)) {
                                                    cam->cam_transform = cam->cam_transform * M;
                                                    mtt::calc_view_matrix(cam);
                                                }
                                                //
                                                break;
                                            }
                                            case UI_TOUCH_STATE_ROTATION_GESTURE_CANCELLED: {
                                                // fallthrough
                                            }
                                            case UI_TOUCH_STATE_ROTATION_GESTURE_ENDED: {
                                                UI_Touch* touch_input = nullptr;
                                                auto* cam = (mtt::Camera*)Input_get_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_ROTATION, &event, &touch_input);
                                                if (cam == nullptr) {
                                                    cam_info_choose(world, event.position, &cam);
                                                    Input_set_user_data_for_touch_gesture_event(u_input, UI_GESTURE_TYPE_ROTATION, &event, &touch_input, cam);
                                                }
                                                if (thing_arrows.is_active) {
                                                    break;
                                                }
                                                
                                                if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    if (!mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_ROTATION)) {
                                                        break;
                                                    }
                                                    
                                                    if (world->dt.recorder.selection_recording.selections.size() > 1) {
                                                        auto it = world->dt.recorder.selection_recording.selections.end() - 1;
                                                        for (;it != world->dt.recorder.selection_recording.selections.begin(); --it) {
                                                            
                                                            mtt::Thing* thing = nullptr;
                                                            if (!world->mtt_world.Thing_try_get(it->ID, &thing)) {
                                                                continue;
                                                            }
                                                            
                                                            if (thing->lock_to_canvas) {
                                                                continue;
                                                            }
                                                            
                                                            
                                                            mtt::Rep* repre = mtt::rep(thing);
                                                            
                                                            
                                                            Mat4 M = m::rotate(Mat4(1.0f), event.rotation, vec3(0.0f, 0.0f, 1.0f));
                                                            
                                                            set_pose_transform(thing, M * repre->pose_transform);
                                                            
                                                            for (usize i = 0; i < repre->colliders.size(); i += 1) {
                                                                mtt::Collider_remove(repre->colliders[i]->system, 0, repre->colliders[i]);
                                                                repre->colliders[i]->transform = repre->pose_transform;
                                                                mtt::push_AABBs(repre->colliders[i]->system, repre->colliders[i], 1, 0);
                                                            }
                                                        }
                                                    } else {
                                                        auto& it = *world->dt.scn_ctx.selected_things.begin();
                                                        
                                                        mtt::Thing* thing = nullptr;
                                                        if (!world->mtt_world.Thing_try_get(it, &thing)) {
                                                            break;
                                                        }
                                                        
                                                        if (thing->lock_to_canvas) {
                                                            break;
                                                        }
                                                        
                                                        
                                                        mtt::Rep* repre = mtt::rep(thing);
                                                        
                                                        
                                                        Mat4 M = m::rotate(Mat4(1.0f), event.rotation, vec3(0.0f, 0.0f, 1.0f));
                                                        
                                                        set_pose_transform(thing, M * repre->pose_transform);
                                                        
                                                        for (usize i = 0; i < repre->colliders.size(); i += 1) {
                                                            mtt::Collider_remove(repre->colliders[i]->system, 0, repre->colliders[i]);
                                                            repre->colliders[i]->transform = repre->pose_transform;
                                                            mtt::push_AABBs(repre->colliders[i]->system, repre->colliders[i], 1, 0);
                                                        }
                                                    }
                                                    
                                                    break;
                                                }
                                                
                                                Mat4 M = m::translate(Mat4(1.0f), vec3(event.position, 0.0f)) *           m::rotate(Mat4(1.0f), -event.rotation, vec3(0.0f, 0.0f, 1.0f)) *
                                                m::translate(Mat4(1.0f), vec3(-event.position, 0.0f));
                                                
                                                
                                                if (mtt::camera_flags_check(cam, mtt::CAMERA_FLAGS_ENABLE_ROTATION)) {
                                                    cam->cam_transform = cam->cam_transform * M;
                                                    mtt::calc_view_matrix(cam);
                                                }
                                                //
                                                break;
                                            }
                                            case UI_TOUCH_STATE_LONG_PRESS_GESTURE_BEGAN: {
                                                if (world->dt.scn_ctx.selected_things.size() > 0) {
                                                    break;
                                                }
                                                
                                                mtt::Camera* cam = nullptr;
                                                cam_info_choose(world, event.position, &cam);
                                                if (cam != &world->cam) {
                                                    break;
                                                }
                                                
                                                world->dt.scn_ctx.long_press_this_frame = true;
                                                world->dt.scn_ctx.long_press_position = event.position;
                                                //                                Speech_discard(&core->speech_system.info_list[0]);
                                                //                                dt::text_view_clear(dt::DrawTalk::ctx());
                                                //                                dt::Selection_Recorder_clear_selections(&world->ct.recorder.selection_recording);
                                                break;
                                            }
                                            case UI_TOUCH_STATE_LONG_PRESS_GESTURE_CHANGED: {
                                                break;
                                            }
                                            case UI_TOUCH_STATE_LONG_PRESS_GESTURE_CANCELLED: {
                                                break;
                                            }
                                            case UI_TOUCH_STATE_LONG_PRESS_GESTURE_ENDED: {
                                                break;
                                            }
#define MAX_SIDE_MENU_WIDTH (core->viewport.width / 4.0)
#define SIDE_MENU_ALPHA (64)
#define BORDER_SWIPE_TESTING (1)
                                            case UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_BEGAN: {
#if BORDER_SWIPE_TESTING
                                                MTT_print("%s", "border swipe from left\n");
                                                auto* vg = nvgGetGlobalContext();
                                                nvgBeginPath(vg);
                                                nvgFillColor(vg, nvgRGBA(255,255,255, SIDE_MENU_ALPHA));
                                                auto& loc = event.position;
                                                auto width = m::min((i64)loc.x, (i64)MAX_SIDE_MENU_WIDTH);
                                                nvgRect(vg, 0, 0, width, core->viewport.height);
                                                nvgFill(vg);
#endif
                                                
                                                break;
                                            }
                                            case UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_CHANGED: {
#if BORDER_SWIPE_TESTING
                                                auto* vg = nvgGetGlobalContext();
                                                nvgBeginPath(vg);
                                                nvgFillColor(vg, nvgRGBA(255,255,255, SIDE_MENU_ALPHA));
                                                auto& loc = event.position;
                                                auto width = m::min((i64)loc.x, (i64)MAX_SIDE_MENU_WIDTH);
                                                nvgRect(vg, 0, 0, width, core->viewport.height);
                                                nvgFill(vg);
#endif
                                                break;
                                            }
                                            case UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_CANCELLED: {
#if BORDER_SWIPE_TESTING
                                                auto* vg = nvgGetGlobalContext();
                                                nvgBeginPath(vg);
                                                nvgFillColor(vg, nvgRGBA(255,255,255, SIDE_MENU_ALPHA));
                                                auto& loc = event.position;
                                                auto width = m::min((i64)loc.x, (i64)MAX_SIDE_MENU_WIDTH);
                                                nvgRect(vg, 0, 0, width, core->viewport.height);
                                                nvgFill(vg);
#endif
                                                break;
                                            }
                                            case UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_ENDED: {
#if BORDER_SWIPE_TESTING
                                                auto* vg = nvgGetGlobalContext();
                                                nvgBeginPath(vg);
                                                nvgFillColor(vg, nvgRGBA(255,255,255, SIDE_MENU_ALPHA));
                                                auto& loc = event.position;
                                                auto width = m::min((i64)loc.x, (i64)MAX_SIDE_MENU_WIDTH);
                                                nvgRect(vg, 0, 0, width, core->viewport.height);
                                                nvgFill(vg);
#endif
                                                break;
                                            }
                                            case UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_BEGAN: {
#if BORDER_SWIPE_TESTING
                                                MTT_print("%s", "border swipe from right\n");
                                                auto* vg = nvgGetGlobalContext();
                                                nvgBeginPath(vg);
                                                nvgFillColor(vg, nvgRGBA(255,255,255, SIDE_MENU_ALPHA));
                                                
                                                auto& loc = event.position;
                                                nvgRect(vg , m::max((i64)loc.x, (i64)core->viewport.width - (i64)MAX_SIDE_MENU_WIDTH), 0, core->viewport.width - loc.x, core->viewport.height);
                                                nvgFill(vg);
#endif
                                                
                                                break;
                                            }
                                            case UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_CHANGED: {
#if BORDER_SWIPE_TESTING
                                                auto* vg = nvgGetGlobalContext();
                                                nvgBeginPath(vg);
                                                nvgFillColor(vg, nvgRGBA(255,255,255, SIDE_MENU_ALPHA));
                                                auto& loc = event.position;
                                                nvgRect(vg , m::max((i64)loc.x, (i64)core->viewport.width - (i64)MAX_SIDE_MENU_WIDTH), 0, core->viewport.width - loc.x, core->viewport.height);
                                                nvgFill(vg);
#endif
                                                break;
                                            }
                                            case UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_CANCELLED: {
#if BORDER_SWIPE_TESTING
                                                auto* vg = nvgGetGlobalContext();
                                                nvgBeginPath(vg);
                                                nvgFillColor(vg, nvgRGBA(255,255,255, SIDE_MENU_ALPHA));
                                                auto& loc = event.position;
                                                nvgRect(vg , m::max((i64)loc.x, (i64)core->viewport.width - (i64)MAX_SIDE_MENU_WIDTH), 0, core->viewport.width - loc.x, core->viewport.height);
                                                nvgFill(vg);
#endif
                                                break;
                                            }
                                            case UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_ENDED: {
#if BORDER_SWIPE_TESTING
                                                auto* vg = nvgGetGlobalContext();
                                                nvgBeginPath(vg);
                                                nvgFillColor(vg, nvgRGBA(255,255,255, SIDE_MENU_ALPHA));
                                                auto& loc = event.position;
                                                nvgRect(vg , m::max((i64)loc.x, (i64)core->viewport.width - (i64)MAX_SIDE_MENU_WIDTH), 0, core->viewport.width - loc.x, core->viewport.height);
                                                nvgFill(vg);
#endif
                                                break;
                                            }
#undef MAX_SIDE_MENU_WIDTH
#undef SIDE_MENU_ALPHA
                                            case UI_TOUCH_STATE_DOUBLE_TAP_NONPREFERRED: {
                                                //                                MTT_print("double tap\n");
                                                //                                if (!Speech_split(&core->speech_system.info_list[0])) {
                                                ////                                    dt::Selection_Recorder_clear_selections(&world->ct.recorder.selection_recording);
                                                //                                }
                                                break;
                                            }
                                            case UI_TOUCH_STATE_SWIPE_BEGAN: {
                                                MTT_FALLTHROUGH;
                                            }
                                            case UI_TOUCH_STATE_SWIPE_CHANGED: {
                                                MTT_FALLTHROUGH;
                                            }
                                            case UI_TOUCH_STATE_SWIPE_CANCELLED: {
                                                MTT_FALLTHROUGH;
                                            }
                                            case UI_TOUCH_STATE_SWIPE_ENDED: {
                                                break;
                                            }
                                            default: { break; }
                                        }
                                        break;
                                    }
                                        
                                        
                                    case dt::SP_STYLUS_MODE_POINT_AT: {
                                        //
                                        break;
                                    }
                                    case dt::SP_STYLUS_MODE_SKETCH_RECOGNITION: {
                                        //
                                        break;
                                    }
                                        
                                        break;
                                }
                            case dt::SP_TOUCH_MODE_SELECTION: {
                                break;
                            }
                            case dt::SP_TOUCH_MODE_THING_MANIPULATION: {
                                break;
                            }
                            case dt::SP_TOUCH_MODE_GESTURE: {
                                break;
                            }
                            default: { break; }
                                break;
                            }
                        }
                    }
                    case UI_TOUCH_TYPE_KEY : {
                        bool do_update_ui_cmd_text = true; //(dt::get_confirm_phase() == dt::CONFIRM_PHASE_SPEECH);
                        //&& !(dt::DrawTalk::ctx()->ui).margin_panels[0].text.is_overriding_speech();
                        //                    if (!do_update_ui_cmd_text) {
                        //                        core->set_command_line_text(core, "", 0, 0);
                        //                    }
                        switch (event.state_type) {
                            case UI_TOUCH_STATE_KEY_REPEATED:
                                MTT_FALLTHROUGH;
                            case UI_TOUCH_STATE_KEY_BEGAN: {
                                UI_KEY_TYPE k = (UI_KEY_TYPE)event.key;
                                if (k >= UI_KEY_TYPE_FIRST_VISUAL && k <= UI_KEY_TYPE_LAST_VISUAL) {
                                    if (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_ALTERNATE))) {
                                        if (k == UI_KEY_TYPE_1) {
                                            pointer_op_set_current(u_input, UI_POINTER_OP_DRAW);
                                            break;
                                        } else if (k == UI_KEY_TYPE_2) {
                                            pointer_op_set_current(u_input, UI_POINTER_OP_ERASE);
                                            break;
                                        } else if (k == UI_KEY_TYPE_3) {
                                            pointer_op_set_current(u_input, UI_POINTER_OP_ARROW_CREATE);
                                            break;
                                        }
                                    }
                                    
                                    if (k == UI_KEY_TYPE_A) {
                                        if (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND))) {
                                            
                                            text_in_selected_all = true;
                                            MTT_print("Command: ]_[%s]_[\n", text_in_buf.c_str());
                                            
                                            if (do_update_ui_cmd_text) {
                                                core->set_command_line_text(core, text_in_buf.c_str(), text_in_buf.size(), SELECT_ALL_MODE_ON);
                                            }
                                            break;
                                        }
                                        
                                    } else if (k == UI_KEY_TYPE_C) {
                                        if (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND))) {
                                            
                                            text_in_selected_all = false;
                                            
                                            // copy from application and paste in general clipboard
                                            
                                            if (text_in_cursor_idx == text_in_buf.size()) {
                                                Clipboard_paste(text_in_buf);
                                            } else {
                                                Clipboard_paste(text_in_buf.substr(0, text_in_cursor_idx));
                                            }
                                            break;
                                        }
                                    } else if (k == UI_KEY_TYPE_V) {
                                        if (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND))) {
                                            
                                            // copy from clipboard and paste in application
                                            text_in_selected_all = false;
                                            mtt::String from_clipboard = Clipboard_copy();
                                            if (from_clipboard.size() > 0) {
                                                if (text_in_cursor_idx == text_in_buf.size()) {
                                                    text_in_buf += from_clipboard;
                                                    text_in_cursor_idx += from_clipboard.size();
                                                } else {
                                                    text_in_buf = text_in_buf.substr(0, text_in_cursor_idx) + from_clipboard + text_in_buf.substr(text_in_cursor_idx);
                                                    text_in_cursor_idx += from_clipboard.size();
                                                }
                                                {
                                                    mtt::String out = text_in_buf.substr(0, text_in_cursor_idx) + "]_[" + ((text_in_cursor_idx != text_in_buf.size()) ? text_in_buf.substr(text_in_cursor_idx) : "");
                                                    if (do_update_ui_cmd_text) {
                                                        core->set_command_line_text(core, out.c_str(), out.size(), 0);
                                                    }
                                                }
                                            }
                                            break;
                                        }
                                    } else if (k == UI_KEY_TYPE_H) {
                                        //                                if ( MTT_CHECK_MODIFIER_KEY(event.key_sub, UI_KEY_MODIFIER_FLAG_CONTROL) ||
                                        //                                    MTT_CHECK_MODIFIER_KEY(event.key_sub, UI_KEY_MODIFIER_FLAG_COMMAND) ) {
                                        //
                                        //                                    core->set_command_line_text(core, "", 0, HIDE_MODE_TOGGLE);
                                        //                                    break;
                                        //                                }
                                        
                                    }
                                    
                                    char c =  ui_key_type_to_char[k][
                                        ((((UI_KEY_MODIFIER_FLAG)event.key_sub) & UI_KEY_MODIFIER_FLAG_SHIFT) != 0) ? 1 : 0];
                                    if (text_in_cursor_idx == text_in_buf.size()) {
                                        text_in_cursor_idx += 1;
                                        
                                        text_in_buf += c;
                                        
                                    } else {
                                        text_in_buf = text_in_buf.substr(0, text_in_cursor_idx) + c + text_in_buf.substr(text_in_cursor_idx);
                                        text_in_cursor_idx += 1;
                                    }
                                    
                                    {
                                        auto* ui = &dt::DrawTalk::ctx()->ui;
                                        ui->margin_panels[0].text.send_text_input(text_in_buf, false);
                                    }
                                    
                                } else if (k == UI_KEY_TYPE_SPACEBAR) {
                                    if (text_in_cursor_idx == text_in_buf.size()) {
                                        text_in_cursor_idx += 1;
                                        
                                        text_in_buf += ' ';
                                        
                                    } else {
                                        text_in_buf = text_in_buf.substr(0, text_in_cursor_idx) + ' ' + text_in_buf.substr(text_in_cursor_idx);
                                        text_in_cursor_idx += 1;
                                    }
                                    
                                    {
                                        auto* ui = &dt::DrawTalk::ctx()->ui;
                                        ui->margin_panels[0].text.send_text_input(text_in_buf, false);
                                    }
                                } else if (k == UI_KEY_TYPE_TAB) {
                                    if (text_in_cursor_idx == text_in_buf.size()) {
                                        text_in_buf += "    ";
                                        text_in_cursor_idx += 4;
                                        
                                    } else {
                                        text_in_buf = text_in_buf.substr(0, text_in_cursor_idx) + "    " + text_in_buf.substr(text_in_cursor_idx);
                                        text_in_cursor_idx += 4;
                                    }
                                    
                                    {
                                        auto* ui = &dt::DrawTalk::ctx()->ui;
                                        ui->margin_panels[0].text.send_text_input(text_in_buf, false);
                                    }
                                    
                                } else if (k == UI_KEY_TYPE_DELETE_OR_BACKSPACE) {
                                    
                                    if ((((UI_KEY_MODIFIER_FLAG)event.key_sub) & UI_KEY_MODIFIER_FLAG_SHIFT) == 0) {
                                        if (!text_in_buf.empty() && text_in_cursor_idx != 0) {
                                            if (text_in_buf.size() == 1) {
                                                text_in_cursor_idx = 0;
                                                text_in_buf.clear();
                                            } else {
                                                text_in_buf = text_in_buf.substr(0, text_in_cursor_idx - 1) + ((text_in_cursor_idx != text_in_buf.size()) ? text_in_buf.substr(text_in_cursor_idx) : "");
                                                text_in_cursor_idx -= 1;
                                            }
                                            
                                        }
                                        
                                        
                                        auto* ui = &dt::DrawTalk::ctx()->ui;
                                        ui->margin_panels[0].text.send_text_input(text_in_buf, false);
                                    } else {
                                        auto& dock = dt::DrawTalk::ctx()->ui.dock;
                                        mtt::Thing* ui_button = world->mtt_world.Thing_get(dock.buttons[dock.label_to_index.find("discard")->second]->thing);
                                        ui_button->input_handlers.on_touch_input_began(&ui_button->input_handlers, ui_button, {}, {}, nullptr, nullptr, 0);
                                    }
                                    
                                } else if (k == UI_KEY_TYPE_RETURN_OR_ENTER) {
                                
                                    if (contains_some_of_flags(event.key_sub, UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND)) {
                                        if (!contains_flags(&event, UI_EVENT_FLAG_OVERRIDE)) {
                                            auto* ui = &dt::DrawTalk::ctx()->ui;
                                            //ui->margin_panels[0].text.send_text_input(text_in_buf, true);
                                            ui->margin_panels[0].text.send_text_input_confirm_command();
                                            if (dt::get_confirm_phase() == dt::CONFIRM_PHASE_ANIMATE) {
                                                
                                            }
                                        }
                                        
                                        text_in_buf += '\n';
                                        text_in_cursor_idx += 1;
                                        
                                        MTT_print("Command entered: [%s]\n", text_in_buf.c_str());
                                        
                                        text_in_selected_all = false;
                                        
                                        text_in_history.move_to_present();
                                        text_in_buf.clear();
                                        text_in_cursor_idx = 0;
                                        
                                    } else if ((((UI_KEY_MODIFIER_FLAG)event.key_sub) & UI_KEY_MODIFIER_FLAG_SHIFT) == 0) {
                                        text_in_selected_all = false;
                                        if (text_in_cursor_idx == text_in_buf.size()) {
                                            text_in_cursor_idx += 1;
                                            
                                            text_in_buf += '\n';
                                            
                                        } else {
                                            text_in_buf = text_in_buf.substr(0, text_in_cursor_idx) + '\n' + text_in_buf.substr(text_in_cursor_idx);
                                            text_in_cursor_idx += 1;
                                        }
                                    } else {
                                        
                                        
                                        text_in_buf += '\n';
                                        text_in_cursor_idx += 1;
                                        
                                        
                                        
                                        bool command_recognized = false;
                                        
                                        MTT_print("Command entered: [%s]\n", text_in_buf.c_str());
                                        
                                        
                                        // query
                                        if (text_in_buf.size() > 2 && text_in_buf.compare(0, 2, "q ") == 0) {
                                            text_in_buf.pop_back();
                                            
                                            MTT_print("Query: [%s]\n", text_in_buf.substr(2).c_str());
                                            
                                            dt::query_related_info_for_unknown_word(dt::DrawTalk::ctx(), text_in_buf.substr(2), [](mtt::String& data, void* doc_ptr, usize ID) {
                                                
                                                ArduinoJson::DynamicJsonDocument& doc = *static_cast<ArduinoJson::DynamicJsonDocument*>(doc_ptr);
                                                
                                                mtt::String debug_print = "";
                                                serializeJsonPretty(doc, debug_print);
                                                std::cout << debug_print << std::endl;
                                                
                                                MTT_print("ID was: %llu\n", ID);
                                            });
                                            
                                            command_recognized = true;
                                            
                                            
                                            // thing print
                                        } else if (text_in_buf.size() > 2 && text_in_buf.compare(0, 2, "r ") == 0) {
                                            mtt::String sub = text_in_buf.substr(2);
                                            cstring rule_str = sub.c_str();
                                            std::vector<dt::Rule_Test> rule = { dt::Rule_Test(mtt_world->ecs_world, rule_str, rule_str)
                                            };
                                            rule[0].init();
                                            dt::print_rule_results(mtt_world, rule);
                                            command_recognized = true;
                                        } else if (text_in_buf.size() > 2 && text_in_buf.compare(0, 2+3, "rcon ") == 0) {
#ifndef NDEBUG
                                            mtt::String sub = text_in_buf.substr(2 + 3);
                                            cstring rule_str = sub.c_str();
                                            std::vector<dt::Rule_Test> rule = {
                                            };
                                            auto test = dt::Rule_Test(mtt_world->ecs_world, rule_str, rule_str);
                                            test.init();
                                            if (test.is_valid) {
                                                continuous_rule_tests.push_back(test);
                                            }
                                            continuous_rule_tests_enabled = (!continuous_rule_tests.empty());
#endif
                                            command_recognized = true;
                                        } else if (text_in_buf.size() > 2 && text_in_buf.compare(0, 2+4, "rcoff\n") == 0) {
#ifndef NDEBUG
                                            mtt::String sub = text_in_buf.substr(2 + 4);
                                            for (usize _ = 0; _ < continuous_rule_tests.size(); _ += 1) {
                                                continuous_rule_tests[_].deinit();
                                            }
                                            continuous_rule_tests.clear();
                                            
                                            continuous_rule_tests_enabled = false;
#endif
                                            command_recognized = true;
                                        }
                                        else if (text_in_buf == "tp\n" || (text_in_buf.size() > 3 && text_in_buf.compare(0, 3, "tp ") == 0)) {
                                            if (text_in_buf.compare("tp -a -v\n") == 0) {
                                                for (auto it = mtt::iterator(mtt_world); mtt::next(&it);) {
                                                    
                                                    Thing* thing = it.value;
                                                    mtt::Thing_print_verbose(thing);
                                                }
                                                command_recognized = true;
                                            } else if (text_in_buf.compare("tp -a\n")) {
                                                for (auto it = mtt::iterator(mtt_world); mtt::next(&it);) {
                                                    
                                                    Thing* thing = it.value;
                                                    mtt::Thing_print(thing);
                                                }
                                                command_recognized = true;
                                            } else if (text_in_buf.compare("tp ids\n") == 0) {
                                                MTT_print("%s", "{\n");
                                                for (auto it = mtt::iterator(mtt_world); mtt::next(&it);) {
                                                    
                                                    Thing* thing = it.value;
                                                    MTT_print("    %llu,\n", thing->id);
                                                }
                                                MTT_print("%s", "}\n");
                                                command_recognized = true;
                                            } else {
                                                bool found = false;
                                                isize idx = text_in_buf.find_first_of(" ");
                                                if (idx >= 0 && idx + 1 != text_in_buf.size()) {
                                                    mtt::Thing* to_find = nullptr;
                                                    std::size_t err_loc = text_in_buf.size();
                                                    usize try_get = stoi(text_in_buf.substr(idx + 1), &err_loc);
                                                    
                                                    std::cout << text_in_buf.substr(idx + 1) << " : " << err_loc << " : " << "[" << try_get << "]" << std::endl;
                                                    if (err_loc > 0 && mtt_world->Thing_try_get(try_get, &to_find)) {
                                                        mtt::Thing_print_verbose(to_find);
                                                        found = true;
                                                    }
                                                }
                                                if (!found) {
                                                    MTT_print("%s", "Thing not found!\n");
                                                }
                                                command_recognized = true;
                                            }
                                            
                                        }
                                        // tp destroy
                                        else if (text_in_buf.compare("tx -a\n") == 0) {
                                            std::vector<mtt::Thing*> to_destroy;
                                            for (auto it = mtt::iterator(mtt_world); mtt::next(&it);) {
                                                
                                                Thing* thing = it.value;
                                                //mtt::Thing_destroy(thing);
                                                to_destroy.push_back(thing);
                                            }
                                            for (usize t = 0; t < to_destroy.size(); t += 1) {
                                                mtt::Thing_destroy(to_destroy[t]);
                                            }
                                            command_recognized = true;
                                        }
                                        else if (text_in_buf.size() > 3 && text_in_buf.compare(0, 3, "tx ") == 0) {
                                            bool found = false;
                                            isize idx = text_in_buf.find_first_of(" ");
                                            if (idx >= 0 && idx + 1 != text_in_buf.size()) {
                                                mtt::Thing* to_find = nullptr;
                                                std::size_t err_loc = text_in_buf.size();
                                                try {
                                                    usize try_get = stoi(text_in_buf.substr(idx + 1), &err_loc);
                                                    
                                                    std::cout << text_in_buf.substr(idx + 1) << " : " << err_loc << " : " << "[" << try_get << "]" << std::endl;
                                                    if (err_loc > 0 && mtt_world->Thing_try_get(try_get, &to_find)) {
                                                        mtt::Thing_destroy(to_find);
                                                        found = true;
                                                    }
                                                } catch (std::exception& ex) {
                                                    MTT_print("%s\n", ex.what());
                                                }
                                            }
                                            if (!found) {
                                                MTT_print("%s", "Thing not found!\n");
                                            }
                                            command_recognized = true;
                                        } else if (text_in_buf.compare("ar -f\n") == 0) {
                                            command_recognized = true;
                                            Augmented_Reality_config(&core->ar_ctx, AUGMENTED_REALITY_FLAG_FACE_TRACKING);
                                            Augmented_Reality_run(&core->ar_ctx);
                                        } else if (text_in_buf.compare("ar -w\n") == 0) {
                                            command_recognized = true;
                                            Augmented_Reality_config(&core->ar_ctx, AUGMENTED_REALITY_FLAG_WORLD_TRACKING);
                                            Augmented_Reality_run(&core->ar_ctx);
                                        } else if (text_in_buf.compare("ar -p\n") == 0) {
                                            command_recognized = true;
                                            Augmented_Reality_pause(&core->ar_ctx);
                                        } else if (text_in_buf.compare("ar -r\n") == 0) {
                                            command_recognized = true;
                                            Augmented_Reality_run(&core->ar_ctx);
                                            // toggle freeze-frame mode
                                        } else if (text_in_buf.compare("ar -freeze\n") == 0) {
                                            command_recognized = true;
                                            core->ar_ctx.freeze_frame_when_off = true;
                                        } else if (text_in_buf.compare("ar -nfreeze\n") == 0) {
                                            command_recognized = true;
                                            core->ar_ctx.freeze_frame_when_off = false;
                                        } else if (text_in_buf.compare("sp -on\n") == 0) {
                                            command_recognized = true;
                                            Speech_set_active_state(&core->speech_system.info_list[0], true);
                                        } else if (text_in_buf.compare("sp -off\n") == 0) {
                                            command_recognized = true;
                                            Speech_discard(&core->speech_system.info_list[0]);
                                            Speech_set_active_state(&core->speech_system.info_list[0], false);
                                            dt::text_view_clear(dt::DrawTalk::ctx());
                                        } else if (text_in_buf.compare("cmd confirm") == 0) {
                                            command_recognized = true;
                                            //                                } else if (text_in_buf.compare("v -on\n") == 0) {
                                            //                                    world_vis.is_on = true;
                                            //                                    world_vis.has_started = true;
                                            //                                    command_recognized = true;
                                            //                                } else if (text_in_buf.compare("v -off\n") == 0) {
                                            //                                    world_vis.is_on = false;
                                            //                                    world_vis.has_started = false;
                                            //                                    command_recognized = true;
                                        } else if (text_in_buf.compare("t -rot\n") == 0) {
#if 0
                                            ROTATE_TEST_ON = !ROTATE_TEST_ON;
#endif
                                            command_recognized = true;
                                            
                                        } else if (text_in_buf_is_equal("t -lat\n")) {
#if 0
                                            LOOK_AT_TEST_ON = !LOOK_AT_TEST_ON;
#endif
                                            command_recognized = true;
                                        } else if (text_in_buf.compare(0, 3, "in ") == 0) {
                                            if (text_in_buf.size() > 4) {
                                                mtt::String in = text_in_buf.substr(3, text_in_buf.size() - 4);
                                                {
                                                    in = mtt::regex_replace_new_str(in, "\n", " ");
                                                    in = mtt::regex_replace_new_str(in, "\r", " ");
                                                }
                                                Speech_process_text_as_input(&core->speech_system.info_list[0], in, 0);
                                            }
                                            command_recognized = true;
                                            // reload scripts
                                        }
                                        else if (text_in_buf.compare(0, 7, "p-draw\n") == 0) {
                                            pointer_op_set_current(u_input, UI_POINTER_OP_DRAW);
                                            command_recognized = true;
                                            MTT_print("%s", "[pen draw tool]\n");
                                        }
                                        else if (text_in_buf.compare(0, 8, "p-erase\n") == 0) {
                                            pointer_op_set_current(u_input, UI_POINTER_OP_ERASE);
                                            command_recognized = true;
                                            MTT_print("%s", "[pen erase tool]\n");
                                        }
                                        else if (text_in_buf.compare("rl\n") == 0) {
                                            
                                        } else if (text_in_buf.compare("stbg -on\n") == 0) {
                                            MTT_follow_system_window(true);
                                            command_recognized = true;
                                        } else if (text_in_buf.compare("stbg -off\n") == 0) {
                                            MTT_follow_system_window(false);
                                            command_recognized = true;
                                        } else if (text_in_buf.compare("reset defaults\n") == 0) {
                                            MTT_reset_defaults();
                                            command_recognized = true;
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
                                        } else if (text_in_buf.compare(0, 9, "video -on") == 0) {
                                            
                                            command_recognized = true;
                                            enable_video = true;
                                        } else if (text_in_buf.compare(0, 10, "video -off") == 0) {
                                            command_recognized = true;
                                            enable_video = false;
                                            mtt::Video_Recording_stop(core->screen_recording);
#endif
                                        } else if (text_in_buf.compare(0, 6, "texst ") == 0) {
                                            command_recognized = true;
                                            auto sub = text_in_buf.substr(6);
                                            while (sub.back() == '\n' || sub.back() == ' ') {
                                                sub.pop_back();
                                            }
                                            
                                            auto& dt_ctx = *dt::DrawTalk::ctx();
                                            auto* mtt_world = dt_ctx.mtt;
                                            Thing* thing = nullptr;
                                            sd::Image* image;
                                            auto* images = sd::images(core->renderer);
                                            if (sd::Images_lookup(images, sub.c_str(), &image)) {
                                                for (auto& selection : dt_ctx.recorder.selection_recording.selections) {
                                                    auto thing_id = selection.ID;
                                                    thing = mtt::Thing_try_get(mtt_world, thing_id);
                                                    
                                                    if (thing == nullptr) {
                                                        continue;
                                                    }
                                                    
                                                    mtt::Rep* rep = mtt::rep(thing);
                                                    mtt::set_pose_transform(thing, m::scale(Mat4(1.0f), rep->pose_transform_values.scale * vec3(image->size.x / image->size.y, 1.0, 1.0f)));
                                                    for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
                                                        (*it)->set_texture_ID(image->texture_ID);
                                                    }
                                                }
                                            }
                                            
                                        } else if (text_in_buf.compare(0, 6, "texft ") == 0) {
                                            command_recognized = true;
                                            auto sub = text_in_buf.substr(6);
                                            while (sub.back() == '\n' || sub.back() == ' ') {
                                                sub.pop_back();
                                            }
                                            
                                            auto& dt_ctx = *dt::DrawTalk::ctx();
                                            auto* mtt_world = dt_ctx.mtt;
                                            Thing* thing = nullptr;
                                            sd::Image* image;
                                            auto* images = sd::images(core->renderer);
                                            if (sd::Images_lookup(images, sub.c_str(), &image)) {
                                                for (auto& selection : dt_ctx.recorder.selection_recording.selections) {
                                                    auto thing_id = selection.ID;
                                                    thing = mtt::Thing_try_get(mtt_world, thing_id);
                                                    
                                                    if (thing == nullptr) {
                                                        continue;
                                                    }
                                                    
                                                    mtt::Rep* rep = mtt::rep(thing);
                                                    mtt::set_pose_transform(thing, m::scale(Mat4(1.0f), vec3(image->size.x, image->size.y, 1.0)));
                                                    for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
                                                        (*it)->set_texture_ID(image->texture_ID);
                                                    }
                                                }
                                            }
                                            
                                        } else if (text_in_buf.compare(0, 6, "texbg ") == 0) {
                                            command_recognized = true;
                                            auto sub = text_in_buf.substr(6);
                                            while (sub.back() == '\n' || sub.back() == ' ') {
                                                sub.pop_back();
                                            }
                                            
                                            sd::Image* image;
                                            auto* images = sd::images(core->renderer);
                                            if (sd::Images_lookup(images, sub.c_str(), &image)) {
                                                world->render_info.bg_texture_id = image->texture_ID;
                                                world->render_info.bg_texture_dimensions = vec2(image->size.x, image->size.y);
                                            } else {
                                                world->render_info.bg_texture_id = sd::Texture_ID_INVALID;
                                            }
                                            
                                            // texture load remote
                                        } else if (text_in_buf.compare(0, 6, "texld ") == 0) {
                                            auto sub = text_in_buf.substr(6);
                                            auto fidx = sub.find_first_of(" ");
                                            if (fidx != std::string::npos) {
                                                mtt::String src_arg = sub.substr(0, fidx);
                                                while (sub.back() == '\n' || sub.back() == ' ') {
                                                    sub.pop_back();
                                                }
                                                mtt::String dst_arg = sub.substr(fidx + 1);
                                                sd::Image_Load_Descriptor desc;
                                                desc.paths.push_back((sd::Path_Info){src_arg, "", dst_arg});
                                                desc.custom_data = (void*)&world->mtt_world;
                                                
                                                
                                                sd::images_load_remote_async(renderer, &desc, [](const sd::Image_Load_Result& result, bool status) -> bool {
                                                    if (status == false) {
                                                        return false;
                                                    } else {
                                                        
                                                        sd::Image* image;
                                                        if (sd::Images_lookup(result.image_storage, result.images[0]->name.c_str(), &image)) {
                                                            
                                                            auto& dt_ctx = *dt::DrawTalk::ctx();
                                                            auto* mtt_world = dt_ctx.mtt;
                                                            
                                                            for (auto& selection : dt_ctx.recorder.selection_recording.selections) {
                                                                auto thing_id = selection.ID;
                                                                mtt::Thing* thing = mtt::Thing_try_get(mtt_world, thing_id);
                                                                if (thing == nullptr) {
                                                                    continue;
                                                                }
                                                                
                                                                mtt::Rep* rep = mtt::rep(thing);
                                                                mtt::set_pose_transform(thing, m::scale(Mat4(1.0f), rep->pose_transform_values.scale * vec3(image->size.x / image->size.y, 1.0, 1.0f)));
                                                                for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
                                                                    (*it)->set_texture_ID(image->texture_ID);
                                                                }
                                                            }
                                                            
                                                        } else {
                                                            
                                                        }
                                                        
                                                        return true;
                                                    }
                                                });
                                            }
                                            
                                            command_recognized = true;
                                        }
                                        // texture load local
                                        else if (text_in_buf.compare(0, 7, "texldl ") == 0) {
                                            auto sub = text_in_buf.substr(7);
                                            auto fidx = sub.find_first_of(" ");
                                            if (fidx != std::string::npos) {
                                                mtt::String src_arg = sub.substr(0, fidx);
                                                while (sub.back() == '\n' || sub.back() == ' ') {
                                                    sub.pop_back();
                                                }
                                                mtt::String dst_arg = sub.substr(fidx + 1);
                                                sd::Image_Load_Descriptor desc;
                                                desc.paths.push_back((sd::Path_Info){src_arg, "", dst_arg});
                                                desc.custom_data = (void*)&world->mtt_world;
                                                
                                                
                                                sd::images_load_async(renderer, &desc, [](const sd::Image_Load_Result& result, bool status) -> bool {
                                                    if (status == false) {
                                                        return false;
                                                    } else {
                                                        
                                                        sd::Image* image;
                                                        if (sd::Images_lookup(result.image_storage, result.images[0]->name.c_str(), &image)) {
                                                            
                                                            auto& dt_ctx = *dt::DrawTalk::ctx();
                                                            auto* mtt_world = dt_ctx.mtt;
                                                            
                                                            for (auto& selection : dt_ctx.recorder.selection_recording.selections) {
                                                                auto thing_id = selection.ID;
                                                                mtt::Thing* thing = mtt::Thing_try_get(mtt_world, thing_id);
                                                                if (thing == nullptr) {
                                                                    continue;
                                                                }
                                                                
                                                                mtt::Rep* rep = mtt::rep(thing);
                                                                mtt::set_pose_transform(thing, m::scale(Mat4(1.0f), rep->pose_transform_values.scale * vec3(image->size.x / image->size.y, 1.0, 1.0f)));
                                                                for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
                                                                    (*it)->set_texture_ID(image->texture_ID);
                                                                }
                                                            }
                                                            
                                                        } else {
                                                            
                                                        }
                                                        
                                                        return true;
                                                    }
                                                });
                                            }
                                            
                                            command_recognized = true;
                                        } else if (text_in_buf.compare(0, 5, "conf\n") == 0) {
                                            init_with_config();
                                            command_recognized = true;
                                        } else if (str_eq(text_in_buf, 0, 13, "color_scheme ")) {
                                            mtt::String src_arg = text_in_buf.substr(13);
                                            while (!src_arg.empty() && (src_arg.back() == '\n' || src_arg.back() == ' ')) {
                                                src_arg.pop_back();
                                            }
                                            
                                            mtt::to_upper(src_arg);
                                            const Color_Scheme_ID* cscheme = nullptr;
                                            mtt::map_try_get(&color_scheme_strings_to_id, src_arg, &cscheme);
                                            if (cscheme) {
                                                set_color_scheme(*cscheme);
                                                {
                                                    sd::set_background_color_rgba_v4(core->renderer, dt::bg_color_default());
                                                    
                                                    
                                                    {
                                                        auto* dt_ctx = dt::DrawTalk::ctx();
                                                        auto* world = dt_ctx->mtt;
                                                        auto& dock = dt_ctx->ui.dock;
                                                        auto* button  = dock.label_to_button["color"];
                                                        auto& label_to_config = dock.label_to_config;
                                                        auto* conf = &label_to_config[button->label];
                                                        mtt::Thing* thing = world->Thing_get(button->thing);
                                                        mtt::Rep* rep = mtt::rep(thing);
                                                        
                                                        switch (mtt::color_scheme()) {
                                                            default: {
                                                                MTT_FALLTHROUGH;
                                                            }
                                                            case mtt::COLOR_SCHEME_LIGHT_MODE: {
                                                                rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_DARK_MODE]->is_enabled = false;
                                                                rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_LIGHT_MODE]->is_enabled = true;
                                                                break;
                                                            }
                                                            case mtt::COLOR_SCHEME_DARK_MODE: {
                                                                rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_DARK_MODE]->is_enabled = true;
                                                                rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_LIGHT_MODE]->is_enabled = false;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            
                                            
                                            
                                            command_recognized = true;
                                        } else if (str_eq(text_in_buf, 0, 6, "pause\n")) {
                                            MTT_Core_toggle_pause(core);
                                            command_recognized = true;
                                        } else if (str_eq(text_in_buf, "s-eval\n")) {
                                            mtt::toggle_should_print_script_eval(&world->mtt_world);
                                            command_recognized = true;
                                        } else if (str_eq(text_in_buf, "log -on\n")) {
                                            debug_mode_log_enable(true);
                                            command_recognized = true;
                                        } else if (str_eq(text_in_buf, "log -off\n")) {
                                            debug_mode_log_enable(false);
                                            command_recognized = true;
                                        }
                                        else if (text_in_buf.compare(0, 5, "dyld\n") == 0) {
#ifndef NDEBUG
                                            send_message_drawtalk_server_quick_command(core, "dyld", "");
                                            command_recognized = true;
#endif
                                        } else if (text_in_buf.compare(0, 6, "dyvar ") == 0 || text_in_buf.compare(0, 5, "dvar ") == 0) {
                                            handle_dyvar(core, text_in_buf);
                                            
                                            command_recognized = true;
                                        }
                                        else if (text_in_buf.compare(0, 6, "dyvcl\n") == 0) {
                                            core->dl_ctx.vars.clear();
                                            command_recognized = true;
                                        } else if (text_in_buf.compare(0, 5, "dycl\n") == 0) {
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
                                            {
                                                core->library("on_unload", &core->dl_ctx);
                                            }
#endif
                                            MTT_Dynamic_Library_close(&core->library);
                                            command_recognized = true;
                                        } else if (text_in_buf.compare(0, 5, "save\n") == 0) {
                                            auto& dt_ctx = *dt::DrawTalk::ctx();
                                            auto* mtt_world = dt_ctx.mtt;
                                            
                                            for (auto& selection : dt_ctx.recorder.selection_recording.selections) {
                                                auto thing_id = selection.ID;
                                                mtt::Thing* thing = mtt::Thing_try_get(mtt_world, thing_id);
                                                if (thing == nullptr) {
                                                    continue;
                                                }
                                                
                                                bool status_ok = mtt::Thing_save_as_preset(thing);
                                                if (!status_ok) {
                                                    MTT_error("%s\n", "could not save thing as preset");
                                                }
                                            }
                                            
                                            command_recognized = true;
                                        } else if (text_in_buf.compare(0, 5, "make ") == 0) {
                                            
                                            mtt::String msg = text_in_buf.substr(5);
                                            trim(msg);
                                            
                                            auto& dt_ctx = *dt::DrawTalk::ctx();
                                            auto* mtt_world = dt_ctx.mtt;
                                            
                                            mtt::Thing* out = mtt::Thing_make_from_preset(mtt_world, msg);
                                            if (out == nullptr) {
                                                MTT_error("%s\n", "could not create thing from preset");
                                            }
                                            
                                            command_recognized = true;
                                        }
                                        
                                        
                                        text_in_selected_all = false;
                                        
                                        if (command_recognized) {
                                            while (!text_in_buf.empty() &&
                                                   (text_in_buf.back() == '\n' ||
                                                    text_in_buf.back() == '\r' ||
                                                    text_in_buf.back() == ' ')) {
                                                text_in_buf.pop_back();
                                            }
                                            if (!text_in_history.is_empty()) {
                                                if (text_in_buf != text_in_history.get_left() && text_in_buf != "") {
                                                    text_in_history.push(text_in_buf);
                                                }
                                            } else {
                                                text_in_history.push(text_in_buf);
                                            }
                                            
                                        } else {
                                            auto* dt_ctx = dt::DrawTalk::ctx();
                                            if (dt::get_confirm_phase() == dt::CONFIRM_PHASE_ANIMATE && !dt_ctx->is_waiting_on_results) {
                                                dt::handle_speech_phase_change(*dt_ctx);
                                            }
                                        }
                                        text_in_history.move_to_present();
                                        text_in_buf.clear();
                                        text_in_cursor_idx = 0;
                                    }
                                    
                                    
                                    
                                } else if (k == UI_KEY_TYPE_LEFT_ARROW) {
                                    if (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND))) {
                                        if (text_in_cursor_idx != 0) {
                                            text_in_cursor_idx = 0;
                                            text_in_selected_all = false;
                                        }
                                        
                                        {
                                            auto* ui = &dt::DrawTalk::ctx()->ui;
                                            ui->margin_panels[0].text.clear_selections();
                                        }
                                        
                                    } else {
                                        if (text_in_cursor_idx != 0) {
                                            text_in_cursor_idx -= 1;
                                            text_in_selected_all = false;
                                        }
                                        
                                        {
                                            auto* ui = &dt::DrawTalk::ctx()->ui;
                                            ui->margin_panels[0].text.move_selections_left();
                                        }
                                    }
                                    
                                    
                                    
                                } else if (k == UI_KEY_TYPE_RIGHT_ARROW) {
                                    if (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND))) {
                                        text_in_cursor_idx = text_in_buf.size();
                                        text_in_selected_all = false;
                                        
                                        {
                                            auto* ui = &dt::DrawTalk::ctx()->ui;
                                            ui->margin_panels[0].text.clear_selections();
                                        }
                                        
                                    } else {
                                        if (text_in_cursor_idx != text_in_buf.size()) {
                                            text_in_cursor_idx += 1;
                                            text_in_selected_all = false;
                                        }
                                        
                                        {
                                            auto* ui = &dt::DrawTalk::ctx()->ui;
                                            ui->margin_panels[0].text.move_selections_right();
                                        }
                                    }
                                } else if (k == UI_KEY_TYPE_UP_ARROW) {
                                    if (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND))) {
                                        text_in_history.rewind_get([&](const mtt::String& str) {
                                            text_in_selected_all = false;
                                            text_in_cursor_idx = 0;
                                            text_in_buf = str;
                                        });
                                        //                                mtt::String str = text_in_history.get_left();
                                        //                                if (text_in_history.rewind()) {
                                        //                                    text_in_selected_all = false;
                                        //                                    text_in_cursor_idx = 0;
                                        //                                    text_in_buf = str;
                                        //                                }
                                    } else
                                    {
                                        auto* ui = &dt::DrawTalk::ctx()->ui;
                                        ui->margin_panels[0].text.move_selections_up();
                                        if (text_in_cursor_idx != 0) {
                                            usize i = text_in_cursor_idx - 1;
                                            while (i != 0 && text_in_buf[i] != '\n') {
                                                i -= 1;
                                            }
                                            text_in_cursor_idx = i;
                                        }
                                        text_in_selected_all = false;
                                    }
                                } else if (k == UI_KEY_TYPE_DOWN_ARROW) {
                                    if (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND))) {
                                        {
                                            const mtt::String& str = text_in_history.forward_get();
                                            //                                    mtt::String str = text_in_history.get_right();
                                            //                                    text_in_history.forwards();
                                            text_in_selected_all = false;
                                            text_in_cursor_idx = 0;
                                            text_in_buf = str;
                                            
                                            
                                        }
                                    } else
                                    {
                                        auto* ui = &dt::DrawTalk::ctx()->ui;
                                        ui->margin_panels[0].text.move_selections_down();
                                        if (text_in_cursor_idx != text_in_buf.size()) {
                                            usize i = text_in_cursor_idx + 1;
                                            while (i != text_in_buf.size() && text_in_buf[i] != '\n') {
                                                i += 1;
                                            }
                                            text_in_cursor_idx = i;
                                        }
                                        text_in_selected_all = false;
                                    }
                                }
                                //                        else if (k == UI_KEY_TYPE_DELETE_OR_BACKSPACE) {
                                //                            // TODO
                                //key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND)))
                                //                        }
                                else {
#undef MTT_CHECK_MODIFIER_KEY
                                    if (event.count > 0) {
                                        auto* chars = event.characters;
                                        if (text_in_cursor_idx == text_in_buf.size()) {
                                            text_in_cursor_idx += 1;
                                            
                                            text_in_buf += chars;
                                            
                                        } else {
                                            text_in_buf = text_in_buf.substr(0, text_in_cursor_idx) + chars + text_in_buf.substr(text_in_cursor_idx);
                                            text_in_cursor_idx += 1;
                                        }
                                        
                                        {
                                            auto* ui = &dt::DrawTalk::ctx()->ui;
                                            ui->margin_panels[0].text.send_text_input(text_in_buf, false);
                                        }
                                    }
                                    
                                }
                                
                                
                                if (text_in_selected_all) {
                                    text_in_selected_all = false;
                                    text_in_buf.clear();
                                    text_in_cursor_idx = 0;
                                    if (do_update_ui_cmd_text) {
                                        core->set_command_line_text(core, "", 0, SELECT_ALL_MODE_OFF);
                                    }
                                }
                                
                                if (!text_in_buf.empty()) {
                                    mtt::String out = text_in_buf.substr(0, text_in_cursor_idx) + "]_[" + ((text_in_cursor_idx != text_in_buf.size()) ? text_in_buf.substr(text_in_cursor_idx) : "");
                                    if (do_update_ui_cmd_text) {
                                        core->set_command_line_text(core, out.c_str(), out.size(), 0);
                                    }
                                    
                                    MTT_print("Command: %s]_[%s\n", text_in_buf.substr(0, text_in_cursor_idx).c_str(), ((text_in_cursor_idx != text_in_buf.size()) ? text_in_buf.substr(text_in_cursor_idx).c_str() : ""));
                                } else {
                                    if (do_update_ui_cmd_text) {
                                        MTT_print("%s", "Command: ]_[\n");
                                        core->set_command_line_text(core, "", 0, 0);
                                    }
                                }
                                
                                
                                //MTT_print("KEY BEGAN: %s\n", ui_key_type_to_string[event.key]);
                                break;
                            }
                            case UI_TOUCH_STATE_KEY_CHANGED: {
                                break;
                            }
                            case UI_TOUCH_STATE_KEY_CANCELLED: {
                                break;
                            }
                            case UI_TOUCH_STATE_KEY_ENDED: {
                                break;
                            }
                            default: {}
                        }
                        
                        break;
                    }
                    default : { break; }
                        break;
                }
                /////////////////////////////////////////////////////////////////////////////////////////////////
                
            }
        }
    }
    
    
#undef PR

    // MARK:- Input end
    
    //    core->time_sim_accumulator_counter_ns = (core->time_sim_accumulator_counter_ns > (MTT_TIMESTEP_NS * 2)) ? MTT_TIMESTEP_NS * 2 : core->time_sim_accumulator_counter_ns;
    //
    sd::set_render_layer(renderer, LAYER_LABEL_COLLISION_DEBUG);
    sd::rewind_layer(renderer, LAYER_LABEL_COLLISION_DEBUG);
    
    static mtt::Set<mtt::Thing_ID> particle_systems;
    
    
    mtt::run_deferred_per_frame(&world->mtt_world, MTT_TIMESTEP, core->time_seconds_prev, core->time_seconds, core, (void*)world);
    
    particle_systems.clear();
    
    
    core->time_sim_accumulator_counter_ns = m::min(core->time_sim_accumulator_counter_ns, MTT_TIMESTEP_NS * 4);
    
    //MTT_print("ACCUMULATE: %llu\n", core->time_sim_accumulator_counter_ns);
    bool IS_PAUSED = (MTT_Core_active_pause_state(core) == true);
    while (core->time_sim_accumulator_counter_ns >= MTT_TIMESTEP_NS || IS_PAUSED) {
        
        if (thing_arrows.is_active) {
            mtt::pan_camera_at_edge(core->viewport, world->cam, core->view_position, thing_arrows.canvas_pos);
        }
        
        twn_update(core->tween_ctx, MTT_TIMESTEP);
        if (!IS_PAUSED) {
            core->time_sim_accumulator_counter_ns -= MTT_TIMESTEP_NS;
            core->time_sim_elapsed += MTT_TIMESTEP;
            core->time_sim_elapsed_ns += MTT_TIMESTEP_NS;

            world->mtt_world.time_seconds_prev = world->mtt_world.time_seconds;
            world->mtt_world.time_ns_prev = world->mtt_world.time_ns;
            world->mtt_world.time_seconds = core->time_sim_elapsed;
            world->mtt_world.time_ns = core->time_sim_elapsed_ns;
            world->mtt_world.eval_count += 1;
        }
        //MTT_print("TIME SIM ELAPSED: %f\n", world->mtt_world.time_seconds);

        mtt::evaluate_world_pre(&world->mtt_world);
        
#ifndef NDEBUG
        if (continuous_rule_tests_enabled && (!continuous_rule_tests.empty())) {
            dt::print_rule_results_no_free(mtt_world, continuous_rule_tests);
        }
#endif
        
        if (!IS_PAUSED) {
            mtt::evaluate_world(&world->mtt_world);
        }
        
        //printf("%llu\n", things->size());
        for (auto it = mtt::iterator(&world->mtt_world); mtt::next(&it);) {
            
            Thing* thing = it.value;
            
            
            if (!mtt::is_root(thing)) {
                continue;
            }
            
            
            
            //transform_stack.clear();
            
            transform_stack.emplace_back((Transform_Hierarchy_Record){
                .parent_transform = mat4(1.0f),
                .thing = thing,
                .parent_rep = nullptr,
            });
            
            
            
            //MTT_print("---------\n");
            while (!transform_stack.empty()) {
                //MTT_print("depth=[%llu]\n", child_depth);
                
                mat4 parent_transform = transform_stack.back().parent_transform;
                thing = transform_stack.back().thing;
                mtt::Rep* parent_rep = transform_stack.back().parent_rep;
                transform_stack.pop_back();
                
                switch (thing->archetype_id) {
                    case mtt::ARCHETYPE_FREEHAND_SKETCH: {
                        if (world->dt.scn_ctx.thing_selected_with_pen == thing->id) {
                            continue;
                        }
                        
                        auto* vel = (vec3*)mtt::access(thing, (usize)FREEHAND_SKETCH_INDEX_VELOCITY);
                        auto* acc = (vec3*)mtt::access(thing, (usize)FREEHAND_SKETCH_INDEX_ACCELERATION);
                        auto* pos = (vec3*)mtt::access(thing, (usize)FREEHAND_SKETCH_INDEX_POSITION);
                        
                        auto* acc_local = mtt::access<vec3>(thing, "acceleration_local");
                        auto* vel_local = mtt::access<vec3>(thing, "velocity_local");
                        
                        mtt::Representation& rep = *mtt::rep(thing);
                        
                        if (true || mtt::is_default_movement(thing->logic.option_flags)) { // physics on or positional?
                            //if (mtt::is_root(thing) && ROTATE_TEST_ON) {
                                //*acc = vec3(0.0f, 9.8f / mtt::MOVEMENT_SCALE, 0.0f);
                            //} else {
                                //*acc = vec3(0.0f, 0.0f, 0.0f);
                            //}
                            
                            
                            vec3 translation = (!IS_PAUSED && thing->selection_count == 0 && !mtt::is_paused(thing)) ? integrate((float32)MTT_TIMESTEP, vel, acc)  + integrate((float32)MTT_TIMESTEP, vel_local, acc_local) : m::vec3_zero();
                            
                            if constexpr (ROTATE_TEST_ON) {
                                
                                
                                //mat4 rot_global = m::mat4_identity();
                                mat4 rot_global = m::rotate(mat4(1.0f), ((float32)MTT_TIMESTEP / 4.0f) * ((rep.forward_dir.x < 0) ? -1.0f : 1.0f), vec3(0.0f, 0.0f, 1.0f));
                                
                                //                            mat4 rot_local = m::rotate(mat4(1.0f), m::sinf32(time) * ((float32)MTT_TIMESTEP / 2.0f) * ((rep.forward_dir.x < 0) ? -1.0f : 1.0f), vec3(0.0f, 0.0f, 1.0f));
                                mat4 rot_local = m::mat4_identity();
                                
                                
                                if (!rep.colliders.empty()) {
                                    vec3 OFF = {rep.colliders[0]->aabb.half_extent.x, 0.0f, 0.0f};
                                    rep.center_offset = OFF;
                                }
                                if ((!mtt::is_root(thing))) {
                                    
                                    rep.model_transform = m::rotate_around_with_matrix(rep.model_transform, rot_global, rot_local, rep.center_offset) * m::translate(translation);
                                    
                                    
                                    
                                    set_pose_transform(thing, m::mat4_identity() * rep.pose_transform);
                                } else {
                                    rep.model_transform = rep.model_transform * m::translate(translation);
                                }
                                rep.model_transform_inverse = m::inverse(rep.model_transform);
                                
                                rep.hierarchy_model_transform = parent_transform * rep.model_transform;
                                //rep.world_transform_inverse = m::inverse(rep.world_transform);
                            } else {
                                
                                if (!rep.colliders.empty()) {
                                    vec3 OFF = {rep.colliders[0]->aabb.half_extent.x, 0.0f, 0.0f};
                                    rep.center_offset = OFF;
                                }
                                {
                                    rep.model_transform = rep.model_transform * m::translate(translation);
                                }
                                rep.model_transform_inverse = m::inverse(rep.model_transform);
                                
                                
                                // TODO: ...
//                                if (!mtt::is_root(thing)) {
//                                    vec3 d_scale;
//                                    quat d_orientation;
//                                    vec3 d_translation;
//                                    vec3 d_skew;
//                                    vec4 d_perspective;
//                                    
//                                    m::decompose(parent_rep->pose_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
//                                    
//                                    
//                                    rep.hierarchy_model_transform = parent_transform * (m::toMat4(d_orientation)) * rep.model_transform;
//                                } else 
                                {
                                    rep.hierarchy_model_transform = parent_transform * rep.model_transform;
                                }
                                
                                //rep.world_transform_inverse = m::inverse(rep.world_transform);
                            }
                            if (thing->is_static) {
                                rep.hierarchy_model_transform = world->cam.cam_transform * rep.hierarchy_model_transform;
                            }
                            
                            vec3 d_scale;
                            quat d_orientation;
                            vec3 d_translation;
                            vec3 d_skew;
                            vec4 d_perspective;
                            {
                                
                                m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                
                                rep.transform.translation = d_translation;
                                
                                
                                const float32 scale_mult = (!thing->is_static) ? 1.0f : d_scale.x;
                                
                                for (usize i_col = 0; i_col < rep.colliders.size(); i_col += 1) {
                                    Collider* collider = rep.colliders[i_col];
                                    Collider_remove(collider->system, 0, collider);
                                    
                                    switch (collider->type) {
                                        case COLLIDER_TYPE_AABB: {
                                            
                                            if (LOOK_AT_TEST_ON && !rep.colliders.empty() && !mtt::is_root(thing)) {
                                                auto& p_xform = rep.pose_transform;
                                                auto curr_xform = p_xform * (
                                                                             ((rep.forward_dir.x < 0) ? m::inverse(m::toMat4(d_orientation)) : m::toMat4(d_orientation))
                                                                             );
                                                
                                                vec3 local_d_scale;
                                                quat local_d_orientation;
                                                vec3 local_d_translation;
                                                vec3 local_d_skew;
                                                vec4 local_d_perspective;
                                                
                                                vec3 local_d_scale_root;
                                                quat local_d_orientation_root;
                                                vec3 local_d_translation_root;
                                                vec3 local_d_skew_root;
                                                vec4 local_d_perspective_root;
                                                
                                                mtt::Thing* root = mtt::get_parent(thing);
                                                if (root == nullptr) {
                                                    goto LOOK_AT_TEST_END;
                                                }
                                                mtt::Rep& root_rep = *mtt::rep(root);
                                                
                                                auto& p_xform_root = root_rep.pose_transform;
                                                auto& root_curr_xform = root_rep.colliders[0]->transform;
                                                
                                                Quat src_q_rot;
                                                Quat dst_q_rot;
                                                
                                                m::decompose(curr_xform, local_d_scale, local_d_orientation, local_d_translation, local_d_skew, local_d_perspective);
                                                
                                                vec3 src_pos = local_d_translation;
                                                
                                                src_q_rot = local_d_orientation;
                                                m::decompose(root_curr_xform, local_d_scale_root, local_d_orientation_root, local_d_translation_root, local_d_skew_root, local_d_perspective_root);
                                                dst_q_rot = local_d_orientation_root;
                                                
                                                vec3 dst_pos = local_d_translation_root;
                                                
                                                auto* src_translation_ptr = mtt::access<vec3>(thing, "position");
                                                auto* dst_translation_ptr = mtt::access<vec3>(root, "position");
                                                ASSERT_MSG(src_translation_ptr != nullptr, "SHOULD NOT BE NULL!");
                                                ASSERT_MSG(dst_translation_ptr != nullptr, "SHOULD NOT BE NULL!");
                                                
                                                vec3 up      = src_q_rot * vec3(0,-1,0);
                                                vec3 right   = src_q_rot * vec3(1,0,0);
                                                sd::save(renderer);
                                                sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD);
                                                sd::set_color_rgba_v4(renderer, vec4(0.0f, 1.0f, 0.0f, 1.0f));
                                                sd::set_path_radius(renderer, 1);
                                                sd::begin_path(renderer);
                                                sd::path_arrow_head_with_tail(renderer, d_translation, *dst_translation_ptr, 10.0f);
                                                sd::break_path(renderer);
                                                sd::set_path_radius(renderer, 4);
                                                
                                                sd::set_color_rgba_v4(renderer, vec4(1.0f, 0.0f, 0.0f, 1.0f));
                                                
                                                float32 scalar = m::max(collider->aabb.half_extent.x, collider->aabb.half_extent.y);
                                                sd::path_arrow_head_with_tail(renderer, d_translation, d_translation + (scalar * up), 10.0f);
                                                sd::break_path(renderer);
                                                
                                                
                                                sd::set_color_rgba_v4(renderer, vec4(0.0f, 1.0f, 0.0f, 1.0f));
                                                sd::path_arrow_head_with_tail(renderer, d_translation, d_translation + (scalar * right), 10.0f);
                                                sd::end_path(renderer);
                                                
                                                vec3 v_to = m::normalize(*dst_translation_ptr - d_translation);
                                                
                                                float32 dt = MTT_TIMESTEP;
                                                float32 r_scale = 10.0f;
                                                
                                                auto [rot_by, sign, dot_product, cross_product] = m::rotation_toward(v_to, right, r_scale, dt);
                                                {
                                                    mat4 r_mat = m::rotate(Mat4(1.0f), rot_by, vec3(0, 0, 1));
                                                    set_pose_transform(thing, r_mat * rep.pose_transform);
                                                }
                                                
                                                sd::restore(renderer);
                                            }
                                        LOOK_AT_TEST_END:;
                                            
                                            
                                            auto& aabb = collider->aabb;
                                            vec3 half_extent_v3 = vec3(aabb.half_extent * scale_mult, 0.0f);
                                            //vec2 half_extent    = vec2(half_extent_v3);
                                            
                                            collider->center_anchor = d_translation;
                                            collider->aabb.tl = collider->center_anchor - half_extent_v3;
                                            collider->aabb.br = collider->center_anchor + half_extent_v3;
                                            collider->transform = rep.pose_transform * (
                                                                                        ((rep.forward_dir.x < 0) ? m::inverse(m::toMat4(d_orientation)) : m::toMat4(d_orientation))
                                                                                        );
                                            
                                            
                                            
                                            push_AABBs(collider->system, collider, 1, 0);
                                            break;
                                        }
                                        case COLLIDER_TYPE_LINE_SEGMENT: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        case COLLIDER_TYPE_CIRCLE: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        case COLLIDER_TYPE_POINT: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        case COLLIDER_TYPE_CONCAVE_HULL: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        default: {
                                            MTT_print("%s", "Warning: unknown collider type\n");
                                            break;
                                        }
                                    }
                                    
                                }
                                
                                
                            }
                            
                            *pos = d_translation;
                            
                            
                            
                            
                        } else if (mtt::is_positional_movement(thing->logic.option_flags)) { // position set directly
                            
                            // TODO: apply same fixes as in velocity-driven movement case
                            
                            
                            
                            
                            rep.model_transform = m::inverse(parent_transform) * m::translate(*pos);
                            rep.model_transform_inverse = m::inverse(rep.model_transform);
                            
                            rep.hierarchy_model_transform = parent_transform * rep.model_transform;
                            //rep.world_transform_inverse = m::inverse(rep.world_transform);
                            
                            vec3 d_scale;
                            quat d_orientation;
                            vec3 d_translation;
                            vec3 d_skew;
                            vec4 d_perspective;
                            {
                                m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                
                                rep.transform.translation = d_translation;
                                
                                for (usize i_col = 0; i_col < rep.colliders.size(); i_col += 1) {
                                    Collider* collider = rep.colliders[i_col];
                                    Collider_remove(collider->system, 0, collider);
                                    
                                    switch (collider->type) {
                                        case COLLIDER_TYPE_AABB: {
                                            auto& aabb = collider->aabb;
                                            vec3 half_extent_v3 = vec3(aabb.half_extent, 0.0f);
                                            
                                            //vec2 half_extent    = vec2(half_extent_v3);
                                            
                                            collider->center_anchor = d_translation;
                                            collider->aabb.tl = collider->center_anchor - half_extent_v3;
                                            collider->aabb.br = collider->center_anchor + half_extent_v3;
                                            collider->transform = rep.pose_transform * (
                                                                                        ((rep.forward_dir.x < 0) ? m::inverse(m::toMat4(d_orientation)) : m::toMat4(d_orientation))
                                                                                        );
                                            
                                            push_AABBs(collider->system, collider, 1, 0);
                                            break;
                                        }
                                        case COLLIDER_TYPE_LINE_SEGMENT: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        case COLLIDER_TYPE_CIRCLE: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        case COLLIDER_TYPE_POINT: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        case COLLIDER_TYPE_CONCAVE_HULL: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        default: {
                                            MTT_print("%s", "Warning: unknown collider type\n");
                                            break;
                                        }
                                    }
                                    
                                }
                            }
                        }
                        
                        for (auto it_child = thing->child_id_set.begin();
                             it_child != thing->child_id_set.end(); ++it_child) {
                            
                            mtt::Thing* child = nullptr;
                            thing->world()->Thing_get(*it_child, &child);
                            transform_stack.emplace_back((Transform_Hierarchy_Record){
                                .parent_transform = rep.hierarchy_model_transform,
                                .thing = child,
                                .parent_rep = &rep,
                            });
                        }
                        
                        break;
                    }
                    case mtt::ARCHETYPE_PARTICLE_SYSTEM: {
                        particle_systems.insert(thing->id);
                        auto* vel = (vec3*)mtt::access(thing, (usize)FREEHAND_SKETCH_INDEX_VELOCITY);
                        auto* acc = (vec3*)mtt::access(thing, (usize)FREEHAND_SKETCH_INDEX_ACCELERATION);
                        auto* pos = (vec3*)mtt::access(thing, (usize)FREEHAND_SKETCH_INDEX_POSITION);
                        {
                            mtt::Representation& rep = *mtt::rep(thing);
                            
                            {
                                vec3 translation = (!IS_PAUSED && thing->selection_count == 0 && !mtt::is_paused(thing)) ? integrate((float32)MTT_TIMESTEP, vel, acc) : m::vec3_zero();
                                
                                rep.model_transform = rep.model_transform * m::translate(translation);
                                rep.model_transform_inverse = m::inverse(rep.model_transform);
                                
                                rep.hierarchy_model_transform = parent_transform * rep.model_transform;
                                //rep.world_transform_inverse = m::inverse(rep.world_transform);
                                
                                vec3 d_scale;
                                quat d_orientation;
                                vec3 d_translation;
                                vec3 d_skew;
                                vec4 d_perspective;
                                {
                                    m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                    
                                    rep.transform.translation = d_translation;
                                    
                                    for (usize i_col = 0; i_col < rep.colliders.size(); i_col += 1) {
                                        Collider* collider = rep.colliders[i_col];
                                        
                                        Collider_remove(collider->system, 0, collider);
                                        
                                        switch (collider->type) {
                                            case COLLIDER_TYPE_AABB: {
                                                auto& aabb = collider->aabb;
                                                vec3 half_extent_v3 = vec3(aabb.half_extent, 0.0f);
                                                vec2 half_extent    = vec2(half_extent_v3);
                                                
                                                collider->center_anchor = d_translation;
                                                collider->aabb.tl = collider->center_anchor - half_extent_v3;
                                                collider->aabb.br = collider->center_anchor + half_extent_v3;
                                                collider->transform = rep.pose_transform * (
                                                                                            ((rep.forward_dir.x < 0) ? m::inverse(m::toMat4(d_orientation)) : m::toMat4(d_orientation))
                                                                                            );
                                                
                                                push_AABBs(collider->system, collider, 1, 0);
                                                break;
                                            }
                                            case COLLIDER_TYPE_LINE_SEGMENT: {
                                                MTT_print("%s", "Warning: need to handle collider\n");
                                                break;
                                            }
                                            case COLLIDER_TYPE_CIRCLE: {
                                                MTT_print("%s", "Warning: need to handle collider\n");
                                                break;
                                            }
                                            case COLLIDER_TYPE_POINT: {
                                                MTT_print("%s", "Warning: need to handle collider\n");
                                                break;
                                            }
                                            case COLLIDER_TYPE_CONCAVE_HULL: {
                                                MTT_print("%s", "Warning: need to handle collider\n");
                                                break;
                                            }
                                            default: {
                                                MTT_print("%s", "Warning: unknown collider type\n");
                                                break;
                                            }
                                        }
                                        
                                    }
                                }
                                
                                
                                auto* position = mtt::access<vec3>(thing, "position");
                                if (position) {
                                    //vec3 extent = vec3((collider->aabb.br - collider->aabb.tl), 0.0f);
                                    *position = d_translation;
                                }
                                
                                for (auto it_child = thing->child_id_set.begin();
                                     it_child != thing->child_id_set.end(); ++it_child) {
                                    
                                    mtt::Thing* child = nullptr;
                                    thing->world()->Thing_get(*it_child, &child);
                                    transform_stack.emplace_back((Transform_Hierarchy_Record){
                                        .parent_transform = rep.hierarchy_model_transform,
                                        .thing = child,
                                        .parent_rep = &rep,
                                    });
                                }
                            }
                        }
                        
                        
                        
                        if (!IS_PAUSED && !mtt::is_paused(thing)) {
                            mtt::access_array<Particle_System_State>( thing, "particle_state_list", [&](auto& array) {
                                
                                
                                for (usize i = 0; i < array.size(); i += 1) {
                                    auto& pc = array[i];
                                    //if (!mtt::is_positional_movement(thing->logic.option_flags)) {
                                    vec3 translation = mtt::integrate((float32)MTT_TIMESTEP, pc.velocity, m::vec3_zero());
                                    
                                    pc.position += translation;
                                    //}
                                    
                                    pc.angle += pc.angular_velocity * MTT_TIMESTEP;
                                }
                            });
                        }
                        break;
                    }
                    case mtt::ARCHETYPE_GROUP:
                        MTT_FALLTHROUGH
                    case mtt::ARCHETYPE_SLIDER:
                        MTT_FALLTHROUGH
                    case mtt::ARCHETYPE_TEXT:
                        MTT_FALLTHROUGH
                    case mtt::ARCHETYPE_NUMBER:
                        MTT_FALLTHROUGH
                    case mtt::ARCHETYPE_SENSOR:
                        MTT_FALLTHROUGH
                    case mtt::ARCHETYPE_NODE_GRAPH:
                        MTT_FALLTHROUGH
                    case mtt::ARCHETYPE_UI_ELEMENT: {
                        
                        if (mtt::is_active(thing)) {
                            mtt::Representation& rep = *mtt::rep(thing);

                            rep.model_transform = rep.model_transform;
                            rep.model_transform_inverse = m::inverse(rep.model_transform);
                            
                            rep.hierarchy_model_transform = parent_transform * rep.model_transform;
                            //rep.world_transform_inverse = m::inverse(rep.world_transform);
                            
                            if (thing->is_static) {
                                rep.hierarchy_model_transform = world->cam.cam_transform * rep.hierarchy_model_transform;
                            }
                            
                            vec3 d_scale;
                            quat d_orientation;
                            vec3 d_translation;
                            vec3 d_skew;
                            vec4 d_perspective;
                            {
                                m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                
                                rep.transform.translation = d_translation;
                                
                                const float32 scale_mult = (!thing->is_static) ? 1.0f : d_scale.x;
                                
                                for (usize i_col = 0; i_col < rep.colliders.size(); i_col += 1) {
                                    Collider* collider = rep.colliders[i_col];
                                    
                                    Collider_remove(collider->system, 0, collider);
                                    
                                    switch (collider->type) {
                                        case COLLIDER_TYPE_AABB: {
                                            auto& aabb = collider->aabb;
                                            vec3 half_extent_v3 = vec3(aabb.half_extent * scale_mult, 0.0f);
                                            vec2 half_extent    = vec2(half_extent_v3);
                                            
                                            collider->center_anchor = d_translation;
                                            collider->aabb.tl = collider->center_anchor - half_extent_v3;
                                            collider->aabb.br = collider->center_anchor + half_extent_v3;
                                            collider->transform = rep.pose_transform * (
                                                                                        ((rep.forward_dir.x < 0) ? m::inverse(m::toMat4(d_orientation)) : m::toMat4(d_orientation))
                                                                                        );
                                            
                                            push_AABBs(collider->system, collider, 1, 0);
                                            break;
                                        }
                                        case COLLIDER_TYPE_LINE_SEGMENT: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        case COLLIDER_TYPE_CIRCLE: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        case COLLIDER_TYPE_POINT: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        case COLLIDER_TYPE_CONCAVE_HULL: {
                                            MTT_print("%s", "Warning: need to handle collider\n");
                                            break;
                                        }
                                        default: {
                                            MTT_print("%s", "Warning: unknown collider type\n");
                                            break;
                                        }
                                    }
                                    
                                }
                            }
                            
                            
                            auto* position = mtt::access<vec3>(thing, "position");
                            if (position) {
                                //vec3 extent = vec3((collider->aabb.br - collider->aabb.tl), 0.0f);
                                *position = d_translation;
                            }
                            
                            for (auto it_child = thing->child_id_set.begin();
                                 it_child != thing->child_id_set.end(); ++it_child) {
                                
                                mtt::Thing* child = nullptr;
                                thing->world()->Thing_get(*it_child, &child);
                                transform_stack.emplace_back((Transform_Hierarchy_Record){
                                    .parent_transform = rep.hierarchy_model_transform,
                                    .thing = child,
                                    .parent_rep = &rep,
                                });
                            }
                        }
                        
                        break;
                    }
                    case mtt::ARCHETYPE_SYSTEM_ELEMENT: {
                        break;
                    }
                }
                
            }
        }
        
        mtt::evaluate_world_post(&world->mtt_world);
        
        if (IS_PAUSED) {
            break;
        }
    }
    
    //    {
    //        static char c = 'a';
    //        static uint32 idx = 0;
    //
    //        mtt::String str_in = "Toio status: " + mtt::String(1, idx + 'a');
    //        mtt::text_update_value(tanginfo__, str_in);
    //        idx = (idx + 1) % 26;
    //        c = 'a' + idx;
    //    }
    //MTT_print("sim_count %llu\n", count);
    // set all final frame transforms for all things
    {
        auto* vg = nvgGetGlobalContext();
        nvgSave(vg);
        
        struct Settings {
            float line_h;
        } font_settings;
        font_settings.line_h = 0;
        
        cstring fonts[2] = {"sans-mono", "sans"};
        
        usize font_idx_sans_mono = 0;
        usize font_idx_sans = 1;
        usize curr_font_idx = font_idx_sans_mono;
        auto set_font = [&]() {
            nvgFontSize(vg, 64.0f);
            nvgFontFace(vg, fonts[curr_font_idx]);
            nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
            nvgTextMetrics(vg, NULL, NULL, &font_settings.line_h);
            if (mtt::color_scheme() == mtt::COLOR_SCHEME_LIGHT_MODE) {
                nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 1.0f));
                nvgStrokeColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 1.0f));
            } else {
                nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
                nvgStrokeColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
            }
        };
        set_font();
        
        Mat4 view = world->cam.view_transform;
        nvgSetViewTransform(vg, value_ptr(view));
        
        sd::save(renderer);
        sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD);
        
        vec3 scale;
        quat orientation;
        vec3 translation;
        vec3 skew;
        vec4 perspective;
        
        m::decompose(world->cam.cam_transform, scale, orientation, translation, skew, perspective);
        
        const float32 link_scale = m::max(1.0f, scale.x);
        
        sd::begin_path(renderer);
        
        for (auto it = mtt::iterator(&world->mtt_world); mtt::next(&it);) {
            mtt::Rep* const rep = mtt::rep(it.value);
            const Mat4 xform = rep->hierarchy_model_transform * rep->pose_transform * m::translate(rep->offset);
            auto& drawable_info_list = rep->render_data.drawable_info_list;
            for (usize i_drawable = 0; i_drawable < drawable_info_list.size(); i_drawable += 1) {
                auto* const drawable = drawable_info_list[i_drawable];
                sd::set_transform(drawable, xform);
            }
            
            if (it.value->archetype_id == mtt::ARCHETYPE_PARTICLE_SYSTEM) {
                
                mtt::Thing* particle_system = it.value;
                mtt::access_array<Particle_System_State>( particle_system, "particle_state_list", [xform, particle_system](auto& array) {
                    const auto& p_xform = xform;
                    
                    auto& dr_list = **mtt::access_pointer_to_pointer<mtt::Dynamic_Array<sd::Drawable_Info>*>(particle_system, "particle_drawable_list");
                    
                    for (usize i = 0; i < array.size(); i += 1) {
                        auto& pc = array[i];
                        
                        
                        auto& dr = dr_list.get_slot(i);
                        
                        dr.set_transform(
                                         p_xform * m::translate(mat4(1.0f), pc.position) * m::rotate(mat4(1.0f), pc.angle, vec3(0.0f, 0.0f, 1.0f)) * m::scale(mat4(1.0f), vec3((float32)pc.scale, (float32)pc.scale, 1.0f))
                                         );
                        
                    }
                    // TODO: ...
                } );
                
                
            } else if (it.value->archetype_id == mtt::ARCHETYPE_NUMBER || it.value->archetype_id == mtt::ARCHETYPE_TEXT) {
                if (mtt::Thing_is_reserved(it.value)) {
                    continue;
                }
                
                auto arch = it.value->archetype_id;
                if (arch == mtt::ARCHETYPE_NUMBER && curr_font_idx != font_idx_sans_mono) {
                    curr_font_idx = font_idx_sans_mono;
                    set_font();
                } else if (arch == mtt::ARCHETYPE_TEXT && curr_font_idx != font_idx_sans) {
                    curr_font_idx = font_idx_sans;
                    set_font();
                }
                
                cstring cs = MTT_string_ref_to_cstring(it.value->label);
                Collider* dst_col = rep->colliders.front();
                
                //mat4 xform = rep->model_transform * rep->pose_transform;
                mat4 local_xform = xform;
                mat4 identity = mat4(1.0f);
                nvgSetModelTransform(vg, value_ptr(local_xform));
                if (mtt::Thing_is_proxy(it.value)) {
                    nvgSetViewTransform(vg, value_ptr(world->dt.ui.top_panel.cam.view_transform));
                    nvgText(vg, dst_col->aabb.tl.x - dst_col->center_anchor.x, dst_col->aabb.tl.y - dst_col->center_anchor.y, cs, NULL);
                    nvgSetViewTransform(vg, value_ptr(view));
                } else {
                    if (it.value->is_static) {
                        nvgSetViewTransform(vg, value_ptr(identity));
                        mat4 final_xform = view * local_xform;
                        vec3 d_scale;
                        {
                            
                            quat d_orientation;
                            vec3 d_translation;
                            vec3 d_skew;
                            vec4 d_perspective;
                            {
                                m::decompose(view, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                            }
                        }
                        nvgSetModelTransform(vg, value_ptr(final_xform));
                        nvgText(vg, (dst_col->aabb.tl.x - dst_col->center_anchor.x) * (d_scale.x), (dst_col->aabb.tl.y - dst_col->center_anchor.y) * (d_scale.x), cs, NULL);
                        nvgSetViewTransform(vg, value_ptr(view));
                    } else {
                        nvgText(vg, dst_col->aabb.tl.x - dst_col->center_anchor.x, dst_col->aabb.tl.y - dst_col->center_anchor.y, cs, NULL);
                    }
                }
                
                nvgSetModelTransform(vg, value_ptr(identity));
                
                auto& in_port = it.value->ports.in_ports;
                
                {
                    
                    auto& dst_q = dst_col->aabb.saved_quad;
                    Quad_As_Segments dst_segments = Quad_segments(&dst_q);
                    
                    Port_Input_List* input_list = nullptr;
                    if (map_try_get(&root_graph(&world->mtt_world)->incoming, it.value->id, &input_list)) {
                        for (auto in_p = input_list->begin(); in_p != input_list->end(); ++in_p) {
                            if (!(*in_p).is_valid()) {
                                continue;
                            }
                            
                            mtt::Thing_ID src_id = (*in_p).src_thingref;
                            mtt::Thing* src_thing = world->mtt_world.Thing_try_get(src_id);
                            mtt::Rep* rep = mtt::rep(src_thing);
                            if (!rep->colliders.empty()) {
                                Collider* src_col = rep->colliders.front();
                                vec3 pos = src_col->center_anchor;
                                
                                auto& src_q = src_col->aabb.saved_quad;
                                Quad_As_Segments src_segments = Quad_segments(&src_q);
                                
                                vec3 src_pos = vec3(pos.x, pos.y, 900.0f);
                                vec3 dst_pos = vec3(dst_col->center_anchor.x, dst_col->center_anchor.y, 900.0f);
                                
                                mtt::Line_Segment s = {src_pos, dst_pos};
                                
                                mtt::Hit hit = {};
                                for (usize i = 0; i < 4; i += 1) {
                                    if (mtt::Line_Segment_intersection(&s, &src_segments.segments[i], &hit)) {
                                        src_pos = vec3(hit.pos, src_pos.z);
                                        
                                        mtt::Hit hit_sub = {};
                                        for (usize j = 0; j < 4; j += 1) {
                                            if (mtt::Line_Segment_intersection(&s, &dst_segments.segments[j], &hit_sub)) {
                                                dst_pos = vec3(hit_sub.pos, dst_pos.z);
                                                break;
                                            }
                                        }
                                        
                                        break;
                                    }
                                }
                                
                                
                                sd::set_color_rgba_v4(renderer, vec4(0.8f, 0.8f, 0.8f, 1.0f));
                                
                                float32 p_scale = m::max(1.0f, scale.x);
                                sd::set_path_radius(renderer, 4 * p_scale);
                                
                                sd::path_bezier_arrow_head_with_tail(renderer, src_pos, vec3((src_pos.x + dst_pos.x) * 0.5f, src_pos.y, src_pos.z), vec3(dst_pos.x, (src_pos.y + dst_pos.y) * 0.5f, dst_pos.z), dst_pos, 12.0f * p_scale, 8);
                                sd::break_path(renderer);
                            }
                        }
                    }
                }
                
            } else if (it.value->archetype_id == ARCHETYPE_FREEHAND_SKETCH && mtt::should_show_attachment_links(&world->mtt_world)) {
                mtt::Thing* child = it.value;
                if (mtt::Thing* parent = mtt::get_parent(child);
                    parent != nullptr) {
                    if (true) {
                        mtt::Rep* rep_child  = mtt::rep(child);
                        mtt::Rep* rep_parent = mtt::rep(parent);
                        if (!(rep->colliders.empty() || rep_parent->colliders.empty())) {
                            Collider* src_col = rep_parent->colliders.front();
                            vec3 src_pos = src_col->center_anchor;
                            
                            Collider* dst_col = rep_child->colliders.front();
                            vec3 dst_pos = dst_col->center_anchor;
                            sd::save(renderer);
                            
                            sd::path_radius(renderer, link_scale);
                            sd::set_color_rgba_v4(renderer, vec4(0.4f, 0.4f, 0.4f, 1.0f));
                            
                            sd::path_vertex_v3(renderer, src_pos);
                            sd::path_vertex_v3(renderer, dst_pos);
                            
                            sd::break_path(renderer);
                            
                            sd::restore(renderer);
                        }
                    }
                    
                }
            }
        }
        
        Mat4 ident = mat4(1.0f);
        nvgSetViewTransform(vg, value_ptr(ident));
        nvgSetModelTransform(vg, value_ptr(ident));
        
        
        nvgRestore(vg);
        
        sd::end_path(renderer);
        sd::restore(renderer);
    }
    
    //if (count > 2) {
    //        printf("%d %llu %llu\n", count, core->time_sim_accumulator_counter, MTT_TIMESTEP_NS);
    //}
    
    // draw UI layer
    
    // if (true || world->dt.ui.changed)
    {
        world->dt.ui.changed = false;
        
        
        
        
        sd::rewind_layer(renderer, LAYER_LABEL_UI);
        
        
        
        sd::set_render_layer(renderer, LAYER_LABEL_UI);
        float32 UI_DEPTH = 999.999f;
        
        {
            sd::set_color_rgba_v4(renderer, world->dt.pen.color);
            
            
            sd::begin_polygon(renderer);
            if (!color_changed_this_frame) MTT_LIKELY {
                sd::rectangle_rounded(renderer, color_current_tl, color_current_dim, UI_DEPTH, 0.4, 4);
            } else {
                //                sd::rectangle_rounded(renderer, color_current_tl + (color_current_dim * 0.25f * 0.5f), color_current_dim * 0.25, UI_DEPTH, 0.4, 4);
            }
            
            off += vec2(25.0f, 0.0f) + vec2(10.0f, 0.0);
            
            //            if (false && world->ct.ui.recording_indicator.is_on) {
            //                sd::set_depth(renderer, 202.0f);
            //                sd::set_color_rgba_v4(renderer, {1.0f, 0.0, 0.0, 1.0});
            //                sd::polygon_convex_regular_v2(renderer, 25.0f/2.0f, off + vec2(25.0f/2.0f, (-25.0f/2.0f) + core->viewport.height), 32);
            //            }
            bool talking_is_likely = currently_talking_is_likely(&core->speech_system.info_list[0]);
            
#if 1
            vec3 panel_y_off = vec3(0, world->dt.ui.top_panel.base.bounds.tl.y + world->dt.ui.top_panel.base.bounds.dimensions.y, 0);
            if (talking_is_likely) {
                //sd::set_depth(renderer, 999.0f);
                //sd::set_color_rgba_v4(renderer, vec4(1.0f, 0.5f, 0.0f, 0.5f));
                sd::set_color_rgba_v4(renderer, vec4(1.0f, 0.5f, 0.0f, 0.5f));
                sd::polygon_convex_regular(renderer, height_off * 0.75f, vec3(vec2(core->viewport.width, 0) - vec2(height_off/8.0f, -height_off/8.0f), 999.99f), 32);
            } else {
                //sd::set_depth(renderer, 999.9f);
                if (mtt::color_scheme() == mtt::COLOR_SCHEME_DARK_MODE) {
                    sd::set_color_rgba_v4(renderer, {0.7f, 0.7, 0.7, 0.5});
                } else {
                    sd::set_color_rgba_v4(renderer, {0.3f, 0.3, 0.3, 0.5});
                }
                sd::polygon_convex_regular(renderer, height_off * 0.75f, vec3(vec2(core->viewport.width, 0) - vec2(height_off/8.0f, -height_off/8.0f), 999.99f), 32);
            }
            //off += vec2(25.0f, 0.0f) + vec2(10.0f, 0.0) + vec2(25.0f/4.0f, height_off / 4.0f);
#endif
            sd::end_polygon(renderer);
            sd::begin_path(renderer);
            vec4 curr_op_box = {};
            vec4 curr_op_box_final = {};
            {
                auto curr_op = pointer_op_current(&input->users[0]);
                auto in_prog_op = pointer_op_in_progress(&input->users[0]);
                
                if (curr_op != in_prog_op && in_prog_op != UI_POINTER_OP_NONE) {
                    curr_op = in_prog_op;
                }
                
                vec2 input_point = vec2(0.0);
                // input point = vec2(16.0f, 16.0f);
                vec2 off_icon = vec2(0.0f);
                float32 max_x = 0.0f;
                for (usize op_i = 0; op_i < UI_POINTER_OP_COUNT; op_i += 1) {
                    auto inspected_op = UI_POINTER_OP_ARRAY[op_i];
                    
                    switch (inspected_op) {
                        case UI_POINTER_OP_DRAW: {
                            sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                            
                            input_point = off_icon + off + vec2(25.0f/2.0f, (-35.0f / 2) + core->viewport.height) + vec2(height_off/2.0f, -height_off/4.0f);
                            
                            auto dst = vec3(input_point, 0.0f) + vec3(vec2(1.0f, 1.0f) * 8, 0.0f) * 2.0f;
                            sd::path_arrow_head_with_tail(renderer, dst, vec3(input_point, 0.0), 5.0f);
                            
                            
                            curr_op_box = {input_point.x, input_point.y, dst.x, dst.y};
                            
                            auto w = curr_op_box[2] - curr_op_box[0];
                            auto expand = ((float32)w)/2.0f;
                            curr_op_box[0] -= expand;
                            curr_op_box[1] -= expand;
                            curr_op_box[2] += expand;
                            curr_op_box[3] += expand;
                            off_icon.x += w * 2;
                            if (something_touchdown__) {
                                float32* b = m::value_ptr(curr_op_box);
                                if (Box_point_check(b, most_recent_canvas_pos_something__)) {
                                    pointer_op_set_current(&input->users[0], inspected_op);
                                }
                            }
                            if (inspected_op == pointer_op_current(&input->users[0])) {
                                curr_op_box_final = curr_op_box;
                            }
                            max_x = curr_op_box[2];
                            break;
                        }
                        case UI_POINTER_OP_ERASE: {
                            if (mtt::color_scheme() == mtt::COLOR_SCHEME_DARK_MODE) {
                                sd::set_color_rgba_v4(renderer, color::WHITE);
                            } else {
                                sd::set_color_rgba_v4(renderer, color::BLACK);
                            }
                            
                            input_point = off_icon + off + vec2(25.0f/2.0f, (-35.0f / 2) + core->viewport.height) + vec2(height_off/1.5f, 0);
                            
                            vec3 src_point = vec3(input_point, 0.0f) + vec3(vec2(1.0f, 1.0f) * 8, 0.0f );
                            vec3 dst_point = vec3(input_point, 0.0f) - vec3(vec2(1.0f, 1.0f) * 8, 0.0f );
                            
                            float32 radius = 5.0f;
                            
                            float32 angle = m::atan2<float32>(dst_point.x - src_point.x, dst_point.y - src_point.y);
                            float32 angle_cos;
                            float32 angle_sin;
                            m::sincos(angle, &angle_sin, &angle_cos);
                            float32 x = radius * angle_cos;
                            float32 y = radius * angle_sin;
                            
                            vec3 top0 = {dst_point.x - x, dst_point.y + y, dst_point.z};
                            vec3 top1 = {dst_point.x + x, dst_point.y - y, dst_point.z};
                            
                            vec3 bottom0 =  {src_point.x - x, src_point.y + y, dst_point.z};
                            vec3 bottom1 = {src_point.x + x, src_point.y - y, dst_point.z};
                            
                            vec3 mid0 = m::lerp(top0, bottom0, 0.333f);
                            vec3 mid1 = m::lerp(top1, bottom1, 0.333f);
                            
                            path_vertex_v3(renderer, top0);
                            path_vertex_v3(renderer, top1);
                            path_vertex_v3(renderer, mid1);
                            path_vertex_v3(renderer, mid0);
                            path_vertex_v3(renderer, top0);
                            break_path(renderer);
                            path_vertex_v3(renderer, mid0);
                            path_vertex_v3(renderer, mid1);
                            path_vertex_v3(renderer, bottom1);
                            path_vertex_v3(renderer, bottom0);
                            path_vertex_v3(renderer, mid0);
                            break_path(renderer);
                            
                            curr_op_box = {-radius+dst_point.x, dst_point.y-radius, src_point.x+radius, src_point.y+radius};
                            
                            auto w = curr_op_box[2] - curr_op_box[0];
                            off_icon.x += w;
                            
                            if (something_touchdown__) {
                                float32* b = m::value_ptr(curr_op_box);
                                if (Box_point_check(b, most_recent_canvas_pos_something__)) {
                                    pointer_op_set_current(&input->users[0], inspected_op);
                                }
                            }
                            if (inspected_op == pointer_op_current(&input->users[0])) {
                                curr_op_box_final = curr_op_box;
                            }
                            max_x = curr_op_box[2];
                            break;
                        }
                        case UI_POINTER_OP_ARROW_CREATE: {
                            
                            if (mtt::color_scheme() == mtt::COLOR_SCHEME_DARK_MODE) {
                                sd::set_color_rgba_v4(renderer, color::WHITE);
                            } else {
                                sd::set_color_rgba_v4(renderer, color::BLACK);
                            }
                            
                            input_point = off_icon + off + vec2(25.0f/2.0f, (-35.0f / 2) + core->viewport.height) + vec2(height_off/2.0f, -height_off/4.0f);
                            
                            auto dst = vec3(input_point, 0.0f) + vec3(vec2(1.0f, 1.0f) * 8, 0.0f) * 2.0f;
                            
                            
                            if (thing_arrows.arrow_type == ARROW_LINK_FLAGS_DIRECTED) {
                                sd::path_arrow_head_bidirectional(renderer, dst, vec3(input_point, 0.0), 5.0f);
                            } else {
                                path_vertex_v3(renderer, dst);
                                path_vertex_v3(renderer, vec3(input_point, 0.0));
                            }
                            
                            curr_op_box = {input_point.x, input_point.y, dst.x, dst.y};
                            
                            auto w = curr_op_box[2] - curr_op_box[0];
                            auto expand = ((float32)w)/2.0f;
                            curr_op_box[0] -= expand;
                            curr_op_box[1] -= expand;
                            curr_op_box[2] += expand;
                            curr_op_box[3] += expand;
                            off_icon.x += w*2;
                            
                            
                            if (something_touchdown__) {
                                float32* b = m::value_ptr(curr_op_box);
                                if (Box_point_check(b, most_recent_canvas_pos_something__)) {
                                    pointer_op_set_current(&input->users[0], inspected_op);
                                }
                            }
                            if (inspected_op == pointer_op_current(&input->users[0])) {
                                curr_op_box_final = curr_op_box;
                            }
                            
                            max_x = curr_op_box[2];
                            
                            break;
                        }
                    }
                }
                max_x_bottom_left_panel = max_x;

            }
            end_path(renderer)->set_transform(m::translate(mat4(1.0f), vec3(0.0f, 0.0f, UI_DEPTH)));
            {
                const SD_vec4 old_color = sd::get_color_rgba(renderer);
                sd::set_color_rgba_v4(renderer, {0.0f, 0.0f, 0.0f, 0.2f});
                sd::begin_polygon(renderer);
                sd::rectangle_w_corners(renderer, vec2(curr_op_box_final[0], curr_op_box_final[1]), vec2(curr_op_box_final[2], curr_op_box_final[3]), 999.9f);
#if PAUSE_TRIGGER_IN_CORNER
                {
                    vec4 pause_corners = pause_trigger.corners(vec2(core->viewport.width, core->viewport.height));
                    sd::rectangle_w_corners(renderer, vec2(pause_corners[0], pause_corners[1]), vec2(pause_corners[2], pause_corners[3]), 999.9f);
                    sd::set_color_rgba_v4(renderer, color::WHITE);
                    if (MTT_Core_active_pause_state(core) == true) {
                        // paused - show play icon
                        //void triangle_equilateral(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side)
                        vec3 center = vec3((vec2(pause_corners[0], pause_corners[1]) + vec2(pause_corners[2], pause_corners[3])) * 0.5f, 999.99f);
                        float32 side = pause_trigger.side_length[0] - (2*pause_trigger.corner_offset[0]);
                        //sd::triangle_equilateral(renderer, center, 1.0f, side);
                        vec2 vh;
                        vec2 vl;
                        vec2 vr;
                        m::triangle_equilateral_points(center, -1.0f, side, &vh, &vl, &vr);
                        
                        vh = m::rotate_around_2D(center, MTT_HALF_PI_32, vh);
                        vl = m::rotate_around_2D(center, MTT_HALF_PI_32, vl);
                        vr = m::rotate_around_2D(center, MTT_HALF_PI_32, vr);
                        
                        float32 half_offset = 0.25*side;
                        vh.x -= half_offset;
                        vl.x -= half_offset;
                        vr.x -= half_offset;
                        sd::triangle(renderer, vec3(vh, 999.99f), vec3(vl, 999.99f), vec3(vr, 999.99f));
                    } else {
                        // playing - show pause icon
                        vec2 dim = vec2(pause_trigger.corner_offset[0] * 0.5f, (pause_trigger.side_length[0] - (2*pause_trigger.corner_offset[0])));
                        vec2 tl = vec2(pause_corners[0], pause_corners[1]) + pause_trigger.corner_offset;
                        vec2 br = tl + dim;
                        sd::rectangle_w_corners(renderer, tl, br, 999.99f);
                        
                        br = vec2(pause_corners[2], pause_corners[3]) - pause_trigger.corner_offset;
                        tl = br - dim;
                        sd::rectangle_w_corners(renderer, tl, br, 999.99f);
                    }
                }
#endif
                sd::end_polygon(renderer);
                sd::set_color_rgba_v4(renderer, old_color);
            }
            begin_path(renderer);
            // meter UI
#define USE_AMPLITUDE_METER_UI (0)
#if (USE_AMPLITUDE_METER_UI)
            {
                auto& panel = dt::DrawTalk::ctx()->ui.margin_panels[0];
                auto& bounds = panel.bounds;
                
                vec2 tl = vec2(core->viewport.width - (bounds.dimensions.x * (1.0f/64.0f)), 0);//bounds.tl;
                vec2 dimensions = bounds.dimensions * vec2(1.0f/64.0f, 1.0f);
                
                dt::UI& ui = dt::DrawTalk::ctx()->ui;
                auto& base = ui.bottom_panel.base;
                //auto bounds_dimensions_y = base.bounds.dimensions.y;
//
//                dimensions.y -= bounds_dimensions_y;
                
                static float prev_amplitude = 0;
                
                //tl.x -= dimensions.x;
                static int was_active = 0;
                
                static float twn_dec = 0.0f;
                
                
                auto color_background_active = vec4(1.0f * 0.3, 0.0f, 0.0f, 0.8f);
                auto color_foreground_active = vec4(1.0f * 0.7, 0.0f, 0.0f, 0.8f);
                auto color_background_inactive = vec4(0.0f, 0.0f, 0.0f, 0.8f);
                constexpr const float32 twn_speed = 0.2f;
                if (Speech_get_active_state(&core->speech_system.info_list[0])) {
                    if (was_active != 1) {
                        twn_dec = 0.0f;
                        auto* twn_ctx = mtt_core_ctx()->tween_ctx;
                        twn_Opt opt = {};
                        opt.time = twn_speed;
                        opt.ease = TWN_LINEAR;
                        opt.abort_if_in_progress = 1;
                        twn_add_f32(twn_ctx, &twn_dec, 1.0f, opt);
                        
                        was_active = 1;
                    }
                    
                    sd::set_color_rgba_v4(renderer, color_background_active);
                    sd::rectangle(renderer, tl, dimensions, 998.0f);
                    
                    
                    float amplitude = core->speech_system.amplitude;
                    tl.y += dimensions.y / 2.0f;
                    tl.y += (1.0 - twn_dec) * dimensions.y / 2.0f;
                    
                    if (talking_is_likely) {
                        tl.y -= (amplitude * 1.3f) * dimensions.y;
                    }
                    
                    tl.y = m::max(0.0f, tl.y);
                    
                    sd::set_color_rgba_v4(renderer, color_foreground_active);
                    sd::rectangle(renderer, tl, vec2(dimensions.x, base.bounds.tl.y - tl.y), 999.0f);
                    prev_amplitude = amplitude;
                    
                } else {
                    if (was_active != 0) {
                        twn_dec = twn_speed;
                        auto* twn_ctx = mtt_core_ctx()->tween_ctx;
                        twn_Opt opt = {};
                        opt.time = twn_speed;
                        opt.ease = TWN_BACKOUT;
                        opt.abort_if_in_progress = 1;
                        twn_add_f32(twn_ctx, &twn_dec, 1.0f, opt);
                        
                        was_active = 0;
                    }
                    
                    sd::set_color_rgba_v4(renderer, color_background_inactive);
                    sd::rectangle(renderer, tl, dimensions, 998.0f);
                    
                    if (twn_dec != 1.0f) {
                        tl.y -= (prev_amplitude * 2.0f) * dimensions.y;
                        tl.y += dimensions.y / 2.0f;
                        tl.y += (twn_dec) * dimensions.y / 2.0f;
                        
                        tl.y = m::max(0.0f, tl.y);
                        
                        sd::set_color_rgba_v4(renderer, color_foreground_active);
                        sd::rectangle(renderer, tl, vec2(dimensions.x, base.bounds.tl.y - tl.y), 999.0f);
                    }
                }
                
                
                
            }
#endif
            
            
            //            sd::set_depth(renderer, 500.0f);
            //            sd::polygon_convex_regular_v2(renderer, 25.0f/2.0f, off + vec2(25.0f/2.0f, (-25.0f/2.0f) + core->viewport.height), 32);
            //
            //            off += vec2(25.0f, 0.0f) + vec2(10.0f, 0.0);
            
            
            
            //        sd::set_color_rgba_v4(renderer, {0.0f, 0.0f, 0.0f, 1.0f});
            //        sd::rectangle(renderer, vec2(core->viewport.width - 30.0f, core->viewport.height - 30.0f), {25.0f, 25.0f}, 202.0f);
            
            constexpr const bool display_touches = true;
            
            
            if (display_touches && input->users[0].show_cursor) {
                vec4 outer_color;
                vec4 inner_color;
                switch (color_scheme()) {
                    case COLOR_SCHEME_LIGHT_MODE: {
                        inner_color = color::WHITE;
                        outer_color = dt::UI_TOUCH_SELECTION_COLOR//core->application_configuration.text_panel_color
                        ;
                        break;
                    }
                    case COLOR_SCHEME_DARK_MODE: {
                        MTT_FALLTHROUGH;
                    }
                    default: {
                        inner_color = color::BLACK;
                        outer_color = dt::UI_TOUCH_SELECTION_COLOR//core->application_configuration.text_panel_color
                        ;
                        break;
                    }
                }
                for (auto it = input->users[0].direct_map.begin(); it != input->users[0].direct_map.end(); ++it) {
                    if (it->second.direct.count > 0 && it->second.direct.state != UI_TOUCH_STATE_ENDED && it->second.direct.state != UI_TOUCH_STATE_CANCELLED) {
#define DRAW_EVEN_IF_NO_SELECTION (true)
#if !DRAW_EVEN_IF_NO_SELECTION
                        if (world->dt.selection_map.find(it->second.key) == world->dt.selection_map.end()) {
                            continue;
                        }
#endif
                        vec2 input_point = it->second.direct.positions[it->second.direct.count - 1];//transform_point(&world->cam, it->second.direct.positions[it->second.direct.count - 1]);
                        
                        sd::set_color_rgba_v4(renderer, outer_color);
                        sd::polygon_convex_regular(renderer, 8.0f, vec3(input_point, 999.0f), 16);
                        sd::set_color_rgba_v4(renderer, inner_color);
                        sd::polygon_convex_regular(renderer, 4.0f, vec3(input_point, 1000.0f), 16);
                    }
                }
                
                //                if (core->look_at_point != vec3(0.0f)) {
                //                    vec2 point = core->look_at_point;
                //                    sd::set_color_rgba_v4(renderer, color::GREEN);
                //                    sd::polygon_convex_regular(renderer, 16.0f, vec3(point, 999.9f), 16);
                //                }
                //                sd::set_color_rgba_v4(renderer, color::RED);
                //                sd::polygon_convex_regular(renderer, 16.0f, vec3(point.y, point.x, 999.9f), 16);
            }
            sd::end_polygon(renderer);
            sd::begin_path(renderer);
            static vec2 most_recent_azimuth_unit_vector = vec2(0.0f);
            static vec2 most_recent_input_point = vec2(0.0f);
            constexpr const uint32 ARROW_LENGTH = 8;
            bool show_a_pointer = false;
            for (auto it = input->users[0].pointer_map.begin(); it != input->users[0].pointer_map.end(); ++it) {
                auto& pointer_map = input->users[0];
                if (input->users[0].pointer_map.size() != 1) {
                    
                }
                show_a_pointer = show_a_pointer || input->users[0].show_cursor;
                if (!input->users[0].show_cursor) {
                    continue;
                }
                sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                if (it->second.pointer.count > 0) {
                    vec2 input_point = it->second.pointer.positions[it->second.pointer.count - 1];//transform_point(&world->cam, it->second.pointer.positions[it->second.pointer.count - 1]);
                    
                    auto& pointer = it->second.pointer;
                    most_recent_azimuth_unit_vector = pointer.azimuth_unit_vector;
                    most_recent_input_point = input_point;
                    auto altitude = pointer.altitude_angle_radians;
                    //
                    //
                    
                    
                    auto curr_op = pointer_op_current(&input->users[0]);
                    auto in_prog_op = pointer_op_in_progress(&input->users[0]);
                    
                    if (curr_op != in_prog_op && in_prog_op != UI_POINTER_OP_NONE) {
                        curr_op = in_prog_op;
                    }
                    
                    switch (curr_op) {
                        case UI_POINTER_OP_DRAW: {
                            sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                            
                            sd::path_arrow_head_with_tail(renderer, vec3(input_point, UI_DEPTH ) + vec3(most_recent_azimuth_unit_vector * ARROW_LENGTH, 0.0f) * (float32)(2.0 + (1.0 * (MTT_HALF_PI_64 - altitude))  ), vec3(input_point, 0.0f), 5.0f);
                            break;
                        }
                        case UI_POINTER_OP_ERASE: {
                            
                            if (mtt::color_scheme() == mtt::COLOR_SCHEME_DARK_MODE) {
                                sd::set_color_rgba_v4(renderer, color::WHITE);
                            } else {
                                sd::set_color_rgba_v4(renderer, color::BLACK);
                            }
                            
                            vec3 src_point = vec3(input_point,  UI_DEPTH  ) + vec3(most_recent_azimuth_unit_vector * ARROW_LENGTH, 0.0f );
                            vec3 dst_point = vec3(input_point,  UI_DEPTH  ) - vec3(most_recent_azimuth_unit_vector * ARROW_LENGTH, 0.0f );
                            
                            float32 radius = 5.0f;
                            
                            float32 angle = m::atan2<float32>(dst_point.x - src_point.x, dst_point.y - src_point.y);
                            float32 x = radius * m::cos(angle);
                            float32 y = radius * m::sin(angle);
                            
                            vec3 top0 = {dst_point.x - x, dst_point.y + y, dst_point.z};
                            vec3 top1 = {dst_point.x + x, dst_point.y - y, dst_point.z};
                            
                            vec3 bottom0 =  {src_point.x - x, src_point.y + y, dst_point.z};
                            vec3 bottom1 = {src_point.x + x, src_point.y - y, dst_point.z};
                            
                            vec3 mid0 = m::lerp(top0, bottom0, 0.333f);
                            vec3 mid1 = m::lerp(top1, bottom1, 0.333f);
                            
                            path_vertex_v3(renderer, top0);
                            path_vertex_v3(renderer, top1);
                            path_vertex_v3(renderer, mid1);
                            path_vertex_v3(renderer, mid0);
                            path_vertex_v3(renderer, top0);
                            break_path(renderer);
                            path_vertex_v3(renderer, mid0);
                            path_vertex_v3(renderer, mid1);
                            path_vertex_v3(renderer, bottom1);
                            path_vertex_v3(renderer, bottom0);
                            path_vertex_v3(renderer, mid0);
                            break_path(renderer);
                            
                            break;
                        }
                        case UI_POINTER_OP_ARROW_CREATE: {
                            sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                            
                            sd::path_arrow_head_bidirectional(renderer, vec3(input_point, UI_DEPTH ) + vec3(most_recent_azimuth_unit_vector * ARROW_LENGTH, 0.0f) * (float32)(2.0 + (1.0 * (MTT_HALF_PI_64 - altitude))  ), vec3(input_point, 0.0f), 5.0f);
                            break;
                        }
                    }
                    
                    
                    //                    sd::polygon_convex_regular(renderer, 8.0f, vec3(input_point, 999.0f), 16);
                    //                    sd::set_color_rgba_v4(renderer, color::BLACK);
                    //                    sd::polygon_convex_regular(renderer, 4.0f, vec3(input_point, 1000.0f), 8);
                }
            }
            
            //            if (show_a_pointer && input->users[0].pointer_map.size() == 0) {
            //
            //
            //                sd::set_color_rgba_v4(renderer, dt::pen_color_default());
            //
            ////                {
            ////
            ////                    sd::path_arrow_head_with_tail(renderer, vec3(most_recent_input_point, 1000.0f) + vec3(most_recent_azimuth_unit_vector * ARROW_LENGTH, 0.0f), vec3(most_recent_input_point, 1000.0f) - vec3(most_recent_azimuth_unit_vector * ARROW_LENGTH, 0.0f), 5.0f);
            ////                }
            //
            //                sd::path_cross(renderer, vec3(most_recent_input_point, 9000.0f), vec3(ARROW_LENGTH * 0.7f, ARROW_LENGTH * 0.7f, 0.0f));
            //            }
            
            if (input->users[0].show_cursor && !show_a_pointer && input->users[0].hover.state != UI_TOUCH_STATE_NONE) {
                
                
                sd::set_color_rgba_v4(renderer, dt::pen_color_default() * vec4(vec3(0.5), 1.0));
                
                vec2 input_point = input->users[0].hover.pos;
                
                most_recent_input_point = input_point;
                auto azimuth_unit_vector = input->users[0].hover.azimuth_unit_vector;
                auto altitude = input->users[0].hover.altitude_angle_radians;
                //
                //
                float32 hover_z = 1.0f - input->users[0].hover.z_pos;
                float32 scale = ARROW_LENGTH * m::max(0.4f, hover_z);
                
                auto curr_op = pointer_op_current(&input->users[0]);
                auto in_prog_op = pointer_op_in_progress(&input->users[0]);
                
                if (curr_op != in_prog_op && in_prog_op != UI_POINTER_OP_NONE) {
                    curr_op = in_prog_op;
                }
                
                switch (curr_op) {
                    default: {
                        MTT_FALLTHROUGH;
                    }
                    case UI_POINTER_OP_DRAW: {
                        sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                        sd::path_arrow_head_with_tail(renderer, vec3(input_point, UI_DEPTH ) + vec3(azimuth_unit_vector * ARROW_LENGTH, 0.0f) * (float32)(2.0 + (1.0 * (MTT_HALF_PI_64 - altitude))  ), vec3(input_point, 0.0f), 5.0f);
                        
                        
                        vec2 intersections = rectangle_ray_intersection_interior_guaranteed(input_point, -azimuth_unit_vector, vec2(0.0f), vec2(core->viewport.width, core->viewport.height));
                        
                        //                        sd::path_vertex_v3(renderer, vec3(input_point, UI_DEPTH));
                        //                        sd::path_vertex_v3(renderer, vec3(vec2(input_point + vec2(-azimuth_unit_vector) * intersections[1]
                        //                                                          //     * (float32)(MTT_HALF_PI_64 - altitude)
                        //                                                               ), UI_DEPTH)
                        //                                           );
                        sd::break_path(renderer);
                        break;
                    }
                    case UI_POINTER_OP_ERASE: {
                        if (mtt::color_scheme() == mtt::COLOR_SCHEME_DARK_MODE) {
                            sd::set_color_rgba_v4(renderer, color::WHITE);
                        } else {
                            sd::set_color_rgba_v4(renderer, color::BLACK);
                        }
                        vec3 src_point = vec3(input_point, UI_DEPTH ) + vec3(azimuth_unit_vector * ARROW_LENGTH, 0.0f) * hover_z;
                        vec3 dst_point = vec3(input_point, UI_DEPTH ) - vec3(azimuth_unit_vector * ARROW_LENGTH, 0.0f) * hover_z;
                        
                        float32 radius = 5.0f * hover_z;
                        
                        float32 angle = m::atan2<float32>(dst_point.x - src_point.x, dst_point.y - src_point.y);
                        float32 x = radius * m::cos(angle);
                        float32 y = radius * m::sin(angle);
                        
                        vec3 top0 = {dst_point.x - x, dst_point.y + y, dst_point.z};
                        vec3 top1 = {dst_point.x + x, dst_point.y - y, dst_point.z};
                        
                        vec3 bottom0 =  {src_point.x - x, src_point.y + y, dst_point.z};
                        vec3 bottom1 = {src_point.x + x, src_point.y - y, dst_point.z};
                        
                        vec3 mid0 = m::lerp(top0, bottom0, 0.333f);
                        vec3 mid1 = m::lerp(top1, bottom1, 0.333f);
                        
                        path_vertex_v3(renderer, top0);
                        path_vertex_v3(renderer, top1);
                        path_vertex_v3(renderer, mid1);
                        path_vertex_v3(renderer, mid0);
                        path_vertex_v3(renderer, top0);
                        break_path(renderer);
                        path_vertex_v3(renderer, mid0);
                        path_vertex_v3(renderer, mid1);
                        path_vertex_v3(renderer, bottom1);
                        path_vertex_v3(renderer, bottom0);
                        path_vertex_v3(renderer, mid0);
                        break_path(renderer);
                        break;
                    }
                    case UI_POINTER_OP_ARROW_CREATE: {
                        sd::set_color_rgba_v4(renderer, world->dt.pen.color);
                        sd::path_arrow_head_bidirectional(renderer, vec3(input_point, UI_DEPTH ) + vec3(azimuth_unit_vector * ARROW_LENGTH, 0.0f) * (float32)(2.0 + (1.0 * (MTT_HALF_PI_64 - altitude))  ), vec3(input_point, 0.0f), 5.0f);
                        
                        
                        vec2 intersections = rectangle_ray_intersection_interior_guaranteed(input_point, -azimuth_unit_vector, vec2(0.0f), vec2(core->viewport.width, core->viewport.height));
                        
                        //                        sd::path_vertex_v3(renderer, vec3(input_point, UI_DEPTH));
                        //                        sd::path_vertex_v3(renderer, vec3(vec2(input_point + vec2(-azimuth_unit_vector) * intersections[1]
                        //                                                          //     * (float32)(MTT_HALF_PI_64 - altitude)
                        //                                                               ), UI_DEPTH)
                        //                                           );
                        sd::break_path(renderer);
                        break;
                    }
                }
                
                //sd::path_cross(renderer, vec3(most_recent_input_point, 9000.0f), vec3(scale, scale, 0.0f));
            }
            sd::end_path(renderer)->set_transform(m::translate(Mat4(1.0f), vec3(0.0f, 0.0f, UI_DEPTH)));
            
            
        };
        {
            sd::set_color_rgba_v4(renderer, {1.0f, 0.0, 1.0, 1.0});
            
            sd::begin_path(renderer);
            
            vec3 src = vec3(0.0f, -4.0f, 0.0f) + vec3(off.x + 5.0f, core->viewport.height - 7.0f, 0.0f );
            vec3 dst = vec3(0.0f, -4.0f, 0.0f) + vec3(off.x + 5.0f, core->viewport.height -25.0f - 3.0f, 0.0f );
            sd::path_arrow_head_with_tail(renderer, src, dst, 25.0f/4.0f);
            
            vec3 scale;
            quat orientation;
            vec3 translation;
            vec3 skew;
            vec4 perspective;
            
            
            vec3 center = vec3(((vec2(src) + vec2(dst)) * 0.5f), 0.0f);
            
            
            
            m::decompose(world->cam.cam_transform, scale, orientation, translation, skew, perspective);
            vec3 angles = m::eulerAngles(orientation);
            auto* arrow = sd::end_path(renderer);
            arrow->set_transform(m::translate(Mat4(1.0f), center + vec3(0.0f, 0.0f, UI_DEPTH)) * m::rotate(Mat4(1.0f), -angles.z, vec3(0.0f, 0.0f, 1.0f)) * m::translate(Mat4(1.0f), -center));
            
            sd::begin_polygon(renderer);
            //sd::set_depth(renderer, 500.0f);
            sd::set_color_rgba_v4(renderer, {0.0f, 0.0f, 1.0f, 0.5f});
            sd::polygon_convex_regular(renderer, 25.0f/2.0f, center, 32);
            
            
            sd::end_polygon(renderer)->set_transform(arrow->get_transform());
        }
        
    }
    
    
    
    
    
    
    sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_TEMP);
    sd::rewind_layer(renderer, LAYER_LABEL_PER_FRAME_TEMP);
    
    { // TEST draw paths behind moved entities
        if ((false)) {
            
            {
                
                
                //sd::path_vertex_v2(renderer, {400, 700});
                //sd::path_vertex_v2(renderer, {500, 400});
                //sd::path_vertex_v2(renderer, {600, 700});
                
                std::vector<vec3> lagrange_poly_points = {
                    {400, 700, 100},
                    {500, 400, 100},
                    {1000, 1000, 100}
                };
                std::vector<std::vector<vec3>> multi = {lagrange_poly_points};
                vec3 center;
                vec2 min;
                vec2 max;
                //align_curve_center(multi, center, min, max);
                align_curve_bottom_left(multi, center, min, max);
                auto lagrange_poly = Lagrange_Polynomial_make(&multi[0]);
                
                Polynomial<vec3> poly = {};
                
                Lagrange_to_quadratic(&lagrange_poly, &poly);
                
                /*
                 for (float64 t = lagrange_poly_points[0].x; t <= lagrange_poly_points.back().x; t += 0.1) {
                 float64 y = Lagrange_Polynomial_evaluate(&lagrange_poly, t);
                 
                 sd::path_vertex_v2(renderer, {(float32)t, (float32)y});
                 }
                 */
                auto color = sd::get_color_rgba(renderer);
                sd::set_color_rgba_v4(renderer, {0.0f, 0.9, 0.0, 0.2});
                sd::begin_path(renderer);
                for (float64 t = poly.points[0].x; t <= poly.points.back().x; t += 1) {
                    float64 y = Polynomial_quadratic_evaluate(&poly, t);
                    
                    sd::path_vertex_v2(renderer, {(float32)t, (float32)y});
                }
                
                {
                    sd::Drawable_Info* drawable = sd::end_path(renderer);
                    
                    vec3 half = vec3((max - min) / 2.0f, 0.0f);
                    drawable->set_transform(m::translate(mat4(1.0f), vec3(center) - vec3(half.x, 0, 0) + vec3(0, half.y, 0)));
                }
                
                sd::begin_polygon(renderer);
                {
                    
                    float32 lerp = m::sin01(time * 4);
                    //for (float64 t = poly.points[0].x; t <= poly.points.back().x; t += 0.1) {
                    
                    
                    float32 y = Polynomial_quadratic_evaluate(&poly, poly.points[0].x + ((poly.points.back().x - poly.points[0].x) * lerp));
                    
                    sd::set_color_rgba_v4(renderer, {1.0, 0.0, 0.0, 1.0f});
                    sd::polygon_convex_regular(renderer, 16.0f, vec3(poly.points[0].x + ((poly.points.back().x - poly.points[0].x) * lerp), y, 0.0f), 16);
                    
                    //}
                    {
                        sd::Drawable_Info* drawable = sd::end_polygon(renderer);
                        
                        vec3 half = vec3((max - min) / 2.0f, 0.0f);
                        drawable->set_transform(m::translate(mat4(1.0f), vec3(center) - vec3(half.x, 0, 0) + vec3(0, half.y, 0)));
                        
                    }
                }
                
                sd::set_color_rgba_v4(renderer, color);
            }
            
            for (auto it = world->dt.recorder.thing_records.begin(); it != world->dt.recorder.thing_records.end(); ++it) {
                
                mtt::Thing_ID thing_id = it->first;
                
                dt::Recording_Event_List events_for_thing = dt::Recording_Events_for_thing(&world->dt.recorder, thing_id);
                
                for (usize ev = events_for_thing.count - 1; ev < events_for_thing.count; ev += 1) {
                    
                    dt::Recording_Event* latest_event = (events_for_thing.events + ev);
                    
                    sd::begin_path(renderer);
                    
                    sd::set_color_rgba_v4(renderer, {0.25f, 0.0f, 0.75f, 1.0f});
                    for (usize i = 0; i < latest_event->states.size(); i += 1) {
                        sd::path_vertex_v3(renderer, latest_event->states[i].absolute_transform.translation);
                    }
                    
                    sd::break_path(renderer);
                    sd::set_color_rgba_v4(renderer, {0.7f, 0.7f, 0.7f, 1.0f});
                    
                    {
                        std::vector<dt::Recording_State>& _ = latest_event->states;
                        if (_.size() > 2) {
                            //#define POS(points, i) points[i].transform.position
#define POS(points, i) points[i]
                            
                            std::vector<vec3> path;
                            for (usize i = 0; i < latest_event->states.size(); i += 1) {
                                path.push_back(latest_event->states[i].absolute_transform.translation);
                            }
                            std::vector<vec3> _;
                            rdp::RamerDouglasPeucker(path, 45, _);
                            for (usize i = 0; i < _.size(); i += 1) {
                                sd::path_vertex_v3(renderer, _[i] + vec3(0.0, 0.0, 205.0));
                            }
                            
                            sd::break_path(renderer);
                            
                            CatmullRom spline;
                            spline.init(_, 10, false);
                            
                            sd::set_color_rgba_v4(renderer, {1.0, 0.0, 0.0, 1.0});
                            
                            //sd::begin_path(renderer);
                            spline.DrawSpline(renderer);
                            //sd::end_path(renderer);
                            
                        } else if (_.size() == 2) {
                            sd::path_vertex_v3(renderer, latest_event->states[0].absolute_transform.translation);
                            sd::path_vertex_v3(renderer, latest_event->states[1].absolute_transform.translation);
                        }
#undef POS
                    }
                    
                    sd::end_path(renderer);
                    
                    
                    
                    
                }
            }
        }
        
        sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME);
        
        if (thing_arrows.is_active) {
            {
                vec3 scale;
                quat orientation;
                vec3 translation;
                vec3 skew;
                vec4 perspective;
                
                m::decompose(world->cam.cam_transform, scale, orientation, translation, skew, perspective);
                
                thing_arrows.draw_arrows(renderer, scale.x);
            }
        }
        if (auto* a_l = mtt::arrow_links(&world->mtt_world); !a_l->empty() && mtt::should_show_verbose_display(&world->mtt_world)) {
            
            vec3 scale;
            quat orientation;
            vec3 translation;
            vec3 skew;
            vec4 perspective;
            
            m::decompose(world->cam.cam_transform, scale, orientation, translation, skew, perspective);
            
            
            auto* nvg_ctx = nvgGetGlobalContext();
            auto* vg = nvg_ctx;
            auto* dt = dt::DrawTalk::ctx();
            nvgSave(nvg_ctx);
            {
                using m::value_ptr;
                mat4 identity(1.0f);
                nvgSetViewTransform(nvg_ctx, value_ptr(identity));
                mat4 M = identity;
                nvgSetModelTransform(nvg_ctx, value_ptr(M));
                
                nvgFontSize(vg, 48.0f * 0.75f);
                nvgFontFace(vg, "sans");
                nvgTextAlign(vg, NVG_ALIGN_CENTER|NVG_ALIGN_TOP);
                nvgTextMetrics(vg, NULL, NULL, &font_settings.lineh);
                
                nvgFillColor(vg, nvgRGBAf(127.0f/255.0f,165.0f/255.0f,255.0f/255.0f,255.0f/255.0f));
            }
            
            auto old_color = sd::get_color_rgba(renderer);
            auto old_path_pixel_radius = sd::get_path_radius(renderer);
            sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD);
            sd::set_color_rgba_v4(renderer, vec4(0.8f, 0.8f, 0.8f, 1.0f));
            
            float32 p_scale = m::max(1.0f, scale.x);
            sd::set_path_radius(renderer, 4 * p_scale);
            auto* arrow_links = a_l;
            auto& arrow_edges = arrow_links->edges_forward;
            auto& arrow_edges_rev = arrow_links->edges_reverse;
            
            sd::begin_path(renderer);
            for (auto adj_it = arrow_edges.begin(); adj_it != arrow_edges.end(); ++adj_it) {
                mtt::Thing_ID src = adj_it->first;
                mtt::Arrow_Link_List& dst_list = adj_it->second;
                mtt::Thing* src_thing = mtt_world->Thing_try_get(src);
                if (src_thing == nullptr) {
                    continue;
                }
                mtt::Rep* src_rep = mtt::rep(src_thing);
                Collider* src_c = src_rep->colliders[0];
                Quad& src_q = src_c->aabb.saved_quad;
                
                
                const usize height_offset = 2;
                float32 y_offset = 0.0f;
                
                //                const Line_Segment src_segments[4] = {
                //                    {src_q.tl, src_q.bl},
                //                    {src_q.bl, src_q.br},
                //                    {src_q.br, src_q.tr},
                //                    {src_q.tr, src_q.tl}
                //                };
                
                vec3 src_pos = src_rep->transform.translation;
                Quad_As_Segments src_segments = Quad_segments(&src_q);
                
                
                for (auto edge_it = dst_list.begin(); edge_it != dst_list.end(); ++edge_it) {
                    if ((edge_it->flags & ARROW_LINK_FLAGS_VISIBLE) != ARROW_LINK_FLAGS_VISIBLE) {
                        continue;
                    }
                    
                    mtt::Thing_ID dst = mtt::arrow_get_thing_id(*edge_it);
                    mtt::Thing* dst_thing = mtt_world->Thing_try_get(dst);
                    if (dst_thing == nullptr) {
                        continue;
                    }
                    
                    mtt::Rep* dst_rep = mtt::rep(dst_thing);
                    
                    
                    vec3 dst_pos = dst_rep->transform.translation;
                    if (!dst_rep->colliders.empty()) {
                        Collider* dst_c = dst_rep->colliders[0];
                        Quad& dst_q = dst_c->aabb.saved_quad;
                        
                        mtt::Line_Segment s = {src_pos, dst_pos};
                        
                        mtt::Hit hit = {};
                        for (usize i = 0; i < 4; i += 1) {
                            if (mtt::Line_Segment_intersection(&s, &src_segments.segments[i], &hit)) {
                                src_pos = vec3(hit.pos, src_pos.z);
                                
                                Quad_As_Segments dst_segments = Quad_segments(&dst_q);
                                
                                mtt::Hit hit_sub = {};
                                for (usize j = 0; j < 4; j += 1) {
                                    if (mtt::Line_Segment_intersection(&s, &dst_segments.segments[j], &hit_sub)) {
                                        dst_pos = vec3(hit_sub.pos, dst_pos.z);
                                        break;
                                    }
                                }
                                
                                break;
                            }
                        }
                        
                        
                    }
                    
                    if ((edge_it->flags & ARROW_LINK_FLAGS_DIRECTED) == ARROW_LINK_FLAGS_DIRECTED) {
                        sd::path_bezier_arrow_head_with_tail(renderer, src_pos, vec3((src_pos.x + dst_pos.x) * 0.5f, src_pos.y, src_pos.z), vec3(dst_pos.x, (src_pos.y + dst_pos.y) * 0.5f, dst_pos.z), dst_pos, 12.0f * p_scale, 8);
                        
                    } else {
                        sd::path_vertex_v3(renderer, src_pos);
                        sd::path_vertex_v3(renderer, dst_pos);
//                        sd::path_bezier(renderer, src_pos, vec3((src_pos.x + dst_pos.x) * 0.5f, src_pos.y, src_pos.z), vec3(dst_pos.x, (src_pos.y + dst_pos.y) * 0.5f, dst_pos.z), dst_pos, 8);
                    }
                    sd::break_path(renderer);
                    
                    
                    
                    
                    auto& label = mtt::arrow_get_label(*edge_it);
                    if (label != "" && label != mtt::text_thing_empty_string) {
                        
                        vec4 canvas_space = world->cam.view_transform * vec4((src_pos + dst_pos) * vec3(0.5f, 0.5f, 1.0f), 1.0f);
                        
                        //                        cstring as_cstr = label.c_str();
                        //                        float bounds[4];
                        //                        nvgTextBounds(vg, 0, 0, as_cstr, as_cstr + label.size(), bounds);
                        
                        //                        float32 text_width = bounds[2] - bounds[0];
                        //                        canvas_space -= text_width * 0.5f;
                        
                        float32 width = nvgText(vg, canvas_space.x, canvas_space.y + y_offset, label.c_str(), NULL);
                        MTT_UNUSED(width);
                        
                        y_offset += font_settings.lineh + height_offset;
                    }
                }
            }
            sd::end_path(renderer);
            
            sd::set_color_rgba_v4(renderer, old_color);
            sd::set_path_radius(renderer, old_path_pixel_radius);
            
            nvgRestore(nvg_ctx);
        }
        
    }
    sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME);
    
    //    dt::Recorder* recorder = &world->sk.recorder;
    //
    //
    //    if (recorder->thing_records.size() > 0) {
    //        sd::begin_path(renderer);
    //        for (usize i = 0; i < recorder->thing_records.size(); i += 1) {
    //
    //        }
    //        sd::end_path(renderer);
    //    }
    {
        auto* things = mtt::Thing_collection(&world->mtt_world);
        
        sd::begin_polygon(renderer);
        
        //        if (test_power.is_valid) {
        //
        //            auto* vec = mtt::access<vec4>(Thing_from_ID(&world->mtt_world, test_power.power_thing), "vector");
        //            if (vec) {
        //                sd::set_color_rgba_v4(renderer, {1.0f, 0.0f, 0.0f, 1.0f});
        //
        //                sd::polygon_convex_regular(renderer, 16.0f, vec3(vec->x, vec->y, vec->z) + vec3(0.0f, 0.0f, 700.0f), 4);
        //
        //            }
        //        }
        
        for (auto it = things->begin(); it != things->end(); ++it) {
            Thing* thing = &it->second;
            
            if constexpr ((false)) {
                Field_Handle out = mtt::lookup(thing, "velocity");
                if (out) {
                    void* velocity = mtt::access(thing, out);
                    (*(vec3*)velocity).x = (float32)thing->id;
                }
                
                //        void* acceleration = mtt::access(thing, "acceleration")); = vec3(0.01f, 0.00f, 0.0f);
                
                auto* acceleration = (vec3*)mtt::access(thing, "acceleration", MTT_VECTOR3);
                if (acceleration) {
                    acceleration->x = 0.45f * thing->id;
                } else {
                    //MTT_error("ERROR: incorrect type access\n");
                }
            }
            
            auto result = world->dt.scn_ctx.selected_things.find(thing->id);
            if (result == world->dt.scn_ctx.selected_things.end()) {
                continue;
            }
            
            Collider* collider = mtt::rep(thing)->colliders[0];
            vec3 center = vec3((collider->aabb.tl + collider->aabb.br) / 2.0f, 0.0f);
            vec3 width = vec3((collider->aabb.br - collider->aabb.tl), 0.0f);
            
            if ((false)) {
                //                for (usize i = 0; i < thing->ports.out_ports.size(); i += 1) {
                //                    Spatial_Alignment* align = &thing->ports.out_port_alignment[i];
                //                    vec3 point = center + (width * align->relative_position) + vec3(0.0f, 0.0f, 400.0f);
                //
                //                    sd::set_color_rgba_v4(renderer, {1.0f, 0.0f, 0.0f, 1.0f});
                //                    sd::polygon_convex_regular(renderer, 16.0f, point, 4);
                //
                //                    //sd::set_color_rgba_v4(renderer, {1.0f, 1.0f, 1.0f, 1.0f});
                //                    //sd::rectangle(renderer, point + vec3(0.0f, -16.0f, 0.0f), vec3(64.0f,16.0f,0.0f), 405.0f);
                //                }
                //                for (usize i = 0; i < thing->ports.in_ports.size(); i += 1) {
                //                    Spatial_Alignment* align = &thing->ports.in_port_alignment[i];
                //                    vec3 point = center + (width * align->relative_position) + vec3(0.0f, 0.0f, 400.0f);
                //
                //                    sd::set_color_rgba_v4(renderer, {1.0f, 1.0f, 0.0f, 1.0f});
                //                    sd::polygon_convex_regular(renderer, 16.0f, point, 3);
                //
                //                    //sd::set_color_rgba_v4(renderer, {1.0f, 1.0f, 1.0f, 1.0f});
                //                    //sd::rectangle(renderer, point + vec3(0.0f, -16.0f, 0.0f), vec3(64.0f,16.0f,0.0f), 405.0f);
                //                }
            }
        }
        sd::end_polygon(renderer);
        
    }
    
    
    
    
    // MARK: Speech
    if constexpr((false)) {
        Speech_Event speech_event;
        while (Speech_poll_event(&core->speech_system.info_list[0], &speech_event)) {
            Speech_Info_View view = Speech_Info_ID_to_record(&core->speech_system.info_list[0], speech_event.ID);
            
            if (view.is_valid) {
                //MTT_print("POLL SPEECH BEGIN [%llu]{\n", view.ID);
                
                Text_Info_print(view.text_info);
                
                dt::push_speech_event(core, &world->mtt_world, &world->dt, &speech_event, &view);
                
                
                
                if (view.text_info->marked_final) {
                    
                }
                
                view.text_info->is_changed = false;
                
                //            MTT_print("timestamps{\n");
                //            for (auto it = view.nl_info->word_timestamps.begin(); it != view.nl_info->word_timestamps.end(); ++it) {
                //                MTT_print("%f\n", it->time);
                //            }
                //            MTT_print("timestamps_end}\n");
                
                //MTT_print("}\nPOLL SPEECH END [%llu]\n", view.ID);
            }
        }
    }
    Natural_Language_Event nl_event;
    while (Natural_Language_poll_event(&core->speech_system.info_list[0], &nl_event)) {
        
        Speech_Info_View view = Speech_Info_ID_to_record(&core->speech_system.info_list[0], nl_event.ID);
        if (!view.is_valid) {
            ASSERT_MSG(false, "%s\n", "why not valid?");
            continue;
        }
        
        //MTT_print("POLL NL BEGIN [%llu]{\n", view.ID);
        
        //Text_Info_print(view.text_info);
        //Natural_Language_Info_print(view.nl_info);
        //        MTT_print("checking speech/nlp: "
        //                  "spe is_changed %d "
        //                  "spe marked_final? %llu "
        //                  "spe modification_count %llu "
        //                  "nlp is_changed %d "
        //                  "nlp is_valid %llu "
        //                  "nlp modification_count %llu\n",
        //                  view.text_info->is_changed,
        //                  (uint64)view.text_info->marked_final,
        //                  view.text_info->modification_count,
        //                  view.nl_info->is_changed,
        //                  (uint64)view.nl_info->is_valid,
        //                  view.nl_info->modification_count);
        
        dt::push_language_event(core, &world->mtt_world, &world->dt, &nl_event, &view);
        
        MTT_user_test_log_enqueue(MTT_USER_TEST_LOG_LABEL_INPUT,
                                  STRLN("{")
                                  STRIDTLN(STRMK("type") ":" STRMK("language") ",")
                                  STRIDTLN(STRMK("contents") ":" + view.text_info->text + ",")
                                  STRIDTLN(STRMK("t") ":" + mtt::to_str(nl_event.timestamp)  + "")
                                  STRLN("},")
                                  );
        
        
        // reset flags to detect new changes after this round
        view.text_info->is_changed = false;
        view.nl_info->is_changed   = false;
        view.nl_info->is_valid     = false;
        
        
        MTT_print("}\nPOLL NL END [%llu]\n", view.ID);
    }
    
    dt::evaluate_completed_speech_events(&world->dt, core, &world->mtt_world);
    
    if (!Speech_get_active_state(&core->speech_system.info_list[0])) {
        Speech_clear_events(&core->speech_system.info_list[0]);
        Natural_Language_clear_events(&core->speech_system.info_list[0]);
    }
    
    dt::on_frame();
    
    //MTT_print("selection_recording count = [%zu]\n", world->dt.recorder.selection_recording.selections.size());
    
    
    //    if (world_vis.is_on) {
    //        if (world_vis.has_started) {
    //            world_vis.has_started = false;
    //            world_vis.t_start = m::floor(time);
    //            core->visualize(core, std::to_string(time));
    //        } else {
    //            if (time - world_vis.t_start >= world_vis.interval) {
    //                do {
    //                    world_vis.t_start += world_vis.interval;
    //                } while (time - world_vis.t_start >= world_vis.interval);
    //
    //                static const usize buf_size = 256;
    //                static char buf[buf_size];
    //                auto cx = snprintf(buf, buf_size, "%" MTT_number_thing_format_str, time);
    //                if (cx >= 0 && cx < buf_size) {
    //                    buf[cx] = '\0';
    //                    core->visualize(core, std::string(buf));
    //                    //MTT_print("%s\n", buf);
    //
    //                }
    //            }
    //
    //        }
    //    }
    
    sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD);
    
    
    vec3 d_scale;
    quat d_orientation;
    vec3 d_translation;
    vec3 d_skew;
    vec4 d_perspective;
    {
        m::decompose(world->cam.view_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
    }
    
    
    const float outline_radius = 2.0f * (1.0f / d_scale[0]);
    const float outline_radius_small = outline_radius * 0.5;
    sd::path_radius(renderer, outline_radius);
    
    // mini map
    sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER_PER_FRAME);
    
    {
        m::decompose(world->dt.ui.top_panel.cam.view_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
    }
    
    
    const float outline_radius_proxy = 2.0f * (1.0f / d_scale[0]);
    const float outline_radius_small_proxy = outline_radius_proxy * 0.5;
    sd::path_radius(renderer, outline_radius_proxy);
    //
    
    
    sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER_PER_FRAME);
    sd::begin_path(renderer);
    sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD);
    sd::begin_path(renderer);
    
    
    const uint32 radius_idx_canvas_thing = 0;
    const uint32 radius_idx_proxy_thing = 1;
    const uint32 radius_idx_normal = 0;
    const uint32 radius_idx_small = 1;
    const float32 radii_for_el[2][2] = {
        {outline_radius, outline_radius_small},
        {outline_radius_proxy, outline_radius_small_proxy},
    };
    
    
    constexpr const bool DEBUG_COLLIDERS = false;
    constexpr const bool DEBUG_UI_COLLIDERS = false;
    
    
    
    
    auto* things = mtt::Thing_collection(&world->mtt_world);
    static mtt::Set<mtt::Thing_ID> drawn_thing_colliders = {};
    drawn_thing_colliders.clear();
    drawn_thing_colliders.reserve(things->size());
    for (auto it = things->begin(); it != things->end(); ++it) {
        
        Thing* thing = &it->second;
        if (drawn_thing_colliders.contains(thing->id)) {
            continue;
        }
        bool should_draw_always = false;
        if (mtt::Thing_is_proxy(thing)) {
            auto* proxy = thing;
            if (dt::selection_is_recorded(proxy)) {
                should_draw_always = true;
            } else {
                continue;
            }
            thing = mtt::Thing_try_get(&world->mtt_world, thing->mapped_thing_id);
            if (thing == nullptr) {
                continue;
            }
            
        }
        drawn_thing_colliders.insert(thing->id);
        Representation* representation = mtt::rep(thing);
        
        
        if (representation->colliders.size() == 0) {
            continue;
        }
        
        Collider* c = representation->colliders[0];
                
        switch (c->type) {
            case COLLIDER_TYPE_AABB: {
                
                
                //            {
                //
                //                vec3 d_scale;
                //                quat d_orientation;
                //                vec3 d_translation;
                //                vec3 d_skew;
                //                vec4 d_perspective;
                //
                //                m::decompose(world->cam.cam_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                //
                //                sd::set_color_rgba_v4(renderer, {1.0f, 0.0f, 1.0f, 0.2f});
                //                sd::polygon_convex_regular(renderer, m::max(1.0f, 32.0f * d_scale.x), vec3(c->center_anchor.x, c->center_anchor.y, 1000.0f), 16);
                //            }
                
                
                if ((mtt::is_active(thing)) && (thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH ||
                                                thing->archetype_id == mtt::ARCHETYPE_TEXT ||
                                                thing->archetype_id == mtt::ARCHETYPE_NUMBER)) {
                    
                    
                    //Box box = calc_transformed_aabb(c);
                    
                    //                vec2 tl = box.tl;
                    //                vec2 br = box.br;
                    //
                    //                sd::vertex_v3(renderer, vec3(tl, -203.0f));
                    //                sd::vertex_v3(renderer, vec3(tl.x, br.y, -203.0f));
                    //                sd::vertex_v3(renderer, vec3(br, -203.0f));
                    //                sd::vertex_v3(renderer, vec3(br, -203.0f));
                    //                sd::vertex_v3(renderer, vec3(br.x, tl.y, -203.0f));
                    //                sd::vertex_v3(renderer, vec3(tl, -203.0f));
                    
                    
                    
                    //sd::set_color_rgba_v4(renderer, {0.0f, 1.0f, 0.1f, 0.2f});
                    //
                    
                    Quad& quad = c->aabb.saved_quad;
                    if (DEBUG_UI_COLLIDERS && c->system->label == "canvas") {
                        sd::set_color_rgba_v4(renderer, {0.0f, 0.0f, 1.0f, 0.5f});
                        
                        sd::path_quad(renderer,  world->cam.cam_transform * vec4(quad.tl, 0.0f, 1.0f), world->cam.cam_transform * vec4(quad.bl, 0.0f, 1.0f), world->cam.cam_transform * vec4(quad.br, 0.0f, 1.0f), world->cam.cam_transform * vec4(quad.tr, 0.0f, 1.0f), 999.0f);
                    }
                    else {
                        
                        if constexpr (DEBUG_COLLIDERS) {
                            sd::set_color_rgba_v4(renderer, {1.0f, 1.0f, 1.0f, 0.2f});
                            sd::path_quad(renderer, quad.tl, quad.bl, quad.br, quad.tr, 999.0f);
                        }
                        
                        Box& box = c->aabb.saved_box;
                        mtt::Thing_ID selected_drawable = world->dt.scn_ctx.thing_selected_drawable;
                        //auto it_words = world->dt.lang_ctx.dictionary.thing_to_word.find(thing->id);
                        if (
                            
                            //it_words != world->dt.lang_ctx.dictionary.thing_to_word.end() && it_words->second.size() >= 1 &&
                            
                            
                            
                            dt::selection_is_recorded(thing) || should_draw_always
                            //&& dt::get_confirm_phase() == dt::CONFIRM_PHASE_SPEECH
                            ) {
                                
                                float32 used_radius;
                                uint32 used_radius_idx = 0;
                                vec4 outline_color;
                                if (thing->id != selected_drawable) {
                                    used_radius = outline_radius_small;
                                    used_radius_idx = radius_idx_small;
                                    
                                    if (mtt::color_scheme() == mtt::COLOR_SCHEME_LIGHT_MODE) {
                                        outline_color = {0.01f, 0.01f, 0.01f, 1.0f};
                                    } else {
                                        outline_color = {0.0f, 0.6f, 0.0f, 1.0f};
                                    }
                                } else {
                                    used_radius = outline_radius;
                                    used_radius_idx = radius_idx_normal;
                                    outline_color = vec4(vec3(dt::UI_BORDER_SELECTION_COLOR) + vec3(0.2f), 1.0f);
                                }
                                sd::path_radius(renderer, used_radius);
                                sd::set_color_rgba_v4(renderer, outline_color);
                                
                                
                                
                                //                        else {
                                //                            sd::set_color_rgba_v4(renderer, dt::TEXT_PANEL_DEFAULT_COLOR);
                                //                        }
                                
                                
                                
                                //
                                
                                
                                
                                if constexpr (DEBUG_COLLIDERS) {
                                    vec3 scale;
                                    quat orientation;
                                    vec3 translation;
                                    vec3 skew;
                                    vec4 perspective;
                                    
                                    
                                    m::decompose(world->cam.cam_transform, scale, orientation, translation, skew, perspective);
                                    
                                    float32 off = 10.0f * scale.x;
                                    sd::path_quad(renderer, box.tl - vec2(off), vec2(box.tl.x - off, box.br.y + off), box.br + vec2(off), vec2(box.br.x + off, box.tl.y - off), 999.0f);
                                } else
                                {
                                    vec2 d0 = quad.tl - quad.br;
                                    vec2 d1 = quad.tr - quad.bl;
                                    
                                    if (d0 != m::vec2_zero() || d1 != m::vec2_zero()) {
                                        vec4 axes = m::normalize(vec4(quad.tl - quad.br, quad.tr - quad.bl)) * used_radius * 4.0f;
                                        if (world->dt.scn_ctx.thing_selected_with_pen != mtt::thing_id(thing)) {
                                            sd::path_quad(renderer,
                                                          quad.tl + vec2(axes[0], axes[1]),
                                                          quad.bl - vec2(axes[2], axes[3]),
                                                          quad.br - vec2(axes[0], axes[1]),
                                                          quad.tr + vec2(axes[2], axes[3]), 999.0f);
                                        }
                                    }
                                    
                                    {
                                        auto* proxies = mtt::Thing_proxies_try_get_for_scene(thing, DT_THING_PROXY_SCENE_IDX_SEMANTIC_DIAGRAM_VIEW);
                                        if (proxies != nullptr) {
                                            sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER_PER_FRAME);
                                            
                                            auto radius_to_use = radii_for_el[radius_idx_proxy_thing][used_radius_idx];
                                            sd::path_radius(renderer, radius_to_use);
                                            sd::set_color_rgba_v4(renderer, outline_color);
                                            
                                            for (auto proxy_id : *proxies) {
                                                sd::break_path(renderer);
                                                mtt::Thing* proxy = mtt::Thing_try_get(mtt_world, proxy_id);
                                                if (proxy == nullptr || !mtt::is_active(proxy)) {
                                                    continue;
                                                }
                                                auto* proxy_rep = mtt::rep(proxy);
                                                Collider* c = proxy_rep->colliders[0];
                                                Quad& quad = c->aabb.saved_quad;
                                                
                                                vec2 d0 = quad.tl - quad.br;
                                                vec2 d1 = quad.tr - quad.bl;
                                                if (d0 != m::vec2_zero() || d1 != m::vec2_zero()) {
                                                    vec4 axes = m::normalize(vec4(quad.tl - quad.br, quad.tr - quad.bl)) * radius_to_use * 4.0f;
                                                    
                                                    sd::path_quad(renderer,
                                                                  quad.tl + vec2(axes[0], axes[1]),
                                                                  quad.bl - vec2(axes[2], axes[3]),
                                                                  quad.br - vec2(axes[0], axes[1]),
                                                                  quad.tr + vec2(axes[2], axes[3]), 999.0f);
                                                }
                                                
                                            }
                                        }
                                        sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD);
                                    }
                                }
                                
                                
                            }
                        else if constexpr (DEBUG_COLLIDERS) {
                            sd::set_color_rgba_v4(renderer, {1.0f, 0.0f, 0.0f, 0.5f});
                            
                            sd::path_quad(renderer, box.tl, vec2(box.tl.x, box.br.y), box.br, vec2(box.br.x, box.tl.y), 999.0f);
                        }
                    }
                    
                }
                break;
            }
            default: { break; }
        }
        //MTT_print("Thing_ID %llu\n", it->second.instance_id);
        sd::break_path(renderer);
    }
    sd::set_render_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD);
    sd::end_path(renderer);
    sd::path_radius(renderer, 1.0f);
    sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER_PER_FRAME);
    sd::end_path(renderer);
    sd::path_radius(renderer, 1.0f);
    
    
    
    if constexpr (DEBUG_COLLIDERS) {
        sd::begin_polygon(renderer);
        for (auto c_it = debug_colliders.begin(); c_it != debug_colliders.end(); ++c_it) {
            Collider* c = *c_it;
            Quad& quad = c->aabb.saved_quad;
            Box& box = c->aabb.saved_box;
            
            sd::set_color_rgba_v4(renderer, {1.0f, 1.0f, 1.0f, 0.2f});
            
            sd::quad(renderer, quad.tl, quad.bl, quad.br, quad.tr, -200.0f);
            
            sd::set_color_rgba_v4(renderer, {1.0f, 0.0f, 0.0f, 0.5f});
            
            sd::quad(renderer, box.tl, vec2(box.tl.x, box.br.y), box.br, vec2(box.br.x, box.tl.y), -220.0f);
        }
        sd::end_polygon(renderer);
    }
    
    
    // MARK: Renderer command lists
    
    sd::clear_commands(renderer);
    
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        {
            core->library("on_before_frame", &core->dl_ctx);
        }
#else
    ext::on_before_frame(&core->dl_ctx);
#endif
    
    sd::Command_List cmd_list = {};
    
    sd::Command_List_begin(renderer, &cmd_list);
    
    sd::render_pass_begin(renderer);
    
    sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_ENABLED);
    
    sd::push_projection_view_transform(renderer, world->main_projection, Mat4(1.0f));
    
    {
        sd::push_texture_pipeline(renderer);
        sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_DISABLED);
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_BACKGROUND_CANVAS);
        sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_ENABLED);
    }
    
    sd::push_color_hsv_pipeline(renderer);
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_STATIC_CANVAS_HSV);
    
    sd::push_color_pipeline(renderer);
    
    //sd::push_texture_pipeline(renderer);
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_STATIC_CANVAS);
    
    
    
    sd::push_texture_pipeline(renderer);
    
    sd::push_projection_view_transform(renderer, world->main_projection, world->cam.view_transform);
    
    //        for (auto it = world->mtt_world.archetype_drawables.begin(); it != world->mtt_world.archetype_drawables.end(); ++it) {
    //            Drawable_Instance_Info& info = it->second;
    //            if (info.array.empty()) {
    //                continue;
    //            }
    //
    //            sd::push_instanced_draw_command_with_drawable_list(renderer, info.source, info.array);
    //        }
    //        world->mtt_world.instancing.push_draw_commands(renderer);
    //sd::push_color_pipeline(renderer);

    
    
    
    
    
    //sd::push_texture_pipeline(renderer);
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_STATIC);
    
    
    {
        auto& scissor_bounds = dt::DrawTalk::ctx()->ui.top_panel.base.bounds;
        sd::push_scissor_rect(renderer, (sd::Scissor_Rect) {
            .x = (uint64)0,
            .y = (uint64)(scissor_bounds.dimensions.y),
            .width = (uint64)core->viewport.width,
            .height = (uint64)(core->viewport.height - scissor_bounds.dimensions.y),
        });
        
        if constexpr ((TEST_PARTICLES_AND_NODE_GRAPH)) {
            for (Thing_ID id : particle_systems) {
                mtt::Thing* thing = mtt::Thing_try_get(&world->mtt_world, id);
                
                mtt::access_array<Particle_System_State>(thing, "particle_state_list", [&](auto& array) {
                    if (array.empty()) {
                        return;
                    }
                    
                    auto* rep = mtt::rep(test_particles.particle_system);
                    auto* d = rep->render_data.drawable_info_list[0];
                    
                    auto& dr_list = **mtt::access_pointer_to_pointer<mtt::Dynamic_Array<sd::Drawable_Info>*>(thing, "particle_drawable_list");
                    sd::push_instanced_draw_command_with_drawable_list(renderer, d, dr_list);
                });
            }
        }
        
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DYNAMIC);
    }
    
    {
        // MARK: draw proxies
        
        usize proxy_scene_count = Thing_Proxy_Scene_count(mtt_world);
        for (usize i = 0; i < proxy_scene_count; i += 1) {
            Thing_Proxy_Scene* proxy_scene = Thing_Proxy_Scene_for_idx(mtt_world, i);
            Thing_Proxy_Scene_prepare_for_render(proxy_scene);
        }
        
        auto proxy_draw = [&](Thing_Proxy_Scene* proxy_scene) {
            // MARK: current way to get data from source for proxies
            auto& proxies = proxy_scene->thing_to_proxy_map;
            auto& thing_drawable_proxy = proxy_scene->proxy_aggregate;
            
            usize t_i = 0;
            for (const auto& [thing_id, proxy_list] : proxies) {
                mtt::Thing* src_thing = mtt::Thing_try_get(mtt_world, thing_id);
                if (src_thing == nullptr) {
                    t_i += 1;
                    continue;
                }
                mtt::Rep* src_rep = mtt::rep(src_thing);
                
                auto& buf = thing_drawable_proxy[t_i];
                buf.clear();
                buf.resize(src_rep->render_data.drawable_info_list.size());
                
                for (const auto& proxy_id : proxy_list) {
                    mtt::Thing* proxy_thing = mtt::Thing_try_get(mtt_world, proxy_id);
                    {
                        mtt::Rep* proxy_rep = mtt::rep(proxy_thing);
#ifndef NDEBUG
                        if (src_rep->render_data.drawable_info_list.size() > proxy_rep->render_data.drawable_info_list.size()) {
                            
                            MTT_print("size mismatch source_size:%lu proxy_size:%lu\n", src_rep->render_data.drawable_info_list.size(), proxy_rep->render_data.drawable_info_list.size());
                        }
#endif
                        usize bounds = m::min(proxy_rep->render_data.drawable_info_list.size(), src_rep->render_data.drawable_info_list.size());
                        for (usize i = 0; i < bounds; i += 1) {
                            buf[i].push_back(*proxy_rep->render_data.drawable_info_list[i]);
                        }
                    }
                }
                for (usize drawable_i = 0; drawable_i < buf.size(); drawable_i += 1) {
                    mtt::Array_Slice<sd::Drawable_Info> list = to_Array_Slice(buf[drawable_i]);
                    sd::push_instanced_draw_command_with_drawable_list(renderer, src_rep->render_data.drawable_info_list[drawable_i], list);
                }
                
                t_i += 1;
            }
        };
        
        auto& scissor_bounds = dt::DrawTalk::ctx()->ui.top_panel.base.bounds;
        sd::push_scissor_rect(renderer, (sd::Scissor_Rect) {
            .x = (uint64)scissor_bounds.tl.x,
            .y = (uint64)scissor_bounds.tl.y,
            .width = (uint64)scissor_bounds.dimensions.x,
            .height = (uint64)scissor_bounds.dimensions.y,
        });
        
        sd::push_projection_view_transform(renderer, world->main_projection, world->dt.ui.top_panel.cam.view_transform);
        
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER_PER_FRAME);
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER);
        
        proxy_draw(Thing_Proxy_Scene_for_idx(mtt_world, DT_THING_PROXY_SCENE_IDX_SEMANTIC_DIAGRAM_VIEW));
        
        // TODO: do this hack
        //sd::push_draw_command_with_info_layer_and_buffer_layer(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER, LAYER_LABEL_DYNAMIC);
        sd::push_scissor_rect(renderer, sd::Viewport_to_Scissor_Rectangle(core->viewport));
        
        sd::push_projection_view_transform(renderer, world->main_projection, mat4(1.0f));
        
        proxy_draw(Thing_Proxy_Scene_for_idx(mtt_world, DT_THING_PROXY_SCENE_IDX_CONTEXT_PANEL_VIEW));
    }
    
    sd::push_projection_view_transform(renderer, world->main_projection, world->cam.view_transform);
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DRAWING);
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_PER_FRAME);
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_PER_FRAME_TEMP);
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DEBUG_STATIC);
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DEBUG_DYNAMIC_CONNECTIONS);
    
    //sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DRAWING_JUST_A_TEST);
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_COLLISION_DEBUG);
    
//        {
//            sd::save(renderer);
//            dt::UI_draw_post();
//            sd::restore(renderer);
//        }
    

    
    //sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_ENABLED);
    
    //sd::push_draw_command_with_layer(renderer, LAYER_LABEL_INSTANCES);
    
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DEBUG_DYNAMIC);
    
    
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_PER_FRAME_WORLD);
    
    
    sd::push_projection_view_transform(renderer, world->main_projection, Mat4(1.0f));
    
    
    
    //
    
    //sd::push_pipeline(renderer, sd::PIPELINE_TEXT);
    //
    //sd::push_pipeline(renderer, sd::PIPELINE_TEXTURE);
    
    //sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_CANVAS);
    sd::set_render_layer(renderer, LAYER_LABEL_SHADOW_IMAGES);
    sd::rewind_layer(renderer, LAYER_LABEL_SHADOW_IMAGES);
#define FILL_LABEL_TEXT (0)
    
    {
        float32 time_seconds = time;
        
        {
            vec3 scale;
            quat orientation;
            vec3 translation;
            vec3 skew;
            vec4 perspective;
            
            
            m::decompose(world->cam.cam_transform, scale, orientation, translation, skew, perspective);
            
            float32 label_size = 48.0f;// = m::clamp(scale.x * 48.0f, 0.01f, 450.0f);
            
            
            
            auto* nvg_ctx = nvgGetGlobalContext();
            
            auto* vg = nvg_ctx;
            auto* dt = dt::DrawTalk::ctx();
            nvgSave(nvg_ctx);
            {
                using m::value_ptr;
                
                mat4 identity(1.0f);
                //
                //
                //
                //nvgSetViewTransform(nvg_ctx, value_ptr(world->cam.view_transform));
                nvgSetViewTransform(nvg_ctx, value_ptr(identity));
                
                
                //
                mat4 M = identity;
                //
                //NVG_save_model_transform(nvg_ctx, value_ptr(M));
                
                nvgSetModelTransform(nvg_ctx, value_ptr(M));
                
                {
                    
                }
                
                // nvgSetViewTransform(nvg_ctx, value_ptr(identity));
                bool something_was_removed = false;
                if (mtt::should_show_verbose_display(&world->mtt_world)) {
                    auto& named_things = dt->lang_ctx.dictionary.thing_to_word;
                    for (auto it = named_things.begin(); it != named_things.end(); ++it) {
                        mtt::Thing_ID thing_id = it->first;
                        
                        mtt::Thing* thing = nullptr;
                        if (!world->mtt_world.Thing_try_get(thing_id, &thing)) {
                            continue;
                        }
                        if (mtt::Thing_is_proxy(thing) || mtt::Thing_is_reserved(thing)) {
                            continue;
                        }
                        
                        mtt::Rep* rep;
                        mtt::rep(thing, &rep);
                        //                std::cout << entry->name << std::endl;
                        //                int BPPPPP = 1;
                        
                        if (rep->colliders.empty()) {
                            continue;
                        }
                        
                        Collider* col = rep->colliders.back();
                        
                        auto& words = it->second;
                        
                        const usize height_offset = 2;
                        //                            vec2 anchor = vec2(col->aabb.saved_quad.tl.x, col->aabb.saved_quad.br.y);
                        //                            vec4 canvas_space = world->cam.view_transform * vec4(anchor, 0.0f, 1.0f);
                        AABB& box = col->aabb;
                        vec2 coords[] = {
                            vec2(world->cam.view_transform * vec4(box.tl.x, box.tl.y, 0.0f, 1.0f)),
                            vec2(world->cam.view_transform * vec4(box.br.x, box.br.y, 0.0f, 1.0f)),
                            vec2(world->cam.view_transform * vec4(box.tl.x, box.br.y, 0.0f, 1.0f)),
                            vec2(world->cam.view_transform * vec4(box.br.x, box.tl.y, 0.0f, 1.0f)),
                        };
                    
                        
                        vec2 canvas_space = coords[0];
                        for (usize ci = 1; ci < 4; ci += 1) {
                            if (coords[ci].y > canvas_space.y) {
                                canvas_space.y = coords[ci].y;
                            }
                            if (coords[ci].x < canvas_space.x) {
                                canvas_space.x = coords[ci].x;
                            }
                        }

                        
                        if (!words.empty()) {
                            nvgFontSize(vg, 48.0f * 0.75f);
                            nvgFontFace(vg, "sans");
                            nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
                            nvgTextMetrics(vg, NULL, NULL, &font_settings.lineh);
                            
                            nvgFillColor(vg, nvgRGBAf(dt::LABEL_COLOR_THING_INSTANCE.r, dt::LABEL_COLOR_THING_INSTANCE.g, dt::LABEL_COLOR_THING_INSTANCE.b, dt::LABEL_COLOR_THING_INSTANCE.a));
                        }
                        const bool should_show_debug_display_enabled = mtt::should_show_debug_display(&world->mtt_world);
                        for (auto w = words.begin(); w != words.end(); ++w) {
                            dt::Word_Dictionary_Entry* entry = (*w);
                            if (entry->should_not_visualize) {
                                continue;
                            }
                            
                            if (entry->name == "thing" && (words.size() > 1 || !should_show_debug_display_enabled)) {
                                continue;
                            }
                            
                            //cstring name = (entry->naarrowme.c_str();
#ifndef NDEBUG
                            const mtt::String cpp_string = ((entry->name) + (should_show_debug_display_enabled ? (":" + std::to_string(thing_id)) : ""));
#else
                            const mtt::String cpp_string = ((entry->name));
#endif
                            cstring name = cpp_string.c_str();
                            //usize len = entry->name.size();
                            
                            float bounds[4];
                            float32 width = nvgTextBounds(vg, canvas_space.x, canvas_space.y, name, NULL, bounds);
                            
                            nvgText(vg, canvas_space.x, canvas_space.y, name, NULL);
                            
#if FILL_LABEL_TEXT
                        
                        
                        sd::begin_polygon(renderer);
                            
//                        sd::set_color_rgba_v4(renderer, color::BLUE);
//                        sd::rectangle(renderer, vec2(canvas_space), vec2(canvas_space.x + 128, canvas_space.y + 128), 900);
                            sd::set_color_rgba_v4(renderer, color::BLACK);
                            sd::rectangle(renderer, vec2(canvas_space.x, canvas_space.y), vec2(width, font_settings.lineh), 900);
                        
                        sd::end_polygon(renderer);
#endif
                            
                            if (world->dt.scn_ctx.long_press_this_frame && !something_was_removed) {
                                if (world->dt.scn_ctx.long_press_position.x > canvas_space.x && world->dt.scn_ctx.long_press_position.y > canvas_space.y && world->dt.scn_ctx.long_press_position.x < (canvas_space.x + width) && world->dt.scn_ctx.long_press_position.y < (canvas_space.y + font_settings.lineh)) {
                                    dt::vis_word_underive_from(thing, entry);
                                    something_was_removed = true;
                                }
                                
                            }
                            
                            canvas_space.y += font_settings.lineh + height_offset;
                            
                        }
                        
                        
                        
                        
                        auto attribs = get_word_attributes(thing);
                        
                        if (!attribs.empty()) {
                            nvgFontSize(vg, 48.0f * 0.4f);
                            nvgFontFace(vg, "sans");
                            nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
                            nvgTextMetrics(vg, NULL, NULL, &font_settings.lineh);
                            
                            
                            nvgFillColor(vg, nvgRGBAf(dt::LABEL_COLOR_ATTRIBUTE.r,dt::LABEL_COLOR_ATTRIBUTE.g,dt::LABEL_COLOR_ATTRIBUTE.b,dt::LABEL_COLOR_ATTRIBUTE.a));
                            
                            for (auto w = attribs.begin(); w != attribs.end(); ++w) {
                                auto ent_name = (*w).attrib.name();
                                cstring name = ent_name.c_str();
                                usize len = ent_name.size();
                                
                                nvgText(vg, canvas_space.x, canvas_space.y, name, name + len);
                                canvas_space.y += font_settings.lineh + height_offset;
                            }
                        }
                        
                        auto* own_attribs = dt::Thing_get_own_attributes(thing);
                        if (!own_attribs->empty()) {
                            nvgFontSize(vg, 48.0f * 0.4f);
                            nvgFontFace(vg, "sans");
                            nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
                            nvgTextMetrics(vg, NULL, NULL, &font_settings.lineh);
                            
                            
                            
                            
                            for (auto w = own_attribs->begin(); w != own_attribs->end(); ++w) {
                                auto ent_name = (*w)->typename_desc.name();
                                cstring name = ent_name.c_str();
                                usize len = ent_name.size();
                                
                                nvgFillColor(vg, nvgRGBAf(dt::LABEL_COLOR_ATTRIBUTE_MODIFIER.r,dt::LABEL_COLOR_ATTRIBUTE_MODIFIER.g,dt::LABEL_COLOR_ATTRIBUTE_MODIFIER.b,dt::LABEL_COLOR_ATTRIBUTE_MODIFIER.a));
                                
                                float bounds[4];
                                float32 width = nvgTextBounds(vg, canvas_space.x, canvas_space.y, name, name + len, bounds);
                                nvgText(vg, canvas_space.x, canvas_space.y, name, name + len);
                                
                                bool is_removed = false;
                                if (world->dt.scn_ctx.long_press_this_frame && !something_was_removed) {
                                    if (world->dt.scn_ctx.long_press_position.x > canvas_space.x && world->dt.scn_ctx.long_press_position.y > canvas_space.y && world->dt.scn_ctx.long_press_position.x < (canvas_space.x + width) && world->dt.scn_ctx.long_press_position.y < (canvas_space.y + font_settings.lineh)) {
                                        dt::Thing_remove_attribute(thing, *w);
                                        is_removed = true;
                                        something_was_removed = true;
                                    }
                                }
                                
                                canvas_space.y += font_settings.lineh + height_offset;
                                
                                if (!is_removed) {
                                    nvgFillColor(vg, nvgRGBAf(dt::LABEL_COLOR_VALUE_ATTRIBUTE.r,dt::LABEL_COLOR_VALUE_ATTRIBUTE.g,dt::LABEL_COLOR_VALUE_ATTRIBUTE.b,dt::LABEL_COLOR_VALUE_ATTRIBUTE.a));
                                    struct To_Remove {
                                        mtt::Thing* self_thing;
                                        dt::Word_Dictionary_Entry* src_entry = nullptr;
                                        dt::Word_Dictionary_Entry* prop = nullptr;
                                        bool should_remove = false;
                                    } to_remove;
                                    mtt::Thing_get_properties_for_attribute(thing, *w, [&](mtt::Thing* self_thing, dt::Word_Dictionary_Entry* src_entry, dt::Word_Dictionary_Entry* prop, const mtt::Any* value) {
                                        
                                        if (value->type == mtt::MTT_FLOAT) {
                                            auto p_ent_name = prop->typename_desc.name();
                                            
#ifndef NDEBUG
                                            if (mtt::should_show_debug_display(mtt::world(self_thing))) {
                                                char buffer[64];
                                                isize len = snprintf(buffer, 64, "%f", value->Float);
                                                len -= 1;
                                                while (len >= 0 && buffer[len] != '.' && buffer[len] == '0') {
                                                    buffer[len] = '\0';
                                                    len -= 1;
                                                }
                                                
                                                mtt::String with_val = mtt::String(p_ent_name) + " : " + mtt::String(buffer);
                                                cstring p_name = with_val.c_str();
                                                usize p_len = with_val.size();
                                                
                                                float bounds[4];
                                                float32 p_width = nvgTextBounds(vg, canvas_space.x + 24, canvas_space.y, p_name, p_name + + p_len, bounds);
                                                nvgText(vg, canvas_space.x + 24, canvas_space.y, p_name, p_name + p_len);
                                                MTT_UNUSED(p_width);
                                                
                                                {
                                                    if (world->dt.scn_ctx.long_press_this_frame && !something_was_removed) {
                                                        if (world->dt.scn_ctx.long_press_position.x > canvas_space.x + 24 && world->dt.scn_ctx.long_press_position.y > canvas_space.y && world->dt.scn_ctx.long_press_position.x < (canvas_space.x + width) && world->dt.scn_ctx.long_press_position.y < (canvas_space.y + font_settings.lineh)) {
                                                            //dt::Thing_remove_attribute(thing, *w);
                                                            to_remove.self_thing = thing;
                                                            to_remove.src_entry = src_entry;
                                                            to_remove.prop = prop;
                                                            to_remove.should_remove = true;
                                                            
                                                            is_removed = true;
                                                            something_was_removed = true;
                                                        }
                                                    }
                                                }
                                            } else
#endif
                                            {
                                                cstring p_name = p_ent_name;
                                                mtt::String as_str = mtt::String(p_name);
                                                usize p_len = as_str.size();
                                                
                                                float bounds[4];
                                                float32 p_width = nvgTextBounds(vg, canvas_space.x + 24, canvas_space.y, p_name, p_name + + p_len, bounds);
                                                nvgText(vg, canvas_space.x + 24, canvas_space.y, p_name, p_name + p_len);
                                                MTT_UNUSED(p_width);
                                                
                                                {
                                                    if (world->dt.scn_ctx.long_press_this_frame && !something_was_removed) {
                                                        if (world->dt.scn_ctx.long_press_position.x > canvas_space.x + 24 && world->dt.scn_ctx.long_press_position.y > canvas_space.y && world->dt.scn_ctx.long_press_position.x < (canvas_space.x + width) && world->dt.scn_ctx.long_press_position.y < (canvas_space.y + font_settings.lineh)) {
                                                            //dt::Thing_remove_attribute(thing, *w);
                                                            to_remove.self_thing = thing;
                                                            to_remove.src_entry = src_entry;
                                                            to_remove.prop = prop;
                                                            to_remove.should_remove = true;
                                                            
                                                            is_removed = true;
                                                            something_was_removed = true;
                                                        }
                                                    }
                                                }
                                            }
                                            
                                            
                                        } else {
                                            auto p_ent_name = prop->typename_desc.name();
                                            cstring p_name = p_ent_name.c_str();
                                            usize p_len = p_ent_name.size();
                                            
                                            float bounds[4];
                                            float32 p_width = nvgTextBounds(vg, canvas_space.x + 24, canvas_space.y, p_name, p_name + + p_len, bounds);
                                            nvgText(vg, canvas_space.x + 24, canvas_space.y, p_name, p_name + p_len);
                                            
                                            {
                                                if (world->dt.scn_ctx.long_press_this_frame && !something_was_removed) {
                                                    if (world->dt.scn_ctx.long_press_position.x > canvas_space.x + 24 && world->dt.scn_ctx.long_press_position.y > canvas_space.y && world->dt.scn_ctx.long_press_position.x < (canvas_space.x + width) && world->dt.scn_ctx.long_press_position.y < (canvas_space.y + font_settings.lineh)) {
                                                        
                                                        
                                                        to_remove.self_thing = thing;
                                                        to_remove.src_entry = src_entry;
                                                        to_remove.prop = prop;
                                                        to_remove.should_remove = true;
                                                        
                                                        is_removed = true;
                                                        something_was_removed = true;
                                                    }
                                                }
                                            }
                                        }
                                        
                                        // TODO: remove?
                                        
                                        canvas_space.y += font_settings.lineh + height_offset;
                                    });
                                    
                                    if (to_remove.should_remove) {
                                        dt::Thing_remove_attribute_property(to_remove.self_thing, to_remove.src_entry, to_remove.prop);
                                        to_remove.should_remove = false;
                                    }
                                }
                            }
                        }
                        
                        //                            nvgFillColor(vg, nvgRGBA(127,165,255,255));
                        //                            dt::Thing_get_relations(mtt::world(thing), mtt::id(thing), [&](mtt::Thing* src, const dt::Word_Dictionary::Active_Relation* relation) {
                        //                                mtt::Thing* target = mtt::Thing_try_get(mtt::world(src), relation->target);
                        //                                if (target == nullptr) {
                        //                                    return;
                        //                                }
                        //
                        //                                auto rel_name = relation->dict_entry->typename_desc.name();
                        //
                        //                                auto find = named_things.find(target->id);
                        //                                if (find != named_things.end()) {
                        //                                    for (auto r_it = find->second.begin(); r_it != find->second.end(); ++r_it) {
                        //                                        auto* target_entry = (*r_it);
                        //                                        if (target_entry->should_not_visualize) {
                        //                                            continue;
                        //                                        }
                        //                                        auto target_name = target_entry->get_query_name();
                        //                                        if (target_name == "thing") {
                        //                                            continue;
                        //                                        }
                        //
                        //                                        mtt::String msg = mtt::String(rel_name) + " : " + mtt::String(target_name);
                        //                                        usize len = msg.size();
                        //                                        cstring msg_as_cstr = msg.c_str();
                        //
                        //                                        float32 width = nvgText(vg, canvas_space.x, canvas_space.y, msg_as_cstr, msg_as_cstr + len);
                        //
                        //                                        if (long_press_this_frame) {
                        //                                            if (long_press_position.x > canvas_space.x && long_press_position.y > canvas_space.y && long_press_position.x < (canvas_space.x + width) && long_press_position.y < (canvas_space.y + font_settings.lineh)) {
                        //                                                dt::Thing_remove_relation(src, relation->dict_entry, target);
                        //                                            }
                        //                                        }
                        //
                        //
                        //                                        canvas_space.y += font_settings.lineh + height_offset;
                        //                                    }
                        //                                } else {
                        //                                    cstring msg_as_cstr = rel_name.c_str();
                        //                                    usize len = rel_name.size();
                        //
                        //                                    float32 width = nvgText(vg, canvas_space.x, canvas_space.y, msg_as_cstr, msg_as_cstr + len);
                        //
                        //                                    if (long_press_this_frame) {
                        //                                        if (long_press_position.x > canvas_space.x && long_press_position.y > canvas_space.y && long_press_position.x < (canvas_space.x + width) && long_press_position.y < (canvas_space.y + font_settings.lineh)) {
                        //                                            dt::Thing_remove_relation(src, relation->dict_entry, target);
                        //                                        }
                        //                                    }
                        //
                        //                                    canvas_space.y += font_settings.lineh + height_offset;
                        //                                }
                        //                            });
                    }
                }
                
                
                
                //NVG_restore_model_transform(nvg_ctx, value_ptr(M));
                
            }
            nvgRestore(nvg_ctx);
        }
        
        {
            
            
            //        sd::set_color_rgba_v4(core->renderer, ndom_panel.text.color);
            //        sd::begin_polygon(core->renderer);
            //            auto* col = first_collider(rep);
            //            auto& aabb = col->aabb;
            //            vec2 offset = vec2(0.0f);
            //            sd::rectangle(core->renderer, aabb.tl + offset, (aabb.br - aabb.tl) * vec2(4.0f, 1.0f), 0.0f);
            //        auto* info = sd::end_polygon(core->renderer);
            //info->set_transform(rep->model_transform * rep->pose_transform);
            
            
#define V2_UI_READY (0)
#if V2_UI_READY
            sd::set_render_layer(core->renderer, LAYER_LABEL_DYNAMIC_CANVAS);
            dt::UI& ui = dt::DrawTalk::ctx()->ui;
            auto& base = ui.bottom_panel.base;
            sd::save(core->renderer);
            sd::set_color_rgba_v4(core->renderer, color::EARTHY_1);
            sd::begin_polygon(core->renderer); {
                sd::rectangle(core->renderer, base.bounds.tl, vec2(base.bounds.dimensions.x, 1.0), 999.0f);
            }
            auto* info = sd::end_polygon(core->renderer);
            sd::restore(core->renderer);
#endif
            
        }
        
        {
            auto debug_print = [](mtt::String msg) {
                auto* vg = nvgGetGlobalContext();
                nvgFontSize(vg, 16.0f);
                nvgFontFace(vg, "sans");
                nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
                float lineh = 0;
                nvgTextMetrics(vg, NULL, NULL, &lineh);
                
                
                nvgFillColor(vg, nvgRGBA(255,165,255,255));
                cstring c_msg = msg.c_str();
                
                //float bounds[4];
                //nvgTextBoxBounds(vg, mtt_core_ctx()->viewport.width / 2.0f, 5, mtt_core_ctx()->viewport.width / 2.0f, c_msg, c_msg + msg.size(), bounds);
                
                auto width = mtt_core_ctx()->viewport.width;
                nvgTextBox(vg, width / 4, 64, width - 4, c_msg, NULL);
                
                
            };
            if (!dt::debug_msg.empty()) {
                //debug_print(dt::debug_msg);
            }
        }
        
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_CANVAS_PER_FRAME);
        
//        for (usize i = 0; i < nodes_as_ids.size(); i += 1) {
//            mtt::Thing_ID thing_id = nodes_as_ids[i];
//            mtt::Thing* thing = mtt::Thing_try_get(mtt_world, thing_id);
//            if (thing == nullptr) {
//                continue;
//            }
//
//            auto* rep = mtt::rep(thing);
//            rep->render_data.drawable_info_list[0]->set_texture_coordinates_modifiers(SD_vec4(SD_vec2(1.0f + 16*m::sin01((float32)time_seconds)), SD_vec2(-0.5*(16*m::sin01((float32)time_seconds)))));
//        }
        
        
        
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_PER_FRAME);
        
        
        

        
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_UI);
        
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_LINE_TEST);

#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        {
            sd::save(renderer);
            core->library("on_frame", &core->dl_ctx);
            sd::restore(renderer);
        }
#else
        ext::on_frame(&core->dl_ctx);
#endif

        
        sd::push_projection_view_transform(renderer, world->main_projection, Mat4(1.0f));
        
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_SHADOW_IMAGES);
        
        sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DYNAMIC_CANVAS);
        
        sd::push_pipeline(renderer, sd::PIPELINE_MODE_NONE);
        sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_DISABLED);
        
        sd::push_custom_command(renderer, {
            .data = (void*)core,
            .state = static_cast<void*>(world),
            .handler = [](auto* state, auto* data) -> void {
                
                MTT_Core* core = static_cast<MTT_Core*>(data);
                DrawTalk_World* const world = static_cast<DrawTalk_World*>(state);
                float32 time_seconds = core->time_seconds;
                MTT_UNUSED(time_seconds);
                
                
                
                auto* nvg_ctx = nvgGetGlobalContext();
                
                
                nvgSetupDraws(nvg_ctx);
                nvgDraw(nvg_ctx);
                nvgFlushOnly(nvg_ctx);
                nvgEndFrameOnly(nvg_ctx);
                
            },
        });
        sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_ENABLED);
        sd::push_color_pipeline(renderer);
    }
    
    
    sd::push_projection_view_transform(renderer, world->main_projection, world->cam.view_transform);
    
    sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_DISABLED);
    
    
    
    sd::push_draw_command_with_layer(renderer, LAYER_LABEL_DRAWTALK_DYNAMIC_PER_FRAME_UI);
    //
    //    sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_ENABLED);
    
    {
        sd::set_depth_write_mode(renderer, sd::DEPTH_WRITE_MODE_DISABLED);
        
        sd::push_projection_view_transform(renderer, world->main_projection, Mat4(1.0f));


        
        auto& indicators = instanced_indicators;
        for (usize i = 0; i < indicators.count; i += 1) {
            auto& info = indicators.render_info_list[i];
            info.instances.clear();
            info.instances.reserve(info.things.size());
            
            for (usize i_el = 0; const auto& el : info.things) {
                mtt::Thing* thing = mtt::Thing_try_get(&world->mtt_world, el);
                if (thing == nullptr) {
                    i_el += 1;
                    continue;
                }
                
                vec2 corner_multiplier = info.corner_multiplier;
                MTT_UNUSED(corner_multiplier);
                
                auto* representation = mtt::rep(thing);
                Collider* c = representation->colliders[0];
                Quad& quad = c->aabb.saved_quad;
                auto pt = quad.tr;
                
                constexpr const float32 scale = 24.0f;
                
                vec2 transformed_pt = world->cam.view_transform * vec4(pt, 0.0f, 1.0f);
                const auto radius = (core->viewport.width/scale) * 4.0f;
                
                // cull
                if (transformed_pt.x < core->viewport.x - radius || transformed_pt.x > core->viewport.width + radius ||
                    transformed_pt.y < core->viewport.y - radius || transformed_pt.y > core->viewport.height + radius) {
                    i_el += 1;
                    continue;
                }
                
                sd::Drawable_Info di = {};
//                di.set_transform(m::translate(Mat4(1.0f), vec3(vec2(core->viewport.width / 2.0f), 0.0f) + vec3((float32)i_el, (float32)i_el, -999.0f)) * m::scale(Mat4(1.0f), vec3(vec2(core->viewport.width/64.0f), 1.0f)));
                
                
                di.set_transform(m::translate(Mat4(1.0f), vec3(transformed_pt, -999.9999f)) * m::rotate(Mat4(1.0f), 8.0f * (float32)world->mtt_world.time_seconds, vec3(0.0f, 0.0f, 1.0f)) * m::scale(Mat4(1.0f), vec3(m::max(vec2(1.0f), vec2((core->viewport.width)/scale)), 1.0f)));
                info.instances.push_back(di);
                
                i_el += 1;
            }
            
            sd::push_instanced_draw_command_with_drawable_list(renderer, info.drawable_source, mtt::to_Array_Slice(info.instances));
        }
    }
    
    sd::render_pass_end(renderer);
    sd::Command_List_end(renderer, &cmd_list);
    
    sd::Command_List_submit(renderer, &cmd_list);
    
    
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        {
            core->library("on_after_frame", &core->dl_ctx);
        }
#else
    ext::on_after_frame(&core->dl_ctx);
#endif
    
    // MARK:- on_frame end
    mtt::frame_end(&world->mtt_world, true);
    
    
    //    for (int i = 0; i < test_arr.cap; i += 1) {
    //        MTT_print("ENTRY: %d\n", test_arr[i]);
    //    }
    
    
    //    ecs_world_stats_t stats;
    //
    //    ecs_get_world_stats(
    //     world->mtt_world.ecs_world.c_ptr(),
    //                                       &stats);
    //
    //
    //    ecs_dump_world_stats(world->mtt_world.ecs_world.c_ptr(), &stats);
    
    {
        auto* external_things = mtt::curr_external_world(&world->mtt_world);
        static mtt::String str_buf;
        str_buf.clear();
        str_buf.reserve(1024);
        str_buf += "{"\
        "\"type\":\"MTT_UPDATE\",\"vals\":[";
        
        bool something_found = false;
        for (auto& [thing_id, ext_thing] : *mtt::External_World_mappings(external_things)) {
            if (!mtt::External_Thing_flags_are_set(&ext_thing, mtt::EXTERNAL_THING_FLAG_UPDATE_PUSH)) {
                continue;
            }
            mtt::Thing* thing = mtt::Thing_try_get(&world->mtt_world, thing_id);
            if (thing != nullptr) {
                something_found = true;
                vec3 scale;
                quat orientation;
                vec3 translation;
                vec3 skew;
                vec4 perspective;
                
                m::decompose(mtt::rep(thing)->hierarchy_model_transform, scale, orientation, translation, skew, perspective);
                
                mtt::String px = std::to_string(translation.x);
                mtt::String py = std::to_string(translation.y);
                mtt::String pz = std::to_string(translation.z);
                
                
                str_buf += "{\"id\":";
                mtt::String id_as_str = std::to_string(thing->id);
                str_buf += id_as_str;
                str_buf += ",\"value\":";
                //std::copy(px.begin(), px.end(), std::back_inserter(buf));
                //std::copy(py.begin(), py.end(), std::back_inserter(buf));
                //std::copy(pz.begin(), pz.end(), std::back_inserter(buf));
                
                str_buf += "[" + px + "," + py + "," + pz + "]},";
                
            };
        }
        if (something_found) {
            str_buf.pop_back();
            
            str_buf += "]}\r\n";
            Extension_Server_write(&core->server_ext, str_buf.data(), str_buf.size());
        }
        
    }
}

void on_exit(MTT_Core* core)
{
    Extension_Server_stop(&core->server_ext);
    Extension_Server_deinit(&core->server_ext);
    
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    mtt::Video_Recording_stop(core->screen_recording);
    mtt::Video_Recording_Context_destroy(core->screen_recording);
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    {
        core->library("on_unload", &core->dl_ctx);
    }
#endif
    MTT_Dynamic_Library_close(&core->library);
#else
    ext::on_unload(&core->dl_ctx);
#endif
    
    
    DrawTalk_World* dt_world = (DrawTalk_World*)core->user_data;
    World_deinit(&dt_world->mtt_world);
    mem::deallocate<DrawTalk_World>(&core->allocator, dt_world);
    mtt::deinit(&core->history);
    
    MTT_user_test_log_deinit();
}


void on_resize(MTT_Core* core, vec2 new_size)
{
    dt::on_resize(core, &((DrawTalk_World*)core->user_data)->dt, new_size);
    
    if (MTT_Core_active_pause_state(core)) {
        return;
    }
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    {
        core->dl_ctx.params = &new_size;
        core->library("on_resize", &core->dl_ctx);
    }
#else
    core->dl_ctx.params = &new_size;
    ext::on_resize(&core->dl_ctx);
#endif
}

}

