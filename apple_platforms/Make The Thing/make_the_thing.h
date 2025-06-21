//
//  make_the_thing.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/11/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef make_the_thing_h
#define make_the_thing_h

static const float64 MTT_TIMESTEP    = 1.0/120.0;
static const uint64 MTT_TIMESTEP_NS = (uint64) ( (float64)1e9 * (MTT_TIMESTEP) );


#include "stratadraw.h"
#include "memory.hpp"



#include "command_history.hpp"

//#include "input.hpp"

#include "speech_info.hpp"

#include "color.hpp"
#include "thing.hpp"

#include "augmented_reality.hpp"

#include "extension_server.hpp"
#include "remote_peers.hpp"
#include "configuration.hpp"

#include "video_recording.h"

#ifndef NDEBUG

#ifdef __GNUC__
#define MTT_NOINLINE __attribute__((noinline))
#else
#define MTT_NOINLINE
#endif

#define MTT_BP() MTT_debug_breakpoint()

MTT_NOINLINE static void MTT_debug_breakpoint(void)
{
    int bp___;
    (void)bp___;
}

#else

#define MTT_BP()
#define MTT_NOINLINE

static inline void MTT_debug_breakpoint(void)
{
}

#endif



extern_link_begin()

#include "c_library.h"

#include "dynamic_library.hpp"

#include "dynamic_library_shared.hpp"

#define SELECT_ALL_MODE_ON  0x11
#define SELECT_ALL_MODE_OFF 0x10
#define HIDE_MODE_TOGGLE  0x100




struct MTT_Core;

// common
struct MTT_Core {
    void* core_platform;
    
    void* user_data;
    
    mem::Allocator allocator;
    usize page_size;
    mem::Allocator memory_pool_allocator_backing;
    mem::Allocator memory_pool_allocator;
    mem::Memory_Pool_Fixed memory_pool;
    
    sd::Renderer* renderer;
    MTT_Video_Recording_Context_Ref screen_recording;
    
    //struct Time_Info {
        uint64 time_nanoseconds;
        uint64 time_nanoseconds_prev;
        uint64 time_delta_nanoseconds;
        float64 time_delta_seconds;
        uint64 time_milliseconds;
        float64 time_seconds;
        uint64 time_seconds_prev;
        
        float64 time_sim_elapsed;
        uint64 time_sim_elapsed_ns;
        
        uint64 time_sim_paused_elapsed_ns;
        float64 time_sim_paused_elapsed_s;
        
        uint64 time_sim_accumulator_counter_ns;
        float64 elapsed;
    //};
    
    bool is_paused;
    
    //float64       time_frame_delta_seconds;
    
    mtt::Command_History history;
    
    Speech_System speech_system;
    
    vec3 view_position;
    mat4 view_transform;
    sd::Viewport viewport;
    
    float64 default_aspect_ratio;
    
    Input input;
    
    sd::Image_Atlas image_atlas;
    

    
    void (*set_command_line_text)(MTT_Core* core, cstring text, usize length, uint64 flags);
    //char* const (*get_debug_visualization)(MTT_Core*);
    
    
    usize (*load_remote_text)(MTT_Core* core, mtt::String message);
    usize (*query_for_data)(MTT_Core* core, const mtt::String& message, void (*process_new_ID)(void*, usize), void* ctx);
    void (*send_message_drawtalk_server[2])(MTT_Core* core, const mtt::String& message_type, const mtt::String& message);
    
   // void (*visualize)(MTT_Core* core, const mtt::String& as_string);
    
    twn_Context* tween_ctx;
    
    vec3 screen_position_left_eye;
    vec3 screen_position_right_eye;
    vec3 look_at_point;
    
    mtt::Augmented_Reality_Context ar_ctx;
    
    Extension_Server_Frontend server_ext;
    
    mtt::Remote_System remote_system;
    
    mtt::Application_Configuration application_configuration;
    
    MTT_Dynamic_Library library;
    mtt::DL_Ctx dl_ctx;
    
    void (*on_application_focus)(MTT_Core*);
    
    FILE* test_log_file;
};

typedef enum DRAWTALK_SERVER_PARALLEL_COMMAND_INDEX {
    DRAWTALK_SERVER_PARALLEL_COMMAND_INDEX_DEFAULT,
    DRAWTALK_SERVER_PARALLEL_COMMAND_INDEX_QUICK_COMMAND,
} DRAWTALK_SERVER_PARALLEL_COMMAND_INDEX;

