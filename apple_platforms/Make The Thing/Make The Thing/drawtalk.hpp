//
//  drawtalk.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/9/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_hpp
#define drawtalk_hpp

//#include "thing_shared_types.hpp"
#include "thing.hpp"
#include "collision.hpp"

#include "drawtalk_shared_types.hpp"

//#include "IITree.h"

//#include "drawtalk_shared_types.hpp"

#include "drawtalk_language_analysis.h"

//#include "tree.hpp"

#include "drawtalk_behavior.hpp"


#include "word_info.hpp"

//#include "drawtalk_definitions_tree.hpp"

#include "drawtalk_ui.hpp"

#include "publish_subscribe.hpp"

#include "scripting.hpp"

#define DT_DEBUG_print(...) //MTT_print(__VA_ARGS__)


namespace dt {



struct DrawTalk;
//

#define EXTENDS(TYPE__) : public TYPE__
//
//struct DrawTalk_Text {
//    mtt::Text_View* main_display;
//};



struct DT_Behavior;


struct Widget {
    mtt::Thing_ID handle;
    mtt::String type;
    mtt::Any value;
    DT_Behavior* behavior;
    mtt::String key;
};



typedef enum Instruction_CHILD_SIDE_TYPE {
    Instruction_CHILD_SIDE_TYPE_LEFT = 0,
    Instruction_CHILD_SIDE_TYPE_RIGHT = 1,
} Instruction_CHILD_TYPE;



struct Instruction {
    Instruction_CHILD_SIDE_TYPE child_side_type = Instruction_CHILD_SIDE_TYPE_LEFT;
    mtt::String type = "";
    mtt::String kind = "";
    mtt::String annotation = "";
    mtt::Any    value = {};
    
    dt::Dynamic_Array<Speech_Property*> prop;
    
    bool confirmed = false;
    bool required  = false;
    bool require_items_if_object = true;
    
    // TODO: if overridden, do not infer coreference
    bool selections_overriden = false;
    
    uint32 should_descend = 1;
    uint32 x_offset = 0;
    
    dt::Dynamic_Array<mtt::Thing_ID> thing_id_list = {};
    dt::Dynamic_Array<mtt::String>   thing_type_list = {};
    mtt::Map<mtt::Thing_ID, mtt::Thing_Proxy_ID> thing_proxies = {};
    

    
    UI_TOUCH_STATE touch_state = UI_TOUCH_STATE_NONE;
    UI_TOUCH_STATE pen_state   = UI_TOUCH_STATE_NONE;
    
    usize visit_id = 0;
    
    enum struct PARENT_LINK_TYPE {
        NONE,
        FROM,
        TO,
        BIDIRECTIONAL
    } link_type = PARENT_LINK_TYPE::NONE;
    
    enum struct LINK_LABEL_TYPE {
        NONE,
        THEN,
        AND,
        DIRECT_OBJECT,
        AGENT,
        INDIRECT_OBJECT,
        OBJECT,
        GOAL,
    } link_label_type = LINK_LABEL_TYPE::NONE;
    
    
    Instruction* parent = nullptr;
    Instruction* instruction_ref = nullptr;
    Instruction* referring_instruction_next = nullptr;
    
    Instruction* next = nullptr;
    
    inline bool is_root(void)
    {
        return this->parent == nullptr;
    }
    
    dt::Dynamic_Array<Instruction*> children[2] = {};
    
    inline usize child_count(Instruction_CHILD_SIDE_TYPE ch_type)
    {
        return children[ch_type].size();
    }
    inline usize child_count(void)
    {
        return children[0].size() + children[1].size();
    }
    inline usize child_list_count(void)
    {
        return sizeof(children) / sizeof(dt::Dynamic_Array<Instruction*>);
    }
    inline bool child_lists_empty(void)
    {
        return child_list_count() != 0;
    }
    inline dt::Dynamic_Array<Instruction*>& child_list(usize i)
    {
        return children[i];
    }
    
    mtt::Thing_ID thing_id = mtt::Thing_ID_INVALID;
    
    
    mtt::Set<mtt::Thing_ID> things_selection_modified_this_input;
    
    bool link_disabled = false;
    
    dt::Dynamic_Array<Instruction*> siblings;
    
    
    mtt::Box bounds;
    
    NVGcolor color;
    NVGcolor color_selected;
    NVGcolor color_current;
    vec4 text_color = vec4(0.0f);
    
    mtt::String display_label = "";
    
    static mem::Pool_Allocation pool;
    
    bool refers_to_random = false;

    static usize count;
    
    MTT_NODISCARD
    static Instruction* make(void)
    {
        ASSERT_MSG(mtt::is_main_thread(), "...");
        count += 1;
        return mem::alloc_init<Instruction>(&Instruction::pool.allocator);
    }
    
    static void destroy(Instruction** instruction)
    {
        ASSERT_MSG(mtt::is_main_thread(), "...");
        count -= 1;
        mem::deallocate<Instruction>(&Instruction::pool.allocator, *instruction);
        *instruction = nullptr;
    }
    
    bool is_plural = false;
    bool is_specific = false;
    bool more_els_than_expected = false;
    
    inline mtt::String to_string()
    {
        mtt::String out = "(Instruction) { type=[" + this->type + "] " + "kind=[" + this->kind + "] } {\n";
        if (this->type == "ACTION") {
            out += "    ";
            for (auto it = this->prop.begin(); it != this->prop.end(); ++it) {
                out += "    " + (*it)->label + "\n";
            }
        } else if (this->kind == "THING_INSTANCE_SELECTION") {
            for (auto it = this->thing_id_list.begin(); it != this->thing_id_list.end(); ++it) {
                out += "    " + std::to_string((*it)) + "\n";
            }
        }
        if (!this->annotation.empty()) {
            out += "    @" + this->annotation;
        }
        out += "}\n";
        
        return out;
    }
};

void Instruction_on_destroy(dt::DrawTalk* dt, dt::Instruction* ins);

void Instruction_add_child(Instruction* self, Instruction* child, Instruction_CHILD_SIDE_TYPE ch_type);
mtt::Thing* Instruction_get_proxy_for_thing(Instruction* ins, mtt::World* world, mtt::Thing_ID thing_id);
void Instruction_set_proxy_for_thing(Instruction* ins, mtt::World* world, mtt::Thing_ID thing_id, mtt::Thing_ID proxy);
void Instruction_destroy_proxy_for_thing(Instruction* ins, mtt::World* world, mtt::Thing_ID thing_id);
bool Instruction_warn_multiple_for_specific_single(Instruction* ins);

bool Instruction_requires_selection_of_things(Instruction* ins);

struct DT_Behavior;
struct DT_Behavior_Catalogue;
struct DT_Args {
    dt::DrawTalk*   dt = nullptr;
    Speech_Property* prop = nullptr;
    Speech_Property* prop_next = nullptr;
    DT_Behavior_Catalogue* cat = nullptr;
    DT_Behavior*     behavior = nullptr;
    DT_Behavior*     behavior_next = nullptr;
    uint64           flags = 0;
    uint64           flags_start_condition = 0;
    uint64           flags_end_condition = 0;
    
