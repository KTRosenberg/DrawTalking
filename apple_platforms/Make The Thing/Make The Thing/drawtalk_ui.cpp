//
//  drawtalk_ui.cpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/18/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "drawtalk_ui.hpp"
#include "drawtalk.hpp"

#include "drawtalk_world.hpp"

#include "number_words_to_values.hpp"
//#include "thing.hpp"

namespace dt {

constexpr float32 layout_padding = 4.0f;
constexpr float32 layout_padding_root = layout_padding * 8.0f;

#define FEEDBACK_ELEMENT_THING_TYPE mtt::ARCHETYPE_UI_ELEMENT

constexpr const cstring LABEL_CAUSES = "→";
constexpr const usize LABEL_CAUSES_LEN = 3;

const vec4 TEXT_PANEL_DEFAULT_COLOR  = //color::BLACK;
vec4(0.2f, 0.2f, 0.2f, 1.0f);
const vec4 UI_BORDER_SELECTION_COLOR = color::make_with_alpha(color::EARTHY_1, 1.0f);
const vec4 UI_TOUCH_SELECTION_COLOR  = color::make_with_alpha(color::EARTHY_1, 1.0f);
const vec4 TEXT_HIGHLIGHT_COLOR      = color::make_with_alpha(color::EARTHY_1, 1.0f);

const vec4 LABEL_COLOR_NOUN      = color::SYSTEM_BLUE;
const vec4 LABEL_COLOR_THING_INSTANCE = LABEL_COLOR_NOUN;
const vec4 LABEL_COLOR_THING_TYPE =  vec4(vec3(1, 75, 200) / 255.0f, 1.0f);//LABEL_COLOR_NOUN;
const vec4 LABEL_COLOR_VERB      = color::RED;//vec4(vec3(77, 34, 179) / 255.0f, 1.0f);
const vec4 LABEL_COLOR_ACTION    = LABEL_COLOR_VERB;
const vec4 LABEL_COLOR_ADJECTIVE = vec4(vec3(168, 23, 0) / 255.0f, 1.0f);
const vec4 LABEL_COLOR_ATTRIBUTE = LABEL_COLOR_ADJECTIVE;
const vec4 LABEL_COLOR_ADVERB    = LABEL_COLOR_ADJECTIVE;
const vec4 LABEL_COLOR_ATTRIBUTE_MODIFIER = LABEL_COLOR_ADVERB;
const vec4 LABEL_COLOR_PRONOUN   = vec4(vec3(1, 75, 200) / 255.0f, 1.0f);
const vec4 LABEL_COLOR_COREFERENCE = LABEL_COLOR_PRONOUN;
const vec4 LABEL_COLOR_TRIGGER   = vec4(0.6f * vec3(color::GREEN), 1.0f);
const vec4 LABEL_COLOR_RESPONSE  = color::RED;
const vec4 LABEL_COLOR_VALUE_ATTRIBUTE = vec4((0.5f * vec3(168, 23, 0)) / 255.0f, 1.0f);



const vec4 LABEL_COLOR_PREPOSITION = vec4(75.0f, 0.0f, 130.0f / 255.0f, 1.0);
const vec4 LABEL_COLOR_PROPERTY = vec4(75.0f, 0.0f, 130.0f / 255.0f, 1.0);

const vec4 BG_COLOR_LIGHT_MODE = color::WHITE;
const vec4 PEN_COLOR_LIGHT_MODE = color::BLACK;

const vec4 BG_COLOR_DARK_MODE = color::BLACK;
const vec4 PEN_COLOR_DARK_MODE = color::WHITE;

const vec4 TEXT_SELECTED_COLOR = vec4(0,0,1.0,0.12);

const vec4 LABEL_COLOR_DEFAULT = color::BLACK;

const vec4 SELECTION_RECTANGLE_COLOR = color::make_with_alpha(color::DARK_PURPLE_1 * 1.75f, 0.1f);

const SD_vec4 PANEL_COLOR = SD_vec4(0.0f, 0.0f, 0.0f, 0.05f);


void update_text()
{
    auto& ui = dt::DrawTalk::ctx()->ui;
    Panel& panel = ui.margin_panels[0];
    Text_Panel& text_panel = panel.text;
    text_panel.must_update = true;
}
void draw_text()
{
    auto& ui = dt::DrawTalk::ctx()->ui;
    Panel& panel = ui.margin_panels[0];
    Text_Panel& text_panel = panel.text;

    //bool must_update = text_panel.must_update;
    
    text_panel.must_update = false;
}

mtt::String selected_text = "";
dt::Dynamic_Array<dt::Text_Panel_Word*> selected_text_list;
Regenerate_Operation deferred_op = {};
bool has_deferred_op = false;
bool drew_this_frame = false;


bool Selection_Intersection_is_valid(Selection_Intersection& selection_intersection)
{
    return (selection_intersection.i_begin != dt::INVALID_SELECTION_INTERSECTION_IDX && selection_intersection.i_end != dt::INVALID_SELECTION_INTERSECTION_IDX);
}



void UI_draw_pre(void)
{
    
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    
    auto* vg = nvgGetGlobalContext();
    
    
    
    UI& ui = dt->ui;
    
    Panel& panel = ui.margin_panels[0];
    Text_Panel& text_panel = panel.text;
    
    mtt::World* world = dt->mtt;
    auto* core_ctx = mtt_core_ctx();
    DrawTalk_World* dt_world = static_cast<DrawTalk_World*>(core_ctx->user_data);
    dt::Control_System* control = &dt_world->controls;
    
    auto* renderer = world->renderer;
    
    auto depths = sd::depth_range_default(renderer);
    
    mtt::Circle c;
    c.radius = ((uint64)(dt->core->viewport.width / 298.0f)) * 2;
    
    selected_text.clear();
    selected_text_list.clear();
    int align = 1;
    switch (align)
    {
        case 0: // right
        {
            auto& viewport = dt->core->viewport;
            float32 width = viewport.width;
            // non-dominant - assume right-handed for now
            Panel& ndom_panel = ui.margin_panels[0];
            vec2 old_bounds = ndom_panel.bounds.tl;
            ndom_panel.bounds.tl = vec2(width - ndom_panel.bounds.dimensions.x, 0);
            if (ndom_panel.bounds.tl != old_bounds) {
                mtt::Thing* thing = dt->mtt->Thing_try_get(ndom_panel.thing);
                if (thing != nullptr) {
                    mtt::Thing_set_position(thing, vec3(ndom_panel.bounds.tl + (ndom_panel.bounds.dimensions / 2.0f), 0.0f));
                }
            }
            break;
        }
        case 1: { // top
            auto& viewport = dt->core->viewport;
            float32 width = viewport.width;
            Panel& ndom_panel = ui.margin_panels[0];
            vec2 old_bounds = ndom_panel.bounds.tl;
            ndom_panel.bounds.tl = vec2(width / 2.0f, 0.0f);
            if (ndom_panel.bounds.tl != old_bounds) {
                mtt::Thing* thing = dt->mtt->Thing_try_get(ndom_panel.thing);
                if (thing != nullptr) {
                    mtt::Thing_set_position(thing, vec3(ndom_panel.bounds.tl + (ndom_panel.bounds.dimensions / 2.0f), 0.0f));
                }
            }
            break;
        } // top
    }
    nvgSave(vg);
    
    using m::value_ptr;
    
    mat4 identity(1.0f);
    //
    //
    //
    //nvgSetViewTransform(nvg_ctx, value_ptr(world->cam.view_transform));
    nvgSetViewTransform(vg, value_ptr(identity));
    
    
    //
    mat4 M = identity;
    //
    //NVG_save_model_transform(nvg_ctx, value_ptr(M));
    
    nvgSetModelTransform(vg, value_ptr(M));
    
    if (true /*|| Speech_get_active_state(&dt->core->speech_system.info_list[0]) */) {
        nvgSave(vg);
        
        nvgFontSize(vg, text_panel.font_size);
        nvgFontFace(vg, text_panel.font_face);
        nvgTextAlign(vg, text_panel.align);
        float lineh = 0;
        nvgTextMetrics(vg, NULL, NULL, &lineh);
        
        sd::set_render_layer(world->renderer, LAYER_LABEL_DYNAMIC_CANVAS);
        
        //sd::set_render_layer(renderer, LAYER_LABEL_CANVAS_PER_FRAME);
        vec4 bg_panel_color = vec4(vec3(0.97f), 1.0f);
        {
            auto& ui_top_panel = dt->ui.top_panel;
            sd::save(renderer);
            
            sd::set_color_rgba_v4(renderer, bg_panel_color);
            sd::begin_drawable(renderer); {
                sd::begin_polygon_no_new_drawable(renderer);
                sd::rectangle(renderer,
                              ui_top_panel.base.bounds.tl,
                              ui_top_panel.base.bounds.dimensions, -999.0f);
                sd::rectangle(renderer,
                              panel.bounds.tl,
                              panel.bounds.dimensions, -999.0f);
                sd::end_polygon_no_new_drawable(renderer);
                sd::end_drawable(renderer);
            
                // MARK: scroll
                {
                    auto scroll_bounds = panel.scroll_bounds();
                    vec2 scroll_top = scroll_bounds.tl;
                    vec2 dim = scroll_bounds.dimensions;
                    
                    sd::set_color_rgba(renderer, SD_vec4(SD_vec3(0.0f, 0.0f, 0.0f), 0.05f));

                    {
//                        if (panel.touch_state == UI_TOUCH_STATE_BEGAN) {
//
//                        } else if (panel.touch_state == UI_TOUCH_STATE_MOVED) {
//
//                        }
                        if (panel.slider_active) {
                            sd::set_color_rgba(renderer, SD_vec4(SD_vec3(0.0f, 0.0f, 0.0f), 0.1f));
                        } else {
                            text_panel.vertical_scroll_offset = m::min(m::max(0.0f, text_panel.vertical_scroll_offset), text_panel.row_offset);
                        }
                        sd::begin_drawable(renderer);
                        sd::begin_polygon_no_new_drawable(renderer);
                        sd::rectangle(renderer, scroll_top, dim, nexttoward(depths[1], depths[0])-1);
                        if (text_panel.invisible_row_count > 0) {
                            const auto& word = text_panel.text.back();
                            //const float half_height = (word.bounds[3] - word.bounds[1]) * 0.5;
                            sd::set_color_rgba(renderer, SD_vec4(0.5f, 0.5f, 0.5f, 0.2f));
//                            if (text_panel.vertical_scroll_offset > half_height) {
                            // at top
//                                sd::rectangle(renderer, scroll_top + vec2(0.0f, dim.y - dim.x), vec2(dim.x, dim.x), depths[1]);
//                            }
//                            if (text_panel.vertical_scroll_offset < text_panel.row_offset - half_height) {
                            // at bottom
//                                sd::rectangle(renderer, scroll_top, vec2(dim.x, dim.x), depths[1]);
//                            }
                            const vec2 more_below = scroll_top + vec2(0.0f, dim.y - dim.x);
                            const vec2 more_above = scroll_top;
                            const auto alpha = m::clamp(text_panel.vertical_scroll_offset, 0.0f, text_panel.row_offset) / text_panel.row_offset;
                            const vec2 top_pos = m::lerp(more_below, more_above, alpha);

                            sd::rectangle(renderer, top_pos, vec2(dim.x, dim.x), depths[1]-1);
                        }
                        sd::end_polygon_no_new_drawable(renderer);
                        sd::end_drawable(renderer);
                    }
                }
                

            }
            
            sd::restore(renderer);
                
                
        }
    
        mtt::String str = "";
        
        sd::begin_polygon(world->renderer);
        
// MARK: draw text panel
        const bool text_panel_text_is_empty = text_panel.text.empty();
        const float32 text_panel_alpha = text_panel.color.a;
        if constexpr ((false)){
            static float64 time_elapsed = 0.0;
            if (!text_panel_text_is_empty) MTT_LIKELY {
                time_elapsed += core_ctx->time_delta_seconds * 8;
                time_elapsed = m::min(time_elapsed, 1.0);
                nvgBeginPath(vg);
                nvgFillColor(vg, nvgRGBAf(text_panel.color.r,text_panel.color.g,text_panel.color.b,text_panel_alpha * time_elapsed));
                nvgRoundedRect(vg, panel.bounds.tl.x, panel.bounds.tl.y, panel.bounds.dimensions.x, panel.bounds.dimensions.y, 1.0);
                nvgFill(vg);
            } else if (time_elapsed > 0.0) {
                time_elapsed -= m::min(time_elapsed, core_ctx->time_delta_seconds * 8);
                time_elapsed = m::max(0.0, time_elapsed);
                nvgBeginPath(vg);
                nvgFillColor(vg, nvgRGBAf(text_panel.color.r,text_panel.color.g,text_panel.color.b,text_panel_alpha * time_elapsed));
                nvgRoundedRect(vg, panel.bounds.tl.x, panel.bounds.tl.y, panel.bounds.dimensions.x, panel.bounds.dimensions.y, 1.0);
                nvgFill(vg);
            } else MTT_LIKELY {
                
            }
        } else {
            
            
                //            nvgBeginPath(vg);
                //            nvgFillColor(vg, nvgRGBAf(text_panel.color.r,text_panel.color.g,text_panel.color.b,text_panel_alpha));
                //            nvgRoundedRect(vg, panel.bounds.tl.x, panel.bounds.tl.y, panel.bounds.dimensions.x, panel.bounds.dimensions.y, 1.0);
                //            nvgFill(vg);
                const vec4 divider_color = vec4(vec3(color::WHITE) * 0.75f, 1.0f);
                const float32 divider_gap = 32.0f;
                
                
                //            sd::set_color_rgba_v4(renderer,
                //                                  vec4(vec3(dt::bg_color_default()), 0.4f));
                //            sd::rectangle(renderer, panel.bounds.tl, panel.bounds.dimensions, 1.0f);
                sd::set_color_rgba_v4(renderer,
                                      divider_color);
                sd::rectangle(renderer, panel.bounds.tl + vec2(0.0f, panel.bounds.dimensions.y), vec2(panel.bounds.dimensions.x, 2.0f), 1.0f);
                
                sd::set_color_rgba_v4(renderer,
                                      dt::bg_color_default());
                // TODO:
                //sd::rectangle(renderer, ui.top_panel.base.bounds.tl, vec3(ui.top_panel.base.bounds.dimensions, 2.0f), 1.0f);
                sd::set_color_rgba_v4(renderer,
                                      divider_color);
            
                
                sd::rectangle(renderer, vec2(0.0f, ui.top_panel.base.bounds.dimensions.y), vec2(ui.top_panel.base.bounds.dimensions.x, 2.0f), 1.0f);
                
                sd::set_color_rgba_v4(renderer,
                                      divider_color);
                // center divider
                sd::rectangle(renderer, panel.bounds.tl - vec2(1.0f, -divider_gap), vec2(2.0f, panel.bounds.dimensions.y - (divider_gap * 2.0f)), 1.0f);
                
                const float32 bottom_panel_height = dt::bottom_panel_height;
                sd::rectangle(renderer, vec2(0.0f, core_ctx->viewport.height - (bottom_panel_height)), vec2(core_ctx->viewport.width, 2.0f), depths[1]);
            
                sd::set_color_rgba_v4(renderer, color::make_with_alpha(bg_panel_color, 0.75f));
                sd::rectangle(renderer, vec2(0.0f, 2.0f + core_ctx->viewport.height - (bottom_panel_height)), vec2(core_ctx->viewport.width, core_ctx->viewport.height - bottom_panel_height) - 2.0f, depths[1]-1);
        }
        sd::end_polygon(renderer);
        nvgRestore(vg);
        
    }
    nvgRestore(vg);
}

void UI_draw(void)
{
//#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
//    if (dt::drew_this_frame) {
//        return;
//    }
//    dt::drew_this_frame = true;
//#endif
    
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    
    auto* vg = nvgGetGlobalContext();
    

    
    UI& ui = dt->ui;
    
    Panel& panel = ui.margin_panels[0];
    Text_Panel& text_panel = panel.text;
    
    mtt::World* world = dt->mtt;
    auto* core_ctx = mtt_core_ctx();
    DrawTalk_World* dt_world = static_cast<DrawTalk_World*>(core_ctx->user_data);
    dt::Control_System* control = &dt_world->controls;
    Input_Record* u_input = &core_ctx->input.users[0];

    mtt::Circle c;
    c.radius = ((uint64)(dt->core->viewport.width / 298.0f)) * 2;
    
    nvgSave(vg);
    
    using m::value_ptr;
    
    mat4 identity(1.0f);
    //
    //
    //
    
    //nvgSetViewTransform(vg, value_ptr(identity));
    
    
    //
    
    float32 scroll_offset_y = text_panel.vertical_scroll_offset;
    //scroll_offset_y = m::sin01(core_ctx->time_seconds) * 100.0;
    
    mat4 M = identity;
    nvgSetModelTransform(vg, value_ptr(M));
    
    {
        nvgSave(vg);
        
        nvgFontSize(vg, text_panel.font_size);
        nvgFontFace(vg, text_panel.font_face);
        nvgTextAlign(vg, text_panel.align);
        float lineh = 0;
        nvgTextMetrics(vg, NULL, NULL, &lineh);
        
        
    
        mtt::String str = "";
        
        
        float32 x_off = panel.bounds.tl.x;
        

        bool something_selected_with_touch = false;
        bool something_selected_with_pen = false;
        
        bool pen_collided_with_word = false;
        bool touch_collided_with_word = false;
        
        const usize selection_count = dt->selection_map.size();
        const usize touch_count_active = control->touch.count_active;
        
        auto& selections = dt->selection_map;
        bool existing_thing_selected = (selection_count != 0);
        if (selection_count == 1) {
            mtt::Thing_ID selected_id = selections.begin()->second.thing;
            if (selected_id == panel.thing) {
                existing_thing_selected = false;
            }
        }
        bool word_selected_with_pen_prev_state = false;
        Text_Panel_Word* word_selected_with_pen = nullptr;
        
        const bool text_panel_text_is_empty = text_panel.text.empty();
        bool first_selection_intersection_item_was_already_selected = false;

        if (!text_panel_text_is_empty) {
            

//            for (usize i = 0; i < text_panel.text.size(); i += 1) {
//                auto& word = text_panel.text[i];
//                str += word.text + ": " + std::to_string(word.line_idx) + "\n";
//
//
//                auto& color = word.color;
//
//                nvgFillColor(vg, nvgRGBf(color[0], color[1], color[2]));
//
//                cstring CS = word.text.c_str();
//                nvgText(vg, word.bounds[0], word.bounds[1] - text_panel.row_offset, CS, NULL);
//            }

            
//            if (panel.is_selected_by_pen) {
//                pos = panel.most_recent_selected_canvas_position;
//            }
            
            Regenerate_Operation regen_op;
            regen_op.type = Regenerate_Operation::TYPE::CONFIRM;
            if (text_panel.delete_mode_on) {
                regen_op.type = Regenerate_Operation::TYPE::DELETE;
            }
#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
            bool defer_deselection = false;
#endif
            
            
            vec2 top = panel.bounds.tl;// + text_panel.offset;
            
            
            for (usize i = 0; i < text_panel.text.size(); i += 1) {
                auto& word = text_panel.text[text_panel.text.size() - i - 1];
                word.is_highlight_selected = false;
            }
            Selection_Rectangle* r;
            bool is_finished_selection_rectangle = false;
            if (!panel.text.selection_rectangles_to_handle.empty()) {
                r = panel.text.selection_rectangles_to_handle.begin();
                is_finished_selection_rectangle = true;
            } else if (!panel.text.selection_rectangles.empty()) {
                r = &panel.text.selection_rectangles.begin()->second;
            } else {
                goto LABEL_NO_RECTS;
            }
            {
                
                vec2 tl = r->start;
                vec2 dimensions = r->extent;
                vec2 br = tl + dimensions;
                
                r->intersection_state = {
                    .tl = tl,
                    .br = br,
                    .i_begin = dt::INVALID_SELECTION_INTERSECTION_IDX,
                    .i_end = dt::INVALID_SELECTION_INTERSECTION_IDX,
                    .state = false,
                    .head_out_of_bounds = true,
                    .tail_out_of_bounds = true
                };
                
                float box_first[4] = {
                    r->curr.x,
                    r->curr.y,
                    POSITIVE_INFINITY,
                    POSITIVE_INFINITY,
                };
                float box_last[4] = {
                    NEGATIVE_INFINITY,
                    NEGATIVE_INFINITY,
                    r->curr.x,
                    r->curr.y
                };
                
                Selection_Intersection& selection_intersection = r->intersection_state;
                usize i_first_visible = ULLONG_MAX;
                usize i_last_visible = 0;
                {
                    usize i = 0;
                    for (; i < text_panel.text.size(); i += 1) {
                        auto& word = text_panel.text[i];
                        
                        float half_height = (word.bounds[3] - word.bounds[1]) / 2.0f;
                        
                        
                        float bounds[4];
                        bounds[0] = x_off + word.bounds[0];
                        bounds[1] = (word.bounds[1] - text_panel.row_offset) + scroll_offset_y;
                        bounds[2] = x_off + word.bounds[2];
                        bounds[3] = (word.bounds[3] - text_panel.row_offset - half_height) + scroll_offset_y;
                        
                        if (bounds[3] < (panel.bounds.tl.y + text_panel.offset.y)) {
                            continue;
                        }
                        i_first_visible = m::min(i_first_visible, i);
                        if ((bounds[3] > panel.bounds.tl.y + panel.bounds.dimensions.y)) {
                            break;
                        }
                        i_last_visible = i;
                        
                        if (panel.text.first_selected_i != dt::INVALID_SELECTION_INTERSECTION_IDX) {
                            if (i == panel.text.first_selected_i) {
                                memcpy(selection_intersection.first_bounds, bounds, sizeof(float[4]));
                            }
                        }
                        
                        {
                            if (mtt::Box_Box_intersection(&(bounds[0]), &(bounds[2]), &(box_first[0]), &(box_first[2]))) {
                                selection_intersection.state = true;
                                selection_intersection.i_begin = i;
                                memcpy(selection_intersection.head_bounds, bounds, sizeof(float[4]));
                                if (panel.text.first_selected_i == dt::INVALID_SELECTION_INTERSECTION_IDX) {
                                    panel.text.first_selected_i = i;
                                    memcpy(selection_intersection.first_bounds, bounds, sizeof(float[4]));
                                }
                                selection_intersection.i_end = i;

                                memcpy(selection_intersection.tail_bounds, bounds, sizeof(float[4]));
                                
                                break;
                            }
                        }
                    }
                    usize pass1_i = i;
                    
                    for (i = text_panel.text.size(); i > 0 && i > pass1_i; i -= 1) {
                        auto& word = text_panel.text[i - 1];
                        
                        float half_height = (word.bounds[3] - word.bounds[1]) / 2.0f;
                        
                        
                        float bounds[4];
                        bounds[0] = x_off + word.bounds[0];
                        bounds[1] = (word.bounds[1] - text_panel.row_offset) + scroll_offset_y;
                        bounds[2] = x_off + word.bounds[2];
                        bounds[3] = (word.bounds[3] - text_panel.row_offset - half_height) + scroll_offset_y;
                        
                        if (bounds[3] < (panel.bounds.tl.y + text_panel.offset.y)) {
                            break;
                        }
                        if ((bounds[3] > panel.bounds.tl.y + panel.bounds.dimensions.y)) {
                            continue;
                        }
                        i_last_visible = m::max(i_last_visible, i - 1);
                        
                        if (panel.text.first_selected_i != dt::INVALID_SELECTION_INTERSECTION_IDX) {
                            if (i - 1 == panel.text.first_selected_i) {
                                memcpy(selection_intersection.first_bounds, bounds, sizeof(float[4]));
                            }
                        }
                        
                        {
                            if (mtt::Box_Box_intersection(&(bounds[0]), &(bounds[2]), &(box_last[0]), &(box_last[2]))) {
                                selection_intersection.i_end = i - 1;
                                memcpy(selection_intersection.tail_bounds, bounds, sizeof(float[4]));
                                break;
                            }
                        }
                    }
                }
                
                if (Selection_Intersection_is_valid(selection_intersection)) {
                    first_selection_intersection_item_was_already_selected = text_panel.text[panel.text.first_selected_i].is_selected;
                    
                    usize i_first = selection_intersection.i_begin;
                    usize i_last = selection_intersection.i_end;
                    
                    if (panel.text.first_selected_i < i_first_visible) {
                        i_first = panel.text.first_selected_i;
                        
                        selection_intersection.head_out_of_bounds = true;
                    } else {
                        selection_intersection.head_out_of_bounds = false;
                        
                        if (i_first > panel.text.first_selected_i) {
                            i_first = panel.text.first_selected_i;
                            selection_intersection.i_begin = i_first;
                            memcpy(selection_intersection.head_bounds, selection_intersection.first_bounds, sizeof(float[4]));
                        }
                    }
                    if (panel.text.first_selected_i > i_last_visible) {
                        i_last = panel.text.first_selected_i;
                        selection_intersection.tail_out_of_bounds = true;
                    } else {
                        selection_intersection.tail_out_of_bounds = false;
                        if (i_last < panel.text.first_selected_i) {
                            i_last = panel.text.first_selected_i;
                            selection_intersection.i_end = i_last;
                            memcpy(selection_intersection.tail_bounds, selection_intersection.first_bounds, sizeof(float[4]));
                        }
                    }
                    
                    
                    for (usize i = i_first; i <= i_last; i += 1) {
                        auto& word = text_panel.text[i];
                        word.is_highlight_selected = true;
                        if (is_finished_selection_rectangle) {
                            //if (core_ctx->application_configuration.text_selection_is_toggle)
                            {
                                //word.is_selected = !word.is_selected;
                                word.is_selected = !first_selection_intersection_item_was_already_selected;
                            }
//                            else {
//                                switch (pointer_op_current(u_input)) {
//                                    default: {
//                                        MTT_FALLTHROUGH;
//                                    }
//                                    case UI_POINTER_OP_DRAW: {
//                                        word.is_selected = true;
//                                        break;
//                                    }
//                                    case UI_POINTER_OP_ERASE: {
//                                        word.is_selected = false;
//                                        break;
//                                    }
//                                }
//                            }
                        }
                    }
                    if (is_finished_selection_rectangle) {
                        panel.text.first_selected_i = INVALID_SELECTION_INTERSECTION_IDX;
                    }
                }
                if (text_panel.text.empty()) {
                    text_panel.vertical_scroll_offset = 0;
                } else if (panel.text.first_selected_i != INVALID_SELECTION_INTERSECTION_IDX) {
                    float32 min_y;
                    float32 max_y;
                    if (selection_intersection.tl.y < selection_intersection.br.y) {
                        min_y = selection_intersection.tl.y;
                        max_y = selection_intersection.br.y;
                    } else {
                        max_y = selection_intersection.tl.y;
                        min_y = selection_intersection.br.y;
                    }
                    
                    if (min_y < panel.bounds.tl.y + (0.75f * text_panel.offset.y)) {
                        //MTT_print("selection moving above\n");
                        
                        auto& word = text_panel.text.back();
                        float half_height = (word.bounds[3] - word.bounds[1]) * 0.5;
                        
                        text_panel.vertical_scroll_offset += 16 * half_height * core_ctx->time_delta_seconds;
                        
                        text_panel.vertical_scroll_offset = m::min(m::max(-half_height, text_panel.vertical_scroll_offset), text_panel.row_offset + half_height);
                        
                    } else if (max_y >= (panel.bounds.tl.y + panel.bounds.dimensions.y)) {
                        //MTT_print("section moving below\n");
                        
                        auto& word = text_panel.text.back();
                        float half_height = (word.bounds[3] - word.bounds[1]) * 0.5;
                        
                        text_panel.vertical_scroll_offset -= 16 * half_height * core_ctx->time_delta_seconds;
                        
                        text_panel.vertical_scroll_offset = m::min(m::max(-half_height, text_panel.vertical_scroll_offset), text_panel.row_offset + half_height);
                    }
                }
            }
        LABEL_NO_RECTS:;
            
            for (usize i = 0; i < text_panel.text.size(); i += 1) {
                auto& word = text_panel.text[text_panel.text.size() - i - 1];
                auto color = word.color;
                
                if (word.is_highlight_selected) {
                    color = dt::SELECTION_RECTANGLE_COLOR;
                }
                

                cstring CS = word.text.c_str();
                
                
                float half_height = (word.bounds[3] - word.bounds[1]) / 2.0f;
                
                float bounds[4];
                bounds[0] = x_off + word.bounds[0];
                bounds[1] = (word.bounds[1] - text_panel.row_offset) + scroll_offset_y;
                bounds[2] = x_off + word.bounds[2];
                bounds[3] = (word.bounds[3] - text_panel.row_offset - half_height) + scroll_offset_y;
                
                if (bounds[3] < (panel.bounds.tl.y + text_panel.offset.y)) {
                    break;
                }
                if ((bounds[3] > panel.bounds.tl.y + panel.bounds.dimensions.y)) {
                    continue;
                }
                
                bool selected_with_touch = false;
                bool selected_with_pen = false;
                
                
                if (panel.touch_state == UI_TOUCH_STATE_BEGAN) {
                    vec2 pos = panel.most_recent_selected_canvas_position_touch;
                    c.center = pos;
                    if (mtt::Box_Circle_intersection(bounds, &c)) {
                        something_selected_with_touch = true;
                        selected_with_touch = true;
                        touch_collided_with_word = true;
                        
                        
//                        if (word.part_of_contraction) {
//                            word.is_selected = !word.is_selected;
//
//                            if (word.matching_contraction_idx < i && i > 0) {
//                                text_panel.text[word.matching_contraction_idx].is_selected = word.is_selected;
//                            }
//                        } else {
                        
                            // NO TOUCH FOR NOW
                            //word.is_selected = !word.is_selected;
//                        }
                    }
//                    else {
//                        if (word.part_of_contraction) {
//                            if (word.matching_contraction_idx < i && i > 0) {
//                                word.is_selected = !text_panel.text[word.matching_contraction_idx].is_selected;
//                            }
//                        }
//                    }
                    
                }

                if (panel.pen_state != UI_TOUCH_STATE_NONE) {
                    vec2 pos = panel.most_recent_selected_canvas_position_pen;
                    c.center = pos;
                    if (mtt::Box_Circle_intersection(bounds, &c)) {
                        pen_collided_with_word = true;
                        if (word.is_selected == false && panel.pen_state == UI_TOUCH_STATE_BEGAN) {
                            text_in_buf.clear();
                            text_in_selected_all = false;
                            text_in_cursor_idx = false;
                        }
                        
//                        if (panel.pen_state == UI_TOUCH_STATE_BEGAN) {
//                            if (word.is_selected) {
//                                word.is_selected = false;
//                            } else {
//                                word.is_selected = true;
//                                something_selected_with_pen = true;
//                                selected_with_pen = true;
//                            }
//                        } else {
                        
//                        }
                       
                        

                        
//                        if (word.part_of_contraction) {
//                            if (word.matching_contraction_idx < i && i > 0) {
//                                text_panel.text[word.matching_contraction_idx].is_selected = word.is_selected;
//                            }
//                        }
                        bool was_mapping_operation = false;
                        {

                            if (spacy_nlp::pos_tag_is_noun_like(word.token->tag) || word.token->pos == spacy_nlp::POS_PRON || word.token->pos == spacy_nlp::POS_PROPN || word.update.mapping_count > 0
                                ) {
                                {
                                    
                                    
                                    for (auto t_it = dt->scn_ctx.selected_things.begin(); t_it != dt->scn_ctx.selected_things.end(); ++t_it) {
                                        
                                        mtt::Thing* thing = world->Thing_try_get(*t_it);
                                        
                                        if (thing == nullptr || ((!dt::can_label_thing(thing)))) {
                                            continue;
                                        }
                                        
                                        
                                        auto* noun_entry = dt::noun_lookup(word.token->lemma);
                                        if (!noun_entry || word.token->lemma == "I") {
                                            noun_entry = dt::noun_add((word.token->prop_ref) ? word.token->prop_ref->label : word.token->lemma);
                                        }
                                        
                                        auto find_it = word.update.mappings.find(thing->id);
                                        if (find_it == word.update.mappings.end()) {
                                            if (word.token->pos != spacy_nlp::POS_PRON) {
                                                dt::vis_word_derive_from(thing, noun_entry);
                                            }
                                            word.update.mappings.insert({thing->id, robin_hood::pair<bool, float64>(true, panel.pen_began_time)});
                                            
                                            word.update.mapping_count += 1;
                                            was_mapping_operation = true;
                                            MTT_print("Mapping Count: %llu\n", word.update.mapping_count);
                                        } else {
                                            
                                            float64 timestamp = find_it->second.second;
                                            
                                            if (timestamp < panel.pen_began_time && dt::is_derived_from(thing, noun_entry)) {
                                                if (word.token->pos != spacy_nlp::POS_PRON) {
                                                    if (noun_entry != nullptr) {
                                                        dt::vis_word_underive_from(thing, noun_entry);
                                                    }
                                                }
                                                
                                                find_it->second.second = panel.pen_began_time;
                                                find_it->second.first = false;
                                                word.update.mapping_count -= 1;
                                                was_mapping_operation = true;
                                                MTT_print("Mapping Count: %llu\n", word.update.mapping_count);
                                            } else if (timestamp < panel.pen_began_time) {
                                                if (noun_entry != nullptr) {
                                                    dt::vis_word_derive_from(thing, noun_entry);
                                                }
                                                find_it->second.second = panel.pen_began_time;
                                                find_it->second.first = true;
                                                word.update.mapping_count += 1;
                                                was_mapping_operation = true;
                                                MTT_print("Mapping Count: %llu\n", word.update.mapping_count);
                                            }
                                            
                                        }
                                        
                                        
                                    
                                    }
                                }
                                
                            }
                            if (word.token->pos == spacy_nlp::POS_ADJ) {
                                
                                
                                for (auto t_it = dt->scn_ctx.selected_things.begin(); t_it != dt->scn_ctx.selected_things.end(); ++t_it) {
                                    
                                    mtt::Thing* thing = world->Thing_try_get(*t_it);
                                    
                                    if (thing == nullptr || !dt::can_label_thing(thing)) {
                                        continue;
                                    }
                                    
                                    
                                    Word_Dictionary_Entry* attrib = dt::attribute_add(word.token->lemma);
                                    
                                    Speech_Property::Prop_List* adj_prop = (word.token->prop_ref) ? word.token->prop_ref->try_get_prop("PROPERTY") : nullptr;
                                    
                                    auto find_it = word.update.mappings.find(thing->id);
                                    if (find_it == word.update.mappings.end()) {

                                        
                                        if (adj_prop == nullptr) {
                                            Thing_add_attribute(thing, attrib);
                                        } else {
                                            const Speech_Property* prev = nullptr;
                                            for (const Speech_Property* mod : *adj_prop) {
                                                if (mod->label == "modifier") {
                                                    float32* value = nullptr;
                                                    if (!mtt::map_try_get(&dt::magnitude_words, mod->value.text, &value)) {
                                                        Thing_add_attribute(thing, word.token->lemma, mod->value.text, {});
                                                    } else {
                                                        if (prev != nullptr && prev->value.text == mod->value.text) {
                                                            Thing_add_attribute(thing, word.token->lemma, mod->value.text, mtt::Any::from_Float(accumulate_word_values((*value))));
                                                        } else {
                                                            Thing_add_attribute(thing, word.token->lemma, mod->value.text, mtt::Any::from_Float(*value));
                                                        }
                                                    }
                                                    prev = mod;
                                                }
                                            }
                                        }

                                        word.update.mappings.insert({thing->id, robin_hood::pair<bool, float64>(true, panel.pen_began_time)});
                                        was_mapping_operation = true;
                                    } else {
                                        
                                        float64 timestamp = find_it->second.second;
                                        
                                        if (adj_prop == nullptr || adj_prop->front()->label != "modifier") {
                                            if (timestamp < panel.pen_began_time && Thing_has_attribute(thing, attrib)) {
                                                    
                                                Thing_remove_attribute(thing, attrib);
                                                
                                                find_it->second.second = panel.pen_began_time;
                                                find_it->second.first = false;
                                                was_mapping_operation = true;
                                            } else if (timestamp < panel.pen_began_time) {
                                                Thing_add_attribute(thing, attrib);

                                                
                                                find_it->second.second = panel.pen_began_time;
                                                find_it->second.first = true;
                                                was_mapping_operation = true;
                                            }
                                        } else {
                                            if (timestamp < panel.pen_began_time && Thing_has_attribute(thing, attrib)) {
                                                
                                                bool removed_modifier = false;
                                                for (const Speech_Property* mod : *adj_prop) {
                                                    removed_modifier |= Thing_remove_attribute_property(thing, word.token->lemma, mod->value.text);
                                                }
                                                
                                                if (!removed_modifier) {
                                                    Thing_remove_attribute(thing, attrib);
                                                }
                                                
                                                find_it->second.second = panel.pen_began_time;
                                                find_it->second.first = false;
                                                was_mapping_operation = true;
                                            } else if (timestamp < panel.pen_began_time) {
                                                float32* value = nullptr;
                                                const Speech_Property* prev = nullptr;
                                                for (const Speech_Property* mod : *adj_prop) {
                                                    if (!mtt::map_try_get(&dt::magnitude_words, mod->value.text, &value)) {
                                                        Thing_add_attribute(thing, word.token->lemma, mod->value.text, {});
                                                    } else {
                                                        if (prev != nullptr && prev->value.text == mod->value.text) {
                                                            Thing_add_attribute(thing, word.token->lemma, mod->value.text, mtt::Any::from_Float(accumulate_word_values((*value))));
                                                        } else {
                                                            Thing_add_attribute(thing, word.token->lemma, mod->value.text, mtt::Any::from_Float(*value));
                                                        }
                                                    }
                                                    prev = mod;
                                                }
                                                
                                                find_it->second.second = panel.pen_began_time;
                                                find_it->second.first = true;
                                                was_mapping_operation = true;
                                            }
                                        }
                                        
                                    }
                                }

                                
                                
                            }
                                // TODO: pronoun for coref?
                                // pos == PRON
                                
                            
                        }
                        if (!was_mapping_operation || touch_count_active == 0) {
                            word_selected_with_pen_prev_state = word.is_selected;
                            
                            if (touch_count_active == 0) {
                                if (panel.pen_state == UI_TOUCH_STATE_ENDED) {
                                    word.is_selected = (touch_count_active == 0) ? !word.is_selected : true;
                                }
                        
                                something_selected_with_pen = word.is_selected;
                                selected_with_pen = word.is_selected;
                                word_selected_with_pen = &word;
                            } else {
                                something_selected_with_pen = true;
                                selected_with_pen = true;
                                word_selected_with_pen = &word;
                            }
                        }
                    } else {

                    }
                }
                    
                {
                    
                    // TODO: separate branches for is_selected
                    if (word.is_selected && text_panel.delete_mode_on) {
                        word.is_selected = false;
                        regen_op.token.insert(word.token);
                    } else if (word.is_selected && text_panel.received_new_text_input && !dt->lang_ctx.parse_q().empty() && !dt->lang_ctx.parse_q().back()->is_finished()) {
                        regen_op.type = (text_panel.text_input.size() > 0) ? Regenerate_Operation::TYPE::RENAME : Regenerate_Operation::TYPE::DELETE;
                        
                        regen_op.do_send = text_panel.do_send;
                        
                        regen_op.token.insert(word.token);
                        
                        text_panel.set_is_overriding_speech(true);
                        
                        if (word.update.mapping_count > 0) {
                            for (auto t_it = word.update.mappings.begin(); t_it != word.update.mappings.end(); ++t_it) {
                                if ((*t_it).second.first == false) {
                                    continue;
                                }
                                mtt::Thing* thing = world->Thing_try_get((*t_it).first);
                                auto* noun_entry = dt::noun_lookup(word.token->lemma);
                                if (noun_entry != nullptr) {
                                    dt::vis_word_underive_from(thing, noun_entry);
                                }
                            }
                        }
                        word.token->text = text_panel.text_input;
                        word.text = text_panel.text_input;
                        
                        
                        
                    } else {
                    
                        if ((word.is_selected || word.update.mapping_count != 0)) {
                            float bounds[4];
                            
                            
                            bounds[0] = x_off + word.bounds[0];
                            bounds[1] = (word.bounds[1] - text_panel.row_offset) + scroll_offset_y;
                            bounds[2] = x_off + word.bounds[2];
                            bounds[3] = (word.bounds[3] - text_panel.row_offset - half_height) + scroll_offset_y;
                            
                            nvgFillColor(vg, nvgRGBf(color[0], color[1], color[2]));
                            nvgText(vg, x_off + word.bounds[0], (word.bounds[1] - text_panel.row_offset) + scroll_offset_y, CS, NULL);
                            
                            
                            if (word.update.mapping_count != 0) {
                                for (auto t_it = word.update.mappings.begin(); t_it != word.update.mappings.end(); ) {
                                    if ((*t_it).second.first == false) {
                                        ++t_it;
                                        continue;
                                    }
                                    
                                    mtt::Thing* thing = dt->mtt->Thing_try_get((*t_it).first);
                                    if (thing == nullptr) {
                                        t_it = word.update.mappings.erase(t_it);
                                        nvgFillColor(vg, nvgRGBA(0,0,0,255/4));
                                        word.update.mapping_count -= 1;
                                    } else {
                                        nvgFillColor(vg, nvgRGBA(0, 181, 204, 255/4));
                                        
                                        mtt::Rep* rep;
                                        mtt::rep(thing, &rep);
                                        if (!rep->colliders.empty() && (word.is_selected || get_confirm_phase() != CONFIRM_PHASE_ANIMATE)) {
                                            nvgStrokeColor(vg, nvgRGBA(0, 181, 204, 255/4));
                                            
                                            nvgBeginPath(vg);
                                            nvgStrokeWidth(vg, 4);
                                            nvgMoveTo(vg, bounds[0], bounds[3]);
                                            DrawTalk_World* cw = dt_world;
                                            auto& view_transform = cw->cam.view_transform;
                                            vec2 xformed = view_transform * vec4(rep->colliders[0]->center_anchor.x, rep->colliders[0]->center_anchor.y,  0.0f, 1.0f);
                                            
                                            
                                            nvgQuadTo(vg, xformed.x, bounds[3], xformed.x, xformed.y);
                                            
                                            nvgStroke(vg);
                                            
                                        }
                                        ++t_it;
                                    }

                                }
                            } else {
                                nvgFillColor(vg, nvgRGBA(0,0,0,255/4));
                            }
                            
                            if (word.is_highlight_selected) {
                                // (should preview selection)
                                if (!first_selection_intersection_item_was_already_selected) {
                                    nvgBeginPath(vg);
                                    nvgFillColor(vg, nvgRGBAf(
                                                              dt::TEXT_SELECTED_COLOR[0],
                                                              dt::TEXT_SELECTED_COLOR[1],
                                                              dt::TEXT_SELECTED_COLOR[2],
                                                              dt::TEXT_SELECTED_COLOR[3]));
                                    nvgRoundedRect(vg, bounds[0], bounds[1], bounds[2] - bounds[0], bounds[3] - bounds[1], 4.0);
                                    nvgFill(vg);
                                }
                            }
                            else if (word.is_selected) {
                                nvgBeginPath(vg);
                                nvgFillColor(vg, nvgRGBAf(
                                                          dt::TEXT_SELECTED_COLOR[0],
                                                          dt::TEXT_SELECTED_COLOR[1],
                                                          dt::TEXT_SELECTED_COLOR[2],
                                                          dt::TEXT_SELECTED_COLOR[3]));
                                nvgRoundedRect(vg, bounds[0], bounds[1], bounds[2] - bounds[0], bounds[3] - bounds[1], 4.0);
                                nvgFill(vg);
                            }
                            
                            
                            
                            
                            
                        } else {
                            nvgFillColor(vg, nvgRGBf(color[0], color[1], color[2]));
                            nvgText(vg, x_off + word.bounds[0], (word.bounds[1] - text_panel.row_offset) + scroll_offset_y, CS, NULL);
                            
                            if (word.is_highlight_selected) {
                                // (should preview selection)
                                if (!first_selection_intersection_item_was_already_selected) {
                                    nvgBeginPath(vg);
                                    nvgFillColor(vg, nvgRGBAf(
                                                              dt::TEXT_SELECTED_COLOR[0],
                                                              dt::TEXT_SELECTED_COLOR[1],
                                                              dt::TEXT_SELECTED_COLOR[2],
                                                              dt::TEXT_SELECTED_COLOR[3]));
                                    nvgRoundedRect(vg, bounds[0], bounds[1], bounds[2] - bounds[0], bounds[3] - bounds[1], 4.0);
                                    nvgFill(vg);
                                }
                                nvgFillColor(vg, nvgRGBf(color[0], color[1], color[2]));
                            }
                        }
                        
                    }
                    
#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
                    if (word.is_selected && panel.pen_state == UI_TOUCH_STATE_ENDED) {
                        
                        
                        if (/*selection_count >= 1 && */touch_count_active > 0) {
                            selected_text_list.push_back(&word);
                        }
                    }
#else
                    if (word.is_selected && panel.pen_state == UI_TOUCH_STATE_ENDED) {
                        
                        
                        if (/*selection_count >= 1 && */touch_count_active > 0) {
                            selected_text_list.push_back(&word);
                        }
                    }
#endif
                }
                

#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
                if (defer_deselection) {
                    word.is_selected = false;
                }
#endif
                

//                if (!word.is_selected) {
//                    //word.update.thing_id = mtt::Thing_ID_INVALID;
//                }

                
                
                
            }
            
            auto pen_state = panel.pen_state;
            auto touch_state = panel.touch_state;
            auto pen_select = something_selected_with_pen;
            auto touch_select = something_selected_with_touch;
            
            if (!regen_op.token.empty() || text_panel.send) {
                text_panel.send = false;
                if (regen_op.type == Regenerate_Operation::TYPE::CONFIRM) {
                    text_panel.set_is_overriding_speech(false);
                    regen_op.do_send = true;
//                    for (auto it = text_panel.text.begin(); it != text_panel.text.end(); ++it) {
//                        (*it).is_selected = false;
//                        (*it).update.thing_id = mtt::Thing_ID_INVALID;
//                    }
                    dt::regenerate_text_view(dt, *(text_panel.text.begin()->token->parse), regen_op, true);

                } else {
                    dt::regenerate_text_view(dt, *(text_panel.text.begin()->token->parse), regen_op, false);

                    if (text_panel.is_overriding_speech()) {
                        deferred_op = regen_op;
                        has_deferred_op = true;
                        
                    } else {
                        has_deferred_op = false;
                    }
                    
                }
            }
                
            if (!text_panel.is_overriding_speech()) {
                has_deferred_op = false;
            } else if (has_deferred_op && text_panel.is_overriding_speech() && pen_state == UI_TOUCH_STATE_ENDED) {
                dt::regenerate_text_view(dt, *(text_panel.text.begin()->token->parse), deferred_op, true);
            }
            
            if (text_panel.send) {
                text_in_buf.clear();
                text_in_selected_all = false;
                text_in_cursor_idx = false;
            }
                
            if (pen_state == UI_TOUCH_STATE_ENDED && !word_selected_with_pen) {
                text_in_buf.clear();
                text_in_selected_all = false;
                text_in_cursor_idx = false;
            }
            
            /*
             if (hit) {
                 caretx = (mx < x+row->width/2) ? x : x+row->width;
                 px = x;
                 nglyphs = nvgTextGlyphPositions(vg, x, y, row->start, row->end, glyphs, 100);
                 for (j = 0; j < nglyphs; j++) {
                     float x0 = glyphs[j].x;
                     float x1 = (j+1 < nglyphs) ? glyphs[j+1].x : x+row->width;
                     float gx = x0 * 0.3f + x1 * 0.7f;
                     if (mx >= px && mx < gx)
                         caretx = glyphs[j].x;
                     px = gx;
                 }
                 nvgBeginPath(vg);
                 nvgFillColor(vg, nvgRGBA(255,192,0,255));
                 nvgRect(vg, caretx, y, 1, lineh);
                 nvgFill(vg);
                 
                 gutter = lnum+1;
                 gx = x - 10;
                 gy = y + lineh/2;
             }
             */
            
        }
            
//        if (!pen_collided_with_word && panel.pen_state == UI_TOUCH_STATE_BEGAN) {
//            for (usize i = 0; i < text_panel.text.size(); i += 1) {
//                text_panel.text[i].is_selected = false;
//            }
//        }
         
        if (selected_text_list.size() == 1) {
            auto& word = *selected_text_list[0];
            if (word.token != nullptr && word.token->pos == spacy_nlp::POS_VERB) {
                selected_text = word.token->lemma + " ";
            } else {
                selected_text = word.text + " ";
            }
        }
        else if (word_selected_with_pen != nullptr && word_selected_with_pen->token != nullptr && selected_text_list.size() == 0) {
            if (word_selected_with_pen->token->pos == spacy_nlp::POS_VERB) {
                selected_text = word_selected_with_pen->token->lemma + " ";
            } else {
                selected_text = word_selected_with_pen->text + " ";
            }
        }
        else {
            usize count = 0;
            for (auto sel_it = selected_text_list.begin(); sel_it != selected_text_list.end(); ++sel_it) {
                auto& word = **sel_it;
                count += word.text.size();
            }
            count += (selected_text_list.size());
            selected_text.reserve(count + 1);
            for (auto sel_it = selected_text_list.begin(); sel_it != selected_text_list.end(); ++sel_it) {
                auto& word = **sel_it;
                selected_text = (word.text + " " + selected_text);
            }
            selected_text += " ";
        }
            
            
        if (panel.touch_state == UI_TOUCH_STATE_BEGAN) {
            text_in_buf.clear();
            text_in_selected_all = false;
            text_in_cursor_idx = false;
//            if (!something_selected_with_touch) {
//                for (auto it = dt->ui.margin_panels[0].text.text.begin(); it != dt->ui.margin_panels[0].text.text.end(); ++it) {
//                    (*it).is_selected = false;
//                    (*it).update.thing_id = mtt::Thing_ID_INVALID;
//                }
//            }
            if (!something_selected_with_touch) {
                //text_panel.set_is_overriding_speech(false);
            }
        } else if (panel.pen_state == UI_TOUCH_STATE_BEGAN && !something_selected_with_pen) {
            text_in_buf.clear();
            text_in_selected_all = false;
            text_in_cursor_idx = false;
//            {
//                for (auto it = dt->ui.margin_panels[0].text.text.begin(); it != dt->ui.margin_panels[0].text.text.end(); ++it) {
//                    (*it).is_selected = false;
//                    (*it).update.thing_id = mtt::Thing_ID_INVALID;
//                }
//            }
            {
                if (pointer_op_in_progress(u_input) != UI_POINTER_OP_DRAW) {
                    switch (pointer_op_in_progress(u_input)) {
                        case UI_POINTER_OP_ERASE: {
                            auto& dock = dt::DrawTalk::ctx()->ui.dock;
                            mtt::Thing* ui_button = world->Thing_get(dock.buttons[dock.label_to_index.find("discard")->second]->thing);
                            ui_button->input_handlers.on_touch_input_began(&ui_button->input_handlers, ui_button, {}, {}, nullptr, nullptr, 0);
                            break;
                        }
                    }
                }
            }
        } else if (panel.pen_state == UI_TOUCH_STATE_ENDED && touch_count_active > 0) {

            if (!existing_thing_selected) {
                if (something_selected_with_pen || !selected_text_list.empty()) {
                    mtt::String& msg = selected_text;
                    if (msg.size() > 1) {
                        mtt::String in = msg.substr(0, msg.size() - 1);
                        isize out = -1;
                        float64 v_out = 0.0;
                        if (dt::text2num({in}, &out)) {
                            auto* number = mtt::Thing_make(world, mtt::ARCHETYPE_NUMBER);
                            mtt::Thing_set_position(number, control->most_recent_bg_touch_transformed);
                            mtt::number_update_value(number, out);
                        } else if (dt::numstr2num(in, &v_out)) {
                            auto* number = mtt::Thing_make(world, mtt::ARCHETYPE_NUMBER);
                            mtt::Thing_set_position(number, control->most_recent_bg_touch_transformed);
                            mtt::number_update_value(number, v_out);
                        } else {
                            auto* text = mtt::Thing_make(world, mtt::ARCHETYPE_TEXT);
                            mtt::Thing_set_position(text, control->most_recent_bg_touch_transformed);
                            mtt::text_update_value(text, in);
                        }
                        
                    }
                } else {
                    mtt::String& msg = text_panel.generated_message;
                    
                    if (!msg.empty()) {
                        mtt::String in = msg;
                        if (msg.back() == '.') {
                            in.pop_back();
                            in.pop_back();
                        }
                        isize out = -1;
                        float64 v_out = 0.0;
                        if (dt::text2num({in}, &out)) {
                            auto* number = mtt::Thing_make(world, mtt::ARCHETYPE_NUMBER);
                            mtt::Thing_set_position(number, control->most_recent_bg_touch_transformed);
                            mtt::number_update_value(number, out);
                        } else if(dt::numstr2num(in, &v_out)) {
                            auto* number = mtt::Thing_make(world, mtt::ARCHETYPE_NUMBER);
                            mtt::Thing_set_position(number, control->most_recent_bg_touch_transformed);
                            mtt::number_update_value(number, v_out);
                        } else {
                            auto* text = mtt::Thing_make(world, mtt::ARCHETYPE_TEXT);
                            mtt::Thing_set_position(text, control->most_recent_bg_touch_transformed);
                            mtt::text_update_value(text, in);
                        }
                    }
                }
            } else {
                
                for (auto it_s = selections.begin(); it_s != selections.end(); ++it_s) {
                    auto* thing = world->Thing_try_get(it_s->second.thing);
                    if (thing != nullptr && (thing->archetype_id == mtt::ARCHETYPE_TEXT || thing->archetype_id == mtt::ARCHETYPE_NUMBER)) {
                        {
                            if (something_selected_with_pen || !selected_text_list.empty()) {
                                mtt::String& msg = selected_text;
                                if (msg.size() > 1) {
                                    mtt::String in = msg.substr(0, msg.size() - 1);
                                    
                                    if (thing->archetype_id == mtt::ARCHETYPE_NUMBER) {
                                        isize out = -1;
                                        float64 v_out = 0.0;
                                        if (dt::text2num({in}, &out)) {
                                            mtt::number_update_value(thing, out);
                                        } else if (dt::numstr2num(in, &v_out)) {
                                            mtt::number_update_value(thing, v_out);
                                        }
                                    } else {
                                        mtt::text_update_value(thing, in);
                                    }
                                }
                            } else {
                                mtt::String& msg = text_panel.generated_message;
                                
                                if (!msg.empty()) {
                                    mtt::String in = msg;

                                    if (thing->archetype_id == mtt::ARCHETYPE_NUMBER) {
                                        isize out = -1;
                                        float64 v_out = 0.0;
                                        if (dt::text2num({in}, &out)) {
                                            mtt::number_update_value(thing, out);
                                        } else if (dt::numstr2num(in, &v_out)) {
                                            mtt::number_update_value(thing, v_out);
                                        }
                                    } else {
                                        if (msg.back() == '.') {
                                            in.pop_back();
                                            in.pop_back();
                                        }
                                        mtt::text_update_value(thing, in);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        
        panel.text.delete_mode_on = false;
        text_panel.received_new_text_input = false;
        text_panel.text_input = "";
        
        panel.touch_state = UI_TOUCH_STATE_NONE;
        panel.pen_state   = UI_TOUCH_STATE_NONE;
        
        if (text_panel.text_input != "") {
            text_panel.text_input = "";
        }
        nvgRestore(vg);
        
    }
    
    nvgSave(vg);
    {
        
//            auto* button
//            button_main->label = labels[i];
//            dock.buttons[i] = button_main;
        
        struct Settings {
            float line_h;
        } font_settings;
        font_settings.line_h = 0;
        nvgFontSize(vg, ui.dock.button_font_size);
        nvgFontFace(vg, "sans");
        nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_CENTER);
        nvgTextMetrics(vg, NULL, NULL, &font_settings.line_h);
        auto* buttons = &ui.dock.buttons;
        const auto button_count = buttons->size();
        for (usize i_button = 0; i_button < button_count; i_button += 1) {
            auto* button = (*buttons)[i_button];
            
            if (!button->show_label) MTT_UNLIKELY {
                continue;
            }

            
            mtt::Thing* thing = dt->mtt->Thing_get(button->thing);
#if !DT_DEFINE_FLEXIBLE_IGNORE_SELECTIONS
            if (!mtt::is_active(thing))
#else
            if (button->alt_state)
#endif
            {

                if (button->alt_state) {
                    nvgFillColor(vg, nvgRGBAf(1.0, 0.0, 0.0, 1.0));
                    
                    if (button->position_is_modified) MTT_UNLIKELY {
                        button->position_is_modified = false;
                        button->saved_center = vec2(*mtt::access<vec3>(thing, "position"));
                    }
                    vec2 pos = button->saved_center;
                    
                    const vec2 dimensions = button->saved_dimensions;
                    
                    const vec2 half_dimensions = dimensions / 2.0f;
                    
//                    pos.x -= half_dimensions.x;
//                    pos.y -= 0.25f*half_dimensions.y;
                    pos -= vec2(half_dimensions.x, 0.25f*half_dimensions.y);
                    

                    cstring label = "Missing Info!";
                    nvgTextBox(vg, pos.x, pos.y, dimensions.x, label, NULL);
                }
                continue;
            }

            if (dt::bg_color_default() == dt::BG_COLOR_DARK_MODE) {
                nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,button->alpha));
            } else {
                nvgFillColor(vg, nvgRGBAf(0,0,0,button->alpha));
            }
            
            if (button->position_is_modified) MTT_UNLIKELY {
                button->position_is_modified = false;
                button->saved_center = vec2(*mtt::access<vec3>(thing, "position"));
            }
            vec2 pos = button->saved_center;
            
            const vec2 dimensions = button->saved_dimensions;
            
            const vec2 half_dimensions = dimensions / 2.0f;
            
//            pos.x -= half_dimensions.x;
//            pos.y -= 0.25f*half_dimensions.y;
            pos -= vec2(half_dimensions.x, 0.25f*half_dimensions.y);
            

            cstring label = button->label.c_str();
            nvgTextBox(vg, pos.x, pos.y, dimensions.x, label, NULL);
        }
        
    }
    nvgRestore(vg);
    
    UI_Button* SPEECH_CONFIRM_UI_BUTTON = ui.dock.buttons[ui.dock.label_to_index.find(DT_SPEECH_COMMAND_BUTTON_NAME)->second];
    SPEECH_CONFIRM_UI_BUTTON->alt_state = false;
    mtt::Thing* SPEECH_CONFIRM_THING = world->Thing_get(SPEECH_CONFIRM_UI_BUTTON->thing);
    mtt::set_should_render(SPEECH_CONFIRM_THING);
    mtt::set_is_active(SPEECH_CONFIRM_THING);
    
    
    auto color_bg = dt::bg_color_default();
    {
        auto* mtt_renderer = dt->mtt->renderer;
        auto& ui_panel = dt->ui.top_panel;
        
        
        sd::set_render_layer(mtt_renderer, LAYER_LABEL_DYNAMIC_BG_LAYER_PER_FRAME);
        
        

        int stroke_width = 1;
        
        int arrow_stroke_width = 1;
        int arrow_radius = M_PI;
        auto& ins_list = dt->lang_ctx.eval_ctx.instructions_saved();
        
        // temp test instructions all added to list
        
        {
            if (ins_list.size() != Instruction::count) {
//                MTT_print("instruction not added correctly ? ins_list:%zu Instruction count:%llu\n", ins_list.size(), Instruction::count);
            }
        }
        
        
//        if (!ins_list.empty()) {
//            MTT_print("---\n");
//        }
//        for (usize i = 0; i < ins_list.size(); i += 1) {
//            MTT_print("(Instruction) %llu %s %s %p\n", i, ins_list[i]->type.c_str(), ins_list[i]->kind.c_str(), ins_list[i]);
//            MTT_print("%s\n", ins_list[i]->to_string().c_str());
//        }
        
        auto& scissor_bounds = ui.top_panel.base.bounds;
        sd::set_color_rgba_v4(mtt_renderer, vec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (!ins_list.empty()) {
            mtt::Thing* root = dt->mtt->Thing_get(dt->lang_ctx.eval_ctx.root_thing_id);
            mtt::Rep* root_rep = mtt::rep(root);
            vec2 offset = root_rep->transform.translation;
            offset = vec2(0.0f);
            
            mat4 root_model = m::translate(mat4(1.0f), root_rep->transform.translation);
            
            
            nvgSave(vg);

            {
                Mat4 view = dt_world->cam.view_transform;
                Mat4 view_inverse = m::inverse(view);
                
                Mat4 view_dst = view;
                Mat4 view_dst_inverse = view_inverse;
                Mat4 view_pip = ui.top_panel.cam.view_transform;
                Mat4 view_pip_inverse = m::inverse(view_pip);
                
                vec3 view_pip_scale = vec3(1, 1, 1);
                float32 view_pip_scale_max_dim = m::max(view_pip_scale.x, view_pip_scale.y);
                {
                    quat d_orientation;
                    vec3 d_translation;
                    vec3 d_skew;
                    vec4 d_perspective;
                    
                    m::decompose(view_pip, view_pip_scale, d_orientation, d_translation, d_skew, d_perspective);
                }
                
                //
                view = view_pip;
                view_inverse = view_pip_inverse;
                nvgSetViewTransform(vg, m::value_ptr(view));
                
                
//                auto half_w = core_ctx->viewport.width / 2.0f;
//                auto half_h = core_ctx->viewport.height / 2.0f;
//
//                //view = m::translate(view, vec3(half_w, half_h, 0.0f));
//
                vec2 scissor_tl = vec2(scissor_bounds.tl.x, scissor_bounds.tl.y);
                vec2 scissor_br = vec2(scissor_bounds.tl.x + scissor_bounds.dimensions.x, scissor_bounds.tl.y + scissor_bounds.dimensions.y);
//                vec2 scissor_dimensions = scissor_bounds.dimensions;
//
//                if (scissor_tl.x < 0.0f) {
//                    scissor_dimensions.x += scissor_tl.x;
//                    scissor_tl.x = 0.0f;
//                }
//                if (scissor_tl.y < 0.0f) {
//                    scissor_dimensions.y += scissor_tl.y;
//                    scissor_tl.y = 0.0f;
//                }
                nvgSetHWScissor(vg, scissor_bounds.tl.x, scissor_bounds.tl.y, scissor_bounds.dimensions.x, scissor_bounds.dimensions.y);
                
                //nvgSetViewTransform(vg, value_ptr(view));

//                nvgBeginPath(vg);
//                nvgFillColor(vg, nvgRGBA(128, 128, 128, 128));
//                nvgCircle(vg, offset.x, offset.y, layout_padding_root / 2.0f);
//                nvgFill(vg);
//
//                nvgBeginPath(vg);
//
//                nvgFillColor(vg, nvgRGBA(200, 200, 200, 255));
//
//                nvgCircle(vg, offset.x, offset.y, layout_padding_root / 4.0f);
//
//                nvgFill(vg);

                sd::begin_polygon(mtt_renderer);
                sd::set_color_rgba_v4(mtt_renderer, vec4(128.0f/255.0f, 128.0f/255.0f, 128.0f/255.0f, 128.0f/255.0f));
                sd::circle(mtt_renderer, layout_padding_root / 2.0f, vec3(0, 0, 0.0f));
                sd::set_color_rgba_v4(mtt_renderer, vec4(200.0f/255.0f, 200.0f/255.0f, 200.0f/255.0f, 1.0f));
                sd::circle(mtt_renderer, layout_padding_root / 4.0f, vec3(0, 0, 0.0f));
                sd::end_polygon(mtt_renderer)->set_transform(mtt::rep(root)->model_transform);
                
                nvgFontSize(vg, dt->lang_ctx.eval_ctx.font_size);
                nvgFontFace(vg, dt->lang_ctx.eval_ctx.font_face);
                nvgTextAlign(vg, dt->lang_ctx.eval_ctx.align);
                nvgTextMetrics(vg, NULL, NULL, &dt->lang_ctx.eval_ctx.layout.height);
                vec4 default_pen_color = dt::pen_color_default_for_color_scheme(mtt::color_scheme());
                
                sd::set_render_layer(mtt_renderer, LAYER_LABEL_DYNAMIC_CANVAS);
                sd::begin_path(dt->mtt->renderer);
                
                for (usize i = 0; i < ins_list.size(); i += 1) {
                    //&ins[i]->bounds
//                    bounds[0] = x_off + word.bounds[0];
//                    bounds[1] = word.bounds[1] - text_panel.row_offset;
//                    bounds[2] = x_off + word.bounds[2];
//                    bounds[3] = word.bounds[3] - text_panel.row_offset - half_height;
                    
                    sd::set_color_rgba_v4(dt->mtt->renderer, vec4(vec3(default_pen_color), 1.0f));

                    auto* ins = ins_list[i];
                    
                    auto bounds = ins->bounds;
                    auto* rep = mtt::rep(ins->thing_id);
                    
                    
                    
                    auto fill_color = ins->color_current;
                    fill_color.a = 1.0f;
//                    if (ins->type == "TRIGGER") {
//                        if (ins->children.empty()) {
//                            fill_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f);
//                        }
//                    } else if (ins->type == "RESPONSE") {
//                        if (ins->children.empty()) {
//                            fill_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f);
//                        }
//                    } else if ((ins->kind == "THING_INSTANCE_SELECTION" ||
//                                ins->kind == "THING_TYPE_SELECTION" ||
//                                ins->type == "PREPOSITION") &&
//                               ins->link_disabled) {
//                        fill_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f);
//                    }
                    
                    nvgStrokeColor(vg, fill_color);
                    nvgStrokeWidth(vg, stroke_width);
                    

                    
                    //mtt::Box child_bounds = ins->bounds;
                    mtt::AABB& aabb = rep->colliders.front()->aabb;
                    mtt::Box child_bounds;
                    child_bounds.tl = aabb.tl;
                    child_bounds.br = aabb.br;
                    vec2 c_center = (child_bounds.tl + child_bounds.br) / 2.0f;
                    bounds = child_bounds;
                    
                    //mat4 model = m::translate(mat4(1.0f), vec3(c_center, 999.0f));
                    //nvgSetModelTransform(vg, m::value_ptr(rep->hierarchy_model_transform));

#define DT_FEEDBACK_BOXES_DEBUG___ 0
#if DT_FEEDBACK_BOXES_DEBUG___
                    sd::save(mtt_renderer);
                    sd::set_color_rgba_v4(mtt_renderer, vec4(0.0f, 0.0f, 1.0f, 1.0f));
                    sd::path_vertex_v3(mtt_renderer, vec3(child_bounds.tl, 999.0f));
                    sd::path_vertex_v3(mtt_renderer, vec3(child_bounds.tl.x, child_bounds.br.y, 999.0f));
                    sd::path_vertex_v3(mtt_renderer, vec3(child_bounds.br.x, child_bounds.br.y, 999.0f));
                    sd::path_vertex_v3(mtt_renderer, vec3(child_bounds.br.x, child_bounds.tl.y, 999.0f));
                    sd::path_vertex_v3(mtt_renderer, vec3(child_bounds.tl, 999.0f));
                    sd::break_path(mtt_renderer);
                    sd::restore(mtt_renderer);
#endif
                    
                    //DrawTalk_World* cw = dt_world;
                    //auto& view_transform = cw->cam.view_transform;
                    
//                    if (ins->kind == "THING_INSTANCE_SELECTION" || ins->kind == "THING_TYPE_SELECTION") {
                    if (ins->kind == "THING_INSTANCE_SELECTION") {
                        
                        usize vertical_offset = 0.0f;
                        usize thing_id_list_count = ins->thing_id_list.size();
                        
                        for (auto it = ins->thing_id_list.begin(); it != ins->thing_id_list.end();) {
                            mtt::Thing* thing = Thing_try_get(world, *it);
                            if (thing == nullptr) {
                                it = ins->thing_id_list.erase(it);
                                Instruction_destroy_proxy_for_thing(ins, world, *it);
                                continue;
                            }

                            mtt::Rep* src_thing_rep = mtt::rep(thing);
                            if (src_thing_rep->colliders.empty() || !thing->do_evaluation) {
                                ++it;
                                continue;
                            }
                            mtt::Thing* proxy = Instruction_get_proxy_for_thing(ins, world, thing->id);
                            mtt::Rep* proxy_rep = nullptr;
                            if (proxy == nullptr) {
                                proxy = mtt::Thing_make_proxy(thing, (mtt::Thing_Make_Proxy_Args) {
                                    .renderer_layer_id = LAYER_LABEL_DYNAMIC_BG_LAYER,
                                    .collision_system = &ui.top_panel.collision_system_world,
                                    .scene_idx = DT_THING_PROXY_SCENE_IDX_SEMANTIC_DIAGRAM_VIEW,
                                });
                                Instruction_set_proxy_for_thing(ins, world, thing->id, proxy->id);
                                
                                proxy->is_user_drawable = false;
                                proxy->is_user_movable = false;
                                proxy_rep = mtt::rep(proxy);
                                
                                for (auto* d_info : proxy_rep->render_data.drawable_info_list) {
                                    d_info->is_enabled = 0;
                                }
                                
                                mtt::unset_is_active(proxy);
                                
                                
                                mtt::send_system_message_deferred(&world->message_passer, mtt::MTT_NONE, proxy->id, nullptr, mtt::Procedure_make([](void* state, mtt::Procedure_Input_Output* io) {
                                    mtt::Thing* thing = io->caller;
                                    if (thing == nullptr) {
                                        return mtt::Procedure_Return_Type();
                                    }
                                    
                                    mtt::Rep* rep = mtt::rep(thing);
                                    for (auto* d_info : rep->render_data.drawable_info_list) {
                                        d_info->is_enabled = 1;
                                    }
                                    
                                    mtt::set_is_active(thing);
                                    
                                    return mtt::Procedure_Return_Type();
                                }, world));
                                
                                
                                // FIXME: creation, addition, and removal of proxies should really go in the callbacks and wherever it's happening in the initial UI generation
                                
                            } else {
                                proxy_rep = mtt::rep(proxy);
                            }
                            ASSERT_MSG(proxy != nullptr, "proxy should exist at this point");

                            
                            
                            auto* ins_box = &rep->colliders[0]->aabb.saved_box;
                            float32 ins_box_width = ins_box->br.x - ins_box->tl.x;
                            
                            auto* src_thing_aabb = &src_thing_rep->colliders[0]->aabb;
                            auto* src_thing_box = &src_thing_aabb->saved_box;
                            
                            float32 src_box_width = m::max_dimension(vec2(src_thing_aabb->half_extent * 2));
                            
//                            float32 scale = (src_box_width > ins_box_width) ? (ins_box_width / src_box_width) : (src_box_width / ins_box_width);
                            float32 scale = 1.0;//(ins_box_width / src_box_width);
                           // scale *= 1.0/view_pip_scale_max_dim;
                            if (src_box_width > ins_box_width) {
                                scale = ins_box_width/src_box_width;
                            } else if (src_box_width < ins_box_width * 0.25f) {
                                scale = src_box_width/(ins_box_width * 0.25f);
                            }
                            
                            bool proxy_selection_is_recorded = dt::selection_is_recorded(proxy);
//                            if (dt::selection_is_recorded(thing) || proxy_selection_is_recorded) {
//                                scale *= 1.5;
//                                scale = m::min(scale, 2.0f);
//                            }
                            
                            mtt::set_pose_transform(proxy, m::scale(Mat4(1.0f), vec3(scale, scale, 1.0f)));

                            //nvgBeginPath(vg);
                            nvgStrokeWidth(vg, stroke_width);
                            nvgBeginPath(vg);
                            
                            auto& src_anchor = proxy_rep->colliders[0]->center_anchor;
                            vec2 src = vec2(src_anchor.x, src_anchor.y);
                            auto& dst_anchor = src_thing_rep->colliders[0]->center_anchor;
                            vec2 dst = vec2(dst_anchor.x, dst_anchor.y);
                            vec2 mid = vec2(src[0], dst[1]);
                            
                            nvgMoveTo(vg, src.x, src.y);

                            auto& proxy_box = proxy_rep->colliders[0]->aabb.saved_box;
                            float32 height = (proxy_box.br.y - proxy_box.tl.y);
                            float32 half_height = height * 0.5;
                            
                            mtt::Thing_set_position(proxy, vec3(c_center[0], child_bounds.br.y + 8 + ((half_height + vertical_offset) * rep->pose_transform_values.scale.y), 999.9f));
                            vertical_offset += 1.25f * height;
                            
                            
                            
                            sd::path_radius(mtt_renderer, arrow_stroke_width);

                            //nvgQuadTo(vg, mid[0], mid[1], dst[0], dst[1]);
                            if (proxy_selection_is_recorded) {
                                vec3 src_canvas = vec3(view_pip * vec4(src, 990.0f, 1.0f));
                                if ((src_canvas.x > scissor_tl.x && src_canvas.x < scissor_br.x &&
                                     src_canvas.y > scissor_tl.y && src_canvas.y < scissor_br.y)) {
                                    
                                    const vec3 dst_canvas = vec3(view_dst * vec4(dst, 990.0f, 1.0f));
                                    
                                    const vec3 mid_canvas = vec3(m::lerp(vec2(src_canvas.x, src_canvas.y), vec2(src_canvas.x, dst_canvas.y), 0.5f), src_canvas.z);
                                    
                                    sd::path_bezier_arrow_head_with_tail(mtt_renderer,
                                                                         src_canvas,
                                                                         m::lerp(src_canvas, mid_canvas, 0.5f),
                                                                         m::lerp(mid_canvas, dst_canvas, 0.5f),
                                                                         dst_canvas,
                                                                         10.0f,
                                                                         10);
                                }
                            }
                            
                            
#if (0)
                            if (ins->refers_to_random) {
                                
                                //nvgText(vg, offset.x + bounds.tl[0], offset.y + bounds.tl[1], label, NULL);
                                
                                nvgText(vg, (src.x + dst.x) * 0.5, (src.y + dst.y) * 0.5, "?", NULL);
                                
                                nvgStrokeColor(vg, nvgRGBAf(color::DARK_PURPLE_1[0], color::DARK_PURPLE_1[1], color::DARK_PURPLE_1[2], 1.0f));
                                
                            } else {
                                nvgStrokeColor(vg, nvgRGBAf(default_pen_color[0], default_pen_color[1], default_pen_color[2], 1.0f));
                            }
#endif
                            nvgStrokeColor(vg, nvgRGBAf(default_pen_color[0], default_pen_color[1], default_pen_color[2], 1.0f));
                            
                            nvgStroke(vg);
                            
                            // TODO: if proxy selected, arrow?
                            
                            ++it;
                        }
                    }
                    
                    nvgStrokeColor(vg, nvgRGBAf(default_pen_color[0], default_pen_color[1], default_pen_color[2], 1.0f));


                    if constexpr ((false)) {
                        if (ins->parent == nullptr) {
                            if (ins->link_disabled) {
                                //nvgStrokeColor(vg, nvgRGBAf(0.5f, 0.0f, 0.0f, 0.5f));
                                nvgStrokeWidth(vg, stroke_width);
                                
                                nvgBeginPath(vg);
                                
                                nvgMoveTo(vg, child_bounds.tl.x, child_bounds.tl.y);
                                nvgLineTo(vg, child_bounds.br.x, child_bounds.br.y);
                                
                                nvgMoveTo(vg, child_bounds.br.x, child_bounds.tl.y);
                                nvgLineTo(vg, child_bounds.tl.x, child_bounds.br.y);
                                
                                nvgStroke(vg);
                            } else {
                                nvgStrokeWidth(vg, stroke_width);
                            }
                        } else {
                            Instruction* parent = ins->parent;
                            auto* parent_rep = mtt::rep(parent->thing_id);
                            mtt::AABB& aabb = parent_rep->colliders.front()->aabb;
                            mtt::Box parent_bounds;
                            parent_bounds.tl = aabb.tl;
                            parent_bounds.br = aabb.br;
                            vec2 c_center = (child_bounds.tl + child_bounds.br) / 2.0f;
                            
                            vec2 p_center = (parent_bounds.tl + parent_bounds.br) / 2.0f;
                            
                            
                            if (ins->link_disabled) {
                                //sd::set_color_rgba_v4(dt->mtt->renderer, vec4(0.5f, 0.0f, 0.0f, 0.5f));
                                //nvgStrokeColor(vg, nvgRGBAf(0.5f, 0.0f, 0.0f, 0.5f));
                                nvgStrokeWidth(vg, stroke_width);
                                
                                if (!((ins->type == "ACTION") ||
                                      (ins->type == "TRIGGER") ||
                                      (ins->type == "RESPONSE") ||
                                      (ins->type == "TRIGGER_RESPONSE"))) {
                                    
                                    nvgBeginPath(vg);
                                    
                                    nvgMoveTo(vg, child_bounds.tl.x, child_bounds.tl.y);
                                    nvgLineTo(vg, child_bounds.br.x, child_bounds.br.y);
                                    
                                    nvgMoveTo(vg, child_bounds.br.x, child_bounds.tl.y);
                                    nvgLineTo(vg, child_bounds.tl.x, child_bounds.br.y);
                                    
                                    nvgStroke(vg);
                                }
                            } else {
                                nvgStrokeWidth(vg, stroke_width);
                            }
                            
                            {
                                // MARK: disabling instruction links for now
                                //                            {
                                //                                switch (ins->link_type) {
                                //                                    case Instruction::PARENT_LINK_TYPE::NONE: {
                                //                                        break;
                                //                                    }
                                //                                    case Instruction::PARENT_LINK_TYPE::FROM: {
                                //                                        vec3 dst = vec3(c_center.x, child_bounds.tl.y, 900.0f);
                                //                                        vec3 src = vec3(p_center.x, parent_bounds.br.y, 900.0f);
                                //                                        sd::path_arrow_head(mtt_renderer, src, m::lerp(src, dst, 0.5f), arrow_radius);
                                //
                                //                                        nvgBeginPath(vg);
                                //
                                //                                        nvgMoveTo(vg, src[0], src[1]);
                                //                                        //                            DrawTalk_World* cw = static_cast<DrawTalk_World*>(mtt_core_ctx()->user_data);
                                //                                        //                            auto& view_transform = cw->cam.view_transform;
                                //                                        //                            vec2 xformed = view_transform * vec4(rep->colliders[0]->center_anchor.x, rep->colliders[0]->center_anchor.y,  0.0f, 1.0f);
                                //
                                //
                                //                                        nvgLineTo(vg, dst[0], dst[1]);
                                //
                                //                                        nvgStroke(vg);
                                //
                                //                                        nvgFillColor(vg, nvgRGBAf(default_pen_color[0], default_pen_color[1], default_pen_color[2], 1.0f));
                                //
                                //                                        //                            {
                                //                                        //                                nvgBeginPath(vg);
                                //                                        //
                                //                                        //                                switch (ins->link_label_type) {
                                //                                        //
                                //                                        //                                    case Instruction::LINK_LABEL_TYPE::NONE: {
                                //                                        //
                                //                                        //                                        break;
                                //                                        //                                    } case Instruction::LINK_LABEL_TYPE::THEN: {
                                //                                        //                                        cstring label = "then";
                                //                                        //
                                //                                        //                                        nvgFillColor(vg, nvgRGBAf(default_pen_color[0], default_pen_color[1], default_pen_color[2], 0.5));
                                //                                        //
                                //                                        //                                        nvgText(vg, offset.x + bounds.tl[0], (offset.y + bounds.tl[1]) - (child_bounds.br[1] - child_bounds.tl[1]), label, NULL);
                                //                                        //
                                //                                        //
                                //                                        //                                        break;
                                //                                        //                                    } case Instruction::LINK_LABEL_TYPE::AND: {
                                //                                        //                                        cstring label = "and";
                                //                                        //
                                //                                        //                                        nvgFillColor(vg, nvgRGBAf(default_pen_color[0], default_pen_color[1], default_pen_color[2], 0.5));
                                //                                        //
                                //                                        //                                        nvgText(vg, offset.x + bounds.tl[0], (offset.y + bounds.tl[1]) - (child_bounds.br[1] - child_bounds.tl[1]), label, NULL);
                                //                                        //
                                //                                        //                                        break;
                                //                                        //                                    } case Instruction::LINK_LABEL_TYPE::DIRECT_OBJECT: {
                                //                                        //
                                //                                        //                                        break;
                                //                                        //                                    } case Instruction::LINK_LABEL_TYPE::AGENT: {
                                //                                        //
                                //                                        //                                        break;
                                //                                        //                                    } case Instruction::LINK_LABEL_TYPE::INDIRECT_OBJECT: {
                                //                                        //
                                //                                        //                                        break;
                                //                                        //                                    } case Instruction::LINK_LABEL_TYPE::OBJECT: {
                                //                                        //
                                //                                        //                                        break;
                                //                                        //                                    } case Instruction::LINK_LABEL_TYPE::GOAL: {
                                //                                        //
                                //                                        //                                        break;
                                //                                        //                                    }
                                //                                        //                                }
                                //                                        //
                                //                                        //
                                //                                        //
                                //                                        //                                nvgFill(vg);
                                //                                        //                                nvgFillColor(vg, nvgRGBAf(default_pen_color[0], default_pen_color[1], default_pen_color[2], 1.0f));
                                //                                        //
                                //                                        //                            }
                                //
                                //                                        break;
                                //                                    }
                                //                                    case Instruction::PARENT_LINK_TYPE::TO: {
                                //                                        vec3 src = vec3(c_center.x, child_bounds.tl.y, 900.0f);
                                //                                        vec3 dst = vec3(p_center.x, parent_bounds.br.y, 900.0f);
                                //                                        sd::path_arrow_head(mtt_renderer, src, m::lerp(src, dst, 0.5f), arrow_radius);
                                //
                                //                                        nvgBeginPath(vg);
                                //
                                //                                        nvgMoveTo(vg, src[0], src[1]);
                                //                                        //                            DrawTalk_World* cw = static_cast<DrawTalk_World*>(mtt_core_ctx()->user_data);
                                //                                        //                            auto& view_transform = cw->cam.view_transform;
                                //                                        //                            vec2 xformed = view_transform * vec4(rep->colliders[0]->center_anchor.x, rep->colliders[0]->center_anchor.y,  0.0f, 1.0f);
                                //
                                //
                                //                                        nvgLineTo(vg, dst[0], dst[1]);
                                //
                                //                                        nvgStroke(vg);
                                //                                        break;
                                //                                    }
                                //                                    case Instruction::PARENT_LINK_TYPE::BIDIRECTIONAL: {
                                //                                        vec3 src = vec3(c_center.x, child_bounds.tl.y, 900.0f);
                                //                                        vec3 dst = vec3(p_center.x, parent_bounds.br.y, 900.0f);
                                //                                        sd::path_arrow_head(mtt_renderer, src, m::lerp(src, dst, 0.25f), arrow_radius);
                                //                                        sd::path_arrow_head(mtt_renderer, dst, m::lerp(dst, src, 0.25f), arrow_radius);
                                //
                                //                                        nvgBeginPath(vg);
                                //
                                //                                        nvgMoveTo(vg, src[0], src[1]);
                                //                                        //                            DrawTalk_World* cw = static_cast<DrawTalk_World*>(mtt_core_ctx()->user_data);
                                //                                        //                            auto& view_transform = cw->cam.view_transform;
                                //                                        //                            vec2 xformed = view_transform * vec4(rep->colliders[0]->center_anchor.x, rep->colliders[0]->center_anchor.y,  0.0f, 1.0f);
                                //
                                //
                                //                                        nvgLineTo(vg, dst[0], dst[1]);
                                //
                                //                                        nvgStroke(vg);
                                //
                                //                                        break;
                                //                                    }
                                //                                }
                                //                            }
                            }
                            
                            sd::break_path(mtt_renderer);
                        }
                    }
                    nvgStrokeColor(vg, fill_color);

                }
                bool speech_confirm_enabled = true;
                for (usize i = 0; i < ins_list.size(); i += 1) {
                    {
                        auto* ins = ins_list[i];
                        auto bounds = ins->bounds;
                        
                        auto* rep = mtt::rep(ins->thing_id);
                        mtt::AABB& aabb = rep->colliders.front()->aabb;
                        mtt::Box child_bounds;
                        child_bounds.tl = aabb.tl;
                        child_bounds.br = aabb.br;
                        vec2 c_center = (child_bounds.tl + child_bounds.br) / 2.0f;
                        bounds = child_bounds;
                        
                        //mat4 model = m::translate(mat4(1.0f), vec3(c_center, 999.0f));
                        //nvgSetModelTransform(vg, m::value_ptr(rep->hierarchy_model_transform));
                        
                        auto fill_color = ins->color_current;
                        fill_color.a = 1.0f;
                        

                        float bound_offset = 2.0f;
                        nvgBeginPath(vg);
                        vec2 dim = vec2(bounds.br.x - bounds.tl.x, bounds.br.y - bounds.tl.y);
                        nvgRoundedRect(vg, bounds.tl.x - bound_offset, bounds.tl.y - bound_offset, dim.x + (2 * bound_offset), dim.y + (2 * bound_offset), 4.0);
                        

                        if (Instruction_requires_selection_of_things(ins)) {
                            nvgStrokeColor(vg, nvgRGBAf(1.0f, 0.0f, 0.0f, 0.5f));
                            nvgStrokeWidth(vg, stroke_width * 4);
                            
                            speech_confirm_enabled = false;
                        } else {
                            nvgStrokeColor(vg, fill_color);
                        }
                        
                        nvgStroke(vg);
                        
                        nvgRoundedRect(vg, offset.x + bounds.tl[0] - bound_offset, offset.y + bounds.tl[1] - bound_offset, dim.x + (2.0f * bound_offset), dim.y + + (2.0f * bound_offset), 4.0);
                        nvgStrokeWidth(vg, stroke_width);
//
//                        nvgFillColor(vg, nvgRGBAf(color_bg[0], color_bg[1], color_bg[2], color_bg[3]));
//
//                        nvgFill(vg);
                        
                        
                        nvgFillColor(vg, nvgRGBAf(ins->text_color[0], ins->text_color[1], ins->text_color[2], ins->text_color[3]));
                        nvgStrokeColor(vg, nvgRGBAf(ins->text_color[0], ins->text_color[1], ins->text_color[2], ins->text_color[3]));
                        
                        cstring label = ins->display_label.c_str();
                        
                        //nvgFillColor(vg, nvgRGBAf(default_pen_color[0], default_pen_color[1], default_pen_color[2], 1.0f));
                        nvgText(vg, offset.x + bounds.tl[0], offset.y + bounds.tl[1], label, NULL);
                        
                        nvgStrokeColor(vg, nvgRGBAf(default_pen_color[0], default_pen_color[1], default_pen_color[2], 1.0f));
                        
                        
//                        bool should_warn_on_singular_specific = Instruction_warn_multiple_for_specific_single(ins);
//                        if (should_warn_on_singular_specific) {
//                            
//                            nvgBeginPath(vg);
//                            
//                            float32 alert_fluctuation = 0.5*m::sin01(6 * (float32)core_ctx->time_seconds);
//                            
//                            nvgCircle(vg, bounds.tl.x - bound_offset + (bounds.br.x - bounds.tl.x)*.5, bounds.tl.y - bound_offset - ((bounds.br.y - bounds.tl.y)), (layout_padding_root * (alert_fluctuation + 1)) * 0.5);
//                            nvgFill(vg);
//                            
//                            cstring q_mark = "?";
//                            
//                            nvgBeginPath(vg);
//                            nvgFillColor(vg, nvgRGBAf(1, 1, 1, 1.0f));
//                            
//                            nvgSave(vg);
//                            
//                            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_CENTER);
//                            
//                            nvgText(vg, bounds.tl.x - bound_offset + (bounds.br.x - bounds.tl.x)*.5, bounds.tl.y - bound_offset - ((bounds.br.y - bounds.tl.y) * 0.75), q_mark, q_mark + 1);
//                            
//                            nvgFill(vg);
//                            
//                            nvgRestore(vg);
//                        }
                        
                    }
                }
                if (!speech_confirm_enabled) {
                    SPEECH_CONFIRM_UI_BUTTON->alt_state = true;
                    mtt::unset_should_render(SPEECH_CONFIRM_THING);
#if !DT_DEFINE_FLEXIBLE_IGNORE_SELECTIONS
                    mtt::unset_is_active(SPEECH_CONFIRM_THING);
#endif
                }
                
                sd::end_path(mtt_renderer);
            }
            nvgRestore(vg);
        }
    }
    
    
    
    nvgRestore(vg);

    auto scissor = sd::Viewport_to_Scissor_Rectangle(core_ctx->viewport);
    nvgSetHWScissor(vg, scissor.x, scissor.y, scissor.width, scissor.height);
}

    
void UI_draw_post(void)
{
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    auto* vg = nvgGetGlobalContext();
    
    UI& ui = dt->ui;
    mtt::World* world = dt->mtt;
    
    MTT_Core* core = mtt_core_ctx();
    sd::Renderer* renderer = core->renderer;
    
    DrawTalk_World* dt_world = static_cast<DrawTalk_World*>(mtt_core_ctx()->user_data);
    dt::Control_System* control = &dt_world->controls;
    
    bool all_required_actions_chosen = false;
    
    
    mtt::Thing* context_view_thing = mtt::Thing_get(world, ui.context_view.thing_id);
    bool selected_this_frame = ui.context_view.selected_this_frame;
    ui.context_view.selected_this_frame = false;
    // MARK: action substitution API
    
    nvgSave(vg);
    
    
    
    auto& viewport = dt->core->viewport;
    mtt::Thing_ID el_id =  dt->ui.element_menu.thing_id;
    mtt::Thing* substitution_menu = mtt::Thing_try_get(dt->mtt, el_id);
//    if ((mtt::priority_layer_is_le(dt->ui.element_menu.priority_layer, mtt::get_priority_layer(dt->mtt)) && substitution_menu != nullptr) && !dt->lang_ctx.eval_ctx.instructions_to_disambiguate.empty()) MTT_UNLIKELY {
    if (substitution_menu != nullptr && dt->ui.element_menu.is_active) MTT_UNLIKELY {
        
        
        using m::value_ptr;
        
        float32 layer_z_back = 999.98f;
        float32 layer_z_front = 999.99f;
        
        mat4 identity = mat4(1.0f);
        
        float lineh = 0;
        nvgTextMetrics(vg, NULL, NULL, &lineh);
        
        nvgSetViewTransform(vg, value_ptr(identity));
        nvgSetModelTransform(vg, value_ptr(identity));
        
        auto& pen_state = substitution_menu->input_handlers.state[mtt::INPUT_MODALITY_FLAG_PEN];
        auto& touch_state = substitution_menu->input_handlers.state[mtt::INPUT_MODALITY_FLAG_TOUCH];
        auto& direct_touched_pos = touch_state.prev_canvas_pos;
        auto& pen_touched_pos = pen_state.prev_canvas_pos;
        
        auto& to_disambiguate_list = dt->lang_ctx.eval_ctx.instructions_to_disambiguate;
        
        sd::save(renderer);
        {

            mtt::set_is_active(substitution_menu);
            
            
            auto& dis = to_disambiguate_list.front();
            auto* ins = dis.ins;
            
            
            mat4& view_transform = ui.top_panel.cam.view_transform;
            
            mat4 inv_scale;
            {
                vec3 scale;
                quat orientation;
                vec3 translation;
                vec3 skew;
                vec4 perspective;
                m::decompose(view_transform, scale, orientation, translation, skew, perspective);
                
                inv_scale = m::inverse(m::scale(mat4(1.0f), m::min(scale, vec3(1,1,1))));
            }
            
            DT_Evaluation_Context* eval_ctx = &dt->lang_ctx.eval_ctx;
            
            
            float32 alert_fluctuation = 0.5*m::sin01(6 * (float32)core->time_seconds);
            
            nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
            nvgFontSize(vg, 32);
            
            cstring str_for_alert = "!";
            cstring str_for_unknown = "?";
            sd::set_color_rgba_v4(renderer, vec4(1.0f, 0.0f, 0.0f, 1.0f));
            
            
            auto prev_layer = sd::get_render_layer(renderer);
            sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_BG_LAYER_PER_FRAME);
            
            vec3 parent_pos = vec3(0.0f, 0.0f, 0.0f);
            {
                auto& scissor_bounds = ui.top_panel.base.bounds;
                vec2 scissor_tl = vec2(scissor_bounds.tl.x, scissor_bounds.tl.y);
                vec2 scissor_br = vec2(scissor_bounds.tl.x + scissor_bounds.dimensions.x, scissor_bounds.tl.y + scissor_bounds.dimensions.y);
                
                nvgSetHWScissor(vg, scissor_bounds.tl.x, scissor_bounds.tl.y, scissor_bounds.dimensions.x, scissor_bounds.dimensions.y);
                
                float32 offset = (layout_padding_root * (alert_fluctuation + 1)) * 0.5;
                
                parent_pos = *mtt::access<vec3>(mtt::Thing_get(world, eval_ctx->root_thing_id), "position");
                {
                    mat4 alert_xform = m::translate(mat4(1.0f), vec3(vec2(parent_pos), 0)) * inv_scale;
                    mat4 text_mat = view_transform * alert_xform;
                    nvgSetModelTransform(vg, value_ptr(text_mat));
                    nvgText(vg, 0, 0, str_for_unknown, str_for_unknown + 1);
                    
                    sd::begin_polygon(renderer);
                    sd::circle(renderer, offset / 2, vec3(vec2(0.0f), layer_z_front));
                    sd::end_polygon(renderer)->set_transform(alert_xform);
                }
                
  
                // MARK: this would show alert icon over the instructions, but I don't like them
//                const usize el_count = to_disambiguate_list.size();
//                for (usize el_i = 0; el_i < el_count; el_i += 1) {
//                    mtt::Thing* th = mtt::Thing_try_get(world, to_disambiguate_list[el_i].ins->thing_id);
//                    if (th == nullptr) MTT_UNLIKELY {
//                        continue;
//                    }
//                    //mtt::Rep* r = mtt::rep(th);
//                    mat4 alert_xform = m::translate(mat4(1.0f), vec3(vec2(mtt::access<vec3>(th, "position")->x, parent_pos.y), 0)) * inv_scale;
//                    mat4 text_mat = view_transform * alert_xform;
//                    nvgSetModelTransform(vg, value_ptr(text_mat));
//                    nvgText(vg, 0, 0, str_for_unknown, str_for_unknown + 1);
//                    
//                    sd::begin_polygon(renderer);
//                    sd::circle(renderer, offset, vec3(vec2(0.0f), layer_z_front));
//                    sd::end_polygon(renderer)->set_transform(alert_xform);
//                }
                
                
            }
            
            
            
            sd::begin_polygon(renderer);
            sd::set_color_rgba_v4(renderer, vec4(0.4f, 0.4f, 0.4f, 0.7f));
            
            
            
#define USE_OLD_SUBSTITUTION_UI (false)
            
            
            
            usize per = ceil(m::sqrt((float32)dis.action_alternatives.size()));
            
            usize column_count = per;
            usize row_count    = per;
            float32 column_width = (viewport.width * 0.5f) / column_count;
            float32 row_height = (viewport.height * 0.5f) / row_count;
            
            const float64 pad_factor = (7.0f/8.0f);
            const float64 pad_offset_factor = 0.5f - (0.5f * pad_factor);//0.5f *
            const vec2 corner_offset = vec2((column_width * pad_offset_factor), (row_height * pad_offset_factor) + row_height * 0.15f);
            const vec2 dimensions = vec2(column_width * pad_factor, row_height * pad_factor);
            
            {
                
                if (dis.results_ready) {


                    {
                        nvgSetModelTransform(vg, value_ptr(identity));
                        nvgSetViewTransform(vg, value_ptr(view_transform));
                        {
                            mtt::Thing* th = mtt::Thing_try_get(world, dis.ins->thing_id);
                            if (th != nullptr) MTT_LIKELY {
                                mtt::Rep* r = mtt::rep(th);
                                auto& aabb = r->colliders.front()->aabb;

                                
                                vec2 tl = aabb.tl + vec2(0, eval_ctx->layout.pad.x);//saved_box.tl;
                                vec2 dimensions = aabb.br - aabb.tl;//saved_box.dimensions;
                                dimensions *= vec2(4, 2);

                                float32 height = dimensions.y + eval_ctx->layout.pad.x * 0.5;
                                
                                //                            sd::begin_polygon(renderer);
                                //                            sd::circle(renderer, 10, vec3(vec2(0.0f), layer_z_front));
                                //                            sd::end_polygon(renderer)->set_transform(alert_xform);
                                
                                
                                
//                                vec2 world_touch =  vec4(direct_touched_pos, 0, 1);
//                                vec2 world_pen =  vec4(pen_touched_pos, 0, 1);
                                
//                                sd::set_color_rgba_v4(renderer, vec4(0.0f, 0.0f, 1.0f, 1.0f));
//                                sd::circle(renderer, 10, vec3(pen_touched_pos, 999));
//                                sd::circle(renderer, 10, vec3(direct_touched_pos, 999));
//                                sd::set_color_rgba_v4(renderer, vec4(0.0f, 1.0f, 0.0f, 1.0f));
//                                sd::circle(renderer, 10, vec3(world_touch, 999));
//                                sd::circle(renderer, 10, vec3(world_pen, 999));
                                vec2 extents = vec2(dimensions.x, (height * dis.action_alternatives.size()) - eval_ctx->layout.pad.x * 0.5);
                                
                                
                                vec2 TL = view_transform * vec4(tl + vec2(0, height), 0,1);
                                vec2 BR = view_transform * vec4(tl + vec2(0, height) + extents, 0,1);
                                vec2 CENTER = (TL + BR) * 0.5;
                                vec2 SCALE = vec2(BR - TL);
                                
                                mtt::Thing_set_position(substitution_menu, vec3(CENTER, layer_z_back));
                                mtt::set_pose_transform(substitution_menu, m::scale(mat4(1.0f), vec3(SCALE, 1.0f)));
                                sd::set_color_rgba_v4(renderer, vec4(1.0f, 0.4f, 0.4f, 0.5f));
                                
                                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_CENTER);
                                nvgFontSize(vg, 24);
                                {
                                    vec2 TL = tl + vec2(0, height);
                                    if (!dis.action_alternatives.empty()) {
                                        cstring label = "unknown, try:";
                                        const usize len = 13;
                                        if (mtt::color_scheme() == mtt::COLOR_SCHEME_LIGHT_MODE) {
                                            nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 1.0f));
                                            sd::rectangle_rounded(renderer, TL, extents, layer_z_front, 0.25f, 8);
                                            nvgText(vg, TL.x, TL.y - eval_ctx->layout.pad.x, label, label + len);
                                            nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
                                        } else {
                                            nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
                                            sd::rectangle_rounded(renderer, TL, extents, layer_z_front, 0.25f, 8);
                                            nvgText(vg, TL.x, TL.y - eval_ctx->layout.pad.x, label, label + len);
                                            nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 1.0f));
                                        }
                                        sd::rectangle_rounded(renderer, tl + vec2(0, height), extents, layer_z_front, 0.25f, 8);
                                    } else {
                                        cstring label = "unknown action";
                                        const usize len = 14;
                                        if (mtt::color_scheme() == mtt::COLOR_SCHEME_LIGHT_MODE) {
                                            nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 1.0f));
                                            nvgText(vg, TL.x, TL.y - eval_ctx->layout.pad.x, label, label + len);
                                            nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
                                        } else {
                                            nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
                                            nvgText(vg, TL.x, TL.y - eval_ctx->layout.pad.x, label, label + len);
                                            nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 1.0f));
                                        }
                                    }
                                }

                                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_CENTER);
                                nvgFontSize(vg, 32);
                                
                                
                                tl.y += height;
                                for (usize entry_idx = 0; entry_idx < dis.action_alternatives.size(); entry_idx += 1) {
                                    float box[4] = {
                                        tl.x,
                                        tl.y,
                                        tl.x + dimensions.x,
                                        tl.y + dimensions.y,
                                    };
                                    
                                    vec2 TL = view_transform * vec4(box[0], box[1], 0,1);
                                    vec2 BR = view_transform * vec4(box[2], box[3], 0,1);
                                    box[0] = TL.x;
                                    box[1] = TL.y;
                                    box[2] = BR.x;
                                    box[3] = BR.y;
                                    
                                    bool collided_touch = ((ui.element_menu.touch_began || ui.element_menu.touch_ended) && mtt::Box_point_check(box, direct_touched_pos));
                                    bool collided_pen = ((ui.element_menu.pen_began || ui.element_menu.pen_ended) && mtt::Box_point_check(box, pen_touched_pos));
                                    
//                                    bool collided_world_touch = ((ui.element_menu.touch_began || ui.element_menu.touch_ended) && mtt::Box_point_check(box, world_touch));
//                                    bool collided_world_pen = ((ui.element_menu.pen_began || ui.element_menu.pen_ended) && mtt::Box_point_check(box, world_pen));

//                                        sd::rectangle_rounded(renderer, tl, dimensions, layer_z_front, 0.4f, 8);
                                        {
                                            vec2 center = vec2(tl.x, (tl.y + tl.y + (1.5*dimensions.y)) * 0.5);
                                            auto& to_display = dis.action_alternatives[entry_idx];
                                            
                                            mtt::String& label = to_display.behavior->key_primary;
                                            cstring label_as_cstr = label.c_str();
                                            
                                            
                                            float bounds[4];
                                            nvgTextBounds(vg, center.x, center.y, label_as_cstr, label_as_cstr + label.size(), bounds);
                                            nvgText(vg, center.x, center.y, label_as_cstr, label_as_cstr + label.size());
                                            if (!to_display.behavior->key_secondary.empty()) {
                                                mtt::String secondary_label = "- " + to_display.behavior->key_secondary;
                                                cstring secondary_label_as_cstr = secondary_label.c_str();
                                                
                                                nvgTextBounds(vg, center.x + bounds[2] + 2, center.y, secondary_label_as_cstr, secondary_label_as_cstr + secondary_label.size(), bounds);
                                                
                                                nvgText(vg, center.x + bounds[2] + 2, center.y, secondary_label_as_cstr, secondary_label_as_cstr + secondary_label.size());
                                            }
                                        }
                                    
                                    sd::set_color_rgba_v4(renderer, LABEL_COLOR_ACTION);
                                    
                                    bool collided = collided_touch || collided_pen;
                                    if (collided) {
                                        if ((collided_touch && ui.element_menu.touch_ended) ||
                                            (collided_pen && ui.element_menu.pen_ended)) {
                                            {
                                                auto& lang_ctx = dt->lang_ctx;
                                                DT_Behavior_Catalogue& bc = lang_ctx.behavior_catalogue;
                                                auto& alternative = dis.action_alternatives[entry_idx];
                                                auto& behavior_label = alternative.behavior->label[0];
                                                
                                                
                                                auto& entry = *bc.insert(dis.ins->prop.front()->label, "", true, behavior_label.first, behavior_label.second);
                                                MTT_UNUSED(entry);
                                                
                                            }
                                            
                                            if (dis.ins->annotation == "unknown") {
                                                dis.ins->annotation = "";
                                            }
                                            auto to_find = dis.ins->prop.front()->label;
                                            to_disambiguate_list.erase(to_disambiguate_list.begin());
                                            for (usize idx = 0; idx < to_disambiguate_list.size();) {
                                                auto* el = to_disambiguate_list[idx].ins;
                                                if (el->prop.front()->label == to_find) {
                                                    el->annotation = "";
                                                    to_disambiguate_list.erase(to_disambiguate_list.begin() + idx);
                                                } else {
                                                    idx += 1;
                                                }
                                            }
                                            
                                            
                                            //                                dt::rebuild_feedback_ui_from_modified_instructions(dt, dt->lang_ctx.eval_ctx);
                                            
                                            if (to_disambiguate_list.empty()) {
                                                all_required_actions_chosen = true;
                                            }
                                        }
                                        sd::set_color_rgba_v4(renderer, vec4(vec3(LABEL_COLOR_ACTION) * 0.75f, 1.0));
                                    }

                                    
                                    sd::rectangle_rounded(renderer, tl, dimensions, layer_z_front, 0.25f, 8);
                                        
//                                    nvgText(vg, tl.x, (tl.y + tl.y + (1.5*dimensions.y)) * 0.5, str_for_unknown, str_for_unknown + 1);
                                    
                                    
                                    
//                                    else if (collided_world_touch || collided_world_pen) {
//                                        nvgText(vg, tl.x, tl.y + height - eval_ctx->layout.pad.x  * 0.5, str_for_unknown, str_for_unknown + 1);
//
//                                        MTT_print("%s\n", "collided worldspace\n");
//                                    }
////
                                    tl.y += height;
                                }
                            }
                        }
                    }
                    sd::end_polygon(renderer);
                    nvgSetViewTransform(vg, value_ptr(identity));
                }
            }
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_CENTER);

            
            auto scissor = sd::Viewport_to_Scissor_Rectangle(core->viewport);
            nvgSetHWScissor(vg, scissor.x, scissor.y, scissor.width, scissor.height);
            sd::set_render_layer(renderer, prev_layer);
            
        }
        sd::restore(renderer);
    } else if (substitution_menu != nullptr) {
        mtt::unset_is_active(substitution_menu);
    }
    nvgRestore(vg);
    
    
    ui.element_menu.pen_ended = false;
    ui.element_menu.touch_ended = false;
    
    if (all_required_actions_chosen) {
        leave_prompt_for_different_actions(dt);
        handle_speech_phase_change(*dt);
    }
    
    //nvgSave(vg);
    {
//        mat4 identity = mat4(1.0f);
//        nvgSetViewTransform(vg, value_ptr(identity));
//        nvgSetModelTransform(vg, value_ptr(identity));
        Panel& panel = ui.margin_panels[0];
        
        sd::save(renderer);
        
        sd::set_render_layer(renderer, LAYER_LABEL_DYNAMIC_CANVAS);
        
        if (!panel.text.selection_rectangles.empty()
            || !panel.text.selection_rectangles_to_handle.empty()) {
            
            
            
            sd::begin_polygon(renderer);
            
            constexpr const float32 radius = 4.0f;
            constexpr const float32 half_radius = radius * 0.5f;
            constexpr const bool DRAW_SELECTION = false;
            if (!panel.text.selection_rectangles.empty()) {
                for (const auto& [k, v] : panel.text.selection_rectangles) {
                    const Selection_Rectangle& r = v;
                    vec2 tl = r.start;
                    vec2 dimensions = r.extent;
                    
                    const Selection_Intersection* intersection = &r.intersection_state;
                    
                    const float32* head_bounds = intersection->head_bounds;
                    const float32* tail_bounds = intersection->tail_bounds;
                    
                    if constexpr ((DRAW_SELECTION)) {
                        sd::set_color_rgba_v4(renderer, dt::SELECTION_RECTANGLE_COLOR);
                        sd::rectangle(renderer, tl, dimensions, 999.9f);
                    }
                    
                    sd::set_color_rgb(renderer, dt::SELECTION_RECTANGLE_COLOR);
                    
                    // selection markers
                    if (!intersection->head_out_of_bounds) {
                        sd::rectangle(renderer, vec2(head_bounds[0] - radius, head_bounds[1] - radius), vec2(radius, radius + radius + head_bounds[3] - head_bounds[1]), 999.9f);
                        sd::circle(renderer, radius, vec3(vec2(head_bounds[0] - half_radius, head_bounds[1] - half_radius), 999.9f));
                    }
                    if (!intersection->tail_out_of_bounds) {
                        sd::rectangle(renderer, vec2(tail_bounds[2], tail_bounds[1] - radius), vec2(radius, radius + radius + tail_bounds[3] - tail_bounds[1]), 999.9f);
                        
                        sd::circle(renderer, radius, vec3(vec2(tail_bounds[2] + half_radius, tail_bounds[3] + half_radius), 999.9f));
                    }
                    
                    
                    //sd::rectangle(renderer, vec2(tail_bounds[0], tail_bounds[1]), vec2(tail_bounds[2], tail_bounds[3]), 999.9f);
//                    sd::rectangle(renderer, vec2(tail_rectangle[0], tail_rectangle[1]), vec2(tail_rectangle[2], tail_rectangle[3]) - vec2(tail_rectangle[0], tail_rectangle[1]), 999.0f);
//                    sd::rectangle(renderer, vec2(tail_rectangle[0], tail_rectangle[1]), vec2(tail_rectangle[2], tail_rectangle[3])
//                                  , 999.0f);
                    
                    
//                    sd::set_color_rgba_v4(renderer, color::set_alpha(dt::SELECTION_RECTANGLE_COLOR, 1.0f));
//                    sd::circle(renderer, radius, vec3(tl, 999.99f));
//                    sd::circle(renderer, radius, vec3(tl + dimensions, 999.99f));
                }
            }
            
            if (!panel.text.selection_rectangles_to_handle.empty()) {
                for (const Selection_Rectangle& r : panel.text.selection_rectangles_to_handle) {
                    vec2 tl = r.start;
                    vec2 dimensions = r.extent;
                    
                    const Selection_Intersection* intersection = &r.intersection_state;
                    MTT_print("i_begin=[%llu] i_end=[%llu]\n", intersection->i_begin, intersection->i_end);
                    if constexpr ((DRAW_SELECTION)) {
                        sd::set_color_rgba_v4(renderer, dt::SELECTION_RECTANGLE_COLOR);
                        sd::rectangle(renderer, tl, dimensions, 999.9f);
                    }
                }
                mtt::clear(&panel.text.selection_rectangles_to_handle);
            }
            
            sd::end_polygon(renderer);
            
        }
        
        
        // MARK: context view
        {

            
            if (mtt::is_active(context_view_thing)) {
                
                sd::begin_drawable(renderer);
                sd::set_color_rgba_v4(world->renderer, dt::PANEL_COLOR);
                sd::begin_polygon_no_new_drawable(renderer);
                const vec2 border_offset = vec2(0,1);
                const vec2 tl = ui.context_view.box.tl;
                const vec2 br = ui.context_view.box.br;
                sd::rectangle_w_corners(world->renderer, tl - border_offset, br + border_offset, -999.9f);
                
                if (dt->ui.context_view.mode != dt->ui.context_view.mode_prev) {
                    handle_context_sensitive_command_clear(dt);
                    dt->ui.context_view.mode_prev = dt->ui.context_view.mode;
                }
                
                if (handle_context_sensitive_command(dt) || dt->ui.context_view.mode == CONTEXT_VIEW_MODE_RULES || dt->ui.context_view.mode == CONTEXT_VIEW_MODE_ACTIONS) {
                    
                    if (dt->ui.context_view.mode == CONTEXT_VIEW_MODE_SEARCH) {
                        const float32 padding = viewport.width * 0.008f;
                        
                        const vec2 tl_inner = tl + padding;
                        const vec2 br_inner = br - padding;
                        
                        float64 w = (br_inner.x - tl_inner.x);
                        float64 h = (br_inner.y - tl_inner.y);
                        //const float64 area_avail = w * h;
                        auto& ctx_view = ui.context_view;
                        auto& to_display = ctx_view.things_to_display;
                        usize count = to_display.size();
                        
                        float64 side = 1;
                        {
                            double x=w, y=h, n=count;//values here
                            double px=ceil(sqrt(n*x/y));
                            double sx,sy;
                            if(floor(px*y/x)*px<n)    //does not fit, y/(x/px)=px*y/x
                                sx=y/ceil(px*y/x);
                            else
                                sx= x/px;
                            double py=ceil(sqrt(n*y/x));
                            if(floor(py*x/y)*py<n)    //does not fit
                                sy=x/ceil(x*py/y);
                            else
                                sy=y/py;
                            side = m::max(sx, sy);
                        }
                        
                        {
                            auto* proxy_scene = Thing_Proxy_Scene_for_idx(world, DT_THING_PROXY_SCENE_IDX_CONTEXT_PANEL_VIEW);
                            auto* map = &proxy_scene->thing_to_proxy_map;
                            auto& things_to_display = ui.context_view.things_to_display;
                            ui.context_view.thing_proxies_to_align.clear();
                            for (usize i = 0; i < things_to_display.size(); i += 1) {
                                bool deferred = false;
                                auto find_it = map->find(things_to_display[i]);
                                mtt::Thing* thing_src = mtt::Thing_try_get(world, things_to_display[i]);
                                if (thing_src == nullptr) {
                                    continue;
                                }
                                mtt::Thing* proxy = nullptr;
                                //mtt::Rep* proxy_rep = nullptr;
                                if (find_it == map->end()) {
                                    deferred = true;
                                    // create proxy
                                    proxy = mtt::Thing_make_proxy(thing_src, (mtt::Thing_Make_Proxy_Args) {
                                        .renderer_layer_id = LAYER_LABEL_DYNAMIC_CANVAS,
                                        .collision_system = &world->collision_system_canvas,
                                        .scene_idx = DT_THING_PROXY_SCENE_IDX_CONTEXT_PANEL_VIEW,
                                    });
                                    
                                    
                                    proxy->is_user_drawable = false;
                                    proxy->is_user_movable = false;
                                    proxy->is_user_destructible = false;
                                    
                                    mtt::Thing_defer_enable_to_next_frame(world, proxy);
                                    
                                    proxy->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                                        mtt::Thing* thing_src = mtt::Thing_try_get(mtt::world(thing), thing->mapped_thing_id);
                                        if (thing_src == nullptr) {
                                            return;
                                        }
                                        
                                        auto* rep = mtt::rep(thing_src);
                                        
                                        DrawTalk_World* dt_world = DrawTalk_World_ctx();
                                        auto* core = mtt_core_ctx();
                                        
                                        auto* cam = &dt_world->cam;
                                        
                                        vec3 d_scale;
                                        quat d_orientation;
                                        vec3 d_translation;
                                        vec3 d_skew;
                                        vec4 d_perspective;
                                        {
                                            m::decompose(rep->hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                        }
                                        
                                        
                                        auto vp_offset = vec3(core->viewport.width * 0.5f, core->viewport.height * 0.5f, 0.0f);
                                        vec3 translation = d_translation - vp_offset;
                                        
                                        
                                        float32 scale = 1.0;
                                        {
                                            auto* src_thing_rep = mtt::rep(thing_src);
                                            auto* src_thing_aabb = &src_thing_rep->colliders[0]->aabb;
                                            //auto* src_thing_aabb_saved = &src_thing_rep->colliders[0]->aabb;
                                            vec2 src_box_max_min;
                                            m::max_min_dimensions(vec2(src_thing_aabb->half_extent * 2), &src_box_max_min);
                                            //                            float32 src_box_max_width = m::max_dimension(vec2(src_thing_aabb->half_extent * 2));
                                            //                            float32 src_box_min_width = m::min_dimension(vec2(src_thing_aabb->half_extent * 2));
                                            
                                            
                                            
                                            //                            float32 scale = 1.0;
                                            //
                                            //                            if (src_box_max_min.x < side || src_box_max_min.y < side) {
                                            //                                scale = (side / src_box_max_min.x) * 0.5f;
                                            //                            } else if (src_box_max_min.x > side || src_box_max_min.y > side) {
                                            //                                scale = (side / src_box_max_min.x) * 0.5f;
                                            //                            }
                                            
                                            auto side = m::min(core->viewport.width, core->viewport.height);
                                            if (src_box_max_min.x < side || src_box_max_min.y < side
                                                || src_box_max_min.x > side || src_box_max_min.y > side) {
                                                scale = (side / src_box_max_min.x) * 0.5f;
                                            }
                                        }
                                        
                                        //                                    cam->cam_transform =  m::translate(Mat4(1.0f), translation) * m::translate(Mat4(1.0f), translation) * m::scale(Mat4(1.0f), vec3(0.25f, 0.25f, 1.0f) * d_scale) - m::translate(Mat4(1.0f), translation);
                                        cam->cam_transform =  m::translate(Mat4(1.0f), translation);
                                        
                                        core->view_position -= d_translation + vp_offset;
                                        mtt::calc_view_matrix(cam);
                                        return;
                                    };
                                    
                                    proxy->input_handlers.on_pen_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
                                        
                                        mtt::Thing* thing_src = Thing_mapped_from_proxy(thing);
                                        if (thing_src == nullptr) {
                                            return mtt::Input_Handler_Return(true);
                                        }
                                        
                                        auto* rep = mtt::rep(thing_src);
                                        
                                        DrawTalk_World* dt_world = DrawTalk_World_ctx();
                                        auto* core = mtt_core_ctx();
                                        
                                        Input* const input = &core->input;
                                        Input_Record* u_input = &input->users[0];
                                        
                                        switch (pointer_op_current(u_input)) {
                                            case UI_POINTER_OP_DRAW: {
                                                //pointer_op_set_current(u_input, UI_POINTER_OP_ERASE);
                                                break;
                                            }
                                            case UI_POINTER_OP_ERASE: {
                                                //pointer_op_set_current(u_input, UI_POINTER_OP_DRAW);
                                                mtt::Thing_destroy(thing_src);
                                                break;
                                            }
                                        }
                                        
                                        return mtt::Input_Handler_Return(true);
                                    };
                                    
                                    proxy->input_handlers.on_pen_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                                        
                                        mtt::Thing* thing_src = Thing_mapped_from_proxy(thing);
                                        if (thing_src == nullptr) {
                                            return;
                                        }
                                        
                                        auto* core = mtt_core_ctx();
                                        Input* const input = &core->input;
                                        Input_Record* u_input = &input->users[0];
                                        
                                        switch (pointer_op_current(u_input)) {
                                            case UI_POINTER_OP_ERASE: {
                                                return;
                                            }
                                        }
                                        
                                        dt::DrawTalk* DT = dt::DrawTalk::ctx();
                                        
                                        usize selected_things_count = DT->scn_ctx.selected_things.size();
                                        
                                        
                                        
                                        
                                        
                                        auto* rep = mtt::rep(thing_src);
                                        
                                        DrawTalk_World* dt_world = DrawTalk_World_ctx();
                                        
                                        
                                        auto* copy = mtt::Thing_copy(thing_src);
                                        
                                        mtt::Thing_set_position(copy, world_pos);
                                    };
                                    
                                    
                                    
                                    
                                    map->insert({mtt::thing_id(thing_src), mtt::thing_id(proxy)});
                                    ctx_view.thing_to_element.insert({mtt::thing_id(thing_src), (dt::Context_View::Element_Record){ mtt::thing_id(proxy), i} });
                                    
                                } else {
                                    // reuse proxy
                                    proxy = mtt::Thing_try_get(world, *((find_it->second).begin() ));
                                    if (proxy == nullptr) {
                                        continue;
                                    }
                                }
                                auto* src_thing_rep = mtt::rep(thing_src);
                                if (src_thing_rep->colliders.empty()) {
                                    continue;
                                }
                                auto* src_thing_aabb = &src_thing_rep->colliders[0]->aabb;
                                vec2 src_box_max_min;
                                m::max_min_dimensions(vec2(src_thing_aabb->half_extent * 2), &src_box_max_min);
                                //                            float32 src_box_max_width = m::max_dimension(vec2(src_thing_aabb->half_extent * 2));
                                //                            float32 src_box_min_width = m::min_dimension(vec2(src_thing_aabb->half_extent * 2));
                                
                                
                                
                                //                            float32 scale = 1.0;
                                //
                                //                            if (src_box_max_min.x < side || src_box_max_min.y < side) {
                                //                                scale = (side / src_box_max_min.x) * 0.5f;
                                //                            } else if (src_box_max_min.x > side || src_box_max_min.y > side) {
                                //                                scale = (side / src_box_max_min.x) * 0.5f;
                                //                            }
                                float32 scale = 1.0;
                                if (src_box_max_min.x < side || src_box_max_min.y < side
                                    || src_box_max_min.x > side || src_box_max_min.y > side) {
                                    scale = (side / src_box_max_min.x) * 0.5f;
                                }
                                //scale = scale * 1.0f;
                                
                                //                            mtt::set_pose_transform(proxy, m::scale(Mat4(1.0f), vec3(scale, scale, 1.0f)));
                                
                                ui.context_view.thing_proxies_to_align.push_back({proxy, scale, deferred});
                                //proxy_rep = mtt::rep(proxy);
                            }
                        }
                        
                        //const vec2 dimensions = vec2(side, side) - padding;
                        const float32 depth = ui.context_view.depth + 1.0f;
                        const float64 half_side = side * 0.5;
                        {
                            const vec4 inner_color = vec4(vec3(bg_color_default()), 1.0f);
                            const vec4 outer_color = vec4(vec3(pen_color_default()) + vec3(0.2f), 1.0f);
                            
                            usize i = 0;
                            const float32 padding_thickness = 4;
                            const vec2 padding_inner = vec2(padding_thickness);
                            const vec2 padding_dim_inner = 2*vec2(padding_thickness);
                            const vec2 padding_outer = 2*padding_inner;
                            const vec2 padding_dim_outer = 2*padding_dim_inner;
                            for (float64 y = tl_inner.y; i < count; y += side) {
                                for (float64 x = tl_inner.x; x + side <= br_inner.x; x += side) {
                                    if (ui.context_view.thing_proxies_to_align.size() <= i) {
                                        goto LABEL_LOOP_END;
                                    }
                                    auto* info = &ui.context_view.thing_proxies_to_align[i];
                                    
                                    
                                    if (!info->position_set_deferred) {
                                        auto* box = &mtt::rep(info->proxy)->colliders[0]->aabb.saved_box;
                                        //sd::rectangle_rounded(world->renderer, vec2(x, y), dimensions, 0.0f, 0.2f, 4);
                                        
                                        sd::set_color_rgba_v4(world->renderer, outer_color);
                                        sd::rectangle_rounded(world->renderer, box->tl - padding_outer, (box->br - box->tl) + padding_dim_outer, 0, 0.2f, 4);
                                        
                                        sd::set_color_rgba_v4(world->renderer, inner_color);
                                        sd::rectangle(world->renderer, box->tl - padding_inner, (box->br - box->tl) + padding_dim_inner, 0.0001f);
                                    }
                                    
                                    vec3 pos_center = vec3(vec2(x, y) + ((float32)half_side), depth);
                                    
                                    
                                    mtt::Thing_set_position(info->proxy, pos_center);
                                    
                                    mtt::set_pose_transform(info->proxy, m::scale(Mat4(1.0f), vec3(info->scale, info->scale, 1.0f)));
                                    
                                    i += 1;
                                    if (i >= count) {
                                        goto LABEL_LOOP_END;
                                    }
                                }
                            }
                        LABEL_LOOP_END:;
                        }
                    } else if (dt->ui.context_view.mode == CONTEXT_VIEW_MODE_RULES) {
                        const float32 padding = viewport.width * 0.008f;
                        
                        const vec2 tl_inner = tl + padding;
                        const vec2 br_inner = br - padding;
                        
                        float64 w = (br_inner.x - tl_inner.x);
                        float64 h = (br_inner.y - tl_inner.y);
                        //const float64 area_avail = w * h;
                        auto& ctx_view = ui.context_view;
                        //auto& to_display = ctx_view.things_to_display;
                        //usize count = to_display.size();
                        
                        auto* rtime = mtt::Runtime::ctx();
                        usize count = rtime->script_tasks.rules_list.size();
                        
                        
                        float64 side = 1;
                        {
                            double x=w, y=h, n=count;//values here
                            double px=ceil(sqrt(n*x/y));
                            double sx,sy;
                            if(floor(px*y/x)*px<n)    //does not fit, y/(x/px)=px*y/x
                                sx=y/ceil(px*y/x);
                            else
                                sx= x/px;
                            double py=ceil(sqrt(n*y/x));
                            if(floor(py*x/y)*py<n)    //does not fit
                                sy=x/ceil(x*py/y);
                            else
                                sy=y/py;
                            side = m::max(sx, sy);
                        }
                        
                        ui.context_view.rule_els.clear();
                        ui.context_view.action_els.clear();
                        ui.context_view.rule_els.reserve(count);
                        for (usize t = 0; t < count; t += 1) {
                            ui.context_view.rule_els.push_back({rtime->script_tasks.rules_list[t]->id, (mtt::Box){}});
                        }
                        std::sort(ui.context_view.rule_els.begin(), ui.context_view.rule_els.end(), [](Context_View::Rule_Element_Record& first, Context_View::Rule_Element_Record& second) {
                            return first.ID < second.ID;
                        });
                        
                        
                        
                        DrawTalk_World* dt_world = DrawTalk_World_ctx();
                        auto* core = mtt_core_ctx();
                        
                        Input* const input = &core->input;
                        Input_Record* u_input = &input->users[0];
                        
                        enum OP {
                            OP_TOGGLE,
                            OP_DELETE,
                        } op;
                        switch (pointer_op_current(u_input)) {
                            default: {
                                MTT_FALLTHROUGH;
                            }
                            case UI_POINTER_OP_DRAW: {
                                //pointer_op_set_current(u_input, UI_POINTER_OP_ERASE);
                                op = OP_TOGGLE;
                                break;
                            }
                            case UI_POINTER_OP_ERASE: {
                                //pointer_op_set_current(u_input, UI_POINTER_OP_DRAW);
                                //mtt::Thing_destroy(thing_src);
                                op = OP_DELETE;
                                break;
                            }
                        }
                        
                        const float32 depth = ui.context_view.depth + 1.0f;
                        const float64 half_side = side * 0.5;
                        {
                            const vec4 inner_color = vec4(vec3(bg_color_default()), 1.0f);
                            const vec4 outer_color = vec4(vec3(pen_color_default()) + vec3(0.2f), 1.0f);
                            
                            usize i = 0;
                            const float32 padding_thickness = 4;
                            const vec2 padding_inner = vec2(padding_thickness);
                            const vec2 padding_dim_inner = 2*vec2(padding_thickness);
                            const vec2 padding_outer = 2*padding_inner;
                            const vec2 padding_dim_outer = 2*padding_dim_inner;
                            
                            auto* vg = nvgGetGlobalContext();
                            nvgSave(vg);
                            nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
                            nvgFontSize(vg, 16);
                            mat4 identity = m::mat4_identity();
                            nvgSetViewTransform(vg, value_ptr(identity));
                            nvgSetModelTransform(vg, value_ptr(identity));
                            nvgFillColor(vg, nvgRGBAf(outer_color.r, outer_color.g, outer_color.b, outer_color.a));
                            
                            
                            
                            for (float64 y = tl_inner.y; i < count; y += side) {
                                for (float64 x = tl_inner.x; x + side <= br_inner.x; x += side) {
                                    if (ui.context_view.rule_els.size() <= i) {
                                        goto LABEL_LOOP_RULES_END;
                                    }
                                    
                                    //auto* box = &ui.context_view.rule_els[i].box;
                                    
                                    vec3 pos_center = vec3(vec2(x, y) + ((float32)half_side), depth);
                                    vec2 pos_center_2d = vec2(pos_center);
                                    
                                    mtt::Box box_ = {
                                        .tl = pos_center_2d - (float32)half_side,
                                        .br = pos_center_2d + (float32)half_side
                                    };
                                    
                                    const mtt::Box* const box = &box_;
                                    
                                    mtt::Script_ID s_id = ui.context_view.rule_els[i].ID;
                                    mtt::Script_Instance* s_inst = (mtt::Script_Instance*)(rtime->id_to_rule_script.find(s_id)->second);
                                    
                                    //sd::rectangle_rounded(world->renderer, vec2(x, y), dimensions, 0.0f, 0.2f, 4);

                                    
                                    sd::set_color_rgba_v4(world->renderer, outer_color);
                                    sd::rectangle_rounded(world->renderer, box->tl - padding_outer, (box->br - box->tl) + padding_dim_outer, 0, 0.2f, 4);
                                    
                                    sd::set_color_rgba_v4(world->renderer, inner_color);
                                    sd::rectangle_rounded(world->renderer, box->tl - padding_inner, (box->br - box->tl) + padding_dim_inner, 0.0001f, 0.2f, 4);
                                    
                                    mtt::String& cpp_label = rtime->rule_label_map.find(s_id)->second;
                                    cstring label = cpp_label.c_str();
                                    nvgTextBox(vg, box->tl.x, pos_center_2d.y, side, label, NULL);
                                    
                                    bool rules_active = s_inst->rules_are_active;
                                    if (selected_this_frame) {
                                        auto* op_state = &context_view_thing->input_handlers.state[mtt::INPUT_MODALITY_FLAG_PEN];
                                        vec2 pen_touched_pos = op_state->prev_canvas_pos;
                                        
                                        float box_check[4];
                                        box_check[0] = box->tl.x;
                                        box_check[1] = box->tl.y;
                                        box_check[2] = box->br.x;
                                        box_check[3] = box->br.y;
                                        if (mtt::Box_point_check(box_check, pen_touched_pos)) {
                                            switch (op) {
                                                case OP_TOGGLE: {
                                                    s_inst->rules_are_active = !s_inst->rules_are_active;
                                                    rules_active = s_inst->rules_are_active;
                                                    break;
                                                }
                                                case OP_DELETE: {
                                                    mtt::Script_Instance_should_terminate(s_inst);
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    if (!rules_active) {
                                        sd::set_color_rgba_v4(world->renderer, vec4(vec3(outer_color), 0.7f));
                                        sd::quad(world->renderer, box->tl, box->tl + vec2(0, 8), box->br, box->br + vec2(0, -8), 999.99999f);
                                        vec2 tr = vec2(box->br.x, box->tl.y);
                                        vec2 bl = vec2(box->tl.x, box->br.y);
                                        sd::quad(world->renderer, tr, tr + vec2(0, 8), bl, bl + vec2(0, -8), 999.99999f);
                                    }
                                    
                                    i += 1;
                                    if (i >= count) {
                                        goto LABEL_LOOP_RULES_END;
                                    }
                                }
                            }
                            nvgRestore(vg);
                        LABEL_LOOP_RULES_END:;
                        }
                    } else if (dt->ui.context_view.mode == CONTEXT_VIEW_MODE_ACTIONS) {
                        const float32 padding = viewport.width * 0.008f;
                        
                        const vec2 tl_inner = tl + padding;
                        const vec2 br_inner = br - padding;
                        
                        float64 w = (br_inner.x - tl_inner.x);
                        float64 h = (br_inner.y - tl_inner.y);

                        if (dt->recorder.selection_recording.selections.empty()) {
                            // TODO: Show all ongoing actions?
                            goto LABEL_ACTION_DISPLAY_END;
                        } else {
                            
                            ui.context_view.rule_els.clear();
                            ui.context_view.action_els.clear();
                            ui.context_view.action_els_things.clear();
                            
                            for (auto it = dt->recorder.selection_recording.selections.begin(); it != dt->recorder.selection_recording.selections.end(); ++it) {
                                if (it->is_duplicate) {
                                    continue;
                                }
                                
                                mtt::Thing* thing = nullptr;
                                if (!world->Thing_try_get(it->ID, &thing)) {
                                    break;
                                }
                                
                                if (mtt::Thing_is_proxy(thing)) {
                                    thing = mtt::Thing_mapped_from_proxy(thing);
                                }
                                
                                ui.context_view.action_els_things.push_back(thing);
//                                ui.context_view.action_els.push_back({.thing_ID = mtt::thing_id(thing), 0, (mtt::Box){}});
                            }
                            if (ui.context_view.action_els_things.empty()) {
                                goto LABEL_ACTION_DISPLAY_END;
                            }
                            // FIXME: would normally store mapping between things and scripts for quick lookup
                            auto& script_tasks = mtt::Runtime::ctx()->script_tasks.list;
                            if (script_tasks.empty()) {
                                goto LABEL_ACTION_DISPLAY_END;
                            }
                            
                            dt::Dynamic_Array<mtt::Active_Action> local_action_list = {};
                            for (auto& el : script_tasks) {
                                if (el->label == "") {
                                    continue;
                                }
                                
                                local_action_list = el->active_action_list;
                                std::sort(local_action_list.begin(), local_action_list.end(), [](mtt::Active_Action& first, mtt::Active_Action& second) {
                                    if (first.src_thing == second.src_thing) {
                                        return (first.dst_thing > second.dst_thing);
                                    }
                                    
                                    return (first.src_thing > second.src_thing);
                                });
                                
                                for (const auto& action : local_action_list) {
                                    mtt::Thing_ID src_thing = action.src_thing;
                                    mtt::Thing_ID dst_thing = action.dst_thing;
                                    
                                    bool found = false;
                                    for (usize t = 0; t < ui.context_view.action_els_things.size(); t += 1) {
                                        mtt::Thing* thing = ui.context_view.action_els_things[t];
                                        if (mtt::thing_id(thing) == src_thing) {
                                            ui.context_view.action_els.push_back({mtt::thing_id(thing), thing, el->id, (mtt::Script_Instance*)el, (mtt::Box){}, action});
                                            found = true;
                                            break;
                                        }
                                    }
                                    if (found) {
                                        break;
                                    }
                                }

                            }
                            
                            if (ui.context_view.action_els.empty()) {
                                goto LABEL_ACTION_DISPLAY_END;
                            }
                            
                            std::sort(ui.context_view.action_els.begin(), ui.context_view.action_els.end(), [](Context_View::Action_Element_Record& first, Context_View::Action_Element_Record& second) {
                                return first.thing_ID < second.thing_ID;
                            });
                            
                            usize count = ui.context_view.action_els.size();
                            float64 side = 1;
                            {
                                double x=w, y=h, n=count;//values here
                                double px=ceil(sqrt(n*x/y));
                                double sx,sy;
                                if(floor(px*y/x)*px<n)    //does not fit, y/(x/px)=px*y/x
                                    sx=y/ceil(px*y/x);
                                else
                                    sx= x/px;
                                double py=ceil(sqrt(n*y/x));
                                if(floor(py*x/y)*py<n)    //does not fit
                                    sy=x/ceil(x*py/y);
                                else
                                    sy=y/py;
                                side = m::max(sx, sy);
                            }
                            
                            DrawTalk_World* dt_world = DrawTalk_World_ctx();
                            auto* core = mtt_core_ctx();
                            
                            Input* const input = &core->input;
                            Input_Record* u_input = &input->users[0];
                            
                            enum OP {
                                OP_TELEPORT,
                                OP_DELETE,
                            } op;
                            switch (pointer_op_current(u_input)) {
                                default: {
                                    MTT_FALLTHROUGH;
                                }
                                case UI_POINTER_OP_DRAW: {
                                    op = OP_TELEPORT;
                                    break;
                                }
                                case UI_POINTER_OP_ERASE: {
                                    op = OP_DELETE;
                                    break;
                                }
                            }
                            
                            
                            
                            const float32 depth = ui.context_view.depth + 1.0f;
                            const float64 half_side = side * 0.5;
                            {
                                const vec4 inner_color = vec4(vec3(bg_color_default()), 1.0f);
                                const vec4 outer_color = vec4(vec3(pen_color_default()) + vec3(0.2f), 1.0f);
                                
                                usize i = 0;
                                const float32 padding_thickness = 4;
                                const vec2 padding_inner = vec2(padding_thickness);
                                const vec2 padding_dim_inner = 2*vec2(padding_thickness);
                                const vec2 padding_outer = 2*padding_inner;
                                const vec2 padding_dim_outer = 2*padding_dim_inner;
                                
                                auto* vg = nvgGetGlobalContext();
                                nvgSave(vg);
                                nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
                                nvgFontSize(vg, 16);
                                mat4 identity = m::mat4_identity();
                                nvgSetViewTransform(vg, value_ptr(identity));
                                nvgSetModelTransform(vg, value_ptr(identity));
                                nvgFillColor(vg, nvgRGBAf(outer_color.r, outer_color.g, outer_color.b, outer_color.a));
                                
                                auto& named_things = dt->lang_ctx.dictionary.thing_to_word;
                                
                                for (float64 y = tl_inner.y; i < count; y += side) {
                                    for (float64 x = tl_inner.x; x + side <= br_inner.x; x += side) {
                                        if (ui.context_view.action_els.size() <= i) {
                                            goto LABEL_LOOP_ACTIONS_END;
                                        }
                                        
                                        vec3 pos_center = vec3(vec2(x, y) + ((float32)half_side), depth);
                                        vec2 pos_center_2d = vec2(pos_center);
                                        
                                        mtt::Box box_ = {
                                            .tl = pos_center_2d - (float32)half_side,
                                            .br = pos_center_2d + (float32)half_side
                                        };
                                        
                                        const mtt::Box* const box = &box_;
                                        
                                        auto& action_el = ui.context_view.action_els[i];
                                        
                                        mtt::Script_Instance* s_inst = action_el.script_ref;
                                        
                                        sd::set_color_rgba_v4(world->renderer, outer_color);
                                        sd::rectangle_rounded(world->renderer, box->tl - padding_outer, (box->br - box->tl) + padding_dim_outer, 0, 0.2f, 4);
                                        
                                        sd::set_color_rgba_v4(world->renderer, inner_color);
                                        sd::rectangle_rounded(world->renderer, box->tl - padding_inner, (box->br - box->tl) + padding_dim_inner, 0.0001f, 0.2f, 4);
                                        
                                        mtt::String src_label = {};
                                        mtt::String dst_label = {};
                                        
                                        if (action_el.action.src_thing != mtt::Thing_ID_INVALID) {
                                            auto words_it = named_things.find(action_el.action.src_thing);
                                            if (words_it != named_things.end()) {
                                                auto& words = words_it->second;
                                                if (words.size() == 1) {
                                                    src_label = std::to_string(action_el.action.src_thing) + " → ";
                                                } else {
                                                    for (auto w = words.begin(); w != words.end(); ++w) {
                                                        auto& name = (*w)->name;
                                                        if (name == "thing") {
                                                            continue;
                                                        }
                                                        
                                                        src_label += name + " ";
                                                    }
                                                    src_label +=  "→ ";
                                                }
                                                
                                            } else {
                                                src_label = std::to_string(action_el.action.src_thing) + " → ";
                                            }
                                            
                                        }
                                        
                                        if (action_el.action.dst_thing != mtt::Thing_ID_INVALID) {
                                            auto words_it = named_things.find(action_el.action.dst_thing);
                                            if (words_it != named_things.end()) {
                                                auto& words = words_it->second;
                                                if (words.size() == 1) {
                                                    src_label = std::to_string(action_el.action.src_thing);
                                                } else {
                                                    for (auto w = words.begin(); w != words.end(); ++w) {
                                                        auto& name = (*w)->name;
                                                        if (name == "thing") {
                                                            continue;
                                                        }
                                                        
                                                        dst_label += name + " ";
                                                    }
                                                }
                                            } else {
                                                dst_label = std::to_string(action_el.action.dst_thing);
                                            }
                                            if (!dst_label.empty()) {
                                                dst_label =  " → " + dst_label;
                                            }
                                        }
                                        
                                        mtt::String cpp_label = src_label + action_el.action.action->name + dst_label;
                                        cstring label = cpp_label.c_str();
                                        nvgTextBox(vg, box->tl.x, pos_center_2d.y, side, label, NULL);
                                        
                                        if (selected_this_frame) {
                                            auto* op_state = &context_view_thing->input_handlers.state[mtt::INPUT_MODALITY_FLAG_PEN];
                                            vec2 pen_touched_pos = op_state->prev_canvas_pos;
                                            
                                            float box_check[4];
                                            box_check[0] = box->tl.x;
                                            box_check[1] = box->tl.y;
                                            box_check[2] = box->br.x;
                                            box_check[3] = box->br.y;
                                            if (mtt::Box_point_check(box_check, pen_touched_pos)) {
                                                switch (op) {
                                                    case OP_DELETE: {
                                                        mtt::Script_Instance_should_terminate(s_inst);
                                                        break;
                                                    }
                                                }
                                            }
                                            op_state = &context_view_thing->input_handlers.state[mtt::INPUT_MODALITY_FLAG_TOUCH];
                                            vec2 direct_touched_pos = op_state->prev_canvas_pos;
                                            if (mtt::Box_point_check(box_check, direct_touched_pos)) {
                                                switch (op) {
                                                    case OP_TELEPORT: {
                                                        mtt::Thing* thing_src = action_el.thing;
                                                        
                                                        auto* rep = mtt::rep(thing_src);
                                                        
                                                        DrawTalk_World* dt_world = DrawTalk_World_ctx();
                                                        auto* core = mtt_core_ctx();
                                                        
                                                        auto* cam = &dt_world->cam;
                                                        
                                                        vec3 d_scale;
                                                        quat d_orientation;
                                                        vec3 d_translation;
                                                        vec3 d_skew;
                                                        vec4 d_perspective;
                                                        {
                                                            m::decompose(rep->hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
                                                        }
                                                        
                                                        
                                                        auto vp_offset = vec3(core->viewport.width * 0.5f, core->viewport.height * 0.5f, 0.0f);
                                                        vec3 translation = d_translation - vp_offset;
                                                        
                                                        
                                                        float32 scale = 1.0;
                                                        {
                                                            auto* src_thing_rep = mtt::rep(thing_src);
                                                            auto* src_thing_aabb = &src_thing_rep->colliders[0]->aabb;
                                                        
                                                            vec2 src_box_max_min;
                                                            m::max_min_dimensions(vec2(src_thing_aabb->half_extent * 2), &src_box_max_min);
                                                         
                                                            
                                                            auto side = m::min(core->viewport.width, core->viewport.height);
                                                            if (src_box_max_min.x < side || src_box_max_min.y < side
                                                                || src_box_max_min.x > side || src_box_max_min.y > side) {
                                                                scale = (side / src_box_max_min.x) * 0.5f;
                                                            }
                                                        }
                                                        
                                                        cam->cam_transform =  m::translate(Mat4(1.0f), translation);
                                                        
                                                        core->view_position -= d_translation + vp_offset;
                                                        mtt::calc_view_matrix(cam);
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        
                                        i += 1;
                                        if (i >= count) {
                                            goto LABEL_LOOP_ACTIONS_END;
                                        }
                                        
                                        
                                    }
                                }
                                nvgRestore(vg);
                            LABEL_LOOP_ACTIONS_END:;
                            }
                            
                        }
                    LABEL_ACTION_DISPLAY_END:;
                    }
                }
                
                sd::end_polygon_no_new_drawable(renderer);
                sd::end_drawable(renderer);
            }
        }
        
        sd::restore(renderer);
    }
    
    
 //   nvgRestore(vg);
}


void Text_Panel::clear()
{
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    
    auto* vg = nvgGetGlobalContext();
    
    auto& selection_map = dt->selection_map;
    
    UI& ui = dt->ui;
    
    this->must_update = true;
    this->row_count = 0;
    this->invisible_row_count = 0;
    this->row_offset = 0;
    this->text.clear();
    this->generated_message.clear();
    
}

void Text_Panel::reset()
{
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    
    auto* vg = nvgGetGlobalContext();
    
    auto& selection_map = dt->selection_map;
    
    UI& ui = dt->ui;
    
    this->must_update = true;
    this->row_count = 0;
    this->invisible_row_count = 0;
    this->row_offset = 0;
    this->generated_message.clear();

    
    
}


extern MTT_String_Ref dt_Instruction_label;


    
float32 build_feedback_ui_from_instructions_OLD(dt::DrawTalk* dt, Instruction* ins, DT_Evaluation_Context& eval);

float32 build_feedback_ui_from_instructions_OLD(dt::DrawTalk* dt, Instruction* ins, DT_Evaluation_Context& eval)
{
    
    mtt::Rep* rep = nullptr;
    //mtt::Thing* el = mtt::Thing_make_with_aabb_corners
    
    //static vec4 instruction_bg_color = vec4(50.0f/255.0f,205.0f/255.0f,50.0f/255.0f,255.0f/2.0f);
    //static vec4 instruction_bg_color_selected = vec4(vec3(instruction_bg_color / 2.0f), 255.0f / 4.0f);
    {
        auto* vg = nvgGetGlobalContext();
        
        nvgSave(vg);
        
        mtt::Box& box = ins->bounds;
        
        
        
        
        ins->color = nvgRGBA(100,100,100,255/2);
        ins->color_selected = nvgRGBA(50,50, 50, 255/2);
        ins->color_current = ins->color;
        
        
        
        
        
        //        if (tok->prop_ref == nullptr) {
        //            color = color::WHITE;
        //        } else if (tok->prop_ref->type_str == "pronoun"){
        //            color = color::BLUE;
        //        } else if ((tok->prop_ref->kind_str == "THING_INSTANCE" ||
        //            tok->prop_ref->kind_str == "THING_TYPE")) {
        //            color = color::SYSTEM_BLUE;
        //        } else if (tok->prop_ref->kind_str == "ACTION") {
        //            if (tok->prop_ref->type_str == "TRIGGER") {
        //                color = color::GREEN;
        //            } else if (tok->prop_ref->type_str == "RESPONSE") {
        //                color = color::RED;
        //            } else {
        //                color = color::ORANGE;
        //            }
        //        } else if (tok->prop_ref->type_str == "ACTION") {
        //            color = color::ORANGE;
        //        } else {
        //            color = color::WHITE;
        //        }
        
        vec4 text_color = vec4(1.0f);
        float bounds[4];
        float text_advance = 0.0;
        
        {
            
            if (ins->type == "ACTION") {
                text_color = dt::LABEL_COLOR_ACTION;
                
                mtt::String cpp_label = "";
                
                for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
                    cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
                }
                
                if (ins->annotation == "unknown") {
                    cpp_label += " (?)";
                    eval.instructions_to_disambiguate.push_back({ins, AMBIGUITY_TYPE_ACTION_UNKNOWN});
                }
                
                cstring label = cpp_label.c_str();
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
                
                ins->display_label = cpp_label;
                
            } else if (ins->type == "TRIGGER") {
                text_color = dt::LABEL_COLOR_TRIGGER;
                
                mtt::String cpp_label = "";
                
                for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
                    cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
                }
                
                cstring label = cpp_label.c_str();
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
                
                ins->display_label = cpp_label;
                
            } else if (ins->type == "RESPONSE") {
                text_color = dt::LABEL_COLOR_RESPONSE;
                
                mtt::String cpp_label = "";
                
                for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
                    cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
                }
                
                
                cstring label = cpp_label.c_str();
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
                
                ins->display_label = cpp_label;
            } else if (ins->type == "TRIGGER_RESPONSE") {
                //text_color = dt::LABEL_COLOR_TRIGGER;
                text_color = pen_color_default();
                bool stops_instead_of_causes = false;
                bool is_relational_trigger = false;
                for (usize ch_list_idx = 0; ch_list_idx < ins->child_list_count(); ch_list_idx += 1) {
                    auto& child_list = ins->child_list(ch_list_idx);
                    for (auto it = child_list.begin(); it != child_list.end(); ++it) {
                        if ((*it)->annotation == "END_CONDITION") {
                            cstring label = "UNTIL";
                            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + 5, bounds);
                            ins->display_label = label;
                            stops_instead_of_causes = true;
                            break;
                        } else if ((*it)->annotation == "RELATION" || (*it)->annotation == "CONTINUOUS") {
                            cstring label = "RELATES TO";
                            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + 10, bounds);
                            ins->display_label = label;
                            is_relational_trigger = true;
                        }
                    }
                }
                
                if (!stops_instead_of_causes && !is_relational_trigger) {
                    cstring label = LABEL_CAUSES;
                    text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + LABEL_CAUSES_LEN, bounds);
                    ins->display_label = label;
                }
                
                
            } else if (ins->kind == "THING_INSTANCE_SELECTION") {
                text_color = dt::LABEL_COLOR_THING_INSTANCE;
                
                mtt::String cpp_label = "";
                if (false && ins->type == "AGENT") {
                    cpp_label += "SOURCE" + ((!ins->prop.empty()) ? " : " + ins->prop.front()->label : "");
                } else {
                    cpp_label += //ins->type +
                    ((!ins->prop.empty()) ? ins->prop.front()->label : "");
                }
                
                if (!ins->prop.empty()) {
                    auto* num_prop = ins->prop.front();
                    if (dt::numeric_value_words.find(num_prop->label) != dt::numeric_value_words.end()) {
                        cpp_label = "# " + cpp_label;
                        auto* val = num_prop->try_get_only_prop("COUNT");
                        if (val != nullptr) {
                            if (val->value.kind_string == "NUMERIC") {
                                cpp_label += " " + mtt::to_string_with_precision(val->value.numeric, 3);
                            } else if (val->value.kind_string == "VECTOR") {
                                cpp_label += " <" + mtt::to_string_with_precision(val->value.vector.x, 3) + ", " + mtt::to_string_with_precision(val->value.vector.y, 3) + ">";
                            }
                        }
                    }
                }
                
                cstring label = cpp_label.c_str();
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
                
                ins->display_label = cpp_label;
                
            } else if (ins->kind == "THING_TYPE_SELECTION") {
                
                text_color = dt::LABEL_COLOR_THING_TYPE;
                
                //mtt::String cpp_label = (((ins->type == "AGENT") ? "SOURCE" : ins->type) + " : type ");
                mtt::String cpp_label = dt::THING_TYPE_SELECTION_INSTRUCTION_LABEL_PREFIX;
                
                if ((ins->prop.empty())) {
                    cpp_label += "?";
                } else {
                    
                    auto* pr = ins->prop.front();
                    
                    dt::Speech_Property::Prop_List* list;
                    if (pr->try_get_prop("COREFERENCE", &list)) {
                        uintptr ref = list->front()->value.reference;
                        if (ref != 0) {
                            Speech_Property* ref_prop = (Speech_Property*)ref;
                            cpp_label += ref_prop->label;
                        } else {
                            cpp_label += "?";
                        }
                    } else {
                        cpp_label += ins->prop.front()->label;
                    }
                    
                }
                
                //if (ins->annotation == "unk)
                
                cstring label = cpp_label.c_str();
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
                
                ins->display_label = cpp_label;
                
            } else if (ins->type == "PREPOSITION") {
                text_color = dt::LABEL_COLOR_PREPOSITION;
                
                cstring label = ins->kind.c_str();
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
                
                ins->display_label = ins->kind;
            } else if (ins->type == "PROPERTY") {
                text_color = dt::LABEL_COLOR_PROPERTY;
                
                cstring label = ins->kind.c_str();
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
                
                ins->display_label = ins->kind;
            } else {
                text_color = color::BLACK;
                
                cstring label = ins->kind.c_str();
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
                
                ins->display_label = ins->kind;
            }
            
        }
        
        ins->text_color = text_color;
        
        // MARK: instruction ui layout

        float prev_x = eval.layout.pos.x;
        float prev_y = eval.layout.pos.y;
        //eval.layout.pos.x += text_advance + eval.layout.pad.x;
        
        
        float next_y = eval.layout.pos.y += (eval.layout.height) + eval.layout.pad.y;
        // prev_x
        
        float max_x = eval.layout.pos.x + text_advance + (eval.layout.pad.x / 2.0f);
        float min_x = max_x;
        float max_x_link_from = max_x;
        {
        
            if (ins->child_count() != 0) {
                for (usize ch_list_idx = 0; ch_list_idx < ins->child_list_count(); ch_list_idx += 1) {
                    auto& child_list = ins->child_list(ch_list_idx);
                    for (usize ch_idx = 0; ch_idx < child_list.size(); ch_idx += 1) {
                        Instruction* ch_ins = child_list[ch_idx];
                        //                    switch (ch_ins->link_type) {
                        //                        case Instruction::PARENT_LINK_TYPE::NONE: {
                        //
                        //                            break;
                        //                        }
                        //                        case Instruction::PARENT_LINK_TYPE::FROM: {
                        //
                        //                            break;
                        //                        }
                        //                        case Instruction::PARENT_LINK_TYPE::TO: {
                        //                            break;
                        //                        }
                        //                        case Instruction::PARENT_LINK_TYPE::BIDIRECTIONAL: {
                        //
                        //                            break;
                        //                        }
                        //                    }
                        eval.layout.pos.y = next_y;
                        float ret_x = build_feedback_ui_from_instructions_OLD(dt, ch_ins, eval);
                        if ((ret_x > max_x)) {
                            max_x = ret_x;
                        }
                        eval.layout.pos.x = max_x + eval.layout.pad.x * 2.0f;
                    }
                }
            } else {
                max_x += layout_padding;
            }
            
            eval.layout.pos.x = prev_x;
            eval.layout.pos.y = prev_y;
            
            nvgRestore(vg);
            
            
            
        }
        
        {
            
            box.tl = vec2(bounds[0], bounds[1]);
            box.br = vec2(eval.layout.pos.x + text_advance, bounds[1] + eval.layout.height);
            
            float32 x_left = box.tl.x;
            float32 x_right = max_x;
            float32 x_quarter = (x_right - x_left) / 4.0f;
            float32 x_width = box.br.x - box.tl.x;
            
            float32 new_x = m::max(max_x, box.br.x);
            //box.br.x = new_x;
            //box.tl.x = new_x - x_width;
//            box.tl.x = x_quarter;
//            box.br.x = x_quarter + x_width;
//            float32 x_width = box.br.x - box.tl.x;
//            float32 OFF = ((max_x - min_x) / 2.0f);
//            box.tl.x = OFF - (x_width / 2.0f);
//            box.br.x = OFF + (x_width / 2.0f);
            
            
//            box.tl.x += x_offset;
//            box.br.x += x_offset;
            
            
            
            auto box_centered = box;
            
            float32 width = box.br.x - box.tl.x;
            float32 h_width = width / 2.0f;
            float32 height = box.br.y - box.tl.y;
            float32 h_height = height / 2.0f;
            vec2 center = (box.br + box.tl) / 2.0f;
            box_centered.tl = vec2(-h_width, -h_height);
            box_centered.br = vec2(h_width, h_height);
            
            mtt::Thing* el = mtt::Thing_make_with_aabb_corners(dt->mtt, FEEDBACK_ELEMENT_THING_TYPE, vec2(1.0f), &rep, box_centered, 999.0f, true);
            el->lock_to_canvas = false;
            mtt::Thing_set_position(el, vec3(center, 999.0f));
            el->forward_input_to_root = true;
            el->is_user_drawable = false;
            *mtt::access_pointer_to_pointer<Instruction*>(el, "ctx_ptr") = ins;
            ins->thing_id = el->id;
            
            MTT_String_Ref* str = mtt::access<MTT_String_Ref>(el, "typename");
            *str = dt_Instruction_label;
            MTT_string_ref_retain(dt_Instruction_label);
            
            {
                
                el->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
                    
                    if (cancel_input_default(event)) {
                        return false;
                    }
                    
                    Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                    
                    auto& eval = dt::DrawTalk::ctx()->lang_ctx.eval_ctx;
                    eval.instruction_selected_with_touch() = ins;
                    ins->color_current = ins->color_selected;
                    
                    ins->touch_state = UI_TOUCH_STATE_BEGAN;
                    
                    
                    return true;
                };
                el->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                    
                    if (cancel_input_default(event)) {
                        return;
                    }
                    
                    Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                    
                    auto& eval = dt::DrawTalk::ctx()->lang_ctx.eval_ctx;
                    
                    ins->touch_state = UI_TOUCH_STATE_MOVED;
                };
                el->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                    
                    if (cancel_input_default(event)) {
                        return;
                    }
                    
                    Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                    
                    auto& eval = dt::DrawTalk::ctx()->lang_ctx.eval_ctx;
                    eval.instruction_selected_with_touch() = nullptr;
                    ins->color_current = ins->color;
                    
                    ins->touch_state = UI_TOUCH_STATE_ENDED;
                    
                    
                    
                };
                
                
                
                el->input_handlers.on_pen_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
                    
                    Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                    
                    auto* dt_ctx = dt::DrawTalk::ctx();
                    auto& eval = dt_ctx->lang_ctx.eval_ctx;
                    eval.instruction_selected_with_pen() = ins;
                    ins->color_current = ins->color_selected;
                    
                    ins->pen_state = UI_TOUCH_STATE_BEGAN;
                    
                    
                    for (auto it = dt_ctx->scn_ctx.selected_things.begin(); it != dt_ctx->scn_ctx.selected_things.end(); ++it) {
                        mtt::Thing* other_thing = dt_ctx->mtt->Thing_try_get(*it);
                        if (other_thing == nullptr ) {
                            continue;
                        }
                        
                        
                        auto find_it = ins->things_selection_modified_this_input.find(other_thing->id);
                        if (find_it == ins->things_selection_modified_this_input.end()) {
                            if (dt::can_label_thing(other_thing) && (ins->kind == "THING_TYPE_SELECTION" || ins->kind == "THING_INSTANCE_SELECTION")) {
                                if (ins->kind == "THING_TYPE_SELECTION") {
                                    ins->kind = "THING_INSTANCE_SELECTION";
                                    for (auto p = ins->prop.begin(); p != ins->prop.end(); ++p) {
                                        if ((*p)->kind_str == "THING_TYPE_SELECTION") {
                                            (*p)->kind_str = "THING_INSTANCE_SELECTION";
                                        }
                                    }
                                }
                                
                                if (ins->kind == "THING_INSTANCE_SELECTION") {
                                    ins->things_selection_modified_this_input.insert(other_thing->id);
                                    bool should_remove = false;
                                    for (auto things_it = ins->thing_id_list.begin(); things_it != ins->thing_id_list.end();) {
                                        if ((*things_it) == other_thing->id) {
                                            mtt::Thing_destroy_proxies(mtt::world(thing), *things_it);
                                            
                                            things_it = ins->thing_id_list.erase(things_it);
                                            
                                            should_remove = true;
                                            break;
                                        } else {
                                            ++things_it;
                                        }
                                    }
                                    
                                    if (!should_remove) {
                                        ASSERT_MSG(std::find(ins->thing_id_list.begin(), ins->thing_id_list.end(), other_thing->id) == ins->thing_id_list.end(), "duplicate");
                                        ins->thing_id_list.push_back(other_thing->id);
                                    }
                                }
                                
                            } else if (other_thing->archetype_id == mtt::ARCHETYPE_UI_ELEMENT) {
                                MTT_String_Ref* type_name = mtt::access<MTT_String_Ref>(other_thing, "typename");
                                if (type_name == nullptr || !MTT_string_ref_is_equal(dt_Instruction_label, *type_name)) {
                                    continue;
                                }
                                //ins->things_selection_modified_this_input.insert(other_thing->id);
                                
                                
                                dt::Instruction* child = *mtt::access_pointer_to_pointer<dt::Instruction*>(other_thing, "ctx_ptr");
                                dt::Instruction* parent = ins;
                                
                                
                                if (child->parent == ins) {
                                    if (child->link_disabled) {
                                        child->link_disabled = false;
                                    } else {
                                        child->link_disabled = true;
                                    }
                                    for (auto p_it = child->prop.begin(); p_it != child->prop.end(); ++p_it) {
                                        (*p_it)->parent_ref_disabled = child->link_disabled;
                                    }
                                } else if (ins->parent == child) {
                                    child = ins;
                                    if (child->link_disabled) {
                                        child->link_disabled = false;
                                    } else {
                                        child->link_disabled = true;
                                    }
                                    for (auto p_it = child->prop.begin(); p_it != child->prop.end(); ++p_it) {
                                        (*p_it)->parent_ref_disabled = child->link_disabled;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (ins->kind == "THING_INSTANCE_SELECTION") {
                        
                        if (ins->thing_id_list.empty()) {
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                
                                prop->value.kind_string = "THING_INSTANCE";
                                prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                            }
                        } else if (ins->thing_id_list.size() == 1) {
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                
                                prop->value.kind_string = "THING_INSTANCE";
                                prop->value.thing = ins->thing_id_list.front();
                                
                                Speech_Property* plural;
                                if (prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                                
                            }
                        } else {
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                
                                prop->value.kind_string = "LIST";
                                prop->value.list.resize(ins->thing_id_list.size());
                                prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = true;
                                }
                                
                                for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                                    prop->value.list[val_i] = Speech_Property::Value();
                                    prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                    prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                                }
                                
                                
                            }
                        }
                    } else if (ins->things_selection_modified_this_input.empty() && dt_ctx->scn_ctx.selected_things.empty()) {
                        ins->link_disabled = !ins->link_disabled;
                        for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                            auto* p = (*p_it);
                            p->parent_ref_disabled = ins->link_disabled;
                            
                        }
                    }
                    
                    return true;
                };
                el->input_handlers.on_pen_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                    
                    Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                    
                    auto* dt_ctx = dt::DrawTalk::ctx();
                    auto& eval = dt::DrawTalk::ctx()->lang_ctx.eval_ctx;
                    
                    ins->pen_state = UI_TOUCH_STATE_MOVED;
                    
                    
                    for (auto it = dt_ctx->scn_ctx.selected_things.begin(); it != dt_ctx->scn_ctx.selected_things.end(); ++it) {
                        mtt::Thing* other_thing = dt_ctx->mtt->Thing_try_get(*it);
                        if (other_thing == nullptr || !dt::can_label_thing(other_thing)) {
                            continue;
                        }
                        
                        auto find_it = ins->things_selection_modified_this_input.find(other_thing->id);
                        if (find_it == ins->things_selection_modified_this_input.end()) {
                            if (dt::can_label_thing(other_thing) && (ins->kind == "THING_TYPE_SELECTION" || ins->kind == "THING_INSTANCE_SELECTION")) {
                                if (ins->kind == "THING_TYPE_SELECTION") {
                                    ins->kind = "THING_INSTANCE_SELECTION";
                                    for (auto p = ins->prop.begin(); p != ins->prop.end(); ++p) {
                                        if ((*p)->kind_str == "THING_TYPE_SELECTION") {
                                            (*p)->kind_str = "THING_INSTANCE_SELECTION";
                                        }
                                    }
                                }
                                
                                if (ins->kind == "THING_INSTANCE_SELECTION") {
                                    ins->things_selection_modified_this_input.insert(other_thing->id);
                                    bool should_remove = false;
                                    for (auto things_it = ins->thing_id_list.begin(); things_it != ins->thing_id_list.end();) {
                                        if ((*things_it) == other_thing->id) {
                                            mtt::Thing_destroy_proxies(mtt::world(thing), *things_it);
                                            things_it = ins->thing_id_list.erase(things_it);
                                            should_remove = true;
                                            break;
                                        } else {
                                            ++things_it;
                                        }
                                    }
                                    
                                    if (!should_remove) {
                                        ASSERT_MSG(std::find(ins->thing_id_list.begin(), ins->thing_id_list.end(), other_thing->id) == ins->thing_id_list.end(), "duplicate");
                                        ins->thing_id_list.push_back(other_thing->id);
                                    }
                                }
                                
                            } else if (other_thing->archetype_id == mtt::ARCHETYPE_UI_ELEMENT) {
                                MTT_String_Ref* type_name = mtt::access<MTT_String_Ref>(other_thing, "typename");
                                if (type_name == nullptr || !MTT_string_ref_is_equal(dt_Instruction_label, *type_name)) {
                                    continue;
                                }
                                //ins->things_selection_modified_this_input.insert(other_thing->id);
                                
                                
                                dt::Instruction* child = *mtt::access_pointer_to_pointer<dt::Instruction*>(other_thing, "ctx_ptr");
                                dt::Instruction* parent = ins;
                                if (child->parent == ins) {
                                    if (child->link_disabled) {
                                        child->link_disabled = false;
                                    } else {
                                        child->link_disabled = true;
                                    }
                                    for (auto p_it = child->prop.begin(); p_it != child->prop.end(); ++p_it) {
                                        (*p_it)->parent_ref_disabled = child->link_disabled;
                                    }
                                } else if (ins->parent == child) {
                                    child = ins;
                                    if (child->link_disabled) {
                                        child->link_disabled = false;
                                    } else {
                                        child->link_disabled = true;
                                    }
                                    for (auto p_it = child->prop.begin(); p_it != child->prop.end(); ++p_it) {
                                        (*p_it)->parent_ref_disabled = child->link_disabled;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (ins->kind == "THING_INSTANCE_SELECTION") {
                        if (ins->thing_id_list.empty()) {
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                
                                prop->value.kind_string = "THING_INSTANCE";
                                prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                            }
                        } else if (ins->thing_id_list.size() == 1) {
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                
                                prop->value.kind_string = "THING_INSTANCE";
                                prop->value.thing = ins->thing_id_list.front();
                                
                                Speech_Property* plural;
                                if (prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                            }
                        } else {
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                
                                prop->value.kind_string = "LIST";
                                prop->value.list.resize(ins->thing_id_list.size());
                                prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = true;
                                }
                                
                                for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                                    prop->value.list[val_i] = Speech_Property::Value();
                                    prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                    prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                                }
                            }
                        }
                    }
                    
                };
                el->input_handlers.on_pen_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                    
                    Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                    
                    auto* dt_ctx = dt::DrawTalk::ctx();
                    auto& eval = dt_ctx->lang_ctx.eval_ctx;
                    eval.instruction_selected_with_pen() = nullptr;
                    
                    ins->color_current = ins->color;
                    
                    ins->pen_state = UI_TOUCH_STATE_ENDED;
                    
                    
                    for (auto it = dt_ctx->scn_ctx.selected_things.begin(); it != dt_ctx->scn_ctx.selected_things.end(); ++it) {
                        mtt::Thing* other_thing = dt_ctx->mtt->Thing_try_get(*it);
                        if (other_thing == nullptr || !dt::can_label_thing(other_thing)) {
                            if (other_thing != nullptr && other_thing->archetype_id == mtt::ARCHETYPE_UI_ELEMENT) {
                                
                                Instruction* other_ins = *mtt::access_pointer_to_pointer<Instruction*>(other_thing, "ctx_ptr");
                                if (other_ins != nullptr) {
                                    if (ins->kind == "THING_INSTANCE_SELECTION" && other_ins->kind == "THING_INSTANCE_SELECTION") {
                                        // do a swap;
                                        std::swap(ins->thing_id_list, other_ins->thing_id_list);
                                        
                                        {
                                            if (ins->thing_id_list.empty()) {
                                                for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                                    auto* prop = *p_it;
                                                    
                                                    prop->value.kind_string = "THING_INSTANCE";
                                                    prop->value.thing = mtt::Thing_ID_INVALID;
                                                    
                                                    Speech_Property* plural;
                                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                                        plural->value.flag = false;
                                                    }
                                                }
                                            } else if (ins->thing_id_list.size() == 1) {
                                                for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                                    auto* prop = *p_it;
                                                    
                                                    prop->value.kind_string = "THING_INSTANCE";
                                                    prop->value.thing = ins->thing_id_list.front();
                                                    
                                                    Speech_Property* plural;
                                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                                        plural->value.flag = false;
                                                    }
                                                }
                                            } else {
                                                for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                                    auto* prop = *p_it;
                                                    
                                                    prop->value.kind_string = "LIST";
                                                    prop->value.list.resize(ins->thing_id_list.size());
                                                    prop->value.thing = mtt::Thing_ID_INVALID;
                                                    
                                                    Speech_Property* plural;
                                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                                        plural->value.flag = true;
                                                    }
                                                    
                                                    for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                                                        prop->value.list[val_i] = Speech_Property::Value();
                                                        prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                                        prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                                                    }
                                                }
                                            }
                                            
                                            // other
                                            if (other_ins->thing_id_list.empty()) {
                                                for (auto p_it = other_ins->prop.begin(); p_it != other_ins->prop.end(); ++p_it) {
                                                    auto* prop = *p_it;
                                                    
                                                    prop->value.kind_string = "THING_INSTANCE";
                                                    prop->value.thing = mtt::Thing_ID_INVALID;
                                                    
                                                    Speech_Property* plural;
                                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                                        plural->value.flag = false;
                                                    }
                                                }
                                            } else if (other_ins->thing_id_list.size() == 1) {
                                                for (auto p_it = other_ins->prop.begin(); p_it != other_ins->prop.end(); ++p_it) {
                                                    auto* prop = *p_it;
                                                    
                                                    prop->value.kind_string = "THING_INSTANCE";
                                                    prop->value.thing = other_ins->thing_id_list.front();
                                                    
                                                    Speech_Property* plural;
                                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                                        plural->value.flag = false;
                                                    }
                                                }
                                            } else {
                                                for (auto p_it = other_ins->prop.begin(); p_it != other_ins->prop.end(); ++p_it) {
                                                    auto* prop = *p_it;
                                                    
                                                    prop->value.kind_string = "LIST";
                                                    prop->value.list.resize(ins->thing_id_list.size());
                                                    prop->value.thing = mtt::Thing_ID_INVALID;
                                                    
                                                    Speech_Property* plural;
                                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                                        plural->value.flag = true;
                                                    }
                                                    
                                                    for (usize val_i = 0; val_i < other_ins->thing_id_list.size(); val_i += 1) {
                                                        prop->value.list[val_i] = Speech_Property::Value();
                                                        prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                                        prop->value.list[val_i].thing = other_ins->thing_id_list[val_i];
                                                    }
                                                }
                                            }
                                        }
                                        
                                        
                                        ins->things_selection_modified_this_input.clear();
                                        other_ins->things_selection_modified_this_input.clear();
                                        
                                    }
                                }
                            }
                            continue;
                        }
                        
                        auto find_it = ins->things_selection_modified_this_input.find(other_thing->id);
                        if (find_it == ins->things_selection_modified_this_input.end()) {
                            if (dt::can_label_thing(other_thing) && (ins->kind == "THING_TYPE_SELECTION" || ins->kind == "THING_INSTANCE_SELECTION")) {
                                if (ins->kind == "THING_TYPE_SELECTION") {
                                    ins->kind = "THING_INSTANCE_SELECTION";
                                    for (auto p = ins->prop.begin(); p != ins->prop.end(); ++p) {
                                        if ((*p)->kind_str == "THING_TYPE_SELECTION") {
                                            (*p)->kind_str = "THING_INSTANCE_SELECTION";
                                        }
                                    }
                                }
                                
                                if (ins->kind == "THING_INSTANCE_SELECTION") {
                                    ins->things_selection_modified_this_input.insert(other_thing->id);
                                    bool should_remove = false;
                                    for (auto things_it = ins->thing_id_list.begin(); things_it != ins->thing_id_list.end();) {
                                        if ((*things_it) == other_thing->id) {
                                            mtt::Thing_destroy_proxies(mtt::world(thing), *things_it);
                                            things_it = ins->thing_id_list.erase(things_it);
                                            should_remove = true;
                                            break;
                                        } else {
                                            ++things_it;
                                        }
                                    }
                                    
                                    if (!should_remove) {
                                        ASSERT_MSG(std::find(ins->thing_id_list.begin(), ins->thing_id_list.end(), other_thing->id) == ins->thing_id_list.end(), "duplicate");
                                        ins->thing_id_list.push_back(other_thing->id);
                                    }
                                    
                                }
                                
                            } else if (other_thing->archetype_id == mtt::ARCHETYPE_UI_ELEMENT) {
                                MTT_String_Ref* type_name = mtt::access<MTT_String_Ref>(other_thing, "typename");
                                if (type_name == nullptr || !MTT_string_ref_is_equal(dt_Instruction_label, *type_name)) {
                                    continue;
                                }
                                //ins->things_selection_modified_this_input.insert(other_thing->id);
                                
                                dt::Instruction* child = *mtt::access_pointer_to_pointer<dt::Instruction*>(other_thing, "ctx_ptr");
                                dt::Instruction* parent = ins;
                                if (child->parent == ins) {
                                    if (child->link_disabled) {
                                        child->link_disabled = false;
                                    } else {
                                        child->link_disabled = true;
                                    }
                                    for (auto p_it = child->prop.begin(); p_it != child->prop.end(); ++p_it) {
                                        (*p_it)->parent_ref_disabled = child->link_disabled;
                                    }
                                } else if (ins->parent == child) {
                                    child = ins;
                                    if (child->link_disabled) {
                                        child->link_disabled = false;
                                    } else {
                                        child->link_disabled = true;
                                    }
                                    for (auto p_it = child->prop.begin(); p_it != child->prop.end(); ++p_it) {
                                        (*p_it)->parent_ref_disabled = child->link_disabled;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (ins->kind == "THING_INSTANCE_SELECTION") {
                        if (ins->thing_id_list.empty()) {
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                
                                prop->value.kind_string = "THING_INSTANCE";
                                prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                            }
                        } else if (ins->thing_id_list.size() == 1) {
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                
                                prop->value.kind_string = "THING_INSTANCE";
                                prop->value.thing = ins->thing_id_list.front();
                                
                                Speech_Property* plural;
                                if (prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                            }
                        } else {
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                
                                prop->value.kind_string = "LIST";
                                prop->value.list.resize(ins->thing_id_list.size());
                                prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = true;
                                }
                                
                                for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                                    prop->value.list[val_i] = Speech_Property::Value();
                                    prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                    prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                                }
                            }
                        }
                    }
                    
                    ins->things_selection_modified_this_input.clear();
                };
            }
            

            
            eval.thing_list.push_back(el->id);

            
        }
        

        return max_x;

        
    }
}


