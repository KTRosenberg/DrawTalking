//
//  drawtalk_behavior.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/18/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_behavior_hpp
#define drawtalk_behavior_hpp
//#include "drawtalk_shared_types.hpp"
//#include "thing_shared_types.hpp"
#include "thing.hpp"
//#include "tree.hpp"
#include "drawtalk_language_analysis.h"
#include "word_info.hpp"


#include "drawtalk_ui.hpp"

namespace dt {

struct Port_Connection_Descriptor {
    mtt::String source_port_name;
    mtt::String target_port_name;
    
    mtt::Thing_ID thing;
};



struct Field_Modify_Descriptor {
    mtt::Any      value;
    mtt::String   name;
    
    mtt::Thing_ID thing;
};

struct Logic_Graph_Modify {
    dt::Dynamic_Array<Port_Connection_Descriptor> ports;
    dt::Dynamic_Array<Field_Modify_Descriptor> fields;
};

struct Logic_Interface {
    // parameters to default or overridden values
    // e.g. height:default:.value
    mtt::Map<mtt::String, mtt::Map_Stable<mtt::String, Port_Connection_Descriptor>> ports;
    mtt::Map<mtt::String, mtt::Map_Stable<mtt::String, Field_Modify_Descriptor>>   fields;
};
struct Logic_Bridges {
    // roles to descriptors (what needs to be connected between causal things
    mtt::Map<mtt::String, mtt::Map_Stable<mtt::String, Logic_Graph_Modify>> bridge;
};

typedef usize Block_Type;
enum BLOCK_TYPE {
    BLOCK_TYPE_NONE,
    BLOCK_TYPE_ROOT,
    BLOCK_TYPE_SCOPE,
    BLOCK_TYPE_PROCEDURE,
    BLOCK_TYPE_PROCEDURE_PARAMETER_LIST,
    BLOCK_TYPE_PROCEDURE_PARAMETER,
    BLOCK_TYPE_GOTO,
    BLOCK_TYPE_GENERATE,
    BLOCK_TYPE_VECTOR,
    BLOCK_TYPE_POINT,
    BLOCK_TYPE_MOVE,
    BLOCK_TYPE_FOLLOW,
    BLOCK_TYPE_WHEN,
    BLOCK_TYPE_CURVE,
    BLOCK_TYPE_NAMESPACE,
    BLOCK_TYPE_OPTION,
    BLOCK_TYPE_NUMBER,
    BLOCK_TYPE_VALUE,
    BLOCK_TYPE_CONSTRUCTOR,
    BLOCK_TYPE_PROCEDURE_CALL,
    BLOCK_TYPE_PROPERTY_SET,
    BLOCK_TYPE_EXPRESSION,
    BLOCK_TYPE_CONNECTION,
    BLOCK_TYPE_JUMP,
    BLOCK_TYPE_END,
    BLOCK_TYPE_PARENT_CONNECTION,
    BLOCK_TYPE_REPRESENTATION,
    BLOCK_TYPE_PHRASE,
    
    BUILTIN_BLOCK_TYPE_COUNT,
};
static const usize BLOCK_TYPE_COUNT = BUILTIN_BLOCK_TYPE_COUNT;

const char* const BLOCK_TYPE_STRINGS[] = {
    [BLOCK_TYPE_NONE] = "BLOCK_TYPE_NONE",
    [BLOCK_TYPE_ROOT] = "BLOCK_TYPE_ROOT",
    [BLOCK_TYPE_SCOPE] = "BLOCK_TYPE_SCOPE",
    [BLOCK_TYPE_PROCEDURE] = "BLOCK_TYPE_PROCEDURE",
    [BLOCK_TYPE_PROCEDURE_PARAMETER_LIST] = "BLOCK_TYPE_PROCEDURE_PARAMETER_LIST",
    [BLOCK_TYPE_PROCEDURE_PARAMETER] = "BLOCK_TYPE_PROCEDURE_PARAMETER",
    [BLOCK_TYPE_GOTO] = "BLOCK_TYPE_GOTO",
    [BLOCK_TYPE_GENERATE] = "BLOCK_TYPE_GENERATE",
    [BLOCK_TYPE_VECTOR] = "BLOCK_TYPE_VECTOR",
    [BLOCK_TYPE_POINT] = "BLOCK_TYPE_POINT",
    [BLOCK_TYPE_MOVE] = "BLOCK_TYPE_MOVE",
    [BLOCK_TYPE_FOLLOW] = "BLOCK_TYPE_FOLLOW",
    [BLOCK_TYPE_WHEN] = "BLOCK_TYPE_WHEN",
    [BLOCK_TYPE_CURVE] = "BLOCK_TYPE_CURVE",
    [BLOCK_TYPE_NAMESPACE] = "BLOCK_TYPE_NAMESPACE",
    [BLOCK_TYPE_OPTION] = "BLOCK_TYPE_OPTION",
    [BLOCK_TYPE_NUMBER] = "BLOCK_TYPE_NUMBER",
    [BLOCK_TYPE_VALUE] = "BLOCK_TYPE_VALUE",
    [BLOCK_TYPE_CONSTRUCTOR] = "BLOCK_TYPE_CONSTRUCTOR",
    [BLOCK_TYPE_PROCEDURE_CALL] = "BLOCK_TYPE_PROCEDURE_CALL",
    [BLOCK_TYPE_PROPERTY_SET] = "BLOCK_TYPE_PROPERTY_SET",
    [BLOCK_TYPE_EXPRESSION] = "BLOCK_TYPE_EXPRESSION",
    [BLOCK_TYPE_CONNECTION] = "BLOCK_TYPE_CONNECTION",
    [BLOCK_TYPE_JUMP] = "BLOCK_TYPE_JUMP",
    [BLOCK_TYPE_END] = "BLOCK_TYPE_END",
    [BLOCK_TYPE_PARENT_CONNECTION] = "BLOCK_TYPE_PARENT_CONNECTION",
    [BLOCK_TYPE_REPRESENTATION] = "BLOCK_TYPE_REPRESENTATION",
    [BLOCK_TYPE_PHRASE] = "BLOCK_TYPE_PHRASE",
};

static const mtt::Map<mtt::String, BLOCK_TYPE> name_to_block_type = {
    {"BLOCK_TYPE_NONE", BLOCK_TYPE_NONE},
    {"BLOCK_TYPE_ROOT", BLOCK_TYPE_ROOT},
    {"BLOCK_TYPE_SCOPE", BLOCK_TYPE_SCOPE},
    {"BLOCK_TYPE_PROCEDURE", BLOCK_TYPE_PROCEDURE},
    {"BLOCK_TYPE_PROCEDURE_PARAMETER_LIST", BLOCK_TYPE_PROCEDURE_PARAMETER_LIST},
    {"BLOCK_TYPE_PROCEDURE_PARAMETER", BLOCK_TYPE_PROCEDURE_PARAMETER},
    {"BLOCK_TYPE_GOTO", BLOCK_TYPE_GOTO},
    {"BLOCK_TYPE_GENERATE", BLOCK_TYPE_GENERATE},
    {"BLOCK_TYPE_VECTOR", BLOCK_TYPE_VECTOR},
    {"BLOCK_TYPE_POINT", BLOCK_TYPE_POINT},
    {"BLOCK_TYPE_MOVE", BLOCK_TYPE_MOVE},
    {"BLOCK_TYPE_FOLLOW", BLOCK_TYPE_FOLLOW},
    {"BLOCK_TYPE_WHEN", BLOCK_TYPE_WHEN},
    {"BLOCK_TYPE_CURVE", BLOCK_TYPE_CURVE},
    {"BLOCK_TYPE_NAMESPACE", BLOCK_TYPE_NAMESPACE},
    {"BLOCK_TYPE_OPTION", BLOCK_TYPE_OPTION},
    {"BLOCK_TYPE_NUMBER", BLOCK_TYPE_NUMBER},
    {"BLOCK_TYPE_VALUE", BLOCK_TYPE_VALUE},
    {"BLOCK_TYPE_CONSTRUCTOR", BLOCK_TYPE_CONSTRUCTOR},
    {"BLOCK_TYPE_PROCEDURE_CALL", BLOCK_TYPE_PROCEDURE_CALL},
    {"BLOCK_TYPE_PROPERTY_SET", BLOCK_TYPE_PROPERTY_SET},
    {"BLOCK_TYPE_EXPRESSION", BLOCK_TYPE_EXPRESSION},
    {"BLOCK_TYPE_CONNECTION", BLOCK_TYPE_CONNECTION},
    {"BLOCK_TYPE_JUMP", BLOCK_TYPE_JUMP},
    {"BLOCK_TYPE_END", BLOCK_TYPE_END},
    {"BLOCK_TYPE_PARENT_CONNECTION", BLOCK_TYPE_PARENT_CONNECTION},
    {"BLOCK_TYPE_REPRESENTATION", BLOCK_TYPE_REPRESENTATION},
    {"BLOCK_TYPE_PHRASE", BLOCK_TYPE_PHRASE},
};

MTT_Tree_Node* mk_searchable(MTT_Tree_Node* node);

typedef void Derived;
struct alignas(16) Behavior_Block {
    
    mtt::String    label;
    usize          sequence_id;
    usize          reference_id;
    
    static usize next_available_id() { static usize _id = 0; _id += 1; return _id; };
    
    uint64 flags;
    
    bool is_valid;
    
    usize collider_id;
    
    BLOCK_TYPE type;
    
    Derived* derived;
    
    mtt::Map<mtt::String, MTT_Tree_Node*> label_to_node;
    
    MTT_Tree_Node* node;
    

    
    
    Behavior_Block* begin_children()
    {
        if (MTT_Tree_child_list_begin(this->node) == nullptr) {
            return nullptr;
        }
        
        return static_cast<Behavior_Block*>(MTT_Tree_child_list_begin(this->node)->data);
    }
    
    static void advance(Behavior_Block** block)
    {
        MTT_Tree_Node* node = (*block)->node->next;
        if (node != nullptr) {
            *block = static_cast<Behavior_Block*>(node->data);
        } else {
            *block = nullptr;
        }
    }
    
    Behavior_Block* end_children()
    {
        return nullptr;
    }
    
    template <typename T> T* get_concrete()
    {
        const Block_Type type = T::block_type();
        ASSERT_MSG(type == this->type, "Incorrect block type\n!");
        return static_cast<T*>(this->derived);
    }
    
    template <typename T> inline T* get_derived()
    {
        return this->get_concrete<T>();
    }
};


///////////

struct Behavior_Block_Type_Info_Record {
    Block_Type  type;
    mtt::String name;
    
    usize  size;
    usize  alignment;
    mem::Pool_Allocation allocation;
};

struct Behavior_Block_Type_Info {
    Dynamic_Array<Behavior_Block_Type_Info_Record> list;
};


struct DrawTalk;

void Behavior_Block_Type_Info_init(DrawTalk* dt, Behavior_Block_Type_Info* block_type_info);


Behavior_Block*
Behavior_Block_make(DrawTalk* ctx);
Behavior_Block* Block_get(MTT_Tree_Node* node);

template <typename T> T* Block_get_derived(dt::Behavior_Block* block)
{
    const Block_Type type = T::block_type();
    ASSERT_MSG(type == block->type, "Incorrect block type\n!");
    return static_cast<T*>(block->derived);
}

template <typename T> T* Block_get_concrete(MTT_Tree_Node* root)
{
    Behavior_Block* block = Block_get(root);
    const Block_Type type = T::block_type();
    ASSERT_MSG(type == block->type, "Incorrect block type\n!");
    return static_cast<T*>(block->derived);
}


void Behavior_Block_destroy(DrawTalk* ctx, dt::Behavior_Block* block);

struct Behavior_Catalogue  {
    Behavior_Block_Type_Info type_info;
};

void Behavior_Catalogue_init(DrawTalk* dt, Behavior_Catalogue* bc);

template <typename T>
static Behavior_Block* Block_make(DrawTalk* dt, dt::Behavior_Catalogue* bc)
{
    const Block_Type type = T::block_type();
    
    auto* type_info = &bc->type_info.list[type];
    
    auto* bb = mem::alloc_init<T>(&type_info->allocation.allocator);
    
    if (bb == nullptr) {
        return nullptr;
    }
    
    bb->base.type    = type;
    bb->base.derived = bb;
    
    MTT_print("Created block [%s]\n", type_info->name.c_str());
    
    return &bb->base;
    
    
}



void set_active_behavior_catalogue(dt::Behavior_Catalogue* bc);
dt::Behavior_Catalogue* get_active_behavior_catalogue(void);

void Block_print(dt::Behavior_Block* block);
void Block_print(void* block);


template <typename T>
static Behavior_Block* Block_make(const mtt::String& label)
{
    dt::Behavior_Catalogue* bc = get_active_behavior_catalogue();
    
    const Block_Type type = T::block_type();
    
    auto* type_info = &bc->type_info.list[type];
    
    auto* bb = mem::alloc_init<T>(&type_info->allocation.allocator);
    
    if (bb == nullptr) {
        return nullptr;
    }
    
    bb->base.type    = (BLOCK_TYPE)type;
    bb->base.derived = bb;
    bb->base.label = label;
    bb->base.reference_id = Behavior_Block::next_available_id();
    
    MTT_print("Created block [%s]\n", type_info->name.c_str());
    
    return &bb->base;
}

Behavior_Block* Block_make(DrawTalk* dt, dt::Behavior_Catalogue* bc, usize type);

template <typename T, bool print=true>
void Block_destroy(DrawTalk* dt, dt::Behavior_Catalogue* bc, T* block)
{
    const Block_Type type = T::block_type();
    
    auto* type_info = &bc->type_info.list[type];
    
    mem::deallocate<T>(&type_info->allocation.allocator, block);
    
    if constexpr (print) {
        MTT_print("destroying type=[%s]\n", type_info->name.c_str());
    }
}

void Block_destroy(DrawTalk* dt, dt::Behavior_Catalogue* bc, usize type, Behavior_Block* block_base);

void Block_destroy_cases(DrawTalk* dt, Behavior_Block* block_base);

//////////





//template <typename T>
//static constexpr Block_Type block_type(void)
//{
//    return BLOCK_TYPE_INVALID;
//}

//#define DEFINE_BLOCK_TYPE_OF(type__, block_type_id__) \
//static template <> Block_Type block_type< type__ >(void);
//
//#define DEFINE_BLOCK_TYPE_OF(type__, block_type_id__) \
//static template <> Block_Type block_type< type__ >(void) { return block_type_id__; }

#define define_get_block_by_generic_type(type_id__) \
static constexpr Block_Type block_type(void) { return type_id__; }

struct alignas(16) None_Block {
    Behavior_Block base;
    using Self_Type = None_Block;
    
    define_get_block_by_generic_type(BLOCK_TYPE_NONE)
};
struct alignas(16) Root_Block {
    Behavior_Block base;
    using Self_Type = Root_Block;
    
    static MTT_Tree_Node* node(const mtt::String& label, const Self_Type& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<Self_Type>(label);
        
        block->label = label;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    
    define_get_block_by_generic_type(BLOCK_TYPE_ROOT)
    
    mtt::Procedure proc;
    
    void set_proc(mtt::Procedure proc)
    {
        this->proc = proc;
    }
};
//DEFINE_BLOCK_TYPE_OF(Root_Block, BLOCK_TYPE_ROOT)

struct alignas(16) Scope_Block {
    Behavior_Block base;
    
