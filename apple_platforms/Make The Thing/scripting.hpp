//
//  scripting.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/16/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef scripting_hpp
#define scripting_hpp

#include "thing_shared_types.hpp"
#include "file_system.hpp"
#include "drawtalk_behavior.hpp"

namespace dt {
struct Speech_Property;

}

namespace mtt {


void terminate_rule_if_dependent_on_thing(mtt::Thing_ID thing_id);

struct Script_Rules {
    dt::rules::Condition_Builder triggers = {};
    dt::rules::Condition_Builder responses = {};
    dt::rules::Condition_Builder end_conditions = {};
    bool has_end_conditions = false;
};

struct Script_Instance {
    Script_ID id = Script_ID_INVALID;
    static inline Script_ID next_avail_ID = 1;
    static inline const usize PRIORITY_FIRST = 0;
    
    usize priority = PRIORITY_FIRST;
    
    Script* source_script   = nullptr;
    Script_Instance* parent = nullptr;
    mtt::Thing_ID caller = mtt::Thing_ID_INVALID;
    
    uint64 creation_time = 0;
    // This is where the per-evaluation data are stored (passed between Things during evaluation)
    Evaluation_Output output = {};
    
    mtt::String label = {};
    
    //Call_Descriptor call = {};
    //Call_Param_List call_params = {};
    
    std::vector<Field_List_Descriptor> thing_initial_fields = {};
    
    std::vector<std::vector<Field_List>> thing_local_fields = {};
    
    std::vector<Active_Action> active_action_list = {};
    
    mtt::Thing_ID agent = mtt::Thing_ID_INVALID;
    bool allow_duplicates_for_agent = false;
    void action(const mtt::String& a, mtt::Thing_ID src, mtt::Thing_ID dst);
    void action(const mtt::String& a, mtt::Thing_ID src);
    void remove_actions(void);
    void remove_actions(mtt::Thing_ID t_id);
    
    Script_Lookup lookup_ = {};
    Script_Lookup* curr_lookup = &lookup_;
    
    Script_Lookup lookup_initial = {};
    
    bool is_own_lookup = true;
    bool preserve_lookup = false;
    
    Script_Lookup& lookup()
    {
        return *curr_lookup;
    }
    
    void set_lookup_copy(Script_Lookup* lu)
    {
        lookup_ = *lu;
        curr_lookup = &lookup_;
        
        is_own_lookup = true;
    }
    
    Script_Lookup* shared_lookup()
    {
        return curr_lookup;
    }
    
    void set_shared_lookup(Script_Lookup* lu)
    {
        curr_lookup = lu;
        
        is_own_lookup = false;
    }
    
    void set_own_lookup()
    {
        curr_lookup = &lookup_;
        
        is_own_lookup = true;
    }
    
    std::vector<Script_Contexts> contexts = {};
    usize ctx_idx = 0;
    
    std::vector<mtt::Any> return_value = {};
    SCRIPT_STATUS status = SCRIPT_STATUS_NOT_STARTED;
    
    inline static mem::Pool_Allocation pool = {};
    inline static Script_Instance* make()
    {
        auto* s_i = mem::alloc_init<Script_Instance>(&Script_Instance::pool.allocator);
        s_i->id = Script_Instance::next_avail_ID;
        Script_Instance::next_avail_ID += 1;
        return s_i;
    }
    inline static void destroy(Script_Instance* s)
    {
        mem::deallocate<Script_Instance>(&Script_Instance::pool.allocator, s);
    }
    
    Script_Instance* init(Script* script, void (*post_init)(Script_Instance* s, void* data), void* data = nullptr);
    Script_Instance* deinit(void);
    
    mtt::Logic_Procedure_Return_Status (*on_start)(mtt::World* world, Script_Instance* self_script, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_begin_frame)(mtt::World* world, Script_Instance* script_instance, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_end_frame)(mtt::World* world, Script_Instance* script_instance, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_cancel)(mtt::World* world, Script_Instance* self_script, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_done)(mtt::World* world, Script_Instance* self_script, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_terminate)(mtt::World* world, Script_Instance* self_script, void* args) = nullptr;
    