void Instruction_on_input(Instruction* ins, dt::DrawTalk* dt_ctx, mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
{
    mtt::World* world = mtt::world(thing);
    bool is_modified = false;
    
    auto& referrer_map = dt_ctx->lang_ctx.eval_ctx.referrer_map;
    auto& referrer_map_reversed = dt_ctx->lang_ctx.eval_ctx.referrer_map_reversed;
    
    for (auto it = dt_ctx->scn_ctx.selected_things.begin(); it != dt_ctx->scn_ctx.selected_things.end(); ++it) {
        mtt::Thing* other_thing = Thing_try_get(world, *it);
        
        
        if (Thing_is_proxy(other_thing)) {
            other_thing = Thing_try_get(world, other_thing->mapped_thing_id);
        }
        
        if (other_thing == nullptr || !dt::can_label_thing(other_thing)) {
            if (other_thing != nullptr && other_thing->archetype_id == mtt::ARCHETYPE_UI_ELEMENT) {
                
                Instruction* other_ins = *mtt::access_pointer_to_pointer<Instruction*>(other_thing, "ctx_ptr");
                if (other_ins != nullptr) {
                    if (ins->kind == "THING_INSTANCE_SELECTION" && other_ins->kind == "THING_INSTANCE_SELECTION") {
                        // do a swap
                        std::swap(ins->thing_id_list, other_ins->thing_id_list);
                        
                        {
                            if (ins->thing_id_list.empty()) {
                                for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                    auto* prop = *p_it;
                                    
                                    prop->value.kind_string = "THING_INSTANCE";
                                    prop->value.thing = mtt::Thing_ID_INVALID;
                                    
                                    Speech_Property* plural;
                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                        plural->value.flag = false;
                                    }
                                    
                                    Speech_Property* spec_prop = nullptr;
                                    if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                        spec_prop->value.flag = true;
                                    }
                                }
                            } else if (ins->thing_id_list.size() == 1) {
                                for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                    auto* prop = *p_it;
                                    
                                    prop->value.kind_string = "THING_INSTANCE";
                                    prop->value.thing = ins->thing_id_list.front();
                                    
                                    Speech_Property* plural;
                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                        plural->value.flag = false;
                                    }
                                    
                                    Speech_Property* spec_prop = nullptr;
                                    if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                        spec_prop->value.flag = true;
                                    }
                                }
                            } else {
                                for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                    auto* prop = *p_it;
                                    
                                    prop->value.kind_string = "LIST";
                                    prop->value.list.resize(ins->thing_id_list.size());
                                    prop->value.thing = mtt::Thing_ID_INVALID;
                                    
                                    Speech_Property* plural;
                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                        plural->value.flag = true;
                                    }
                                    
                                    
                                    Speech_Property* spec_prop = nullptr;
                                    if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                        spec_prop->value.flag = true;
                                    }
                                    
                                    for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                                        prop->value.list[val_i] = Speech_Property::Value();
                                        prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                        prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                                    }
                                }
                            }
                            
                            // other
                            if (other_ins->thing_id_list.empty()) {
                                for (auto p_it = other_ins->prop.begin(); p_it != other_ins->prop.end(); ++p_it) {
                                    auto* prop = *p_it;
                                    
                                    prop->value.kind_string = "THING_INSTANCE";
                                    prop->value.thing = mtt::Thing_ID_INVALID;
                                    
                                    Speech_Property* plural;
                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                        plural->value.flag = false;
                                    }
                                    
                                    Speech_Property* spec_prop = nullptr;
                                    if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                        spec_prop->value.flag = true;
                                    }
                                }
                            } else if (other_ins->thing_id_list.size() == 1) {
                                for (auto p_it = other_ins->prop.begin(); p_it != other_ins->prop.end(); ++p_it) {
                                    auto* prop = *p_it;
                                    
                                    prop->value.kind_string = "THING_INSTANCE";
                                    prop->value.thing = other_ins->thing_id_list.front();
                                    
                                    Speech_Property* plural;
                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                        plural->value.flag = false;
                                    }
                                    
                                    Speech_Property* spec_prop = nullptr;
                                    if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                        spec_prop->value.flag = true;
                                    }
                                }
                            } else {
                                for (auto p_it = other_ins->prop.begin(); p_it != other_ins->prop.end(); ++p_it) {
                                    auto* prop = *p_it;
                                    
                                    prop->value.kind_string = "LIST";
                                    prop->value.list.resize(ins->thing_id_list.size());
                                    prop->value.thing = mtt::Thing_ID_INVALID;
                                    
                                    Speech_Property* plural;
                                    if (prop->try_get_only_prop("PLURAL", &plural)) {
                                        plural->value.flag = true;
                                    }
                                    
                                    Speech_Property* spec_prop = nullptr;
                                    if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                        spec_prop->value.flag = true;
                                    }
                                    
                                    for (usize val_i = 0; val_i < other_ins->thing_id_list.size(); val_i += 1) {
                                        prop->value.list[val_i] = Speech_Property::Value();
                                        prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                        prop->value.list[val_i].thing = other_ins->thing_id_list[val_i];
                                    }
                                }
                            }
                        }
                        
                        
                        ins->things_selection_modified_this_input.clear();
                        other_ins->things_selection_modified_this_input.clear();
                        is_modified = true;
                    }
                }
            }
            continue;
        }
        
        //auto find_it = ins->things_selection_modified_this_input.find(other_thing->id);
        //if (find_it == ins->things_selection_modified_this_input.end())
        {
            if (dt::can_label_thing(other_thing) && (ins->kind == "THING_TYPE_SELECTION" || ins->kind == "THING_INSTANCE_SELECTION")) {
                if (ins->kind == "THING_TYPE_SELECTION") {
                    ins->kind = "THING_INSTANCE_SELECTION";
                    for (auto p = ins->prop.begin(); p != ins->prop.end(); ++p) {
                        if ((*p)->kind_str == "THING_TYPE_SELECTION") {
                            (*p)->kind_str = "THING_INSTANCE_SELECTION";
                        }
                    }
                }
                
                if (ins->kind == "THING_INSTANCE_SELECTION") {
                    //ins->things_selection_modified_this_input.insert(other_thing->id);
                    bool should_remove = false;
                    for (auto things_it = ins->thing_id_list.begin(); things_it != ins->thing_id_list.end();) {
                        if ((*things_it) == other_thing->id) {
                            Instruction_destroy_proxy_for_thing(ins, world, *things_it);
                            things_it = ins->thing_id_list.erase(things_it);
                            should_remove = true;
                            is_modified = true;
                            
                            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                                auto* prop = *p_it;
                                dt::Speech_Property::Prop_List* coref_prop_list = nullptr;
                                if (prop->try_get_prop("COREFERENCE", &coref_prop_list)) {
                                    for (auto it = coref_prop_list->begin(); it != coref_prop_list->end(); ++it) {
                                        
                                        
                                        
                                        
                                        
                                        uintptr ref = (*it)->value.reference;
                                        Speech_Property* ref_prop = (Speech_Property*)ref;
                                        Instruction* instruction = ref_prop->instruction->instruction_ref;
                                        Instruction* instruction_initial = instruction;
                                        while (instruction) {
                                            for (auto ref_it = instruction->thing_id_list.begin(); ref_it != instruction->thing_id_list.end(); ) {
                                                if ((*ref_it) == other_thing->id) {
                                                    Instruction_destroy_proxy_for_thing(instruction, world, (*ref_it));
                                                    ref_it = instruction->thing_id_list.erase(ref_it);
                                                    break;
                                                } else {
                                                    ++ref_it;
                                                }
                                            }
                                            instruction = (instruction != instruction->instruction_ref && instruction->instruction_ref != instruction_initial) ? instruction->instruction_ref : nullptr;
                                        }
                                    }
                                }
                            }
                            {
                                Instruction* instruction_curr = ins->referring_instruction_next;
                                Instruction* instruction_initial = instruction_curr;
                                while (instruction_curr) {
                                    for (auto ref_it = instruction_curr->thing_id_list.begin(); ref_it != instruction_curr->thing_id_list.end(); ) {
                                        if ((*ref_it) == other_thing->id) {
                                            Instruction_destroy_proxy_for_thing(instruction_curr, world, (*ref_it));
                                            ref_it = instruction_curr->thing_id_list.erase(ref_it);
                                            break;
                                        } else {
                                            ++ref_it;
                                        }
                                    }
                                    instruction_curr = instruction_curr->referring_instruction_next;
                                    if (instruction_curr == instruction_initial) {
                                        break;
                                    }
                                }
                            }
                            break;
                        } else {
                            ++things_it;
                        }
                    }
                    
                    
                    if (!should_remove) {
//                        ASSERT_MSG(std::find(ins->thing_id_list.begin(), ins->thing_id_list.end(), other_thing->id) == ins->thing_id_list.end(), "duplicate");
                        ins->thing_id_list.push_back(other_thing->id);
                    }
                    
                }
                
            } else if (other_thing->archetype_id == mtt::ARCHETYPE_UI_ELEMENT) {
                MTT_String_Ref* type_name = mtt::access<MTT_String_Ref>(other_thing, "typename");
                if (type_name == nullptr || !MTT_string_ref_is_equal(dt_Instruction_label, *type_name)) {
                    continue;
                }
                //ins->things_selection_modified_this_input.insert(other_thing->id);
                is_modified = true;
                dt::Instruction* child = *mtt::access_pointer_to_pointer<dt::Instruction*>(other_thing, "ctx_ptr");
                dt::Instruction* parent = ins;
                if (child->parent == ins) {
                    if (child->link_disabled) {
                        child->link_disabled = false;
                    } else {
                        child->link_disabled = true;
                    }
                    for (auto p_it = child->prop.begin(); p_it != child->prop.end(); ++p_it) {
                        (*p_it)->parent_ref_disabled = child->link_disabled;
                    }
                } else if (ins->parent == child) {
                    child = ins;
                    if (child->link_disabled) {
                        child->link_disabled = false;
                    } else {
                        child->link_disabled = true;
                    }
                    for (auto p_it = child->prop.begin(); p_it != child->prop.end(); ++p_it) {
                        (*p_it)->parent_ref_disabled = child->link_disabled;
                    }
                }
            }
        }
    }
    
    if (ins->kind == "THING_INSTANCE_SELECTION") {
        is_modified = true;
        if (ins->thing_id_list.empty()) {
            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                auto* prop = *p_it;
                
                prop->value.kind_string = "THING_INSTANCE";
                prop->value.thing = mtt::Thing_ID_INVALID;
                
                Speech_Property* plural;
                if (prop->try_get_only_prop("PLURAL", &plural)) {
                    plural->value.flag = false;
                }
                
                Speech_Property* spec_prop = nullptr;
                if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                    spec_prop->value.flag = true;
                }
                
                dt::Speech_Property::Prop_List* coref_prop_list = nullptr;
                if (prop->try_get_prop("COREFERENCE", &coref_prop_list)) {
                    for (auto it = coref_prop_list->begin(); it != coref_prop_list->end(); ++it) {
                        uintptr ref = (*it)->value.reference;
                        Speech_Property* ref_prop = (Speech_Property*)ref;
                        Instruction* instruction = ref_prop->instruction->instruction_ref;
                        Instruction* instruction_initial = instruction;
                        while (instruction) {
                            instruction->thing_id_list = ins->thing_id_list;
                            for (auto it_sub = instruction->prop.begin(); it_sub != instruction->prop.end(); ++it_sub) {
                                ref_prop = *it_sub;
                                
                                ref_prop->value.kind_string = "THING_INSTANCE";
                                ref_prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (ref_prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                                
                                Speech_Property* spec_prop = nullptr;
                                if (ref_prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                    spec_prop->value.flag = true;
                                }
                                //MTT_print("%s:%s\n", instruction->type.c_str(), instruction->kind.c_str());
                                ref_prop->value.list.clear();
                            }
                            instruction = (instruction != instruction->instruction_ref && instruction->instruction_ref != instruction_initial) ? instruction->instruction_ref : nullptr;
                        }
                    }
                }
                {
                    auto find_it = referrer_map.find(prop);
                    if (find_it != referrer_map.end()) {
                        for (auto* ref : find_it->second) {
                            
                            ref = ref->get_active_parent();
                            Instruction* instruction = ((Speech_Property*)ref)->instruction;
                            
                            instruction->thing_id_list = ins->thing_id_list;
                            for (auto it_sub = instruction->prop.begin(); it_sub != instruction->prop.end(); ++it_sub) {
                                Speech_Property* ref_prop = *it_sub;
                                
                                ref_prop->value.kind_string = "THING_INSTANCE";
                                ref_prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (ref_prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                                
                                Speech_Property* spec_prop = nullptr;
                                if (ref_prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                    spec_prop->value.flag = true;
                                }
                                //MTT_print("%s:%s\n", instruction->type.c_str(), instruction->kind.c_str());
                                ref_prop->value.list.clear();
                            }
                        }
                    }
                }
            }
        } else if (ins->thing_id_list.size() == 1) {
            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                auto* prop = *p_it;
                
                prop->value.kind_string = "THING_INSTANCE";
                prop->value.thing = ins->thing_id_list.front();
                
                Speech_Property* plural;
                if (prop->try_get_only_prop("PLURAL", &plural)) {
                    plural->value.flag = false;
                }
                
                Speech_Property* spec_prop = nullptr;
                if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                    spec_prop->value.flag = true;
                }
                
                dt::Speech_Property::Prop_List* coref_prop_list = nullptr;
                if (prop->try_get_prop("COREFERENCE", &coref_prop_list)) {
                    for (auto it = coref_prop_list->begin(); it != coref_prop_list->end(); ++it) {
                        uintptr ref = (*it)->value.reference;
                        Speech_Property* ref_prop = (Speech_Property*)ref;
                        Instruction* instruction = ref_prop->instruction->instruction_ref;
                        Instruction* instruction_initial = ref_prop->instruction->instruction_ref;
                        while (instruction) {
                            instruction->thing_id_list = ins->thing_id_list;
                            for (auto it_sub = instruction->prop.begin(); it_sub != instruction->prop.end(); ++it_sub) {
                                ref_prop = *it_sub;
                                
                                ref_prop->value.kind_string = "THING_INSTANCE";
                                ref_prop->value.thing = ins->thing_id_list.front();
                                
                                Speech_Property* plural;
                                if (ref_prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                                
                                Speech_Property* spec_prop = nullptr;
                                if (ref_prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                    spec_prop->value.flag = true;
                                }
                                //MTT_print("%s:%s\n", instruction->type.c_str(), instruction->kind.c_str());
                                ref_prop->value.list.clear();
                            }
                            instruction = (instruction != instruction->instruction_ref && instruction->instruction_ref != instruction_initial) ? instruction->instruction_ref : nullptr;
                        }
                    }
                    
                }
                {
                    auto find_it = referrer_map.find(prop);
                    if (find_it != referrer_map.end()) {
                        for (auto* ref : find_it->second) {
                            ref = ref->get_active_parent();
                            Instruction* instruction = ((Speech_Property*)ref)->instruction;
                                    
                            instruction->thing_id_list = ins->thing_id_list;
                            for (auto it_sub = instruction->prop.begin(); it_sub != instruction->prop.end(); ++it_sub) {
                                Speech_Property* ref_prop = *it_sub;
                                
                                ref_prop->value.kind_string = "THING_INSTANCE";
                                ref_prop->value.thing = ins->thing_id_list.front();
                                
                                Speech_Property* plural;
                                if (ref_prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = false;
                                }
                                
                                Speech_Property* spec_prop = nullptr;
                                if (ref_prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                    spec_prop->value.flag = true;
                                }
                                //MTT_print("%s:%s\n", instruction->type.c_str(), instruction->kind.c_str());
                                ref_prop->value.list.clear();
                            }
                        }
                    }
                }
            }
        } else {
            for (auto p_it = ins->prop.begin(); p_it != ins->prop.end(); ++p_it) {
                auto* prop = *p_it;
                
                prop->value.kind_string = "LIST";
                prop->value.list.resize(ins->thing_id_list.size());
                prop->value.thing = mtt::Thing_ID_INVALID;
                
                Speech_Property* plural;
                if (prop->try_get_only_prop("PLURAL", &plural)) {
                    plural->value.flag = true;
                }
                
                Speech_Property* spec_prop = nullptr;
                if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                    spec_prop->value.flag = true;
                }
                
                dt::Speech_Property::Prop_List* coref_prop_list = nullptr;
                if (prop->try_get_prop("COREFERENCE", &coref_prop_list)) {
                    for (auto it = coref_prop_list->begin(); it != coref_prop_list->end(); ++it) {
                        uintptr ref = (*it)->value.reference;
                        Speech_Property* ref_prop = (Speech_Property*)ref;
                        Instruction* instruction = ref_prop->instruction->instruction_ref;
                        Instruction* instruction_initial = ref_prop->instruction->instruction_ref;
                        while (instruction) {
                            instruction->thing_id_list = ins->thing_id_list;
                            for (auto it_sub = instruction->prop.begin(); it_sub != instruction->prop.end(); ++it_sub) {
                                ref_prop = *it_sub;
                                
                                ref_prop->value.kind_string = "LIST";
                                ref_prop->value.list.resize(ins->thing_id_list.size());
                                ref_prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (ref_prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = true;
                                }
                                
                                Speech_Property* spec_prop = nullptr;
                                if (ref_prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                    spec_prop->value.flag = true;
                                }
                                //MTT_print("%s:%s\n", instruction->type.c_str(), instruction->kind.c_str());
                                ref_prop->value.list.clear();
                                ref_prop->value.list.resize(ins->thing_id_list.size());
                                for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                                    ref_prop->value.list[val_i] = Speech_Property::Value();
                                    ref_prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                    ref_prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                                }
                            }
                            instruction = (instruction != instruction->instruction_ref && instruction->instruction_ref != instruction_initial) ? instruction->instruction_ref : nullptr;
                        }
                    }
                }
                {
                    auto find_it = referrer_map.find(prop);
                    if (find_it != referrer_map.end()) {
                        for (auto* ref : find_it->second) {
                            ref = ref->get_active_parent();
                            Instruction* instruction = ((Speech_Property*)ref)->instruction;
                            
                            instruction->thing_id_list = ins->thing_id_list;
                            for (auto it_sub = instruction->prop.begin(); it_sub != instruction->prop.end(); ++it_sub) {
                                Speech_Property* ref_prop = *it_sub;
                                
                                ref_prop->value.kind_string = "LIST";
                                ref_prop->value.list.resize(ins->thing_id_list.size());
                                ref_prop->value.thing = mtt::Thing_ID_INVALID;
                                
                                Speech_Property* plural;
                                if (ref_prop->try_get_only_prop("PLURAL", &plural)) {
                                    plural->value.flag = true;
                                }
                                
                                Speech_Property* spec_prop = nullptr;
                                if (ref_prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                                    spec_prop->value.flag = true;
                                }
                                //MTT_print("%s:%s\n", instruction->type.c_str(), instruction->kind.c_str());
                                ref_prop->value.list.clear();
                                ref_prop->value.list.resize(ins->thing_id_list.size());
                                for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                                    ref_prop->value.list[val_i] = Speech_Property::Value();
                                    ref_prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                    ref_prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                                }
                            }
                        }
                    }
                }
                
                for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                    prop->value.list[val_i] = Speech_Property::Value();
                    prop->value.list[val_i].kind_string = "THING_INSTANCE";
                    prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                }
                
            }
        }
    }
    ins->selections_overriden |= is_modified;
    //ins->things_selection_modified_this_input.clear();
}


