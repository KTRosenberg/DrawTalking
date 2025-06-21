//
//  drawtalk_ui.hpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/18/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_ui_hpp
#define drawtalk_ui_hpp

#include "drawtalk_shared_types.hpp"
#include "collision.hpp"
#include "make_the_thing.h"
#include "render_layer_labels.hpp"
#include "color.hpp"
#include "thing.hpp"

namespace dt {

struct UI_Recording_Indicator {
    bool is_on;
    vec2 position;
    vec4 color;
};

struct Speech_Token;

struct Thing_Update {
    //mtt::Thing_ID thing_id = mtt::Thing_ID_INVALID;
    Speech_Token* token = nullptr;

    // valid, timestamp
    mtt::Map_Stable<mtt::Thing_ID, robin_hood::pair<bool, float64>> mappings;
    usize mapping_count = 0;
    
    //float64 timestamp = 0.0;
    
};

struct Text_Panel_Word {
    mtt::String text;
    float32 bounds[4];
    vec4 color;
    uint64 line_idx;
    bool is_selected = false;
    dt::Speech_Token* token = nullptr;
    
    bool part_of_contraction = false;
    usize matching_contraction_idx = -1ull;
    
    Thing_Update update;
    
    bool is_highlight_selected = false;
    float32 computed_bounds[4];
};

//enum SELECTION_RECTANGLE_STATE {
//    SELECTION_RECTANGLE_STATE_IN_PROGRESS,
//    SELECTION_RECTANGLE_STATE_DONE,
//};

struct Selection_Intersection {
    vec2 tl;
    vec2 br;
    usize i_begin;
    usize i_end;
    bool state;
    bool head_out_of_bounds;
    bool tail_out_of_bounds;
    float head_bounds[4];
    float tail_bounds[4];
    float first_bounds[4];
};
bool Selection_Intersection_is_valid(Selection_Intersection& selection_intersection);

struct Selection_Rectangle {
    vec2 end = vec2(0.0f);
    vec2 start = vec2(0.0f);
    vec2 extent = vec2(0.0f);
    vec2 init_origin = vec2(0.0f);
    float32 min_area_to_activate = 16.0f;
    float32 area = 0.0f;
    vec2 curr = vec2(0.0f);
    Selection_Intersection intersection_state = {};
    //SELECTION_RECTANGLE_STATE state = SELECTION_RECTANGLE_STATE_IN_PROGRESS;
};

extern const vec4 TEXT_PANEL_DEFAULT_COLOR;
extern const vec4 UI_BORDER_SELECTION_COLOR;
extern const vec4 UI_TOUCH_SELECTION_COLOR;
extern const vec4 TEXT_HIGHLIGHT_COLOR;
const usize INVALID_SELECTION_INTERSECTION_IDX = ULLONG_MAX;

struct Text_Panel {
    //mtt::Box bounds = {};
    vec2 offset = vec2(0.0f, 0.0f);
    
    usize row_sep_factor = 2;
    
    usize row_count = 0;
    isize invisible_row_count = 0;
    
    dt::Dynamic_Array<Text_Panel_Word> text;
    
    mtt::Map<UI_Key, Selection_Rectangle> selection_rectangles = {};
    mtt::Array<Selection_Rectangle, 20> selection_rectangles_to_handle = {};
    usize first_selected_i = INVALID_SELECTION_INTERSECTION_IDX;
    //mtt::Array<Selection_Rectangle, 20> selection_rectangles = {};
   // mtt::Array<Selection_Rectangle, 20> selection_rectangles_to_handle = {};
    
    float32 vertical_scroll_offset = 0.0;
    
    /*
     nvgFontSize(vg, 28.0f);
     nvgFontFace(vg, "sans");
     nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
     */
    float32 font_size = 22.0f;
    cstring font_face = "sans";

    vec4 color = TEXT_PANEL_DEFAULT_COLOR;

    int align = NVG_ALIGN_LEFT|NVG_ALIGN_TOP;
    
    bool must_update = false;
    
    bool speech_is_overridden_ = false;
    
    bool was_edited = false;
    inline bool is_overriding_speech()
    {
        return this->speech_is_overridden_;
    }
    
