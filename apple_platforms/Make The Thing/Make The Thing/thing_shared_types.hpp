//
//  thing_shared_types.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/24/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef thing_shared_types_hpp
#define thing_shared_types_hpp



//#include "cpp_common.hpp"

namespace mtt {


struct World;
struct Thing;
struct Thing_Archetype;
struct Representation;
typedef Representation Rep;
struct External_Thing;
struct External_World;


typedef usize Thing_ID;

using Thing_Child_List = std::vector<mtt::Thing_ID>;


typedef uint64 Thing_Archetype_ID;
typedef Thing_ID Thing_Group_ID;
typedef Thing_ID Thing_Proxy_ID;

struct Thing_Proxy_Info {
    Thing_Proxy_ID id;
    usize scene_id;
};

typedef uint64 Context_ID;

constexpr const Context_ID Context_ID_ROOT     = 0;
constexpr const Context_ID Context_ID_SUB      = 1;
constexpr const Context_ID Context_ID_DEFAULT = Context_ID_ROOT;

typedef uint64 Script_ID;
constexpr const Script_ID Script_ID_INVALID = 0;

struct Script;
struct Script_Instance;
typedef uintptr Script_Instance_Ref;

constexpr auto FOREVER = ULLONG_MAX;
constexpr auto RANDOM_TIME = -1.0f;

struct Connection;
struct Eval_Connection_Graph;

struct Script_Context;
struct Script_Contexts;

typedef mtt::Map<Thing_ID, mtt::External_Thing> External_Mappings;

bool Thing_try_get(mtt::World* world, Thing_ID id, Thing** thing);
Thing* Thing_try_get(mtt::World* world, Thing_ID id);
Thing* Thing_get(mtt::World* world, Thing_ID id);
void Thing_get(mtt::World* world, Thing_ID id, Thing** thing);
bool Thing_Archetype_try_get(mtt::World* world, Thing_Archetype_ID id, Thing_Archetype** arch);
void Thing_Archetype_get(mtt::World* world, Thing_Archetype_ID id, Thing_Archetype** arch);

mtt::Thing_ID id(mtt::Thing* thing);

mtt::World* world(mtt::Thing* thing);


typedef uint64 Behaviour_ID;


typedef flecs::entity Entity;
typedef ecs_entity_t  Entity_ID;
//typedef flecs::entity Attribute;
typedef ecs_entity_t Attribute_ID;

static constexpr const Thing_ID Thing_ID_INVALID = 0;

static inline bool Thing_ID_is_valid(Thing_ID id)
{
    return id != Thing_ID_INVALID;
}

static inline bool Thing_is_valid(Thing* thing)
{
    return thing != nullptr;
}

typedef enum THING_EDGE_FLAG {
    THING_EDGE_FLAG_MOVE_TO_DST,
    THING_EDGE_FLAG_TELEPORT_TO_DST,
    THING_EDGE_FLAG_DEFAULT = THING_EDGE_FLAG_MOVE_TO_DST,
} THING_EDGE_FLAG;
struct Thing_Edge {
    mtt::Thing_ID id = mtt::Thing_ID_INVALID;
    THING_EDGE_FLAG flags = THING_EDGE_FLAG_DEFAULT;
};
static inline mtt::Thing_ID edge_id(Thing_Edge& e)
{
    return e.id;
}
static inline mtt::Thing_ID edge_id(const Thing_Edge& e)
{
    return e.id;
}

using Thing_Edge_List = std::vector<Thing_Edge>;

using Thing_To_Thing_List_Map = mtt::Map_Stable<mtt::Thing_ID, Thing_Edge_List>;

using Node_Graph_Edge_List = Thing_Edge_List;

using Thing_ID_List = std::vector<mtt::Thing_ID>;

struct Attribute {
    flecs::entity id;
    
    const mtt::String name()
    {
        return id.name().c_str();
    }
    
    operator usize() { return id.id(); }
};

struct Transform {
    vec3 translation;
    quat rotation;
    vec3 scale;
    
    mat4 matrix;
    mat4 matrix_inverse;
    
#ifndef NDEBUG
    bool began = false;
#endif
    
