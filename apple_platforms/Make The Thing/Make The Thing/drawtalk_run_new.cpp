//
//  drawtalk_run_new.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 2/25/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#include "drawtalk_run_new.hpp"
#include "thing_lib.hpp"




namespace dt {

void create_new_action(mtt::Script* script, const mtt::String& label);

using namespace mtt;

using Prop             = Speech_Property;
using Prop_List        = Speech_Property::Prop_List;
using Active_Prop_List = Speech_Property::Active_Prop_List;

#define DT_DO_INSERT_PRINT_FOR_ACTION (0)
#if DT_DO_INSERT_PRINT_FOR_ACTION
#define DT_INSERT_PRINT_FOR_ACTION(scb, label) do { mtt::thing_make( &scb , mtt::ARCHETYPE_PRINT, label ) } while (0)
#else
#define DT_INSERT_PRINT_FOR_ACTION(...)
#endif



bool is_referenced_in_response(bool is_response, auto& refs, auto* prop)
{
    return refs.contains(prop);
}

typedef enum ENTRY_TYPE {
    ENTRY_TYPE_UNKNOWN,
    ENTRY_TYPE_ACTION,
    ENTRY_TYPE_TRIGGER_RESPONSE,
} ENTRY_TYPE;

struct Entries {
    dt::Dynamic_Array<ENTRY_TYPE> types = {};
    Speech_Property::Prop_List    props = {};
    
    inline void push_back(ENTRY_TYPE type, Speech_Property* prop)
    {
        types.push_back(type);
        props.push_back(prop);
    }
    
    inline void clear(void)
    {
        types.clear();
        props.clear();
    }
    
    inline usize size(void)
    {
        return props.size();
    }
    
    struct Pair {
        ENTRY_TYPE type;
        Speech_Property& prop;
    };
    
    const Pair get(usize i)
    {
        return {types[i], *props[i]};
    }
};

//const auto fill_in_agent_from_parent = [](Speech_Property::Active_Prop_List& prop_list) {
//    auto* self_prop = prop_list.container.front();
//    if (!self_prop->has_prop("AGENT")) {
//        auto* parent = self_prop->get_active_parent();
//        if (parent != nullptr) {
//            auto* parent_agent = parent->try_get_only_prop("AGENT");
//            if (parent_agent != nullptr) {
//                self_prop->push_prop("AGENT", parent_agent->copy());
//            }
//        }
//    }
//};

inline static mtt::Map_Stable<dt::Speech_Property*, mtt::Thing*> prop_thing_mapping = {};




void handle_call_generation(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args);

void action_proc_AND(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args);
void action_proc_THEN(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args);
void action_proc(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args);



struct COUNT_AND_TIME_Result {
    usize count = 1;
    usize right_count = 0;
    bool is_lefthand = false;
    float64 time_interval = 0.0f;
    bool count_per_interval = false;
    bool interval_per_count = false;
};

COUNT_AND_TIME_Result action_proc_COUNT_AND_TIME(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args);


void handle_call_generation(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args)
{
    // TODO: ...
    
    DT_Behavior* b = nullptr;
    if (cat->lookup(prop.label, prop.sub_label, &b)) {
        DT_print("FOUND ACTION: %s\n", (prop.label + ":" + prop.sub_label).c_str());

        //call_action(&scb, ...)
        dt::DT_Args args = {};
        args.dt = dt::DrawTalk::ctx();
        args.prop = &prop;
        args.cat = cat;
        args.script_builder = &scb;
        args.behavior = b;
        args.gen_args = gen_args;
        
        //b->on_generate
        if (b->on_generate(args).status == false) {
            scb.is_valid = false;
            return;
        } else if (b->on_generate_post(args).status == false) {
            scb.is_valid = false;
            return;
        }

    } else {
        DT_print("NOT FOUND!\n");
        scb.is_valid = false;
    }
}


COUNT_AND_TIME_Result action_proc_COUNT_AND_TIME(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args)
{
    bool back_found        = false;
    bool forth_found       = false;
    bool forever_found     = false;
    bool repeatedly_found  = false;
    bool endlessly_found   = false;
    bool infinitely_found  = false;
    bool infinite_time_interval_found = false;
    
    usize over_found_count = 0;
    usize times = 1;
    usize rtimes = 1;
    float64 time_interval = 0.0;
    
    COUNT_AND_TIME_Result out = {};
    
    bool lefthand = false;
    
    prop.get_only_then("TIME", [&](auto& time_prop) {
        if (time_prop->token->i < prop.token->i) {
            lefthand = true;
        }
        if (word_is_temporal(time_prop->label)) {
            if (time_prop->type_str == "INTERVAL") {
                infinite_time_interval_found = true;
            }
            time_prop->get_only_then("COUNT", [&](auto& interval_prop) {
                
                out.interval_per_count = true;
                // in seconds
                auto time_value = value_NUMERIC(interval_prop);
                if (time_prop->label == "minute") {
                    time_interval = time_value * 60;
                } else if (time_prop->label == "second") {
                    time_interval = time_value * 1;
                } else if (time_prop->label == "millisecond") {
                    time_interval = time_value / 1000.0;
                }
            });
            time_prop->get_only_then("PROPERTY", [&](auto& property_prop) {
                if (property_prop->value.kind_string == "TEXT") {
                    if (property_prop->value.text == "few") {
                        auto time_value = 3;
                        if (time_prop->label == "minute") {
                            time_interval = time_value * 60;
                        } else if (time_prop->label == "second") {
                            time_interval = time_value * 1;
                        } else if (time_prop->label == "millisecond") {
                            time_interval = time_value / 1000.0;
                        }
                    }
                }
            });

        }
    });
    
    prop.get_only_then("PREPOSITION", [&](auto& prep_prop) {
        prep_prop->get_only_then("TIME", [&](auto& time_prop) {
            if (time_prop->token == nullptr || prop.token == nullptr) {
                return;
            }
            
            if (time_prop->token->i < prop.token->i) {
                lefthand = true;
            }
            if (word_is_temporal(time_prop->label)) {
                if (time_prop->type_str == "INTERVAL") {
                    infinite_time_interval_found = true;
                }
                time_prop->get_only_then("COUNT", [&](auto& interval_prop) {
                    
                    out.interval_per_count = true;
                    // in seconds
                    auto time_value = value_NUMERIC(interval_prop);
                    if (time_prop->label == "minute") {
                        time_interval = time_value * 60;
                    } else if (time_prop->label == "second") {
                        time_interval = time_value * 1;
                    } else if (time_prop->label == "millisecond") {
                        time_interval = time_value / 1000.0;
                    }
                });
                time_prop->get_only_then("PROPERTY", [&](auto& property_prop) {
                    if (property_prop->value.kind_string == "TEXT") {
                        if (property_prop->value.text == "few") {
                            auto time_value = 3;
                            if (time_prop->label == "minute") {
                                time_interval = time_value * 60;
                            } else if (time_prop->label == "second") {
                                time_interval = time_value * 1;
                            } else if (time_prop->label == "millisecond") {
                                time_interval = time_value / 1000.0;
                            }
                        }
                    }
                });

            }
        });
    });
    
    
    
    for (auto pr : prop.get_active("PROPERTY")) {
        auto locallefthand = false;
        if (pr->token != nullptr && pr->token->i < prop.token->i) {
            locallefthand = true;
            lefthand = true;
        }
        if (pr->value.kind_string == "TEXT") {
            auto& text = pr->value.text;
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
            } else if (text == "twice") {
                if (!locallefthand) {
                    rtimes = 2;
                } else {
                    times = 2;
                }
            } else if (text == "thrice" || text == "few") {
                if (!locallefthand) {
                    rtimes = 3;
                } else {
                    times = 3;
                }
            } else if (text == "once") {
                if (!locallefthand) {
                    rtimes = 1;
                } else {
                    times = 1;
                }
            } else if (pr->label == "time") {
                pr->get_only_then("COUNT", [&](auto& count_prop) {
                    if (locallefthand) {
                        times = value_NUMERIC(count_prop);
                    } else {
                        rtimes = value_NUMERIC(count_prop);
                    }
                    pr->get_only_then("PROPERTY", [&](auto& sub_prop) {
                        if (word_is_temporal(sub_prop->label)) {
                            sub_prop->get_only_then("COUNT", [&](auto& count_per_interval_prop) {
                                // in seconds
                                auto time_value = value_NUMERIC(count_per_interval_prop);
                                if (sub_prop->label == "minute") {
                                    time_interval = time_value * 60;
                                } else if (sub_prop->label == "second") {
                                    time_interval = time_value * 1;
                                } else if (sub_prop->label == "millisecond") {
                                    time_interval = time_value / 1000.0;
                                }
                                
                                if (!out.interval_per_count) {
                                    out.count_per_interval = true;
                                }
                            });
                        }
                    });
                });
                
            } else if (word_is_temporal(pr->label)) {
                
            } else {
                pr->get_only_then("COUNT", [&](auto& count_prop) {
                    auto v = value_NUMERIC(count_prop);
                    
                    if (pr->value.kind_string == "TEXT") {
                        if (pr->value.text == "more") {
                            if (!locallefthand) {
                                rtimes = v;
                            } else {
                                times = v;
                            }
                        }
                    }
                });
            }
        } else {
            if (word_is_temporal(pr->label)) {
                pr->get_only_then("COUNT", [&](auto& count_prop) {
                    //float32 val = value_NUMERIC(count_prop);
                });
            } else if (pr->label == "time") {
                pr->get_only_then("COUNT", [&](auto& count_prop) {
                    if (locallefthand) {
                        times = value_NUMERIC(count_prop);
                    } else {
                        rtimes = value_NUMERIC(count_prop);
                    }
                    pr->get_only_then("PROPERTY", [&](auto& sub_prop) {
                        if (word_is_temporal(sub_prop->label)) {
                            sub_prop->get_only_then("COUNT", [&](auto& count_per_interval_prop) {
                                // in seconds
                                auto time_value = value_NUMERIC(count_per_interval_prop);
                                if (sub_prop->label == "minute") {
                                    time_interval = time_value * 60;
                                } else if (sub_prop->label == "second") {
                                    time_interval = time_value * 1;
                                } else if (sub_prop->label == "millisecond") {
                                    time_interval = time_value / 1000.0;
                                }
                                
                                if (!out.interval_per_count) {
                                    out.count_per_interval = true;
                                }
                            });
                        }
                    });
                });
                pr->get_only_then("RELATION", [&](auto& prep_prop) {
                    prep_prop->get_only_then("TIME", [&](auto& time_prop) {
                        if (time_prop->token->i < prop.token->i) {
                            lefthand = true;
                        }
                        if (word_is_temporal(time_prop->label)) {
                            time_prop->get_only_then("COUNT", [&](auto& interval_prop) {
                                
                                out.interval_per_count = true;
                                // in seconds
                                auto time_value = value_NUMERIC(interval_prop);
                                if (time_prop->label == "minute") {
                                    time_interval = time_value * 60;
                                } else if (time_prop->label == "second") {
                                    time_interval = time_value * 1;
                                } else if (time_prop->label == "millisecond") {
                                    time_interval = time_value / 1000.0;
                                }
                            });
                            time_prop->get_only_then("PROPERTY", [&](auto& property_prop) {
                                if (property_prop->value.kind_string == "TEXT") {
                                    if (property_prop->value.text == "few") {
                                        auto time_value = 3;
                                        if (time_prop->label == "minute") {
                                            time_interval = time_value * 60;
                                        } else if (time_prop->label == "second") {
                                            time_interval = time_value * 1;
                                        } else if (time_prop->label == "millisecond") {
                                            time_interval = time_value / 1000.0;
                                        }
                                    }
                                }
                            });

                        }
                    });
                });
            }
        }
    }

    if (times == 1) {
        if (( /*(back_found && forth_found) || */
            (forever_found) ||
            (repeatedly_found) ||
            (endlessly_found) ||
            (infinitely_found) ||
            (over_found_count > 1) ||
            (infinite_time_interval_found)
                           )) {
                               if (lefthand) {
                                   times = mtt::FOREVER;
                               } else {
                                   rtimes = mtt::FOREVER;
                               }
                           }
    }
        
//            bool ok = mtt::add_connection(args.world, selector,  std::to_string(selector_port), selector, "0");
//            ASSERT_MSG(ok, "connection failed!\n");
//
//            cmd->stop = [](Command* cmd) -> bool {
//                auto& roots = cmd->element_map["ROOT"];
//                for (auto it = roots.begin(); it != roots.end(); ++it) {
//                    mtt::Thing_destroy_self_and_connected(mtt::ctx(), itv(it));
//                }
//
//                dt::Command::destroy(cmd);
//                return false;
//            };
        
