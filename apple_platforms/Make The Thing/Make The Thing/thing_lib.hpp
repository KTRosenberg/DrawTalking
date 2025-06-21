//
//  thing_lib.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/28/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef thing_lib_hpp
#define thing_lib_hpp

#include "thing.hpp"
#include "thing_private.hpp"

namespace mtt {

struct Jump_State {
    bool is_init = false;
    float32 interpolation_value = 0.0f;
    vec3 position_source_start = vec3(0.0f);
    bool target_use_start_position = false;
    vec3 position_target_start = vec3(0.0f);
    vec3 raw_position_source_start = vec3(0.0f);
};

struct Move_State {
    vec3 current_velocity = vec3(0.0f);
    vec3 current_acceleration = vec3(0.0f);
    
    vec3 direction = m::vec3_zero();
    float32 magnitude = 0.0f;
    vec3 prev_direction = m::vec3_zero();
    float32 prev_magnitude = 0.0f;
    
    vec3 prev_pos = vec3(0.0f);
    
    bool prev_was_done = false;
};

struct List_Cycler {
    usize prev_idx = 0;
    usize curr_idx = 0;
};


struct Rule_Eval_Comp_Vertex {
    std::vector<dt::Word_Dictionary_Entry*> label_entry = {};
    mtt::String label_key = "";
    mtt::Thing_ID thing_ID = mtt::Thing_ID_INVALID;
};

struct Rule_Eval_Comp_Edge {
    mtt::Thing_ID in = mtt::Thing_ID_INVALID;
    std::vector<dt::Word_Dictionary_Entry*> in_label_entry = {};
    mtt::String in_label_key = "";
    mtt::Thing_ID out = mtt::Thing_ID_INVALID;
    std::vector<dt::Word_Dictionary_Entry*> out_label_entry = {};
    mtt::String out_label_key = "";
};
struct Rule_Eval_Comp_Edge_Arrow {
    mtt::Thing_ID in = mtt::Thing_ID_INVALID;
    std::vector<dt::Word_Dictionary_Entry*> in_label_entry = {};
    mtt::String in_label_key = "";
    mtt::Thing_ID out = mtt::Thing_ID_INVALID;
    std::vector<dt::Word_Dictionary_Entry*> out_label_entry = {};
    mtt::String out_label_key = "";
    ARROW_LINK_FLAGS arrow_type = ARROW_LINK_FLAGS_DIRECTED;
};

struct Rule_Eval_Comp_Structure {
    std::vector<Rule_Eval_Comp_Vertex> vertex_list = {};
    std::vector<Rule_Eval_Comp_Edge> edge_list = {};
    std::vector<Rule_Eval_Comp_Edge_Arrow> arrow_list = {};
};

struct Rule_Eval_State {
    Script_ID to_invoke_script_ID = Script_ID_INVALID;
    uint64 flags = 0;
    
    //std::vector<mtt::Thing_ID> prev_results = {};
    std::vector<mtt::Thing_ID> curr_results = {};
    mtt::Set<mtt::String> dup_check = {};
    mtt::Map_Stable<mtt::Thing_ID, usize> thing_update_time_for_value_checks = {};
    mtt::Map_Stable<mtt::Thing_ID, usize> thing_update_time_for_value_exceed_checks = {};
    usize update_count = 0;
    
    std::vector<Rule_Eval_Comp_Structure> comp_structure_target = {};
};


struct Rotate_State {
    uint64 TODO = 0;
    float32 angle_radians = 0.0;
    float32 direction = 0.0;
    vec3 axes = vec3(0.0f, 0.0f, 1.0f);
    vec3 offset = vec3(0.0f, 0.0f, 0.0f);
    //Mat4 original_pose = Mat4(1.0f);
    Mat4 global_transform;
    bool is_init = false;
    
    //     add_field({
    //         world, fields, ports, logic, "angle_radians", MTT_FLOAT, 0
    //     });
    //
    //     add_field({
    //         world, fields, ports, logic, "direction", MTT_FLOAT, 0
    //     });
    //
    //     add_field({
    //         world, fields, ports, logic, "axes", MTT_VECTOR3, 0
    //     });
    //
    //     add_field({
    //         world, fields, ports, logic, "offset", MTT_VECTOR3, 0
    //     });
    
};

struct Rotate_Around_Deferred_Args {
    mtt::Thing_ID src_id;
    mtt::Thing_ID tgt_id;
    float64 impulse;
    mtt::Script_ID script_id;
    mtt::World* world;
    
    void (*proc)(Rotate_Around_Deferred_Args* args);
};

typedef uint64 External_Call_ID;
struct External_State {
    External_Call_ID call_id = 0;
    usize timeout = 0;
    bool is_init = false;
    usize t_start = 0;
};

}


#endif /* thing_lib_hpp */
