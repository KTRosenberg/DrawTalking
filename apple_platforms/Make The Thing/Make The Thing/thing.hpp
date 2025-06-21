//
//  thing.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/5/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef thing_hpp
#define thing_hpp

#include "thing_shared_types.hpp"

#include "string_intern.hpp"




#include "collision.hpp"
#include "stratadraw.h"
#include "color.hpp"


#ifdef MTT_DYNAMIC_LIBRARY_CLIENT
typedef struct b2WorldId
{
    uint16_t index1;
    uint16_t revision;
} b2WorldId;
#endif

typedef b2WorldId Physics_World;


namespace dt {
struct Word_Dictionary_Entry;
}


namespace mtt {

#define MTT_ADD_FIELD(name__, type__, type_sub__) add_field({world, fields, ports, logic, name__, type__, type_sub__ }, builder)
#define MTT_ADD_FIELD_T(name__, TYPE_T__) add_field< TYPE_T__ >({world, fields, ports, logic, name__, MTT_CUSTOM, 0 }, builder, STRING_MAKE( TYPE_T__ ) )

#define MTT_ADD_FIELD_BOOLEAN(name__, value__ )\
add_field< bool >({world, fields, ports, logic, name__, MTT_BOOLEAN, 0 }, builder, STRING_MAKE( bool ) , \
[](void* data) { \
new (data) bool; \
*((bool*)data) = value__ ; \
}, \
[](void* data_dst, void* data_src) { \
(*reinterpret_cast<bool*>(data_dst)) = (*reinterpret_cast<bool*>(data_src)); \
}, \
[](void* data) { \
\
}, \
mtt::MTT_FIELD_FLAG_NONE \
)

#define MTT_ADD_FIELD_T_WITH_DEFAULT(name__, TYPE_T__, value__ )\
add_field< TYPE_T__ >({world, fields, ports, logic, name__, MTT_CUSTOM, 0 }, builder, STRING_MAKE( TYPE_T__ ) , \
[](void* data) { \
auto* out = new (data) TYPE_T__; \
*out = value__ ; \
}, \
[](void* data_dst, void* data_src) { \
(*reinterpret_cast< TYPE_T__ *>(data_dst)) = (*reinterpret_cast< TYPE_T__ *>(data_src)); \
}, \
[](void* data) { \
reinterpret_cast< TYPE_T__ *>(data)->~ TYPE_T__ (); \
}, \
mtt::MTT_FIELD_FLAG_NONE \
)

#define MTT_ADD_FIELD_PRIMITIVE_WITH_DEFAULT(name__, TYPE_T__, native_type__ , value__ )\
add_field< native_type__ >({world, fields, ports, logic, name__, TYPE_T__, MTT_NONE }, builder, STRING_MAKE( native_type__ ) , \
[](void* data) { \
auto* out = new (data) native_type__; \
*out = value__ ; \
}, \
[](void* data_dst, void* data_src) { \
(*reinterpret_cast< native_type__ *>(data_dst)) = (*reinterpret_cast< native_type__ *>(data_src)); \
}, \
[](void* data) { \
\
}, \
mtt::MTT_FIELD_FLAG_NONE \
)


#define MTT_ADD_FIELD_T_EX(name__, TYPE_T__, on_make__, on_copy__, on_destroy__, flags__) \
add_field< TYPE_T__ >({world, fields, ports, logic, name__, MTT_CUSTOM, 0 }, builder, STRING_MAKE( TYPE_T__ ) , on_make__, on_copy__, on_destroy__, flags__ )



void set_ctx(mtt::World* world);
mtt::World* ctx(void);

static const float32 MOVEMENT_SCALE = 128.0;



struct Thing_Ref {
    Thing_ID id       = mtt::Thing_ID_INVALID;
    mtt::World* world = nullptr;
    
    Thing_Ref(void) {}
    Thing_Ref(mtt::Thing* thing);
    Thing_Ref(Thing_ID id, mtt::World* world);
    Thing_Ref(Thing_ID id);
    operator Thing_ID() { return this->id; }
    
    bool operator==(Thing_Ref& other) { return this->id == other.id; }
    bool operator<(Thing_Ref& other)  { return this->id < other.id; }
    bool operator<=(Thing_Ref& other) { return this->id <= other.id; }
    bool operator>=(Thing_Ref& other) { return this->id <= other.id; }
    bool operator>(Thing_Ref& other)  { return this->id < other.id; }
    
    bool operator==(const Thing_Ref& other) const { return this->id == other.id; }
    bool operator<(const Thing_Ref& other)  const { return this->id < other.id; }
    bool operator<=(const Thing_Ref& other) const { return this->id <= other.id; }
    bool operator>=(const Thing_Ref& other) const { return this->id <= other.id; }
    bool operator>(const Thing_Ref& other)  const { return this->id < other.id; }
    
    bool try_get(mtt::Thing** thing);
    mtt::Thing* try_get(void);
    
    
    bool is_valid() { return this->id != Thing_ID_INVALID; }
    
    struct Status {
        mtt::Thing* thing;
        bool ok;
    };
    
    Status get(void)
    {
        mtt::Thing* thing = this->try_get();
        return {
            .thing = thing,
            .ok = (thing != nullptr)
        };
    }
};

static_assert(std::is_standard_layout<Thing_Ref>::value && std::is_trivially_copyable<Thing_Ref>::value, "check that Thing_Ref has standard layout and is trivially copiable");





struct Image_Representation {
    u8 temp;
};

typedef enum SHAPE_TYPE {
    SHAPE_TYPE_PATH,
    SHAPE_TYPE_POLYGON,
    SHAPE_TYPE_QUAD,
    SHAPE_TYPE_RECTANGLE,
    SHAPE_TYPE_TRIANGLE,
    SHAPE_TYPE_CONTOUR,
    SHAPE_TYPE_CONVEX_POLYGON,
    
    SHAPE_TYPE_COUNT
} SHAPE_TYPE;



typedef enum MTT_TYPE : uint32 {
    MTT_NONE = 0,
    MTT_FLOAT,
    MTT_FLOAT64,
    MTT_VECTOR2,
    MTT_VECTOR3,
    MTT_VECTOR4,
    MTT_COLOR_RGBA,
    MTT_INT32,
    MTT_INT64,
    MTT_BOOLEAN,
    MTT_CHAR,
    MTT_LIST,
    MTT_MAP,
    MTT_SET,
    MTT_POLYCURVE,
    MTT_TEXT,
    MTT_TAG,
    MTT_TAG_LIST,
    MTT_POINTER,
    MTT_ANY,
    MTT_PROCEDURE,
    MTT_THING,
    MTT_THING_LIST,
    //    MTT_QUERY,
    MTT_STRING,
    MTT_UNKNOWN,
    MTT_MATRIX4,
    MTT_ARRAY_SLICE,
    MTT_SCRIPT_PROPERTY,
    MTT_CUSTOM,
    
    MTT_TYPE_COUNT
} MTT_TYPE;




typedef mtt::Dynamic_Array<mtt::Dynamic_Array<vec3>> Polycurve;
typedef mtt::Dynamic_Array<Thing_Ref> Thing_List;
typedef mtt::Dynamic_Array<MTT_String_Ref> String_List;


struct Control_Curve_Info {
    //uint64        type;
    //uint64        mode;
    uintptr     points;
    uintptr     distance_sum;
    //uint64        handler;
};

//static mem::Pool_Allocation control_curve_pool;

struct Procedure_Input_Output;
struct Procedure_Return_Type {
    bool value;
    
    Procedure_Return_Type(void) :
    value(true) {}
    
    Procedure_Return_Type(bool value) :
    value(value) {}
    
    operator bool() {
        return this->value;
    }
};

static_assert(std::is_standard_layout<Procedure_Return_Type>::value && std::is_trivially_copyable<Procedure_Return_Type>::value, "check that Procedure_Return_Type has standard layout");

typedef Procedure_Return_Type (*Procedure_Handler)(void* state, Procedure_Input_Output* args);

struct Procedure {
    Procedure_Handler handler;
    void*             state;
    
    
    Procedure_Return_Type operator()(Procedure_Input_Output* args)
    {
        if (this->handler) {
            return this->handler(this->state, args);
        }
        return Procedure_Return_Type(false);
    }
    
    Procedure_Return_Type operator()()
    {
        if (this->handler) {
            return this->handler(this->state, nullptr);
        }
        return Procedure_Return_Type(false);
    }
};
inline static Procedure Procedure_make(Procedure_Return_Type (*handler)(void* state, Procedure_Input_Output* args), void* state)
{
    Procedure proc;
    proc.handler = handler;
    proc.state = state;
    return proc;
}
inline static Procedure Procedure_make(Procedure_Handler handler)
{
    return Procedure_make(handler, NULL);
}
#define MTT_PROC(body__, state__) Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type { body__ }, state__)

static_assert(std::is_standard_layout<Procedure>::value && std::is_trivially_copyable<Procedure>::value, "check that Procedure has standard layout and is trivially copiable");

struct Destroy_Command {
    Thing_ID thing_id;
    
    float32 time_remaining;
    float32 time_delay;
    usize frame_delay = 0;
    
    bool affects_connected;
    bool do_fade_animation;
};

static_assert(std::is_standard_layout<Destroy_Command>::value && std::is_trivially_copyable<Destroy_Command>::value, "check that Destroy_Command has standard layout and is trivially copiable");




MTT_String_Pool* string_pool(void);
MTT_NODISCARD MTT_String_Ref string(cstring str);
MTT_NODISCARD MTT_String_Ref string(cstring str, MTT_String_Length length);
bool string_free(MTT_String_Ref& str);
MTT_NODISCARD cstring string_get(MTT_String_Ref& str);
MTT_NODISCARD MTT_String_Ref string_ref_get(cstring str);

#define MTT_ANY_GEN(__name) \
union alignas(16) __name { \
Mat4 Matrix4; \
Procedure Procedure; \
\
vec4    Vector4; \
vec3    Color_RGBA; \
vec3    Vector3; \
vec2    Vector2; \
\
int64   Int64; \
float64 Float64; \
Thing_ID thing_id; \
int64   Boolean; \
MTT_String_Ref String; \
\
uintptr List; \
uintptr Thing_List; \
uintptr String_List; \
uintptr Polycurve; \
uintptr Map; \
uintptr Set; \
uintptr Reference_Type; \
\
float32 Float; \
\
int32   Int32; \
\
unsigned char Char; \
mtt::Array_Slice_Raw Array_Slice;\
uintptr Script_Property; \
}\

MTT_ANY_GEN(Any_Raw);





struct alignas(16) Any {
    uint64 info = 0;
    mtt::MTT_TYPE type = mtt::MTT_NONE;
    mtt::MTT_TYPE contained_type = mtt::MTT_NONE;
    MTT_ANY_GEN();
    
    //    void from_Polycurve(const Control_Curve_Info& arg)
    //    {
    //        this->type = MTT_POLYCURVE; this->Control_Curve = arg;
    //    }
    
    
    static Any from_Procedure(const mtt::Procedure& arg)
    {
        return {.type = MTT_PROCEDURE, .Procedure = arg};
    }
    void set_Procedure(const mtt::Procedure& val)
    {
        this->type = MTT_PROCEDURE;
        this->Procedure = val;
    }
    
    static Any from_Vector4(const vec4& arg)
    {
        return {.type = MTT_VECTOR4, .Vector4 = arg};
    }
    static Any from_Vector4(const float32& val0, const float32& val1, const float32& val2, const float32& val3)
    {
        return {.type = MTT_VECTOR4, .Vector4 = vec4(val0, val1, val2, val3)};
    }
    void set_Vector4(const vec4& val)
    {
        this->type = MTT_VECTOR4;
        this->Vector4 = val;
    }
    void set_Vector4(const float32& val0, const float32& val1, const float32& val2, const float32& val3)
    {
        this->type = MTT_VECTOR4;
        this->Vector4 = vec4(val0, val1, val2, val3);
    }
    
    static Any from_Vector3(const vec3& arg)
    {
        return {.type = MTT_VECTOR3, .Vector3 = arg};
    }
    static Any from_Vector3(const float32& val0, const float32& val1, const float32& val2)
    {
        return {.type = MTT_VECTOR3, .Vector3 = vec3(val0, val1, val2)};
    }
    void set_Vector3(const vec3& val)
    {
        this->type = MTT_VECTOR3;
        this->Vector3 = val;
    }
    void set_Vector3(const float32& val0, const float32& val1, const float32& val2)
    {
        this->type = MTT_VECTOR3;
        this->Vector3 = vec3(val0, val1, val2);
    }
    
    static Any from_Vector2(const vec2& arg)
    {
        return {.type = MTT_VECTOR2, .Vector2 = arg};
    }
    static Any from_Vector2(const float32& val0, const float32& val1)
    {
        return {.type = MTT_VECTOR2, .Vector2 = vec2(val0, val1)};
    }
    void set_Vector2(const vec2& val)
    {
        this->type = MTT_VECTOR2;
        this->Vector2 = val;
    }
    void set_Vector2(const float32& val0, const float32& val1)
    {
        this->type = MTT_VECTOR2;
        this->Vector2 = vec2(val0, val1);
    }
    
    
    static Any from_Int64(const int64& arg)
    {
        return {.type = MTT_INT64, .Int64 = arg};
    }
    static Any from_UInt64(const uint64& arg)
    {
        return {.type = MTT_INT64, .Int64 = (int64)arg};
    }
    void set_Int64(const int64& val)
    {
        this->type = MTT_INT64;
        this->Int64 = val;
    }
    void set_UInt64(const uint64& val)
    {
        this->type = MTT_INT64;
        this->Int64 = val;
    }
    
    static Any from_Int32(const int32& arg)
    {
        return {.type = MTT_INT32, .Int32 = arg};
    }
    static Any from_UInt32(const uint32& arg)
    {
        return {.type = MTT_INT32, .Int32 = (int32)arg};
    }
    void set_Int32(const int32& val)
    {
        this->type = MTT_INT32;
        this->Int32 = val;
    }
    void set_UInt32(const uint32& val)
    {
        this->type = MTT_INT32;
        this->Int32 = val;
    }
    
    static Any from_Thing_ID(const Thing_ID& arg)
    {
        return {.type = MTT_THING, .thing_id = arg};
    }
    void set_Thing_ID(const Thing_ID& arg)
    {
        this->type = MTT_THING;
        this->thing_id = arg;
    }
    
    static Any from_Boolean(const int64& arg)
    {
        return {.type = MTT_BOOLEAN, .Boolean = arg};
    }
    void set_Boolean(const int64& val)
    {
        this->type = MTT_BOOLEAN;
        this->Boolean = val;
    }
    void set_Boolean(const bool& val)
    {
        this->type = MTT_BOOLEAN;
        this->Boolean = val ? 1 : 0;
    }
    //    uintptr List;
    //    uintptr Map;
    //    uintptr Set;
    //    uintptr Text;
    //    uintptr Polycurve;
    
    static Any from_Reference_Type(const uintptr& arg)
    {
        return {.type = MTT_POINTER, .Reference_Type = arg};
    }
    static Any from_Reference_Type(void* arg)
    {
        return {.type = MTT_POINTER, .Reference_Type = (uintptr)(arg)};
    }
    void set_Reference_Type(const void* val)
    {
        this->type = MTT_POINTER;
        this->Reference_Type = (uintptr)val;
    }
    void set_Reference_Type(void* val)
    {
        this->type = MTT_POINTER;
        this->Reference_Type = (uintptr)val;
    }
    void set_Reference_Type(const uintptr& val)
    {
        this->type = MTT_POINTER;
        this->Reference_Type = val;
    }
    
    
    void set_Script_Property(struct Script_Property& p)
    {
        this->type = MTT_SCRIPT_PROPERTY;
        this->Script_Property = (uintptr)&p;
    }
    
    struct Script_Property& get_Script_Property(void)
    {
        return *((mtt::Script_Property*)this->Script_Property);
    }
    
    
    void set_List(const void* val, MTT_TYPE contained_type)
    {
        this->type = MTT_LIST;
        this->contained_type = contained_type;
        this->Reference_Type = (uintptr)val;
    }
    void set_List(const uintptr& val, MTT_TYPE contained_type)
    {
        this->type = MTT_LIST;
        this->contained_type = contained_type;
        this->Reference_Type = val;
    }
    
    
    static Any from_Float(const float32& arg)
    {
        return {.type = MTT_FLOAT, .Float = arg};
    }
    void set_Float(const float32& val)
    {
        this->type = MTT_FLOAT;
        this->Float = val;
    }
    static Any from_Float32(const float32& arg)
    {
        return {.type = MTT_FLOAT, .Float = arg};
    }
    void set_Float32(const float32& val)
    {
        this->type = MTT_FLOAT;
        this->Float = val;
    }
    
    static Any from_Float64(const float64& arg)
    {
        return {.type = MTT_FLOAT64, .Float64 = arg};
    }
    void set_Float64(const float64& val)
    {
        this->type = MTT_FLOAT64;
        this->Float64 = val;
    }
    
    //    static Any from_Int32(const int32& arg)
    //    {
    //        return {.type = MTT_INT32, .Int32 = arg};
    //    }
    //    void set_Int32(const int32& val)
    //    {
    //        this->type = MTT_INT32;
    //        this->Int32 = val;
    //    }
    
    static Any from_Char(const unsigned char& arg)
    {
        return {.type = MTT_CHAR, .Char = arg};
    }
    void set_Char(const char& val)
    {
        this->type = MTT_CHAR;
        this->Char = val;
    }
    
    static Any from_String(const MTT_String_Ref& arg)
    {
        return {.type = MTT_STRING, .String = arg};
    }
    void set_String(const MTT_String_Ref& val)
    {
        this->type = MTT_STRING;
        this->String = val;
    }
    static Any from_String(const cstring& arg)
    {
        return {.type = MTT_STRING, .String = mtt::string(arg)};
    }
    void set_String(cstring& val)
    {
        this->type = MTT_STRING;
        this->String = mtt::string(val);
    }
    
    cstring get_cstring()
    {
        return string_get(this->String);
    }
    
    //    static Any from_Query(mtt::Query& arg)
    //    {
    //        return {.type = MTT_QUERY, .Query = arg};
    //    }
    //    void set_Query(mtt::Query& val)
    //    {
    //        this->type = MTT_QUERY;
    //        this->Query = val;
    //    }
    
    static Any from_Matrix4(Mat4& arg)
    {
        return {.type = MTT_MATRIX4, .Matrix4 = arg};
    }
    void set_Matrix4(Mat4& val)
    {
        this->type = MTT_MATRIX4;
        this->Matrix4 = val;
    }
    
    
    static Any from_Array_Slice(mtt::Array_Slice_Raw& arg)
    {
        return {.type = MTT_ARRAY_SLICE, .Array_Slice = arg};
    }
    void set_Array_Slice(mtt::Array_Slice_Raw& val)
    {
        this->type = MTT_ARRAY_SLICE;
        this->Array_Slice = val;
    }
    
    bool is_valid(void)
    {
        return this->type != MTT_NONE;
    }
    
    
    
};

struct Rule_Var_Record {
    mtt::Rule_Var_Handle var = -1;
    usize idx = 0;
    std::vector<mtt::Any> values = {};
};
struct Rule_Var_Record_One_Result {
    mtt::Rule_Var_Handle var = -1;
    usize idx = 0;
    mtt::Any value = {};
};

static inline bool Rule_Var_Record_One_Result_is_equal(const Rule_Var_Record_One_Result& a, const Rule_Var_Record_One_Result& b)
{
    return ((a.var == b.var) && (a.idx == b.idx));
}


using Var_Lookup = mtt::Map<mtt::String, mtt::Rule_Var_Record_One_Result>;

struct MTT_NONE_Type {
    using TYPE = MTT_TYPE;
    static constexpr const TYPE type_id = MTT_NONE;
    
    static inline void* raw_value_address(mtt::Any* any)
    {
#define this any
        return nullptr;
#undef this
    }
};
struct MTT_FLOAT_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_FLOAT; using CONCRETE_TYPE = float32;
    
    static inline void* raw_value_address(mtt::Any* any)
    {
#define this any
        return (void*)&this->Float;
#undef this
    }
    static inline CONCRETE_TYPE value(mtt::Any* any)
    {
#define this any
        return this->Float;
#undef this
    }
};
struct MTT_FLOAT64_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_FLOAT64; using CONCRETE_TYPE = float64;
    static inline void* raw_value_address(mtt::Any* any)
    {
#define this any
        return (void*)&this->Float64;
#undef this
    }
    static inline CONCRETE_TYPE value(mtt::Any* any)
    {
#define this any
        return this->Float64;
#undef this
    }
};

struct MTT_VECTOR2_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_VECTOR2; using CONCRETE_TYPE = Vec2; };
struct MTT_VECTOR3_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_VECTOR3; using CONCRETE_TYPE = Vec3; };
struct MTT_VECTOR4_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_VECTOR4; using CONCRETE_TYPE = Vec4; };
struct MTT_COLOR_RGBA_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_COLOR_RGBA; };
struct MTT_INT32_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_INT32; };
struct MTT_INT64_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_INT64; };
struct MTT_BOOLEAN_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_BOOLEAN; };
struct MTT_CHAR_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_CHAR; };
struct MTT_LIST_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_LIST; };
struct MTT_MAP_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_MAP; };
struct MTT_SET_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_SET; };
struct MTT_POLYCURVE_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_POLYCURVE; };
struct MTT_TEXT_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_TEXT; };
struct MTT_TAG_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_TAG; };
struct MTT_TAG_LIST_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_TAG_LIST; };
struct MTT_POINTER_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_POINTER; };
struct MTT_ANY_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_ANY; };
struct MTT_PROCEDURE_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_PROCEDURE; };
struct MTT_THING_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_THING; };
struct MTT_THING_LIST_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_THING_LIST; };
//struct MTT_QUERY_Type {
//    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_QUERY; };
struct MTT_STRING_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_STRING; };
struct MTT_UNKNOWN_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_UNKNOWN; };
struct MTT_MATRIX_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_MATRIX4; };
struct MTT_ARRAY_SLICE_Type {
    using TYPE = MTT_TYPE; static constexpr const TYPE type_id = MTT_ARRAY_SLICE; };



static_assert(std::is_standard_layout<Any>::value && std::is_trivially_copyable<Any>::value, "check that Any has standard layout and is trivially copiable");

struct Procedure_Input_Output {
    mtt::Thing* caller;
    void*       input;
    usize       input_count;
    uint64      input_flags;
    uint64      output_flags;
    
    mtt::Any    output;
};


struct Procedure_With_Any {
    Procedure_Handler handler = {};
    mtt::Any          state = {};
    
    Procedure_Return_Type operator()(Procedure_Input_Output* args)
    {
        if (this->handler) {
            return this->handler(&this->state, args);
        }
        return Procedure_Return_Type(false);
    }
    
    Procedure_Return_Type operator()()
    {
        if (this->handler) {
            return this->handler(&this->state, nullptr);
        }
        return Procedure_Return_Type(false);
    }
};
inline static Procedure_With_Any Procedure_With_Any_make(Procedure_Return_Type (*handler)(void* state, Procedure_Input_Output* args), mtt::Any* state)
{
    Procedure_With_Any proc;
    proc.handler = handler;
    proc.state = *state;
    return proc;
}
inline static Procedure_With_Any Procedure_With_Any_make(Procedure_Handler handler)
{
    mtt::Any state = (mtt::Any){};
    return Procedure_With_Any_make(handler, &state);
}
#define MTT_PROC_WITH_ANY(body__, state__) Procedure_make([](mtt::Any* state, Procedure_Input_Output* args) -> Procedure_Return_Type { body__ }, state__)

static_assert(std::is_standard_layout<Procedure_With_Any>::value && std::is_trivially_copyable<Procedure_With_Any>::value, "check that Procedure_With_Any has standard layout and is trivially copiable");

struct Type_Meta {
    MTT_TYPE type;
    String name;
    usize alloc_byte_size;
    usize actual_byte_size;
    bool is_collection;
    mtt::Any default_value;
    void* def_val = nullptr;
};