static inline void send_message_drawtalk_server(MTT_Core* core, const mtt::String& message_type, const mtt::String& message, usize parallel_command_index)
{
    core->send_message_drawtalk_server[parallel_command_index](core, message_type, message);
}

static inline void send_message_drawtalk_server_default(MTT_Core* core, const mtt::String& message_type, const mtt::String& message)
{
    core->send_message_drawtalk_server[(usize)DRAWTALK_SERVER_PARALLEL_COMMAND_INDEX_DEFAULT](core, message_type, message);
}

static inline void send_message_drawtalk_server_quick_command(MTT_Core* core, const mtt::String& message_type, const mtt::String& message)
{
    core->send_message_drawtalk_server[(usize)DRAWTALK_SERVER_PARALLEL_COMMAND_INDEX_QUICK_COMMAND](core, message_type, message);
}


static inline void MTT_Core_toggle_pause(MTT_Core* core)
{
    core->is_paused = !core->is_paused;
    core->time_sim_accumulator_counter_ns = 0;
}
static inline bool MTT_Core_toggle_pause_and_check(MTT_Core* core)
{
    MTT_Core_toggle_pause(core);
    return core->is_paused;
}
static inline void MTT_Core_pause(MTT_Core* core)
{
    core->time_sim_accumulator_counter_ns = 0;
    core->is_paused = true;
}
static inline void MTT_Core_resume(MTT_Core* core)
{
    core->time_sim_accumulator_counter_ns = 0;
    core->time_delta_nanoseconds = 0;
    core->time_delta_seconds = 0.0;
    
    core->is_paused = false;
}

static inline bool MTT_Core_active_pause_state(MTT_Core* core)
{
    return core->is_paused;
}

void MTT_Core_init(MTT_Core* core, void* core_platform, mem::Allocator* allocator);
void MTT_Core_deinit(MTT_Core* core);

struct MTT_Core* mtt_core_ctx(void);

bool is_main_thread(void);


void* MTT_default_heap_allocate(void* data, usize byte_count);
void MTT_default_heap_deallocate(void* data, void* memory, usize count);
void* MTT_default_heap_resize(void* data, void* memory, usize byte_count, usize old_byte_count);

void MTT_follow_system_window(bool state);

void MTT_reset_defaults(void);

extern_link_end()

namespace mtt {

void setup(MTT_Core* core);
void on_frame(MTT_Core* core);
void on_exit(MTT_Core* core);
void on_resize(MTT_Core* core, vec2 new_size);

}


#endif /* make_the_thing_h */

#ifdef MTT_IMPLEMENTATION
#undef MTT_IMPLEMENTATION

extern_link_begin()

#include "c_library.h"

extern_link_end()

#include "mtt_core_platform.h"

extern_link_begin()

//#include "string_intern.h"

MTT_Core* global_mtt_core_ctx;

void* MTT_default_heap_allocate(void* data, usize byte_count)
{
    return default_heap_allocate(data, byte_count);
}

void MTT_default_heap_deallocate(void* data, void* memory, usize count)
{
    default_heap_deallocate(data, memory, count);
}

//void default_heap_deallocate_all(void* data, usize count)
//{
//    return free(memory);
//}

void* MTT_default_heap_resize(void* data, void* memory, usize byte_count, usize old_byte_count)
{
    return default_heap_resize(data, memory, byte_count, old_byte_count);
}

void MTT_Core_init(MTT_Core* core, void* core_platform, mem::Allocator* allocator)
{
    core->core_platform = core_platform;
    core->allocator = *allocator;
    core->time_sim_accumulator_counter_ns = 0;
    core->time_sim_elapsed = 0.0;
    core->is_paused = true;
    core->viewport = {0.0f, 0.0f, 0.0f, 0.0f};
    
    //core->get_debug_visualization = nullptr;
    
    core->history = {};
        
    core->tween_ctx = (twn_Context*)twn_new(1024);
    
    global_mtt_core_ctx = core;
    
    core->application_configuration = {};
    core->application_configuration.custom_data = (void*)core;
    
    core->on_application_focus = NULL;
}

void MTT_Core_deinit(MTT_Core* core)
{
    mem::deallocate(&core->allocator, core->tween_ctx);
}

float64 system_timestamp_to_mtt_seconds_with_core(struct MTT_Core* core, float64 target_system_timestamp)
{
    return system_timestamp_to_mtt_seconds(core->elapsed, core->time_seconds, target_system_timestamp);
}

struct MTT_Core* mtt_core_ctx(void)
{
    return global_mtt_core_ctx;
}


extern_link_end()

#endif

