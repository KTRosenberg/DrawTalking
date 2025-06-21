//
//  input.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/27/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef input_hpp
#define input_hpp

extern mtt::String text_in_buf;
extern usize text_in_cursor_idx;
extern bool text_in_selected_all;

typedef uintptr_t UI_Key;
constexpr const UI_Key UI_Key_INVALID = ULLONG_MAX;
constexpr const usize COMMAND_HISTORY_MAX = 256;
struct Input_Command_History {
    mtt::String cmd_history[COMMAND_HISTORY_MAX] = {};
    
    isize top = 0;
    isize saved_top = 0;
    isize saved_count = 0;
    usize count = 0;
    
    bool is_empty();
    
    void push(mtt::String& to_save);
    bool rewind(void);
    bool forwards(void);
    
    const mtt::String& get_top(void);
    
    const mtt::String& get_left(void);
    
    
    
    const mtt::String& get_right(void);
    
    template <typename PROC>
    void rewind_get(PROC&& proc)
    {
        const mtt::String& res = get_left();
        if (rewind()) {
            proc(res);
        }
        
    }
    
    const mtt::String& forward_get(void);
    
    void move_to_present(void);
};

extern Input_Command_History text_in_history;


extern_link_begin()

typedef enum UI_KEY_TYPE {
    UI_KEY_TYPE_UNKNOWN,
    UI_KEY_TYPE_A,
    UI_KEY_TYPE_B,
    UI_KEY_TYPE_C,
    UI_KEY_TYPE_D,
    UI_KEY_TYPE_E,
    UI_KEY_TYPE_F,
    UI_KEY_TYPE_G,
    UI_KEY_TYPE_H,
    UI_KEY_TYPE_I,
    UI_KEY_TYPE_J,
    UI_KEY_TYPE_K,
    UI_KEY_TYPE_L,
    UI_KEY_TYPE_M,
    UI_KEY_TYPE_N,
    UI_KEY_TYPE_O,
    UI_KEY_TYPE_P,
    UI_KEY_TYPE_Q,
    UI_KEY_TYPE_R,
    UI_KEY_TYPE_S,
    UI_KEY_TYPE_T,
    UI_KEY_TYPE_U,
    UI_KEY_TYPE_V,
    UI_KEY_TYPE_W,
    UI_KEY_TYPE_X,
    UI_KEY_TYPE_Y,
    UI_KEY_TYPE_Z,
    UI_KEY_TYPE_1,
    UI_KEY_TYPE_2,
    UI_KEY_TYPE_3,
    UI_KEY_TYPE_4,
    UI_KEY_TYPE_5,
    UI_KEY_TYPE_6,
    UI_KEY_TYPE_7,
    UI_KEY_TYPE_8,
    UI_KEY_TYPE_9,
    UI_KEY_TYPE_0,
    
    UI_KEY_TYPE_HYPHEN,
    UI_KEY_TYPE_EQUAL_SIGN,
    UI_KEY_TYPE_COMMA,
    UI_KEY_TYPE_GRAVE_ACCENT_AND_TILDE,
    UI_KEY_TYPE_PERIOD,
    UI_KEY_TYPE_APOSTROPHE,
    UI_KEY_TYPE_BACKSLASH,
    UI_KEY_TYPE_SLASH,
    
    UI_KEY_TYPE_DELETE_OR_BACKSPACE,
    UI_KEY_TYPE_RETURN_OR_ENTER,
    UI_KEY_TYPE_LEFT_SHIFT,
    UI_KEY_TYPE_RIGHT_SHIFT,
    UI_KEY_TYPE_LEFT_CONTROL,
    UI_KEY_TYPE_RIGHT_CONTROL,
    UI_KEY_TYPE_LEFT_ALT,
    UI_KEY_TYPE_RIGHT_ALT,
    UI_KEY_TYPE_APPLICATION,
    UI_KEY_TYPE_SPACEBAR,
    UI_KEY_TYPE_UP_ARROW,
    UI_KEY_TYPE_DOWN_ARROW,
    UI_KEY_TYPE_LEFT_ARROW,
    UI_KEY_TYPE_RIGHT_ARROW,
    UI_KEY_TYPE_TAB,
    
    UI_KEY_TYPE_COUNT,
} UI_KEY_TYPE;

