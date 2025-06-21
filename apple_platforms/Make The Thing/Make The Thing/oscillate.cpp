//
//  TEST_THING_TYPE.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 2/20/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#include "thing.hpp"
#include "thing_private.hpp"

namespace mtt {


void oscillate_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                (meta[MTT_BOOLEAN].alloc_byte_size) +
                                
                                (meta[MTT_THING].alloc_byte_size) +
                                (meta[MTT_THING].alloc_byte_size) +
                                
                                (meta[MTT_VECTOR3].alloc_byte_size) +
                                (meta[MTT_THING].alloc_byte_size) +
                                (meta[MTT_FLOAT].alloc_byte_size) +
                                (meta[MTT_VECTOR3].alloc_byte_size) +
                                (meta[MTT_FLOAT].alloc_byte_size) +
                                (meta[MTT_FLOAT].alloc_byte_size) +
                                (meta[MTT_FLOAT].alloc_byte_size) +
                                (meta[MTT_TEXT].alloc_byte_size) +
                                (meta[MTT_TEXT].alloc_byte_size) +
                                (meta[MTT_TEXT].alloc_byte_size) +
                                (meta[MTT_VECTOR3].alloc_byte_size) +
                                (meta[MTT_INT64].alloc_byte_size)
                                );
    
    
    add_field({world, fields, ports, logic, "is_init" ,             MTT_BOOLEAN, 0});
    
    add_field({world, fields, ports, logic, "source",       MTT_THING, 0});
    add_field({world, fields, ports, logic, "destination",  MTT_THING, 0});
    
    
    add_field({world, fields, ports, logic, "position_destination", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "thing_ceiling",        MTT_THING, 0, });
    
    add_field({world, fields, ports, logic, "position_ceiling", MTT_FLOAT, 0}, temp_mem<float32>(POSITIVE_INFINITY));
    add_field({world, fields, ports, logic, "destination_offset", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "speed",              MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "interpolation",      MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "jump_height",      MTT_FLOAT, 0});
    
    
    add_field({world, fields, ports, logic, "source_selector", MTT_TEXT, 0});
    add_field({world, fields, ports, logic, "destination_selector", MTT_TEXT, 0});
    add_field({world, fields, ports, logic, "ceiling_selectors", MTT_TEXT, 0});
    add_field({world, fields, ports, logic,
        "initial_position", MTT_VECTOR3, 0});
    
    
    add_field({world, fields, ports, logic, "cycle_count",      MTT_INT64, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic,  "active", MTT_BOOLEAN, 0, {ALIGN_DOWN}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "interpolation", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    archetype->on_thing_make = [](Thing* thing) {
        
        auto* src_selector = mtt::access<MTT_String_Ref>(thing, "source_selector");
        MTT_string_ref_release(src_selector);
        (*src_selector) = MTT_string_add(0, "center");
        auto* dst_selector = mtt::access<MTT_String_Ref>(thing, "destination_selector");
        MTT_string_ref_release(dst_selector);
        (*dst_selector) = MTT_string_add(0, "top_with_offset");
        
        
        auto* is_init = mtt::access<bool>(thing, "is_init");
        *is_init = false;
        
        auto* speed = mtt::access<float32>(thing, "speed");
        *speed = 1.0f;
        
        auto* cycle_count = mtt::access<uint64>(thing, "cycle_count");
        *cycle_count = 1;
    };
    
    archetype->on_thing_destroy = [](Thing* thing) {
        //        Message msg;
        //        msg.selector = "set_movement_option";
        //        msg.input_value.type = MTT_INT64;
        //        msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_DEFAULT;
        //        msg.sender = thing->id;
        //
        //        auto* src = mtt::access<mtt::Thing_List>(thing, "source");
        //
        //
        //        for (usize idx = 0, count = src->size(); idx < count; idx += 1) {
        //            mtt::Thing_Ref* s_ref = &((*src)[idx]);
        //
        //
        //            mtt::Thing* src_thing = nullptr;
        //
        //
        //
        //            if (!(s_ref->try_get(&src_thing))) {
        //                continue;
        //            }
        //
        //            selector_invoke(src_thing, &msg);
        //        }
    };
}

LOGIC_PROC_RETURN_TYPE oscillate_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE oscillate_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    auto* is_init = mtt::access<bool>(thing, "is_init");
    if (!thing_group_is_active(thing)) {
        *is_init = false;
        return true;
    }
    
    auto IN_active = mtt::get_in_port(thing, input, "active");
    if (IN_active.status != PORT_STATUS_OK) {
        auto result = mtt::get_out_port(thing, 0);
        if (result.status == PORT_STATUS_OK) {
            result.value.out.set_Boolean(false);
        }
    } else {
        if (IN_active.value.out.Boolean == false) {
            *is_init = false;
            auto result = mtt::get_out_port(thing, 0);
            if (result.status == PORT_STATUS_OK) {
                result.value.out.set_Boolean(false);
            }
            return true;
        }
    }
    
    auto* src      = mtt::access<mtt::Thing_Ref>(thing, "source");
    auto* dst      = mtt::access<mtt::Thing_Ref>(thing, "destination");
    auto* src_selector = mtt::access<MTT_String_Ref>(thing, "source_selector");
    auto* dst_selector = mtt::access<MTT_String_Ref>(thing, "destination_selector");
    
    // TODO: speed
    auto* speed           = mtt::access<float32>(thing, "speed");
    auto* jump_height_ref = mtt::access<float32>(thing, "jump_height");
    auto* cycle_count      = mtt::access<uint64>(thing, "cycle_count");
    
    auto* interpolation_ref = mtt::access<float32>(thing, "interpolation");
    
    auto IN_interpolation = mtt::get_in_port(thing, input, "interpolation");
    
    mtt::Thing_Ref* s_ref = src;
    
    
    mtt::Thing* src_thing = nullptr;
    if (!(s_ref->try_get(&src_thing))) {
        return true;
    }
    
    
    auto* init_src_ptr = mtt::access<vec3>(thing, "initial_position");
    auto* src_position_ptr = mtt::access<vec3>(src_thing, "position");
    if (src_position_ptr == nullptr) {
        MTT_error("%s", "position is required!\n");
        return true;
    }
    mtt::Thing_Ref* d_ref = dst;
    mtt::Thing* dst_thing = nullptr;
    if (!d_ref->try_get(&dst_thing)) {
        return true;
    }
    
    if (!(*is_init)) {
        
        {
            Message msg;
            msg.sender   = thing->id;
            msg.selector = *src_selector;
            selector_invoke(src_thing, &msg);
            ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
            *init_src_ptr = *src_position_ptr;
            
            thing->on_start(thing, src_thing, (void*)dst_thing);
            
        }
        *is_init = true;
        *interpolation_ref = 0.0f;
    }
    
    float32 jump_height = *jump_height_ref;
    
    
    float32 interpolation = 0.0f;
    if (IN_interpolation.status == mtt::PORT_STATUS_OK) {
        interpolation = IN_interpolation.value.out.Float;
        interpolation = m::clamp(interpolation, 0.0f, 1.0f);
        *interpolation_ref = interpolation;
    } else {
        interpolation = *interpolation_ref;
        interpolation = m::clamp(interpolation, 0.0f, 1.0f);
        *interpolation_ref += MTT_TIMESTEP * *speed;
    }
#define IS_DONE (0)
    auto result = mtt::get_out_port(thing, IS_DONE);
#undef IS_DONE
    if (result.status == mtt::PORT_STATUS_OK) {
        result.value.out.set_Boolean(interpolation >= 1.0f);
        
        if (result.value.out.Boolean == true) {
            for (usize idx = 0; idx < 1; idx += 1) {
                mtt::Thing_Ref* s_ref = src;
                
                
                mtt::Thing* src_thing = nullptr;
                
                if (s_ref->try_get(&src_thing)) {
                    Message msg;
                    msg.selector = string_ref_get("set_movement_option");
                    msg.input_value.type = MTT_INT64;
                    msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_DEFAULT;
                    selector_invoke(src_thing, &msg);
                }
            }
            
            
            
            return false;
        }
    } else if (interpolation >= 1.0f) {
        for (usize idx = 0; idx < 1; idx += 1) {
            mtt::Thing_Ref* s_ref = src;
            
            
            mtt::Thing* src_thing = nullptr;
            
            if (s_ref->try_get(&src_thing)) {
                Message msg;
                msg.selector = string_ref_get("set_movement_option");
                msg.input_value.type = MTT_INT64;
                msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_DEFAULT;
                selector_invoke(src_thing, &msg);
            }
        }
        
        
        return false;
    }
    
    
    
    
    
    
    
    
    {
        {
            
            
            
            vec3 src_pos = {0.0f, 0.0f, 0.0f};
            vec3 dst_pos = {0.0f, 0.0f, 0.0f};
            vec2 init_src = vec2(init_src_ptr->x, init_src_ptr->y);
            
            
            
            {
                Message msg;
                msg.sender   = thing->id;
                msg.selector = *src_selector;
                selector_invoke(src_thing, &msg);
                ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
                src_pos = msg.output_value.Vector3;
            }
            {
                Message msg;
                msg.sender   = src_thing->id;
                msg.selector = *dst_selector;
                selector_invoke(dst_thing, &msg);
                ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
                dst_pos = msg.output_value.Vector3;
            }
            {
                Message msg;
                msg.selector = string_ref_get("set_movement_option");
                msg.input_value.type = MTT_INT64;
                msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_POSITIONAL;
                selector_invoke(src_thing, &msg);
            }
            
            {
                
                vec2 cur = vec2(0.0f);
                {
                    const float32 TODO_CEILING_PARAM = NEGATIVE_INFINITY;
                    float32 ceiling = TODO_CEILING_PARAM;
                    
                    interpolation = m::inverse_smoothstep(interpolation);
                    
                    
                    vec2 dst = vec2(dst_pos.x, dst_pos.y);
                    cur.x = m::lerp(init_src.x, dst_pos.x, interpolation);
                    
                    
                    float32 sin_c = (dst.y - jump_height < ceiling) ? (init_src.y - ceiling) * 0.75 : jump_height;
                    
                    
                    cur.y = m::max(ceiling, m::lerp(init_src.y, dst_pos.y, interpolation) + m::sin(interpolation * -MTT_PI_32 * *cycle_count) * m::max(0.0f, sin_c));
                }
                
                //                std::cout << "src=[" << m::to_string(init_src) << "] cur=[" << m::to_string(cur) << "] dst=[" << m::to_string(dst_pos) << "]" << std::endl;
                vec3 original_position = *src_position_ptr;
                
                {
                    mtt::Rep* rep = mtt::rep(src_thing);
                    float32 x_diff = init_src_ptr->x - cur.x;
                    if (m::abs(x_diff) > 10) {
                        if (original_position.x < cur.x) {
                            if (rep->forward_dir.x < 0) {
                                mtt::flip_left_right(src_thing);
                            }
                        } else if (original_position.x > cur.x) {
                            if (rep->forward_dir.x > 0) {
                                mtt::flip_left_right(src_thing);
                            }
                        }
                    }
                }
                *src_position_ptr = vec3(cur.x, cur.y, src_position_ptr->z);
                mtt::Thing_set_position(src_thing, *src_position_ptr);
            }
        }
    }
    
    
    
    
    return true;
}




}
