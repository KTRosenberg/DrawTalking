//
//  configuration.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/25/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef configuration_hpp
#define configuration_hpp

#include "color_schemes.hpp"
#include "drawtalk_ui.hpp"
#include "misc.hpp"

namespace mtt {

struct Application_Configuration {
    vec4 text_panel_color             = dt::TEXT_PANEL_DEFAULT_COLOR;
    mtt::Color_Scheme_ID color_scheme = mtt::COLOR_SCHEME_LIGHT_MODE;
    bool follow_system_window = false;
    void* custom_data = nullptr;
    bool enable_speech = true;
    bool text_selection_is_toggle = true;
    
    Net_Connection_Info host = {};
    
    mtt::String system_directory = "";
};

void Application_Configuration_init(Application_Configuration* config, void* result);
void Application_Configuration_init_from_string(Application_Configuration* config, void* str);
void Application_Configuration_reapply(Application_Configuration* config, void* data);


}

#endif /* configuration_hpp */
