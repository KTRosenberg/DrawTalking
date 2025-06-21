//
//  drawtalk_world_interface.hpp
//  dynamic_dev
//
//  Created by Toby Rosenberg on 4/13/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_world_interface_hpp
#define drawtalk_world_interface_hpp

struct alignas(16) DrawTalk_World;


struct Render_Info;
struct Input_Info;

namespace dt {
struct DrawTalk;
struct Control_System;

}

namespace mtt {
struct World;
struct External_World;
struct Camera;
}


dt::DrawTalk* DrawTalk_from(DrawTalk_World* ctx);
dt::Control_System* Control_System_from(DrawTalk_World* ctx);
mtt::World* World_from(DrawTalk_World* ctx);
mtt::External_World* External_World_from(DrawTalk_World* ctx);
mtt::Camera* Camera_from(DrawTalk_World* ctx);
mat4 main_projection_from(DrawTalk_World* ctx);
sd::Viewport* scaled_viewport_from(DrawTalk_World* ctx);


#endif /* drawtalk_world_interface_hpp */