    out.count       = times;
    out.right_count = rtimes;
    out.is_lefthand = lefthand;
    out.time_interval = time_interval;
    
//    MTT_print("count: %llu, lh: %s, %f\n", out.count, bool_str(out.is_lefthand), out.time_interval);
//    MTT_BP();
    return out;
}

void handle_AND_with_THEN(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Speech_Property::Active_Prop_List seq_sim_list, Speech_Property::Active_Prop_List seq_then_list, Gen_Args gen_args);

void handle_AND_with_THEN(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Speech_Property::Active_Prop_List seq_sim_list, Speech_Property::Active_Prop_List seq_then_list, Gen_Args gen_args)
{
    
    DT_scope_open();
    //mtt::begin_sequence_group(&scb, "sequence_group : " + std::to_string(i));
    if (scb.depth == 0) {
        mtt::begin_parallel_group(&scb, "ROOT");
        DT_print("[BEGIN(ROOT)]\n");
    } else {
        mtt::begin_parallel_group(&scb);
        DT_print("[BEGIN]\n");
    }
    
    DT_scope_open();
    
    auto res = action_proc_COUNT_AND_TIME(scb, cat, prop, i, gen_args);
    auto count = res.count;
    auto rcount = res.right_count;
    auto is_lefthand = res.is_lefthand;
//    auto is_interval_per_count = res.interval_per_count;
//    auto is_count_per_interval = res.count_per_interval;
    auto time_interval = res.time_interval;
    if (is_lefthand) {
        gen_args.should_override_time = true;
        gen_args.time_override = time_interval;
    } else if (time_interval != 0.0f) {
        gen_args.should_override_time = false;
    }

    {
        cstring agent_label = prop.has_prop("AGENT") ? prop.try_get_only_prop("AGENT")->label.c_str() : "n/a";
        

        if (gen_args.should_override_time) {
            if (is_lefthand) {
                DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, gen_args.time_override);
                DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                    
                
                auto ARGS = gen_args;
                ARGS.count = rcount;
                ARGS.has_time_interval = (time_interval != 0.0);
                ARGS.time_interval = gen_args.time_override;
                handle_call_generation(scb, cat, prop, i, ARGS);
            } else {
                DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, gen_args.time_override);
                DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                    
                
                auto ARGS = gen_args;
                ARGS.count = rcount;
                ARGS.has_time_interval = (time_interval != 0.0);
                ARGS.time_interval = gen_args.time_override;
                handle_call_generation(scb, cat, prop, i, ARGS);
            }
        } else if (time_interval == 0.0) {
            if (is_lefthand) {
                DT_print("[SCHED %s self=%s count=%llu]\n", prop.label.c_str(), agent_label, rcount);
                DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
            
                auto ARGS = gen_args;
            
                ARGS.count = rcount;
                ARGS.has_time_interval = false;
                handle_call_generation(scb, cat, prop, i, ARGS);
            } else {
                DT_print("[SCHED %s self=%s count=%llu]\n", prop.label.c_str(), agent_label, rcount);
                DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
            
                auto ARGS = gen_args;
            
                ARGS.count = rcount;
                ARGS.has_time_interval = false;
                handle_call_generation(scb, cat, prop, i, ARGS);
            }
        } else {
            if (is_lefthand) {
                DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, time_interval);
                DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                
                auto ARGS = gen_args;
                
                ARGS.count = rcount;
                ARGS.time_interval = time_interval;
                ARGS.has_time_interval = true;
                
                handle_call_generation(scb, cat, prop, i, ARGS);
            } else {
                DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, time_interval);
                DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                
                auto ARGS = gen_args;
                
                ARGS.count = rcount;
                ARGS.time_interval = time_interval;
                ARGS.has_time_interval = true;
                
                handle_call_generation(scb, cat, prop, i, ARGS);
            }
        }
        
    }
    
    auto& then = *seq_then_list.begin().it;
    auto& sim  = *seq_sim_list.begin().it;
    gen_args.should_override_time = false;
    // "then" before "and"
    if (then->token->i < sim->token->i) {
        for (auto then : seq_then_list) {
            mtt::wait_for_all_scheduled_ended(&scb);
            action_proc_THEN(scb, cat, *then, i, gen_args);
        }
        mtt::wait_for_all_scheduled_ended(&scb);
        DT_scope_close();
        
        mtt::End_Parallel_Group_Args end_args = {};
        if (!is_lefthand) {
            DT_print("[END]\n");
        } else {
            //if (time_interval == 0.0) {
                DT_print("[END count=%llu]\n", count);
                end_args.count = count;
                end_args.has_count = true;
//            } else {
//                DT_print("[END count=%llu, time=%f]\n", count, time_interval);
//                end_args.count = count;
//                end_args.has_count = true;
//                end_args.time_interval = time_interval;
//                end_args.has_time_interval = true;
//            }
        }
        DT_scope_close();
        
        
        mtt::end_parallel_group(&scb, end_args);
        
        DT_scope_open();
        //mtt::begin_sequence_group(&scb, "sequence_group : " + std::to_string(i));
        
        
        if (scb.depth == 0) {
            mtt::begin_parallel_group(&scb, "ROOT");
            DT_print("[BEGIN(ROOT)]\n");
        } else {
            mtt::begin_parallel_group(&scb);
            DT_print("[BEGIN]\n");
        }
        
        DT_scope_open();
        
        for (auto and_also : seq_sim_list) {
            action_proc_AND(scb, cat, *and_also, i, gen_args);
        }
    } else { // "and" before "then"
        
        // ???
        for (auto and_also : seq_sim_list) {
            action_proc_AND(scb, cat, *and_also, i, gen_args);
        }
        for (auto then : seq_then_list) {
            mtt::wait_for_all_scheduled_ended(&scb);
            gen_args.should_override_time = false;
            action_proc_THEN(scb, cat, *then, i, gen_args);
        }
    }
    
    
    mtt::wait_for_all_scheduled_ended(&scb);
    DT_scope_close();
    
    mtt::End_Parallel_Group_Args end_args = {};
    
    if (!is_lefthand) {
        DT_print("[END]\n");
    } else {
  //      if (time_interval == 0.0) {
            DT_print("[END count=%llu]\n", count);
            end_args.count = count;
            end_args.has_count = true;
//        } else {
//            DT_print("[END count=%llu, time=%f]\n", count, time_interval);
//            end_args.count = count;
//            end_args.has_count = true;
//            end_args.time_interval = time_interval;
//            end_args.has_time_interval = true;
//        }
    }
    DT_scope_close();
    
    mtt::end_parallel_group(&scb, end_args);
}

void action_proc_AND(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args)
{
    //ASSERT_MSG(scb.state == mtt::SCRIPT_BUILDER_STATE::NONE, "STATE should be none");
    
    bool has_sim  = false;
    bool has_then = false;
    auto seq_sim_list = prop.get_active("SEQUENCE_SIMULTANEOUS");
    auto seq_then_list = prop.get_active("SEQUENCE_THEN");
    
    if (is_valid(seq_sim_list)) {
        has_sim = true;
    }
    
    if (is_valid(seq_then_list)) {
        has_then = true;
    }
    
    if (has_sim && has_then) {
        handle_AND_with_THEN(scb, cat, prop, i, seq_sim_list, seq_then_list, gen_args);
        return;
    }
    
    {
//                                std::vector<Speech_Property*> then_stack = {};
//                                std::vector<
        auto res = action_proc_COUNT_AND_TIME(scb, cat, prop, i, gen_args);
        auto count = res.count;
        auto rcount = res.right_count;
        auto is_lefthand = res.is_lefthand;
//        auto is_interval_per_count = res.interval_per_count;
//        auto is_count_per_interval = res.count_per_interval;
        auto time_interval = res.time_interval;
        if (is_lefthand) {
            gen_args.should_override_time = true;
            gen_args.time_override = time_interval;
        } else if (time_interval != 0.0f) {
            gen_args.should_override_time = false;
        }
        
        {
            
            {
                cstring agent_label = prop.has_prop("AGENT") ? prop.try_get_only_prop("AGENT")->label.c_str() : "n/a";
                

                if (gen_args.should_override_time) {
                    if (is_lefthand) {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, gen_args.time_override);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                            
                        
                        auto ARGS = gen_args;
                        ARGS.count = rcount;
                        ARGS.has_time_interval = true;
                        ARGS.has_time_interval = (time_interval != 0.0);
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    } else {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, gen_args.time_override);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                            
                        
                        auto ARGS = gen_args;
                        ARGS.count = rcount;
                        ARGS.has_time_interval = true;
                        ARGS.has_time_interval = (time_interval != 0.0);
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    }
                } else if (time_interval == 0.0) {
                    if (is_lefthand) {
                        DT_print("[SCHED %s self=%s count=%llu]\n", prop.label.c_str(), agent_label, rcount);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                    
                        auto ARGS = gen_args;
                    
                        ARGS.count = rcount;
                        ARGS.has_time_interval = false;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    } else {
                        DT_print("[SCHED %s self=%s count=%llu]\n", prop.label.c_str(), agent_label, rcount);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                    
                        auto ARGS = gen_args;
                    
                        ARGS.count = rcount;
                        ARGS.has_time_interval = false;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    }
                } else {
                    if (is_lefthand) {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, time_interval);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                        
                        auto ARGS = gen_args;
                        
                        ARGS.count = rcount;
                        ARGS.time_interval = time_interval;
                        ARGS.has_time_interval = true;
                        
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    } else {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, time_interval);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                        
                        auto ARGS = gen_args;
                        
                        ARGS.count = rcount;
                        ARGS.time_interval = time_interval;
                        ARGS.has_time_interval = true;
                        
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    }
                }
                
            }
            
            if (has_then) {
                gen_args.should_override_time = false;
                for (auto then : seq_then_list) {
                    mtt::wait_for_all_scheduled_ended(&scb);
                    action_proc_THEN(scb, cat, *then, i, gen_args);
                }
            } else if (has_sim) {
                for (auto and_also : seq_sim_list) {
                    action_proc_AND(scb, cat, *and_also, i, gen_args);
                }
            } else {

            }
            
//            DT_Behavior* b = nullptr;
//            if (cat->lookup(prop.label, prop.sub_label, &b)) {
//                DT_print("FOUND ACTION: %s\n", (prop.label + ":" + prop.sub_label).c_str());
//
//                //call_action(&scb, ...)
//
//            } else {
//                DT_print("NOT FOUND!\n");
//            }
            
            
        }

        
        
    }
}

