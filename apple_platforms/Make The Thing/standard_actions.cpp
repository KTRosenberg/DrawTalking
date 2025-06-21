//
//  standard_actions.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/17/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#include "standard_actions.hpp"
#include "drawtalk_world.hpp"

#include "thing_lib.hpp"

namespace dt {

using namespace mtt;


using Def     = DT_Behavior;
using Def_Ret = Def::Procedure_Return;

using Prop             = Speech_Property;
using Prop_List        = Speech_Property::Prop_List;
using Active_Prop_List = Speech_Property::Active_Prop_List;

using Script = mtt::Script;
using Script_Instance = mtt::Script_Instance;
using Script_Builder = mtt::Script_Builder;




void init_call_param_with_instances_or_query(Call_Param& param, Call_Param::TYPE type, usize count=1);
void init_call_param_with_instances_or_query(Call_Param& param, Call_Param::TYPE type, usize count)
{
    
}


auto update_values_with_multipliers = [](Script_Lookup& lookup, Multipliers& multipliers) {
    Script_Lookup_update_var_for_each(lookup, "height", DEFAULT_LOOKUP_SCOPE, [&](Script_Property& p) {
        p.value.Float *= multipliers.height;
    });
    Script_Lookup_update_var_for_each(lookup, "speed", DEFAULT_LOOKUP_SCOPE, [&](Script_Property& p) {
        MTT_BP();
        p.value.Float *= multipliers.speed;
        MTT_BP();
    });
};
auto set_self = [](Script_Instance* script_instance, usize src_idx, Call_Descriptor* call) {
    // initialize the "self" entity by removing all others
    auto* plist = Script_Lookup_get_var_with_key(script_instance, ARG_source, DEFAULT_LOOKUP_SCOPE);
    std::swap((*plist)[src_idx], (*plist)[0]);
    plist->resize(1);
    script_instance->agent = (*plist)[0].value.thing_id;
    auto& self_plist = call->params().param_list(0);
    auto& self_params_lookup = self_plist.lookup;
    auto* other_plist = Script_Lookup_get_var_with_key(self_params_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
    
//                                if (other_plist != nullptr) {
//                                    for (auto& val : *other_plist) {
//                                        if (val.value.thing_id == script_instance->agent) {
//                                            (*other_plist)[0] = val;
//                                            (*other_plist).resize(1);
//                                            break;
//                                        }
//                                    }
//                                }
    for (auto& self_p : self_plist.params) {
        if (self_p.property_lookup_mapping.target_key == ARG_source) {
            for (auto& val : self_p.prop_list) {
                if (val.value.thing_id == script_instance->agent) {
                    self_p.prop_list[0] = val;
                    self_p.prop_list.resize(1);
                    //mtt::Any_print(val.value);
                    MTT_BP();
                    break;
                }
            }
        }
    }
};

void activate_actions(Script_Instance* script, mtt::Thing_ID id)
{
    auto& label = script->label;
    script->action(label, id);
    
    auto* objects = Script_Lookup_get_var_group(script->lookup(), mtt::ARG_object);
    auto* targets  = Script_Lookup_get_var_group(script->lookup(), mtt::ARG_target);
    
    
    //mtt::Script_Lookup_print(script->lookup());
    
    if (objects != nullptr) {
        for (auto& [key, val] : *objects) {
            if (key == DEFAULT_LOOKUP_SCOPE) {
                continue;
            }
            for (auto& entry : val) {
                if (entry.value.type == MTT_THING) {
                    mtt::String full_label = label + key;
                    script->action(full_label, id, entry.value.thing_id);
                }
            }
        }
    }
    if (targets != nullptr) {
        for (auto& [key, val] : *targets) {
            for (auto& entry : val) {
                if (entry.value.type == MTT_THING) {
                    script->action(label, id, entry.value.thing_id);
                }
            }
        }
    }
}

auto on_invoke_default = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
    
    
    
    
    
    // instantiate a script based on the final arguments passed-in
    
    
    if (call->source_script_id == Script_ID_INVALID) {
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
    }
   
    Script* script_to_call = Script_lookup(call->source_script_id);
    if (script_to_call == nullptr) {
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
    }
    

    
    
    auto& plist = call->params().param_list(0);
    auto& params_lookup = plist.lookup;
    auto& shared_lookup = script_to_call->lookup();
    
    

    
//                    MTT_print("params");
//                    Script_Lookup_print(params_lookup);
//                    MTT_print("shared");
//                    Script_Lookup_print(shared_lookup);
//                    MTT_BP();
//                    MTT_print("final_merged");
    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
//                    Script_Lookup_print(final_lookup);
//                    MTT_BP();
    
    for (const auto& modifier : call->modifiers) {
        mtt::Script_Lookup_set_var_with_key(final_lookup, ARG_any, modifier.label, {
            Script_Property_make(mtt::Any::from_Boolean(modifier.is_negated)),
            Script_Property_make(mtt::Any::from_Boolean(!modifier.is_negated))
        });
    }
//            Script_Instance* script_instance = Script_Instance_call_from_script(callee_script, s);
//            Script_Instance* script_instance2 = Script_Instance_call_from_script(callee_script, s);
    
    auto& Q = Script_current_queue(caller_script);
    
    auto call_copy = *call;
    
    auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
    if (src_entries != nullptr) {
        for (usize src_idx = 0; auto& src_entry : *src_entries) {
            MTT_UNUSED(src_entry);
            
            Q.push_back({
                .list = {},
                .call_info = call_copy,
            });
            call = &Q.back().call_info;

            
            
            auto& preconditions = Script_lookup(call->source_script_id)->get_preconditions();
            for (usize i = 0; i < preconditions.size(); i += 1) {
                auto preid = preconditions[i].id;
                Script* script_to_call_precondition = Script_lookup(preid);
                if (script_to_call_precondition == nullptr) {
                    continue;
                }
                // TODO: compare our arguments with what is in the lookup-table
                auto* script_instance = Script_Operation_List_enqueue_script(Q.back(), script_to_call_precondition, caller_script, (void*)call);
                
                Script_Lookup precond_final_lookup = merge_lookups_into_from(script_instance->lookup(), final_lookup);
                script_instance->set_lookup_copy(&precond_final_lookup);

                
                update_values_with_multipliers(precond_final_lookup, call->multipliers);
                set_self(script_instance, src_idx, call);
                
                script_instance->lookup_initial = precond_final_lookup;
            }
            
                
                //Script_Lookup_print(final_lookup);
                //MTT_BP();
                
                auto* script_instance = Script_Operation_List_enqueue_script(Q.back(), script_to_call, caller_script, (void*)call);
                script_instance->label = call->label;
                
                
                script_instance->set_lookup_copy(&final_lookup);

                update_values_with_multipliers(script_instance->lookup(), call->multipliers);
                set_self(script_instance, src_idx, call);
                script_instance->lookup_initial = script_instance->lookup();

            
            //MTT_print("to_call");
            //Script_Lookup_print(script_instance->lookup());
            //MTT_BP();
            //Script_print_as_things(script_to_call);
            //MTT_BP();
            
            
            
            if (on_new_script != nullptr) {
                on_new_script(Q.back().list.front().script, args);
            }
            
            mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, Q.back().list.front().script);
            
            
            src_idx += 1;
        }
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
};


#define DEFAULT_ATTRIBUTES \
{dt::attribute_add("fierce"),   2.5},  \
{dt::attribute_add("energetic"),  2},  \
{dt::attribute_add("hyper"),      2},  \
{dt::attribute_add("swift"),    2.0},  \
{dt::attribute_add("spry"),     1.9},  \
{dt::attribute_add("excited"),  1.9},  \
{dt::attribute_add("speedy"),   1.5},  \
{dt::attribute_add("quick"),    1.5},  \
{dt::attribute_add("fast"),     1.5},  \
{dt::attribute_add("happy"),    1.5},  \
{dt::attribute_add("slow"),     0.5},  \
{dt::attribute_add("sluggish"), 0.5},  \
{dt::attribute_add("unhappy"),  0.5},  \
{dt::attribute_add("sad"),      0.5},  \
{dt::attribute_add("lethargic"),0.1},  \
{dt::attribute_add("tired"),    0.1},  \
{dt::attribute_add("labored"),  0.05}, \
{dt::attribute_add("strained"), 0.05}, \
{dt::attribute_add("exhausted"),0.05}, \
{dt::attribute_add("motionless"),0.0}, \
{dt::attribute_add("immobile"),  0.0}, \




void count_AND_specificity_constraint(Speech_Property* prop, usize* count, bool* specificity)
{
    auto* pspecific = prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC");
    bool specific_value = true;
    if (pspecific != nullptr) {
        specific_value = pspecific->value.flag;
    }
    
    auto* pcount = prop->try_get_only_prop("COUNT");
    usize count_value = 1;
    if (pcount != nullptr) {
        if (specific_value) {
            count_value = pcount->value.numeric;
        } else {
            if (pcount->value.numeric == 1) {
                auto* is_plural = prop->try_get_only_prop("PLURAL");
                if (is_plural != nullptr) {
                    if (is_plural->value.flag == false) {
                        count_value = 1;
                    } else {
                        count_value = ULLONG_MAX;
                    }
                } else {
                    count_value = 1;
                }
            } else {
                count_value = pcount->value.numeric;
            }
        }
    }
    
    
    *count       = count_value;
    *specificity = specific_value;
}


template <typename T>
void init_multipliers_and_args(T& property_list, Multipliers& multipliers, Call_Param_List& params)
{
    for (auto& prop : property_list) {
        
        float32 secondary_multiplier = 1.0f;
#define DT_SECONDARY_MULTIPLIER_W_POSITIVE_CORRECTION(secondary_multiplier, v)  ( (v >= 1.0) ? secondary_multiplier : (1.0 / ( (secondary_multiplier != 0) ? secondary_multiplier : 1.0 )))
        
        auto mod_list = prop->try_get_prop("PROPERTY");
        if (mod_list != nullptr) {
            for (auto& mod : *mod_list) {
                
                if (mod->value.kind_string == "TEXT") {
                    float32 val = 1.0f;
                    
                    if (magnitude_values_lookup(mod->value.text, &val)) {
                        secondary_multiplier *= val;
                    }
                }
            }
        }

        
        {
            float32 v = 1.0f;
            if (magnitude_values_lookup(prop->value.text, &v)) {
                if (!multipliers.has_intensity) {
                    multipliers.has_intensity = true;
                    multipliers.intensity = v;
                } else {
                    multipliers.intensity *= v;
                }
                auto old = multipliers.intensity;
                multipliers.intensity *= DT_SECONDARY_MULTIPLIER_W_POSITIVE_CORRECTION(secondary_multiplier, v);
                if (isnan(multipliers.intensity)) {
                    multipliers.intensity = old;
                }
            }
            
        }
        {
            float32 v = 1.0f;
            if (speed_values_lookup(prop->value.text, &v)) {
                if (!multipliers.has_speed) {
                    multipliers.has_speed = true;
                    multipliers.speed = v;
                } else {
                    multipliers.speed *= v;
                }
                auto old = multipliers.speed;
                multipliers.speed *= DT_SECONDARY_MULTIPLIER_W_POSITIVE_CORRECTION(secondary_multiplier, v);
                if (isnan(multipliers.speed)) {
                    multipliers.speed = old;
                }
            }
            
        }
        
        {
            float32 v = 1.0f;
            if (height_values_lookup(prop->value.text, &v)) {
                if (!multipliers.has_height) {
                    multipliers.has_height = true;
                    multipliers.height = v;
                } else {
                    multipliers.height *= v;
                }
                auto old = multipliers.height;
                multipliers.height *= DT_SECONDARY_MULTIPLIER_W_POSITIVE_CORRECTION(secondary_multiplier, v);
                if (isnan(multipliers.height)) {
                    multipliers.height = old;
                }
            }
            
        }
        
        {
            mtt::String dir = {};
            if (lookup_directional(prop->value.text, &dir)) {
                auto& param = params.params.emplace_back();
                param.type = Call_Param::TYPE::SELECTOR;
                param.selector_name  = "ARG_directional";
                param.selector_value = dir;
            }
        }
    }
#undef DT_SECONDARY_MULTIPLIER_W_POSITIVE_CORRECTION
}

struct Script_Property_W_Speech {
    Script_Property script_property;
    Speech_Property* speech;
    uint64 flags = 0;
};
using Script_Property_W_Speech_List = std::vector<Script_Property_W_Speech>;

struct Prep {
    mtt::String prep;
    Speech_Property* prop;
    Prop_List instance_props;
    Script_Property_W_Speech_List instances;
    Script_Property_W_Speech_List types;
    Prop_List type_props;
};

void fill_with_properties(Script_Property_W_Speech* el) {
    auto* prop = el->speech;
    auto* script_property = &el->script_property;
    // handle attributes
    Prop_List*  property_prop = nullptr;
    auto* dict = dt::word_dict();
    if (prop->try_get_prop("PROPERTY", &property_prop)) {
        for (auto it_prop = property_prop->begin(); it_prop != property_prop->end(); ++it_prop) {
            auto* property = *it_prop;
            if (property->label != "trait" ||  dict->is_ignored_attribute(property->value.text)) {
                continue;
            }
            
            script_property->modifiers.push_back(property->value.text);
        }
    }
    
    Speech_Property* count_prop = nullptr;
    if (prop->refers_to == nullptr) {
        if (prop->try_get_only_prop("COUNT", &count_prop)) {
            script_property->counter = count_prop->value.numeric;
        }
    }
}

struct Common_State {
    bool has_agents         = false;
    bool has_direct_objects = false;
    bool has_prepositions   = false;
    bool has_properties     = false;
    
    Prop_List agent_list = {}, direct_object_list = {}, property_list = {};
    
    
    mtt::Map<mtt::String, Prep> prepositions = {};
    
    Script_Property_W_Speech_List agent_instance_list = {}, direct_object_instance_list = {};
    Script_Property_W_Speech_List agent_type_list = {}, direct_object_type_list = {};
    
    Speech_Property& root_prop;
    
    bool is_looping_sequence = false;
    
    void init(Speech_Property& prop)
    {
        {
            auto _ = prop.get_active("AGENT");
            has_agents         = is_valid(_);
            if (has_agents) {
                bool count = 0;
                has_agents = false;
                for (auto p : _) {
                    
                    if (p->kind_str == "THING_INSTANCE") {
                        auto& list = agent_instance_list;
                        for_each_thing_instance_value(*p, [&count, p, &list](mtt::Thing* thing) {
                            count += 1;
                            list.push_back({{.value = mtt::Any::from_Thing_ID(thing->id), .label = p->label}, p});
                            fill_with_properties(&list.back());
                        });
                        if (count > 0) {
                            has_agents = true;
                            agent_list.push_back(p);
                        }
                        
                    } else if (p->kind_str == "THING_TYPE") {
                        has_agents = true;
                        
                        auto& list = agent_type_list;
                        list.push_back({{.value = mtt::Any::from_String(p->label.c_str()), .label = p->label}, p});
                        agent_list.push_back(p);
                        Speech_Property* spec_prop;
                        if (p->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                            list.back().script_property.type_is_random_selection = true;
                            if (spec_prop->label == "all") {
                                list.back().script_property.type_should_choose_all = true;
                            }
                        }
                        fill_with_properties(&list.back());
                    }
                }
            }
        }
        {
            auto _ = prop.get_active("DIRECT_OBJECT");
            has_direct_objects = is_valid(_);
            if (has_direct_objects) {
                bool count = 0;
                has_direct_objects = false;
                for (auto p : _) {
                    
                    if (p->kind_str == "THING_INSTANCE") {
                        auto& list = direct_object_instance_list;
                        for_each_thing_instance_value(*p, [&count, &list, p](mtt::Thing* thing) {
                            count += 1;
                            list.push_back({{.value = mtt::Any::from_Thing_ID(thing->id), .label = p->label}, p});
                            fill_with_properties(&list.back());
                        });
                        if (count > 0) {
                            has_direct_objects = true;
                            direct_object_list.push_back(p);
                        }
                        
                    } else if (p->kind_str == "THING_TYPE") {
                        has_direct_objects = true;
                        auto& list = direct_object_type_list;
                        list.push_back({{.value = mtt::Any::from_String(p->label.c_str()), .label = p->label}, p});
                        direct_object_list.push_back(p);
                        Speech_Property* spec_prop;
                        if (p->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                            list.back().script_property.type_is_random_selection = true;
                            if (spec_prop->label == "all") {
                                list.back().script_property.type_should_choose_all = true;
                            }
                        }
                        fill_with_properties(&list.back());
                    }
                }
            }
        }
        {
            auto _ = prop.get_active("ACTION");
            bool has_action_trait = is_valid(_);
            if (has_action_trait) {
                has_action_trait = false;
                //bool count = 0;
                for (auto p : _) {
                    if (p->kind_str == "ACTION") {
                        has_action_trait = true;
                        auto& list = direct_object_type_list;
                        list.push_back({{.value = mtt::Any::from_String(p->label.c_str()), .label = p->label}, p});
                        direct_object_list.push_back(p);
                        list.back().script_property.type_is_action = true;
                    }
                }
                has_direct_objects = has_direct_objects || has_action_trait;
            }
        }
        {
            auto _ = prop.get_active("PREPOSITION");
            has_prepositions   = is_valid(_);
            if (has_prepositions) {
                bool count = 0;
                has_prepositions = false;
                for (auto p : _) {
                    auto& pinfo = prepositions[p->label];
                    pinfo.prep = p->label;
                    for (auto obj : p->get_active("OBJECT")) {
                        if (obj->kind_str == "THING_INSTANCE") {
                            auto& list = pinfo.instances;
                            for_each_thing_instance_value(*obj, [&count, &list, p](mtt::Thing* thing) {
                                count += 1;
                                list.push_back({{.value = mtt::Any::from_Thing_ID(thing->id), .label = p->label}, p});
                                fill_with_properties(&list.back());
                            });
                            if (count > 0) {
                                has_prepositions = true;
                                pinfo.instance_props.push_back(obj);
                            }
                            
                        } else if (obj->kind_str == "THING_TYPE") {
                            has_prepositions = true;
                            auto& list = pinfo.types;
                            if (obj->refers_to != nullptr) {
//#ifndef NDEBUG
//                                obj->refers_to->print();
//#endif
                            }
                            list.push_back({{.value = mtt::Any::from_String(obj->label.c_str()), .label = p->label}, obj});
                            pinfo.type_props.push_back(obj);
                            Speech_Property* spec_prop;
                            if (p->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                list.back().script_property.type_is_random_selection = true;
                                if (spec_prop->label == "all") {
                                    list.back().script_property.type_should_choose_all = true;
                                }
                            }
                            fill_with_properties(&list.back());
                        } else if (obj->kind_str == "VALUE_TYPE") {
                            has_prepositions = true;
                            auto& list = pinfo.types;
                            list.push_back({{.value = mtt::Any::from_String(obj->label.c_str()), .label = p->label}, obj});
                            pinfo.type_props.push_back(obj);
                            fill_with_properties(&list.back());
                        }
                    }
                }
            }
        }
        {
            auto _ = prop.get_active("PROPERTY");
            has_properties     = is_valid(_);
            for (auto p : _) {
                property_list.push_back(p);
            }
        }
    }
    
    Common_State(Speech_Property& prop) :
    root_prop(prop)
    {
        init(prop);
    }
};

struct Match_Text_W_Label {
    mtt::String label = "";
    mtt::String text = "";
};
bool find_match(auto& prop_list, const auto& to_match, auto& out)
{
    usize count_to_find = to_match.size();
    if (count_to_find == 0) {
        return false;
    }
    
    
    auto it = to_match.begin();
    for (auto property : prop_list) {
        if (it->label == "" || property->label == it->label) {
            if (property->value.kind_string == "TEXT") {
                if (property->value.text == it->text) {
                    insert<Speech_Property*>(out, property);
                    count_to_find -= 1;
                    
                    if (count_to_find == 0) {
                        return true;
                    }
                    ++it;
                }
            }
        }
    }
    return false;
}


void build_query(Common_State& st, Speech_Property* attrib_prop, Call_Param& param, const mtt::String& str)
{
    mtt::String attributes = "";
    auto* property_list = attrib_prop->try_get_prop("PROPERTY");
    if (property_list != nullptr) {
        for (auto* attrib : *property_list) {
            if (attrib->label == "trait") {
                Speech_Property* n_prop = nullptr;
                attrib->try_get_only_prop("NEGATED", &n_prop);
                attribute_add(attrib->value.text);
                
                
                if ((n_prop != nullptr && n_prop->value.flag == true)) {
                    
                    attributes += ", !IsA(" + MTT_Q_VAR_S(str) + ", attribute." + attrib->value.text + ")";
                    continue;
                }
                //auto* attrib = dt::attribute_add("type::" + property->value.text);
                attributes += ", IsA(" + MTT_Q_VAR_S(str) + ", attribute." + attrib->value.text + ")";
            }
        }
    }
    
    param.query_param.query_rule = Query_Rule_make(mtt::ctx(), "!Prefab(" + MTT_Q_VAR_S(str) + "), IsA(" + MTT_Q_VAR_S(str) + "SUB, " + str + "), " + MTT_Q_VAR_S(str) + "SUB(" + MTT_Q_VAR_S(str) + ")" + attributes);
}






Call_Descriptor* call_insert(mtt::Script_Builder* scb, mtt::Gen_Args* gen_args);
Call_Descriptor* call_insert(mtt::Script_Builder* scb, mtt::Gen_Args* gen_args)
{
    mtt::Thing* ins_call = mtt::thing_make(scb, mtt::ARCHETYPE_CALL);
    mtt::Call_Descriptor* call = mtt::access<mtt::Call_Descriptor>(ins_call, "state");
    //MTT_print(">>>%p\n", call);
    call->count = gen_args->count;
    call->count_remaining = gen_args->count;
    call->has_time_interval = gen_args->has_time_interval;
    call->time_interval = gen_args->time_interval;
    call->time_remaining = call->time_interval;
    call->source_script_id = Script_ID_INVALID;
    call->is_active = true;
    return call;
}



// MARK: on generate

struct Default_On_Generate_Args_Default {
    static constexpr const bool is_query_override = false;
    static constexpr const bool is_new_action = false;
    
    static inline constexpr bool type_param_should_use_own_query(Script_Property_W_Speech&, mtt::Script_Builder*, Common_State&, DT_Args&, DT_Args_Ex&, usize*, const mtt::String&, Call_Param&, bool*)
    {
        return true;
    }
};



struct Default_On_Generate_Args_Conditional {
    static constexpr const bool is_query_override = true;
    static constexpr const bool is_new_action = false;
    
    static inline bool type_param_should_use_own_query(
                                                       Script_Property_W_Speech& entry, mtt::Script_Builder* scb, Common_State& st, DT_Args& args, DT_Args_Ex& args2, usize* count_if_should_use_query_instead_of_rule, mtt::String& role, Call_Param& param, bool* do_override_count_if_is_nonspecific)
    {
        constexpr const usize MAX_COUNT = ULLONG_MAX;
        if (entry.script_property.type_is_random_selection) {
            *count_if_should_use_query_instead_of_rule = MAX_COUNT;
            *do_override_count_if_is_nonspecific = true;
            return true;
        } else {
            // check if desired variable is in the table. If not, then a non-constant entry is going
            // to be treated as picking all of a specific type
            
            auto& prop = *args.prop;
            auto ID = prop.ID;
            auto& roles = scb->condition_builders.trigger->per_prop_roles;
            auto find_it = roles.map.find(ID);
            ASSERT_MSG(find_it != roles.map.end(), "%s\n", "???");
#ifndef NDEBUG
            scb->condition_builders.trigger->print_roles_for_prop(&prop);
#endif
            auto& per_prop_map = find_it->second;
            auto find_role_it = per_prop_map.role_to_var.find(role);
            if (find_role_it != per_prop_map.role_to_var.end()) {
                mtt::Map_Stable<mtt::String, rules::Var_Info>& v_info = find_role_it->second;
                
                mtt::Map<mtt::String, mtt::Rule_Var_Record_One_Result>& var_lookup = scb->condition_builders.trigger->var_lookup;
                MTT_print("%s\n", "{");
                param.rule_vars.clear();
                for (const auto& [var_name, var_info] : v_info) {
                    MTT_print("\t{%s\n", var_name.c_str());
                    auto var_record_it = var_lookup.find(var_name);
                    if (var_record_it != var_lookup.end()) {
                        mtt::Rule_Var_Record_One_Result* record = &var_record_it->second;
                        MTT_print("\t\tid=[%d]:idx=[%llu]\n", record->var, record->idx);
                        param.rule_vars.push_back(*record);
                    }
                    MTT_print("%s\n", "\t}");
                }
                MTT_print("%s\n", "}");
            } else {
                *count_if_should_use_query_instead_of_rule = MAX_COUNT;
                *do_override_count_if_is_nonspecific = true;
                return true;
            }
            
            if (param.rule_vars.empty()) {
                *count_if_should_use_query_instead_of_rule = MAX_COUNT;
                *do_override_count_if_is_nonspecific = true;
                return true;
            }
            
            param.type = Call_Param::TYPE::RULE_RESULT;
            return false;
        }
    }
};


struct Default_On_Generate_Args_New_Action {
    static constexpr const bool is_query_override = true;
    static constexpr const bool is_new_action = true;
    