    mtt::Script_Builder* script_builder = nullptr;
    mtt::Script* script          = nullptr;
    mtt::Script_Instance* caller = nullptr;
    mtt::Gen_Args gen_args = {};
};
struct DT_Args_Ex {
    bool supports_no_src = false;
    bool treat_direct_objects_as_types = false;
    bool treat_prepositions_as_types = false;
};

static mtt::Set<mtt::String> time_value_words = {
    "year",
    "day",
    "week",
    "month",
    "minute",
    "second",
    "millisecond",
    "century",
    "decade",
    "duration",
};
static inline bool word_is_temporal(const mtt::String& key)
{
    return time_value_words.contains(key);
}





static mtt::Set<mtt::String> numeric_value_words = {
    "height",
    "width",
    "depth"
    "length",
    "size",
    "span",
    "breadth",
    "distance",
    "longitude",
    "lattidute",
    "meter",
    "metre",
    "centimeter",
    "centimetre",
    "inch",
    "yard",
    "mile",
    "kilometer",
    "kilometre",
    "tonne",
    "ton",
    "gram",
    "kilogram",
    "weight",
    "mass",
    "magnitude",
    "pound",
    "liter",
    "litre",
    "milligram",
    "unit",
    "degree",
    "radian",
    "temperature",
    "rate",
    "speed",
    "acceleration",
    "velocity",
    "value",
    "cost",
    "price",
    "amount",
    "quantity",
    
    "mm",
    "cm",
    "m",
    "km",
    "ft"
    "in",
    "yd",
    "º",
    "circumference",
    "radius",
    "amplitude",
    "voltage",
    "volt",
    "wattage",
    "watt",
    "amperage",
    "ohm",
    "density",
    "mph"
    "ms",
    "s",
    "h",
    "year",
    "day",
    "week",
    "month",
    "minute",
    "second",
    "time",
    "century",
    "decade",
    "duration",
    "infinity",
    "volume",
    "mg",
    "g",
    "dollar",
    "$",
    "£",
    "%",
    "¢",
    "¥",
    "renminbi",
    "euro",
    "Newton",
    "force",
    "friction",
    "torque",
    "Newton-metre",
    "Newton-meter",
    "n",
    "penny",
    "nickel",
    "dime",
    "quarter",
    "half",
    "momentum",
    "€",
    "angle",
    "mb"
    "gb",
    "kb"
};

static inline bool word_is_numeric(const mtt::String& key)
{
    return numeric_value_words.contains(key);
}



struct Meta {
    uint64 timestamp;
};

struct Language_Context;
struct DT_Evaluation_Context;
struct DT_Behavior_Catalogue;

struct Command;
typedef uint64 Command_ID;
static Command_ID Command_ID_INVALID;
struct Command_List;




static constexpr const uint32 ALLOW_NO_PARALLEL  = 0;
static constexpr const uint32 ALLOW_ALL_PARALLEL = 1;


struct DT_Behavior {
    typedef mtt::Bool_Result Procedure_Return;
#define DT_BEHAVIOR_PROC_PARAMS dt::DT_Args& args, void* self_args
    typedef Procedure_Return (*Procedure_Handle)(DT_BEHAVIOR_PROC_PARAMS);

    
    
    
    mtt::Map<mtt::String, mtt::Script_ID> scripts = {};
    
    MTT_NODISCARD
    mtt::Script* Script_lookup(const mtt::String& key)
    {
        mtt::Script_ID* id = nullptr;
        if (mtt::map_try_get(&scripts, key, &id)) {
            return mtt::Script_lookup(*id);
        } else {
            return nullptr;
        }
    }
    
    bool register_script(const mtt::String& key, mtt::Script* script)
    {
        if (script->id == mtt::Script_ID_INVALID) {
            return false;
        }
        
        scripts[key] = script->id;
        
        return true;
    }
    
    
    struct Procedure {
        Procedure_Handle proc = [](DT_BEHAVIOR_PROC_PARAMS) -> auto { return mtt::Result_make(true); };
        
        Procedure_Return operator()(dt::DT_Args& args, DT_Behavior_Catalogue& cat, DT_Behavior& behavior, void* custom_args)
        {
            if (proc == nullptr) {
                return mtt::Result_make(false, (void*)nullptr);
            }
            return proc(args, custom_args);
        }
        
        Procedure_Return operator()(dt::DT_Args& args)
        {
            if (proc == nullptr) {
                return mtt::Result_make(false, (void*)nullptr);
            }
            return proc(args, nullptr);
        }
        
        Procedure(void)
        {
        }
        
        
        Procedure(Procedure_Handle proc)
        {
            this->proc = proc;
        }
    };

    Procedure init{};
    Procedure on_generate{};
    Procedure on_generate_post{};
    Procedure on_end{};
    
    DT_Behavior& set_on_generate(Procedure_Handle proc)
    {
        this->on_generate = proc;
        return *this;
    }
    DT_Behavior& set_on_generate_post(Procedure_Handle proc)
    {
        this->on_generate_post = proc;
        return *this;
    }
    
    
    
    //mtt::Map_Stable<mtt::String, DT_Behavior::Procedure> proc_map = {};
    
    
    
    struct Proc_Key {
        typedef mtt::String VALUE_TYPE;
        static inline VALUE_TYPE VALUE_TYPE_ANY = "ANY";
        static inline VALUE_TYPE VALUE_TYPE_ONE = "ONE";
        static inline VALUE_TYPE VALUE_TYPE_MANY = "MANY";
            
        mtt::String arg = "";
        VALUE_TYPE val = "";//VALUE_TYPE_ANY;
        
        bool operator()(const Proc_Key& k0, const Proc_Key& k1) const
            {return k0.arg == k1.arg;}
        
        bool operator()(Proc_Key& k0, Proc_Key& k1) const
            {return k0.arg == k1.arg;}
    };
    struct proc_key_hash {
        size_t operator()(const Proc_Key& k) const noexcept {
            return std::hash<mtt::String>()(k.arg);
        }
        size_t operator()(Proc_Key& k) const noexcept {
            return std::hash<mtt::String>()(k.arg);
        }
    };
    
    struct proc_key_equal_to {
        bool operator()(const Proc_Key& k0, const Proc_Key& k1) const
            {return k0.arg == k1.arg;}
        
        bool operator()(Proc_Key& k0, Proc_Key& k1) const
            {return k0.arg == k1.arg;}
    };
    
    struct Procedure_Mapping {
        robin_hood::unordered_node_set<Proc_Key, proc_key_hash, proc_key_equal_to> keys = {};
        DT_Behavior::Procedure proc = {};
    };
    