    inline void set_is_overriding_speech(bool state)
    {
        this->speech_is_overridden_ = state;
        
        //Speech_set_active_state(&mtt_core_ctx()->speech_system.info_list[0], !state);
        
        
        //Speech_set_active_state(&mtt_core_ctx()->speech_system.info_list[0], !state);
        
        //Speech_discard()
    }
    
    
    void init()
    {
        Array_init(&this->selection_rectangles_to_handle);
    }
    void clear();
    void reset();
    
    bool send = false;
    void send_text_input_confirm_command()
    {
        this->send = true;
    }
    
    float row_offset = 0.0f;
    
    bool delete_mode_on = false;
    
    mtt::String text_input = "";
    bool do_send = false;
    bool received_new_text_input = false;
    
    mtt::String generated_message = "";
    
    inline void send_text_input(mtt::String msg, bool do_send)
    {
        this->text_input = msg;
        this->do_send = do_send;
        this->received_new_text_input = true;
        this->was_edited = true;
    }
    
    void move_selections_left()
    {
        if (this->text.empty()) {
            return;
        }
        
        for (usize i = 1; i < this->text.size(); i += 1) {
            this->text[i - 1].is_selected = this->text[i].is_selected;
            this->text[i].is_selected = false;
        }
    }
    
    void clear_selections()
    {
        for (usize i = 0; i < this->text.size(); i += 1) {
            this->text[i].is_selected = false;
        }
    }
    
    void move_selections_right()
    {
        if (this->text.empty()) {
            return;
        }
        
        for (usize i = this->text.size() - 1; i > 0; i -= 1) {
            
            this->text[i].is_selected = this->text[i - 1].is_selected;
            this->text[i - 1].is_selected = false;
        }
        
        text_input_clear();
    }
    
    void move_selections_up()
    {
        if (this->text.empty()) {
            return;
        }
        
        static mtt::Set<usize> already_picked;
        already_picked.clear();
        
        isize i = 0;
        for (; i < this->text.size(); i += 1) {
            if (this->text[i].line_idx == 0) {
                this->text[i].is_selected = false;
            } else {
                break;
            }
        }
        for (; i < this->text.size(); i += 1) {
            auto& word = this->text[i];
            if (!word.is_selected) {
                continue;
            }
            
            usize line_idx = word.line_idx;
            
            Text_Panel_Word* word_cand = nullptr;
            usize idx = -1ull;
            float32 dist2 = POSITIVE_INFINITY;
            for (isize j = i - 1; j >= 0; j -= 1) {
                auto& word_cmp = this->text[j];
                if (word_cmp.line_idx == line_idx) {
                    continue;
                }
                
                
                float32 dist2_comp = m::dist_squared(vec3(word_cmp.bounds[0], word_cmp.bounds[1], 0.0f), vec3(word.bounds[0], word.bounds[1], 0.0f));
                if (dist2_comp <= dist2) {
                    if (already_picked.find(j) == already_picked.end()) {
                        dist2 = dist2_comp;
                        word_cand = &word_cmp;
                        idx = j;
                    }
                } else {
                    break;
                }
            }
            
            if (word_cand != nullptr) {
                word.is_selected = false;
                word_cand->is_selected = true;
                already_picked.insert(idx);
            }
        }
        
        text_input_clear();
    }
    