    static inline bool type_param_should_use_own_query(
                                                       Script_Property_W_Speech& entry, mtt::Script_Builder* scb, Common_State& st, DT_Args& args, DT_Args_Ex& args2, usize* count_if_should_use_query_instead_of_rule, mtt::String& role, Call_Param& param, bool* do_override_count_if_is_nonspecific)
    {
//        if (entry.script_property.type_is_random_selection) {
//            return true;
//        } else {
//            return false;
//        }
        return false;
    }
};

template <typename ARGS> auto default_on_generate_inner(Common_State& st, Call_Descriptor** call_out, DT_Args& args, DT_Args_Ex& args2)
{
    auto& def    = *args.behavior;
    auto& prop   = *args.prop;

    auto* scb    = args.script_builder;
       
    if (!args2.supports_no_src && !st.has_agents) {
        return mtt::Result_make(false);
    }
    
//    if (scb->is_building_conditions) {
//#ifndef NDEBUG
//        prop.print();
//#endif
//        auto* conds = &scb->condition_builders;
//
//        return mtt::Result_make(true);
//    }
    if constexpr (ARGS::is_query_override && !ARGS::is_new_action) {
        auto* prop_ptr = &prop;
        auto ID = prop_ptr->ID;
        auto& roles = scb->condition_builders.trigger->per_prop_roles;
        auto find_it = roles.map.find(ID);
        //ASSERT_MSG(find_it != roles.map.end(), "%s\n", "???");
        //scb->condition_builders.trigger->print_roles_for_prop(prop_ptr);
    }
    
    Call_Descriptor* call = call_insert(scb, &args.gen_args);
    *call_out = call;
    call->set_script("", 0);
    //auto& param_lists = call->param_lists;
    call->params().push_param_list();
    auto current_param_list_index = call->params().current_index;
    {
        {
            auto& param_list = call->params().curr_param_list();
            
            prop.call_desc_ref.call_id = call->call_instruction;
            prop.call_desc_ref.param_list_index = current_param_list_index;
            {
                auto& types     = st.agent_type_list;
                auto& instances = st.agent_instance_list;
                
                
                {
                    if (!instances.empty()) {
                        Call_Param param = {};
                        
                        param.type = Call_Param::TYPE::FIXED_THINGS;
                        param.property_lookup_mapping.source_key = "AGENT";
                        param.property_lookup_mapping.target_key = mtt::ARG_source;
                        param.selector_name  = mtt::ARG_source_offset_selector;
                        param.selector_value = mtt::ARG_source_offset_selector_default;
                        
                        usize count = 1;
                        bool is_specific = true;
                        
                        count_AND_specificity_constraint(st.agent_list.front(), &count, &is_specific);
                        
                        param.count_constraint = count;
                        if (!is_specific) {
                            param.type = Call_Param::TYPE::QUERY;
                            
                            
                            MTT_print("%s", "???\n");
                        } else {
                            for (auto& instance : instances) {
                                instance.script_property.selector_name  = param.selector_name;
                                instance.script_property.selector_value = param.selector_value;
                                instance.script_property.counter = instance.script_property.counter;
                                param.prop_list.push_back(instance.script_property);
                            }
                        }
                        
                        param_list.params.push_back(param);
                    
                        
                    }
                    if (!types.empty()) {
                        
                        for (auto& type_entry : types) {
                            Call_Param param = {};
                            
                            type_entry.speech->call_desc_ref = prop.call_desc_ref;
                            const usize entry_idx = param_list.params.size();
                            type_entry.speech->call_desc_ref.param_list_entry_index = entry_idx;
                            {
                                auto* refers_to = type_entry.speech->refers_to;
                                if (refers_to != nullptr && refers_to->call_desc_ref.is_valid()) {
                                    param.is_ref = true;
                                    auto* call_desc_ref = &refers_to->call_desc_ref;
                                    param.ref.call_id = call_desc_ref->call_id;
                                    param.ref.param_list_index = call_desc_ref->param_list_index;
                                    param.ref.param_list_entry_index = call_desc_ref->param_list_entry_index;
                                    param.allow_prev_seen_thing_props = !refers_to->treat_as_unique;
                                } else {
                                    param.allow_prev_seen_thing_props = !type_entry.speech->treat_as_unique;
                                }
                            }
                        
                            
                            param.property_lookup_mapping.source_key = "AGENT";
                            param.property_lookup_mapping.target_key = mtt::ARG_source;
                            param.selector_name  = mtt::ARG_source_offset_selector;
                            param.selector_value = mtt::ARG_source_offset_selector_default;
                            param.label = type_entry.script_property.label;
                            
                            mtt::String str = MTT_string_ref_to_cstring(type_entry.script_property.value.String);
                            noun_add(str);
                            
                            usize count = 1;
                            bool do_override_count_if_nonspecific = false;
                            if (ARGS::type_param_should_use_own_query(type_entry, scb, st, args, args2, &count, param.property_lookup_mapping.source_key, param, &do_override_count_if_nonspecific)) {
                                param.type = Call_Param::TYPE::QUERY;
                                
                                
                                bool is_specific = true;
                                usize init_count = count;
                                count_AND_specificity_constraint(type_entry.speech, &count, &is_specific);
                                
                                if (do_override_count_if_nonspecific && !is_specific && !type_entry.script_property.type_is_random_selection) {
                                    count = init_count;
                                } else if (type_entry.script_property.type_should_choose_all) {
                                    count = ULLONG_MAX;
                                }
                                
                                param.count_constraint = count;
                                
                                build_query(st, type_entry.speech, param, str);
                                if (Query_Rule_is_valid(&param.query_param.query_rule)) {
                                    auto& handle = param.query_param.rule_handles.emplace_back();
                                    
                                    handle.name = str;
                                    handle.var = Query_Rule_Var_for_name(&param.query_param.query_rule, str.c_str());
                                    
                                    
                                    param_list.params.push_back(param);
                                } else {
                                    // ???
                                }
                            } else if constexpr (ARGS::is_query_override) {
                                if constexpr (ARGS::is_new_action) {
                                    param.type = Call_Param::TYPE::ARG_THINGS;
                                } else {
                                    param.type = Call_Param::TYPE::RULE_RESULT;
                                }
                                
                                param_list.params.push_back(param);
                            }
                        }
                    }
                }
                
            }
        }
        
        {
            
            if (st.has_properties) {
                
                {
                    init_multipliers_and_args(st.property_list, call->multipliers, call->params().curr_param_list());
                }
                
                {
                    std::vector<Speech_Property*> looping = {};
                    if (find_match(st.property_list, (std::vector<Match_Text_W_Label>){
                        (Match_Text_W_Label) { .text = "back",  .label = "modifier" },
                        (Match_Text_W_Label) { .text = "forth", .label = "modifier" },
                    },
                    looping)) {
                        st.is_looping_sequence = true;
                    };
                }
                auto* dict = dt::word_dict();
                auto& call_modifiers = st.property_list;
                for (usize i = 0; i < call_modifiers.size(); i += 1) {
                    auto* prop = call_modifiers[i];
                    auto& label = prop->label;
                    
                    auto* parent = prop->get_parent();
                    bool is_negated = false;
                    if (parent != nullptr && (parent->label == "become" || parent->label == "is")) {
                        Speech_Property* n_prop = nullptr;
                        parent->try_get_only_prop("NEGATED", &n_prop);
                        if (n_prop != nullptr && n_prop->value.flag == true) {
                            is_negated = true;
                        }
                    }
                    
                    if (prop->value.text == "completely" || prop->value.text == "entirely") {
                        call->modifiers.push_back({prop->value.text, is_negated});
                        continue;
                    } else if ((label != "trait" && label != "modifier") || dict->is_ignored_attribute(prop->value.text)) {
                        continue;
                    }
                        
                    call->modifiers.push_back({prop->value.text, is_negated});
                    
                }
            }
            
        }
    }
    
    if (st.has_prepositions || st.has_direct_objects) {
//            if (st.has_prepositions && st.has_direct_objects) {
//
//            } else
        if (st.has_prepositions) {
            
            auto& param_list = call->params().curr_param_list();
            
            
            for (auto& p : st.prepositions) {
                auto& prep = p.second;
                auto& label = prep.prep;
                if (label == "between") {
                    st.is_looping_sequence = true;
                }
                
                mtt::String spatial = {};
                if (!lookup_spatial(label, &spatial)) {
                    spatial = mtt::ARG_object_offset_selector_default;
                } else {
                    MTT_BP();
                }
                
                auto& types     = prep.types;
                auto& instances = prep.instances;
                
                
                {
                    if (!instances.empty()) {
                        Call_Param param = {};
                        //usize param_list_entry_idx = 0;
//                        for (auto& instance : instances) {
////                            auto* refers_to = instance.speech->refers_to;
////                            if (refers_to != nullptr) {
////                                if (refers_to->call_desc_ref.is_valid()) {
////                                    mtt::Call_Descriptor* referenced_call = refers_to->call_desc_ref.call_desc;
////                                    param.is_ref = true;
////                                    param.ref.call_desc = referenced_call;
////                                    param.ref.param_list_index = current_param_list_index;
////                                    param.ref.param_list_entry_index = param_list_entry_idx;
////                                    param.type = Call_Param::TYPE::FIXED_THINGS;
////                                    param_list_entry_idx += 1;
////                                    param_list.params.push_back(param);
////                                    param_list_entry_idx += 1;
////                                }
////                            }
////                            param_list_entry_idx += 1;
//                        }
                        
                        param.type = Call_Param::TYPE::FIXED_THINGS;
                        param.property_lookup_mapping.source_key = "OBJECT";
                        if (label == "with") {
                            param.property_lookup_mapping.target_key = mtt::ARG_object_aux;
                            param.selector_name  = mtt::ARG_object_aux_offset_selector;
                        } else {
                            param.property_lookup_mapping.target_key = mtt::ARG_object;
                            param.selector_name  = mtt::ARG_object_offset_selector;
                        }
                        param.selector_value = spatial;
                        param.label =  label;
                        
                        usize count = 1;
                        bool is_specific = true;
                        
                        count_AND_specificity_constraint(prep.instance_props.front(), &count, &is_specific);
                        
                        param.count_constraint = count;
                        if (!is_specific) {
                            param.type = Call_Param::TYPE::QUERY;
                            MTT_print("%s", "???\n");
                        } else {
                            for (auto& instance : instances) {
                                instance.script_property.selector_name = param.selector_name;
                                instance.script_property.selector_value = param.selector_value;
                                instance.script_property.label = label;
                                instance.script_property.scope = label;

                                param.prop_list.push_back(instance.script_property);
                            }
                        }
                        
                        param_list.params.push_back(param);
                    }
                    if (!types.empty()) {
                        for (auto& type_entry : types) {
                            Call_Param param = {};
                            type_entry.speech->call_desc_ref = prop.call_desc_ref;
                            const usize entry_idx = param_list.params.size();
                            type_entry.speech->call_desc_ref.param_list_entry_index = entry_idx;
                            {
                                auto* refers_to = type_entry.speech->refers_to;
                                if (refers_to != nullptr && refers_to->call_desc_ref.is_valid()) {
                                    param.is_ref = true;
                                    auto* call_desc_ref = &refers_to->call_desc_ref;
                                    param.ref.call_id = call_desc_ref->call_id;
                                    param.ref.param_list_index = call_desc_ref->param_list_index;
                                    param.ref.param_list_entry_index = call_desc_ref->param_list_entry_index;
                                    param.allow_prev_seen_thing_props = !refers_to->treat_as_unique;
                                } else {
                                    param.allow_prev_seen_thing_props = !type_entry.speech->treat_as_unique;
                                }
                            }
                        
                            
                            param.property_lookup_mapping.source_key = "OBJECT";
                            if (label == "with" || label == "beside") {
                                param.property_lookup_mapping.target_key = mtt::ARG_object_aux;
                                param.selector_name  = mtt::ARG_object_aux_offset_selector;
                            } else {
                                param.property_lookup_mapping.target_key = mtt::ARG_object;
                                param.selector_name  = mtt::ARG_object_offset_selector;
                            }
                            param.selector_value = spatial;
                            param.label = label;
                            
                            mtt::String str;
                            if (!MTT_string_ref_is_valid(type_entry.script_property.value.String)) {
                                str = "anonymous_symbol";
                                type_entry.script_property.value.String = mtt::string(str.c_str());
                                noun_add(str);
                            } else {
                                str = MTT_string_ref_to_cstring(type_entry.script_property.value.String);
                                noun_add(str);
                            }
                            
                            usize count = 1;
                            bool do_override_count_if_nonspecific = false;
                            if (ARGS::type_param_should_use_own_query(type_entry, scb, st, args, args2, &count, param.property_lookup_mapping.source_key, param, &do_override_count_if_nonspecific)) {
                                param.type = Call_Param::TYPE::QUERY;
                                
                                
                                bool is_specific = true;
                                usize init_count = count;
                                count_AND_specificity_constraint(type_entry.speech, &count, &is_specific);
                                
                                if (do_override_count_if_nonspecific && !is_specific && !type_entry.script_property.type_is_random_selection) {
                                    if (type_entry.speech->value.kind_string != "THING_INSTANCE") {
                                        count = init_count;
                                    }
                                } else if (type_entry.script_property.type_should_choose_all) {
                                    count = ULLONG_MAX;
                                }
                                param.count_constraint = count;
                                
                                if (!param.is_ref) {
                                    
                                    
                                    
                                    if (args2.treat_prepositions_as_types) {
                                        param.type = Call_Param::TYPE::TYPES;
                                        param.prop_list.push_back(type_entry.script_property);
                                        param_list.params.push_back(param);
                                    } else {
                                        
                                        build_query(st, type_entry.speech, param, str);
                                        
                                        if (Query_Rule_is_valid(&param.query_param.query_rule)) {
                                            auto& handle = param.query_param.rule_handles.emplace_back();
                                            
                                            handle.name = str;
                                            handle.var = Query_Rule_Var_for_name(&param.query_param.query_rule, str.c_str());
                                            
                                            
                                            // TEST: ...
                                            
                                            
                                            
                                            param_list.params.push_back(param);
                                        } else {
                                            // ???
                                        }
                                    }
                                } else {
                                    param_list.params.push_back(param);
                                }
                            } else if constexpr (ARGS::is_query_override) {
                                if constexpr (ARGS::is_new_action) {
                                    param.type = Call_Param::TYPE::ARG_THINGS;
                                } else {
                                    param.type = Call_Param::TYPE::RULE_RESULT;
                                }
                                
                                param_list.params.push_back(param);
                            }
                        }
                    }
                }
                
            }
        }
        if (st.has_direct_objects) {
            auto& param_list = call->params().curr_param_list();
            
            
            {
                auto& types     = st.direct_object_type_list;
                auto& instances = st.direct_object_instance_list;
                
                
                {
                    if (!instances.empty()) {
                        Call_Param param = {};
                        
                        param.type = Call_Param::TYPE::FIXED_THINGS;
                        param.property_lookup_mapping.source_key = "DIRECT_OBJECT";
                        param.property_lookup_mapping.target_key = mtt::ARG_target;
                        param.selector_name  = mtt::ARG_target_offset_selector;
                        param.selector_value = mtt::ARG_target_offset_selector_default;
                        
                        usize count = 1;
                        bool is_specific = true;
                        
                        count_AND_specificity_constraint(st.direct_object_list.front(), &count, &is_specific);
                        
                        param.count_constraint = count;
                        if (!is_specific) {
                            param.type = Call_Param::TYPE::QUERY;
                            
                            
                            MTT_print("%s", "???\n");
                        } else {
                            for (auto& instance : instances) {
                                instance.script_property.selector_name = param.selector_name;
                                instance.script_property.selector_value = param.selector_value;
                                param.prop_list.push_back(instance.script_property);
                            }
                        }
                        
                        param_list.params.push_back(param);
                    
                        
                    }
                    if (!types.empty()) {
                        
                        for (auto& type_entry : types) {
                            Call_Param param = {};
                            
                            type_entry.speech->call_desc_ref = prop.call_desc_ref;
                            const usize entry_idx = param_list.params.size();
                            type_entry.speech->call_desc_ref.param_list_entry_index = entry_idx;
                            {
                                auto* refers_to = type_entry.speech->refers_to;
                                if (refers_to != nullptr && refers_to->call_desc_ref.is_valid()) {
                                    param.is_ref = true;
                                    auto* call_desc_ref = &refers_to->call_desc_ref;
                                    param.ref.call_id = call_desc_ref->call_id;
                                    param.ref.param_list_index = call_desc_ref->param_list_index;
                                    param.ref.param_list_entry_index = call_desc_ref->param_list_entry_index;
                                    param.allow_prev_seen_thing_props = !refers_to->treat_as_unique;
                                } else {
                                    param.allow_prev_seen_thing_props = !type_entry.speech->treat_as_unique;
                                }
                            }

                            
                            param.property_lookup_mapping.source_key = "DIRECT_OBJECT";
                            param.property_lookup_mapping.target_key = mtt::ARG_target;
                            param.selector_name  = mtt::ARG_target_offset_selector;
                            param.selector_value = mtt::ARG_target_offset_selector_default;
                            
                            mtt::String str = MTT_string_ref_to_cstring(type_entry.script_property.value.String);
                            noun_add(str);
                            
                            usize count = 1;
                            bool do_override_count_if_nonspecific = false;
                            if (ARGS::type_param_should_use_own_query(type_entry, scb, st, args, args2, &count, param.property_lookup_mapping.source_key, param, &do_override_count_if_nonspecific)) {
                                param.type = Call_Param::TYPE::QUERY;
                                
                                
                                bool is_specific = true;
                                usize init_count = count;
                                count_AND_specificity_constraint(type_entry.speech, &count, &is_specific);
                                
                                if (do_override_count_if_nonspecific && !is_specific && !type_entry.script_property.type_is_random_selection) {
                                    count = init_count;
                                } else if (type_entry.script_property.type_should_choose_all) {
                                    count = ULLONG_MAX;
                                }
                                param.count_constraint = count;
                                
                                if (type_entry.script_property.type_is_action || args2.treat_direct_objects_as_types) {
                                    param.type = Call_Param::TYPE::TYPES;
                                    param.prop_list.push_back(type_entry.script_property);
                                    param_list.params.push_back(param);
                                } else {
                                    
                                    build_query(st, type_entry.speech, param, str);
                                    if (Query_Rule_is_valid(&param.query_param.query_rule)) {
                                        auto& handle = param.query_param.rule_handles.emplace_back();
                                        
                                        handle.name = str;
                                        handle.var = Query_Rule_Var_for_name(&param.query_param.query_rule, str.c_str());
                                        
                                        
                                        param_list.params.push_back(param);
                                    } else {
                                        // ???
                                    }
                                }
                                
                            } else if constexpr (ARGS::is_query_override) {
                                if (!type_entry.script_property.type_is_action) {
                                    if constexpr (ARGS::is_new_action) {
                                        param.type = Call_Param::TYPE::ARG_THINGS;
                                    } else {
                                        param.type = Call_Param::TYPE::RULE_RESULT;
                                    }
                                } else {
                                    param.type = Call_Param::TYPE::TYPES;
                                    param.prop_list.push_back(type_entry.script_property);
                                }
                                param_list.params.push_back(param);
                            }
                        }
                    }
                }
                
            }
        }
        call->is_ping_pong_looping_sequence = st.is_looping_sequence;
//            for (usize i = 0; i < call->param_list_count(); i += 1) {
//                call->param_list(i).print();
//            }
    
        
// TODO: back and forth
//            if (st.has_back_and_forth && st.direct_object_list.size() > 1) {
//                auto& param_list = call->push_param_list();
////                for (auto& p : call->param_list(0).params) {
////                    param_list.params.push_back(p);
////                    if (param_list.params.back().)
////                }
//                {
//                    Call_Param param = {};
//                    param.type = Call_Param::TYPE::FIXED_THINGS;
//                    param.property_lookup_mapping.source_key = "AGENT";
//                    //param.property_lookup_mapping.target_key = mtt::ARG_source;
//                    param_list.params.push_back(param);
//                }
//                {
//                    Call_Param param = {};
//                    param.type = Call_Param::TYPE::SELECTOR;
//                    param.property_lookup_mapping.source_key = "AGENT";
//                    //param.property_lookup_mapping.target_key = mtt::ARG_source;
//                    param.selector_name  = mtt::ARG_source_offset_selector;
//                    param.selector_value = mtt::ARG_source_offset_selector_default;
//                    param_list.params.push_back(param);
//                }
//            }
    }
    return mtt::Result_make(true);
}

auto default_on_generate(Common_State& st, Call_Descriptor** call_out, DT_Args& args, DT_Args_Ex args2 = {})
{
    auto* scb = args.script_builder;
    
    if (scb->is_building_conditions) {
        return default_on_generate_inner<Default_On_Generate_Args_Conditional>(st, call_out, args, args2);
    } else if (scb->is_building_new_action) {
        return default_on_generate_inner<Default_On_Generate_Args_New_Action>(st, call_out, args, args2);
    } else {
        return default_on_generate_inner<Default_On_Generate_Args_Default>(st, call_out, args, args2);
    }    
}

// MARK: end on generate




// MARK: main initialization of actions

usize which = 0;
void load_standard_actions(dt::DrawTalk* dt)
{
    auto& lang_ctx = dt->lang_ctx;
    DT_Behavior_Catalogue& cat = lang_ctx.behavior_catalogue;
    
    
    
    mtt::World* world = dt->mtt;
    auto* root_graph = mtt::curr_graph(mtt::ctx());
    //   auto* bla = Thing_get_attribute_value(src, A);
    //    define_action(cat, "test", "", [&](auto& def) {
    //        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
    //            scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
    //
    //            mtt::begin_parallel_group(scb, "ROOT");
    //            mtt::thing_make(scb, mtt::ARCHETYPE_PRINT, "THIS_IS_A_TEST");
    //            mtt::thing_make(scb, mtt::ARCHETYPE_DEBUG__, "DEBUG");
    //
    //            mtt::End_Parallel_Group_Args args = {};
    //
    //            args.has_count = true;
    //            args.count = 1;
    //            args.reset_on_looparound = true;
    //            mtt::end_parallel_group(scb, args);
    //        });
    //        def.register_script("DEFAULT", script);
    //        script->label = "test";
    //
    //
    //
    //        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
    //            DT_scope_open();
    //            DT_print("Hello from script: %p\n", script);
    //            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    //        };
    //        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
    //            DT_print("Goodbye from script: %p\n", script);
    //            DT_scope_close();
    //            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    //        };
    //        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
    //            DT_print("started\n");
    //            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    //        };
    //        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
    //            DT_print("done\n");
    //            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
    //        };
    //        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
    //            DT_print("canceled\n");
    //            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
    //        };
    //
    //
    //        mtt::Script_Lookup_set_var_with_key(script->lookup(), "intensity", mtt::DEFAULT_LOOKUP_SCOPE, {
    //            Script_Property_make(mtt::Any::from_Float(1.0f))
    //        });
    //
    //
    //        Script_print_as_things(script);
    //
    //
    //
    //
    //    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
    //
    //
    //
    //        // generate code for the call's arguments
    //
    //        auto& def = *args.behavior;
    //        auto* script = def.Script_lookup("DEFAULT");
    //        auto* scb = args.script_builder;
    //        auto& prop = *args.prop;
    //        (void)prop;
    //        ASSERT_MSG(script != nullptr, "script should exist!");
    //
    //#if 1
    //        auto begin = begin_parallel_group(scb);
    //        begin.state->should_push_new_context = true;
    //
    //        Call_Descriptor* call = call_insert(scb, &args.gen_args, script);
    //
    //        wait_for_all_scheduled_ended(scb);
    //
    //
    //        End_Parallel_Group_Args pg_args = {};
    //        pg_args.reset_on_looparound = false;
    //        pg_args.has_count = true;
    //        pg_args.count = 1;
    //        auto end = end_parallel_group(scb, pg_args);
    //        MTT_UNUSED(end);
    //
    //        ASSERT_MSG(call != nullptr, "should exist!");
    //        MTT_BP();
    //#else
    //        Call_Descriptor* call = call_insert(scb, &args.gen_args, script);
    //#endif
    //
    //
    //        call->user_data = &def;
    //        call->on_invoke = [](Call_Descriptor* call, Script_Instance* s, Script_Instance** out) -> Logic_Procedure_Return_Status {
    //
    //            // instantiate a script based on the final arguments passed-in
    //
    //
    //            if (call->source_script_id == Script_ID_INVALID) {
    //                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
    //            }
    //
    //            Script* callee_script = Script_lookup(call->source_script_id);
    //            if (callee_script == nullptr) {
    //                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
    //            }
    //
    //
    //
    //
    //
    ////            else {
    ////                script_instance->set_lookup(&s->lookup());
    ////            }
    //
    //            auto& Q = Script_current_queue(s);
    //
    //            Q.push_back({
    //                .list = {},
    //                .call_info = *call,
    //            });
    //
    //
    ////            Script_Instance* script_instance = Script_Instance_call_from_script(callee_script, s);
    ////            Script_Instance* script_instance2 = Script_Instance_call_from_script(callee_script, s);
    //
    //            for (auto& [pre_id, pre_match_lookup] : callee_script->get_preconditions()) {
    //                Script* callee_script_precondition = Script_lookup(pre_id);
    //                if (callee_script_precondition == nullptr) {
    //                    continue;
    //                }
    //                // TODO: compare our arguments with what is in the lookup-table
    //                Script_Operation_List_enqueue_script(Q.back(), callee_script_precondition, s);
    //                //Q.back().list.push_back({Script_Instance_call_from_script(callee_script, s)});
    //            }
    //
    //
    //            Script_Operation_List_enqueue_script(Q.back(), callee_script, s);
    //            Script_Operation_List_enqueue_script(Q.back(), callee_script, s);
    //
    //            mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, Q.back().list.front().script);
    //
    //            if (out != nullptr) {
    //                *out = Q.back().list.front().script;
    //            }
    //
    //            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    //        };
    //
    //
    //        return mtt::Result_make(true);
    //    });
    
    
    {
        define_action(cat, "jump", "", [&](auto& def) {
            
            auto script_make = [](mtt::World* world, auto& target_key) -> Script* {
                Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    /*
                     mtt::begin_parallel_group(scb, "ROOT");
                     {
                     // TODO: iterator on destinations goes here
                     mtt::begin_parallel_group(scb);
                     {
                     mtt::lookup_get(scb, "height");
                     mtt::lookup_get(scb, "speed");
                     mtt::lookup_get(scb, "cycle_count");
                     mtt::lookup_get(scb, ARG_source);
                     //auto* get_initial_position = mtt::thing_make(scb, mtt::ARCHETYPE_GET_FIELD, "get_initial_position");
                     
                     {
                     auto* jmp = mtt::thing_make(scb, mtt::ARCHETYPE_JUMP, "JUMP_TEST");
                     auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                     if (!mtt::add_connection(scb->world, jmp, "is_done", ret, "is_active")) {
                     ASSERT_MSG(false, "?");
                     }
                     
                     //                        auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                     //                        MTT_UNUSED(contin);
                     mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                     
                     }
                     }
                     mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                     }
                     //            End_Parallel_Group_Args pg_args = {};
                     //            pg_args.reset_on_looparound = false;
                     //            pg_args.has_count = true;
                     //            pg_args.count = 1;
                     //            auto end = end_parallel_group(scb, pg_args);
                     //            MTT_UNUSED(end);
                     mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                     });
                     */
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        
                        
                        // TODO: iterator on destinations goes here
                        mtt::begin_parallel_group(scb);
                        {
                            // MARK: get all dynamic variables
                            mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                            auto* tgt = mtt::lookup_get(scb, target_key);
                            auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                            if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                                ASSERT_MSG(false, "?");
                            }
                            
                            
                            auto* height      = mtt::lookup_get(scb, "height");
                            static dt::Word_Val_Pair height_words[] = {
                                DEFAULT_ATTRIBUTES
                                {nullptr},
                            };
                            auto* process_height = dt::word_multiply_procedure_make(&height_words[0], nullptr);
                            mtt::thing(scb, process_height);
                            {
                                bool ok = false;
                                
                                ok = mtt::add_connection(mtt::ctx(), height, "value_single", process_height, "in");
                                ASSERT_MSG(ok, "???");
                                
                                
                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_height, "in_source");
                                
                                ASSERT_MSG(ok, "???");
                            }
                            
                            auto* speed       = mtt::lookup_get(scb, "speed");
                            static dt::Word_Val_Pair speed_words[] = {
                                DEFAULT_ATTRIBUTES
                                {nullptr},
                            };
                            auto* process_speed = dt::word_multiply_procedure_make(&speed_words[0], nullptr);
                            mtt::thing(scb, process_speed);
                            {
                                bool ok = false;
                                
                                ok = mtt::add_connection(mtt::ctx(), speed, "value_single", process_speed, "in");
                                ASSERT_MSG(ok, "???");
                                
                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_speed, "in_source");
                                
                                ASSERT_MSG(ok, "???");
                            }
                            
                            auto* cycle_count = mtt::lookup_get(scb, "cycle_count");
                            
                            auto* should_attach = mtt::lookup_get(scb, "should_attach");
                            
                            {
                                auto* jmp = mtt::thing_make(scb, mtt::ARCHETYPE_JUMP);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_height, "out", jmp, "height");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_speed, "out", jmp, "speed");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", jmp, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  tgt, "value_single", jmp, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "selector", jmp, "source_selector");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  tgt, "selector", jmp, "target_selector");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  cycle_count, "value_single", jmp, "cycle_count");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  should_attach, "value_single", jmp, "should_attach");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                
                                
                                if (!mtt::add_connection(scb->world, jmp, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                //                                auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                                MTT_UNUSED(contin);
                                
                                
                                
                                //                        mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            }
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    End_Parallel_Group_Args pg_args = {};
                    pg_args.reset_on_looparound = true;
                    pg_args.has_count = true;
                    pg_args.count = 1;
                    auto end = end_parallel_group(scb, pg_args);
                    MTT_UNUSED(end);
                    //mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                });
                script->label = "jump";
                
                return script;
            };
            
            
            Script* default_script = script_make(world, mtt::ARG_object);
            Script* targets_script = script_make(world, mtt::ARG_target);
            Script* objects_script = default_script;
            Script* scripts[2] = {targets_script, objects_script};
            
            def.register_script(DEFAULT_LOOKUP_SCOPE, default_script);
            def.register_script(ARG_target,           targets_script);
            def.register_script(ARG_object,           objects_script);
            
            
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts[i];
                
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(100.0f))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(1.0f))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Boolean(true))
                });
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_source_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_target_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_source_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_source_offset_selector_default.c_str()))
                //        });
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_target_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_target_offset_selector_default.c_str()))
                //        });
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        
                        
                        auto* V = Script_Lookup_get_var_with_key(script, mtt::ARG_object,  mtt::DEFAULT_LOOKUP_SCOPE);
                        auto* V2 = Script_Lookup_get_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE);
                        if ((V != nullptr && V->size() == 0) && (V2 != nullptr && V2->size() == 0)) {
                            Message msg;
                            msg.sender   = id;
                            msg.selector = string_ref_get("center");
                            selector_invoke(source_thing, &msg);
                            
                            mtt::Any& out = msg.output_value;
                            vec3 position = out.Vector3;
                            Script_Lookup_set_var_with_key(script, mtt::ARG_object, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                            Script_Lookup_set_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                        }
                        
                        
                        //                Script_Lookup_set_var_with_key(script, "position_source_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        //
                        //                Script_Lookup_set_var_with_key(script, "position_target_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                        
                        
                        
                        
                        //                auto q = Query_Rule_make(mtt::ctx(), "!Prefab(_X), jump(_X)");
                        //                MTT_BP();
                        //                ASSERT_MSG(Query_Rule_is_valid(&q), "???");
                        //                Query_Rule_Var_Info rv;
                        //                Query_Rule_Var_for_name(&q, "X", &rv);
                        //                Query_Rule_results_for_var(&q, rv.var, [](auto* result) {
                        //
                        //                    MTT_BP();
                        //                    return true;
                        //                });
                        //                MTT_BP();
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            auto* scb    = args.script_builder;
            
            
            
            //prop.print();
            
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
    }
    {
        define_action_alias(cat, "swim", "", "jump", "").set_on_generate_post([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            auto& def    = *args.behavior;
            
            Script* scripts[3] = {def.Script_lookup(mtt::DEFAULT_LOOKUP_SCOPE), def.Script_lookup(mtt::ARG_object), def.Script_lookup(mtt::ARG_target)};
            for (usize i = 0; i < 3; i += 1) {
                auto* script = scripts[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(12.0f))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(5.0f))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(1.2f))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Boolean(false))
                });
            }
            
            
            return mtt::Result_make(true);
        });
    }
//    {
//        define_action_alias(cat, "fly", "", "jump", "").set_on_generate_post([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
//            auto& def    = *args.behavior;
//            
//            Script* scripts[3] = {def.Script_lookup(mtt::DEFAULT_LOOKUP_SCOPE), def.Script_lookup(mtt::ARG_object), def.Script_lookup(mtt::ARG_target)};
//            for (usize i = 0; i < 3; i += 1) {
//                auto* script = scripts[i];
//                
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(6.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(7.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(0.4f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Boolean(false))
//                });
//            }
//            
//            
//            return mtt::Result_make(true);
//        });
//    }

    define_action_alias(cat, "dive", "", "jump", "").set_on_generate_post([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        auto& def    = *args.behavior;
        
        Script* scripts[3] = {def.Script_lookup(mtt::DEFAULT_LOOKUP_SCOPE), def.Script_lookup(mtt::ARG_object), def.Script_lookup(mtt::ARG_target)};
        for (usize i = 0; i < 3; i += 1) {
            auto* script = scripts[i];
            
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(200.0f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(1.6f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(3.5f))
            });
            
        }
        
        
        return mtt::Result_make(true);
    });
    define_action_alias(cat, "pounce", "", "jump", "").set_on_generate_post([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        auto& def    = *args.behavior;
        
        auto* script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
        
        Script* scripts[3] = {def.Script_lookup(mtt::DEFAULT_LOOKUP_SCOPE), def.Script_lookup(mtt::ARG_object), def.Script_lookup(mtt::ARG_target)};
        for (usize i = 0; i < 3; i += 1) {
            auto* script = scripts[i];
            
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(300.0f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(1.0f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(4.0f))
            });
            
        }
        
        
        return mtt::Result_make(true);
    });
    {
        define_action(cat, "move", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                            MTT_UNUSED(contin);
                            }
                            
                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
            //            {
            //                DT_Behavior* b = nullptr;
            //                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
            //                if (cat.lookup("jump", "", &b)) {
            //                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
            //                    script_only->add_precondition(other->id);
            //                } else {
            //                    ASSERT_MSG(false, ">?");
            //                }
            //
            //            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  tgt, "selector", mov, "target_selector");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                            MTT_UNUSED(contin);
                            }
                            
                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                // TODO: ...
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,           objects_script);
            
            Script* targets_objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
                        
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "child")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                if (!mtt::add_connection(scb->world, src, "value_single", attach, "parent")) {
                                    ASSERT_MSG(false, "?");
                                }

                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }
                            
                            auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                                ASSERT_MSG(false, "?");
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* dst = mtt::lookup_get(scb, ARG_target);

                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
