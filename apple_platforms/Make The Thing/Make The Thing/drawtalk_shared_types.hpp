//
//  drawtalk_shared_types.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/2/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_shared_types_hpp
#define drawtalk_shared_types_hpp

#include "camera.hpp"
#include "job_dispatch.hpp"


#define DT_DEFINE_FLEXIBLE_IGNORE_SELECTIONS (1)

namespace dt {



struct DrawTalk;

struct Rule;

namespace rules {
struct Condition_Builder;
}

typedef uint64 ID;
typedef int64 sID;

template<typename T>
using Dynamic_Array = std::vector<T>;

struct Scene_Context {
    mtt::Set_Stable<mtt::Thing_ID> selected_things;
    mtt::Thing_ID thing_selected_with_pen;
    mtt::Thing_ID thing_most_recently_selected_with_touch;
    mtt::Thing_ID thing_selected_drawable;
    dt::Dynamic_Array<vec2> candidate_deictic_references;
    dt::Dynamic_Array<mtt::Thing_ID> references;
    
    struct Camera {
        mtt::Thing_ID cam_thing_id;
        mtt::Camera* cam_ref;
    } cam;
    
    
    dt::Dynamic_Array<dt::Rule*> rules;
    
    bool long_press_this_frame = false;
    vec2 long_press_position = {};
};

}

struct DT_Rule_Query {
    ecs_rule_t* rule_ptr = nullptr;
    mtt::String rule_string = {};
    mtt::World* world = nullptr;
    bool is_owned_memory;
    bool is_original = true;
    bool is_init = false;
    mtt::String main_var = "";
};

namespace dt {

MTT_Logger* logger(void);

namespace fmt {

struct Formatter {
    mtt::String indent;
};

extern Formatter* ctx;

static inline void set_ctx(Formatter* formatter)
{
    ctx = formatter;
}

static inline mtt::String& get_indent(void)
{
    return ctx->indent;
}

static inline void do_indent(void)
{
    get_indent() += "    ";
}

static inline void undo_indent(void)
{
    mtt::String& indent = get_indent();
    
    if (indent.empty()) {
        return;
    }
    
    for (usize i = 0; i < 4; i += 1) {
        indent.pop_back();
    }
}





//#define NO_DT_print
#if !defined(NDEBUG) && !defined(NO_DT_print)
//#define DT_print(...)



#define DT_print(...) printf("%s", fmt::get_indent().c_str()); printf( __VA_ARGS__)
#define DT_scope_open() printf("{\n"); fmt::do_indent()
#define DT_scope_close() fmt::undo_indent(); DT_print("}\n")
#define DT_scope_opennb() fmt::do_indent()
#define DT_scope_closenb() fmt::undo_indent()
#else
#define DT_print(...)
#define DT_scope_open()
#define DT_scope_close()
#define DT_scope_opennb()
#define DT_scope_closenb()
#endif

}

struct DT_Behavior_Config;
struct DT_Behavior_Catalogue;
struct Word_Dictionary_Entry;

struct Attribute_Info {
    float64 value = 1.0;
    sint32 sentiment = +1;
};

struct Word_Val_Pair {
    Word_Dictionary_Entry* word = nullptr;
    float32 value = 1.0f;
};



    
struct Sketch_Library {
    // sketches must be set to non-user-destructible
    
    mtt::Map_Stable<Word_Dictionary_Entry*, mtt::Map_Stable<mtt::Thing_ID, mtt::Set_Stable<mtt::Thing_ID>>> examples = {};
    
    
    void add(Word_Dictionary_Entry* entry, mtt::Thing* thing);
    void remove(Word_Dictionary_Entry* entry, mtt::Thing* thing);
};

DrawTalk* ctx(void);



}

#endif /* drawtalk_shared_types_hpp */
