//
//  erase_tool.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 1/17/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef erase_tool_hpp
#define erase_tool_hpp

namespace mtt {

struct Erase_Tool_Args {
    sd::Renderer* renderer;
};

struct Intersection_Result {
    bool something_erased;
    bool will_be_deleted;
};

Intersection_Result erase_tool_began(Input_Record* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags, void* info);
Intersection_Result erase_tool_moved(Input_Record* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags, void* info);
Intersection_Result erase_tool_cancelled(Input_Record* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags, void* info);
Intersection_Result erase_tool_ended(Input_Record* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags, void* info);

}

#endif /* erase_tool_hpp */
