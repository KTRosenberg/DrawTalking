//
//  drawtalk_run.cpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 8/13/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "drawtalk_run.hpp"

namespace dt::old {

Run_Result run(dt::DrawTalk* dt_ctx)
{
    return {};
    
    DT_scope_open();
    
    Language_Context* lang_ctx = &dt_ctx->lang_ctx;
    DT_Evaluation_Context* eval_ctx = &lang_ctx->eval_ctx;
    ex::Exec_Args* args = &eval_ctx->args;
    DT_Behavior_Catalogue* cat = &lang_ctx->behavior_catalogue;
    
    using Prop      = Speech_Property;
    using Prop_List = Speech_Property::Prop_List;
    
    // TODO: ...
    

    dt::Dynamic_Array<Speech_Property*>* cmd_list = nullptr;
    if (!args->root->try_get_prop("CMD_LIST", &cmd_list)) {
        return run_failed();
    }
    
    for (usize c = 0; c < cmd_list->size(); c += 1) {
        Speech_Property* cmd = (*cmd_list)[c];
        args->root_cmd = cmd;
        args->root_cmd->print();
        Prop_List* action = nullptr;
        Prop_List* trigger_response = nullptr;
        
        dt::Command_List cmds;
        
        
        dt::Dynamic_Array<Speech_Property*> list;

        
        dt::Dynamic_Array<T_R> trigger_response_list;
        Prop_List* sequence_then         = nullptr;
        if (args->root_cmd->try_get_prop("ACTION", &action)) {
            for (usize i = 0; i < action->size(); i += 1) {
                Speech_Property& prop = *((*action)[i]);
                if (prop.should_ignore()) {
                    continue;
                }
                list.push_back(&prop);
            }
        }
        
        // if trigger response is disabled, can still use the actions underneath, standalone if not disabled
        if (args->root_cmd->try_get_prop("TRIGGER_RESPONSE", &trigger_response)) {
            for (usize i = 0; i < trigger_response->size(); i += 1) {
                Speech_Property& prop = *((*trigger_response)[i]);
                Speech_Property::Prop_List* trigger_list  = nullptr;
                Speech_Property::Prop_List* response_list = nullptr;
                if (prop.parent_ref_disabled) {
                    if (prop.try_get_prop("TRIGGER", &trigger_list)) {
                        for (usize t = 0; t < trigger_list->size(); t += 1) {
                            auto* sub_prop = (*trigger_list)[t];
                            if (sub_prop->parent_ref_disabled) {
                                continue;
                            }
                            
                            list.push_back(sub_prop);
                        }
                    }
                    if (prop.try_get_prop("RESPONSE", &response_list)) {
                        for (usize r = 0; r < response_list->size(); r += 1) {
                            auto* sub_prop = (*response_list)[r];
                            if (sub_prop->parent_ref_disabled) {
                                continue;
                            }
                            
                            list.push_back(sub_prop);
                        }
                    }
                    continue;
                } else if (prop.try_get_prop("TRIGGER", &trigger_list) &&
                           prop.try_get_prop("RESPONSE", &response_list)) {
                    T_R tr;
                    tr.container = &prop;
                    tr.triggers = trigger_list;
                    tr.responses = response_list;
                    trigger_response_list.push_back(tr);
                }
                
            }
        }
        
        for (usize i = 0; i < list.size(); i += 1) {
                
            Speech_Property& prop = *list[i];
            
            prop.print();

            Prop_List* agent                 = nullptr;
            Prop_List* object                = nullptr;
            Prop_List* direct_object         = nullptr;
            Prop_List* indirect_object       = nullptr;
            
            Prop_List* property              = nullptr;
            Prop_List* time                  = nullptr;
            Prop_List* preposition           = nullptr;
            //Prop_List* preposition_object    = nullptr;
            //Prop_List* action                = nullptr;

            prop.try_get_prop("AGENT",           &agent);
            prop.try_get_prop("OBJECT",          &object);
            prop.try_get_prop("DIRECT_OBJECT",   &direct_object);
            prop.try_get_prop("INDIRECT_OBJECT", &indirect_object);
            // check if parent link is there. If not, should treat as an independent action
            prop.try_get_prop("SEQUENCE_THEN",   &sequence_then);
            prop.try_get_prop("PROPERTY",        &property);
            // will require queries
            prop.try_get_prop("TIME",            &time);
            
            
//                dt::Command* cmd = dt::Command::make();
            //cmd->meta.timestamp = dt_ctx->core->time_seconds;
//                if (sequence_then) {
//                    int BP = 0;
//                    auto TMP = []() -> dt::Command* { return dt::Command::make(); };
//                    dt::Command* next = TMP();
//                    next->meta.timestamp = dt_ctx->core->time_seconds;
//                    cmd->next_cmd = next->id;
//                }
            
            
            //dt::Dynamic_Array<Prop_List*> preposition_object_records;
            
            auto gen_thing_args = [&]()
            {
                
                
            };
            
            
            
            DT_Behavior* b = nullptr;
            if (cat->lookup(prop.label, prop.sub_label, &b)) {
                DT_print("RUNNING ACTION: %s\n", (prop.label + ":" + prop.sub_label).c_str());

                DT_Behavior::Args dt_args = DT_Behavior::Args::make(
                                                                    dt_ctx, cat, b, &prop, &list, args->root, &cmds
                                                                    );
                
                b->start(dt_args);
                
            } else {
                DT_print("NOT FOUND!\n");
            }
            

            
        }
        for (usize tr = 0; tr < trigger_response_list.size(); tr += 1){
            for (usize i = 0; i < trigger_response->size(); i += 1) {
                T_R& t_r = trigger_response_list[i];
                
                auto& triggers = *t_r.triggers;
                auto& responses = *t_r.responses;
                if (triggers.empty() || responses.empty()) {
                    continue;
                }
                
                rules::Condition_Builder cb;
                
                
                for (usize t_i = 0; t_i < triggers.size(); t_i += 1) {
                    auto& prop = *triggers[t_i];
                    if (prop.annotation == "RELATION") {
                        DT_Behavior* b = nullptr;
                        
                        if (cat->lookup(prop.label, prop.sub_label, &b)) {
                            DT_Behavior::Args dt_args = DT_Behavior::Args::make(
                                dt_ctx, cat, b, &prop, &list, args->root, &cmds
                            );

                            dt_args.is_trigger    = true;
                            dt_args.is_response   = false;
                            dt_args.is_relation   = true;
                            dt_args.is_continuous = false;
                            dt_args.conditional_builder = &cb;
                            
                            b->start(dt_args);
                        }
                    } else if (prop.annotation == "CONTINUOUS") {
                        DT_Behavior* b = nullptr;
                        
                        if (cat->lookup(prop.label, prop.sub_label, &b)) {
                            DT_Behavior::Args dt_args = DT_Behavior::Args::make(
                                dt_ctx, cat, b, &prop, &list, args->root, &cmds
                            );
 
                            dt_args.is_trigger    = true;
                            dt_args.is_response   = false;
                            dt_args.is_relation   = true;
                            dt_args.is_continuous = true;
                            dt_args.conditional_builder = &cb;
                            
                            b->start(dt_args);
                        }
                    } else {
                        DT_Behavior* b = nullptr;
                        
                        if (cat->lookup(prop.label, prop.sub_label, &b)) {
                            DT_Behavior::Args dt_args = DT_Behavior::Args::make(
                                dt_ctx, cat, b, &prop, &list, args->root, &cmds
                            );

                            dt_args.is_trigger    = true;
                            dt_args.is_response   = false;
                            dt_args.is_relation   = false;
                            dt_args.is_continuous = false;
                            dt_args.conditional_builder = &cb;
                            
                            b->start(dt_args);
                        }
                    }
                }
                
                for (usize r_i = 0; r_i < responses.size(); r_i += 1) {
                    auto& prop = *responses[r_i];
                    if (prop.annotation == "RELATION") {
                        DT_Behavior* b = nullptr;
                        
                        if (cat->lookup(prop.label, prop.sub_label, &b)) {
                            DT_Behavior::Args dt_args = DT_Behavior::Args::make(
                                dt_ctx, cat, b, &prop, &list, args->root, &cmds
                            );

                            dt_args.is_trigger    = false;
                            dt_args.is_response   = true;
                            dt_args.is_relation   = true;
                            dt_args.is_continuous = false;
                            dt_args.conditional_builder = &cb;
                            
                            b->start(dt_args);
                        }
                    } else if (prop.annotation == "CONTINUOUS") {
                        DT_Behavior* b = nullptr;
                        
                        if (cat->lookup(prop.label, prop.sub_label, &b)) {
                            DT_Behavior::Args dt_args = DT_Behavior::Args::make(
                                dt_ctx, cat, b, &prop, &list, args->root, &cmds
                            );

                            dt_args.is_trigger    = false;
                            dt_args.is_response   = true;
                            dt_args.is_relation   = true;
                            dt_args.is_continuous = true;
                            dt_args.conditional_builder = &cb;
                            
                            b->start(dt_args);
                        }
                    } else {
                        DT_Behavior* b = nullptr;
                        
                        if (cat->lookup(prop.label, prop.sub_label, &b)) {
                            DT_Behavior::Args dt_args = DT_Behavior::Args::make(
                                dt_ctx, cat, b, &prop, &list, args->root, &cmds
                            );

                            dt_args.is_trigger    = false;
                            dt_args.is_response   = true;
                            dt_args.is_relation   = false;
                            dt_args.is_continuous = false;
                            dt_args.conditional_builder = &cb;
                            
                            b->start(dt_args);
                        }
                    }
                }
            }
        }
        
    }
    
    DT_scope_close();
    
    return run_ok(nullptr);
}

}