void action_proc_THEN(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args)
{
    //ASSERT_MSG(scb.state == mtt::SCRIPT_BUILDER_STATE::NONE, "STATE should be none");
    
    bool has_sim  = false;
    bool has_then = false;
    auto seq_sim_list = prop.get_active("SEQUENCE_SIMULTANEOUS");
    auto seq_then_list = prop.get_active("SEQUENCE_THEN");
    
    if (is_valid(seq_sim_list)) {
        has_sim = true;
    }
    
    if (is_valid(seq_then_list)) {
        has_then = true;
    }
    
    if (has_sim && has_then) {
        handle_AND_with_THEN(scb, cat, prop, i, seq_sim_list, seq_then_list, gen_args);
        return;
    }
    
    {
        DT_scope_open();
        //mtt::begin_sequence_group(&scb, "sequence_group : " + std::to_string(i));
        
        
        if (scb.depth == 0) {
            mtt::begin_parallel_group(&scb, "ROOT");
            DT_print("[BEGIN(ROOT)]\n");
        } else {
            mtt::begin_parallel_group(&scb);
            DT_print("[BEGIN]\n");
        }
        
        DT_scope_open();
        
//                                std::vector<Speech_Property*> then_stack = {};
//                                std::vector<
        auto res = action_proc_COUNT_AND_TIME(scb, cat, prop, i, gen_args);
        auto count = res.count;
        auto rcount = res.right_count;
        auto is_lefthand = res.is_lefthand;
        //auto is_interval_per_count = res.interval_per_count;
        //auto is_count_per_interval = res.count_per_interval;
        auto time_interval = res.time_interval;
        {
            if (is_lefthand) {
                gen_args.should_override_time = true;
                gen_args.time_override = time_interval;
            } else if (time_interval != 0.0f) {
                gen_args.should_override_time = false;
            }
                
            {
                cstring agent_label = prop.has_prop("AGENT") ? prop.try_get_only_prop("AGENT")->label.c_str() : "n/a";
                

                if (gen_args.should_override_time) {
                    if (is_lefthand) {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, gen_args.time_override);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                            
                        
                        auto ARGS = gen_args;
                        ARGS.count = rcount;
                        ARGS.has_time_interval = (time_interval != 0.0);
                        ARGS.time_interval = gen_args.time_override;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    } else {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, gen_args.time_override);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                            
                        
                        auto ARGS = gen_args;
                        ARGS.count = rcount;
                        ARGS.has_time_interval = (time_interval != 0.0);
                        ARGS.time_interval = gen_args.time_override;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    }
                } else if (time_interval == 0.0) {
                    if (is_lefthand) {
                        DT_print("[SCHED %s self=%s count=%llu]\n", prop.label.c_str(), agent_label, rcount);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                    
                        auto ARGS = gen_args;
                    
                        ARGS.count = rcount;
                        ARGS.has_time_interval = false;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    } else {
                        DT_print("[SCHED %s self=%s count=%llu]\n", prop.label.c_str(), agent_label, rcount);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                    
                        auto ARGS = gen_args;
                    
                        ARGS.count = rcount;
                        ARGS.has_time_interval = false;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    }
                } else {
                    if (is_lefthand) {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, time_interval);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                        
                        auto ARGS = gen_args;
                        
                        ARGS.count = rcount;
                        ARGS.time_interval = time_interval;
                        ARGS.has_time_interval = true;
                        
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    } else {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, time_interval);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                        
                        auto ARGS = gen_args;
                        
                        ARGS.count = rcount;
                        ARGS.time_interval = time_interval;
                        ARGS.has_time_interval = true;
                        
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    }
                }
                
            }
            
            if (has_then) {
                gen_args.should_override_time = false;
                for (auto then : seq_then_list) {
                    mtt::wait_for_all_scheduled_ended(&scb);
                    action_proc_THEN(scb, cat, *then, i, gen_args);
                }
            } else if (has_sim) {
                for (auto and_also : seq_sim_list) {
                    action_proc_AND(scb, cat, *and_also, i, gen_args);
                }
            } else {

            }
            
//            DT_Behavior* b = nullptr;
//            if (cat->lookup(prop.label, prop.sub_label, &b)) {
//                DT_print("FOUND ACTION: %s\n", (prop.label + ":" + prop.sub_label).c_str());
//
//                //call_action(&scb, ...)
//
//            } else {
//                DT_print("NOT FOUND!\n");
//            }
            
            
        }
        
        
        mtt::wait_for_all_scheduled_ended(&scb);
        DT_scope_close();
        
        mtt::End_Parallel_Group_Args args = {};
        
        if (!is_lefthand) {
            DT_print("[END]\n");
        } else {
           // if (time_interval == 0.0) {
                DT_print("[END count=%llu]\n", count);
                args.count = count;
                args.has_count = true;
           // } else {
//                DT_print("[END count=%llu, time=%f]\n", count, time_interval);
//                args.count = count;
//                args.has_count = true;
//                args.time_interval = time_interval;
//                args.has_time_interval = true;
//            }
        }
        DT_scope_close();
        
        mtt::end_parallel_group(&scb, args);
        
        //mtt::begin_sequence_group(&scb, "sequence_group : " + std::to_string(i));
        
    }
}



void action_proc(mtt::Script_Builder& scb, DT_Behavior_Catalogue* cat, Speech_Property& prop, usize i, Gen_Args gen_args)
{
    //ASSERT_MSG(scb.state == mtt::SCRIPT_BUILDER_STATE::NONE, "STATE should be none");
    bool has_sim  = false;
    bool has_then = false;
    auto seq_sim_list = prop.get_active("SEQUENCE_SIMULTANEOUS");
    auto seq_then_list = prop.get_active("SEQUENCE_THEN");
    
    if (is_valid(seq_sim_list)) {
        has_sim = true;
    }
    
    if (is_valid(seq_then_list)) {
        has_then = true;
    }
    
    if (has_sim && has_then) {
        handle_AND_with_THEN(scb, cat, prop, i, seq_sim_list, seq_then_list, gen_args);
        return;
    }
    
    
    {
        
        //mtt::begin_sequence_group(&scb, "sequence_group : " + std::to_string(i));
        
        
        if (scb.depth == 0) {
            mtt::begin_parallel_group(&scb, "ROOT");
            DT_print("[BEGIN(ROOT)]\n");
        } else {
            mtt::begin_parallel_group(&scb);
            DT_print("[BEGIN]\n");
        }
        
        DT_scope_open();
        
//                                std::vector<Speech_Property*> then_stack = {};
//                                std::vector<
        auto res = action_proc_COUNT_AND_TIME(scb, cat, prop, i, gen_args);
        auto count = res.count;
        auto rcount = res.right_count;
        auto is_lefthand = res.is_lefthand;
        //auto is_interval_per_count = res.interval_per_count;
        //auto is_count_per_interval = res.count_per_interval;
        auto time_interval = res.time_interval;
        
        if (is_lefthand) {
            gen_args.should_override_time = true;
            gen_args.time_override = time_interval;
        } else if (time_interval != 0.0f) {
            gen_args.should_override_time = false;
        }
        
        {
            
            {
                cstring agent_label = prop.has_prop("AGENT") ? prop.try_get_only_prop("AGENT")->label.c_str() : "n/a";
                

                if (gen_args.should_override_time) {
                    if (is_lefthand) {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, gen_args.time_override);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                            
                        
                        auto ARGS = gen_args;
                        ARGS.count = rcount;
                        ARGS.has_time_interval = (time_interval != 0.0);
                        ARGS.time_interval = gen_args.time_override;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    } else {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, gen_args.time_override);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                            
                        
                        auto ARGS = gen_args;
                        ARGS.count = rcount;
                        ARGS.has_time_interval = (time_interval != 0.0);
                        ARGS.time_interval = gen_args.time_override;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    }
                } else if (time_interval == 0.0) {
                    if (is_lefthand) {
                        DT_print("[SCHED %s self=%s count=%llu]\n", prop.label.c_str(), agent_label, rcount);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                    
                        auto ARGS = gen_args;
                    
                        ARGS.count = rcount;
                        ARGS.has_time_interval = false;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    } else {
                        DT_print("[SCHED %s self=%s count=%llu]\n", prop.label.c_str(), agent_label, rcount);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                    
                        auto ARGS = gen_args;
                    
                        ARGS.count = rcount;
                        ARGS.has_time_interval = false;
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    }
                } else {
                    if (is_lefthand) {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, time_interval);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                        
                        auto ARGS = gen_args;
                        
                        ARGS.count = rcount;
                        ARGS.time_interval = time_interval;
                        ARGS.has_time_interval = true;
                        
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    } else {
                        DT_print("[SCHED %s self=%s count=%llu time=%f]\n", prop.label.c_str(), agent_label, rcount, time_interval);
                        DT_INSERT_PRINT_FOR_ACTION(scb, prop.label);
                        
                        auto ARGS = gen_args;
                        
                        ARGS.count = rcount;
                        ARGS.time_interval = time_interval;
                        ARGS.has_time_interval = true;
                        
                        handle_call_generation(scb, cat, prop, i, ARGS);
                    }
                }
                
            }
            
            //            DT_Behavior* b = nullptr;
            //            if (cat->lookup(prop.label, prop.sub_label, &b)) {
            //                DT_print("FOUND ACTION: %s\n", (prop.label + ":" + prop.sub_label).c_str());
            //
            //                //call_action(&scb, ...)
            //
            //            } else {
            //                DT_print("NOT FOUND!\n");
            //            }

            
            
            if (has_then) {
                gen_args.should_override_time = false;
                for (auto then : seq_then_list) {
                    mtt::wait_for_all_scheduled_ended(&scb);
                    action_proc_THEN(scb, cat, *then, i, gen_args);
                }
            } else if (has_sim) {
                for (auto and_also : seq_sim_list) {
                    action_proc_AND(scb, cat, *and_also, i, gen_args);
                }
            } else {

            }
            
        }
        
        mtt::wait_for_all_scheduled_ended(&scb);
        DT_scope_close();
        
        mtt::End_Parallel_Group_Args args = {};
        
        if (!is_lefthand) {
            DT_print("[END]\n");
        } else {
//            if (time_interval == 0.0) {
                DT_print("[END count=%llu]\n", count);
                args.count = count;
                args.has_count = true;
//            } else {
//                DT_print("[END count=%llu, time=%f]\n", count, time_interval);
//                args.count = count;
//                args.has_count = true;
//                args.time_interval = time_interval;
//                args.has_time_interval = true;
//            }
        }
        DT_scope_close();
        
        mtt::end_parallel_group(&scb, args);
        
        //mtt::begin_sequence_group(&scb, "sequence_group : " + std::to_string(i));
        
    }
}

Entries entries = {};

mtt::Map<mtt::String, uint64> first_cases = {
    {"RELATION", 1},
    {"CONTINUOUS", 2},
};

mtt::Map<mtt::String, uint64> second_cases = {
    {"RELATION", 1},
    {"CONTINUOUS", 2},
};

inline bool is_negated(Speech_Property* p)
{
    if (auto* p_negated = p->try_get_only_prop("NEGATED"); p_negated != nullptr) {
        return p_negated->value.flag;
    }
    
    return false;
}

struct Rule_Create_Args {
    bool is_response;
    Speech_Property::Ref_Map& refs;
    DT_Behavior_Catalogue* cat;
};

struct Attrib {
    mtt::String label;
    bool is_negated;
};
struct Attrib_Info {
    std::vector<Attrib> list;
    std::vector<mtt::String> as_str;
    mtt::String key = "";
};

void rule_create_handle_attrib_info(Speech_Property* _, Attrib_Info& list)
{
    {
        // handle attributes
        Prop_List*  property_prop = nullptr;
        auto* dict = dt::word_dict();
        if (_->try_get_prop("PROPERTY", &property_prop)) {
            for (auto it_prop = property_prop->begin(); it_prop != property_prop->end(); ++it_prop) {
                auto* property = *it_prop;
                if (property->label != "trait" ||  dict->is_ignored_attribute(property->value.text)) {
                    continue;
                }
                Speech_Property* n_prop = nullptr;
                property->try_get_only_prop("NEGATED", &n_prop);
                bool is_negated = (n_prop != nullptr && n_prop->value.flag == true);
                
                attribute_add(property->value.text);
                
                list.list.push_back({property->value.text, is_negated});
            }
        }
        for (usize i = 0; i < list.list.size(); i += 1) {
            auto& el = list.list[i];
            list.as_str.push_back(el.label + "_" + ((el.is_negated) ? "N":"Y"));
        }
        std::sort(list.as_str.begin(), list.as_str.end());
        
        for (usize i = 0; i < list.as_str.size(); i += 1) {
            list.key += list.as_str[i] + "_";
        }
    }
}