    Transform& do_translate(vec3 translation);
    Transform& do_translate_xy(float32 x, float32 y);
    Transform& do_rotate(float32 angle_rad, vec3 axis);
    Transform& do_rotate_z(float32 angle_rad);
    Transform& do_scale(vec3 scale);
    Transform& do_scale_uniform(float32 scale);
};

void Transform_init(Transform* xf);

vec3 translation(Transform* xf);
quat rotation(Transform* xf);
float32 rotation_z(Transform* xf);
vec3 rotation_euler(Transform* xf);
vec3 scale(Transform* xf);

void translate(Transform* xf, vec3 translation);
void translate_xy(Transform* xf, float32 x, float32 y);
void rotate(Transform* xf, float32 angle_rad, vec3 axis);
void rotate_z(Transform* xf, float32 angle_rad);
void scale(Transform* xf, vec3 scale);
void scale_uniform(Transform* xf, float32 scale);

void calc_trs_mat4(Transform* xf);
mat4 trs_mat4(Transform* xf);
mat4 trs_mat4_inverse(Transform* xf);
void Transform_init(Transform* xf);
void transform_edit_begin(Transform* xf);
void transform_edit_end(Transform* xf);

struct Label {
    mtt::String label;
};

struct Thing_Make_Args {
    Context_ID ctx_id = Context_ID_DEFAULT;
    bool should_init_fields = true;
};

using Script_Label = mtt::String;

typedef enum LOGIC_PROCEDURE_RETURN_STATUS_TYPE {
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_SCOPE,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_IMMEDIATE_SUSPEND,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_IMMEDIATE_SUSPEND_NO_RESET,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_BREAK_SUSPEND,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED_SUSPEND,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ENTER_SCOPE,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_EXIT_SCOPE,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_WAS_STOPPED,
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DEFAULT = LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED,
} LOGIC_PROCEDURE_RETURN_STATUS_TYPE;

struct Logic_Procedure_Return_Status {
    LOGIC_PROCEDURE_RETURN_STATUS_TYPE type = LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DEFAULT;
    mtt::Thing* continuation = nullptr;
    uint64 value = 0;
    
    Logic_Procedure_Return_Status() {}
    Logic_Procedure_Return_Status(bool value) {
        this->type = (value) ?
            LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DEFAULT :
            LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
    }
    
    Logic_Procedure_Return_Status(LOGIC_PROCEDURE_RETURN_STATUS_TYPE type) : type(type) {}
    