static Type_Meta meta[] = {
    {
        MTT_NONE, STRING_MAKE(MTT_NONE), align_up(sizeof(uintptr), 16), sizeof(uintptr), false, mtt::Any::from_Reference_Type((uintptr)0)
    },
    {
        MTT_FLOAT, STRING_MAKE(MTT_FLOAT), align_up(sizeof(float32), 16), sizeof(float32), false, mtt::Any::from_Float(0)
    },
    {
        MTT_FLOAT64, STRING_MAKE(MTT_FLOAT64), align_up(sizeof(float64), 16), sizeof(float64), false, mtt::Any::from_Float64(0)
    },
    {
        MTT_VECTOR2, STRING_MAKE(MTT_VECTOR2),  align_up(sizeof(vec2), 16), sizeof(vec2), false, mtt::Any::from_Vector2(vec2(0.0f))
    },
    {
        MTT_VECTOR3, STRING_MAKE(MTT_VECTOR3),  align_up(sizeof(vec3), 16), sizeof(vec3), false, mtt::Any::from_Vector3(vec3(0.0f))
    },
    {
        MTT_VECTOR4, STRING_MAKE(MTT_VECTOR4),  align_up(sizeof(vec4), 16), sizeof(vec4), false, mtt::Any::from_Vector4(vec4(0.0f)),
    },
    {
        MTT_COLOR_RGBA, STRING_MAKE(MTT_COLOR_RGBA),  align_up(sizeof(vec4), 16), sizeof(vec4), false, {.type = MTT_COLOR_RGBA, .Color_RGBA = vec4(1.0f)}
    },
    {
        MTT_INT32, STRING_MAKE(MTT_INT32),  align_up(sizeof(int32), 16), sizeof(int32), false, mtt::Any::from_Int32(0)
    },
    {
        MTT_INT64, STRING_MAKE(MTT_INT64),  align_up(sizeof(int64), 16), sizeof(int64), false,  mtt::Any::from_Int64(0)
    },
    {
        MTT_BOOLEAN, STRING_MAKE(MTT_BOOLEAN),  align_up(sizeof(uint32), 16), sizeof(uint32), false,  mtt::Any::from_Boolean(false)
    },
    {
        MTT_CHAR, STRING_MAKE(MTT_CHAR),  align_up(sizeof(char), 16), sizeof(char), false,  mtt::Any::from_Char('\0'),
    },
    {
        MTT_LIST, STRING_MAKE(MTT_LIST),  align_up(sizeof(uintptr), 16), sizeof(uintptr), true, {.type = MTT_LIST, .List = 0}
    },
    {
        MTT_MAP, STRING_MAKE(MTT_MAP),  align_up(sizeof(uintptr), 16), sizeof(uintptr), true, {.type = MTT_MAP, .Map = 0}
    },
    {
        MTT_SET, STRING_MAKE(MTT_SET),  align_up(sizeof(uintptr), 16), sizeof(uintptr), true, {.type = MTT_SET, .Set = 0}
    },
    {
        MTT_POLYCURVE, STRING_MAKE(MTT_POLYCURVE),  align_up(sizeof(uintptr), 16), sizeof(uintptr), false, {.type = MTT_POLYCURVE, .Polycurve = 0}
    },
    {
        MTT_TEXT, STRING_MAKE(MTT_String_Ref),  align_up(sizeof(MTT_String_Ref), 16), sizeof(uintptr), false, {.type = MTT_TEXT, .String = 0}
    },
    {
        MTT_TAG, STRING_MAKE(MTT_String_Ref),  align_up(sizeof(MTT_String_Ref), 16), sizeof(uintptr), false, {.type = MTT_TAG, .String = 0}
    },
    {
        MTT_TAG_LIST, STRING_MAKE(MTT_TAG_LIST),  align_up(sizeof(uintptr), 16), sizeof(uintptr), true, {.type = MTT_TAG_LIST, .String_List = 0}
    },
    {
        MTT_POINTER, STRING_MAKE(MTT_POINTER),  align_up(sizeof(uintptr), 16), sizeof(uintptr), false, mtt::Any::from_Reference_Type((uintptr)0)
    },
    {
        MTT_ANY, STRING_MAKE(MTT_ANY),  align_up(sizeof(mtt::Any), 16), sizeof(mtt::Any), false, {.type = MTT_ANY, .Reference_Type = 0}
    },
    {
        MTT_PROCEDURE, STRING_MAKE(MTT_PROCEDURE), align_up(sizeof(mtt::Procedure), 16), sizeof(mtt::Procedure), false, {.type = MTT_PROCEDURE, .Procedure = {}}
    },
    {
        MTT_THING, STRING_MAKE(MTT_THING),  align_up(sizeof(mtt::Thing_Ref), 16), sizeof(mtt::Thing_Ref), false, mtt::Any::from_Thing_ID(mtt::Thing_ID_INVALID)
    },
    {
        MTT_THING_LIST, STRING_MAKE(MTT_THING_LIST),  align_up(sizeof(uintptr), 16), sizeof(uintptr), false, {.type = MTT_THING_LIST, .Thing_List = 0}
    },
    //    {
    //        MTT_QUERY, STRING_MAKE(MTT_QUERY),  align_up(sizeof(mtt::Query), 16), sizeof(mtt::Query), false, {.type = MTT_QUERY, .Query = {}}
    //    },
    {
        MTT_STRING, STRING_MAKE(MTT_STRING),  align_up(sizeof(MTT_String_Ref), 16), sizeof(MTT_String_Ref), false, {.type = MTT_STRING, .String = 0}
    },
    {
        MTT_UNKNOWN, STRING_MAKE(MTT_UNKNOWN), align_up(sizeof(uintptr), 16), sizeof(uintptr), false, {.type = MTT_NONE, .Reference_Type = 0}
    },
    {
        MTT_MATRIX4, STRING_MAKE(MTT_MATRIX4), align_up(sizeof(Mat4), 16), sizeof(Mat4), false, {.type = MTT_MATRIX4, .Matrix4 = Mat4(1.0f)}
    },
    {
        MTT_ARRAY_SLICE, STRING_MAKE(MTT_ARRAY_SLICE), align_up(sizeof(mtt::Array_Slice_Raw), 16), sizeof(mtt::Array_Slice_Raw), true, {.type = MTT_ARRAY_SLICE, .Array_Slice = {}}
    },
    {
        MTT_SCRIPT_PROPERTY, STRING_MAKE(MTT_SCRIPT_PROPERTY),  align_up(sizeof(uintptr), 16), sizeof(uintptr), true, {.type = MTT_SCRIPT_PROPERTY, .Script_Property = 0}
    },
    {
        MTT_CUSTOM, STRING_MAKE(MTT_CUSTOM), align_up(sizeof(uintptr), 16), sizeof(uintptr), false, {.type = MTT_CUSTOM, .Reference_Type = 0}
    },
};

void Any_print(mtt::Any& any);
inline static void Any_print_basic(mtt::Any& any)
{
    auto type = any.type;
    switch (type) {
            
        case MTT_NONE:
            MTT_print("%s\n", meta[type].name.c_str());
            break;
        case MTT_FLOAT:
            MTT_print("%s:%f\n", meta[type].name.c_str(), any.Float);
            break;
        case MTT_FLOAT64:
            MTT_print("%s:%f\n", meta[type].name.c_str(), any.Float64);
            break;
        case MTT_VECTOR2:
            MTT_print("%s:<%f, %f>\n", meta[type].name.c_str(), any.Vector2.x, any.Vector2.y);
            break;
        case MTT_VECTOR3:
            MTT_print("%s:<%f, %f, %f>\n", meta[type].name.c_str(), any.Vector2.x, any.Vector3.y, any.Vector3.z);
            break;
        case MTT_VECTOR4:
            MTT_print("%s:<%f, %f, %f, %f>\n", meta[type].name.c_str(), any.Vector4[0], any.Vector4[1], any.Vector4[2], any.Vector4[3]);
            break;
        case MTT_COLOR_RGBA:
            MTT_print("%s:<%f, %f, %f, %f>\n", meta[type].name.c_str(), any.Color_RGBA[0], any.Color_RGBA[1], any.Color_RGBA[2], any.Color_RGBA[3]);
            break;
        case MTT_INT32:
            MTT_print("%s:%d\n", meta[type].name.c_str(), any.Int32);
            break;
        case MTT_INT64:
            MTT_print("%s:%lld\n", meta[type].name.c_str(), any.Int64);
            break;
        case MTT_BOOLEAN:
            MTT_print("%s:%s\n", meta[type].name.c_str(), (any.Boolean != 0) ? "true" : "false");
            break;
        case MTT_CHAR:
            MTT_print("%s:%c\n", meta[type].name.c_str(), any.Char);
            break;
        case MTT_LIST:
            // TODO: ...
            break;
        case MTT_MAP:
            // TODO: ...
            break;
        case MTT_SET:
            // TODO: ...
            break;
        case MTT_POLYCURVE:
            // TODO: ...
            break;
        case MTT_TEXT:
            MTT_print("%s\n", MTT_string_ref_to_cstring_checked(any.String));
            break;
        case MTT_TAG:
            MTT_print("%s\n", MTT_string_ref_to_cstring_checked(any.String));
            break;
        case MTT_TAG_LIST:
            // TODO: ...
            break;
        case MTT_POINTER:
            // TODO: ...
            break;
        case MTT_ANY:
            // TODO: ...
            break;
        case MTT_PROCEDURE:
            // TODO: ...
            break;
        case MTT_THING:
            //mtt::Thing_print(mtt::ctx(), any.thing_id);
            MTT_print("%s:%lld\n", meta[MTT_THING].name.c_str(), any.thing_id);
            break;
            //        case MTT_THING_LIST:
            //            // TODO: ...
            //            break;
        case MTT_STRING:
            MTT_print("%s\n", MTT_string_ref_to_cstring_checked(any.String));
            break;
        default: {
            break;
        }
            //        case MTT_UNKNOWN:
            //
            //            break;
            //        case MTT_MATRIX4:
            //
            //            break;
            //        case MTT_ARRAY_SLICE:
            //
            //            break;
            //        case MTT_CUSTOM:
            //
            //            break;
            //        case MTT_TYPE_COUNT:
            //
            //            break;
    }
}


static inline void* raw_value_address(mtt::Any* any)
{
    
#define this any
    switch ((this->type))
    {
        case MTT_FLOAT:
            return (void*)&this->Float;
            break;
        case MTT_FLOAT64:
            return (void*)&this->Float64;
            break;
        case MTT_VECTOR2:
            return (void*)&this->Vector2;
            break;
        case MTT_VECTOR3:
            return (void*)&this->Vector3;
            break;
        case MTT_VECTOR4:
            return (void*)&this->Vector4;
            break;
        case MTT_COLOR_RGBA:
            return (void*)&this->Color_RGBA;
            break;
        case MTT_INT32:
            return (void*)&this->Int32;
            break;
        case MTT_INT64:
            return (void*)&this->Int64;
            break;
        case MTT_BOOLEAN:
            return (void*)&this->Boolean;
            break;
        case MTT_CHAR:
            return (void*)&this->Char;
            break;
        case MTT_LIST:
            return (void*)&this->List;
            break;
        case MTT_MAP:
            return (void*)&this->Map;
            break;
        case MTT_SET:
            return (void*)&this->Set;
            break;
        case MTT_POLYCURVE:
            return (void*)&this->Polycurve;
            break;
        case MTT_TEXT:
            return (void*)&this->String;
            break;
        case MTT_TAG:
            return (void*)&this->String;
            break;
        case MTT_TAG_LIST:
            return (void*)&this->String_List;
            break;
        case MTT_POINTER:
            return (void*)&this->Reference_Type;
            break;
        case MTT_ANY:
            return (void*)this;
            break;
        case MTT_PROCEDURE:
            return (void*)&this->Procedure;
            break;
        case MTT_THING:
            return (void*)&this->thing_id;
            break;
        case MTT_THING_LIST:
            return (void*)&this->Thing_List;
            break;
            //    case MTT_QUERY:
            //        return (void*)&this->Query;
            //        break;
        case MTT_STRING:
            return (void*)&this->String;
            break;
        case MTT_MATRIX4:
            return (void*)&this->Matrix4;
            break;
        case MTT_ARRAY_SLICE:
            return (void*)&this->Array_Slice;
            break;
        default:
            return nullptr;
    }
    
    return nullptr;
#undef this
}




constexpr const uint64 OPTION_FLAGS_MOVEMENT_DEFAULT    = 0;
constexpr const uint64 OPTION_FLAGS_MOVEMENT_POSITIONAL = 1;

static inline bool is_default_movement(uint64 flags)
{
    return (flags & OPTION_FLAGS_MOVEMENT_POSITIONAL) == 0;
}

static inline bool is_positional_movement(uint64 flags)
{
    return (flags & OPTION_FLAGS_MOVEMENT_POSITIONAL) == 1;
}

void set_movement_mode(mtt::Thing* thing, uint64 flag);

static inline cstring movement_type_string(uint64 flags)
{
    if (is_default_movement(flags)) {
        return "movement type = [default]";
    } else if (is_positional_movement(flags)) {
        return "movement type = [positional]";
    } else {
        return "movement type = [?]";
    }
}



constexpr const uint64 MOVEMENT_MODE_SET                 = 1;
constexpr const uint64 CONTROL_CURVE_OPTION_POINTS       = 0;
constexpr const uint64 CONTROL_CURVE_OPTION_PARAMETRIC   = 1;


constexpr const uint64 FOLLOWER_OPTION_CHASE = 0;
constexpr const uint64 FOLLOWER_OPTION_FLEE  = 1;

#undef STR


constexpr const uint64 BUILTIN_FIELD_EVENT_UNCHANGED_OR_INVALID = 0;
constexpr const uint64 BUILTIN_FIELD_EVENT_INCREASED            = 1;
constexpr const uint64 BUILTIN_FIELD_EVENT_DECREASED            = 2;

enum FIELD_FLAG {
    MTT_FIELD_FLAG_NONE          = 0,
    MTT_FIELD_FLAG_READ_ONLY     = 1,
    MTT_FIELD_FLAG_COPY_ON_WRITE = 1 << 1,
};

struct Field_Event {
    uint64 type;
    float32 value;
};

static inline void Field_Event_print(Field_Event* ev)
{
    static cstring FIELD_EVENT_STRINGS[] = {
        [BUILTIN_FIELD_EVENT_UNCHANGED_OR_INVALID] =
        "BUILTIN_FIELD_EVENT_UNCHANGED_OR_INVALID",
        [BUILTIN_FIELD_EVENT_INCREASED] = "BUILTIN_FIELD_EVENT_INCREASED",
        [BUILTIN_FIELD_EVENT_DECREASED] = "BUILTIN_FIELD_EVENT_DECREASED",
    };
    
    if (ev->type > 2) {
        MTT_print("%s", "Field_Event type unknown\n");
    } else {
        MTT_print("Field_Event type=[%s]\n", FIELD_EVENT_STRINGS[ev->type]);
    }
}

struct Field_List_Descriptor;

struct Field_Handle {
    usize index;
    bool is_valid;
    Field_List_Descriptor* field_desc;
    operator bool() const { return this->is_valid; }
};

struct Watcher {
    uint64 watcher_id = 0;
    mtt::Thing_ID id = mtt::Thing_ID_INVALID;
    
    my::Function<void(mtt::Thing*, Field_Event)> proc = [](mtt::Thing* thing, Field_Event ev) {};
    my::Function<void(mtt::Thing*)> on_watched_destroy_proc = [](mtt::Thing* thing) {};
    mtt::String  tag = "";
    Field_Handle field_handle = {};
    
    static uint64 next_id;
    static Watcher watch(mtt::Thing* thing,   const mtt::String& tag, my::Function<void(mtt::Thing*, Field_Event)> proc, my::Function<void(mtt::Thing*)> on_watched_destroy_proc);
    static void    unwatch(mtt::Thing* thing, const mtt::String& tag, uint64 watcher_id);
};

// Fields describe data
struct Field {
    MTT_TYPE type;
    MTT_TYPE contained_type;
    usize    byte_offset;
    usize    byte_count;
    FIELD_FLAG flags;
    
    void* data_from(void* base) {
        return (void*)((uintptr)(base) + this->byte_offset);
    }
    
    void (*on_make)(void*) = [](void*) {};
    void (*on_copy)(void*, void*) = [](void*, void*) {};
    void (*on_destroy)(void*) = [](void*) {};
    
    usize    index;
    
    void* watchers;
};



struct Field_List {
    bool is_active = true;
    bool is_active_group = true;
    
    void* contents = nullptr;
    usize byte_offset = 0;
    usize byte_count = 0;
};

struct Field_List_Descriptor {
    SmallVector<Field, 4> fields = {};
    Field_List data = {};
    
    Map<String, uint64>*   name_to_idx = nullptr;
    Map<uint64, String>*   idx_to_name  = nullptr;
    bool is_init = false;
};


void set_event_state(mtt::Thing* thing, const mtt::String& label, const Field_Event& event);
Field_Event* get_event_state(mtt::Thing* thing, const mtt::String& label);




void MTT_Field_print(Field_List* field_list, Field_List_Descriptor* desc, String* tag);
void MTT_Field_print(Field_List* field_list, Field_List_Descriptor* desc, String* tag, Field* field);
void MTT_Field_List_print(Field_List* field_list, Field_List_Descriptor* desc);

struct Field_List_Descriptor;
struct Port_Descriptor;
struct Logic;


// Ports describe inputs and outputs
struct Port {
    MTT_TYPE type = MTT_NONE;
    MTT_TYPE contained_type = MTT_NONE;
    int64  index = 0;
    MTT_String_Ref tag;
    
    bool     is_in_port = true;
    //    bool     is_visible = false;
    bool     is_active = false;
    
    
    
    //    Result<vec3> (*get_and_update_position)(World* world, Thing* thing, void* user_data);
    //    void (*on_remove_connection)(World* world, Thing* thing, void* user_data);
};
struct Spatial_Alignment {
    
    
    vec3 relative_position;
    
    enum struct SPACE_FLAG {
        WORLD,
        CANVAS,
    } space_flag = SPACE_FLAG::WORLD;
    
    void (*proc)(mtt::Thing* source, Spatial_Alignment*) = [](mtt::Thing* source, Spatial_Alignment* align) { };
};


struct Port_List {
    usize begin_index;
    usize count;
};


using Port_Array = SmallVector<Port, 32>;

struct Port_Descriptor {
    Port_Array in_ports;
    //std::vector<Spatial_Alignment> in_port_alignment;
    Port_Array out_ports;
    //std::vector<Spatial_Alignment> out_port_alignment;
    Map<String, uint64>  in_name_to_idx;
    Map<String, uint64>  out_name_to_idx;
};





void MTT_Port_print(Port* desc, MTT_String_Ref* tag, Port* port);
void MTT_Port_Descriptor_print(Port_Descriptor* desc);

constexpr const bool PORT_STATUS_OK = true;

#define MTT_THING_TYPES \
MTT_T_X(FREEHAND_SKETCH, freehand_sketch) \
MTT_T_X(MOVER, mover) \
MTT_T_X(FOLLOWER, mover) \
MTT_T_X(CONTROL_CURVE, control_curve) \
MTT_T_X(TAG, tag) \
MTT_T_X(SENSOR, sensor) \
MTT_T_X(BOLT, bolt) \
MTT_T_X(ROTATOR, rotator) \
MTT_T_X(GRAVITY, gravity) \
MTT_T_X(AND, and) \
MTT_T_X(OR, or) \
MTT_T_X(XOR, xor) \
MTT_T_X(NOT, not) \
MTT_T_X(TIMER, timer) \
MTT_T_X(TEXT, text) \
MTT_T_X(POWER, power) \
MTT_T_X(GROUP, group) \
MTT_T_X(VECTOR, vector) \
MTT_T_X(DESTROYER, destroyer) \
MTT_T_X(TOGGLE, toggle) \
MTT_T_X(SELECTOR, selector) \
MTT_T_X(EMITTER, emitter) \
MTT_T_X(CONVERTER, converter) \
MTT_T_X(MESSENGER, messenger) \
MTT_T_X(RECEIVER, receiver) \
MTT_T_X(JUMP, jump) \
MTT_T_X(RUN_GROUP, run_group) \
MTT_T_X(END, end) \
MTT_T_X(DISTANCE, distance) \
MTT_T_X(UI_ELEMENT, ui_element) \
MTT_T_X(REFERENCE, reference) \
MTT_T_X(RANDOM, random) \
MTT_T_X(QUERY, query) \
MTT_T_X(NUMBER, number) \
MTT_T_X(SLIDER, slider) \
MTT_T_X(DIFFERENCE, difference) \
MTT_T_X(COUNTER, counter) \
MTT_T_X(SIGN, sign) \
MTT_T_X(VALUE, value) \
MTT_T_X(FLOAT32_VALUE, float32_value) \
MTT_T_X(COMPARISON, comparison) \
MTT_T_X(SET_FIELD, set_field) \
MTT_T_X(GET_FIELD, get_field) \
MTT_T_X(SET_RUN_PROP, set_run_prop) \
MTT_T_X(GET_RUN_PROP, get_run_prop) \
MTT_T_X(VARIABLE, variable) \
MTT_T_X(PARAMETER, parameter) \
MTT_T_X(RETURN, return) \
MTT_T_X(CONDITIONAL_BRANCHER, conditional_brancher) \
MTT_T_X(INTERPRETER, interpreter) \
MTT_T_X(CALL, call) \
MTT_T_X(CALL_FOR_EACH, call_for_each) \
MTT_T_X(RUN, run) \
MTT_T_X(RE_INIT, re_init) \
MTT_T_X(SUSPEND, suspend) \
MTT_T_X(IF, if) \
MTT_T_X(COSINE, cosine) \
MTT_T_X(SINE, sine) \
MTT_T_X(COSINE01, cosine01) \
MTT_T_X(SINE01, sine01) \
MTT_T_X(TIME, time) \
MTT_T_X(SET_ACTIVE, set_active) \
MTT_T_X(SET_INACTIVE, set_inactive) \
MTT_T_X(LABEL, label) \
MTT_T_X(GOTO, goto) \
MTT_T_X(MATRIX_MATRIX_MULTIPLY, matrix_matrix_multiply) \
MTT_T_X(MULTIPLY, multiply) \
MTT_T_X(ADD, add) \
MTT_T_X(SUBTRACT, subtract) \
MTT_T_X(DIVIDE, divide) \
MTT_T_X(ROTATE_AROUND, rotate_around) \
MTT_T_X(GROUP_BLOCK_BEGIN, group_block_begin) \
MTT_T_X(GROUP_BLOCK_END, group_block_end) \
MTT_T_X(CODE_PROCEDURE, code_procedure) \
MTT_T_X(ATTACH_TO_PARENT, attach_to_parent) \
MTT_T_X(DETACH_FROM_PARENT, detach_from_parent) \
MTT_T_X(PARTICLE_SYSTEM, particle_system) \
MTT_T_X(PATH_FOLLOWER, path_follower) \
MTT_T_X(NODE_GRAPH, node_graph) \
MTT_T_X(WAIT, wait) \
MTT_T_X(ROTATE_TO, rotate_to) \
MTT_T_X(LOOK_AT, look_at) \
MTT_T_X(SAVE_PHYSICS,    save_physics) \
MTT_T_X(RESTORE_PHYSICS, restore_physics) \
MTT_T_X(DESTROY_THING_AND_CONNECTED, destroy_thing_and_connected) \
MTT_T_X(PRINT, print) \
MTT_T_X(FOR_EACH_BEGIN, for_each_begin) \
MTT_T_X(FOR_EACH_END, for_each_end) \
MTT_T_X(FOR_EACH_PERSISTENT_BEGIN, for_each_persistent_begin) \
MTT_T_X(FOR_EACH_PERSISTENT_END, for_each_persistent_end) \
MTT_T_X(BREAK, break) \
MTT_T_X(CONTINUE, continue) \
MTT_T_X(SET_PROPERTY, set_property) \
MTT_T_X(GET_PROPERTY, get_property) \
MTT_T_X(SET_CALL_PARAMETER, set_call_parameter) \
MTT_T_X(DEBUG__, debug__) \
MTT_T_X(RETURN_STATUS_CONTROL, return_status_control) \
MTT_T_X(LIST_CYCLER, list_cycler) \
MTT_T_X(MOVE, move) \
MTT_T_X(RULE_EVAL, rule_eval) \
MTT_T_X(SYSTEM_ELEMENT, system_element) \
MTT_T_X(MOVE_AWAY, move_away) \
MTT_T_X(ROTATE, rotate) \
MTT_T_X(EXTERNAL, external) \



//MTT_T_X(SET_TO_VELOCITY_MOVEMENT,   set_to_velocity_movement) \
//MTT_T_X(SET_TO_POSITIONAL_MOVEMENT, set_to_positional_movement) \
//MTT_T_X(PUSH_MOVEMENT_TYPE, push_movement_type) \
//MTT_T_X(POP_MOVEMENT_TYPE,  pop_movement_type)


typedef enum ARCHETYPE {
#define MTT_T_X(name, init) ARCHETYPE_ ## name,
    MTT_THING_TYPES
    
    ARCHETYPE_COUNT,
    THING_TYPE_COUNT = ARCHETYPE_COUNT,
#undef MTT_T_X
} ARCHETYPE;

typedef ARCHETYPE THING_TYPE;

const char* const ARCHETYPE_NAMES[] = {
#define MTT_T_X(name, init) [ARCHETYPE_ ## name] = "ARCHETYPE_" STRING_MAKE(name),
    MTT_THING_TYPES
#undef MTT_T_X
};