#define DT_OLD_V2_FEEDBACK_UI (0)
void build_feedback_ui_from_instructions_inner(dt::DrawTalk* dt, Instruction* ins, DT_Evaluation_Context& eval)
{
    mtt::Rep* rep = nullptr;

    
    
    mtt::Box& box = ins->bounds;
    
    vec4 text_color = vec4(1.0f);
    float bounds[4];
    float text_advance = 0.0;
    
    
    
    
    
    
    // MARK: instruction ui layout


    //eval.layout.pos.x += text_advance + eval.layout.pad.x;
    
    
    float prev_x = eval.layout.pos.x;
    float prev_y = eval.layout.pos.y;
    
    //float next_y = eval.layout.pos.y + eval.layout.height + eval.layout.pad.y;
    float next_y = eval.layout.pos.y + (eval.layout.height * 0.25);
    float next_y_no_descend = eval.layout.pos.y;
    const float next_y_which[2] = {next_y_no_descend, next_y};
    // prev_x
    eval.layout.pos.y = next_y;
    {
        if (ins->child_count(Instruction_CHILD_SIDE_TYPE_LEFT) != 0) {
            auto& child_list = ins->child_list(Instruction_CHILD_SIDE_TYPE_LEFT);
            for (usize ch_idx = 0; ch_idx < child_list.size(); ch_idx += 1) {
                Instruction* ch_ins = child_list[ch_idx];
                eval.layout.pos.y = next_y_which[
                    ch_ins->should_descend
                ];
                build_feedback_ui_from_instructions_inner(dt, ch_ins, eval);
            }
        }
    }
    eval.layout.pos.y = prev_y;
    eval.layout.pos.x += eval.layout.pad.x * 0.5;
    
    const float x_offsets[2] = {0, eval.layout.pad.x * 2};
    const float x_offset = x_offsets[ins->x_offset];
    eval.layout.pos.x += x_offset;
    {
        ins->color = nvgRGBA(100,100,100,255/2);
        ins->color_selected = nvgRGBA(50,50, 50, 255/2);
        ins->color_current = ins->color;
        
        auto* vg = nvgGetGlobalContext();
        
        nvgSave(vg);
        
        if (ins->type == "ACTION") {
            text_color = dt::LABEL_COLOR_ACTION;
            
            mtt::String cpp_label = "";
            
            for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
                cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
            }
            
            if (ins->annotation == "unknown") {
                //cpp_label += " (?)";
                eval.instructions_to_disambiguate.push_back({ins, AMBIGUITY_TYPE_ACTION_UNKNOWN});
            }
            
            if (cpp_label == "be") {
                cpp_label = "is";
            }
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
            
        } else if (ins->type == "TRIGGER") {
            text_color = dt::LABEL_COLOR_TRIGGER;
            
            mtt::String cpp_label = "";
            
            for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
                cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
            }
            
            if (cpp_label == "be") {
                cpp_label = "is";
            }
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
            
        } else if (ins->type == "RESPONSE") {
            text_color = dt::LABEL_COLOR_RESPONSE;
            
            mtt::String cpp_label = "";
            
            for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
                cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
            }
            
            if (cpp_label == "be") {
                cpp_label = "is";
            }
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
        } else if (ins->type == "TRIGGER_RESPONSE") {
            //text_color = dt::LABEL_COLOR_TRIGGER;
            text_color = pen_color_default();
            bool stops_instead_of_causes = false;
            bool is_relational_trigger = false;
            for (usize ch_list_idx = 0; ch_list_idx < ins->child_list_count(); ch_list_idx += 1) {
                auto& child_list = ins->child_list(ch_list_idx);
                for (auto it = child_list.begin(); it != child_list.end(); ++it) {
                    if ((*it)->annotation == "END_CONDITION") {
                        cstring label = "UNTIL";
                        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + 5, bounds);
                        ins->display_label = label;
                        stops_instead_of_causes = true;
                        break;
                    } else if ((*it)->annotation == "RELATION" || (*it)->annotation == "CONTINUOUS") {
                        cstring label = "RELATES TO";
                        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + 10, bounds);
                        ins->display_label = label;
                        is_relational_trigger = true;
                    }
                }
            }
            
            if (!stops_instead_of_causes && !is_relational_trigger) {
                cstring label = LABEL_CAUSES;
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + LABEL_CAUSES_LEN, bounds);
                ins->display_label = label;
            }
            
            
        } else if (ins->kind == "THING_INSTANCE_SELECTION") {
            text_color = dt::LABEL_COLOR_THING_INSTANCE;
            if (ins->prop.front()->type_str == "pronoun" ||
                ins->prop.front()->value.is_reference) {
                text_color = dt::LABEL_COLOR_PRONOUN;
            }
            
            mtt::String cpp_label = "";
//            if (false && ins->type == "AGENT") {
//                cpp_label += "SOURCE" + ((!ins->prop.empty()) ? " : " + ins->prop.front()->label : "");
//            } else
            {
                cpp_label += //ins->type +
                ((!ins->prop.empty()) ? ins->prop.front()->label : "");
            }
            
            if (!ins->prop.empty()) {
                auto* num_prop = ins->prop.front();
                if (dt::numeric_value_words.find(num_prop->label) != dt::numeric_value_words.end()) {
                    cpp_label = "# " + cpp_label;
                    auto* val = num_prop->try_get_only_prop("COUNT");
                    if (val != nullptr) {
                        if (val->value.kind_string == "NUMERIC") {
                            cpp_label += " " + mtt::to_string_with_precision(val->value.numeric, 3);
                        } else if (val->value.kind_string == "VECTOR") {
                            cpp_label += " <" + mtt::to_string_with_precision(val->value.vector.x, 3) + ", " + mtt::to_string_with_precision(val->value.vector.y, 3) + ">";
                        }
                    }
                }
            }
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
            
        } else if (ins->kind == "THING_TYPE_SELECTION") {
            
            text_color = dt::LABEL_COLOR_THING_TYPE;
            
            //mtt::String cpp_label = (((ins->type == "AGENT") ? "SOURCE" : ins->type) + " : type ");
            mtt::String cpp_label = dt::THING_TYPE_SELECTION_INSTRUCTION_LABEL_PREFIX;
            
            if ((ins->prop.empty())) {
                cpp_label += "?";
            } else {
                
                auto* pr = ins->prop.front();
                
                dt::Speech_Property::Prop_List* list;
                if (pr->try_get_prop("COREFERENCE", &list)) {
                    uintptr ref = list->front()->value.reference;
                    if (ref != 0) {
                        Speech_Property* ref_prop = (Speech_Property*)ref;
                        cpp_label += ref_prop->label;
                    } else {
                        cpp_label += "?";
                    }
                } else {
                    cpp_label += ins->prop.front()->label;
                }
                
            }
            
            //if (ins->annotation == "unk)
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
            
        } else if (ins->type == "PREPOSITION") {
            text_color = dt::LABEL_COLOR_PREPOSITION;
            
            cstring label = ins->kind.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = ins->kind;
        } else if (ins->type == "PROPERTY") {
            text_color = dt::LABEL_COLOR_PROPERTY;
            
            cstring label = ins->kind.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = ins->kind;
        } else {
            text_color = color::BLACK;
            
            cstring label = ins->kind.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = ins->kind;
        }
        nvgRestore(vg);
        
        
        ins->text_color = text_color;
    }
    
    {
        box.tl = vec2(bounds[0], bounds[1]);
        box.br = vec2(bounds[0] + text_advance, bounds[3]);
        
        eval.layout.pos.x = box.br.x + (eval.layout.pad.x * 0.5f) + x_offset;
        
        auto box_centered = box;
        
        float32 width = box.br.x - box.tl.x;
        float32 h_width = width / 2.0f;
        float32 height = box.br.y - box.tl.y;
        float32 h_height = height / 2.0f;
        vec2 center = (box.br + box.tl) / 2.0f;
        box_centered.tl = vec2(-h_width, -h_height);
        box_centered.br = vec2(h_width, h_height);
        
        mtt::Thing* el = mtt::Thing_make_with_aabb_corners(dt->mtt, FEEDBACK_ELEMENT_THING_TYPE, vec2(1.0f), &rep, box_centered, 999.0f, true, &dt->ui.top_panel.collision_system_world);
        el->lock_to_canvas = false;
        mtt::Thing_set_position(el, vec3(center, 999.0f));
        el->forward_input_to_root = true;
        el->is_user_drawable = false;
        // MARK: for the moment, disallow deletion by user
        mtt::unset_flag(el, mtt::THING_FLAG_is_user_erasable);
        el->is_user_destructible = false;
        *mtt::access_pointer_to_pointer<Instruction*>(el, "ctx_ptr") = ins;
        ins->thing_id = el->id;
        
        MTT_String_Ref* str = mtt::access<MTT_String_Ref>(el, "typename");
        *str = dt_Instruction_label;
        MTT_string_ref_retain(dt_Instruction_label);
        
        {
#define INSTRUCTION_DEBUG_PRINT(...)
            //MTT_print(__VA_ARGS__)
            el->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
                
                if (cancel_input_default(event)) {
                    return false;
                }
                
                Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                INSTRUCTION_DEBUG_PRINT("on_touch_input_began %s\n", ins->display_label.c_str());
                
                auto& eval = dt::DrawTalk::ctx()->lang_ctx.eval_ctx;
                eval.instruction_selected_with_touch() = ins;
                ins->color_current = ins->color_selected;
                
                ins->touch_state = UI_TOUCH_STATE_BEGAN;
                
                
                return true;
            };
            el->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                
                if (cancel_input_default(event)) {
                    return;
                }
                
                Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                INSTRUCTION_DEBUG_PRINT("on_touch_input_moved %s\n", ins->display_label.c_str());
                
                auto& eval = dt::DrawTalk::ctx()->lang_ctx.eval_ctx;
                
                ins->touch_state = UI_TOUCH_STATE_MOVED;
            };
            el->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                
                if (cancel_input_default(event)) {
                    return;
                }
                
                Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                INSTRUCTION_DEBUG_PRINT("on_touch_input_ended %s\n", ins->display_label.c_str());
                
                auto& eval = dt::DrawTalk::ctx()->lang_ctx.eval_ctx;
                eval.instruction_selected_with_touch() = nullptr;
                ins->color_current = ins->color;
                
                ins->touch_state = UI_TOUCH_STATE_ENDED;

            };
            
            el->input_handlers.on_pen_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
                
                Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                INSTRUCTION_DEBUG_PRINT("on_pen_input_began %s\n", ins->display_label.c_str());
                
                auto* dt_ctx = dt::DrawTalk::ctx();
                auto& eval = dt_ctx->lang_ctx.eval_ctx;
                eval.instruction_selected_with_pen() = ins;
                ins->color_current = ins->color_selected;
                
                ins->pen_state = UI_TOUCH_STATE_BEGAN;
                
                
                Instruction_on_input(ins, dt_ctx, in, thing, world_pos, canvas_pos, event, touch, flags);
                
                return true;
            };
            
            el->input_handlers.on_pen_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                //
                Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                INSTRUCTION_DEBUG_PRINT("on_pen_input_moved %s\n", ins->display_label.c_str());
                //
                //                    auto* dt_ctx = dt::DrawTalk::ctx();
                //                    auto& eval = dt::DrawTalk::ctx()->lang_ctx.eval_ctx;
                //
                ins->pen_state = UI_TOUCH_STATE_MOVED;
            };
            

            el->input_handlers.on_pen_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
                
                
                Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                INSTRUCTION_DEBUG_PRINT("on_pen_input_ended %s\n", ins->display_label.c_str());

                
                auto* dt_ctx = dt::DrawTalk::ctx();
                auto& eval = dt_ctx->lang_ctx.eval_ctx;
                eval.instruction_selected_with_pen() = nullptr;
                
                ins->color_current = ins->color;
                
                ins->pen_state = UI_TOUCH_STATE_ENDED;
                
                //Instruction_on_input(ins, dt_ctx, in, thing, world_pos, canvas_pos, event, touch, flags);
            };
            
            el->on_destroy = [](mtt::Thing* thing) {
                Instruction* ins = *mtt::access_pointer_to_pointer<Instruction*>(thing, "ctx_ptr");
                
                mtt::World* world = mtt::world(thing);
                auto& proxies = ins->thing_proxies;
                for (auto [thing_id, proxy_id] : proxies) {
                    Thing_destroy(world, proxy_id);
                }
                proxies.clear();
            };
