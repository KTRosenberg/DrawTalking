//
//  standard_actions.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/17/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef standard_actions_hpp
#define standard_actions_hpp

#include "drawtalk.hpp"
#include "drawtalk_behavior.hpp"

namespace dt {

void load_standard_actions(dt::DrawTalk* dt);

void create_new_action(mtt::Script* script, const mtt::String& label);

}


#endif /* standard_actions_hpp */