typedef enum UI_KEY_MODIFIER_FLAG {
    UI_KEY_MODIFIER_FLAG_ALPHASHIFT = 1 << 16, // capslock
    UI_KEY_MODIFIER_FLAG_SHIFT      = 1 << 17,
    UI_KEY_MODIFIER_FLAG_CONTROL    = 1 << 18,
    UI_KEY_MODIFIER_FLAG_ALTERNATE  = 1 << 19,
    UI_KEY_MODIFIER_FLAG_COMMAND    = 1 << 20,
    UI_KEY_MODIFIER_FLAG_NUMERICPAD = 1 << 21,
} UI_KEY_MODIFIER_FLAG;

static const UI_KEY_MODIFIER_FLAG UI_KEY_MODIFIER_FLAG_MASK =
(UI_KEY_MODIFIER_FLAG)(
                       UI_KEY_MODIFIER_FLAG_ALPHASHIFT |
                       UI_KEY_MODIFIER_FLAG_SHIFT      |
                       UI_KEY_MODIFIER_FLAG_CONTROL    |
                       UI_KEY_MODIFIER_FLAG_ALTERNATE  |
                       UI_KEY_MODIFIER_FLAG_COMMAND    |
                       UI_KEY_MODIFIER_FLAG_NUMERICPAD);

static const char* const ui_key_modifier_flag_to_string[(1 << 21) + 1] = {
    [(uint64)UI_KEY_MODIFIER_FLAG_ALPHASHIFT] = "UI_KEY_MODIFIER_FLAG_ALPHASHIFT",
    [(uint64)UI_KEY_MODIFIER_FLAG_SHIFT]      = "UI_KEY_MODIFIER_FLAG_SHIFT",
    [(uint64)UI_KEY_MODIFIER_FLAG_CONTROL]    = "UI_KEY_MODIFIER_FLAG_CONTROL",
    [(uint64)UI_KEY_MODIFIER_FLAG_ALTERNATE]  = "UI_KEY_MODIFIER_FLAG_ALTERNATE",
    [(uint64)UI_KEY_MODIFIER_FLAG_COMMAND]    = "UI_KEY_MODIFIER_FLAG_COMMAND",
    [(uint64)UI_KEY_MODIFIER_FLAG_NUMERICPAD] = "UI_KEY_MODIFIER_FLAG_NUMERICPAD",
};

static inline UI_KEY_MODIFIER_FLAG ui_key_modifier_flags(uint64 raw_to_flags)
{
    return (UI_KEY_MODIFIER_FLAG)(raw_to_flags & ((uint64)UI_KEY_MODIFIER_FLAG_MASK));
}


static const UI_KEY_TYPE UI_KEY_TYPE_FIRST_ALPHANUMERIC = UI_KEY_TYPE_A;
static const UI_KEY_TYPE UI_KEY_TYPE_LAST_ALPHANUMERIC  = UI_KEY_TYPE_0;

static const UI_KEY_TYPE UI_KEY_TYPE_FIRST_VISUAL = UI_KEY_TYPE_A;
static const UI_KEY_TYPE UI_KEY_TYPE_LAST_VISUAL  = UI_KEY_TYPE_SLASH;