#undef INSTRUCTION_DEBUG_PRINT
        }
        

        eval.thing_list.push_back(el->id);
    }
    
    eval.layout.pos.x += eval.layout.pad.x * 0.5;
    
    ins->visit_id = eval.next_instruction_visit_id;
    eval.next_instruction_visit_id += 1;
    {
        if (ins->child_count(Instruction_CHILD_SIDE_TYPE_RIGHT) != 0) {
            auto& child_list = ins->child_list(Instruction_CHILD_SIDE_TYPE_RIGHT);
            for (usize ch_idx = 0; ch_idx < child_list.size(); ch_idx += 1) {
                Instruction* ch_ins = child_list[ch_idx];
                eval.layout.pos.y = next_y_which[
                    ch_ins->should_descend
                ];
                build_feedback_ui_from_instructions_inner(dt, ch_ins, eval);
            }
        }
    }
    eval.layout.pos.y = prev_y;
    
    
    
}
void build_feedback_ui_from_instructions(dt::DrawTalk* dt, DT_Evaluation_Context& eval)
{
    
    DT_Evaluation_Context* eval_ctx = &dt->lang_ctx.eval_ctx;
    eval_ctx->layout = {};
    
    eval_ctx->layout.pos    = vec2(0.0f, 0.0f);
    eval_ctx->layout.height = 1.0f;
    eval_ctx->layout.pad = vec2(layout_padding, layout_padding * 4.0f); // vec2(mtt_core_ctx()->viewport.width / layout_padding);
    
    auto* vg = nvgGetGlobalContext();
    
    nvgSave(vg);
    
    nvgFontSize(vg, eval.font_size);
    nvgFontFace(vg, eval.font_face);
    nvgTextAlign(vg, eval.align);
    nvgTextMetrics(vg, NULL, NULL, &eval.layout.height);
    
    
    mtt::Rep* root_rep;
    mtt::Thing* root = mtt::Thing_make_with_aabb_corners(dt->mtt, FEEDBACK_ELEMENT_THING_TYPE, vec2(1.0f), &root_rep, { .tl = vec2(-layout_padding_root / 2.0f), .br = vec2(layout_padding_root / 2.0f) }, 900.0f, true, &dt->ui.top_panel.collision_system_world);
    root->lock_to_canvas = false;
    root->is_user_drawable = false;
    root->is_user_movable = true;
    // MARK: for the moment, disallow deletion by user
    mtt::unset_flag(root, mtt::THING_FLAG_is_user_erasable);
    root->is_user_destructible = false;
    eval.root_thing_id = root->id;
    
    
    DT_scope_open();
    
    

    
    
    auto* instructions = &eval_ctx->instructions_saved();
    
//    for (usize i = 0; i < instructions->size(); i += 1) {
//        auto* ins = (*instructions)[i];
//        MTT_print("%llu %s\n", i, ins->to_string().c_str());
//    }
    eval.layout.pos += layout_padding_root * 0.5f;
    
    float32 max_x = eval_ctx->layout.pos.x;
#if DT_OLD_V2_FEEDBACK_UI
    DT_print("FIND ROOT INSTRUCTIONS\n");
    for (usize i = 0; i < instructions->size(); i += 1) {
        
        auto* ins = (*instructions)[i];

        // find roots
        if (ins->parent == nullptr) {
            float32 max_x_cand = build_feedback_ui_from_instructions_OLD(ct, ins, *eval_ctx);
            if (max_x_cand > max_x) {
                max_x = max_x_cand;
            }
            
            eval.layout.pos.x = max_x + eval.layout.pad.x;
            
        }
    }
    
#else
    
    for (usize i = 0; i < instructions->size(); i += 1) {
        
        auto* ins = (*instructions)[i];

        // find roots
        if (ins->is_root()) {
            eval_ctx->next_instruction_visit_id = 0;
            build_feedback_ui_from_instructions_inner(dt, ins, *eval_ctx);
        }
    }
    
    

#endif
    
    const usize instructions_count = instructions->size();
    for (usize i = 0; i < instructions_count; i += 1) {
        mtt::connect_parent_to_child(dt->mtt, root->id, ((*instructions)[i])->thing_id);
    }
    
    mtt::Thing_set_position(root, vec3(vec2(layout_padding_root * 0.5f), 999.0f));
    DT_scope_close();
    
    nvgRestore(vg);
}


