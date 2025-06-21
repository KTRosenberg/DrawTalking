//
//  thing_lib.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/28/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#include "thing_lib.hpp"

namespace mtt {


//void selector_init(ARCHETYPE_PARAM_LIST)
//{
//    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
//                                meta[MTT_INT64].alloc_byte_size + meta[MTT_INT64].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size
//                                );
//    
//    add_field({world, fields, ports, logic, "cycle_index", MTT_INT64, 0});
//    
//    
//    // TODO: support a default value
//    add_field({world, fields, ports, logic, "index_count", MTT_INT64, 0}, temp_mem<uint64>(30));
//    add_field({world, fields, ports, logic, "prev_cycle_state", MTT_BOOLEAN, 0}, temp_mem<bool>(false));
//    
//    
//    
//    Thing_add_in_port({world, fields, ports, logic, "cycle", MTT_INT64, 0, {ALIGN_DOWN}, nullptr});
//    Thing_add_in_port({world, fields, ports, logic,  "active", MTT_BOOLEAN, 0, {ALIGN_DOWN}, nullptr});
//    for (usize idx = 0; idx < 30; idx += 1) {
//        Thing_add_in_port({world, fields, ports, logic, std::to_string(idx), MTT_BOOLEAN, 0, {ALIGN_LEFT + ALIGN_DOWN * (idx * 0.15f)}, nullptr});
//        
//        Thing_add_out_port({world, fields, ports, logic, std::to_string(idx), MTT_BOOLEAN, 0, {ALIGN_LEFT + ALIGN_DOWN * (idx * 0.15f)}, nullptr});
//    }
//    
//    
//    
//    archetype->logic.proc = selector_procedure;
//    
//    archetype->logic.option = 0;
//    archetype->logic.option_flags = 0;
//    
//    archetype->on_thing_make = [](Thing* thing) {
//        auto* init_cycle_state = mtt::access<bool>(thing, "prev_cycle_state");
//        *init_cycle_state = false;
//    };
//}



MTT_DEFINE_INITIALIZER(list_cycler)
{    
    MTT_ADD_FIELD_T("state", List_Cycler);
    
    
    Thing_add_in_port({world, fields, ports, logic, "el_list", MTT_LIST, MTT_ANY,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "reset", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_ANY, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_out_port({world, fields, ports, logic, "selector", MTT_TEXT, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_out_port({world, fields, ports, logic, "index", MTT_INT64, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
}

MTT_DEFINE_LOGIC_PROCEDURE(list_cycler)
{
    auto& state =  *mtt::access<List_Cycler>(thing, "state");

    auto IN_el_list = get_in_port(thing, input, "el_list");
    auto IN_reset = get_in_port(thing, input, "reset");
    
    auto OUT_value = get_out_port(thing, "value");
    auto OUT_selector = get_out_port(thing, "selector");
    
    auto OUT_index = get_out_port(thing, "index");
    OUT_index.value.out.set_Int64(state.curr_idx);
    
    return true;
}




MTT_DEFINE_INITIALIZER(jump)
{
    Thing_add_in_port({world, fields, ports, logic, "height", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "speed", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "cycle_count", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "should_attach", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
//    Thing_add_in_port({world, fields, ports, logic, "source", MTT_ANY, 0,
//        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
//    });
    Thing_add_in_port({world, fields, ports, logic, "source_selector", MTT_TEXT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });


    Thing_add_in_port({world, fields, ports, logic, "target", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "target_selector", MTT_TEXT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    Thing_add_in_port({world, fields, ports, logic, "interpolation", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "reset", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    MTT_ADD_FIELD_T("state", Jump_State);
}

MTT_DEFINE_LOGIC_PROCEDURE(jump)
{
    auto* state = mtt::access<Jump_State>(thing, "state");
    
    auto IN_height           = get_in_port(thing, input, "height");
    auto IN_speed            = get_in_port(thing, input, "speed");
    
    auto IN_source           = get_in_port(thing, input, "source");
    auto IN_source_selector  = get_in_port(thing, input, "source_selector");
    
    auto IN_target           = get_in_port(thing, input, "target");
    auto IN_target_selector  = get_in_port(thing, input, "target_selector");
    
    auto IN_reset            = get_in_port(thing, input, "reset");
    
    auto IN_interpolation = mtt::get_in_port(thing, input, "interpolation");
    
    auto IN_cycle_count = mtt::get_in_port(thing, input, "cycle_count");

//    ASSERT_MSG(IN_height.status == PORT_STATUS_OK, "???");
//    ASSERT_MSG(IN_speed.status == PORT_STATUS_OK, "???");
//    
//    MTT_print("{\n");
//    mtt::Any_print(IN_height.value.out);
//    mtt::Any_print(IN_speed.value.out);
//    mtt::Any_print(IN_source.value.out);
//    mtt::Any_print(IN_source_selector.value.out);
//    mtt::Any_print(IN_target.value.out);
//    mtt::Any_print(IN_target_selector.value.out);
//    MTT_print("}\n");
    
    
    float32 speed = (IN_speed.status == PORT_STATUS_OK) ? IN_speed.value.out.Float : 1.0f;
    float32 height = (IN_height.status == PORT_STATUS_OK) ? m::max(1.0f, IN_height.value.out.Float) : 1.0f;
    float32 cycle_count = (IN_cycle_count.status == PORT_STATUS_OK) ? IN_cycle_count.value.out.Float : 1;
    
    struct State {
        mtt::Thing* src_thing = nullptr;
        
        mtt::Thing* tgt_thing = nullptr;
        vec3 tgt_vec = {};
        
        
        MTT_String_Ref src_selector = {};
        MTT_String_Ref tgt_selector = {};
        
        MTT_TYPE tgt_type = MTT_NONE;
    } sel_state;
    sel_state.src_selector = IN_source_selector.value.out.String;
    sel_state.tgt_selector = IN_target_selector.value.out.String;
    
    switch (IN_source.value.out.type) {
        case MTT_VECTOR3: {
            break;
        }
        case MTT_THING: {
            sel_state.src_thing = mtt::Thing_try_get(world, IN_source.value.out.thing_id);
            if (sel_state.src_thing == nullptr) {
                // failed, need to quit
                state->is_init = true;
                state->interpolation_value = 0.0f;
//                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                get_out_port(thing, "is_done", [&](Evaluation_Output_Entry_Port& entry) {
                    bool is_done = true;
                    entry.out.set_Boolean(is_done);
                    state->is_init = false;
                });
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            } else if (mtt::should_be_immobile(sel_state.src_thing)) {
                
                state->is_init = true;
                state->interpolation_value = 0.0f;
                if (mtt::input_should_cancel_animation(sel_state.src_thing)) {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                }
                
                get_out_port(thing, "is_done", [&](Evaluation_Output_Entry_Port& entry) {
                    bool is_done = true;
                    entry.out.set_Boolean(is_done);
                    state->is_init = false;
                });
                
//                state->position_source_start = *mtt::access<vec3>(sel_state.src_thing, "position");
//                {
//                    Message msg;
//                    msg.sender   = thing->id;
//                    msg.selector = string_ref_get("center");
//                    selector_invoke(sel_state.src_thing, &msg);
//                    
//                    mtt::Any& out = msg.output_value;
//                    vec3 position = out.Vector3;
//                    Script_Lookup_set_var_with_key(s, mtt::ARG_object, mtt::DEFAULT_LOOKUP_SCOPE, {
//                        {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
//                    });
//                }
                
                
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            sel_state.src_selector = IN_source_selector.value.out.String;
            break;
        }
        default: {
            MTT_log_error("%s\n", "unsupported input");
        }
    }
    
    
    switch (IN_target.value.out.type) {
        case MTT_VECTOR3: {
            sel_state.tgt_vec = IN_target.value.out.Vector3;
            sel_state.tgt_type = MTT_VECTOR3;
            break;
        }
        case MTT_THING: {
            sel_state.tgt_thing = mtt::Thing_try_get(world, IN_target.value.out.thing_id);
            sel_state.tgt_type = MTT_THING;
            
            if (sel_state.tgt_thing == nullptr) {
                get_out_port(thing, "is_done", [&](Evaluation_Output_Entry_Port& entry) {
                    bool is_done = true;
                    entry.out.set_Boolean(is_done);
                    state->is_init = false;
                });
                // failed, need to quit
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
            }
            sel_state.tgt_selector = IN_target_selector.value.out.String;
            break;
        }
        case MTT_NONE: {
            break;
        }
        default: {
            MTT_log_error("%s\n", "unsupported input");
            state->is_init = false;
        }
    }
    
    if (!state->is_init) {
        state->is_init = true;
        state->interpolation_value = 0.0f;
        if (sel_state.tgt_type == MTT_THING) {
            if (mtt::is_ancestor_of(sel_state.src_thing, sel_state.tgt_thing)) {
                mtt::disconnect_parent_from_children(world, sel_state.src_thing);
            }
            
            // FIXME: should not really use the words directly here
            auto* target_var = Script_Lookup_get_var_with_key(s->lookup(), ARG_object, DEFAULT_LOOKUP_SCOPE);
            if (target_var != nullptr) {
                auto& t_var = (*target_var)[0];
                if (t_var.label == "toward" || t_var.label == "towards") {
                    state->target_use_start_position = true;
                    {
                        Message msg = {};
                        msg.sender = sel_state.tgt_thing->id;
                        msg.selector = sel_state.tgt_selector;
                        
                        selector_invoke(sel_state.tgt_thing, &msg);
                        ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
                        state->position_target_start = msg.output_value.Vector3;
                    }
                }
            }
            
            
        }

        {
             
            Message msg = {};
            msg.sender = sel_state.src_thing->id;
            msg.selector = sel_state.src_selector;
            
            selector_invoke(sel_state.src_thing, &msg);
            ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
            state->position_source_start = msg.output_value.Vector3;
            state->raw_position_source_start = *mtt::access<vec3>(sel_state.src_thing, "position");
        }
    }
    
    float32 interpolation = 0.0f;
    if (IN_interpolation.status == mtt::PORT_STATUS_OK) {
        interpolation = IN_interpolation.value.out.Float;
        interpolation = m::clamp(interpolation, 0.0f, 1.0f);
        state->interpolation_value = interpolation;
    } else {
        interpolation = state->interpolation_value;
        interpolation = m::clamp(interpolation, 0.0f, 1.0f);
        state->interpolation_value += MTT_TIMESTEP * speed;
        //MTT_print("INTERPOLATION_VALUE: %f", state->interpolation_value);
    }
    
    
    
    if (!get_out_port(thing, "is_done", [&](Evaluation_Output_Entry_Port& entry) {
        bool is_done = interpolation >= 1.0f;
        entry.out.set_Boolean(is_done);
        state->is_init = !is_done;
        if (is_done) {
            if (sel_state.tgt_type == MTT_THING) {
                const auto cmp = mtt::String(MTT_string_ref_to_cstring(sel_state.tgt_selector));

                auto IN_should_attach = get_in_port(thing, input, "should_attach");
                if (IN_should_attach.status != PORT_STATUS_OK || IN_should_attach.value.out.Boolean == true) {
                    if (cmp == ARG_spatial_top || cmp == ARG_spatial_center) {
                        if (!mtt::is_ancestor_of(sel_state.tgt_thing, sel_state.src_thing)) {
                            mtt::connect_parent_to_child(world, sel_state.tgt_thing, sel_state.src_thing);
                        }
                    } else if (sel_state.tgt_thing->id == sel_state.src_thing->parent_thing_id) {
                        mtt::disconnect_child_from_parent(world, sel_state.src_thing);
                    }
                }
                
            }
        }
        
    })) {
        
        if (interpolation >= 1.0f) {
            state->interpolation_value = 0.0f;
            state->is_init = false;
            
            if (sel_state.tgt_type == MTT_THING) {
                const auto cmp = mtt::String(MTT_string_ref_to_cstring(sel_state.tgt_selector));

                auto IN_should_attach = get_in_port(thing, input, "should_attach");
                if (IN_should_attach.status != PORT_STATUS_OK || IN_should_attach.value.out.Boolean == true) {
                    if (cmp == ARG_spatial_top || cmp == ARG_spatial_center) {
                        mtt::connect_parent_to_child(world, sel_state.tgt_thing, sel_state.src_thing);
                    } else if (sel_state.tgt_thing->id == sel_state.src_thing->parent_thing_id) {
                        mtt::disconnect_child_from_parent(world, sel_state.src_thing);
                    }
                }
                
            }
        }
    }
    
    
    //MTT_print("JUMP_INTERPOLATE: %f\n", interpolation);
    
    {
        vec3 src_pos = {0.0f, 0.0f, 0.0f};
        vec3 dst_pos = {0.0f, 0.0f, 0.0f};
        vec2 init_src = vec2(state->position_source_start);
        
        
        
        {
            Message msg;
            msg.sender   = thing->id;
            msg.selector = sel_state.src_selector;
            
            selector_invoke(sel_state.src_thing, &msg);
            ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
            src_pos = msg.output_value.Vector3;
        }
        if (sel_state.tgt_type == MTT_THING) {
            if (!state->target_use_start_position) {
                Message msg;
                msg.sender   = sel_state.src_thing->id;
                msg.selector = sel_state.tgt_selector;
                msg.input_value.set_Float(1.5);
                
                Message msg_2;
                msg_2.sender = sel_state.src_thing->id;
                msg_2.selector = sel_state.tgt_selector;
                msg_2.input_value.set_Vector3(state->raw_position_source_start);
                msg.next = &msg_2;
                selector_invoke(sel_state.tgt_thing, &msg);
#ifndef NDEBUG
                //MTT_print("%s\n", string_get(msg.selector));
#endif
                ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
                dst_pos = msg.output_value.Vector3;
                dst_pos.y += (MTT_const_string_ref_is_equal_cstring(msg.selector, "top_with_offset")) ? 1 : 0;
            } else {
                dst_pos = state->position_target_start;
            }
        } else if (sel_state.tgt_type == MTT_VECTOR3) {
            dst_pos = sel_state.tgt_vec;
        }
        
        {
            
            vec2 cur = vec2(0.0f);
            vec3* src_position_ptr = mtt::access<vec3>(sel_state.src_thing, "position");
            {
                const float32 TODO_CEILING_PARAM = NEGATIVE_INFINITY;
                float32 ceiling = TODO_CEILING_PARAM;
                
                interpolation = m::inverse_smoothstep(interpolation);
                
                
                vec2 dst = vec2(dst_pos.x, dst_pos.y);
                
                //sel_state.tgt_vec = IN_target.value.out.Vector3;
                if (sel_state.tgt_type != MTT_VECTOR3) {
                    cur.x = m::lerp(init_src.x, dst_pos.x, interpolation);
                } else {
                    cur.x = src_position_ptr->x;
                }
                
                
                float32 sin_c = (dst.y - height < ceiling) ? (init_src.y - ceiling) * 0.75 : height;
                
                
                cur.y = m::max(ceiling, m::lerp(init_src.y, dst_pos.y, interpolation) + m::sin(interpolation * -MTT_PI_32 * cycle_count) * m::max(0.0f, sin_c));
            }
            
            //                std::cout << "src=[" << m::to_string(init_src) << "] cur=[" << m::to_string(cur) << "] dst=[" << m::to_string(dst_pos) << "]" << std::endl;
            
            vec3* init_src_ptr = &state->position_source_start;
            
            vec3 original_position = *src_position_ptr;
            
            if (interpolation < 0.5f || (!MTT_string_ref_is_valid(sel_state.tgt_selector) || (mtt::String(MTT_string_ref_to_cstring(sel_state.tgt_selector)) != ARG_spatial_over))) {
                mtt::Rep* rep = mtt::rep(sel_state.src_thing);
                float32 x_diff = init_src_ptr->x - cur.x;
                if (m::abs(x_diff) > 10) {
                    if (original_position.x < cur.x) {
                        if (heading_direction_x(rep) < 0) {
                            mtt::flip_left_right(sel_state.src_thing);
                        }
                    } else if (original_position.x > cur.x) {
                        if (heading_direction_x(rep) > 0) {
                            mtt::flip_left_right(sel_state.src_thing);
                        }
                    }
                }
            }
            *src_position_ptr = vec3(cur.x, cur.y, src_position_ptr->z);
            mtt::Thing_set_position(sel_state.src_thing, *src_position_ptr);
        }
    }
    
    
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}


void rotator_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_FLOAT].alloc_byte_size + meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_THING].alloc_byte_size);
    add_field({
        world, fields, ports, logic, "angle_radians", MTT_FLOAT, 0
    });
    
    add_field({
        world, fields, ports, logic, "direction", MTT_FLOAT, 0
    });
    
    add_field({
        world, fields, ports, logic, "axes", MTT_VECTOR3, 0
    });
    
    add_field({world, fields, ports, logic, "thing", MTT_THING, 0});
    
    
    
    Thing_add_in_port({world, fields, ports, logic, "speed", MTT_FLOAT, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "angle_radians", MTT_FLOAT, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "direction", MTT_FLOAT, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "axes", MTT_VECTOR3, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "thing", MTT_THING, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    

    Thing_add_out_port({world, fields, ports, logic, "angle_radians", MTT_BOOLEAN, 0,
        {(ALIGN_RIGHT * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "active", MTT_BOOLEAN, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    
    archetype->logic.proc = rotator_procedure;
    archetype->logic.option = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        auto* direction = mtt::access<float32>(thing, "direction");
        *direction = 1.0f;
        auto* axes = mtt::access<vec3>(thing, "axes");
        *axes = vec3(0.0f, 0.0f, 1.0f);
    };
}


LOGIC_PROC_RETURN_TYPE rotator_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE rotator_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    
    Thing* target_thing = mtt::get_parent_with_archetype(thing, ARCHETYPE_FREEHAND_SKETCH);
    if (target_thing == nullptr) {
        target_thing = mtt::get_parent_with_archetype(thing, ARCHETYPE_NUMBER);
    }
    
    auto in_direction = get_in_port(thing, input, "direction");
    float32 direction = 0.0f;
    if (in_direction.status == PORT_STATUS_OK && in_direction.value.out.type == MTT_FLOAT) {
        direction = in_direction.value.out.Float;
    } else {
        direction = *mtt::access<float32>(thing, "direction");
    }
    
    float32 speed = 1.0f;
    auto in_speed = get_in_port(thing, input, "speed");
    if (in_speed.status == PORT_STATUS_OK) {
        speed = in_speed.value.out.Float;
    }
    
    if (direction == 0.0f || speed == 0.0f) {
        return true;
    }
    
    
    if (target_thing != nullptr) {
        float32 timestep = world->timestep;
        
        
        auto* axes = mtt::access<vec3>(thing, "axes");
        
        Representation* target_rep;
        mtt::rep(target_thing, &target_rep);
        
        //        Representation* rot_rep;
        //        mtt::rep(thing, &rot_rep);
        
        //        vec3 d_scale;
        //        quat d_orientation;
        //        vec3 d_translation;
        //        vec3 d_skew;
        //        vec4 d_perspective;
        //        m::decompose(rot_rep->world_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
        
        Collider_remove(target_rep->colliders[0]->system, 0, target_rep->colliders[0]);
        
        set_pose_transform(target_thing, m::rotate(Mat4(1.0f), timestep * direction * speed, *axes) * target_rep->pose_transform);
        
        push_AABBs(target_rep->colliders[0]->system, target_rep->colliders[0], 1, 0);
    }
    
    for (mtt::Thing_Ref* target =
         mtt::access<mtt::Thing_Ref>(thing, "thing");
         target != nullptr;) {
        
        Thing* target_thing = nullptr;
        if (target->try_get(&target_thing)) {
            float32 timestep = world->timestep;
            
            auto* axes = mtt::access<vec3>(thing, "axes");
            
            Representation* target_rep;
            mtt::rep(target_thing, &target_rep);
            
            Collider_remove(target_rep->colliders[0]->system, 0, target_rep->colliders[0]);
            
            set_pose_transform(target_thing, m::rotate(Mat4(1.0f), timestep * direction * speed, *axes) * target_rep->pose_transform);
            
            push_AABBs(target_rep->colliders[0]->system, target_rep->colliders[0], 1, 0);
            
        }
        break;
    }
    
    
    
    return true;
}



MTT_DEFINE_INITIALIZER(move)
{
    MTT_ADD_FIELD_T("state", Move_State);
    
    Thing_add_in_port({world, fields, ports, logic, "velocity", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selector", MTT_TEXT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selectors", MTT_LIST, MTT_SCRIPT_PROPERTY,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "target_selectors", MTT_LIST, MTT_SCRIPT_PROPERTY,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "target_selector", MTT_TEXT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    Thing_add_in_port({world, fields, ports, logic, "velocity_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    
    Thing_add_in_port({world, fields, ports, logic, "source", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "should_attach", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
}

MTT_DEFINE_LOGIC_PROCEDURE(move)
{
    auto* state = mtt::access<Move_State>(thing, "state");
    
    auto IN_velocity             = get_in_port(thing, input, "velocity");
    vec3 velocity = (IN_velocity.status == PORT_STATUS_OK) ? IN_velocity.value.out.Vector3 : m::vec3_zero();
    
    auto IN_acceleration         = get_in_port(thing, input, "acceleration");
    vec3 acceleration = (IN_acceleration.status == PORT_STATUS_OK) ? IN_acceleration.value.out.Vector3 : vec3(1.0f, 1.0f, 1.0f);
    
    auto IN_velocity_local       = get_in_port(thing, input, "velocity_local");
    vec3 velocity_local = (IN_velocity_local.status == PORT_STATUS_OK) ? IN_velocity_local.value.out.Vector3 : m::vec3_zero();
    
    auto IN_acceleration_local   = get_in_port(thing, input, "acceleration_local");
    vec3 acceleration_local = (IN_acceleration_local.status == PORT_STATUS_OK) ? IN_acceleration_local.value.out.Vector3 : m::vec3_zero();
    
    
    auto IN_velocity_magnitude       = get_in_port(thing, input, "velocity_magnitude");
    float32 velocity_magnitude = (IN_velocity_magnitude.status == PORT_STATUS_OK) ? IN_velocity_magnitude.value.out.Float : 1.0f;
    auto IN_velocity_local_magnitude       = get_in_port(thing, input, "velocity_local_magnitude");
    float32 velocity_local_magnitude = (IN_velocity_local_magnitude.status == PORT_STATUS_OK) ? IN_velocity_local_magnitude.value.out.Float : 1.0f;
    
    auto IN_velocity_direction       = get_in_port(thing, input, "velocity_direction");
    vec3 velocity_direction = (IN_velocity_direction.status == PORT_STATUS_OK) ? IN_velocity_direction.value.out.Vector3 : m::vec3_zero();
    auto IN_velocity_local_direction       = get_in_port(thing, input, "velocity_local_direction");
    vec3 velocity_local_direction = (IN_velocity_local_direction.status == PORT_STATUS_OK) ? IN_velocity_local_direction.value.out.Vector3 : m::vec3_zero();
    
    auto IN_source_selector = get_in_port(thing, input, "source_selector");
    
    
    auto IN_source_selectors = get_in_port(thing, input, "source_selectors");
    
    
    
    auto OUT_is_done = get_out_port(thing, "is_done");
    OUT_is_done.value.out.set_Boolean(false);
    
    
    struct State {
        mtt::Thing* src_thing = nullptr;
        mtt::Thing* tgt_thing = nullptr;
        vec3 tgt_vec3 = m::vec3_zero();
        MTT_TYPE tgt_type = MTT_NONE;
        float32 magnitude = 0.0f;
        vec3 velocity = m::vec3_zero();
        vec3 acceleration = m::vec3_zero();
        MTT_String_Ref dir_selector = {};
        bool has_direction_selector = false;
        bool has_target_selector = false;
        bool has_target_selector_single = false;
        MTT_String_Ref target_selector = {};
    } st;
    
    st.magnitude = velocity_magnitude;
    
    st.acceleration = acceleration;
    if (IN_source_selector.status == PORT_STATUS_OK) {
        st.dir_selector = IN_source_selector.value.out.String;
        st.has_direction_selector = true;
    }
    
    auto IN_target_selector = get_in_port(thing, input, "target_selectors");
    if (IN_target_selector.status == PORT_STATUS_OK) {
        auto* prop_list = (Script_Property_List*)IN_target_selector.value.out.List;
        st.target_selector = (*prop_list)[0].value.String;
        st.has_target_selector = true;
    }
    
    auto IN_target_selector_single  = get_in_port(thing, input, "target_selector");
    if (IN_target_selector_single.status == PORT_STATUS_OK) {
        st.target_selector = IN_target_selector_single.value.out.String;
        st.has_target_selector_single = true;
    }
    
    auto IN_source = get_in_port(thing, input, "source");
    if (IN_source.status != PORT_STATUS_OK) {
        //return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
        OUT_is_done.value.out.set_Boolean(true);
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }
    auto IN_target = get_in_port(thing, input, "target");
    if (IN_target.status == PORT_STATUS_OK) {
        st.tgt_type = IN_target.value.out.type;
        st.tgt_thing = mtt::Thing_try_get(world, IN_target.value.out.thing_id);
        if (st.tgt_thing == nullptr) {
            OUT_is_done.value.out.set_Boolean(true);
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
        }
    }
    
    Script_Property_List* all_sources = nullptr;
    bool loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = false;
    usize loop_idx = 0;
    switch (IN_source.value.out.type) {
        default: {
            st.src_thing = mtt::Thing_try_get(world, IN_source.value.out.thing_id);
            if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            break;
        }
        case MTT_LIST: {
            all_sources = (Script_Property_List*)IN_source.value.out.List;
            if (all_sources->empty()) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            st.src_thing = mtt::Thing_try_get(world, (*all_sources)[loop_idx].value.thing_id);
            if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = true;
            break;
        }
    }
    
    do {
        vec3* V = mtt::access<vec3>(st.src_thing, "velocity");
        vec3* A = mtt::access<vec3>(st.src_thing, "acceleration");
        if (A != nullptr) {
            acceleration += *A;
        }
        
        
        switch (IN_target.value.out.type) {
            case MTT_THING: {
                st.tgt_type = MTT_THING;
                
                
                if (mtt::is_ancestor_of(st.src_thing, st.tgt_thing)) {
                    auto IN_should_attach = get_in_port(thing, input, "should_attach");
                    if (IN_should_attach.status != PORT_STATUS_OK || !IN_should_attach.value.out.Boolean) {
                        mtt::disconnect_child_from_parent(world, st.tgt_thing);
                    }
                }
                
                
                
                vec3* tgt_pos_ptr = mtt::access<vec3>(st.tgt_thing, "position");
                vec3 tgt_pos = *tgt_pos_ptr;
                if (st.has_target_selector || st.has_target_selector_single) {
                    Message msg;
                    msg.sender   = st.src_thing->id;
                    msg.selector = st.target_selector;
                    selector_invoke(st.tgt_thing, &msg);
                    
                    tgt_pos = msg.output_value.Vector3;
                }
                
                vec3* src_pos_ptr = mtt::access<vec3>(st.src_thing, "position");
                vec3 src_pos = *src_pos_ptr;
                
                vec3* vel_current_ptr = mtt::access<vec3>(st.src_thing, "velocity");
                vec3  vel_current = *vel_current_ptr;
                vec3  vel_prev = vel_current;
                
                vec3 diff = tgt_pos - src_pos;
                vec3 norm = (diff == m::vec3_zero()) ? m::vec3_zero() : m::normalize(diff);
                vel_current += diff * (float32)MTT_TIMESTEP * acceleration;
                
                
                float32 mag_current = vel_current == m::vec3_zero() ? 0.0f : m::length(vel_current);
                {
                    float32 mag_max     = velocity_magnitude;
                    vec3 dir = (vel_current == m::vec3_zero()) ? m::vec3_zero() : m::normalize(vel_current);
                    if (mag_current > mag_max) {
                        vel_current = dir * mag_max;
                        mag_current = mag_max;
                    }
                }
                
                vec3 pos_future = src_pos + ((float32)mtt::MOVEMENT_SCALE * vel_current * (float32)MTT_TIMESTEP);
                vec3 diff_future = tgt_pos - pos_future;
                auto norm_future = (diff_future == m::vec3_zero()) ? m::vec3_zero() : m::normalize(diff_future);
                auto s_future = m::sign(norm_future);
                auto s_curr = m::sign(norm);
                MTT_UNUSED(s_curr);
                if ((m::dist_squared(tgt_pos, pos_future) < 25.0f * 25.0f) || s_future != s_curr/* || s_future == vec3(0.0f) || m::dot(norm, diff_future) < 0.0f*/) {
                    
                    Message msg;
                    if (st.has_target_selector || st.has_target_selector_single) {
                        msg.sender   = st.src_thing->id;
                        msg.selector = st.target_selector;
                    } else {
                        msg.sender   = st.src_thing->id;
                        msg.selector = mtt::string("center");
                    }
                    
                    selector_invoke(st.tgt_thing, &msg);
                    ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
                    
                    vec3* tgt_velocity_ptr = mtt::access<vec3>(st.tgt_thing, "velocity");
                    vec3 tgt_velocity = (tgt_velocity_ptr != nullptr) ? *tgt_velocity_ptr : m::vec3_zero();
                    
                    if (tgt_velocity != m::vec3_zero()) {
                        float32 MAG = m::length(tgt_velocity);
                        if (MAG <= mag_current) {
                            *vel_current_ptr = MAG * m::normalize(tgt_velocity);
                            mtt::Thing_set_position(st.src_thing, msg.output_value.Vector3);
                        }
                    } else {
                        *vel_current_ptr = vec3(0.0f, 0.0f, 0.0f);
                        mtt::Thing_set_position(st.src_thing, msg.output_value.Vector3);
                        
                    }
                    //
                    //                *vel_current_ptr = (tgt_velocity == m::vec3_zero()) ? m::vec3_zero() : mag_current * m::normalize(tgt_velocity);
                    
                    
                    OUT_is_done.value.out.set_Boolean(true);
                } else {
                    *vel_current_ptr = vel_current;
                }
                
                
                if (m::sign(*vel_current_ptr) != m::sign(vel_prev)) {
                    
                    auto* rep = mtt::rep(st.src_thing);
                    
                    if (vel_current_ptr->x > 0) {
                        if (heading_direction_x(rep) < 0) {
                            mtt::flip_left_right(st.src_thing);
                        }
                    } else if (vel_current_ptr->x < 0) {
                        if (heading_direction_x(rep) > 0) {
                            mtt::flip_left_right(st.src_thing);
                        }
                    }
                }
                
                break;
            }
                // treat as direction
            case MTT_VECTOR3: {
                
                
                break;
            }
            case MTT_STRING:
                MTT_FALLTHROUGH;
            default: {
                
                
                if (IN_source_selectors.status == PORT_STATUS_OK) {
                    st.tgt_type = MTT_VECTOR3;
                    
                    Script_Property_List* all = (Script_Property_List*)IN_source_selectors.value.out.List;
                    for (auto& sel : *all) {
                        vec3 dir = m::vec3_zero();
                        mtt::String dir_sel = sel.value.get_cstring();
                        if (dir_sel == ARG_dir_up) {
                            dir = vec3(0.0f, -1.0f, 0.0f);
                        } else if (dir_sel == ARG_dir_down) {
                            dir = vec3(0.0f, 1.0f, 0.0f);
                        } else if (dir_sel == ARG_dir_left) {
                            dir = vec3(-1.0f, 0.0f, 0.0f);
                        } else if (dir_sel == ARG_dir_right) {
                            dir = vec3(1.0f, 0.0f, 0.0f);
                        } else if (dir_sel == ARG_dir_none) {
                            //dir = vec3(0.0f, 0.0f, 0.0f);
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                        }
                        
                        float32 prev_sign = m::sign(V->x);
                        vec3 force = st.magnitude * dir * acceleration.y;
                        *V += force * (float32)MTT_TIMESTEP;
                        
                        float32 m_adjust = m::length(*V);
                        if (m_adjust > velocity_magnitude) {
                            dir = m::normalize(*V);
                            *V = velocity_magnitude * dir;
                        }
                        if (m::sign(V->x) != prev_sign) {
                            
                            auto* rep = mtt::rep(st.src_thing);
                            
                            if (V->x > 0) {
                                if (heading_direction_x(rep) < 0) {
                                    mtt::flip_left_right(st.src_thing);
                                }
                            } else if (V->x < 0) {
                                if (heading_direction_x(rep) > 0) {
                                    mtt::flip_left_right(st.src_thing);
                                }
                            }
                        }
                        
                    }
                } else if (st.has_direction_selector) {
                    st.tgt_type = MTT_VECTOR3;
                    
                    mtt::String dir_sel = MTT_string_ref_to_cstring(st.dir_selector);
                    
                    vec3 dir = m::vec3_zero();
                    if (dir_sel == ARG_dir_up) {
                        dir = vec3(0.0f, -1.0f, 0.0f);
                    } else if (dir_sel == ARG_dir_down) {
                        dir = vec3(0.0f, 1.0f, 0.0f);
                    } else if (dir_sel == ARG_dir_left) {
                        dir = vec3(-1.0f, 0.0f, 0.0f);
                    } else if (dir_sel == ARG_dir_right) {
                        dir = vec3(1.0f, 0.0f, 0.0f);
                    } else if (dir_sel == ARG_dir_none) {
                        //dir = vec3(0.0f, 0.0f, 0.0f);
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                    }
                    
                    
                    float32 prev_sign = m::sign(V->x);
                    vec3 force = st.magnitude * dir;
                    *V += force * (float32)MTT_TIMESTEP;
                    
                    float32 m_adjust = m::length(*V);
                    if (m_adjust > velocity_magnitude) {
                        dir = m::normalize(*V);
                        *V = velocity_magnitude * dir;
                    }
                    if (m::sign(V->x) != prev_sign) {
                        
                        auto* rep = mtt::rep(st.src_thing);
                        if (heading_direction_x(rep) < 0) {
                            mtt::flip_left_right(st.src_thing);
                        }
                        
                        if (heading_direction_x(rep) > 0) {
                            mtt::flip_left_right(st.src_thing);
                        }
                    }
                    
                } else {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                }
                
                break;
            }
        }
        
        
        if (loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP) {
            loop_idx += 1;
            if (all_sources->size() > loop_idx) {
                st.src_thing = mtt::Thing_try_get(world, (*all_sources)[loop_idx].value.thing_id);
                if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                }
            } else {
                loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = false;
            }
        }
    } while (loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP);

    return true;
}

MTT_DEFINE_INITIALIZER(rule_eval)
{
    MTT_ADD_FIELD_T("state", Rule_Eval_State);
}

void w_search(mtt::Thing_ID thing_id, std::vector<dt::Word_Dictionary_Entry*>& words, mtt::Set_Stable<mtt::String>& ignore)
{
    auto& named_things = dt::word_dict()->thing_to_word;
    auto words_find = named_things.find(thing_id);
    if (words_find != named_things.end()) {
        for (auto* word : words_find->second) {
            if (ignore.contains(word->name)) {
                continue;
            }
            
            words.push_back(word);
        }
    }
    std::sort(std::begin(words), std::end(words), [](dt::Word_Dictionary_Entry* a, dt::Word_Dictionary_Entry* b) {
        return a->name <= b->name;
    });
}

bool label_entry_tgt_contains_comp(std::vector<dt::Word_Dictionary_Entry*>& tgt, std::vector<dt::Word_Dictionary_Entry*>& comp)
{
    // for each el in target vertex
    usize comp_i = 0;
    for (usize target_i = 0; target_i < tgt.size(); target_i += 1) {
        auto& tgt_to_find = tgt[target_i];
        
        // for each el in comp vertex, try to find each el in the target vertex
        bool found_sub_el = false;
        for (; comp_i < comp.size(); comp_i += 1) {
            if (comp[comp_i] == tgt_to_find) {
                found_sub_el = true;
                break;
            }
        }
        if (!found_sub_el) {
            // not found
            return false;
        }
    }
    return true;
}

MTT_DEFINE_LOGIC_PROCEDURE(rule_eval)
{
    bool rules_are_valid = s->rules_are_valid;
    
    if (!rules_are_valid) {
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
    }
    
    if (!thing_group_is_active(thing) || !s->rules_are_active) {
        return true;
    }
    
    auto* state = mtt::access<Rule_Eval_State>(thing, "state");
    state->update_count += 1;
    
    Script_Instance* rule_script = s;
    ASSERT_MSG(rule_script->is_rule(), "Should be a rule!\n");
    Script_Rules* rules = &rule_script->rules;
    
    auto& r = rules->triggers;
    //mtt::Var_Lookup& vars = r.var_lookup;
    auto& rule_list = r.rules;
    //auto& iterators = r.rule_iterators;
    ecs_world_t* ecs_world = world->ecs_world.c_ptr();

    bool is_valid = true;
    usize min_result_count = ULLONG_MAX;
    for (usize i = 0; i < r.rules.size(); i += 1) {
        auto* rule = r.rules[i].rule;
        auto& vars = r.vars_for_rule_idx[i];
        for (usize v = 0; v < vars.size(); v += 1) {
            vars[v].values.clear();
        }
        ecs_iter_t it = ecs_rule_iter(ecs_world, rule);
        
        //usize min_result_count = ULLONG_MAX;
        
        state->curr_results.clear();
        state->curr_results.reserve(vars.size());
        //state->prev_results.clear();
        //state->prev_results.resize(vars.size());
        state->dup_check.clear();
        usize result_count = 0;
        bool is_valid_for_rule = false;
        while(ecs_rule_next(&it)) {
            
            
//#ifndef NDEBUG
//            MTT_print("%s\n", ecs_iter_str(&it));
//#endif
            if (ecs_rule_var_count(rule) == 0) {
                continue;
            }
            
            
            
            for (usize v = 0; v < vars.size(); v += 1) {
                ecs_entity_t var_result = ecs_iter_get_var(&it, vars[v].var);
                
//                if (ecs_has_id(ecs_world, var_result, flecs::Prefab)) {
//                    continue;
//                }
                auto ent_cpp = flecs::entity(it.world, var_result);
                //if (ent_cpp.has<mtt::Thing_Info>()) {
                    mtt::Thing_ID thing_id = ent_cpp.get<mtt::Thing_Info>()->thing_id;
                    //vars[v].values.push_back(mtt::Any::from_Thing_ID(thing_id));
                    
                    state->curr_results.push_back(thing_id);
                //}
            }
            if (vars.size() != state->curr_results.size()) {
                continue;
            }
            
            const auto KEY = std::string((char*)state->curr_results.data(), state->curr_results.size() * sizeof(mtt::Thing_ID));
            bool is_same = state->dup_check.contains(KEY);
            if (!is_same) {
                is_valid_for_rule = true;
                //state->prev_results = state->curr_results;
                for (usize v = 0; v < vars.size(); v += 1) {
                    mtt::Thing_ID thing_id = state->curr_results[v];
                    vars[v].values.push_back(mtt::Any::from_Thing_ID(thing_id));
                    //MTT_print("id=[%llu], var=[%s]\n", thing_id, ecs_rule_var_name(rule, vars[v].var));
                }
                result_count += 1;
                state->dup_check.insert(KEY);
            }
            
            
            state->curr_results.clear();
        }
        if (!is_valid_for_rule) {
            is_valid = false;
            break;
        }
        min_result_count = m::min(min_result_count, result_count);
    }
    
    // TODO: TEMP HACK: value checks only on known values.
    bool is_valid_for_value_checks = false;
    for (usize i = 0; i < r.value_clauses.size(); i += 1) {
        auto& val_check = r.value_clauses[i];
        if (val_check[0].name == "equalbegin") {
            
            auto* el = &val_check[1];
            mtt::Thing_ID thing_id = el->value.thing_id;
            mtt::Thing* thing = mtt::Thing_try_get(world, thing_id);
            if (mtt::thing_type_id(thing) != ARCHETYPE_NUMBER) {
                continue;
            }
            
            auto thing_val = number_get_value(thing);
            
            
            auto* value = &val_check[2];
            
            bool comp_equals = false;
            switch (value->value.type) {
                case MTT_FLOAT64: {
                    float64 val_against = value->value.Float64;
                    comp_equals = (thing_val == val_against);
                    break;
                }
                case MTT_FLOAT: {
                    float32 val_against = value->value.Float;
                    comp_equals = (thing_val == val_against);
                    break;
                }
                default: { break; }
            }
            
            auto find_it = state->thing_update_time_for_value_checks.find(thing_id);
            if (comp_equals) {
                if (find_it != state->thing_update_time_for_value_checks.end()) {
                } else {
                    
                    is_valid_for_value_checks = true;
                    
                    state->thing_update_time_for_value_checks.insert({thing_id, state->update_count});
                    
                    Script* script = Script_lookup(state->to_invoke_script_ID);
                    if (script == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                    }
                    auto* script_instance = mtt::Script_Instance_from_script(script);
                    script_instance->rules_ref = rules;
                    mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, script_instance);
                }
            } else {
                if (find_it != state->thing_update_time_for_value_checks.end()) {
                    state->thing_update_time_for_value_checks.erase(thing_id);
                }
            }
        } else if (val_check[0].name == "exceedbegin") {
            
            auto* el = &val_check[1];
            mtt::Thing_ID thing_id = el->value.thing_id;
            mtt::Thing* thing = mtt::Thing_try_get(world, thing_id);
            if (mtt::thing_type_id(thing) != ARCHETYPE_NUMBER) {
                continue;
            }
            
            auto thing_val = number_get_value(thing);
            
            
            auto* value = &val_check[2];
            
            bool comp_exceeds = false;
            switch (value->value.type) {
                case MTT_FLOAT64: {
                    float64 val_against = value->value.Float64;
                    comp_exceeds = (thing_val > val_against);
                    break;
                }
                case MTT_FLOAT: {
                    float32 val_against = value->value.Float;
                    comp_exceeds = (thing_val > val_against);
                    break;
                }
                default: { break; }
            }
            
            auto find_it = state->thing_update_time_for_value_exceed_checks.find(thing_id);
            if (comp_exceeds) {
                if (find_it != state->thing_update_time_for_value_exceed_checks.end()) {
                } else {
                    
                    is_valid_for_value_checks = true;
                    
                    state->thing_update_time_for_value_exceed_checks.insert({thing_id, state->update_count});
                    
                    Script* script = Script_lookup(state->to_invoke_script_ID);
                    if (script == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                    }
                    auto* script_instance = mtt::Script_Instance_from_script(script);
                    script_instance->rules_ref = rules;
                    mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, script_instance);
                }
            } else {
                if (find_it != state->thing_update_time_for_value_exceed_checks.end()) {
                    state->thing_update_time_for_value_exceed_checks.erase(thing_id);
                }
            }
        } else if (val_check[0].name == "existbegin") {
            auto thing_id = mtt::Thing_ID_INVALID;
            {
                auto thing_val = 0.0;
                auto& name = val_check[1].name;
                
                {
                    for (usize r_i = 0; r_i < r.rules.size(); r_i += 1) {
                        auto& vars = r.vars_for_rule_idx[r_i];
                        auto v = ecs_rule_find_var(r.rules[r_i].rule, name.c_str());
                        if (v != mtt::Rule_Var_Handle_INVALID) {
                            
                            for (usize v_i = 0; v_i < vars.size(); v_i += 1) {
                                if (vars[v_i].var == v) {
                                    thing_val += vars[v_i].values.size();
                                }
                            }
                            
                        }
                    }
                    
                    auto* value = &val_check[2];
                    
                    bool comp_equals = false;
                    switch (value->value.type) {
                        case MTT_FLOAT64: {
                            float64 val_against = value->value.Float64;
                            comp_equals = (thing_val == val_against);
                            break;
                        }
                        case MTT_FLOAT: {
                            float32 val_against = value->value.Float;
                            comp_equals = (thing_val == val_against);
                            break;
                        }
                        default: { break; }
                    }
                    
                    auto find_it = state->thing_update_time_for_value_checks.find(thing_id);
                    if (comp_equals) {
                        if (find_it != state->thing_update_time_for_value_checks.end()) {
                        } else {
                            
                            is_valid_for_value_checks = true;
                            
                            state->thing_update_time_for_value_checks.insert({thing_id, state->update_count});
                            
                            Script* script = Script_lookup(state->to_invoke_script_ID);
                            if (script == nullptr) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            auto* script_instance = mtt::Script_Instance_from_script(script);
                            script_instance->rules_ref = rules;
                            mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, script_instance);
                        }
                    } else {
                        if (find_it != state->thing_update_time_for_value_checks.end()) {
                            state->thing_update_time_for_value_checks.erase(thing_id);
                        }
                    }
                }
            }
        } else if (val_check[0].name == "formbegin") {
            auto op_thing_id = mtt::Thing_ID_INVALID;
            {
                auto& name = val_check[1].name;
                auto thing_count = val_check[2].value.Float64;
                // put all results together
                std::vector<mtt::Any> aggregated = {};
                {
                    for (usize r_i = 0; r_i < r.rules.size(); r_i += 1) {
                        auto& vars = r.vars_for_rule_idx[r_i];
                        auto v = ecs_rule_find_var(r.rules[r_i].rule, name.c_str());
                        if (v != mtt::Rule_Var_Handle_INVALID) {
                            
                            for (usize v_i = 0; v_i < vars.size(); v_i += 1) {
                                if (vars[v_i].var == v) {
                                    //thing_val += vars[v_i].values.size();
                                    //aggregated.push_back();
                                    auto& vals = vars[v_i].values;
                                    
                                    
                                    // FIXME: only support the roots for now:
//                                    aggregated.insert(std::end(aggregated), std::begin(vals), std::end(vals));
                                    for (usize val_idx = 0; val_idx < vals.size(); val_idx += 1) {
                                        auto& val = vals[val_idx];
                                        
                                        mtt::Thing* th = mtt::Thing_try_get(world, val.thing_id);
                                        if (th == nullptr) {
                                            continue;
                                        }
                                        if (mtt::is_root(th)) {
                                            aggregated.push_back(val);
                                        }
                                    }
                                }
                            }
                            
                        }
                    }
                    
                    auto* value = &val_check[2];
                    
                    const mtt::String str_tgt = MTT_string_ref_to_cstring(val_check[3].value.String);
                    
                    
                //{
                    auto& dt_ctx = *dt::DrawTalk::ctx();
                    auto& named_things = dt_ctx.lang_ctx.dictionary.thing_to_word;
                    
                    auto& comp_structure_target = state->comp_structure_target;
                    
                    
                    struct Comp_Structure_Args {
                        Rule_Eval_Comp_Structure* tgt;
                        decltype(dt_ctx.lang_ctx.dictionary.thing_to_word)* named_things;
                        dt::DrawTalk* dt_ctx;
                        mtt::World* world;
                        mtt::Set_Stable<mtt::String> ignore;
                    };
                    
                    auto Rule_Eval_Comp_Structure_print = [](Rule_Eval_Comp_Structure* comp)
                    {
                        return;
                        
                        auto& vertex_list = comp->vertex_list;
                        auto& edge_list = comp->edge_list;
                        
                        MTT_print("%s\n", "(Rule_Eval_Comp_Structure) {");
                        MTT_print("    %s\n", "V:[");
                        for (usize i = 0; i < vertex_list.size(); i += 1) {
                            auto& v = vertex_list[i];
                            for (usize j = 0; j < v.label_entry.size(); j += 1) {
                                MTT_print("    %s\n", v.label_entry[j]->name.c_str());
                            }
                        }
                        MTT_print("    %s\n", "]");
                        MTT_print("    %s\n", "E:[");
                        for (usize i = 0; i < edge_list.size(); i += 1) {
                            auto& e = edge_list[i];
                            MTT_print("    (%llu:%s)->(%llu:%s)\n", e.in, e.in_label_key.c_str(), e.out, e.out_label_key.c_str());
                        }
                        MTT_print("    %s\n", "]");
                        MTT_print("%s\n", "}");
                    };


                    
                    const auto prepare_for_comparison = [](mtt::Thing_ID thing_root, Comp_Structure_Args& args) {
                        
                        
                        auto& in = args;
                        auto& named_things = *(in.named_things);
                        auto& tgt = *(in.tgt);
                        auto& dt_ctx = *(in.dt_ctx);
                        auto* world = in.world;
                        
                        breadth_first_traverse_thing_hierarchy_w_args(world, thing_root, [](mtt::World* world, mtt::Thing* thing, void* args) {
                            auto& in = *((Comp_Structure_Args*)args);
                            auto& named_things = *(in.named_things);
                            auto& tgt = *(in.tgt);
                            auto& dt_ctx = *(in.dt_ctx);
                            
                            mtt::Thing_ID thing_id = mtt::thing_id(thing);
                            //tgt.vertex_list.push_back()
                            Rule_Eval_Comp_Vertex vtx = {};
                            
                            auto make_key = [](std::vector<dt::Word_Dictionary_Entry*>& els) {
                                mtt::String out = {};
                                for (usize i = 0; i < els.size(); i += 1) {
                                    out += els[i]->name + ":";
                                }
                                return out;
                            };
                            
                            std::vector<dt::Word_Dictionary_Entry*> curr_w = {};
                            w_search(thing_id, curr_w, in.ignore);
                            vtx.label_entry = curr_w;
                            vtx.label_key = make_key(curr_w);
                            vtx.thing_ID = thing_id;
                            
                            
                            tgt.vertex_list.push_back(vtx);
                            mtt::Thing_ID parent_thing_id = mtt::get_parent_ID(thing);
                            if (mtt::Thing_ID_is_valid(parent_thing_id)) {
                                // TODO: undirected only for now
                                
                                std::vector<dt::Word_Dictionary_Entry*> parent_w = {};
                                w_search(parent_thing_id, parent_w, in.ignore);
                                
                                auto parent_key = make_key(parent_w);
                                auto current_key = make_key(curr_w);
                                
                                Rule_Eval_Comp_Edge e0 = {
                                    .in = parent_thing_id,
                                    .in_label_entry = parent_w,
                                    .in_label_key = parent_key,
                                    .out = thing_id,
                                    .out_label_entry = curr_w,
                                    .out_label_key = current_key,
                                };
                                tgt.edge_list.push_back(e0);
                                
                                Rule_Eval_Comp_Edge e1 = {
                                    .in = thing_id,
                                    .in_label_entry = curr_w,
                                    .in_label_key = current_key,
                                    .out = parent_thing_id,
                                    .out_label_entry = parent_w,
                                    .out_label_key = parent_key,
                                };
                                tgt.edge_list.push_back(e1);
                            }
                        }, &args);
                        
                        if ((false)) {
                            // TODO: ...
//                            breadth_first_traverse_thing_arrows_w_args(world, thing_root, [](mtt::World* world, mtt::Thing* thing, void* args) {
//                                auto& in = *((Comp_Structure_Args*)args);
//                                auto& named_things = *(in.named_things);
//                                auto& tgt = *(in.tgt);
//                                auto& dt_ctx = *(in.dt_ctx);
//                                
//                                mtt::Thing_ID thing_id = mtt::thing_id(thing);
//                                
//                                Rule_Eval_Comp_Vertex vtx = {};
//                                
//                                bool found = false;
//                                for (usize v = 0; v < tgt.vertex_list.size(); v += 1) {
//                                    if (tgt.vertex_list[v].thing_ID = thing_id) {
//                                        vtx = tgt.vertex_list[v];
//                                        found = true;
//                                        break;
//                                    }
//                                }
//                                
//                                
//                                
//                                std::vector<dt::Word_Dictionary_Entry*> curr_w = {};
//                                if (!found) {
//                                    auto make_key = [](std::vector<dt::Word_Dictionary_Entry*>& els) {
//                                        mtt::String out = {};
//                                        for (usize i = 0; i < els.size(); i += 1) {
//                                            out += els[i]->name + ":";
//                                        }
//                                        return out;
//                                    };
//                                    
//                                    w_search(thing_id, curr_w, in.ignore);
//                                    vtx.label_entry = curr_w;
//                                    vtx.label_key = make_key(curr_w);
//                                    vtx.thing_ID = thing_id;
//                                    tgt.vertex_list.push_back(vtx);
//                                }
//                                
//                                auto* arrows = arrow_links(world);
//                                
//                                
//                                std::vector<dt::Word_Dictionary_Entry*> parent_w = {};
//                                w_search(parent_thing_id, parent_w, in.ignore);
//                                
//                                auto parent_key = make_key(parent_w);
//                                auto current_key = make_key(curr_w);
//                                
//                                Rule_Eval_Comp_Edge e0 = {
//                                    .in = parent_thing_id,
//                                    .in_label_entry = parent_w,
//                                    .in_label_key = parent_key,
//                                    .out = thing_id,
//                                    .out_label_entry = curr_w,
//                                    .out_label_key = current_key,
//                                };
//                                tgt.edge_list.push_back(e0);
//                                
//                                Rule_Eval_Comp_Edge e1 = {
//                                    .in = thing_id,
//                                    .in_label_entry = curr_w,
//                                    .in_label_key = current_key,
//                                    .out = parent_thing_id,
//                                    .out_label_entry = parent_w,
//                                    .out_label_key = parent_key,
//                                };
//                                tgt.edge_list.push_back(e1);
//                                
//                                
//                            }, &args);
                        }
                        
                        std::sort(std::begin(tgt.vertex_list), std::end(tgt.vertex_list), [](Rule_Eval_Comp_Vertex& a, Rule_Eval_Comp_Vertex& b) {
                            return a.label_key <= b.label_key;
                        });
                        
                        
                        std::sort(std::begin(tgt.edge_list), std::end(tgt.edge_list), [&](Rule_Eval_Comp_Edge& a, Rule_Eval_Comp_Edge& b) {
                            if (a.in_label_key == b.in_label_key) {
                                return a.out_label_key <= b.out_label_key;
                            }
                            
                            return a.in_label_key <= b.in_label_key;
                        });
                        
                        
                        // remove duplicates
                        for (auto it = tgt.edge_list.begin(); it != tgt.edge_list.end();) {
                            if (it + 1 == tgt.edge_list.end()) {
                                break;
                            }
                            
                            auto& el_0 = *it;
                            auto& el_1 = *(it + 1);
                            if (el_0.in == el_1.in && 
                                el_0.out == el_1.out &&
                                el_0.in_label_key == el_1.in_label_key &&
                                el_0.out_label_key == el_1.out_label_key) {
                                it = tgt.edge_list.erase(it);
                            } else {
                                ++it;
                            }
                        }
                        
                        std::sort(std::begin(tgt.arrow_list), std::end(tgt.arrow_list), [&](Rule_Eval_Comp_Edge_Arrow& a, Rule_Eval_Comp_Edge_Arrow& b) {
                            if (a.in_label_key == b.in_label_key) {
                                return a.out_label_key <= b.out_label_key;
                            }
                            
                            return a.in_label_key <= b.in_label_key;
                        });
                        
                    };
                    
                    auto handle_unsatisfied = [&]() {
                        auto find_it = state->thing_update_time_for_value_checks.find(op_thing_id);
                        if (find_it != state->thing_update_time_for_value_checks.end()) {
                            state->thing_update_time_for_value_checks.erase(op_thing_id);
                        }
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    };
                    
                    if (comp_structure_target.empty()) {
                        comp_structure_target.emplace_back();
                        auto& tgt = comp_structure_target.back();
                        // init
                        mtt::Set<mtt::Thing_ID>* choices = {};
                        if (!Thing_find_preset(world, str_tgt, &choices)) {
                            return handle_unsatisfied();
                        }
                        // FIXME: currently only support single root, directed connection edges, single word
                        
                        Comp_Structure_Args args = {
                            .tgt = &tgt,
                            .named_things = &named_things,
                            .dt_ctx = &dt_ctx,
                            .world = world,
                            .ignore = {"thing", str_tgt},
                        };
                        

                        prepare_for_comparison(*choices->begin(), args);
                    }
                    
                    //
                //}
                    Rule_Eval_Comp_Structure_print(&comp_structure_target.back());
                    
                    bool success = false;
                    
                    if (aggregated.empty()) {
                        return handle_unsatisfied();
                    }
                    
                    
                    
                    for (usize cand_i = 0 ; cand_i < aggregated.size(); cand_i += 1) {
                        
                        // prepare each candidate for structural comparison
                        
                        Rule_Eval_Comp_Structure cand_comp = {};
                        
                        Comp_Structure_Args args = {
                            .tgt = &cand_comp,
                            .named_things = &named_things,
                            .dt_ctx = &dt_ctx,
                            .world = world,
                            .ignore = {"thing", str_tgt},
                        };
                        
                        prepare_for_comparison(aggregated[cand_i].thing_id, args);
                        
                        Rule_Eval_Comp_Structure_print(&cand_comp);
                        
                        
                        {
                            auto& TGT = comp_structure_target.back();
                            auto& v_tgt = TGT.vertex_list;
                            auto& e_tgt = TGT.edge_list;
                            
                            auto& v_against = cand_comp.vertex_list;
                            auto& e_against = cand_comp.edge_list;
                            
                            // NOTE: current idea would be to find subset in against that fulfills target
                            
                            // check for vertices
                            
                            
                            
                            if (v_tgt.size() > v_against.size()) {
                                return handle_unsatisfied();
                            }
                            
                            // compare vertices
                            
                            // for each target vertex
                            for (usize v = 0; v < v_tgt.size(); v += 1) {
                                auto& V_tgt_el = v_tgt[v];
                                auto& v_comp_el = v_against[v];
                                // for each comp vertex
                                {
                                    if (!label_entry_tgt_contains_comp(V_tgt_el.label_entry, v_comp_el.label_entry)) {
                                        return handle_unsatisfied();
                                    }
                                }
                            }
                            
                            // compare edges
                            for (usize e = 0; e < e_tgt.size(); e += 1) {
                                auto& E_tgt_el = e_tgt[e];
                                auto& E_comp_el = e_against[e];
                                
                                {
                                    if (!label_entry_tgt_contains_comp(E_tgt_el.in_label_entry, E_comp_el.in_label_entry)) {
                                        return handle_unsatisfied();
                                    }
                                }
                                
                                {
                                    if (!label_entry_tgt_contains_comp(E_tgt_el.out_label_entry, E_comp_el.out_label_entry)) {
                                        return handle_unsatisfied();
                                    }
                                }
                                
                                
                            }
                            
                            // TODO: compare arrow edges
                            
                            //MTT_print("%s\n", "Success!");
                            auto find_it = state->thing_update_time_for_value_checks.find(op_thing_id);
                            if (find_it != state->thing_update_time_for_value_checks.end()) {
                            } else {
                                state->thing_update_time_for_value_checks.insert({op_thing_id, state->update_count});
                                
                                Script* script = Script_lookup(state->to_invoke_script_ID);
                                if (script == nullptr) {
                                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                                }
                                auto* script_instance = mtt::Script_Instance_from_script(script);
                                script_instance->rules_ref = rules;
                                mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, script_instance);
                            }
                        }
                    }
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                    
                    
                    
                    // TODO: ...
                    if constexpr ((false)) 
                    {
//
//                        bool comp_equals = false;
//                        switch (value->value.type) {
//                            case MTT_FLOAT64: {
//                                float64 val_against = value->value.Float64;
//                                comp_equals = (thing_val == val_against);
//                                break;
//                            }
//                            case MTT_FLOAT: {
//                                float32 val_against = value->value.Float;
//                                comp_equals = (thing_val == val_against);
//                                break;
//                            }
//                            default: { break; }
//                        }
//
//                        auto find_it = state->thing_update_time_for_value_checks.find(thing_id);
//                        if (comp_equals) {
//                            if (find_it != state->thing_update_time_for_value_checks.end()) {
//                            } else {
//
//                                is_valid_for_value_checks = true;
//
//                                state->thing_update_time_for_value_checks.insert({thing_id, state->update_count});
//
//                                Script* script = Script_lookup(state->to_invoke_script_ID);
//                                if (script == nullptr) {
//                                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
//                                }
//                                auto* script_instance = mtt::Script_Instance_from_script(script);
//                                script_instance->rules_ref = rules;
//                                mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, script_instance);
//                            }
//                        } else {
//                            if (find_it != state->thing_update_time_for_value_checks.end()) {
//                                state->thing_update_time_for_value_checks.erase(thing_id);
//                            }
//                        }
                    }
                }
            }
        }
    }
    if (!r.value_clauses.empty()) {
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }
    if (!is_valid || min_result_count == ULLONG_MAX) {
        
    } else {
        
        // iterate through each rule's variables again and instantiate scripts for each result
        for (usize i = 0; i < min_result_count; i += 1) {
            Script* script = Script_lookup(state->to_invoke_script_ID);
            
            if (script == nullptr) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
            }
            //Script_print_as_things(script);
            
            auto* script_instance = mtt::Script_Instance_from_script(script);
            script_instance->rules_ref = rules;
            
            for (usize r_idx = 0; r_idx < r.rules.size(); r_idx += 1) {
                auto& vars = r.vars_for_rule_idx[r_idx];
                for (usize v_idx = 0; v_idx < vars.size(); v_idx += 1) {
                    auto& var = vars[v_idx];
                    
                    Rule_Var_Record_One_Result result_out = {
                        .var = var.var,
                        .idx = var.idx,
                        .value = var.values[i],
                    };
                    
                    script_instance->rule_vars.push_back(result_out);
                    
                }
            }
            //MTT_print("%s\n", "starting script");
            mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, script_instance);
        }
    }
    
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}


MTT_DEFINE_INITIALIZER(system_element)
{
    mtt::set_is_actor(archetype);
    
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_VECTOR3].alloc_byte_size);
    
    add_field({world, fields, ports, logic, "velocity",     MTT_VECTOR3, 0}, builder);
    add_field({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0}, builder);
    add_field({world, fields, ports, logic, "position",     MTT_VECTOR3, 0}, builder);
    
    MTT_ADD_FIELD("acceleration_local", MTT_VECTOR3, MTT_NONE);
    MTT_ADD_FIELD("velocity_local",     MTT_VECTOR3, MTT_NONE);
    
    
    Thing_add_in_port({world, fields, ports, logic,  "color",    MTT_COLOR_RGBA, 0, {{-0.5f, -0.25f, 0.0f}}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "velocity", MTT_VECTOR3, 0, {{-0.5f, 0.0f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "acceleration", MTT_VECTOR3, 0, {{-0.5f, 0.25f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "position", MTT_VECTOR3, 0, {{-0.5f, -0.25f, 0.0f}}, nullptr });
    
    Thing_add_in_port({world, fields, ports, logic,  "option", MTT_INT64, 0, {{-0.5f, -0.25f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "option:flags", MTT_INT64, 0, {{-0.5f, -0.25f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "destroy", MTT_ANY, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    
    Thing_add_in_port({world, fields, ports, logic,  "scale", MTT_VECTOR3, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "color_factor", MTT_VECTOR4, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "color_addition", MTT_VECTOR4, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    
    Thing_add_in_port({world, fields, ports, logic,  "rotation", MTT_VECTOR3, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "transform", MTT_MATRIX4, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic,  "angle_radians", MTT_FLOAT, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic,  "angle_degrees", MTT_FLOAT, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    
    
    Thing_add_out_port({world, fields, ports, logic, "top",    MTT_VECTOR3, 0, {{0.0f, -0.5f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "right",  MTT_VECTOR3, 0, {{0.5f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "bottom", MTT_VECTOR3, 0, {{0.0f, 0.5f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "left",   MTT_VECTOR3, 0, {{-0.5f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "center", MTT_VECTOR3, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    
    Thing_add_out_port({world, fields, ports, logic, "position", MTT_VECTOR3, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "position_x", MTT_FLOAT, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "position_y", MTT_FLOAT, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "position_y_negated", MTT_FLOAT, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "position_z", MTT_FLOAT, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    
    Thing_add_out_port({world, fields, ports, logic, "scale", MTT_VECTOR3, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    
    Thing_add_out_port({world, fields, ports, logic, "transform", MTT_MATRIX4, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    
    
    
    
    auto set_position = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        Message* msg = (Message*)state;
        
        *mtt::access<vec3>(args->caller, "position") = *(vec3*)(args->input);
        
        return Procedure_Return_Type();
    });
    auto set_velocity = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        Message* msg = (Message*)state;
        
        *mtt::access<vec3>(args->caller, "velocity") = *(vec3*)(args->input);
        
        return Procedure_Return_Type();
    });
    auto stop_moving = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        
        *mtt::access<vec3>(args->caller, "velocity") = vec3(0.0f);
        
        return Procedure_Return_Type();
    });
    auto get_center = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);

            
            Thing* thing = args->caller;
            World* world = thing->world();
            args->output.type = MTT_VECTOR3;

            
            auto* center_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *center_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 center_position = vec3((br + tl) / 2.0f, 0.0f);
                    
                    *center_out = center_position;
                    
                    break;
                }
                default: {
                    *center_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    auto get_top_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.y -= half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, tl.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    auto get_bottom_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.y +=  2 * half_extent.y * mult;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, tl.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    auto get_beside_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        
        mtt::Message* msg = static_cast<mtt::Message*>(args->input);
        
        args->output.type = MTT_VECTOR3;

        Thing* thing = args->caller;

        World* world = thing->world();
        
        Thing* sender = nullptr;
        if (world->Thing_try_get(msg->sender, &sender)) {
            
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;

            
            auto* tgt_pos = mtt::access<vec3>(thing, "position");
            auto* src_pos = mtt::access<vec3>(sender, "position");
            if (src_pos->x < tgt_pos->x) {
                msg->selector = mtt::string("left_with_offset");
                msg->input_value.set_Float(mult);
                selector_invoke(thing, msg);
                args->output.set_Vector3(msg->output_value.Vector3);
            } else {
                msg->selector = mtt::string("right_with_offset");
                msg->input_value.set_Float(mult);
                selector_invoke(thing, msg);
                args->output.set_Vector3(msg->output_value.Vector3);
            }
            MTT_BP();
        }
        
        
        return true;
    });
    
    auto get_over_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        
        mtt::Message* msg = static_cast<mtt::Message*>(args->input);
        
        args->output.type = MTT_VECTOR3;

        Thing* thing = args->caller;

        World* world = thing->world();
        
        Thing* sender = nullptr;
        if (world->Thing_try_get(msg->sender, &sender)) {
            
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;

            
            auto* tgt_pos = mtt::access<vec3>(thing, "position");
            auto* src_pos = mtt::access<vec3>(sender, "position");
            if (src_pos->x > tgt_pos->x) {
                msg->selector = mtt::string("left_with_offset");
                msg->input_value.set_Float(mult);
                selector_invoke(thing, msg);
                args->output.set_Vector3(msg->output_value.Vector3);
            } else {
                msg->selector = mtt::string("right_with_offset");
                msg->input_value.set_Float(mult);
                selector_invoke(thing, msg);
                args->output.set_Vector3(msg->output_value.Vector3);
            }
            MTT_BP();
        }
        
        
        return true;
    });
    
    auto get_left_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.x -= 2 * half_extent.x;
                        offset.y += half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, tl.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    auto get_right_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;

            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.x += 2 * half_extent.x;
                        offset.y += half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, tl.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    
    
    add_selector(archetype, "position", set_position);
    add_selector(archetype, "velocity", set_velocity);
    add_selector(archetype, "stop",     stop_moving);
    add_selector(archetype, "center",   get_center);
    add_selector(archetype, "top_with_offset",     get_top_with_offset);
    add_selector(archetype, "bottom_with_offset",  get_bottom_with_offset);
    add_selector(archetype, "left_with_offset",    get_left_with_offset);
    add_selector(archetype, "right_with_offset",   get_right_with_offset);
    add_selector(archetype, "beside_with_offset",   get_beside_with_offset);
    add_selector(archetype, "over_with_offset",   get_over_with_offset);
    //    add_selector(archetype, "set_movement_option", MTT_PROC(
    //        Thing* thing = args->caller;
    //        World* world = thing->world();
    //        args->output.type = MTT_NONE;
    //
    //        Message* msg = (Message*)args->input;
    //        auto movement_flag = (uint64)msg->input_value.Int64;
    //        thing->logic.option_flags = modify_bit(thing->logic.option_flags, 0, movement_flag);
    //
    //        return Procedure_Return_Type();
    //    ));
    MTT_add_selector(archetype, "set_movement_option", {
        Thing* thing = args->caller;
        World* world = thing->world();
        args->output.type = MTT_NONE;
        
        Message* msg = (Message*)args->input;
        auto movement_flag = (uint64)msg->input_value.Int64;
        thing->logic.option_flags = modify_bit(thing->logic.option_flags, 0, movement_flag);
        // TODO: switch to using
        //mtt::set_movement_mode(thing, movement_flag);
        
        return Procedure_Return_Type();
    }, nullptr);
    
    archetype->message_handler = [](auto* msg) {
        switch (msg->type) {
        default: {
            break;
        }
        }
    };
    
    archetype->logic.proc = freehand_sketch_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->eval_priority = 1;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        
        
    };
}
MTT_DEFINE_LOGIC_PROCEDURE(system_element)
{
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}


MTT_DEFINE_INITIALIZER(move_away)
{
    MTT_ADD_FIELD_T("state", Move_State);
    
    Thing_add_in_port({world, fields, ports, logic, "velocity", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selector", MTT_TEXT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selectors", MTT_LIST, MTT_SCRIPT_PROPERTY,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "velocity_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    
    Thing_add_in_port({world, fields, ports, logic, "source", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
}

MTT_DEFINE_LOGIC_PROCEDURE(move_away)
{
    auto* state = mtt::access<Move_State>(thing, "state");
    
    auto IN_velocity             = get_in_port(thing, input, "velocity");
    vec3 velocity = (IN_velocity.status == PORT_STATUS_OK) ? IN_velocity.value.out.Vector3 : m::vec3_zero();
    
    auto IN_acceleration         = get_in_port(thing, input, "acceleration");
    vec3 acceleration = (IN_acceleration.status == PORT_STATUS_OK) ? IN_acceleration.value.out.Vector3 : vec3(1.0f, 1.0f, 1.0f);
    
    auto IN_velocity_local       = get_in_port(thing, input, "velocity_local");
    vec3 velocity_local = (IN_velocity_local.status == PORT_STATUS_OK) ? IN_velocity_local.value.out.Vector3 : m::vec3_zero();
    
    auto IN_acceleration_local   = get_in_port(thing, input, "acceleration_local");
    vec3 acceleration_local = (IN_acceleration_local.status == PORT_STATUS_OK) ? IN_acceleration_local.value.out.Vector3 : m::vec3_zero();
    
    
    auto IN_velocity_magnitude       = get_in_port(thing, input, "velocity_magnitude");
    float32 velocity_magnitude = (IN_velocity_magnitude.status == PORT_STATUS_OK) ? IN_velocity_magnitude.value.out.Float : 1.0f;
    auto IN_velocity_local_magnitude       = get_in_port(thing, input, "velocity_local_magnitude");
    float32 velocity_local_magnitude = (IN_velocity_local_magnitude.status == PORT_STATUS_OK) ? IN_velocity_local_magnitude.value.out.Float : 1.0f;
    
    auto IN_velocity_direction       = get_in_port(thing, input, "velocity_direction");
    vec3 velocity_direction = (IN_velocity_direction.status == PORT_STATUS_OK) ? IN_velocity_direction.value.out.Vector3 : m::vec3_zero();
    auto IN_velocity_local_direction       = get_in_port(thing, input, "velocity_local_direction");
    vec3 velocity_local_direction = (IN_velocity_local_direction.status == PORT_STATUS_OK) ? IN_velocity_local_direction.value.out.Vector3 : m::vec3_zero();
    
    auto IN_source_selector = get_in_port(thing, input, "source_selector");
    
    
    auto IN_source_selectors = get_in_port(thing, input, "source_selectors");
    
    auto OUT_is_done = get_out_port(thing, "is_done");
    OUT_is_done.value.out.set_Boolean(false);
    
    
    const float32 radius_TEMP = 512.0f;
    
    
    struct State {
        mtt::Thing* src_thing = nullptr;
        mtt::Thing* tgt_thing = nullptr;
        vec3 tgt_vec3 = m::vec3_zero();
        MTT_TYPE tgt_type = MTT_NONE;
        float32 magnitude = 0.0f;
        vec3 velocity = m::vec3_zero();
        vec3 acceleration = m::vec3_zero();
        MTT_String_Ref dir_selector = {};
        bool has_direction_selector = false;
    } st;
    
    st.magnitude = velocity_magnitude;
    
    st.acceleration = acceleration;
    if (IN_source_selector.status == PORT_STATUS_OK) {
        st.dir_selector = IN_source_selector.value.out.String;
        st.has_direction_selector = true;
    }
    
    auto IN_source = get_in_port(thing, input, "source");
    if (IN_source.status != PORT_STATUS_OK) {
        //return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
        OUT_is_done.value.out.set_Boolean(true);
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }
    auto IN_target = get_in_port(thing, input, "target");
    if (IN_target.status == PORT_STATUS_OK) {
        st.tgt_type = IN_target.value.out.type;
        st.tgt_thing = mtt::Thing_try_get(world, IN_target.value.out.thing_id);
        if (st.tgt_thing == nullptr) {
            OUT_is_done.value.out.set_Boolean(true);
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
        }
    }
    
    Script_Property_List* all_sources = nullptr;
    bool loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = false;
    usize loop_idx = 0;
    switch (IN_source.value.out.type) {
        default: {
            st.src_thing = mtt::Thing_try_get(world, IN_source.value.out.thing_id);
            if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            break;
        }
        case MTT_LIST: {
            all_sources = (Script_Property_List*)IN_source.value.out.List;
            if (all_sources->empty()) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            st.src_thing = mtt::Thing_try_get(world, (*all_sources)[loop_idx].value.thing_id);
            if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = true;
            break;
        }
    }
    
    do {
        vec3* V = mtt::access<vec3>(st.src_thing, "velocity");
        vec3* A = mtt::access<vec3>(st.src_thing, "acceleration");
        if (A != nullptr) {
            acceleration += *A;
        }
        
        
        switch (IN_target.value.out.type) {
            case MTT_THING: {
                st.tgt_type = MTT_THING;
                
                
                vec3* tgt_pos_ptr = mtt::access<vec3>(st.tgt_thing, "position");
                vec3 tgt_pos = *tgt_pos_ptr;
                vec3* src_pos_ptr = mtt::access<vec3>(st.src_thing, "position");
                vec3 src_pos = *src_pos_ptr;
                
                vec3* vel_current_ptr = mtt::access<vec3>(st.src_thing, "velocity");
                vec3  vel_current = *vel_current_ptr;
                vec3  vel_prev = vel_current;
                
                vec3 diff = tgt_pos - src_pos;
                vec3 norm = (diff == m::vec3_zero()) ? m::vec3_zero() : m::normalize(diff);
                vel_current -= diff * (float32)MTT_TIMESTEP * acceleration;
                
                
                float32 mag_current = vel_current == m::vec3_zero() ? 0.0f : m::length(vel_current);
                {
                    float32 mag_max     = velocity_magnitude;
                    vec3 dir = (vel_current == m::vec3_zero()) ? m::vec3_zero() : m::normalize(vel_current);
                    if (mag_current > mag_max) {
                        vel_current = dir * mag_max;
                        mag_current = mag_max;
                    }
                }
                
                vec3 pos_future = src_pos + ((float32)mtt::MOVEMENT_SCALE * vel_current * (float32)MTT_TIMESTEP);
                vec3 diff_future = tgt_pos - pos_future;
                auto norm_future = (diff_future == m::vec3_zero()) ? m::vec3_zero() : m::normalize(diff_future);
                auto s_future = m::sign(norm_future);
                auto s_curr = m::sign(norm);
                MTT_UNUSED(s_curr);
                if ((m::dist_squared(tgt_pos, pos_future) > radius_TEMP * radius_TEMP)) {
                    
//                    Message msg;
//                    msg.sender   = st.src_thing->id;
//                    msg.selector = mtt::string("center");
//                    selector_invoke(st.tgt_thing, &msg);
//                    ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
                    
//                    vec3 tgt_velocity = *mtt::access<vec3>(st.tgt_thing, "velocity");
//                    
//                    if (tgt_velocity != m::vec3_zero()) {
//                        float32 MAG = m::length(tgt_velocity);
//                        if (MAG <= mag_current) {
//                            *vel_current_ptr = MAG * m::normalize(tgt_velocity);
//                            mtt::Thing_set_position(st.src_thing, msg.output_value.Vector3);
//                        }
//                    } else {
//                        *vel_current_ptr = vec3(0.0f, 0.0f, 0.0f);
//                        mtt::Thing_set_position(st.src_thing, msg.output_value.Vector3);
//                        
//                    }
                    if (!state->prev_was_done) {
                        state->prev_was_done = true;
                        *vel_current_ptr = vec3(0.0f, 0.0f, 0.0f);
                    }
                    //
                    //                *vel_current_ptr = (tgt_velocity == m::vec3_zero()) ? m::vec3_zero() : mag_current * m::normalize(tgt_velocity);
                    
                    
                    //OUT_is_done.value.out.set_Boolean(true);
                } else {
                    state->prev_was_done = false;
                    *vel_current_ptr = vel_current;
                }
                
                
                if (m::sign(*vel_current_ptr) != m::sign(vel_prev)) {
                    
                    auto* rep = mtt::rep(st.src_thing);
                    
                    if (vel_current_ptr->x > 0) {
                        if (heading_direction_x(rep) < 0) {
                            mtt::flip_left_right(st.src_thing);
                        }
                    } else if (vel_current_ptr->x < 0) {
                        if (heading_direction_x(rep) > 0) {
                            mtt::flip_left_right(st.src_thing);
                        }
                    }
                }
                
                break;
            }
                // treat as direction
            case MTT_VECTOR3: {
                
                
                break;
            }
            case MTT_STRING:
                MTT_FALLTHROUGH;
            default: {
                
                
                if (IN_source_selectors.status == PORT_STATUS_OK) {
                    st.tgt_type = MTT_VECTOR3;
                    
                    Script_Property_List* all = (Script_Property_List*)IN_source_selectors.value.out.List;
                    for (auto& sel : *all) {
                        vec3 dir = m::vec3_zero();
                        mtt::String dir_sel = sel.value.get_cstring();
                        if (dir_sel == ARG_dir_up) {
                            dir = vec3(0.0f, -1.0f, 0.0f);
                        } else if (dir_sel == ARG_dir_down) {
                            dir = vec3(0.0f, 1.0f, 0.0f);
                        } else if (dir_sel == ARG_dir_left) {
                            dir = vec3(-1.0f, 0.0f, 0.0f);
                        } else if (dir_sel == ARG_dir_right) {
                            dir = vec3(1.0f, 0.0f, 0.0f);
                        } else if (dir_sel == ARG_dir_none) {
                            //dir = vec3(0.0f, 0.0f, 0.0f);
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                        }
                        
                        float32 prev_sign = m::sign(V->x);
                        vec3 force = st.magnitude * dir * acceleration.y;
                        *V += force * (float32)MTT_TIMESTEP;
                        
                        float32 m_adjust = m::length(*V);
                        if (m_adjust > velocity_magnitude) {
                            dir = m::normalize(*V);
                            *V = velocity_magnitude * dir;
                        }
                        if (m::sign(V->x) != prev_sign) {
                            
                            auto* rep = mtt::rep(st.src_thing);
                            
                            if (V->x > 0) {
                                if (heading_direction_x(rep) < 0) {
                                    mtt::flip_left_right(st.src_thing);
                                }
                            } else if (V->x < 0) {
                                if (heading_direction_x(rep) > 0) {
                                    mtt::flip_left_right(st.src_thing);
                                }
                            }
                        }
                        
                    }
                } else if (st.has_direction_selector) {
                    st.tgt_type = MTT_VECTOR3;
                    
                    mtt::String dir_sel = MTT_string_ref_to_cstring(st.dir_selector);
                    
                    vec3 dir = m::vec3_zero();
                    if (dir_sel == ARG_dir_up) {
                        dir = vec3(0.0f, -1.0f, 0.0f);
                    } else if (dir_sel == ARG_dir_down) {
                        dir = vec3(0.0f, 1.0f, 0.0f);
                    } else if (dir_sel == ARG_dir_left) {
                        dir = vec3(-1.0f, 0.0f, 0.0f);
                    } else if (dir_sel == ARG_dir_right) {
                        dir = vec3(1.0f, 0.0f, 0.0f);
                    } else if (dir_sel == ARG_dir_none) {
                        //dir = vec3(0.0f, 0.0f, 0.0f);
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                    }
                    
                    
                    float32 prev_sign = m::sign(V->x);
                    vec3 force = st.magnitude * dir;
                    *V += force * (float32)MTT_TIMESTEP;
                    
                    float32 m_adjust = m::length(*V);
                    if (m_adjust > velocity_magnitude) {
                        dir = m::normalize(*V);
                        *V = velocity_magnitude * dir;
                    }
                    if (m::sign(V->x) != prev_sign) {
                        
                        auto* rep = mtt::rep(st.src_thing);
                        if (heading_direction_x(rep) < 0) {
                            mtt::flip_left_right(st.src_thing);
                        }
                        
                        if (heading_direction_x(rep) > 0) {
                            mtt::flip_left_right(st.src_thing);
                        }
                    }
                    
                } else {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                }
                
                break;
            }
        }
        
        
        if (loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP) {
            loop_idx += 1;
            if (all_sources->size() > loop_idx) {
                st.src_thing = mtt::Thing_try_get(world, (*all_sources)[loop_idx].value.thing_id);
                if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                }
            } else {
                loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = false;
            }
        }
    } while (loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP);

    return true;
}



MTT_DEFINE_INITIALIZER(rotate_around)
{
    MTT_ADD_FIELD_T("state", Rotate_State);
    
    Thing_add_in_port({world, fields, ports, logic, "velocity", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selector", MTT_TEXT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selectors", MTT_LIST, MTT_SCRIPT_PROPERTY,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "velocity_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    
    Thing_add_in_port({world, fields, ports, logic, "source", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "angle_radians", MTT_BOOLEAN, 0,
        {(ALIGN_RIGHT * 0.5f)}, nullptr
    });
    
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "angle_radians", MTT_BOOLEAN, 0,
        {(ALIGN_RIGHT * 0.5f)}, nullptr
    });
    
    
//    archetype->logic.proc = rotate_procedure;
//    archetype->logic.option = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
//        auto* direction = mtt::access<float32>(thing, "direction");
//        *direction = 1.0f;
//        auto* axes = mtt::access<vec3>(thing, "axes");
//        *axes = vec3(0.0f, 0.0f, 1.0f);
    };
}

MTT_DEFINE_LOGIC_PROCEDURE(rotate_around)
{
    auto* state = mtt::access<Rotate_State>(thing, "state");
    
    auto IN_velocity             = get_in_port(thing, input, "velocity");
    vec3 velocity = (IN_velocity.status == PORT_STATUS_OK) ? IN_velocity.value.out.Vector3 : m::vec3_zero();
    
    auto IN_acceleration         = get_in_port(thing, input, "acceleration");
    vec3 acceleration = (IN_acceleration.status == PORT_STATUS_OK) ? IN_acceleration.value.out.Vector3 : vec3(1.0f, 1.0f, 1.0f);
    
    auto IN_velocity_local       = get_in_port(thing, input, "velocity_local");
    vec3 velocity_local = (IN_velocity_local.status == PORT_STATUS_OK) ? IN_velocity_local.value.out.Vector3 : m::vec3_zero();
    
    auto IN_acceleration_local   = get_in_port(thing, input, "acceleration_local");
    vec3 acceleration_local = (IN_acceleration_local.status == PORT_STATUS_OK) ? IN_acceleration_local.value.out.Vector3 : m::vec3_zero();
    
    
    auto IN_velocity_magnitude       = get_in_port(thing, input, "velocity_magnitude");
    float32 velocity_magnitude = (IN_velocity_magnitude.status == PORT_STATUS_OK) ? IN_velocity_magnitude.value.out.Float : 1.0f;
    auto IN_velocity_local_magnitude       = get_in_port(thing, input, "velocity_local_magnitude");
    float32 velocity_local_magnitude = (IN_velocity_local_magnitude.status == PORT_STATUS_OK) ? IN_velocity_local_magnitude.value.out.Float : 1.0f;
    
    auto IN_velocity_direction       = get_in_port(thing, input, "velocity_direction");
    vec3 velocity_direction = (IN_velocity_direction.status == PORT_STATUS_OK) ? IN_velocity_direction.value.out.Vector3 : m::vec3_zero();
    auto IN_velocity_local_direction       = get_in_port(thing, input, "velocity_local_direction");
    vec3 velocity_local_direction = (IN_velocity_local_direction.status == PORT_STATUS_OK) ? IN_velocity_local_direction.value.out.Vector3 : m::vec3_zero();
    
    auto IN_source_selector = get_in_port(thing, input, "source_selector");
    
    
    auto IN_source_selectors = get_in_port(thing, input, "source_selectors");
    
    auto OUT_is_done = get_out_port(thing, "is_done");
    OUT_is_done.value.out.set_Boolean(false);
    
//    struct Rotate_Around_Deferred_Args {
//        usize test = 0;
//        mtt::World* world;
//    };
//    auto& allocator = *mtt::buckets_allocator(world);
//    auto* deferred_args = mem::alloc_init<Rotate_Around_Deferred_Args>(&allocator);
//    deferred_args->test = world->time_seconds;
//    deferred_args->world = world;
//
//    mtt::send_system_message_deferred_after_script_eval(&world->message_passer, mtt::MTT_NONE, mtt::id(thing), deferred_args, mtt::Procedure_make([](void* state, mtt::Procedure_Input_Output* io) {
//        
//        Rotate_Around_Deferred_Args* args = static_cast<Rotate_Around_Deferred_Args*>(mtt::Message_contents(mtt::Message_from(io)));
//        
//        //MTT_log_debug("script=[%s\n] time=[%llu]\n", "rotate around", args->test);
//        // TODO
//        
//        mem::deinit_deallocate<Rotate_Around_Deferred_Args>(mtt::buckets_allocator(args->world), args);
//        return mtt::Procedure_Return_Type();
//    }));
    
    struct State {
        mtt::Thing* src_thing = nullptr;
        mtt::Thing* tgt_thing = nullptr;
        vec3 tgt_vec3 = m::vec3_zero();
        MTT_TYPE tgt_type = MTT_NONE;
        float32 magnitude = 0.0f;
        vec3 velocity = m::vec3_zero();
        vec3 acceleration = m::vec3_zero();
        MTT_String_Ref dir_selector = {};
        bool has_direction_selector = false;
    } st;
    
    st.magnitude = velocity_magnitude;
    
    st.acceleration = acceleration;
    if (IN_source_selector.status == PORT_STATUS_OK) {
        st.dir_selector = IN_source_selector.value.out.String;
        st.has_direction_selector = true;
    }
    
    auto IN_source = get_in_port(thing, input, "source");
    if (IN_source.status != PORT_STATUS_OK) {
        //return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
        OUT_is_done.value.out.set_Boolean(true);
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }
    
    auto IN_target = get_in_port(thing, input, "target");
    if (IN_target.status == PORT_STATUS_OK) {
        st.tgt_type = IN_target.value.out.type;
        st.tgt_thing = mtt::Thing_try_get(world, IN_target.value.out.thing_id);
        if (st.tgt_thing == nullptr) {
            OUT_is_done.value.out.set_Boolean(true);
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
        }
    }
    
    Script_Property_List* all_sources = nullptr;
    bool loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = false;
    usize loop_idx = 0;
    switch (IN_source.value.out.type) {
        default: {
            st.src_thing = mtt::Thing_try_get(world, IN_source.value.out.thing_id);
            if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            break;
        }
        case MTT_LIST: {
            all_sources = (Script_Property_List*)IN_source.value.out.List;
            if (all_sources->empty()) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            st.src_thing = mtt::Thing_try_get(world, (*all_sources)[loop_idx].value.thing_id);
            if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = true;
            break;
        }
    }
    
    if (!state->is_init) {
        state->is_init = true;
        //state->original_pose = mtt::rep(st.src_thing)->pose_transform;
        state->global_transform = mat4(1.0f);
    }
    
    do {
        vec3* V = mtt::access<vec3>(st.src_thing, "velocity");
        vec3* A = mtt::access<vec3>(st.src_thing, "acceleration");
        if (A != nullptr) {
            acceleration += *A;
        }
        
        
        switch (IN_target.value.out.type) {
            case MTT_THING: {
                st.tgt_type = MTT_THING;
                
                float32 dir = 1;
                if (IN_source_selectors.status == PORT_STATUS_OK) {
                    Script_Property_List* all = (Script_Property_List*)IN_source_selectors.value.out.List;
                    for (auto& sel : *all) {
                        mtt::String dir_sel = sel.value.get_cstring();
                        if (dir_sel == ARG_dir_up) {
                            dir = -1;
                        } else if (dir_sel == ARG_dir_down) {
                            dir = 1;
                        } else if (dir_sel == ARG_dir_left) {
                            dir = -1;
                        } else if (dir_sel == ARG_dir_right) {
                            dir = 1;
                        } else if (dir_sel == ARG_dir_clockwise) {
                            dir = 1;
                        } else if (dir_sel == ARG_dir_anticlockwise) {
                            dir = -1;
                        }
                        else if (dir_sel == ARG_dir_none) {
                            //dir = vec3(0.0f, 0.0f, 0.0f);
                            dir = 1;
                        }
                    }
                        
                } else if (st.has_direction_selector) {
                    mtt::String dir_sel = MTT_string_ref_to_cstring(st.dir_selector);
                    if (dir_sel == ARG_dir_up) {
                        dir = -1;
                    } else if (dir_sel == ARG_dir_down) {
                        dir = 1;
                    } else if (dir_sel == ARG_dir_left) {
                        dir = -1;
                    } else if (dir_sel == ARG_dir_right) {
                        dir = 1;
                    } else if (dir_sel == ARG_dir_clockwise) {
                        dir = 1;
                    } else if (dir_sel == ARG_dir_anticlockwise) {
                        dir = -1;
                    }
                    else if (dir_sel == ARG_dir_none) {
                        dir = 1;
                    }
                }
                
                
                vec3* tgt_pos_ptr = mtt::access<vec3>(st.tgt_thing, "position");
                vec3 tgt_pos = *tgt_pos_ptr;
                vec3* src_pos_ptr = mtt::access<vec3>(st.src_thing, "position");
                vec3 src_pos = *src_pos_ptr;
                
                vec3* vel_current_ptr = mtt::access<vec3>(st.src_thing, "velocity");
                vec3  vel_current = *vel_current_ptr;
                vec3  vel_prev = vel_current;
                
                vec3 diff = tgt_pos - src_pos;
                vec3 norm = (diff == m::vec3_zero()) ? m::vec3_zero() : m::normalize(diff);
                vel_current += diff * (float32)MTT_TIMESTEP * acceleration;
                
#if 0
                float32 mag_current = vel_current == m::vec3_zero() ? 0.0f : m::length(vel_current);
                
                {
                    float32 mag_max     = velocity_magnitude;
                    vec3 dir = (vel_current == m::vec3_zero()) ? m::vec3_zero() : m::normalize(vel_current);
                    if (mag_current > mag_max) {
                        vel_current = dir * mag_max;
                        mag_current = mag_max;
                    }
                }
#else
                float32 mag_max = velocity_magnitude;
                vel_current = ((vel_current == m::vec3_zero()) ? m::vec3_zero() : m::normalize(vel_current)) * mag_max;
                float32 mag_current = mag_max;
                
#endif
                
                {
                    auto* rep = mtt::rep(st.src_thing);
                    auto* tgt_rep = mtt::rep(st.tgt_thing);
                    
                    mat4* rot_global = &state->global_transform;
                    
                    
                    vec3 OFF = tgt_pos;
                    

                    
                    constexpr const bool OLD_NO_DEFERRED = false;
                    
                    
                    if constexpr ((OLD_NO_DEFERRED)) {
                        mtt::Thing_ID parent_id = mtt::get_parent_ID(st.src_thing);
                        if (mtt::Thing_ID_is_valid(parent_id)) {
                            mtt::disconnect_child_from_parent(world, st.src_thing);
                            
                            vec3 xformed_point = (m::translate(Mat4(1.0f), OFF) *m::rotate(mat4(1.0f), mag_current * ((float32)MTT_TIMESTEP) * dir, vec3(0.0f, 0.0f, 1.0f)) * m::translate(Mat4(1.0f), -OFF)) * vec4(src_pos, 1.0f);
                            
                            rep->model_transform = rep->model_transform * m::translate(Mat4(1.0f), vec3(m::angular_impulse((float64)((float32)MTT_TIMESTEP) * dir * 2, vec2(src_pos), vec2(tgt_pos)), 0.0f));
                            
                            
                            rep->model_transform_inverse = m::inverse(rep->model_transform);
                            
                            //set_pose_transform(st.src_thing, m::mat4_identity() * rep->pose_transform);
                            
                            mtt::connect_parent_to_child(world, parent_id, mtt::thing_id(st.src_thing));
                            
                        } else {
                            //                        rep->model_transform = m::rotate_around_with_matrix(rep->model_transform, *rot_global, mat4(1.0f), OFF);
                            vec3 xformed_point = (m::translate(Mat4(1.0f), OFF) *m::rotate(mat4(1.0f), mag_current * ((float32)MTT_TIMESTEP) * dir, vec3(0.0f, 0.0f, 1.0f)) * m::translate(Mat4(1.0f), -OFF)) * vec4(src_pos, 1.0f);
                            
                            //rep->model_transform = rep->model_transform * m::translate(Mat4(1.0f), xformed_point - src_pos);
                            rep->model_transform = rep->model_transform * m::translate(Mat4(1.0f), vec3(m::angular_impulse((float64)((float32)MTT_TIMESTEP) * dir * 2, vec2(src_pos), vec2(tgt_pos)), 0.0f));
                            
                            
                            rep->model_transform_inverse = m::inverse(rep->model_transform);
                            
                            //set_pose_transform(st.src_thing, m::mat4_identity() * rep->pose_transform);
                        }
                    } else {
                        Rotate_Around_Deferred_Args rot_args = {
                            .src_id = mtt::id(st.src_thing),
                            .tgt_id = mtt::id(st.tgt_thing),
                            .impulse = dir * 2,
                            .script_id = s->id,
                            .world = world,
                            
                            .proc = [](Rotate_Around_Deferred_Args* args) {
                                mtt::World* world = args->world;
                                auto src_id = args->src_id;
                                auto tgt_id = args->tgt_id;
                                
                                
                                mtt::Thing* src_thing = mtt::Thing_try_get(world, src_id);
                                mtt::Thing* tgt_thing = mtt::Thing_try_get(world, tgt_id);
                                if (src_thing == nullptr ||tgt_thing == nullptr) {
                                    return;
                                }
                                
                                auto* src_rep = mtt::rep(src_thing);
                                
                                auto impulse_factor = args->impulse;
                                
                                
                                mtt::Thing_ID parent_id = mtt::get_parent_ID(src_thing);
                                
                                if (mtt::Thing_ID_is_valid(parent_id)) {
                                    
                                    mtt::disconnect_child_from_parent(world, src_thing);
                                    
                                    vec3* tgt_pos_ptr = mtt::access<vec3>(tgt_thing, "position");
                                    vec3 tgt_pos = *tgt_pos_ptr;
                                    vec3* src_pos_ptr = mtt::access<vec3>(src_thing, "position");
                                    vec3 src_pos = *src_pos_ptr;
                                    
                                    
                                    
                                    src_rep->model_transform = src_rep->model_transform * m::translate(Mat4(1.0f), vec3(m::angular_impulse((float64)((float32)MTT_TIMESTEP) * impulse_factor, vec2(src_pos), vec2(tgt_pos)), 0.0f));
                                    
                                    
                                    src_rep->model_transform_inverse = m::inverse(src_rep->model_transform);
                                    
                                    //set_pose_transform(st.src_thing, m::mat4_identity() * rep->pose_transform);
                                    
                                    mtt::connect_parent_to_child(world, parent_id, src_id);
                                    
                                    
                                } else {
                                    
                                    vec3* tgt_pos_ptr = mtt::access<vec3>(tgt_thing, "position");
                                    vec3 tgt_pos = *tgt_pos_ptr;
                                    vec3* src_pos_ptr = mtt::access<vec3>(src_thing, "position");
                                    vec3 src_pos = *src_pos_ptr;
                                    
                                    src_rep->model_transform = src_rep->model_transform * m::translate(Mat4(1.0f), vec3(m::angular_impulse((float64)((float32)MTT_TIMESTEP) * impulse_factor, vec2(src_pos), vec2(tgt_pos)), 0.0f));
                                    
                                    
                                    src_rep->model_transform_inverse = m::inverse(src_rep->model_transform);
                                }
                                
                                
                                
                            },
                        };
                        
                        auto* s_args = Script_shared_args_update(s->source_script);
                        auto& sh_args = *((dt::Dynamic_Array<Rotate_Around_Deferred_Args>*)s_args->data);
                        
                        sh_args.push_back(rot_args);
                            
                    }
                    
                    
                    
                    
//                    vec3 scale;
//                    quat orientation;
//                    vec3 translation;
//                    vec3 skew;
//                    vec4 perspective;
//
//                    m::decompose(rep->pose_transform, scale, orientation, translation, skew, perspective);
//                    
//                    float32 dt = MTT_TIMESTEP;
//                    float32 r_scale = mag_current;
//                    auto [rot_by, sign, dot_product, cross_product] = m::rotation_toward(norm, orientation * vec3(0,-1,0), r_scale, dt);
//                    {
//                        mat4 r_mat = m::rotate(Mat4(1.0f), rot_by, vec3(0, 0, 1));
//                        set_pose_transform(st.src_thing, r_mat * rep->pose_transform);
//                    }
                    
                    
//                    set_pose_transform(st.src_thing, rep->pose_transform * m::rotate(Mat4(1.0f), (float32)MTT_TIMESTEP * st.magnitude * norm, {0.0f, 0.0f, 1.0f}));
                }
                

                
//                if (m::sign(*vel_current_ptr) != m::sign(vel_prev)) {
//
//                    auto* rep = mtt::rep(st.src_thing);
//
//                    if (vel_current_ptr->x > 0) {
//                        if (heading_direction_x(rep) < 0) {
//                            mtt::flip_left_right(st.src_thing);
//                        }
//                    } else if (vel_current_ptr->x < 0) {
//                        if (heading_direction_x(rep) > 0) {
//                            mtt::flip_left_right(st.src_thing);
//                        }
//                    }
//                }
                
                break;
            }
                // treat as direction
            case MTT_VECTOR3: {
                
                
                break;
            }
            case MTT_STRING:
                MTT_FALLTHROUGH;
            default: {
                
                
                if (IN_source_selectors.status == PORT_STATUS_OK) {
                    st.tgt_type = MTT_VECTOR3;
                    
                    Script_Property_List* all = (Script_Property_List*)IN_source_selectors.value.out.List;
                    for (auto& sel : *all) {
                        float32 dir = 1;
                        mtt::String dir_sel = sel.value.get_cstring();
                        if (dir_sel == ARG_dir_up) {
                            dir = -1;
                        } else if (dir_sel == ARG_dir_down) {
                            dir = 1;
                        } else if (dir_sel == ARG_dir_left) {
                            dir = -1;
                        } else if (dir_sel == ARG_dir_right) {
                            dir = 1;
                        } else if (dir_sel == ARG_dir_clockwise) {
                            dir = 1;
                        } else if (dir_sel == ARG_dir_anticlockwise) {
                            dir = -1;
                        }
                        else if (dir_sel == ARG_dir_none) {
                            //dir = vec3(0.0f, 0.0f, 0.0f);
                            dir = 1;
                        }
                        
                        
                        auto* rep = mtt::rep(st.src_thing);
                        
                        
                        set_pose_transform(st.src_thing, rep->pose_transform * m::rotate(Mat4(1.0f), (float32)MTT_TIMESTEP * st.magnitude * dir, {0.0f, 0.0f, 1.0f}));
//                        {
//                            vec3 scale;
//                            quat orientation;
//                            vec3 translation;
//                            vec3 skew;
//                            vec4 perspective;
//
//
//
//
//
//
//                            m::decompose(rep->pose_transform, scale, orientation, translation, skew, perspective);
//                            vec3 angles = m::eulerAngles(orientation);
//                            if (angles.z < 0) {
//                                angles.z += 2*MTT_PI;
//                            }
//                            m::Vec3_print(angles);
//
//                        }
                        
                        
                        ///////////////////
//                        {
//                            float32 prev_sign = m::sign(V->x);
//                            vec3 force = st.magnitude * dir * acceleration.y;
//                            *V += force * (float32)MTT_TIMESTEP;
//
//                            float32 m_adjust = m::length(*V);
//                            if (m_adjust > velocity_magnitude) {
//                                dir = m::normalize(*V);
//                                *V = velocity_magnitude * dir;
//                            }
//                        }
                        //////////////////
                        
                        
                        
                        
                        
//                        if (m::sign(V->x) != prev_sign) {
//
//                            auto* rep = mtt::rep(st.src_thing);
//
//                            if (V->x > 0) {
//                                if (heading_direction_x(rep) < 0) {
//                                    mtt::flip_left_right(st.src_thing);
//                                }
//                            } else if (V->x < 0) {
//                                if (heading_direction_x(rep) > 0) {
//                                    mtt::flip_left_right(st.src_thing);
//                                }
//                            }
//                        }
                        
                    }
                } else if (st.has_direction_selector) {
                    st.tgt_type = MTT_VECTOR3;
                    
                    float32 dir = 1;
                    mtt::String dir_sel = MTT_string_ref_to_cstring(st.dir_selector);
                    if (dir_sel == ARG_dir_up) {
                        dir = -1;
                    } else if (dir_sel == ARG_dir_down) {
                        dir = 1;
                    } else if (dir_sel == ARG_dir_left) {
                        dir = -1;
                    } else if (dir_sel == ARG_dir_right) {
                        dir = 1;
                    } else if (dir_sel == ARG_dir_clockwise) {
                        dir = 1;
                    } else if (dir_sel == ARG_dir_anticlockwise) {
                        dir = -1;
                    }
                    else if (dir_sel == ARG_dir_none) {
                        dir = 1;
                    }
                    
                    auto* rep = mtt::rep(st.src_thing);
                    
                    
                    set_pose_transform(st.src_thing, rep->pose_transform * m::rotate(Mat4(1.0f), (float32)MTT_TIMESTEP * st.magnitude * dir, {0.0f, 0.0f, 1.0f}));

                    
                } else {
                    //return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                    auto* rep = mtt::rep(st.src_thing);
                    set_pose_transform(st.src_thing, rep->pose_transform * m::rotate(Mat4(1.0f), (float32)MTT_TIMESTEP * st.magnitude * 1, {0.0f, 0.0f, 1.0f}));
                }
                
                break;
            }
        }
        
        
        if (loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP) {
            loop_idx += 1;
            if (all_sources->size() > loop_idx) {
                st.src_thing = mtt::Thing_try_get(world, (*all_sources)[loop_idx].value.thing_id);
                if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                }
            } else {
                loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = false;
            }
        }
    } while (loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP);

    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(rotate_to)
{
    MTT_ADD_FIELD_T("state", Rotate_State);
    
    Thing_add_in_port({world, fields, ports, logic, "velocity", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selector", MTT_TEXT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selectors", MTT_LIST, MTT_SCRIPT_PROPERTY,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "velocity_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "angle_radians", MTT_BOOLEAN, 0,
        {(ALIGN_RIGHT * 0.5f)}, nullptr
    });
    
    
    Thing_add_in_port({world, fields, ports, logic, "source", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "angle_radians", MTT_BOOLEAN, 0,
        {(ALIGN_RIGHT * 0.5f)}, nullptr
    });
    
    
//    archetype->logic.proc = rotate_procedure;
//    archetype->logic.option = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
//        auto* direction = mtt::access<float32>(thing, "direction");
//        *direction = 1.0f;
//        auto* axes = mtt::access<vec3>(thing, "axes");
//        *axes = vec3(0.0f, 0.0f, 1.0f);
    };
}
MTT_DEFINE_LOGIC_PROCEDURE(rotate_to)
{
    if (!thing_group_is_active(thing)) {
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(rotate)
{
    MTT_ADD_FIELD_T("state", Rotate_State);
    
    Thing_add_in_port({world, fields, ports, logic, "velocity", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration_local", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selector", MTT_TEXT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "source_selectors", MTT_LIST, MTT_SCRIPT_PROPERTY,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_in_port({world, fields, ports, logic, "velocity_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_magnitude", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "velocity_local_direction", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "angle_radians", MTT_BOOLEAN, 0,
        {(ALIGN_RIGHT * 0.5f)}, nullptr
    });
    
    
    Thing_add_in_port({world, fields, ports, logic, "source", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "angle_radians", MTT_BOOLEAN, 0,
        {(ALIGN_RIGHT * 0.5f)}, nullptr
    });
    
    
//    archetype->logic.proc = rotate_procedure;
//    archetype->logic.option = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
//        auto* direction = mtt::access<float32>(thing, "direction");
//        *direction = 1.0f;
//        auto* axes = mtt::access<vec3>(thing, "axes");
//        *axes = vec3(0.0f, 0.0f, 1.0f);
    };
}

MTT_DEFINE_LOGIC_PROCEDURE(rotate)
{
    auto* state = mtt::access<Rotate_State>(thing, "state");
    
    auto IN_velocity             = get_in_port(thing, input, "velocity");
    vec3 velocity = (IN_velocity.status == PORT_STATUS_OK) ? IN_velocity.value.out.Vector3 : m::vec3_zero();
    
    auto IN_acceleration         = get_in_port(thing, input, "acceleration");
    vec3 acceleration = (IN_acceleration.status == PORT_STATUS_OK) ? IN_acceleration.value.out.Vector3 : vec3(1.0f, 1.0f, 1.0f);
    
    auto IN_velocity_local       = get_in_port(thing, input, "velocity_local");
    vec3 velocity_local = (IN_velocity_local.status == PORT_STATUS_OK) ? IN_velocity_local.value.out.Vector3 : m::vec3_zero();
    
    auto IN_acceleration_local   = get_in_port(thing, input, "acceleration_local");
    vec3 acceleration_local = (IN_acceleration_local.status == PORT_STATUS_OK) ? IN_acceleration_local.value.out.Vector3 : m::vec3_zero();
    
    
    auto IN_velocity_magnitude       = get_in_port(thing, input, "velocity_magnitude");
    float32 velocity_magnitude = (IN_velocity_magnitude.status == PORT_STATUS_OK) ? IN_velocity_magnitude.value.out.Float : 1.0f;
    auto IN_velocity_local_magnitude       = get_in_port(thing, input, "velocity_local_magnitude");
    float32 velocity_local_magnitude = (IN_velocity_local_magnitude.status == PORT_STATUS_OK) ? IN_velocity_local_magnitude.value.out.Float : 1.0f;
    
    auto IN_velocity_direction       = get_in_port(thing, input, "velocity_direction");
    vec3 velocity_direction = (IN_velocity_direction.status == PORT_STATUS_OK) ? IN_velocity_direction.value.out.Vector3 : m::vec3_zero();
    auto IN_velocity_local_direction       = get_in_port(thing, input, "velocity_local_direction");
    vec3 velocity_local_direction = (IN_velocity_local_direction.status == PORT_STATUS_OK) ? IN_velocity_local_direction.value.out.Vector3 : m::vec3_zero();
    
    auto IN_source_selector = get_in_port(thing, input, "source_selector");
    
    
    auto IN_source_selectors = get_in_port(thing, input, "source_selectors");
    
    auto OUT_is_done = get_out_port(thing, "is_done");
    OUT_is_done.value.out.set_Boolean(false);
    
    
    
    
    struct State {
        mtt::Thing* src_thing = nullptr;
        mtt::Thing* tgt_thing = nullptr;
        vec3 tgt_vec3 = m::vec3_zero();
        MTT_TYPE tgt_type = MTT_NONE;
        float32 magnitude = 0.0f;
        vec3 velocity = m::vec3_zero();
        vec3 acceleration = m::vec3_zero();
        MTT_String_Ref dir_selector = {};
        bool has_direction_selector = false;
    } st;
    
    st.magnitude = velocity_magnitude;
    
    st.acceleration = acceleration;
    if (IN_source_selector.status == PORT_STATUS_OK) {
        st.dir_selector = IN_source_selector.value.out.String;
        st.has_direction_selector = true;
    }
    
    auto IN_source = get_in_port(thing, input, "source");
    if (IN_source.status != PORT_STATUS_OK) {
        //return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
        OUT_is_done.value.out.set_Boolean(true);
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }
    
    auto IN_target = get_in_port(thing, input, "target");
    if (IN_target.status == PORT_STATUS_OK) {
        st.tgt_type = IN_target.value.out.type;
        st.tgt_thing = mtt::Thing_try_get(world, IN_target.value.out.thing_id);
        if (st.tgt_thing == nullptr) {
            OUT_is_done.value.out.set_Boolean(true);
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
        }
    }
    
    Script_Property_List* all_sources = nullptr;
    bool loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = false;
    usize loop_idx = 0;
    switch (IN_source.value.out.type) {
        default: {
            st.src_thing = mtt::Thing_try_get(world, IN_source.value.out.thing_id);
            if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            break;
        }
        case MTT_LIST: {
            all_sources = (Script_Property_List*)IN_source.value.out.List;
            if (all_sources->empty()) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            st.src_thing = mtt::Thing_try_get(world, (*all_sources)[loop_idx].value.thing_id);
            if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
            loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = true;
            break;
        }
    }
    
    if (!state->is_init) {
        state->is_init = true;
        //state->original_pose = mtt::rep(st.src_thing)->pose_transform;
    }
    
    do {
        vec3* V = mtt::access<vec3>(st.src_thing, "velocity");
        vec3* A = mtt::access<vec3>(st.src_thing, "acceleration");
        if (A != nullptr) {
            acceleration += *A;
        }
        
        
        switch (IN_target.value.out.type) {
            case MTT_THING: {
                st.tgt_type = MTT_THING;
                
                float32 towards = 1.0;
                
                auto* PLIST = Script_Lookup_get_var_with_key(s, ARG_directional, DEFAULT_LOOKUP_SCOPE);
                if (!PLIST->empty()) {
                    mtt::String dir_val = PLIST->begin()->value.get_cstring();
                    if (dir_val == ARG_dir_away) {
                        towards = -1.0;
                    }
                }
                
                auto* rep = mtt::rep(st.src_thing);
                
                
                vec3* tgt_pos_ptr = mtt::access<vec3>(st.tgt_thing, "position");
                vec3 tgt_pos = *tgt_pos_ptr;
                vec3* src_pos_ptr = mtt::access<vec3>(st.src_thing, "position");
                vec3 src_pos = *src_pos_ptr;
                
                vec3* vel_current_ptr = mtt::access<vec3>(st.src_thing, "velocity");
                vec3  vel_current = *vel_current_ptr;
                vec3  vel_prev = vel_current;
                
                vec3 diff = tgt_pos - src_pos;
                vec3 norm = (diff == m::vec3_zero()) ? m::vec3_zero() : m::normalize(diff);
                vel_current += diff * (float32)MTT_TIMESTEP * acceleration;
                
                
                float32 mag_current = vel_current == m::vec3_zero() ? 0.0f : m::length(vel_current);
                
                {
                    float32 mag_max     = velocity_magnitude;
                    vec3 dir = (vel_current == m::vec3_zero()) ? m::vec3_zero() : m::normalize(vel_current);
                    if (mag_current > mag_max) {
                        vel_current = dir * mag_max;
                        mag_current = mag_max;
                    }
                }
                
                {
                    auto* rep = mtt::rep(st.src_thing);
                    auto* tgt_rep = mtt::rep(st.tgt_thing);
                    
                    
                    vec3 scale;
                    quat orientation;
                    vec3 translation;
                    vec3 skew;
                    vec4 perspective;

                    m::decompose(rep->pose_transform, scale, orientation, translation, skew, perspective);
                    
                    float32 dt = MTT_TIMESTEP;
                    float32 r_scale = mag_current;
                    auto [rot_by, sign, dot_product, cross_product] = m::rotation_toward(norm, orientation * vec3(0,-1,0), r_scale * towards, dt);
                    {
                        mat4 r_mat = m::rotate(Mat4(1.0f), rot_by, vec3(0, 0, 1));
                        set_pose_transform(st.src_thing, r_mat * rep->pose_transform);
                        
                        //MTT_print("rotate: dot=[%f]\n", dot_product);
                                                
                        if (abs((towards * 1.0) - dot_product) < 0.001) {
                            OUT_is_done.value.out.set_Boolean(true);
                        }
                    }
                    
                    
//                    set_pose_transform(st.src_thing, rep->pose_transform * m::rotate(Mat4(1.0f), (float32)MTT_TIMESTEP * st.magnitude * norm, {0.0f, 0.0f, 1.0f}));
                }
                
                
#if 0
                vec3 pos_future = src_pos + ((float32)mtt::MOVEMENT_SCALE * vel_current * (float32)MTT_TIMESTEP);
                vec3 diff_future = tgt_pos - pos_future;
                auto norm_future = (diff_future == m::vec3_zero()) ? m::vec3_zero() : m::normalize(diff_future);
                auto s_future = m::sign(norm_future);
                auto s_curr = m::sign(norm);
                MTT_UNUSED(s_curr);
                if ((m::dist_squared(tgt_pos, pos_future) < 25.0f * 25.0f) || s_future != s_curr/* || s_future == vec3(0.0f) || m::dot(norm, diff_future) < 0.0f*/) {
                    
                    Message msg;
                    msg.sender   = st.src_thing->id;
                    msg.selector = mtt::string("center");
                    selector_invoke(st.tgt_thing, &msg);
                    ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
                    
                    vec3 tgt_velocity = *mtt::access<vec3>(st.tgt_thing, "velocity");
                    
                    if (tgt_velocity != m::vec3_zero()) {
                        float32 MAG = m::length(tgt_velocity);
                        if (MAG <= mag_current) {
                            *vel_current_ptr = MAG * m::normalize(tgt_velocity);
                            mtt::Thing_set_position(st.src_thing, msg.output_value.Vector3);
                        }
                    } else {
                        *vel_current_ptr = vec3(0.0f, 0.0f, 0.0f);
                        mtt::Thing_set_position(st.src_thing, msg.output_value.Vector3);
                        
                    }

                    //
                    //                *vel_current_ptr = (tgt_velocity == m::vec3_zero()) ? m::vec3_zero() : mag_current * m::normalize(tgt_velocity);
                    
                    
                    OUT_is_done.value.out.set_Boolean(true);
                } else {
                    *vel_current_ptr = vel_current;
                }
#endif
                
//                if (m::sign(*vel_current_ptr) != m::sign(vel_prev)) {
//                    
//                    auto* rep = mtt::rep(st.src_thing);
//                    
//                    if (vel_current_ptr->x > 0) {
//                        if (heading_direction_x(rep) < 0) {
//                            mtt::flip_left_right(st.src_thing);
//                        }
//                    } else if (vel_current_ptr->x < 0) {
//                        if (heading_direction_x(rep) > 0) {
//                            mtt::flip_left_right(st.src_thing);
//                        }
//                    }
//                }
                
                break;
            }
                // treat as direction
            case MTT_VECTOR3: {
                
                
                break;
            }
            case MTT_STRING:
                MTT_FALLTHROUGH;
            default: {
                
                
                if (IN_source_selectors.status == PORT_STATUS_OK) {
                    st.tgt_type = MTT_VECTOR3;
                    
                    Script_Property_List* all = (Script_Property_List*)IN_source_selectors.value.out.List;
                    for (auto& sel : *all) {
                        float32 dir = 1;
                        mtt::String dir_sel = sel.value.get_cstring();
                        if (dir_sel == ARG_dir_up) {
                            dir = -1;
                        } else if (dir_sel == ARG_dir_down) {
                            dir = 1;
                        } else if (dir_sel == ARG_dir_left) {
                            dir = -1;
                        } else if (dir_sel == ARG_dir_right) {
                            dir = 1;
                        } else if (dir_sel == ARG_dir_clockwise) {
                            dir = 1;
                        } else if (dir_sel == ARG_dir_anticlockwise) {
                            dir = -1;
                        } else if (dir_sel == ARG_dir_away) {
                            dir = 1;
                            // PLACEHOLDER
                        } else if (dir_sel == ARG_dir_none) {
                            //dir = vec3(0.0f, 0.0f, 0.0f);
                            dir = 1;
                        }
                        
                        auto* rep = mtt::rep(st.src_thing);
                        
                        
                        set_pose_transform(st.src_thing, rep->pose_transform * m::rotate(Mat4(1.0f), (float32)MTT_TIMESTEP * st.magnitude * dir, {0.0f, 0.0f, 1.0f}));
//                        {
//                            vec3 scale;
//                            quat orientation;
//                            vec3 translation;
//                            vec3 skew;
//                            vec4 perspective;
//                            
//                            
//                            
//                            
//                            
//                            
//                            m::decompose(rep->pose_transform, scale, orientation, translation, skew, perspective);
//                            vec3 angles = m::eulerAngles(orientation);
//                            if (angles.z < 0) {
//                                angles.z += 2*MTT_PI;
//                            }
//                            m::Vec3_print(angles);
//
//                        }
                        
                        
                        ///////////////////
//                        {
//                            float32 prev_sign = m::sign(V->x);
//                            vec3 force = st.magnitude * dir * acceleration.y;
//                            *V += force * (float32)MTT_TIMESTEP;
//                            
//                            float32 m_adjust = m::length(*V);
//                            if (m_adjust > velocity_magnitude) {
//                                dir = m::normalize(*V);
//                                *V = velocity_magnitude * dir;
//                            }
//                        }
                        //////////////////
                        
                        
                        
                        
                        
//                        if (m::sign(V->x) != prev_sign) {
//                            
//                            auto* rep = mtt::rep(st.src_thing);
//                            
//                            if (V->x > 0) {
//                                if (heading_direction_x(rep) < 0) {
//                                    mtt::flip_left_right(st.src_thing);
//                                }
//                            } else if (V->x < 0) {
//                                if (heading_direction_x(rep) > 0) {
//                                    mtt::flip_left_right(st.src_thing);
//                                }
//                            }
//                        }
                        
                    }
                } else if (st.has_direction_selector) {
                    st.tgt_type = MTT_VECTOR3;
                    
                    float32 dir = 1;
                    mtt::String dir_sel = MTT_string_ref_to_cstring(st.dir_selector);
                    if (dir_sel == ARG_dir_up) {
                        dir = -1;
                    } else if (dir_sel == ARG_dir_down) {
                        dir = 1;
                    } else if (dir_sel == ARG_dir_left) {
                        dir = -1;
                    } else if (dir_sel == ARG_dir_right) {
                        dir = 1;
                    } else if (dir_sel == ARG_dir_clockwise) {
                        dir = 1;
                    } else if (dir_sel == ARG_dir_anticlockwise) {
                        dir = -1;
                    }
                    else if (dir_sel == ARG_dir_none) {
                        //dir = vec3(0.0f, 0.0f, 0.0f);
                        //return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                        dir = 1;
                    }
                    

                    
                    auto* rep = mtt::rep(st.src_thing);
                    
                    
                    set_pose_transform(st.src_thing, rep->pose_transform * m::rotate(Mat4(1.0f), (float32)MTT_TIMESTEP * st.magnitude * dir, {0.0f, 0.0f, 1.0f}));
//                    ///////////
//                    {
//                        float32 prev_sign = m::sign(V->x);
//                        vec3 force = st.magnitude * dir;
//                        *V += force * (float32)MTT_TIMESTEP;
//                        
//                        float32 m_adjust = m::length(*V);
//                        if (m_adjust > velocity_magnitude) {
//                            dir = m::normalize(*V);
//                            *V = velocity_magnitude * dir;
//                        }
//                    }
                    ////////////////
                    
//                    if (m::sign(V->x) != prev_sign) {
//                        
//                        auto* rep = mtt::rep(st.src_thing);
//                        if (heading_direction_x(rep) < 0) {
//                            mtt::flip_left_right(st.src_thing);
//                        }
//                        
//                        if (heading_direction_x(rep) > 0) {
//                            mtt::flip_left_right(st.src_thing);
//                        }
//                    }
                    
                } else {
                    //return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                    auto* rep = mtt::rep(st.src_thing);
                    set_pose_transform(st.src_thing, rep->pose_transform * m::rotate(Mat4(1.0f), (float32)MTT_TIMESTEP * st.magnitude * 1, {0.0f, 0.0f, 1.0f}));
                }
                
                break;
            }
        }
        
        
        if (loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP) {
            loop_idx += 1;
            if (all_sources->size() > loop_idx) {
                st.src_thing = mtt::Thing_try_get(world, (*all_sources)[loop_idx].value.thing_id);
                if (st.src_thing == nullptr || mtt::input_should_cancel_animation(st.src_thing)) {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                }
            } else {
                loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP = false;
            }
        }
    } while (loop_over_sources_HACK_SHOULD_SUPPORT_EXTERNAL_LOOP);

    return true;
}

// TODO: ...

External_Call_ID next_external_call_id = 0;
External_Call_ID external_call_id_make(void)
{
    return ++next_external_call_id;
}

MTT_DEFINE_INITIALIZER(external)
{
    MTT_ADD_FIELD_T("state", External_State);
}

MTT_DEFINE_LOGIC_PROCEDURE(external)
{
    auto* state = mtt::access<External_State>(thing, "state");
    
    usize time_elapsed = 0;
    if (!state->is_init) {
        state->is_init = true;
        state->call_id = external_call_id_make();
        if (state->timeout > 0) {
            state->t_start = mtt_time_nanoseconds();
        }
    }
    if (state->timeout > 0) {
        time_elapsed = mtt_time_nanoseconds() - state->t_start;
        if (time_elapsed >= state->timeout) {
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
        }
    }
    
    // TODO: will need to suspend until external command returns
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    // LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND
}


}