    robin_hood::unordered_node_map<mtt::String, Procedure_Mapping> procedure_map = {};
    
    struct Procedure_Key_Builder {
        Procedure_Mapping mapping = {};
        Procedure_Key_Builder& key_insert(const mtt::String& arg,  const Proc_Key::VALUE_TYPE& val)
        {
            mapping.keys.insert({arg, val});
            return *this;
        }
    };
    
    mtt::String register_procedure(Procedure_Key_Builder& b, DT_Behavior::Procedure&& proc)
    {
        b.mapping.proc = proc;
        std::vector<Proc_Key> key;
        for (auto& entry : b.mapping.keys) {
            key.push_back(entry);
        }
        std::sort(key.begin(), key.end(), [](const Proc_Key& a, const Proc_Key& b) -> bool {
            return a.arg <= b.arg;
        });
        mtt::String str_key = "";
        for (auto& entry : key) {
            str_key += entry.arg + ":" + entry.val + " ";
        }
        if (str_key.empty()) {
            return "";
        }
        
        mtt::String final_key = str_key.substr(0, str_key.size() - 1);
        procedure_map[final_key] = b.mapping;
        return final_key;
    }
    DT_Behavior::Procedure lookup_closest_procedure(Procedure_Key_Builder& b)
    {
        /*
         seen = {}

         for s in inputs:
           seen[s] = True

         best_ss = None
         best_ss_num = 0

         for ss in subsets:
           all_present = True
           for s in ss:
             if s not in seen:
               all_present = False
               break
           if all_present and len(ss) > best_ss_num:
             best_ss_num = len(s)
             best_ss = ss
         */
        Procedure_Mapping* best_match = nullptr;
        usize best_len = 0;
        usize best_val_match = 0;
        
        for (auto& [key, subset] : procedure_map) {
            bool all_present = true;
            usize val_match = 0;
            for (auto& subset_element : subset.keys) {
                auto find_it = b.mapping.keys.find(subset_element);
                if (find_it != b.mapping.keys.end()) {
                    if (find_it->val == subset_element.val) {
                        val_match += 1;
                    }
                } else {
                    all_present = false;
                    break;
                }
            }
            if (all_present) {
                if (subset.keys.size() > best_len) {
                    best_len = subset.keys.size();
                    best_match = &subset;
                    best_val_match = val_match;
                } else if (subset.keys.size() == best_len && val_match > best_val_match) {
                    best_match = &subset;
                    best_val_match = val_match;
                }
            }
        }
        return (best_match != nullptr) ? best_match->proc : nullptr;
    }
    
//    DT_Behavior::Procedure
//
//    DT_Behavior::Procedure register_procedure(const mtt::String& key, DT_Behavior::Procedure&& proc)
//    {
//        proc_map[key] = proc;
//        return proc;
//    }
//    DT_Behavior::Procedure lookup_procedure(const mtt::String& key)
//    {
//        auto it_find = proc_map.find(key);
//        if (it_find != proc_map.end()) {
//            return nullptr;
//        }
//        return it_find->second;
//    }
    
    
    
    
    using Label = robin_hood::pair<mtt::String, mtt::String>;
    dt::Dynamic_Array<Label> label;
    dt::Dynamic_Array<Label> alias_label;
    bool is_alias = false;
    
    uint32 parallel_groups_empty_flag = ALLOW_NO_PARALLEL;
    mtt::Set<mtt::String> parallel_groups;
    
    mtt::String key_primary   = "";
    mtt::String key_secondary = "";
    
    mtt::String alias_key_primary   = "";
    mtt::String alias_key_secondary = "";
    
    bool ends_after_transition = true;
    
    DT_Behavior* active_alias = nullptr;
    
    struct Attribute {
        mtt::String label;
        bool is_required;
        
        Widget widget;
    };
    mtt::Map<mtt::String, Attribute> attributes;
    
    struct Args {
        mtt::World* world;
        dt::DrawTalk* dt;
        dt::DT_Behavior_Catalogue* catalogue;
        DT_Behavior* behavior;
        Speech_Property* current;
        
        dt::Dynamic_Array<Speech_Property*>* siblings;
        Speech_Property* root;
        
        Command_List* cmd_list;
        
        Language_Context* lang_ctx;
        dt::DT_Evaluation_Context* eval_ctx;
        
        rules::Condition_Builder* conditional_builder;
        
        mtt::Map_Stable<mtt::String, DT_Rule_Query> queries;
        
        Command* prev_command = nullptr;
        
        bool is_deferred = false;
        
        bool is_trigger = false;
        bool is_response = false;
        bool is_relation = false;
        bool is_continuous = false;
        

    
        
        static Args make(dt::DrawTalk* dt, DT_Behavior_Catalogue* catalogue, DT_Behavior* behavior, Speech_Property* current, dt::Dynamic_Array<Speech_Property*>* siblings, Speech_Property* root, Command_List* cmd_list);
    };
    
    
    
    Command* start(Args& args)
    {
        return this->on_start(args);
    }
    
//    Command* start_trigger(Args& args)
//    {
//        return this->on_start_trigger(args);
//    }
    
    bool stop(Command_ID id)
    {
        return this->on_stop(id);
    }
    
    Command* transition_to(Command_ID id, Args& args)
    {
        return this->on_transition_to(id, args);
    }
    
    
    
    // how to connect
    Logic_Interface logic_interfaces;
    Logic_Bridges logic_bridges;
    
    
    
//    struct Config {
//        typedef enum OPTION {
//            OPTION_INVALID         = 0,
//            OPTION_SPEECH_PROPERTY = 1,
//            OPTION_MAP             = 2,
//        } OPTION;
//
//        using Result = mtt::Result<usize, bool>;
//
//        static constexpr cstring OPTION_STRINGS[] = {
//            [OPTION_INVALID]         = "OPTION_INVALID",
//            [OPTION_SPEECH_PROPERTY] = "OPTION_SPEECH_PROPERTY",
//            [OPTION_MAP]             = "OPTION_SPEECH_MAP",
//            NULL
//        };
//
//        static cstring option_string(OPTION option)
//        {
//            return OPTION_STRINGS[option];
//        }
//
//        bool is_valid = false;
//        DT_Behavior* behavior                      = nullptr;
//        OPTION       option                        = OPTION_INVALID;
//
//        Speech_Property* root_prop                = nullptr;
//        Speech_Property::Prop_List* containing_proplist = nullptr;
//        //usize idx_in_prop_list                    = 0;
//        Speech_Property* prop;
//
//        dt::Command* command                      = nullptr;
//
//
//    } config;
//
//    DT_Behavior()
//    {
//        this->config.behavior = this;
//    }
    
