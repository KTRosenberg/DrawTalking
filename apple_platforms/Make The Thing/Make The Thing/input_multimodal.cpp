//
//  input_tool.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/9/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "input_multimodal.hpp"

namespace dt {


typedef void (*PROC_Control_process_input_began)(Control* tool, Input_Record* in, UI_Touch* t, void* user_data);
typedef void (*PROC_Control_process_input_moved)(Control* tool, Input_Record* in, UI_Touch* t, void* user_data);
typedef void (*PROC_Control_process_input_ended)(Control* tool, Input_Record* in, UI_Touch* t, void* user_data);
typedef void (*PROC_Control_process_input_cancelled)(Control* tool, Input_Record* in, UI_Touch* t, void* user_data);
typedef void (*PROC_Control_idle)(Control* tool, Input_Record* in, void* user_data);



    
#define FINGERS_PER_HUMAN (10)
void Control_System_init(Control_System* sys,
std::string tag,
mtt::World* mtt,
void* user_data)
{
    sys->tag = tag;
    sys->world = mtt;
    sys->user_data = user_data;
    
    usize id = 0;
    {
        Control* t = &sys->stylus.base;
        t->tag = tag;
        
        t->active_state = SP_CONTROL_ACTIVE_STATE_OFF;
        t->user_data = user_data;
        t->sys = sys;
        t->active_operation = nullptr;
        
        sys->tag_to_control[t->tag] = id;
        id += 1;
        {
            Control_Stylus* info = &sys->stylus;
            info->mode = SP_STYLUS_MODE_NONE;
            for (usize i = 0; i < SP_STYLUS_MODE_COUNT; i += 1) {
                create_stylus_operation(sys, SP_STYLUS_MODE_AS_STRING[i], nullptr, nullptr, nullptr, nullptr, nullptr);
            }
            for (usize i = 0; i < SP_TOUCH_MODE_COUNT; i += 1) {
                create_stylus_operation(sys, SP_TOUCH_MODE_AS_STRING[i], nullptr, nullptr, nullptr, nullptr, nullptr);
            }
        }
    }
    {
        Control* t = &sys->touch.base;
        t->tag = tag;
        t->active_state = SP_CONTROL_ACTIVE_STATE_OFF;
        t->user_data = user_data;
        t->sys = sys;
        t->active_operation = nullptr;
        
        sys->tag_to_control[t->tag] = id;
        
        id += 1;
        {
            Control_Touch* info = &sys->touch;
            info->mode = SP_TOUCH_MODE_NONE;
            info->touch_sequence.reserve(FINGERS_PER_HUMAN);
            info->count_active = 0;
        }
    }
}
#undef FINGERS_PER_HUMAN

void Control_System_input_began(Control_System*, Input_Record*, UI_Touch*, void*)
{
    
}
void Control_System_input_moved(Control_System*, Input_Record*, UI_Touch*, void*)
{
    
}
void Control_System_input_ended(Control_System*, Input_Record*, UI_Touch*, void*)
{
    
}
void Control_System_input_cancelled(Control_System*, Input_Record*, UI_Touch*, void*)
{
    
}
void Control_System_idle(Control_System*, Input_Record*, UI_Touch*, void*)
{
    
}



static usize create_stylus_operation(Control_System* sys, const std::string tag,
                         PROC_Control_process_input_began     input_began,
                         PROC_Control_process_input_moved     input_moved,
                         PROC_Control_process_input_ended     input_ended,
                         PROC_Control_process_input_cancelled input_cancelled,
                         PROC_Control_idle idle)
{
    sys->stylus_operations.emplace_back(Control_Operation());
    Control_Operation* op = &sys->stylus_operations.back();
    
    op->id  = sys->stylus_operations.size() - 1;
    op->tag = tag;
    sys->tag_to_stylus_operation[tag] = op->id;
    op->input_began      = input_began;
    op->input_moved      = input_moved;
    op->input_ended      = input_ended;
    op->input_cancelled  = input_cancelled;
    op->idle             = idle;
    
    return op->id;
}

static usize create_touch_operation(Control_System* sys, const std::string tag,
                         PROC_Control_process_input_began     input_began,
                         PROC_Control_process_input_moved     input_moved,
                         PROC_Control_process_input_ended     input_ended,
                         PROC_Control_process_input_cancelled input_cancelled,
                         PROC_Control_idle idle)
{
    sys->touch_operations.emplace_back(Control_Operation());
    Control_Operation* op = &sys->touch_operations.back();
    
    op->id  = sys->touch_operations.size() - 1;
    op->tag = tag;
    sys->tag_to_touch_operation[tag] = op->id;
    op->input_began      = input_began;
    op->input_moved      = input_moved;
    op->input_ended      = input_ended;
    op->input_cancelled  = input_cancelled;
    op->idle             = idle;
    
    return op->id;
}

static Control_Operation* get_stylus_operation(Control_System* sys, std::string tag)
{
    auto result = sys->tag_to_stylus_operation.find(tag);
    if (result == sys->tag_to_stylus_operation.end()) {
        return nullptr;
    }
    
    return get_stylus_operation(sys, result->second);
}

static Control_Operation* get_touch_operation(Control_System* sys, std::string tag)
{
    auto result = sys->tag_to_touch_operation.find(tag);
    if (result == sys->tag_to_touch_operation.end()) {
        return nullptr;
    }
    
    return get_touch_operation(sys, result->second);
}

}