    void move_selections_down()
    {
        if (this->text.empty()) {
            return;
        }
        
        static mtt::Set<usize> already_picked;
        already_picked.clear();
        
        isize i = this->text.size() - 1;
        for (; i >= 0; i -= 1) {
            if (this->text[i].line_idx == this->text.back().line_idx) {
                this->text[i].is_selected = false;
            } else {
                break;
            }
        }
        for (; i >= 0; i -= 1) {
            auto& word = this->text[i];
            if (!word.is_selected) {
                continue;
            }
            
            usize line_idx = word.line_idx;
            
            Text_Panel_Word* word_cand = nullptr;
            usize idx = -1ull;
            float32 dist2 = POSITIVE_INFINITY;
            for (isize j = i + 1; j < this->text.size(); j += 1) {
                auto& word_cmp = this->text[j];
                if (word_cmp.line_idx == line_idx) {
                    continue;
                }
                
                
                float32 dist2_comp = m::dist_squared(vec3(word_cmp.bounds[0], word_cmp.bounds[1], 0.0f), vec3(word.bounds[0], word.bounds[1], 0.0f));
                if (dist2_comp <= dist2) {
                    if (already_picked.find(j) == already_picked.end()) {
                        dist2 = dist2_comp;
                        word_cand = &word_cmp;
                        idx = j;
                    }
                } else {
                    break;
                }
            }
            
            if (word_cand != nullptr) {
                word.is_selected = false;
                word_cand->is_selected = true;
                already_picked.insert(idx);
            }
        }
        
        text_input_clear();
    }
    
};

void update_text();

struct UI_Slider {
    float32 offset = 0.0f;
    float32 lower_bound = 0.0f;
    float32 upper_bound = 1.0f;
    bool show_labels    = true;
};


struct Panel {
    mtt::Thing_ID thing = mtt::Thing_ID_INVALID;
    mtt::Box bounds = {};
    vec2 offset = vec2(0.0f, 0.0f);
    Text_Panel text = {};
    
    //dt::Dynamic_Array<vec2> pen_selections;
    vec2 most_recent_selected_canvas_position_touch;
    vec2 most_recent_selected_world_position_touch;
    vec2 most_recent_selected_canvas_position_pen;
    vec2 most_recent_selected_world_position_pen;
    bool is_selected_by_touch_prev = false;
    bool is_selected_by_touch = false;
    bool is_selected_by_pen_prev = false;
    bool is_selected_by_pen = false;
    

    
    bool is_active = false;

    
    UI_TOUCH_STATE touch_state = UI_TOUCH_STATE_NONE;
    float64 touch_began_time = 0.0;
    UI_TOUCH_STATE pen_state   = UI_TOUCH_STATE_NONE;
    float64 pen_began_time = 0.0;

    
    dt::Dynamic_Array<Panel> sub;
    
    UI_Slider slider_for_text = {};
    bool slider_active = false;
    vec2 slider_prev_pos = vec2(0.0f);
    UI_Key slider_input_key = UI_Key_INVALID;
    
    inline mtt::Box scroll_bounds(void)
    {
        float32 w = text.offset.x * 0.25;
        float32 pad = text.offset.x * 0.2;
        return (mtt::Box){
            .tl = vec2(this->bounds.tl + vec2(text.offset.x - w - pad, text.offset.y)),
            .dimensions =
            vec2(w, this->bounds.dimensions.y - text.offset.y * 2)};
    }
    
    UI_Key most_recent_touch_key = 0;
};


typedef enum MARGIN_PANEL_LOCATION {
    MARGIN_PANEL_LOCATION_LEFT,
    MARGIN_PANEL_LOCATION_RIGHT,
    
    MARGIN_PANEL_LOCATION_COUNT
} MARGIN_PANEL_LOCATION;

typedef uint64 UI_BUTTON_FLAG;

struct UI_Button {
    mtt::Thing_ID thing = mtt::Thing_ID_INVALID;
    mtt::String label = "";
    mtt::String name = "";
    vec2 saved_center = vec2(0.0f, 0.0f);
    bool position_is_modified = false;
    vec2 saved_dimensions = vec2(0.0f, 0.0f);
    float32 alpha = 1.0;
    bool show_label = true;
    UI_BUTTON_FLAG flags = 0;
    bool alt_state = false;
};





struct UI_Dock {
    struct Entry_Config {
        LAYER_LABEL render_layer;
        bool show_label = true;
        bool hsv_color = false;
        bool override_box = false;
        mtt::Box box;
        bool ignore_graphics = false;
    };
    mtt::Thing_ID thing_group = mtt::Thing_ID_INVALID;
    dt::Dynamic_Array<UI_Button*> buttons;
    dt::Dynamic_Array<mtt::String> labels;
    mtt::Map<mtt::String, usize> label_to_index;
    mtt::Map<mtt::String, vec4> label_to_color;
    mtt::Map<mtt::String, UI_Button*> label_to_button;
    mtt::Map<mtt::String, Entry_Config> label_to_config;
    float32 button_font_size = 11.0f;
};

struct UI_Feedback_Node {
    static mem::Pool_Allocation pool;
    
