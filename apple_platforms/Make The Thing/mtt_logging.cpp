//
//  mtt_logging.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/7/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#include "mtt_logging.h"



#ifndef NDEBUG

#ifdef __cplusplus
extern "C" {
#endif

bool debug_mode_log_is_enabled_ = true;
void debug_mode_log_enable(bool state)
{
    debug_mode_log_is_enabled_ = state;
}

bool debug_mode_log_is_enabled(void)
{
    return debug_mode_log_is_enabled_;
}

#ifdef __cplusplus
}
#endif


MTT_Logger default_active_logger_ = {.logger = os_log_create("mtt.main", "mtt")
, .buf = new mtt::String() };
#else

#ifdef __cplusplus
extern "C" {
#endif

void debug_mode_log_enable(bool state)
{
    (void)state;
}

bool debug_mode_log_is_enabled(void)
{
    return false;
}

#ifdef __cplusplus
}
#endif

MTT_Logger default_active_logger_ = {.logger = OS_LOG_DEFAULT
, .buf = new mtt::String() };
#endif
MTT_Logger active_logger_ = default_active_logger_;

#ifdef __cplusplus
extern "C" {
#endif

void MTT_set_active_logger(MTT_Logger* logger)
{
    active_logger_ = *logger;
}

struct MTT_Logger* MTT_active_logger(void)
{
    return &active_logger_;
}

void MTT_set_default_active_logger(void)
{
    active_logger_ = default_active_logger_;
}

void MTT_Logger_init(struct MTT_Logger* logger_out, cstring subsystem, cstring category)
{
    *logger_out = (MTT_Logger){ .logger = os_log_create(subsystem, category), .buf = new mtt::String() };
}

void MTT_Logger_destroy(struct MTT_Logger* logger)
{
    // NO-OP
}

#ifdef __cplusplus
}
#endif

#ifndef NDEBUG


#else

#endif
