//
//  input.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/27/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "input.hpp"
extern_link_begin()
void Input_init(Input* input)
{
    MTT_print("%s\n", "input initialized");
    
    input->users = std::vector<Input_Record>();
    
    for (usize uid = 0; uid < 1; uid += 1) {
        auto* user = &input->users.emplace_back(Input_Record());
        user->direct_count = 0;
        user->pointer_count = 0;
        user->touches_free_count = 0;
        user->pointers_free_count = 0;
        user->pointers_to_remove_count = 0;
        user->touches_to_remove_count = 0;
        user->show_cursor = true;
    }
}

void Input_end_of_frame(Input* input)
{
    Input_Record* rec = &input->users[0];
    
    for (usize i = 0; i < rec->pointers_to_remove_count;) {
        UI_Touch* to_remove = &rec->pointers_to_remove[i];
        rec->pointer_map.erase(to_remove->key);
        i += 1;
    }
    //MTT_print("pointer map size: %zu\n", rec->pointer_map.size());
    for (usize i = 0; i < rec->touches_to_remove_count;) {
        UI_Touch* to_remove = &rec->touches_to_remove[i];
        rec->direct_map.erase(to_remove->key);
        i += 1;
    }
    //MTT_print("direct map size: %zu\n", rec->direct_map.size());
    rec->touches_to_remove_count = 0;
    rec->pointers_to_remove_count = 0;    
}

usize Input_next_free_pointer_touch(Input* input, usize uid)
{
    Input_Record* rec = &input->users[uid];
    if (rec->pointers_free_count > 0) {
        rec->pointers_free_count -= 1;
        return rec->pointers_free[rec->pointers_free_count];
    }
    rec->pointer_count += 1;
    return rec->pointer_count - 1;
}
usize Input_next_free_direct_touch(Input* input, usize uid)
{
    Input_Record* rec = &input->users[uid];
    if (rec->touches_free_count > 0) {
        rec->touches_free_count -= 1;
        return rec->touches_free[rec->touches_free_count];
    }
    rec->direct_count += 1;
    return rec->direct_count - 1;
}

void Input_schedule_pointer_touch_removal(Input* input, usize uid, UI_Touch* touch)
{
    Input_Record* rec = &input->users[uid];
    rec->pointers_to_remove[rec->pointers_to_remove_count] = *touch;
    rec->pointers_to_remove_count += 1;
}
void Input_schedule_direct_touch_removal(Input* input, usize uid, UI_Touch* touch)
{
    Input_Record* rec = &input->users[uid];
    //MTT_print("schedule removal %lu count %llu\n", touch->key, rec->touches_to_remove_count + 1);

    rec->touches_to_remove[rec->touches_to_remove_count] = *touch;
    rec->touches_to_remove_count += 1;
}

bool Input_find_touch_for_event(Input_Record* u_input, UI_Event* event, UI_Touch** t_el)
{
    auto result = u_input->direct_map.find(event->key);
    if (result == u_input->direct_map.end()) {
        return false;
    }
    
    *t_el = &result->second;
    
    return true;
}

void* Input_set_user_data_for_touch_event(Input_Record* u_input, UI_Event* event, UI_Touch** t_el, void* user_data)
{
    auto result = u_input->direct_map.find(event->key);
    if (result == u_input->direct_map.end()) {
        return nullptr;
    }
    
    *t_el = &result->second;
    result->second.user_data = user_data;
    
    return user_data;
}

void* Input_get_user_data_for_touch_event(Input_Record* u_input, UI_Event* event, UI_Touch** t_el)
{
    auto result = u_input->direct_map.find(event->key);
    if (result == u_input->direct_map.end()) {
        return nullptr;
    }
    
    *t_el = &result->second;
    
    return result->second.user_data;
}

void* Input_get_user_data_for_touch(Input_Record* u_input, UI_Touch* t_el)
{
    return t_el->user_data;
}
void* Input_set_user_data_for_touch(Input_Record* u_input, UI_Touch* t_el, void* user_data)
{
    t_el->user_data = user_data;
    return user_data;
}

void Input_clear_user_data_for_touch(Input_Record* u_input, UI_Touch* t_el)
{
    t_el->user_data = nullptr;
}

void* Input_set_user_data_for_touch_gesture_event(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Event* event, UI_Touch** t_el, void* user_data)
{
    auto* el = &u_input->gesture_input_map[gesture];
    el->user_data = user_data;
    *t_el = el;
    return user_data;
}

uint64 Input_set_flags_for_touch_gesture_event(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Event* event, UI_Touch** t_el, uint64 flags)
{
    auto* el = &u_input->gesture_input_map[gesture];
    el->flags = flags;
    *t_el = el;
    return flags;
}