    MTT_NODISCARD
    static UI_Feedback_Node* make(void)
    {
        return mem::alloc_init<UI_Feedback_Node>(&UI_Feedback_Node::pool.allocator);
    }
    
    static void destroy(UI_Feedback_Node** mem)
    {
        mem::deallocate<UI_Feedback_Node>(&UI_Feedback_Node::pool.allocator, *mem);
        *mem = nullptr;
    }
};

struct Word_Substitution_Menu {
    mtt::Thing_ID thing_id = mtt::Thing_ID_INVALID;
    //uint64 priority_layer = 0;
    bool is_active = false;
    bool touch_began = false;
    bool pen_began   = false;
    bool pen_ended   = false;
    bool touch_ended = false;
    vec2 pen_end_canvas_pos = vec2(0.0f);
    vec2 touch_end_canvas_pos = vec2(0.0f);
    
    inline void set_is_active(void)
    {
        this->is_active = true;
    }
    inline void set_is_inactive(void)
    {
        this->is_active = false;
    }
};

struct Panel_Base {
    mtt::Thing_ID thing = mtt::Thing_ID_INVALID;
    mtt::Box bounds = {};
    vec2 offset;
    UI_TOUCH_STATE touch_state = UI_TOUCH_STATE_NONE;
    float64 touch_began_time = 0.0;
    UI_TOUCH_STATE pen_state   = UI_TOUCH_STATE_NONE;
    float64 pen_began_time = 0.0;
    
    bool is_active = false;
    
    vec4 color = color::BLACK;
};

struct Bottom_Panel {
    Panel_Base base = {};
};

struct Top_Panel {
    Panel_Base base = {};
    mtt::Collision_System collision_system_world = {};
    mtt::Collision_System collision_system_canvas = {};
    mtt::Collision_System_Group_World_Canvas collision_system_group = {
        .world = &collision_system_world,
        .canvas = &collision_system_canvas
    };
    mtt::Camera cam = {};
    
    UI_Key is_selected_key = UI_Key_INVALID;
};

struct Scroll_Panel {
    Panel_Base base = {};
    UI_Slider axis[1];
    
};

struct Active_Things_Panel {
    Scroll_Panel scroll_base = {};
    mtt::Dynamic_Array<mtt::Thing_Proxy_ID> things = {};
};

mtt::Input_Handler_Return Scroll_Panel_on_touch_input_began(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags);
void Scroll_Panel_on_touch_input_moved(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags);
void Scroll_Panel_on_touch_input_ended(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags);

mtt::Input_Handler_Return Scroll_Panel_on_pen_input_began(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags);
void Scroll_Panel_on_pen_input_moved(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags);
void Scroll_Panel_on_pen_input_ended(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags);

void Scroll_Panel_init(Scroll_Panel* self, mtt::World* world);

struct Speech_Property;

enum CONTEXT_VIEW_MODE {
    CONTEXT_VIEW_MODE_SEARCH,
    CONTEXT_VIEW_MODE_RULES,
    CONTEXT_VIEW_MODE_ACTIONS,
    CONTEXT_VIEW_MODE_DEFAULT = CONTEXT_VIEW_MODE_SEARCH,
};

constexpr const float64 CONTEXT_VIEW_T_INTERVAL = 0.25;
struct Context_View {
    mtt::Thing_ID thing_id = mtt::Thing_ID_INVALID;
    
    float64 time_elapsed_since_update = CONTEXT_VIEW_T_INTERVAL;
    
    CONTEXT_VIEW_MODE mode = CONTEXT_VIEW_MODE_DEFAULT;
    CONTEXT_VIEW_MODE mode_prev = CONTEXT_VIEW_MODE_DEFAULT;
    
    bool selected_this_frame = false;
    
    struct Label_Record {
        mtt::String label;
        DT_Rule_Query* query = nullptr;
        dt::Speech_Property* prop_ref = nullptr;
        
        using TYPE = Label_Record;
        