//                                    auto* detach = mtt::thing_make(scb, mtt::ARCHETYPE_DETACH_FROM_PARENT);
//                                    if (!mtt::add_connection(scb->world, mov, "is_done", detach, "is_active")) {
//                                        ASSERT_MSG(false, "?");
//                                    }
//                                    if (!mtt::add_connection(scb->world, dst, "value_single", detach, "child")) {
//                                        ASSERT_MSG(false, "?");
//                                    }
                                auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, dst, "value_single", attach, "child")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "parent")) {
                                    ASSERT_MSG(false, "?");
                                }

                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_objects_script->label = def.key_primary;
            def.register_script(ARG_target + ":" + ARG_object, targets_objects_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script, targets_objects_script};
            for (usize i = 0; i < 4; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
        
    }
    
    {
        define_action_alias(cat, "fly", "", "move", "");
    }
    
    define_action_alias(cat, "run", "", "move", "").set_on_generate_post([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        auto& def    = *args.behavior;
        
        Script* scripts[3] = {def.Script_lookup(mtt::DEFAULT_LOOKUP_SCOPE), def.Script_lookup(mtt::ARG_object), def.Script_lookup(mtt::ARG_target)};
        for (usize i = 0; i < 3; i += 1) {
            auto* script = scripts[i];
            
            
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(8.0f))
            });
        }
        
        return mtt::Result_make(true);
    });

    define_action(cat, "pull", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });

    define_action(cat, "touch", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "tap", "", true, "touch", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
//    define_action(cat, "press", "", true, "rotate", "", [&](auto& def) {
//        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
//            
//        });
//    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
//        
//        
//        
//        return mtt::Result_make(true);
//    });
    
    
    //    define_action(cat, "go", "", true, "rotate", "", [&](auto& def) {
    //        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
    //
    //        });
    //    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
    //
    //
    //
    //        return mtt::Result_make(true);
    //    });
    
    
    
    {
        define_action(cat, "appear", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    
                }
//                End_Parallel_Group_Args pg_args = {};
//                pg_args.reset_on_looparound = true;
//                pg_args.has_count = true;
//                pg_args.count = 1;
                auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            script_agent->is_infinite = true;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            def.register_script(mtt::ARG_source, script_agent);
            def.register_script(mtt::ARG_object, script_agent);
            def.register_script(mtt::ARG_target, script_agent);
            
            Script* scripts_common_args[] = {script_agent};
            for (usize i = 0; i < 1; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {

                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                    auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                    bool has_tgts = false;
                    bool has_objs = false;
                    
                    mtt::World* world = mtt::ctx();
                    
                    
                    mtt::Set<mtt::Thing_ID> to_destroy = {};
                    if (tgt_entries != nullptr && !tgt_entries->empty()) {
                        has_tgts = true;
                        
                        for (auto& val : *tgt_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    if (obj_entries != nullptr && !obj_entries->empty()) {
                        has_objs = true;
                        
                        for (auto& val : *obj_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                        for (auto& val : *src_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    
                    //                    auto* twn_ctx = mtt_core_ctx()->tween_ctx;
                    //                    twn_Opt opt = {};
                    //                    opt.time = (call->has_time_interval) ? call->time_interval : 0.0f;
                    //                    opt.ease = TWN_QUADINOUT;
                    //                    opt.do_forwards_and_backwards = 0;
                    //                    opt.abort_if_in_progress = 1;
                    
                    
                    
                    for (auto thing_id : to_destroy) {
                        //                        Destroy_Command& cmd = world->to_destroy.emplace_back();
                        //                        cmd.thing_id = thing_id;
                        //                        cmd.affects_connected = true;
                        //                        cmd.do_fade_animation = true;
                        //                        cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
                        //                        cmd.time_remaining = cmd.time_delay;
                        
                        mtt::Thing* thing = mtt::Thing_try_get(world, thing_id);
                        if (thing == nullptr) {
                            continue;
                        }
                        //
                        //                        auto& render_data = mtt::rep(thing)->render_data.drawable_info_list;
                        //                        for (auto* dr : render_data) {
                        //                            
                        //                            twn_add_f32(twn_ctx, &dr->get_color_factor()[3], 1.0f, opt);
                        //                        }
                        mtt::set_should_render(thing);
                        
                        
                    }
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            Script_Property& prop = (*res)[0];
                            mtt::Any& val = prop.value;
                            auto id = val.thing_id;
                            //          auto& selector_name = prop.selector_name;
                            //            auto& selector_value = prop.selector_value;
                            
                            
                            
                            activate_actions(script, id);
                            
                            
                            
                            
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
        
    }
    {
        define_action_alias(cat, "reappear", "", "appear", "");
    }
    {
        define_action(cat, "destroy", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
                auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            script_agent->is_infinite = false;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            def.register_script(mtt::ARG_source, script_agent);
            def.register_script(mtt::ARG_object, script_agent);
            def.register_script(mtt::ARG_target, script_agent);
            
            Script* scripts_common_args[] = {script_agent};
            for (usize i = 0; i < 1; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = true}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            

            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                   
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    

                    
//                    if (to_destroy.empty() && src_entries) {
//                        for (auto& val : *src_entries) {
//                            if (val.value.type == MTT_THING) {
//                                mtt::Thing_ID id = val.value.thing_id;
//                                to_destroy.insert(id);
//                            }
//                        }
//                    }
                    
//                    for (auto thing_id : to_destroy) {
//                        Destroy_Command& cmd = world->to_destroy.emplace_back();
//                        cmd.thing_id = thing_id;
//                        cmd.affects_connected = false;
//                        cmd.do_fade_animation = true;
//                        cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
//                        cmd.time_remaining = cmd.time_delay;
//                    }
                    
                    struct Destroy_Args {
                        mtt::Set<mtt::Thing_ID> to_destroy = {};
                        bool destroy_connected = false;
                    };
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            auto* tgt_entries = Script_Lookup_get_var_with_key(script, ARG_target, DEFAULT_LOOKUP_SCOPE);
                            auto* obj_entries = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                            bool has_tgts = false;
                            bool has_objs = false;
                            
                            
                            const auto do_destroy_connected = [](const std::vector<Call_Descriptor::Modifier_Info>& mod)
                            {
                                for (usize i = 0; i < mod.size(); i += 1) {
                                    if (mod[i].label == "completely" || mod[i].label == "entirely") {
                                        return !mod[i].is_negated;
                                    }
                                }
                                return false;
                            };
                            auto* call = Call_Descriptor_from_Script_Instance(script);
                            auto& call_modifiers = call->modifiers;
                            bool destroy_all = do_destroy_connected(call_modifiers);
                            
                            auto& to_destroy_args = *(new Destroy_Args());
                            to_destroy_args.destroy_connected = destroy_all;
                            script->args = (void*)&to_destroy_args;
                            
                            
                            
                            
                            if (tgt_entries != nullptr && !tgt_entries->empty()) {
                                has_tgts = true;
                                
                                for (auto& val : *tgt_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_destroy_args.to_destroy.insert(id);
                                    }
                                }
                            }
                            if (obj_entries != nullptr && !obj_entries->empty()) {
                                has_objs = true;
                                
                                for (auto& val : *obj_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_destroy_args.to_destroy.insert(id);
                                    }
                                }
                            }
                            if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                                for (auto& val : *src_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_destroy_args.to_destroy.insert(id);
                                    }
                                }
                            }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                  //          auto& selector_name = prop.selector_name;
                //            auto& selector_value = prop.selector_value;
                            
                
                            
                            

                            
                            

                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            auto* to_destroy_ptr = (Destroy_Args*)args;
                            if (to_destroy_ptr == nullptr) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                            }
                            auto& to_destroy_args = *to_destroy_ptr;
                            bool affects_connected = to_destroy_args.destroy_connected;
                            for (auto thing_id : to_destroy_args.to_destroy) {
                                if (!mtt::Thing_try_get(world, thing_id)) {
                                    continue;
                                }
                                
                                Destroy_Command& cmd = world->to_destroy.emplace_back();
                                cmd.thing_id = thing_id;
                                cmd.affects_connected = affects_connected;
                                cmd.do_fade_animation = false;
                                cmd.time_delay = 0;
                                cmd.frame_delay = 1;
                                //cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
                                cmd.time_remaining = 0;//cmd.time_delay;
                            }
                            delete (&to_destroy_args);
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    

                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    {
        define_action_alias(cat, "destruct", "", "destroy", "");
    }
    {
        define_action_alias(cat, "demolish", "", "destroy", "");
    }
    {
        define_action_alias(cat, "delete", "", "destroy", "");
    }
    {
        define_action(cat, "disappear", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
//                    mtt::begin_parallel_group(scb);
//                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            script_agent->is_infinite = false;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            def.register_script(mtt::ARG_source, script_agent);
            def.register_script(mtt::ARG_object, script_agent);
            def.register_script(mtt::ARG_target, script_agent);
            
            Script* scripts_common_args[] = {script_agent};
            for (usize i = 0; i < 1; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            

            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                   
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                    auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                    bool has_tgts = false;
                    bool has_objs = false;
                    
                    mtt::World* world = mtt::ctx();
                    
                    
                    mtt::Set<mtt::Thing_ID> to_destroy = {};
                    if (tgt_entries != nullptr && !tgt_entries->empty()) {
                        has_tgts = true;
                        
                        for (auto& val : *tgt_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    if (obj_entries != nullptr && !obj_entries->empty()) {
                        has_objs = true;
                        
                        for (auto& val : *obj_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                        for (auto& val : *src_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    
                    
//                    auto* twn_ctx = mtt_core_ctx()->tween_ctx;
//                    twn_Opt opt = {};
//                    opt.time = (call->has_time_interval) ? call->time_interval : 0.0f;
//                    opt.ease = TWN_QUADINOUT;
//                    opt.do_forwards_and_backwards = 0;
//                    opt.abort_if_in_progress = 1;
                    
                    
                    
                    for (auto thing_id : to_destroy) {
//                        Destroy_Command& cmd = world->to_destroy.emplace_back();
//                        cmd.thing_id = thing_id;
//                        cmd.affects_connected = true;
//                        cmd.do_fade_animation = true;
//                        cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
//                        cmd.time_remaining = cmd.time_delay;
                        
                        mtt::Thing* thing = mtt::Thing_try_get(world, thing_id);
                        if (thing == nullptr) {
                            continue;
                        }
                        mtt::unset_should_render(thing);
//                        auto& render_data = mtt::rep(thing)->render_data.drawable_info_list;
//                        for (auto* dr : render_data) {
////                            opt.user_data = (void*)mtt::thing_id(thing);
////                            opt.callback = [](void* data) {
////                                mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx()(mtt::Thing*)data);
////                                
////                            };
////                            twn_add_f32(twn_ctx, &dr->get_color_factor()[3], 0.0f, opt);
//                        }
                        
                        
                    }
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            Script_Property& prop = (*res)[0];
                            mtt::Any& val = prop.value;
                            auto id = val.thing_id;
                  //          auto& selector_name = prop.selector_name;
                //            auto& selector_value = prop.selector_value;
                            
                
                            
                            activate_actions(script, id);

                            
                            

                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    

                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    {
        define_action_alias(cat, "vanish", "", "disappear", "");
    }
    // TODO: explode should be different, so this is temporary
    {
        define_action_alias(cat, "explode", "", "destruct", "");
    }
    
    define_action(cat, "see", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        return mtt::Result_make(true);
    });
    
    {
        define_action(cat, "follow", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
//            {
//                DT_Behavior* b = nullptr;
//                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
//                if (cat.lookup("jump", "", &b)) {
//                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
//                    script_only->add_precondition(other->id);
//                } else {
//                    ASSERT_MSG(false, ">?");
//                }
//
//            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
//                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
//                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
//                            ASSERT_MSG(false, "?");
//                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
//                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
//                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
//                            ASSERT_MSG(false, "?");
//                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,targets_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script};
            for (usize i = 0; i < 3; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
            
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
          //          auto& selector_name = prop.selector_name;
        //            auto& selector_value = prop.selector_value;
                    
        
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        

                        activate_actions(script, id);
                    }
                    
                    

                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    {
        define_action(cat, "attract", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
//            {
//                DT_Behavior* b = nullptr;
//                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
//                if (cat.lookup("jump", "", &b)) {
//                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
//                    script_only->add_precondition(other->id);
//                } else {
//                    ASSERT_MSG(false, ">?");
//                }
//
//            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_object);
//                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
//                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
//                            ASSERT_MSG(false, "?");
//                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_target);
//                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
//                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
//                            ASSERT_MSG(false, "?");
//                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_properties", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
//                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
//                                    ASSERT_MSG(false, "?");
//                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,targets_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script};
            for (usize i = 0; i < 3; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
            
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
          //          auto& selector_name = prop.selector_name;
        //            auto& selector_value = prop.selector_value;
                    
        
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        

                        activate_actions(script, id);
                    }
                    
                    

                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    define_action_alias(cat, "shiver", "", "jump", "").set_on_generate_post([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        auto& def    = *args.behavior;

        Script* scripts[3] = {def.Script_lookup(mtt::DEFAULT_LOOKUP_SCOPE), def.Script_lookup(mtt::ARG_object), def.Script_lookup(mtt::ARG_target)};
        for (usize i = 0; i < 3; i += 1) {
            auto* script = scripts[i];
            
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(12.0f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(42.0f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(0.5f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Boolean(false))
            });
        }
        
        
        return mtt::Result_make(true);
    });
    
    define_action_alias(cat, "oscillate", "", "jump", "").set_on_generate_post([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        auto& def    = *args.behavior;

        Script* scripts[3] = {def.Script_lookup(mtt::DEFAULT_LOOKUP_SCOPE), def.Script_lookup(mtt::ARG_object), def.Script_lookup(mtt::ARG_target)};
        for (usize i = 0; i < 3; i += 1) {
            auto* script = scripts[i];
            
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(100.0f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(12.0f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Float(0.2f))
            });
            mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Boolean(false))
            });
        }
        
        
        return mtt::Result_make(true);
    });
    
    {
        define_action(cat, "create", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
    //                    mtt::begin_parallel_group(scb);
    //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                                
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = true, .treat_direct_objects_as_types = true}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            auto* tgt_entries = Script_Lookup_get_var_with_key(script, ARG_target, DEFAULT_LOOKUP_SCOPE);
                            auto* obj_entries = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                            bool has_tgts = false;
                            bool has_objs = false;
                            
                            std::vector<mtt::Thing_ID> to_create = {};
                            std::vector<mtt::String> to_create_labels = {};
                            std::vector<mtt::Thing_ID> to_locate = {};
                            
                            struct String_Pair {
                                mtt::String selector_name;
                                mtt::String selector_value;
                            };
                            std::vector<String_Pair> to_locate_vals = {};
                            if (tgt_entries == nullptr || tgt_entries->empty()) {
                                tgt_entries = src_entries;
                            }
                            if (tgt_entries != nullptr && !tgt_entries->empty()) {
                                has_tgts = true;
                                for (auto& val : *tgt_entries) {
                                    if (val.value.type == MTT_THING) {
                                        
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_create.push_back(id);
                                    } else if (val.value.type == MTT_STRING) {
                                        
                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                        to_create_labels.push_back(str);
                                    }
                                }
                            }
                            
                            if (obj_entries != nullptr && !obj_entries->empty()) {
                                has_objs = true;
                                
                                for (auto& val : *obj_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_locate.push_back(id);
                                        to_locate_vals.push_back({val.selector_name, val.selector_value});
                                    }
                                }
                            }
                            if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                                for (auto& val : *src_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_create.push_back(id);
                                    }
                                }
                            }
                                
                            if (to_create_labels.empty() && to_create.empty()) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            auto* dict = dt::word_dict();
                            
                            auto* action_entry = verb_add("create");
                            auto* action_entry_appear = verb_add("appear");
                            
                            Script_Instance* s_to_return = (script->parent != nullptr) ? script->parent : script;
                            
                            
                            Input* const input = &mtt_core_ctx()->input;
                            auto& touches = input->users[0].touches;
                            usize touches_count = input->users[0].direct_count;
                            // TODO: disambiguate "beside" with touch
                            
                            if (!to_locate.empty()) {
                                for (usize i = 0; i < to_create_labels.size(); i += 1) {
                                    auto& label = to_create_labels[i];
                                    for (usize j = 0; j < to_locate.size(); j += 1) {
                                        mtt::Thing* thing_dst = mtt::Thing_try_get(world, to_locate[j]);
                                        if (thing_dst == nullptr) {
                                            continue;
                                        }
                                        
                                        auto& val = to_locate_vals[j];
                                        
                                        
                                        vec3* pos = mtt::access<vec3>(thing_dst, "position");
                                        if (pos == nullptr) {
                                            continue;
                                        }
                                        vec3 dst_pos = *pos;
                                        pos = &dst_pos;

                                        
                                        mtt::Thing* out = mtt::Thing_make_from_preset(world, label);
                                        if (out == nullptr) {
                                            auto* entry = noun_add(label);
                                            if (entry->things.empty()) {
                                                continue;
                                            }
                                            
                                            auto idx = MTT_Random_range(0, entry->things.size());
                                            auto it = entry->things.begin();
                                            for (isize i = 0; i < idx; i += 1) {
                                                ++it;
                                            }
                                            mtt::Thing_ID choice_id = it->first;
                                            
                                            out = mtt::Thing_copy_recursively(world, choice_id);
                                            if (out == nullptr) {
                                                continue;
                                            }
                                            
                                            mtt::Thing_save_as_preset(out);
                                        }
                                        
                                        {
                                            Message msg;
                                            msg.sender   = mtt::thing_id(out);
                                            msg.selector = string_ref_get(val.selector_value.c_str());
                                            selector_invoke(thing_dst, &msg);
                                            dst_pos = msg.output_value.Vector3;
                                        }
                                        
                                        Thing_set_position(out, *pos);
                                        
                                        if (!src_entries->empty()) {
                                            for (const auto& el : *src_entries) {
                                                auto& val = el.value;
                                                if (val.type != MTT_THING) {
                                                    continue;
                                                }
                                                mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                                Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_END);
                                            }
                                        }
                                        
                                        Thing_add_action_event_instantaneous(out, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(out, action_entry_appear, nullptr, VERB_EVENT_END);
                                        
                                        Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                    }
                                    
                                }
                                
                                for (usize i = 0; i < to_create.size(); i += 1) {
                                    mtt::Thing* thing_src = mtt::Thing_try_get(world, to_create[i]);
                                    if (thing_src == nullptr) {
                                        continue;
                                    }
                                    
                                    for (usize j = 0; j < to_locate.size(); j += 1) {
                                        mtt::Thing* thing_dst = mtt::Thing_try_get(world, to_locate[j]);
                                        if (thing_dst == nullptr) {
                                            continue;
                                        }
                                        
                                        vec3* pos = mtt::access<vec3>(thing_dst, "position");
                                        if (pos == nullptr) {
                                            continue;
                                        }
                                            
                                        mtt::Thing* out = mtt::Thing_copy_recursively(thing_src);
                                        if (out == nullptr) {
                                            continue;
                                        }
                                        
                                        Thing_set_position(out, *pos);
                                        
                                        if (!src_entries->empty()) {
                                            for (const auto& el : *src_entries) {
                                                auto& val = el.value;
                                                if (val.type != MTT_THING) {
                                                    continue;
                                                }
                                                mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                                Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_END);
                                            }
                                        }
                                        Thing_add_action_event_instantaneous(out, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(out, action_entry_appear, nullptr, VERB_EVENT_END);
                                        Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                    }
                                }
                            } else {
                                auto* cam = &DrawTalk_World_ctx()->cam;
                                vec3 pos;// = cam->position;
                                
                                {
                                    vec3 d_scale;
                                    quat d_orientation;
                                    vec3 d_translation;
                                    vec3 d_skew;
                                    vec4 d_perspective;
                                    {
                                        auto* vp = &mtt_core_ctx()->viewport;
                                        
                                        
                                        m::decompose(cam->cam_transform * m::translate(Mat4(1.0f), vec3(vp->width, vp->height, 0.0f) * 0.5f), d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                        
                                        pos = d_translation;
                                    }
                                    
                                }
                                
                                
                                for (usize i = 0; i < to_create_labels.size(); i += 1) {
                                    auto& label = to_create_labels[i];
                                    
                                    mtt::Thing* out = mtt::Thing_make_from_preset(world, label);
                                    if (out == nullptr) {
                                        auto* entry = noun_add(label);
                                        if (entry->things.empty()) {
                                            continue;
                                        }
                                        
                                        auto idx = MTT_Random_range(0, entry->things.size());
                                        auto it = entry->things.begin();
                                        for (isize i = 0; i < idx; i += 1) {
                                            ++it;
                                        }
                                        mtt::Thing_ID choice_id = it->first;
                                        
                                        out = mtt::Thing_copy_recursively(world, choice_id);
                                        if (out == nullptr) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_save_as_preset(out);
                                    }
                                    
                                    Thing_set_position(out, pos);
                                    
                                    if (!src_entries->empty()) {
                                        for (const auto& el : *src_entries) {
                                            auto& val = el.value;
                                            if (val.type != MTT_THING) {
                                                continue;
                                            }
                                            mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                            if (src == nullptr) {
                                                continue;
                                            }
                                            
                                            Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                            Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_END);
                                        }
                                    }
                                    
                                    Thing_add_action_event_instantaneous(out, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
                                    Thing_add_action_event_instantaneous(out, action_entry_appear, nullptr, VERB_EVENT_END);
                                    Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                }
                                
                                for (usize i = 0; i < to_create.size(); i += 1) {
                                    mtt::Thing* thing_src = mtt::Thing_try_get(world, to_create[i]);
                                    if (thing_src == nullptr) {
                                        continue;
                                    }
                                    
                                    mtt::Thing* out = mtt::Thing_copy_recursively(thing_src);
                                    if (out == nullptr) {
                                        continue;
                                    }
                                    
                                    Thing_set_position(out, pos);
                                    
                                    if (!src_entries->empty()) {
                                        for (const auto& el : *src_entries) {
                                            auto& val = el.value;
                                            if (val.type != MTT_THING) {
                                                continue;
                                            }
                                            mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                            if (src == nullptr) {
                                                continue;
                                            }
                                            
                                            Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                            Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_END);
                                        }
                                    }
                                    
                                    Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                    
                     
                                }
                            }
                            
                            /*
                            if (has_objs) {
                                auto* from_entries_ptr = src_entries;
                                if (has_tgts) {
                                    from_entries_ptr = tgt_entries;
                                }
                                
                                auto& from = *from_entries_ptr;
                                auto& to = *obj_entries;
                                
                                usize from_count = from.size();
                                usize to_count = to.size();
                                
                                if (to_count == 1) {
                                    auto& val = to[0];
                                    if (val.value.type != MTT_THING) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    mtt::Thing_ID id = val.value.thing_id;
                                    mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                    if (dst == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    
                                    vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                    if (dst_pos_ptr == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    vec3 dst_pos = *dst_pos_ptr;
                                    
                                    for (usize i = 0; i < from_count; i += 1) {
                                        auto& val_src = from[i];
                                        if (val_src.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID src_id = val_src.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_set_position(src, dst_pos);
                                    }
                                    
                                } else {
                                    if (to_count > from_count) {
                                        to_count = from_count;
                                    }
                                    
                                    usize min_cap = m::min(from_count, to_count);
                                    usize i = 0;
                                    for (; i < min_cap; i += 1) {
                                        auto& val_to = to[i];
                                        auto& val_from = from[i];
                                        if (val_to.value.type != MTT_THING || val_from.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID dst_id = val_to.value.thing_id;
                                        mtt::Thing* dst = mtt::Thing_try_get(world, dst_id);
                                        if (dst == nullptr) {
                                            continue;
                                        }
                                        mtt::Thing_ID src_id = val_from.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                        if (dst_pos_ptr == nullptr) {
                                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                        }
                                        vec3 dst_pos = *dst_pos_ptr;
                                        
                                        mtt::Thing_set_position(src, dst_pos);
                                        
                                    }
                                    if (i < from_count) {
                                        usize j = 0;
                                        for (; j < to_count; j += 1) {
                                            if (to[j].value.type == MTT_THING) {
                                                break;
                                            }
                                        }
                                        {
                                            auto& val = to[j];
                                            if (val.value.type != MTT_THING) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            mtt::Thing_ID id = val.value.thing_id;
                                            mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                            if (dst == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            
                                            vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                            if (dst_pos_ptr == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            vec3 dst_pos = *dst_pos_ptr;
                                            
                                            for (; i < from_count; i += 1) {
                                                auto& val_src = from[i];
                                                if (val.value.type != MTT_THING) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_ID src_id = val.value.thing_id;
                                                mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                                
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_set_position(src, dst_pos);
                                            }
                                        }
                                    }
                                    
                                }
                                
                            } else if (has_tgts) {
                                
                            } else {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                             */
                            //                        if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                            //                            for (auto& val : *src_entries) {
                            //                                if (val.value.type == MTT_THING) {
                            //                                    mtt::Thing_ID id = val.value.thing_id;
                            //                                    to_destroy.insert(id);
                            //                                }
                            //                            }
                            //                        }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            //                        auto& to_destroy = *(mtt::Set<mtt::Thing_ID>*)args;
                            //                        for (auto thing_id : to_destroy) {
                            //                            if (!mtt::Thing_try_get(world, thing_id)) {
                            //                                continue;
                            //                            }
                            //
                            //                            Destroy_Command& cmd = world->to_destroy.emplace_back();
                            //                            cmd.thing_id = thing_id;
                            //                            cmd.affects_connected = false;
                            //                            cmd.do_fade_animation = false;
                            //                            cmd.time_delay = 0;
                            //                            cmd.frame_delay = 1;
                            //                            //cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
                            //                            cmd.time_remaining = 0;//cmd.time_delay;
                            //                        }
                            //                        delete (&to_destroy);
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
        
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    }
    
    define_action_alias(cat, "make", "", "create", "");
    define_action_alias(cat, "spawn", "", "create", "");
    define_action_alias(cat, "copy", "", "create", "");
    define_action_alias(cat, "clone", "", "create", "");
    define_action_alias(cat, "duplicate", "", "create", "");
    
    {
        define_action(cat, "fill", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
    //                    mtt::begin_parallel_group(scb);
    //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                                
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, 
                                    {.supports_no_src = true, .treat_direct_objects_as_types = false, .treat_prepositions_as_types = true}
                                    ).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            //Script_Lookup_print(script->lookup());
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            auto* tgt_entries = Script_Lookup_get_var_with_key(script, ARG_target, DEFAULT_LOOKUP_SCOPE);
                            //auto* obj_entries = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                            auto* obj_entries = Script_Lookup_get_var_with_key(script, ARG_object_aux, DEFAULT_LOOKUP_SCOPE);
                            bool has_tgts = false;
                            bool has_objs = false;
                            
                            
                            std::vector<mtt::Thing_ID> to_create = {};
                            std::vector<mtt::String> to_create_labels = {};
                            std::vector<mtt::Thing_ID> to_locate = {};
                            if (tgt_entries == nullptr || tgt_entries->empty()) {
                                tgt_entries = src_entries;
                            }
                            if (tgt_entries != nullptr && !tgt_entries->empty()) {
                                has_tgts = true;
                                for (auto& val : *tgt_entries) {
                                    if (val.value.type == MTT_THING) {
                                        
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_locate.push_back(id);
                                    }
                                }
                            }
                            
                            if (obj_entries != nullptr && !obj_entries->empty()) {
                                has_objs = true;
                                
                                for (auto& val : *obj_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_create.push_back(id);
                                    } else if (val.value.type == MTT_STRING) {
                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                        to_create_labels.push_back(str);
                                    }
                                }
                            }
                            if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                                for (auto& val : *src_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_create.push_back(id);
                                    }  else if (val.value.type == MTT_STRING) {
                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                        to_create_labels.push_back(str);
                                    }
                                }
                            }
                                
                            if (to_create_labels.empty() && to_create.empty()) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            auto* dict = dt::word_dict();
                            
                            auto* action_entry = verb_add("create");
                            auto* action_entry_appear = verb_add("appear");
                            
                            Script_Instance* s_to_return = (script->parent != nullptr) ? script->parent : script;
                            
                            if (!to_locate.empty()) {
                                for (usize i = 0; i < to_create_labels.size(); i += 1) {
                                    auto& label = to_create_labels[i];
                                    for (usize j = 0; j < to_locate.size(); j += 1) {
                                        mtt::Thing* thing_dst = mtt::Thing_try_get(world, to_locate[j]);
                                        if (thing_dst == nullptr) {
                                            continue;
                                        }
                                        
//                                        vec3* pos = mtt::access<vec3>(thing_dst, "position");
//                                        if (pos == nullptr) {
//                                            continue;
//                                        }
                                        mtt::Rep* rep_dst = mtt::rep(thing_dst);
                                        if (rep_dst->colliders.empty()) {
                                            continue;
                                        }
                                        
                                        auto& tgt_box = rep_dst->colliders.front()->aabb.saved_box;
                                        vec2 tgt_tl = tgt_box.tl;
                                        vec2 tgt_br = tgt_box.br;
                                        auto& pose = rep_dst->pose_transform;
                                        
                                        // TODO: ... actually align correctly
                                        
                                        
                                        
                                        mtt::Thing* out = mtt::Thing_make_from_preset(world, label);
                                        if (out == nullptr) {
                                            auto* entry = noun_add(label);
                                            if (entry->things.empty()) {
                                                continue;
                                            }
                                            
                                            auto idx = MTT_Random_range(0, entry->things.size());
                                            auto it = entry->things.begin();
                                            for (isize i = 0; i < idx; i += 1) {
                                                ++it;
                                            }
                                            mtt::Thing_ID choice_id = it->first;
                                            
                                            out = mtt::Thing_copy_recursively(world, choice_id);
                                            if (out == nullptr) {
                                                continue;
                                            }
                                            
                                            //mtt::Thing_save_as_preset(out);
                                        }
                                        
                                        mtt::Rep* rep_out = mtt::rep(out);
                                        auto offset = rep_out->colliders.front()->aabb.half_extent;
                                        tgt_tl += offset;
                                        tgt_br -= offset;
                                        for (usize y = tgt_tl.y; y < tgt_br.y; y += (offset.y * 2)) {
                                            for (usize x = tgt_tl.x; x < tgt_br.x; x += (offset.x * 2)) {
                                                mtt::Thing* to_fill_with = mtt::Thing_copy_recursively(out);
                                                mtt::Thing_set_position(to_fill_with, vec3(x, y, 0.0f));
                                                mtt::connect_parent_to_child(world, thing_dst, to_fill_with);
                                            }
                                        }
                                        Thing_destroy_self_and_connected(world, mtt::thing_id(out));
                                        
                                        
                                        //
                                        
                                        //Thing_set_position(out, *pos);
                                        
                                        if (!src_entries->empty()) {
                                            for (const auto& el : *src_entries) {
                                                auto& val = el.value;
                                                if (val.type != MTT_THING) {
                                                    continue;
                                                }
                                                mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                            }
                                        }
                                        
                                        Thing_add_action_event_instantaneous(out, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
                                        
                                        Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                    }
                                    
                                }
                                
                                for (usize i = 0; i < to_create.size(); i += 1) {
                                    mtt::Thing* thing_src = mtt::Thing_try_get(world, to_create[i]);
                                    if (thing_src == nullptr) {
                                        continue;
                                    }
                                    
                                    for (usize j = 0; j < to_locate.size(); j += 1) {
                                        mtt::Thing* thing_dst = mtt::Thing_try_get(world, to_locate[j]);
                                        if (thing_dst == nullptr) {
                                            continue;
                                        }
                                        
//                                        vec3* pos = mtt::access<vec3>(thing_dst, "position");
//                                        if (pos == nullptr) {
//                                            continue;
//                                        }
                                        mtt::Rep* rep_dst = mtt::rep(thing_dst);
                                        if (rep_dst->colliders.empty()) {
                                            continue;
                                        }
                                        
                                        auto& tgt_box = rep_dst->colliders.front()->aabb.saved_box;
                                        vec2 tgt_tl = tgt_box.tl;
                                        vec2 tgt_br = tgt_box.br;
                                        auto& pose = rep_dst->pose_transform;
                                            
                                        mtt::Thing* out = thing_src;
                                        
                                        mtt::Rep* rep_out = mtt::rep(out);
                                        auto offset = rep_out->colliders.front()->aabb.half_extent;
                                        tgt_tl += offset;
                                        tgt_br -= offset;
                                        for (usize y = tgt_tl.y; y < tgt_br.y; y += (offset.y * 2)) {
                                            for (usize x = tgt_tl.x; x < tgt_br.x; x += (offset.x * 2)) {
                                                mtt::Thing* to_fill_with = mtt::Thing_copy_recursively(out);
                                                mtt::Thing_set_position(to_fill_with, vec3(x, y, 0.0f));
                                                mtt::connect_parent_to_child(world, thing_dst, to_fill_with);
                                            }
                                        }
                                        
                                        
                                        
                                        if (!src_entries->empty()) {
                                            for (const auto& el : *src_entries) {
                                                auto& val = el.value;
                                                if (val.type != MTT_THING) {
                                                    continue;
                                                }
                                                mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                            }
                                        }
                                        Thing_add_action_event_instantaneous(out, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
                                        Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                    }
                                }
                            } else {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                                
                                // doesn't really make sense for fill to do the following:
                                
                                auto* cam = &DrawTalk_World_ctx()->cam;
                                vec3 pos;// = cam->position;
                                
                                {
                                    vec3 d_scale;
                                    quat d_orientation;
                                    vec3 d_translation;
                                    vec3 d_skew;
                                    vec4 d_perspective;
                                    {
                                        auto* vp = &mtt_core_ctx()->viewport;
                                        
                                        
                                        m::decompose(cam->cam_transform * m::translate(Mat4(1.0f), vec3(vp->width, vp->height, 0.0f) * 0.5f), d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                        
                                        pos = d_translation;
                                    }
                                    
                                }
                                
                                
                                for (usize i = 0; i < to_create_labels.size(); i += 1) {
                                    auto& label = to_create_labels[i];
                                    
                                    mtt::Thing* out = mtt::Thing_make_from_preset(world, label);
                                    if (out == nullptr) {
                                        auto* entry = noun_add(label);
                                        if (entry->things.empty()) {
                                            continue;
                                        }
                                        
                                        auto idx = MTT_Random_range(0, entry->things.size());
                                        auto it = entry->things.begin();
                                        for (isize i = 0; i < idx; i += 1) {
                                            ++it;
                                        }
                                        mtt::Thing_ID choice_id = it->first;
                                        
                                        out = mtt::Thing_copy_recursively(world, choice_id);
                                        if (out == nullptr) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_save_as_preset(out);
                                    }
                                    
                                    Thing_set_position(out, pos);
                                    
                                    if (!src_entries->empty()) {
                                        for (const auto& el : *src_entries) {
                                            auto& val = el.value;
                                            if (val.type != MTT_THING) {
                                                continue;
                                            }
                                            mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                            if (src == nullptr) {
                                                continue;
                                            }
                                            
                                            Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                        }
                                    }
                                    
                                    Thing_add_action_event_instantaneous(out, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
                                    Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                }
                                
                                for (usize i = 0; i < to_create.size(); i += 1) {
                                    mtt::Thing* thing_src = mtt::Thing_try_get(world, to_create[i]);
                                    if (thing_src == nullptr) {
                                        continue;
                                    }
                                    
                                    mtt::Thing* out = mtt::Thing_copy_recursively(thing_src);
                                    if (out == nullptr) {
                                        continue;
                                    }
                                    
                                    Thing_set_position(out, pos);
                                    
                                    if (!src_entries->empty()) {
                                        for (const auto& el : *src_entries) {
                                            auto& val = el.value;
                                            if (val.type != MTT_THING) {
                                                continue;
                                            }
                                            mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                            if (src == nullptr) {
                                                continue;
                                            }
                                            
                                            Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                        }
                                    }
                                    
                                    Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                    
                     
                                }
                            }
                            
                            /*
                            if (has_objs) {
                                auto* from_entries_ptr = src_entries;
                                if (has_tgts) {
                                    from_entries_ptr = tgt_entries;
                                }
                                
                                auto& from = *from_entries_ptr;
                                auto& to = *obj_entries;
                                
                                usize from_count = from.size();
                                usize to_count = to.size();
                                
                                if (to_count == 1) {
                                    auto& val = to[0];
                                    if (val.value.type != MTT_THING) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    mtt::Thing_ID id = val.value.thing_id;
                                    mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                    if (dst == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    
                                    vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                    if (dst_pos_ptr == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    vec3 dst_pos = *dst_pos_ptr;
                                    
                                    for (usize i = 0; i < from_count; i += 1) {
                                        auto& val_src = from[i];
                                        if (val_src.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID src_id = val_src.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_set_position(src, dst_pos);
                                    }
                                    
                                } else {
                                    if (to_count > from_count) {
                                        to_count = from_count;
                                    }
                                    
                                    usize min_cap = m::min(from_count, to_count);
                                    usize i = 0;
                                    for (; i < min_cap; i += 1) {
                                        auto& val_to = to[i];
                                        auto& val_from = from[i];
                                        if (val_to.value.type != MTT_THING || val_from.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID dst_id = val_to.value.thing_id;
                                        mtt::Thing* dst = mtt::Thing_try_get(world, dst_id);
                                        if (dst == nullptr) {
                                            continue;
                                        }
                                        mtt::Thing_ID src_id = val_from.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                        if (dst_pos_ptr == nullptr) {
                                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                        }
                                        vec3 dst_pos = *dst_pos_ptr;
                                        
                                        mtt::Thing_set_position(src, dst_pos);
                                        
                                    }
                                    if (i < from_count) {
                                        usize j = 0;
                                        for (; j < to_count; j += 1) {
                                            if (to[j].value.type == MTT_THING) {
                                                break;
                                            }
                                        }
                                        {
                                            auto& val = to[j];
                                            if (val.value.type != MTT_THING) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            mtt::Thing_ID id = val.value.thing_id;
                                            mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                            if (dst == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            
                                            vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                            if (dst_pos_ptr == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            vec3 dst_pos = *dst_pos_ptr;
                                            
                                            for (; i < from_count; i += 1) {
                                                auto& val_src = from[i];
                                                if (val.value.type != MTT_THING) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_ID src_id = val.value.thing_id;
                                                mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                                
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_set_position(src, dst_pos);
                                            }
                                        }
                                    }
                                    
                                }
                                
                            } else if (has_tgts) {
                                
                            } else {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                             */
                            //                        if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                            //                            for (auto& val : *src_entries) {
                            //                                if (val.value.type == MTT_THING) {
                            //                                    mtt::Thing_ID id = val.value.thing_id;
                            //                                    to_destroy.insert(id);
                            //                                }
                            //                            }
                            //                        }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            //                        auto& to_destroy = *(mtt::Set<mtt::Thing_ID>*)args;
                            //                        for (auto thing_id : to_destroy) {
                            //                            if (!mtt::Thing_try_get(world, thing_id)) {
                            //                                continue;
                            //                            }
                            //
                            //                            Destroy_Command& cmd = world->to_destroy.emplace_back();
                            //                            cmd.thing_id = thing_id;
                            //                            cmd.affects_connected = false;
                            //                            cmd.do_fade_animation = false;
                            //                            cmd.time_delay = 0;
                            //                            cmd.frame_delay = 1;
                            //                            //cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
                            //                            cmd.time_remaining = 0;//cmd.time_delay;
                            //                        }
                            //                        delete (&to_destroy);
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
        
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    }
    
    define_action_alias(cat, "pack", "", "fill", "");
    
    {
        define_action(cat, "increase", "", [&](auto& def) {
            
            auto script_make = [](mtt::World* world, auto& target_key) -> Script* {
                Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        mtt::begin_parallel_group(scb);
                        {
                            // MARK: get all dynamic variables
                            mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                            
                            
//                            auto* height      = mtt::lookup_get(scb, "height");
//                            static dt::Word_Val_Pair height_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_height = dt::word_multiply_procedure_make(&height_words[0], nullptr);
//                            mtt::thing(scb, process_height);
//                            {
//                                bool ok = false;
//                                
//                                ok = mtt::add_connection(mtt::ctx(), height, "value_single", process_height, "in");
//                                ASSERT_MSG(ok, "???");
//                                
//                                
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_height, "in_source");
//                                
//                                ASSERT_MSG(ok, "???");
//                            }
//                            
//                            auto* speed       = mtt::lookup_get(scb, "speed");
//                            static dt::Word_Val_Pair speed_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_speed = dt::word_multiply_procedure_make(&speed_words[0], nullptr);
//                            mtt::thing(scb, process_speed);
//                            {
//                                bool ok = false;
//                                
//                                ok = mtt::add_connection(mtt::ctx(), speed, "value_single", process_speed, "in");
//                                ASSERT_MSG(ok, "???");
//                                
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_speed, "in_source");
//                                
//                                ASSERT_MSG(ok, "???");
//                            }
//                            
//                            auto* cycle_count = mtt::lookup_get(scb, "cycle_count");
//                            
//                            auto* should_attach = mtt::lookup_get(scb, "should_attach");
                            
                            {
                                //auto* jmp = mtt::thing_make(scb, mtt::ARCHETYPE_JUMP);
                                
                                // void (*proc)(mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg)
                                auto process_main = mtt::code_procedure_make_thing_in([](mtt::World* world, Script_Instance* script, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg, bool is_valid, mtt::Thing* thing)
                                {
                                    if (is_valid) {
                                        switch (mtt::thing_type_id(thing)) {
                                            case ARCHETYPE_FREEHAND_SKETCH: {
                                                break;
                                            }
                                            case ARCHETYPE_NUMBER: {
                                                auto* var = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                                                float64 inc_val = 1;
                                                if (var != nullptr && !var->empty()) {
                                                    float64 total = 0;
                                                    for (usize i = 0; i < var->size(); i += 1) {
                                                        total += (*var)[i].counter;
                                                    }
                                                    inc_val = total;
                                                }
                                                auto* val = mtt::access<float32>(thing, "value");
                                                mtt::number_update_value(thing, *val + inc_val);
                                                break;
                                            }
                                            default: {
                                                MTT_print("%s %llu\n", "unsupported thing", mtt::thing_id(thing));
                                                break;
                                            }
                                        }
                                        OUT_arg.value.out.set_Float(0);
                                    } else {
                                        OUT_arg.value.out.set_Float(0);
                                    }
                                });
                                mtt::thing(scb, process_main);
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_main, "in_source");
                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_height, "out", jmp, "height");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_speed, "out", jmp, "speed");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "value_single", jmp, "source");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "value_single", jmp, "target");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "selector", jmp, "source_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "selector", jmp, "target_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  cycle_count, "value_single", jmp, "cycle_count");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  should_attach, "value_single", jmp, "should_attach");
//                                    ASSERT_MSG(ok, "???");
//                                }
                                
                                
                                
//                                if (!mtt::add_connection(scb->world, jmp, "is_done", tgt, "increment_index")) {
//                                    ASSERT_MSG(false, "?");
//                                }
                                
                                //                                auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                                MTT_UNUSED(contin);
                                
                                
                                
                                //                        mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            }
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    }
                    auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                    //mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                });
                script->label = "increase";
                
                return script;
            };
            
            
            Script* default_script = script_make(world, mtt::ARG_object);
            Script* targets_script = script_make(world, mtt::ARG_target);
            Script* objects_script = default_script;
            Script* scripts[2] = {targets_script, objects_script};
            
            def.register_script(DEFAULT_LOOKUP_SCOPE, default_script);
            def.register_script(ARG_target,           targets_script);
            def.register_script(ARG_object,           objects_script);
            
            
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts[i];
                
                
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(100.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(2.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(1.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Boolean(true))
//                });
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_source_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_target_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                //mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_source_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_source_offset_selector_default.c_str()))
                //        });
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_target_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_target_offset_selector_default.c_str()))
                //        });
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        
                        
                        auto* V = Script_Lookup_get_var_with_key(script, mtt::ARG_object,  mtt::DEFAULT_LOOKUP_SCOPE);
                        auto* V2 = Script_Lookup_get_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE);
                        if ((V != nullptr && V->size() == 0) && (V2 != nullptr && V2->size() == 0)) {
                            Message msg;
                            msg.sender   = id;
                            msg.selector = string_ref_get("center");
                            selector_invoke(source_thing, &msg);
                            
                            mtt::Any& out = msg.output_value;
                            vec3 position = out.Vector3;
                            Script_Lookup_set_var_with_key(script, mtt::ARG_object, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                            Script_Lookup_set_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                        }
                        
                        
                        //                Script_Lookup_set_var_with_key(script, "position_source_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        //
                        //                Script_Lookup_set_var_with_key(script, "position_target_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                        
                        
                        
                        
                        //                auto q = Query_Rule_make(mtt::ctx(), "!Prefab(_X), jump(_X)");
                        //                MTT_BP();
                        //                ASSERT_MSG(Query_Rule_is_valid(&q), "???");
                        //                Query_Rule_Var_Info rv;
                        //                Query_Rule_Var_for_name(&q, "X", &rv);
                        //                Query_Rule_results_for_var(&q, rv.var, [](auto* result) {
                        //
                        //                    MTT_BP();
                        //                    return true;
                        //                });
                        //                MTT_BP();
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            auto* scb    = args.script_builder;
            
            
            
            //prop.print();
            
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.treat_prepositions_as_types = true }).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
        
        
        define_action_alias(cat, "increment", "", "increase", "");
    }
    
    
    {
        define_action(cat, "decrease", "", [&](auto& def) {
            
            auto script_make = [](mtt::World* world, auto& target_key) -> Script* {
                Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        mtt::begin_parallel_group(scb);
                        {
                            // MARK: get all dynamic variables
                            mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                            
                            
//                            auto* height      = mtt::lookup_get(scb, "height");
//                            static dt::Word_Val_Pair height_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_height = dt::word_multiply_procedure_make(&height_words[0], nullptr);
//                            mtt::thing(scb, process_height);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), height, "value_single", process_height, "in");
//                                ASSERT_MSG(ok, "???");
//
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_height, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* speed       = mtt::lookup_get(scb, "speed");
//                            static dt::Word_Val_Pair speed_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_speed = dt::word_multiply_procedure_make(&speed_words[0], nullptr);
//                            mtt::thing(scb, process_speed);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), speed, "value_single", process_speed, "in");
//                                ASSERT_MSG(ok, "???");
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_speed, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* cycle_count = mtt::lookup_get(scb, "cycle_count");
//
//                            auto* should_attach = mtt::lookup_get(scb, "should_attach");
                            
                            {
                                //auto* jmp = mtt::thing_make(scb, mtt::ARCHETYPE_JUMP);
                                
                                // void (*proc)(mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg)
                                auto process_main = mtt::code_procedure_make_thing_in([](mtt::World* world, Script_Instance* script, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg, bool is_valid, mtt::Thing* thing)
                                {
                                    if (is_valid) {
                                        switch (mtt::thing_type_id(thing)) {
                                            case ARCHETYPE_FREEHAND_SKETCH: {
                                                break;
                                            }
                                            case ARCHETYPE_NUMBER: {
                                                //Script_Lookup_print(script->lookup());;
                                                auto* var = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                                                float64 dec_val = 1;
                                                if (var != nullptr && !var->empty()) {
                                                    float64 total = 0;
                                                    for (usize i = 0; i < var->size(); i += 1) {
                                                        total += (*var)[i].counter;
                                                    }
                                                    dec_val = total;
                                                }
                                                auto* val = mtt::access<float32>(thing, "value");
                                                mtt::number_update_value(thing, *val - dec_val);
                                                break;
                                            }
                                            default: {
                                                MTT_print("%s %llu\n", "unsupported thing", mtt::thing_id(thing));
                                                break;
                                            }
                                        }
                                        OUT_arg.value.out.set_Float(0);
                                    } else {
                                        OUT_arg.value.out.set_Float(0);
                                    }
                                });
                                mtt::thing(scb, process_main);
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_main, "in_source");
                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_height, "out", jmp, "height");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_speed, "out", jmp, "speed");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "value_single", jmp, "source");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "value_single", jmp, "target");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "selector", jmp, "source_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "selector", jmp, "target_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  cycle_count, "value_single", jmp, "cycle_count");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  should_attach, "value_single", jmp, "should_attach");
//                                    ASSERT_MSG(ok, "???");
//                                }
                                
                                
                                
//                                if (!mtt::add_connection(scb->world, jmp, "is_done", tgt, "increment_index")) {
//                                    ASSERT_MSG(false, "?");
//                                }
                                
                                //                                auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                                MTT_UNUSED(contin);
                                
                                
                                
                                //                        mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            }
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    }
                    auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                    //mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                });
                script->label = "increase";
                
                return script;
            };
            
            
            Script* default_script = script_make(world, mtt::ARG_object);
            Script* targets_script = script_make(world, mtt::ARG_target);
            Script* objects_script = default_script;
            Script* scripts[2] = {targets_script, objects_script};
            
            def.register_script(DEFAULT_LOOKUP_SCOPE, default_script);
            def.register_script(ARG_target,           targets_script);
            def.register_script(ARG_object,           objects_script);
            
            
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts[i];
                
                
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(100.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(2.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(1.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Boolean(true))
//                });
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_source_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_target_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                //mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_source_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_source_offset_selector_default.c_str()))
                //        });
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_target_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_target_offset_selector_default.c_str()))
                //        });
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        
                        auto* V = Script_Lookup_get_var_with_key(script, mtt::ARG_object,  mtt::DEFAULT_LOOKUP_SCOPE);
                        auto* V2 = Script_Lookup_get_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE);
                        if ((V != nullptr && V->size() == 0) && (V2 != nullptr && V2->size() == 0)) {
                            Message msg;
                            msg.sender   = id;
                            msg.selector = string_ref_get("center");
                            selector_invoke(source_thing, &msg);
                            
                            mtt::Any& out = msg.output_value;
                            vec3 position = out.Vector3;
                            Script_Lookup_set_var_with_key(script, mtt::ARG_object, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                            Script_Lookup_set_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                        }
                        
                        
                        //                Script_Lookup_set_var_with_key(script, "position_source_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        //
                        //                Script_Lookup_set_var_with_key(script, "position_target_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                        
                        
                        
                        
                        //                auto q = Query_Rule_make(mtt::ctx(), "!Prefab(_X), jump(_X)");
                        //                MTT_BP();
                        //                ASSERT_MSG(Query_Rule_is_valid(&q), "???");
                        //                Query_Rule_Var_Info rv;
                        //                Query_Rule_Var_for_name(&q, "X", &rv);
                        //                Query_Rule_results_for_var(&q, rv.var, [](auto* result) {
                        //
                        //                    MTT_BP();
                        //                    return true;
                        //                });
                        //                MTT_BP();
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            auto* scb    = args.script_builder;
            
            
            
            //prop.print();
            
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.treat_prepositions_as_types = true}).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
        
        define_action_alias(cat, "decrement", "", "decrease", "");
    }
    
    {
        define_action(cat, "multiply", "", [&](auto& def) {
            
            auto script_make = [](mtt::World* world, auto& target_key) -> Script* {
                Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        mtt::begin_parallel_group(scb);
                        {
                            // MARK: get all dynamic variables
                            mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                            
                            
//                            auto* height      = mtt::lookup_get(scb, "height");
//                            static dt::Word_Val_Pair height_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_height = dt::word_multiply_procedure_make(&height_words[0], nullptr);
//                            mtt::thing(scb, process_height);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), height, "value_single", process_height, "in");
//                                ASSERT_MSG(ok, "???");
//
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_height, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* speed       = mtt::lookup_get(scb, "speed");
//                            static dt::Word_Val_Pair speed_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_speed = dt::word_multiply_procedure_make(&speed_words[0], nullptr);
//                            mtt::thing(scb, process_speed);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), speed, "value_single", process_speed, "in");
//                                ASSERT_MSG(ok, "???");
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_speed, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* cycle_count = mtt::lookup_get(scb, "cycle_count");
//
//                            auto* should_attach = mtt::lookup_get(scb, "should_attach");
                            
                            {
                                //auto* jmp = mtt::thing_make(scb, mtt::ARCHETYPE_JUMP);
                                
                                // void (*proc)(mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg)
                                auto process_main = mtt::code_procedure_make_thing_in([](mtt::World* world, Script_Instance* script, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg, bool is_valid, mtt::Thing* thing)
                                {
                                    if (is_valid) {
                                        switch (mtt::thing_type_id(thing)) {
                                            case ARCHETYPE_FREEHAND_SKETCH: {
                                                break;
                                            }
                                            case ARCHETYPE_NUMBER: {
                                                auto* var = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                                                float64 inc_val = 1;
                                                if (var != nullptr && !var->empty()) {
                                                    float64 total = 0;
                                                    for (usize i = 0; i < var->size(); i += 1) {
                                                        total += (*var)[i].counter;
                                                    }
                                                    inc_val = total;
                                                }
                                                auto* val = mtt::access<float32>(thing, "value");
                                                mtt::number_update_value(thing, *val * inc_val);
                                                break;
                                            }
                                            default: {
                                                MTT_print("%s %llu\n", "unsupported thing", mtt::thing_id(thing));
                                                break;
                                            }
                                        }
                                        OUT_arg.value.out.set_Float(0);
                                    } else {
                                        OUT_arg.value.out.set_Float(0);
                                    }
                                });
                                mtt::thing(scb, process_main);
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_main, "in_source");
                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_height, "out", jmp, "height");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_speed, "out", jmp, "speed");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "value_single", jmp, "source");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "value_single", jmp, "target");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "selector", jmp, "source_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "selector", jmp, "target_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  cycle_count, "value_single", jmp, "cycle_count");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  should_attach, "value_single", jmp, "should_attach");