float32 rebuild_feedback_ui_from_modified_instructions_OLD(dt::DrawTalk* dt, Instruction* ins, DT_Evaluation_Context& eval)
{
    
    mtt::Rep* rep = nullptr;
    
    auto* vg = nvgGetGlobalContext();
    
    nvgSave(vg);
    
    mtt::Box& box = ins->bounds;
    
    
    
    
    ins->color = nvgRGBA(100,100,100,255/2);
    ins->color_selected = nvgRGBA(50,50, 50, 255/2);
    ins->color_current = ins->color;
    
    vec4 text_color = vec4(1.0f);
    float bounds[4];
    float text_advance = 0.0;
    
    if (ins->type == "ACTION") {
        text_color = dt::LABEL_COLOR_ACTION;
        
        mtt::String cpp_label = "";
        
        for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
            cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
        }
        
        if (ins->annotation == "unknown") {
            cpp_label += " (?)";
            //eval.instructions_to_disambiguate.push_back({ins, AMBIGUITY_TYPE_ACTION_UNKNOWN});
        }
        
        cstring label = cpp_label.c_str();
        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
        
        ins->display_label = cpp_label;
        
    } else if (ins->type == "TRIGGER") {
        text_color = dt::LABEL_COLOR_TRIGGER;
        
        mtt::String cpp_label = "";
        
        for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
            cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
        }
        
        cstring label = cpp_label.c_str();
        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
        
        ins->display_label = cpp_label;
        
    } else if (ins->type == "RESPONSE") {
        text_color = dt::LABEL_COLOR_RESPONSE;
        
        mtt::String cpp_label = "";
        
        for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
            cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
        }
        
        
        cstring label = cpp_label.c_str();
        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
        
        ins->display_label = cpp_label;
    } else if (ins->type == "TRIGGER_RESPONSE") {
        //text_color = dt::LABEL_COLOR_ACTION;
        text_color = pen_color_default();
        bool stops_instead_of_causes = false;
        bool is_relational_trigger = false;
        for (usize ch_list_idx = 0; ch_list_idx < ins->child_list_count(); ch_list_idx += 1) {
            auto& child_list = ins->child_list(ch_list_idx);
            for (auto it = child_list.begin(); it != child_list.end(); ++it) {
                if ((*it)->annotation == "END_CONDITION") {
                    cstring label = "UNTIL";
                    text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + 5, bounds);
                    ins->display_label = label;
                    stops_instead_of_causes = true;
                    break;
                } else if ((*it)->annotation == "RELATION" || (*it)->annotation == "CONTINUOUS") {
                    cstring label = "RELATES TO";
                    text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + 10, bounds);
                    ins->display_label = label;
                    is_relational_trigger = true;
                }
            }
        }
        
        if (!stops_instead_of_causes && !is_relational_trigger) {
            cstring label = LABEL_CAUSES;
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + LABEL_CAUSES_LEN, bounds);
            ins->display_label = label;
        }
        
        
    } else if (ins->kind == "THING_INSTANCE_SELECTION") {
        text_color = dt::LABEL_COLOR_THING_INSTANCE;
        
        mtt::String cpp_label = "";
        if (ins->type == "AGENT") {
            cpp_label += "SOURCE" + ((!ins->prop.empty()) ? " : " + ins->prop.front()->label : "");
        } else {
            cpp_label += ins->type + ((!ins->prop.empty()) ? " : " + ins->prop.front()->label : "");
        }
        
        if (!ins->prop.empty()) {
            auto* num_prop = ins->prop.front();
            if (dt::numeric_value_words.find(num_prop->label) != dt::numeric_value_words.end()) {
                cpp_label = "# " + cpp_label;
                auto* val = num_prop->try_get_only_prop("COUNT");
                if (val != nullptr) {
                    if (val->value.kind_string == "NUMERIC") {
                        cpp_label += " " + mtt::to_string_with_precision(val->value.numeric, 3);
                    } else if (val->value.kind_string == "VECTOR") {
                        cpp_label += " <" + mtt::to_string_with_precision(val->value.vector.x, 3) + ", " + mtt::to_string_with_precision(val->value.vector.y, 3) + ">";
                    }
                }
            }
        }
        
        cstring label = cpp_label.c_str();
        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
        
        ins->display_label = cpp_label;
        
    } else if (ins->kind == "THING_TYPE_SELECTION") {
        
        text_color = dt::LABEL_COLOR_THING_TYPE;
        
        mtt::String cpp_label = (((ins->type == "AGENT") ? "SOURCE" : ins->type) + " : type ");
        
        if ((ins->prop.empty())) {
            cpp_label += "?";
        } else {
            
            auto* pr = ins->prop.front();
            
            dt::Speech_Property::Prop_List* list;
            if (pr->try_get_prop("COREFERENCE", &list)) {
                uintptr ref = list->front()->value.reference;
                if (ref != 0) {
                    Speech_Property* ref_prop = (Speech_Property*)ref;
                    cpp_label += ref_prop->label;
                } else {
                    cpp_label += "?";
                }
            } else {
                cpp_label += ins->prop.front()->label;
            }
            
        }
        
        //if (ins->annotation == "unk)
        
        cstring label = cpp_label.c_str();
        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
        
        ins->display_label = cpp_label;
        
    } else if (ins->type == "PREPOSITION") {
        text_color = dt::LABEL_COLOR_PREPOSITION;
        
        cstring label = ins->kind.c_str();
        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
        
        ins->display_label = ins->kind;
    } else if (ins->type == "PROPERTY") {
        text_color = dt::LABEL_COLOR_PROPERTY;
        
        cstring label = ins->kind.c_str();
        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
        
        ins->display_label = ins->kind;
    }
    
    ins->text_color = text_color;
    
    
    
    
    
    box.tl = vec2(bounds[0], bounds[1]);
    box.br = vec2(eval.layout.pos.x + text_advance, bounds[1] + eval.layout.height);
    
    mtt::Thing* el = dt->mtt->Thing_try_get(ins->thing_id);
    rep = mtt::rep(el);
    ASSERT_MSG(el != nullptr, "The UI element should still exist\n");
    mtt::Thing_set_aabb_corners(el, &rep, box, 999.9f);
   

    {
        // MARK: instruction ui layout
        
        float prev_x = eval.layout.pos.x;
        float prev_y = eval.layout.pos.y;
        //eval.layout.pos.x += text_advance + eval.layout.pad.x;
        eval.layout.pos.y += (eval.layout.height) + eval.layout.pad.y;
        
        float max_x = eval.layout.pos.x + text_advance + eval.layout.pad.x;
        
        if (ins->child_count() != 0) {
            for (usize ch_list_idx = 0; ch_list_idx < ins->child_list_count(); ch_list_idx += 1) {
                auto& child_list = ins->child_list(ch_list_idx);
                for (usize ch_idx = 0; ch_idx < child_list.size(); ch_idx += 1) {
                    Instruction* ch_ins = child_list[ch_idx];
                    float ret_x = rebuild_feedback_ui_from_modified_instructions_OLD(dt, ch_ins, eval);
                    if (ret_x > max_x) {
                        max_x = ret_x;
                    }
                    eval.layout.pos.x = max_x + eval.layout.pad.x;
                }
            }
        } else {
            max_x += 10.0f;
        }
        
        eval.layout.pos.x = prev_x;
        eval.layout.pos.y = prev_y;
        
        nvgRestore(vg);
        
        
        return max_x;
        
    }
}