        bool operator==(TYPE& other) { return this->label == other.label; }
        bool operator<(TYPE& other)  { return this->label < other.label; }
        bool operator<=(TYPE& other) { return this->label <= other.label; }
        bool operator>=(TYPE& other) { return this->label <= other.label; }
        bool operator>(TYPE& other)  { return this->label < other.label; }
        
        bool operator==(const TYPE& other) const { return this->label == other.label; }
        bool operator<(const TYPE& other)  const { return this->label < other.label; }
        bool operator<=(const TYPE& other) const { return this->label <= other.label; }
        bool operator>=(const TYPE& other) const { return this->label <= other.label; }
        bool operator>(const TYPE& other)  const { return this->label < other.label; }
    };
    
    struct Element_Record {
        mtt::Thing_Proxy_ID proxy_id = mtt::Thing_ID_INVALID;
        usize idx = 0;
    };
    // Things to turn into proxies (update per-frame for now)
    dt::Dynamic_Array<mtt::Thing_ID> src_elements = {};
    // Map of thing to relevant proxy (update per-frame for now)
    mtt::Map_Stable<mtt::Thing_ID, Element_Record> thing_to_element = {};
    // Use to figure out if something should be remomoved from the map (update per-frame for now)
    mtt::Set<mtt::Thing_ID> elements_curr = {};
    
    // Proxies to display (update per-frame for now)
    dt::Dynamic_Array<mtt::Thing_Proxy_ID> active_elements = {};
    
    mtt::Set_Stable<mtt::Thing_ID> found_set = {};
    
    mtt::Map<mtt::String, DT_Rule_Query> queries = {};
    
    dt::Dynamic_Array<mtt::Thing_ID> things_to_display = {};
    
    dt::Dynamic_Array<DT_Rule_Query*> queries_this_frame = {};
    
    mtt::Set<mtt::String> unique_labels = {};
    
    dt::Dynamic_Array<Label_Record> labels = {};
    
    struct Rule_Element_Record {
        mtt::Script_ID ID = mtt::Script_ID_INVALID;
        mtt::Box box = {};
    };
    dt::Dynamic_Array<Rule_Element_Record> rule_els = {};
    
    struct Action_Element_Record {
        mtt::Thing_ID thing_ID = mtt::Thing_ID_INVALID;
        mtt::Thing* thing = nullptr;
        mtt::Script_ID ID = mtt::Script_ID_INVALID;
        mtt::Script_Instance* script_ref = nullptr;
        mtt::Box box = {};
        mtt::Active_Action action = {};
    };
    dt::Dynamic_Array<Action_Element_Record> action_els = {};
    dt::Dynamic_Array<mtt::Thing*> action_els_things = {};
    
    struct Proxies_To_Align_Info {
        mtt::Thing* proxy;
        float32 scale;
        bool position_set_deferred;
    };
    dt::Dynamic_Array<Proxies_To_Align_Info> thing_proxies_to_align = {};
    
    
    mtt::Box box = {};
    float32 depth = 0.0f;
    
    //DT_Rule_Query search = {};
    
    mtt::String query_str = {};
};

struct UI {
    bool changed = false;
    float32 hsv_slider = 0.0;
    float32 hsv_slider_offset = 0.0;
    UI_Recording_Indicator recording_indicator = {};
    
    Panel margin_panels[MARGIN_PANEL_LOCATION_COUNT] = {};
    usize margin_panels_count = 1;
    
    Bottom_Panel bottom_panel = {};
    Top_Panel    top_panel = {};
    Active_Things_Panel active_things_panel = {};
    
    UI_Dock dock;
    
    Word_Substitution_Menu element_menu = {};
    Context_View context_view = {};
};

bool Context_View_enable(DrawTalk* dt);
void Context_View_disable(DrawTalk* dt);



void set_ui_layer(dt::DrawTalk* ctx, dt::UI* ui, uint64 priority_layer);

void UI_draw_pre(void);
void UI_draw(void);
void UI_draw_post(void);

struct Pen {
    vec4 color;
    uint16 pixel_radius;
};

struct DT_Evaluation_Context;
struct Instruction;

static cstring THING_TYPE_SELECTION_INSTRUCTION_LABEL_PREFIX = "*";

void build_feedback_ui_from_instructions(dt::DrawTalk* dt, DT_Evaluation_Context& eval);



void rebuild_feedback_ui_from_modified_instructions(dt::DrawTalk* dt, dt::DT_Evaluation_Context& eval);


void prompt_for_different_actions(dt::DrawTalk* dt);
void leave_prompt_for_different_actions(dt::DrawTalk* dt);


#define DT_USE_WORD_LABELS_IN_FEEDBACK (0)
}