    Script_Rules rules = {};
    Script_Rules* rules_ref = nullptr;
    std::vector<Rule_Var_Record_One_Result> rule_vars = {};
    bool rules_are_valid = true;
    bool rules_are_active = true;
    
    void* args = nullptr;
    
    inline bool is_rule(void)
    {
        return source_script->is_rule;
    }
};

void Script_Instance_append_return_value(Script_Instance* s, const mtt::Any& value);

struct Script_Instance_compare__ID_and_Priority {
    inline bool operator() (const Script_Instance* a, const Script_Instance* b)
    {
        return (a != b) && ( (a->priority != b->priority) ? a->priority < b->priority : a->id < b->id );
    }
};

template <typename PROC>
static inline void add_args_to_stop(Script_Instance* s_i, PROC proc)
{
    auto& map = Runtime::ctx()->Script_Instance_things_to_stop;
    auto f_it = map.find((Script_Instance_Ref)s_i);
    mtt::Dynamic_Array<mtt::Thing_ID>* container = nullptr;
    if (f_it != map.end()) {
        container = &(f_it->second);
    } else {
        mtt::Dynamic_Array<mtt::Thing_ID> new_container = {};
        mtt::Array_init(&new_container, *mtt::args_allocator());
        container = &(map.insert({(Script_Instance_Ref)s_i, new_container}).first->second);
    }
    
    proc(*container);
}


typedef void RAW_SCRIPT;
void from_JSON_files(mtt::World* world, std::vector<RAW_SCRIPT*>& out, Text_File_Load_Proc callback = nullptr);
void from_JSON_file(mtt::World* world, std::vector<std::string>& path_list, std::vector<RAW_SCRIPT*>& out, Text_File_Load_Proc callback = nullptr);



void sort_things(World* world, std::vector<mtt::Thing*>& things, Eval_Connection_Graph& G);

void sort_things(World* world, std::vector<mtt::Thing*>& things, Eval_Connection_Graph& G);

enum struct SCRIPT_BUILDER_STATE : uint64 {
    NONE,
    ACTION,
    RESPONSE,
    GOAL,
};


struct Deferred_Script_Builder_Op {
    dt::Speech_Property* prop;
};

struct Script_Builder_Condition_Builders {
    dt::rules::Condition_Builder* trigger = nullptr;
    dt::rules::Condition_Builder* response = nullptr;
    dt::rules::Condition_Builder* end_condition = nullptr;
};
struct Script_Builder {
    Script* script = nullptr;
    mtt::World* world = nullptr;
    SCRIPT_BUILDER_STATE state = SCRIPT_BUILDER_STATE::NONE;
    std::vector<mtt::Thing*> things = {};
    std::vector<mtt::Thing*> group_stack = {};
    std::vector<mtt::Thing*> roots = {};
    usize depth = 0;
    void* extra_info = nullptr;
    bool is_valid = true;
    
    std::vector<Deferred_Script_Builder_Op> deferred_ops = {};
    