    //mtt::Map<mtt::String, Config> configure_procs;
    

    
//    my::Function<Config::Result(DT_Behavior::Config*)> commit = [](DT_Behavior::Config* config) -> Config::Result
//    {
//        Config::Result result;
////        if (config->root_prop == nullptr || config->containing_proplist == nullptr || config->prop == nullptr || config->option == Config::OPTION_INVALID) {
////            result.status = false;
////            return result;
////        }
//        int BP = 0;
//        result.status = true;
//        return result;
//    };
    
    using On_Start_proc_type = my::Function<Command*(Args&)>;
    //using On_Start_Trigger_proc_type = my::Function<Command*(Args&)>;
    using On_Stop_proc_type  = my::Function<bool(Command_ID)>;
    using On_Transition_to_proc_type = my::Function<Command*(Command_ID, Args&)>;
    
    
    /** callback for behavior start
     */
    On_Start_proc_type on_start                      = [](Args& args) -> Command* {  return nullptr; };
    /** callback for behavior condition check
     */
    //On_Start_Trigger_proc_type on_start_trigger                      = [](Args& args) -> Command* {  return nullptr; };
    /** callback for behavior stop
     */
    On_Stop_proc_type on_stop                      = [](Command_ID id) -> bool { return false; };
    /** callback for behavior transition
     */
    On_Transition_to_proc_type on_transition_to  = [](Command_ID id, Args& args) { return nullptr; };
    
    /** set callback for behavior start
     *
     * @param proc The procedure callback
     */
    void set_on_start(On_Start_proc_type proc)
    {
        this->on_start = proc;
    }
    
    void set_on_stop(On_Stop_proc_type proc)
    {
        this->on_stop = proc;
    }
    
    void set_on_transition_to(On_Transition_to_proc_type proc)
    {
        this->on_transition_to = proc;
    }
};

using Behavior = DT_Behavior;


struct Command {
    
    Command_ID id = Command_ID_INVALID;
    
    static Command_ID next_id;
    
    mtt::String label = "";
    // root
    mtt::Thing_ID root_thing_id = mtt::Thing_ID_INVALID;
    mtt::Map_Stable<mtt::String, dt::Dynamic_Array<mtt::Thing_Ref>> element_map;
    
    // use for stopping a command "owned" by a Thing
    
    static mtt::Map_Stable<Command_ID, Command*> commands;
    static std::unordered_map<mtt::Thing_ID, dt::Dynamic_Array<Command_ID>> thing_agent_to_cmd;
    
    dt::Dynamic_Array<DT_Behavior*> behavior_list;
    DT_Args args = DT_Args();
    
    bool finalized = false;
    
    bool must_be_canceled = true;
    
    Command_ID successor_id = Command_ID_INVALID;
    Command_ID predecessor_id = Command_ID_INVALID;
    
    
    Logic_Interface interop;
    
    
//    enum STATE {
//        STATE_WAITING_FOR_FEEDBACK,
//        STATE_RUNNING,
//        STATE_STOPPED,
//    } state;
    
    rules::Rule_ID rule = rules::Rule_ID_INVALID;
    
    //Command_ID next_cmd  = Command_ID_INVALID;
    
    
    
    
    my::Function<bool(Command*)> stop   = [](Command* cmd_id) -> bool { return false; };
    my::Function<void(Command*)> cancel = [](Command* cmd_id) {};
    
    Meta meta = {};
    
    
    
    static mem::Pool_Allocation pool;
    
    MTT_NODISCARD static Command* make()
    {
        auto* cmd = mem::alloc_init<Command>(&Command::pool.allocator);
        Command::init(cmd);
        return cmd;
    }
    
    static void init(Command* cmd)
    {
        Command::next_id += 1;
        cmd->id = Command::next_id;
        Command::commands.insert({cmd->id, cmd});
    }
    
    static Command* lookup(Command_ID id)
    {
        auto find_it = Command::commands.find(id);
        if (find_it == Command::commands.end()) {
            return nullptr;
        }
        
        return find_it->second;
    }
    
    static void destroy(Command_ID id)
    {
        Command* cmd = Command::lookup(id);
        if (cmd == nullptr) {
            return;
        }
        Command::commands.erase(id);
        mem::deallocate<Command>(&Command::pool.allocator, cmd);
    }
    static void destroy(Command* cmd)
    {
        cmd = Command::lookup(cmd->id);
        if (cmd == nullptr) {
            return;
        }
        
        Command::commands.erase(cmd->id);
        mem::deallocate<Command>(&Command::pool.allocator, cmd);
    }
};

struct Command_List {
    dt::Dynamic_Array<dt::Command*> cmds;
};




enum CONFIRM_PHASE {
    CONFIRM_PHASE_SPEECH,
    CONFIRM_PHASE_ANIMATE,
    CONFIRM_PHASE_WAITING,
};

struct Language_Context;



namespace ex {

struct Query_Search {
    usize* ident_counter = 0;
    
    void init(usize* ident_counter)
    {
        this->ident_counter = ident_counter;
    }
    
    mtt::String get_query_ident()
    {
        mtt::String str="" ;
        usize x=(*this->ident_counter)++, y=1; do { str +='A' + x % ('Z'-'A'+1); x/=('Z'-'A'+1); y *= 26; } while (y < x);
        return str;
    }
};

struct Exec_Args {
    Speech_Property* prop;
    DrawTalk* dt;
    Parse* parse;
    Speech_Property* root;
    Speech_Property* root_cmd;
    Language_Context* lang_ctx;
    mtt::Set_Stable<mtt::Thing_ID>* already_chosen;
    DT_Rule_Query_Builder builder;
    

    usize* ident_counter = 0;
    
    usize* deixis_i = 0;
    
    bool is_relation = false;
    
    mtt::Set_Stable<Speech_Property*> processed;
    
    bool treat_as_type = false;
    
    struct Deferred {
        Instruction* ins;
        Speech_Property* prop;
        Speech_Property* ref_prop;
        bool(*proc)(Deferred*);
    };
    dt::Dynamic_Array<Deferred>* deferred;
    
    // char alphabet[] = "abc123*&"; and then str += alphabet[x%(sizeof(alphabet)-1)]
    mtt::String get_query_ident()
    {
        mtt::String str="" ;
        usize x=(*this->ident_counter)++, y=1; do { str +='A' + x % ('Z'-'A'+1); x/=('Z'-'A'+1); y *= 26; } while (y < x);
        return str;
    }
     
    
    