    static MTT_Tree_Node* node(const mtt::String& label, const Scope_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Scope_Block>(label);
        
        block->label = label;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    
    define_get_block_by_generic_type(BLOCK_TYPE_SCOPE)
};
//DEFINE_BLOCK_TYPE_OF(Scope_Block, BLOCK_TYPE_SCOPE)

struct alignas(16) Generator_Block {
    Behavior_Block base;
    
    mtt::Thing_ID thing_id;
    
    define_get_block_by_generic_type(BLOCK_TYPE_GENERATE)
};
//DEFINE_BLOCK_TYPE_OF(Generator_Block, BLOCK_TYPE_GENERATE)

struct alignas(16) Vector_Block {
    Behavior_Block base;
    
    uint64 mode; // := constant | reference
    union {
        vec4 point;
        mtt::Connection connection;
    };
    
    mtt::Thing_ID thing_id;

    define_get_block_by_generic_type(BLOCK_TYPE_VECTOR)
};


struct alignas(16) Point_Block {
    Behavior_Block base;
    
    uint64 mode; // := constant | reference
    union {
        vec4 point;
        mtt::Connection connection;
    };
    
    mtt::Thing_ID thing_id;
    
    define_get_block_by_generic_type(BLOCK_TYPE_POINT)
};


typedef enum TEMPLATE_PARAM_TYPE {
    TEMPLATE_PARAM_TYPE_SOURCE,
    TEMPLATE_PARAM_TYPE_SOURCE_LIST,
    TEMPLATE_PARAM_TYPE_OBJECT,
    TEMPLATE_PARAM_TYPE_OBJECT_LIST,
    TEMPLATE_PARAM_TYPE_SPATIAL_CONSTRAINT_DISTANCE,
    TEMPLATE_PARAM_TYPE_SPATIAL_CONSTRAINT_CEILING,
    
    TEMPLATE_PARAM_TYPE_COUNT
} TEMPLATE_PARAM_TYPE;

static const char* const template_param_type_str[] = {
    [TEMPLATE_PARAM_TYPE_SOURCE] = "TEMPLATE_PARAM_TYPE_SOURCE",
    [TEMPLATE_PARAM_TYPE_SOURCE_LIST] = "TEMPLATE_PARAM_TYPE_SOURCE_LIST",
    [TEMPLATE_PARAM_TYPE_OBJECT] = "TEMPLATE_PARAM_TYPE_OBJECT",
    [TEMPLATE_PARAM_TYPE_OBJECT_LIST] = "TEMPLATE_PARAM_TYPE_OBJECT_LIST",
    [TEMPLATE_PARAM_TYPE_SPATIAL_CONSTRAINT_DISTANCE] = "TEMPLATE_PARAM_TYPE_SPATIAL_CONSTRAINT_DISTANCE",
    [TEMPLATE_PARAM_TYPE_SPATIAL_CONSTRAINT_CEILING] = "TEMPLATE_PARAM_TYPE_SPATIAL_CONSTRAINT_CEILING",
};

typedef enum PARAM_REQUIRED {
    PARAM_REQUIRED_NO  = 0,
    PARAM_REQUIRED_YES = 1,
} PARAM_REQUIRED;

static const char* const param_required_strings[] = {
    [PARAM_REQUIRED_NO]  = "PARAM_REQUIRED_NO",
    [PARAM_REQUIRED_YES] = "PARAM_REQUIRED_YES",
};


struct alignas(16) Procedure_Parameter_List_Block;

struct alignas(16) Procedure_Block {
    Behavior_Block base;
    
    mtt::Thing_ID thing_id;
    
    // TODO: // decide, differentiate between calling block procedures in the tree and calling native registered procedures and possibly operation code?
    Dynamic_Array<mtt::String> identifier;
    
    static MTT_Tree_Node* node(const mtt::String& label, const Procedure_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Procedure_Block>(label);
        
        block->label = label;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    
    define_get_block_by_generic_type(BLOCK_TYPE_PROCEDURE)
    
    Procedure_Parameter_List_Block* get_param_list_block()
    {
        MTT_Tree_Node* param_list = this->base.node->child_list.head;
        
        return static_cast<Procedure_Parameter_List_Block*>(
                static_cast<Behavior_Block*>(param_list->data)->derived);
    }
    
    Behavior_Block* begin_body()
    {
        MTT_Tree_Node* next = MTT_Tree_child_list_begin(this->base.node)->next;
        if (next == nullptr) {
            return nullptr;
        }
        Behavior_Block* out = static_cast<Behavior_Block*>(next->data);
        

        return out;
    }
    
    void advance_body(Behavior_Block** body)
    {
        MTT_Tree_Node* next = (*body)->node->next;
        if (next != nullptr) {
            *body = static_cast<Behavior_Block*>(next->data);
        } else {
            *body = nullptr;
        }
    }
    
    Behavior_Block* end_body()
    {
        return nullptr;
    }
    
//    Behavior_Block* get_body()
//    {
//        MTT_Tree_Node* rest = this->base.node->child_list.head->next;
//        if (rest == NULL) {
//            return NULL;
//        }
//
//        return static_cast<Behavior_Block*>(rest->data);
//    }
    
};

typedef enum PARAM_COMBINE_MODE {
    PARAM_COMBINE_MODE_NO,
    PARAM_COMBINE_MODE_YES,

} PARAM_COMBINE_MODE;

typedef enum WIDGET {
    WIDGET_NONE,
    WIDGET_POSITIONAL_OFFSET,
    WIDGET_ABSOLUTE_POSITION,
    
} WIDGET;

struct alignas(16) Procedure_Parameter_Block {
    Behavior_Block base;
    
    mtt::String type_name;
    dt::PARAM_REQUIRED requirement;
    dt::PARAM_COMBINE_MODE combine_mode;
    
