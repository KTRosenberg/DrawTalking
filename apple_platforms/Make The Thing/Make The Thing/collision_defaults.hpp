//
//  collision_defaults.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/28/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef collision_defaults_hpp
#define collision_defaults_hpp

#include "collision.hpp"

namespace mtt {

// MARK: default handlers
Collision_Handler_Result collision_handler_AABB_default(Broad_Phase_Collision_Record* collisions, Collision_System* system);



Collision_Handler_Result collision_handler_AABB_default_pair(Broad_Phase_Pair* col_cand, Collision_System* system);


Collision_Handler_Result collision_handler_Line_Segment_default(Broad_Phase_Collision_Record* collisions, Collision_System* system);

Collision_Handler_Result collision_handler_Point_default(Broad_Phase_Collision_Record* collisions, Collision_System* system);

Collision_Handler_Result collision_handler_Concave_Hull_default(Broad_Phase_Collision_Record* collisions, Collision_System* system);

Collision_Handler_Result collision_handler_Circle_default(Broad_Phase_Collision_Record* collisions, Collision_System* system);

}

#endif /* collision_defaults_hpp */