    Logic_Procedure_Return_Status(LOGIC_PROCEDURE_RETURN_STATUS_TYPE type,
                                  mtt::Thing* continuation) :
    type(type), continuation(continuation) {}
};

Script* Script_lookup(Script_ID id);

Script* Script_make(void);
Script* Script_lookup(Script_ID id);

void process_script(World* world, Script_Instance* script/*Thing_Storage& things,*/);

void Script_Instance_start_or_resume(Script_Instance* s);
void Script_Instance_suspend(Script_Instance* s);
void Script_Instance_cancel(Script_Instance* s);
void Script_Instance_should_terminate(Script_Instance* s);
void Script_Instance_terminate(Script_Instance* s);
void Script_Instance_stop(Script_Instance* s);

MTT_NODISCARD
Script_Instance* Script_Instance_from_script(Script* script, void (*post_init)(Script_Instance* s, void* data) = nullptr, void* data = nullptr);

MTT_NODISCARD
Script_Instance* Script_Instance_call_from_script(Script* script, Script_Instance* caller, void* caller_info, void (*post_init)(Script_Instance* s, void* data) = nullptr, void* data = nullptr);

void Script_Instance_destroy(Script_Instance* s);

void init_script_instance_instructions(Script_Instance* s);
void init_script_instance_instructions_range(Script_Instance* s, usize first_index, usize last_index);

void Script_print_as_things(Script* script, Script_Instance* instance = nullptr);

void init_port_entries(mtt::World* world, Eval_Connection_Graph& connections);

void reinit_port_entries(mtt::World* world, Eval_Connection_Graph& connections, struct Evaluation_Output& eval_out, usize first_index, usize last_index);
void reinit_port_entries(mtt::World* world, Eval_Connection_Graph& connections, usize first_index, usize last_index);


enum struct SCRIPT_CALLING_CONVENTION : uint64 {
    ONE_TO_ONE,
    ONE_TO_MANY,
    MANY_TO_ONE,
    MANY_TO_MANY,
};

Logic_Procedure_Return_Status jump_to_beginning(Script_Instance* s, Script_Context** s_ctx, usize eval_index, bool reset);


static const mtt::String DEFAULT_LOOKUP_SCOPE = "*";

void Thing_print(World* world, Thing_ID id);

static const mtt::String ARG_source     = "ARG_source";
static const mtt::String ARG_target     = "ARG_target";
static const mtt::String ARG_object     = "ARG_object";
static const mtt::String ARG_object_aux = "ARG_object_aux";

static const mtt::String ARG_source_offset_selector         = "ARG_source_offset_selector";
static const mtt::String ARG_target_offset_selector         = "ARG_target_offset_selector";
static const mtt::String ARG_object_offset_selector         = "ARG_object_offset_selector";
static const mtt::String ARG_object_aux_offset_selector     = "ARG_object_aux_offset_selector";

static const mtt::String ARG_spatial = "ARG_spatial";
static const mtt::String ARG_spatial_center = "center";
static const mtt::String ARG_spatial_top    = "top_with_offset";
static const mtt::String ARG_spatial_bottom = "bottom_with_offset";
static const mtt::String ARG_spatial_left   = "left_with_offset";
static const mtt::String ARG_spatial_right  = "right_with_offset";
static const mtt::String ARG_spatial_beside = "beside_with_offset";
static const mtt::String ARG_spatial_over   = "over_with_offset";

static const mtt::String ARG_directional   = "ARG_directional";
static const mtt::String ARG_dir_none   = "dir_none";
static const mtt::String ARG_dir_up     = "dir_up";
static const mtt::String ARG_dir_down   = "dir_down";
static const mtt::String ARG_dir_left   = "dir_left";
static const mtt::String ARG_dir_right  = "dir_right";
static const mtt::String ARG_dir_clockwise  = "dir_clockwise";
static const mtt::String ARG_dir_anticlockwise  = "dir_anticlockwise";
static const mtt::String ARG_dir_away  = "dir_away";
static const mtt::String ARG_dir_towards  = "dir_towards";

static const mtt::String ARG_source_offset_selector_default     = ARG_spatial_center;
static const mtt::String ARG_target_offset_selector_default     = ARG_spatial_center;
static const mtt::String ARG_object_offset_selector_default     = ARG_spatial_center;
static const mtt::String ARG_object_aux_offset_selector_default = ARG_spatial_center;

static const mtt::String ARG_any = "ARG_any";


static mtt::Map<mtt::String, mtt::String> ARG_direction_defaults = {
    {"up",         ARG_dir_up},
    {"upwards",    ARG_dir_up},
    {"upward",    ARG_dir_up},
    {"down",       ARG_dir_down},
    {"downwards",  ARG_dir_down},
    {"downward",   ARG_dir_down},
    {"left",       ARG_dir_left},
    {"leftwards",  ARG_dir_left},
    {"leftward",   ARG_dir_left},
    {"right",      ARG_dir_right},
    {"rightward",  ARG_dir_right},
    {"rightwards", ARG_dir_right},
    {"clockwise",  ARG_dir_clockwise},
    {"anticlockwise", ARG_dir_anticlockwise},
    {"counterclockwise", ARG_dir_anticlockwise},
    {"away", ARG_dir_away},
    {"towards", ARG_dir_towards},
    {"toward", ARG_dir_towards},
};
static mtt::Map<mtt::String, mtt::String> ARG_spatial_defaults = {
    {"onto",    ARG_spatial_top},
    {"on",      ARG_spatial_top},
    {"atop",    ARG_spatial_top},
    {"top",     ARG_spatial_top},
    {"up",      ARG_spatial_top},
    {"above",   ARG_spatial_top},
    {"bottom",  ARG_spatial_bottom},
    {"down",    ARG_spatial_bottom},
    {"beneath", ARG_spatial_bottom},
    {"below",   ARG_spatial_bottom},
    {"under",   ARG_spatial_bottom},
    {"underneath", ARG_spatial_bottom},
    {"left",    ARG_spatial_left},
    {"right",   ARG_spatial_right},
    {"beside",  ARG_spatial_beside},
    {"next",    ARG_spatial_beside},
    {"in",      ARG_spatial_center},
    {"into",    ARG_spatial_center},
    {"to",      ARG_spatial_center},
    {"at",      ARG_spatial_center},
    {"inside",  ARG_spatial_center},
    {"over",  ARG_spatial_over},
};

static inline bool lookup_spatial(const mtt::String& key, mtt::String* out)
{
    auto find_it = ARG_spatial_defaults.find(key);
    if (find_it == ARG_spatial_defaults.end()) {
        return false;
    }
    
    *out = find_it->second;
    return true;
}

static inline bool lookup_directional(const mtt::String& key, mtt::String* out)
{
    auto find_it = ARG_direction_defaults.find(key);
    if (find_it == ARG_direction_defaults.end()) {
        return false;
    }
    
    *out = find_it->second;
    return true;
}

struct Multipliers {
    float32 speed     = 1.0f;
    float32 height    = 1.0f;
    float32 size      = 1.0f;
    float32 intensity = 1.0f;
    float32 distance  = 0.0f;
    float32 raw_value = 0.0f;
    bool has_speed    = false,
        has_height    = false,
        has_size      = false,
        has_intensity = false,
        has_distance  = false,
        has_raw_value = false;
    
    
    
    void print()
    {
        MTT_print("%s",
                  "(Multipliers) {\n"
                  );
        if (has_speed) {
            MTT_print("    speed: %f\n", speed);
        }
        if (has_height) {
            MTT_print("    height: %f\n", height);
        }
        if (has_size) {
            MTT_print("    size: %f\n", size);
        }
        if (has_intensity) {
            MTT_print("    intensity: %f\n", intensity);
        }
        if (has_distance) {
            MTT_print("    distance: %f\n", distance);
        }
        if (has_raw_value) {
            MTT_print("    raw_value: %f\n", raw_value);
        }
        
        MTT_print("%s", "}\n");
    }
};



extern void* global_debug_args;
extern void (*global_debug_proc)(void);

using Rule_Var_Handle = int32;
constexpr const Rule_Var_Handle Rule_Var_Handle_INVALID = -1;

struct Script_Property;


bool should_be_immobile(mtt::Thing* thing);
bool input_should_cancel_animation(mtt::Thing* thing);


}


#endif /* thing_shared_types_hpp */