//                                    ASSERT_MSG(ok, "???");
//                                }
                                
                                
                                
//                                if (!mtt::add_connection(scb->world, jmp, "is_done", tgt, "increment_index")) {
//                                    ASSERT_MSG(false, "?");
//                                }
                                
                                //                                auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                                MTT_UNUSED(contin);
                                
                                
                                
                                //                        mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            }
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    }
                    auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                    //mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                });
                script->label = "multiply";
                
                return script;
            };
            
            
            Script* default_script = script_make(world, mtt::ARG_object);
            Script* targets_script = script_make(world, mtt::ARG_target);
            Script* objects_script = default_script;
            Script* scripts[2] = {targets_script, objects_script};
            
            def.register_script(DEFAULT_LOOKUP_SCOPE, default_script);
            def.register_script(ARG_target,           targets_script);
            def.register_script(ARG_object,           objects_script);
            
            
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts[i];
                
                
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(100.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(2.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(1.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Boolean(true))
//                });
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_source_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_target_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                //mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_source_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_source_offset_selector_default.c_str()))
                //        });
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_target_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_target_offset_selector_default.c_str()))
                //        });
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        
                        
                        auto* V = Script_Lookup_get_var_with_key(script, mtt::ARG_object,  mtt::DEFAULT_LOOKUP_SCOPE);
                        auto* V2 = Script_Lookup_get_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE);
                        if ((V != nullptr && V->size() == 0) && (V2 != nullptr && V2->size() == 0)) {
                            Message msg;
                            msg.sender   = id;
                            msg.selector = string_ref_get("center");
                            selector_invoke(source_thing, &msg);
                            
                            mtt::Any& out = msg.output_value;
                            vec3 position = out.Vector3;
                            Script_Lookup_set_var_with_key(script, mtt::ARG_object, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                            Script_Lookup_set_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                        }
                        
                        
                        //                Script_Lookup_set_var_with_key(script, "position_source_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        //
                        //                Script_Lookup_set_var_with_key(script, "position_target_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                        
                        
                        
                        
                        //                auto q = Query_Rule_make(mtt::ctx(), "!Prefab(_X), jump(_X)");
                        //                MTT_BP();
                        //                ASSERT_MSG(Query_Rule_is_valid(&q), "???");
                        //                Query_Rule_Var_Info rv;
                        //                Query_Rule_Var_for_name(&q, "X", &rv);
                        //                Query_Rule_results_for_var(&q, rv.var, [](auto* result) {
                        //
                        //                    MTT_BP();
                        //                    return true;
                        //                });
                        //                MTT_BP();
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            auto* scb    = args.script_builder;
            
            
            
            //prop.print();
            
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.treat_prepositions_as_types = true }).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
    }
    
    {
        define_action(cat, "divide", "", [&](auto& def) {
            
            auto script_make = [](mtt::World* world, auto& target_key) -> Script* {
                Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        mtt::begin_parallel_group(scb);
                        {
                            // MARK: get all dynamic variables
                            mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                            
                            
//                            auto* height      = mtt::lookup_get(scb, "height");
//                            static dt::Word_Val_Pair height_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_height = dt::word_multiply_procedure_make(&height_words[0], nullptr);
//                            mtt::thing(scb, process_height);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), height, "value_single", process_height, "in");
//                                ASSERT_MSG(ok, "???");
//
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_height, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* speed       = mtt::lookup_get(scb, "speed");
//                            static dt::Word_Val_Pair speed_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_speed = dt::word_multiply_procedure_make(&speed_words[0], nullptr);
//                            mtt::thing(scb, process_speed);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), speed, "value_single", process_speed, "in");
//                                ASSERT_MSG(ok, "???");
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_speed, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* cycle_count = mtt::lookup_get(scb, "cycle_count");
//
//                            auto* should_attach = mtt::lookup_get(scb, "should_attach");
                            
                            {
                                //auto* jmp = mtt::thing_make(scb, mtt::ARCHETYPE_JUMP);
                                
                                // void (*proc)(mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg)
                                auto process_main = mtt::code_procedure_make_thing_in([](mtt::World* world, Script_Instance* script, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg, bool is_valid, mtt::Thing* thing)
                                {
                                    if (is_valid) {
                                        switch (mtt::thing_type_id(thing)) {
                                            case ARCHETYPE_FREEHAND_SKETCH: {
                                                break;
                                            }
                                            case ARCHETYPE_NUMBER: {
                                                auto* var = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                                                float64 inc_val = 1;
                                                if (var != nullptr && !var->empty()) {
                                                    float64 total = 0;
                                                    for (usize i = 0; i < var->size(); i += 1) {
                                                        total += (*var)[i].counter;
                                                    }
                                                    inc_val = total;
                                                }
                                                auto* val = mtt::access<float32>(thing, "value");
                                                mtt::number_update_value(thing, *val / ((inc_val == 0) ? 1 : inc_val));
                                                break;
                                            }
                                            default: {
                                                MTT_print("%s %llu\n", "unsupported thing", mtt::thing_id(thing));
                                                break;
                                            }
                                        }
                                        OUT_arg.value.out.set_Float(0);
                                    } else {
                                        OUT_arg.value.out.set_Float(0);
                                    }
                                });
                                mtt::thing(scb, process_main);
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_main, "in_source");
                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_height, "out", jmp, "height");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_speed, "out", jmp, "speed");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "value_single", jmp, "source");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "value_single", jmp, "target");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "selector", jmp, "source_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "selector", jmp, "target_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  cycle_count, "value_single", jmp, "cycle_count");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  should_attach, "value_single", jmp, "should_attach");
//                                    ASSERT_MSG(ok, "???");
//                                }
                                
                                
                                
//                                if (!mtt::add_connection(scb->world, jmp, "is_done", tgt, "increment_index")) {
//                                    ASSERT_MSG(false, "?");
//                                }
                                
                                //                                auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                                MTT_UNUSED(contin);
                                
                                
                                
                                //                        mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            }
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    }
                    auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                    //mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                });
                script->label = "divide";
                
                return script;
            };
            
            
            Script* default_script = script_make(world, mtt::ARG_object);
            Script* targets_script = script_make(world, mtt::ARG_target);
            Script* objects_script = default_script;
            Script* scripts[2] = {targets_script, objects_script};
            
            def.register_script(DEFAULT_LOOKUP_SCOPE, default_script);
            def.register_script(ARG_target,           targets_script);
            def.register_script(ARG_object,           objects_script);
            
            
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts[i];
                
                
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(100.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(2.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(1.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Boolean(true))
//                });
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_source_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_target_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                //mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_source_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_source_offset_selector_default.c_str()))
                //        });
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_target_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_target_offset_selector_default.c_str()))
                //        });
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        
                        
                        auto* V = Script_Lookup_get_var_with_key(script, mtt::ARG_object,  mtt::DEFAULT_LOOKUP_SCOPE);
                        auto* V2 = Script_Lookup_get_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE);
                        if ((V != nullptr && V->size() == 0) && (V2 != nullptr && V2->size() == 0)) {
                            Message msg;
                            msg.sender   = id;
                            msg.selector = string_ref_get("center");
                            selector_invoke(source_thing, &msg);
                            
                            mtt::Any& out = msg.output_value;
                            vec3 position = out.Vector3;
                            Script_Lookup_set_var_with_key(script, mtt::ARG_object, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                            Script_Lookup_set_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                        }
                        
                        
                        //                Script_Lookup_set_var_with_key(script, "position_source_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        //
                        //                Script_Lookup_set_var_with_key(script, "position_target_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                        
                        
                        
                        
                        //                auto q = Query_Rule_make(mtt::ctx(), "!Prefab(_X), jump(_X)");
                        //                MTT_BP();
                        //                ASSERT_MSG(Query_Rule_is_valid(&q), "???");
                        //                Query_Rule_Var_Info rv;
                        //                Query_Rule_Var_for_name(&q, "X", &rv);
                        //                Query_Rule_results_for_var(&q, rv.var, [](auto* result) {
                        //
                        //                    MTT_BP();
                        //                    return true;
                        //                });
                        //                MTT_BP();
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            auto* scb    = args.script_builder;
            
            
            
            //prop.print();
            
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.treat_prepositions_as_types = true }).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
    }
    
    {
        define_action(cat, "accelerate", "", [&](auto& def) {
            
            auto script_make = [](mtt::World* world, auto& target_key) -> Script* {
                Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        mtt::begin_parallel_group(scb);
                        {
                            // MARK: get all dynamic variables
                            mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                            
                            
//                            auto* height      = mtt::lookup_get(scb, "height");
//                            static dt::Word_Val_Pair height_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_height = dt::word_multiply_procedure_make(&height_words[0], nullptr);
//                            mtt::thing(scb, process_height);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), height, "value_single", process_height, "in");
//                                ASSERT_MSG(ok, "???");
//
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_height, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* speed       = mtt::lookup_get(scb, "speed");
//                            static dt::Word_Val_Pair speed_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_speed = dt::word_multiply_procedure_make(&speed_words[0], nullptr);
//                            mtt::thing(scb, process_speed);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), speed, "value_single", process_speed, "in");
//                                ASSERT_MSG(ok, "???");
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_speed, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* cycle_count = mtt::lookup_get(scb, "cycle_count");
//
//                            auto* should_attach = mtt::lookup_get(scb, "should_attach");
                            
                            {
                                //auto* jmp = mtt::thing_make(scb, mtt::ARCHETYPE_JUMP);
                                
                                // void (*proc)(mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg)
                                auto process_main = mtt::code_procedure_make_thing_in([](mtt::World* world, Script_Instance* script, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg, bool is_valid, mtt::Thing* thing)
                                {
                                    if (is_valid) {
                                        switch (mtt::thing_type_id(thing)) {
                                            case ARCHETYPE_FREEHAND_SKETCH: {
                                                auto* var = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                                                bool has_cap = false;
                                                float32 cap = 0;
                                                if (var->size() > 0) {
                                                    for (usize i = 0; i < var->size(); i += 1) {
                                                        auto& entry = ((*var)[i]);
                                                        if (entry.label == "until") {
                                                            cap = entry.counter;
                                                            has_cap = true;
                                                            break;
                                                        }
                                                    }
                                                }
                                                if (var->front().value.type == MTT_THING) {
                                                    mtt::Thing* thing_other = mtt::Thing_try_get(world, var->front().value.thing_id);
                                                    if (thing_other == nullptr) {
                                                        break;
                                                    }
                                                    
                                                    switch (mtt::thing_type_id(thing_other)) {
                                                        case ARCHETYPE_FREEHAND_SKETCH: {
                                                            auto* vel = mtt::access<vec3>(thing, "velocity");
                                                            auto* vel_other = mtt::access<vec3>(thing_other, "velocity");
                                                            if (vel != nullptr && vel_other != nullptr) {
                                                                *vel += *vel_other;
                                                                if (has_cap) {
                                                                    auto len = m::length(*vel);
                                                                    auto dir = m::normalize(*vel);
                                                                    len = m::clamp(len, -cap, cap);
                                                                    *vel = len * dir;
                                                                }
                                                            }
                                                            break;
                                                        }
                                                        case ARCHETYPE_NUMBER: {
                                                            auto* vel = mtt::access<vec3>(thing, "velocity");
                                                            if (vel != nullptr) {
                                                                auto* val = mtt::access<float32>(thing_other, "value");
                                                                *vel = *vel + *val;
                                                                if (has_cap) {
                                                                    auto len = m::length(*vel);
                                                                    auto dir = m::normalize(*vel);
                                                                    len = m::clamp(len, -cap, cap);
                                                                    *vel = len * dir;
                                                                }
                                                            }
                                                            break;
                                                        }
                                                        default: {
                                                            break;
                                                        }
                                                    }
                                                    
                                                    break;
                                                }
                                                
                                                float64 inc_val = 1;
                                                if (var != nullptr && !var->empty()) {
                                                    float64 total = 0;
                                                    for (usize i = 0; i < var->size(); i += 1) {
                                                        if ((*var)[i].label == "until") {
                                                            continue;
                                                        }
                                                        total += (*var)[i].counter;
                                                    }
                                                    inc_val = total;
                                                }
                                                auto* vel = mtt::access<vec3>(thing, "velocity");
                                                auto* acc = mtt::access<vec3>(thing, "acceleration");
                                                auto magnitude = m::length(*vel);
                                                if (*vel == m::vec3_zero()) {
                                                    *vel += vec3(inc_val, 0.0f, 0.0f);
                                                } else {
                                                    auto dir = m::normalize(*vel);
                                                    magnitude += inc_val;
                                                    *vel = magnitude * dir;
                                                }
                                                
                                                if (has_cap) {
                                                    auto len = m::length(*vel);
                                                    auto dir = m::normalize(*vel);
                                                    len = m::clamp(len, -cap, cap);
                                                    *vel = len * dir;
                                                }
                                                
                                                break;
                                            }
                                            case ARCHETYPE_NUMBER: {
                                                break;
                                            }
                                            default: {
                                                MTT_print("%s %llu\n", "unsupported thing", mtt::thing_id(thing));
                                                break;
                                            }
                                        }
                                        OUT_arg.value.out.set_Float(0);
                                    } else {
                                        OUT_arg.value.out.set_Float(0);
                                    }
                                });
                                mtt::thing(scb, process_main);
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_main, "in_source");
                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_height, "out", jmp, "height");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_speed, "out", jmp, "speed");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "value_single", jmp, "source");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "value_single", jmp, "target");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "selector", jmp, "source_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "selector", jmp, "target_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  cycle_count, "value_single", jmp, "cycle_count");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  should_attach, "value_single", jmp, "should_attach");