void* Input_get_user_data_for_touch_gesture_event(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Event* event, UI_Touch** t_el)
{
    auto* el = &u_input->gesture_input_map[gesture];
    *t_el = el;
    return el->user_data;
}

uint64 Input_get_flags_for_touch_gesture_event(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Event* event, UI_Touch** t_el)
{
    auto* el = &u_input->gesture_input_map[gesture];
    *t_el = el;
    return el->flags;
}

void* Input_set_user_data_for_touch_gesture(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Touch** t_el, void* user_data)
{
    auto* el = &u_input->gesture_input_map[gesture];
    el->user_data = user_data;
    *t_el = el;
    return el->user_data;
}

void* Input_get_user_data_for_touch_gesture(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Touch** t_el)
{
    auto* el = &u_input->gesture_input_map[gesture];
    *t_el = el;
    return el->user_data;
}



void UI_Event_printf(UI_Event* event)
{
    MTT_print("UI_Event{\n\t"
              "state_type=[%s]\n\t"
              "input_type=[%s]\n\t"
              "key=[%lu]\n"
              "}\n",
              ui_state_type_to_string[event->state_type],
              ui_touch_type_to_string[event->input_type],
              event->key
    );
}


extern_link_end()

void Input_push_event(Input* input, usize uid, const UI_Event& event)
{
    input->users[uid].event_queue.push(event);
}
void Input_push_event(Input_Record* input_record, const UI_Event& event)
{
    input_record->event_queue.push(event);
}

bool Input_poll_event(Input* input, usize uid, UI_Event* event)
{
    std::queue<UI_Event>* q = &input->users[uid].event_queue;
    if (q->empty()) {
        return false;
    }
    *event = q->front();
    q->pop();
    return true;
}

bool Input_poll_event(Input_Record* input_record, UI_Event* event)
{
    std::queue<UI_Event>* q = &input_record->event_queue;
    if (q->empty()) {
        return false;
    }
    *event = q->front();
    q->pop();
    return true;
}

mtt::String text_in_buf = "";
usize text_in_cursor_idx = 0;
bool text_in_selected_all = false;
Input_Command_History text_in_history = {};

void text_input_clear(void)
{
    text_in_buf.clear();
    text_in_selected_all = false;
    text_in_cursor_idx = false;
}

bool text_in_buf_is_equal(const mtt::String& against)
{
    return (text_in_buf.compare(against) == 0);
}

bool Input_Command_History::is_empty() {
    return count == 0;
}

void Input_Command_History::push(mtt::String& to_save)
{
    cmd_history[top] = to_save;
    count = (count != COMMAND_HISTORY_MAX) ? count + 1 : COMMAND_HISTORY_MAX;
    saved_count = count;
    top = (top + 1) % COMMAND_HISTORY_MAX;
    saved_top = top;
}
bool Input_Command_History::rewind(void)
{
    if (saved_count <= 0) {
        return false;
    }
    saved_count -= 1;
    saved_top = ((saved_top + COMMAND_HISTORY_MAX) - 1) % COMMAND_HISTORY_MAX;
    return true;
}
bool Input_Command_History::forwards(void)
{
    if (saved_count >= count) {
        return false;
    }
    
    saved_top = (saved_top + 1) % COMMAND_HISTORY_MAX;
    saved_count += 1;
    return true;
}

const mtt::String& Input_Command_History::get_top(void)
{
    isize idx = ((top + COMMAND_HISTORY_MAX) - 1) % COMMAND_HISTORY_MAX;
    return cmd_history[idx];
}

const mtt::String& Input_Command_History::get_left(void)
{
    if (saved_count == 0) {
        static const mtt::String empty_string = "";
        return empty_string;
    }
    isize idx = ((saved_top + COMMAND_HISTORY_MAX) - 1) % COMMAND_HISTORY_MAX;
    return cmd_history[idx];
}



const mtt::String& Input_Command_History::get_right(void) {
    {
        if (saved_count == count) {
            static const mtt::String empty_string = "";
            return empty_string;
        }
        isize idx = (saved_top + 1) % COMMAND_HISTORY_MAX;
        return cmd_history[idx];
    }
}

const mtt::String& Input_Command_History::forward_get(void)
{
    const mtt::String& res = get_right();
    forwards();
    return res;
}

void Input_Command_History::move_to_present(void)
{
    while (forwards());
}