static inline const char* const arch_to_name(mtt::Thing_Archetype_ID id)
{
    return ARCHETYPE_NAMES[(ARCHETYPE)id];
}

static const mtt::Map<mtt::String, ARCHETYPE> name_to_arch = {
#define MTT_T_X(name, name_id) {"ARCHETYPE_" STRING_MAKE(name), ARCHETYPE_ ## name},
    MTT_THING_TYPES
#undef MTT_T_X
};

static const mtt::Map<mtt::String, ARCHETYPE> short_name_to_arch = {
    // all-lowercase or all-uppercase
#define MTT_T_X(name, name_id) {STRING_MAKE(name), ARCHETYPE_ ## name},
    MTT_THING_TYPES
#undef MTT_T_X
    
#define MTT_T_X(name, name_id) {STRING_MAKE(name_id), ARCHETYPE_ ## name},
    MTT_THING_TYPES
#undef MTT_T_X
};

struct Port_Create_Info {
    World* world;
    Field_List_Descriptor* field_desc;
    Port_Descriptor* ports;
    Logic* logic;
    String tag;
    usize type;
    usize contained_type;
    Spatial_Alignment align;
    Result<vec3> (*get_and_update_position)(World* world, Thing* thing, void* user_data);
    void (*on_remove_connection)(World* world, Thing* thing, void* user_data);
};

struct Field_Create_Info {
    World* world;
    Field_List_Descriptor* field_desc;
    Port_Descriptor* ports;
    Logic* logic;
    String tag;
    usize type;
    usize contained_type;
};

struct Thing_Archetype;



struct Archetype_Param_List {
    World*                 world;
    Field_List_Descriptor* fields;
    Port_Descriptor*       ports;
    Logic*                 logic;
};

inline void Archetype_Param_List_init(Archetype_Param_List* list, World* world, Field_List_Descriptor* fields, Port_Descriptor* ports, Logic* logic)
{
    list->world  = world;
    list->fields = fields;
    list->ports  = ports;
    list->logic  = logic;
}

inline Archetype_Param_List Archetype_Param_List_make(World* world, Field_List_Descriptor* fields, Port_Descriptor* ports, Logic* logic)
{
    Archetype_Param_List list;
    Archetype_Param_List_init(
                              &list, world, fields, ports, logic
                              );
    
    return list;
}



#define PORT_PARAM_LIST Port_Create_Info info

#define FIELD_PARAM_LIST Field_Create_Info info

void Thing_alloc_init_field_list(FIELD_PARAM_LIST, usize bytes_required);

struct Field_List_Builder {
    usize byte_size;
    mtt::Thing_Archetype* archetype;
    World* world;
    Field_List_Descriptor* fields;
    Port_Descriptor* ports;
    Logic* logic;
    
    struct Init_State;
    typedef void (*Init_Proc)(Init_State*, Field_Create_Info, void*, usize, FIELD_FLAG);
    
    
    struct Init_State {
        Init_Proc proc;
        Field_Create_Info info;
        void* value;
        FIELD_FLAG flags;
        void (*on_make)(void* data) = [](void*) {};
        void (*on_copy)(void* data_dst, void* data_src) = [](void*, void*) {};
        void (*on_destroy)(void* data) = [](void*) {};
        usize byte_count = 0;
    };
    
    std::vector<Init_State> initializers;
    
    void init(mtt::Thing_Archetype* archetype, World* world, Field_List_Descriptor* fields, Port_Descriptor* ports, Logic* logic)
    {
        this->byte_size = 0;
        this->archetype = archetype;
        this->world = world;
        this->fields = fields;
        this->ports = ports;
        this->logic = logic;
    }
    
    void append(Init_Proc proc, Field_Create_Info& info, void* value, FIELD_FLAG flags)
    {
        this->byte_size += meta[info.type].alloc_byte_size;
        auto state = Init_State();
        state.proc = proc;
        state.info = info;
        state.value = value;
        state.flags = flags;
        state.byte_count = meta[info.type].alloc_byte_size;
        this->initializers.push_back(state);
    }
    
    void append(Init_Proc proc, Field_Create_Info& info, const mtt::String& type_name, usize byte_size, void (*on_make)(void* data), void (*on_copy)(void* data_dst, void* data_src), void (*on_destroy)(void* data), FIELD_FLAG flags)
    {
        this->byte_size += align_up(byte_size, 16);
        auto state = Init_State();
        state.proc = proc;
        state.info = info;
        state.value = nullptr;
        state.flags = flags;
        state.on_make = on_make;
        state.on_copy = on_copy;
        state.on_destroy = on_destroy;
        state.byte_count = align_up(byte_size, 16);
        this->initializers.push_back(state);
    }
    
    void build(void)
    {
        if (fields->is_init) {
            return;
        }
        
        Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                    byte_size);
        
        for (usize i = 0; i < initializers.size(); i += 1) {
            auto& init_state = this->initializers[i];
            init_state.proc(&init_state, init_state.info, init_state.value, init_state.byte_count, init_state.flags);
        }
        
        this->initializers.clear();
    }
    
};

#define ARCHETYPE_PARAM_LIST mtt::Thing_Archetype* archetype, World* world, Field_List_Descriptor* fields, Port_Descriptor* ports, Logic* logic, Field_List_Builder& builder

typedef void (*Builtin_Archetype_Initializer)(ARCHETYPE_PARAM_LIST);


typedef enum FREEHAND_SKETCH_DEFAULT_INDEX {
    FREEHAND_SKETCH_INDEX_VELOCITY     = 0,
    FREEHAND_SKETCH_INDEX_ACCELERATION = 1,
    FREEHAND_SKETCH_INDEX_POSITION     = 2,
} FREEHAND_SKETCH_DEFAULT_INDEX;



typedef enum JUMP_OPTION_FLAG : uint64 {
    JUMP_OPTION_FLAG_CEILING_THING    = 1 << 0,
    JUMP_OPTION_FLAG_CEILING_POSITION = 1 << 1,
    JUMP_OPTION_FLAG_CEILING_NONE     = 1 << 2,
    
    JUMP_OPTION_FLAG_SOURCE_TYPE_THING    = 1 << 3,
    JUMP_OPTION_FLAG_SOURCE_TYPE_CONSTANT = 1 << 4,
    JUMP_OPTION_FLAG_SOURCE_TYPE_PORT     = 1 << 5,
    
    JUMP_OPTION_FLAG_DESTINATION_TYPE_THING    = 1 << 6,
    JUMP_OPTION_FLAG_DESTINATION_TYPE_CONSTANT = 1 << 7,
    JUMP_OPTION_FLAG_DESTINATION_TYPE_PORT     = 1 << 8,
} JUMP_OPTION_FLAG;



float32 number_get_value(mtt::Thing* thing);
void number_update_value(mtt::Thing* thing, float32 value);
cstring text_get_value(mtt::Thing* thing);
void text_update_value(mtt::Thing* thing, mtt::String& value);


#define MTT_T_X(name, init) void init ## _init(ARCHETYPE_PARAM_LIST);
MTT_THING_TYPES
#undef MTT_T_X

static Builtin_Archetype_Initializer ARCHETYPE_INITIALIZERS[] = {
#define MTT_T_X(name, init) [ARCHETYPE_ ## name] = init ## _init,
    MTT_THING_TYPES
#undef MTT_T_X
};




typedef enum COMPARISON_TYPE {
    COMPARISON_TYPE_LESS = -1,
    COMPARISON_TYPE_GREATER = 1,
    COMPARISON_TYPE_EQUAL = 0,
    COMPARISON_TYPE_LESS_EQUAL,
    COMPARISON_TYPE_GREATER_EQUAL,
} COMPARISON_TYPE;



template<typename T>
struct connection_less_than_key
{
    inline bool operator() (const T& struct1, const T& struct2)
    {
        return (struct1.src_ref < struct2.src_ref);
    }
};

template<typename T>
struct id_less_than
{
    inline bool operator() (const T& struct1, const T& struct2)
    {
        return (struct1.conn.src_ref < struct2.conn.src_ref);
    }
};

template<typename T>
struct id_equals
{
    inline bool operator() (const T& struct1, const T& struct2)
    {
        return (struct1.id == struct2.id);
    }
};

template<typename T>
struct dst_id_equals
{
    inline bool operator() (const T& struct1, const Thing_ID id)
    {
        return (struct1.conn.dst_ref == id);
    }
};

template<typename T>
struct src_id_equals
{
    inline bool operator() (const T& struct1, const Thing_ID id)
    {
        return (struct1.conn.src_ref == id);
    }
};


constexpr const u8 CONNECTION_VALID = 0x1;
constexpr const usize EXECUTION_PORT = ULONG_MAX;
constexpr const uint32 REGULAR_CONNECTION_PRIORITY   = 0;
constexpr const uint32 EXECUTION_CONNECTION_PRIORITY = 1;

// all specific ports
struct Connection_Header {
    uint64   src_port_ref = 0;
    uint64   dst_port_ref = 0;
    
    //    union {
    //        u8 flags;
    ////        struct {
    ////            u8 broken_cycle;
    ////        };
    //    };
    
    //u8 flags = 0;
    unsigned int priority = 0;
    
    bool operator == (const Connection_Header& other) const
    {
        return src_port_ref == other.src_port_ref
        && dst_port_ref == other.dst_port_ref;
    }
};
static_assert(std::is_standard_layout<Connection_Header>::value && std::is_trivially_copyable<Connection_Header>::value, "check that Connection_Header has standard layout and is trivially copiable");



struct Connection {
    Thing_ID src_thingref = mtt::Thing_ID_INVALID;
    Thing_ID dst_thingref = mtt::Thing_ID_INVALID;
    
    Connection_Header header;
    
    bool is_same_connection(const Connection& other) const
    {
        return dst_thingref == other.dst_thingref
        && src_thingref  == other.src_thingref
        && header == other.header;
    }
    
    inline bool operator==(const Connection& other) const { return this->is_same_connection(other); }
    
    bool is_valid() const
    {
        return src_thingref != mtt::Thing_ID_INVALID &&
        dst_thingref != mtt::Thing_ID_INVALID;
    }
};
static_assert(std::is_standard_layout<Connection>::value && std::is_trivially_copyable<Connection>::value, "check that Connection has standard layout and is trivially copiable");

struct Connection_Template_Header {
    mtt::String src_port_name;
    mtt::String dst_port_name;
};
struct Connection_Template {
    Connection connection;
    
    //std::vector<mtt::String> src_label;
    //std::vector<mtt::String> dst_label;
    
    Connection_Template_Header header;
    
    //
    bool update_per_frame;
};

}

#ifndef NDEBUG
namespace std {
extern template class std::vector<mtt::Connection>;
}
extern template class robin_hood::detail::Table<true, 80, unsigned long long, std::vector<mtt::Connection>, robin_hood::hash<unsigned long long>, std::equal_to<unsigned long long>>;
#endif

namespace mtt {

typedef std::vector<Connection> Port_Input_List;

struct Logic_Procedure_Param_List {
    World* world;
    Thing* thing;
    Port_Input_List* input;
};

using LOGIC_PROC_RETURN_TYPE = Logic_Procedure_Return_Status;

#define LOGIC_PROCEDURE_PARAM_LIST World* world, Thing* thing, Port_Input_List* input, Script_Instance* s, Script_Context** s_ctx, void* args
#define LOGIC_PROCEDURE_PARAM_LIST_ARGS world, thing, input, s, s_ctx, args
typedef LOGIC_PROC_RETURN_TYPE (*Logic_Procedure)(LOGIC_PROCEDURE_PARAM_LIST);

#define MTT_T_X(name, proc) LOGIC_PROC_RETURN_TYPE proc ## _procedure(LOGIC_PROCEDURE_PARAM_LIST);
MTT_THING_TYPES
#undef MTT_T_X

static Logic_Procedure ARCHETYPE_LOGIC_PROCEDURES[] = {
#define MTT_T_X(name, proc) [ARCHETYPE_ ## name] = proc ## _procedure,
    MTT_THING_TYPES
#undef MTT_T_X
};

#define MTT_DEFINE_LOGIC_PROCEDURE(name__) LOGIC_PROC_RETURN_TYPE name__ ## _procedure( LOGIC_PROCEDURE_PARAM_LIST )

#define MTT_DEFINE_INITIALIZER(name__) void name__ ## _init( ARCHETYPE_PARAM_LIST )

struct Thing_Field_Initializer {
    mtt::String name;
    
    uint64 type;
    uint64 contained_type;
    
    Procedure Procedure;
    
    mtt::Any field;
};

struct Thing_Option {
    uint64 type;
};




struct Thing_Arg {
    Connection_Header hdr;
    String src_port_name;
    String dst_port_name;
};
struct Thing_Arg_Collection {
    std::vector<Thing_Arg> args;
};

struct Evaluation_Output_Entry_Port {
    Thing_ID ID = mtt::Thing_ID_INVALID;
    //    uint64 type;
    //    uint64 contained_type;
    
    //Procedure Procedure;
    
    mtt::Any out = {};
    
    bool is_ignored = false;
};

struct Evaluation_Output_Entry {
    //Thing_ID ID;
    //uint64   output_type;
    //usize    byte_offset;
    //uint64   type;
    //bool     is_ready;
    
    //std::vector<Evaluation_Output_Entry_Port> port_entries;
    usize first_port_index = 0;
};

struct Evaluation_Output {
    //    void* buffer;
    //    usize byte_count;
    //    usize count;
    //    usize cap;
    std::vector<std::vector<Evaluation_Output_Entry_Port>> port_entries_ =  {{}};
    usize port_entries_index = 0;
    std::vector<Evaluation_Output_Entry> list;
    //    mem::Allocator* allocator;
    //    usize offset;
    //usize index;
    inline std::vector<Evaluation_Output_Entry_Port>& port_entries(void)
    {
        return port_entries_[port_entries_index];
    }
    inline std::vector<Evaluation_Output_Entry_Port>& port_entries(usize index)
    {
        return port_entries_[index];
    }
    inline void set_port_entries_index(usize index)
    {
        port_entries_index = index;
    }
    inline void ensure_port_entries_count(usize count)
    {
        port_entries_.resize(count);
    }
};


struct Logic {
    //Thing_ID thing;
    u64 option;
    u64 option_flags;
    Logic_Procedure proc;
    //bool is_init = false;
    u64 prev_option;
    u64 prev_option_flags;
    
    void save_history(void)
    {
        this->prev_option       = this->option;
        this->prev_option_flags = this->option_flags;
    }
    
    void restore_history(void)
    {
        this->option       = this->prev_option;
        this->option_flags = this->prev_option_flags;
    }
};


constexpr const usize MAX_CONNECTIONS = 2048;
struct Connection_Pool_Entry {
    bool is_occupied;
    union {
        Connection connection;
        uint64     index;
    };
};
struct Connection_Pool {
    usize count;
    Connection connections[MAX_CONNECTIONS];
};




Result<Evaluation_Output_Entry_Port&> get_in_port(World* world, Thing* thing, Port_Input_List* input_list, uint64 port_idx);
Result<Evaluation_Output_Entry_Port&> get_in_port(World* world, Thing* thing, Port_Input_List* input_list, const String& tag);

struct Value_Entry {
    mtt::Any value = {};
    mtt::String name = {};
};
mtt::Dynamic_Array<Value_Entry> get_active_inputs(Thing* thing, Port_Input_List* input_list);

Result<Evaluation_Output_Entry_Port&> get_in_port(Thing* thing, Port_Input_List* input_list, uint64 port_idx);
Result<Evaluation_Output_Entry_Port&> get_in_port(Thing* thing, Port_Input_List* input_list, const String& tag);

template <typename T>
T get_in_port_or(Thing* thing, Port_Input_List* input_list, uint64 port_idx, const T alt);
template <typename T>
T get_in_port_or(Thing* thing, Port_Input_List* input_list, const String& tag, const T alt);
template <typename T>
typename T::CONCRETE_TYPE get_in_port_or(Thing* thing, Port_Input_List* input_list, uint64 port_idx, const typename T::CONCRETE_TYPE alt);
template <typename T>
typename T::CONCRETE_TYPE get_in_port_or(Thing* thing, Port_Input_List* input_list, const String& tag, const typename T::CONCRETE_TYPE alt);


Result<Evaluation_Output_Entry_Port&> get_out_port(Thing* thing, uint64 port_idx);
Result<Evaluation_Output_Entry_Port&> get_out_port(Thing* thing, const String& tag);


template <typename PROC>
bool get_in_port(Thing* thing, Port_Input_List* input_list, const String& tag, PROC&& proc)
{
    Result<Evaluation_Output_Entry_Port&> result = get_in_port(thing, input_list, tag);
    if (result.status == PORT_STATUS_OK) {
        proc(result.value);
        return true;
    }
    
    return false;
}


template <typename PROC>
bool get_out_port(Thing* thing, const String& tag, PROC&& proc)
{
    Result<Evaluation_Output_Entry_Port&> result = get_out_port(thing, tag);
    if (result.status == PORT_STATUS_OK) {
        proc(result.value);
        return true;
    }
    
    return false;
}


bool in_port_is_active(Thing* thing, uint64 port_idx);
bool in_port_is_active(Thing* thing, const String& tag);

bool out_port_is_active(Thing* thing, const String& tag);
bool out_port_is_active(Thing* thing, uint64 port_idx);

// old!
Result<Evaluation_Output_Entry_Port&> get_out_port(World* world, Thing* thing, uint64 port_idx);
Result<Evaluation_Output_Entry_Port&> get_out_port(World* world, Thing* thing, const String& tag);





static constexpr const uint64 MAX_WORLD_GRAPH_ENTRIES = 16000;//2048;





struct Eval_Op_Info {
    // id of the thing
    // representing the code object or the in-world object
    Thing_ID thing_id;
    // cached pointer (may be a bad idea if I switch to non-stable pointers)
    Thing*   thing;
    // function to call for executing the operator
    Logic    logic;
    
    usize eval_index = 0;
};

typedef Thing* Eval_Op;



struct Eval_Op_Info_List {
    std::vector<Eval_Op_Info> list;
};

struct Eval_Op_Data {
    // TODO: implementation
};
struct Eval_Op_Data_List {
    std::vector<Eval_Op_Data> list;
};


struct Script;
struct Script_Instance;

struct Script_Property {
    mtt::Any value = {};
    mtt::String label = {};
    mtt::String selector_name = {};
    mtt::String selector_value = {};
    mtt::String scope = mtt::DEFAULT_LOOKUP_SCOPE;
    mtt::String display_annotation = {};
    bool type_is_random_selection = false;
    bool type_should_choose_all = false;
    bool type_is_action = false;
    float64 counter = 0;
    std::vector<mtt::String> modifiers;
};

void Script_Property_print(Script_Property* s_prop);

inline static Script_Property Script_Property_make(const mtt::Any& value, const mtt::String& label="", const mtt::String& display_annotation="")
{
    return {value, label, "", "", display_annotation};
}

using Script_Property_List = std::vector<Script_Property>;
using Script_Property_Lookup = mtt::Map_Stable<mtt::String, Script_Property_List>;
struct Script_Property_Mapping {
    
    mtt::String          source_key = {};
    mtt::String          target_key = {};
    
    // UNUSED?
    mtt::String          varname_key = {};
    mtt::String          scope_key   = {};
    
    
    
    void print()
    {
        MTT_print("%s", "(Script_Property_Mapping) {\n");
        MTT_print("     source_key:%s\n", source_key.c_str());
        MTT_print("     target_key:%s\n", target_key.c_str());
        MTT_print("%s", "}\n");
    }
};
using Script_Lookup = mtt::Map_Stable<mtt::String, Script_Property_Lookup>;

MTT_NODISCARD
Script_Property_List* Script_Lookup_get_var_with_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var);
void Script_Lookup_set_var_group(Script_Lookup& lookup, const mtt::String& key_ctx, Script_Property_Lookup& var_group);
MTT_NODISCARD
Script_Property_Lookup* Script_Lookup_get_var_group(Script_Lookup& lookup, const mtt::String& key_ctx);
void Script_Lookup_set_var_with_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var, const Script_Property_List& val);
void Script_Lookup_set_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var);
MTT_NODISCARD
Script_Property_List* Script_Lookup_get_var_with_key(Script_Instance* s, const mtt::String& key_ctx, const mtt::String& key_var);
void Script_Lookup_set_var_with_key(Script_Instance* s, const mtt::String& key_ctx, const mtt::String& key_var, const Script_Property_List& val);

template <typename PROC>
bool Script_Lookup_update_var(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var, PROC&& proc)
{
    auto* result = Script_Lookup_get_var_with_key(lookup, key_ctx, key_var);
    if (result != nullptr) {
        proc(*result);
        return true;
    }
    
    return false;
}

void Script_Lookup_mtt_print(Script_Instance* s);


template <typename PROC>
bool Script_Lookup_update_var_for_each(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var, PROC&& proc)
{
    auto* result = Script_Lookup_get_var_with_key(lookup, key_ctx, key_var);
    if (result != nullptr) {
        for (auto& el : *result) {
            proc(el);
        }
        return true;
    }
    
    return false;
}



inline static Script_Property_List SCRIPT_PROPERTY_LIST_INVALID = {};
struct Property_Access {
    mtt::String key = {};
    mtt::String scope = mtt::DEFAULT_LOOKUP_SCOPE;
    
    usize index = 0;
    
    void print(const mtt::String& indent, Script_Instance* s)
    {
        if (s == nullptr) {
            MTT_print("(Access):[%s]:[%s]", key.c_str(), scope.data());
            return;
        }
        
        Script_Property_List* prop = Script_Lookup_get_var_with_key(s, this->key, this->scope);
        if (prop == nullptr) {
            return;
        }
        MTT_print("%s", "{\n");
        for (usize i = 0; i < prop->size(); i += 1) {
            MTT_print("%s[%s]:[%s]:", indent.c_str(), this->key.c_str(), this->scope.c_str()); mtt::Any_print(((*prop)[i]).value);
        }
        MTT_print("%s", "}\n");
    }
    
    
    void update_in_ports(mtt::Thing* thing, Script_Instance* s)
    {
        auto in = get_out_port(thing, 0);
        if (in.status != PORT_STATUS_OK) {
            return;
        }
        
        //Script_Property_List* prop = Script_Lookup_set_var_with_key(s, this->key, this->scope);
        //        if (prop == nullptr) {
        //            return;
        //        }
        
    }
    
    void update_out_ports(mtt::Thing* thing, Port_Input_List* input, Script_Instance* s)
    {
        auto out_value_properties = get_out_port(thing, "value_properties");
        auto out_value_single     = get_out_port(thing, "value_single");
        auto out_selector         = get_out_port(thing, "selector");
        auto out_prop_single      = get_out_port(thing, "value_property_single");
        auto in_increment_index   = get_in_port(thing, input, "increment_index");
        auto out_is_done          = get_out_port(thing, "is_done");
        out_is_done.value.out.set_Boolean(false);
        
        {
            Script_Property_List* prop = Script_Lookup_get_var_with_key(s, this->key, this->scope);
            if (prop == nullptr || prop->empty()) {
                out_value_properties.value.out.set_List((void*)&SCRIPT_PROPERTY_LIST_INVALID, MTT_SCRIPT_PROPERTY);
                
                out_prop_single.value.out.set_Reference_Type((uintptr)0);
                out_prop_single.value.out = {};
                
                out_prop_single.value.out.type = MTT_SCRIPT_PROPERTY;
                out_selector.value.out.set_String("_");
                
                
                out_value_properties.value.is_ignored  = true;
                out_prop_single.value.is_ignored = true;
                out_selector.value.is_ignored    = true;
                out_prop_single.value.is_ignored = true;
                return;
            }
            
            
            out_value_properties.value.is_ignored  = false;
            out_prop_single.value.is_ignored = false;
            out_selector.value.is_ignored    = false;
            out_prop_single.value.is_ignored = false;
            
            
            
            if (in_increment_index.status == PORT_STATUS_OK) {
                if (in_increment_index.value.out.Boolean == true) {
                    if (this->index < prop->size()) {
                        this->index += 1;
                    }
                    //if (this->index >= prop->
                    if (this->index >= prop->size()) {
                        
                        out_is_done.value.out.set_Boolean(true);
                        if (out_is_done.status != PORT_STATUS_OK) {
                            this->index = 0;
                        }
                        
                        out_value_properties.value.out.set_List((void*)&SCRIPT_PROPERTY_LIST_INVALID, MTT_SCRIPT_PROPERTY);
                        
                        out_prop_single.value.out.set_Reference_Type((uintptr)0);
                        out_prop_single.value.out = {};
                        
                        out_prop_single.value.out.type = MTT_SCRIPT_PROPERTY;
                        out_selector.value.out.set_String("_");
                        
                        
                        out_value_properties.value.is_ignored  = true;
                        out_prop_single.value.is_ignored = true;
                        out_selector.value.is_ignored    = true;
                        out_prop_single.value.is_ignored = true;
                        return;
                    } else {
                        
                    }
                }
            }
            
            
            
            
            //            MTT_print("{\n");
            //            for (usize i = 0; i < prop->size(); i += 1) {
            //                MTT_print("  [%s]:[%s]:", this->key.c_str(), this->scope.c_str()); mtt::Any_print(((*prop)[i]).value);
            //            }
            //            MTT_print("}\n");
            
            out_value_properties.value.out.set_List((void*)prop, MTT_SCRIPT_PROPERTY);
            out_prop_single.value.out.set_Reference_Type((uintptr)(&(*prop)[this->index]));
            out_value_single.value.out = (*prop)[this->index].value;
            out_selector.value.out.set_String((*prop)[this->index].selector_value.c_str());
        }
    }
};