static const char* const ui_key_type_to_string[0xFFFF] = {
    "UI_KEY_TYPE_UNKNOWN",
    "UI_KEY_TYPE_A",
    "UI_KEY_TYPE_B",
    "UI_KEY_TYPE_C",
    "UI_KEY_TYPE_D",
    "UI_KEY_TYPE_E",
    "UI_KEY_TYPE_F",
    "UI_KEY_TYPE_G",
    "UI_KEY_TYPE_H",
    "UI_KEY_TYPE_I",
    "UI_KEY_TYPE_J",
    "UI_KEY_TYPE_K",
    "UI_KEY_TYPE_L",
    "UI_KEY_TYPE_M",
    "UI_KEY_TYPE_N",
    "UI_KEY_TYPE_O",
    "UI_KEY_TYPE_P",
    "UI_KEY_TYPE_Q",
    "UI_KEY_TYPE_R",
    "UI_KEY_TYPE_S",
    "UI_KEY_TYPE_T",
    "UI_KEY_TYPE_U",
    "UI_KEY_TYPE_V",
    "UI_KEY_TYPE_W",
    "UI_KEY_TYPE_X",
    "UI_KEY_TYPE_Y",
    "UI_KEY_TYPE_Z",
    "UI_KEY_TYPE_1",
    "UI_KEY_TYPE_2",
    "UI_KEY_TYPE_3",
    "UI_KEY_TYPE_4",
    "UI_KEY_TYPE_5",
    "UI_KEY_TYPE_6",
    "UI_KEY_TYPE_7",
    "UI_KEY_TYPE_8",
    "UI_KEY_TYPE_9",
    "UI_KEY_TYPE_0",
    "UI_KEY_TYPE_HYPHEN",
    "UI_KEY_TYPE_EQUAL_SIGN",
    "UI_KEY_TYPE_COMMA",
    "UI_KEY_TYPE_GRAVE_ACCENT_AND_TILDE",
    "UI_KEY_TYPE_PERIOD",
    "UI_KEY_TYPE_APOSTROPHE",
    "UI_KEY_TYPE_BACKSLASH",
    "UI_KEY_TYPE_SLASH",
    "UI_KEY_TYPE_DELETE_OR_BACKSPACE",
    "UI_KEY_TYPE_RETURN_OR_ENTER",
    "UI_KEY_TYPE_LEFT_SHIFT",
    "UI_KEY_TYPE_RIGHT_SHIFT",
    "UI_KEY_TYPE_LEFT_CONTROL",
    "UI_KEY_TYPE_RIGHT_CONTROL",
    "UI_KEY_TYPE_LEFT_ALT",
    "UI_KEY_TYPE_RIGHT_ALT",
    "UI_KEY_TYPE_APPLICATION",
    "UI_KEY_TYPE_SPACEBAR",
    "UI_KEY_TYPE_UP_ARROW",
    "UI_KEY_TYPE_DOWN_ARROW",
    "UI_KEY_TYPE_LEFT_ARROW",
    "UI_KEY_TYPE_RIGHT_ARROW",
    "UI_KEY_TYPE_TAB",
};