void handle_dyvar(MTT_Core* core, mtt::String str_cmd)
{
    using namespace mtt;
    
    str_cmd = str_cmd.substr(5);
    trim(str_cmd);
    
    size_t find_type_idx = str_cmd.find(":");
    size_t find_set_idx = str_cmd.find("=");
    if (find_type_idx != std::string::npos && find_set_idx != std::string::npos && (find_type_idx < find_set_idx) &&
        find_set_idx + 1 < str_cmd.size()) {
        
        auto name_str = str_cmd.substr(0, find_type_idx);
        trim(name_str);
        auto type_str = str_cmd.substr(find_type_idx + 1, find_set_idx - (find_type_idx + 1));
        trim(type_str);
        auto val_str = str_cmd.substr(find_set_idx + 1);
        trim(val_str);
        
        if (!name_str.empty() && !type_str.empty() && !val_str.empty()){
            if (type_str == "u64") {
                core->dl_ctx.vars[name_str] = mtt::Any::from_UInt64(stoull(val_str));
            } else if (type_str == "i64") {
                core->dl_ctx.vars[name_str] = mtt::Any::from_Int64(stoll(val_str));
            } else if (type_str == "u32") {
                core->dl_ctx.vars[name_str] = mtt::Any::from_UInt32((uint32)stoul(val_str));
            } else if (type_str == "i32") {
                core->dl_ctx.vars[name_str] = mtt::Any::from_Int32(stoi(val_str));
            } else if (type_str == "f" || type_str == "f32") {
                core->dl_ctx.vars[name_str] = mtt::Any::from_Float32(stof(val_str));
            } else if (type_str == "f64") {
                core->dl_ctx.vars[name_str] = mtt::Any::from_Float64(stod(val_str));
            } else if (type_str == "s") {
                core->dl_ctx.vars[name_str] = mtt::Any::from_String(val_str.c_str());
            } else if (type_str == "c") {
                core->dl_ctx.vars[name_str] = mtt::Any::from_Char(val_str[0]);
            } else if (type_str == "p") {
                uintptr ptr = stoull(val_str, 0, 16);
                core->dl_ctx.vars[name_str] = mtt::Any::from_Reference_Type(ptr);
            } else if (type_str == "v2") {
                size_t find_comma = val_str.find(",");
                if (find_comma != std::string::npos) {
                    mtt::String vx = val_str.substr(0, find_comma);
                    if (find_comma + 1 < val_str.size()) {
                        val_str = val_str.substr(find_comma + 1);
                        trim(val_str);
                        mtt::String& vy = val_str;
                        
                        core->dl_ctx.vars[name_str] = mtt::Any::from_Vector2(
                                                                             stof(vx),
                                                                             stof(vy)
                                                                             );
                    }
                }
            } else if (type_str == "v3") {
                size_t find_comma = val_str.find(",");
                if (find_comma != std::string::npos) {
                    mtt::String vx = val_str.substr(0, find_comma);
                    if (find_comma + 1 < val_str.size()) {
                        val_str = val_str.substr(find_comma + 1);
                        trim(val_str);
                        find_comma = val_str.find(",");
                        if (find_comma != std::string::npos) {
                            mtt::String vy = val_str.substr(0, find_comma);
                            if (find_comma + 1 < val_str.size()) {
                                val_str = val_str.substr(find_comma + 1);
                                trim(val_str);
                                mtt::String& vz = val_str;
                                
                                core->dl_ctx.vars[name_str] = mtt::Any::from_Vector3(
                                                                                     stof(vx),
                                                                                     stof(vy),
                                                                                     stof(vz)
                                                                                     );
                            }
                        }
                    }
                }
            } else if (type_str == "v4") {
                size_t find_comma = val_str.find(",");
                if (find_comma != std::string::npos) {
                    mtt::String vx = val_str.substr(0, find_comma);
                    if (find_comma + 1 < val_str.size()) {
                        val_str = val_str.substr(find_comma + 1);
                        trim(val_str);
                        find_comma = val_str.find(",");
                        if (find_comma != std::string::npos) {
                            mtt::String vy = val_str.substr(0, find_comma);
                            if (find_comma + 1 < val_str.size()) {
                                val_str = val_str.substr(find_comma + 1);
                                trim(val_str);
                                find_comma = val_str.find(",");
                                if (find_comma != std::string::npos) {
                                    mtt::String vz = val_str.substr(0, find_comma);
                                    if (find_comma + 1 < val_str.size()) {
                                        val_str = val_str.substr(find_comma + 1);
                                        trim(val_str);
                                        mtt::String& vw = val_str;
                                        
                                        core->dl_ctx.vars[name_str] = mtt::Any::from_Vector4(
                                                                                             stof(vx),
                                                                                             stof(vy),
                                                                                             stof(vz),
                                                                                             stof(vw)
                                                                                             );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