    Exec_Args() :
    prop(nullptr), dt(nullptr), parse(nullptr), root(nullptr), root_cmd(nullptr), lang_ctx(nullptr)
    {}
};

enum struct MTT_NODISCARD Exec_Status {
    OK,
    FAILED,
};

struct MTT_NODISCARD Exec_Return {
    bool is_missing_info = false;
    Instruction* ins;
};



cstring Exec_Status_Strings[] = {
    [(uint64)Exec_Status::OK] = "OK",
    [(uint64)Exec_Status::FAILED] = "FAILED",
};

using Exec_Result = mtt::Result<Exec_Return, Exec_Status>;
inline Exec_Result exec_failed() { return (Exec_Result){ .status = Exec_Status::FAILED}; };
inline Exec_Result exec_ok(Instruction* ins) { return (Exec_Result){ .status = Exec_Status::OK, .value = (Exec_Return){.is_missing_info = false, .ins = ins}}; };

}

struct Layout {
    vec2 pos       = vec2(0.0f);
    float32 height = 0.0f;
    vec2 pad       = vec2(10.0f);
    float32 parent_width = 0.0f;
};


struct DT_Behavior;
struct DT_Behavior_Catalogue;

typedef enum AMBIGUITY_TYPE {
    AMBIGUITY_TYPE_ACTION_UNKNOWN,
    AMBIGUITY_TYPE_ACTION_MULTIPLE_CHOICES,
} AMBIGUITY_TYPE;

struct Disambiguation_Action_Alternative {
    DT_Behavior* behavior = nullptr;
};
struct Disambiguation {
    dt::Instruction* ins = nullptr;
    AMBIGUITY_TYPE ambiguity_type = AMBIGUITY_TYPE_ACTION_UNKNOWN;
    bool results_ready = false;
    dt::Dynamic_Array<Disambiguation_Action_Alternative> action_alternatives;
};

struct DT_Evaluation_Context {
    ex::Exec_Args                   args;
    
    std::deque<Command_ID>             cmds;
    
    mtt::Map_Stable<mtt::Thing_ID, Command_ID>         thing_to_cmds;
    mtt::Map_Stable<mtt::Thing_ID, dt::rules::Rule_ID> thing_to_associated_rules;
    
    dt::Dynamic_Array<Instruction*> instructions_;
    inline dt::Dynamic_Array<Instruction*>& instructions()
    {
        return this->instructions_;
    }
    dt::Dynamic_Array<Instruction*> instructions_saved_;
    inline dt::Dynamic_Array<Instruction*>& instructions_saved()
    {
        return this->instructions_saved_;
    }
    usize next_instruction_visit_id = 0;
    dt::Dynamic_Array<Widget>       widgets;
    dt::Dynamic_Array<Disambiguation> instructions_to_disambiguate;
    bool prompting_for_different_actions = false;
    
    dt::Instruction* instruction_selected_with_touch_ = nullptr;
    dt::Instruction*& instruction_selected_with_touch()
    {
        return this->instruction_selected_with_touch_;
    }
    dt::Instruction* instruction_selected_with_pen_ = nullptr;
    dt::Instruction*& instruction_selected_with_pen()
    {
        return this->instruction_selected_with_pen_;
    }
    
    mtt::Map<Speech_Property*, std::vector<Speech_Property*>> referrer_map = {};
    mtt::Map<Speech_Property*, std::vector<Speech_Property*>> referrer_map_reversed = {};
    
    CONFIRM_PHASE phase = CONFIRM_PHASE_SPEECH;
    
    //
    dt::Dynamic_Array<UI_Feedback_Node*> feedback_nodes;
    dt::Dynamic_Array<mtt::Thing_ID> root_feedback_ui_list;
    Layout layout;
    
    float32 font_size = 24.0f * 1.2;
    cstring font_face = "sans";
    int align = NVG_ALIGN_LEFT|NVG_ALIGN_TOP;
    
    dt::Dynamic_Array<mtt::Thing_ID> thing_list;
    
    mtt::Thing_ID root_thing_id = mtt::Thing_ID_INVALID;
};

CONFIRM_PHASE get_confirm_phase();

void set_confirm_phase(CONFIRM_PHASE phase);

struct DT_Behavior_Config;

struct RegisteredFunc {
    std::string name;
    void (*func)(dt::DrawTalk* dt, DT_Behavior_Config* conf);
};

struct DT_Behavior_Config {
    std::string name;
    inline static std::vector<RegisteredFunc> list__;
    inline static std::vector<RegisteredFunc>& list(void)
    {
        return DT_Behavior_Config::list__;
    }
};

//DT_Behavior_Config DT_Behavior_Config_make(const mtt::String& name, void (*init)(dt::DrawTalk*))
//{
//    DT_Behavior_Config conf;
//    conf.name = name;
//    conf.init = init;
//    return conf;
//}




std::vector<RegisteredFunc>& DT_Behavior_preloaded_behavior_configs(void);

void DT_Behavior_Catalogue_register_module(RegisteredFunc rf);

struct DT_Behavior_Catalogue {
    
    using Alternatives_Map = mtt::Map_Stable<mtt::String, DT_Behavior>;
    
    // label to form to behavior
    mtt::Map_Stable<mtt::String, Alternatives_Map> map;
    mtt::Map_Stable<mtt::String, mtt::Set<DT_Behavior*>> parallel_groups;
    
    void insert_in_parallel_group(const mtt::String& group_label, const mtt::String& label, const mtt::String& sub_label)
    {
        DT_Behavior* b;
        if (!this->lookup(label, sub_label, &b)) {
            return;
        }
        
        b->parallel_groups.insert(group_label);
        this->parallel_groups[group_label].insert(b);
    }
    void remove_from_parallel_group(const mtt::String& group_label, const mtt::String& label, const mtt::String& sub_label)
    {
        DT_Behavior* b;
        if (!this->lookup(label, sub_label, &b)) {
            return;
        }
        
        auto found_it = b->parallel_groups.find(group_label);
        if (found_it != b->parallel_groups.end()) {
            b->parallel_groups.erase(found_it);
            auto found_group_it = this->parallel_groups.find(group_label);
            auto found_entry = found_group_it->second.find(b);
            found_group_it->second.erase(found_entry);
        }
    }
    