struct Query_Rule {
    ecs_rule_t* rule = nullptr;
    mtt::String string_rep = {};
    mtt::World* world_ = nullptr;
    uint64 ref_count = 1;
    bool is_copy = false;
};

using Query_Rule_Var = int32;
struct Query_Rule_Var_Info {
    Query_Rule_Var var = 0;
    mtt::String name = {};
};


World* world(Query_Rule* q);

bool Query_Rule_is_valid(Query_Rule* q);

Rule_Var_Handle Query_Rule_Var_for_name(Query_Rule* q, cstring name);
void Query_Rule_Var_for_name(Query_Rule* q, cstring name, Query_Rule_Var_Info* v_handle);

MTT_NODISCARD
Query_Rule Query_Rule_make(mtt::World* world, const mtt::String& query_string);
void Query_Rule_destroy(Query_Rule* query);
const mtt::String& Query_string(Query_Rule* q);
bool Query_Rule_is_valid(Query_Rule* q);


struct Query_Param {
    Query_Rule query_rule = {};
    std::vector<Query_Rule_Var_Info> rule_handles = {};
    
    
    Query_Rule& rule() { return query_rule; }
    Query_Rule_Var var() { return (!rule_handles.empty()) ? rule_handles[0].var : Rule_Var_Handle_INVALID; }
};

struct Hierarchical_Specifier {
    bool is_instance = true;
    mtt::String type_label = "";
    mtt::Thing_ID thing_instance_id = mtt::Thing_ID_INVALID;
};

struct Call_Descriptor;

struct Call_Param {
    enum struct TYPE {
        FIXED_THINGS,
        QUERY,
        LOCATION,
        SELECTOR,
        SET_FIELD,
        VALUE,
        OP_SWAP_ARGS,
        RULE_RESULT,
        ARG_THINGS,
        TYPES,
    } type = TYPE::FIXED_THINGS;
    inline static cstring TYPE_strings[] = {
        "FIXED_THINGS",
        "QUERY",
        "LOCATION",
        "SELECTOR",
        "SET_FIELD",
        "VALUE",
        "OP_SWAP_ARGS",
        "RULE_RESULT",
        "ARG_THINGS",
        "TYPES",
    };
    
    enum struct SELECTION_TYPE {
        SPECIFIC,
        RANDOM,
    } selection_type = SELECTION_TYPE::SPECIFIC;
    enum struct GET_TYPE {
        SELF,
        FIELD,
        PORT,
        SELECTOR,
        USE_PROCEDURE,
    } get_type = GET_TYPE::SELF;
    
    
    mtt::Any user_data = {};
    void (*proc)(Script_Instance* script_to_modify, Call_Param* call_param, mtt::Any* user_data) = nullptr;
    
    
    //
    // property lookup table:
    Script_Property_Mapping property_lookup_mapping = {};
    Script_Property_List prop_list   = {};
    mtt::Set<mtt::Thing_ID> prev_seen = {};
    bool allow_prev_seen_thing_props = true;
    Script_Property prev_first_property = {};
    
    Query_Param query_param = {};
    bool is_ref = false;
    struct Call_Param_Ref {
        mtt::Thing_ID call_id = mtt::Thing_ID_INVALID;
        usize param_list_index = 0;
        usize param_list_entry_index = 0;
    } ref = {};
    usize count_constraint = 0;
    bool did_early_context_done = false;
    // action might require a selector or "getter" as an argument
    mtt::String selector_name = {};
    mtt::String selector_value = {};
    // or a particular field name
    mtt::String field_name = {};
    // or a connection output from the specific thing
    mtt::Connection connection = {};
    // optional?
    bool is_required_param = true;
    
    std::vector<Hierarchical_Specifier> hierarchical_specifier = {};
    
    mtt::String value_name = {};
    Multipliers multipliers = {};
    
    
    mtt::String label = {};
    mtt::String sub_label = {};
    
    Script_Property_Mapping swap_args_from = {};
    Script_Property_Mapping swap_args_to = {};
    
    std::vector<mtt::Rule_Var_Record_One_Result> rule_vars = {};
    
    
    void print()
    {
        MTT_print("%s", "(Call_Param) {\n");
        MTT_print("  type: %s\n", TYPE_strings[(unsigned int)type]);
        property_lookup_mapping.print();
        MTT_print("selector: %s:%s\n", selector_name.c_str(), selector_value.c_str());
        if (type == TYPE::QUERY) {
            MTT_print("query: %s\n", query_param.query_rule.string_rep.c_str());
        }
        MTT_print("label: %s\n", label.c_str());
        MTT_print("%s", "}\n");
    }
    
};
struct Call_Param_List {
    std::vector<Call_Param> params = {};
    Script_Lookup lookup = {};
    
    
    void print()
    {
        MTT_print("%s", "(Call_Param_List) {\n");
        for (auto& param : params) {
            param.print();
        }
        MTT_print("%s", "}\n");
    }
};


typedef enum SCRIPT_STATUS {
    SCRIPT_STATUS_NOT_STARTED,
    SCRIPT_STATUS_STARTED,
    SCRIPT_STATUS_CANCELED,
    SCRIPT_STATUS_DONE,
    SCRIPT_STATUS_DONE_SHOULD_TERMINATE,
    SCRIPT_STATUS_TERMINATED,
    SCRIPT_STATUS_STOPPED,
    SCRIPT_STATUS_ERROR,
} SCRIPT_STATUS;

static cstring SCRIPT_STATUS_strings[] = {
    "SCRIPT_STATUS_NOT_STARTED",
    "SCRIPT_STATUS_STARTED",
    "SCRIPT_STATUS_CANCELED",
    "SCRIPT_STATUS_DONE",
    "SCRIPT_STATUS_DONE_SHOULD_TERMINATE",
    "SCRIPT_STATUS_TERMINATED",
    "SCRIPT_STATUS_STOPPED",
    "SCRIPT_STATUS_ERROR",
};
static inline cstring SCRIPT_STATUS_string(SCRIPT_STATUS status)
{
    return SCRIPT_STATUS_strings[(int)status];
}

// Instance, meaning, multiple copies of the data corresponding
// to the source script can run

struct Call_Descriptor {
    Script_ID source_script_id = Script_ID_INVALID;
    bool is_active = true;
    usize   count = 1;
    usize   count_remaining = 1;
    float64 time_interval     = 0.0;
    float64 time_remaining    = 0.0;
    float64 last_checked_time = -1.0;
    bool has_time_interval    = false;
    bool should_share_lookup_table = true;
    mtt::Thing_ID call_instruction = mtt::Thing_ID_INVALID;
    mtt::Thing_ID call_instruction_src = mtt::Thing_ID_INVALID;
    
    bool should_timeout_if_not_done = false;
    
    SCRIPT_CALLING_CONVENTION calling_convention = {};
    
    //uint64 TEST_FLAG = 0;
    
    struct Params {
        std::vector<Call_Param_List> param_lists = {};
        Call_Param_List& curr_param_list() {
            return param_lists[current_index];
        }
        Call_Param_List& push_param_list()
        {
            return param_lists.emplace_back();
        }
        
        usize param_list_count()
        {
            return param_lists.size();
        }
        Call_Param_List& param_list(usize i)
        {
            return param_lists[i];
        }
        usize current_index = 0;
    };
    Params params_ = {};
    Params params_saved_ = {};
    inline Params& params(void)
    {
        return this->params_;
    }
    inline Params& params_saved(void)
    {
        return this->params_saved_;
    }
    
    inline void save_params(void)
    {
        this->params_saved_ = this->params_;
    }
    
    void set_script(const mtt::String& label, mtt::Script_ID id)
    {
        this->source_script_id = id;
        this->label = label;
    }
    //    void increment_param_list_index()
    //    {
    //        param_list_index = (param_list_index + 1) % param_lists.size();
    //    }
    
    
    
    
    

    //usize param_list_index = 0;
    
    //bool is_sequence = false;
    //usize sequence_index = 0;
    
    void* user_data = nullptr;
    
    mtt::String label = {};
    
    Multipliers multipliers = {};
    bool is_ping_pong_looping_sequence = false;
    
    
    bool do_check_return_value = false;
    
    MTT_NODISCARD
    Logic_Procedure_Return_Status (*on_invoke)(Call_Descriptor* call, Script_Instance* caller, void (*on_new_script)(Script_Instance* si, void* args), void* args) = nullptr;
    
    void set_on_invoke(Logic_Procedure_Return_Status (*on_invoke)(Call_Descriptor* call, Script_Instance* caller, void (*on_new_script)(Script_Instance* si, void* args), void* args), void* user_data)
    {
        this->on_invoke = on_invoke;
        this->user_data = user_data;
    }
    
    
    
    inline void reset(void)
    {
        count_remaining = count;
        time_remaining = time_interval;
        this->params().current_index = 0;
    }
    
    struct Modifier_Info {
        mtt::String label = "";
        bool is_negated = false;
    };
    std::vector<Modifier_Info> modifiers;
    //
    //    Script_Instance* script = nullptr;
    
};
void Call_Descriptor_stop_for_source(Call_Descriptor* call, mtt::Thing_ID thing_to_stop, uint64 stop_time);

mtt::Call_Descriptor* Call_Descriptor_from_Thing(mtt::World* world, mtt::Thing_ID thing_id);
mtt::Call_Descriptor* Call_Descriptor_from_Thing(mtt::Thing* thing);



struct Trigger_Response_Command {
    Call_Descriptor call = {};
};

struct Script_Operation {
    Script_Instance* script = nullptr;
    usize type = 0;
};
struct Script_Operation_List {
    std::vector<Script_Operation> list = {};
    Call_Descriptor call_info = {};
    
    bool is_waiting_on_subscope = false;
    mtt::Thing* subscope = nullptr;
    //Call_Param_List initial_call_params = {};
};
struct Active_Scripts {
    std::vector<Script_Instance*> list = {};
};
typedef std::vector<Script_Operation_List> Script_Operation_Queue;

struct Script_Instance;

mtt::Call_Descriptor* Call_Descriptor_from_Script_Instance(Script_Instance* s);


typedef enum WAIT_CONDITION {
    WAIT_CONDITION_ALL_SCHEDULED_ENDED,
    WAIT_CONDITION_TIME_INTERVAL,
    WAIT_CONDITION_EVENT,
    WAIT_CONDITION_DEFAULT = WAIT_CONDITION_ALL_SCHEDULED_ENDED,
} WAIT_CONDITION;



struct Group_Block_Begin {
    //Script_Operation_Queue operation_q = {};
    mtt::Thing_ID matching_end = mtt::Thing_ID_INVALID;
    bool is_root = false;
    bool should_push_new_context = false;
};

struct Group_Block_End {
    mtt::Thing_ID matching_begin = mtt::Thing_ID_INVALID;
    usize count = 0;
    float32 time_interval = 0.0f;
    float32 time_remaining = 0.0f;
    float32 last_checked_time = -1.0f;
    bool has_count = false;
    bool has_time_interval = false;
    bool is_matched_with_root = false;
    bool reset_upon_jump = false;
    bool has_exited = false;
};

struct Wait_Block {
    mtt::Any value = {};
    WAIT_CONDITION wait_condition = WAIT_CONDITION_DEFAULT;
};

struct For_Each_Loop_Begin {
    isize count = 0;
    isize index = 0;
    mtt::Thing_ID loop_end = mtt::Thing_ID_INVALID;
    bool is_started = false;
    bool is_infinite = false;
    // no need for per-iteration state, wait until each
    // iteration is done
    bool is_stateless = true;
    bool is_sequence = false;
    
    bool wait_for_iteration_to_end = false;
};
struct For_Each_Loop_End {
    mtt::Thing_ID loop_begin = mtt::Thing_ID_INVALID;
};
struct Break_Statement {
    mtt::Thing_ID continuation = mtt::Thing_ID_INVALID;
};
struct Return_Statement {
    mtt::Any value = {};
};
struct Continue_Statement {
    bool should_reset_on_jump = false;
};
struct Goto_Statement {
    mtt::Thing_ID continuation = mtt::Thing_ID_INVALID;
};

inline static float32 time_interval(Wait_Block* wait)
{
    ASSERT_MSG(wait->value.type == MTT_FLOAT, "incorrect type");
    return wait->value.Float;
}
inline static void time_interval(Wait_Block* wait, float32 value)
{
    ASSERT_MSG(wait->value.type == MTT_FLOAT, "incorrect type");
    wait->value.set_Float(value);
}

struct Script_Context {
    usize instruction_idx = 0;
    usize first_idx = 0;
    usize continuation_idx = 0;
    Script_Operation_Queue local_ops = {};
};
struct Script_Contexts {
    std::vector<Script_Context> ctx_stack = {};
    usize loop_iteration = 0;
    bool is_done = false;
    usize slot_idx = 0;
};

struct Active_Action {
    dt::Word_Dictionary_Entry* action = nullptr;
    mtt::Thing_ID src_thing       = mtt::Thing_ID_INVALID;
    mtt::Thing_ID dst_thing       = mtt::Thing_ID_INVALID;
};






// MARK: Script Lookup


usize Script_context_push(Script_Instance* s, usize continuation_instruction_idx, usize last_index_scope);
usize Script_context_push_for_ctx_id(Script_Instance* s, usize continuation_instruction_idx, usize last_index_scope, usize ctx_idx);
usize Script_context_pop(Script_Instance* s);

void Script_set_current_context(Script_Instance* s, usize idx);
usize Script_get_current_context_idx(Script_Instance* s);

Script_Context& Script_current_context_state(Script_Instance* s);

Script_Context& Script_current_context(Script_Contexts& s_contexts);
Script_Operation_Queue& Script_current_queue(Script_Instance* s);
Script_Instance* Script_Operation_List_enqueue_script(Script_Operation_List& L, Script* callee_script, Script_Instance* s, void* caller_info);



struct Script_Command {
    Script_Instance* script_instance = nullptr;
    Script_Label label = {};
};
struct Script_Command_Sequence {
    mtt::Thing_ID self_thing = mtt::Thing_ID_INVALID;
    std::vector<Script_Command> cmds = {};
    
    inline static mtt::Map<mtt::Thing_ID, Script_Command_Sequence> for_thing = {};
};

struct Eval_Connection_Graph {
    
    Evaluation_Output output = {};
    //Connection_Pool   connection_pool = {};
    //Field_Pool        field_pool;
    
    //    std::vector<Port> in_ports;
    //    std::vector<Port> out_ports;
    
    //Thing_ID sorted_things[MAX_WORLD_GRAPH_ENTRIES];
    std::vector<Eval_Op> sorted_things_direct = {};
    usize visited_count = 0;
    
    bool is_modified = false;
    
    
    Map<Thing_ID, Port_Input_List> incoming = {};
    Map<Thing_ID, std::vector<Port_Input_List>> outgoing = {};
    
    Map_Stable<Thing_ID, mtt::Set<Thing_ID>> incoming_execution = {};
    Map_Stable<Thing_ID, mtt::Set<Thing_ID>> outgoing_execution = {};
};

struct Script_Tasks {
    std::vector<Script_Instance*> rules_list = {};
    std::vector<Script_Instance*> list = {};
};

inline static Script_Property_List LOOKUP_INVALID = {};

struct Script_Precondition {
    Script_ID id = Script_ID_INVALID;
    Script_Lookup match = {};
};

void Script_Lookup_print(Script_Lookup& lookup);

struct Script_Record;

struct Script_Shared_Args {
    void* ctx = nullptr;
    void* data = nullptr;
    void (*procedure_pre_script_eval)(Script* script, void* self) = nullptr;
    void (*procedure_post_script_eval)(Script* script, void* self) = nullptr;
};

// Think of this like a template for the running script
struct Script {
    Script_ID id = Script_ID_INVALID;
    static inline Script_ID next_avail_ID = 1;
    static mtt::Map_Stable<Script_ID, Script> scripts;
    static mtt::Map_Stable<mtt::String, mtt::Map<SCRIPT_CALLING_CONVENTION, Script_ID>> scripts_by_name_and_calling_convention;
    
    static mtt::Map_Stable<Script_ID, Script_Shared_Args> shared_args;
    
    
    Script_Lookup lookup_ = {};
    bool is_alias = false;
    
    bool allow_duplicates_for_agent = false;
    
    inline Script_Lookup& lookup(void)
    {
        return lookup_;
    }
    
    Script* alias_make(void)
    {
        // This works for now because an alias will just be readonly besides its lookup table
        Script* s = &Script::scripts[Script::next_avail_ID];
        s->id = Script::next_avail_ID;
        Script::next_avail_ID += 1;
        
        usize own_id = s->id;
        *s = *this;
        s->id = own_id;
        
        s->is_alias = true;
        s->ref_count = 0;
        
        return s;
    }
    
    
    
    std::vector<Script_Precondition> preconditions;
    
    void add_precondition(Script_ID script_id)
    {
        Script_Precondition pc = {};
        pc.id = script_id;
        preconditions.push_back(pc);
    }
    
    std::vector<Script_Precondition>& get_preconditions()
    {
        return this->preconditions;
    }
    
    Script_Label label = {};
    Script_Label sub_label = {};
    
    SCRIPT_CALLING_CONVENTION calling_convention = {};
    
    std::vector<Script_Contexts> contexts = {};
    
    Eval_Connection_Graph connections = {};
    
    bool preserve_lookup = false;
    bool share_lookup_with_sub_scripts = false;
    // if set to true, does not depend on child scripts
    // e.g. trigger/response drivers do not need to check child statuses
    bool detach_children = false;
    
    usize ref_count = 0;
    bool destroy_upon_ref_count_0 = false;
    bool is_infinite = false;
    
    
    
    mtt::Logic_Procedure_Return_Status (*on_start)(mtt::World* world, Script_Instance* self_script, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_begin_frame)(mtt::World* world, Script_Instance* script_instance, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_end_frame)(mtt::World* world, Script_Instance* script_instance, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_cancel)(mtt::World* world, Script_Instance* self_script, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_done)(mtt::World* world, Script_Instance* self_script, void* args) = nullptr;
    mtt::Logic_Procedure_Return_Status (*on_terminate)(mtt::World* world, Script_Instance* self_script, void* args) = nullptr;
    
    bool is_rule = false;
};
Script_Property_List& Script_lookup_var_with_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var);
MTT_NODISCARD
Script_Property_List& lookup_set_var_with_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var);

Script_Shared_Args* Script_type_args(void);
Script_Shared_Args* Script_shared_args_update(Script* s);
void Script_shared_eval_pre(void);
void Script_shared_eval_post(void);

struct Script_Evaluation_Context {
    Script_Instance* root_script = nullptr;
    
    static Script_Evaluation_Context* make(void);
    static void finish(Script_Evaluation_Context* ctx);
    static void destroy(Script_Evaluation_Context** ctx);
};

struct Signal_Mailbox {
    mtt::Map<mtt::String, std::vector<mtt::Any>> signals = {};
};
void Signal_Mailbox_push(Signal_Mailbox* mb, const mtt::String& name, mtt::Any& data);
void Signal_Mailbox_clear(Signal_Mailbox* mb);
std::vector<mtt::Any>* Signal_Mailbox_access(Signal_Mailbox* mb, const mtt::String& name);

struct Runtime {
    Script_Evaluation_Context* eval_self = nullptr;
    
    Script_Tasks script_tasks = {};
    
    Signal_Mailbox signal_mailbox = {};
    
    mtt::Map<mtt::Thing_ID, mtt::Map_Stable<mtt::Thing_ID, uint64>> sources_to_stop_for_call = {};
    
    mtt::Map_Stable<Script_Instance_Ref, mtt::Dynamic_Array<mtt::Thing_ID>> Script_Instance_things_to_stop = {};
    
    mtt::Map_Stable<mtt::Thing_ID, mtt::Set<Script_ID>> Thing_ID_to_Rule_Script = {};
    
    mtt::Map_Stable<mtt::Script_ID, mtt::Script_Instance_Ref> id_to_rule_script = {};
    mtt::Map<mtt::Script_ID, mtt::String> rule_label_map = {};
    
    static Runtime* ctx(void);
};
void push_script_task(Script_Tasks* tasks, Script_Instance* script);

struct Script_Rules;
void push_script_rule_task(Script_Tasks* tasks, Script_Instance* script, Script_Rules& rules);








template <typename K, typename V, typename Map_Type>
void map_get(Map_Type* map, K key, V** out)
{
    auto result = map->find(key);
    
    *out = &result->second;
}

template <typename K, typename V, typename Map_Type>
V* map_get(Map_Type* map, K key)
{
    auto result = map->find(key);
    
    return &result->second;
}

template <typename K, typename V, typename Map_Type>
inline void map_get(Map_Type& map, K key, V** out)
{
    return map_get<K, V, Map_Type>(&map, key, out);
}

template <typename K, typename V, typename Map_Type>
inline V* map_get(Map_Type& map, K key)
{
    return map_get<K, V, Map_Type>(&map, key);
}

template <typename K, typename V, typename Map_Type>
bool map_try_get(Map_Type* map, K key, V** out)
{
    auto result = map->find(key);
    if (result == map->end()) {
        *out = nullptr;
        return false;
    }
    
    *out = &result->second;
    
    return true;
}

template <typename K, typename V, typename Map_Type>
bool map_try_get(Map_Type* map, K key, const V** out)
{
    auto result = map->find(key);
    if (result == map->end()) {
        *out = nullptr;
        return false;
    }
    
    *out = &result->second;
    
    return true;
}

template <typename K, typename V>
mtt::Result<V*> map_try_get(mtt::Map<K, V>* map, K key)
{
    auto result = map->find(key);
    if (result == map->end()) {
        return {false, nullptr};
    }
    
    return {true, &result->second};
}
template <typename K, typename V>
mtt::Result<V*> map_try_get(mtt::Map<K, V>& map, K key)
{
    return map_try_get<K, V>(&map, key);
}

template <typename K, typename V>
mtt::Result<V*> map_try_get(mtt::Map_Stable<K, V>* map, K key)
{
    auto result = map->find(key);
    if (result == map->end()) {
        return {false, nullptr};
    }
    
    return {true, &result->second};
}
template <typename K, typename V>
mtt::Result<V*> map_try_get(mtt::Map_Stable<K, V>& map, K key)
{
    return map_try_get<K, V>(&map, key);
}


template <typename K, typename V, typename Map_Type>
inline bool map_try_get(Map_Type& map, K key, V** out)
{
    return map_try_get<K, V, Map_Type>(&map, key, out);
}

//template <typename K, typename V, typename Map_Type>
//mtt::Result<V*> map_try_get(Map_Type& map, K key)
//{
//    return map_try_get<K, V, Map_Type>(&map, key);
//}

template <typename K, typename V, typename Map_Type>
bool map_get_or_set(Map_Type* map, K key, V** out)
{
    auto result = map->find(key);
    if (result == map->end()) {
        auto it = map->emplace(key, V());
        *out = &it.first->second;
        
        return false;
    }
    
    *out = &result->second;
    
    return true;
}
template <typename K, typename V, typename Map_Type>
inline bool map_get_or_set(Map_Type& map, K key, V** out)
{
    return map_get_or_set<K, V, Map_Type>(&map, key, out);
}


template <typename K, typename V, typename Map_Type>
void map_set(Map_Type* map, K key, V val, V** out)
{
    auto it = map->insert({key, val});
    *out = &it.first->second;
}
template <typename K, typename V, typename Map_Type>
inline void map_set(Map_Type& map, K key, V val, V** out)
{
    map_set<K, V, Map_Type>(&map, key, val, out);
}

template <typename K, typename Map_Type>
inline void map_erase(Map_Type& map, const K& key)
{
    map.erase(key);
}

template <typename K, typename Map_Type>
inline void map_erase(Map_Type* map, const K& key)
{
    map->erase(key);
}

template <typename K, typename V>
inline void map_set(mtt::Map<K, V>* map, K key, V val)
{
    V* ptr;
    (void)ptr;
    map_set<K, V>(map, key, val, &ptr);
}
template <typename K, typename V>
inline void map_set(mtt::Map<K, V>& map, K key, V val)
{
    V* ptr;
    (void)ptr;
    map_set<K, V>(&map, key, val, &ptr);
}