//                                    ASSERT_MSG(ok, "???");
//                                }
                                
                                
                                
//                                if (!mtt::add_connection(scb->world, jmp, "is_done", tgt, "increment_index")) {
//                                    ASSERT_MSG(false, "?");
//                                }
                                
                                //                                auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                                MTT_UNUSED(contin);
                                
                                
                                
                                //                        mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            }
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    }
                    auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                    //mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                });
                script->label = "accelerate";
                
                return script;
            };
            
            
            Script* default_script = script_make(world, mtt::ARG_object);
            Script* targets_script = script_make(world, mtt::ARG_target);
            Script* objects_script = default_script;
            Script* scripts[2] = {targets_script, objects_script};
            
            def.register_script(DEFAULT_LOOKUP_SCOPE, default_script);
            def.register_script(ARG_target,           targets_script);
            def.register_script(ARG_object,           objects_script);
            
            
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts[i];
                
                
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(100.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(2.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(1.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Boolean(true))
//                });
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_source_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_target_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                //mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_source_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_source_offset_selector_default.c_str()))
                //        });
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_target_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_target_offset_selector_default.c_str()))
                //        });
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        
                        
                        auto* V = Script_Lookup_get_var_with_key(script, mtt::ARG_object,  mtt::DEFAULT_LOOKUP_SCOPE);
                        auto* V2 = Script_Lookup_get_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE);
                        if ((V != nullptr && V->size() == 0) && (V2 != nullptr && V2->size() == 0)) {
                            Message msg;
                            msg.sender   = id;
                            msg.selector = string_ref_get("center");
                            selector_invoke(source_thing, &msg);
                            
                            mtt::Any& out = msg.output_value;
                            vec3 position = out.Vector3;
                            Script_Lookup_set_var_with_key(script, mtt::ARG_object, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                            Script_Lookup_set_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                        }
                        
                        
                        //                Script_Lookup_set_var_with_key(script, "position_source_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        //
                        //                Script_Lookup_set_var_with_key(script, "position_target_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                        
                        
                        
                        
                        //                auto q = Query_Rule_make(mtt::ctx(), "!Prefab(_X), jump(_X)");
                        //                MTT_BP();
                        //                ASSERT_MSG(Query_Rule_is_valid(&q), "???");
                        //                Query_Rule_Var_Info rv;
                        //                Query_Rule_Var_for_name(&q, "X", &rv);
                        //                Query_Rule_results_for_var(&q, rv.var, [](auto* result) {
                        //
                        //                    MTT_BP();
                        //                    return true;
                        //                });
                        //                MTT_BP();
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            auto* scb    = args.script_builder;
            
            
            
            //prop.print();
            
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.treat_prepositions_as_types = true }).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
    }
    
    {
        define_action(cat, "slow", "", [&](auto& def) {
            
            auto script_make = [](mtt::World* world, auto& target_key) -> Script* {
                Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        mtt::begin_parallel_group(scb);
                        {
                            // MARK: get all dynamic variables
                            mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                            
                            
//                            auto* height      = mtt::lookup_get(scb, "height");
//                            static dt::Word_Val_Pair height_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_height = dt::word_multiply_procedure_make(&height_words[0], nullptr);
//                            mtt::thing(scb, process_height);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), height, "value_single", process_height, "in");
//                                ASSERT_MSG(ok, "???");
//
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_height, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* speed       = mtt::lookup_get(scb, "speed");
//                            static dt::Word_Val_Pair speed_words[] = {
//                                DEFAULT_ATTRIBUTES
//                                {nullptr},
//                            };
//                            auto* process_speed = dt::word_multiply_procedure_make(&speed_words[0], nullptr);
//                            mtt::thing(scb, process_speed);
//                            {
//                                bool ok = false;
//
//                                ok = mtt::add_connection(mtt::ctx(), speed, "value_single", process_speed, "in");
//                                ASSERT_MSG(ok, "???");
//
//                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_speed, "in_source");
//
//                                ASSERT_MSG(ok, "???");
//                            }
//
//                            auto* cycle_count = mtt::lookup_get(scb, "cycle_count");
//
//                            auto* should_attach = mtt::lookup_get(scb, "should_attach");
                            
                            {
                                //auto* jmp = mtt::thing_make(scb, mtt::ARCHETYPE_JUMP);
                                
                                // void (*proc)(mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg)
                                auto process_main = mtt::code_procedure_make_thing_in([](mtt::World* world, Script_Instance* script, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg, bool is_valid, mtt::Thing* thing)
                                {
                                    if (is_valid) {
                                        switch (mtt::thing_type_id(thing)) {
                                            case ARCHETYPE_FREEHAND_SKETCH: {
                                                auto* var = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                                                bool has_cap = false;
                                                float32 cap = 0;
                                                if (var == nullptr) {
                                                    break;
                                                }
                                                if (var->size() > 0) {
                                                    for (usize i = 0; i < var->size(); i += 1) {
                                                        auto& entry = ((*var)[i]);
                                                        if (entry.label == "until") {
                                                            cap = entry.counter;
                                                            has_cap = true;
                                                            break;
                                                        }
                                                    }
                                                }
                                                if (var->front().value.type == MTT_THING) {
                                                    mtt::Thing* thing_other = mtt::Thing_try_get(world, var->front().value.thing_id);
                                                    if (thing_other == nullptr) {
                                                        break;
                                                    }
                                                    
                                                    switch (mtt::thing_type_id(thing_other)) {
                                                        case ARCHETYPE_FREEHAND_SKETCH: {
                                                            auto* vel = mtt::access<vec3>(thing, "velocity");
                                                            auto* vel_other = mtt::access<vec3>(thing_other, "velocity");
                                                            if (vel != nullptr && vel_other != nullptr) {
                                                                if (*vel != m::vec3_zero()) {
                                                                    auto dir = m::normalize(*vel);
                                                                    *vel -= *vel_other;
                                                                    if (m::dot(dir, *vel) < 0.0f) {
                                                                        *vel = m::vec3_zero();
                                                                    }
                                                                }
                                                                if (has_cap) {
                                                                    auto len = m::length(*vel);
                                                                    auto dir = m::normalize(*vel);
                                                                    len = m::clamp(len, 0.0f, cap);
                                                                    *vel = len * dir;
                                                                }
                                                            }
                                                            break;
                                                        }
                                                        case ARCHETYPE_NUMBER: {
                                                            auto* vel = mtt::access<vec3>(thing, "velocity");
                                                            if (vel != nullptr) {
                                                                auto* val = mtt::access<float32>(thing_other, "value");
                                                                if (*vel != m::vec3_zero()) {
                                                                    auto dir = m::normalize(*vel);
                                                                    *vel = *vel - *val;
                                                                    if (*vel != m::vec3_zero() && m::dot(dir, *vel) < 0.0f) {
                                                                        *vel = m::vec3_zero();
                                                                    }
                                                                }
                                                                if (has_cap) {
                                                                    auto len = m::length(*vel);
                                                                    auto dir = m::normalize(*vel);
                                                                    len = m::clamp(len, 0.0f, cap);
                                                                    *vel = len * dir;
                                                                }
                                                            }
                                                            break;
                                                        }
                                                        default: {
                                                            break;
                                                        }
                                                    }
                                                    
                                                    break;
                                                }
                                                
                                                float64 inc_val = 1;
                                                if (var != nullptr && !var->empty()) {
                                                    float64 total = 0;
                                                    for (usize i = 0; i < var->size(); i += 1) {
                                                        if ((*var)[i].label == "until") {
                                                            continue;
                                                        }
                                                        total += (*var)[i].counter;
                                                    }
                                                    inc_val = total;
                                                }
                                                auto* vel = mtt::access<vec3>(thing, "velocity");
                                                auto* acc = mtt::access<vec3>(thing, "acceleration");
                                                auto magnitude = m::length(*vel);
                                                if (*vel == m::vec3_zero()) {
                                                    break;
                                                } else {
                                                    if (*vel != m::vec3_zero()) {
                                                        auto dir = m::normalize(*vel);
                                                        magnitude -= inc_val;
                                                        if (magnitude < 0) {
                                                            *vel = m::vec3_zero();
                                                        } else {
                                                            *vel -= magnitude * dir;
                                                        }
                                                    }
                                                }
                                                
                                                if (has_cap) {
                                                    auto len = m::length(*vel);
                                                    auto dir = m::normalize(*vel);
                                                    len = m::clamp(len, 0.0f, cap);
                                                    *vel = len * dir;
                                                }
                                                
                                                break;
                                            }
                                            case ARCHETYPE_NUMBER: {
                                                break;
                                            }
                                            default: {
                                                MTT_print("%s %llu\n", "unsupported thing", mtt::thing_id(thing));
                                                break;
                                            }
                                        }
                                        OUT_arg.value.out.set_Float(0);
                                    } else {
                                        OUT_arg.value.out.set_Float(0);
                                    }
                                });
                                mtt::thing(scb, process_main);
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_main, "in_source");
                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_height, "out", jmp, "height");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(), process_speed, "out", jmp, "speed");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "value_single", jmp, "source");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "value_single", jmp, "target");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  src, "selector", jmp, "source_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  tgt, "selector", jmp, "target_selector");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  cycle_count, "value_single", jmp, "cycle_count");
//                                    ASSERT_MSG(ok, "???");
//                                }
//                                {
//                                    bool ok = mtt::add_connection(mtt::ctx(),
//                                                                  should_attach, "value_single", jmp, "should_attach");
//                                    ASSERT_MSG(ok, "???");
//                                }
                                
                                
                                
//                                if (!mtt::add_connection(scb->world, jmp, "is_done", tgt, "increment_index")) {
//                                    ASSERT_MSG(false, "?");
//                                }
                                
                                //                                auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                                MTT_UNUSED(contin);
                                
                                
                                
                                //                        mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            }
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    }
                    auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                    //mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                });
                script->label = "slow";
                
                return script;
            };
            
            
            Script* default_script = script_make(world, mtt::ARG_object);
            Script* targets_script = script_make(world, mtt::ARG_target);
            Script* objects_script = default_script;
            Script* scripts[2] = {targets_script, objects_script};
            
            def.register_script(DEFAULT_LOOKUP_SCOPE, default_script);
            def.register_script(ARG_target,           targets_script);
            def.register_script(ARG_object,           objects_script);
            
            
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts[i];
                
                
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(100.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(2.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Float(1.0f))
//                });
//                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
//                    Script_Property_make(mtt::Any::from_Boolean(true))
//                });
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_source_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                //        mtt::Script_Lookup_set_key(script->lookup(), "position_target_start",   mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                //mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_source_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_source_offset_selector_default.c_str()))
                //        });
                //        mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_target_offset_selector, mtt::DEFAULT_LOOKUP_SCOPE, {
                //            Script_Property_make(mtt::Any::from_String(mtt::ARG_target_offset_selector_default.c_str()))
                //        });
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        
                        
                        auto* V = Script_Lookup_get_var_with_key(script, mtt::ARG_object,  mtt::DEFAULT_LOOKUP_SCOPE);
                        auto* V2 = Script_Lookup_get_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE);
                        if ((V != nullptr && V->size() == 0) && (V2 != nullptr && V2->size() == 0)) {
                            Message msg;
                            msg.sender   = id;
                            msg.selector = string_ref_get("center");
                            selector_invoke(source_thing, &msg);
                            
                            mtt::Any& out = msg.output_value;
                            vec3 position = out.Vector3;
                            Script_Lookup_set_var_with_key(script, mtt::ARG_object, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                            Script_Lookup_set_var_with_key(script, mtt::ARG_target, mtt::DEFAULT_LOOKUP_SCOPE, {
                                {.value = mtt::Any::from_Vector3(position)/*, .selector_name = selector_name, .selector_value = prop.selector_value*/}
                            });
                        }
                        
                        
                        //                Script_Lookup_set_var_with_key(script, "position_source_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        //
                        //                Script_Lookup_set_var_with_key(script, "position_target_start", mtt::DEFAULT_LOOKUP_SCOPE, {
                        //                    {.value = mtt::Any::from_Vector3(position), .selector_name = selector_name, .selector_value = prop.selector_value}
                        //                });
                        
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                        
                        
                        
                        
                        //                auto q = Query_Rule_make(mtt::ctx(), "!Prefab(_X), jump(_X)");
                        //                MTT_BP();
                        //                ASSERT_MSG(Query_Rule_is_valid(&q), "???");
                        //                Query_Rule_Var_Info rv;
                        //                Query_Rule_Var_for_name(&q, "X", &rv);
                        //                Query_Rule_results_for_var(&q, rv.var, [](auto* result) {
                        //
                        //                    MTT_BP();
                        //                    return true;
                        //                });
                        //                MTT_BP();
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            auto* scb    = args.script_builder;
            
            
            
            //prop.print();
            
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.treat_prepositions_as_types = true }).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
    }
    

    
    define_action(cat, "rise", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    
    
    define_action(cat, "plant", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "draw", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });

    define_action(cat, "collide", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });

    

    
    {
        define_action(cat, "transform", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
    //                    mtt::begin_parallel_group(scb);
    //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                                
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = true, .treat_direct_objects_as_types = true, .treat_prepositions_as_types = true}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            
                            

                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                        script->on_terminate = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            auto* tgt_entries = Script_Lookup_get_var_with_key(script, ARG_target, DEFAULT_LOOKUP_SCOPE);
                            auto* obj_entries = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                            bool has_tgts = false;
                            bool has_objs = false;
                            
                            
                            
                            // TODO: transform all things of type T into the other type
                            struct Label {
                                mtt::String type;
                                std::vector<mtt::String> modifiers;
                            };
                            struct Label_Instance {
                                mtt::Thing_ID thing_id;
                                //std::vector<mtt::String> modifiers;
                            };
                            std::vector<Label_Instance> to_transform = {};
                            std::vector<Label> to_transform_labels = {};
                            std::vector<Label_Instance> to_transform_into = {};
                            // TODO: ... might be necessary transform into type of something
                            std::vector<Label> to_transform_into_labels = {};
                            if (tgt_entries == nullptr || tgt_entries->empty()) {
                                tgt_entries = src_entries;
                            }
                            if (tgt_entries != nullptr && !tgt_entries->empty()) {
                                has_tgts = true;
                                for (auto& val : *tgt_entries) {
                                    if (val.value.type == MTT_THING) {
                                        
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_transform.push_back({id});
                                    } else if (val.value.type == MTT_STRING) {
                                        
                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                        to_transform_labels.push_back({str, val.modifiers});
                                    }
                                }
                            }
                            
                            if (obj_entries != nullptr && !obj_entries->empty()) {
                                has_objs = true;
                                
                                for (auto& val : *obj_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_transform_into.push_back({id});
                                    }  else if (val.value.type == MTT_STRING) {
                                        
                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                        to_transform_into_labels.push_back({str, val.modifiers});
                                    }
                                }
                            }
                            if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                                for (auto& val : *src_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        to_transform.push_back({id});
                                    }
                                }
                            }
                                
                            if (to_transform_labels.empty() && to_transform.empty()) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            auto* dict = dt::word_dict();
                            
                            auto* action_entry = verb_root_equivalence(verb_add("transform"));
                            auto* action_entry_into = verb_root_equivalence(verb_add("transforminto"));
                            auto* action_entry_appear = verb_root_equivalence(verb_add("appear"));
                            auto* action_entry_become = verb_root_equivalence(verb_add("become"));
                            
                            Script_Instance* s_to_return = (script->parent != nullptr) ? script->parent : script;
                            
                            if (!to_transform_into.empty() || !to_transform_into_labels.empty()) {
                                // TODO: ... labels
                                if constexpr ((false)) {
                                    for (usize i = 0; i < to_transform_labels.size(); i += 1) {
                                        auto& label = to_transform_labels[i];
                                        for (usize j = 0; j < to_transform_into.size(); j += 1) {
                                            mtt::Thing* thing_dst = mtt::Thing_try_get(world, to_transform_into[j].thing_id);
                                            if (thing_dst == nullptr) {
                                                continue;
                                            }
                                            
                                            vec3* pos = mtt::access<vec3>(thing_dst, "position");
                                            if (pos == nullptr) {
                                                continue;
                                            }
                                            
                                            mtt::Thing* out = mtt::Thing_make_from_preset(world, label.type);
                                            if (out == nullptr) {
                                                auto* entry = noun_add(label.type);
                                                if (entry->things.empty()) {
                                                    continue;
                                                }
                                                
                                                auto idx = MTT_Random_range(0, entry->things.size());
                                                auto it = entry->things.begin();
                                                for (isize i = 0; i < idx; i += 1) {
                                                    ++it;
                                                }
                                                mtt::Thing_ID choice_id = it->first;
                                                
                                                out = mtt::Thing_copy_recursively(world, choice_id);
                                                if (out == nullptr) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_save_as_preset(out);
                                            }
                                            
                                            Thing_set_position(out, *pos);
                                            
                                            if (!src_entries->empty()) {
                                                for (const auto& el : *src_entries) {
                                                    auto& val = el.value;
                                                    if (val.type != MTT_THING) {
                                                        continue;
                                                    }
                                                    mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                                    if (src == nullptr) {
                                                        continue;
                                                    }
                                                    
                                                    Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                                    
                                                }
                                            }
                                            Thing_add_action_event_instantaneous(thing_dst, action_entry_into, out, VERB_EVENT_BEGIN);
                                            
    //                                            Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                        }
                                        
                                    }
                                }
                                
                                bool transformed = false;
                                if (!to_transform_into_labels.empty()) {
                                    for (usize i = 0; i < to_transform.size(); i += 1) {
                                        mtt::Thing* thing_src = mtt::Thing_try_get(world, to_transform[i].thing_id);
                                        if (thing_src == nullptr) {
                                            continue;
                                        }
                                        
                                        vec3* pos = mtt::access<vec3>(thing_src, "position");
                                        if (pos == nullptr) {
                                            continue;
                                        }
                                        
                                        usize j = m::min((usize)i, (usize)(to_transform_into_labels.size() - 1));
                                        
                                        const mtt::String& label = to_transform_into_labels[j].type;
                                        
                                        mtt::Thing* out = (to_transform_into_labels[j].modifiers.empty()) ? mtt::Thing_make_from_preset(world, label) : mtt::Thing_make_from_preset(world, label, to_Array_Slice(to_transform_into_labels[j].modifiers));
                                        if (out == nullptr) {
                                            auto* entry = noun_add(label);
                                            if (entry->things.empty()) {
                                                continue;
                                            }
                                            
                                            auto idx = MTT_Random_range(0, entry->things.size());
                                            auto it = entry->things.begin();
                                            for (isize i = 0; i < idx; i += 1) {
                                                ++it;
                                            }
                                            mtt::Thing_ID choice_id = it->first;
                                            
                                            out = mtt::Thing_copy_recursively(world, choice_id);
                                            if (out == nullptr) {
                                                continue;
                                            }
                                            
                                            mtt::Thing_save_as_preset(out);
                                        }
                                        
                                        Thing_set_position(out, *pos);
                                        
                                        {
                                            Thing_overwrite_root_representation(thing_src, out);
                                            
                                            unset_should_render(out);
                                            
                                            Thing_destroy(world, out);
                                        }
                                        
                                        transformed = true;
                                        
                                        if (!src_entries->empty()) {
                                            for (const auto& el : *src_entries) {
                                                auto& val = el.value;
                                                if (val.type != MTT_THING) {
                                                    continue;
                                                }
                                                mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                Thing_add_action_event_instantaneous(src, action_entry, out, VERB_EVENT_BEGIN);
                                            }
                                        }
                                        
                                        Thing_add_action_event_instantaneous(thing_src, action_entry_into, out, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(thing_src, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
                                        for (usize mod_idx = 0; mod_idx < to_transform_into_labels[j].modifiers.size(); mod_idx += 1) {
//                                            Thing_add_attribute_event_instantaneous(thing_src, action_entry_become, attribute_lookup(to_transform_into_labels[j].modifiers[mod_idx]), VERB_EVENT_BEGIN);
                                            Thing_add_attribute_event_instantaneous(thing_src, action_entry_appear, attribute_lookup(to_transform_into_labels[j].modifiers[mod_idx]), VERB_EVENT_BEGIN);
                                            Thing_add_attribute_event_instantaneous(thing_src, action_entry_into, attribute_lookup(to_transform_into_labels[j].modifiers[mod_idx]), VERB_EVENT_BEGIN);
                                        }
                                    }
                                }
                                
                                if (!transformed && !to_transform_into.empty()) {
                                    for (usize i = 0; i < to_transform.size(); i += 1) {
                                        mtt::Thing* thing_src = mtt::Thing_try_get(world, to_transform[i].thing_id);
                                        if (thing_src == nullptr) {
                                            continue;
                                        }
                                        
                                        usize j = m::min((usize)i, (usize)(to_transform_into.size() - 1));
                                        
                                        //for (usize j = 0; j < to_transform_into.size(); j += 1)
                                        {
                                            mtt::Thing* thing_dst = mtt::Thing_try_get(world, to_transform_into[j].thing_id);
                                            if (thing_dst == nullptr) {
                                                continue;
                                            }
                                            
                                            //                                        vec3* pos = mtt::access<vec3>(thing_dst, "position");
                                            //                                        if (pos == nullptr) {
                                            //                                            continue;
                                            //                                        }
                                            //
                                            //                                        mtt::Thing* out = mtt::Thing_copy_recursively(thing_src);
                                            //                                        if (out == nullptr) {
                                            //                                            continue;
                                            //                                        }
                                            //
                                            //                                        Thing_set_position(out, *pos);
                                            
                                            
                                            
                                            // overwrite root of source
                                            Thing_overwrite_root_representation(thing_src, thing_dst);
                                            
                                            Thing_add_action_event_instantaneous(thing_src, action_entry, thing_dst, VERB_EVENT_BEGIN);
                                            Thing_add_action_event_instantaneous(thing_src, action_entry_into, thing_dst, VERB_EVENT_BEGIN);
                                            Thing_add_action_event_instantaneous(thing_src, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
    //                                            if (!src_entries->empty()) {
    //                                                for (const auto& el : *src_entries) {
    //                                                    auto& val = el.value;
    //                                                    if (val.type != MTT_THING) {
    //                                                        continue;
    //                                                    }
    //                                                    mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
    //                                                    if (src == nullptr) {
    //                                                        continue;
    //                                                    }
    //
    //
    //                                                }
    //                                            }
                                            
    //                                            Script_Instance_append_return_value(s_to_return, mtt::Any::from_Thing_ID(mtt::thing_id(out)));
                                        }
                                    }
                                }
                            } else {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
        
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    }
    
    {
        define_action(cat, "become", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
    //                    mtt::begin_parallel_group(scb);
    //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                                
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = false, .treat_direct_objects_as_types = true, .treat_prepositions_as_types = true}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            mtt::Thing* caller_thing = mtt::Thing_try_get(world, script->caller);
                            if (caller_thing == nullptr) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            Call_Descriptor* call = mtt::access<Call_Descriptor>(caller_thing, "state");
                            
                            auto& modifiers = call->modifiers;
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* tgt_entries = Script_Lookup_get_var_with_key(script, ARG_target, DEFAULT_LOOKUP_SCOPE);
//
                            
                            if (src_entries != nullptr && !src_entries->empty()) {
                                auto* action_entry = verb_add("become");
                                auto* action_entry_appear = verb_add("appear");
                                auto* action_entry_disappear = verb_add("disappear");
                                for (auto& val : *src_entries) {
                                    if (val.value.type == MTT_THING) {
                                        
                                        mtt::Thing_ID id = val.value.thing_id;
                                        mtt::Thing* thing = mtt::Thing_try_get(world, id);
                                        if (thing == nullptr) {
                                            continue;
                                        }
                                        
                                        for (usize i = 0; i < modifiers.size(); i += 1) {
                                            auto& modifier = modifiers[i];
                                            auto* el = attribute_add(modifier.label);
                                            if (!modifier.is_negated) {
                                                Thing_add_attribute(thing, el);
                                                
                                                Thing_add_attribute_event_instantaneous(thing, action_entry, el, VERB_EVENT_BEGIN);
                                                Thing_add_attribute_event_instantaneous(thing, action_entry_appear, el, VERB_EVENT_BEGIN);
                                            } else {
                                                if (Thing_has_attribute(thing, el)) {
                                                    Thing_remove_attribute(thing, el);
//                                                    Thing_add_attribute_event_instantaneous(thing, action_entry, el, VERB_EVENT_END);
//                                                    Thing_add_attribute_event_instantaneous(thing, action_entry_disappear, el, VERB_EVENT_BEGIN);
                                                }
                                            }
                                        }
                                        
                                        auto* dt = dt::DrawTalk::ctx();
                                        if (tgt_entries != nullptr && !tgt_entries->empty()) {
                                            for (auto& val : *tgt_entries) {
                                                if (val.value.type == MTT_STRING) {
                                                    const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                                    vis_word_derive_from(thing, noun_add(str));
//                                                    auto* action_entry = verb_add("become" + str);
                                                    Thing_add_action_event_instantaneous(thing, action_entry, nullptr, VERB_EVENT_BEGIN);
                                                    Thing_add_action_event_instantaneous(thing, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
                                                } else if (val.value.type == MTT_THING) {
                                                    auto& named_things = dt->lang_ctx.dictionary.thing_to_word;
                                                    mtt::Thing* thing_tgt = mtt::Thing_try_get(world, val.value.thing_id);
                                                    if (thing_tgt == nullptr) {
                                                        continue;
                                                    }
                                                    
                                                    auto find_it = named_things.find(val.value.thing_id);
                                                    if (find_it == named_things.end()) {
                                                        continue;
                                                    }
                                                    
                                                    auto& words = find_it->second;
                                                    
                                                    for (auto w = words.begin(); w != words.end(); ++w) {
                                                        dt::Word_Dictionary_Entry* entry = (*w);
                                                        const mtt::String cpp_string = ((entry->name));
                                                        if (entry->name == "thing") {
                                                            continue;
                                                        }
                                                        vis_word_derive_from(thing, noun_add(cpp_string));
                                                        
                                                        Thing_add_action_event_instantaneous(thing, action_entry, nullptr, VERB_EVENT_BEGIN);
                                                        Thing_add_action_event_instantaneous(thing, action_entry_appear, nullptr, VERB_EVENT_BEGIN);
                                                        
                                                        
                                                    }
                                                }
                                            }
                                        }
                                    }
//                                    else if (val.value.type == MTT_STRING) {
//                                        
//                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
//                                        to_transform_labels.push_back(str);
//                                    }
                                }
                            } else {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
        
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    }
    
    define_action_alias(cat, "is", "", "become", "");
    define_action_alias(cat, "be", "", "become", "");
    
    define_action(cat, "form", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    

    
    {
        define_action(cat, "equal", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
    //                    mtt::begin_parallel_group(scb);
    //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                                
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = false}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            mtt::Thing* caller_thing = mtt::Thing_try_get(world, script->caller);
                            if (caller_thing == nullptr) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            Call_Descriptor* call = mtt::access<Call_Descriptor>(caller_thing, "state");
                            
                            auto& modifiers = call->modifiers;
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
//
                            
                            if (src_entries != nullptr && !src_entries->empty()) {
                                //auto* action_entry = verb_add("become");
                                for (auto& val : *src_entries) {
                                    if (val.value.type == MTT_THING) {
                                        
                                        mtt::Thing_ID id = val.value.thing_id;
                                        mtt::Thing* thing = mtt::Thing_try_get(world, id);
                                        if (thing == nullptr) {
                                            continue;
                                        }
                                        
                                        auto type = mtt::thing_type_id(thing);
                                        
                                        if (type == mtt::ARCHETYPE_NUMBER) {
                                            float64 counter = val.counter;
                                            
                                            
                                            number_update_value(thing, counter);
                                        }
                                        
//                                        for (usize i = 0; i < modifiers.size(); i += 1) {
//                                            auto* el = attribute_add(modifiers[i]);
//                                            Thing_add_attribute(thing, el);
//                                            
//                                            Thing_add_attribute_event_instantaneous(thing, action_entry, el, VERB_EVENT_BEGIN);
//                                        }
                                    }
//                                    else if (val.value.type == MTT_STRING) {
//
//                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
//                                        to_transform_labels.push_back(str);
//                                    }
                                }
                            } else {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
        
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    }
    // TODO: ...
    {
        define_action(cat, "count", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
    //                    mtt::begin_parallel_group(scb);
    //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                                
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = false, .treat_direct_objects_as_types = true}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            mtt::Thing* caller_thing = mtt::Thing_try_get(world, script->caller);
                            if (caller_thing == nullptr) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            Call_Descriptor* call = mtt::access<Call_Descriptor>(caller_thing, "state");
                            
                            auto& modifiers = call->modifiers;
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            auto* tgt_entries = Script_Lookup_get_var_with_key(script, ARG_target, DEFAULT_LOOKUP_SCOPE);
//
                            
                            if (src_entries != nullptr && !src_entries->empty()) {
                                //auto* action_entry = verb_add("become");
                                for (auto& val : *src_entries) {
                                    if (val.value.type == MTT_THING) {
                                        
                                        mtt::Thing_ID id = val.value.thing_id;
                                        mtt::Thing* thing = mtt::Thing_try_get(world, id);
                                        if (thing == nullptr) {
                                            continue;
                                        }
                                        
                                        auto type = mtt::thing_type_id(thing);
                                        
                                        if (type == mtt::ARCHETYPE_NUMBER) {
                                            float64 counter = val.counter;
                                            
                                            if (tgt_entries != nullptr && !tgt_entries->empty()) {
                                                for (auto& val : *tgt_entries) {
                                                    if (val.value.type == MTT_STRING) {
                                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                                        
                                                        {
                                                            
                                                        }
                                                    }
                                                }
                                            }
                                            
                                            number_update_value(thing, counter);
                                        } else {
                                            // ... UNUSED CASE
                                        }
                                        
//                                        for (usize i = 0; i < modifiers.size(); i += 1) {
//                                            auto* el = attribute_add(modifiers[i]);
//                                            Thing_add_attribute(thing, el);
//
//                                            Thing_add_attribute_event_instantaneous(thing, action_entry, el, VERB_EVENT_BEGIN);
//                                        }
                                    } else if (val.value.type == MTT_STRING) {
                                        // ... UNUSED CASE
                                    }
//                                    else if (val.value.type == MTT_STRING) {
//
//                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
//                                        to_transform_labels.push_back(str);
//                                    }
                                }
                            } else {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
        
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    }
    // TODO: ...
    {
        define_action(cat, "exist", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
    //                    mtt::begin_parallel_group(scb);
    //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                                
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = false}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            mtt::Thing* caller_thing = mtt::Thing_try_get(world, script->caller);
                            if (caller_thing == nullptr) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            Call_Descriptor* call = mtt::access<Call_Descriptor>(caller_thing, "state");
                            
                            auto& modifiers = call->modifiers;
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
//
                            
                            if (src_entries != nullptr && !src_entries->empty()) {
                                //auto* action_entry = verb_add("become");
                                for (auto& val : *src_entries) {
                                    if (val.value.type == MTT_THING) {
                                        
                                        mtt::Thing_ID id = val.value.thing_id;
                                        mtt::Thing* thing = mtt::Thing_try_get(world, id);
                                        if (thing == nullptr) {
                                            continue;
                                        }
                                        
                                        auto type = mtt::thing_type_id(thing);
                                        
                                        if (type == mtt::ARCHETYPE_NUMBER) {
                                            float64 counter = val.counter;
                                            
                                            
                                            number_update_value(thing, counter);
                                        }
                                        
//                                        for (usize i = 0; i < modifiers.size(); i += 1) {
//                                            auto* el = attribute_add(modifiers[i]);
//                                            Thing_add_attribute(thing, el);
//
//                                            Thing_add_attribute_event_instantaneous(thing, action_entry, el, VERB_EVENT_BEGIN);
//                                        }
                                    }
//                                    else if (val.value.type == MTT_STRING) {
//
//                                        const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
//                                        to_transform_labels.push_back(str);
//                                    }
                                }
                            } else {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
        
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    }
    {
        define_action(cat, "stop", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
//                End_Parallel_Group_Args pg_args = {};
//                pg_args.reset_on_looparound = true;
//                pg_args.has_count = true;
//                pg_args.count = 1;
                auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            
            Script* script_target = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            
            script_target->label = def.key_primary;
            def.register_script(ARG_target, script_target);
            
            Script* scripts_common_args[] = {script_agent, script_target};
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_target;
                }
                
                {
                    
                    call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                        if (call->source_script_id == Script_ID_INVALID) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                        }
                       
                        Script* script_to_call = Script_lookup(call->source_script_id);
                        if (script_to_call == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                        }
                        
                        auto& plist = call->params().param_list(0);
                        auto& params_lookup = plist.lookup;
                        auto& shared_lookup = script_to_call->lookup();
                        auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                        
                        auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                        auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                        auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                        bool has_tgts = false;
                        bool has_objs = false;
                        
                        mtt::World* world = mtt::ctx();
                        

                        mtt::Set<mtt::Thing_ID> to_stop = {};
                        mtt::Set<mtt::String> to_stop_labels = {};
                        if (tgt_entries != nullptr && !tgt_entries->empty()) {
                            
                            
                            for (auto& val : *tgt_entries) {
                                if (val.value.type == MTT_THING) {
                                    has_tgts = true;
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_stop.insert(id);
                                } else if (val.value.type == MTT_STRING) {
                                    const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                    to_stop_labels.insert(str);
                                }
                            }
                        }
                        if (obj_entries != nullptr && !obj_entries->empty()) {
                            has_objs = true;
                            
                            for (auto& val : *obj_entries) {
                                if (val.value.type == MTT_THING) {
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_stop.insert(id);
                                }
                            }
                        }
                        if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                            for (auto& val : *src_entries) {
                                if (val.value.type == MTT_THING) {
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_stop.insert(id);
                                }
                            }
                        }
                        
                        
                        auto* rtime = Runtime::ctx();
                    
                        auto& task_list = rtime->script_tasks.list;
                        //MTT_print("TASK COUNT: %lu\n", task_list.size());
                        for (isize i = task_list.size() - 1; i >= 0; ) {
                            
                            auto* task = task_list[i];
#ifndef NDEBUG
                            Script_Lookup_print(task->lookup());
#endif
      
                            if (to_stop.contains(task->agent) && (to_stop_labels.empty() || to_stop_labels.contains(task->label))) {
                                
                         
                                //task->status = SCRIPT_STATUS_DONE_SHOULD_TERMINATE;
                                
                                
//                                mtt::add_args_to_stop(task, [&](auto& container) {
//                                    container.push_back(task->agent);
//                                });
//

                                mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), task->agent);
                                
                                if (thing != nullptr) {
                                    vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                                    if (velocity != nullptr) {
                                        *velocity = vec3(0.0f);
                                    }
                                    vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                                    if (acceleration != nullptr) {
                                        *acceleration = vec3(0.0f);
                                    }
                                }
                                
                                //mtt::Thing* caller = mtt::Thing_try_get(world, task->caller);
                                
                                //mtt::Call_Descriptor* call_desc = mtt::access<Call_Descriptor>(caller, "state");
                                
    //                            if (task->on_done) {
    //                                task->on_done(world, task, nullptr);
    //                            }
                                
                                //Script_Instance_cancel(task);
                                //Script_Instance_terminate(task);
                                
                                // TODO:
                                //ASSERT_MSG(false, "finish the stop");
                                
                                
                                bool something_found = false;

                                std::vector<Script_Instance*> to_stop_stack = {task};
                                std::swap(task_list[i], task_list[task_list.size() - 1]);
                                task_list.pop_back();
                                do {
                                    do {
                                        something_found = false;
                                        for (isize j = task_list.size() - 1; j >= 0; j -= 1) {
                                            auto* task_j = task_list[j];
                                            if (task_j->parent == to_stop_stack.back()) {
                                                to_stop_stack.push_back(task_j);
                                                something_found = true;

                                                std::swap(task_list[j], task_list[task_list.size() - 1]);
                                                task_list.pop_back();
                                            }
                                        }
                                    } while (something_found);
                                    
                                    Script_Instance_stop(to_stop_stack.back());
                                    to_stop_stack.pop_back();
                                } while (!to_stop_stack.empty());
                                
//                                Script_Instance_stop(task);
//                                
                                if (!task_list.empty()) {
                                    i = task_list.size() - 1;
                                }
                            } else {
                                i -= 1;
                            }
                        }
                        
                        auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                            script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                                
                                
                                Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                                
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                      //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                                
                    
                                
                                activate_actions(script, id);

                                
                                

                                
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                            };
                            script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                                auto* t = mtt::Thing_try_get(world, script->agent);
                                if (t == nullptr) {
                                    script->remove_actions();
                                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                                }
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            };
                            script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            };
                            script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                            };
                            script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                            };
                        }, args);
                        

                        
                        
                        return out;
                    };
                }
                
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    {
        define_action(cat, "stay", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
//                End_Parallel_Group_Args pg_args = {};
//                pg_args.reset_on_looparound = true;
//                pg_args.has_count = true;
//                pg_args.count = 1;
                auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            
            Script* script_target = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            
            script_target->label = def.key_primary;
            def.register_script(ARG_target, script_target);
            
            Script* scripts_common_args[] = {script_agent, script_target};
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_target;
                }
                
                {
                    
                    call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                        if (call->source_script_id == Script_ID_INVALID) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                        }
                       
                        Script* script_to_call = Script_lookup(call->source_script_id);
                        if (script_to_call == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                        }
                        
                        auto& plist = call->params().param_list(0);
                        auto& params_lookup = plist.lookup;
                        auto& shared_lookup = script_to_call->lookup();
                        auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                        
                        auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                        auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                        auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                        bool has_tgts = false;
                        bool has_objs = false;
                        
                        mtt::World* world = mtt::ctx();
                        

                        mtt::Set<mtt::Thing_ID> to_stop = {};
                        mtt::Set<mtt::String> to_stop_labels = {};
                        if (tgt_entries != nullptr && !tgt_entries->empty()) {
                            
                            
                            for (auto& val : *tgt_entries) {
                                if (val.value.type == MTT_THING) {
                                    has_tgts = true;
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_stop.insert(id);
                                } else if (val.value.type == MTT_STRING) {
                                    const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                    to_stop_labels.insert(str);
                                }
                            }
                        }
                        if (obj_entries != nullptr && !obj_entries->empty()) {
                            has_objs = true;
                            
                            for (auto& val : *obj_entries) {
                                if (val.value.type == MTT_THING) {
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_stop.insert(id);
                                }
                            }
                        }
                        if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                            for (auto& val : *src_entries) {
                                if (val.value.type == MTT_THING) {
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_stop.insert(id);
                                }
                            }
                        }
                        
                        for (auto& el : to_stop) {
                            mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), el);
                            
                            if (thing != nullptr) {
                                vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                                if (velocity != nullptr) {
                                    *velocity = vec3(0.0f);
                                }
                                vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                                if (acceleration != nullptr) {
                                    *acceleration = vec3(0.0f);
                                }
                            }
                        }
                        
                        auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                            script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                                
                                
                                Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                                
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                      //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                                
                    
                                
                                activate_actions(script, id);

                                
                                

                                
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                            };
                            script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                                auto* t = mtt::Thing_try_get(world, script->agent);
                                if (t == nullptr) {
                                    script->remove_actions();
                                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                                }
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            };
                            script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            };
                            script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                            };
                            script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                            };
                        }, args);
                        

                        
                        
                        return out;
                    };
                }
                
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    define_action(cat, "need", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "have", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "show", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "hide", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "enter", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "exit", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
//    define_action(cat, "sail", "", [&](auto& def) {
//        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
//
//        });
//    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
//
//
//
//        return mtt::Result_make(true);
//    });
    define_action(cat, "find", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "flow", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "fear", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    

    {
        define_action(cat, "teleport", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        //                    mtt::begin_parallel_group(scb);
                        //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                    
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = true}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            auto* tgt_entries = Script_Lookup_get_var_with_key(script, ARG_target, DEFAULT_LOOKUP_SCOPE);
                            auto* obj_entries = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                            auto* dir_entries = Script_Lookup_get_var_with_key(script, mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                            auto* spatial_entries = Script_Lookup_get_var_with_key(script, mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE);
                            bool has_tgts = false;
                            bool has_objs = false;
                            
                            bool has_dir = false;
                            bool has_spatial = false;
                            
                            if (src_entries == nullptr || src_entries->empty()) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            if (tgt_entries != nullptr && !tgt_entries->empty()) {
                                has_tgts = true;
                                //
                                //                            for (auto& val : *tgt_entries) {
                                //                                if (val.value.type == MTT_THING) {
                                //                                    mtt::Thing_ID id = val.value.thing_id;
                                //                                    to_destroy.insert(id);
                                //                                }
                                //                            }
                            }
                            if (obj_entries != nullptr && !obj_entries->empty()) {
                                has_objs = true;
                                
                                //                            for (auto& val : *obj_entries) {
                                //                                if (val.value.type == MTT_THING) {
                                //                                    mtt::Thing_ID id = val.value.thing_id;
                                //                                    to_destroy.insert(id);
                                //                                }
                                //                            }
                            }
                            
                            if (has_objs) {
                                auto* from_entries_ptr = src_entries;
                                if (has_tgts) {
                                    from_entries_ptr = tgt_entries;
                                }
                                
                                auto& from = *from_entries_ptr;
                                auto& to = *obj_entries;
                                
                                usize from_count = from.size();
                                usize to_count = to.size();
                                
                                if (to_count == 1) {
                                    auto& val = to[0];
                                    if (val.value.type != MTT_THING) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    mtt::Thing_ID id = val.value.thing_id;
                                    
                                    mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                    if (dst == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    
                                    vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                    if (dst_pos_ptr == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    vec3 dst_pos = *dst_pos_ptr;
                                    bool should_offset = (val.selector_name == ARG_object_offset_selector);
                                    
                                    
                                    
                                    for (usize i = 0; i < from_count; i += 1) {
                                        auto& val_src = from[i];
                                        if (val_src.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID src_id = val_src.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        if (should_offset) {
                                            // HARD-CODE assume teleport up or down
                                            {
                                                Message msg;
                                                msg.sender   = mtt::thing_id(src);
                                                msg.selector = string_ref_get(val.selector_value.c_str());
                                                selector_invoke(dst, &msg);
                                                dst_pos = msg.output_value.Vector3;
                                            }
                                            
                                            float32 vertical_offset = 0;
                                            if (val.selector_value == "top_with_offset") {
                                                vertical_offset -= 4;
                                            } else if (val.selector_value == "bottom_with_offset") {
                                                vertical_offset += 4;
                                            }
                                            
                                            
                                            {
                                                vec3 final_pos = dst_pos;
                                                if (Things_are_overlapping(world, src, dst)) {
                                                    final_pos.x = mtt::access<vec3>(src, "position")->x;
                                                }
                                                
                                                
                                                final_pos.y += vertical_offset;
                                                
                                                mtt::Thing_set_position(src, final_pos);
                                            }
                                            //                                        else {
                                            //                                            mtt::Thing_set_position(src, dst_pos + vec3(0, vertical_offset, 0));
                                            //                                        }
                                        } else {
                                            mtt::Thing_set_position(src, dst_pos);
                                        }
                                    }
                                    
                                } else {
                                    if (to_count > from_count) {
                                        to_count = from_count;
                                    }
                                    
                                    usize min_cap = m::min(from_count, to_count);
                                    usize i = 0;
                                    for (; i < min_cap; i += 1) {
                                        auto& val_to = to[i];
                                        auto& val_from = from[i];
                                        if (val_to.value.type != MTT_THING || val_from.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID dst_id = val_to.value.thing_id;
                                        mtt::Thing* dst = mtt::Thing_try_get(world, dst_id);
                                        if (dst == nullptr) {
                                            continue;
                                        }
                                        mtt::Thing_ID src_id = val_from.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                        if (dst_pos_ptr == nullptr) {
                                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                        }
                                        vec3 dst_pos = *dst_pos_ptr;
                                        
                                        mtt::Thing_set_position(src, dst_pos);
                                        
                                    }
                                    if (i < from_count) {
                                        usize j = 0;
                                        for (; j < to_count; j += 1) {
                                            if (to[j].value.type == MTT_THING) {
                                                break;
                                            }
                                        }
                                        {
                                            auto& val = to[j];
                                            if (val.value.type != MTT_THING) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            mtt::Thing_ID id = val.value.thing_id;
                                            mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                            if (dst == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            
                                            vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                            if (dst_pos_ptr == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            vec3 dst_pos = *dst_pos_ptr;
                                            
                                            for (; i < from_count; i += 1) {
                                                auto& val_src = from[i];
                                                if (val.value.type != MTT_THING) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_ID src_id = val.value.thing_id;
                                                mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                                
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_set_position(src, dst_pos);
                                            }
                                        }
                                    }
                                    
                                }
                                
                            } else {
                                // TODO: maybe never, but could want to teleport in a direction
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            //                        if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                            //                            for (auto& val : *src_entries) {
                            //                                if (val.value.type == MTT_THING) {
                            //                                    mtt::Thing_ID id = val.value.thing_id;
                            //                                    to_destroy.insert(id);
                            //                                }
                            //                            }
                            //                        }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            //                        auto& to_destroy = *(mtt::Set<mtt::Thing_ID>*)args;
                            //                        for (auto thing_id : to_destroy) {
                            //                            if (!mtt::Thing_try_get(world, thing_id)) {
                            //                                continue;
                            //                            }
                            //
                            //                            Destroy_Command& cmd = world->to_destroy.emplace_back();
                            //                            cmd.thing_id = thing_id;
                            //                            cmd.affects_connected = false;
                            //                            cmd.do_fade_animation = false;
                            //                            cmd.time_delay = 0;
                            //                            cmd.frame_delay = 1;
                            //                            //cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
                            //                            cmd.time_remaining = 0;//cmd.time_delay;
                            //                        }
                            //                        delete (&to_destroy);
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
    }
    define_action_alias(cat, "warp", "", "teleport", "");
    
    define_action(cat, "connect", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "disconnect", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });

    define_action(cat, "grow", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    define_action(cat, "shrink", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });

    

    define_action(cat, "face", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });

    
    define_action(cat, "swing", "", [&](auto& def) {
        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
            
        });
    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
        
        
        
        return mtt::Result_make(true);
    });
    
//    define_action(cat, "ride", "", [&](auto& def) {
//        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
//
//        });
//    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
//
//
//
//        return mtt::Result_make(true);
//    });
    
    {
        define_action(cat, "activate", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            def.register_script(mtt::ARG_source, script_agent);
            def.register_script(mtt::ARG_object, script_agent);
            def.register_script(mtt::ARG_target, script_agent);
            
            Script* scripts_common_args[] = {script_agent};
            for (usize i = 0; i < 1; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            

            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                   
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                    auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                    bool has_tgts = false;
                    bool has_objs = false;
                    
                    mtt::World* world = mtt::ctx();
                    
                    
                    mtt::Set<mtt::Thing_ID> to_destroy = {};
                    if (tgt_entries != nullptr && !tgt_entries->empty()) {
                        has_tgts = true;
                        
                        for (auto& val : *tgt_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    if (obj_entries != nullptr && !obj_entries->empty()) {
                        has_objs = true;
                        
                        for (auto& val : *obj_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    if (src_entries != nullptr && !src_entries->empty()) {
                        if (!has_tgts && !has_objs) {
                            for (auto& val : *src_entries) {
                                if (val.value.type == MTT_THING) {
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_destroy.insert(id);
                                }
                            }
                            for (auto thing_id : to_destroy) {
                                mtt::Thing_apply_self_and_children(world, thing_id, [](mtt::World* world, mtt::Thing* thing, void* args) {
                                    auto* action_entry = verb_add("activate");
                                    mtt::set_is_active(thing);
                                    mtt::set_should_render(thing);
                                    mtt::unset_flag(thing, THING_FLAG_is_paused);
                                    mtt::Thing_set_activate_colliders(thing);
                                    Thing_add_action_event_instantaneous(thing, action_entry, nullptr, VERB_EVENT_BEGIN);
                                }, nullptr);
                            }
                        } else {
                            for (auto thing_id : to_destroy) {
                                mtt::Thing_apply_self_and_children(world, thing_id, [](mtt::World* world, mtt::Thing* thing, void* args) {
                                    auto* action_entry = verb_add("activate");
                                    mtt::set_is_active(thing);
                                    mtt::set_should_render(thing);
                                    mtt::unset_flag(thing, THING_FLAG_is_paused);
                                    mtt::Thing_set_activate_colliders(thing);
                                    
                                    Script_Property_List& src_list = *(Script_Property_List*)args;
                                    
                                    for (auto& el : src_list) {
                                        mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        Thing_add_action_event_instantaneous(src, action_entry, thing, VERB_EVENT_BEGIN);
                                    }
                                }, src_entries);
                            }
                        }
                    }
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            // FIXME: this will be invalid if there's no real arg source... need some additional cases
                            Script_Property& prop = (*res)[0];
                            mtt::Any& val = prop.value;
                            auto id = val.thing_id;
                  //          auto& selector_name = prop.selector_name;
                //            auto& selector_value = prop.selector_value;
                            
                
                            
                            activate_actions(script, id);

                            
                            

                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    

                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    {
        define_action(cat, "deactivate", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            def.register_script(mtt::ARG_source, script_agent);
            def.register_script(mtt::ARG_object, script_agent);
            def.register_script(mtt::ARG_target, script_agent);
            
            Script* scripts_common_args[] = {script_agent};
            for (usize i = 0; i < 1; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            

            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                   
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                    auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                    bool has_tgts = false;
                    bool has_objs = false;
                    
                    mtt::World* world = mtt::ctx();
                    
                    
                    mtt::Set<mtt::Thing_ID> to_destroy = {};
                    if (tgt_entries != nullptr && !tgt_entries->empty()) {
                        has_tgts = true;
                        
                        for (auto& val : *tgt_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    if (obj_entries != nullptr && !obj_entries->empty()) {
                        has_objs = true;
                        
                        for (auto& val : *obj_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                to_destroy.insert(id);
                            }
                        }
                    }
                    
                    
                    
//                    auto* twn_ctx = mtt_core_ctx()->tween_ctx;
//                    twn_Opt opt = {};
//                    opt.time = (call->has_time_interval) ? call->time_interval : 0.0f;
//                    opt.ease = TWN_QUADINOUT;
//                    opt.do_forwards_and_backwards = 0;
//                    opt.abort_if_in_progress = 1;
                    
                    
                    if (src_entries != nullptr && !src_entries->empty()) {
                        if (!has_tgts && !has_objs) {
                            for (auto& val : *src_entries) {
                                if (val.value.type == MTT_THING) {
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_destroy.insert(id);
                                }
                            }
                            for (auto thing_id : to_destroy) {
                                mtt::Thing_apply_self_and_children(world, thing_id, [](mtt::World* world, mtt::Thing* thing, void* args) {
                                    auto* action_entry = verb_add("deactivate");
                                    mtt::unset_is_active(thing);
                                    mtt::unset_should_render(thing);
                                    mtt::set_flag(thing, THING_FLAG_is_paused);
                                    Thing_deactivate_colliders(thing);
                                    Thing_add_action_event_instantaneous(thing, action_entry, nullptr, VERB_EVENT_BEGIN);
                                }, nullptr);
                            }
                        } else {
                            for (auto thing_id : to_destroy) {
                                mtt::Thing_apply_self_and_children(world, thing_id, [](mtt::World* world, mtt::Thing* thing, void* args) {
                                    auto* action_entry = verb_add("deactivate");
                                    mtt::unset_is_active(thing);
                                    mtt::unset_should_render(thing);
                                    mtt::set_flag(thing, THING_FLAG_is_paused);
                                    Thing_deactivate_colliders(thing);
                                    
                                    Script_Property_List& src_list = *(Script_Property_List*)args;
                                    
                                    for (auto& el : src_list) {
                                        mtt::Thing* src = mtt::Thing_try_get(world, el.value.thing_id);
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        Thing_add_action_event_instantaneous(src, action_entry, thing, VERB_EVENT_BEGIN);
                                    }
                                }, src_entries);
                            }
                        }
                    }
                    
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            Script_Property& prop = (*res)[0];
                            mtt::Any& val = prop.value;
                            auto id = val.thing_id;
                  //          auto& selector_name = prop.selector_name;
                //            auto& selector_value = prop.selector_value;
                            
                
                            
                            activate_actions(script, id);

                            
                            

                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    

                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    {
        define_action(cat, "attach", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            def.register_script(mtt::ARG_source, script_agent);
            def.register_script(mtt::ARG_object, script_agent);
            def.register_script(mtt::ARG_target, script_agent);
            
            Script* scripts_common_args[] = {script_agent};
            for (usize i = 0; i < 1; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            

            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                   
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                    auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                    bool has_tgts = false;
                    bool has_objs = false;
                    
                    mtt::World* world = mtt::ctx();
                    
                    
                    //mtt::Set<mtt::Thing_ID> to_destroy = {};
                    
                    auto* dict = dt::word_dict();
                    
                    auto* action_entry = verb_add("attachto");
                    
                    
                    if (tgt_entries != nullptr && !tgt_entries->empty() && obj_entries != nullptr && !obj_entries->empty() && src_entries != nullptr && !src_entries->empty()) {
                        for (auto& val : *tgt_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                //to_destroy.insert(id);
                                mtt::Thing* child = mtt::Thing_try_get(world, id);
                                if (child == nullptr) {
                                    continue;
                                }
                                
                                for (auto& val : *obj_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID oid = val.value.thing_id;
                                        mtt::Thing* parent = mtt::Thing_try_get(world, oid);
                                        if (parent == nullptr) {
                                            continue;
                                        }
                                        
                                        if (mtt::is_ancestor_of(child, parent)) {
                                            mtt::disconnect_parent_from_children(world, child);
                                        }
                                        
                                        mtt::connect_parent_to_child(world, parent, child);
                                        
                                        Thing_add_action_event_instantaneous(child, action_entry, parent, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(child, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(parent, action_entry, child, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(parent, action_entry, nullptr, VERB_EVENT_BEGIN);
                                    }
                                }
                            }
                        }
                    }
                    else if (tgt_entries != nullptr && !tgt_entries->empty() && src_entries != nullptr && !src_entries->empty()) {
                        has_tgts = true;
                        for (auto& val : *src_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID pid = val.value.thing_id;
                                //to_destroy.insert(id);
                                mtt::Thing* p = mtt::Thing_try_get(world, pid);
                                if (p == nullptr) {
                                    continue;
                                }
                                
                                for (auto& val : *tgt_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        mtt::Thing* c = mtt::Thing_try_get(world, id);
                                        if (c == nullptr) {
                                            continue;
                                        }
                                        
                                        if (mtt::is_ancestor_of(c, p)) {
                                            mtt::disconnect_parent_from_children(world, c);
                                        }
                                        
                                        
                                        mtt::connect_parent_to_child(world, p, c);
                                        Thing_add_action_event_instantaneous(p, action_entry, c, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(p, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(c, action_entry, p, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(c, action_entry, nullptr, VERB_EVENT_BEGIN);
                                    }
                                }
                            }
                        }
                    }
                    else if (obj_entries != nullptr && !obj_entries->empty() && src_entries != nullptr && !src_entries->empty()) {
                        has_objs = true;
                        
                        for (auto& val : *src_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID pid = val.value.thing_id;
                                //to_destroy.insert(id);
                                mtt::Thing* child = mtt::Thing_try_get(world, pid);
                                if (child == nullptr) {
                                    continue;
                                }
                                
                                for (auto& val : *obj_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        mtt::Thing* parent = mtt::Thing_try_get(world, id);
                                        if (parent == nullptr) {
                                            continue;
                                        }
                                        
                                        if (mtt::is_ancestor_of(child, parent)) {
                                            mtt::disconnect_parent_from_children(world, child);
                                        }
                                        
                                        
                                        mtt::connect_parent_to_child(world, parent, child);
                                        Thing_add_action_event_instantaneous(child, action_entry, parent, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(child, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(parent, action_entry, child, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(parent, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        
                                    }
                                }
                            }
                        }
                    } else {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }

//                    auto* twn_ctx = mtt_core_ctx()->tween_ctx;
//                    twn_Opt opt = {};
//                    opt.time = (call->has_time_interval) ? call->time_interval : 0.0f;
//                    opt.ease = TWN_QUADINOUT;
//                    opt.do_forwards_and_backwards = 0;
//                    opt.abort_if_in_progress = 1;
//
//
//
//                    for (auto thing_id : to_destroy) {
////                        Destroy_Command& cmd = world->to_destroy.emplace_back();
////                        cmd.thing_id = thing_id;
////                        cmd.affects_connected = true;
////                        cmd.do_fade_animation = true;
////                        cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
////                        cmd.time_remaining = cmd.time_delay;
//
//                        mtt::Thing* thing = mtt::Thing_try_get(world, thing_id);
//                        if (thing == nullptr) {
//                            continue;
//                        }
//
//                        auto& render_data = mtt::rep(thing)->render_data.drawable_info_list;
//                        for (auto* dr : render_data) {
//
//                            twn_add_f32(twn_ctx, &dr->color_factor[3], 1.0f, opt);
//                        }
//
//
//                    }
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            Script_Property& prop = (*res)[0];
                            mtt::Any& val = prop.value;
                            auto id = val.thing_id;
                  //          auto& selector_name = prop.selector_name;
                //            auto& selector_value = prop.selector_value;
                            
                
                            
                            activate_actions(script, id);

                            
                            

                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    

                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    {
        define_action(cat, "detach", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            def.register_script(mtt::ARG_source, script_agent);
            def.register_script(mtt::ARG_object, script_agent);
            def.register_script(mtt::ARG_target, script_agent);
            
            Script* scripts_common_args[] = {script_agent};
            for (usize i = 0; i < 1; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            

            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                   
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                    auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                    bool has_tgts = false;
                    bool has_objs = false;
                    
                    mtt::World* world = mtt::ctx();
                    
                    
                    //mtt::Set<mtt::Thing_ID> to_destroy = {};
                    
                    auto* dict = dt::word_dict();
                    
                    auto* action_entry = verb_add("detach");
                    
                    
                    if (tgt_entries != nullptr && !tgt_entries->empty() && obj_entries != nullptr && !obj_entries->empty() && src_entries != nullptr && !src_entries->empty()) {
                        for (auto& val : *tgt_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                //to_destroy.insert(id);
                                mtt::Thing* child = mtt::Thing_try_get(world, id);
                                if (child == nullptr) {
                                    continue;
                                }
                                
                                for (auto& val : *obj_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID oid = val.value.thing_id;
                                        mtt::Thing* parent = mtt::Thing_try_get(world, oid);
                                        if (parent == nullptr) {
                                            continue;
                                        }
                                        
                                        if (!mtt::is_ancestor_of(parent, child)) {
                                            disconnect_parent_from_children(world, parent);
                                            Thing_add_action_event_instantaneous(parent, action_entry, nullptr, VERB_EVENT_BEGIN);
                                            continue;
                                            
                                        }
                                        
                                        disconnect_child_from_parent(world, child);
                                        Thing_add_action_event_instantaneous(child, action_entry, parent, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(child, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(parent, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(parent, action_entry, child, VERB_EVENT_BEGIN);
                                    }
                                }
                            }
                        }
                    }
                    else if (tgt_entries != nullptr && !tgt_entries->empty() && src_entries != nullptr && !src_entries->empty()) {
                        has_tgts = true;
                        for (auto& val : *src_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID pid = val.value.thing_id;
                                //to_destroy.insert(id);
                                mtt::Thing* p = mtt::Thing_try_get(world, pid);
                                if (p == nullptr) {
                                    continue;
                                }
                                
                                for (auto& val : *tgt_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        mtt::Thing* c = mtt::Thing_try_get(world, id);
                                        if (c == nullptr) {
                                            continue;
                                        }
                                        
                                        if (!mtt::is_ancestor_of(p, c)) {
                                            disconnect_parent_from_children(world, p);
                                            Thing_add_action_event_instantaneous(p, action_entry, nullptr, VERB_EVENT_BEGIN);
                                            continue;
                                        }
                                        
                                        
                                        disconnect_child_from_parent(world, c);
                                        Thing_add_action_event_instantaneous(c, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(p, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(c, action_entry, p, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(p, action_entry, c, VERB_EVENT_BEGIN);
                                    }
                                }
                            }
                        }
                    }
                    else if (obj_entries != nullptr && !obj_entries->empty() && src_entries != nullptr && !src_entries->empty()) {
                        has_objs = true;
                        
                        for (auto& val : *src_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID pid = val.value.thing_id;
                                //to_destroy.insert(id);
                                mtt::Thing* child = mtt::Thing_try_get(world, pid);
                                if (child == nullptr) {
                                    continue;
                                }
                                
                                for (auto& val : *obj_entries) {
                                    if (val.value.type == MTT_THING) {
                                        mtt::Thing_ID id = val.value.thing_id;
                                        mtt::Thing* parent = mtt::Thing_try_get(world, id);
                                        if (parent == nullptr) {
                                            continue;
                                        }
                                        
                                        if (mtt::is_ancestor_of(child, parent)) {
                                            mtt::disconnect_parent_from_children(world, child);
                                            Thing_add_action_event_instantaneous(child, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        }
                                        
                                        
                                        mtt::disconnect_child_from_parent(world, child);
                                        Thing_add_action_event_instantaneous(child, action_entry, nullptr, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(child, action_entry, parent, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(parent, action_entry, child, VERB_EVENT_BEGIN);
                                        Thing_add_action_event_instantaneous(parent, action_entry, nullptr, VERB_EVENT_BEGIN);
                                    }
                                }
                            }
                        }
                    } else if (src_entries != nullptr) {
                        for (auto& val : *src_entries) {
                            if (val.value.type == MTT_THING) {
                                mtt::Thing_ID id = val.value.thing_id;
                                mtt::Thing* thing = mtt::Thing_try_get(world, id);
                                
                                mtt::disconnect_child_from_parent(world, thing);
                                mtt::disconnect_parent_from_children(world, thing);
                                
                                Thing_add_action_event_instantaneous(thing, action_entry, nullptr, VERB_EVENT_BEGIN);
                                
                            }
                        }
                    } else {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }

//                    auto* twn_ctx = mtt_core_ctx()->tween_ctx;
//                    twn_Opt opt = {};
//                    opt.time = (call->has_time_interval) ? call->time_interval : 0.0f;
//                    opt.ease = TWN_QUADINOUT;
//                    opt.do_forwards_and_backwards = 0;
//                    opt.abort_if_in_progress = 1;
//
//
//
//                    for (auto thing_id : to_destroy) {
////                        Destroy_Command& cmd = world->to_destroy.emplace_back();
////                        cmd.thing_id = thing_id;
////                        cmd.affects_connected = true;
////                        cmd.do_fade_animation = true;
////                        cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
////                        cmd.time_remaining = cmd.time_delay;
//
//                        mtt::Thing* thing = mtt::Thing_try_get(world, thing_id);
//                        if (thing == nullptr) {
//                            continue;
//                        }
//
//                        auto& render_data = mtt::rep(thing)->render_data.drawable_info_list;
//                        for (auto* dr : render_data) {
//
//                            twn_add_f32(twn_ctx, &dr->color_factor[3], 1.0f, opt);
//                        }
//
//
//                    }
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            Script_Property& prop = (*res)[0];
                            mtt::Any& val = prop.value;
                            auto id = val.thing_id;
                  //          auto& selector_name = prop.selector_name;
                //            auto& selector_value = prop.selector_value;
                            
                
                            
                            activate_actions(script, id);

                            
                            

                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    

                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    {
        define_action(cat, "get", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            //script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
//            {
//                DT_Behavior* b = nullptr;
//                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
//                if (cat.lookup("jump", "", &b)) {
//                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
//                    script_only->add_precondition(other->id);
//                } else {
//                    ASSERT_MSG(false, ">?");
//                }
//
//            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "child")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                if (!mtt::add_connection(scb->world, src, "value_single", attach, "parent")) {
                                    ASSERT_MSG(false, "?");
                                }

                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,           targets_script);
            
            Script* targets_objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "child")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                if (!mtt::add_connection(scb->world, src, "value_single", attach, "parent")) {
                                    ASSERT_MSG(false, "?");
                                }

                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* dst = mtt::lookup_get(scb, ARG_target);

                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
//                                    auto* detach = mtt::thing_make(scb, mtt::ARCHETYPE_DETACH_FROM_PARENT);
//                                    if (!mtt::add_connection(scb->world, mov, "is_done", detach, "is_active")) {
//                                        ASSERT_MSG(false, "?");
//                                    }
//                                    if (!mtt::add_connection(scb->world, dst, "value_single", detach, "child")) {
//                                        ASSERT_MSG(false, "?");
//                                    }
                                auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, dst, "value_single", attach, "child")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "parent")) {
                                    ASSERT_MSG(false, "?");
                                }

                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_objects_script->label = def.key_primary;
            def.register_script(ARG_target + ":" + ARG_object, targets_objects_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script, targets_objects_script};
            for (usize i = 0; i < 4; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_String(ARG_spatial_top.c_str()))
                });
            
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
          //          auto& selector_name = prop.selector_name;
        //            auto& selector_value = prop.selector_value;
                    
        
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        

                        activate_actions(script, id);
                    }
                    
                    

                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            auto* script_w_target_and_object = def.Script_lookup(ARG_target + ":" + ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target_and_object != nullptr, "script should exist!");
            
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects && st.has_prepositions) {
                    script = script_w_target_and_object;
                } else if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    {
        define_action(cat, "climb", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            //script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
//            {
//                DT_Behavior* b = nullptr;
//                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
//                if (cat.lookup("jump", "", &b)) {
//                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
//                    script_only->add_precondition(other->id);
//                } else {
//                    ASSERT_MSG(false, ">?");
//                }
//
//            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        mtt::Thing* dir = mtt::lookup_get(scb, "TOP_DEFAULT");
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        mtt::Thing* dir = mtt::lookup_get(scb, "BOTTOM_DEFAULT");
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
                        mtt::Thing* dir = mtt::lookup_get(scb, "BOTTOM_DEFAULT");
//                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
//                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
//                            ASSERT_MSG(false, "?");
//                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "target_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }
                            
                            auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                                ASSERT_MSG(false, "?");
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
                        mtt::Thing* dir = mtt::lookup_get(scb, "TOP_DEFAULT");
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }

                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "target_selectors");;
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,           targets_script);
            
            Script* targets_objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "child")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                if (!mtt::add_connection(scb->world, src, "value_single", attach, "parent")) {
                                    ASSERT_MSG(false, "?");
                                }

                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* dst = mtt::lookup_get(scb, ARG_target);

                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
//                                    auto* detach = mtt::thing_make(scb, mtt::ARCHETYPE_DETACH_FROM_PARENT);
//                                    if (!mtt::add_connection(scb->world, mov, "is_done", detach, "is_active")) {
//                                        ASSERT_MSG(false, "?");
//                                    }
//                                    if (!mtt::add_connection(scb->world, dst, "value_single", detach, "child")) {
//                                        ASSERT_MSG(false, "?");
//                                    }
                                auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, dst, "value_single", attach, "child")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "parent")) {
                                    ASSERT_MSG(false, "?");
                                }

                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_objects_script->label = def.key_primary;
            def.register_script(ARG_target + ":" + ARG_object, targets_objects_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script, targets_objects_script};
            for (usize i = 0; i < 4; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_String(ARG_spatial_top.c_str()))
                });
                

                

            
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
          //          auto& selector_name = prop.selector_name;
        //            auto& selector_value = prop.selector_value;
                    
        
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        
                        mtt::Script_Lookup_set_key(script->lookup(), "BOTTOM_DEFAULT", mtt::DEFAULT_LOOKUP_SCOPE);
                        mtt::Script_Lookup_set_var_with_key(script->lookup(), "BOTTOM_DEFAULT", mtt::DEFAULT_LOOKUP_SCOPE, {
                            Script_Property_make(mtt::Any::from_String(ARG_spatial_bottom.c_str()))
                        });
                        mtt::Script_Lookup_set_key(script->lookup(), "TOP_DEFAULT", mtt::DEFAULT_LOOKUP_SCOPE);
                        mtt::Script_Lookup_set_var_with_key(script->lookup(), "TOP_DEFAULT", mtt::DEFAULT_LOOKUP_SCOPE, {
                            Script_Property_make(mtt::Any::from_String(ARG_spatial_top.c_str()))
                        });
                        

                        activate_actions(script, id);
                    }
                    
                    

                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            auto* script_w_target_and_object = def.Script_lookup(ARG_target + ":" + ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target_and_object != nullptr, "script should exist!");
            
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects && st.has_prepositions) {
                    script = script_w_target_and_object;
                } else if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    
    {
        define_action(cat, "give", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
//            {
//                DT_Behavior* b = nullptr;
//                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
//                if (cat.lookup("jump", "", &b)) {
//                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
//                    script_only->add_precondition(other->id);
//                } else {
//                    ASSERT_MSG(false, ">?");
//                }
//
//            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                    ASSERT_MSG(false, "?");
                                }
                                if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "child")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                if (!mtt::add_connection(scb->world, src, "value_single", attach, "parent")) {
                                    ASSERT_MSG(false, "?");
                                }

                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,           targets_script);
            
            
            
            
                Script* targets_objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        auto* should_attach = mtt::lookup_get(scb, "should_attach");
                        mtt::begin_parallel_group(scb);
                        {
                            mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
                            
                            
                            auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                            static dt::Word_Val_Pair velocity_words[] = {
                                DEFAULT_ATTRIBUTES
                                {nullptr},
                            };
                            auto* process_velocity_magnitude =
                            dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                            mtt::thing(scb, process_velocity_magnitude);
                            {
                                bool ok = false;
                                
                                ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                                ASSERT_MSG(ok, "???");
                                
                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                                ASSERT_MSG(ok, "???");
                                
                                {
                                    auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                    {

                                        bool ok = mtt::add_connection(mtt::ctx(),
                                                                   should_attach, "value_single", mov, "should_attach");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(),
                                                                      src, "value_single", mov, "source");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    
                                    if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                        ASSERT_MSG(false, "?");
                                    }
                                    
                                    auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                    if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                        ASSERT_MSG(false, "?");
                                    }
                                    if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "child")) {
                                        ASSERT_MSG(false, "?");
                                    }
                                    
                                    if (!mtt::add_connection(scb->world, src, "value_single", attach, "parent")) {
                                        ASSERT_MSG(false, "?");
                                    }

                                    
        //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
        //                            MTT_UNUSED(contin);
                                }
                                
                                auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                                if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                                    ASSERT_MSG(false, "?");
                                }

                            }
                            
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                        
                        mtt::begin_parallel_group(scb);
                        {
                            mtt::Thing* dst = mtt::lookup_get(scb, ARG_target);

                            mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                            auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                            if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                                ASSERT_MSG(false, "?");
                            }
                            
                            auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                            static dt::Word_Val_Pair velocity_words[] = {
                                DEFAULT_ATTRIBUTES
                                {nullptr},
                            };
                            auto* process_velocity_magnitude =
                            dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                            mtt::thing(scb, process_velocity_magnitude);
                            {
                                bool ok = false;
                                
                                ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                                ASSERT_MSG(ok, "???");
                                
                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                                ASSERT_MSG(ok, "???");
                                
                                {
                                    auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                    
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(),
                                                                      src, "value_single", mov, "source");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    
                                    if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                        ASSERT_MSG(false, "?");
                                    }
                                    
//                                    auto* detach = mtt::thing_make(scb, mtt::ARCHETYPE_DETACH_FROM_PARENT);
//                                    if (!mtt::add_connection(scb->world, mov, "is_done", detach, "is_active")) {
//                                        ASSERT_MSG(false, "?");
//                                    }
//                                    if (!mtt::add_connection(scb->world, dst, "value_single", detach, "child")) {
//                                        ASSERT_MSG(false, "?");
//                                    }
                                    auto* attach = mtt::thing_make(scb, mtt::ARCHETYPE_ATTACH_TO_PARENT);
                                    if (!mtt::add_connection(scb->world, mov, "is_done", attach, "is_active")) {
                                        ASSERT_MSG(false, "?");
                                    }
                                    if (!mtt::add_connection(scb->world, dst, "value_single", attach, "child")) {
                                        ASSERT_MSG(false, "?");
                                    }
                                    if (!mtt::add_connection(scb->world, tgt, "value_single", attach, "parent")) {
                                        ASSERT_MSG(false, "?");
                                    }

                                    
        //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
        //                            MTT_UNUSED(contin);
                                }

                            }
                            
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                        
                    }
                    End_Parallel_Group_Args pg_args = {};
                    pg_args.reset_on_looparound = true;
                    pg_args.has_count = true;
                    pg_args.count = 1;
                    auto end = end_parallel_group(scb, pg_args);
                    MTT_UNUSED(end);
                });
                targets_objects_script->label = def.key_primary;
                def.register_script(ARG_target + ":" + ARG_object, targets_objects_script);
            
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script, targets_objects_script};
            for (usize i = 0; i < 4; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_String(ARG_spatial_top.c_str()))
                });
                
                
                
                
            
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
          //          auto& selector_name = prop.selector_name;
        //            auto& selector_value = prop.selector_value;
                    
        
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        

                        activate_actions(script, id);
                    }
                    
                    

                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            mtt::Script_Lookup_set_var_with_key(targets_objects_script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
                Script_Property_make(mtt::Any::from_Boolean(true))
            });
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            auto* script_w_target_and_object = def.Script_lookup(ARG_target + ":" + ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target_and_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects && st.has_prepositions) {
                    script = script_w_target_and_object;
                } else if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    // TODO: maybe take should be different?
    {
        define_action_alias(cat, "take", "", "give", "");
    }
    {
        define_action_alias(cat, "bring", "", "give", "");
    }
    {
        define_action_alias(cat, "transport", "", "give", "");
    }
    {
        define_action_alias(cat, "push", "", "give", "");
    }