void rebuild_feedback_ui_from_modified_instructions_inner(dt::DrawTalk* dt, Instruction* ins, DT_Evaluation_Context& eval)
{
    mtt::Rep* rep = nullptr;

    
    
    mtt::Box& box = ins->bounds;
    
    vec4 text_color = vec4(1.0f);
    float bounds[4];
    float text_advance = 0.0;
    
    
    
    
    
    
    // MARK: instruction ui layout


    //eval.layout.pos.x += text_advance + eval.layout.pad.x;
    
    
    float prev_x = eval.layout.pos.x;
    float prev_y = eval.layout.pos.y;
    
    //float next_y = eval.layout.pos.y + eval.layout.height + eval.layout.pad.y;
    float next_y = eval.layout.pos.y + (eval.layout.height * 0.25);
    float next_y_no_descend = eval.layout.pos.y;
    const float next_y_which[2] = {next_y_no_descend, next_y};
    // prev_x
    eval.layout.pos.y = next_y;
    {
        if (ins->child_count(Instruction_CHILD_SIDE_TYPE_LEFT) != 0) {
            auto& child_list = ins->child_list(Instruction_CHILD_SIDE_TYPE_LEFT);
            for (usize ch_idx = 0; ch_idx < child_list.size(); ch_idx += 1) {
                Instruction* ch_ins = child_list[ch_idx];
                eval.layout.pos.y = next_y_which[
                    ch_ins->should_descend
                ];
                build_feedback_ui_from_instructions_inner(dt, ch_ins, eval);
            }
        }
    }
    eval.layout.pos.y = prev_y;
    eval.layout.pos.x += eval.layout.pad.x * 0.5;
    
    const float x_offsets[2] = {0, eval.layout.pad.x * 2};
    const float x_offset = x_offsets[ins->x_offset];
    eval.layout.pos.x += x_offset;
    {
        ins->color = nvgRGBA(100,100,100,255/2);
        ins->color_selected = nvgRGBA(50,50, 50, 255/2);
        ins->color_current = ins->color;
        
        auto* vg = nvgGetGlobalContext();
        
        nvgSave(vg);
        
        if (ins->type == "ACTION") {
            text_color = dt::LABEL_COLOR_ACTION;
            
            mtt::String cpp_label = "";
            
            for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
                cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
            }
            
            if (ins->annotation == "unknown") {
                //cpp_label += " (?)";
//                eval.instructions_to_disambiguate.push_back({ins, AMBIGUITY_TYPE_ACTION_UNKNOWN});
            }
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
            
        } else if (ins->type == "TRIGGER") {
            text_color = dt::LABEL_COLOR_TRIGGER;
            
            mtt::String cpp_label = "";
            
            for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
                cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
            }
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
            
        } else if (ins->type == "RESPONSE") {
            text_color = dt::LABEL_COLOR_RESPONSE;
            
            mtt::String cpp_label = "";
            
            for (auto it = ins->prop.begin(); it != ins->prop.end(); ++it) {
                cpp_label += (*it)->label + (((*it)->sub_label.empty()) ? "" : " : " + (*it)->sub_label);
            }
            
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
        } else if (ins->type == "TRIGGER_RESPONSE") {
            //text_color = dt::LABEL_COLOR_TRIGGER;
            text_color = pen_color_default();
            bool stops_instead_of_causes = false;
            bool is_relational_trigger = false;
            for (usize ch_list_idx = 0; ch_list_idx < ins->child_list_count(); ch_list_idx += 1) {
                auto& child_list = ins->child_list(ch_list_idx);
                for (auto it = child_list.begin(); it != child_list.end(); ++it) {
                    if ((*it)->annotation == "END_CONDITION") {
                        cstring label = "UNTIL";
                        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + 5, bounds);
                        ins->display_label = label;
                        stops_instead_of_causes = true;
                        break;
                    } else if ((*it)->annotation == "RELATION" || (*it)->annotation == "CONTINUOUS") {
                        cstring label = "RELATES TO";
                        text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + 10, bounds);
                        ins->display_label = label;
                        is_relational_trigger = true;
                    }
                }
            }
            
            if (!stops_instead_of_causes && !is_relational_trigger) {
                cstring label = LABEL_CAUSES;
                text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, label + LABEL_CAUSES_LEN, bounds);
                ins->display_label = label;
            }
            
            
        } else if (ins->kind == "THING_INSTANCE_SELECTION") {
            text_color = dt::LABEL_COLOR_THING_INSTANCE;
            if (ins->prop.front()->type_str == "pronoun" ||
                ins->prop.front()->value.is_reference) {
                text_color = dt::LABEL_COLOR_PRONOUN;
            }
            
            mtt::String cpp_label = "";
            if (false && ins->type == "AGENT") {
                cpp_label += "SOURCE" + ((!ins->prop.empty()) ? " : " + ins->prop.front()->label : "");
            } else {
                cpp_label += //ins->type +
                ((!ins->prop.empty()) ? ins->prop.front()->label : "");
            }
            
            if (!ins->prop.empty()) {
                auto* num_prop = ins->prop.front();
                if (dt::numeric_value_words.find(num_prop->label) != dt::numeric_value_words.end()) {
                    cpp_label = "# " + cpp_label;
                    auto* val = num_prop->try_get_only_prop("COUNT");
                    if (val != nullptr) {
                        if (val->value.kind_string == "NUMERIC") {
                            cpp_label += " " + mtt::to_string_with_precision(val->value.numeric, 3);
                        } else if (val->value.kind_string == "VECTOR") {
                            cpp_label += " <" + mtt::to_string_with_precision(val->value.vector.x, 3) + ", " + mtt::to_string_with_precision(val->value.vector.y, 3) + ">";
                        }
                    }
                }
            }
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
            
        } else if (ins->kind == "THING_TYPE_SELECTION") {
            
            text_color = dt::LABEL_COLOR_THING_TYPE;
            
            //mtt::String cpp_label = (((ins->type == "AGENT") ? "SOURCE" : ins->type) + " : type ");
            mtt::String cpp_label = dt::THING_TYPE_SELECTION_INSTRUCTION_LABEL_PREFIX;
            
            if ((ins->prop.empty())) {
                cpp_label += "?";
            } else {
                
                auto* pr = ins->prop.front();
                
                dt::Speech_Property::Prop_List* list;
                if (pr->try_get_prop("COREFERENCE", &list)) {
                    uintptr ref = list->front()->value.reference;
                    if (ref != 0) {
                        Speech_Property* ref_prop = (Speech_Property*)ref;
                        cpp_label += ref_prop->label;
                    } else {
                        cpp_label += "?";
                    }
                } else {
                    cpp_label += ins->prop.front()->label;
                }
                
            }
            
            //if (ins->annotation == "unk)
            
            cstring label = cpp_label.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = cpp_label;
            
        } else if (ins->type == "PREPOSITION") {
            text_color = dt::LABEL_COLOR_PREPOSITION;
            
            cstring label = ins->kind.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = ins->kind;
        } else if (ins->type == "PROPERTY") {
            text_color = dt::LABEL_COLOR_PROPERTY;
            
            cstring label = ins->kind.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = ins->kind;
        } else {
            text_color = color::BLACK;
            
            cstring label = ins->kind.c_str();
            text_advance = nvgTextBounds(vg, eval.layout.pos.x, eval.layout.pos.y, label, NULL, bounds);
            
            ins->display_label = ins->kind;
        }
        nvgRestore(vg);
        
        
        ins->text_color = text_color;
    }
    
    {
        box.tl = vec2(bounds[0], bounds[1]);
        box.br = vec2(bounds[0] + text_advance, bounds[3]);
        
        eval.layout.pos.x = box.br.x + (eval.layout.pad.x * 0.5f) + x_offset;
        
        auto box_centered = box;
        
        float32 width = box.br.x - box.tl.x;
        float32 h_width = width / 2.0f;
        float32 height = box.br.y - box.tl.y;
        float32 h_height = height / 2.0f;
        vec2 center = (box.br + box.tl) / 2.0f;
        box_centered.tl = vec2(-h_width, -h_height);
        box_centered.br = vec2(h_width, h_height);
        
        mtt::Thing* el = dt->mtt->Thing_try_get(ins->thing_id);
        rep = mtt::rep(el);
        ASSERT_MSG(el != nullptr, "The UI element should still exist\n");
        mtt::Thing_set_aabb_corners(el, &rep, box, 999.9f);
    }
    
    eval.layout.pos.x += eval.layout.pad.x * 0.5;
    {
        if (ins->child_count(Instruction_CHILD_SIDE_TYPE_RIGHT) != 0) {
            auto& child_list = ins->child_list(Instruction_CHILD_SIDE_TYPE_RIGHT);
            for (usize ch_idx = 0; ch_idx < child_list.size(); ch_idx += 1) {
                Instruction* ch_ins = child_list[ch_idx];
                eval.layout.pos.y = next_y_which[
                    ch_ins->should_descend
                ];
                build_feedback_ui_from_instructions_inner(dt, ch_ins, eval);
            }
        }
    }
    eval.layout.pos.y = prev_y;
    
}