    Script_Builder_Condition_Builders condition_builders = {};
    bool is_building_conditions = false;
    bool has_end_condition = false;
    bool is_building_new_action = false;
};

Script_Builder Script_Builder_make(mtt::World* world);
using SBRET = uint64;

struct Parallel_Group_Begin_Out {
    mtt::Thing* thing = nullptr;
    Group_Block_Begin* state = nullptr;
};

Parallel_Group_Begin_Out begin_parallel_group(Script_Builder* scb, const mtt::String& label="");
struct End_Parallel_Group_Args {
    usize count = 1.0f;
    //float32 time_interval = 0.0;
    bool has_count         = false;
    bool reset_on_looparound = false;
    //bool has_time_interval = false;
};

static inline End_Parallel_Group_Args End_Parallel_Group_Args_done()
{
    return (End_Parallel_Group_Args) {
        .count = 1,
        .has_count = true,
        .reset_on_looparound = true,
    };
}

static inline End_Parallel_Group_Args End_Parallel_Group_Args_endless_loop()
{
    return (End_Parallel_Group_Args) {
        .count = mtt::FOREVER,
        .has_count = true,
        .reset_on_looparound = false,
    };
}
static inline End_Parallel_Group_Args End_Parallel_Group_Args_endless_loop_reset_on_looparound()
{
    return (End_Parallel_Group_Args) {
        .count = mtt::FOREVER,
        .has_count = true,
        .reset_on_looparound = true,
    };
}

static inline End_Parallel_Group_Args End_Parallel_Group_Args_endless_loop(bool reset_on_looparound)
{
    return (End_Parallel_Group_Args) {
        .count = mtt::FOREVER,
        .has_count = true,
        .reset_on_looparound = reset_on_looparound,
    };
}

struct Parallel_Group_End_Out {
    mtt::Thing* thing = nullptr;
    Group_Block_End* state = nullptr;
};

Parallel_Group_End_Out end_parallel_group(Script_Builder* scb, const End_Parallel_Group_Args& args, const mtt::String& label="");
//mtt::Thing* begin_sequence_group(Script_Builder* scb, const mtt::String& label="");
//mtt::Thing* end_sequence_group(Script_Builder* scb, const mtt::String& label="");

mtt::Thing* wait_for_time_interval(Script_Builder* scb, float32 time_interval_seconds);
mtt::Thing* wait_for_event(Script_Builder* scb, const mtt::String& event_name);
mtt::Thing* wait_for_all_scheduled_ended(Script_Builder* scb);

typedef enum Call_Action_Param_Type {
    Call_Action_Param_Type_Connection,
    Call_Action_Param_Type_Field,
} Call_Action_Param_Type;;
struct Call_Action_Param {
    Call_Action_Param_Type type = {};
    mtt::String name = {};
    mtt::Any value = {};
};
struct Call_Action_Params {
    usize count = 1;
    float32 time_interval = 0.0f;
    std::vector<Call_Action_Param> params = {};
};

SBRET call_action(Script_Builder* scb, const Call_Action_Params& params);
inline static SBRET call_action(Script_Builder* scb)
{
    return call_action(scb, {});
}
using SC_PROC_TYPE = void (*)(void*);

struct Rule_Eval_State;
Rule_Eval_State* rule_result_eval(Script_Builder* scb, const mtt::String& label="");

SBRET begin_rule_result_loop(Script_Builder* scb, void* rule);
SBRET rule_result_true_case(Script_Builder* scb);
SBRET rule_result_otherwise_case(Script_Builder* scb);
SBRET end_rule_result_loop(Script_Builder* scb);

SBRET procedure_on_enter(Script_Builder* scb, SC_PROC_TYPE&& proc);
SBRET procedure_body(Script_Builder* scb, SC_PROC_TYPE&& proc);
SBRET procedure_on_exit(Script_Builder* scb, SC_PROC_TYPE&& proc);
SBRET labeled_phase(Script_Builder* scb, const mtt::String& label);
mtt::Thing* thing(Script_Builder* scb, mtt::Thing* t);
mtt::Thing* thing_make(Script_Builder* scb, mtt::Thing_Archetype_ID type, const mtt::String& label="");
mtt::Thing* lookup_get(Script_Builder* scb, const mtt::String& key, const mtt::String& scope_key=DEFAULT_LOOKUP_SCOPE);
//mtt::Thing* lookup_get_things(Script_Builder* scb, const mtt::String& key, const mtt::String& scope_key=ANY_LOOKUP_SCOPE);

Script* Script_Builder_build(Script_Builder* scb);

void Script_Instance_destroy(Script_Instance* s);

uint64 Script_Instance_root_creation_time(Script_Instance* s);



struct Loop_Status {
    bool should_enter = true;
    mtt::Thing* continuation = nullptr;
    bool blocks = false;
};
Loop_Status begin_loop(Script_Instance* s, mtt::Thing* src_thing);
//void advance_loop(Script_Instance* s, mtt::Thing* src_thing);
void end_loop(Script_Instance* s, mtt::Thing* src_thing);


template <typename PROC>
MTT_NODISCARD
Script* Script_Builder_do(mtt::World* world, PROC&& proc)
{
    auto* prev_g = mtt::curr_graph(world);
    auto scb = Script_Builder_make(world);
    mtt::set_graph(world, &scb.script->connections);
    proc(world, &scb);
    auto* script = Script_Builder_build(&scb);
    //ASSERT_MSG(script != nullptr, "creation of script failed");
    mtt::set_graph(world, prev_g);
    return script;
}

template <typename PROC>
MTT_NODISCARD
Script* Script_Builder_do_infinite(mtt::World* world, PROC&& proc, bool reset_on_looparound = false)
{
    auto* prev_g = mtt::curr_graph(world);
    auto scb = Script_Builder_make(world);
    
    mtt::set_graph(world, &scb.script->connections);
    
    
    mtt::begin_parallel_group(&scb, "ROOT");
    

    mtt::End_Parallel_Group_Args args = {};
    
    
    proc(world, &scb);
    
    args.has_count = true;
    args.count = mtt::FOREVER;
    args.reset_on_looparound = false;
    mtt::end_parallel_group(&scb, args);
    
    auto* script = Script_Builder_build(&scb);
    ASSERT_MSG(script != nullptr, "creation of script failed");
    mtt::set_graph(world, prev_g);
    return script;
}


MTT_NODISCARD
static inline Script* Script_Builder_do(mtt::World* world, void (*proc)(mtt::World* world, Script_Builder* scb, void* args), void* proc_args)
{
    auto scb = Script_Builder_make(world);
    auto* prev_g = mtt::curr_graph(world);
    mtt::set_graph(world, &scb.script->connections);
    proc(world, &scb, proc_args);
    auto* script = Script_Builder_build(&scb);
    ASSERT_MSG(script != nullptr, "creation of script failed");
    mtt::set_graph(world, prev_g);
    return script;
}

static inline Script* Script_Builder_do_infinite(mtt::World* world, void (*proc)(mtt::World* world, Script_Builder* scb, void* args), void* proc_args, bool reset_on_looparound = false)
{
    auto* prev_g = mtt::curr_graph(world);
    auto scb = Script_Builder_make(world);
    
    mtt::set_graph(world, &scb.script->connections);
    
    
    mtt::begin_parallel_group(&scb, "ROOT");
    

    mtt::End_Parallel_Group_Args args = {};
    
    
    proc(world, &scb, proc_args);
    
    args.has_count = true;
    args.count = mtt::FOREVER;
    args.reset_on_looparound = false;
    mtt::end_parallel_group(&scb, args);
    
    auto* script = Script_Builder_build(&scb);
    ASSERT_MSG(script != nullptr, "creation of script failed");
    mtt::set_graph(world, prev_g);
    return script;
}

template <typename PROC>
void Script_Builder_root_graph(Script_Builder* sb, PROC&& proc)
{
    auto* prev_graph = mtt::curr_graph(sb->world);
    
    mtt::set_graph_to_root(sb->world);
    
    proc(sb);
    
    mtt::set_graph(sb->world, prev_graph);
}


struct Gen_Args {
    usize count = 1;
    
    float64 time_interval = 0.0f;
    
    float64 time_override = 0.0f;
    
    bool has_time_interval = false;
    bool should_override_time = false;
};


Script_Lookup merge_lookups_into_from(const Script_Lookup& into, const Script_Lookup& from);


bool should_print_script_eval(mtt::World* world);
void toggle_should_print_script_eval(mtt::World* world);




}


#endif /* scripting_hpp */
