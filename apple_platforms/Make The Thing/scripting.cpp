//
//  scripting.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/16/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "scripting.hpp"
#include "thing_lib.hpp"

namespace mtt {

cstring root = "scripts";

void from_JSON_files(mtt::World* world, std::vector<RAW_SCRIPT*>& out, Text_File_Load_Proc callback)
{
    cstring file_extension = "json";
    std::vector<std::string> paths;
    mtt::File_System_find_file_paths_with_extension(root, &file_extension, 1, paths);
    
    from_JSON_file(world, paths, out, callback);
}


void from_JSON_file(mtt::World* world, std::vector<std::string>& path_list, std::vector<RAW_SCRIPT*>& out, Text_File_Load_Proc callback)
{
    Text_File_Load_Descriptor desc;
    if (callback == nullptr) {
        callback = [](Text_File_Load_Result result, bool status) -> bool {
            return true;
        };
    }
    
    desc.paths.resize(path_list.size());
    
    for (usize i = 0; i < desc.paths.size(); i += 1) {
        desc.paths[i].path = path_list[i];
        desc.paths[i].extension = "json";
    }
    desc.custom_data = (void*)callback;
    Text_File_load_async(&desc, [](Text_File_Load_Result result, bool status) -> bool {
        ((Text_File_Load_Proc)result.custom_data)(result, status);
        
        
        for (usize i = 0; i < result.text.size(); i += 1) {
            usize size = 4096 * 2;
            ArduinoJson::DynamicJsonDocument doc(size);
            DeserializationError error = deserializeJson(doc, result.text[i]);
            while (error) {
                if (strcmp(error.c_str(), "NoMemory") != 0) {
                    MTT_error("%s", "JSON deserialization memory issue\n");
                    break;
                } else {
                    doc.clear();
                }
                
                size = size * 2;
                doc = ArduinoJson::DynamicJsonDocument(size);
                error = deserializeJson(doc, result.text[i]);
            }
            std::cout << doc << std::endl;
            doc.clear();
        }
        
        
        return true;
    });
}

void init_script_instance_instructions(Script_Instance* s)
{
    Script* source_s = s->source_script;
    
    auto& things = source_s->connections.sorted_things_direct;
    
    s->thing_local_fields.resize(things.size());
    for (usize i = 0; i < things.size(); i += 1) {
        s->thing_local_fields[i].resize(1);
    }
    
    for (usize i = 0; i < things.size(); i += 1) {
        mtt::Thing* src_thing = things[i];
        mtt::World* world = mtt::world(src_thing);
//        if (src_thing->archetype_id == mtt::ARCHETYPE_CALL &&
//            s->thing_local_fields[i][0].contents != nullptr) {
//            continue;
//        }
        
        
        mtt::init_field_list_from_source(
                                         world,
                                         field_allocator(world),
                                         &s->thing_local_fields[i][0],
                                         //&src_thing->field_descriptor
                                         &s->thing_initial_fields[i]
                                         );
    }
    
    if (false) {
#define this s
    Script* source_s = this->source_script;
    
    auto& things = source_s->connections.sorted_things_direct;
    
//        mtt::Set<void*> bla = {};
//        bool prev_size = 0;
    for (usize i = 0; i < things.size(); i += 1) {
        mtt::Thing* src_thing = things[i];
        mtt::World* world = mtt::world(src_thing);
        
        //mtt::set_active_fields_to_default(src_thing);
        
        ASSERT_MSG(i < this->thing_local_fields.size(), "???");
        
        for (usize f = 0; f < this->thing_local_fields[i].size(); f += 1) {
            destroy_fields(*field_allocator(world), src_thing->field_descriptor, this->thing_local_fields[i][f]);
        }
        destroy_fields(*field_allocator(world), src_thing->field_descriptor, this->thing_initial_fields[i].data);
        
    }
        
    }
#undef this
}

void init_script_instance_instructions_range(Script_Instance* s, usize first_index, usize last_index)
{
    Script* source_s = s->source_script;
    
    auto& things = source_s->connections.sorted_things_direct;
    
    usize exclusive_bound = last_index + 1;
    for (usize i = first_index; i < exclusive_bound; i += 1) {
        s->thing_local_fields[i].resize(1);
    }
    
    for (usize i = first_index; i < exclusive_bound; i += 1) {
        mtt::Thing* src_thing = things[i];
        mtt::World* world = mtt::world(src_thing);
        

        mtt::init_field_list_from_source(
                                         world,
                                         field_allocator(world),
                                         &s->thing_local_fields[i][0],
                                         //&src_thing->field_descriptor
                                         &s->thing_initial_fields[i]
                                         );
    }
}


usize get_loop_iteration(Script_Instance* s)
{
    return s->contexts[s->ctx_idx].loop_iteration;
}
void set_loop_iteration(Script_Instance* s, usize iteration)
{
    s->contexts[s->ctx_idx].loop_iteration = iteration;
}

Loop_Status begin_loop(Script_Instance* s, mtt::Thing* src_thing)
{
    auto* b_loop = mtt::access<mtt::For_Each_Loop_Begin>(src_thing, "state");
    auto* e_loop = mtt::Thing_get(mtt::world(src_thing), b_loop->loop_end);
    auto& things = s->source_script->connections.sorted_things_direct;
    
    Loop_Status status = {};
    
    if (b_loop->index >= b_loop->count || b_loop->index <= 0) {
        status.should_enter = false;
        status.continuation = (e_loop->eval_index + 1 < s->source_script->connections.sorted_things_direct.size()) ? s->source_script->connections.sorted_things_direct[e_loop->eval_index + 1] : nullptr;
    } else {
        status.should_enter = true;
        

        
        isize begin_idx = src_thing->eval_index;
        isize end_idx = e_loop->eval_index;
        usize count = b_loop->count;
        
        
        if (!b_loop->is_started) {
            b_loop->is_started = true;
            b_loop->index = 0;

            mtt::World* world = mtt::world(src_thing);

            s->contexts.push_back((Script_Contexts){});
            Script_Context ctx = {};
            ctx.first_idx = src_thing->eval_index;
            ctx.instruction_idx = ctx.first_idx;
            s->contexts.back().ctx_stack.push_back(ctx);
            
            usize bound = count;
            if (b_loop->is_stateless) {
                bound = 1;
            }
            
            for (isize i = begin_idx + 1; i < end_idx - 1; i += 1) {
                mtt::Thing* thing = things[i];
                auto& slot = s->thing_local_fields[thing->eval_index];
                usize count_of_slot = slot.size();
                for (usize iteration = 0; iteration < bound; iteration += 1) {
                    if (iteration >= count_of_slot) {
                        slot.push_back({});
                    }
                }

                for (usize iteration = 0; iteration < bound; iteration += 1) {
//                    if (src_thing->archetype_id == mtt::ARCHETYPE_CALL &&
//                        s->thing_local_fields[i][0].contents != nullptr) {
//                        continue;
//                    }
                    mtt::init_field_list_from_source(
                                                     world,
                                                     field_allocator(world),
                                                     &slot[iteration],
                                                     //&thing->field_descriptor
                                                     &s->thing_initial_fields[i]
                                                     
                                                     );
                }
            }
            // re-initialize port entries
            s->output.ensure_port_entries_count(b_loop->count);
//            for (auto i = begin_idx + 1; i < end_idx - 1; i += 1) {
//                for (auto it_count = 0; it_count < b_loop->count; it_count += 1) {
//                    s->output.port_entries(it_count)[i].out = {};
//                }
//            }
        } else if (b_loop->is_stateless) {



            mtt::World* world = mtt::world(src_thing);


            
            usize bound = 1;
            
            for (isize i = begin_idx + 1; i < end_idx - 1; i += 1) {
                mtt::Thing* thing = things[i];
                auto& slot = s->thing_local_fields[thing->eval_index];
                usize count_of_slot = slot.size();
                for (usize iteration = 0; iteration < bound; iteration += 1) {
                    if (iteration >= count_of_slot) {
                        slot.push_back({});
                    }
                }

                for (usize iteration = 0; iteration < bound; iteration += 1) {
//                    if (src_thing->archetype_id == mtt::ARCHETYPE_CALL &&
//                        s->thing_local_fields[i][0].contents != nullptr) {
//                        continue;
//                    }
                    mtt::init_field_list_from_source(
                                                     world,
                                                     field_allocator(world),
                                                     &slot[iteration],
                                                     //&thing->field_descriptor
                                                     &s->thing_initial_fields[i]
                                                     
                                                     );
                }
            }
            // re-initialize port entries
            s->output.ensure_port_entries_count(b_loop->count);
//            for (auto i = begin_idx + 1; i < end_idx - 1; i += 1) {
//                for (auto it_count = 0; it_count < b_loop->count; it_count += 1) {
//                    s->output.port_entries(it_count)[i].out = {};
//                }
//            }
        }
        
        set_loop_iteration(s, b_loop->index);
        s->output.set_port_entries_index(b_loop->index);
        
        for (isize i = begin_idx + 1; i < end_idx - 1; i += 1) {
            mtt::Thing* thing = things[i];
            ASSERT_MSG(i == thing->eval_index, "i should correspond to Thing position in list");
            auto& slot = s->thing_local_fields[thing->eval_index];
            mtt::set_active_fields(thing, &slot[b_loop->index]);
        }
    }
    
    return status;
}
//void advance_loop(Script_Instance* s, mtt::Thing* src_thing)
//{
//    auto& slot = s->thing_local_fields[src_thing->eval_index];
//
//    mtt::set_active_fields(src_thing, &slot[get_loop_iteration(s)]);
//}
void end_loop(Script_Instance* s, mtt::Thing* src_thing, usize i)
{
    auto* e_loop = mtt::access<mtt::For_Each_Loop_End>(src_thing, "state");
    auto* b_loop = mtt::access<mtt::For_Each_Loop_Begin>(mtt::Thing_get(mtt::world(src_thing), e_loop->loop_begin), "state");
    b_loop->index = 0;
    set_loop_iteration(s, b_loop->index);
    s->output.set_port_entries_index(b_loop->index);
}


Script_Instance* Script_Instance::init(Script* script, void (*post_init)(Script_Instance* s, void* data), void* data)
{
    this->source_script     = script;
    this->contexts          = script->contexts;
    this->output            = script->connections.output;
    ASSERT_MSG(script->connections.output.list.size() > 0, "???");
    this->on_start          = script->on_start;
    this->on_begin_frame    = script->on_begin_frame;
    this->on_end_frame      = script->on_end_frame;
    this->on_cancel         = script->on_cancel;
    this->on_done           = script->on_done;
    this->on_terminate      = script->on_terminate;
    this->label             = script->label;
    this->allow_duplicates_for_agent = script->allow_duplicates_for_agent;
    
    mtt::World* world = mtt::ctx();
    
    
    Script* source_s = this->source_script;
    auto& things = source_s->connections.sorted_things_direct;
    auto& initial_fields = this->thing_initial_fields;
    initial_fields.resize(things.size());
    for (usize i = 0; i < things.size(); i += 1) {
        mtt::Thing* src_thing = things[i];
        this->thing_initial_fields[i].data.contents = nullptr;
        mtt::copy_field_list(world, field_allocator(world), &initial_fields[i], &src_thing->field_descriptor);
    }
    
    
    
    
    this->preserve_lookup = script->preserve_lookup;
    this->set_lookup_copy(&script->lookup());
    
//    if (post_init != nullptr) {
//        for (usize i = 0; i < things.size(); i += 1) {
//            mtt::Thing* src_thing = things[i];
//            mtt::set_active_fields(src_thing, &(initial_fields[i].data));
//        }
//        post_init(this, data);
//    }
    
    init_script_instance_instructions(this);
    
    script->ref_count += 1;
    
    this->creation_time = mtt_time_nanoseconds();//mtt_core_ctx()->time_nanoseconds;
    
    return this;
}
Script_Instance* Script_Instance::deinit(void)
{
    Script* source_s = this->source_script;
    
    auto& things = source_s->connections.sorted_things_direct;
    
    for (usize i = 0; i < things.size(); i += 1) {
        mtt::Thing* src_thing = things[i];
        mtt::World* world = mtt::world(src_thing);
        
        set_active_fields_to_default(src_thing);
        
        for (usize f = 0; f < thing_local_fields[i].size(); f += 1) {
            destroy_fields(*field_allocator(world), src_thing->field_descriptor, thing_local_fields[i][f]);
        }
        destroy_fields(*field_allocator(world), src_thing->field_descriptor, this->thing_initial_fields[i].data);
    }
    
    this->status = SCRIPT_STATUS_TERMINATED;
    
    {
        Script* parent = source_s;
        if (parent != nullptr) {
            parent->ref_count -= 1;
            if (parent->destroy_upon_ref_count_0 && parent->ref_count == 0) {
                auto* prev_graph = mtt::curr_graph(mtt::ctx());
                mtt::set_graph(mtt::ctx(), &parent->connections);
                for (isize i = parent->connections.sorted_things_direct.size() - 1; i >= 0; i -= 1) {
                    mtt::Thing* thing = parent->connections.sorted_things_direct[i];
                    
                    if (thing->graph != mtt::root_graph(mtt::world(thing))) {
                        thing->graph = nullptr;
                    }
                    
                    mtt::Thing_destroy(thing);
                }
                mtt::set_graph(mtt::ctx(), prev_graph);
                Script::scripts.erase(parent->id);
            }
        }
        
        
    }
    return this;
}

void Script_Instance_start_or_resume(Script_Instance* s)
{
    assert(s->status != SCRIPT_STATUS_STOPPED);
    s->status = SCRIPT_STATUS_STARTED;
    Script* source_s = s->source_script;
    
}
void Script_Instance_suspend(Script_Instance* s)
{
    s->status = SCRIPT_STATUS_STARTED;
    Script* source_s = s->source_script;
    auto& things = source_s->connections.sorted_things_direct;
    
//    for (usize i = 0; i < things.size(); i += 1) {
//        mtt::Thing* src_thing = things[i];
//        mtt::World* world = mtt::world(src_thing);
////        mtt::init_field_list_from_source(
////                                         world,
////                                         field_allocator(world),
////                                         s->thing_local_fields[i],
////                                         &src_thing->field_descriptor,
////                                         &src_thing->field_descriptor.data);
//    }
}
void Script_Instance_cancel(Script_Instance* s)
{
    
    Script* source_s = s->source_script;
    auto& things = source_s->connections.sorted_things_direct;
    if (s->source_script->on_cancel) {
        s->source_script->on_cancel(mtt::ctx(), s, NULL);
    }
    
    s->status = SCRIPT_STATUS_DONE;
}
void Script_Instance_should_terminate(Script_Instance* s)
{
    s->status = SCRIPT_STATUS_DONE_SHOULD_TERMINATE;
}

void Script_Instance_terminate(Script_Instance* s)
{
    s->status = SCRIPT_STATUS_TERMINATED;
    Script* source_s = s->source_script;
    auto& things = source_s->connections.sorted_things_direct;
    
    if (s->on_terminate != nullptr) {
        s->on_terminate(mtt::ctx(), s, s->args);
    }
    
    return;
    
    for (usize i = 0; i < things.size(); i += 1) {
        mtt::Thing* src_thing = things[i];
        mtt::World* world = mtt::world(src_thing);
//        mtt::init_field_list_from_source(
//                                         world,
//                                         field_allocator(world),
//                                         s->thing_local_fields[i],
//                                         &src_thing->field_descriptor,
//                                         &src_thing->field_descriptor.data);
    }
}

void Script_Instance_stop(Script_Instance* s)
{
    s->status = SCRIPT_STATUS_STOPPED;
    s->remove_actions(s->agent);
//
//    Script* source_s = s->source_script;
//    auto& things = source_s->connections.sorted_things_direct;
//
//    if (s->on_terminate != nullptr) {
//        s->on_terminate(mtt::ctx(), s, (void*)nullptr);
//    }
//
//    for (usize i = 0; i < things.size(); i += 1) {
//        mtt::Thing* src_thing = things[i];
//        mtt::World* world = mtt::world(src_thing);
////        mtt::init_field_list_from_source(
////                                         world,
////                                         field_allocator(world),
////                                         s->thing_local_fields[i],
////                                         &src_thing->field_descriptor,
////                                         &src_thing->field_descriptor.data);
//    }
}

void terminate_rule_if_dependent_on_thing(mtt::Thing_ID thing_id)
{
    auto* RT = Runtime::ctx();
    auto* map = &RT->Thing_ID_to_Rule_Script;
    //
    auto find_it = map->find(thing_id);
    
    if (find_it != map->end()) {
        auto &r_set = find_it->second;
        for (auto r_it = r_set.begin(); r_it != r_set.end(); ++r_it) {
            auto s_id = (Script_ID)*r_it;
            
            Script_Instance* s = nullptr;
            auto f_script = RT->id_to_rule_script.find(s_id);
            if (f_script == RT->id_to_rule_script.end()) {
                map->erase(find_it);
                return;
            } else {
                s = (Script_Instance*)f_script->second;
            }
            RT->id_to_rule_script.erase(f_script);
            
            
            s->rules_are_valid = false;
            assert(s->is_rule());
            //assert(!s->rules.triggers.rules.empty());
            
            auto& rules = s->rules.triggers.rules;
            for (usize i = 0; i < rules.size(); i += 1) {
                Query_Rule_destroy(&rules[i]);
            }
            rules.clear();
            
            Script_Instance_terminate(s);
        }
        map->erase(find_it);
    }
    
}


Script_Evaluation_Context* Script_Evaluation_Context::make(void)
{
    return mem::allocate<Script_Evaluation_Context>(mtt::buckets_allocator());
}
void Script_Evaluation_Context::finish(Script_Evaluation_Context* ctx)
{
    // TODO:
}
void Script_Evaluation_Context::destroy(Script_Evaluation_Context** ctx)
{
    mem::deallocate<Script_Evaluation_Context>(mtt::buckets_allocator(), *ctx);
    *ctx = nullptr;
}



void sort_things(World* world, std::vector<mtt::Thing*>& things, Eval_Connection_Graph& G)
{
    ASSERT_MSG(&G == mtt::curr_graph(world), "Incorrect connection graph!");
    
    G.sorted_things_direct.clear();
    G.sorted_things_direct.reserve(things.size());
    
    sort_things_by_execution(world, G);
    
    for (auto it = things.begin(); it != things.end(); ++it) {
        Thing* thing = *it;
        // guard
        if (thing->is_visited) {
            continue;
        }
        thing->is_visited = true;
        
        sort_things_internal(world, G, thing);
    }
    
    std::stable_sort(G.sorted_things_direct.begin(), G.sorted_things_direct.end(), Thing_compare_by_eval_priority());
    
    for (usize i = 0; i < G.sorted_things_direct.size(); i += 1) {
        G.sorted_things_direct[i]->eval_index = i;
    }
    //    for (usize i = 1; i < G.sorted_things_count; i += 1) {
    //        MTT_print("%llu\n", G.sorted_things_direct[i]->eval_priority);
    //        assert(G.sorted_things_direct[i]->eval_priority >= G.sorted_things_direct[i]->eval_priority);
    //    }
    
    Things_mark_unvisited(world, things);
    
    G.output.list.resize(root_graph(world)->sorted_things_direct.size());
}

void process_script(World* world, Script_Instance* script/*Thing_Storage& things,*/)
{
    Eval_Connection_Graph& G = script->source_script->connections;
    
    //eval_out.offset = 0;
    //eval_out.index  = 0;
    
    //Thing_ID* sorted_things   = G.sorted_things;
    usize sorted_things_count = G.sorted_things_direct.size();
    if (sorted_things_count == 0) {
        return;
    }
    
    Evaluation_Output& eval_out = G.output;
    
    auto* prev_graph = mtt::curr_graph(world);
    mtt::set_graph(world, &G);
    
    Thing** sorted_things = &G.sorted_things_direct[0];
    usize port_index = 0;
    
    auto& thing_local_fields = script->thing_local_fields;

    
    
    
//    for (usize thing_idx = 0; thing_idx < sorted_things_count; thing_idx += 1) {
////        Thing* const thing = sorted_things[thing_idx];
////        Thing_ID const thing_id = thing->id;
////
////
////        Port_Input_List* input_list = nullptr;
////        map_get(root_graph(world)->incoming, thing_id, &input_list);
////        const bool ok = thing->logic.proc(world, thing, input_list, NULL);
////        if (!ok) {
////            thing->on_end(thing);
////            //thing->on_end = [](mtt::Thing*){};
////        }
//    }

    
    bool is_done = true;
    const usize ctx_count = script->contexts.size();
    if (script->status == SCRIPT_STATUS_NOT_STARTED) {
        script->status = SCRIPT_STATUS_STARTED;
        
        if (script->agent != mtt::Thing_ID_INVALID) {
            auto& label = script->source_script->label;
            auto* rtime = Runtime::ctx();
            auto& task_list = rtime->script_tasks.list;
            //MTT_print("TASK COUNT: %lu\n", task_list.size());
            
            for (isize i = task_list.size() - 1; i >= 0; i -= 1) {
                auto& tsk = task_list[i];
                if (tsk != script && tsk->status != SCRIPT_STATUS_CANCELED && !tsk->allow_duplicates_for_agent && tsk->label == label && tsk->agent == script->agent) {
                    //Script_Instance_should_terminate(tsk);
                    Script_Instance_stop(tsk);
                }
            }
        }
            
        for (usize ctx_idx = 0; ctx_idx < ctx_count; ctx_idx += 1) {
            auto* const script_ctx = &script->contexts[ctx_idx];
            script_ctx->is_done = false;
            script_ctx->slot_idx = ctx_idx;
        }
        
        
        if (script->on_start != nullptr) {
            for (usize i = 0; i < sorted_things_count; i += 1) {
                mtt::Thing* thing = sorted_things[i];
                //mtt::set_active_fields(thing, &thing_local_fields[thing->eval_index][0]);
            }

            auto res = script->on_start(world, script, script->args);
            auto [result_type, result_continuation, result_value] = res;
        }
//        for (usize thing_idx = 0; thing_idx < sorted_things_count; thing_idx += 1) {
//
//            //eval_out.index = thing_idx;
//
//            Thing* const thing = sorted_things[thing_idx];
//            Thing_ID const thing_id = thing->id;
//
//            {
//                //eval_out.list[thing_idx].port_entries.resize(thing->ports.out_ports.size());
//                eval_out.list[thing_idx].first_port_index = port_index;
//
//                auto* const out_ports = &thing->ports.out_ports;
//                const usize port_count = out_ports->size();
//                usize next_port_index = port_index + port_count;
//                while (eval_out.port_entries().size() < next_port_index) {
//                    eval_out.port_entries().push_back({});
//                }
//                for (usize i = 0; i < port_count; i += 1) {
//                    auto* const entry = (&eval_out.port_entries()[port_index + i]);
//                    entry->ID                 = thing_id;
//                    entry->out.type           = (*out_ports)[i].type;
//                    entry->out.contained_type = (*out_ports)[i].contained_type;
//
//                    entry->is_ignored = false;
//                }
//                port_index += port_count;
//            }
//        }
    }

    if (script->on_begin_frame != nullptr) {
        auto res = script->on_begin_frame(world, script, script->args);
        auto [result_type, result_continuation, result_value] = res;
        switch (result_type) {
            case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE: {
                script->status = SCRIPT_STATUS_DONE_SHOULD_TERMINATE;
                is_done = true;
                goto LABEL_EXIT;
                break;
            }
            default: {
                break;
            }
        }
    }
    
    if (script->status == SCRIPT_STATUS_STOPPED) {
        goto LABEL_EXIT;
    }
    
    for (isize ctx_idx = ctx_count - 1; ctx_idx >= 0; ) {
        script->contexts[ctx_idx].slot_idx = ctx_idx;
        if (script->contexts[ctx_idx].is_done) {
            ctx_idx -= 1;
            continue;
        }
        
        Script_set_current_context(script, ctx_idx);


        auto* ctx_state = &Script_current_context_state(script);
        
        Script_Instance_start_or_resume(script);

        
        for (; ctx_state->instruction_idx < sorted_things_count; ) {
//#ifndef NDEBUG
            if (mtt::should_print_script_eval(world)) {
                MTT_print("instruction index: %llu s-id: %p\n", ctx_state->instruction_idx, script);
                Script_print_as_things(script->source_script, script);
                
            }
//#endif
            Thing* const thing = sorted_things[ctx_state->instruction_idx];
            Thing_ID const thing_id = thing->id;
            
            {
                usize iteration = mtt::get_loop_iteration(script);
                
                mtt::set_active_fields(thing, &thing_local_fields[thing->eval_index][iteration]);
                thing->eval_out = &script->output;
                script->output.set_port_entries_index(iteration);
                
            }
            
            Port_Input_List* input_list = nullptr;
            map_get(curr_graph(world)->incoming, thing_id, &input_list);
            
            auto res = thing->logic.proc(world, thing, input_list, script, &ctx_state, nullptr);
            auto [result_type, result_continuation, result_value] = res;
            ctx_state = &Script_current_context_state(script);
            switch (result_type) {
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ENTER_SCOPE: {
                    ctx_state->instruction_idx += 1;
                    break;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_EXIT_SCOPE: {
                    break;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND: {
                    is_done &= false;
                    script->contexts[ctx_idx].is_done = false;
                    goto LABEL_CONTEXT_EVAL_END_STEP;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP: {
                    is_done &= false;
                    script->contexts[ctx_idx].is_done = false;
                    break;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_IMMEDIATE_SUSPEND: {
                    
                    
                    is_done &= false;
                    script->contexts[ctx_idx].is_done = false;
                    goto LABEL_CONTEXT_EVAL_END_STEP;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_IMMEDIATE_SUSPEND_NO_RESET: {
                    
                    
                    is_done &= false;
                    script->contexts[ctx_idx].is_done = false;
                    goto LABEL_CONTEXT_EVAL_END_STEP;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE: {
                    script->status = SCRIPT_STATUS_DONE;
                    is_done = true;
                    goto LABEL_EXIT;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE: {
                    script->status = SCRIPT_STATUS_DONE_SHOULD_TERMINATE;
                    is_done = true;
                    goto LABEL_EXIT;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_WAS_STOPPED: {
                    script->status = SCRIPT_STATUS_DONE;
                    goto LABEL_EXIT;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE: {
                    is_done &= true;
                    script->contexts[ctx_idx].is_done = true;
                    goto LABEL_CONTEXT_EVAL_END_STEP;
                }
                case LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR: {
                    is_done = true;
                    script->status = SCRIPT_STATUS_ERROR;
                    script->contexts[ctx_idx].is_done = true;
                    goto LABEL_CONTEXT_EVAL_END_STEP;
                }
                default: {
                    ctx_state->instruction_idx += 1;
                    break;
                }
            }
            
            
        }
        if (ctx_state->instruction_idx >= sorted_things_count) {
            script->contexts[ctx_idx].is_done = true;
        }
        is_done &= script->contexts[ctx_idx].is_done;
        
        LABEL_CONTEXT_EVAL_END_STEP:;
        
        ctx_idx -= 1;
    }
    
    LABEL_EXIT:;
    
    

    
    if (is_done) {
        if (script->status != SCRIPT_STATUS_DONE_SHOULD_TERMINATE && script->status != SCRIPT_STATUS_ERROR) {
            script->status = SCRIPT_STATUS_DONE;
        }
        
        if (script->on_done != nullptr) {
            auto res = script->on_done(world, script, script->args);
            auto [result_type, result_continuation, result_value] = res;
        }
    }
    
    
    mtt::set_graph(world, prev_graph);
}

void reinit_port_entries(mtt::World* world, Eval_Connection_Graph& connections, usize first_index, usize last_index)
{
    Evaluation_Output& eval_out = connections.output;
    reinit_port_entries(world, connections, eval_out, first_index, last_index);
}


void reinit_port_entries(mtt::World* world, Eval_Connection_Graph& connections, Evaluation_Output& eval_out, usize first_index, usize last_index)
{
    usize sorted_things_count = connections.sorted_things_direct.size();
    if (sorted_things_count == 0) {
        return;
    }
    
    auto* prev_graph = mtt::curr_graph(world);
    mtt::set_graph(world, &connections);
    
    
    Thing** sorted_things = &connections.sorted_things_direct[0];
    usize port_index = 0;
    
    for (usize thing_idx = first_index; thing_idx < last_index; thing_idx += 1) {
        
        //eval_out.index = thing_idx;
        
        Thing* const thing = sorted_things[thing_idx];
        Thing_ID const thing_id = thing->id;
        
        {
            //eval_out.list[thing_idx].port_entries.resize(thing->ports.out_ports.size());
            eval_out.list[thing_idx].first_port_index = port_index;
            
            auto* const out_ports = &thing->ports.out_ports;
            const usize port_count = out_ports->size();
            usize next_port_index = port_index + port_count;
            while (eval_out.port_entries().size() < next_port_index) {
                eval_out.port_entries().push_back({});
            }
            for (usize e = 0; e < eval_out.port_entries_.size(); e += 1) {
                for (usize i = 0; i < port_count; i += 1) {
                    auto* const entry = (&eval_out.port_entries_[e][port_index + i]);
                    entry->ID                 = thing_id;
                    entry->out = {};
                    entry->out.type           = (*out_ports)[i].type;
                    entry->out.contained_type = (*out_ports)[i].contained_type;
                    
                    entry->is_ignored = false;
                    
                }
            }
            port_index += port_count;
        }
    }
    
    mtt::set_graph(world, prev_graph);
}
void init_port_entries(mtt::World* world, Eval_Connection_Graph& connections)
{
    Evaluation_Output& eval_out = connections.output;
    
    usize sorted_things_count = connections.sorted_things_direct.size();
    if (sorted_things_count == 0) {
        return;
    }
    
    auto* prev_graph = mtt::curr_graph(world);
    mtt::set_graph(world, &connections);
    
    
    Thing** sorted_things = &connections.sorted_things_direct[0];
    usize port_index = 0;
    mtt::World_Graph_init(&connections, &world->allocator, connections.sorted_things_direct.size());
    for (usize thing_idx = 0; thing_idx < sorted_things_count; thing_idx += 1) {
        
        //eval_out.index = thing_idx;
        
        Thing* const thing = sorted_things[thing_idx];
        Thing_ID const thing_id = thing->id;
        
        {
            //eval_out.list[thing_idx].port_entries.resize(thing->ports.out_ports.size());
            eval_out.list[thing_idx].first_port_index = port_index;
            
            auto* const out_ports = &thing->ports.out_ports;
            const usize port_count = out_ports->size();
            usize next_port_index = port_index + port_count;
            while (eval_out.port_entries().size() < next_port_index) {
                eval_out.port_entries().push_back({});
            }
            for (usize i = 0; i < port_count; i += 1) {
                auto* const entry = (&eval_out.port_entries()[port_index + i]);
                entry->ID                 = thing_id;
                entry->out = {};
                entry->out.type           = (*out_ports)[i].type;
                entry->out.contained_type = (*out_ports)[i].contained_type;
                
                entry->is_ignored = false;
            }
            port_index += port_count;
        }
    }
    
    mtt::set_graph(world, prev_graph);
}


Script* Script_make(mtt::World* world)
{
    Script* new_script = &Script::scripts[Script::next_avail_ID];
    new_script->id = Script::next_avail_ID;
    Script::next_avail_ID += 1;

    return new_script;
}

Script* Script_lookup(Script_ID id)
{
    Script* out = nullptr;
    mtt::map_try_get(&Script::scripts, id, &out);
    return out;
}

MTT_NODISCARD
Script_Builder Script_Builder_make(mtt::World* world)
{
    return (Script_Builder) {
        .script = Script_make(world),
        .world = world,
    };
}

Parallel_Group_Begin_Out begin_parallel_group(Script_Builder* scb, const mtt::String& label)
{
    //scb->script->connections.
    auto* t = mtt::Thing_make(scb->world, mtt::ARCHETYPE_GROUP_BLOCK_BEGIN);
    mtt::Thing_set_label(t, label);
    auto* state = mtt::access<Group_Block_Begin>(t, "state");
    if (scb->depth == 0) {
        scb->roots.push_back(t);
        state->is_root = true;
    }
    scb->depth += 1;
    scb->things.push_back(t);
    scb->group_stack.push_back(t);
    return {t, state};
}

Parallel_Group_End_Out end_parallel_group(Script_Builder* scb, const End_Parallel_Group_Args& args, const mtt::String& label)
{
    auto* t = mtt::Thing_make(scb->world, mtt::ARCHETYPE_GROUP_BLOCK_END);
    mtt::Thing_set_label(t, label);
    ASSERT_MSG(scb->depth > 0, "Mismatched depth");
    scb->depth -= 1;
    auto* matching_begin = scb->group_stack.back();
    scb->things.push_back(t);
    
    auto* state = mtt::access<Group_Block_End>(t, "state");
    ASSERT_MSG(state != nullptr, "state should exist");
    
    *state = {};
    state->count = args.count;
    //state->time_interval = args.time_interval;
    state->time_remaining = state->time_interval;
    state->has_count = args.has_count;
    //state->has_time_interval = args.has_time_interval;
    state->reset_upon_jump = args.reset_on_looparound;
    state->matching_begin = matching_begin->id;
    
    ASSERT_MSG(matching_begin->id != mtt::Thing_ID_INVALID, "matching begin id is invalid!");
    
    auto* begin_state = mtt::access<Group_Block_Begin>(matching_begin, "state");
    ASSERT_MSG(begin_state != nullptr, "state should exist");
    begin_state->matching_end = t->id;
    state->is_matched_with_root = begin_state->is_root;
    
    scb->group_stack.pop_back();
    return {t, state};
}

//mtt::Thing* begin_sequence_group(Script_Builder* scb, const mtt::String& label)
//{
//    auto* t = mtt::Thing_make(scb->world, mtt::ARCHETYPE_SEQUENCE_GROUP_BLOCK_BEGIN);
//    mtt::Thing_set_label(t, label);
//    scb->things.push_back(t);
//    return t;
//}
//
//mtt::Thing* end_sequence_group(Script_Builder* scb, const mtt::String& label)
//{
//    auto* t = mtt::Thing_make(scb->world, mtt::ARCHETYPE_SEQUENCE_GROUP_BLOCK_END);
//    mtt::Thing_set_label(t, label);
//    scb->things.push_back(t);
//    return t;
//}
mtt::Thing* wait_for_all_scheduled_ended(Script_Builder* scb)
{
    using namespace dt;
    DT_print("[WAIT]\n");
    mtt::Thing* wait = mtt::Thing_make(scb->world, mtt::ARCHETYPE_WAIT);
    auto* state = mtt::access<mtt::Wait_Block>(wait, "state");
    state->wait_condition = mtt::WAIT_CONDITION_ALL_SCHEDULED_ENDED;
    scb->things.push_back(wait);
    return wait;
}

mtt::Thing* thing_make(Script_Builder* scb, mtt::Thing_Archetype_ID type, const mtt::String& label)
{
    mtt::Thing* t = mtt::Thing_make(scb->world, type);
    mtt::Thing_set_label(t, label);
    scb->things.push_back(t);
    
    return t;
}
mtt::Thing* thing(Script_Builder* scb, mtt::Thing* t)
{
    scb->things.push_back(t);
    
    return t;
}


Rule_Eval_State* rule_result_eval(Script_Builder* scb, const mtt::String& label)
{
    auto* t = mtt::Thing_make(scb->world, mtt::ARCHETYPE_RULE_EVAL);
    mtt::Thing_set_label(t, label);
    auto* state = mtt::access<mtt::Rule_Eval_State>(t, "state");
    scb->things.push_back(t);
    return state;
}



mtt::Thing* lookup_get(Script_Builder* scb, const mtt::String& key, const mtt::String& scope_key)
{
    mtt::Thing* get = mtt::thing_make(scb, mtt::ARCHETYPE_GET_PROPERTY);
    mtt::access_if_exists<Property_Access>(get, "state", [&](Property_Access* pa) {
        pa->key = key;
        pa->scope = scope_key;
    });
    
    return get;
}

//mtt::Thing* lookup_get_things(Script_Builder* scb, const mtt::String& key, const mtt::String& scope_key)
//{
//    return lookup_get(scb, key, scope_key);
//}

mtt::Thing* thing(Script_Builder* scb, mtt::Thing* t, const mtt::String& label)
{
    if (label != "") {
        mtt::Thing_set_label(t, label);
    }
    scb->things.push_back(t);
    
    return t;
}




Script* Script_Builder_build(Script_Builder* scb)
{
    if (scb->things.empty() || !scb->is_valid) {
        Script::scripts.erase(scb->script->id);
        return nullptr;
    }
    
    for (usize i = 1; i < scb->things.size(); i += 1) {
        mtt::Thing* thing_i_prev = scb->things[i - 1];
        mtt::Thing* thing_i =  scb->things[i];
        if (thing_i_prev->graph == thing_i->graph) {
            mtt::add_execution_connection(thing_i_prev, thing_i);
        }
    }
    
    sort_things(scb->world, scb->things, scb->script->connections);
    
    
    scb->script->contexts.reserve(scb->roots.size());
    
    for (auto* root : scb->roots) {
        scb->script->contexts.push_back((Script_Contexts){});
        Script_Context ctx = {};
        ctx.first_idx = root->eval_index;
        ctx.instruction_idx = ctx.first_idx;
        scb->script->contexts.back().ctx_stack.push_back(ctx);
    }
    
    init_port_entries(scb->world, scb->script->connections);
    
    return scb->script;
}

Script_Instance* Script_Instance_from_script(Script* script, void (*post_init)(Script_Instance* s, void* data), void* data)
{
    auto* alloc = Script_Instance::make();
    return alloc->init(script, post_init, data);
}
Script_Instance* Script_Instance_call_from_script(Script* script, Script_Instance* caller, void* caller_info, void (*post_init)(Script_Instance* s, void* data), void* data)
{
    auto* script_instance = Script_Instance_from_script(script, post_init, data);
    script_instance->priority = caller->priority + 1;
    script_instance->parent = caller;
    if (caller != nullptr && caller->source_script->share_lookup_with_sub_scripts) {
        script_instance->set_shared_lookup(&caller->lookup());
    }
    
    script_instance->caller = ((Call_Descriptor*)caller_info)->call_instruction;
    
    return script_instance;
}

void Script_Instance_destroy(Script_Instance* s)
{
    Script_Instance::destroy(s->deinit());
}

void Script_print_as_things(Script* script, Script_Instance* instance)
{
    using namespace dt;
    mtt::String indent = "";
    
    usize instruction_idx = ~0;
    if (instance != nullptr) {
        instruction_idx = Script_current_context_state(instance).instruction_idx;
    }
    
    for (usize idx = 0; const auto& op : script->connections.sorted_things_direct) {
        switch (op->archetype_id) {
            case mtt::ARCHETYPE_GROUP_BLOCK_BEGIN: {
                DT_scope_open();
                if (idx == instruction_idx) {
                    MTT_print("%s", "=>\n");
                }
                MTT_print("%llu: %s", idx, indent.c_str());
                
                indent += "    ";
                mtt::Thing_print(op);
                
                break;
            }
            case mtt::ARCHETYPE_GROUP_BLOCK_END: {
                indent = indent.substr(0, indent.size() - 4);
                if (idx == instruction_idx) {
                    MTT_print("%s", "=>\n");
                }
                MTT_print("%llu: %s", idx, indent.c_str());
                mtt::Thing_print(op);
                DT_scope_close();
                
                break;
            }
            case mtt::ARCHETYPE_GET_PROPERTY: {
//                if (idx == instruction_idx) {
//                    MTT_print("%s", "=>\n");
//                }
//                MTT_print("%llu: %s", idx, indent.c_str());
//                mtt::Thing_print(op);
//                MTT_print("%s%s%s<", indent.c_str(), indent.c_str(), indent.c_str());
//                Property_Access* access = mtt::access<mtt::Property_Access>(op, "state");
//                access->print(indent + indent + indent, instance);
//                MTT_print("%s%s%s>\n", indent.c_str(), indent.c_str(), indent.c_str());
                break;
            }
            default: {
                if (idx == instruction_idx) {
                    MTT_print("%s", "=>\n");
                }
                MTT_print("%llu: %s", idx, indent.c_str());
                mtt::Thing_print(op);
                break;
            }
        }
        idx += 1;
    }
}


Logic_Procedure_Return_Status jump_to_beginning(Script_Instance* s, Script_Context** s_ctx, usize eval_index, bool reset)
{
    usize begin_idx = (*s_ctx)->first_idx;
    usize end_index = eval_index;
    (*s_ctx)->instruction_idx = begin_idx + 1;
    if (reset && (begin_idx + 1 < end_index)) {
        init_script_instance_instructions_range(s, (*s_ctx)->instruction_idx, end_index - 1);
        reinit_port_entries(mtt::ctx(), s->source_script->connections, s->output, (*s_ctx)->instruction_idx, end_index - 1);
    }
    if (reset) {
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_IMMEDIATE_SUSPEND;
    } else {
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_IMMEDIATE_SUSPEND_NO_RESET;
    }
}


Script_Lookup merge_lookups_into_from(const Script_Lookup& into, const Script_Lookup& from)
{
    Script_Lookup out = into;
    if (from.empty()) {
        return out;
    }
    
    for (auto& [key_from, value_from] : from) {
        out[key_from] = value_from;
    }
    
    return out;
};


bool should_print_script_eval(mtt::World* world)
{
    return world->show_script_eval_print;
}

void toggle_should_print_script_eval(mtt::World* world)
{
    world->show_script_eval_print = !world->show_script_eval_print;
}

uint64 Script_Instance_root_creation_time(Script_Instance* s)
{
    while (s->parent != nullptr) {
        s = s->parent;
    }
    return s->creation_time;
}

void Script_Instance_append_return_value(Script_Instance* s, const mtt::Any& value)
{
    s->return_value.push_back(value);
}

}
