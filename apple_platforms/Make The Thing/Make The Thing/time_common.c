//
//  time_common.c
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/19/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "time_common.h"

//#include "make_the_thing.h"

extern_link_begin()

uint64 mtt_t_start;

float64 system_timestamp_to_mtt_seconds(float64 system_timestamp, float64 mtt_time_seconds, float64 target_system_timestamp)
{
    return mtt_time_seconds - (system_timestamp - target_system_timestamp);
}

extern_link_end()