bool rule_create(Active_Prop_List& prop_list_, rules::Condition_Builder& cb, const Rule_Create_Args& args)
{
    bool is_response = args.is_response;
    Speech_Property::Ref_Map& refs = args.refs;
    
    bool success = true;
    for (auto l_prop_ptr : prop_list_) {
        auto& l_prop = *l_prop_ptr;
        
        const VERB_EVENT& event = l_prop.action_event;
        cb.event = event;
        
        switch (first_cases[l_prop.annotation]) {
        default: {
            {
                struct State {
                    bool has_agent           = false;
                    bool has_direct_object   = false;
                    bool has_preposition     = false;
                    bool has_indirect_object = false;
                    
                    struct Relational {
                        bool pobj_has_rel = false;
                    } rst = {};
                    
                    bool has_properties = false;
                    
                } st = {};
                
                auto agent_list = l_prop.get_active("AGENT");
                st.has_agent = is_valid(agent_list);
                auto direct_object_list = l_prop.get_active("DIRECT_OBJECT");
                st.has_direct_object = is_valid(direct_object_list);
                auto preposition_list = l_prop.get_active("PREPOSITION");
                st.has_preposition = is_valid(preposition_list);
                auto indirect_object_list = l_prop.get_active("INDIRECT_OBJECT");
                st.has_indirect_object = is_valid(indirect_object_list);
                auto properties_list = l_prop.get_active("PROPERTY");
                st.has_properties = is_valid(properties_list);
                if (st.has_properties) {
                    for (const auto el : properties_list) {
                        attribute_add(el->value.text);
                    }
                }
                
                // MARK: simplified for now...
                
                bool create_action = false;
                if (st.has_agent) {
                    MTT_BP();
                    
                    
                    Word_Dictionary_Entry* action = nullptr;
                    mtt::String verb;
                    
                    if (l_prop.label != "stop") {
                        action = verb_lookup(l_prop.label);
                        if (action == nullptr) {
                            // for now, treat as creating a new verb
                            create_action = true;
                            cb.result.is_new_action = true;
                            cb.result.label = l_prop.label;
                            action = verb_add(l_prop.label);
                            
                            return true;
                        } else {
                            action = verb_root_equivalence(verb_add(l_prop.label));
                        }
                        verb = Word_Dictionary_Entry_name_for_verb_with_event(action, event);
                    } else {
                        auto action_list = l_prop.get_active("ACTION");
                        if (!is_valid(action_list)) {
                            success = false;
                            break;
                        }
                        
                        
                        Speech_Property* action_prop = action_list.first();
                        mtt::String label = action_prop->label;
                        action = verb_lookup(label);
                        
                        if (action == nullptr) {
                            // for now, treat as creating a new verb
                            create_action = true;
                            cb.result.is_new_action = true;
                            cb.result.label = label;
                            action = verb_add(label);
                            return true;
                        } else {
                            action = verb_root_equivalence(verb_add(label));
                        }
                        verb = Word_Dictionary_Entry_name_for_verb_with_event(action, VERB_EVENT_END);
                    }
                    
                    
                    
                    
                    if (st.has_direct_object && st.has_preposition && st.has_indirect_object) {
                        MTT_BP();
                        //mtt::String p_action = l_prop.label
                    } else if (st.has_direct_object && st.has_preposition) {
                        MTT_BP();
                    } else if (st.has_direct_object && st.has_indirect_object) {
                        MTT_BP();
                    } else if (st.has_direct_object) {
                        MTT_BP();
                        
                        auto& valid_agent_list = agent_list;
                        //dt::get_prop_or_coref_list(agent_list, &valid_agent_list);
                                                            
                        for (auto _ : valid_agent_list) {
                            auto* noun_type = noun_add(_->label);
                            (void)noun_type;
                            if (_->kind_str == "THING_TYPE") {
                                

                                Attrib_Info info = {};
                                rule_create_handle_attrib_info(_, info);
                                auto final_key = _->label + info.key;
                                
                                
                                mtt::String agent_var_name_result = "";
                                if (is_referenced_in_response(is_response, refs, _)) {
                                    agent_var_name_result = cb.set_var(final_key, refs[_], _);
                                } else {
                                    agent_var_name_result = cb.set_var(final_key, "", _);
                                }
                                
                                auto agent_var_name = _->label;
                                auto agent_var_name_sub = cb.set_var(final_key + "SUB", "", _);
                                
                                
//                                                cb.add_clause({
//                                                    {.name = _->label, .is_variable = false, .is_negated = false},
//                                                    {.name = agent_var_name, .is_variable = true},
//                                                });
                                
                                cb.add_clause({
                                    {.name = "IsA", .is_variable = false, .is_negated = false},
                                    {.name = agent_var_name_sub, .is_variable = true},
                                    {.name = agent_var_name, .is_variable = false},
                                });
                                
                                cb.add_clause({
                                    {.name = agent_var_name_sub, .is_variable = true, .is_negated = false},
                                    {.name = agent_var_name_result, .is_variable = true},
                                });
                                
                                cb.add_clause({
                                    {.name = "Prefab", .is_variable = false, .is_negated = true},
                                    {.name = agent_var_name_result, .is_variable = true},
                                });
                                
                                
                                for (usize i = 0; i < info.list.size(); i += 1) {
                                    cb.add_clause({
                                        {.name = "IsA", .is_variable = false, .is_negated = info.list[i].is_negated},
                                        {.name = agent_var_name_result, .is_variable = true},
                                        {.name = "attribute." + info.list[i].label, .is_variable = false}
                                    });
                                }
                                
                                //mtt::String query = "IsA(_SUBTYPE, " + label + "), _SUBTYPE(" + "_" + searched_var + ")";
                                auto& valid_direct_object_list = direct_object_list;
                                //dt::get_prop_or_coref_list(direct_object_list, &valid_direct_object_list);
                                
                                std::vector<mtt::Thing*> all2 = {};
                                for (auto _2 : valid_direct_object_list) {
                                    auto* noun_type = noun_add(_2->label);
                                    (void)noun_type;
                                    if (_2->kind_str == "THING_TYPE") {
                                        
                                        
                                        // TODO: fix negation generation
                                        Attrib_Info info = {};
                                        rule_create_handle_attrib_info(_2, info);
                                        auto final_key = _2->label + info.key;
                                        
                                        if (verb == "formbegin") {
                                            Speech_Property* count_prop = nullptr;
                                            float64 count_to_exist = 1;
                                            if (_2->try_get_only_prop("COUNT", &count_prop)) {
                                                count_to_exist = count_prop->value.numeric;
                                            }
                                            cb.add_value_clause({
                                                {.name = verb, .is_variable = false, .is_negated = is_negated((&l_prop))},
                                                {.name = agent_var_name_result, .is_variable = true},
                                                {.value = mtt::Any::from_Float64(count_to_exist)},
                                                {.value = mtt::Any::from_String(_2->label.c_str())},
                                            });
                                            bool do_ignore = false;
                                            for (usize v_f_i = 0; v_f_i < cb.vars_to_find.size(); v_f_i += 1) {
                                                if (cb.vars_to_find[v_f_i] == agent_var_name_result) {
                                                    do_ignore = true;
                                                }
                                            }
                                            if (!do_ignore) {
                                                cb.vars_to_find.push_back(agent_var_name_result);
                                            }
                                        } else {
                                            
                                            mtt::String direct_object_var_name_result = "";
                                            if (is_referenced_in_response(is_response, refs, _2)) {
                                                direct_object_var_name_result = cb.set_var_direct_name(final_key, refs[_2], _2);
                                            } else {
                                                direct_object_var_name_result = cb.set_var_direct_name(final_key, "", _2);
                                            }
                                            
                                            auto direct_object_var_name = _2->label;
                                            auto direct_object_var_name_sub = cb.set_var_direct_name(final_key + "SUB", "", _2);
                                            
                                            
                                            
                                            //                                                        [NEGATED] = [
                                            //                                                        {
                                            //                                                            label=[], tag=[], type=[], kind=[], key=[NEGATED], idx=[0]
                                            //                                                            value={
                                            //                                                                FLAG=[true]
                                            //                                                            }
                                            //                                                        }
                                            
                                            cb.add_clause({
                                                {.name = "IsA", .is_variable = false, .is_negated = false},
                                                {.name = direct_object_var_name_sub, .is_variable = true},
                                                {.name = direct_object_var_name, .is_variable = false},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = direct_object_var_name_sub, .is_variable = true, .is_negated = false},
                                                {.name = direct_object_var_name_result, .is_variable = true},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = "Prefab", .is_variable = false, .is_negated = true},
                                                {.name = direct_object_var_name_result, .is_variable = true},
                                            });
                                            
                                            
                                            
                                            
                                            cb.add_clause({
                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = agent_var_name_result, .is_variable = true},
                                                {.name = direct_object_var_name_result, .is_variable = true},
                                            });
                                            
                                            for (usize i = 0; i < info.list.size(); i += 1) {
                                                cb.add_clause({
                                                    {.name = "IsA", .is_variable = false, .is_negated = info.list[i].is_negated},
                                                    {.name = direct_object_var_name_result, .is_variable = true},
                                                    {.name = info.list[i].label, .is_variable = false}
                                                });
                                            }
                                        }
                                        
                                    } else if (_2->kind_str == "THING_INSTANCE") {
                                        bool something_found = for_each_thing_instance_value(*_2, [&](mtt::Thing* thing) {
                                            all2.push_back(thing);
                                        });
                                        
                                        // treat as type for now
                                        
                                        if (all2.size() > 1 || all2.size() == 0) {
                                            _2->kind_str = "THING_TYPE";
                                            mtt::String direct_object_var_name_result = "";
                                            if (is_referenced_in_response(is_response, refs, _2)) {
                                                direct_object_var_name_result = cb.set_var_direct_name(_2->label, refs[_2], _2);
                                            } else {
                                                direct_object_var_name_result = cb.set_var_direct_name(_2->label, "", _2);
                                            }
                                            
                                            auto direct_object_var_name = _2->label;
                                            auto direct_object_var_name_sub = cb.set_var_direct_name(_2->label + "SUB", "", _2);
                                            
                                            cb.add_clause({
                                                {.name = "IsA", .is_variable = false, .is_negated = false},
                                                {.name = direct_object_var_name_sub, .is_variable = true},
                                                {.name = direct_object_var_name, .is_variable = false},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = direct_object_var_name_sub, .is_variable = true, .is_negated = false},
                                                {.name = direct_object_var_name_result, .is_variable = true},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = "Prefab", .is_variable = false, .is_negated = true},
                                                {.name = direct_object_var_name_result, .is_variable = true},
                                            });

                                            cb.add_clause({
                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = agent_var_name_result, .is_variable = true},
                                                {.name = direct_object_var_name_result, .is_variable = true},
                                            });

                                        } else { // single value is okay for now
                                            mtt::String direct_object_var_name = search_id(all2[0]);
                                            
                                            if (is_referenced_in_response(is_response, refs, _2)) {
                                                direct_object_var_name = cb.set_const(_2->label, refs[_2], _2);
                                            } else {
                                                direct_object_var_name = cb.set_const(_2->label, "DIRECT_OBJECT", _2);
                                            }
                                            cb.set_thing_dependency(mtt::thing_id(all2[0]));
                                            
                                            cb.add_clause({
                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = agent_var_name_result, .is_variable = true},
                                                {.name = direct_object_var_name, .is_variable = true},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = "attribute.id_is", .is_variable = false, .is_negated = false},
                                                {.name = direct_object_var_name, .is_variable = true},
                                                {.name = direct_object_var_name, .is_variable = false},
                                            });
                                            
                                        }
                                    }
                                }
                                
                            } else if (_->kind_str == "THING_INSTANCE") {
                                std::vector<mtt::Thing*> all = {};
                                bool something_found = for_each_thing_instance_value(*_, [&](mtt::Thing* thing) {
                                    all.push_back(thing);
                                });

                                
                                mtt::String agent_var_name;
                                // treat as type for now
                                if (all.size() > 1 || all.size() == 0) {
                                    _->kind_str = "THING_TYPE";
                                    mtt::String agent_var_name_result = "";
                                    if (is_referenced_in_response(is_response, refs, _)) {
                                        agent_var_name_result = cb.set_var(_->label, refs[_], _);
                                    } else {
                                        agent_var_name_result = cb.set_var(_->label, "", _);
                                    }
                                    
                                    auto agent_var_name = _->label;
                                    auto agent_var_name_sub = cb.set_var(_->label + "SUB", "", _);
                                    
//                                                    cb.add_clause({
//                                                        {.name = _->label, .is_variable = false, .is_negated = false},
//                                                        {.name = agent_var_name, .is_variable = true},
//                                                    });
                                    
                                    cb.add_clause({
                                        {.name = "IsA", .is_variable = false, .is_negated = false},
                                        {.name = agent_var_name_sub, .is_variable = true},
                                        {.name = agent_var_name, .is_variable = false},
                                    });
                                    
                                    cb.add_clause({
                                        {.name = agent_var_name_sub, .is_variable = true, .is_negated = false},
                                        {.name = agent_var_name_result, .is_variable = true},
                                    });
                                    agent_var_name = agent_var_name_result;
                                    
                                    cb.add_clause({
                                        {.name = "Prefab", .is_variable = false, .is_negated = true},
                                        {.name = agent_var_name_result, .is_variable = true},
                                    });
                                    
                                    
                                } else { // single value is okay for now
                                    
                                    agent_var_name = search_id(all[0]);
                                    if (is_referenced_in_response(is_response, refs, _)) {
                                        cb.set_const(agent_var_name, refs[_], _);
                                    } else {
                                        cb.set_const(agent_var_name, "AGENT", _);
                                    }
                                    cb.set_thing_dependency(mtt::thing_id(all[0]));
                                    
                                    cb.add_clause({
                                        {.name = "attribute.id_is", .is_variable = false, .is_negated = false},
                                        {.name = agent_var_name, .is_variable = true},
                                        {.name = agent_var_name, .is_variable = false},
                                    });
//                                    cb.add_clause({
//                                        {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
//                                        {.name = agent_var_name, .is_variable = true, .is_const = false},
//                                    });
                                }
                                
                                auto& valid_direct_object_list = direct_object_list;
                                //dt::get_prop_or_coref_list(direct_object_list, &valid_direct_object_list);
                                
                                std::vector<mtt::Thing*> all2 = {};
                                for (auto _2 : valid_direct_object_list) {
                                    auto* noun_type = noun_add(_2->label);
                                    (void)noun_type;
                                    if (_2->kind_str == "THING_TYPE") {
                                        
                                        
                                        // TODO: fix negation generation
                                        
                                        
                                        
//                                                        [NEGATED] = [
//                                                        {
//                                                            label=[], tag=[], type=[], kind=[], key=[NEGATED], idx=[0]
//                                                            value={
//                                                                FLAG=[true]
//                                                            }
//                                                        }
                                    
                                        auto direct_object_var_name = _2->label;
                                        auto direct_object_var_name_sub = cb.set_var_direct_name(_2->label + "SUB", "", _2);
                                        mtt::String direct_object_var_name_result = "";
                                        if (is_referenced_in_response(is_response, refs, _2)) {
                                            direct_object_var_name_result = cb.set_var_direct_name(_2->label, refs[_2], _2);
                                        } else {
                                            direct_object_var_name_result = cb.set_var_direct_name(_2->label, "", _2);
                                        }
                                        
                                        cb.add_clause({
                                            {.name = "IsA", .is_variable = false, .is_negated = false},
                                            {.name = direct_object_var_name_sub, .is_variable = true},
                                            {.name = direct_object_var_name, .is_variable = false},
                                        });
                                        
                                        cb.add_clause({
                                            {.name = direct_object_var_name_sub, .is_variable = true, .is_negated = false},
                                            {.name = direct_object_var_name_result, .is_variable = true},
                                        });
                                        
                                        cb.add_clause({
                                            {.name = "Prefab", .is_variable = false, .is_negated = true},
                                            {.name = direct_object_var_name_result, .is_variable = true},
                                        });

                                        cb.add_clause({
                                            {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                            {.name = agent_var_name, .is_variable = true},
                                            {.name = direct_object_var_name_result, .is_variable = true},
                                        });
                                        
                                    } else if (_2->kind_str == "THING_INSTANCE") {
                                        bool something_found = for_each_thing_instance_value(*_2, [&](mtt::Thing* thing) {
                                            all2.push_back(thing);
                                        });
                                        
                                        
                                        // treat as type for now
                                        if (all2.size() > 1 || all2.size() == 0) {
                                            _2->kind_str = "THING_TYPE";
                                            auto direct_object_var_name = cb.set_var_direct_name(_2->label, "DIRECT_OBJECT", _2);
                                            
                                            cb.add_clause({
                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = agent_var_name, .is_variable = true},
                                                {.name = direct_object_var_name, .is_variable = true},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = "Prefab", .is_variable = false, .is_negated = true},
                                                {.name = direct_object_var_name, .is_variable = true},
                                            });
                                            
                                        } else { // single value is okay for now
                                            mtt::String direct_object_var_name = search_id(all2[0]);
                                            
                                            if (is_referenced_in_response(is_response, refs, _2)) {
                                                cb.set_const(direct_object_var_name, refs[_2], _2);
                                            } else {
                                                cb.set_const(direct_object_var_name, "DIRECT_OBJECT", _2);
                                            }
                                            cb.set_thing_dependency(mtt::thing_id(all2[0]));
                                            
//                                            cb.add_clause({
//                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
//                                                {.name = agent_var_name, .is_variable = true, .is_id = true},
//                                                {.name = direct_object_var_name, .is_variable = true, .is_id = true},
//                                            });
                                            
                                            cb.add_clause({
                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = agent_var_name, .is_variable = true},
                                                {.name = direct_object_var_name, .is_variable = true},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = "attribute.id_is", .is_variable = false, .is_negated = false},
                                                {.name = direct_object_var_name, .is_variable = true, .is_const = true},
                                                {.name = direct_object_var_name, .is_variable = false},
                                            });
                                            
                                        }
                                    }
                                }
                                
                                
                            } else {
                                success = false;
                                return false;
                            }
                        }
                        
                        auto property_list = l_prop.get_active("PROPERTY");
                        
                        
                    } else if (st.has_preposition) {
                        MTT_BP();
                        
                        auto& valid_agent_list = agent_list;
                        //dt::get_prop_or_coref_list(agent_list, &valid_agent_list);

                        for (auto _ : valid_agent_list) {
                            auto* noun_type = noun_add(_->label);
                            (void)noun_type;
                            if (_->kind_str == "THING_TYPE") {
                                
                                Attrib_Info info = {};
                                rule_create_handle_attrib_info(_, info);
                                auto final_key = _->label + info.key;
                                
                                
                                mtt::String agent_var_name_result = "";
                                if (is_referenced_in_response(is_response, refs, _)) {
                                    agent_var_name_result = cb.set_var(final_key, refs[_], _);
                                } else {
                                    agent_var_name_result = cb.set_var(final_key, "", _);
                                }
                                
                                auto agent_var_name = _->label;
                                auto agent_var_name_sub = cb.set_var(final_key + "SUB", "", _);
                                
                                
//                                                cb.add_clause({
//                                                    {.name = _->label, .is_variable = false, .is_negated = false},
//                                                    {.name = agent_var_name, .is_variable = true},
//                                                });
                                
                                cb.add_clause({
                                    {.name = "IsA", .is_variable = false, .is_negated = false},
                                    {.name = agent_var_name_sub, .is_variable = true},
                                    {.name = agent_var_name, .is_variable = false},
                                });
                                
                                cb.add_clause({
                                    {.name = agent_var_name_sub, .is_variable = true, .is_negated = false},
                                    {.name = agent_var_name_result, .is_variable = true},
                                });
                                
                                cb.add_clause({
                                    {.name = "Prefab", .is_variable = false, .is_negated = true},
                                    {.name = agent_var_name_result, .is_variable = true},
                                });
                                

                                for (usize i = 0; i < info.list.size(); i += 1) {
                                    cb.add_clause({
                                        {.name = "IsA", .is_variable = false, .is_negated = info.list[i].is_negated},
                                        {.name = agent_var_name_result, .is_variable = true},
                                        {.name = "attribute." + info.list[i].label, .is_variable = false}
                                    });
                                }
                                
                                
                                //mtt::String query = "IsA(_SUBTYPE, " + label + "), _SUBTYPE(" + "_" + searched_var + ")";
                                for (auto _p : preposition_list) {
                                    mtt::String pverb = l_prop.label + _p->label;
                                    auto* action = verb_root_equivalence(verb_add(pverb));
                                    (void)action;
                                    pverb = Word_Dictionary_Entry_name_for_verb_with_event(action, event);
                                    
                                    auto object_list = _p->get_active("OBJECT");

                                    
                                    auto& valid_object_list = object_list;
                                    //dt::get_prop_or_coref_list(object_list, &valid_object_list);
                                    
                                    std::vector<mtt::Thing*> all2 = {};
                                    for (auto _2 : valid_object_list) {
                                        auto* noun_type = noun_add(_2->label);
                                        (void)noun_type;
                                        if (_2->kind_str == "THING_TYPE") {
                                            
                                            
                                            // TODO: fix negation generation
                                            
                                            Attrib_Info info = {};
                                            rule_create_handle_attrib_info(_2, info);
                                            auto final_key = _2->label + info.key;
                                            
                                            auto object_var_name = _2->label;
                                            auto object_var_name_sub = cb.set_var_direct_name(final_key + "SUB", "", _2);
                                            mtt::String object_var_name_result = "";
                                            if (is_referenced_in_response(is_response, refs, _2)) {
                                                object_var_name_result = cb.set_var_direct_name(final_key, refs[_2], _2);
                                            } else {
                                                object_var_name_result = cb.set_var_direct_name(final_key, "", _2);
                                            }
                                            
                                            
                                            
//                                                        [NEGATED] = [
//                                                        {
//                                                            label=[], tag=[], type=[], kind=[], key=[NEGATED], idx=[0]
//                                                            value={
//                                                                FLAG=[true]
//                                                            }
//                                                        }
                                            
                                            cb.add_clause({
                                                {.name = "IsA", .is_variable = false, .is_negated = false},
                                                {.name = object_var_name_sub, .is_variable = true},
                                                {.name = object_var_name, .is_variable = false},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = object_var_name_sub, .is_variable = true, .is_negated = false},
                                                {.name = object_var_name_result, .is_variable = true},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = "Prefab", .is_variable = false, .is_negated = true},
                                                {.name = object_var_name_result, .is_variable = true},
                                            });
                                            
                                            
                                            cb.add_clause({
                                                {.name = pverb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = agent_var_name_result, .is_variable = true},
                                                {.name = object_var_name_result, .is_variable = true},
                                            });
                                            
                                            for (usize i = 0; i < info.list.size(); i += 1) {
                                                cb.add_clause({
                                                    {.name = "IsA", .is_variable = false, .is_negated = info.list[i].is_negated},
                                                    {.name = object_var_name_result, .is_variable = true},
                                                    {.name = "attribute." + info.list[i].label, .is_variable = false}
                                                });
                                            }
                                            
                                            
                                        } else if (_2->kind_str == "THING_INSTANCE") {
                                            bool something_found = for_each_thing_instance_value(*_2, [&](mtt::Thing* thing) {
                                                all2.push_back(thing);
                                            });
                                            
                                            
                                            // treat as type for now
                                            if (all2.size() > 1 || all2.size() == 0) {
                                                _2->kind_str = "THING_TYPE";
                                                auto object_var_name = _2->label;
                                                auto object_var_name_sub = cb.set_var_direct_name(_2->label + "SUB", "", _2);
                                                
                                                mtt::String object_var_name_result = "";
                                                if (is_referenced_in_response(is_response, refs, _2)) {
                                                    object_var_name_result = cb.set_var_direct_name(_2->label, refs[_2], _2);
                                                } else {
                                                    object_var_name_result = cb.set_var_direct_name(_2->label, "", _2);
                                                }
                                                
                                                cb.add_clause({
                                                    {.name = "IsA", .is_variable = false, .is_negated = false},
                                                    {.name = object_var_name_sub, .is_variable = true},
                                                    {.name = object_var_name, .is_variable = false},
                                                });
                                                
                                                cb.add_clause({
                                                    {.name = object_var_name_sub, .is_variable = true, .is_negated = false},
                                                    {.name = object_var_name_result, .is_variable = true},
                                                });
                                                
                                                cb.add_clause({
                                                    {.name = "Prefab", .is_variable = false, .is_negated = true},
                                                    {.name = object_var_name_result, .is_variable = true},
                                                });

                                                cb.add_clause({
                                                    {.name = pverb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                    {.name = agent_var_name_result, .is_variable = true},
                                                    {.name = object_var_name_result, .is_variable = true},
                                                });

                                            } else { // single value is okay for now
                                                mtt::String object_var_name = search_id(all2[0]);
                                                if (is_referenced_in_response(is_response, refs, _2)) {
                                                    cb.set_const(object_var_name, refs[_2], _2);
                                                } else {
                                                    cb.set_const(object_var_name, _2->label, _2);
                                                }
                                                cb.set_thing_dependency(mtt::thing_id(all2[0]));
                                                
                                                cb.add_clause({
                                                    {.name = "attribute.id_is", .is_variable = false, .is_negated = false},
                                                    {.name = object_var_name, .is_variable = true},
                                                    {.name = object_var_name, .is_variable = false},
                                                });
                                                
                                                cb.add_clause({
                                                    {.name = pverb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                    {.name = agent_var_name_result, .is_variable = true},
                                                    {.name = object_var_name, .is_variable = true},
                                                });
                                                
                                            }
                                        }
                                    }
                                }
                                
                            } else if (_->kind_str == "THING_INSTANCE") {
                                std::vector<mtt::Thing*> all = {};
                                bool something_found = for_each_thing_instance_value(*_, [&](mtt::Thing* thing) {
                                    all.push_back(thing);
                                });

                                
                                mtt::String agent_var_name;
                                // treat as type for now
                                if (all.size() > 1 || all.size() == 0) {
                                    _->kind_str = "THING_TYPE";
                                    mtt::String agent_var_name_result = "";
                                    if (is_referenced_in_response(is_response, refs, _)) {
                                        agent_var_name_result = cb.set_var(_->label, refs[_], _);
                                    } else {
                                        agent_var_name_result = cb.set_var(_->label, "", _);
                                    }
                                    
                                    auto agent_var_name = _->label;
                                    auto agent_var_name_sub = cb.set_var(_->label + "SUB", "", _);
                                    
//                                                    cb.add_clause({
//                                                        {.name = _->label, .is_variable = false, .is_negated = false},
//                                                        {.name = agent_var_name, .is_variable = true},
//                                                    });
                                    
                                    cb.add_clause({
                                        {.name = "IsA", .is_variable = false, .is_negated = false},
                                        {.name = agent_var_name_sub, .is_variable = true},
                                        {.name = agent_var_name, .is_variable = false},
                                    });
                                    
                                    cb.add_clause({
                                        {.name = agent_var_name_sub, .is_variable = true, .is_negated = false},
                                        {.name = agent_var_name_result, .is_variable = true},
                                    });
                                    agent_var_name = agent_var_name_result;
                                    
                                    cb.add_clause({
                                        {.name = "Prefab", .is_variable = false, .is_negated = true},
                                        {.name = agent_var_name_result, .is_variable = true},
                                    });
                                    
                                    
                                } else { // single value is okay for now
                                    
                                    agent_var_name = search_id(all[0]);
                                    if (is_referenced_in_response(is_response, refs, _)) {
                                        cb.set_const(agent_var_name, refs[_], _);
                                    } else {
                                        cb.set_const(agent_var_name, "AGENT", _);
                                    }
                                    cb.set_thing_dependency(mtt::thing_id(all[0]));
                                    
                                    cb.add_clause({
                                        {.name = "attribute.id_is", .is_variable = false, .is_negated = false},
                                        {.name = agent_var_name, .is_variable = true},
                                        {.name = agent_var_name, .is_variable = false},
                                    });
                                }
                                
                                for (auto _p : preposition_list) {
                                    mtt::String pverb = l_prop.label + _p->label;
                                    auto* action = verb_root_equivalence(verb_add(pverb));
                                    (void)action;
                                    pverb = Word_Dictionary_Entry_name_for_verb_with_event(action, event);
                                
                                    auto object_list = _p->get_active("OBJECT");
                                    auto& valid_object_list = object_list;
                                    //dt::get_prop_or_coref_list(object_list, &valid_object_list);
                                    std::vector<mtt::Thing*> all2 = {};
                                    for (auto _2 : valid_object_list) {
                                        auto* noun_type = noun_add(_2->label);
                                        (void)noun_type;
                                        if (_2->kind_str == "THING_TYPE") {
                                            
                                            
                                            // TODO: fix negation generation
                                            
                                            
                                            
//                                                        [NEGATED] = [
//                                                        {
//                                                            label=[], tag=[], type=[], kind=[], key=[NEGATED], idx=[0]
//                                                            value={
//                                                                FLAG=[true]
//                                                            }
//                                                        }
                                        
                                            auto object_var_name = _2->label;
                                            auto object_var_name_sub = cb.set_var_direct_name(_2->label + "SUB", "", _2);
                                            
                                            mtt::String object_var_name_result = "";
                                            if (is_referenced_in_response(is_response, refs, _2)) {
                                                object_var_name_result = cb.set_var_direct_name(_2->label, refs[_2], _2);
                                            } else {
                                                object_var_name_result = cb.set_var_direct_name(_2->label, "", _2);
                                            }
                                            
                                            cb.add_clause({
                                                {.name = "IsA", .is_variable = false, .is_negated = false},
                                                {.name = object_var_name_sub, .is_variable = true},
                                                {.name = object_var_name, .is_variable = false},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = object_var_name_sub, .is_variable = true, .is_negated = false},
                                                {.name = object_var_name_result, .is_variable = true},
                                            });
                                            
                                            cb.add_clause({
                                                {.name = "Prefab", .is_variable = false, .is_negated = true},
                                                {.name = object_var_name_result, .is_variable = true},
                                            });

                                            cb.add_clause({
                                                {.name = pverb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = agent_var_name, .is_variable = true, .is_id = true},
                                                {.name = object_var_name_result, .is_variable = true},
                                            });
                                            
                                        } else if (_2->kind_str == "THING_INSTANCE") {
                                            bool something_found = for_each_thing_instance_value(*_2, [&](mtt::Thing* thing) {
                                                all2.push_back(thing);
                                            });
                                            
                                            
                                            
                                            // treat as type for now
                                            if (all2.size() > 1 || all2.size() == 0) {
                                                _2->kind_str = "THING_TYPE";
                                                mtt::String object_var_name = "";
                                                if (is_referenced_in_response(is_response, refs, _2)) {
                                                    object_var_name = cb.set_var_direct_name(_2->label, refs[_2], _2);
                                                } else {
                                                    object_var_name = cb.set_var_direct_name(_2->label, "", _2);
                                                }
                                                
                                                cb.add_clause({
                                                    {.name = pverb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                    {.name = agent_var_name, .is_variable = true, .is_id = true},
                                                    {.name = object_var_name, .is_variable = true},
                                                });
                                                
                                                cb.add_clause({
                                                    {.name = "Prefab", .is_variable = false, .is_negated = true},
                                                    {.name = object_var_name, .is_variable = true},
                                                });
                                                
                                            } else { // single value is okay for now
                                                mtt::String object_var_name = search_id(all2[0]);
                                                
                                                if (is_referenced_in_response(is_response, refs, _2)) {
                                                    object_var_name = cb.set_const(object_var_name, refs[_2], _2);
                                                }
                                                cb.set_thing_dependency(mtt::thing_id(all2[0]));
                                                
                                                cb.add_clause({
                                                    {.name = pverb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                    {.name = agent_var_name, .is_variable = true},
                                                    {.name = object_var_name, .is_variable = true},
                                                });
                                                
                                                cb.add_clause({
                                                    {.name = "attribute.id_is", .is_variable = false, .is_negated = false},
                                                    {.name = object_var_name, .is_variable = true},
                                                    {.name = object_var_name, .is_variable = false},
                                                });
                                                
                                                
                                            }
                                        }
                                    }
                                }
                                
                                
                            } else {
                                success = false;
                                return false;
                            }
                        }
                        MTT_BP();
                    } else { // only an agent
                        
                        
                        
                        
                        
                        auto& valid_agent_list = agent_list;
                        //dt::get_prop_or_coref_list(agent_list, &valid_agent_list);
                        
                        mtt::String prev_agent_var_name_result = "";
                        
                        
                        dt::Dynamic_Array<dt::rules::Condition_Builder::Sym> single = {};
                        
                        {
                            usize idx = 0;
                            for (; auto _ : valid_agent_list) {
                                auto* noun_type = noun_add(_->label);
                                (void)noun_type;
                                if (_->kind_str == "THING_TYPE") {
                                    
                                    
                                    Attrib_Info info = {};
                                    rule_create_handle_attrib_info(_, info);
                                    auto final_key = _->label + info.key;
                                    
                                    mtt::String agent_var_name_result = "";
                                    if (is_referenced_in_response(is_response, refs, _)) {
                                        agent_var_name_result = cb.set_var(final_key, refs[_], _);
                                    } else {
                                        agent_var_name_result = cb.set_var(final_key, "", _);
                                    }
                                    
                                    auto agent_var_name = _->label;
                                    auto agent_var_name_sub = cb.set_var(final_key + "SUB", "", _);
                                    
                                    
                                    
//                                                cb.add_clause({
//                                                    {.name = _->label, .is_variable = false, .is_negated = false},
//                                                    {.name = agent_var_name, .is_variable = true},
//                                                });
                                    
                                    cb.add_clause({
                                        {.name = "IsA", .is_variable = false, .is_negated = false},
                                        {.name = agent_var_name_sub, .is_variable = true},
                                        {.name = agent_var_name, .is_variable = false},
                                    });
                                    
                                    cb.add_clause({
                                        {.name = agent_var_name_sub, .is_variable = true, .is_negated = false},
                                        {.name = agent_var_name_result, .is_variable = true},
                                    });
                                    
                                    cb.add_clause({
                                        {.name = "Prefab", .is_variable = false, .is_negated = true},
                                        {.name = agent_var_name_result, .is_variable = true},
                                    });
                                    
                                    if (prev_agent_var_name_result != "") {
                                        cb.add_clause({
                                            {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                            {.name = prev_agent_var_name_result, .is_variable = true},
                                            {.name = agent_var_name_result, .is_variable = true},
                                           // {.name = direct_object_var_name_result, .is_variable = true},
                                        });
                                    } else {
                                        if (st.has_properties && verb == "becomebegin") {
                                            auto* dict = dt::word_dict();
                                            for (const auto* prop_ : properties_list) {
                                                if (prop_->label == "trait" && !dict->is_ignored_attribute(prop_->value.text)) {
                                                    
                                                    cb.add_clause({
                                                        {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                        {.name = agent_var_name_result, .is_variable = true},
                                                        {.name = "attribute." + prop_->value.text, .is_variable = false},
                                                    });
                                                    
                                                }
                                            }
                                        } else if (verb == "existbegin") {
                                            Speech_Property* count_prop = nullptr;
                                            float64 count_to_exist = 1;
                                            if (_->try_get_only_prop("COUNT", &count_prop)) {
                                                count_to_exist = count_prop->value.numeric;
                                            }
                                            cb.add_value_clause({
                                                {.name = verb, .is_variable = false, .is_negated = is_negated((&l_prop))},
                                                {.name = agent_var_name_result, .is_variable = true},
                                                {.value = mtt::Any::from_Float64(count_to_exist)},
                                            });
                                            bool do_ignore = false;
                                            for (usize v_f_i = 0; v_f_i < cb.vars_to_find.size(); v_f_i += 1) {
                                                if (cb.vars_to_find[v_f_i] == agent_var_name_result) {
                                                    do_ignore = true;
                                                }
                                            }
                                            if (!do_ignore) {
                                                cb.vars_to_find.push_back(agent_var_name_result);
                                            }
                                            
                                        } else {
                                            single = {
                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = agent_var_name_result, .is_variable = true},
                                            };
                                        }
                                    }
                                    prev_agent_var_name_result = agent_var_name_result;
                                    
                                    for (usize i = 0; i < info.list.size(); i += 1) {
                                        cb.add_clause({
                                            {.name = "IsA", .is_variable = false, .is_negated = info.list[i].is_negated},
                                            {.name = agent_var_name_result, .is_variable = true},
                                            {.name = "attribute." + info.list[i].label, .is_variable = false}
                                        });
                                    }
                                    
                                } else if (_->kind_str == "THING_INSTANCE") {
                                    std::vector<mtt::Thing*> all = {};
                                    bool something_found = for_each_thing_instance_value(*_, [&](mtt::Thing* thing) {
                                        all.push_back(thing);
                                    });
                                    
                                    
                                    mtt::String agent_var_name;
                                    // treat as type for now
                                    if (all.size() > 1 || all.size() == 0) {
                                        _->kind_str = "THING_TYPE";
                                        mtt::String agent_var_name_result = "";
                                        if (is_referenced_in_response(is_response, refs, _)) {
                                            agent_var_name_result = cb.set_var(_->label, refs[_], _);
                                        } else {
                                            agent_var_name_result = cb.set_var(_->label, "AGENT", _);
                                        }
                                        
                                        auto agent_var_name = _->label;
                                        auto agent_var_name_sub = cb.set_var(_->label + "SUB", "", _);
                                        
//                                                    cb.add_clause({
//                                                        {.name = _->label, .is_variable = false, .is_negated = false},
//                                                        {.name = agent_var_name, .is_variable = true},
//                                                    });
                                        
                                        cb.add_clause({
                                            {.name = "IsA", .is_variable = false, .is_negated = false},
                                            {.name = agent_var_name_sub, .is_variable = true},
                                            {.name = agent_var_name, .is_variable = false},
                                        });
                                        
                                        cb.add_clause({
                                            {.name = "Prefab", .is_variable = false, .is_negated = true},
                                            {.name = agent_var_name_result, .is_variable = true},
                                        });
                                        
                                        cb.add_clause({
                                            {.name = agent_var_name_sub, .is_variable = true, .is_negated = false},
                                            {.name = agent_var_name_result, .is_variable = true},
                                        });
                                        if (prev_agent_var_name_result != "") {
                                            cb.add_clause({
                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = prev_agent_var_name_result, .is_variable = true},
                                                {.name = agent_var_name_result, .is_variable = true},
                                               // {.name = direct_object_var_name_result, .is_variable = true},
                                            });
                                        } else {
                                            single = {
                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                {.name = agent_var_name_result, .is_variable = true},
                                            };
                                        }
                                        prev_agent_var_name_result = agent_var_name_result;
                                        

                                        
                                        
                                    } else { // single value is okay for now
                                        
                                        agent_var_name = search_id(all[0]);
                                        if (is_referenced_in_response(is_response, refs, _)) {
                                            cb.set_const(agent_var_name, refs[_], _);
                                        } else {
                                            cb.set_const(agent_var_name, "AGENT", _);
                                        }
                                        cb.set_thing_dependency(mtt::thing_id(all[0]));
                                        
                                        cb.add_clause({
                                            {.name = "attribute.id_is", .is_variable = false, .is_negated = false},
                                            {.name = agent_var_name, .is_variable = true, .is_const = true},
                                            {.name = agent_var_name, .is_variable = false},
                                        });
                                        
                                        if (st.has_properties && verb == "becomebegin") {
                                            auto* dict = dt::word_dict();
                                            for (const auto* prop_ : properties_list) {
                                                if (prop_->label == "trait" && !dict->is_ignored_attribute(prop_->value.text)) {
                                                    
                                                    cb.add_clause({
                                                        {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                        {.name = agent_var_name, .is_variable = true},
                                                        {.name = "attribute." + prop_->value.text, .is_variable = false},
                                                    });
                                                    
                                                }
                                            }
                                        } else {
                                            
                                            if (verb == "equalbegin" || verb == "exceedbegin") {
                                                Speech_Property* count_prop = nullptr;
                                                if (_->try_get_only_prop("COUNT", &count_prop)) {
                                                    cb.add_value_clause({
                                                        {.name = verb, .is_variable = false, .is_negated = is_negated((&l_prop))},
                                                        {.name = agent_var_name, .is_variable = false, .is_const = true, .is_id = true, .value = mtt::Any::from_Thing_ID(thing_id(all[0]))},
                                                        {.value = mtt::Any::from_Float64(count_prop->value.numeric)},
                                                    });
                                                }
                                                
                                                
                                            } else {
                                                single = {
                                                    {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
                                                    {.name = agent_var_name, .is_variable = true, .is_const = false},
                                                };
                                                
                                            }
                                        }
                                        
                                        
                                    }
                                    
//                                                        if (prev_agent_var_name_result != "") {
//                                                            cb.add_clause({
//                                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
//                                                                {.name = prev_agent_var_name_result, .is_variable = true},
//                                                                {.name = agent_var_name, .is_variable = true},
//                                                               // {.name = direct_object_var_name_result, .is_variable = true},
//                                                            });
//                                                        } else {
//                                                            single = {
//                                                                {.name = verb, .is_variable = false, .is_negated = is_negated(&l_prop)},
//                                                                {.name = agent_var_name, .is_variable = true},
//                                                            };
//                                                        }
//                                                        prev_agent_var_name_result = agent_var_name;
                                    
                                    
                                } else {
                                    success = false;
                                    return false;
                                }
                                idx += 1;
                            }
                            if (idx == 1) {
                                cb.add_clause(single);
                            }
                        }
                        
                    }
                } else if (st.has_direct_object) {
                    if (st.has_preposition && st.has_indirect_object) {
                        MTT_BP();
                    } else if (st.has_preposition) {
                        MTT_BP();
                    } else { // only a direct object
                        MTT_BP();
                    }
                } else {
                    success = false;
                }
                
                
            }
            break;
        }
        case 1: {
            int x = 0;
            break;
        }
        case 2: {
            int x = 0;
            break;
        }
        }
    }
    return true;
}