//    define_action(cat, "take", "", [&](auto& def) {
//        Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
//
//        });
//    }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
//
//
//
//        return mtt::Result_make(true);
//    });

    
    {
        define_action(cat, "throw", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
//            {
//                DT_Behavior* b = nullptr;
//                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
//                if (cat.lookup("jump", "", &b)) {
//                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
//                    script_only->add_precondition(other->id);
//                } else {
//                    ASSERT_MSG(false, ">?");
//                }
//
//            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                 

                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,           targets_script);
            
            
            
            
                Script* targets_objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::begin_parallel_group(scb);
                        {
                            mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
                            auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_BREAK);
                            if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                                ASSERT_MSG(false, "?");
                            }
                            
                            auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                            static dt::Word_Val_Pair velocity_words[] = {
                                DEFAULT_ATTRIBUTES
                                {nullptr},
                            };
                            auto* process_velocity_magnitude =
                            dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                            mtt::thing(scb, process_velocity_magnitude);
                            {
                                bool ok = false;
                                
                                ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                                ASSERT_MSG(ok, "???");
                                
                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                                ASSERT_MSG(ok, "???");
                                
                                {
                                    auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                    
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(),
                                                                      src, "value_single", mov, "source");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    
                                    if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                        ASSERT_MSG(false, "?");
                                    }
                                                                        
        //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
        //                            MTT_UNUSED(contin);
                                }

                            }
                            
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                        
                        mtt::begin_parallel_group(scb);
                        {
                            mtt::Thing* dst = mtt::lookup_get(scb, ARG_target);

                            mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                            auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                            if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                                ASSERT_MSG(false, "?");
                            }
                            
                            auto* height      = mtt::lookup_get(scb, "height");
                            static dt::Word_Val_Pair height_words[] = {
                                DEFAULT_ATTRIBUTES
                                {nullptr},
                            };
                            auto* process_height = dt::word_multiply_procedure_make(&height_words[0], nullptr);
                            mtt::thing(scb, process_height);
                            {
                                bool ok = false;
                                
                                ok = mtt::add_connection(mtt::ctx(), height, "value_single", process_height, "in");
                                ASSERT_MSG(ok, "???");
                                
                                
                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_height, "in_source");
                                
                                ASSERT_MSG(ok, "???");
                            }
                            
                            auto* speed       = mtt::lookup_get(scb, "speed");
                            static dt::Word_Val_Pair speed_words[] = {
                                DEFAULT_ATTRIBUTES
                                {nullptr},
                            };
                            auto* process_speed = dt::word_multiply_procedure_make(&speed_words[0], nullptr);
                            mtt::thing(scb, process_speed);
                            {
                                bool ok = false;
                                
                                ok = mtt::add_connection(mtt::ctx(), speed, "value_single", process_speed, "in");
                                ASSERT_MSG(ok, "???");
                                
                                ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_speed, "in_source");
                                
                                ASSERT_MSG(ok, "???");
                            }
                            
                            auto* cycle_count = mtt::lookup_get(scb, "cycle_count");
                            {
                                bool ok = false;
                                
                                
                                {
                                    auto* should_attach = mtt::lookup_get(scb, "should_attach");
                                    
                                    auto* jmp = mtt::thing_make(scb, mtt::ARCHETYPE_JUMP);
                                    
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(), process_height, "out", jmp, "height");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(), process_speed, "out", jmp, "speed");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(),
                                                                      should_attach, "value_single", jmp, "should_attach");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(),
                                                                      src, "selector", jmp, "source_selector");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(),
                                                                      tgt, "selector", jmp, "target_selector");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(),
                                                                      dst, "value_single", jmp, "source");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", jmp, "target");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    {
                                        bool ok = mtt::add_connection(mtt::ctx(),
                                                                      cycle_count, "value_single", jmp, "cycle_count");
                                        ASSERT_MSG(ok, "???");
                                    }
                                    
                                    if (!mtt::add_connection(scb->world, jmp, "is_done", tgt, "increment_index")) {
                                        ASSERT_MSG(false, "?");
                                    }
                                }

                            }
                            
                        }
                        mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                        
                    }
                    End_Parallel_Group_Args pg_args = {};
                    pg_args.reset_on_looparound = true;
                    pg_args.has_count = true;
                    pg_args.count = 1;
                    auto end = end_parallel_group(scb, pg_args);
                    MTT_UNUSED(end);
                });
                targets_objects_script->label = def.key_primary;
                def.register_script(ARG_target + ":" + ARG_object, targets_objects_script);
            
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script, targets_objects_script};
            for (usize i = 0; i < 4; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "height", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(200.0f))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "cycle_count",  mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(1.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_String(ARG_spatial_top.c_str()))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "should_attach",  mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Boolean(true))
                });
            
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
          //          auto& selector_name = prop.selector_name;
        //            auto& selector_value = prop.selector_value;
                    
        
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        

                        activate_actions(script, id);
                    }
                    
                    

                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            auto* script_w_target_and_object = def.Script_lookup(ARG_target + ":" + ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target_and_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects && st.has_prepositions) {
                    script = script_w_target_and_object;
                } else if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    {
        define_action(cat, "fall", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* acc      = mtt::lookup_get(scb, "acceleration");
                                
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                

                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), acc, "value_single", mov, "acceleration");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