static const char ui_key_type_to_char[0xFFFF][2] = {
    {'\0', '\0'},
    {'a', 'A'},
    {'b', 'B'},
    {'c', 'B'},
    {'d', 'D'},
    {'e', 'E'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'i', 'I'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {'m', 'M'},
    {'n', 'N'},
    {'o', 'O'},
    {'p', 'P'},
    {'q', 'Q'},
    {'r', 'R'},
    {'s', 'S'},
    {'t', 'T'},
    {'u', 'U'},
    {'v', 'V'},
    {'w', 'W'},
    {'x', 'X'},
    {'y', 'Y'},
    {'z', 'Z'},
    {'1', '!'},
    {'2', '@'},
    {'3', '#'},
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {',', '<'},
    {'`', '~'},
    {'.', '>'},
    {'\'', '"'},
    {'\\', '|'},
    {'/', '?'},
};

extern UI_KEY_TYPE MTT_key_mapping[];

typedef enum UI_TOUCH_TYPE {
    UI_TOUCH_TYPE_DIRECT,
    UI_TOUCH_TYPE_POINTER,
    UI_TOUCH_TYPE_KEY,
    UI_TOUCH_TYPE_COUNT
} UI_TOUCH_TYPE;

static const char* const ui_touch_type_to_string[] = {
    "UI_TOUCH_TYPE_DIRECT",
    "UI_TOUCH_TYPE_POINTER",
    "UI_TOUCH_TYPE_KEY",
};

typedef enum UI_TOUCH_STATE {
    UI_TOUCH_STATE_BEGAN,
    UI_TOUCH_STATE_MOVED,
    UI_TOUCH_STATE_CANCELLED,
    UI_TOUCH_STATE_ENDED,
    

    
    UI_TOUCH_STATE_PAN_GESTURE_BEGAN,
    UI_TOUCH_STATE_PAN_GESTURE_CHANGED,
    UI_TOUCH_STATE_PAN_GESTURE_CANCELLED,
    UI_TOUCH_STATE_PAN_GESTURE_ENDED,

    
    UI_TOUCH_STATE_PINCH_GESTURE_BEGAN,
    UI_TOUCH_STATE_PINCH_GESTURE_CHANGED,
    UI_TOUCH_STATE_PINCH_GESTURE_CANCELLED,
    UI_TOUCH_STATE_PINCH_GESTURE_ENDED,
    
    
    UI_TOUCH_STATE_ROTATION_GESTURE_BEGAN,
    UI_TOUCH_STATE_ROTATION_GESTURE_CHANGED,
    UI_TOUCH_STATE_ROTATION_GESTURE_CANCELLED,
    UI_TOUCH_STATE_ROTATION_GESTURE_ENDED,
    
    UI_TOUCH_STATE_LONG_PRESS_GESTURE_BEGAN,
    UI_TOUCH_STATE_LONG_PRESS_GESTURE_CHANGED,
    UI_TOUCH_STATE_LONG_PRESS_GESTURE_CANCELLED,
    UI_TOUCH_STATE_LONG_PRESS_GESTURE_ENDED,
    
    UI_TOUCH_STATE_SWIPE_BEGAN,
    UI_TOUCH_STATE_SWIPE_CHANGED,
    UI_TOUCH_STATE_SWIPE_CANCELLED,
    UI_TOUCH_STATE_SWIPE_ENDED,
    
    UI_TOUCH_STATE_DOUBLE_TAP_PREFERRED,
    
    UI_TOUCH_STATE_DOUBLE_TAP_NONPREFERRED,
    
    UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_BEGAN,
    UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_CHANGED,
    UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_CANCELLED,
    UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_ENDED,
    UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_BEGAN,
    UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_CHANGED,
    UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_CANCELLED,
    UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_ENDED,
    
    UI_TOUCH_STATE_KEY_BEGAN,
    UI_TOUCH_STATE_KEY_CHANGED,
    UI_TOUCH_STATE_KEY_CANCELLED,
    UI_TOUCH_STATE_KEY_ENDED,
    UI_TOUCH_STATE_KEY_REPEATED,
    
    UI_HOVER_STATE_BEGAN,
    UI_HOVER_STATE_CHANGED,
    UI_HOVER_STATE_CANCELLED,
    UI_HOVER_STATE_ENDED,
    
    UI_TOUCH_STATE_PRESSED_BUTTON_BEGAN,
    UI_TOUCH_STATE_PRESSED_BUTTON_CHANGED,
    UI_TOUCH_STATE_PRESSED_BUTTON_CANCELLED,
    UI_TOUCH_STATE_PRESSED_BUTTON_ENDED,
    
    UI_TOUCH_STATE_NONE,
    
    UI_TOUCH_STATE_COUNT
} UI_TOUCH_STATE;

typedef enum UI_GESTURE_TYPE {
    UI_GESTURE_TYPE_PAN,
    UI_GESTURE_TYPE_PINCH,
    UI_GESTURE_TYPE_ROTATION,
    UI_GESTURE_TYPE_LONG_PRESS,
    UI_GESTURE_TYPE_SWIPE,
    UI_GESTURE_TYPE_BORDER_SWIPE_FROM_LEFT,
    UI_GESTURE_TYPE_BORDER_SWIPE_FROM_RIGHT,
    UI_GESTURE_TYPE_COUNT
} UI_GESTURE_TYPE;


static inline bool ui_state_is_hover_event(UI_TOUCH_STATE st)
{
    return st >= UI_HOVER_STATE_BEGAN && st <= UI_HOVER_STATE_ENDED;
}

static const char* const ui_state_type_to_string[] = {
    "UI_TOUCH_STATE_BEGAN",
    "UI_TOUCH_STATE_MOVED",
    "UI_TOUCH_STATE_CANCELLED",
    "UI_TOUCH_STATE_ENDED",
    

    
    "UI_TOUCH_STATE_PAN_GESTURE_BEGAN",
    "UI_TOUCH_STATE_PAN_GESTURE_CHANGED",
    "UI_TOUCH_STATE_PAN_GESTURE_CANCELLED",
    "UI_TOUCH_STATE_PAN_GESTURE_ENDED",

    
    "UI_TOUCH_STATE_PINCH_GESTURE_BEGAN",
    "UI_TOUCH_STATE_PINCH_GESTURE_CHANGED",
    "UI_TOUCH_STATE_PINCH_GESTURE_CANCELLED",
    "UI_TOUCH_STATE_PINCH_GESTURE_ENDED",
    
    
    "UI_TOUCH_STATE_ROTATION_GESTURE_BEGAN",
    "UI_TOUCH_STATE_ROTATION_GESTURE_CHANGED",
    "UI_TOUCH_STATE_ROTATION_GESTURE_CANCELLED",
    "UI_TOUCH_STATE_ROTATION_GESTURE_ENDED",
    
    "UI_TOUCH_STATE_LONG_PRESS_GESTURE_BEGAN",
    "UI_TOUCH_STATE_LONG_PRESS_GESTURE_CHANGED",
    "UI_TOUCH_STATE_LONG_PRESS_GESTURE_CANCELLED",
    "UI_TOUCH_STATE_LONG_PRESS_GESTURE_ENDED",
    
    "UI_TOUCH_STATE_SWIPE_BEGAN",
    "UI_TOUCH_STATE_SWIPE_CHANGED",
    "UI_TOUCH_STATE_SWIPE_CANCELLED",
    "UI_TOUCH_STATE_SWIPE_ENDED",
    
    "UI_TOUCH_STATE_DOUBLE_TAP_PREFERRED",
    
    "UI_TOUCH_STATE_DOUBLE_TAP_NONPREFERRED",
    
    "UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_BEGAN",
    "UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_CHANGED",
    "UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_CANCELLED",
    "UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_ENDED",
    "UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_BEGAN",
    "UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_CHANGED",
    "UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_CANCELLED",
    "UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_ENDED",
    
    "UI_TOUCH_STATE_KEY_BEGAN",
    "UI_TOUCH_STATE_KEY_CHANGED",
    "UI_TOUCH_STATE_KEY_CANCELLED",
    "UI_TOUCH_STATE_KEY_ENDED",
    "UI_TOUCH_STATE_KEY_REPEATED",
    
    "UI_HOVER_STATE_BEGAN",
    "UI_HOVER_STATE_CHANGED",
    "UI_HOVER_STATE_CANCELLED",
    "UI_HOVER_STATE_ENDED",
    
    "UI_TOUCH_STATE_PRESSED_BUTTON_BEGAN",
    "UI_TOUCH_STATE_PRESSED_BUTTON_CHANGED",
    "UI_TOUCH_STATE_PRESSED_BUTTON_CANCELLED",
    "UI_TOUCH_STATE_PRESSED_BUTTON_ENDED",
    
    "UI_TOUCH_STATE_NONE",
};

#define UI_TOUCH_MAX_ENTRIES (32)
struct UI_Touch_Direct {
    vec2    positions[UI_TOUCH_MAX_ENTRIES];
    float64 timestamps[UI_TOUCH_MAX_ENTRIES];
    usize   count;
    vec2 prev_position;
    vec2 position_delta;
    vec2 world_position_delta;
    
    UI_TOUCH_STATE state;
};

struct UI_Touch_Pointer {
    vec2    positions[UI_TOUCH_MAX_ENTRIES];
    float64 forces[UI_TOUCH_MAX_ENTRIES];
    float64 timestamps[UI_TOUCH_MAX_ENTRIES];
    usize   count;
    //std::vector<float64> altitude_angles;
    //std::vector<float64> azimuth_angles;
    //std::vector<vec2>    azimuth_unit_vectors;
    
    //vec2    prev_position;
    vec2 prev_position;
    vec2 position_delta;
    vec2 world_position_delta;
    //float64 prev_altitude_angle;
    float64 azimuth;
    vec2_64 azimuth_unit_vector;
    float64 altitude_angle_radians;
    float64 roll;
    
    float64 average_force;
    UI_TOUCH_STATE state;
//    UI_HOVER_STATE hover_state;
    
//    float64 z_position = 0.0;
//    float64 get_z_position(void)
//    {
//        return z_position;
//    }
//    void set_z_position(float64 z_pos)
//    {
//        z_position = z_pos;
//    }
};

struct UI_Touch {
    UI_TOUCH_TYPE type;
    UI_TOUCH_STATE state;
    union {
        // array[2] to store prev/next history
        UI_Touch_Direct  direct;
        UI_Touch_Pointer pointer;
    };

    UI_Key key;
    usize sequence_number;
    void* user_data;
    uint64 flags;
//    void* user_data_2;
//    void* user_data_3;
};

const uint32 UI_EVENT_FLAG_CONTEXT_MODE = (1 << 0);
const uint32 UI_EVENT_FLAG_OVERRIDE     = (1 << 1);

struct UI_Event {
    UI_TOUCH_STATE state_type;
    UI_TOUCH_TYPE input_type;
    UI_Key key;
    UI_Key key_sub;
    union {
        vec2 scale;
        vec2 translation;
        float32 rotation;
        char characters[16];
        struct {
            float64 azimuth;
            float64 altitude_angle_radians;
            vec2_64 azimuth_unit_vector;
        } pointer_info;
    };
    vec2 position;
    float64 z_position;

    usize count;
    
    float64 timestamp;
    uint32 flags = 0;
};

static inline void position(UI_Event* ev, vec2 v2)
{
    ev->position = v2;
}
static inline void position(UI_Event* ev, vec3 v3)
{
    ev->position = vec2(v3);
    ev->z_position = v3.z;
}
static inline vec3 position(UI_Event* ev)
{
    return vec3(ev->position, ev->z_position);
}

static inline bool cancel_input_default(UI_Event* event)
{
    return (event->flags & UI_EVENT_FLAG_CONTEXT_MODE) != 0;
}
static inline bool contains_flags(UI_Event* event, uint32 flags)
{
    return (event->flags & flags) == flags;
}
static inline bool contains_none_of_flags(UI_Event* event, uint32 flags)
{
    return (event->flags & flags) == 0;
}
static inline bool contains_some_of_flags(UI_Event* event, uint32 flags)
{
    return (event->flags & flags) != 0;
}
static inline bool contains_flags(UI_KEY_MODIFIER_FLAG src_flags, uint32 flags)
{
    return (src_flags & flags) == flags;
}
static inline bool contains_none_of_flags(UI_KEY_MODIFIER_FLAG src_flags, uint32 flags)
{
    return (src_flags & flags) == 0;
}
static inline bool contains_some_of_flags(UI_KEY_MODIFIER_FLAG src_flags, uint32 flags)
{
    return (src_flags & flags) != 0;
}
static inline bool contains_flags(uintptr_t src_flags, uint32 flags)
{
    return (src_flags & flags) == flags;
}
static inline bool contains_none_of_flags(uintptr_t src_flags, uint32 flags)
{
    return (src_flags & flags) == 0;
}
static inline bool contains_some_of_flags(uintptr_t src_flags, uint32 flags)
{
    return (src_flags & flags) != 0;
}

typedef uint32 UI_POINTER_OP;
static const UI_POINTER_OP UI_POINTER_OP_NONE  = 0;
static const UI_POINTER_OP UI_POINTER_OP_DRAW  = 1;
static const UI_POINTER_OP UI_POINTER_OP_ERASE = 2;
static const UI_POINTER_OP UI_POINTER_OP_ARROW_CREATE = 3;
static const usize UI_POINTER_OP_COUNT = 3;
static const UI_POINTER_OP UI_POINTER_OP_ARRAY[UI_POINTER_OP_COUNT] = {
    UI_POINTER_OP_DRAW,
    UI_POINTER_OP_ERASE,
    UI_POINTER_OP_ARROW_CREATE,
};

typedef enum UI_POINTER_OP_ARROW_CREATE_TYPE {
    UI_POINTER_OP_ARROW_CREATE_TYPE_DIRECTED,
    UI_POINTER_OP_ARROW_CREATE_TYPE_UNDIRECTED,
    UI_POINTER_OP_ARROW_CREATE_TYPE_DEFAULT = UI_POINTER_OP_ARROW_CREATE_TYPE_DIRECTED,
} UI_POINTER_OP_ARROW_CREATE_TYPE;


#define MAX_SIMULTANEOUS_TOUCHES (32)
struct Input_Record {
    //robin_hood::unordered_map<UI_Key, UI_Touch*>* input_map;
    robin_hood::unordered_node_map<UI_Key, UI_Touch> direct_map;
    robin_hood::unordered_node_map<UI_Key, UI_Touch> pointer_map;
    
    // input data addresses to sequence number
    uintptr_t direct_sequence_number  = 0;
    uintptr_t pointer_sequence_number = 0;
    robin_hood::unordered_node_map<UI_Key, UI_Key> direct_input_map;
    robin_hood::unordered_node_map<UI_Key, UI_Key> pointer_input_map;
    
    UI_Touch gesture_input_map[UI_GESTURE_TYPE_COUNT];
    
    //UI_Touch_Event events[128];
    //usize event_count;
    //usize first_event_idx;
    std::queue<UI_Event> event_queue;
    
    usize touches_free_count;
    usize touches_free[MAX_SIMULTANEOUS_TOUCHES];
    
    usize pointers_free_count;
    usize pointers_free[MAX_SIMULTANEOUS_TOUCHES];
    
    usize pointers_to_remove_count;
    UI_Touch pointers_to_remove[MAX_SIMULTANEOUS_TOUCHES];
    usize touches_to_remove_count;
    UI_Touch touches_to_remove[MAX_SIMULTANEOUS_TOUCHES];
    
    usize direct_count;
    UI_Touch touches[MAX_SIMULTANEOUS_TOUCHES];
    usize pointer_count;
    UI_Touch pointers[MAX_SIMULTANEOUS_TOUCHES];
    
    bool show_cursor;
    
    UI_POINTER_OP current_op     = UI_POINTER_OP_DRAW;
    UI_POINTER_OP in_progress_op = UI_POINTER_OP_NONE;
    
    void* user_data;
    
    bool key_press_state[(usize)UI_KEY_TYPE_COUNT];
    UI_KEY_MODIFIER_FLAG modifier_flags = (UI_KEY_MODIFIER_FLAG)0;
    
    struct {
        UI_TOUCH_STATE state = UI_TOUCH_STATE_NONE;
        vec2 pos = vec2(0.0f);
        float32 z_pos = 0.0f;
        float64 azimuth = 0.0;
        float64 altitude_angle_radians = 0.0;
        vec2_64 azimuth_unit_vector = vec2_64(1.0);
    } hover;
    
    float64 press_t_start = 0.0f;
    float64 press_t_end = 0.0f;
};

static inline void pointer_op_set_current(Input_Record* record, UI_POINTER_OP op)
{
    record->current_op = op;
}
static inline UI_POINTER_OP pointer_op_current(Input_Record* record)
{
    return record->current_op;
}
static inline void pointer_op_set_in_progress(Input_Record* record, UI_POINTER_OP op)
{
    record->in_progress_op = op;
}
static inline UI_POINTER_OP pointer_op_in_progress(Input_Record* record)
{
    return record->in_progress_op;
}

static inline bool key_is_pressed(Input_Record* record, UI_KEY_TYPE key_type)
{
    return record->key_press_state[(usize)key_type];
}
static inline bool set_key_is_pressed(Input_Record* record, UI_KEY_TYPE key_type)
{
    bool prev = record->key_press_state[(usize)key_type];
    record->key_press_state[(usize)key_type] = true;
    return prev;
}
static inline bool unset_key_is_pressed(Input_Record* record, UI_KEY_TYPE key_type)
{
    bool prev = record->key_press_state[(usize)key_type];
    record->key_press_state[(usize)key_type] = false;
    return prev;
}

static inline bool key_modifier_flags_are_set(Input_Record* record, UI_KEY_MODIFIER_FLAG modifier_flags)
{
    return (record->modifier_flags & modifier_flags) != 0;
}


struct Input;

struct Input_Deferred_Record {
    void* ctx = nullptr;
    void (*callback)(Input* input, void* ctx) = nullptr;
};
struct Input_Deferred {
    std::vector<Input_Deferred_Record> records = {};
};

struct Input {
    std::vector<Input_Record> users;
    Input_Deferred deferred = {};
};

static inline void Input_Deferred_push(Input* input, const Input_Deferred_Record& record)
{
    input->deferred.records.push_back(record);
}
static inline void Input_Deferred_process_all(Input* input)
{
    Input_Deferred* in_deferred = &input->deferred;
    auto& records = in_deferred->records;
    usize count = records.size();
    for (usize i = count; i > 0; i -= 1) {
        auto* record = &records[i - 1];
        record->callback(input, record->ctx);
    }
    records.clear();
}

void Input_init(Input* input);

void Input_end_of_frame(Input* input);

usize Input_next_free_pointer_touch(Input* input, usize uid);
usize Input_next_free_direct_touch(Input* input, usize uid);

void Input_schedule_pointer_touch_removal(Input* input, usize uid, UI_Touch* touch);
void Input_schedule_direct_touch_removal(Input* input, usize uid, UI_Touch* touch);
bool Input_find_touch_for_event(Input_Record* u_input, UI_Event* event, UI_Touch** t_el);

void* Input_set_user_data_for_touch_event(Input_Record* u_input, UI_Event* event, UI_Touch** t_el, void* user_data);
void* Input_get_user_data_for_touch_event(Input_Record* u_input, UI_Event* event, UI_Touch** t_el);

uint64 Input_set_flags_for_touch_gesture_event(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Event* event, UI_Touch** t_el, uint64 flags);
uint64 Input_get_flags_for_touch_gesture_event(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Event* event, UI_Touch** t_el);

void* Input_get_user_data_for_touch(Input_Record* u_input, UI_Touch* t_el);
void* Input_set_user_data_for_touch(Input_Record* u_input, UI_Touch* t_el, void* user_data);
void Input_clear_user_data_for_touch(Input_Record* u_input, UI_Touch* t_el);

void* Input_set_user_data_for_touch_gesture_event(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Event* event, UI_Touch** t_el, void* user_data);
void* Input_get_user_data_for_touch_gesture_event(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Event* event, UI_Touch** t_el);
void* Input_set_user_data_for_touch_gesture(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Touch** t_el, void* user_data);
void* Input_get_user_data_for_touch_gesture(Input_Record* u_input, UI_GESTURE_TYPE gesture, UI_Touch** t_el);


void UI_Event_printf(UI_Event* event);



extern_link_end()


void Input_push_event(Input* input, usize uid, const UI_Event& event);

void Input_push_event(Input_Record* input_record, const UI_Event& event);

bool Input_poll_event(Input* input, usize uid, UI_Event* event);

bool Input_poll_event(Input_Record* input_record, UI_Event* event);

void text_input_clear(void);

bool text_in_buf_is_equal(const mtt::String& against);

#define MTT_CHECK_MODIFIER_KEY(key__, key_comp__) ((( (UI_KEY_MODIFIER_FLAG) key__ ) & key_comp__ ) != 0)

struct MTT_Core;
void handle_dyvar(MTT_Core* core, mtt::String str_cmd);



#endif /* input_hpp */