void rebuild_feedback_ui_from_modified_instructions(dt::DrawTalk* dt, DT_Evaluation_Context& eval)
{
    DT_Evaluation_Context* eval_ctx = &eval;
    eval_ctx->layout = {};

    eval_ctx->layout.pos    = vec2(0.0f, 0.0f);
    eval_ctx->layout.height = 1.0f;
    eval_ctx->layout.pad = vec2(layout_padding, layout_padding * 4.0f);
    
    auto* vg = nvgGetGlobalContext();
    
    nvgSave(vg);
    
    nvgFontSize(vg, eval.font_size);
    nvgFontFace(vg, eval.font_face);
    nvgTextAlign(vg, eval.align);
    nvgTextMetrics(vg, NULL, NULL, &eval.layout.height);
    
    DT_scope_open();
    
    auto* instructions = &eval_ctx->instructions_saved();
    ASSERT_MSG(instructions != nullptr, "Why would theis be nullptr?");
    
    mtt::Thing* root = dt->mtt->Thing_try_get(eval_ctx->root_thing_id);
    ASSERT_MSG(root != nullptr, "Eval instructions should never be null here!");
    
    for (usize i = 0; i < instructions->size(); i += 1) {
        mtt::disconnect_child_from_parent(dt->mtt, ((*instructions)[i])->thing_id);
    }
    vec3 saved_position = *mtt::access<vec3>(root, "position");
    mtt::Thing_set_position(root, vec3(0.0f, 0.0f, 999.0f));
    
    eval.layout.pos += layout_padding_root * 0.5f;
    
    float32 max_x = eval_ctx->layout.pos.x;
    
#if DT_OLD_V2_FEEDBACK_UI
    {
        float32 max_x = eval_ctx->layout.pos.x;
        for (usize i = 0; i < instructions->size(); i += 1) {
            auto* ins = (*instructions)[i];
            // find roots
            if (ins->parent == nullptr) {
                float32 max_x_cand = rebuild_feedback_ui_from_modified_instructions_OLD(ct, ins, *eval_ctx);
                if (max_x_cand > max_x) {
                    max_x = max_x_cand;
                }
                
                eval.layout.pos.x = max_x + eval.layout.pad.x;
                
            }
        }
    }
#else
    for (usize i = 0; i < instructions->size(); i += 1) {
        
        auto* ins = (*instructions)[i];

        // find roots
        if (ins->is_root()) {
            rebuild_feedback_ui_from_modified_instructions_inner(dt, ins, *eval_ctx);
        }
    }
#endif
    
    const usize instructions_count = instructions->size();
    for (usize i = 0; i < instructions_count; i += 1) {
        mtt::connect_parent_to_child(dt->mtt, root->id, ((*instructions)[i])->thing_id);
    }

    mtt::Thing_set_position(root, saved_position);
    DT_scope_close();
    
    nvgRestore(vg);
}