//            {
//                DT_Behavior* b = nullptr;
//                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
//                if (cat.lookup("jump", "", &b)) {
//                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
//                    script_only->add_precondition(other->id);
//                } else {
//                    ASSERT_MSG(false, ">?");
//                }
//
//            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* acc      = mtt::lookup_get(scb, "acceleration");
                                
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE);
                                

                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), acc, "value_single", mov, "acceleration");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                // TODO: ...
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,           objects_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script};
            for (usize i = 0; i < 3; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(4.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_var_with_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_String(ARG_dir_down.c_str()))
                });
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "acceleration", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Vector3(vec3(0.0f, 9.81, 0.0f)))
                });
            
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
          //          auto& selector_name = prop.selector_name;
        //            auto& selector_value = prop.selector_value;
                    
        
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        

                        activate_actions(script, id);
                    }
                    
                    

                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    {
        define_action(cat, "avoid", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE_AWAY);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_properties", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->allow_duplicates_for_agent = true;
            script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
//            {
//                DT_Behavior* b = nullptr;
//                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
//                if (cat.lookup("jump", "", &b)) {
//                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
//                    script_only->add_precondition(other->id);
//                } else {
//                    ASSERT_MSG(false, ">?");
//                }
//
//            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
//                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
//                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
//                            ASSERT_MSG(false, "?");
//                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE_AWAY);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_properties", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->allow_duplicates_for_agent = true;
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_target);
//                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
//                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
//                            ASSERT_MSG(false, "?");
//                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE_AWAY);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_properties", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->allow_duplicates_for_agent = true;
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,targets_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script};
            for (usize i = 0; i < 3; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
            
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
          //          auto& selector_name = prop.selector_name;
        //            auto& selector_value = prop.selector_value;
                    
        
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        

                        activate_actions(script, id);
                    }
                    
                    

                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    {
        define_action(cat, "repel", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE_AWAY);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_properties", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", mov, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->allow_duplicates_for_agent = true;
            script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
//            {
//                DT_Behavior* b = nullptr;
//                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
//                if (cat.lookup("jump", "", &b)) {
//                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
//                    script_only->add_precondition(other->id);
//                } else {
//                    ASSERT_MSG(false, ">?");
//                }
//
//            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_object);
//                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
//                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
//                            ASSERT_MSG(false, "?");
//                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE_AWAY);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_properties", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->allow_duplicates_for_agent = true;
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_target);
//                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
//                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
//                            ASSERT_MSG(false, "?");
//                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* mov = mtt::thing_make(scb, mtt::ARCHETYPE_MOVE_AWAY);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", mov, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_properties", mov, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", mov, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, mov, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
    //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
    //                            MTT_UNUSED(contin);
                            }

                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->allow_duplicates_for_agent = true;
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,targets_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script};
            for (usize i = 0; i < 3; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
            
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
          //          auto& selector_name = prop.selector_name;
        //            auto& selector_value = prop.selector_value;
                    
        
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        

                        activate_actions(script, id);
                    }
                    
                    

                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    {
        define_action(cat, "rotate", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* rot = mtt::thing_make(scb, mtt::ARCHETYPE_ROTATE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", rot, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", rot, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", rot, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                            MTT_UNUSED(contin);
                            }
                            
                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
            //            {
            //                DT_Behavior* b = nullptr;
            //                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
            //                if (cat.lookup("jump", "", &b)) {
            //                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
            //                    script_only->add_precondition(other->id);
            //                } else {
            //                    ASSERT_MSG(false, ">?");
            //                }
            //
            //            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* rot = mtt::thing_make(scb, mtt::ARCHETYPE_ROTATE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", rot, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", rot, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", rot, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", rot, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, rot, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                            MTT_UNUSED(contin);
                            }
                            
                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                // TODO: ...
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,           objects_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script};
            for (usize i = 0; i < 3; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
        
    }
    {
        define_action_alias(cat, "spin", "", "rotate", "");
    }
    
    {
        define_action(cat, "revolve", "", [&](auto& def) {
            
            Script* script_only = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* rot = mtt::thing_make(scb, mtt::ARCHETYPE_ROTATE);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", rot, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", rot, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", rot, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                            MTT_UNUSED(contin);
                            }
                            
                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            script_only->is_infinite = true;
            script_only->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_only);
            def.register_script(ARG_source, script_only);
            
            //            {
            //                DT_Behavior* b = nullptr;
            //                auto& cat = dt::DrawTalk::ctx()->lang_ctx.behavior_catalogue;
            //                if (cat.lookup("jump", "", &b)) {
            //                    auto* other = b->Script_lookup(DEFAULT_LOOKUP_SCOPE);
            //                    script_only->add_precondition(other->id);
            //                } else {
            //                    ASSERT_MSG(false, ">?");
            //                }
            //
            //            }
            
            Script* objects_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    mtt::begin_parallel_group(scb);
                    {
                        mtt::Thing* src = mtt::lookup_get(scb, ARG_source);
                        mtt::Thing* tgt = mtt::lookup_get(scb, ARG_object);
                        mtt::Thing* dir = mtt::lookup_get(scb, ARG_directional);
                        auto* ret  = mtt::thing_make(scb, mtt::ARCHETYPE_RETURN);
                        if (!mtt::add_connection(scb->world, tgt, "is_done", ret, "is_active")) {
                            ASSERT_MSG(false, "?");
                        }
                        
                        auto* velocity_magnitude      = mtt::lookup_get(scb, "speed");
                        static dt::Word_Val_Pair velocity_words[] = {
                            DEFAULT_ATTRIBUTES
                            {nullptr},
                        };
                        auto* process_velocity_magnitude =
                        dt::word_multiply_procedure_make(&velocity_words[0], nullptr);
                        mtt::thing(scb, process_velocity_magnitude);
                        {
                            bool ok = false;
                            
                            ok = mtt::add_connection(mtt::ctx(), velocity_magnitude, "value_single", process_velocity_magnitude, "in");
                            ASSERT_MSG(ok, "???");
                            
                            ok = mtt::add_connection(mtt::ctx(), src, "value_single", process_velocity_magnitude, "in_source");
                            ASSERT_MSG(ok, "???");
                            
                            {
                                auto* rot = mtt::thing_make(scb, mtt::ARCHETYPE_ROTATE_AROUND);
                                
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), process_velocity_magnitude, "out", rot, "velocity_magnitude");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(),
                                                                  src, "value_single", rot, "source");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), dir, "value_properties", rot, "source_selectors");
                                    ASSERT_MSG(ok, "???");
                                }
                                {
                                    bool ok = mtt::add_connection(mtt::ctx(), tgt, "value_single", rot, "target");
                                    ASSERT_MSG(ok, "???");
                                }
                                
                                if (!mtt::add_connection(scb->world, rot, "is_done", tgt, "increment_index")) {
                                    ASSERT_MSG(false, "?");
                                }
                                
                                //                            auto* contin = mtt::thing_make(scb, mtt::ARCHETYPE_CONTINUE);
                                //                            MTT_UNUSED(contin);
                            }
                            
                        }
                        
                    }
                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            objects_script->label = def.key_primary;
            def.register_script(ARG_object, objects_script);
            
            auto* s_args = Script_shared_args_update(objects_script);
            s_args->data = new dt::Dynamic_Array<mtt::Rotate_Around_Deferred_Args>();
