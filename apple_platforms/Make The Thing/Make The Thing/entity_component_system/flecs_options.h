//
//  flecs_options.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 8/18/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef flecs_options_h
#define flecs_options_h

#define FLECS_CUSTOM_BUILD
#define FLECS_CPP
#define FLECS_PARSER
#define FLECS_RULES
#define FLECS_OS_API_IMPL
#define MTT_FLECS_NEW

#ifdef MTT_FLECS_NEW
#define MTT_Q_VAR_NOT "!"
#define MTT_Q_VAR_PREFIX "$"
#define MTT_Q_VAR_PREFIX_CH '$'
#define MTT_Q_VAR_L(VAR_) "$" VAR_
#define MTT_Q_VAR_S(VAR_) std::string("$" + VAR_)
#else
#define MTT_Q_VAR_NOT "!"
#define MTT_Q_VAR_PREFIX "_"
#define MTT_Q_VAR_PREFIX_CH '_'
#define MTT_Q_VAR_L(VAR_) "_" VAR_
#define MTT_Q_VAR_S(VAR_) std::string("_" + VAR_)
#endif


#endif /* flecs_options_h */
