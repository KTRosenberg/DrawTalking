//
//  flecs_legacy.h
//  Make The Thing
//
//  Created by Karl Rosenberg on 9/4/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef flecs_legacy_h
#define flecs_legacy_h

inline static ecs_rule_t* mtt_ecs_rule_new(ecs_world_t* world, const char* expr)
{
    ecs_filter_desc_t desc = {};
    desc.expr = expr;
    return ecs_rule_init(world, &desc);
}

inline static void mtt_ecs_rule_free(
    ecs_rule_t* rule)
{
    ecs_rule_fini(rule);
}


#endif /* flecs_legacy_h */