namespace dt {

// old
//static const vec4 LABEL_COLOR_NOUN      = color::SYSTEM_BLUE;
//static const vec4 LABEL_COLOR_THING_INSTANCE = LABEL_COLOR_NOUN;
//static const vec4 LABEL_COLOR_THING_TYPE = LABEL_COLOR_NOUN;
//static const vec4 LABEL_COLOR_VERB      = color::ORANGE;
//static const vec4 LABEL_COLOR_ACTION    = LABEL_COLOR_VERB;
//static const vec4 LABEL_COLOR_ADJECTIVE = vec4(1.0f, 220.0f / 255.0f,0,1.0f);
//static const vec4 LABEL_COLOR_ATTRIBUTE = LABEL_COLOR_ADJECTIVE;
//static const vec4 LABEL_COLOR_ADVERB    = vec4(1.0f, 120.0f / 255.0f,0,1.0f);
//static const vec4 LABEL_COLOR_ATTRIBUTE_MODIFIER = LABEL_COLOR_ADVERB;
//static const vec4 LABEL_COLOR_PRONOUN   = color::MAGENTA;
//static const vec4 LABEL_COLOR_TRIGGER   = color::GREEN;
//static const vec4 LABEL_COLOR_RESPONSE  = color::RED;
//static const vec4 LABEL_COLOR_VALUE_ATTRIBUTE = vec4(220.0f/255.0f,220.0f/255.0f,142.0f/255.0f,1.0);
// new
extern const vec4 LABEL_COLOR_NOUN;
extern const vec4 LABEL_COLOR_THING_INSTANCE;
extern const vec4 LABEL_COLOR_THING_TYPE //LABEL_COLOR_NOUN
;
extern const vec4 LABEL_COLOR_VERB;
extern const vec4 LABEL_COLOR_ACTION;
extern const vec4 LABEL_COLOR_ADJECTIVE;
extern const vec4 LABEL_COLOR_ATTRIBUTE;
extern const vec4 LABEL_COLOR_ADVERB;
extern const vec4 LABEL_COLOR_ATTRIBUTE_MODIFIER;
extern const vec4 LABEL_COLOR_PRONOUN;
extern const vec4 LABEL_COLOR_COREFERENCE;
extern const vec4 LABEL_COLOR_TRIGGER;
extern const vec4 LABEL_COLOR_RESPONSE;
extern const vec4 LABEL_COLOR_VALUE_ATTRIBUTE;

extern const vec4 LABEL_COLOR_PREPOSITION;
extern const vec4 LABEL_COLOR_PROPERTY;

extern const vec4 BG_COLOR_LIGHT_MODE;
extern const vec4 PEN_COLOR_LIGHT_MODE;

extern const vec4 BG_COLOR_DARK_MODE;
extern const vec4 PEN_COLOR_DARK_MODE;

extern const vec4 TEXT_SELECTED_COLOR;

vec4 bg_color_default(void);
vec4 bg_color_default_for_color_scheme(mtt::Color_Scheme_ID id);

vec4 pen_color_default(void);
vec4 pen_color_default_for_color_scheme(mtt::Color_Scheme_ID id);



extern const vec4 LABEL_COLOR_DEFAULT;

extern const vec4 SELECTION_RECTANGLE_COLOR;

extern const SD_vec4 PANEL_COLOR;

bool map_view_selected(mtt::World* world, DrawTalk* dt, vec2 canvas_pos);

mtt::Collision_System_Group_World_Canvas* map_view_collision_system_group(DrawTalk* dt);

static const float32 bottom_panel_height = 75;
    
}

#endif /* drawtalk_ui_hpp */
