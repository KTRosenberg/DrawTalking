//
//  configuration.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/25/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#include "configuration.hpp"
#include "file_system.hpp"



namespace mtt {

auto handle_color = [](JsonArrayConst& el, vec4* dst, vec4 default_value) -> bool {
    if (el.isNull()) {
        *dst = default_value;
        return false;
    }
    vec4 color = {};
    for (isize32 i = 0; auto c_vals : el) {
        color[i] = c_vals.as<float32>();
        i += 1;
    }
    *dst = color;
    
    return true;
};

void Application_Configuration_init_from_string(Application_Configuration* config, void* str)
{
    mtt::String& string_in = *((mtt::String*)str);
    
    usize size = 4096 * 2;
    ArduinoJson::DynamicJsonDocument doc(size);
    DeserializationError error = deserializeJson(doc, string_in);
    while (error) {
        if (strcmp(error.c_str(), "NoMemory") != 0) {
            MTT_error("%s", "JSON deserialization memory issue\n");
            break;
        } else {
            doc.clear();
        }
        
        size = size * 2;
        doc = ArduinoJson::DynamicJsonDocument(size);
        error = deserializeJson(doc, string_in);
    }
    if (doc.containsKey("text_panel_color")) {
        auto text_color_el = doc["text_panel_color"];
        auto text_color_arr = text_color_el.as<JsonArrayConst>();
        handle_color(text_color_arr, &config->text_panel_color, dt::TEXT_PANEL_DEFAULT_COLOR);
    }
    if (doc.containsKey("color_scheme")) {
        auto scheme = doc["color_scheme"];
        if (scheme.is<JsonInteger>()) {
            //mtt::set_color_scheme((mtt::Color_Scheme_ID)scheme.as<JsonInteger>());
            config->color_scheme = (mtt::Color_Scheme_ID)scheme.as<JsonInteger>();
            //sd::set_background_color_rgba_v4(core->renderer, dt::bg_color_default());
        } else if (scheme.is<JsonUInt>()) {
            //mtt::set_color_scheme((mtt::Color_Scheme_ID)scheme.as<JsonUInt>());
            //sd::set_background_color_rgba_v4(core->renderer, dt::bg_color_default());
            config->color_scheme = (mtt::Color_Scheme_ID)scheme.as<JsonInteger>();
        } else if (scheme.is<JsonString>()) {
            mtt::String str = scheme;
            str = mtt::str_to_upper(str);
            const mtt::Color_Scheme_ID* color_scheme_id_ptr = nullptr;
            if (mtt::map_try_get(&color_scheme_strings_to_id, str, &color_scheme_id_ptr)) {
                config->color_scheme = *color_scheme_id_ptr;
            }
        }
    }
    if (doc.containsKey("rc")) {
        auto remote_cmd = doc["rc"];
        if (remote_cmd.is<JsonString>()) {
            text_in_buf = mtt::String(remote_cmd);
            auto* core = (MTT_Core*)config->custom_data;
            auto* input = &core->input;
            Input_Record* input_record = &input->users[0];
            
            {
                UI_Event key_event = {};
                
                key_event.input_type = UI_TOUCH_TYPE_KEY;
                key_event.state_type = UI_TOUCH_STATE_KEY_BEGAN;
                key_event.key        = UI_KEY_TYPE_RETURN_OR_ENTER;
                key_event.key_sub    = UI_KEY_MODIFIER_FLAG_SHIFT;
                key_event.timestamp  = 0;
                set_key_is_pressed(input_record, (UI_KEY_TYPE)key_event.key);
                key_event.count = 0;
                key_event.flags = UI_EVENT_FLAG_OVERRIDE;
                
                Input_push_event(input_record, key_event);
            }
            {
                UI_Event key_event = {};
                
                key_event.input_type = UI_TOUCH_TYPE_KEY;
                key_event.state_type = UI_TOUCH_STATE_KEY_ENDED;
                key_event.key        = UI_KEY_TYPE_RETURN_OR_ENTER;
                key_event.key_sub    = 0;
                key_event.timestamp  = 0;
                unset_key_is_pressed(input_record, (UI_KEY_TYPE)key_event.key);
                key_event.count = 0;
                key_event.flags = UI_EVENT_FLAG_OVERRIDE;
                
                Input_push_event(input_record, key_event);
            }
        }
    }
    if (doc.containsKey("text_selection_is_toggle")) {
        auto scheme = doc["text_selection_is_toggle"];
        if (scheme.is<JsonInteger>()) {
            config->text_selection_is_toggle = ((scheme.as<JsonInteger>() != 0) ? true : false);
        } else if (scheme.is<bool>()) {
            config->text_selection_is_toggle = scheme.as<bool>();
        }
    }
    doc.clear();
}



void Application_Configuration_init(Application_Configuration* config, void* result_raw)
{
    auto& result = *static_cast<mtt::Text_File_Load_Result*>(result_raw);
    
    for (usize i = 0; i < result.text.size(); i += 1) {
        Application_Configuration_init_from_string(config, (void*)(&result.text[i]));
    }
    return;
}


}