template <typename K, typename V>
inline void map_set(mtt::Map_Stable<K, V>* map, K key, V val)
{
    V* ptr;
    (void)ptr;
    map_set<K, V>(map, key, val, &ptr);
}
template <typename K, typename V>
inline void map_set(mtt::Map_Stable<K, V>& map, K key, V val)
{
    V* ptr;
    (void)ptr;
    map_set<K, V>(&map, key, val, &ptr);
}

/////////////////////

constexpr const usize Evaluation_Output_Entry_count_default_init = 2048;
//static Evaluation_Output_Entry entries[Evaluation_Output_Entry_count_cap];
void World_Graph_init(Eval_Connection_Graph* G, mem::Allocator* allocator, usize count = Evaluation_Output_Entry_count_default_init);

void Connections_init(Connection_Pool* connection_pool, mem::Allocator*);
using Drawable_Info_List = std::vector<sd::Drawable_Info*>;
struct Render_Data {
    //sd::Render_Layer* layer;
    Drawable_Info_List drawable_info_list;
    bool is_shared = false;
    
    void (*_on_copy_inner)(void*, Render_Data&, Render_Data&, void*);
    
    inline void set_on_copy(void (*proc)(void*, Render_Data&, Render_Data&, void*))
    {
        this->_on_copy_inner = proc;
    }
    
    using On_Copy_Callback = void (*)(void*, Render_Data&, Render_Data&, void*);
    
    inline On_Copy_Callback get_on_copy(void)
    {
        return this->_on_copy_inner;
    }
    
    inline void on_copy(void* ctx, Render_Data& dst, void* args)
    {
        if (this->_on_copy_inner != nullptr) {
            this->_on_copy_inner(ctx, dst, *this, args);
        }
    }
    
    Render_Data();
};

struct GFX_Attrib_Range {
    vec4 color;
    sd::SHAPE_MODE shape_mode;
    usize i_begin;
    usize count;
    
    GFX_Attrib_Range() : color(vec4(0.0f)), shape_mode(sd::SHAPE_MODE_NONE), i_begin(0), count(0) {}
};