    struct On_Off_Shortcut_Pair {
        mtt::String on;
        mtt::String off;
    };
    mtt::Map_Stable<mtt::String, On_Off_Shortcut_Pair> on_off_shortcut_behaviors = {};
    void on_off_shortcut_behaviors_add(const mtt::String& on_label, const mtt::String& off_label)
    {
        this->on_off_shortcut_behaviors.insert({on_label, (On_Off_Shortcut_Pair){on_label, off_label}});
    }
    On_Off_Shortcut_Pair* is_on_off_shortcut_behavior(const mtt::String& label) {
        auto find_it = this->on_off_shortcut_behaviors.find(label);
        if (find_it != this->on_off_shortcut_behaviors.end()) {
            return &find_it->second;
        }
        return nullptr;
    }

#define LABEL_SUB_LABEL_SEPARATOR ":"
    DT_Behavior* insert(const mtt::String& label, const mtt::String& sub_label, bool is_alias, const mtt::String& alias_label, const mtt::String& alias_sub_label)
    {

        // TODO: sublabel support
        
        DT_Behavior* entry_ptr = nullptr;
        if (this->lookup(label, sub_label, &entry_ptr)) {
            ASSERT_MSG(false, "Should not exist already!");
            return entry_ptr;
        }
        
        dt::verb_add(label);
        
        auto& entry = this->map[label][sub_label];
        entry.label.push_back(robin_hood::pair<mtt::String, mtt::String>(label, sub_label));
        entry.is_alias = is_alias;
        entry.key_primary = label;
        entry.key_secondary = sub_label;
        if (is_alias) {
            entry.alias_label.push_back(robin_hood::pair<mtt::String, mtt::String>(alias_label, alias_sub_label));
            entry.alias_key_primary = alias_label;
            entry.alias_key_secondary = alias_sub_label;
            
            DT_Behavior* base = nullptr;
            if (!this->lookup(alias_label, alias_sub_label, &base)) {
                base = this->insert(alias_label, alias_sub_label, false, "", "");
                ASSERT_MSG(false, "Should not support alias of non-existent behavior for now!");
            }
            
            entry.set_on_start(base->on_start);
            entry.set_on_stop(base->on_stop);
            entry.set_on_transition_to(base->on_transition_to);
            entry.logic_interfaces = base->logic_interfaces;
            
            entry.init = base->init;
            entry.on_generate = base->on_generate;
            entry.on_generate_post = base->on_generate_post;
            entry.on_end = base->on_end;
            
            auto* base_scripts = &base->scripts;
            entry.scripts.reserve(base_scripts->size());
            for (auto [key, script_id] : *base_scripts) {
                mtt::Script* sc = mtt::Script_lookup(script_id);
                if (sc == nullptr) {
                    continue;
                }
                
                entry.scripts[key] = sc->alias_make()->id;
            }
            
        }
           
        return &entry;
    }
    inline DT_Behavior& define_action(const mtt::String& label, const mtt::String& sub_label, bool is_alias, const mtt::String& alias_label, const mtt::String& alias_sub_label)
    {
        return *this->insert(label, sub_label, is_alias, alias_label, alias_sub_label);
    }
    inline DT_Behavior& define_action(const mtt::String& label, const mtt::String& sub_label)
    {
        return *this->insert(label, sub_label, false, "", "");
    }
    
    struct Compare_Routine_increasing {
        using ARG_TYPE = robin_hood::pair<mtt::String, mtt::String>;
        bool operator()(ARG_TYPE& a, ARG_TYPE& b) const {
            if (a.first == b.first) {
                return (a.second <= b.second);
            } else {
                return (a.first <= b.first);
            }
        }
        
        
//        bool operator()(ARG_TYPE a, ARG_TYPE b) {
//            if (a.first == b.first) {
//                return (a.second <= b.second);
//            } else {
//                return (a.first <= b.first);
//            }
//        }
    };
    
    struct Compare_Routine_increasing_with_data {
        using ARG_TYPE = std::tuple<mtt::String, mtt::String, void*>;
        bool operator()(ARG_TYPE& a, ARG_TYPE& b) const {
            if (std::get<0>(a) == std::get<0>(b)) {
                return (std::get<1>(a) <= std::get<1>(b));
            } else {
                return (std::get<0>(a) <= std::get<0>(b));
            }
        }
        
        
//        bool operator()(ARG_TYPE a, ARG_TYPE b) const {
//            if (std::get<0>(a) == std::get<0>(b)) {
//                return (std::get<1>(a) <= std::get<1>(b));
//            } else {
//                return (std::get<0>(a) <= std::get<0>(b));
//            }
//        }
    };
    
    static void create_sort_key_and_sort_elements_in_place(dt::Dynamic_Array<std::tuple<mtt::String, mtt::String, void*>>& in, robin_hood::pair<mtt::String, mtt::String>* out_key)
    {
        std::sort(in.begin(), in.end(), Compare_Routine_increasing_with_data());
        
        mtt::String key = std::get<0>(in[0]);
        mtt::String key2 = std::get<1>(in[0]);
        for (usize i = 1; i < in.size(); i += 1) {
            key  += "," +  std::get<0>(in[i]);
            key2 += "," + std::get<1>(in[i]);
        }
        
        out_key->first = key;
        out_key->second = key2;
    }
    
    DT_Behavior* insert_composite(dt::Dynamic_Array<robin_hood::pair<mtt::String, mtt::String>>& label, bool is_alias, dt::Dynamic_Array<robin_hood::pair<mtt::String, mtt::String>>& alias_label)
    {
        DT_Behavior behavior;
        behavior.label = label;
        
        std::sort(behavior.label.begin(), behavior.label.end(), Compare_Routine_increasing());
        
        mtt::String key1 = behavior.label[0].first;
        for (usize i = 1; i < behavior.label.size(); i += 1) {
            key1 += "," + behavior.label[i].first;
        }
        mtt::String key2 = behavior.label[0].second;
        for (usize i = 1; i < behavior.label.size(); i += 1) {
            key2 += "," + behavior.label[i].second;
        }
        
        DT_Behavior* behavior_ptr = nullptr;
        if (this->lookup(key1, key2, &behavior_ptr)) {
            return behavior_ptr;
        }
        
        this->map[key1][key2] = behavior;
        auto& entry    = this->map[key1][key2];
        
        entry.is_alias = is_alias;
        if (is_alias) {
            mtt::String alias_key = "";
            
            
            entry.alias_label     = alias_label;
            std::sort(behavior.alias_label.begin(), behavior.alias_label.end(), Compare_Routine_increasing());
            
#define first_(arg__) arg__.first
#define second_(arg__) arg__.second
            mtt::String key = first_(behavior.alias_label[0]);
            mtt::String key2 = second_(behavior.alias_label[0]);
            for (usize i = 1; i < behavior.alias_label.size(); i += 1) {
                key  += "," +  first_(behavior.alias_label[i]);
                key2 += "," +  second_(behavior.alias_label[i]);
            }
#undef first_
#undef second_
            
            entry.alias_key_primary = key;
            entry.alias_key_secondary = key2;
            
        }
        
        
        return &entry;
    }
#undef LABEL_SUB_LABEL_SEPARATOR
    

    
    