    static MTT_Tree_Node* node(const mtt::String& label, const Procedure_Parameter_Block& params, std::initializer_list<MTT_Tree_Node*> children)
    {
        dt::Behavior_Block* block = Block_make<dt::Procedure_Parameter_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::Procedure_Parameter_Block*>(block->derived);
        
        actual->type_name    = params.type_name;
        actual->requirement  = params.requirement;
        actual->combine_mode = params.combine_mode;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    static MTT_Tree_Node* node(const mtt::String& label, const Procedure_Parameter_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Procedure_Parameter_Block>(label);
        
        block->label = label;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    
    define_get_block_by_generic_type(BLOCK_TYPE_PROCEDURE_PARAMETER)
};

struct alignas(16) Procedure_Parameter_List_Block {
    Behavior_Block base;
    
    static MTT_Tree_Node* node(const mtt::String& label, const Procedure_Parameter_List_Block& params, std::initializer_list<MTT_Tree_Node*> children)
    {
        dt::Behavior_Block* block = Block_make<dt::Procedure_Parameter_List_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::Procedure_Parameter_List_Block*>(block->derived);
        (void)actual;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    static MTT_Tree_Node* node(const mtt::String& label, const Procedure_Parameter_List_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Procedure_Parameter_List_Block>(label);
        
        block->label = label;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    
    define_get_block_by_generic_type(BLOCK_TYPE_PROCEDURE_PARAMETER_LIST)
    
    Procedure_Parameter_Block* begin()
    {
        return static_cast<Procedure_Parameter_Block*>(static_cast<Behavior_Block*>(MTT_Tree_child_list_begin(this->base.node)->data)->derived);
    }
    
    void advance(Procedure_Parameter_Block** param)
    {
        MTT_Tree_Node* next = (*param)->base.node->next;
        if (next != nullptr) {
            *param = static_cast<Procedure_Parameter_Block*>(static_cast<Behavior_Block*>(next->data)->derived);
        } else {
            *param = nullptr;
        }
    }
    
    Procedure_Parameter_Block* end()
    {
        return nullptr;
    }
//    MTT_Tree_Node* it;
//    Procedure_Parameter_Block* begin()
//    {
//        this->it = MTT_Tree_child_list_begin(this->base.node);
//
//        return (Procedure_Parameter_Block*)it->data;
//    }
//
//    Procedure_Parameter_Block* curr_param()
//    {
//        return (Procedure_Parameter_Block*)it->data;
//    }
//
//    void advance()
//    {
//        MTT_Tree_child_list_advance(&this->it);
//    }
//
//    Procedure_Parameter_Block* end()
//    {
//        return nullptr;
//    }
//    for (auto* it = MTT_Tree_child_list_begin(this->base.node);
//         it != MTT_Tree_child_list_end(this->base.node); MTT_Tree_child_list_advance(&it)) {
//    }
    
    
    
    
};



struct alignas(16) Goto_Block {
    Behavior_Block base;
    
    mtt::String label;
    
    static MTT_Tree_Node* node(const mtt::String& label, const Goto_Block& params, std::initializer_list<MTT_Tree_Node*> children)
    {
        dt::Behavior_Block* block = Block_make<dt::Procedure_Parameter_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::Goto_Block*>(block->derived);
        
        actual->label    = params.label;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    static MTT_Tree_Node* node(const mtt::String& label, const Goto_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Goto_Block>(label);
        
        block->label = label;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    
    define_get_block_by_generic_type(BLOCK_TYPE_GOTO)
};

struct alignas(16) Mover_Block {
    Behavior_Block base;
    
    mtt::Thing_ID thing_id;
    
    define_get_block_by_generic_type(BLOCK_TYPE_MOVE)
};

struct alignas(16) Follower_Block {
    Behavior_Block base;
    
    mtt::Thing_ID thing_id;
    
    define_get_block_by_generic_type(BLOCK_TYPE_FOLLOW)
};


struct alignas(16) When_Block {
    Behavior_Block base;
    
    
    define_get_block_by_generic_type(BLOCK_TYPE_WHEN)
};


struct alignas(16) Curve_Block {
    Behavior_Block base;
    
    
    define_get_block_by_generic_type(BLOCK_TYPE_CURVE)
};

struct alignas(16) Namespace_Block {
    Behavior_Block base;
    
    // TODO: use to disambiguate which scope to visit, based on arguments (overloading)
    mtt::String key;
    
    
    static MTT_Tree_Node* node(const mtt::String& label, const Namespace_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Namespace_Block>(label);
        
        block->label = label;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    
    define_get_block_by_generic_type(BLOCK_TYPE_NAMESPACE)
};

struct alignas(16) Option_Block {
    Behavior_Block base;
    
    
    define_get_block_by_generic_type(BLOCK_TYPE_OPTION)
};

struct alignas(16) Number_Block {
    Behavior_Block base;
    
    
    define_get_block_by_generic_type(BLOCK_TYPE_NUMBER)
};


enum VALUE_BLOCK_KIND {
    VALUE_BLOCK_KIND_THING_REF      = (1ll <<  0),
    VALUE_BLOCK_KIND_CONSTANT       = (1ll <<  1),
    VALUE_BLOCK_KIND_THING_FIELD    = (1ll <<  2),
    VALUE_BLOCK_KIND_THING_IN_PORT =  (1ll <<  3),
    VALUE_BLOCK_KIND_THING_SELECTOR = (1ll <<  4),
    VALUE_BLOCK_KIND_EXPRESSION     = (1ll <<  5),
    VALUE_BLOCK_KIND_LABEL          = (1ll <<  6),
    VALUE_BLOCK_KIND_PARAMETER      = (1ll <<  7),
    VALUE_BLOCK_KIND_MULTIPLE       = (1ll <<  8),
    VALUE_BLOCK_KIND_TYPE_NAME      = (1ll <<  9),
    VALUE_BLOCK_KIND_IDENTIFIER     = (1ll << 10),
    VALUE_BLOCK_KIND_NUMBER         = (1ll << 11),
};


struct alignas(16) Value_Block {
    Behavior_Block base;
    
    VALUE_BLOCK_KIND kind;
    mtt::Any default_value;
    bool is_list;
    
    mtt::String type_name;
    std::vector<std::vector<mtt::String>> identifiers;
    std::vector<mtt::String>              selectors;
    
    dt::PARAM_REQUIRED requirement;
    dt::WIDGET widget;
    bool use_widget;
    
    
    define_get_block_by_generic_type(BLOCK_TYPE_VALUE)
    
    

    static MTT_Tree_Node* node(const mtt::String& label, const Value_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Value_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::Value_Block*>(block->derived);
        
        actual->kind          = (VALUE_BLOCK_KIND)params.kind;
        actual->default_value = params.default_value;
        actual->requirement   = params.requirement;
        actual->identifiers   = params.identifiers;
        actual->type_name     = params.type_name;
        actual->is_list       = params.is_list;
        actual->selectors     = params.selectors;
        actual->widget        = params.widget;
        actual->use_widget    = params.use_widget;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
};

struct alignas(16) Constructor_Block {
    Behavior_Block base;
    
    mtt::Thing_Archetype_ID arch_id;
    bool is_visible;
    bool is_user_destructible;
    bool is_locked;
    
    define_get_block_by_generic_type(BLOCK_TYPE_CONSTRUCTOR)
    
    static MTT_Tree_Node* node(const mtt::String& label, const Constructor_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Constructor_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::Constructor_Block*>(block->derived);
        
        actual->arch_id              = params.arch_id;
        actual->is_visible           = params.is_visible;
        actual->is_user_destructible = params.is_user_destructible;
        actual->is_locked            = params.is_locked;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
};

struct alignas(16) Procedure_Call_Block {
    Behavior_Block base;
    
    // TODO: // decide, differentiate between calling block procedures in the tree and calling native registered procedures and possibly operation code?
    Dynamic_Array<mtt::String> identifiers;
    
    define_get_block_by_generic_type(BLOCK_TYPE_PROCEDURE_CALL)
};

struct alignas(16) Property_Set_Block {
    Behavior_Block base;
    
    
    define_get_block_by_generic_type(BLOCK_TYPE_PROPERTY_SET)
};

struct alignas(16) Expression_Block {
    Behavior_Block base;
    
    
    define_get_block_by_generic_type(BLOCK_TYPE_EXPRESSION)
};

struct alignas(16) Connection_Block {
    Behavior_Block base;
    
    mtt::Connection_Template connection;
    
    define_get_block_by_generic_type(BLOCK_TYPE_CONNECTION)
    
    
    static MTT_Tree_Node* node(const mtt::String& label, const Connection_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Connection_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::Connection_Block*>(block->derived);
        
        actual->connection = params.connection;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
};

struct alignas(16) Parent_Connection_Block {
    Behavior_Block base;
    
    mtt::String child_label;
    mtt::String parent_label;
    
    define_get_block_by_generic_type(BLOCK_TYPE_PARENT_CONNECTION)
    
    static MTT_Tree_Node* node(const mtt::String& label, const Parent_Connection_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Parent_Connection_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::Parent_Connection_Block*>(block->derived);
        actual->child_label = params.child_label;
        actual->parent_label = params.parent_label;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
};

struct alignas(16) Jump_Block {
    Behavior_Block base;
    
    std::vector<std::vector<mtt::String>> sources;
    std::vector<std::vector<mtt::String>> targets;
    std::vector<std::vector<mtt::String>> height;
    std::vector<std::vector<mtt::String>> ceiling;
    float32 default_height;
    float32 default_ceiling;
    
    
    static MTT_Tree_Node* node(const mtt::String& label, const Jump_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Jump_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::Jump_Block*>(block->derived);
        
        actual->sources         = params.sources;
        actual->targets         = params.targets;
        actual->height          = params.height;
        actual->ceiling         = params.ceiling;
        actual->default_height  = params.default_height;
        actual->default_ceiling = params.default_ceiling;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    
  
    define_get_block_by_generic_type(BLOCK_TYPE_JUMP)
};

enum END_KIND {
    END_KIND_OUT_PORT,
    END_KIND_FIELD,
    END_KIND_CONDITION
};

struct alignas(16) End_Block {
    Behavior_Block base;
    
    END_KIND kind;
    std::vector<std::vector<mtt::String>> sources;
    mtt::Any expected_value;
    
    static MTT_Tree_Node* node(const mtt::String& label, const End_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::End_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::End_Block*>(block->derived);
        
        actual->kind           = params.kind;
        actual->sources        = params.sources;
        actual->expected_value = params.expected_value;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
    
    define_get_block_by_generic_type(BLOCK_TYPE_END)
};

struct alignas(16) Representation_Block {
    Behavior_Block base;
    
    mtt::String kind;
    std::vector<std::vector<mtt::String>> sources;
    std::vector<std::vector<mtt::String>> instructions;
    std::vector<std::vector<mtt::String>> paths;
    std::vector<mtt::Representation> reps;
    
    static MTT_Tree_Node* node(const mtt::String& label, const Representation_Block& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<dt::Representation_Block>(label);
        
        block->label = label;
        
        auto* actual = static_cast<dt::Representation_Block*>(block->derived);
        
        actual->kind           = params.kind;
        actual->sources        = params.sources;
        actual->instructions   = params.instructions;
        actual->paths          = params.paths;
        actual->reps           = params.reps;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
        
    define_get_block_by_generic_type(BLOCK_TYPE_REPRESENTATION)
};

struct alignas(16) Phrase_Block {
    Behavior_Block base;
    
    using Self_Type = Phrase_Block;
    
    mtt::String kind;
    std::vector<std::vector<mtt::String>> instructions;
    std::vector<mtt::String> requirements;
    
    static MTT_Tree_Node* node(const mtt::String& label, const Self_Type& params, const std::vector<MTT_Tree_Node*>& children)
    {
        dt::Behavior_Block* block = Block_make<Self_Type>(label);
        
        block->label = label;
        
        auto* actual = static_cast<Self_Type*>(block->derived);
        
        actual->instructions   = params.instructions;
        actual->requirements   = params.requirements;
        
        return mtt::tree_node_mk((void*)block, mk_searchable, children);
    }
        
    define_get_block_by_generic_type(BLOCK_TYPE_PHRASE)
};


template <typename T, typename Initialization_Proc>
static T* Block_init(const mtt::String& label, const T& params, Initialization_Proc init)
{
    dt::Behavior_Block* block = Block_make<T>(label);
    
    block->label = label;
    
    auto* actual = static_cast<T*>(block->derived);
    
    init(actual);
    
    return actual;
}



#define INIT_BLOCK_TYPE_INFO(enum__, type__) \
.type = enum__, .name = #enum__ , .size = sizeof(type__), .alignment = alignof(type__), .allocation = {}


static std::vector<Behavior_Block_Type_Info_Record> builtin_block_type_info = {
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_NONE, None_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_ROOT, Root_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_SCOPE, Scope_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_PROCEDURE, Procedure_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_PROCEDURE_PARAMETER_LIST, Procedure_Parameter_List_Block) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_PROCEDURE_PARAMETER, Procedure_Parameter_Block) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_GOTO, Goto_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_GENERATE, Generator_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_VECTOR, Vector_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_POINT, Point_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_MOVE, Mover_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_FOLLOW, Follower_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_WHEN, When_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_CURVE, Curve_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_NAMESPACE, Namespace_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_OPTION, Option_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_NUMBER, Number_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_VALUE, Value_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_CONSTRUCTOR, Constructor_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_PROCEDURE_CALL, Procedure_Call_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_PROPERTY_SET, Property_Set_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_EXPRESSION, Expression_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_CONNECTION, Connection_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_JUMP, Jump_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_END, End_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_PARENT_CONNECTION, Parent_Connection_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_REPRESENTATION, Representation_Block ) },
    { INIT_BLOCK_TYPE_INFO(BLOCK_TYPE_PHRASE, Phrase_Block ) },
};







struct Evaluation_Procedure {
    u8 temp;
};

struct Program;


void register_argument(void);



struct Evaluation_Instruction {
    unsigned int temp;
};
// per-entity evaluation state
struct Evaluation_State_Trait {
    mtt::String action_label;
};


struct Eval_End_Handle {
    uint64 primary_id;
    uint64 secondary_id;
    mtt::World* world;
    mem::Allocator* dealloc;
};

struct Evaluation_Context;
typedef uint64 Eval_ID;


struct Eval_Handle {
    Eval_ID id;
    Evaluation_Context* ctx;
};

struct Identifiers {
    mtt::Map_Stable<mtt::String, mtt::Any> map;
    mtt::Thing_ID id;
};

struct Program_Sequence_Node {
    MTT_Tree_Node* node;
    
    mtt::Thing_ID id;
    mtt::String label;
    Identifiers identifiers;
    
    // number of child programs finished
    usize unfinished_child_count;
    
    inline bool is_container() const
    {
        return (this->id == mtt::Thing_ID_INVALID);
    }
    
    inline void set_as_container()
    {
        this->id = mtt::Thing_ID_INVALID;
    }
    
    inline void set_id(mtt::Thing_ID id)
    {
        this->id = id;
    }
    
    void init();
    
    void deinit();
    
    static Program_Sequence_Node* get(MTT_Tree_Node* node);
    
    inline Program_Sequence_Node* parent()
    {
        return this->get(this->node->parent);
    }
    inline Program_Sequence_Node* next()
    {
        return this->get(this->node->next);
    }
    inline Program_Sequence_Node* prev()
    {
        return this->get(this->node->prev);
    }
    inline Program_Sequence_Node* begin_children()
    {
        return this->get(MTT_Tree_child_list_begin(this->node));
    }
    inline Program_Sequence_Node* end_children()
    {
        return this->get(MTT_Tree_child_list_end(this->node));
    }
    
    inline void advance(Program_Sequence_Node** it)
    {
        *it = this->get((*it)->node->next);
    }
    inline void retreat(Program_Sequence_Node** it)
    {
        *it = this->get((*it)->node->prev);
    }
};


struct Evaluation_Context;

struct Program_Sequence {
    MTT_Tree tree;
    mtt::String label;
    
    mtt::Thing_ID id;
    
    usize next_program_id;
    
    Evaluation_Context* eval_ctx;
    
    usize gen_program_id()
    {
        this->next_program_id += 1;
        return this->next_program_id;
    }
    
    Dynamic_Array<Program_Sequence_Node*> active_programs;
    
    Program_Sequence() : label(""), id(mtt::Thing_ID_INVALID), next_program_id(0), eval_ctx(nullptr) {}
    
    void run_begin();
    void run_end();
    
    void init();
    void deinit();
    
    bool empty() const
    {
        return (this->tree.root == nullptr);
    }
    
    Program_Sequence_Node* append_new_program()
    {
        MTT_Tree* the_tree = &this->tree;
        
        Program_Sequence_Node* seq = Program_Sequence_Node::get(MTT_Tree_append_child_node(the_tree, MTT_Tree_root(the_tree), MTT_Tree_Node_make(the_tree, nullptr)));
        
        return seq;
    }
};

enum I_TYPE {
    SET_PORT_CONNECTION,
    SET_SELECTOR,
    SET_FIELD,
    SET_OPTION,
};

//struct Stack_Record {
//    Dynamic_Array<Program_Sequence*> tops;
//
//};
//struct Program_Stack {
//    Dynamic_Array<Stack_Record> stack;
//
//    void new_record()
//    {
//        stack.push_back(Stack_Record());
//    }
//};
//struct Call__ {
//    struct Param {
//        mtt::String identifier;
//        mtt::String type_name;
//
//        uint64 info;
//        Dynamic_Array<mtt::Any> param;
//    };
//    mtt::Map_Stable<mtt::String, Param> params;
//    mtt::String label;
//
//    Evaluation_Context* eval_state;
//
//    Word_Info* info;
//
//    bool treat_source_params_as_one;
//    bool treat_object_params_as_one;
//};

struct Deferred_Instruction {
    mtt::Procedure proc;
};
struct Evaluation_Context {
    static usize eval_id() { static usize count = 0; count += 1; return count; }
    
    
    Program_Sequence programs;
    //Program_Stack program_stack;
    
//    mtt::Set_Stable<mtt::Thing_ID> active_targets;
        
    usize id;
    bool is_active;
    
    //dt::Dynamic_Array<dt::Call__> calls;
    
    dt::Behavior_Block* it_body;
    struct Stack_Record {
        dt::Behavior_Block* block;
    };
        
    Dynamic_Array<Deferred_Instruction> instructions;
    Dynamic_Array<dt::Behavior_Block*>  stack;
    
    void init()
    {
        this->programs = Program_Sequence();
        this->programs.init();
        this->programs.eval_ctx = this;
        bool is_active = false;
    }
    
    void set_id(usize id)
    {
        this->id = id;
    }
    
    void run_begin();
    void run_end();
    
    void activate();
    void deactivate();
};




struct System_Evaluation_Context {
    dt::Speech_Property* references;
    
    // root group to context
    mtt::Map_Stable<mtt::Thing_ID, Evaluation_Context> eval_contexts;
    Dynamic_Array<Evaluation_Context*> eval_history;
    
    dt::Dynamic_Array<Speech_Property*> fragments;
    
    //mtt::Map_Stable<mtt::Thing_ID, mtt::Set_Stable<mtt::Thing_ID>> thing_to_active_eval;
    
    mtt::Map_Stable<mtt::Thing_ID, Dynamic_Array<Speech_Token*>> thing_to_tokens;
    Dynamic_Array<Speech_Token*> dep_token_stack;
    
    Evaluation_Context* active_ctx;
    
    dt::Dynamic_Array<Word_Info*> verb_list;
    dt::Dynamic_Array<Word_Info*> noun_list;
    dt::Dynamic_Array<Word_Info*> rule_list;
    
    dt::Dynamic_Array<mtt::Thing_ID> card_list;
    
    
    
    
    void set_active_context(Evaluation_Context* ctx)
    {
        this->active_ctx = ctx;
    }

    void start_new_eval()
    {
        thing_to_tokens.clear();
        verb_list.clear();
        noun_list.clear();
        rule_list.clear();
        fragments.clear();
    
    }
    
    void eval_history_append(Evaluation_Context* eval_ctx);
    

    bool get_evaluation_context(usize eval_id, Evaluation_Context** ctx)
    {
        auto result = mtt::map_try_get<mtt::Thing_ID, Evaluation_Context>(&this->eval_contexts, eval_id);
        
        if (result.status == false) {
            return false;
        }
        
        *ctx = result.value;
        
        return true;
    }
    
    void create_evaluation_context(Evaluation_Context** out)
    {
        usize id = Evaluation_Context::eval_id();

        mtt::map_set(&this->eval_contexts, id, Evaluation_Context(), out);
        
        (*out)->set_id(id);
    }

};


struct Rule_Test {
    ecs_world_t* world;
    mtt::String rule_str;
    mtt::String msg;
    ecs_rule_t* rule;
    bool is_valid;
    
    
    
    Rule_Test(ecs_world_t* world, mtt::String rule_str, mtt::String msg) :
    world(world), rule_str(rule_str), msg(msg) {
        this->is_valid = false;
    }
    
    void init(void)
    {
        cstring str = this->rule_str.c_str();
        ecs_rule_t* rule = mtt_ecs_rule_new(this->world, str);
        if (rule == NULL) {
            this->is_valid = false;
            this->rule = NULL;
            return;
        }
        this->rule = rule;
        this->is_valid = true;
    }
    
    void deinit(void)
    {
        if (this->rule != NULL) {
            mtt_ecs_rule_free(this->rule);
        }
        this->rule = NULL;
        this->is_valid = false;
    }
};


// trigger-responses + relations




struct Data_Flow {
    DT_Rule_Query q;
    
    struct Port {
        mtt::MTT_TYPE type = mtt::MTT_NONE;
        mtt::String port_name = {};
    };
    struct Field {
        mtt::MTT_TYPE type = mtt::MTT_NONE;
        
        mtt::String port_name = {};
    };
    struct Selector {
        mtt::MTT_TYPE type = mtt::MTT_NONE;
        
        mtt::String name = {};
    };
    
    bool is_field    = false;
    bool is_port     = false;
    bool is_selector = false;
    
    Port w_port = {};
    Field w_field = {};
    Selector w_selector = {};
};

struct DT_Rule_Query_Builder {
    
    using Variable = mtt::String;
    
    mtt::Set<Variable> variables;
    
    struct Term {
        std::vector<Variable> variables;
        mtt::String relation;
        
        Term() {}
        Term(std::vector<Variable>& variables, mtt::String& relation) : variables(variables), relation(relation) {}
        
        mtt::String to_string(void)
        {
            if (this->variables.empty()) {
                return "";
            }
            
            mtt::String term_string = this->relation + "(";
            usize i = 0;
            for (; i < this->variables.size() - 1; i += 1) {
                term_string += this->variables[i] + ",";
            }
            term_string += this->variables[i] + ")";
            
            return term_string;
        }
    };
    
    std::vector<Term> terms;
    usize next_instance_id = 0;
    
    void push_variable(mtt::String name)
    {
        is_changed = true;
        variables.insert(Variable(name));
    }
    
    void push_term(std::vector<Variable> variables, mtt::String relation)
    {
        is_changed = true;
        terms.push_back(Term(variables, relation));
    }
    
    bool is_changed = false;
    
    mtt::String saved_query_string = "";
    mtt::String to_string(void)
    {
        if (this->terms.empty()) {
            return "";
        }
        if (!this->is_changed) {
            return this->saved_query_string;
        }
        
        mtt::String query_string = "";
        usize i = 0;
        for (; i < this->terms.size() - 1; i += 1) {
            query_string += this->terms[i].to_string() + ",";
        }
        query_string += this->terms[i].to_string() + ")";
        
        this->saved_query_string = query_string;
        this->is_changed = false;
        
        return query_string;
    }
};

DT_Rule_Query* DT_Rule_Clause_make(mtt::World* world, mtt::String& rule_string);




DT_Rule_Query* DT_Rule_Query_make(mtt::World* world, mtt::String& rule_string, DT_Rule_Query* rc);


DT_Rule_Query* DT_Rule_Clause_make_from_builder(mtt::World* world, DT_Rule_Query_Builder& builder);


DT_Rule_Query* DT_Rule_Clause_make_from_builder(mtt::World* world, DT_Rule_Query_Builder& builder, DT_Rule_Query* rc);


void DT_Rule_Clause_destroy(DT_Rule_Query* clause);
inline static void DT_Rule_Query_destroy(DT_Rule_Query* clause)
{
    DT_Rule_Clause_destroy(clause);
}


void init_rules(std::vector<Rule_Test>& rule_list);

void print_rule_results(mtt::World* world, std::vector<Rule_Test>& rule_list);
void print_rule_results_no_free(mtt::World* world, std::vector<Rule_Test>& rule_list);
void print_rule_results_raw(mtt::World* world, ecs_rule_t* rule);

struct Rule;
struct Trigger;
struct Response;
struct Speech_Property;



struct Speech_Instructions {
    uint64 instruction_idx;
    std::vector<Speech_Property*> instruction_list;
    static mtt::String indent;
    
    
    static usize next_sequence_id;
    static void init(void)
    {
        Speech_Instructions::next_sequence_id = 0;
    }
};




struct Speech_Instruction_Evaluation {
    std::deque<Speech_Instructions*> instruction_history;
    
    static Speech_Instructions* current_instruction;
    static mem::Allocator* allocator(void);
    
    static Speech_Instructions* current(void)
    {
        return Speech_Instruction_Evaluation::current_instruction;
    }
    
  
};




namespace rules {


struct Rule_Variable {
    using TYPE = Rule_Variable;
    mtt::Rule_Var_Handle handle;

    mtt::String name;
    
    MTT_NODISCARD static Rule_Variable* make();
    static void destroy(Rule_Variable* ref);
    
    Rule_Variable(void) {}
};



enum struct EXPIRATION {
    NEVER          = 0,
    ONCE           = (1 << 0),
    VARIABLE_VALUE = (1 << 1)
};


struct Trigger {
    using TYPE = Trigger;
    static constexpr cstring TYPENAME = "Trigger";
    
    
    
    enum struct MODE {
        QUERY = 0,
        CONDITION_EQUAL         = (1 << 0),
        CONDITION_LESS_EQUAL    = (1 << 1),
        CONDITION_LESS          = (1 << 2),
        CONDITION_GREATER       = (1 << 3),
        CONDITION_GREATER_EQUAL = (1 << 4),
    };
    
    
    
    

    EXPIRATION expiration = EXPIRATION::NEVER;
    
    enum struct KIND {
        ONCE,
        CONTINUOUS,
    };
    
    struct Clause {
        using type = Trigger::Clause;
        
        usize idx = 0;
        
        Trigger::KIND kind = Trigger::KIND::ONCE;
        MODE mode = MODE::QUERY;
        
        ecs_rule_t* rule_query = nullptr;
        mtt::String rule_string = "";
        
        // some clauses might involve checking or setting values on Things
        // e.g. the numerical output of a "battery" entity is hooked into something
        mtt::Thing_ID value_source = mtt::Thing_ID_INVALID;
        mtt::String   value_get = "";
        mtt::Any      value_to_match = {};
        mtt::Any      value_to_compare = {};
        
        mtt::Thing_ID target = mtt::Thing_ID_INVALID;
        mtt::Thing_ID target_get = mtt::Thing_ID_INVALID;
        //
        
        bool treat_as_negation = false;
        
        
        //dt::Dynamic_Array<Rule_Variable*> vars;
        // the variables are named
        
        mtt::Map_Stable<mtt::Rule_Var_Handle, mtt::String> query_var_to_var_name;
        
        // mapping the key (e.g. subject, object, etc.) to a list of matched entities
        //mtt::Map_Stable<mtt::String, dt::Dynamic_Array<Rule_Variable*>> role_to_var_list;
        
        // query library's iterator
        ecs_iter_t result_iterator;
        

        
        MTT_NODISCARD static Trigger::Clause* make();
        static void destroy(Trigger::Clause* ref);
        
        struct Compare_Routine_increasing_by_mode {
            bool operator()(const Trigger::Clause& a, const Trigger::Clause& b) const {
                return (static_cast<uint64>(a.mode) <= static_cast<uint64>(b.mode));
            }
            
            bool operator()(Trigger::Clause& a, const Trigger::Clause& b) const {
                return (static_cast<uint64>(a.mode) <= static_cast<uint64>(b.mode));
            }
        };
    };
    
    mtt::Map_Stable<mtt::String, Rule_Variable*> name_to_ref;
    
    std::vector<Trigger::Clause*> clause_list;
    
    MTT_NODISCARD static Trigger* make();
    static void destroy(Trigger* ref);
    
    Trigger(void) {}
    
};

struct Response {
    using TYPE = Response;
    static constexpr cstring TYPENAME = "Response";
    
    EXPIRATION expiration = EXPIRATION::NEVER;
    
    enum struct MODE {
        QUERY = 0,
        CONDITION_EQUAL         = (1 << 0),
        CONDITION_LESS_EQUAL    = (1 << 1),
        CONDITION_LESS          = (1 << 2),
        CONDITION_GREATER       = (1 << 3),
        CONDITION_GREATER_EQUAL = (1 << 4),
    };
    
    enum struct KIND {
        EVENT_START,
        EVENT_STOP,
        FOLLOW_UP_TRIGGER,
    };
    
    struct Clause {
        using type = Response::Clause;
        
        Response::MODE mode = Response::MODE::QUERY;
        
        Response::KIND kind;
//            MODE mode;
        
        
        //dt::Dynamic_Array<Trigger*> triggers_to_enable;
        
        mtt::Thing_ID value_source = mtt::Thing_ID_INVALID;
        mtt::String   value_name = "";
        mtt::Any      value_to_compare = {}; 
        my::Function<void(void*)> function = [](void*){};
        
        
        dt::Dynamic_Array<mtt::String> action_key;
        
        mtt::Map_Stable<mtt::Rule_Var_Handle, mtt::String> query_var_to_var_name;

        
        ecs_iter_t result_iterator;
        
        
        MTT_NODISCARD static Response::Clause* make();
        static void destroy(Response::Clause* ref);
        
        struct Compare_Routine_increasing_by_mode {
            bool operator()(const Trigger::Clause& a, const Trigger::Clause& b) const {
                return (static_cast<uint64>(a.mode) <= static_cast<uint64>(b.mode));
            }
            
            bool operator()(Trigger::Clause& a, const Trigger::Clause& b) const {
                return (static_cast<uint64>(a.mode) <= static_cast<uint64>(b.mode));
            }
        };
    };
    
    mtt::Map_Stable<mtt::String, Rule_Variable*> name_to_ref;
    
    dt::Dynamic_Array<Response::Clause*> clause_list;
    
    Response(void) {}
    
    
    

    MTT_NODISCARD static Response* make();
    static void destroy(Response* ref);
    
};


typedef uint64 Rule_ID;
static Rule_ID Rule_ID_INVALID = 0;

struct Rule {
    using TYPE = Rule;
    static constexpr cstring TYPENAME = "Rule";
    
    mtt::String label = "";
    
    static Rule_ID next_id;
    Rule_ID id;
    Trigger trigger;
    Response response;
    
    static mtt::Map_Stable<Rule_ID, Rule*> rules;
    
    MTT_NODISCARD static Rule* make();
    
    static void init(Rule* rule);
    
    static void destroy(Rule* ref);
    static void destroy(Rule_ID id);
    static Rule* lookup(Rule_ID id);
    
    mtt::Map_Stable<mtt::String, Rule_Variable*>                 name_to_variable;
    //mtt::Map_Stable<mtt::String, dt::Dynamic_Array<mtt::String>> variable_to_roles;
    //mtt::Map_Stable<mtt::String, dt::Dynamic_Array<mtt::String>> role_to_variables;
    
    
    Rule(void) {}
};

}

struct alignas(16) Speech_Command {
    void* user_data;
};
struct alignas(16) Speech_Commands {
    std::deque<Speech_Command> cmds;
};

struct Instruction;

struct alignas(16) Speech_Property {
    using ID_TYPE = uint64;
    using Tok  = dt::Speech_Token;
    using Word = dt::Word_Info;
    
    typedef std::vector<Speech_Property*> Prop_List;
    typedef Prop_List::iterator Prop_List_iterator;
    
//    Speech_Property* referenced_by = nullptr;
//    bool is_reference = false;
    Speech_Property* refers_to = nullptr;
#ifndef NDEBUG
    ID_TYPE copy_src_ID = 0;
#endif
    Speech_Property* coref_parent = nullptr;
    struct Active_Prop_List_Iterator {
        Prop_List_iterator it;
        Prop_List_iterator end;
        
        Active_Prop_List_Iterator(Prop_List_iterator begin, Prop_List_iterator end) :
        it(begin), end(end) { while (!is_valid()) { ++it; } }
        
        bool is_valid() {
            return (it == end) || ((*it != 0) && !(*it)->should_ignore());
        }
        
        Prop_List_iterator::value_type operator*() const { return *it; }
        Active_Prop_List_Iterator& operator++() { do ++it; while (!is_valid()); return *this; }
        bool operator == (const Active_Prop_List_Iterator& other) const { return it == other.it; }
        bool operator != (const Active_Prop_List_Iterator& other) const { return it != other.it; }
    };
    
    struct Active_Prop_List {
        Prop_List& container;
        Active_Prop_List(Prop_List& container) : container(container) {}
        Active_Prop_List_Iterator begin() {
            return Active_Prop_List_Iterator(container.begin(), container.end());
        }
        Active_Prop_List_Iterator end() {
            return Active_Prop_List_Iterator(container.end(), container.end());
        }
        bool is_valid()
        {
            return !this->container.empty();
        }
        
        Speech_Property* first()
        {
            auto it = this->begin();
            if (it == this->end()) {
                return nullptr;
            }
            return (*it);
        }
    };
    
    
    bool was_preprocessed = false;
    
    bool uses_deixis = false;
    
    bool ignore_selections = false;
    
    ID_TYPE ID = 0;
    mtt::Map_Stable<mtt::String, Prop_List> properties;
    
    VERB_EVENT action_event = VERB_EVENT_BEGIN;
    
    decltype(auto) begin()
    {
        return properties.begin();
    }
    decltype(auto) end()
    {
        return properties.end();
    }
    decltype(auto) cbegin()
    {
        return properties.cbegin();
    }
    decltype(auto) cend()
    {
        return properties.cend();
    }
    
    Thing_Update* candidate_selections = nullptr;
    
    bool should_ignore()
    {
        return parent_ref_disabled;
    }
    
    bool parent_ref_disabled = false;
    
    inline Speech_Property* get_active_parent(void)
    {
        return (!this->parent_ref_disabled) ? this->parent : nullptr;
    }
    
    inline Speech_Property* get_parent(void)
    {
        return this->parent;
    }
    
    // e.g. for instructions
    Instruction* instruction = nullptr;
    
    
    DT_Rule_Query search_rule = {};
    dt::DT_Rule_Query_Builder search_rule_builder = dt::DT_Rule_Query_Builder();
    
    bool treat_as_unique = false;
    bool treat_as_different_next_in_sequence = false;
   
    struct Call_Desc_Ref {
        mtt::Thing_ID call_id = mtt::Thing_ID_INVALID;
        usize param_list_index = 0;
        usize param_list_entry_index = 0;
        
        inline bool call_desc(mtt::Call_Descriptor** out)
        {
            mtt::Thing_ID id = this->call_id;
            if (id != mtt::Thing_ID_INVALID) {
                mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), id);
                if (thing == nullptr) {
                    return false;
                }
                mtt::Call_Descriptor* call = mtt::access<mtt::Call_Descriptor>(thing, "state");
                *out = call;
                return true;
            }
            return false;
        }
        
        inline bool is_valid(void)
        {
            mtt::Thing_ID id = this->call_id;
            if (id != mtt::Thing_ID_INVALID) {
                mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), id);
                if (thing == nullptr) {
                    return false;
                }
                return true;
            }
            return false;
        }
    } call_desc_ref = {};
    
    void move_prop(const mtt::String& key_from, const mtt::String& key_to, Speech_Property* dst_prop)
    {
        auto it_find = this->properties.find(key_from);
        if (it_find == this->properties.end()) {
            return;
        }
        
        Prop_List to_move = it_find->second;
        
//        auto it_find_to = dst_prop->properties.find(key_to);
//        if (it_find_to == dst_prop->properties.end()) {
//
//            dst_prop->push_prop_list(key_to, to_move);
//
//
//            this->properties.erase(it_find);
//            return;
//        }
        
        dst_prop->push_prop_list(key_to, to_move);
        
        this->properties.erase(it_find);
        
    }
    
    void rename_prop(const mtt::String& key_from, const mtt::String& key_to)
    {
        auto it_find = this->properties.find(key_from);
        if (it_find == this->properties.end()) {
            return;
        }
        
        Prop_List to_move = it_find->second;
        
        auto it_find_to = this->properties.find(key_to);
        if (it_find_to == this->properties.end()) {
            this->properties[key_to] = to_move;
            this->properties.erase(it_find);
            return;
        }
        
        it_find_to->second.insert(it_find_to->second.begin(), to_move.begin(), to_move.end());
        this->properties.erase(it_find);
        
    }
    
    Speech_Property* push_prop(const mtt::String& key, Speech_Property* ins)
    {
        auto& props = this->properties[key];
        if (props.empty()) {
            ins->idx = props.size();
            props.push_back(ins);
            
        } else {
            bool exists = false;
            FOR_ITER(p_it, props, ++p_it) {
                if ((*p_it) == ins) {
                    exists = true;
                    ASSERT_MSG(exists, "Property %s should not exist", ins->label.c_str());
                    break;
                }
            }
            if (!exists) {
                ins->idx = props.size();
                props.push_back(ins);
            }
        }
        ins->key = key;
        ins->parent = this;
        return ins;
    }
    
    void push_prop_list(const mtt::String& key, Prop_List& prop_list)
    {
        for (auto it = prop_list.begin(); it != prop_list.end(); ++it) {
            this->push_prop(key, *it);
        }
    }
    
    Speech_Property* push_prop(const mtt::String& key) {
        auto it_found = this->properties.find(key);
        
//        if (it_found != this->properties.end()) {
//            DT_print("Prop %s already exists. Is this intentional?\n", key.c_str());
//        }
        
        auto& prop = *Speech_Property::make();
        prop.key = key;
        auto& entry = this->properties[key];
        entry.push_back(&prop);
        prop.idx = entry.size() - 1;
        
        prop.parent = this;
        
        return &prop;
    }
    
    Speech_Property* last_prop(const mtt::String& key)
    {
        auto find = this->properties.find(key);
        if (find == this->properties.end() || find->second.empty()) {
            return nullptr;
        }
        
        return find->second.back();
    }
    
    bool try_get_prop(const mtt::String& key, Prop_List** p_ptr)
    {
        auto find = this->properties.find(key);
        if (find == this->properties.end() || find->second.empty()) {
            return false;
        }
        
        *p_ptr = &find->second;
        
        return true;
    }
    
    MTT_NODISCARD Prop_List* try_get_prop_empty_ok(const mtt::String& key)
    {
        auto find = this->properties.find(key);
        if (find == this->properties.end()) {
            return nullptr;
        }
        
        return &find->second;
    }
    
    const Prop_List* try_get_prop(const mtt::String& key) const
    {
        auto find = this->properties.find(key);
        if (find == this->properties.end() || find->second.empty()) {
            return nullptr;
        }
        
        return &find->second;
    }
    
    static inline Prop_List Prop_List_INVALID = {};
    static inline Active_Prop_List Active_Prop_List_INVALID = Prop_List_INVALID;
    Prop_List& get(cstring& name)
    {
        if (Prop_List* pl = this->try_get_prop(name)) {
            return *pl;
        }
      
        return Prop_List_INVALID;
    }
    static Prop_List& get(Speech_Property* prop, cstring& name)
    {
        if (prop) {
            return prop->get(name);
        }
        return Prop_List_INVALID;
    }
    
    template <typename PROC>
    void get_then(cstring& name, PROC&& proc)
    {
        if (Prop_List* pl = this->try_get_prop(name)) {
            if (!pl->empty()) {
                proc(pl);
            }
        }
    }
    template <typename PROC>
    static Prop_List& get_then(Speech_Property* prop, cstring& name, PROC&& proc)
    {
        if (prop) {
            return prop->get_then(name, proc);
        }
    }
    
    template <typename PROC>
    void get_only_then(cstring& name, PROC&& proc)
    {
        if (Prop_List* pl = this->try_get_prop(name)) {
            if (!pl->empty()) {
                proc(pl->front());
            }
        }
    }
    template <typename PROC>
    static void get_only_then(Speech_Property* prop, cstring& name, PROC&& proc)
    {
        if (prop) {
            prop->get_only_then(name, proc);
        }
    }
    
    bool is_empty_prop_list(Prop_List& pl)
    {
        return &pl == &Speech_Property::Prop_List_INVALID;
    }
    

    Active_Prop_List get_active(cstring& name)
    {
        Prop_List& pl = get(name);
        if (is_empty_prop_list(pl)) {
            return Active_Prop_List_INVALID;
        }
        
        return Active_Prop_List(pl);
    }
    
    static Active_Prop_List get_active(Speech_Property* prop, cstring& name)
    {
        if (prop) {
            return prop->get_active(prop, name);
        }
        return Active_Prop_List_INVALID;
    }
    
    template <typename PROC>
    void for_each_w_inactive(cstring& name, PROC&& proc)
    {
        for (usize i = 0; auto& p : get(name)) {
            proc(p, i);
            i += 1;
        }
    }
    template <typename PROC>
    static void for_each_w_inactive(Speech_Property* prop, cstring& name, PROC&& proc)
    {
        if (prop) {
            prop->for_each_w_inactive(name, proc);
        }
    }
    
    template <typename PROC, typename PROC_ELSE>
    void for_each_else_w_inactive(cstring& name, PROC&& proc, PROC_ELSE&& proc_else)
    {
        auto& entries = get(name);
        if (!is_empty_prop_list(entries)) {
            for (usize i = 0; auto& p : entries) {
                proc(p, i);
                i += 1;
            }
        } else {
            proc_else();
        }
    }
    template <typename PROC, typename PROC_ELSE>
    static void for_each_else_w_inactive(Speech_Property* prop, cstring& name, PROC&& proc, PROC_ELSE&& proc_else)
    {
        if (prop) {
            prop->for_each_else_w_inactive(proc, proc_else);
        }
    }
    
    template <typename PROC, typename PROC_ELSE>
    void for_each_else(cstring& name, PROC&& proc, PROC_ELSE&& proc_else)
    {
        auto& entries = get(name);
        if (!is_empty_prop_list(entries)) {
            for (usize i = 0; auto& p : entries) {
                if (p->should_ignore()) {
                    continue;
                }
                
                proc(p, i);
                i += 1;
            }
        } else {
            proc_else();
        }
    }
    template <typename PROC, typename PROC_ELSE>
    static void for_each_else(Speech_Property* prop, cstring& name, PROC&& proc, PROC_ELSE&& proc_else)
    {
        if (prop) {
            prop->for_each_else(name, proc, proc_else);
        }
    }
    
    template <typename PROC>
    void for_each(cstring& name, PROC&& proc)
    {
        for (usize i = 0; auto& p : get(name)) {
            if (p->should_ignore()) {
                continue;
            }
            
            proc(p, i);
            i += 1;
        }
    }
    template <typename PROC>
    static void for_each(Speech_Property* prop, cstring& name, PROC&& proc)
    {
        if (prop) {
            prop->for_each(name, proc);
        }
    }

    
    bool try_get_only_prop(const mtt::String& key, Speech_Property** p_ptr)
    {
        std::vector<Speech_Property*>* prop_list = nullptr;
        if (this->try_get_prop(key, &prop_list) && !prop_list->empty()) {
            *p_ptr = prop_list->front();
            return true;
        }
        
        return false;
    }
    
    MTT_NODISCARD
    Speech_Property* try_get_only_prop(const mtt::String& key)
    {
        std::vector<Speech_Property*>* prop_list = nullptr;
        if (this->try_get_prop(key, &prop_list) && !prop_list->empty()) {
            return prop_list->front();
        }
        return nullptr;
    }
    
    MTT_NODISCARD
    Prop_List* try_get_prop(const mtt::String& key)
    {
        Prop_List* out = nullptr;
        this->try_get_prop(key, &out);
        return out;
    }
    
    bool has_prop(const mtt::String& key)
    {
        return this->properties.find(key) != this->properties.end();
    }
    
    bool remove_prop(const mtt::String& key)
    {
        auto find = this->properties.find(key);
        if (find == this->properties.end() || find->second.empty()) {
            return false;
        }
        for (auto p_it = find->second.begin(); p_it != find->second.end(); ++p_it) {
            (*p_it)->parent = nullptr;
            //Speech_Property::destroy_recursive((*p_it));
        }
        this->properties.erase(find);
        
        return true;
    }
    
    bool remove_prop(Speech_Property* prop)
    {
        auto find = this->properties.find(prop->key);
        if (find == this->properties.end() || find->second.empty()) {
            return false;
        }
        for (auto p_it = find->second.begin(); p_it != find->second.end(); ++p_it) {
            if ((*p_it) == prop) {
                find->second.erase(p_it);
                //Speech_Property::destroy_recursive((*p_it));
                return true;
            }
        }
        
        return false;
    }
    
    bool remove_prop_no_parent_clear(const mtt::String& key)
    {
        auto find = this->properties.find(key);
        if (find == this->properties.end() || find->second.empty()) {
            return false;
        }
        this->properties.erase(find);
        
        return true;
    }
    
    

    inline static mtt::Map_Stable<Speech_Property*, Speech_Property*> original_to_copy = {};
    inline static mtt::Map_Stable<Speech_Property*, Speech_Property*> copy_to_original = {};
    inline static std::vector<Speech_Property*> copied_refs_to_resolve = {};
    inline static std::vector<Speech_Property*> referenced_by_to_resolve = {};
    Speech_Property* copy_internal(void)
    {
        Speech_Property* prop_copy = Speech_Property::make();
        usize ID = prop_copy->ID;
        *prop_copy = *this;
        prop_copy->search_rule.is_original = false;
        prop_copy->ID = ID;
        prop_copy->properties.clear();
#ifndef NDEBUG
        prop_copy->copy_src_ID = this->ID;
#endif
        
        original_to_copy.insert({this, prop_copy});
        copy_to_original.insert({prop_copy, this});
        if (this->value.kind_string == "REFERENCE") {
            copied_refs_to_resolve.push_back(prop_copy);
        }
//        if (this->referenced_by != nullptr) {
//            referenced_by_to_resolve.push_back(this);
//        }
        
        for (auto it = this->properties.begin(); it != this->properties.end(); ++it) {
            mtt::String key = it->first;
            
            Prop_List& list = it->second;
            for (usize i = 0; i < list.size(); i += 1) {
                Speech_Property* sub_copy = list[i]->copy_internal();
                prop_copy->push_prop(key, sub_copy);
            }
        }
        
        return prop_copy;
    }
    
    Speech_Property* copy_bla_internal(void)
    {
        Speech_Property* prop_copy = Speech_Property::make();
        usize ID = prop_copy->ID;
        *prop_copy = *this;
        prop_copy->search_rule.is_original = false;
        prop_copy->ID = ID;
        prop_copy->properties.clear();
#ifndef NDEBUG
        prop_copy->copy_src_ID = this->ID;
#endif
        
        for (auto it = this->properties.begin(); it != this->properties.end(); ++it) {
            mtt::String key = it->first;
            
            Prop_List& list = it->second;
            for (usize i = 0; i < list.size(); i += 1) {
                Speech_Property* sub_copy = list[i]->copy_bla_internal();
                prop_copy->push_prop(key, sub_copy);
            }
        }
        
        return prop_copy;
    }
    
    Speech_Property* copy(void)
    {
        this->original_to_copy.clear();
        this->copy_to_original.clear();
        this->copied_refs_to_resolve.clear();
        this->referenced_by_to_resolve.clear();
        
        Speech_Property* new_root = this->copy_internal();
        
        for (Speech_Property* ref_holder : this->copied_refs_to_resolve) {
            Speech_Property* src_ref = (Speech_Property*)ref_holder->value.reference;
            auto find_it = this->original_to_copy.find(src_ref);
            if (find_it != this->original_to_copy.end()) {
                Speech_Property* dst_ref = find_it->second;
                ref_holder->value.reference = (uintptr)dst_ref;
            }
        }
//        for (Speech_Property* ref_by_tgt : this->referenced_by_to_resolve) {
//            auto find_it = this->original_to_copy.find(ref_by_tgt->referenced_by);
//            if (find_it != this->original_to_copy.end()) {
//                Speech_Property* ref_by_new = find_it->second;
//                ref_by_tgt->referenced_by = ref_by_new;
//            } else {
//                ASSERT_MSG(false, "should be impossible");
//            }
//        }
        
        
        return new_root;
    }
    
    
    Speech_Property* copy_and_resolve_refs_internal(void)
    {
        Speech_Property* prop_copy = Speech_Property::make();
        usize ID = prop_copy->ID;
        *prop_copy = *this;
        prop_copy->search_rule.is_original = false;
        prop_copy->ID = ID;
        prop_copy->properties.clear();
#ifndef NDEBUG
        prop_copy->copy_src_ID = this->ID;
#endif
        
        original_to_copy.insert({this, prop_copy});
        copy_to_original.insert({prop_copy, this});
        if (this->value.kind_string == "REFERENCE") {
            copied_refs_to_resolve.push_back(prop_copy);
        }
//        if (this->referenced_by != nullptr) {
//            referenced_by_to_resolve.push_back(this);
//        }
        
        for (auto it = this->properties.begin(); it != this->properties.end(); ++it) {
            mtt::String key = it->first;
            
            Prop_List& list = it->second;
            for (usize i = 0; i < list.size(); i += 1) {
                Speech_Property* sub_copy = list[i]->copy_and_resolve_refs_internal();
                prop_copy->push_prop(key, sub_copy);
            }
        }
        
        return prop_copy;
    }
    
    using Ref_Map = mtt::Map<Speech_Property*, mtt::Set<Speech_Property*>>;
    using Ref_Map_By_ID = mtt::Map<Speech_Property::ID_TYPE, Speech_Property::ID_TYPE>;
    using Ref_Map_Reverse_By_ID = mtt::Map<Speech_Property::ID_TYPE, mtt::Set<Speech_Property::ID_TYPE>>;
    struct Prop_With_Ref_Map {
        Speech_Property* root;
        Ref_Map prop_to_refs = {};
        
        Ref_Map_By_ID refer_to_map = {};
        Ref_Map_Reverse_By_ID referred_by_map = {};
        
        inline void PRINT_REF_MAP()
        {
            MTT_print("%s\n", "refer to{");
            for (auto& [key, val] : refer_to_map) {
                MTT_print("\t%llu refers to %llu\n", key, val);
            }
            MTT_print("%s\n", "}");
        }
    };
    
    
    
    
    
    Prop_With_Ref_Map copy_and_resolve_refs(void)
    {
        original_to_copy.clear();
        copied_refs_to_resolve.clear();
        referenced_by_to_resolve.clear();
        
        Speech_Property* new_root = this->copy_and_resolve_refs_internal();
        
        for (Speech_Property* ref_holder : this->copied_refs_to_resolve) {
            Speech_Property* src_ref = (Speech_Property*)ref_holder->value.reference;
            auto find_it = this->original_to_copy.find(src_ref);
            if (find_it != this->original_to_copy.end()) {
                Speech_Property* dst_ref = find_it->second;
                ref_holder->value.reference = (uintptr)dst_ref;
            }
        }
        
        Prop_With_Ref_Map prop_w_refs = {};
        prop_w_refs.root = new_root;
        
//        for (Speech_Property* ref_by_tgt : this->referenced_by_to_resolve) {
//            auto find_it = this->original_to_copy.find(ref_by_tgt->referenced_by);
//            if (find_it != this->original_to_copy.end()) {
//                Speech_Property* ref_by_new = find_it->second;
//                ref_by_tgt->referenced_by = ref_by_new;
//            } else {
//                ASSERT_MSG(false, "should be impossible");
//            }
//        }
        //prop_w_refs.root->print();

        mtt::Set_Stable<Speech_Property*> coref_holders = {};
        mtt::Set_Stable<Speech_Property*> coref_parents = {};
        new_root->traverse_preorder([&](Speech_Property* p) {
            if (p->has_prop("COREFERENCE") && !p->uses_deixis) {
                coref_parents.insert(p->parent);
                coref_holders.insert(p);
                return false;
            }
            
            return true;
        });
        
        for (Speech_Property* p : coref_holders) {
            Speech_Property* parent = p->parent; // e.g. agent
            usize idx_in_parent = p->idx;
            const mtt::String& key_in_parent = p->key;
            Prop_List* p_props = &parent->properties[key_in_parent];
            
            Prop_List plist = {};

            if (parent->label == "equal" || parent->label == "exceed") {
                continue;
            }

            for (usize i = 0; i < p_props->size(); i += 1) {
                Speech_Property* role_prop = (*p_props)[i];
                
                Prop_List* cor_list = nullptr;
                if (role_prop->try_get_prop("COREFERENCE", &cor_list)) {
                    for (usize _ = 0; _ < cor_list->size(); _ +=1 ) {
                        auto REF = (*cor_list)[_]->value.reference;
                        //auto REF_ID = ((Speech_Property*)REF)->ID;
                        //auto REF_ID_PARENT = ((Speech_Property*)REF)->get_parent()->ID;
                        auto* referenced_prop_copy = ((Speech_Property*)REF)->copy_bla_internal();
                        auto COPY_REF = (uintptr)referenced_prop_copy;
                        //auto COPY_REF_ID = referenced_prop_copy->ID;
                        //auto COPY_REF_ID_PARENT = ((Speech_Property*)COPY_REF)->get_parent()->ID;
                        //((Speech_Property*)(*cor_list)[_]->value.reference)->referenced_by = referenced_prop_copy;
                        prop_w_refs.prop_to_refs[((Speech_Property*)(*cor_list)[_]->value.reference)].insert(referenced_prop_copy);
                        referenced_prop_copy->coref_parent = parent;
                        //prop_w_refs.refer_to_map[((Speech_Property*)(*cor_list)[_])->ID] = (((Speech_Property*)(*cor_list)[_]->value.reference)->ID);
                        prop_w_refs.refer_to_map[((Speech_Property*)(*cor_list)[_])->ID] = (((Speech_Property*)(*cor_list)[_]->value.reference)->ID);
                        prop_w_refs.referred_by_map[(((Speech_Property*)(*cor_list)[_]->value.reference))->ID].insert(((Speech_Property*)(*cor_list)[_])->ID);
                        

                        referenced_prop_copy->refers_to =  ((Speech_Property*)(*cor_list)[_]->value.reference);
                        plist.push_back(referenced_prop_copy);
                    }
                } else {
                    plist.push_back(role_prop);
                }
            }
            for (usize i = 0; i < plist.size(); i += 1) {
                plist[i]->idx = i;
            }
            *p_props = plist;
        }

//        for (Speech_Property* p : coref_parents) {
//            Speech_Property::destroy_recursive(p);
//        }
        
        return prop_w_refs;
    }
    
    
    enum struct TYPE_FLAG  {
        UNKNOWN    = 0,
        NOUN       = (1 << 0),
        VERB       = (1 << 1),
        RULE       = (1 << 2),
        SEQUENCE   = (1 << 3),
        SELECTION  = (1 << 4),
        NUMBER     = (1 << 5),
        VALUE      = (1 << 6),
        TRIGGER    = (1 << 7),
        ATTRIBUTE  = (1 << 8),
        PRONOUN    = (1 << 9),
    };
    uint64 type_flag = (uint64)TYPE_FLAG::UNKNOWN;
    
    
    uint64 flags = (uint64)0x0;
    
    uint64 mode_flags = 0;
    
    mtt::String label     = "";    // textual label
    mtt::String sub_label = "";    // textual label
    mtt::String type_str  = ""; // type of node
    mtt::String kind_str  = ""; // modifier based on the type
    mtt::String key       = "";      // lookup
    mtt::String tag_str   = "";  // system tagging e.g. noun
    
    
    
    
    
    mtt::String sequence_type_str; // relates to sequencing of events
    
    mtt::String annotation = "";
    
    usize idx = 0;
    
    Speech_Property* parent = nullptr;
    

    Tok* token = nullptr;
    
    uint64 sequence_number = 0;
    
    Rule* rule = nullptr;

    
    enum struct EVAL_FLAG {
        READY = 0,
        MUST_FILL_UNKNOWN = (1 << 0),
        CAN_IGNORE_UNKNOWN = (1 << 1)
    };
    
    uint64 eval_flags = (uint64)EVAL_FLAG::READY;
    
    static inline ID_TYPE next_avail_id = 0;
    static Speech_Property* make(void)
    {
        next_avail_id += 1;
        auto* prop = mem::alloc_init<Speech_Property>(Speech_Instruction_Evaluation::allocator());
        prop->ID = next_avail_id;

        return prop;
    }
    static void destroy(Speech_Property* property)
    {
        if (property == nullptr) {
            return;
        }
        
        DT_Rule_Clause_destroy(&property->search_rule);
        mem::deallocate<Speech_Property>(Speech_Instruction_Evaluation::allocator(), property);
    }
    static void destroy_recursive(Speech_Property* property)
    {
        if (property == nullptr) {
            return;
        }
        
        DT_Rule_Clause_destroy(&property->search_rule);
        
        while (!property->properties.empty()) {
            auto& p_list = property->properties.begin()->second;
            
            while (!p_list.empty()) {
                Speech_Property::destroy_recursive(p_list.back());
                p_list.pop_back();
            }
            property->properties.erase(property->properties.begin());
        }

        //if (!instruction->is_reference) {
        destroy(property);
        //}
    }
    
    static Speech_Property* make_clone(Speech_Property* property)
    {
        Speech_Property* cloned = Speech_Property::make();
        *cloned = *property;
        cloned->search_rule.is_original = false;
        
        return cloned;
    }
    

    template <typename PROC>
    void traverse_preorder(PROC&& proc)
    {
        if (!proc(this)) {
            return;
        }
        {
            if (this->value.kind_string != "NONE") {
                {
                    if (this->value.kind_string == "REFERENCE") {
                        Speech_Property* ref = (Speech_Property*)this->value.reference;
                        if (ref != nullptr) {
                            ref->traverse_preorder(proc);
                        }
                    } else {
                        //ASSERT_MSG(false, "Speech_Instruction::Value should always have a type flag!\n");
                    }
                }
            }
            
            for (auto it = this->properties.begin(); it != this->properties.end(); ++it) {
                auto& prop_list = it->second;
                for (auto p_it = prop_list.begin(); p_it != prop_list.end(); ++p_it) {
                    dt::Speech_Property* prop = *p_it;
                    prop->traverse_preorder(proc);
                }
            }
        }
    }

    

    
    static const mtt::Map<mtt::String, TYPE_FLAG> STRING_TO_TYPE_FLAG;
    static mtt::Map<uint64, mtt::String> TYPE_FLAG_TO_STRING;

    
    enum struct CONTEXT_FLAG {
        
    } context_flag = (CONTEXT_FLAG)0x0;
    
    enum struct ATTRIBUTE_FLAG {
        UNKNOWN = 0,
        COLOR,
        SPEED,
        MAGNITUDE_OR_VALUE,
        DIRECTION,
        TIMING,
        QUANTITY,
        RANDOMNESS,
        PROPERTY_OF,
        END_CONDITION,
        SINGULAR_OR_PLURAL,
        FIRST_UNRESERVED_ = (ATTRIBUTE_FLAG::END_CONDITION + 1)
    };
    static const mtt::Map<mtt::String, ATTRIBUTE_FLAG> STRING_TO_ATTRIBUTE_FLAG;
    static mtt::Map<uint64, mtt::String> ATTRIBUTE_FLAG_TO_STRING;
    
    enum struct END_CONDITION {
        NONE,
        TRIGGER,
    };

    
    enum struct DEFAULT_SPEED_FLAG {
        MOTIONLESS,
        SLOW,
        REGULAR,
        FAST,
        IMMEDIATE,
        QUICK = DEFAULT_SPEED_FLAG::FAST,
    };
    static const mtt::Map<mtt::String, DEFAULT_SPEED_FLAG> STRING_TO_DEFAULT_SPEED_FLAG;
    
    enum struct DEFAULT_TIME_FREQUENCY_FLAG {
        NEVER     = 0,
        RARELY    = (1 << 0),
        REGULARLY = (1 << 1),
        OFTEN     = (1 << 2),
        ALWAYS    = (1 << 3),
        
        SOMETIMES    = DEFAULT_TIME_FREQUENCY_FLAG::RARELY,
        FREQUENTLY   = DEFAULT_TIME_FREQUENCY_FLAG::OFTEN,
        EVERY_FEW    = DEFAULT_TIME_FREQUENCY_FLAG::REGULARLY,
        OCCASIONALLY = DEFAULT_TIME_FREQUENCY_FLAG::RARELY,
    };
    static const mtt::Map<mtt::String, DEFAULT_TIME_FREQUENCY_FLAG> STRING_TO_DEFAULT_TIME_FREQUENCY_FLAG;

    
    enum struct TIME_UNIT {
        SECOND,
        MINUTE,
        HOUR,
        DAY,
        WEEK,
        MONTH,
        YEAR,
        DECADE,
        CENTURY,
    };
    
    enum struct TIME_EXPRESSION {
        DAWN,
        MORNING,
        NOON,
        MIDDAY = TIME_EXPRESSION::NOON,
        AFTERNOON,
        EVENING,
        TWILIGHT,
        DUSK = TIME_EXPRESSION::EVENING,
        NIGHT,
        NIGHTFALL = TIME_EXPRESSION::NIGHT,
        MIDNIGHT,
        O_CLOCK,
        AM,
        PM,
    };
    
    static const mtt::Map<mtt::String, TIME_UNIT> STRING_TO_TIME_UNIT;
    static const mtt::Map<mtt::String, TIME_EXPRESSION> STRING_TO_TIME_EXPRESSION;
    
    static constexpr float32 DEFAULT_SPEED_VALUES[] = {
        [(uint64)DEFAULT_SPEED_FLAG::MOTIONLESS]   = 0.0f,
        [(uint64)DEFAULT_SPEED_FLAG::SLOW]         = 1.0f,
        [(uint64)DEFAULT_SPEED_FLAG::REGULAR]      = 10.0f,
        [(uint64)DEFAULT_SPEED_FLAG::FAST]         = 100.0f,
        [(uint64)DEFAULT_SPEED_FLAG::IMMEDIATE]    = POSITIVE_INFINITY,
    };
    
    static constexpr float32 DEFAULT_TIME_FREQUENCY_VALUES[] = {
        [(uint64)DEFAULT_TIME_FREQUENCY_FLAG::NEVER]     = POSITIVE_INFINITY,
        [(uint64)DEFAULT_TIME_FREQUENCY_FLAG::RARELY]    = 100.0f,
        [(uint64)DEFAULT_TIME_FREQUENCY_FLAG::REGULARLY] = 10.0f,
        [(uint64)DEFAULT_TIME_FREQUENCY_FLAG::OFTEN]     = 1.0f,
        [(uint64)DEFAULT_TIME_FREQUENCY_FLAG::ALWAYS]    = 0.0f,
    };
    
    enum struct EXECUTION_TIME_FLAG {
        DO_IMMEDIATELY,
        WAIT_UNTIL_CURRENT_IS_DONE
    } execution_time_flag;
    
    // out-of-date
    enum  struct ROLE {
        AGENT = (1 << 0),
        PATIENT = (1 << 1),
        DIRECT_OBJECT = (1 << 2),
        // see below, maybe the indirect object should be distinct in the system
        INDIRECT_OBJECT_OR_OBJECT_OF_PREPOSITION = (1 << 3),
        
        TEMPORAL = (1 << 4),
        
        FROM_SOURCE = (1 << 5),
        TO_DESTINATION = (1 << 6),
        
        SPATIAL_RELATION = (1 << 7),
        // not sure if should be separate - maybe
        OBJECT_OF_PREPOSITION = (1 << 8)
        
    };
    static const mtt::Map<mtt::String, ROLE> STRING_TO_ROLE;
    static mtt::Map<uint64, mtt::String> ROLE_TO_STRING;

    
    enum struct SPATIAL_RELATION {
        ABOVE         = (1 << 0),
        BELOW         = (1 << 1),
        NEXT_TO_LEFT  = (1 << 2),
        NEXT_TO_RIGHT = (1 << 3),
        NEXT_TO_ANGLE = (1 << 4),
        OFF_OF        = (1 << 5),
        ATOP          = (1 << 6),
        
        AROUND       = SPATIAL_RELATION::NEXT_TO_ANGLE,
        BESIDE_LEFT  = SPATIAL_RELATION::NEXT_TO_LEFT,
        BESIDE_RIGHT = SPATIAL_RELATION::NEXT_TO_RIGHT,
        ON_TOP_OF    = SPATIAL_RELATION::ATOP,
    };
    static const mtt::Map<mtt::String, SPATIAL_RELATION> STRING_TO_SPATIAL_RELATION;
    
    enum struct VERB_FLAG {
        DEFAULT = 0,
        
        // when x acts on y, y acts on x in the same ways
        SYMMETRIC        = (1 << 0),
        
        // when the verb is invoked upon an event, not immedately,
        // e.g. x can deflect b,
        IS_ABILITY_OR_DEFERRED_EVENT = (1 << 1),
        
        // blocks ability
        PROHIBITED = (1 << 2),
        
        // sets a certain property, e.g. is happy
        STATE = (1 << 3),
        
        // defines relationship e.g. parent of, has a dog
        RELATIONSHIP = (1 << 4)
    };
    
    constexpr static const char* const ROLE_STRINGS[] = {
        [(uint64)ROLE::AGENT]           = "AGENT",
        [(uint64)ROLE::PATIENT]         = "PATIENT",
        [(uint64)ROLE::TEMPORAL]        = "TEMPORAL",
        [(uint64)ROLE::DIRECT_OBJECT]   = "DIRECT_OBJECT",
        [(uint64)ROLE::INDIRECT_OBJECT_OR_OBJECT_OF_PREPOSITION] = "INDIRECT_OBJECT_OR_OBJECT_OF_PREPOSITION",
    };
    
    
    

    
    struct Noun;
    struct Verb;

    struct Sequence;
    struct Selection;
    struct Number;
    struct Value;
    
    struct Attribute;
    struct Symbol;
    
    enum struct VALUE_KIND {
        NONE,
        THING_TYPE,
        THING_INSTANCE,
        PROPERTY_OF_THING,
        FUNCTION,
        TEXT,
        NUMERIC,
        FLAG,
        VECTOR,
        LIST,
        REFERENCE,
    };
    
    static const mtt::Map<mtt::String, VALUE_KIND> STRING_TO_VALUE_KIND;
    static mtt::Map<uint64, mtt::String> VALUE_KIND_TO_STRING;
    
    struct Value {
        using TYPE = Value;
        static constexpr cstring TYPENAME = "Value";
        
        VALUE_KIND kind;
        mtt::String kind_string = "NONE";
        
        float64       numeric = 0.0f;
        vec4          vector = vec4(0.0f);
        mtt::String   text = "";
        mtt::Thing_ID thing = mtt::Thing_ID_INVALID;
        bool          flag = false;
        
        bool is_reference = false;
        bool is_referenced = false;
    
        
        std::vector<Value> list;
        
        uintptr reference;
        
        Value(void) {}
    };
    Value value;
//    Value mutated_value;
//
//
//
//
//    struct Noun {
//        using TYPE = Noun;
//        static constexpr cstring TYPENAME = "Noun";
//
//        Noun(void) {}
//    };
//    Noun noun;
//
//    struct Pronoun {
//        using TYPE = Pronoun;
//        static constexpr cstring TYPENAME = "Pronoun";
//
//        Pronoun(void) {}
//    };
//    Pronoun pronoun;
//
//    struct Verb {
//        using TYPE = Verb;
//        static constexpr cstring TYPENAME = "Verb";
//
//        Verb(void) {}
//
////        mtt::Map_Stable<mtt::String, std::vector<Speech_Instruction*>> attributes;
//    };
//    Verb verb;
//
//
//
//    //typedef struct dt::Rule* dt::Rule_Ref;
//    using Rule_Ref = Rule*;
//
//    enum struct SEQUENCE_FLAG {
//        CONCURRENT,
//        BEFORE,
//        AFTER,
//    };
    
//    struct Sequence {
//        using TYPE = Sequence;
//        static constexpr cstring TYPENAME = "Seequence";
//
//        Sequence(void) {}
//    };
//    Sequence sequence;
//
//    struct Selection {
//        using TYPE = Selection;
//        static constexpr cstring TYPENAME = "Selection";
//
//        Selection(void) {}
//    };
//    Selection selection;
//
//    struct Number {
//        using TYPE = Number;
//        static constexpr cstring TYPENAME = "Number";
//
//        Number(void) {}
//    };
//    Number number;
//
//
//    struct Variable {
//        using TYPE = Variable;
//        static constexpr cstring TYPENAME = "Variable";
//
//        VALUE_KIND kind;
//        mtt::String kind_string;
//        Value value;
//
//        Variable(void) {}
//    };
//    Variable variable;
//
//
//
//    struct Attribute {
//        using TYPE = Attribute;
//        static constexpr cstring TYPENAME = "Attribute";
//
//        Attribute(void) {}
//    };
//    Attribute attribute;
    
//    void Attribute_print(void)
//    {
//        auto* current = Speech_Instruction_Evaluation::current();
//
//
//        DT_scope_open();
//
//        //DT_print("{%s seq:[%llu] label:[%s]){\n", TYPE_FLAG_TO_STRING[this->type_flag].c_str(), this->sequence_number, this->label.c_str());
//        {
//            {
//                FOR_ITER(prop_list, this->properties, ++prop_list) {
//                    FOR_ITER(prop, prop_list->second, ++prop) {
//                        dt::Speech_Instruction* ins = *prop;
//                        ins->print();
//                    }
//                    DT_print(",\n");
//                }
//
//            }
//
//        }
//        DT_scope_close();
//    }
    
    struct Push_Context {
        using TYPE = Push_Context;
        u8 _;
        
        Push_Context(void) {}
    };
    //Push_Context push_context;
    
    void Push_Context_print(void)
    {
        //auto* current = Speech_Instruction_Evaluation::current();

        DT_print("{%s seq:[%llu] label:[%s]){\n", TYPE_FLAG_TO_STRING[this->type_flag].c_str(), this->sequence_number, this->label.c_str());
    }
    
    struct Clear_And_Set_Context {
        using TYPE = Clear_And_Set_Context;
        
        Clear_And_Set_Context(void) {}
        u8 _;
    };
    //Clear_And_Set_Context clear_and_set_context;
    
    void Clear_And_Set_Context_print(void)
    {
        //auto* current = Speech_Instruction_Evaluation::current();

        DT_print("{%s seq:[%llu] label:[%s]){\n", TYPE_FLAG_TO_STRING[this->type_flag].c_str(), this->sequence_number, this->label.c_str());
    }
    
    void print(void);
    static void print_list(std::vector<Value>& list);
    void print_from_root(void);
    
    
    Speech_Property* get_root(void);

    

    Speech_Property(void) {}

    Speech_Property* refer_next = nullptr;
};

//struct Value {
//    using TYPE = Value;
//    static constexpr cstring TYPENAME = "Value";
//
//    VALUE_KIND kind;
//    mtt::String kind_string = "NONE";
//
//    float64       numeric = 0.0f;
//    vec4          vector = vec4(0.0f);
//    mtt::String   text = "";
//    mtt::Thing_ID thing = mtt::Thing_ID_INVALID;
//    bool          flag = false;
//
//    bool is_reference = false;
//    bool is_referenced = false;
//
//
//    std::vector<Value> list;
//
//    uintptr reference;
//
//    Value(void) {}
//};
//Value value;
struct Text_Value_Cases {
    
};

template <typename PROC>
static bool value_THING_INSTANCE(Speech_Property::Value& value, PROC&& proc)
{
    if (value.thing == mtt::Thing_ID_INVALID) {
        return false;
    }
    
    return mtt::Thing_try_get_then(mtt::ctx(), value.thing, proc);
}
template <typename PROC>
static void value_THING_INSTANCE(Speech_Property* p, PROC&& proc)
{
    value_THING_INSTANCE(p->value, proc);
}



static inline auto value_NUMERIC(Speech_Property::Value& value)
{
    return value.numeric;
}
static inline auto value_NUMERIC(Speech_Property* prop)
{
    return value_NUMERIC(prop->value);
}
template <typename PROC>
static void value_NUMERIC(Speech_Property::Value& value, PROC&& proc)
{
    proc(value.numeric);
}
template <typename PROC>
static void value_NUMERIC(Speech_Property* p, PROC&& proc)
{
    value_NUMERIC(p->value, proc);
}

//template <typename PROC_INSTANCE, typename PROC_TYPE>
//static inline bool value_handle_Thing_Instance_or_Thing_Type(Speech_Property::Value& val, PROC_INSTANCE&& proc_instance, PROC_TYPE&& proc_type)
//{
//    if (val.kind_string == "THING_INSTANCE") {
//        proc_instance(val);
//        return true;
//    } else if (val.kind_string == "THING_TYPE") {
//        proc_type(val);
//        return true;
//    } else {
//        return false;
//    }
//}

template <typename PROC>
static void value_TEXT(Speech_Property::Value& value, PROC&& proc)
{
    proc(value.text);
}
template <typename PROC>
static void value_TEXT(Speech_Property* p, PROC&& proc)
{
    value_TEXT(p->value, proc);
}

template <typename PROC_IF, typename PROC_ELSE>
static void for_each_value_single_or_as_group(Speech_Property& prop, PROC_IF&& proc_if, PROC_ELSE&& proc_else)
{
    if (prop.value.kind_string != "LIST") {
        proc_if(prop.value);
    } else {
        proc_else(prop.value.list);
    }
}
template <typename PROC>
static void for_each_value(Speech_Property& prop, PROC&& proc)
{
    if (prop.value.kind_string != "LIST") {
        proc(prop.value);
    } else {
        for (auto& value : prop.value.list) {
            proc(value);
        }
    }
}

template <typename PROC>
static bool for_each_thing_instance_value(Speech_Property& prop, PROC&& proc)
{
    if (prop.value.kind_string != "LIST") {
        if (prop.value.thing != mtt::Thing_ID_INVALID) {
            mtt::Thing* t = mtt::Thing_try_get_then(mtt::ctx(), prop.value.thing, proc);
            return t != nullptr;
        } else {
            return false;
        }
    } else {
        bool at_least_one_valid = false;
        for (auto& value : prop.value.list) {
            if (value.thing == mtt::Thing_ID_INVALID) {
                continue;
            }
            
            mtt::Thing* t = mtt::Thing_try_get_then(mtt::ctx(), value.thing, proc);
            at_least_one_valid |= (t != nullptr);
        }
        return at_least_one_valid;
    }
}

static inline void for_each_shallow_copy_active_into(cstring key, Speech_Property::Prop_List* dst, Speech_Property* src)
{
    src->for_each(key, [&](auto& p, auto _) {
        dst->push_back(p);
    });
}

static inline void for_each_shallow_copy_active_into(Speech_Property::Prop_List* dst, Speech_Property::Prop_List* src)
{
    for (auto& p : *src) {
        if (!p->should_ignore()) {
            dst->push_back(p);
        }
    }
}

template <typename PROC>
static void for_each(Speech_Property::Prop_List* prop_list, PROC&& proc)
{
    for (auto& p : *prop_list) {
        proc(p);
    }
}

static inline bool is_valid(Speech_Property::Prop_List& pl)
{
    return (&pl != &Speech_Property::Prop_List_INVALID) && !pl.empty();
}
static inline bool is_valid(Speech_Property::Active_Prop_List& pl)
{
    return pl.is_valid();
}

static inline bool is_valid_trigger_response(Speech_Property* prop)
{
    return (!prop->should_ignore() && prop->has_prop("TRIGGER") && prop->has_prop("RESPONSE"));
}

template <typename PROC>
static bool for_each_w_inactive(Speech_Property* prop, cstring& name, PROC&& proc)
{
    if (prop == nullptr) {
        return false;
    }
    auto& result = prop->get(name);
    if (!is_valid(result)) {
        return false;
    }
    for (auto& p : result) {
        proc(p);
    }
    
    return true;
}

template <typename PROC>
static bool for_each(Speech_Property* prop, cstring& name, PROC&& proc)
{
    if (prop == nullptr) {
        return false;
    }
    auto& result = prop->get(name);
    if (!is_valid(result)) {
        return false;
    }
    
    for (auto& p : prop->get(name)) {
        if (p->should_ignore()) {
            continue;
        }
        
        proc(p);
    }
    
    return true;
}

static inline const mtt::String& value_kind(Speech_Property& prop)
{
    return prop.value.kind_string;
}

namespace rules {

struct Conditions {
    enum struct TYPE {
        QUERY = 0,
        COMPARE,
        RELATION,
    } type;
    
    
    static constexpr cstring TYPE_STRINGS[] = {
        "QUERY",
        "COMPARE",
        "RELATION",
    };
    
    struct Comparison {
        enum struct TYPE {
            EQUAL,
            LESS_EQUAL,
            LESS,
            GREATER,
            GREATER_EQUAL,
        } type;
        static constexpr cstring COMPARE_STRINGS[] = {
            "EQUAL",
            "LESS_EQUAL",
            "LESS",
            "GREATER",
            "GREATER_EQUAL",
        };
        
        float32 value;
    };
    
    
    enum struct KIND {
        START,
        STOP
    };
    static constexpr cstring KIND_STRINGS[] = {
        "START",
        "STOP"
    };
    
    struct Query {
        DT_Rule_Query rule = {};
    };
    
    struct Sub_Condition {
        TYPE type;
        KIND kind;
        Comparison comparison;
        
        Query q = {};
        
        dt::Data_Flow flow = {};

        std::vector<Port_Connection_Descriptor> ports = {};
        std::vector<Field_Modify_Descriptor> field_modify = {};
        
        Logic_Bridges logic_bridges;
        dt::Speech_Property::Prop_List* props;
    };
    
    std::vector<Sub_Condition> trigger_clauses;
    std::vector<Sub_Condition> response_clauses;
    
    mtt::Map<mtt::String, mtt::Set<mtt::String>> var_to_role;
    mtt::Map<mtt::String, mtt::Set<mtt::String>> var_label_to_role;
    mtt::Map<mtt::String, mtt::Set<mtt::String>> role_to_var;
};



struct Var_Info {
    bool is_const = false;
    bool has_value = false;
    mtt::Any value = {};
    mtt::String label = "";
};

struct Condition_Builder_Result {
    bool is_new_action = false;
    mtt::String label;
};
struct Condition_Builder {
    
    //mtt::Map<mtt::String, Rule_Variable*> var_symbol_map;
    struct Condition_Builder_Element {
        mtt::String term_str;
        uint64 key;
    };
    std::vector<std::vector<Condition_Builder_Element>> combined = {};
    std::vector<mtt::String> terms = {};
    std::vector<mtt::Query_Rule> rules = {};
    std::vector<ecs_iter_t> rule_iterators = {};
    std::vector<std::vector<mtt::Rule_Var_Record>> vars_for_rule_idx = {};
    mtt::String combined_string = {};
    dt::Dynamic_Array<dt::Dynamic_Array<mtt::String>> conn_components;
    
    Conditions conditions = {};
    Conditions conditions_for_response = {};
    
    
    // for resolving dependencies between clauses
    //mtt::Map<mtt::String, dt::Dynamic_Array<mtt::String>> dependency_graph;
    
    mtt::Map<mtt::String, dt::Dynamic_Array<uint64>> var_to_clause_indices;
    mtt::Map<mtt::String, dt::Dynamic_Array<uint64>> var_to_value_clause_indices;
    
    
    mtt::Map<uint64, dt::Dynamic_Array<mtt::String>> clause_indices_to_var;
    mtt::Map<uint64, dt::Dynamic_Array<mtt::String>> value_clause_indices_to_var;
    
    
    
    mtt::Map<mtt::String, mtt::Set<mtt::String>> var_to_role;
    mtt::Map<mtt::String, mtt::Map_Stable<mtt::String, Var_Info>> role_to_var;
    mtt::Map<mtt::String, mtt::Set<mtt::String>> var_label_to_role;
    mtt::Map<mtt::String, mtt::Set<mtt::String>> role_to_var_label;
    
    mtt::Map<mtt::String, mtt::Set<dt::Speech_Property*>> var_label_to_prop;
    
    mtt::Map<mtt::String, mtt::String>           var_to_var_label;
    mtt::Map<mtt::String, mtt::String>           var_label_to_var;
    
    struct Per_Prop_Roles {
        struct Mappings {
            mtt::Map<mtt::String, mtt::Set<mtt::String>> var_to_role = {};
            mtt::Map<mtt::String, mtt::Map_Stable<mtt::String, Var_Info>> role_to_var = {};
            mtt::Map<mtt::String, mtt::Set<mtt::String>> var_label_to_role = {};
            mtt::Map<mtt::String, mtt::Set<mtt::String>> role_to_var_label = {};
        };
        mtt::Map<Speech_Property::ID_TYPE, Mappings> map = {};
    } per_prop_roles = {};
 
    struct Annotation {
        mtt::String label = {};
        mtt::String relation = "parent_of";
    };
    
    mtt::Map<mtt::String, std::vector<Annotation>> var_to_annotations;
    mtt::Map<Speech_Property*, mtt::Set<mtt::String>> prop_to_var;
    
    
    mtt::Var_Lookup var_lookup = {};
    
    Condition_Builder_Result result = {};
    
    dt::VERB_EVENT event = dt::VERB_EVENT_BEGIN;
    
    mtt::String varname(const mtt::String& in)
    {
#ifdef MTT_FLECS_NEW
        if (in[0] != '$') {
            return "$" + in;
        }
#else
        if (in[0] != '_') {
            return "_" + in;
        }
#endif
        return in;
    }
    
    std::vector<mtt::String> vars_to_find = {};
    
    struct Sym {
        mtt::String name = "";
        bool is_variable = true;
        bool is_const = false;
        bool is_negated = false;
        bool is_id = false;
        bool var_is_used = false;
        mtt::Rule_Var_Handle var = {};
        Speech_Property* prop = nullptr;
        Word_Dictionary_Entry* entry = nullptr;
        mtt::Any value = {};
        
        Logic_Bridges logic_bridges;
        dt::Speech_Property::Prop_List* props;
    };
    
    mtt::Map_Stable<mtt::String, Sym> name_to_sym;
    
    dt::Dynamic_Array<dt::Dynamic_Array<Sym>> clauses;
    dt::Dynamic_Array<dt::Dynamic_Array<Sym>> value_clauses;
    dt::Dynamic_Array<dt::Dynamic_Array<Sym>> relation_clauses;
    
    Condition_Builder() {
        
    }
    

    
    void add_clause(const dt::Dynamic_Array<Sym>& el)
    {
        bool is_duplicate = false;
        for (usize i = 0; i < this->clauses.size(); i += 1) {
            auto& c = this->clauses[i];
            if (c.size() != el.size()) {
                continue;
            }
            
            is_duplicate = true;
            for (usize j = 0; j < c.size(); j += 1) {
                if (el[j].name != c[j].name) {
                    is_duplicate = false;
                    goto LABEL_DUPLICATE_CHECK_DONE;
                }
            }
        }
    LABEL_DUPLICATE_CHECK_DONE:;
//        if (is_duplicate) {
//            return;
//        }
        
        this->clauses.push_back(el);
        for (usize i = 0; i < el.size(); i += 1) {
            auto& el_i = el[i];
            if (el_i.is_variable || el_i.is_const) {
                var_to_clause_indices[el_i.name].push_back(this->clauses.size() - 1);
                clause_indices_to_var[this->clauses.size() - 1].push_back(el_i.name);
            }
            this->name_to_sym[el_i.name] = el_i;
            
        }
    }
    void add_value_clause(const dt::Dynamic_Array<Sym>& el)
    {
        this->value_clauses.push_back(el);
    }
    void add_clause(dt::Dynamic_Array<Sym>& el)
    {
        bool is_duplicate = false;
        for (usize i = 0; i < this->clauses.size(); i += 1) {
            auto& c = this->clauses[i];
            if (c.size() != el.size()) {
                continue;
            }
            
            is_duplicate = true;
            for (usize j = 0; j < c.size(); j += 1) {
                if (el[j].name != c[j].name) {
                    is_duplicate = false;
                    goto LABEL_DUPLICATE_CHECK_DONE;
                }
            }
        }
    LABEL_DUPLICATE_CHECK_DONE:;
        if (is_duplicate) {
            return;
        }
        
        this->clauses.push_back(el);
        for (usize i = 0; i < el.size(); i += 1) {
            auto& el_i = el[i];
            if (el_i.is_variable || el_i.is_const) {
                var_to_clause_indices[el_i.name].push_back(this->clauses.size() - 1);
                clause_indices_to_var[this->clauses.size() - 1].push_back(el_i.name);
            }
            this->name_to_sym[el_i.name] = el_i;
        }
    }
    void add_value_clause(dt::Dynamic_Array<Sym>& el)
    {
        this->value_clauses.push_back(el);
    }
    
    void add_relation_clause(dt::Dynamic_Array<Sym>& el)
    {
        this->relation_clauses.push_back(el);
    }
    
    
    
    mtt::Random_Suffix rng_suffix = {};
    
    mtt::Map<mtt::String, mtt::String> var_name_to_var_name_formatted = {};
    inline void var_name_to_var_name_formatted_reset(void)
    {
        var_name_to_var_name_formatted.clear();
    }
    
    mtt::String var_name_format(const mtt::String& str, bool always_new)
    {
        auto find_it = this->var_name_to_var_name_formatted.find(str);
        if (always_new || find_it == this->var_name_to_var_name_formatted.end()) {
            const mtt::String formatted = str + rng_suffix.get_query_ident();
            this->var_name_to_var_name_formatted.insert({str, formatted});
            return formatted;
        } else {
            return find_it->second;
        }
        //return mtt::str_toupper(str);
    }
    mtt::String var_name_format(const mtt::String& str)
    {
        return var_name_format(str, false);
    }
//    mtt::String set_var(mtt::String& var_name, mtt::String& role_name, Speech_Property* prop)
//    {
//        auto var = var_name_format(var_name);
//        var_to_role[var].insert(role_name);
//        role_to_var[role_name].insert(var);
//
//        prop_to_var[prop].insert(var);
//
//        return var;
//    }
    
    using Trait_List = std::vector<mtt::String>;

    static mtt::String gen_label(const mtt::String& var_name, Trait_List& traits)
    {
        std::sort(traits.begin(), traits.end());
        mtt::String out = var_name;
        for (usize i = 0; i < traits.size(); i += 1) {
            out += "_" + traits[i];
        }
        return out;
    }
    
    mtt::String set_var(const mtt::String& var_name, std::vector<mtt::String> role_names, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name_format(var_name);
        for (const auto& role_name : role_names) {
            var_to_role[var].insert(role_name);
            var_label_to_role[var_name].insert(role_name);
            role_to_var[role_name].insert({var, (Var_Info){.is_const = false}});
            
            role_to_var_label[role_name].insert(var_name);
            var_to_var_label[var] = var_name;
            var_label_to_var[var_name] = var;
            
        }

        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::String set_var_direct_name(const mtt::String& var_name, std::vector<mtt::String> role_names, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name_format(var_name, true);
        for (const auto& role_name : role_names) {
            var_to_role[var].insert(role_name);
            var_label_to_role[var_name].insert(role_name);
            role_to_var[role_name].insert({var, (Var_Info){.is_const = false}});
            
            role_to_var_label[role_name].insert(var_name);
            var_to_var_label[var] = var_name;
            var_label_to_var[var_name] = var;
            
        }

        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::String set_const(const mtt::String& var_name, mtt::Any value, std::vector<mtt::String> role_names, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name;
        for (const auto& role_name : role_names) {
            var_to_role[var].insert(role_name);
            var_label_to_role[var_name].insert(role_name);
            role_to_var[role_name].insert({var, (Var_Info){.is_const = true, .value = value}});
            
            role_to_var_label[role_name].insert(var_name);
            var_to_var_label[var] = var_name;
            var_label_to_var[var_name] = var;
            
        }

        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::String set_var_w_set(const mtt::String& var_name, mtt::Set<mtt::String> role_names, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name_format(var_name);
        for (const auto& role_name : role_names) {
            var_to_role[var].insert(role_name);
            var_label_to_role[var_name].insert(role_name);
            role_to_var[role_name].insert({var, (Var_Info){.is_const = false}});
            
            role_to_var_label[role_name].insert(var_name);
            var_to_var_label[var] = var_name;
            var_label_to_var[var_name] = var;
        }
        
        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::String set_var_w_set_direct_name(const mtt::String& var_name, mtt::Set<mtt::String> role_names, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name_format(var_name, true);
        for (const auto& role_name : role_names) {
            var_to_role[var].insert(role_name);
            var_label_to_role[var_name].insert(role_name);
            role_to_var[role_name].insert({var, (Var_Info){.is_const = false}});
            
            role_to_var_label[role_name].insert(var_name);
            var_to_var_label[var] = var_name;
            var_label_to_var[var_name] = var;
        }
        
        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::String set_const_w_set(const mtt::String& var_name, mtt::Set<mtt::String> role_names, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name;
        for (const auto& role_name : role_names) {
            var_to_role[var].insert(role_name);
            var_label_to_role[var_name].insert(role_name);
            role_to_var[role_name].insert({var, (Var_Info){.is_const = true}});
            
            role_to_var_label[role_name].insert(var_name);
            var_to_var_label[var] = var_name;
            var_label_to_var[var_name] = var;
        }
        
        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::String set_var(const mtt::String& var_name, mtt::Set<Speech_Property*> props, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name_format(var_name);
        for (const auto& role_prop : props) {
            const mtt::String& role_name = role_prop->key;
            var_to_role[var].insert(role_name);
            var_label_to_role[var_name].insert(role_name);
            role_to_var[role_name].insert({var, (Var_Info){.is_const = false}});
            
            role_to_var_label[role_name].insert(var_name);
            var_to_var_label[var] = var_name;
            var_label_to_var[var_name] = var;
        }
        
        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }

    mtt::String set_var_direct_name(const mtt::String& var_name, mtt::Set<Speech_Property*> props, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name_format(var_name, true);
        for (const auto& role_prop : props) {
            const mtt::String& role_name = role_prop->key;
            var_to_role[var].insert(role_name);
            var_label_to_role[var_name].insert(role_name);
            role_to_var[role_name].insert({var, (Var_Info){.is_const = false}});
            
            role_to_var_label[role_name].insert(var_name);
            var_to_var_label[var] = var_name;
            var_label_to_var[var_name] = var;
        }
        
        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::String set_const(const mtt::String& var_name, mtt::Set<Speech_Property*> props, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name;
        for (const auto& role_prop : props) {
            const mtt::String& role_name = role_prop->key;
            var_to_role[var].insert(role_name);
            var_label_to_role[var_name].insert(role_name);
            role_to_var[role_name].insert({var, (Var_Info){.is_const = true}});
            
            role_to_var_label[role_name].insert(var_name);
            var_to_var_label[var] = var_name;
            var_label_to_var[var_name] = var;
        }
        
        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    
    
    mtt::String set_var(const mtt::String& var_name, const mtt::String& role_name, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name_format(var_name);
        
        var_to_role[var].insert(role_name);
        var_label_to_role[var_name].insert(role_name);
        role_to_var[role_name].insert({var, (Var_Info){.is_const = false}});
        
        role_to_var_label[role_name].insert(var_name);
        var_to_var_label[var] = var_name;
        var_label_to_var[var_name] = var;
        
        
        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::String set_var_direct_name(const mtt::String& var_name, const mtt::String& role_name, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name_format(var_name, true);
        
        var_to_role[var].insert(role_name);
        var_label_to_role[var_name].insert(role_name);
        role_to_var[role_name].insert({var, (Var_Info){.is_const = false}});
        
        role_to_var_label[role_name].insert(var_name);
        var_to_var_label[var] = var_name;
        var_label_to_var[var_name] = var;
        
        
        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::String set_const(const mtt::String& var_name, const mtt::String& role_name, Speech_Property* prop, const std::vector<Annotation>& annotations={})
    {
        auto var = var_name;
        
        var_to_role[var].insert(role_name);
        var_label_to_role[var_name].insert(role_name);
        role_to_var[role_name].insert({var, (Var_Info){.is_const = true}});
        
        role_to_var_label[role_name].insert(var_name);
        var_to_var_label[var] = var_name;
        var_label_to_var[var_name] = var;
        
        
        prop_to_var[prop].insert(var);
        var_to_annotations[var] = annotations;
        
        return var;
    }
    
    mtt::Set_Stable<mtt::Thing_ID> dependent_thing_ids = {};
    // some rules depend on specific things that must be tracked
    // for automatic deletion of the rule
    inline void set_thing_dependency(mtt::Thing_ID thing_id)
    {
        dependent_thing_ids.insert(thing_id);
    }
//    mtt::String set_var(cstring var_name, cstring role_name, Speech_Property* prop)
//    {
//        auto var = var_name_format(var_name);
//        var_to_role[var].insert(role_name);
//        role_to_var[role_name].insert(var);
//
//        prop_to_var[prop].insert(var);
//
//        return var;
//    }
    
    
    
    void print()
    {
        MTT_print("%s", "(Condition_Builder) {\n");
        MTT_print("%s", "   clauses:\n");
        for (usize i = 0; i < this->clauses.size(); i += 1) {
            auto& entry = this->clauses[i];
            usize len = entry.size();
            switch (len) {
            case 3: {
                MTT_print("       %s%s(%s, %s)\n",
                          entry[0].is_negated ? MTT_Q_VAR_NOT : " " ,
                          mtt::String( (entry[0].is_variable ? MTT_Q_VAR_PREFIX : "") + entry[0].name).c_str(),
                          mtt::String( (entry[1].is_variable ? MTT_Q_VAR_PREFIX : "") + entry[1].name).c_str(),
                          mtt::String( (entry[2].is_variable ? MTT_Q_VAR_PREFIX : "") + entry[2].name).c_str()
                          );
                break;
            }
            case 2: {
                MTT_print("       %s%s(%s)\n",
                          entry[0].is_negated ? MTT_Q_VAR_NOT : " " ,
                          mtt::String((entry[0].is_variable ? MTT_Q_VAR_PREFIX : "") + entry[0].name).c_str(),
                          mtt::String((entry[1].is_variable ? MTT_Q_VAR_PREFIX : "") + entry[1].name).c_str()
                          );
                break;
            }
            }
        }
        MTT_print("%s", "   value clauses:\n");
        for (usize i = 0; i < this->value_clauses.size(); i += 1) {
            auto& entry = this->value_clauses[i];
            usize len = entry.size();
            switch (len) {
            case 3: {
                MTT_print("        %s(%s, %s)\n",
                          entry[0].name.c_str(),
                          entry[1].name.c_str(),
                          entry[2].name.c_str()
                          );
                break;
            }
            case 2: {
                MTT_print("        %s(%s)\n",
                          entry[0].name.c_str(),
                          entry[1].name.c_str()
                          );
                break;
            }
            }
        }
        MTT_print("%s", "    var to role:\n");
        for (auto it = var_to_role.begin(); it != var_to_role.end(); ++it) {
            MTT_print("        %s\n", it->first.c_str());
            for (auto it_v = it->second.begin(); it_v != it->second.end(); ++it_v) {
                MTT_print("            %s\n", (*it_v).c_str());
            }
        }
        MTT_print("%s", "    [var label to role]:\n");
        for (auto it = var_label_to_role.begin(); it != var_label_to_role.end(); ++it) {
            MTT_print("        %s\n", it->first.c_str());
            for (auto it_v = it->second.begin(); it_v != it->second.end(); ++it_v) {
                MTT_print("            %s\n", (*it_v).c_str());
            }
        }
        MTT_print("%s", "    [role to var label]:\n");
        for (auto it = role_to_var_label.begin(); it != role_to_var_label.end(); ++it) {
            MTT_print("        %s\n", it->first.c_str());
            for (auto it_v = it->second.begin(); it_v != it->second.end(); ++it_v) {
                MTT_print("            %s\n", (*it_v).c_str());
            }
        }
        MTT_print("%s", "}\n");
        MTT_print("%s", "    role to var:\n");
        for (auto it = role_to_var.begin(); it != role_to_var.end(); ++it) {
            MTT_print("        %s\n", it->first.c_str());
            for (auto it_v = it->second.begin(); it_v != it->second.end(); ++it_v) {
                auto& val = (*it_v);
                MTT_print("            %s : is_const=[%s], has_value=[%s]\n", val.first.c_str(), bool_str(val.second.is_const), bool_str(val.second.has_value));
            }
        }
        MTT_print("%s", "}\n");
        
        MTT_print("%s", "    FINAL role to var:\n");
        {
            for (auto p_it = per_prop_roles.map.begin(); p_it != per_prop_roles.map.end(); ++p_it) {
                auto& role_to_var_local = p_it->second.role_to_var;
                for (auto it = role_to_var_local.begin(); it != role_to_var_local.end(); ++it) {
                    MTT_print("        %llu\n", p_it->first);
                    MTT_print("        %s\n", it->first.c_str());
                    for (auto it_v = it->second.begin(); it_v != it->second.end(); ++it_v) {
                        auto& val = (*it_v);
                        auto& el = val.second;
                        MTT_print("            %s : is_const=[%s], has_value=[%s:%llu], label=[%s]\n", varname(val.first).c_str(), bool_str(el.is_const), bool_str(el.has_value), ((el.has_value) ? el.value.thing_id : mtt::Thing_ID_INVALID), el.label.c_str());
                    }
                }
            }
        }
        MTT_print("%s", "}\n");
    }
    
    bool print_roles_for_prop(Speech_Property* prop)
    {
        auto find_it = per_prop_roles.map.find(prop->ID);
        if (find_it == per_prop_roles.map.end()) {
            return false;
        }
        auto& r_el = find_it->second;
        for (auto it = r_el.role_to_var.begin(); it != r_el.role_to_var.end(); ++it) {
            MTT_print("        %s\n", it->first.c_str());
            for (auto it_v = it->second.begin(); it_v != it->second.end(); ++it_v) {
                auto& val = (*it_v);
                auto& el = val.second;
                MTT_print("            %s : is_const=[%s], has_value=[%s:%llu], label=[%s]\n", varname(val.first).c_str(), bool_str(el.is_const), bool_str(el.has_value), ((el.has_value) ? el.value.thing_id : mtt::Thing_ID_INVALID), el.label.c_str());
            }
        }
        
        return true;
    }
    
    void build();
    
};

}

struct Behaviors {
    struct Record {
        my::Function<void(Speech_Command* cmd), 1024> procedure;
    };
    
    mtt::Map<mtt::String, Record> lookup;
    mtt::Map<mtt::String, usize> function_lookup;
};


bool is_derived_from(mtt::Thing* thing, Word_Dictionary_Entry* entry);
bool is_derived_from(mtt::Thing_ID thing_id, Word_Dictionary_Entry* entry);

struct Attribute_Properties {
    // TODO: map? ecs?
    mtt::Any value;
};


void get_prop_or_coref_list_(Speech_Property::Active_Prop_List& pl, Speech_Property::Prop_List* valid_list);




}

#endif /* drawtalk_behavior_hpp */