// based on NanoVG's Text align enum
//enum OFFSET_ALIGN {
//    // Horizontal align
//    OFFSET_ALIGN_CENTER     = 1<<0,    // Default Align to center.
//    OFFSET_ALIGN_LEFT         = 1<<1,    // Align horizontally to left.
//    OFFSET_ALIGN_RIGHT     = 1<<2,    // Align text horizontally to right.
//    // Vertical align
//    OFFSET_ALIGN_TOP         = 1<<3,    // Align vertically to top.
//    OFFSET_ALIGN_MIDDLE    = 1<<4,    // Align vertically to middle.
//    OFFSET_ALIGN_BOTTOM    = 1<<5,    // Align vertically to bottom.
//};
// multiply by these values
enum OFFSET_ALIGN : int64 {
    // Horizontal align
    OFFSET_ALIGN_CENTER      =  0,    // Default Align to center.
    OFFSET_ALIGN_LEFT        = -1,    // Align horizontally to left.
    OFFSET_ALIGN_RIGHT       =  1,    // Align text horizontally to right.
    // Vertical align
    OFFSET_ALIGN_TOP         = -1,    // Align vertically to top.
    OFFSET_ALIGN_MIDDLE      =  0,    // Align vertically to middle.
    OFFSET_ALIGN_BOTTOM      =  1,    // Align vertically to bottom.
};
static inline vec2 offset_align_to_vec2(const OFFSET_ALIGN align)
{
    return vec2(((uint64)align) & 0xFFFF'FFFF'0000'0000,
                ((uint64)align) & 0x0000'0000'FFFF'FFFF);
}
static inline vec2 offset_align_to_vec2(vec2 offset, const OFFSET_ALIGN align)
{
    return offset * vec2(((uint64)align) & 0xFFFF'FFFF'0000'0000,
                         ((uint64)align) & 0x0000'0000'FFFF'FFFF);
}

// switch to something like here: https://gamedev.stackexchange.com/questions/174319/dealing-with-more-complex-entities-in-an-ecs-architecture/174327#174327
//
// parent = parentIndex[current];
// worldPosition[current]    = worldPosition[parent]
//     + worldOrientation[parent] * localPosition[current];
//
// worldOrientation[current] = worldOrientation[parent] * localOrientation[current];
// worldScale[current]       = worldScale[parent]       * localScale[current];
//
//
//
typedef uint16 REPRESENTATION_TYPE;

static const REPRESENTATION_TYPE REPRESENTATION_TYPE_CURVE = 1;
static const REPRESENTATION_TYPE REPRESENTATION_TYPE_IMAGE = 2;

struct Representation {
    
    mat4      model_transform;
    mat4      model_transform_inverse;
    
    mat4      hierarchy_model_transform;
    
    mat4      pose_transform;
    
    vec3 offset;
    
    vec3 init_forward_dir;
    vec3 forward_dir;
    vec3 center_offset;
    
    OFFSET_ALIGN offset_align;
    
    // world
    Transform transform;
    // local non-shared
    Transform pose_transform_values;
    
    // for LERPs based on original
    //mat4 pose_transform_original;
    // local non-shared
    //Transform pose_transform_values_original;
    ///
    
    
    // local shared
    //Transform local_transform_values;
    
    Render_Data render_data;
    
    vec3 velocity;
    vec3 acceleration;
    
    vec3 saved_velocity;
    vec3 saved_acceleration;
    
    std::vector<std::vector<vec3>>        points;
    std::vector<std::vector<vec3>>        points_alt;
    std::vector<std::vector<GFX_Attrib_Range>> colors;
    std::vector<std::vector<float64>>     radii;
    std::vector<Collider*>                colliders;
    std::vector<Spatial_Alignment>        alignment_anchors;
    std::vector<REPRESENTATION_TYPE>      representation_types;
    
    void append_new(REPRESENTATION_TYPE type)
    {
        points.emplace_back();
        colors.emplace_back();
        radii.emplace_back();
        representation_types.push_back(type);
    }
    
    
    sd::Shader_ID shader_id;
    
    
    Representation()
    {
        this->render_data = Render_Data();
        this->render_data.is_shared = false;
        //this->render_data.layer = nullptr;
        Transform_init(&this->transform);
        Transform_init(&this->pose_transform_values);
//        this->pose_transform_values = {};
        
//        this->pose_transform_values.translation = vec3(0.0f, 0.0f, 0.0f);
//        this->pose_transform_values.rotation = vec3(0.0f, 0.0f, 0.0f);
//        this->pose_transform_values.scale    = vec3(1.0f, 1.0f, 1.0f);
        
        this->model_transform         = mat4(1.0);
        this->model_transform_inverse = mat4(1.0);
        this->hierarchy_model_transform         = mat4(1.0);
        //this->world_transform_inverse = mat4(1.0);
        this->pose_transform          = mat4(1.0);
        this->offset       = vec3(0.0);
        
        this->offset_align = OFFSET_ALIGN_CENTER;
        
        this->init_forward_dir = vec3(1.0f, 0.0f, 0.0f);
        this->forward_dir = vec3(1.0f, 0.0f, 0.0f);
        
        this->center_offset = vec3(0.0f, 0.0f, 0.0f);
        
        this->velocity     = vec3(0.0);
        this->acceleration = vec3(0.0);
        
        this->saved_velocity     = vec3(0.0);
        this->saved_acceleration = vec3(0.0);
        
        this->shader_id = sd::Shader_ID_INVALID;
    }
    
};






static inline Collider* first_collider(mtt::Rep* rep)
{
    return rep->colliders.back();
}

struct Component_Descriptor {
    String tag;
    usize  data;
    usize  type_id;
};
struct Component_Instance_Data {
    String tag;
    std::vector<MTT_TYPE> types;
    std::vector<void*>* data;
};



struct Component {
    uint64 id;
    String tag;
    usize data_size;
    Map<Thing_ID, Component_Instance_Data> thing_to_component_data;
};


enum TRIGGER_TYPE {
    TRIGGER_TYPE_DEFAULT,
    TRIGGER_TYPE_ONE_SHOT,
};
struct Trigger {
    TRIGGER_TYPE type;
    mtt::Procedure proc;
};
struct Input_Triggers {
    vec3 prev_position;
    
    Trigger on_input_began;
    Trigger on_input_changed;
    Trigger on_input_ended;
    Trigger on_input_cancelled;
};

constexpr const uint64 INPUT_MODALITY_BIT = 0;
typedef enum INPUT_MODALITY_FLAG {
    INPUT_MODALITY_FLAG_PEN = 0,
    INPUT_MODALITY_FLAG_TOUCH,
    INPUT_MODALITY_FLAG_TOUCH2,
    INPUT_MODALITY_FLAG_TOUCH3,
    INPUT_MODALITY_FLAG_TOUCH4,
    INPUT_MODALITY_FLAG_TOUCH5,
    INPUT_MODALITY_FLAG_TOUCH6,
    INPUT_MODALITY_FLAG_TOUCH7,
    INPUT_MODALITY_FLAG_TOUCH8,
    INPUT_MODALITY_FLAG_TOUCH9,
    INPUT_MODALITY_FLAG_TOUCH10,
    INPUT_MODALITY_FLAG_TOUCH11,
    INPUT_MODALITY_FLAG_TOUCH12,
    INPUT_MODALITY_FLAG_TOUCH13,
    INPUT_MODALITY_FLAG_TOUCH14,
    INPUT_MODALITY_FLAG_TOUCH15,
    INPUT_MODALITY_FLAG_TOUCH16,
    INPUT_MODALITY_FLAG_TOUCH17,
    INPUT_MODALITY_FLAG_TOUCH18,
    INPUT_MODALITY_FLAG_TOUCH19,
    INPUT_MODALITY_FLAG_TOUCH20,
    INPUT_MODALITY_FLAG_PEN_HOVER,
    
    INPUT_MODALITY_FLAG_COUNT,
} INPUT_MODALITY_FLAG;


constexpr const uint64 INPUT_EVENT_BIT = 1;
typedef enum INPUT_EVENT_FLAG {
    INPUT_EVENT_FLAG_CREATED_NEW
} INPUT_EVENT_FLAG;

typedef enum Input_Handler_Return_STATUS {
    Input_Handler_Return_STATUS_CANCELED = 0,
    Input_Handler_Return_STATUS_OK = 1,
    Input_Handler_Return_STATUS_DO_RECURSE = 2,
} Input_Handler_Return_STATUS;

struct Input_Handler_Return {
    Input_Handler_Return_STATUS val = Input_Handler_Return_STATUS_OK;
    Input_Handler_Return(bool val) : val(val?Input_Handler_Return_STATUS_OK : Input_Handler_Return_STATUS_CANCELED) {}
    Input_Handler_Return(void) : val(Input_Handler_Return_STATUS_OK) {}
    Input_Handler_Return(Input_Handler_Return_STATUS val) : val(val) {}
    
    operator bool()
    {
        return val != Input_Handler_Return_STATUS_CANCELED;
    }
};

//typedef bool Input_Handler_Return;
struct Input_Handlers {
    
    struct State {
        vec3 prev_world_pos = vec3(0.0f);
        vec2 prev_canvas_pos = vec2(0.0f);
        vec3 first_world_pos = vec3(0.0f);
        vec2 first_canvas_pos = vec2(0.0f);
        
        //vec3 delta = vec3(0.0f);
        
        UI_TOUCH_STATE state = UI_TOUCH_STATE_NONE;
        usize active_count = 0;
    } state[INPUT_MODALITY_FLAG_COUNT];
    
    Input_Handler_Return (*on_touch_input_began)(Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) = nullptr;
    void (*on_touch_input_moved)(Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) = nullptr;
    void (*on_touch_input_ended)(Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) = nullptr;
    
    Input_Handler_Return (*on_pen_input_began)(Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) = nullptr;
    void (*on_pen_input_moved)(Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) = nullptr;
    void (*on_pen_input_ended)(Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) = nullptr;
    
    Input_Handler_Return (*on_hover_input_began)(Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) = nullptr;
    void (*on_hover_input_moved)(Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) = nullptr;
    void (*on_hover_input_ended)(Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) = nullptr;
    
    void* custom_data = nullptr;
    
    
    Input_Handler_Return do_on_touch_input_began(Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
    {
        auto* const state = &this->state[INPUT_MODALITY_FLAG_TOUCH];
        
        state->state = UI_TOUCH_STATE_BEGAN;
        //state->delta = vec3(0.0f);
        state->active_count += 1;
        state->first_world_pos = world_pos;
        state->first_canvas_pos = canvas_pos;
        
        Input_Handler_Return ret = {};
        if (this->on_touch_input_began != nullptr) {
            ret = on_touch_input_began(this, thing, world_pos, canvas_pos, event, touch, flags);
        }
        
        state->prev_world_pos = world_pos;
        state->prev_canvas_pos = canvas_pos;
        
        return ret;
    }
    
    void do_on_touch_input_moved(Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
    {
        auto* const state = &this->state[INPUT_MODALITY_FLAG_TOUCH];
        state->state = UI_TOUCH_STATE_MOVED;
        //state->delta += world_pos - state->prev_world_pos;
        
        
        if (this->on_touch_input_moved != nullptr) {
            on_touch_input_moved(this, thing, world_pos, canvas_pos, event, touch, flags);
        }
        
        
        state->prev_world_pos = world_pos;
        state->prev_canvas_pos = canvas_pos;
    }
    
    void do_on_touch_input_ended(Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
    {
        auto* const state = &this->state[INPUT_MODALITY_FLAG_TOUCH];
        state->state = UI_TOUCH_STATE_ENDED;
        //state->delta += world_pos - state->prev_world_pos;
        if (this->on_touch_input_ended != nullptr) {
            on_touch_input_ended(this, thing, world_pos, canvas_pos, event, touch, flags);
        }
        
        state->state = UI_TOUCH_STATE_NONE;
        
        state->active_count -= 1;
        
        state->prev_world_pos = world_pos;
        state->prev_canvas_pos = canvas_pos;
    }
    
    Input_Handler_Return do_on_pen_input_began(Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
    {
        auto* const state = &this->state[INPUT_MODALITY_FLAG_PEN];
        state->state = UI_TOUCH_STATE_BEGAN;
        //state->delta = vec3(0.0f);
        state->first_world_pos = world_pos;
        state->first_canvas_pos = canvas_pos;
        
        state->active_count += 1;
        
        Input_Handler_Return ret = {};
        if (this->on_pen_input_began != nullptr) {
            ret = on_pen_input_began(this, thing, world_pos, canvas_pos, event, touch, flags);
        }
        
        
        state->prev_world_pos = world_pos;
        state->prev_canvas_pos = canvas_pos;
        
        return ret;
    }
    
    void do_on_pen_input_moved(Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
    {
        auto* const state = &this->state[INPUT_MODALITY_FLAG_PEN];
        state->state = UI_TOUCH_STATE_MOVED;
        //state->delta += world_pos - state->prev_world_pos;
        if (this->on_pen_input_moved != nullptr) {
            on_pen_input_moved(this, thing, world_pos, canvas_pos, event, touch, flags);
        }
        
        
        state->prev_world_pos = world_pos;
        state->prev_canvas_pos = canvas_pos;
    }
    
    void do_on_pen_input_ended(Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
    {
        auto* const state = &this->state[INPUT_MODALITY_FLAG_PEN];
        state->state = UI_TOUCH_STATE_ENDED;
        //state->delta += world_pos - state->prev_world_pos;
        if (this->on_pen_input_ended != nullptr) {
            on_pen_input_ended(this, thing, world_pos, canvas_pos, event, touch, flags);
        }
        
        
        state->prev_world_pos = world_pos;
        state->prev_canvas_pos = canvas_pos;
        
        state->state = UI_TOUCH_STATE_NONE;
        
        state->active_count -= 1;
    }
    
    Input_Handler_Return do_on_hover_input_began(Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
    {
        auto* const state = &this->state[INPUT_MODALITY_FLAG_PEN_HOVER];
        state->state = UI_TOUCH_STATE_BEGAN;
        //state->delta = vec3(0.0f);
        state->first_world_pos = world_pos;
        state->first_canvas_pos = canvas_pos;
        
        state->active_count += 1;
        
        Input_Handler_Return ret = {};
        if (this->on_hover_input_began != nullptr) {
            ret = on_hover_input_began(this, thing, world_pos, canvas_pos, event, touch, flags);
        }
        
        
        state->prev_world_pos = world_pos;
        state->prev_canvas_pos = canvas_pos;
        
        return ret;
    }
    
    void do_on_hover_input_moved(Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
    {
        auto* const state = &this->state[INPUT_MODALITY_FLAG_PEN_HOVER];
        state->state = UI_TOUCH_STATE_MOVED;
        //state->delta += world_pos - state->prev_world_pos;
        if (this->on_hover_input_moved != nullptr) {
            on_hover_input_moved(this, thing, world_pos, canvas_pos, event, touch, flags);
        }
        
        
        state->prev_world_pos = world_pos;
        state->prev_canvas_pos = canvas_pos;
    }
    
    void do_on_hover_input_ended(Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
    {
        auto* const state = &this->state[INPUT_MODALITY_FLAG_PEN_HOVER];
        state->state = UI_TOUCH_STATE_ENDED;
        //state->delta += world_pos - state->prev_world_pos;
        if (this->on_hover_input_ended != nullptr) {
            on_hover_input_ended(this, thing, world_pos, canvas_pos, event, touch, flags);
        }
        
        
        state->prev_world_pos = world_pos;
        state->prev_canvas_pos = canvas_pos;
        
        state->state = UI_TOUCH_STATE_NONE;
        
        state->active_count -= 1;
    }
    
    
};

typedef enum THING_FLAG : uint64 {
    THING_FLAG_EMPTY = 0,
    THING_FLAG_is_resident = (1 << 0),
    THING_FLAG_is_active = (1 << 1),
    THING_FLAG_is_active_group = (1 << 2),
    THING_FLAG_is_root = (1 << 3),
    THING_FLAG_is_visible = (1 << 4),
    THING_FLAG_is_locked = (1 << 5),
    THING_FLAG_is_user_destructible = (1 << 6),
    THING_FLAG_is_user_drawable = (1 << 7),
    THING_FLAG_do_evaluation = (1 << 8),
    THING_FLAG_is_user_movable = (1 << 9),
    THING_FLAG_lock_user_movement_if_not_root = (1 << 10),
    THING_FLAG_lock_to_canvas = (1 << 11),
    THING_FLAG_forward_input_to_root = (1 << 12),
    THING_FLAG_is_actor = (1 << 13),
    THING_FLAG_should_defer_destruction = (1 << 14),
    THING_FLAG_is_user_erasable = (1 << 15),
    THING_FLAG_is_paused = (1 << 16),
} THING_FLAG;



struct Message;

static inline void message_handler_noop(Message* msg) {};

struct alignas(16) Thing_Archetype {
    mtt::String tag;
    Thing_Archetype_ID id;
    Thing_Archetype_ID parent;
    mtt::Set<Thing_Archetype_ID> children;
    mtt::Set<Thing_ID> instance_ids;
    
    usize eval_priority;
    
    bool is_actor;
    
    THING_FLAG flags;
    //Map<Attribute_ID, Behaviour_ID> attribute_to_behaviour;
    mtt::World* world_;
    
    mtt::World* world()
    {
        return mtt::ctx();
    }
    
    //Representation                         representation;
    Field_List_Descriptor field_descriptor;
    Port_Descriptor         port_descriptor;
    Logic logic;
    
    
    
    Field_List_Descriptor field_descriptor_for_template;
    Port_Descriptor port_descriptor_for_template;
    
    mtt::Map<MTT_String_Ref_ID, mtt::Procedure> selector_to_proc;
    
    void (*on_thing_make)(Thing* thing) = nullptr;
    void (*on_thing_destroy)(Thing* thing) = nullptr;
    void (*on_thing_copy)(Thing* thing_to, Thing* thing_from) = nullptr;
    
    Input_Handlers input_handlers;
    
    void (*message_handler)(Message* msg) = nullptr;
    
    bool ignore_selection;
};

#define MTT_FREEHAND_SKETCH_OPTION_MASK_DIRECT_POSITION(option) ((option & (1)) == 1)


struct Thing_Info {
    Thing_ID thing_id;
    World* world;
};

struct Thing_Is_Proxy {};


struct Thing_ID_Component {
    int id;
};

//struct Attribute {
//    Tag tag;
//    Attribute_ID id;
//};
//
//
//struct Attribute_Collection {
//    Map<Tag, Attribute_ID> tag_to_attribute;
//};

struct Attribute_Result {
    const void* attribute;
    bool  is_exact_match;
};

static inline void new_attr(World* mtt_world, const char *name, usize size, ecs_entity_t* ent);
static inline const void* get_attr(World* mtt_world, ecs_entity_t e, ecs_entity_t component_id);
static inline const Result<Attribute_Result> get_attr(World* mtt_world, ecs_entity_t e, const char *name);

struct alignas(16) Thing {
    Thing_ID            id;
    Thing_Archetype_ID  archetype_id;
    //Thing_Group_ID      group_id;
    Thing_ID            root_thing_id;
    Thing_ID            mapped_thing_id;
    
    Entity ecs_entity;
    
    Logic                   logic;
    Field_List_Descriptor   field_descriptor;
    Field_List              active_fields;
    
    Port_Descriptor         ports;
    uint64                  eval_index;
    usize                   eval_priority;
    
    Context_ID ctx_id = Context_ID_DEFAULT;
    Eval_Connection_Graph* graph    = nullptr;
    Evaluation_Output*     eval_out = nullptr;
    
    inline bool operator==(const Thing& other) const { return this->id == other.id; }
    
    
    struct alignas(16) {
        bool is_resident;
        bool is_root;
        bool is_visible;
        bool is_locked;
        bool is_visited;
        bool is_user_destructible;
        bool is_user_drawable;
        bool do_evaluation;
        bool is_user_movable;
        bool lock_user_movement_if_not_root;
        bool lock_to_canvas;
        bool forward_input_to_root;
        bool should_defer_destruction = false;
        bool is_static = false;
        bool is_reserved;
        THING_FLAG flags;
    };
    
    Representation representation;
    
    Input_Handlers input_handlers;
    
    void (*message_handler)(Message* msg);
    
    //World* world_;
    inline World* world()
    {
        return mtt::ctx();
    }
    
    Thing_ID next_id;
    Thing_ID prev_id;
    Thing_ID first_child;
    Thing_ID parent_thing_id;
    
    Thing_ID saved_parent_thing_id;
    
    
    Thing_Child_List child_id_set;
    
    
    
    
    //    bool is_resident;
    //    bool is_root;
    //    bool is_visible;
    //    bool is_locked;
    //    bool is_visited;
    //    bool is_user_destructible;
    //    bool is_user_drawable;
    //    bool do_evaluation;
    //    bool is_user_movable;
    //    bool lock_user_movement_if_not_root;
    //    bool lock_to_canvas;
    //    bool forward_input_to_root;
    //
    //    THING_FLAG flags;
    
    
    //    Port_List               in_port_list;
    //    Port_List               out_port_list;
    
    void (*on_destroy)(Thing* thing);
    
    
    
    usize selection_count;
    
    void (*on_run_init)(Thing* thing) = nullptr;
    
    
    
    
    MTT_String_Ref label;
};

inline static Thing_ID thing_id(mtt::Thing* thing)
{
    return thing->id;
}
inline static Thing_ID thing_type_id(mtt::Thing* thing)
{
    return thing->archetype_id;
}

static inline void* field_contents(mtt::Thing* thing)
{
    // //   thing->field_descriptor.data.contents
    return thing->active_fields.contents;
}

Field_List& default_field_list(mtt::Thing* thing);
void set_active_fields_to_default(mtt::Thing* thing);
void set_active_fields(mtt::Thing* thing, Field_List* field_list);


using Thing_Storage = Map_Stable<mtt::Thing_ID, mtt::Thing>;

struct Connection_Compare_eval_index {
    Thing_Storage& things;
    
    inline bool operator() (const Connection& a, const Connection& b)
    {
        return things.find(b.dst_thingref)->second.eval_index > things.find(a.dst_thingref)->second.eval_index;
    }
};

struct Connection_Compare_port_index {
    inline bool operator() (const Connection& a, const Connection& b)
    {
        return b.header.dst_port_ref > a.header.dst_port_ref;
    }
};


struct Connection_Compare_src_id {
    inline bool operator() (const Connection& a, const Connection& b)
    {
        return b.src_thingref > a.src_thingref;
        
    }
};
struct Connection_Compare_dst_id {
    inline bool operator() (const Connection& a, const Connection& b)
    {
        return b.dst_thingref > a.dst_thingref;
    }
};

struct Connection_Find_By_Dst_Procedure {
    inline bool operator() (const Connection& connection, Thing_ID target_dst)
    {
        return target_dst == connection.dst_thingref;
    }
};

struct Thing_compare_by_eval_priority {
    inline bool operator() (const Thing* a, const Thing* b)
    {
        return a->eval_priority < b->eval_priority;
    }
};


struct Sensor {
    Thing_ID thing_id;
};


struct Thing_Group {
    Thing_Group_ID parent_group_id;
    Thing_Group_ID group_id;
    robin_hood::unordered_flat_set<Thing_Group_ID> child_group_id_set;
    
    robin_hood::unordered_flat_set<Thing_ID> things;
};

constexpr const usize THING_PAGE_SIZE = 0xFFF + 1;
constexpr const uint64 THING_LOOKUP_OFFSET_MASK = THING_PAGE_SIZE - 1;

typedef enum ARROW_LINK_FLAGS {
    ARROW_LINK_FLAGS_DIRECTED = 1 << 1,
    ARROW_LINK_FLAGS_VISIBLE = 1 << 0,
    ARROW_LINK_FLAGS_DEFAULT = ARROW_LINK_FLAGS_VISIBLE,
} ARROW_LINK_FLAGS;
struct Arrow_Link {
    mtt::Thing_ID id = mtt::Thing_ID_INVALID;
    mtt::String label = "";
    ARROW_LINK_FLAGS flags = ARROW_LINK_FLAGS_DEFAULT;
};
inline static mtt::Thing_ID arrow_get_thing_id(Arrow_Link& arrow)
{
    return arrow.id;
}
inline static void arrow_set_label(Arrow_Link& arrow, mtt::String& str)
{
    arrow.label = str;
}
inline static void arrow_set_label(Arrow_Link& arrow, cstring str)
{
    arrow.label = str;
}
inline static const mtt::String& arrow_get_label(Arrow_Link& arrow)
{
    return arrow.label;
}

void arrows_copy(mtt::Thing* thing);


using Arrow_Link_List = std::vector<Arrow_Link>;

struct Arrow_Links {
    mtt::Map_Stable<mtt::Thing_ID, mtt::Arrow_Link_List> edges_forward;
    mtt::Map_Stable<mtt::Thing_ID, mtt::Arrow_Link_List> edges_reverse;
    
    bool is_visible_flag = true;
    void toggle_visibility(void)
    {
        this->is_visible_flag = !this->is_visible_flag;
    }
    
    bool is_visible(void)
    {
        return this->is_visible_flag;
    }
    
    bool empty(void)
    {
        return edges_forward.empty();
    }
};

void arrows_reverse(mtt::Thing* root_thing);
typedef std::unordered_set<mtt::Thing_ID> Thing_Proxy_Storage;
struct Thing_Make_Proxy_Args {
    int64 renderer_layer_id = -1;
    mtt::Collision_System* collision_system = nullptr;
    usize scene_idx = 0;
};

struct Thing_Proxy_Scene {
    mtt::Map_Stable<mtt::Thing_ID, Thing_Proxy_Storage> thing_to_proxy_map = {};
    // thing
    std::vector<
        // per drawable
        std::vector<
            // per proxy
            std::vector<sd::Drawable_Info>
        >
    >
    proxy_aggregate = {};
    //mtt::Set<Thing_Proxy_ID> proxies = {};
    usize id = 0;
    
    
    void (*on_clear)(Thing_Proxy_Scene* scene) = [](Thing_Proxy_Scene*) {};
};

struct Thing_Collection {
    mtt::Dynamic_Array<mtt::Array<mtt::Thing, THING_PAGE_SIZE>> things;
    
    usize count;
    
    
    Thing_Storage instances;
    
    mtt::Map<mtt::Thing_ID, Script_Lookup> properties;
    
    mtt::Map<mtt::Thing_ID, Active_Scripts> active_scripts = {};
    
    Thing_Storage instances_unstaged;
    
    mtt::Map_Stable<mtt::Thing_ID, Thing_Proxy_Storage> thing_to_proxy_map = {};
    //mtt::Dynamic_Array<Thing_Proxy_Scene> proxy_scenes = {};
    std::vector<Thing_Proxy_Scene> proxy_scenes = {};
    mtt::Map_Stable<Thing_Proxy_ID, usize> proxy_id_to_scene_id = {};
    
    
    //Map<Thing_Group_ID, Thing_Group>         groups;
    //    Map<Thing_ID, Map<Thing_ID, THING_FLAG>> thing_to_run_ctx_specific_flags;
    //Map<Thing_ID, Behaviour>             behaviour;
    //Map<Thing_ID, Logic>                     logic;
    
    Thing_ID next_thing_id;
    Thing_Group_ID next_thing_group_id;
    
    
    mtt::Map<mtt::Thing_ID, mtt::Map<MTT_String_Ref_ID, mtt::Procedure>> own_selector_to_proc;
    
    Arrow_Links arrows;
    
    mtt::Map_Stable<mtt::Thing_ID, UI_Key> marked_for_moving = {};
    mtt::Set<mtt::Thing_ID> to_unmark_for_moving = {};
};

void Thing_Proxy_Scene_make_count(mtt::World* world, usize count);
usize Thing_Proxy_Scene_count(mtt::World* world);
Thing_Proxy_Scene* Thing_Proxy_Scene_for_idx(mtt::World* world, usize idx);
Thing_Proxy_Scene* Thing_Proxy_Scene_for_Thing_Proxy_ID(mtt::World* world, Thing_Proxy_ID id);
Thing_Proxy_Scene* Thing_Proxy_Scene_for_Thing_Proxy(mtt::World* world, Thing* thing_proxy);
void Thing_Proxy_Scene_prepare_for_render(Thing_Proxy_Scene* scene);
void Thing_Proxy_Scene_remove_proxy(World* world, Thing* thing_proxy);

bool Thing_marked_for_moving(mtt::Thing* thing, UI_Key key);
bool Thing_unmark_for_moving(mtt::Thing* thing, UI_Key key);







struct Thing_Archetype_Collection {
    Map<Thing_Archetype_ID, Thing_Archetype>  instances;
    Thing_Archetype_ID next_thing_archetype_id;
};

static constexpr const uint64 MESSAGE_TYPE_FROM_TAG    = 0;
static constexpr const uint64 MESSAGE_TYPE_FROM_ENTITY = 1;

struct Message {
    uint64   type = MTT_NONE;
    uint64   flags = 0;
    Thing_ID sender = mtt::Thing_ID_INVALID;
    
    usize    length = 0;
    void*    contents = nullptr;
    mtt::Any input_value;
    
    MTT_String_Ref selector;
    
    mtt::Procedure proc;
    
    mtt::Any output_value;
    
    Message* next;
};

inline static Message* Message_from(Procedure_Input_Output* io)
{
    return static_cast<Message*>(io->input);
}
inline static void* Message_contents(Message* msg)
{
    return msg->contents;
}

struct Broadcast {
    uint64 type;
    uint64 flags;
    Thing_ID sender;
    
    mtt::String selector;
    mtt::String event;
    
    usize length;
    void* contents;
    mtt::Any selection_contents;
    
    mtt::Procedure proc;
};

struct Messages_Record {
    Thing_ID recipient;
    std::deque<Message> from_tags;
    std::deque<Message> from_entities;
};

struct Message_Passer {
    mtt::Map<Thing_ID, Messages_Record> message_map;
    std::deque<Message> system_messages;
    std::deque<Message> system_messages_deferred;
    std::deque<Message> system_messages_deferred_before_scripts;
    std::deque<Message> system_messages_deferred_after_scripts;
    std::deque<Broadcast> broadcasts;
    
    mtt::World* world;
};

void Message_Passer_init(Message_Passer* message_passer, mem::Allocator* allocator);

Messages_Record* messages(Message_Passer* message_passer, Thing* thing);
Messages_Record* messages(Message_Passer* message_passer, Thing_ID thing);

void send_message_to_sensor_from_tag(Message_Passer* message_passer, Thing_ID from_tag,    Thing_ID to, void* with_contents);

void send_message_to_sensor_from_entity(Message_Passer* message_passer, Thing_ID from_entity, Thing_ID to, void* with_contents);

void send_message_to_sensor_from_tag(Message_Passer* message_passer, Thing_ID from_tag, void* with_contents, Messages_Record* record);
void send_message_to_sensor_from_entity(Message_Passer* message_passer, Thing_ID from_entity, void* with_contents, Messages_Record* record);

//void send_message(Message_Passer* message_passer, Thing_ID from_entity, to_entity) {
//
//}

void send_system_message(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents);

void send_system_message(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents, mtt::Procedure with_callback);

void send_system_message_deferred(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents);

void send_system_message_deferred(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents, mtt::Procedure with_callback);

void send_system_message_deferred_before_script_eval(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents, mtt::Procedure with_callback);

void send_system_message_deferred_after_script_eval(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents, mtt::Procedure with_callback);

void send_message_to_entity(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, Thing_ID to_entity, void* with_contents);
void send_message_to_entity(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, Thing_ID to_entity, void* with_contents, mtt::Procedure with_callback);


typedef void* Thing_To_Make_Info;

static const uint32 DESTROY_IS_ACTIVE_MASK   = 0x00000000FFFFFFFF;
static const uint64 DESTROY_CONNECTIONS_MASK = 0xFFFFFFFF00000000;





struct Drawable_Instance_Info {
    sd::Drawable_Info* source;
    mem::Allocator allocator;
    //    std::vector<mtt::Array<sd::Drawable_Info, 256>> instances;
    //    //std::vector<uint64> id_to_instance;
    //    std::vector<uint64> free_id_list;
    mem::Pool_Allocation list_pool_alloc;
    mem::Pool_Allocation drawable_pool_alloc;
    MTT_List list;
    mtt::Dynamic_Array<sd::Drawable_Info*> array;
    
    
    
    void init(sd::Drawable_Info* drawable, mem::Allocator allocator);
    void deinit(void);
    
    
    sd::Drawable_Info* create_instance();
    void destroy_instance(sd::Drawable_Info* drawable);
};
struct Thing_Archetype_Drawable_Instances {
    mtt::Map<mtt::Thing_Archetype_ID, Drawable_Instance_Info> arch_to_drawable;
    // backing allocator
    mem::Allocator allocator;
    
    void init(mem::Allocator allocator);
    
    void register_drawable(mtt::Thing_Archetype_ID arch_id, sd::Drawable_Info* drawable)
    {
        auto it_found = arch_to_drawable.find(arch_id);
        if (it_found != arch_to_drawable.end()) {
            MTT_error("%s", "Already registered this archetype\n");
            return;
        }
        
        arch_to_drawable.emplace(arch_id, Drawable_Instance_Info());
        
        auto it_ = this->arch_to_drawable.find(arch_id);
        assert(it_ != this->arch_to_drawable.end());
        auto* info = &it_->second;
        info->init(drawable, this->allocator);
        
    }
    void deregister_drawable(mtt::Thing_Archetype_ID arch_id)
    {
        auto it_found = arch_to_drawable.find(arch_id);
        if (it_found == arch_to_drawable.end()) {
            MTT_error("%s", "This archetype has not been registered\n");
            return;
        }
        it_found->second.deinit();
        arch_to_drawable.erase(it_found);
    }
    
    mtt::Map<mtt::Thing_Archetype_ID, Drawable_Instance_Info>::iterator begin()
    {
        return this->arch_to_drawable.begin();
    }
    mtt::Map<mtt::Thing_Archetype_ID, Drawable_Instance_Info>::iterator end()
    {
        return this->arch_to_drawable.end();
    }
};

void eval_rules(mtt::World* world);

const usize INTERACTION_TRIGGER_TYPE_ACTION             = 0;
const usize INTERACTION_TRIGGER_TYPE_CONDITIONAL_ACTION = 1;

const usize INTERACTION_TRIGGER_TYPE_SEQUENTIAL_ACTION  = 2;


struct Action_Interaction {
    u8 tmp;
};
struct Conditional_Action_Interaction {
    u8 tmp;
};
struct Sequential_Action_Interaction {
    u8 tmp;
    Sequential_Action_Interaction* next;
};
struct Interaction_Trigger {
    mtt::String key;
    
    usize type;
    
    Action_Interaction             action;      // e,g, explode
    Conditional_Action_Interaction cond_action; // e.g. eval only if true
    Sequential_Action_Interaction  seq_action;  // replace this interaction trigger with the next one
    
    mtt::String first_tag;
    mtt::String second_tag;  
};





struct Thing_Metadata {
    mtt::String label;
    void* data;
};

struct Instancing {
    mem::Pool_Allocation drawable_pool;
    mtt::Map<sd::Drawable_Info*, MTT_List> lists;
    
    void push(sd::Drawable_Info* src, sd::Drawable_Info* inst)
    {
        auto find_it = lists.find(src);
        if (find_it == lists.end()) {
            MTT_List& l = lists[src];
            MTT_List_append(&l, inst);
        } else {
            MTT_List& l = find_it->second;
            MTT_List_append(&l, inst);
        }
    }
    void clear()
    {
        for (auto it = lists.begin(); it != lists.end(); ++it) {
            MTT_List_destroy(&it->second);
        }
        lists.clear();
    }
    
    void push_draw_commands(sd::Renderer* renderer)
    {
        for (auto it = lists.begin(); it != lists.end(); ++it) {
            MTT_List* l = &it->second;
            if (MTT_List_is_empty(l)) {
                continue;
            }
            
            sd::push_instanced_draw_command_with_drawable_list(renderer, it->first, l);
        }
    }
    
};



static const Priority_Layer PRIORITY_LAYER_DEFAULT = 0;
static inline Priority_Layer Priority_Layer_default(void)
{
    return PRIORITY_LAYER_DEFAULT;
}

typedef enum EXTERNAL_THING_FLAG {
    EXTERNAL_THING_FLAG_UPDATE_PUSH = (1 << 0),
    EXTERNAL_THING_FLAG_UPDATE_PULL = (1 << 0),
    EXTERNAL_THINGFLAG_UPDATE_PUSH_PULL = EXTERNAL_THING_FLAG_UPDATE_PUSH | EXTERNAL_THING_FLAG_UPDATE_PULL,
} EXTERNAL_THING_FLAG;
struct External_Thing {
    mtt::Thing_ID id = mtt::Thing_ID_INVALID;
    // push or pull updates
    EXTERNAL_THING_FLAG flags = EXTERNAL_THING_FLAG_UPDATE_PUSH;
};

struct External_World {
    mtt::Thing_ID id = 1;
    mtt::Map<mtt::Thing_ID, External_Thing> mappings;
};

External_World* curr_external_world(mtt::World* world);
void set_curr_external_world_id(mtt::World* world, usize id);
External_Thing* External_Thing_map(mtt::External_World* world, mtt::Thing* thing);
void External_World_reset(mtt::External_World* world);
External_Mappings* External_World_mappings(mtt::External_World* ext_world);
bool External_Thing_flags_are_set(mtt::External_Thing* thing, mtt::EXTERNAL_THING_FLAG flags);

struct To_Clear {
    mtt::Thing_ID thing_id = mtt::Thing_ID_INVALID;
    dt::Word_Dictionary_Entry* entry = nullptr;
};

constexpr const usize ALLOCATOR_TEMPORARY_COUNT = 1;
struct World {
    usize step_count;
    Thing_Archetype_Collection archetypes;
    Thing_Collection           things;
    
    //mtt::Map<mtt::String, Attribute> attributes;
    
    Eval_Connection_Graph root_graph;
    Eval_Connection_Graph* current_graph = &root_graph;
    Eval_Connection_Graph* saved_graph = &root_graph;
    Runtime runtime;
    
    Message_Passer message_passer;
    
    mem::Allocator allocator;
    //mem::Allocator cpp_allocator;
    
    mem::Memory_Pool_Fixed memory_pool;
    mem::Allocator pool_allocator;
    
    mem::Pool_Allocation drawable_pool = {};
    
    mem::Buckets_Allocation buckets;
    mem::Buckets_Allocation message_allocation;
    mem::Buckets_Allocation arg_allocation;
    
    bool no_deletion_zone_on = false;
    

    MTT_String_Pool string_pool;
    
    float64 time_seconds = 0.0;
    float64 timestep = 0.0f;
    float64 time_seconds_prev = 0.0;
    uint64 time_ns = 0;
    uint64 time_ns_prev = 0;
    uint64 timestep_ns = 0;
    usize eval_count = 0;
    
    std::deque<Thing_To_Make_Info> to_make;
    std::deque<Destroy_Command>    to_destroy;
    std::deque<mtt::Thing_ID>      to_destroy_end;
    
    std::vector<To_Clear> to_clear;
    
    std::deque<Thing_ID> to_disable;
    std::deque<Thing_ID> to_enable;
    
    flecs::world ecs_world;
    
    flecs::entity IS_ATTRIBUTE_TAG;
    
    External_World ext_worlds[2];
    usize curr_ext_world_id = 1;
    
    Physics_World physics_world;
    
    void (*on_thing_make)(mtt::World* world, mtt::Thing* thing);
    
    flecs::query<mtt::Thing_Info, mtt::Sensor> sensor_query;
    
    sd::Renderer* renderer;
    
    std::vector<mtt::Trigger_Response_Command> rules;
    
    void* user_data;
    
    Priority_Layer priority_layer = PRIORITY_LAYER_DEFAULT;
    
    std::vector<Priority_Layer> priority_layer_stack = {};
    
    
    mtt::Collision_System collision_system;
    mtt::Collision_System collision_system_canvas;
    
    mtt::Collision_System_Group_World_Canvas collision_system_group = {
        .world = &collision_system,
        .canvas = &collision_system_canvas
    };
    
    Instancing instancing;
    
    inline bool Thing_try_get(Thing_ID id, Thing** thing)
    {
        return mtt::Thing_try_get(this, id, thing);
    }
    
    inline Thing* Thing_try_get(Thing_ID id)
    {
        return mtt::Thing_try_get(this, id);
    }
    
    inline Thing* Thing_get(Thing_ID id)
    {
        return mtt::Thing_get(this, id);
    }
    
    inline void Thing_get(Thing_ID id, Thing** thing)
    {
        mtt::Thing_get(this, id, thing);
    }
    
    inline bool Thing_Archetype_try_get(Thing_Archetype_ID id, Thing_Archetype** arch)
    {
        return mtt::Thing_Archetype_try_get(this, id, arch);
    }
    
    inline void Thing_Archetype_get(Thing_Archetype_ID id, Thing_Archetype** arch)
    {
        mtt::Thing_Archetype_get(this, id, arch);
    }
    
    std::deque<mtt::Thing_ID> traversal_queue;
    std::vector<Thing*> thing_buffer;
    std::vector<Thing*> to_enable_list;
    std::vector<Thing*> to_disable_list;
    
    // need per-frame transient memory
    
    // per simulation step
    mem::Allocator allocator_temporary_[ALLOCATOR_TEMPORARY_COUNT];
    mem::Arena memory_temporary_[ALLOCATOR_TEMPORARY_COUNT];
    // only reset the arena after a few frames
    usize per_frame_reset_counter_ = 0;
//    inline mem::Allocator& allocator_temporary(void) { return allocator_temporary_[per_frame_reset_counter_]; }
//    inline void allocator_temporary_begin_frame(void) {
//        per_frame_reset_counter_ = (per_frame_reset_counter_ + 1) % ALLOCATOR_TEMPORARY_COUNT;
//        mem::Arena_rewind(&memory_temporary_[per_frame_reset_counter_]);
//    }
        inline mem::Allocator& allocator_temporary(void) { return allocator_temporary_[0]; }
    inline void allocator_temporary_begin_frame(void) {
        
        if (per_frame_reset_counter_ >= 32) {
            per_frame_reset_counter_ = 0;
            mem::Arena_rewind(&memory_temporary_[0]);
        } else {
            per_frame_reset_counter_ = (per_frame_reset_counter_ + 1);
        }
    }
    
    mtt::Map<mtt::Thing_ID, Input_Triggers> input_triggers;
    
    Thing_Child_List saved_children;
    
    Thing_Archetype_Drawable_Instances archetype_drawables;
    
    dt::Word_Dictionary_Entry* collide;
    
    flecs::entity collide_tag;
    flecs::entity collide_begin_tag;
    flecs::entity collide_end_tag;
    
    dt::Word_Dictionary_Entry* select;
    
    flecs::entity select_tag;
    flecs::entity select_begin_tag;
    flecs::entity select_end_tag;
    
    dt::Word_Dictionary_Entry* overlap;
    
    flecs::entity overlap_tag;
    flecs::entity overlap_begin_tag;
    flecs::entity overlap_end_tag;
//    dt::Word_Dictionary_Entry* exit;
    
    
    
    dt::Word_Dictionary_Entry* equivalent_to;
    std::vector<Collision_Record> collisions_to_remove;
    usize collisions_to_remove_count;
    
    //flecs::entity IS_ATTRIBUTE_TAG;
    
    mtt::Map<mtt::String, mtt::Interaction_Trigger> interactions;
    
    
    
    os_log_t log;
    
    mtt::Map_Stable<mtt::Thing_ID, Thing_Metadata> id_to_metadata;
    

    my::Function<void(mtt::Thing* thing), 1024> custom_on_thing_make = [](mtt::Thing* thing) {};
    
    
    void clear_all_of_type(mtt::ARCHETYPE arch);
    void clear_all_of_type_ignore_flags(mtt::ARCHETYPE arch);
    
    mem::Pool_Allocation field_list_pool;
    
    
    
    std::vector<bool(*)(mtt::World* world, float32 fixed_dt, float32 time_prev, float32 time, float32 realtime_dt, MTT_Core* core, void* ctx)> deferred_per_frame;
    
    bool show_verbose = true;
    bool show_attachment_links = false;
    bool show_debug   = false;
    bool show_script_eval_print = false;
    
    mtt::Map_Stable<mtt::String, mtt::Set<mtt::Thing_ID>> thing_saved_presets = {};
    mtt::Map_Stable<mtt::Thing_ID, bool> thing_saved_presents_rendering_states = {};
    
    mtt::Map<mtt::Thing_ID, mtt::Set<mtt::Thing_ID>> thing_refs = {};
};

External_World* curr_external_world(mtt::World* world);

void set_graph(mtt::World* world, Eval_Connection_Graph* G);
void set_graph_to_root(mtt::World* world);
Eval_Connection_Graph* curr_graph(mtt::World* world);
Eval_Connection_Graph* root_graph(mtt::World* world);
bool current_is_root_graph(mtt::World* world);
bool is_root_graph(mtt::World* world, Eval_Connection_Graph* G);
void save_current_graph(mtt::World* world);
void restore_current_graph(mtt::World* world);

void thing_refer_to(mtt::Thing* thing, mtt::Thing* tgt);
void thing_refer_to(mtt::Thing* thing, mtt::Thing_ID tgt);
void thing_remove_ref_to(mtt::Thing* thing, mtt::Thing_ID tgt);
void thing_remove_referrents(mtt::Thing* thing);
mtt::Set<mtt::Thing_ID>* thing_referrents(mtt::Thing* thing);
mtt::Set<mtt::Thing_ID>* thing_referrents(mtt::World* world, mtt::Thing_ID thing_id);


void run_deferred_per_frame(mtt::World* world, float32 fixed_dt, float32 time_prev, float32 time, MTT_Core* core, void*);
void add_deferred_per_frame(bool (*op)(mtt::World* world, float32 fixed_dt, float32 time_prev, float32 time, float32 realtime_dt, MTT_Core* core, void* ctx));

inline cstring str(MTT_String_Ref ref)
{
    return MTT_string_ref_to_cstring(ref);
}




void Rule_destroy(mtt::World* world, uint64 id);

inline static mtt::Representation* rep(mtt::Thing* thing, mtt::Representation** out)
{
    *out = &thing->representation;
    return &thing->representation;
}
inline static mtt::Representation* rep(mtt::Thing* thing)
{
    return &thing->representation;
}

inline static mtt::Representation* rep(mtt::World* world, mtt::Thing_ID id)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, id);

    return (thing != nullptr) ? rep(thing) : nullptr;
}

inline static mtt::Representation* rep(mtt::Thing_ID id)
{
    return rep(mtt::ctx(), id);
}

void flip_left_right(mtt::Thing* thing);

void flip_init_direction(mtt::Thing* thing);

float32 heading_direction_x(mtt::Rep* rep);
float32 heading_direction_x(mtt::Thing* thing);


const char* message_string(mtt::World* world, cstring str);

const char* message_string(mtt::World* world, mtt::String& str);

bool Thing_get(usize i, Thing* container, Thing** dst);

MTT_NODISCARD
mtt::Thing* Thing_make(World* world, Thing_Archetype_ID arch_id, const Thing_Make_Args& = {});


Thing_ID Thing_make(World* world, Thing_Archetype_ID arch_id, Thing** out, const Thing_Make_Args& = {});

void Thing_reinit(Thing* thing);

void Thing_set_label(Thing* thing, const mtt::String& label);
cstring Thing_cstring_from_label(Thing* thing);
mtt::String Thing_label(Thing* thing);
cstring search_id(Thing* thing);
void Thing_set_locked(Thing* thing, bool lock_state);
bool Thing_is_locked(Thing* thing);

void Thing_print(World* world, Thing* thing);
void Thing_print(Thing* thing);
void Thing_print(World* world, Thing_ID id);

void Thing_print_verbose(World* world, Thing* id);
void Thing_print_verbose(Thing* thing);
void Thing_print_verbose(World* world, Thing_ID id);

bool Thing_Archetype_make(World* world, Thing_Archetype** out, String tag);

mtt::Thing* Thing_make_with_collider(World* world, mtt::Rep** rep, mtt::ARCHETYPE arch, mtt::COLLIDER_TYPE collider_type, bool is_in_world = true, mtt::Collision_System* collision_system = nullptr);

struct World_Descriptor {
    sd::Renderer*   renderer;
    mem::Allocator* allocator;
};

Thing* Thing_copy(Thing* thing);
Thing* Thing_copy(World* world, Thing_ID ID);

Thing* Thing_copy_recursively(Thing* thing);
Thing* Thing_copy_recursively(World* world, Thing_ID ID);

Thing* Thing_make_proxy(Thing* thing_src, const Thing_Make_Proxy_Args& args);
Thing* Thing_make_proxy(World* world, Thing_ID ID_src, const Thing_Make_Proxy_Args& args);


World World_make(void);
void World_init(World* world, const World_Descriptor& desc);
void World_deinit(mtt::World* world);

Thing* Thing_from_ID(World* world, Thing_ID id);
Thing* Thing_from_tag(World* world, String tag);

Thing_Archetype* Thing_Archetype_from_ID(World* world, Thing_Archetype_ID id);

void Thing_destroy(World* world, Thing_ID id);
void Thing_destroy(mtt::Thing* thing);
void Thing_destroy(World* world, mtt::Thing* thing);

void render(World* world, sd::Renderer* renderer);

void Thing_Archetype_init_builtin_freehand_sketch(World* world, Thing_Archetype* arch);

void Thing_print_all(World* world);

void Thing_add_in_port(PORT_PARAM_LIST);
void Thing_add_in_port(mtt::Thing* thing, PORT_PARAM_LIST);

void Thing_add_out_port(PORT_PARAM_LIST);
void Thing_add_out_port(mtt::Thing* thing, PORT_PARAM_LIST);

void Thing_remove_in_port(mtt::Thing* thing, const mtt::String& name);
void Thing_remove_out_port(mtt::Thing* thing, const mtt::String& name);



void add_field(Field_Create_Info info, void* value = nullptr, FIELD_FLAG flags = MTT_FIELD_FLAG_NONE);
void add_field(Field_Create_Info info, Field_List_Builder& builder, void* value = nullptr, FIELD_FLAG flags = MTT_FIELD_FLAG_NONE);
void add_field(Field_Create_Info info, Field_List_Builder& builder, const mtt::String& type_name, usize byte_size, void (*on_make)(void* data), void (*on_copy)(void* data_dst, void* data_src), void (*on_destroy)(void* data), FIELD_FLAG flags = MTT_FIELD_FLAG_NONE);

template <typename T>
void add_field(Field_Create_Info info, Field_List_Builder& builder, const mtt::String& type_name, void (*on_make)(void* data), void (*on_copy)(void* data_dst, void* data_src), void (*on_destroy)(void* data), FIELD_FLAG flags = MTT_FIELD_FLAG_NONE)
{
    add_field(info, builder, type_name, align_up(sizeof(T), 16), on_make, on_copy, on_destroy, flags);
}

template <typename T>
void add_field(Field_Create_Info info, Field_List_Builder& builder, const mtt::String& type_name, FIELD_FLAG flags =  MTT_FIELD_FLAG_NONE)
{
    add_field(info, builder, type_name, align_up(sizeof(T), 16),
    [](void* data) {
        new (data) T();
    },
    [](void* data_dst, void* data_src) {
        new (data_dst) T(*((T*)data_src));
    },
    [](void* data) {
        reinterpret_cast<T*>(data)->~T();
    },
    flags);
}






void copy_field_list(mtt::World* world, mem::Allocator* allocator, Field_List_Descriptor* dst, Field_List_Descriptor* src);
void init_field_list_from_source(mtt::World* world, mem::Allocator* allocator, Field_List* data, Field_List_Descriptor* src);

void destroy_fields(mem::Allocator& allocator, Field_List_Descriptor& field_descriptor, Field_List& data);
void clear_field_contents(Field_List_Descriptor& field_descriptor, Field_List& data);

Thing_Storage* Thing_collection(World* world);




template <bool validate = true>
Field_Handle lookup(Thing* thing, const String& tag)
{
    if constexpr (validate) {
        auto field_idx = thing->field_descriptor.name_to_idx->find(tag);
        if (field_idx == thing->field_descriptor.name_to_idx->end()) {
            return {.is_valid = false };
        }
        
        return {
            .index = field_idx->second,
            .is_valid = true,
            .field_desc = &thing->field_descriptor
        };
    } else {
        return {
            .index = (*thing->field_descriptor.name_to_idx)[tag],
            .is_valid = true,
            .field_desc = &thing->field_descriptor
        };
    }
}

template <bool validate = true>
Field_Handle lookup(World* world, Thing_ID id, const String& tag)
{
    if constexpr (!validate) {
        Thing* thing = world->Thing_get(id);
        return lookup<validate>(thing, tag);
    } else {
        Thing* thing = world->Thing_try_get(id);
        if (thing == nullptr) {
            Field_Handle handle;
            handle.is_valid = false;
            return handle;
        } else {
            return lookup<validate>(thing, tag);
        }
            
    }
}


void* access(World* world, Thing_ID id, Field_Handle handle);

void* access(World* world, Thing_ID id, Field_Handle handle, MTT_TYPE type);

void* access(Thing* thing, Field_Handle handle);

void* access(Thing* thing, Field_Handle handle, MTT_TYPE type);

void* access(Thing* thing, const String& tag, MTT_TYPE type);

void* access(Thing* thing, usize index);

template <typename T>
T* access(Thing* thing, const String& tag)
{
    return (T*)mtt::access(thing, tag, MTT_CUSTOM);
}

template <typename T, typename Proc>
void access(Thing* thing, const String& tag, Proc&& proc)
{
    proc(mtt::access<T>(thing, tag));
}

template <typename T, typename Proc>
bool access_if_exists(Thing* thing, const String& tag, Proc&& proc)
{
    T* field = mtt::access<T>(thing, tag);
    if (field == nullptr) {
        return false;
    }
    
    proc(field);
    
    return true;
}

template <typename T, typename Proc>
T* access_and_modify(Thing* thing, const mtt::String& tag, Proc&& proc, Field_Event* ev_out);
template <typename T, typename Proc>
T* access_and_modify(Thing* thing, const mtt::String& tag, Proc&& proc, Field_Event* ev_out);

template <typename T, typename Proc>
const T* access_write(Thing* thing, const mtt::String& tag, Proc&& proc)
{
    auto handle = lookup<false>(thing, tag);
    auto* info = &thing->field_descriptor.fields[handle.index];
    T* data = ((T*)info->data_from(field_contents(thing)));

    proc(thing, data);
    
    return data;
}


template <typename T, typename Proc>
T* access_readonly(Thing* thing, const mtt::String& tag, Proc&& proc);

template <>
float32* access<float32>(Thing* thing, const String& tag);

template <>
vec2* access<vec2>(Thing* thing, const String& tag);

template <>
vec3* access<vec3>(Thing* thing, const String& tag);

template <>
vec4* access<vec4>(Thing* thing, const String& tag);

template <>
color::rgba* access<color::rgba>(Thing* thing, const String& tag);

template <>
bool* access<bool>(Thing* thing, const String& tag);

template <>
mtt::String* access<mtt::String>(Thing* thing, const String& tag);
template <>
MTT_String_Ref_Modify* access<MTT_String_Ref_Modify>(Thing* thing, const String& tag);
template <>
MTT_String_Ref* access<MTT_String_Ref>(Thing* thing, const String& tag);
template <>
const char* access<const char>(Thing* thing, const String& tag);

template <>
uint8* access<uint8>(Thing* thing, const String& tag);

template <>
int8* access<int8>(Thing* thing, const String& tag);

template <>
uint16* access<uint16>(Thing* thing, const String& tag);

template <>
int16* access<int16>(Thing* thing, const String& tag);

template <>
uint32* access<uint32>(Thing* thing, const String& tag);

template <>
int32* access<int32>(Thing* thing, const String& tag);

template <>
uint64* access<uint64>(Thing* thing, const String& tag);

template <>
int64* access<int64>(Thing* thing, const String& tag);

template <>
Any* access<Any>(Thing* thing, const String& tag);

template <>
mtt::Procedure* access<mtt::Procedure>(Thing* thing, const String& tag);


template <>
void** access<void*>(Thing* thing, const String& tag);



template <typename T>
T* access_pointer(Thing* thing, const String& tag);

template <typename T>
T* access_pointer_to_pointer(Thing* thing, const String& tag);

template <typename T>
T* access_pointer(Thing* thing, const String& tag)
{
    T* ptr = (T*)mtt::access(thing, tag, MTT_POINTER);
    if (ptr == nullptr) {
        return nullptr;
    }
    return ptr;
}

template <typename T>
T* access_custom(Thing* thing, const String& tag)
{
    T* ptr = (T*)mtt::access(thing, tag, MTT_CUSTOM);
    if (ptr == nullptr) {
        return nullptr;
    }
    return ptr;
}

template <typename T>
T* access_pointer_to_pointer(Thing* thing, const String& tag);

template <typename T>
T* access_pointer_to_pointer(Thing* thing, const String& tag)
{
    T* ptr = (T*)mtt::access(thing, tag, MTT_POINTER);
    if (ptr == nullptr) {
        return nullptr;
    }
    return ptr;
}







template <>
Thing_Ref* access<Thing_Ref>(Thing* thing, const String& tag);

template <>
Thing_List* access<Thing_List>(Thing* thing, const String& tag);

template <>
String_List* access<String_List>(Thing* thing, const String& tag);


bool add_connection(World* world, mtt::Thing* src, uint64 src_port, mtt::Thing* dst, uint64 dst_port);

bool add_connection(World* world, Thing_ID src, const String& src_tag, Thing_ID dst, const String& dst_tag, Thing** src_thing_out, Thing** dst_thing_out);

bool add_connection(World* world, Thing* src, const String& src_tag, Thing* dst, const String& dst_tag);

bool add_connection(World* world, Thing_ID src, const String& src_tag, Thing_ID dst, const String& dst_tag, my::Function<void(World*, Thing*, Thing*, void*)> callback, void* state, Thing** src_thing_out, Thing** dst_thing_out);
bool add_connection(World* world, Thing* src, const String& src_tag, Thing* dst, const String& dst_tag, my::Function<void(World*, Thing*, Thing*, void*)> callback, void* state);

bool name_to_out_port_id(World* world, Thing_ID thing_id, const String& tag, uint64* out);
bool name_to_in_port_id(World* world, Thing_ID thing_id, const String& tag, uint64* out);
bool remove_connection(World* world, Connection& connection);

bool remove_connection_at_dst_port(World* world, Thing_ID thing_id, uint64 port_idx);
bool remove_connection_at_dst(mtt::Thing* dst, const mtt::String& dst_port_name);

bool remove_all_connections(World* world, Thing_ID thing_id);

void add_execution_connection(mtt::Thing* thing_src, mtt::Thing* thing_dst);
void remove_execution_connection(mtt::Thing* thing_src, mtt::Thing* thing_dst);
void remove_incoming_execution_connections(mtt::Thing* thing);
void remove_outgoing_execution_connections(mtt::Thing* thing);
void remove_all_execution_connections(mtt::Thing* thing);
void delete_execution_connections(mtt::Thing* thing);

void evaluate_world_pre(World* world);
void evaluate_world(World* world);
void evaluate_world_post(World* world);

void logic_print_internal(World* world, Thing* thing);
void logic_print(World* world);

void frame_begin(World* world, bool do_clear);
void frame_end(World* world, bool do_clear);

void Connection_print(Connection* connection, World* world);

enum LOGIC_OPTION_SENSOR {
    LOGIC_OPTION_SENSOR_RECTANGLE_BOUND,
    LOGIC_OPTION_SENSOR_CIRCLE_BOUND,
    
    LOGIC_OPTION_SENSOR_COUNT
};

Collision_Handler_Result collision_handler_Tag_Sensor_default(Broad_Phase_Collision_Record* collisions, Collision_System* collision_handler_Tag_Sensor_default);

template <typename Procedure>
void Thing_apply_self_and_children(mtt::World* world, mtt::Thing_ID thing_id, Procedure&& proc, void* args)
{
    auto& q = world->traversal_queue;
    q.emplace_back(thing_id);
    
    while (!q.empty()) {
        mtt::Thing_ID id = q.front();
        q.pop_front();
        
        mtt::Thing* thing = nullptr;
        if (world->Thing_try_get(id, &thing)) {
            if (thing->is_visited) {
                return;
            }
            
            //thing->is_visited = true;
            
            proc(world, thing, args);
            
            for (auto it_children = thing->child_id_set.begin();
                 it_children != thing->child_id_set.end();
                 ++it_children) {
                
                q.emplace_back(*it_children);
            }
        }
    }
}

template <typename Procedure>
void Thing_apply_self_and_children(mtt::World* world, mtt::Thing* thing, Procedure proc, void* args)
{
    Thing_apply_self_and_children(world, thing->id, proc, args);
}


template <typename Procedure>
void Thing_apply_all_attached(mtt::World* world, mtt::Thing* thing, Procedure proc, void* args);
template <typename Procedure>
void Thing_apply_self_and_attached(mtt::World* world, mtt::Thing_ID thing_id, Procedure proc, void* args);

template <typename Procedure>
void breadth_first_traverse_thing_hierarchy(mtt::World* world, mtt::Thing_ID thing_id, Procedure proc, void* args);

void breadth_first_traverse_thing_hierarchy_w_args(mtt::World* world, mtt::Thing_ID thing_id, void (*proc)(mtt::World* world, mtt::Thing* thing, void* args), void* args);

void breadth_first_traverse_thing_arrows_w_args(mtt::World* world, mtt::Thing_ID thing_id, void (*proc)(mtt::World* world, mtt::Thing* thing, void* args), void* args);

template <typename Procedure>
mtt::Thing* breadth_first_traverse_thing_hierarchy_with_stopping_condition(mtt::World* world, mtt::Thing_ID thing_id, Procedure proc, void* args);

void Things_mark_unvisited(mtt::World* world);
void Things_mark_unvisited(Thing* const things, const usize count);
void Things_mark_unvisited(mtt::World* world, std::vector<mtt::Thing*>& things);

void Thing_destroy_self_and_connected(mtt::World* world, Thing_ID thing_id);
void Thing_destroy_if_predicate_true(mtt::World* world, Thing_ID thing_id, bool (*destroy)(mtt::Thing*));

//{
//    mtt::Thing* thing = nullptr;
//    if (world->Thing_try_get(thing_id, &thing)) {
//        breadth_first_traverse_thing_hierarchy(world, thing, proc);
//    }
//}

void connect_parent_to_child(mtt::World* world, mtt::Thing* parent, mtt::Thing* child);
void connect_parent_to_child(mtt::World* world, mtt::Thing_ID parent, mtt::Thing_ID child);

void disconnect_child_from_parent(mtt::World* world, mtt::Thing* child_thing);
void disconnect_child_from_parent(mtt::World* world, mtt::Thing_ID child);
void disconnect_parent_from_children(mtt::World* world, mtt::Thing* thing);

void remove_thing_from_hierarchy(mtt::World* world, mtt::Thing* thing);

void add_to_group(mtt::World* world, mtt::Thing* group, mtt::Thing* child);
void remove_from_group(mtt::World* world, mtt::Thing* group, mtt::Thing* child);



bool is_root(mtt::Thing* thing);
bool is_root(mtt::World* world, mtt::Thing_ID thing_id);

mtt::Thing_ID get_parent_ID(mtt::Thing* thing);
mtt::Thing* get_parent(mtt::Thing* thing);

bool exists_in_other_hierarchy(mtt::Thing* thing, mtt::Thing* other);


bool thing_group_is_active(mtt::Thing* const thing);
bool thing_group_set_active_state(mtt::Thing* const thing, bool active_state);
bool thing_group_set_active_state(mtt::World* world, mtt::Thing_ID const thing_id, bool active_state);


void set_option(Thing* thing, uint64 option, uint64 flags);

constexpr const uint64 BUILTIN_CURVE_PROC_TYPE_NONE                = 0;
constexpr const uint64 BUILTIN_CURVE_PROC_TYPE_LAGRANGE_POLYNOMIAL = 1;
constexpr const uint64 BUILTIN_CURVE_PROC_TYPE_CATMULLROM          = 2;

constexpr const uint64 BUILTIN_CURVE_PROC_TYPE_COUNT               = 3;

constexpr const uint64 BUILTIN_CURVE_PROC_TYPES[] = {
    BUILTIN_CURVE_PROC_TYPE_NONE,
    BUILTIN_CURVE_PROC_TYPE_LAGRANGE_POLYNOMIAL,
    BUILTIN_CURVE_PROC_TYPE_CATMULLROM,
    0
};

struct Curve_Procedure {
    mtt::Procedure* base;
    uint64 type;
    
    std::variant<
        Lagrange_Polynomial_3D,
        CatmullRom
    > state;
    
    Curve_Procedure(void)
    : type(BUILTIN_CURVE_PROC_TYPE_NONE)
    {}
    

};


mtt::Thing_Archetype* archetype_of(Thing* thing);

void add_selector(mtt::Thing_Archetype* arch, cstring name, const mtt::Procedure& proc);
bool add_selector(mtt::World* world, mtt::Thing_Archetype_ID id, cstring name, const mtt::Procedure& proc);
void add_own_selector(mtt::Thing* thing, cstring name, const mtt::Procedure& proc);


#define MTT_add_selector(arch__, name__, body__, state__) add_selector(arch__, name__, MTT_PROC(body__, state__))
#define MTT_add_own_selector(thing__, name__, body__, state__) add_own_selector(thing__, name__, MTT_PROC(body__, state__))

void remove_own_selector(mtt::Thing* thing, cstring name);

Procedure_Return_Type selector_invoke(mtt::Thing_Archetype* arch, mtt::Thing* thing, Message* message);
Procedure_Return_Type selector_invoke(mtt::Thing* thing, Message* message);

mtt::Thing* get_group(mtt::Thing* thing);
mtt::Thing* get_root_group(mtt::Thing* thing);
mtt::Thing* get_root(mtt::Thing* thing);

mtt::Thing* get_lowest_child(mtt::Thing* thing);


mtt::Thing* get_parent_with_archetype(mtt::Thing* thing, mtt::Thing_Archetype_ID arch);
mtt::Thing* get_first_child_with_archetype(mtt::Thing* thing, mtt::Thing_Archetype_ID arch);


const uint64 DRAWABLE_INFO_SHARED_FLAG_BITMASK = 1;


void Thing_set_position(mtt::Thing* thing, vec3 position);

mtt::Thing* Thing_set_position(mtt::World* world, mtt::Thing_ID id, vec3 position);

MTT_NODISCARD mtt::Thing* Thing_make_with_unit_collider(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, mtt::COLLIDER_TYPE collider_type, vec3 in_position, bool is_in_world = true, mtt::Collision_System* collision_system = nullptr);

MTT_NODISCARD mtt::Thing* Thing_make_with_aabb_dimensions(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, vec3 in_position, vec2 dimensions, bool is_in_world, mtt::Collision_System* collision_system = nullptr);

MTT_NODISCARD mtt::Thing* Thing_make_with_aabb_corners(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, mtt::Box box, float32 z_position, bool is_in_world, mtt::Collision_System* collision_system = nullptr);

void Thing_set_aabb_corners(mtt::Thing* thing, mtt::Rep** rep_out, mtt::Box box, float32 z_position);

//void Thing_set_aabb_corners(mtt::Thing* thing, mtt::Box box, float32 z_position);

void Thing_set_aabb_dimensions(mtt::Thing* thing, vec2 scale_2d, mtt::Rep** rep_out, vec3 in_position, vec2 dimensions);


void set_pose_transform(mtt::Thing* thing, const Mat4& mat);
void set_pose_transform(mtt::Thing* thing, const Mat4* mat);

struct Thing_Iterator {
    mtt::Thing* value;
    mtt::Dynamic_Array<mtt::Array<mtt::Thing, THING_PAGE_SIZE>>* things;
    usize idx;
    usize count_remaining;
    usize max_idx;
    
    
    mtt::Thing_Storage::iterator it;
    mtt::World* world;
    
};

Thing_Iterator iterator(mtt::World* world);

bool next(Thing_Iterator* it);


inline usize Thing_count(mtt::World* world)
{
    return world->things.count;
}



struct Attrib_Record {
    flecs::entity attrib;
};


mtt::String attrib_indent(uint32 lvl);
void attrib_visit_components(const flecs::entity& e, uint32 lvl = 0);

void attrib_visit_is_a(const flecs::entity& e, dt::Word_Dictionary_Entry* entry, uint32 lvl = 0);

std::vector<Attrib_Record>& get_word_attributes(mtt::Thing* thing);


void Thing_get_properties_for_attribute(mtt::Thing* src, dt::Word_Dictionary_Entry* entry, my::Function<void(mtt::Thing*, dt::Word_Dictionary_Entry*, dt::Word_Dictionary_Entry*, const mtt::Any*)> proc);



bool is_user_movable(mtt::Thing* thing);

mtt::Thing_ID get_thing_most_recently_selected_with_touch(mtt::World* world);
mtt::Thing* get_thing_ptr_most_recently_selected_with_touch(mtt::World* world);
void set_thing_most_recently_selected_with_touch(mtt::World* world, mtt::Thing_ID ID, int LINE = -1, const char* const file = "");
void* selected_things(mtt::World* world);





usize increment_selection_count(mtt::Thing* thing);
usize decrement_selection_count(mtt::Thing* thing);
usize selection_count(mtt::Thing* thing);


bool is_ancestor_of(mtt::Thing* possible_parent, mtt::Thing* thing);

template <typename Proc>
bool true_for_some_ancestor(mtt::Thing* thing, Proc&& proc)
{
    while (!mtt::is_root(thing)) {
        thing->world()->Thing_get(thing->parent_thing_id, &thing);
        if (proc(thing)) {
            return true;
        }
    }
    
    return false;
}



void init_flag(THING_FLAG* src);
void set_flag(THING_FLAG* src, THING_FLAG flag);
void unset_flag(THING_FLAG* src, THING_FLAG flag);
bool flag_is_set(THING_FLAG* src, THING_FLAG flag);
void set_flag(THING_FLAG* src, THING_FLAG flag, bool state);

void init_flag(mtt::Thing* thing);
void set_flag(Thing* thing, THING_FLAG flag);
void unset_flag(Thing* thing, THING_FLAG flag);
bool flag_is_set(Thing* thing, THING_FLAG flag);
void set_flag(Thing* thing, THING_FLAG flag, bool state);

Field_List* fields(mtt::Thing* thing);

static inline bool is_active(mtt::Thing* thing)
{
    //return mtt::flag_is_set(thing, THING_FLAG_is_active);
    return fields(thing)->is_active;
}
static inline bool is_active(mtt::Thing_Archetype* thing)
{
    //return mtt::flag_is_set(thing, THING_FLAG_is_active);
    return thing->field_descriptor.data.is_active;
}
static inline void set_is_active(mtt::Thing* thing)
{
    fields(thing)->is_active = true;
   // mtt::set_flag(thing, THING_FLAG_is_active);
}
static inline void set_is_active(mtt::Thing_Archetype* thing)
{
    thing->field_descriptor.data.is_active = true;
}
static inline void unset_is_active(mtt::Thing* thing)
{
    fields(thing)->is_active = false;
   // mtt::unset_flag(thing, THING_FLAG_is_active);
}
static inline void set_should_render(mtt::Thing* thing)
{
    auto* r = mtt::rep(thing);
    for (auto& d : r->render_data.drawable_info_list) {
        d->is_enabled = true;
    }
}
static inline void unset_should_render(mtt::Thing* thing)
{
    auto* r = mtt::rep(thing);
    for (auto& d : r->render_data.drawable_info_list) {
        d->is_enabled = false;
    }
}

static inline bool is_rendering_something(mtt::Thing* thing) {
    auto* r = mtt::rep(thing);
    for (auto& d : r->render_data.drawable_info_list) {
        if (d->is_enabled) {
            return true;
        }
    }
    
    return false;
}
static inline void match_should_render(mtt::Thing* thing, mtt::Thing* thing_to_match)
{
    auto* r_thing = mtt::rep(thing);
    auto* r_match = mtt::rep(thing_to_match);
    auto& dr_list_src = r_match->render_data.drawable_info_list;
    auto& dr_list_tgt = r_thing->render_data.drawable_info_list;
    usize len = m::min((usize)dr_list_src.size(), (usize)dr_list_tgt.size());
    for (usize i = 0; i < len; i += 1) {
        dr_list_tgt[i]->is_enabled = dr_list_src[i]->is_enabled;
    }
}
static inline void unset_is_active(mtt::Thing_Archetype* thing)
{
    thing->field_descriptor.data.is_active = false;
    // mtt::unset_flag(thing, THING_FLAG_is_active);
}
static inline bool is_user_drawable(mtt::Thing* thing)
{
    return thing->is_user_drawable;
}
static inline bool is_paused(mtt::Thing* thing)
{
    return flag_is_set(&thing->flags, THING_FLAG_is_paused);
}
static inline bool is_active_group(mtt::Thing* thing, mtt::Thing_ID ctx=0)
{
    return fields(thing)->is_active_group;
    //return mtt::flag_is_set(&thing->world()->things.thing_to_run_ctx_specific_flags[thing->id][ctx], THING_FLAG_is_active_group);
}
static inline bool is_active_group(mtt::Thing_Archetype* thing, mtt::Thing_ID ctx=0)
{
    return thing->field_descriptor.data.is_active_group;
    //return mtt::flag_is_set(&thing->world()->things.thing_to_run_ctx_specific_flags[thing->id][ctx], THING_FLAG_is_active_group);
}
static inline void set_is_active_group(mtt::Thing* thing, mtt::Thing_ID ctx=0)
{
    fields(thing)->is_active_group = true;
   // mtt::set_flag(&thing->world()->things.thing_to_run_ctx_specific_flags[thing->id][ctx], THING_FLAG_is_active_group);
}
static inline void set_is_active_group(mtt::Thing_Archetype* thing, mtt::Thing_ID ctx=0)
{
    thing->field_descriptor.data.is_active_group = true;
    // mtt::set_flag(&thing->world()->things.thing_to_run_ctx_specific_flags[thing->id][ctx], THING_FLAG_is_active_group);
}
static inline void unset_is_active_group(mtt::Thing* thing, mtt::Thing_ID ctx=0)
{
    fields(thing)->is_active_group = false;
    //mtt::unset_flag(&thing->world()->things.thing_to_run_ctx_specific_flags[thing->id][ctx], THING_FLAG_is_active_group);
}
static inline void unset_is_active_group(mtt::Thing_Archetype* thing, mtt::Thing_ID ctx=0)
{
    thing->field_descriptor.data.is_active_group = false;
    //mtt::unset_flag(&thing->world()->things.thing_to_run_ctx_specific_flags[thing->id][ctx], THING_FLAG_is_active_group);
}
static inline void set_is_active_group(mtt::Thing* thing, bool state, mtt::Thing_ID ctx=0)
{
//    mtt::set_flag(&thing->world()->things.thing_to_run_ctx_specific_flags[thing->id][ctx], THING_FLAG_is_active_group, state);
    fields(thing)->is_active_group = state;
}
static inline void set_is_active_group(mtt::Thing_Archetype* thing, bool state, mtt::Thing_ID ctx=0)
{
    //    mtt::set_flag(&thing->world()->things.thing_to_run_ctx_specific_flags[thing->id][ctx], THING_FLAG_is_active_group, state);
    thing->field_descriptor.data.is_active_group = state;
}

void set_is_actor(mtt::Thing_Archetype* t);
void unset_is_actor(mtt::Thing_Archetype* t);
bool is_actor(mtt::Thing_Archetype* t);
bool is_actor(mtt::Thing* thing);



template<typename T>
void* make_temp(T** out);
template<typename T>
void* temp_mem(const T& in);
template<typename T>
Array_Slice<T> make_temp(T** out, usize n);

mem::Allocator* field_allocator(mtt::World* world);
mem::Allocator* buckets_allocator(mtt::World* world);
mem::Allocator* message_allocator(mtt::World* world);

inline static mem::Allocator* field_allocator()
{
    return field_allocator(mtt::ctx());
}

inline static mem::Allocator* buckets_allocator()
{
    return buckets_allocator(mtt::ctx());
}

inline static mem::Allocator* message_allocator()
{
    return message_allocator(mtt::ctx());
}

inline static mem::Allocator* args_allocator()
{
    return &mtt::ctx()->arg_allocation.allocator;
}


void message_free(Message* msg);

bool actor_process_messages(mtt::Thing* thing, Messages_Record* msgs, void (*handler)(Message* msg));


struct Message_Queue {
    mtt::Dynamic_Array<Message> messages;
    
    usize head;
    usize tail;
    usize count;
    
    void init(usize initial_count);
    void deinit(void);
    
    void enqueue(Message* msg);
    void dequeue(void(*handler)(Message* msg));
    void dequeue_discard(void);
    void dequeue_all(void(*handler)(Message* msg));
    inline bool is_empty(void)
    {
        return this->count == 0;
    }
    inline bool is_full(void)
    {
        return this->count == this->messages.cap;
    }
    
    void clear(void);
    
    inline usize cap(void)
    {
        return this->messages.cap;
    }
    
    
};

template <typename PROC>
void Message_Queue_dequeue_with_closure(Message_Queue* q, PROC&& proc);

void Message_Queue_print(Message_Queue* q);


struct Selector_Label {
    MTT_String_Ref ref;
    
    Selector_Label(MTT_String_Ref ref) : ref(ref)
    {
    }
    
    operator cstring()
    {
        return string_get(this->ref);
    }
    
    operator MTT_String_Ref_ID()
    {
        return this->ref.id;
    }
};

Field_List* fields(mtt::Thing* thing);

//struct alignas(16) Particle_State {
//    vec3    position;
//    vec3    velocity;
//    vec4    color;
//    float32 angular_velocity;
//    float32 angle;
//    float32 scale;
//    mtt::Thing_ID target;
//};
struct alignas(16) Particle_System_State {
    //sd::Drawable_Info drawable;
//    mtt::Dynamic_Array<Particle_State>    state_list;
//    mtt::Dynamic_Array<sd::Drawable_Info> instance_list;
    
    vec3    position = vec3(0.0f);
    vec3    velocity = vec3(0.0f);
    float32 max_velocity_magnitude = POSITIVE_INFINITY;
    vec4    color = vec4(1.0f);
    float32 angular_velocity = 0.0f;
    float32 angle = 0.0f;
    float32 scale = 1.0f;
    float32 initial_scale = 1.0f;
    float32 lifetime_accumulator = 0.0f;
    float32 lifetime = POSITIVE_INFINITY;
    mtt::Thing_ID target = mtt::Thing_ID_INVALID;
    
    void (*distance_procedure)(Particle_System_State* ps);
    
    void print(void) {
//#ifndef NDEBUG
//        std::cout << "(Particle_State) {" << std::endl <<
//        "    position " << m::to_string(position) << std::endl <<
//        "    velocity " << m::to_string(velocity) << std::endl <<
//        "    angle " << angle << std::endl <<
//        "    scale " << scale << std::endl <<
//        "}" << std::endl;
//#endif
    }
    
};
void Particle_State_update(Particle_System_State*);

struct Node_Graph_Source_Target {
    mtt::Thing_Archetype_ID type;
    mtt::Thing_ID src       = mtt::Thing_ID_INVALID;
    mtt::Thing_ID tgt       = mtt::Thing_ID_INVALID;
};


typedef enum NODE_GRAPH_FOLLOW_FLAG {
    NODE_GRAPH_FOLLOW_FLAG_FORWARD_NEIGHBORS  = (1 << 0),
    NODE_GRAPH_FOLLOW_FLAG_ANY                = (1 << 1),
    NODE_GRAPH_FOLLOW_FLAG_DEFAULT = NODE_GRAPH_FOLLOW_FLAG_FORWARD_NEIGHBORS,
} NODE_GRAPH_FOLLOW_FLAG;

struct Node_Graph_State {
    Thing_To_Thing_List_Map map;
    Thing_To_Thing_List_Map reverse_map;
    mtt::Map<mtt::Thing_ID, Node_Graph_Source_Target> src_tgts;
    std::vector<mtt::Thing_ID> nodes;
    NODE_GRAPH_FOLLOW_FLAG flags = NODE_GRAPH_FOLLOW_FLAG_DEFAULT;
};

template <typename PROC>
static inline void node_graph_add_follower(Node_Graph_State* s, mtt::Thing* src, PROC&& proc)
{
    auto& record = s->src_tgts[src->id];
    record.src = src->id;
    proc(record);
}
static inline Node_Graph_Source_Target* node_graph_get_follower(Node_Graph_State* s, mtt::Thing_ID id)
{
    auto find_it = s->src_tgts.find(id);
    if (find_it != s->src_tgts.end()) {
        return &find_it->second;
    }
    
    return nullptr;
}

static inline void node_graph_remove_follower(Node_Graph_State* s, mtt::Thing* src)
{
    auto find_it = s->src_tgts.find(src->id);
    if (find_it != s->src_tgts.end()) {
        s->src_tgts.erase(find_it);
    }
}
static inline void node_graph_add_graph_node(Node_Graph_State* s, mtt::Thing_ID id)
{
    s->map.emplace(id, Thing_Edge_List());
    s->reverse_map.emplace(id, Thing_Edge_List());
}
static inline void node_graph_remove_graph_node(Node_Graph_State* s, mtt::Thing_ID id)
{
    {
        auto find_it = s->map.find(id);
        if (find_it != s->map.end()) {
            s->map.erase(find_it);
        }
    }
    {
        for (auto find_it = s->nodes.begin(); find_it != s->nodes.end(); ++find_it) {
            if (*find_it == id) {
                s->nodes.erase(find_it);
                break;
            }
        }
        
    }
    
}


static inline void node_graph_remove_graph_edge(Node_Graph_State* s, mtt::Thing_ID src, mtt::Thing_ID dst)
{
    auto& list = s->map[src];
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (edge_id(*it) == dst) {
            list.erase(it);
            auto find_it = s->reverse_map.find(dst);
            if (find_it != s->reverse_map.end()) {
                for (auto in_it = find_it->second.begin(); in_it != find_it->second.end(); ++in_it) {
                    if (edge_id(*in_it) == src) {
                        find_it->second.erase(in_it);
                        break;
                    }
                }
            }
            return;
        }
    }
}

template <typename PROC>
static inline bool node_graph_remove_graph_node_with_callback(Node_Graph_State* s, mtt::Thing_ID id, PROC callback)
{
    {
        bool found = false;
        for (auto find_it = s->nodes.begin(); find_it != s->nodes.end(); ++find_it) {
            if (*find_it == id) {
                s->nodes.erase(find_it);
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    auto out_find_it = s->map.find(id);
    auto in_find_it  = s->reverse_map.find(id);
    ASSERT_MSG(out_find_it != s->map.end() && in_find_it != s->reverse_map.end(), "Should exist!");
    
    
    Thing_Edge_List e_out = out_find_it->second;
    Thing_Edge_List e_in  = in_find_it->second;
    
//    if (!in_find_it->second.empty()) {
//        usize init_size = in_find_it->second.size();
//        do {
//            node_graph_remove_graph_edge(s, *((in_find_it->second).begin()), id);
//            if (in_find_it->second.size() == init_size) {
//                break;
//            }
//        } while (1);
//
//    }
    {
        auto& list = s->reverse_map[id];
        for (auto it = list.begin(); it != list.end(); ++it) {
            auto& l_from = s->map[edge_id(*it)];
            for (auto it_from = l_from.begin(); it_from != l_from.end();) {
                if (edge_id(*it_from) == id) {
                    it_from = l_from.erase(it_from);
                } else {
                    ++it_from;
                }
            }
        }
    }
    
    s->map.erase(out_find_it);
    s->reverse_map.erase(in_find_it);
    
    callback(id, e_out, e_in);
    return true;

}

static inline void node_graph_add_graph_edge(Node_Graph_State* s, mtt::Thing_ID src, mtt::Thing_ID dst)
{
    if (src == dst) {
        return;
    }
    
    auto& list = s->map[src];
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (edge_id(*it) == dst) {
            return;
        }
    }
    list.push_back({.id = dst});
    
    s->reverse_map[dst].push_back({.id = src});
}



static inline void node_graph_set_graph_edge_list(Node_Graph_State* s, mtt::Thing_ID src, const Node_Graph_Edge_List& list)
{
    s->map.insert({src, list});
    for (usize i = 0; i < list.size(); i += 1) {
        s->reverse_map[edge_id(list[i])].push_back(list[i]);
    }
}
static inline void node_graph_set_graph_edge_list(Node_Graph_State* s, mtt::Thing_ID src, Node_Graph_Edge_List& list)
{
    s->map.insert({src, list});
    for (usize i = 0; i < list.size(); i += 1) {
        s->reverse_map[edge_id(list[i])].push_back(list[i]);
    }
}
static inline void node_graph_set_graph_edge_list_with_thing_id_list(Node_Graph_State* s, mtt::Thing_ID src, const std::vector<mtt::Thing_ID>& list)
{
    Thing_Edge_List edge_list;
    for (usize i = 0; i < list.size(); i += 1) {
        edge_list.push_back((Thing_Edge){.id = list[i]});
    }
    s->map.insert({src, edge_list});
    for (usize i = 0; i < list.size(); i += 1) {
        
        s->reverse_map[list[i]].push_back({.id = src});
    }
}
static inline void node_graph_set_graph_edge_list_with_thing_id_list(Node_Graph_State* s, mtt::Thing_ID src, Thing_ID_List& list)
{
    Thing_Edge_List edge_list;
    for (usize i = 0; i < list.size(); i += 1) {
        edge_list.push_back((Thing_Edge){.id = list[i]});
    }
    s->map.insert({src, edge_list});
    for (usize i = 0; i < list.size(); i += 1) {
        s->reverse_map[list[i]].push_back({.id = src});
    }
}

static inline Node_Graph_Edge_List* node_graph_get_graph_edge_list(Node_Graph_State* s, mtt::Thing_ID src)
{
    auto find_it = s->map.find(src);
    if (find_it != s->map.end()) {
        return &find_it->second;
    }
    
    return nullptr;
}
static inline Node_Graph_Edge_List* node_graph_get_graph_incoming_edge_list(Node_Graph_State* s, mtt::Thing_ID dst)
{
    auto find_it = s->reverse_map.find(dst);
    if (find_it != s->reverse_map.end()) {
        return &find_it->second;
    }
    
    return nullptr;
}




static inline void node_graph_print(Node_Graph_State* s)
{
//    printf("(Thing_Type.Node_Graph){\n");
//    for (auto it = s->map.begin(); it != s->map.end(); ++it) {
//        printf("    %llu {\n", it->first);
//        for (auto it_sub = it->second.begin(); it_sub != it->second.end(); ++it_sub) {
//            printf("        %llu,\n", *it_sub);
//        }
//        printf("    }\n");
//    }
//
//    printf("}\n");
}
static inline void node_graph_print(mtt::Thing* thing)
{
    if (thing->archetype_id != ARCHETYPE_NODE_GRAPH) {
        return;
    }
    
    node_graph_print(mtt::access<Node_Graph_State>(thing, "state"));
}

template <typename T, typename PROCEDURE>
bool access_array(mtt::Thing* thing, const mtt::String& name, PROCEDURE proc)
{
    mtt::Dynamic_Array<T>** arr = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<T>*>(thing, name);
    if (arr == nullptr) {
        return false;
    }
    
    proc(**arr);

    return true;
}

#define MTT_GET_ARRAY_FIELD(TYPE_T__, thing__, name__, in_var_name__, proc_body__) \
access_array< TYPE_T__ > ( thing__, name__, in_var_name__ [&](mtt::Dynamic_Array< TYPE_T__ >& in_var_name__ ) \
    proc_body__ \
\
)

template <typename PROCEDURE>
bool access_thing_list(mtt::Thing* thing, const mtt::String& name, PROCEDURE proc)
{
    mtt::Dynamic_Array<mtt::Thing_Ref>* arr = mtt::access<mtt::Thing_List>(thing, name);
    if (arr == nullptr) {
        return false;
    }
    
    proc(*arr);
    
    return true;
}


extern cstring text_thing_empty_string;

inline static Arrow_Links* arrow_links(mtt::World* world)
{
    return &world->things.arrows;
}

void arrow_edge_add(mtt::World* world, mtt::Thing_ID src, mtt::Thing_ID dst, const mtt::String& label = "", ARROW_LINK_FLAGS flags = ARROW_LINK_FLAGS_DEFAULT);

bool arrow_edge_add_or_if_exists_remove(mtt::World* world, mtt::Thing_ID src, mtt::Thing_ID dst, const mtt::String& label = "", ARROW_LINK_FLAGS flag = ARROW_LINK_FLAGS_DEFAULT);

void arrow_edge_remove(mtt::World* world, mtt::Thing_ID src, mtt::Thing_ID dst, const mtt::String& label = "");

void arrow_edge_remove_all_with_source(mtt::Thing* src);

bool arrow_edge_label_is_equal(mtt::Arrow_Link& a, mtt::Arrow_Link& b);

void toggle_verbose_display(mtt::World* world);
void toggle_attachment_links_display(mtt::World* world);
bool should_show_verbose_display(mtt::World* world);
bool should_show_attachment_links(mtt::World* world);
void toggle_debug_display(mtt::World* world);
bool should_show_debug_display(mtt::World* world);

void set_priority_layer(mtt::World* world, Priority_Layer l);
void push_priority_layer(mtt::World* world, Priority_Layer l);
Priority_Layer pop_priority_layer(mtt::World* world);
Priority_Layer get_priority_layer(mtt::World* world);

void Thing_set_priority_layer(mtt::Thing* thing, Priority_Layer l);

inline static void Thing_set_priority_layer(mtt::World* world, mtt::Thing_ID id, Priority_Layer l)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, id);
    if (thing != nullptr) {
        Thing_set_priority_layer(thing, l);
    }
}

template <typename PROC>
mtt::Thing* Thing_try_get_then(mtt::World* world, mtt::Thing_ID id, PROC&& proc)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, id);
    if (thing != nullptr) {
        proc(thing);
        return thing;
    }
    return nullptr;
}
template <typename PROC>
mtt::Thing* Thing_try_get_as_archetype_then(mtt::World* world, mtt::Thing_ID id, mtt::Thing_Archetype_ID type_id, PROC&& proc)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, id);
    if (thing != nullptr && type_id == thing->archetype_id) {
        proc(thing);
        return thing;
    }
    return nullptr;
}


void sort_things_by_execution(World* world, Eval_Connection_Graph& G);

void sort_things_internal(World* world, Eval_Connection_Graph& G, Thing* dst);


void access_write_string(mtt::Thing* thing, const mtt::String& name, const mtt::String& set_to);
cstring access_string(mtt::Thing* thing, const mtt::String& name);


#define MTT_number_thing_format_str ".3f"




template <typename PROC>
static void Query_Rule_results_for_var(Query_Rule* q, Rule_Var_Handle var, PROC&& proc)
{
    if (var == Rule_Var_Handle_INVALID) {
        return;
    }
    
    static mtt::Set<mtt::Thing_ID> found = {};
    found.clear();
    auto* ecs = q->world_->ecs_world.c_ptr();
    ecs_iter_t it = ecs_rule_iter(ecs, q->rule);
    while (ecs_rule_next(&it)) {
        ecs_entity_t result = ecs_iter_get_var(&it, var);
  //      auto _ = Query_Rule_Var_for_name(q, "bedSUB");
//        ecs_entity_t r2 = ecs_iter_get_var(&it, _);
        
        if (result == 0) {
            continue;
        }

        auto ent = flecs::entity(it.world, result);
        ASSERT_MSG(ent.has<mtt::Thing_Info>(), "???");
        
        auto id = ent.get<mtt::Thing_Info>()->thing_id;
        if (found.contains(id)) {
            continue;
        }
        found.insert(id);
        
        mtt::Thing* thing = mtt::Thing_try_get(q->world_, id);
        if (thing == nullptr) {
            continue;
        } else if (!proc(thing)) {
            return;
        }

    }
}



mtt::Thing* code_procedure_make(Logic_Procedure&& proc, void* args);

mtt::Thing* code_procedure_make_thing_in(void (*proc)(mtt::World* world, Script_Instance* script, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg, bool is_valid, mtt::Thing* thing));

template<typename T>
void* make_temp(T** out)
{
    *out = mem::alloc_init<T>(&mtt::ctx()->allocator_temporary());
    return *out;
}
template<typename T>
void* temp_mem(const T& in)
{
    T* ptr = mem::alloc_init<T>(&mtt::ctx()->allocator_temporary());
    *ptr = in;
    return ptr;
}
template<typename T>
Array_Slice<T> make_temp(T** out, usize n)
{
    Array_Slice<T> data = {};
    data.count = n;
    data.data = mem::alloc_init_array<T>(&mtt::ctx()->allocator_temporary());
    *out = data.data;
    return data;
}

static inline vec3 xform(vec3& world_point, mtt::Rep& rep)
{
    world_point = m::inverse(rep.pose_transform) * rep.model_transform_inverse * vec4(world_point, 1.0);
    return world_point;
}

MTT_NODISCARD static inline vec2 adjust_translation_in_hierarchy(vec2& translation, mtt::Thing* thing)
{
    mtt::Thing_ID parent = mtt::get_parent_ID(thing);
    
    if (mtt::Thing_ID_is_valid(parent)) {
        
        vec3 d_scale;
        quat d_orientation;
        vec3 d_translation;
        vec3 d_skew;
        vec4 d_perspective;
        
        mat4 parent_model_h = mat4(1.0f);
        
        //mtt::Thing* thing_parent = mtt::Thing_try_get(mtt::world(thing), parent);
        //mtt::Rep* parent_rep = mtt::rep(thing_parent);
        mtt::Rep* parent_rep = mtt::rep(mtt::world(thing), parent);
        parent_model_h = parent_rep->hierarchy_model_transform;
        
        m::decompose(parent_model_h, d_scale, d_orientation, d_translation, d_skew, d_perspective);
        parent_model_h = m::toMat4(d_orientation) * m::scale(mat4(1.0f), d_scale);
        
        translation = m::inverse(parent_model_h) * vec4(translation, 0.0f, 1.0f);
    }
    
    return translation;
}


void Thing_attach_curve(mtt::Thing* thing);
void Thing_attach_image(mtt::Thing* thing);


void Thing_set_is_proxy(mtt::Thing* proxy, mtt::Thing* of, usize scene_id);
void Thing_unset_is_proxy(mtt::Thing* thing);
bool Thing_is_proxy(mtt::Thing* thing);
bool Thing_is_proxy(mtt::World* world, mtt::Thing_ID id);
mtt::Thing* Thing_mapped_from_proxy(mtt::Thing* thing);
mtt::Thing* Thing_mapped_from_proxy(mtt::World* world, mtt::Thing_ID thing_id);


MTT_NODISCARD Thing_Proxy_Storage* Thing_proxies_try_get(mtt::Thing* thing);
MTT_NODISCARD Thing_Proxy_Storage* Thing_proxies_try_get(mtt::World* world, mtt::Thing_ID id);
MTT_NODISCARD Thing_Proxy_Storage* Thing_proxies_try_get_for_scene(mtt::Thing* thing, usize scene_idx);
MTT_NODISCARD Thing_Proxy_Storage* Thing_proxies_try_get_for_scene(mtt::World* world, mtt::Thing_ID id, usize scene_idx);

void destroy_all_proxies(mtt::World* world);
void destroy_all_proxies_for_scene_idx(mtt::World* world, usize idx);

void Thing_destroy_proxies(mtt::World* world, mtt::Thing_ID thing_id);

bool Thing_save_as_preset(mtt::Thing* thing);
mtt::Thing* Thing_make_from_preset(mtt::World* world, const mtt::String& key);
mtt::Thing* Thing_make_from_preset(mtt::World* world, const mtt::String& key, mtt::Array_Slice<mtt::String> modifier_list);
bool Thing_find_preset(mtt::World* world, const mtt::String& key, mtt::Set<mtt::Thing_ID>** out);
bool Thing_find_preset(mtt::World* world, const mtt::String& key, mtt::Array_Slice<mtt::String> modifier_list, mtt::Set<mtt::Thing_ID>* in_out);

bool Thing_is_reserved(mtt::Thing* thing);
bool Thing_is_reserved(mtt::World* world, mtt::Thing_ID id);


void clear_args_to_stop(Script_Instance* s_i);


void set_logic_proc_for_archetype(mtt::World* world, mtt::Thing_Archetype_ID id, mtt::Logic_Procedure proc);

void Thing_defer_enable_to_next_frame(mtt::World* world, mtt::Thing* thing);

void Thing_overwrite_root_representation(Thing* dst, Thing* src);

void Thing_set_activate_colliders(mtt::Thing* thing);
void Thing_deactivate_colliders(mtt::Thing* thing);
}

#endif /* thing_hpp */