    MTT_NODISCARD inline bool lookup(const mtt::String& key, const mtt::String& key_secondary, DT_Behavior** out)
    {
        //DT_print("lookup %s %s\n", key.c_str(), key_secondary.c_str());
        DT_Behavior_Catalogue* cat = this;
        Alternatives_Map* entry_map = nullptr;;

        auto outer_find = cat->map.find(key);
        if (outer_find == cat->map.end()) {
            return false;
        } else {
            entry_map = &(outer_find->second);
        }
        
        auto inner_find = entry_map->find(key_secondary);
        if (inner_find == entry_map->end()) {
            return false;
        }
        
        DT_Behavior* behavior = &inner_find->second;
        
        
        *out = behavior;
        return true;
    };
    
    
    MTT_NODISCARD inline bool lookup(const mtt::String& key, Alternatives_Map** out)
    {
        DT_Behavior_Catalogue* cat = this;
    
        auto outer_find = cat->map.find(key);
        if (outer_find == cat->map.end()) {
            return false;
        } else {
            *out = &(outer_find->second);
            return true;
        }
    }
    
};


template <typename PROC>
inline static DT_Behavior& define_action(DT_Behavior_Catalogue& cat, const mtt::String& label, const mtt::String& sub_label, bool is_alias, const mtt::String& alias_label, const mtt::String& alias_sub_label, PROC&& proc=[](DT_Behavior& def) -> void {});

template <typename PROC>
inline static DT_Behavior& define_action(DT_Behavior_Catalogue& cat, const mtt::String& label, const mtt::String& sub_label, bool is_alias, const mtt::String& alias_label, const mtt::String& alias_sub_label, PROC&& proc)
{
    auto& def = cat.define_action(label, sub_label, is_alias, alias_label, alias_sub_label);
    proc(def);
    return def;
}



template <typename PROC>
inline static DT_Behavior& define_action(DT_Behavior_Catalogue& cat, const mtt::String& label, const mtt::String& sub_label, PROC&& proc);
template <typename PROC>
inline static DT_Behavior& define_action(DT_Behavior_Catalogue& cat, const mtt::String& label, const mtt::String& sub_label, PROC&& proc)
{
    auto& def = cat.define_action(label, sub_label);
    proc(def);
    return def;
}

template <typename PROC>
inline static DT_Behavior& define_action_alias(DT_Behavior_Catalogue& cat, const mtt::String& label, const mtt::String& sub_label, const mtt::String& alias_label, const mtt::String& alias_sub_label, PROC&& proc);
template <typename PROC>
inline static DT_Behavior& define_action_alias(DT_Behavior_Catalogue& cat, const mtt::String& label, const mtt::String& sub_label, const mtt::String& alias_label, const mtt::String& alias_sub_label, PROC&& proc)
{
    return define_action(cat, label, sub_label, true, alias_label, alias_sub_label, proc);
}


inline static DT_Behavior& define_action(DT_Behavior_Catalogue& cat, const mtt::String& label, const mtt::String& sub_label)
{
    return cat.define_action(label, sub_label);
}


inline static DT_Behavior& define_action_alias(DT_Behavior_Catalogue& cat, const mtt::String& label, const mtt::String& sub_label, const mtt::String& alias_label, const mtt::String& alias_sub_label)
{
    return define_action(cat, label, sub_label, true, alias_label, alias_sub_label, [](DT_Behavior& def) {});
}

struct Label_Assignment_Guess {
    std::vector<dt::ID> to_resolve;
    int64 idx = -1;
};

struct Label_Assignment_Guessing {
    mtt::Map_Stable<dt::ID, Label_Assignment_Guess> guesses;
    
    bool find_or_make(dt::ID id, Label_Assignment_Guess** guess)
    {
        if (!mtt::map_try_get(&this->guesses, id, guess)) {
            mtt::map_set(&this->guesses, id, Label_Assignment_Guess(), guess);
            return false;
        }
        return true;
    }
};

struct Deixis_Context {
    char tmp;
};

void try_labeling_via_deixis_match(dt::DrawTalk* dt);

static const usize parse_history_size = 3;
struct Language_Context {
    // temporary
    std::deque<Parse*> parse_q_;
    
    inline std::deque<Parse*>& parse_q(void)
    {
        return this->parse_q_;
    }
    
    // final
    std::deque<Parse*> parse_history_;
    
    inline std::deque<Parse*>& parse_history(void)
    {
        return this->parse_history_;
    }
    
    Label_Assignment_Guessing type_label_guessing;
    
    // deixis
    dt::ID latest_id = 0;
    dt::ID deixis_resolve_offset_i = 0;
    isize prev_deixis_len = -1;
    dt::Dynamic_Array<dt::Speech_Property*> deixis = {};
    
    void set_current_id(dt::ID id)
    {
        // change in ID
        if (id != latest_id) {
            //DT_print("ID CHANGED FROM %llu to %llu=====\n", latest_id, id);
            deixis_resolve_offset_i = 0;
            deixis.clear();
        }
        latest_id = id;
    }
    
    void deixis_reset(void)
    {
        deixis_resolve_offset_i = 0;
        deixis.clear();
        prev_deixis_len = -1;
    }
    
    dt::Speech_Property* use_deixis(void)
    {
        if (deixis_resolve_offset_i >= deixis.size()) {
            return nullptr;
        }
        
        dt::Speech_Property* prop = deixis[deixis_resolve_offset_i];
        deixis_resolve_offset_i += 1;
        return prop;
    }
    
    void deixis_push(dt::Speech_Property* prop)
    {
        deixis.push_back(prop);
    }
    
    void deixis_sort(void)
    {
        std::sort(deixis.begin(), deixis.end(), [](dt::Speech_Property* const & a, dt::Speech_Property* const & b) -> bool {
            return b->token->i > a->token->i;
        });
    }
    //
    
    struct Token_Buffer {
        std::deque<Speech_Token*> token_q;
        
        void fill_with(dt::Dynamic_Array<Speech_Token*>& token_list)
        {
            for (usize i = 0; i < token_list.size(); i += 1) {
                this->token_q.push_back(token_list[i]);
            }
        }
        
        void remove_back(usize n)
        {
            for (usize i = 0; i < n; i += 1) {
                this->token_q.pop_back();
            }
        }
        
        void remove_front(usize n)
        {
            ASSERT_MSG(this->token_q.size() >= n, "size is too small, so some sort of de-synch happened!");
            for (usize i = 0; i < n; i += 1) {
                this->token_q.pop_front();
            }
        }
        
    } token_buffer;
    
    DT_Evaluation_Context eval_ctx;
    
    DT_Behavior_Catalogue behavior_catalogue;
//    mtt::Action_Collection actions;
//    mtt::Entity_Collection entities;
    dt::Behavior_Catalogue behaviors;
    dt::Word_Dictionary dictionary;
    
    Parse* current_parse = nullptr;
    
    
    
    mtt::Set<mtt::Thing*> cards;
    
    std::deque<rules::Rule*> active_rules;
    mtt::Map_Stable<mtt::String, dt::Dynamic_Array<rules::Rule*>> rule_map;
    
    
    Language_Context();
    
};



struct alignas(16) BG_Data_Context {
    DrawTalk* dt = nullptr;
    //Natural_Language_Event ev;
    //Speech_Info_View sp_view;
    //mtt::World* world = nullptr;
    dt::Selection_Recording selection_recording;
    Natural_Language_Data* nl_data_;
    usize nl_data_idx_;
    inline Natural_Language_Data* nl_data(void)
    {
        return this->nl_data_;
    }
    inline Natural_Language_Data** nl_data_ptr(void)
    {
        return &this->nl_data_;
    }
    inline void nl_data(Natural_Language_Data* nl_data)
    {
        this->nl_data_ = nl_data;
    }
    inline void nl_data_idx(usize idx)
    {
        //MTT_print("Set idx to: %llu doc queue size%zu\n", idx, nl_data_->doc_queue.size());
        this->nl_data_idx_ = idx;
    }
    