Run_Result run(dt::DrawTalk* dt_ctx)
{
    DT_scope_open();
    
    Language_Context* lang_ctx = &dt_ctx->lang_ctx;
    DT_Evaluation_Context* eval_ctx = &lang_ctx->eval_ctx;
    ex::Exec_Args* args = &eval_ctx->args;
    DT_Behavior_Catalogue* cat = &lang_ctx->behavior_catalogue;
    mtt::World* world = dt_ctx->mtt;
    
    
    entries = {};
    
    
    prop_thing_mapping.clear();

    
//    static dt::Dynamic_Array<ENTRY_TYPE> entry_types = {};
//    //static dt::Dynamic_Array<Speech_Property*> entry_props = {};
//    static Speech_Property::Prop_List entry_props = {};

    if (auto& cmd_list_initial = Prop::get(args->root, "CMD_LIST"); !is_valid(cmd_list_initial)) {
        return run_failed();
    } else {
        
        for (auto* cmd : cmd_list_initial) {
            cmd->print();
        }
        
        struct Final_Props {
            Speech_Property* root;
            //Prop_List* original_cmds;
            Speech_Property::Prop_With_Ref_Map result;
        } props = {.root = args->root};
        
        mtt::job_dispatch_serial_sync((void*)&props, [](void* j_ctx) {
            Final_Props* props = (Final_Props*)j_ctx;
            
            props->root->traverse_preorder([](Speech_Property* prop_ptr) {
                auto& prop = *prop_ptr;
                
                auto seq_sim_list = prop.get_active("SEQUENCE_SIMULTANEOUS");
                auto seq_then_list = prop.get_active("SEQUENCE_THEN");
                
                const auto fill_in_agent_from_parent = [](Speech_Property::Active_Prop_List& prop_list) {
                    auto* self_prop = prop_list.container.front();
                    if (!self_prop->has_prop("AGENT")) {
                        auto* parent = self_prop->get_active_parent();
                        if (parent != nullptr) {
                            auto* parent_agent = parent->try_get_only_prop("AGENT");
                            if (parent_agent != nullptr) {
                                self_prop->push_prop("AGENT", parent_agent->copy());
                            }
                        }
                    }
                };
                
                if (is_valid(seq_sim_list)) {
                    fill_in_agent_from_parent(seq_sim_list);
                }
                
                if (is_valid(seq_then_list)) {
                    fill_in_agent_from_parent(seq_then_list);
                }
                
                
                if (prop.type_str != "pronoun") {
                    return true;
                }
                

                
                return true;
            });
            
            props->result = props->root->copy_and_resolve_refs();

            //props->result.root->print();
            MTT_BP();
        });
        
        
        auto& cmd_list = Prop::get(props.result.root, "CMD_LIST");
        auto& refs = props.result.prop_to_refs;
        
#ifndef NDEBUG
        for (auto* cmd : cmd_list) {
            cmd->print();
        }
#endif

        
        
        for (auto* cmd : cmd_list) {
            entries.clear();
            
            
            
            // MARK: preprocess disabled properties
            {
#ifndef NDEBUG
                cmd->print();
#endif
                for (auto* e : cmd->get_active("ACTION")) {
                    entries.push_back(ENTRY_TYPE_ACTION, e);
                }
                for (auto* e : cmd->get("TRIGGER_RESPONSE")) {
                    auto& trigger_list = e->get("TRIGGER");
                    auto& response_list = e->get("RESPONSE");
                    
                    bool triggers_valid = is_valid(trigger_list);
                    bool responses_valid = is_valid(response_list);
                    if (!triggers_valid && !responses_valid) {
                        continue;
                    }
                    if (!e->should_ignore() && (triggers_valid && responses_valid)) {
                        entries.push_back(ENTRY_TYPE_TRIGGER_RESPONSE, e);
                        continue;
                    }
                    for (auto* e_trigger : e->get_active("TRIGGER")) {
                        entries.push_back(ENTRY_TYPE_ACTION, e_trigger);
                    }
                    for (auto* e_response : e->get_active("RESPONSE")) {
                        entries.push_back(ENTRY_TYPE_ACTION, e_response);
                    }
                }
            }
            
            // for building a trigger/response call
            Call_Param_List call_params = {};
            
            
            for (usize i = 0, entry_count = entries.size(); i < entry_count; ) {
                auto& prop = *entries.props[i];
                auto type  =  entries.types[i];
                
                switch (type) {
                case ENTRY_TYPE_ACTION: {
                    // TODO:
                    mtt::Script_Builder scb = mtt::Script_Builder_make(world);
                    auto* saved_world_graph = mtt::curr_graph(world);
                    mtt::set_graph(world, &scb.script->connections);
                    scb.state = mtt::SCRIPT_BUILDER_STATE::NONE;
                    
                    
                    if (auto& goal_list = prop.get("GOAL"); is_valid(goal_list)) {
                        // TODO: goal logic (e.g. need)
                        
                        i += 1;
                    } else {
                        
                        scb.state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                        DT_scope_open();
                        Gen_Args gen_args = {};
                        action_proc(scb, cat, prop, i, gen_args);
                        DT_scope_close();
                        scb.state = mtt::SCRIPT_BUILDER_STATE::NONE;
                        
                        i += 1;
                    }
                    
                    {
                        auto* script = mtt::Script_Builder_build(&scb);
                        
                        mtt::set_graph(world, saved_world_graph);
                        
                        if (script == nullptr) {
                            return run_failed();
                        }
                        
                        
                        
                        
                        //script->connections.sorted_things_direct
                        
                        script->destroy_upon_ref_count_0 = true;
#ifndef NDEBUG
                        Script_print_as_things(script);
#endif
                        auto* script_instance = mtt::Script_Instance_from_script(script);
                        mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, script_instance);
                    }
                    
                    
                    break;
                }
                case ENTRY_TYPE_TRIGGER_RESPONSE: {
                    
                    mtt::Script_Builder scb = mtt::Script_Builder_make(world);
                    auto* saved_world_graph = mtt::curr_graph(world);
                    mtt::set_graph(world, &scb.script->connections);
                    scb.state = mtt::SCRIPT_BUILDER_STATE::NONE;
                    
                    
                    auto trigger_list  = prop.get_active("TRIGGER");
                    auto response_list = prop.get_active("RESPONSE");
                    auto end_condition_list = prop.get_active("END_CONDITION");

                    
                    dt::rules::Condition_Builder cb = {};
                    
                    
                    MTT_BP();
                LABEL_LOOP_EXIT:;
                    
                    if (!rule_create(trigger_list, cb, (Rule_Create_Args) {
                        .is_response = false,
                        .refs = refs,
                        .cat = cat,
                    })) {
                        MTT_BP();
                        mtt::set_graph(world, saved_world_graph);
                        return run_failed();
                    }
                    
                    if (cb.result.is_new_action) {
                        
                        scb.is_building_new_action = true;
                        scb.is_building_conditions = false;
                        
                        
                        for (auto l_prop_ptr : response_list) {
                            auto& prop = *l_prop_ptr;
                            
                            scb.state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                            DT_scope_open();
                            Gen_Args gen_args = {};
                            action_proc(scb, cat, prop, i, gen_args);
                            DT_scope_close();
                            scb.state = mtt::SCRIPT_BUILDER_STATE::NONE;
                        }
                        
                        i += 1;

                        
                        {
                            auto* script = mtt::Script_Builder_build(&scb);
                            
                            mtt::set_graph(world, saved_world_graph);
                            
                            if (script == nullptr) {
                                return run_failed();
                            }
                            
                            
                            
                            
                            //script->connections.sorted_things_direct
                            
                            script->destroy_upon_ref_count_0 = false;
#ifndef NDEBUG
                            Script_print_as_things(script);
#endif
                            
                            dt::create_new_action(script, cb.result.label);
                            
//                            auto* script_instance = mtt::Script_Instance_from_script(script);
//                            mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, script_instance);
                        }
                        
                        
                        break;
                    }
                    

                    
                    dt::rules::Condition_Builder cb_trigger = {};
                    dt::rules::Condition_Builder cb_response = {};
                    dt::rules::Condition_Builder cb_end_condition = {};
                    
                    
                    
                    //props.result.PRINT_REF_MAP();
                    for (auto l_prop_ptr : response_list) {
                        auto& l_prop = *l_prop_ptr;
                        
                        
                        
                        auto& prop_roles = cb.per_prop_roles;
                        //l_prop.print();
                        l_prop.traverse_preorder([&](Speech_Property* p) {
                            
                            
                            Speech_Property* PARENT = p->get_parent();
                            while (PARENT != nullptr && PARENT->kind_str != "ACTION" && PARENT->kind_str != "EXISTENTIAL") {
                                PARENT = PARENT->get_parent();
                            }
                            auto& pr = prop_roles.map[PARENT->ID];
                        
                            
                            if (p->kind_str == "THING_TYPE") {
                                Attrib_Info info = {};
                                rule_create_handle_attrib_info(p, info);
                                auto final_key = p->label + info.key;
                                
                                auto find_it = cb.var_label_to_role.find(final_key);
                                if (find_it != cb.var_label_to_role.end()) {
                                    //if (find_it->second.size() == 1 && (*find_it->second.begin()) == "")
                                    {
                                      //  find_it->second.clear();
                                        find_it->second.insert(p->key);
                                        
                                        auto var_fmt = cb.var_name_format(final_key);
                                        cb.var_to_role[var_fmt].clear();
                                        cb.var_to_role[var_fmt].insert(p->key);
                                        cb.role_to_var_label[p->key].insert(final_key);
                                        cb.role_to_var[p->key].insert({var_fmt, (rules::Var_Info){.is_const = false}});
                                        cb.var_label_to_prop[final_key].insert(p);
                                        
                                        
                                        pr.var_to_role[var_fmt].insert(p->key);
                                        pr.role_to_var_label[p->key].insert(final_key);
                                        pr.role_to_var[p->key].insert({var_fmt, (rules::Var_Info){
                                            .is_const = false,
                                            .has_value = false,
                                            .label = final_key}
                                        });
                                        
                                        cb.vars_to_find.push_back(var_fmt);
                                        
                                        {
                                            Speech_Property* COREF_PARENT = p->coref_parent;
                                            if (COREF_PARENT != nullptr) {
                                                while (COREF_PARENT != nullptr && COREF_PARENT->kind_str != "ACTION" && COREF_PARENT->kind_str != "EXISTENTIAL") {
                                                    COREF_PARENT = COREF_PARENT->get_parent();
                                                }
                                                prop_roles.map[COREF_PARENT->ID] = pr;
                                            }
                                        }
                                    }
                                }
                            } else if (p->kind_str == "THING_INSTANCE") {
                                MTT_print("PARENT: %llu, P: %llu\n", PARENT->ID, p->ID);
                                bool something_found = false;

                                
                                for_each_thing_instance_value(*p, [&](mtt::Thing* thing) {
                                    auto KEY = "MTTID" + std::to_string(mtt::thing_id(thing));
                                    auto find_it = cb.var_label_to_role.find(KEY);
                                    if (find_it != cb.var_label_to_role.end()) {
                                        if (find_it->second.size() == 1 && (*find_it->second.begin()) == "") {
                                            find_it->second.clear();
                                            find_it->second.insert(p->key);
                                            
                                            auto var_fmt = KEY;
                                            cb.var_to_role[var_fmt].clear();
                                            cb.var_to_role[var_fmt].insert(p->key);
                                            cb.role_to_var_label[p->key].insert(p->label);
                                            cb.role_to_var[p->key].insert({var_fmt, (rules::Var_Info){.is_const = true, .has_value = true, .value = mtt::Any::from_Thing_ID(thing->id)}});
                                            cb.var_label_to_prop[var_fmt].insert(p);
                                            something_found = true;
                                            
                                            pr.var_to_role[var_fmt].insert(p->key);
                                            pr.role_to_var_label[p->key].insert(p->label);
                                            pr.role_to_var[p->key].insert({var_fmt, (rules::Var_Info){
                                                .is_const = true,
                                                .has_value = true,
                                                .value = mtt::Any::from_Thing_ID(thing->id),
                                                .label = p->label}
                                            });
                                        }
                                    }
                                });
                                
                                if (!something_found) {
                                    for_each_thing_instance_value(*p, [&](mtt::Thing* thing) {
                                        auto KEY = "MTTID" + std::to_string(mtt::thing_id(thing));
                                        //auto find_it = cb.var_label_to_role.find(KEY);
                                        
                                        {
                                            auto var_fmt = KEY;
                                            cb.var_to_role[var_fmt].clear();
                                            cb.var_to_role[var_fmt].insert(p->key);
                                            cb.role_to_var[p->key].insert({var_fmt, (rules::Var_Info){.is_const = true, .has_value = true, .value = mtt::Any::from_Thing_ID(thing->id)}});
                                            cb.var_label_to_prop[var_fmt].insert(p);
                                            
                                            pr.var_to_role[var_fmt].insert(p->key);
                                            pr.role_to_var_label[p->key].insert(p->label);
                                            pr.role_to_var[p->key].insert({var_fmt, (rules::Var_Info){
                                                .is_const = true,
                                                .has_value = true,
                                                .value = mtt::Any::from_Thing_ID(thing->id),
                                                .label = p->label}
                                            });
                                            
                                        }
                                    });
                                }
                                
                                Speech_Property* COREF_PARENT = p->coref_parent;
                                {
                                    
                                    if (COREF_PARENT != nullptr) {
                                        while (COREF_PARENT != nullptr && COREF_PARENT->kind_str != "ACTION" && COREF_PARENT->kind_str != "EXISTENTIAL") {
                                            COREF_PARENT = COREF_PARENT->get_parent();
                                        }
                                        prop_roles.map[COREF_PARENT->ID] = pr;
                                    }
                                }
                            }
                            return true;
                        });
                    }
                    
                    //cb.print();
                    
                    cb.build();
                    auto CB_RESULT = cb.result;
                    cb_trigger = cb;
                    cb.print();
                    MTT_BP();
                    cb = {};
                    
                    
                    //MTT_print("%s", "RESPONSE\n");
//                    if (!handle_main(response_list, cb, true)) {
//                        MTT_BP();
//                        return run_failed();
//                    }
//
//                    cb.print();
//                    MTT_BP();
//                    cb.build();
//                    cb_response = cb;
//
//                    cb = {};
                    
                    {
                        
                    }
                    
                    
                    for (auto l_prop_ptr : response_list) {
                        auto& l_prop = *l_prop_ptr;
                        //l_prop.print();
                        switch (second_cases[l_prop.annotation]) {
                        default: {
                            
                            scb.state = mtt::SCRIPT_BUILDER_STATE::RESPONSE;
                            scb.condition_builders = {
                                .trigger = &cb_trigger,
                                .response = &cb_response,
                                .end_condition = &cb_end_condition
                            };
                            scb.is_building_conditions = true;
                            scb.has_end_condition = end_condition_list.is_valid();
                            ASSERT_MSG(!scb.has_end_condition, "%s\n", "Should be temporarily unused in all cases now!");
                            DT_scope_open();
                            Gen_Args gen_args = {};
                            action_proc(scb, cat,  l_prop, i, gen_args);
                            DT_scope_close();
                            scb.state = mtt::SCRIPT_BUILDER_STATE::NONE;
                            // TODO: this will need to be wrapped with the trigger
                            scb.is_building_conditions = false;
                            scb.has_end_condition = false;
                            break;
                        }
                        case 1: {
                            break;
                        }
                        case 2: {
                            break;
                        }
                        }
                    }
                    
                    i += 1;
                    
                    {
                        auto* script = mtt::Script_Builder_build(&scb);
                        
                        mtt::set_graph(world, saved_world_graph);
                        
                        if (script == nullptr) {
                            return run_failed();
                        }
                        
                        
                        
                        
                        //script->connections.sorted_things_direct
                        
                        script->destroy_upon_ref_count_0 = false;
                        
                        // NOTE: should not instantiate
                        Script_print_as_things(script);
                        
                        Script_Rules rules = {
                            std::move(*scb.condition_builders.trigger),
                            std::move(*scb.condition_builders.response),
                            ((scb.condition_builders.end_condition != nullptr) ? std::move(*scb.condition_builders.end_condition) : (dt::rules::Condition_Builder){}),
                                .has_end_conditions = (scb.condition_builders.end_condition != nullptr)
                        };
                        
                        {
                            // this is the script that calls the generated script when results come in
                            mtt::Script_Builder scb = mtt::Script_Builder_make(world);
                            auto* saved_world_graph = mtt::curr_graph(world);
                            mtt::set_graph(world, &scb.script->connections);
                            scb.state = mtt::SCRIPT_BUILDER_STATE::NONE;
                            mtt::String label_str = {};
                            {
                                mtt::begin_parallel_group(&scb, "ROOT");
                                
                                usize label_len = 0;
                                auto& ins_saved = dt_ctx->lang_ctx.eval_ctx.instructions_saved();
                                
                                dt::Dynamic_Array<Instruction*> sorted = ins_saved;
                                
                                std::sort(sorted.begin(), sorted.end(), [&](Instruction* first, Instruction* second) {
                                    return first->visit_id < second->visit_id;
                                });
                                
                                for (usize i = 0; i < ins_saved.size(); i += 1) {
                                    label_len += ins_saved[i]->display_label.size() + 1;
                                }
                                
//                                label_str.reserve(m::max(2llu, label_len + dt::verb_event_suffix[cb_trigger.event].size() + 1));
//                                label_str += dt::verb_event_suffix[cb_trigger.event] + ":";
                                
                                for (isize i = 0; i < sorted.size() - 1; i += 1) {
                                    label_str += sorted[i]->display_label + " ";
                                }
                                for (isize i = sorted.size() - 1; i < sorted.size(); i += 1) {
                                    label_str += sorted[i]->display_label;
                                }
                                mtt::Rule_Eval_State* rule_eval_state = rule_result_eval(&scb, label_str);
                                
                                rule_eval_state->to_invoke_script_ID = script->id;
                                mtt::end_parallel_group(&scb, End_Parallel_Group_Args_endless_loop());
                            }
                            if (CB_RESULT.is_new_action) {
                                dt::create_new_action(script, CB_RESULT.label);
                            }
                            
                            auto* caller_script = mtt::Script_Builder_build(&scb);
                            caller_script->destroy_upon_ref_count_0 = false;
                            caller_script->is_rule = true;
                            auto* rule_script_instance = mtt::Script_Instance_from_script(caller_script);
                            
                            mtt::Runtime::ctx()->rule_label_map.insert({rule_script_instance->id, label_str});
                            
                            //Script_print_as_things(caller_script);
                            
                            mtt::set_graph(world, saved_world_graph);
                            
                            mtt::push_script_rule_task(&mtt::Runtime::ctx()->script_tasks, rule_script_instance, rules);
                            
                            
                            
                        }
                        
                    }
                    
                    break;
                }
                case ENTRY_TYPE_UNKNOWN: {
                    return run_failed();
                    ASSERT_MSG(false, "Should not have unknown entry type");
                    i += 1;
                    
                    break;
                }
                }
                
                
            }

            // ...
            MTT_BP();
        }
        
        
        
        mtt::job_dispatch_serial_sync((void*)(props.result.root), [](void* j_ctx) {
            Speech_Property* root = (Speech_Property*)j_ctx;
            Speech_Property::destroy_recursive(root);
        });
    }
    /*
     for (auto* e : cmd->get_active("ACTION")) {
         entry_types.push_back(ENTRY_TYPE_ACTION);
         entry_props.push_back(e);
     }
     for (auto* e : cmd->get("TRIGGER_RESPONSE")) {
         auto& trigger_list = e->get("TRIGGER");
         auto& response_list = e->get("RESPONSE");
         if (!is_valid(trigger_list) && !is_valid(response_list)) {
             continue;
         }
         if (!e->should_ignore()) {
             entry_types.push_back(ENTRY_TYPE_TRIGGER_RESPONSE);
             entry_props.push_back(e);
             continue;
         }
         for (auto* e_trigger : e->get_active("TRIGGER")) {
             entry_types.push_back(ENTRY_TYPE_ACTION);
             entry_props.push_back(e_trigger);
         }
         for (auto* e_response : e->get_active("RESPONSE")) {
             entry_types.push_back(ENTRY_TYPE_ACTION);
             entry_props.push_back(e_response);
         }
     }
     */

    
    
    
    DT_scope_close();
    
    return run_ok(nullptr);
}

}

//bool aHasMulti = A && Size(A) > 1;
//bool hasBC = B && C
//if (aHasMulti && hasBC)
//{
//    ACTION_WITH_A_B_C(A,B,C);
//}
//else if (aHasMulti) {
//    //other code
//}
//else if (hasBC) {
//    ACTION_WITH_B_C(B, C)
//}
//else {
//    //idk
//}