//            s_args->procedure_pre_script_eval = [](Script* s, void* args) {
//                auto* sh_args = (Script_Shared_Args*)args;
//                auto& ops = *(dt::Dynamic_Array<mtt::Rotate_Around_Deferred_Args>*)sh_args->data;
//            };
            s_args->procedure_post_script_eval = [](Script* s, void* args) {
                auto* sh_args = (Script_Shared_Args*)args;
                auto& ops = *(dt::Dynamic_Array<mtt::Rotate_Around_Deferred_Args>*)sh_args->data;
                
                // TODO:
                
                std::stable_sort(ops.begin(), ops.end(), [](const mtt::Rotate_Around_Deferred_Args& a, const mtt::Rotate_Around_Deferred_Args& b) {
                    return  a.tgt_id < b.tgt_id;
                });
                
                for (usize i = 0; i < ops.size(); i += 1) {
                    auto* op = &ops[i];
                    
                    op->proc(op);
                }
                
                ops.clear();
            };
            
            Script* targets_script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                // TODO: ...
                mtt::begin_parallel_group(scb, "ROOT");
                {
                    
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            targets_script->label = def.key_primary;
            def.register_script(ARG_target,           objects_script);
            
            Script* scripts_common_args[] = {script_only, objects_script, targets_script};
            for (usize i = 0; i < 3; i += 1) {
                Script* script = scripts_common_args[i];
                
                mtt::Script_Lookup_set_var_with_key(script->lookup(), "speed", mtt::DEFAULT_LOOKUP_SCOPE, {
                    Script_Property_make(mtt::Any::from_Float(2.0f))
                });
                
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                
                
                script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    
                    
                    Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    
                    Script_Property& prop = (*res)[0];
                    mtt::Any& val = prop.value;
                    auto id = val.thing_id;
                    //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                    
                    
                    
                    {
                        auto* source_thing = mtt::Thing_try_get(mtt::ctx(), id);
                        if (source_thing == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                        }
                        
                        //Script_Lookup_print(script->lookup());
                        
                        
                        activate_actions(script, id);
                    }
                    
                    
                    
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                    
                };
                script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                };
                script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                };
                script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                    script->remove_actions();
                    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), script->agent);
                    
                    if (thing != nullptr) {
                        
                        vec3* velocity     = mtt::access<vec3>(thing, "velocity");
                        if (velocity != nullptr) {
                            *velocity = vec3(0.0f);
                        }
                        vec3* acceleration = mtt::access<vec3>(thing, "acceleration");
                        if (acceleration != nullptr) {
                            *acceleration = vec3(0.0f);
                        }
                    }
                    
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                };
            }
            
            
            
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            auto* script_w_object = def.Script_lookup(ARG_object);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            ASSERT_MSG(script_w_object != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
                //            script = def.Script_lookup(ARG_target);
                //
                //        } else if (st.has_prepositions) {
                //            script = def.Script_lookup(ARG_object);
                //
                //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_object;
                }
                
                call->on_invoke = on_invoke_default;
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
        
    }
    {
        define_action_alias(cat, "orbit", "", "revolve", "");
    }
    
    cat.on_off_shortcut_behaviors_add("press", "unpress");
    
    {
        define_action(cat, "reflect", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        //                    mtt::begin_parallel_group(scb);
                        //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                    
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = false}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            auto* tgt_entries = Script_Lookup_get_var_with_key(script, ARG_target, DEFAULT_LOOKUP_SCOPE);
                            auto* obj_entries = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                            auto* dir_entries = Script_Lookup_get_var_with_key(script, mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                            auto* spatial_entries = Script_Lookup_get_var_with_key(script, mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE);
                            bool has_tgts = false;
                            bool has_objs = false;
                            
                            bool has_dir = false;
                            bool has_spatial = false;
                            
                            if (src_entries == nullptr || src_entries->empty()) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            if (tgt_entries != nullptr && !tgt_entries->empty()) {
                                has_tgts = true;
                                //
                                //                            for (auto& val : *tgt_entries) {
                                //                                if (val.value.type == MTT_THING) {
                                //                                    mtt::Thing_ID id = val.value.thing_id;
                                //                                    to_destroy.insert(id);
                                //                                }
                                //                            }
                            }
                            if (obj_entries != nullptr && !obj_entries->empty()) {
                                has_objs = true;
                                
                                //                            for (auto& val : *obj_entries) {
                                //                                if (val.value.type == MTT_THING) {
                                //                                    mtt::Thing_ID id = val.value.thing_id;
                                //                                    to_destroy.insert(id);
                                //                                }
                                //                            }
                            }
                            
                            if (has_objs) {
                                auto* from_entries_ptr = src_entries;
                                if (has_tgts) {
                                    from_entries_ptr = tgt_entries;
                                }
                                
                                auto& from = *from_entries_ptr;
                                auto& to = *obj_entries;
                                
                                usize from_count = from.size();
                                usize to_count = to.size();
                                
                                if (to_count == 1) {
                                    auto& val = to[0];
                                    if (val.value.type != MTT_THING) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    mtt::Thing_ID id = val.value.thing_id;
                                    
                                    mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                    if (dst == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    
                                    vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                    if (dst_pos_ptr == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    vec3 dst_pos = *dst_pos_ptr;
                                    bool should_offset = (val.selector_name == ARG_object_offset_selector);
                                    
                                    
                                    
                                    for (usize i = 0; i < from_count; i += 1) {
                                        auto& val_src = from[i];
                                        if (val_src.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID src_id = val_src.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        if (should_offset) {
                                            // HARD-CODE assume teleport up or down
                                            {
                                                Message msg;
                                                msg.sender   = mtt::thing_id(src);
                                                msg.selector = string_ref_get(val.selector_value.c_str());
                                                selector_invoke(dst, &msg);
                                                dst_pos = msg.output_value.Vector3;
                                            }
                                            
                                            float32 vertical_offset = 0;
                                            if (val.selector_value == "top_with_offset") {
                                                vertical_offset -= 2;
                                            } else if (val.selector_value == "bottom_with_offset") {
                                                vertical_offset += 2;
                                            }
                                            
                                            //if (Things_are_overlapping(world, src, dst))
                                            {
                                                vec3* src_pos_ptr = mtt::access<vec3>(src, "position");
                                                vec3 final_pos = *src_pos_ptr;
                                                final_pos.y = dst_pos.y;
                                                
                                                
                                                final_pos.y += vertical_offset;
                                                
                                                mtt::Thing_set_position(src, final_pos);
                                            }
                                            //                                        else {
                                            //                                            mtt::Thing_set_position(src, dst_pos + vec3(0, vertical_offset, 0));
                                            //                                        }
                                        } else {
                                            mtt::Thing_set_position(src, dst_pos);
                                        }
                                    }
                                    
                                } else {
                                    if (to_count > from_count) {
                                        to_count = from_count;
                                    }
                                    
                                    usize min_cap = m::min(from_count, to_count);
                                    usize i = 0;
                                    for (; i < min_cap; i += 1) {
                                        auto& val_to = to[i];
                                        auto& val_from = from[i];
                                        if (val_to.value.type != MTT_THING || val_from.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID dst_id = val_to.value.thing_id;
                                        mtt::Thing* dst = mtt::Thing_try_get(world, dst_id);
                                        if (dst == nullptr) {
                                            continue;
                                        }
                                        mtt::Thing_ID src_id = val_from.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                        if (dst_pos_ptr == nullptr) {
                                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                        }
                                        vec3 dst_pos = *dst_pos_ptr;
                                        
                                        mtt::Thing_set_position(src, dst_pos);
                                        
                                    }
                                    if (i < from_count) {
                                        usize j = 0;
                                        for (; j < to_count; j += 1) {
                                            if (to[j].value.type == MTT_THING) {
                                                break;
                                            }
                                        }
                                        {
                                            auto& val = to[j];
                                            if (val.value.type != MTT_THING) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            mtt::Thing_ID id = val.value.thing_id;
                                            mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                            if (dst == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            
                                            vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                            if (dst_pos_ptr == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            vec3 dst_pos = *dst_pos_ptr;
                                            
                                            for (; i < from_count; i += 1) {
                                                auto& val_src = from[i];
                                                if (val.value.type != MTT_THING) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_ID src_id = val.value.thing_id;
                                                mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                                
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_set_position(src, dst_pos);
                                            }
                                        }
                                    }
                                    
                                }
                                
                            } else {
                                //auto* from_entries_ptr = src_entries;
                                if (has_tgts) {
                                    //from_entries_ptr = tgt_entries;
                                    
                                    
                                    auto& from = src_entries;
                                    auto& tgt = tgt_entries;
                                    
                                    usize from_count = from->size();
                                    if (from_count == 0) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    
                                    usize tgt_count = tgt->size();
                                    if (tgt_count == 0) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    
                                    auto& val_src = (*from)[0];
                                    mtt::Thing_ID src_id = val_src.value.thing_id;
                                    mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                    vec3* src_pos_ptr = mtt::access<vec3>(src, "position");
                                    vec3 src_pos = *src_pos_ptr;
                                    vec3* src_vel_ptr = mtt::access<vec3>(src, "velocity");
                                    vec3 src_vel = (src_vel_ptr != nullptr) ? *src_vel_ptr : vec3(0.0f, 0.0f, 0.0f);
                                    
                                    mat4& src_pose = mtt::rep(src)->pose_transform;
                                    vec3 src_normal_forwards = m::normalize(vec3(src_pose * vec4(heading_direction_x(mtt::rep(src)), 0.0f, 0.0f, 1.0f)));
                                    
                                    float32 angle = m::atan2pos_32(src_normal_forwards.y, src_normal_forwards.x);
                                    
                                    const auto quantize_vector_angle = [](float32 round_angle, vec3& v) {
                                        float32 angle = m::atan2pos_32(v.y, v.x);
                                        if (fmod(angle, round_angle) != 0.0f) {
                                            float32 new_angle = (float32)glm::round(angle / round_angle) * round_angle;
                                            v = vec3((float32)m::cos(new_angle), -(float32)m::sin(new_angle), v.z);
                                            for (usize32 comp = 0; comp < 3; comp += 1) {
                                                const auto el = v[comp];
                                                if (el * m::sign(el) < 0.0001f) {
                                                    v[comp] = 0.0f;
                                                }
                                            }
                                        }
                                    };
                                    //quantize_vector_angle(M_PI * 0.25f, src_normal_forwards);
                                    
                                    
                                    vec3 src_normal_backwards = -src_normal_forwards;
                                    
                                    // treat as 2-sided
                                    
                                    
                                    // bool Line_Segment_intersection_query(Line_Segment* s0, Line_Segment* s1, Hit* out)

                                    const auto collider_quad = [](mtt::Thing* th, Quad* quad_out) {
                                        mtt::Rep* rep = mtt::rep(th);
                                        if (!rep->colliders.empty()) {
                                            *quad_out = rep->colliders[0]->aabb.saved_quad;
                                            return true;
                                        }
                                        return false;
                                    };
                                    
                                    
                                    Quad src_quad;
                                    bool src_has_collider = collider_quad(src, &src_quad);
                                    
                                    usize active_tgt_count = 0;
                                    for (usize i = 0; i < tgt_count; i += 1) {
                                        auto& val_tgt = (*tgt)[i];
                                        if (val_tgt.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID tgt_id = val_tgt.value.thing_id;
                                        mtt::Thing* tgt_thing = mtt::Thing_try_get(world, tgt_id);
                                        
                                        if (tgt_thing == nullptr) {
                                            continue;
                                        }
                                        
                                        vec3* tgt_vel_ptr = mtt::access<vec3>(tgt_thing, "velocity");
                                        if (tgt_vel_ptr != nullptr) {
                                            vec3 tgt_pos = *mtt::access<vec3>(tgt_thing, "position");
                                            vec3 tgt_vel = *tgt_vel_ptr;
                                            
                                            vec3 reflector_to_tgt = tgt_pos - src_pos;
                                            
                                            // use actual angle of the object
                                            if (!src_has_collider) {
                                                if (reflector_to_tgt == m::vec3_zero()) {
                                                    reflector_to_tgt = src_normal_backwards;
                                                    auto magnitude = m::length(tgt_vel);
                                                    // assume left / right reflection
                                                    
                                                    auto norm_reflector_to_target = reflector_to_tgt;
                                                    auto A = m::dot(src_normal_forwards, norm_reflector_to_target);
                                                    auto B = m::dot(src_normal_backwards, norm_reflector_to_target);
                                                    vec3 reflection_vector = m::vec3_zero();
                                                    
                                                    reflection_vector = glm::reflect(-norm_reflector_to_target, (A < 0.0f) ? src_normal_forwards : src_normal_backwards/*norm_reflector_to_target*/);
                                                    *tgt_vel_ptr = ((magnitude == 0.0f) ? 1.0f : magnitude) * reflection_vector;
                                                    
                                                    mtt::flip_left_right(tgt_thing);
                                                } else {
                                                    
                                                    auto magnitude = m::length(tgt_vel);
                                                    // assume left / right reflection
                                                    
                                                    auto norm_reflector_to_target = normalize(reflector_to_tgt);
                                                    auto A = m::dot(src_normal_forwards, norm_reflector_to_target);
                                                    auto B = m::dot(src_normal_backwards, norm_reflector_to_target);
                                                    vec3 reflection_vector = m::vec3_zero();
                                                    
                                                    reflection_vector = glm::reflect(-norm_reflector_to_target, (A < 0.0f) ? src_normal_forwards : src_normal_backwards/*norm_reflector_to_target*/);
                                                    *tgt_vel_ptr = ((magnitude == 0.0f) ? 1.0f : magnitude) * reflection_vector;
                                                    
                                                    mtt::flip_left_right(tgt_thing);
                                                }
                                            } else {
                                                // look at the sides of the shape
                                                
                                                Quad tgt_quad;
                                                bool tgt_has_collider = collider_quad(tgt_thing, &tgt_quad);
                                                if (!tgt_has_collider) {
                                                    MTT_log_error("%s\n", "tgt should have a collider!");
                                                    continue;
                                                }
                                                
                                                
                                                
                                                Quad_As_Segments src_segments = Quad_segments(&src_quad);
                                                Quad_As_Segments dst_segments = Quad_segments(&tgt_quad);
                                                
                                                //vec3 src_pos = vec3(pos.x, pos.y, 900.0f);
                                                //vec3 dst_pos = vec3(dst_col->center_anchor.x, dst_col->center_anchor.y, 900.0f);
                                                
                                                //mtt::Line_Segment s = {src_pos, dst_pos};
                                                
                                                mtt::Hit hit = {};
                                                
                                                
                                                

                                                
                                                vec3 hit_pos = {};
                                                vec3 hit_norm = vec3(1.0f, 0.0f, 0.0f);
                                                auto hit_dist2 = POSITIVE_INFINITY;
                                                bool was_hit = false;
                                                mtt::Line_Segment final_l_src;
                                                mtt::Line_Segment final_l_dst;
                                                
                                                if (tgt_vel == m::vec3_zero()) {
                                                    for (usize i = 0; i < 4; i += 1) {
                                                        auto& segment_src = src_segments.segments[i];
                                                        for (usize j = 0; j < 4; j += 1) {
                                                            auto& segment_dst = dst_segments.segments[j];
                                                            if (mtt::Line_Segment_intersection(&segment_src, &segment_dst, &hit)) {
                                                                auto try_hit_pos = vec3(hit.pos, src_pos.z);
                                                                if (!was_hit) {
                                                                    was_hit = true;
                                                                }
                                                                
                                                                const auto dist2 = m::dist_squared(try_hit_pos, src_pos);
                                                                if (dist2 < hit_dist2) {
                                                                    hit_dist2 = dist2;
                                                                    hit_pos = try_hit_pos;
                                                                    final_l_src = segment_src;
                                                                    final_l_dst = segment_dst;
                                                                }
                                                            }
                                                        }
                                                    }
                                                } else {
                                                    auto extension = m::normalize(tgt_vel) * (m::dist(tgt_quad.tr, tgt_quad.bl) + m::dist(tgt_quad.tl, tgt_quad.br) );
                                                    final_l_dst = {.a = tgt_pos - extension, .b = tgt_pos + extension};
                                                    
                                                    for (usize i = 0; i < 4; i += 1) {
                                                        auto& segment_src = src_segments.segments[i];
                                                        {
                                                            auto& segment_dst = final_l_dst;
                                                            if (mtt::Line_Segment_intersection(&segment_src, &segment_dst, &hit)) {
                                                                auto try_hit_pos = vec3(hit.pos, src_pos.z);
                                                                if (!was_hit) {
                                                                    was_hit = true;
                                                                }
                                                                
                                                                const auto dist2 = m::dist_squared(try_hit_pos, src_pos);
                                                                if (dist2 < hit_dist2) {
                                                                    hit_dist2 = dist2;
                                                                    hit_pos = try_hit_pos;
                                                                    final_l_src = segment_src;
                                                                    final_l_dst = segment_dst;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                if (was_hit) {
//                                                    m::Vec2_print(segment.a);
//                                                    m::Vec2_print(segment.b);
//                                                    m::Vec3_print(tgt_pos);
//                                                    m::Vec3_print(src_pos);
                                                    
                                                    
                                                    //(dy,-dx) or (-dy,dx)
                                                    auto& segment = final_l_src;
                                                    auto dx = segment.b.x - segment.a.x;
                                                    auto dy = segment.b.y - segment.a.y;
                                                    auto src_normal_forwards = normalize(vec2(dy, -dx));
                                                    auto src_normal_backwards = normalize(vec2(-dy, dx));
                                                    
                                                    auto magnitude = m::length(tgt_vel);
                                                    
                                                    vec3 incident_vector = m::vec3_zero();
                                                    if (tgt_vel == m::vec3_zero()) {
                                                        incident_vector = m::normalize(hit_pos - tgt_pos);
                                                    } else {
                                                        incident_vector = m::normalize(tgt_vel);
                                                    }
                                                    //quantize_vector_angle(M_PI * 0.25f, incident_vector);
                                                    
                                                    //auto norm_reflector_to_target = normalize(reflector_to_tgt_corrected);
                                                    auto A = m::dot(vec3(src_normal_forwards, 0.0f), incident_vector);
                                                    auto B = m::dot(vec3(src_normal_backwards, 0.0f), incident_vector);
                                                    
                                                    vec3 reflection_vector = glm::reflect(incident_vector, (A < 0.0f) ? vec3(src_normal_forwards, 0.0f) : vec3(src_normal_backwards, 0.0f)/*norm_reflector_to_target*/);
                                                    
                                                    *tgt_vel_ptr = ((magnitude == 0.0f) ? 1.0f : magnitude) * m::normalize(reflection_vector);
                                                    
                                                    mtt::flip_left_right(tgt_thing);
                                                    
                                                    {
                                                        auto* rtime = Runtime::ctx();
                                                    
                                                        auto& task_list = rtime->script_tasks.list;
                                                        //MTT_print("TASK COUNT: %lu\n", task_list.size());
                                                        for (isize i = task_list.size() - 1; i >= 0; ) {
                                                            
                                                            auto* task = task_list[i];

                                                            if (task->agent == tgt_id && task->label == "move") {
                                                                mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), task->agent);
                                                                bool something_found = false;

                                                                std::vector<Script_Instance*> to_stop_stack = {task};
                                                                std::swap(task_list[i], task_list[task_list.size() - 1]);
                                                                task_list.pop_back();
                                                                do {
                                                                    do {
                                                                        something_found = false;
                                                                        for (isize j = task_list.size() - 1; j >= 0; j -= 1) {
                                                                            auto* task_j = task_list[j];
                                                                            if (task_j->parent == to_stop_stack.back()) {
                                                                                to_stop_stack.push_back(task_j);
                                                                                something_found = true;

                                                                                std::swap(task_list[j], task_list[task_list.size() - 1]);
                                                                                task_list.pop_back();
                                                                            }
                                                                        }
                                                                    } while (something_found);
                                                                    
                                                                    Script_Instance_stop(to_stop_stack.back());
                                                                    to_stop_stack.pop_back();
                                                                } while (!to_stop_stack.empty());
                                                                
                                //                                Script_Instance_stop(task);
                                //
                                                                if (!task_list.empty()) {
                                                                    i = task_list.size() - 1;
                                                                }
                                                            } else {
                                                                i -= 1;
                                                            }
                                                        }
                                                    }
                                                    
                                                    break;
                                                }
                                            }
                                            //m::Vec3_print(reflection_vector);
                                            
                                            
                                        }
                                        
                                        
                                        active_tgt_count += 1;
                                        
                                    }
                                    if (active_tgt_count == 0) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    
                                } else {
                                    
                                }
                                
                                
                                
                                
                                
                                
                            }
                            //                        if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                            //                            for (auto& val : *src_entries) {
                            //                                if (val.value.type == MTT_THING) {
                            //                                    mtt::Thing_ID id = val.value.thing_id;
                            //                                    to_destroy.insert(id);
                            //                                }
                            //                            }
                            //                        }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            //                        auto& to_destroy = *(mtt::Set<mtt::Thing_ID>*)args;
                            //                        for (auto thing_id : to_destroy) {
                            //                            if (!mtt::Thing_try_get(world, thing_id)) {
                            //                                continue;
                            //                            }
                            //
                            //                            Destroy_Command& cmd = world->to_destroy.emplace_back();
                            //                            cmd.thing_id = thing_id;
                            //                            cmd.affects_connected = false;
                            //                            cmd.do_fade_animation = false;
                            //                            cmd.time_delay = 0;
                            //                            cmd.frame_delay = 1;
                            //                            //cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
                            //                            cmd.time_remaining = 0;//cmd.time_delay;
                            //                        }
                            //                        delete (&to_destroy);
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
    }
    
    {
        define_action(cat, "apply", "", [&](auto& def) {
            Script* script = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                
                Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                    scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                    
                    mtt::begin_parallel_group(scb, "ROOT");
                    {
                        //                    mtt::begin_parallel_group(scb);
                        //                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
                    }
                    auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
                    MTT_UNUSED(end);
                });
                
                script_agent->label = def.key_primary;
                script_agent->is_infinite = false;
                def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
                def.register_script(mtt::ARG_source, script_agent);
                def.register_script(mtt::ARG_object, script_agent);
                def.register_script(mtt::ARG_target, script_agent);
                
                Script* scripts_common_args[] = {script_agent};
                for (usize i = 0; i < 1; i += 1) {
                    Script* script = scripts_common_args[i];
                    
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                    mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE);
                }
                
            });
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args, {.supports_no_src = true, .treat_direct_objects_as_types = true}).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            
            
            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    // TODO:
                    MTT_log_debug("%s\n", "TODO!");
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            auto* src_entries = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            auto* tgt_entries = Script_Lookup_get_var_with_key(script, ARG_target, DEFAULT_LOOKUP_SCOPE);
                            auto* obj_entries = Script_Lookup_get_var_with_key(script, ARG_object, DEFAULT_LOOKUP_SCOPE);
                            auto* dir_entries = Script_Lookup_get_var_with_key(script, mtt::ARG_directional, mtt::DEFAULT_LOOKUP_SCOPE);
                            auto* spatial_entries = Script_Lookup_get_var_with_key(script, mtt::ARG_spatial, mtt::DEFAULT_LOOKUP_SCOPE);
                            bool has_tgts = false;
                            bool has_objs = false;
                            
                            bool has_dir = false;
                            bool has_spatial = false;
                            
                            if (src_entries == nullptr || src_entries->empty()) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            
                            if (tgt_entries != nullptr && !tgt_entries->empty()) {
                                has_tgts = true;
                                //
                                //                            for (auto& val : *tgt_entries) {
                                //                                if (val.value.type == MTT_THING) {
                                //                                    mtt::Thing_ID id = val.value.thing_id;
                                //                                    to_destroy.insert(id);
                                //                                }
                                //                            }
                            }
                            if (obj_entries != nullptr && !obj_entries->empty()) {
                                has_objs = true;
                                
                                //                            for (auto& val : *obj_entries) {
                                //                                if (val.value.type == MTT_THING) {
                                //                                    mtt::Thing_ID id = val.value.thing_id;
                                //                                    to_destroy.insert(id);
                                //                                }
                                //                            }
                            }
                            
                            if (has_objs) {
                                auto* from_entries_ptr = src_entries;
                                if (has_tgts) {
                                    from_entries_ptr = tgt_entries;
                                }
                                
                                auto& from = *from_entries_ptr;
                                auto& to = *obj_entries;
                                
                                usize from_count = from.size();
                                usize to_count = to.size();
                                
                                if (to_count == 1) {
                                    auto& val = to[0];
                                    if (val.value.type != MTT_THING) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    mtt::Thing_ID id = val.value.thing_id;
                                    
                                    mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                    if (dst == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    
                                    vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                    if (dst_pos_ptr == nullptr) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                    }
                                    vec3 dst_pos = *dst_pos_ptr;
                                    bool should_offset = (val.selector_name == ARG_object_offset_selector);
                                    
                                    
                                    
                                    for (usize i = 0; i < from_count; i += 1) {
                                        auto& val_src = from[i];
                                        if (val_src.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID src_id = val_src.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        if (should_offset) {
                                            // HARD-CODE assume teleport up or down
                                            {
                                                Message msg;
                                                msg.sender   = mtt::thing_id(src);
                                                msg.selector = string_ref_get(val.selector_value.c_str());
                                                selector_invoke(dst, &msg);
                                                dst_pos = msg.output_value.Vector3;
                                            }
                                            
                                            float32 vertical_offset = 0;
                                            if (val.selector_value == "top_with_offset") {
                                                vertical_offset -= 2;
                                            } else if (val.selector_value == "bottom_with_offset") {
                                                vertical_offset += 2;
                                            }
                                            
                                            //if (Things_are_overlapping(world, src, dst))
                                            {
                                                vec3* src_pos_ptr = mtt::access<vec3>(src, "position");
                                                vec3 final_pos = *src_pos_ptr;
                                                final_pos.y = dst_pos.y;
                                                
                                                
                                                final_pos.y += vertical_offset;
                                                
                                                mtt::Thing_set_position(src, final_pos);
                                            }
                                            //                                        else {
                                            //                                            mtt::Thing_set_position(src, dst_pos + vec3(0, vertical_offset, 0));
                                            //                                        }
                                        } else {
                                            mtt::Thing_set_position(src, dst_pos);
                                        }
                                    }
                                    
                                } else {
                                    if (to_count > from_count) {
                                        to_count = from_count;
                                    }
                                    
                                    usize min_cap = m::min(from_count, to_count);
                                    usize i = 0;
                                    for (; i < min_cap; i += 1) {
                                        auto& val_to = to[i];
                                        auto& val_from = from[i];
                                        if (val_to.value.type != MTT_THING || val_from.value.type != MTT_THING) {
                                            continue;
                                        }
                                        
                                        mtt::Thing_ID dst_id = val_to.value.thing_id;
                                        mtt::Thing* dst = mtt::Thing_try_get(world, dst_id);
                                        if (dst == nullptr) {
                                            continue;
                                        }
                                        mtt::Thing_ID src_id = val_from.value.thing_id;
                                        mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                        if (src == nullptr) {
                                            continue;
                                        }
                                        
                                        vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                        if (dst_pos_ptr == nullptr) {
                                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                        }
                                        vec3 dst_pos = *dst_pos_ptr;
                                        
                                        mtt::Thing_set_position(src, dst_pos);
                                        
                                    }
                                    if (i < from_count) {
                                        usize j = 0;
                                        for (; j < to_count; j += 1) {
                                            if (to[j].value.type == MTT_THING) {
                                                break;
                                            }
                                        }
                                        {
                                            auto& val = to[j];
                                            if (val.value.type != MTT_THING) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            mtt::Thing_ID id = val.value.thing_id;
                                            mtt::Thing* dst = mtt::Thing_try_get(world, id);
                                            if (dst == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            
                                            vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
                                            if (dst_pos_ptr == nullptr) {
                                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                                            }
                                            vec3 dst_pos = *dst_pos_ptr;
                                            
                                            for (; i < from_count; i += 1) {
                                                auto& val_src = from[i];
                                                if (val.value.type != MTT_THING) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_ID src_id = val.value.thing_id;
                                                mtt::Thing* src = mtt::Thing_try_get(world, src_id);
                                                
                                                if (src == nullptr) {
                                                    continue;
                                                }
                                                
                                                mtt::Thing_set_position(src, dst_pos);
                                            }
                                        }
                                    }
                                    
                                }
                                
                            } else {
                                // TODO: maybe never, but could want to teleport in a direction
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                            }
                            //                        if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                            //                            for (auto& val : *src_entries) {
                            //                                if (val.value.type == MTT_THING) {
                            //                                    mtt::Thing_ID id = val.value.thing_id;
                            //                                    to_destroy.insert(id);
                            //                                }
                            //                            }
                            //                        }
                            
                            if (res->size() > 0) {
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                                activate_actions(script, id);
                            }
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            
                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            
                            //                        auto& to_destroy = *(mtt::Set<mtt::Thing_ID>*)args;
                            //                        for (auto thing_id : to_destroy) {
                            //                            if (!mtt::Thing_try_get(world, thing_id)) {
                            //                                continue;
                            //                            }
                            //
                            //                            Destroy_Command& cmd = world->to_destroy.emplace_back();
                            //                            cmd.thing_id = thing_id;
                            //                            cmd.affects_connected = false;
                            //                            cmd.do_fade_animation = false;
                            //                            cmd.time_delay = 0;
                            //                            cmd.frame_delay = 1;
                            //                            //cmd.time_delay = (call->has_time_interval) ? call->time_interval : 0.0f;
                            //                            cmd.time_remaining = 0;//cmd.time_delay;
                            //                        }
                            //                        delete (&to_destroy);
                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;
            
            return mtt::Result_make(true);
        });
    }
    
    {
        define_action(cat, "reference", "", [&](auto& def) {
            
            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
//                End_Parallel_Group_Args pg_args = {};
//                pg_args.reset_on_looparound = true;
//                pg_args.has_count = true;
//                pg_args.count = 1;
                auto end = end_parallel_group(scb, mtt::End_Parallel_Group_Args_done());
                MTT_UNUSED(end);
            });
            
            script_agent->label = def.key_primary;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            
            Script* script_target = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
                
                mtt::begin_parallel_group(scb, "ROOT");
                {
                   
                }
                End_Parallel_Group_Args pg_args = {};
                pg_args.reset_on_looparound = true;
                pg_args.has_count = true;
                pg_args.count = 1;
                auto end = end_parallel_group(scb, pg_args);
                MTT_UNUSED(end);
            });
            
            script_target->label = def.key_primary;
            def.register_script(ARG_target, script_target);
            
            Script* scripts_common_args[] = {script_agent, script_target};
            for (usize i = 0; i < 2; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            
            
            auto* script_default  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            auto* script_w_target = def.Script_lookup(ARG_target);
            
            ASSERT_MSG(script_default  != nullptr, "script should exist!");
            ASSERT_MSG(script_w_target != nullptr, "script should exist!");
            
            
            mtt::Script* script = nullptr;
            
            //if (st.has_direct_objects)
            {
    //            script = def.Script_lookup(ARG_target);
    //
    //        } else if (st.has_prepositions) {
    //            script = def.Script_lookup(ARG_object);
    //
    //        } else if (st.has_agents) {
                script = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
                if (st.has_direct_objects) {
                    script = script_w_target;
                } else if (st.has_prepositions) {
                    script = script_w_target;
                }
                
                {
                    
                    call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                        if (call->source_script_id == Script_ID_INVALID) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                        }
                       
                        Script* script_to_call = Script_lookup(call->source_script_id);
                        if (script_to_call == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                        }
                        
                        auto& plist = call->params().param_list(0);
                        auto& params_lookup = plist.lookup;
                        auto& shared_lookup = script_to_call->lookup();
                        auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                        
                        auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                        if (src_entries == nullptr) {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                        }
                        
                        auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                        auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                        bool has_tgts = false;
                        bool has_objs = false;
                        
                        mtt::World* world = mtt::ctx();
                        

                        mtt::Set<mtt::Thing_ID> to_ref = {};
                        mtt::Set<mtt::String> to_ref_labels = {};
                        if (tgt_entries != nullptr && !tgt_entries->empty()) {
                            
                            
                            for (auto& val : *tgt_entries) {
                                if (val.value.type == MTT_THING) {
                                    has_tgts = true;
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_ref.insert(id);
                                } else if (val.value.type == MTT_STRING) {
                                    const mtt::String str = MTT_string_ref_to_cstring(val.value.String);
                                    to_ref_labels.insert(str);
                                }
                            }
                        }
                        if (obj_entries != nullptr && !obj_entries->empty()) {
                            has_objs = true;
                            
                            for (auto& val : *obj_entries) {
                                if (val.value.type == MTT_THING) {
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_ref.insert(id);
                                }
                            }
                        }
                        if (!has_tgts && !has_objs && src_entries != nullptr && !src_entries->empty()) {
                            for (auto& val : *src_entries) {
                                if (val.value.type == MTT_THING) {
                                    mtt::Thing_ID id = val.value.thing_id;
                                    to_ref.insert(id);
                                }
                            }
                        }
                                                
                        auto* action_entry = verb_root_equivalence(verb_add("reference"));
                        
                        for (usize src_i = 0; src_i < src_entries->size(); src_i += 1) {
                            mtt::Thing_ID thing_src_id = ((*src_entries)[src_i]).value.thing_id;
                            mtt::Thing* thing_src = mtt::Thing_try_get(world, thing_src_id);
                            if (thing_src == nullptr) {
                                continue;
                            }
                            
                            thing_remove_referrents(thing_src);
                            
                            for (const auto& tgt : to_ref) {
                                mtt::Thing* thing_tgt = mtt::Thing_try_get(world, tgt);
                                if (thing_tgt == nullptr) {
                                    continue;
                                }
                                
                                mtt::thing_refer_to(thing_src, tgt);
                                
                                Thing_add_action_event_instantaneous(thing_src, action_entry, thing_tgt, VERB_EVENT_BEGIN);
                            }
                        }
                        // TODO: labels
                        
                        auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                            script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                                
                                
                                Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                                
                                Script_Property& prop = (*res)[0];
                                mtt::Any& val = prop.value;
                                auto id = val.thing_id;
                      //          auto& selector_name = prop.selector_name;
                    //            auto& selector_value = prop.selector_value;
                                
                    
                                
                                activate_actions(script, id);

                                
                                

                                
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                            };
                            script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                                auto* t = mtt::Thing_try_get(world, script->agent);
                                if (t == nullptr) {
                                    script->remove_actions();
                                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                                }
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            };
                            script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                            };
                            script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                            };
                            script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                            };
                        }, args);
                        

                        
                        
                        return out;
                    };
                }
                
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
    
    

    
    mtt::set_graph(world, root_graph);
}

void create_new_action(mtt::Script* script, const mtt::String& label)
{
    auto* dt = dt::DrawTalk::ctx();
    auto& lang_ctx = dt->lang_ctx;
    dt::DT_Behavior_Catalogue& cat = lang_ctx.behavior_catalogue;
    
    
    mtt::World* world = dt->mtt;
    
    mtt::String label_to_use = label;
    {
        define_action(cat, label_to_use, "", [&](auto& def) {
            Script* script_agent = script;
//
//            Script* script_agent = mtt::Script_Builder_do(world, [&](mtt::World* world, Script_Builder* scb) {
//                scb->state = mtt::SCRIPT_BUILDER_STATE::ACTION;
//                
//                mtt::begin_parallel_group(scb, "ROOT");
//                {
////                    mtt::begin_parallel_group(scb);
////                    mtt::end_parallel_group(scb, mtt::End_Parallel_Group_Args_endless_loop());
//                }
//                auto end = end_parallel_group(scb, End_Parallel_Group_Args_done());
//                MTT_UNUSED(end);
//            });
            
            script_agent->label = def.key_primary;
            script_agent->is_infinite = false;
            def.register_script(DEFAULT_LOOKUP_SCOPE, script_agent);
            def.register_script(mtt::ARG_source, script_agent);
            def.register_script(mtt::ARG_object, script_agent);
            def.register_script(mtt::ARG_target, script_agent);
            
            Script* scripts_common_args[] = {script_agent};
            for (usize i = 0; i < 1; i += 1) {
                Script* script = scripts_common_args[i];
                            
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_source,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_object,     mtt::DEFAULT_LOOKUP_SCOPE);
                mtt::Script_Lookup_set_key(script->lookup(), mtt::ARG_target,     mtt::DEFAULT_LOOKUP_SCOPE);
            }
            

                
            
        }).set_on_generate([](DT_BEHAVIOR_PROC_PARAMS) -> auto {
            
            auto& def    = *args.behavior;
            auto& prop   = *args.prop;
            
            struct State : public Common_State {
                State(Speech_Property& prop) : Common_State(prop) {}
            };
            State st = State(prop);
            
            Call_Descriptor* call = nullptr;
            if (default_on_generate(st, &call, args).status == false) {
                return mtt::Result_make(false);
            }
            
            mtt::Script* script  = def.Script_lookup(DEFAULT_LOOKUP_SCOPE);
            

            
            {
                
                call->on_invoke = [](Call_Descriptor* call, Script_Instance* caller_script, void (*on_new_script)(Script_Instance* si, void* args), void* args) -> Logic_Procedure_Return_Status {
                    
                    if (call->source_script_id == Script_ID_INVALID) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                   
                    Script* script_to_call = Script_lookup(call->source_script_id);
                    if (script_to_call == nullptr) {
                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
                    }
                    
                    auto& plist = call->params().param_list(0);
                    auto& params_lookup = plist.lookup;
                    auto& shared_lookup = script_to_call->lookup();
                    auto final_lookup = merge_lookups_into_from(shared_lookup, params_lookup);
                    
                    auto* src_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_source, DEFAULT_LOOKUP_SCOPE);
                    auto* tgt_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_target, DEFAULT_LOOKUP_SCOPE);
                    auto* obj_entries = Script_Lookup_get_var_with_key(final_lookup, ARG_object, DEFAULT_LOOKUP_SCOPE);
                    
                    auto out = on_invoke_default(call, caller_script, [](Script_Instance* script, void* args) {
                        script->on_start = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            
                            
                            Script_Property_List* res = Script_Lookup_get_var_with_key(script, ARG_source, DEFAULT_LOOKUP_SCOPE);
                            
                            Script_Property& prop = (*res)[0];
                            mtt::Any& val = prop.value;
                            auto id = val.thing_id;
                  //          auto& selector_name = prop.selector_name;
                //            auto& selector_value = prop.selector_value;
                            
                
                            
                            activate_actions(script, id);

                            
                            

                            
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;

                        };
                        script->on_begin_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            auto* t = mtt::Thing_try_get(world, script->agent);
                            if (t == nullptr) {
                                script->remove_actions();
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                            }
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_end_frame = [](mtt::World* world, Script_Instance* script, void* args) -> mtt::Logic_Procedure_Return_Status {
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
                        };
                        script->on_cancel = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
                        };
                        script->on_done = [](mtt::World* world, Script_Instance* script, void* args) -> Logic_Procedure_Return_Status {
                            script->remove_actions();
                            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
                        };
                    }, args);
                    

                    
                    
                    return out;
                };
            }
            
            call->set_script(def.key_primary, script->id);
            call->user_data = &def;

            return mtt::Result_make(true);
        });
    
    }
     
    
    
}

}
