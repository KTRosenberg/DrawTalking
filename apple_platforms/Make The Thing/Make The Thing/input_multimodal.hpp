//
//  input_multimodal.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/9/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef input_tool_hpp
#define input_tool_hpp

#include "thing.hpp"

namespace dt {

typedef enum SP_STYLUS_MODE {
    SP_STYLUS_MODE_NONE,
    SP_STYLUS_MODE_DRAW,
    SP_STYLUS_MODE_POINT_AT,
    SP_STYLUS_MODE_SKETCH_RECOGNITION,
    
    SP_STYLUS_MODE_COUNT,
} SP_STYLUS_MODE;

const char* const SP_STYLUS_MODE_AS_STRING[] = {
    "SP_STYLUS_MODE_NONE",
    "SP_STYLUS_MODE_DRAW",
    "SP_STYLUS_MODE_POINT_AT",
    "SP_STYLUS_MODE_SKETCH_RECOGNITION",
};

typedef enum SP_TOUCH_MODE {
    SP_TOUCH_MODE_NONE,
    SP_TOUCH_MODE_THING_MANIPULATION,
    SP_TOUCH_MODE_SELECTION,
    SP_TOUCH_MODE_GESTURE,
    
    SP_TOUCH_MODE_COUNT,
} SP_TOUCH_MODE;

const char* const SP_TOUCH_MODE_AS_STRING[] = {
    "SP_TOUCH_MODE_NONE",
    "SP_TOUCH_MODE_THING_MANIPULATION",
    "SP_TOUCH_MODE_SELECTION",
    "SP_TOUCH_MODE_GESTURE",
};

typedef enum SP_CONTROL_ACTIVE_STATE {
    SP_CONTROL_ACTIVE_STATE_ON,
    SP_CONTROL_ACTIVE_STATE_OFF,
    
    SP_TOOL_ACTIVE_STATE_COUNT,
} SP_CONTROL_ACTIVE_STATE;

const char* const SP_CONTROL_MODE_AS_STRING[] = {
    "SP_CONTROL_ACTIVE_STATE_ON",
    "SP_CONTROL_ACTIVE_STATE_OFF",
};



struct Control;

typedef void (*PROC_Control_process_input_began)(Control* tool, Input_Record* in, UI_Touch* t, void* user_data);
typedef void (*PROC_Control_process_input_moved)(Control* tool, Input_Record* in, UI_Touch* t, void* user_data);
typedef void (*PROC_Control_process_input_ended)(Control* tool, Input_Record* in, UI_Touch* t, void* user_data);
typedef void (*PROC_Control_process_input_cancelled)(Control* tool, Input_Record* in, UI_Touch* t, void* user_data);
typedef void (*PROC_Control_idle)(Control* tool, Input_Record* in, void* user_data);

// operation handlers that a tool calls
struct Control_Operation {
    std::string tag;
    usize id;
    
    PROC_Control_process_input_began     input_began;
    PROC_Control_process_input_moved     input_moved;
    PROC_Control_process_input_ended     input_ended;
    PROC_Control_process_input_cancelled input_cancelled;
    PROC_Control_idle idle;
    
    void* user_data;
};


struct Control_System;

struct Control {
    SP_CONTROL_ACTIVE_STATE active_state;
    
    std::string tag;
    usize id;

    usize  count;
    usize  count_active;
    uint64 priority;
    
    Control_Operation* active_operation;
    
    
    void* user_data;
    
    Control_System* sys;
};

struct Control_Stylus {
    Control base;
    SP_STYLUS_MODE mode;
};
struct Control_Touch {
    Control base;
    std::vector<usize> touch_sequence;
    usize count_active;
    SP_TOUCH_MODE mode;
};



// holds all tool information
struct Control_System {
    
    std::string tag;
    
    robin_hood::unordered_map<std::string, usize> tag_to_control;
    robin_hood::unordered_map<std::string, usize> tag_to_stylus_operation;
    robin_hood::unordered_map<std::string, usize> tag_to_touch_operation;
    
    std::vector<Control_Operation> stylus_operations;
    std::vector<Control_Operation> touch_operations;

    Control_Touch   touch;
    Control_Stylus stylus;
    
    void* user_data;
    
    mtt::World* world;
    
    vec3 most_recent_bg_touch_canvas      = vec3(0.0f);
    vec3 most_recent_bg_touch_transformed = vec3(0.0f);
};

void Control_System_input_began(Control_System*, Input_Record*, UI_Touch*, void*);
void Control_System_input_moved(Control_System*, Input_Record*, UI_Touch*, void*);
void Control_System_input_ended(Control_System*, Input_Record*, UI_Touch*, void*);
void Control_System_input_cancelled(Control_System*, Input_Record*, UI_Touch*, void*);
void Control_System_idle(Control_System*, Input_Record*, UI_Touch*, void*);

void Control_System_init(Control_System* sys,
                      std::string tag,
                      mtt::World* mtt,
                      void* user_data);


static usize create_touch_operation(Control_System* sys, std::string tag,
                         PROC_Control_process_input_began     input_began,
                         PROC_Control_process_input_moved     input_moved,
                         PROC_Control_process_input_ended     input_ended,
                         PROC_Control_process_input_cancelled input_cancelled,
                         PROC_Control_idle idle);

static usize create_stylus_operation(Control_System* sys, std::string tag,
    PROC_Control_process_input_began     input_began,
    PROC_Control_process_input_moved     input_moved,
    PROC_Control_process_input_ended     input_ended,
    PROC_Control_process_input_cancelled input_cancelled,
    PROC_Control_idle idle);

static Control_Operation* get_stylus_operation(Control_System* sys, std::string tag);

static inline Control_Operation* get_stylus_operation(Control_System* sys, usize id)
{
    return &sys->stylus_operations[id];
}

static Control_Operation* get_touch_operation(Control_System* sys, std::string tag);

static inline Control_Operation* get_touch_operation(Control_System* sys, usize id)
{
    return &sys->touch_operations[id];
}

}

#endif /* input_tool_hpp */