void set_ui_layer(dt::DrawTalk* ctx, dt::UI* ui, uint64 priority_layer)
{
    uint64 prev_priority_layer = mtt::get_priority_layer(ctx->mtt);
    mtt::push_priority_layer(ctx->mtt, prev_priority_layer);
    mtt::set_priority_layer(ctx->mtt, priority_layer);
}
    
#define DT_OLD_BLOCKING_PROMPT_FOR_DIFFERENT_ACTIONS (false)
void prompt_for_different_actions(dt::DrawTalk* dt)
{
    auto& eval_ctx = dt->lang_ctx.eval_ctx;
    auto* ui = &dt->ui;
    
    if (dt->lang_ctx.eval_ctx.prompting_for_different_actions) {
        return;
    }
    
    dt->lang_ctx.eval_ctx.prompting_for_different_actions = true;
    
    ui->element_menu.set_is_active();
    
    mtt::Thing* t = mtt::Thing_try_get(dt->mtt, ui->element_menu.thing_id);
    if (t != nullptr) {
        mtt::Rep* r = mtt::rep(t);
        for (usize c_i = 0; c_i < r->colliders.size(); c_i += 1) {
            mtt::Collider* col = r->colliders[c_i];
            mtt::push_AABB(col->system, col);
        }
        mtt::set_is_active(t);
    }
    
    
#if DT_OLD_BLOCKING_PROMPT_FOR_DIFFERENT_ACTIONS
        
        
        
        set_ui_layer(dt, ui, dt->ui.element_menu.priority_layer);
        
        
        //auto* button_confirm = ui->dock.label_to_button.find(DT_SPEECH_COMMAND_BUTTON_NAME)->second;
        //auto* button_discard = ui->dock.label_to_button.find("discard")->second;
        //auto* button_show_hide = ui->dock.label_to_button.find("toggle labels")->second;
        auto& buttons = ui->dock.buttons;
        for (auto it = buttons.begin(); it != buttons.end(); ++it) {
            UI_Button* button = *it;
            if (button->name == DT_SPEECH_COMMAND_BUTTON_NAME ||
                button->name == "discard" ||
                button->name == "toggle labels") {
                
                mtt::Thing* thing = mtt::Thing_try_get(dt->mtt, button->thing);
                if (thing != nullptr) {
                    auto* r = mtt::rep(thing);
                    for (auto c : r->colliders) {
                        c->priority_layer = ui->element_menu.priority_layer;
                    }
                }
                
            } else {
                mtt::Thing* thing = mtt::Thing_try_get(dt->mtt, button->thing);
                if (thing != nullptr) {
                    mtt::unset_is_active(thing);
                    mtt::unset_should_render(thing);
                }
                
            }
        }
        
        auto l_to_set = dt->ui.element_menu.priority_layer;
        Thing_try_get_then(dt->mtt, dt->lang_ctx.eval_ctx.root_thing_id, [l_to_set](mtt::Thing* thing) {
            Thing_set_priority_layer(thing, l_to_set);
        });
#endif
    
    struct CTX {
        dt::DrawTalk* dt;
        usize idx;
    };
    
    for (usize idx = 0; auto& dis : dt->lang_ctx.eval_ctx.instructions_to_disambiguate) {
        
            
//        CTX* in = mem::allocate<CTX>(&(dt->mtt->allocator));
//        in->dt = dt;
//        in->idx = idx;
        
        dt::query_related_info_for_unknown_word(dt::DrawTalk::ctx(), dis.ins->prop.front()->label, "v", [dt, idx](mtt::String& data, void* doc_ptr, usize ID) {
            
            //ASSERT_MSG(mtt::is_main_thread(), "should be in main thread!");
            
            ArduinoJson::DynamicJsonDocument& doc = *static_cast<ArduinoJson::DynamicJsonDocument*>(doc_ptr);
            
            mtt::String debug_print = "";
            serializeJsonPretty(doc, debug_print);

            MTT_print("ID was: %llu\n", ID);
            
            Language_Context* lang_ctx = &dt->lang_ctx;
            DT_Evaluation_Context* eval_ctx = &lang_ctx->eval_ctx;
            (void)eval_ctx;
            DT_Behavior_Catalogue* cat = &lang_ctx->behavior_catalogue;
            
            {
                mtt::Set<DT_Behavior*> found_matches;
                const auto root = doc["entries"].as<JsonArrayConst>();
                for (const auto entry : root) {
                    {
                        const auto hypernyms = entry["hyper"]["v"].as<JsonArrayConst>();
                        for (usize hyper_idx = 0; const auto hypernym : hypernyms) {
                            mtt::String label = std::string(hypernym.as<JsonArrayConst>()[0]);
                            //std::cout << "[" << label << "]" << std::endl;
                            DT_Behavior_Catalogue::Alternatives_Map* b = nullptr;
                            if (cat->lookup(label, &b)) {
                                for (auto it = b->begin(); it != b->end(); ++it) {
                                    auto& match = it->second;
                                    if (!found_matches.contains(&match)) {
                                        dt->lang_ctx.eval_ctx.instructions_to_disambiguate[idx].action_alternatives.push_back({.behavior = &match});
                                        found_matches.insert(&match);
                                    } else {
//                                        std::cout << "duplicate: " << match.key_primary + ":" << match.key_secondary << std::endl;
                                    }
                                }
                                
                            }
                            
                            hyper_idx += 1;
                        }
                    }
                    {
                        const auto hyponyms = entry["hypo"]["v"].as<JsonArrayConst>();
                        for (usize hypo_idx = 0; const auto hyponym : hyponyms) {
                            mtt::String label = std::string(hyponym.as<JsonArrayConst>()[0]);
                            //std::cout << "[" << label << "]" << std::endl;
                            DT_Behavior_Catalogue::Alternatives_Map* b = nullptr;
                            if (cat->lookup(label, &b)) {
                                for (auto it = b->begin(); it != b->end(); ++it) {
                                    auto& match = it->second;
                                    if (!found_matches.contains(&match)) {
                                        dt->lang_ctx.eval_ctx.instructions_to_disambiguate[idx].action_alternatives.push_back({.behavior = &match});
                                        found_matches.insert(&match);
                                    } else {
//                                        std::cout << "duplicate: " << match.key_primary + ":" << match.key_secondary << std::endl;
                                    }
                                }
                            }
                            hypo_idx += 1;
                        }
                    }
                }
            }
//            std::cout << "BEGIN" << std::endl;
//            for (const auto& e : dt->lang_ctx.eval_ctx.instructions_to_disambiguate[idx].action_alternatives) {
//                std::cout << e.behavior->key_primary + ":" << e.behavior->key_secondary << std::endl;
//            }
//            std::cout << "END" << std::endl;
            //std::cout << "{" << std::endl;
            //std::cout << "RESULTS ARRIVED FOR: idx=[" << idx << "]";
            //std::cout << debug_print << std::endl;
            dt->lang_ctx.eval_ctx.instructions_to_disambiguate[idx].results_ready = true;
            //std::cout << "}" << std::endl;
            MTT_print("{\nRESULTS ARRIVED FOR: idx=[%llu] %s\n}\n", idx, debug_print.c_str());
            //mem::deallocate<CTX>(dt->mtt->allocator);
        });
        
        idx += 1;
    }
    
    
}
    
void leave_prompt_for_different_actions(dt::DrawTalk* dt)
{
    auto& eval_ctx = dt->lang_ctx.eval_ctx;
    auto* ui = &dt->ui;
    
    dt->lang_ctx.eval_ctx.prompting_for_different_actions = false;
    ui->element_menu.set_is_inactive();
    mtt::Thing* t = mtt::Thing_try_get(dt->mtt, ui->element_menu.thing_id);
    if (t != nullptr) {
        mtt::Rep* r = mtt::rep(t);
        for (usize c_i = 0; c_i < r->colliders.size(); c_i += 1) {
            mtt::Collider_remove(nullptr, 0, r->colliders[c_i]);
        }
        mtt::unset_is_active(t);
    }
#if DT_OLD_BLOCKING_PROMPT_FOR_DIFFERENT_ACTIONS
        
        auto prev_layer = mtt::pop_priority_layer(dt->mtt);
        mtt::set_priority_layer(dt->mtt, prev_layer);
        
        //auto* button_confirm = ui->dock.label_to_button.find(DT_SPEECH_COMMAND_BUTTON_NAME)->second;
        //auto* button_discard = ui->dock.label_to_button.find("discard")->second;
        //auto* button_show_hide = ui->dock.label_to_button.find("toggle labels")->second;
        auto& buttons = ui->dock.buttons;
        for (auto it = buttons.begin(); it != buttons.end(); ++it) {
            UI_Button* button = *it;
            if (button->name == DT_SPEECH_COMMAND_BUTTON_NAME ||
                button->name == "discard" ||
                button->name == "toggle labels") {
                
                Thing_try_get_then(dt->mtt, button->thing, [prev_layer](mtt::Thing* thing) {
                    Thing_set_priority_layer(thing, prev_layer);
                });
                
            } else {
                Thing_try_get_then(dt->mtt, button->thing, [](auto thing) {
                    mtt::set_is_active(thing);
                    mtt::set_should_render(thing);
                });
                
            }
        }
        
        Thing_try_get_then(dt->mtt, dt->lang_ctx.eval_ctx.root_thing_id, [prev_layer](mtt::Thing* thing) {
            Thing_set_priority_layer(thing, prev_layer);
        });
        
#endif
    
}



mem::Pool_Allocation UI_Feedback_Node::pool = (mem::Pool_Allocation){};
    

}

namespace dt {

static const vec4 bg_choice[2] = {
    BG_COLOR_DARK_MODE,
    BG_COLOR_LIGHT_MODE,
};

vec4 bg_color_default(void)
{
    return bg_choice[mtt::color_scheme()];
}

vec4 bg_color_default_for_color_scheme(mtt::Color_Scheme_ID id)
{
    return bg_choice[id];
}

static const vec4 pen_choice[2] = {
    PEN_COLOR_DARK_MODE,
    PEN_COLOR_LIGHT_MODE,
};

vec4 pen_color_default(void)
{
    return pen_choice[mtt::color_scheme()];
}

vec4 pen_color_default_for_color_scheme(mtt::Color_Scheme_ID id)
{
    return pen_choice[id];
}


void Scroll_Panel_init(Scroll_Panel* self, mtt::World* world)
{
    mtt::Thing* thing = nullptr;
    if (!mtt::Thing_try_get(world, self->base.thing, &thing)) {
        return;
    }
    
    thing->input_handlers.on_touch_input_began = Scroll_Panel_on_touch_input_began;
    thing->input_handlers.on_touch_input_moved = Scroll_Panel_on_touch_input_moved;
    thing->input_handlers.on_touch_input_ended = Scroll_Panel_on_touch_input_ended;
    
    thing->input_handlers.on_pen_input_began = Scroll_Panel_on_pen_input_began;
    thing->input_handlers.on_pen_input_moved = Scroll_Panel_on_pen_input_moved;
    thing->input_handlers.on_pen_input_ended = Scroll_Panel_on_pen_input_ended;
}

mtt::Input_Handler_Return Scroll_Panel_on_touch_input_began(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
{
    return true;
}

void Scroll_Panel_on_touch_input_moved(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
{
    
}

void Scroll_Panel_on_touch_input_ended(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
{
    
}

mtt::Input_Handler_Return Scroll_Panel_on_pen_input_began(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
{
    return true;
}

void Scroll_Panel_on_pen_input_moved(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
{
    
}

void Scroll_Panel_on_pen_input_ended(mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags)
{
    
}

bool map_view_selected(mtt::World* world, DrawTalk* dt, vec2 canvas_pos)
{
    bool is_map_view_selected = false;
    
    mtt::Collider* query_collider_result;
    mtt::Hit pre_hit;
    float32 pre_min_area = POSITIVE_INFINITY;
    auto* map_area = mtt::Thing_get(world, dt->ui.top_panel.base.thing);
    mtt::Rep* rep = mtt::rep(map_area);
    mtt::Collider* map_view_collider = rep->colliders.front();
    
    if (mtt::point_query_on_colliders(&map_view_collider, 1, canvas_pos, &query_collider_result, pre_hit, &pre_min_area)) {
        is_map_view_selected = true;
    }
    
    return is_map_view_selected;
}

mtt::Collision_System_Group_World_Canvas* map_view_collision_system_group(DrawTalk* dt)
{
    return &dt->ui.top_panel.collision_system_group;
}

bool Context_View_enable(dt::DrawTalk* dt)
{
    mtt::World* world = dt->mtt;
    Context_View* ctx_view = &dt->ui.context_view;
    auto* thing_context_view = mtt::Thing_get(world, ctx_view->thing_id);
    
    if (!mtt::is_active(thing_context_view)) {
        ctx_view->time_elapsed_since_update = CONTEXT_VIEW_T_INTERVAL;
        mtt::set_is_active(thing_context_view);
    }
    
    return true;//handle_context_sensitive_command(dt);
}
void Context_View_disable(dt::DrawTalk* dt)
{
    Context_View* ctx_view = &dt->ui.context_view;
    mtt::World* world = dt->mtt;
    auto* thing_context_view = mtt::Thing_get(world, ctx_view->thing_id);
    
    usize selected_things_count = dt->scn_ctx.selected_things.size();
    
    
    
    if (!mtt::is_active(thing_context_view)) {
        return;
    }
    
    ctx_view->time_elapsed_since_update = CONTEXT_VIEW_T_INTERVAL;
    mtt::unset_is_active(thing_context_view);
    
    handle_context_sensitive_command_clear(dt);
}



}