    Parse* parse_ = nullptr;
    inline Parse* parse(void)
    
    {
        return this->parse_;
    }
    inline Parse** parse_ptr(void)
    {
        return &this->parse_;
    }
    inline void parse(Parse* parse)
    {
        this->parse_ = parse;
    }
    
    
    
    struct Result {
        Speech_Property* props = nullptr;
        bool is_finished;
    } result;
};





struct alignas(16) DrawTalk {
    
    //static bool is_ready;
    dt::UI ui;
    dt::Pen pen;
    dt::Recorder recorder;
    dt::Scene_Context scn_ctx;
    mtt::Map<usize, Selection> selection_map;
    mtt::World* mtt;
    MTT_Core* core;
    Language_Context lang_ctx;

    //DrawTalk_Text dt_text_view;
    
    Behaviors behaviors;
    
    Sketch_Library sketch_library = {};
    
    uint64 mode;
    
    // temp contained type
    //std::deque<int> event_queue;
    
    //mtt::Publish_Subscribe subscriptions;
    
    mem::Pool_Allocation node_allocation;
    mem::Pool_Allocation parse_allocation;
    mem::Pool_Allocation token_allocation;
    mem::Pool_Allocation list_node_allocation;
    mem::Pool_Allocation tree_node_allocation;
    mem::Pool_Allocation instructions;
    mem::Pool_Allocation rules;
    mem::Pool_Allocation triggers;
    mem::Pool_Allocation trigger_clauses;
    mem::Pool_Allocation responses;
    mem::Pool_Allocation response_clauses;
    mem::Pool_Allocation rule_variables;
    mem::Pool_Allocation bg_ctx_data;
    
    MTT_Logger parse_logger = {};
    
    MTT_Tree sys_tree;
    //System_Tree_Search search;
    
    System_Evaluation_Context sys_eval_ctx;
    
    std::unordered_map<usize, std::function<void(mtt::String&, void*, usize)>> word_data_query_map;
    
    mtt::String indent_str = "";
    
    int is_waiting_on_results = false;
    
    void do_indent(void)
    {
        this->indent_str += "    ";
    }
    
    void undo_indent(void)
    {
        if (this->indent_str.empty()) {
            return;
        }
        
        for (usize i = 0; i < 4; i += 1) {
            this->indent_str.pop_back();
        }
    }
    
    
    static DrawTalk* global_ctx;
    static DrawTalk* ctx() { return global_ctx; }
    
    fmt::Formatter formatter;
    
    my::Function<void(void)> deferred_confirm = []() {};
    
//    struct Relation_to_Remove {
//        my::Function<void(void)> procedure;
//    };
//    dt::Dynamic_Array<Relation_to_Remove> relations_to_remove;
};



Word_Dictionary* word_dict(void);

void DrawTalk_init(MTT_Core* core, mtt::World* world, DrawTalk* dt);

static const vec4 default_text_color = {245.0f/255.0f, 245.0f/255.0f, 220.0f/255.0f, 1.0f};

struct Lang_Eval_Args {
    dt::DrawTalk* dt;
    dt::Parse* parse;
    dt::Speech_Token* curr_dep_token;
};

Speech_Property* lang_compile(dt::DrawTalk* dt, dt::Parse* parse);

void lang_eval_compiled(dt::DrawTalk* dt, dt::Parse* parse, Speech_Property* comp);


void on_load(mtt::String& path, mtt::String& data);

template <typename Proc>
usize query_related_info_for_unknown_word(dt::DrawTalk* dt, const mtt::String& word, const mtt::String& part_of_speech, Proc&& proc)
{
    
    struct Info {
        dt::DrawTalk* dt;
        Proc proc;
        Info(dt::DrawTalk* dt, Proc p) : dt(dt), proc(p) {}
    };
    Info info(dt, proc);
    
    mtt::String key = word + " " + part_of_speech;
    usize ID = dt->core->query_for_data(dt->core, key, [](void* ctx, usize ID) {
        Info* info = (Info*)ctx;
        ASSERT_MSG(info->dt->word_data_query_map.find(ID) == info->dt->word_data_query_map.end(), "SHOULD BE UNIQUE ID");
        info->dt->word_data_query_map.insert({ID, info->proc});
    }, &info);
    
    return ID;
}

template <typename Proc>
usize query_related_info_for_unknown_word(dt::DrawTalk* dt, const mtt::String& key, Proc&& proc)
{
    struct Info {
        dt::DrawTalk* dt;
        Proc proc;
        Info(dt::DrawTalk* dt, Proc p) : dt(dt), proc(p) {}
    };
    Info info(dt, proc);
    
    usize ID = dt->core->query_for_data(dt->core, key, [](void* ctx, usize ID) {
        Info* info = (Info*)ctx;
        info->dt->word_data_query_map.insert({ID, info->proc});
    }, &info);
    
    return ID;
}

void on_query_result(mtt::String& data, usize ID);

void text_view_clear(DrawTalk* dt);

void evaluate_completed_speech_events(dt::DrawTalk* dt, MTT_Core* core, mtt::World* world);

struct Regenerate_Operation {
    enum struct TYPE {
        DELETE,
        RENAME,
        CONFIRM,
    } type;
    
    bool do_send = true;
    
    mtt::Set_Stable<Speech_Token*> token;
};

void regenerate_text_view(dt::DrawTalk* dt, dt::Parse& parse, Regenerate_Operation& op, bool compute_semantics = true);
void generate_text_view_from_Parse(dt::DrawTalk* dt, dt::Parse& parse);

extern mtt::String debug_msg;

void on_frame(void);


inline static bool can_label_thing(mtt::Thing* thing)
{
    return thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH || thing->archetype_id == mtt::ARCHETYPE_TEXT || thing->archetype_id == mtt::ARCHETYPE_NUMBER || thing->archetype_id == mtt::ARCHETYPE_SLIDER;
}

extern bool drew_this_frame;

void handle_speech_phase_change(dt::DrawTalk& dt_ctx, mtt::Thing* thing);
void handle_speech_phase_change(dt::DrawTalk& dt_ctx);



mtt::Thing* word_multiply_procedure_make(dt::Word_Val_Pair words[], void* args);


bool handle_context_sensitive_command(dt::DrawTalk* dt);
bool handle_context_sensitive_command_clear(dt::DrawTalk* dt);

#define DT_CONTEXT_BUTTON_NAME "find"
#define DT_SPEECH_COMMAND_BUTTON_NAME "language action"

void on_resize(MTT_Core* core, dt::DrawTalk* dt, vec2 new_size);

}

#endif /* drawtalk_hpp */
