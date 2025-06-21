//
//  standard_behaviors.cpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 8/20/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "standard_behaviors.hpp"

namespace dt {

static usize MAX_AND = 32;

void load_standard_behaviors(dt::DrawTalk* dt)
{
    auto& lang_ctx = dt->lang_ctx;
    DT_Behavior_Catalogue& bc = lang_ctx.behavior_catalogue;
    {
        return;
        
        bc.insert("be",        "", false, "", "");
        bc.insert("become",    "", false, "", "");
        bc.insert("have",      "", false, "", "");
        bc.insert("possess",   "", false, "", "");
        bc.insert("own",       "", false, "", "");
        bc.insert("carry",     "", false, "", "");
        bc.insert("quit",      "state",    false, "", "");
        bc.insert("throw",     "physical", false, "", "");
        bc.insert("toss",      "physical", true, "throw", "physical");
        
        // MARK: jump
        {
            auto& entry = *bc.insert("jump",      "", false, "", "");
            entry.logic_interfaces.fields["height"]["DEFAULT"].value = mtt::Any::from_Float(100.0f);
            entry.logic_interfaces.fields["speed"]["DEFAULT"].value = mtt::Any::from_Float(2.0f);
            entry.logic_interfaces.fields["cycle_count"]["DEFAULT"].value = mtt::Any::from_Int64(1);
            
//            entry.register_procedure("AGENT:1 OBJECT:+", <#DT_Behavior::Procedure &&proc#>)

//            DT_Behavior::Procedure_Key_Builder pb;
//            pb.key_insert("AGENT", DT_Behavior::Proc_Key::VALUE_TYPE_ONE).key_insert("OBJECT", DT_Behavior::Proc_Key::VALUE_TYPE_MANY);
//            auto out = entry.register_procedure(pb, [](DT_Args& args, Speech_Property& prop) {  return (DT_Behavior::Procedure_Return){0, true}; });
//            
//            pb.key_insert("DIRECT_OBJECT", DT_Behavior::Proc_Key::VALUE_TYPE_ANY);
//            
//            auto proc = entry.lookup_closest_procedure(pb);
            
            
            entry.set_on_start([](DT_Behavior::Args& args) -> Command* {
                
                using Prop_List = Speech_Property::Prop_List;
                
                Prop_List* agent                 = nullptr;
                Prop_List* object                = nullptr;
                Prop_List* direct_object         = nullptr;
                Prop_List* indirect_object       = nullptr;
                
                Prop_List* property              = nullptr;
                Prop_List* time                  = nullptr;
                Prop_List* preposition           = nullptr;
                Prop_List* sequence_then         = nullptr;
                Prop_List* sequence_sim         = nullptr;
                //Prop_List* preposition_object    = nullptr;
                //Prop_List* action                = nullptr;
                
                DT_Behavior_Catalogue& catalogue = *args.catalogue;
                DT_Behavior& b = *args.behavior;
                
                Speech_Property& prop = *args.current;
                
                
                prop.try_get_prop("AGENT",           &agent);
                prop.try_get_prop("OBJECT",          &object);
                prop.try_get_prop("DIRECT_OBJECT",   &direct_object);
                prop.try_get_prop("INDIRECT_OBJECT", &indirect_object);
                // check if parent link is there. If not, should treat as an independent action
                prop.try_get_prop("SEQUENCE_THEN",   &sequence_then);
                prop.try_get_prop("SEQUENCE_SIMULTANEOUS",   &sequence_sim);
                prop.try_get_prop("PROPERTY",        &property);
                // will require queries
                prop.try_get_prop("TIME",            &time);
                
                prop.try_get_prop("PREPOSITION",     &preposition);
                
                if (args.is_trigger) {
                    return nullptr;
                } else if (args.is_relation) {
                    return nullptr;
                }
                
                MTT_print("%s", "=-===========================================================\n");
                //prop.print();
                if (agent == nullptr || agent->empty()) {
                    return nullptr;
                }
                

                
                struct Role_Entity;
                struct Role_Entity {
                    mtt::String label = "";
                    mtt::String sub_label;
                    mtt::String key;
                    mtt::String type;
                    
                    mtt::Thing_List things;
                    Port_Connection_Descriptor ports;
                    Field_Modify_Descriptor fields;
                    
                    mtt::Map_Stable<mtt::String, Role_Entity*> map;
                    
                    Role_Entity() {}
                };
                mtt::Map_Stable<mtt::String, Role_Entity*> map = {};
                
                
                
                
                mtt::Thing_List list;
                
                
                map["onto"]  = {};
                map["under"] = {};
                
                /*
                 auto* src      = mtt::access<mtt::Thing_List>(thing, "source");
                 auto* dst      = mtt::access<mtt::Thing_List>(thing, "destination");
                 auto* init_src_ptr = mtt::access<vec3>(thing, "initial_position");
                 auto* src_selectors = mtt::access<std::vector<mtt::String>>(thing, "source_selector");
                 auto* dst_selectors = mtt::access<std::vector<mtt::String>>(thing, "destination_selector");
                 
                 // TODO: speed
                 auto* speed           = mtt::access<float32>(thing, "speed");
                 auto* jump_height_ref = mtt::access<float32>(thing, "jump_height");
                 
                 ASSERT_MSG(src      != nullptr, "");
                 ASSERT_MSG(dst      != nullptr, "");
                 ASSERT_MSG(init_src_ptr != nullptr, "");
                 ASSERT_MSG(src_selectors != nullptr, "");
                 ASSERT_MSG(dst_selectors != nullptr, "");
                 
                 float32 jump_height = *jump_height_ref;
                 auto* interpolation_ref = mtt::access<float32>(thing, "interpolation");
                 */
                if (preposition == nullptr && sequence_then != nullptr && !sequence_then->empty()) {
                    sequence_then->front()->try_get_prop("PREPOSITION", &preposition);
                }
                
                
                
                

                
                
                auto process_source = [&](mtt::Thing* source) -> Command* {
                    
                    
                    
                    
                    // TODO: for now just cancel all commands
                    
                    if (!args.is_deferred) {
                        auto find_cmd_ids = Command::thing_agent_to_cmd.find(source->id);
                        if (find_cmd_ids != Command::thing_agent_to_cmd.end()) {
                            for (auto c_it = find_cmd_ids->second.begin(); c_it != find_cmd_ids->second.end();) {
                                
                                Command_ID c_id = (*c_it);
                                Command* c_ = Command::lookup(c_id);
                                if (c_ == nullptr) {
                                    c_it = find_cmd_ids->second.erase(c_it);
                                    continue;
                                }
                                if (c_->must_be_canceled) {
                                    if (!c_->stop(c_)) {
                                        c_it = find_cmd_ids->second.erase(c_it);
                                    }
                                    //Command::destroy(c_->id);
                                } else {
                                    ++c_it;
                                }
                            }
                        }
                    }
                    
                    Command* cmd = Command::make();
                    cmd->behavior_list.push_back(&b);
                    cmd->label = b.key_primary + ":" + b.key_secondary;
                    
                    auto* root_thing = mtt::Thing_make(args.world, mtt::ARCHETYPE_GROUP);

                    cmd->element_map["ROOT"].push_back(mtt::Thing_Ref(root_thing));
                    
                    Prop_List prepositions;
                    if (preposition != nullptr) {
                        for (auto p_it = preposition->begin(); p_it != preposition->end(); ++p_it) {
                            auto& prep = *(*p_it);
                            if (prep.parent_ref_disabled) {
                                continue;
                            }
                            
                            prepositions.push_back(&prep);
                        }
                    }
                    
                    {
                        
                        if (prepositions.empty()) {
                            
                            auto* original_pos = mtt::access<vec3>(source, "position");
                            
                            mtt::Rep* rep = nullptr;
                            mtt::Thing* thing_temp_target = mtt::Thing_make_with_unit_collider(args.world, mtt::ARCHETYPE_FREEHAND_SKETCH, vec2(1.0f), &rep, mtt::COLLIDER_TYPE_AABB, *original_pos);
                            
                            auto* jump_thing = mtt::Thing_make(args.world, mtt::ARCHETYPE_JUMP);
                            mtt::Thing* thing = jump_thing;
                            mtt::connect_parent_to_child(args.world, root_thing, jump_thing);
                            
                            
                            cmd->element_map["ACTION"].push_back(mtt::Thing_Ref(jump_thing));
                            
                            auto* src      = mtt::access<mtt::Thing_Ref>(thing, "source");
                            auto* dst      = mtt::access<mtt::Thing_Ref>(thing, "destination");
                            
                            auto* src_selector = mtt::access<MTT_String_Ref>(thing, "source_selector");
                            MTT_string_ref_release(src_selector);
                            (*src_selector) = MTT_string_add(0, "center");
                            auto* dst_selector = mtt::access<MTT_String_Ref>(thing, "destination_selector");
                            MTT_string_ref_release(dst_selector);
                            (*dst_selector) = MTT_string_add(0, "center");
                            

                            *src = mtt::Thing_Ref(source);
                            *dst = mtt::Thing_Ref(thing_temp_target);
                            
                            auto* jump_height_ref = mtt::access<float32>(thing, "jump_height");
                            auto* cycle_count = mtt::access<uint64>(thing, "cycle_count");
                            
                            float32 height = b.logic_interfaces.fields["height"]["DEFAULT"].value.Float;
                            *jump_height_ref = height;
                            
                            *cycle_count = b.logic_interfaces.fields["cycle_count"]["DEFAULT"].value.Int64;
                            
                            
                            mtt::connect_parent_to_child(args.world, root_thing, thing_temp_target);
                            mtt::disconnect_child_from_parent(source->world(), source);
                            
                            auto* end = mtt::Thing_make(source->world(), mtt::ARCHETYPE_END);
                            mtt::connect_parent_to_child(args.world, root_thing, end);
                            cmd->element_map["END"].push_back(mtt::Thing_Ref(end));
                            
                            mtt::add_connection(args.world, jump_thing, "is_done", end, "is_done");
                            
                            
                            Command_ID cmd_id = cmd->id;
                            mtt::Thing_ID root_id = root_thing->id;
                            
                            // TEST: ...
                            thing->on_start = [](mtt::Thing* self_thing, mtt::Thing* thing, void* dst) {

                                mtt::disconnect_child_from_parent(thing->world(), thing);
//                                if (mtt::is_ancestor_of(thing, (mtt::Thing*)dst)) {
//                                    mtt::set_movement_mode((mtt::Thing*)dst, mtt::OPTION_FLAGS_MOVEMENT_POSITIONAL);
//                                }
                                
                                
                                dt::Thing_add_action(thing, dt::verb_lookup("jump"), nullptr);
                            };
                            
                            thing->on_end = [](mtt::Thing* jmp)
                            {
                                auto* src = mtt::access<mtt::Thing_Ref>(jmp, "source");
                                auto* dst = mtt::access<mtt::Thing_Ref>(jmp, "destination");
                                
                                if (!src->is_valid() || !dst->is_valid()) {
                                    return;
                                }
                                
                                mtt::Thing* src_thing = jmp->world()->Thing_try_get(src->id);
                                mtt::Thing* dst_thing = jmp->world()->Thing_try_get(dst->id);
                                if (src_thing == nullptr || dst_thing == nullptr) {
                                    return;
                                }

                                
//                                mtt::set_movement_mode(src_thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
                                
                                dt::Thing_remove_action(src_thing, dt::verb_lookup("jump"), nullptr);
                            };
                            
                            end->on_end = [root_id, cmd_id](mtt::Thing* end)
                            {
                                mtt::Thing_destroy_self_and_connected(mtt::ctx(), root_id);
                                dt::Command::destroy(cmd_id);
                            };

                            Command::thing_agent_to_cmd[source->id].push_back(cmd->id);
                        } else {
                            
                            auto* selector = mtt::Thing_make(args.world, mtt::ARCHETYPE_SELECTOR);
                            mtt::connect_parent_to_child(args.world, root_thing, selector);
                            
                            usize selector_out_count = 0;
                            
                            cmd->element_map["SELECTOR"].push_back(mtt::Thing_Ref(selector));
                            
                        
                            mtt::Thing* jump_thing = nullptr;
                            bool valid_preposition_exists = false;
                            
                            
                            
                            // NOTE: only one preposition for now
                            for (auto p_it = prepositions.begin(); p_it != prepositions.end(); ++p_it) {
                                auto& prep = *(*p_it);
                                
                                {
                                    if (!prep.try_get_prop("OBJECT", &object)) {
                                        //
                                        continue;
                                    } else {
                                        

                                        

                                        
                                        for (auto o_it = object->begin(); o_it != object->end(); ++o_it) {
                                            auto& obj = *itv(o_it);
                                            
                                            if (obj.parent_ref_disabled) {
                                                continue;
                                            }

                                            
                                            dt::Dynamic_Array<mtt::Thing_ID> targets;
                                            mtt::Thing_ID id = mtt::Thing_ID_INVALID;
                                            if (obj.value.kind_string == "THING_INSTANCE") {
                                                targets.push_back(obj.value.thing);
                                            } else if (obj.value.kind_string == "LIST") {
                                                targets.reserve(obj.value.list.size());
                                                for (auto t = obj.value.list.begin(); t != obj.value.list.end(); ++t) {
                                                    targets.push_back((*t).thing);
                                                }
                                            }
                                            for (auto id_it = targets.begin(); id_it != targets.end(); ++id_it) {
                                                id = *id_it;
                                                
                                                
                                                mtt::Thing* target  = args.world->Thing_try_get(id);
                                                if (!source || !target) {
                                                    MTT_error("Missing target! %s %d\n", __PRETTY_FUNCTION__, __LINE__);
                                                    continue;
                                                }
                                                
                                                if (source == target) {
                                                    continue;
                                                }
                                                
                                                jump_thing = mtt::Thing_make(args.world, mtt::ARCHETYPE_JUMP);
                                                mtt::Thing* thing = jump_thing;
                                                
                                                cmd->element_map["ACTION"].push_back(mtt::Thing_Ref(jump_thing));
                                                mtt::connect_parent_to_child(args.world, root_thing, jump_thing);
                                                
                                                auto* src      = mtt::access<mtt::Thing_Ref>(thing, "source");
                                                auto* dst      = mtt::access<mtt::Thing_Ref>(thing, "destination");
                                                
                                                *src = mtt::Thing_Ref(source);
                                                
                                                thing->on_start = [&](mtt::Thing* thing, mtt::Thing* _, void* dst) {

                                                    
                                                    
                                                    
                                                    auto* src_selector = mtt::access<MTT_String_Ref>(thing, "source_selector");
                                                    MTT_string_ref_release(src_selector);
                                                    (*src_selector) = MTT_string_add(0, "center");
                                                    auto* dst_selector = mtt::access<MTT_String_Ref>(thing, "destination_selector");
                                                    MTT_string_ref_release(dst_selector);
                                                    
                                                    
                                                    
                                                    if (prep.label == "onto" || prep.label == "on" || prep.label == "atop" || prep.label == "to" || prep.label == "between") {
                                                        (*dst_selector) = MTT_string_add(0, "top_with_offset");
                                                    } else if (prep.label == "below" || prep.label == "beneath" || prep.label == "under" || prep.label == "under") {
                                                        (*dst_selector) = MTT_string_add(0, "bottom_with_offset");
                                                    } else if (prep.label == "beside" || prep.label == "near") {
                                                        auto* src_pos = mtt::access<vec3>(source, "position");
                                                        auto* dst_pos = mtt::access<vec3>(target, "position");
                                                        if (src_pos->x < dst_pos->x) {
                                                            (*dst_selector) = MTT_string_add(0, "left_with_offset");
                                                        } else {
                                                            (*dst_selector) = MTT_string_add(0, "right_with_offset");
                                                        }
                                                    } else if (prep.label == "inside") {
                                                        (*dst_selector) = MTT_string_add(0, "center");
                                                    } else {
                                                        (*dst_selector) = MTT_string_add(0, "top_with_offset");
                                                    }
                      
                                                    thing = _;
                                                    
                                                    mtt::disconnect_child_from_parent(thing->world(), thing);
                                                    if (mtt::is_ancestor_of(thing, (mtt::Thing*)dst)) {
//                                                        mtt::set_movement_mode((mtt::Thing*)dst, mtt::OPTION_FLAGS_MOVEMENT_POSITIONAL);
                                                    }
                                                    
                                                    dt::Thing_add_action(thing, dt::verb_lookup("jump"), nullptr);

                                                };
                                                
                                                
                                                {

                                                    *dst = mtt::Thing_Ref(target);
                                                    
                                                    //auto* init_src_ptr = mtt::access<vec3>(thing, "initial_position");
                                                    //auto* original_pos = mtt::access<vec3>(source, "position");
                                                    //*init_src_ptr = *original_pos;
                                                    

                                                    
                                                    auto* jump_height_ref = mtt::access<float32>(thing, "jump_height");
                                                    auto* cycle_count = mtt::access<uint64>(thing, "cycle_count");
                                                    
                                                
                                                    float32 height = b.logic_interfaces.fields["height"]["DEFAULT"].value.Float;
                                                    *jump_height_ref = height;
                                                    
                                                    *cycle_count = b.logic_interfaces.fields["cycle_count"]["DEFAULT"].value.Int64;


                                                    if (prep.label == "with") {
                                                        thing->on_end = [](mtt::Thing* jmp)
                                                        {
                                                            //auto* is_init = mtt::access<bool>(jmp, "is_init");
                                                            //*is_init = false;
                                                            
                                                            auto* src = mtt::access<mtt::Thing_Ref>(jmp, "source");
                                                            auto* dst = mtt::access<mtt::Thing_Ref>(jmp, "destination");
                                                            
                                                            if (!src->is_valid() || !dst->is_valid()) {
                                                                return;
                                                            }
                                                            
                                                            mtt::Thing* src_thing = jmp->world()->Thing_try_get(src->id);
                                                            mtt::Thing* dst_thing = jmp->world()->Thing_try_get(dst->id);
//                                                            if (src_thing == nullptr || dst_thing == nullptr) {
//                                                                if (dst_thing != nullptr) {
//                                                                    mtt::set_movement_mode(dst_thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
//                                                                } else if (src_thing != nullptr) {
//                                                                    mtt::set_movement_mode(src_thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
//                                                                }
//                                                                return;
//                                                            }

//                                                            auto* dst_selectors = mtt::access<std::vector<mtt::String>>(jmp, "destination_selectors");
                                                            
                                                            
                                                            mtt::disconnect_child_from_parent(src_thing->world(), dst_thing);
                                                            mtt::connect_parent_to_child(src_thing->world(), src_thing, dst_thing);
                                                            
                                                            mtt::set_movement_mode(src_thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
                                                            mtt::set_movement_mode(dst_thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
                                                            
                                                            dt::Thing_remove_action(src_thing, dt::verb_lookup("jump"), nullptr);
                                                        };
                                                    } else {
                                                        thing->on_end = [](mtt::Thing* jmp)
                                                        {
                                                            //auto* is_init = mtt::access<bool>(jmp, "is_init");
                                                            //*is_init = false;
                                                            
                                                            auto* src = mtt::access<mtt::Thing_Ref>(jmp, "source");
                                                            auto* dst = mtt::access<mtt::Thing_Ref>(jmp, "destination");
                                                            
                                                            if (!src->is_valid() || !dst->is_valid()) {
                                                                return;
                                                            }
                                                            
                                                            mtt::Thing* src_thing = jmp->world()->Thing_try_get(src->id);
                                                            mtt::Thing* dst_thing = jmp->world()->Thing_try_get(dst->id);
//                                                            if (src_thing == nullptr || dst_thing == nullptr) {
//                                                                if (dst_thing != nullptr) {
//                                                                    mtt::set_movement_mode(dst_thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
//                                                                } else if (src_thing != nullptr) {
//                                                                    mtt::set_movement_mode(src_thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
//                                                                }
//                                                                return;
//                                                            }

                                                            auto* dst_selector = mtt::access<MTT_String_Ref>(jmp, "destination_selector");

                                                            
                                                            if (!mtt::is_ancestor_of(src_thing, dst_thing) &&
                                                                !MTT_string_ref_is_equal_cstring(*dst_selector, "left_with_offset") && !MTT_string_ref_is_equal_cstring(*dst_selector, "right_with_offset")) {
                                                                mtt::connect_parent_to_child(src_thing->world(), dst_thing, src_thing);
                                                            }
//                                                            mtt::set_movement_mode(src_thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
//                                                            mtt::set_movement_mode(dst_thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
                                                            
                                                            dt::Thing_remove_action(src_thing, dt::verb_lookup("jump"), nullptr);
                                                            

                                                        };
                                                        
                                                    }
                                                    
                                                    valid_preposition_exists = true;
                                                }
                                                
                                                
                                            }
                                            

                                            
                                        }
                                        
                                        
                                    }
                                }
                            }
                            
                            if (valid_preposition_exists) {
                                Command::thing_agent_to_cmd[source->id].push_back(cmd->id);
                            } else {
                                Command::destroy(cmd->id);
                                return nullptr;
                            }
                            
                            auto& actions = cmd->element_map["ACTION"];
                            usize selector_port = 0;
                            FOR_ITER(a_it, actions, ++a_it) {
                                auto& val = itv(a_it);
                                mtt::Thing* th;
                                if (!val.try_get(&th)) {
                                    continue;
                                }
                                
                                {
                                    bool ok = mtt::add_connection(args.world, selector, std::to_string(selector_port), th, "active");
                                    ASSERT_MSG(ok, "connection failed!\n");
                                }
                                {
                                    bool ok = mtt::add_connection(args.world, th, "is_done", selector, std::to_string(selector_port + 1));
                                    ASSERT_MSG(ok, "connection failed!\n");
                                }
                                
                                selector_port += 1;
                            }

                            
                            bool back_found  = false;
                            bool forth_found = false;
                            bool forever_found     = false;
                            bool repeatedly_found  = false;
                            bool endlessly_found   = false;
                            bool infinitely_found  = false;
                            usize over_found_count = 0;
                            float32 times = 1;
                            if (property != nullptr) {
                                FOR_PTR_ITER(a_props, property, ++a_props) {
                                    auto* pr = itv(a_props);
                                    value_TEXT(pr, [&](auto& text) {
                                        if (text == "back") {
                                            back_found = true;
                                        } else if (text == "forth") {
                                            forth_found = true;
                                        } else if (text == "forever" || text == "evermore" || text == "forevermore") {
                                            forever_found = true;
                                        } else if (text == "repeatedly") {
                                            repeatedly_found = true;
                                        } else if (text == "endlessly") {
                                            endlessly_found = true;
                                        } else if (text == "infinitely") {
                                            infinitely_found = true;
                                        } else if (text == "over") {
                                            over_found_count += 1;
                                        } else if (word_is_temporal(pr->label)) {
                                            // TODO
                                            pr->get_only_then("COUNT", [&](auto& count_prop) {
                                                times = value_NUMERIC(count_prop);
                                            });
                                            
                                        } else if (text == "twice") {
                                            times = 2;
                                        } else if (text == "thrice") {
                                            times = 3;
                                        } else if (text == "once") {
                                            times = 1;
                                        }
                                    });
                                    
                                }
                            }
                            Command_ID cmd_id = cmd->id;
                            mtt::Thing_ID root_id = root_thing->id;
                            
                            if (times == 1 && ((back_found && forth_found) ||
                                (forever_found) ||
                                (repeatedly_found) ||
                                (endlessly_found) ||
                                (infinitely_found) ||
                                (over_found_count > 1)
                                )) {
                                {
                                    bool ok = mtt::add_connection(args.world, selector,  std::to_string(selector_port), selector, "0");
                                    ASSERT_MSG(ok, "connection failed!\n");
                                    
                                    cmd->stop = [](Command* cmd) -> bool {
                                        auto& roots = cmd->element_map["ROOT"];
                                        for (auto it = roots.begin(); it != roots.end(); ++it) {
                                            mtt::Thing_destroy_self_and_connected(mtt::ctx(), itv(it));
                                        }
                                        
                                        dt::Command::destroy(cmd);
                                        return false;
                                    };
                                }
                            } else if (times > 1) {

                                
                                mtt::Thing* and_thing = mtt::Thing_make(args.world, mtt::ARCHETYPE_AND);
                                cmd->element_map["LOGIC"].push_back(and_thing);
                                
                                {
                                    // end when the counter reaches the target, increment counter to target otherwise
                                    auto* counter = mtt::Thing_make(source->world(), mtt::ARCHETYPE_COUNTER);
                                    mtt::connect_parent_to_child(args.world, root_thing, counter);
                                    cmd->element_map["COUNT"].push_back(mtt::Thing_Ref(counter));
                                    
                                    auto* max_count = mtt::access<uint64>(counter, "max_count");
                                    *max_count = times;
                                    
                                    {
                                        //bool ok = mtt::add_connection(args.world, selector,  std::to_string(selector_port), counter, "increment");
                                        //ASSERT_MSG(ok, "connection failed!\n");
                                    }
                                    auto* end = mtt::Thing_make(source->world(), mtt::ARCHETYPE_END);
                                    mtt::connect_parent_to_child(args.world, root_thing, end);
                                    cmd->element_map["END"].push_back(mtt::Thing_Ref(end));
                                    mtt::Thing* last_action = nullptr;
                                    {
                                        
                                        {
                                            
                                            actions.back().try_get(&last_action);
                                            
                                            mtt::add_connection(args.world, last_action, "is_done", counter, "increment");

                                        }
                                        
                                        auto* selector_port_count = mtt::access<uint64>(selector, "index_count");
                                        *selector_port_count = selector_port + 1;
                                        
                                        Command_ID cmd_id = cmd->id;
                                        end->on_end = [cmd_id](mtt::Thing* end)
                                        {
                                            mtt::Thing_destroy_self_and_connected(mtt::ctx(), end->id);
                                            dt::Command::destroy(cmd_id);
                                        };
                                    }
                                    
                                    mtt::Thing* not_thing_selector = mtt::Thing_make(args.world, mtt::ARCHETYPE_NOT);
                                    mtt::connect_parent_to_child(args.world, root_thing, not_thing_selector);
//                                    mtt::Thing* and_thing_selector = mtt::Thing_make(args.world, mtt::ARCHETYPE_AND);
//                                    mtt::connect_parent_to_child(args.world, root_thing, and_thing_selector);
                                    //mtt::Thing* not_thing = mtt::Thin g_make(args.world, mtt::ARCHETYPE_NOT);
                                    
                                    mtt::Thing* and_thing_done = mtt::Thing_make(args.world, mtt::ARCHETYPE_AND);
                                    cmd->element_map["LOGIC"].push_back(not_thing_selector);
                                    cmd->element_map["LOGIC"].push_back(and_thing_done);
//                                    cmd->element_map["LOGIC"].push_back(and_thing_selector);
                                    //cmd->element_map["LOGIC"].push_back(and_thing);
                                    // loop around to beginning
                                    {

                                        // BEFORE: //
                                        mtt::Thing* and_done2 = mtt::Thing_make(args.world, mtt::ARCHETYPE_AND);
                                        if (true) {
                                            bool ok =  mtt::add_connection(args.world, counter, "result", not_thing_selector, "value");
                                            ok = ok && mtt::add_connection(args.world, not_thing_selector, "value", and_thing, "0");
                                            ok = ok && mtt::add_connection(args.world, selector, std::to_string(selector_port), and_thing, "1");
                                            ok = ok && mtt::add_connection(args.world, and_thing, "value", selector, "0");
                                        
                                            ok = ok && mtt::add_connection(args.world, selector, std::to_string(selector_port), and_thing_done, "0");
                                            ok = ok && mtt::add_connection(args.world, counter, "result", and_thing_done, "1");
                                        
                                            
                                            ok = ok && mtt::add_connection(args.world, and_thing_done, "value", and_done2, "0");
                                            ok = ok && mtt::add_connection(args.world, last_action, "is_done", and_done2, "1");
                                            ok = ok && mtt::add_connection(args.world, and_done2, "value", end, "is_done");
                                            ASSERT_MSG(ok, "connection failed!\n");
                                        }
                                        
//                                        if constexpr ((false)) {
//                                            // if last port + counter result + jump done -> end : loop around
//
//                                            bool ok = true;
//                                            mtt::Thing* and0 = and_thing_done;
//                                            mtt::Thing* and1 = and_done2;
//
//                                            mtt::Thing* not_done = not_thing_selector;
//                                            mtt::Thing* and_for_else = mtt::Thing_make(args.world, mtt::ARCHETYPE_AND);
//                                            ok = ok && mtt::add_connection(args.world, selector, std::to_string(selector_port), and0, "0");
//                                            ok = ok && mtt::add_connection(args.world, counter, "result", and0, "1");
//                                                ok = ok && mtt::add_connection(args.world, and0, "value", and1, "0");
//                                                ok = ok && mtt::add_connection(args.world, last_action, "is_done", and1, "1");
//                                            ok = ok && mtt::add_connection(args.world, and1, "value", end, "is_done");
//
//                                            ok = ok && mtt::add_connection(args.world, selector, std::to_string(selector_port), and_for_else, "0");
//                                            ok = ok && mtt::add_connection(args.world, and1, "value", not_done, "value");
//                                            ok = ok && mtt::add_connection(args.world, not_done, "value", and_for_else, "1");
//                                            ok = ok && mtt::add_connection(args.world, and_for_else, "value", selector, "0");
//
//                                            ASSERT_MSG(ok, "connection failed!\n");
//                                        }
                                        cmd->stop = [](Command* cmd) {
                                            auto& roots = cmd->element_map["ROOT"];
                                            for (auto it = roots.begin(); it != roots.end(); ++it) {
                                                mtt::Thing_destroy_self_and_connected(mtt::ctx(), itv(it));
                                            }

                                            dt::Command::destroy(cmd);
                                            return false;
                                        };
                                    }
                                }
                                
                                
                            } else {
                                
                                auto* end = mtt::Thing_make(source->world(), mtt::ARCHETYPE_END);
                                mtt::connect_parent_to_child(args.world, root_thing, end);
                                cmd->element_map["END"].push_back(mtt::Thing_Ref(end));
                                mtt::add_connection(args.world, selector, std::to_string(selector_port), end, "is_done");
                                
                                auto* selector_port_count = mtt::access<uint64>(selector, "index_count");
                                *selector_port_count = selector_port + 1;
                                
                                Command_ID cmd_id = cmd->id;
                                end->on_end = [cmd_id](mtt::Thing* end)
                                {
                                    mtt::Thing_destroy_self_and_connected(mtt::ctx(), end->id);
                                    dt::Command::destroy(cmd_id);
                                };
                            }
                        }
                        
                        
                        args.cmd_list->cmds.push_back(cmd);
                        return cmd;
                    }
                    
                };
                usize cmd_count = 0;
                

                
                dt::Dynamic_Array<mtt::Thing*> sources;
                for (usize i_src = 0; i_src < agent->size(); i_src += 1) {
                    auto* prop = (*agent)[i_src];
                    
                    if (prop->parent_ref_disabled) {
                        continue;
                    }
                    
                    mtt::Thing* source =  nullptr;
                    if (prop->value.kind_string == "THING_INSTANCE") {
                        source = args.world->Thing_try_get(prop->value.thing);
                        if (source != nullptr) {
                            sources.push_back(source);
                        }
                    } else if (prop->value.kind_string == "LIST") {
                        for (usize e = 0; e < prop->value.list.size(); e += 1) {
                            if (prop->value.list[e].kind_string == "THING_INSTANCE") {
                                source = args.world->Thing_try_get(prop->value.list[e].thing);
                                if (source != nullptr) {
                                    sources.push_back(source);
                                }
                            }
                        }
                    }
                }
                
                
                Command head;
                Command* last_cmd = &head;
                
                for (usize i_src = 0; i_src < sources.size(); i_src += 1) {
                    mtt::Thing* source = sources[i_src];

                    Command* inner_cmd = process_source(source);
                    if (inner_cmd != nullptr) {
                        cmd_count += 1;
                    } else {
                        continue;
                    }
                    last_cmd->successor_id = inner_cmd->id;
                    inner_cmd->predecessor_id = last_cmd->id;
                    last_cmd = inner_cmd;
                }
                
                if (&head != last_cmd) {
                    args.prev_command = last_cmd;
                }
                
                if (sequence_then != nullptr) {

                    {

                        for (auto seq_it = sequence_then->begin(); seq_it != sequence_then->end(); ++seq_it) {
                            DT_Behavior* b = nullptr;
                            if (args.catalogue->lookup((*seq_it)->label, (*seq_it)->sub_label, &b)) {
                                DT_Behavior::Args args_cpy = args;
                                args_cpy.behavior = b;
                                args_cpy.current = *seq_it;
                                args_cpy.siblings = sequence_then;
                                args_cpy.is_deferred = true;
                                
                                if (last_cmd == nullptr || last_cmd->element_map["END"].empty()) {
                                    continue;
                                }
                                auto x = last_cmd->element_map["END"].back();
                                mtt::Thing* end_trigger;
                                x.try_get(&end_trigger);
                                
                                mtt::Thing* on_counter = mtt::Thing_make(args.world, mtt::ARCHETYPE_COUNTER);
                                auto* max_count = mtt::access<uint64>(on_counter, "max_count");
                                *max_count = 1;
                                mtt::connect_parent_to_child(args.world, end_trigger, on_counter);
                                mtt::add_connection(args.world, end_trigger, "is_done", on_counter, "increment");
                                
                                last_cmd = b->start(args_cpy);
                                
                                if (last_cmd != nullptr) {
                                    auto& sub_roots = last_cmd->element_map["ROOT"];
                                    for (auto s_it = sub_roots.begin(); s_it != sub_roots.end(); ++s_it) {
                                        
                                        mtt::Thing* root_sub_t;
                                        (*s_it).try_get(&root_sub_t);
                                        
                                        mtt::add_connection(args.world, on_counter, "result", root_sub_t, "active");
                                        
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (sequence_sim != nullptr) {

                    {

                        for (auto seq_it = sequence_sim->begin(); seq_it != sequence_sim->end(); ++seq_it) {
                            DT_Behavior* b = nullptr;
                            if (args.catalogue->lookup((*seq_it)->label, (*seq_it)->sub_label, &b)) {
                                DT_Behavior::Args args_cpy = args;
                                args_cpy.behavior = b;
                                args_cpy.current = *seq_it;
                                args_cpy.siblings = sequence_sim;
                                args_cpy.is_deferred = true;
                                
                                auto x = last_cmd->element_map["END"].back();
                                mtt::Thing* end_trigger;
                                x.try_get(&end_trigger);
                                
                                mtt::Thing* on_counter = mtt::Thing_make(args.world, mtt::ARCHETYPE_COUNTER);
                                auto* max_count = mtt::access<uint64>(on_counter, "max_count");
                                *max_count = 1;
                                auto* count = mtt::access<uint64>(on_counter, "count");
                                *count = 1;
                                mtt::connect_parent_to_child(args.world, end_trigger, on_counter);
                                mtt::add_connection(args.world, end_trigger, "is_done", on_counter, "increment");
                                
                                last_cmd = b->start(args_cpy);
                                
                                if (last_cmd != nullptr) {
                                    auto& sub_roots = last_cmd->element_map["ROOT"];
                                    for (auto s_it = sub_roots.begin(); s_it != sub_roots.end(); ++s_it) {
                                        
                                        mtt::Thing* root_sub_t;
                                        (*s_it).try_get(&root_sub_t);
                                        
                                        mtt::add_connection(args.world, on_counter, "result", root_sub_t, "active");
                                        
                                    }
                                }
                            }
                        }
                    }
                }
                
                return Command::lookup(head.successor_id);
                
            });
            
        }
        
        {
            auto& entry = *bc.insert("glide", "", true, "jump", "");
            entry.logic_interfaces.fields["height"]["DEFAULT"].value = mtt::Any::from_Float(10.0f);
        }
        {
            auto& entry = *bc.insert("pounce", "", true, "jump", "");
            entry.logic_interfaces.fields["height"]["DEFAULT"].value = mtt::Any::from_Float(500.0f);
            entry.logic_interfaces.fields["speed"]["DEFAULT"].value = mtt::Any::from_Float(4.0f);
        }
        {
            auto& entry = *bc.insert("snake", "", true, "jump", "");
            entry.logic_interfaces.fields["height"]["DEFAULT"].value = mtt::Any::from_Float(75.0f);
            entry.logic_interfaces.fields["cycle_count"]["DEFAULT"].value = mtt::Any::from_Int64(8);
        }
        {
            auto& entry = *bc.insert("swim", "", true, "jump", "");
            entry.logic_interfaces.fields["height"]["DEFAULT"].value = mtt::Any::from_Float(40.0f);
            entry.logic_interfaces.fields["cycle_count"]["DEFAULT"].value = mtt::Any::from_Int64(5);
        }
        
        
        
        bc.insert("move",      "physical", false, "", "");
        bc.insert("run",       "physical", true,  "move", "physical");
        bc.insert("operate",   "relation", false,  "", "");
        bc.insert("follow",    "physical", false, "", "");
        bc.insert("chase",     "physical", true,  "follow", "physical");
        bc.insert("flee",      "physical", false, "", "");
        bc.insert("destroy",   "",         false, "", "");
        bc.insert("annihilate","",         false, "", "");
        bc.insert("destruct",  "",         true,  "destroy", "");
        bc.insert("create",    "",         false, "", "");
        bc.insert("rotate",    "",         false, "", "");
        bc.insert("push",      "physical", false, "", "");
        bc.insert("pull",      "physical", false, "", "");
        bc.insert("collide",   "", false, "", "");
        bc.insert("overlap",   "", false, "", "");
        bc.insert("deflect",   "", false, "", "");
        bc.insert("fall",      "", false, "", "");
        bc.insert("throw",     "", false, "", "");
        auto& drop = *bc.insert("drop",      "", false, "", "");
        drop.set_on_start([](DT_Behavior::Args& args) -> Command* {
            
            using Prop_List = Speech_Property::Prop_List;
            
            Prop_List* agent                 = nullptr;
            Prop_List* object                = nullptr;
            Prop_List* direct_object         = nullptr;
            Prop_List* indirect_object       = nullptr;
            
            Prop_List* property              = nullptr;
            Prop_List* time                  = nullptr;
            Prop_List* preposition           = nullptr;
            Prop_List* sequence_then         = nullptr;
            Prop_List* sequence_sim         = nullptr;
            //Prop_List* preposition_object    = nullptr;
            //Prop_List* action                = nullptr;
            
            DT_Behavior_Catalogue& catalogue = *args.catalogue;
            DT_Behavior& b = *args.behavior;
            
            Speech_Property& prop = *args.current;
            
            
            prop.try_get_prop("AGENT",           &agent);
            prop.try_get_prop("OBJECT",          &object);
            prop.try_get_prop("DIRECT_OBJECT",   &direct_object);
            prop.try_get_prop("INDIRECT_OBJECT", &indirect_object);
            // check if parent link is there. If not, should treat as an independent action
            prop.try_get_prop("SEQUENCE_THEN",   &sequence_then);
            prop.try_get_prop("SEQUENCE_SIMULTANEOUS",   &sequence_sim);
            prop.try_get_prop("PROPERTY",        &property);
            // will require queries
            prop.try_get_prop("TIME",            &time);
            
            prop.try_get_prop("PREPOSITION",     &preposition);
            
            if (args.is_trigger) {
                return nullptr;
            } else if (args.is_relation) {
                return nullptr;
            }
            
            MTT_print("%s", "=-===========================================================\n");
            //prop.print();
            if (agent == nullptr || agent->empty()) {
                return nullptr;
            }
            

            
            struct Role_Entity;
            struct Role_Entity {
                mtt::String label = "";
                mtt::String sub_label;
                mtt::String key;
                mtt::String type;
                
                mtt::Thing_List things;
                Port_Connection_Descriptor ports;
                Field_Modify_Descriptor fields;
                
                mtt::Map_Stable<mtt::String, Role_Entity*> map;
                
                Role_Entity() {}
            };
            mtt::Map_Stable<mtt::String, Role_Entity*> map = {};
            
            
            
            
            mtt::Thing_List list;
            
            if (preposition == nullptr && sequence_then != nullptr && !sequence_then->empty()) {
                sequence_then->front()->try_get_prop("PREPOSITION", &preposition);
            }
            if (direct_object == nullptr && sequence_then != nullptr && !sequence_then->empty()) {
                sequence_then->front()->try_get_prop("DIRECT_OBJECT", &direct_object);
            }
            

            auto process_source = [&](mtt::Thing* source) -> Command* {
                Prop_List prepositions;
                bool has_onto = false;
                if (preposition != nullptr) {
                    for (auto p_it = preposition->begin(); p_it != preposition->end(); ++p_it) {
                        auto& prep = *(*p_it);
                        if (prep.parent_ref_disabled) {
                            continue;
                        }
                        
                        if (prep.label == "onto") {
                            has_onto = true;
                            break;
                        }
                        
                        prepositions.push_back(&prep);
                    }
                }
                
                if (has_onto) {
                    DT_Behavior* j_b = nullptr;
                    if (catalogue.lookup("jump", "", &j_b)) {
                        return j_b->start(args);
                    }
                    return nullptr;
                }
                
                Prop_List direct_objects;
                
                if (direct_object != nullptr) {
                    for (auto do_it = direct_object->begin(); do_it != direct_object->end(); ++do_it) {
                        auto& dobj = *(*do_it);
                        if (dobj.parent_ref_disabled) {
                            continue;
                        }
                        
                        
                        
                        direct_objects.push_back(&dobj);
                    }
                    if (direct_objects.empty()) {
                        return nullptr;
                    }
                } else {
                    return nullptr;
                }
                

                Command* cmd = Command::make();
                cmd->behavior_list.push_back(&b);
                cmd->label = b.key_primary + ":" + b.key_secondary;
                auto* root_thing = mtt::Thing_make(args.world, mtt::ARCHETYPE_GROUP);
                cmd->element_map["ROOT"].push_back(mtt::Thing_Ref(root_thing));
                auto root_id = root_thing->id;

                for (auto do_it = direct_objects.begin(); do_it != direct_objects.end(); ++do_it) {
                    auto& dobj = *(*do_it);

                    dt::Dynamic_Array<mtt::Thing_ID> targets;
                    mtt::Thing_ID id = mtt::Thing_ID_INVALID;
                    if (dobj.value.kind_string == "THING_INSTANCE") {
                        targets.push_back(dobj.value.thing);
                    } else if (dobj.value.kind_string == "LIST") {
                        targets.reserve(dobj.value.list.size());
                        for (auto t = dobj.value.list.begin(); t != dobj.value.list.end(); ++t) {
                            targets.push_back((*t).thing);
                        }
                    }
                    for (auto id_it = targets.begin(); id_it != targets.end(); ++id_it) {
                        id = *id_it;
                        
                        
                        mtt::Thing* target  = args.world->Thing_try_get(id);
                        if (!source || !target) {
                            MTT_error("Missing target! %s %d\n", __PRETTY_FUNCTION__, __LINE__);
                            continue;
                        }
                        
                        if (source == target) {
                            continue;
                        }
                        
                        auto* end = mtt::Thing_make(source->world(), mtt::ARCHETYPE_END);
                        mtt::connect_parent_to_child(args.world, root_thing, end);
                        cmd->element_map["END"].push_back(mtt::Thing_Ref(end));
                        
                        
                        auto* power = mtt::Thing_make(args.world, mtt::ARCHETYPE_POWER);
                        auto* power_val = mtt::access<float32>(power, "power");
                        *power_val = 1.0f;
                        mtt::connect_parent_to_child(args.world, root_thing, power);
                        mtt::add_connection(args.world, power, "power_digital", end, "is_done");
                        
                        auto cmd_id = cmd->id;
                        auto source_id = source->id;
                        auto target_id = target->id;
                        end->on_end = [root_id, cmd_id, source_id, target_id](mtt::Thing* end)
                        {
                            mtt::Thing* source = end->world()->Thing_try_get(source_id);
                            mtt::Thing* target = end->world()->Thing_try_get(target_id);
                            
                            if (source != nullptr && target != nullptr) {
                                if (mtt::exists_in_other_hierarchy(target, source)) {
                                    mtt::disconnect_child_from_parent(target->world(), target);
                                }
                            }
                            
                            mtt::Thing_destroy_self_and_connected(mtt::ctx(), root_id);
                            dt::Command::destroy(cmd_id);
                        };
                    }
                }
                args.cmd_list->cmds.push_back(cmd);
                return cmd;
            };
            usize cmd_count = 0;
            

            
            dt::Dynamic_Array<mtt::Thing*> sources;
            for (usize i_src = 0; i_src < agent->size(); i_src += 1) {
                auto* prop = (*agent)[i_src];
                
                if (prop->parent_ref_disabled) {
                    continue;
                }
                
                mtt::Thing* source =  nullptr;
                if (prop->value.kind_string == "THING_INSTANCE") {
                    source = args.world->Thing_try_get(prop->value.thing);
                    if (source != nullptr) {
                        sources.push_back(source);
                    }
                } else if (prop->value.kind_string == "LIST") {
                    for (usize e = 0; e < prop->value.list.size(); e += 1) {
                        if (prop->value.list[e].kind_string == "THING_INSTANCE") {
                            source = args.world->Thing_try_get(prop->value.list[e].thing);
                            if (source != nullptr) {
                                sources.push_back(source);
                            }
                        }
                    }
                }
            }
            
            
            Command head;
            Command* last_cmd = &head;
            
            for (usize i_src = 0; i_src < sources.size(); i_src += 1) {
                mtt::Thing* source = sources[i_src];

                Command* inner_cmd = process_source(source);
                if (inner_cmd != nullptr) {
                    cmd_count += 1;
                } else {
                    continue;
                }
                last_cmd->successor_id = inner_cmd->id;
                inner_cmd->predecessor_id = last_cmd->id;
                last_cmd = inner_cmd;
            }
            if (&head != last_cmd) {
                args.prev_command = last_cmd;
            }
            
            if (sequence_then != nullptr) {

                {

                    for (auto seq_it = sequence_then->begin(); seq_it != sequence_then->end(); ++seq_it) {
                        DT_Behavior* b = nullptr;
                        if (args.catalogue->lookup((*seq_it)->label, (*seq_it)->sub_label, &b)) {
                            DT_Behavior::Args args_cpy = args;
                            args_cpy.behavior = b;
                            args_cpy.current = *seq_it;
                            args_cpy.siblings = sequence_then;
                            args_cpy.is_deferred = true;
                            
                            if (last_cmd->element_map.find("END") == last_cmd->element_map.end()) {
                                continue;
                            }
                            auto x = last_cmd->element_map["END"].back();
                            mtt::Thing* end_trigger;
                            x.try_get(&end_trigger);
                            
                            mtt::Thing* on_counter = mtt::Thing_make(args.world, mtt::ARCHETYPE_COUNTER);
                            auto* max_count = mtt::access<uint64>(on_counter, "max_count");
                            *max_count = 1;
                            mtt::connect_parent_to_child(args.world, end_trigger, on_counter);
                            mtt::add_connection(args.world, end_trigger, "is_done", on_counter, "increment");
                            
                            last_cmd = b->start(args_cpy);
                            
                            if (last_cmd != nullptr) {
                                auto& sub_roots = last_cmd->element_map["ROOT"];
                                for (auto s_it = sub_roots.begin(); s_it != sub_roots.end(); ++s_it) {
                                    
                                    mtt::Thing* root_sub_t;
                                    (*s_it).try_get(&root_sub_t);
                                    
                                    mtt::add_connection(args.world, on_counter, "result", root_sub_t, "active");
                                    
                                }
                            }
                        }
                    }
                }
            }
            
            if (sequence_sim != nullptr) {

                {

                    for (auto seq_it = sequence_sim->begin(); seq_it != sequence_sim->end(); ++seq_it) {
                        DT_Behavior* b = nullptr;
                        if (args.catalogue->lookup((*seq_it)->label, (*seq_it)->sub_label, &b)) {
                            DT_Behavior::Args args_cpy = args;
                            args_cpy.behavior = b;
                            args_cpy.current = *seq_it;
                            args_cpy.siblings = sequence_sim;
                            args_cpy.is_deferred = true;
                            
                            if (last_cmd->element_map.find("END") == last_cmd->element_map.end()) {
                                continue;
                            }
                            
                            auto x = last_cmd->element_map["END"].back();
                            mtt::Thing* end_trigger;
                            x.try_get(&end_trigger);
                            
                            mtt::Thing* on_counter = mtt::Thing_make(args.world, mtt::ARCHETYPE_COUNTER);
                            auto* max_count = mtt::access<uint64>(on_counter, "max_count");
                            *max_count = 1;
                            auto* count = mtt::access<uint64>(on_counter, "count");
                            *count = 1;
                            mtt::connect_parent_to_child(args.world, end_trigger, on_counter);
                            mtt::add_connection(args.world, end_trigger, "is_done", on_counter, "increment");
                            
                            last_cmd = b->start(args_cpy);
                            
                            if (last_cmd != nullptr) {
                                auto& sub_roots = last_cmd->element_map["ROOT"];
                                for (auto s_it = sub_roots.begin(); s_it != sub_roots.end(); ++s_it) {
                                    
                                    mtt::Thing* root_sub_t;
                                    (*s_it).try_get(&root_sub_t);
                                    
                                    mtt::add_connection(args.world, on_counter, "result", root_sub_t, "active");
                                    
                                }
                            }
                        }
                    }
                }
            }
            
            return Command::lookup(head.successor_id);
            
        });
        bc.insert("pick",      "", false, "", "");
        bc.insert("fly",       "", false, "", "");
        bc.insert("ignore",    "", false, "", "");
        bc.insert("notice",    "", false, "", "");
        bc.insert("see",       "", true,  "notice", "");
        bc.insert("make",      "", true, "create", "");
        bc.insert("spawn",     "", true, "create", "");
        bc.insert("emit",      "", false, "", "");
        bc.insert("grow",      "", false, "", "");
        bc.insert("shrink",    "", false, "", "");
        bc.insert("take",      "", false, "", "");
        bc.insert("steal",     "", true, "take", "");
        bc.insert("transform", "", false, "", "");
        bc.insert("morph",     "", true, "transform", "");
        bc.insert("live",      "", false, "", "");
        bc.insert("exist",     "", false, "", "");
        {
            auto& entry = *bc.insert("increase",  "", false, "", "");
            entry.set_on_start([](DT_Behavior::Args& args) -> Command* {
                
                using Prop_List = Speech_Property::Prop_List;
                
                Prop_List* agent                 = nullptr;
                Prop_List* object                = nullptr;
                Prop_List* direct_object         = nullptr;
                Prop_List* indirect_object       = nullptr;
                
                Prop_List* property              = nullptr;
                Prop_List* time                  = nullptr;
                Prop_List* preposition           = nullptr;
                Prop_List* sequence_then         = nullptr;
                Prop_List* sequence_sim         = nullptr;
                //Prop_List* preposition_object    = nullptr;
                //Prop_List* action                = nullptr;
                
                DT_Behavior_Catalogue& catalogue = *args.catalogue;
                DT_Behavior& b = *args.behavior;
                
                Speech_Property& prop = *args.current;
                
                prop.try_get_prop("AGENT",           &agent);
                prop.try_get_prop("OBJECT",          &object);
                prop.try_get_prop("DIRECT_OBJECT",   &direct_object);
                prop.try_get_prop("INDIRECT_OBJECT", &indirect_object);
                // check if parent link is there. If not, should treat as an independent action
                prop.try_get_prop("SEQUENCE_THEN",   &sequence_then);
                prop.try_get_prop("SEQUENCE_SIMULTANEOUS",   &sequence_sim);
                prop.try_get_prop("PROPERTY",        &property);
                // will require queries
                prop.try_get_prop("TIME",            &time);
                
                prop.try_get_prop("PREPOSITION",     &preposition);
                
                Command* cmd = nullptr;
                
                
                if (args.is_trigger) {
                    
                    
                    if (args.is_relation) {
                     
                        
                        // TODO: ... create the logic graph
                        
                        
                        //auto& interop = cmd->interop["out"];
                        //interop.ports
                        
                        //
                        
                        if (agent != nullptr) {
                            if (direct_object != nullptr) {
                                
                            } else {
                                {

                                    
                                    
                                    dt::Dynamic_Array<Speech_Property*> agents;
                                    for (auto a_it = agent->begin(); a_it != agent->end(); ++a_it) {
                                        auto* prop = *a_it;
                                        if (prop->should_ignore()) {
                                            continue;
                                        }
                                        agents.push_back(prop);
                                    }
                                    if (agents.empty()) {
                                        return nullptr;
                                    }
                                    if (agents.size() == 1) {
                                        
                                        
                                        
                                        cmd = Command::make();
                                        mtt::Thing* root = mtt::Thing_make(args.world, mtt::ARCHETYPE_GROUP);
                                        cmd->element_map["ROOT"].push_back(mtt::Thing_Ref(root));
                                        mtt::Thing* and_gate = mtt::Thing_make(args.world, mtt::ARCHETYPE_AND);
                                        
                                        usize and_port = 0;
                                        
                                        mtt::connect_parent_to_child(args.world, root, and_gate);
                                        
                                        
                                        auto* prop = agents[0];
                                        
                                        mtt::Thing* src = args.world->Thing_try_get(prop->value.thing);


                                        if (numeric_value_words.find(prop->label)
                                            != numeric_value_words.end()) {
                                            
                                            
                                            //if (prop->value.kind_string == "THING_INSTANCE") {
                                                vec3 position = {};
                                                
                                                if (src == nullptr) {
                                                    
                                                } else {
                                                    
                                                    // replace the original thing with a number representing the value
                                                    mtt::Thing* number = mtt::Thing_make(args.world, mtt::ARCHETYPE_NUMBER);
                                                    {
                                                        position = *mtt::access<vec3>(src, "position");
                                                        mtt::Thing_set_position(number, position);
                                                        mtt::Thing_destroy(args.world, src);
                                                        dt::vis_word_derive_from(number, dt::noun_add(prop->label));
                                                        mtt::connect_parent_to_child(args.world, root, number);
                                                    }
                                                    
                                                    Prop_List* rel;
                                                    if (prop->try_get_prop("RELATION", &rel)) {
                                                        if (rel != nullptr) {
                                                            for (auto r_it = rel->begin(); r_it != rel->end(); ++r_it) {
                                                                auto* r_prop = *r_it;
                                                                if (r_prop->value.kind_string == "THING_INSTANCE") {
                                                                    mtt::Thing* src = args.world->Thing_try_get(r_prop->value.thing);
                                                                    if (src != nullptr) {
                                                                        if (prop->label == "height" || prop->label == "altitude") {
                                                                            
                                                                        } else if (prop->label == "distance") {
                                                                            
                                                                        }
                                                                    }
                                                                } else if (r_prop->value.kind_string == "LIST") {
                                                                    for (auto l_it = r_prop->value.list.begin(); l_it != r_prop->value.list.end(); ++l_it) {
                                                                        if ((*l_it).kind_string == "THING_INSTANCE") {
                                                                            mtt::Thing* src = args.world->Thing_try_get((*l_it).thing);
                                                                            if (src != nullptr) {
                                                                                if (prop->label == "height" || prop->label == "altitude") {
                                                                                    
                                                                                } else if (prop->label == "distance") {
                                                                                   
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                        
                                                    return nullptr;
                                                }
                                            
//                                            }//else if (prop->value.kind_string == "LIST") {
//                                                for (auto v_it = prop->value.list.begin();
//                                                     v_it != prop->value.list.end(); ++v_it) {
//
//                                                }
//                                            }
                                        }
                                    } else {
                                        
                                    }
                                    
                                }
                            }
                        } else if (direct_object != nullptr) {
                            
                        } else {
                            return nullptr;
                        }
                        
                        
                    } else {
                        
                    }
                    
                    return cmd;
                } else if (args.is_response) {
                    return nullptr;
                } else {
                    
                    
                    dt::Dynamic_Array<Speech_Property*> list;
                    usize increment = 0;
                    
                    if (agent == nullptr) {
                        if (direct_object == nullptr) {
                            return nullptr;
                        }
                        
                        increment = 1;
                        
                        for (auto a_it = direct_object->begin(); a_it != direct_object->end(); ++a_it) {
                            auto* prop = *a_it;
                            if (prop->should_ignore()) {
                                continue;
                            }
                            list.push_back(prop);
                        }
                        
                        
                    } else if (direct_object == nullptr) {
                        
                        increment = 1;
                        for (auto a_it = agent->begin(); a_it != agent->end(); ++a_it) {
                            auto* prop = *a_it;
                            if (prop->should_ignore()) {
                                continue;
                            }
                            list.push_back(prop);
                        }
                    } else {
                        prop.for_each("AGENT", [&](auto a_prop, auto _) {
                            for_each_value(*a_prop, [&](auto& val) {
                                value_THING_INSTANCE(val, [&](auto* thing) {
                                    if (thing->archetype_id == mtt::ARCHETYPE_NUMBER) {
                                        auto* val = mtt::access<float32>(thing, "value");
                                        increment += *val;
                                    }
                                });
                            });
                        });
                        
                        for (auto a_it = direct_object->begin(); a_it != direct_object->end(); ++a_it) {
                            auto* prop = *a_it;
                            if (prop->should_ignore()) {
                                continue;
                            }
                            list.push_back(prop);
                        }
                    }
                    
                    
                    //if (preposition != nullptr) {
                        
                        
//                        for (auto p_it = preposition->begin(); p_it != preposition->end(); ++p_it) {
//                            if ((*p_it)->should_ignore()) {
//                                continue;
//                            }
//
//                            if ((*p_it)->label == "by") {
                    
                            //if (property != nullptr)
                                for (auto a_it = list.begin(); a_it < list.end(); ++a_it) {
                                    auto* prop = *a_it;
                                    mtt::Thing* src = args.world->Thing_try_get(prop->value.thing);
                                    
                                    if (src->archetype_id == mtt::ARCHETYPE_NUMBER) {
                                        auto* val = mtt::access<float32>(src, "value");
                                        mtt::number_update_value(src, *val + increment);
                                    }
                                }
                    
                    
                            
                            
                        
                        return nullptr;
                    //}
                }
                
                
                return cmd;
                
                
            });
        }
        bc.insert("decrease",  "", false, "", "");
        bc.insert("cut",       "", false, "", "");
        bc.insert("rise",      "", false, "", "");
        bc.insert("ascend",    "", false, "", "");
        bc.insert("descend",   "", false, "", "");
        bc.insert("circulate", "", false, "", "");
        bc.insert("flow",      "", false, "", "");
        bc.insert("attach",    "", false, "", "");
        bc.insert("detach",    "", false, "", "");
        bc.insert("levitate",  "", false, "", "");
        bc.insert("hover",     "", true, "levitate", "");
        
    }
    
    {
        auto& functions = DT_Behavior_Config::list();
        for (auto& F : functions) {
            dt::DT_Behavior_Config conf = {};
            conf.name = F.name;
            F.func(dt, &conf);
            MTT_print("BEHAVIOR MODULE: [%s]\n", mtt::c_str(conf.name));
        }
        //dt::mod::test_behavior::init(ct);
    }
    MTT_BP();
}

}
