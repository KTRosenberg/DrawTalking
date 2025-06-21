//
//  speech_timestamp.c
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/18/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "speech_timestamp.h"

extern_link_begin()

void Speech_Timestamp_init(Speech_Timestamp* ts)
{
    ts->time = 0.0;
    ts->generation = 0;
    ts->time_elapsed = 0.0;
    ts->time_elapsed_initial_offset = 0.0;
    ts->system_time_diff = 0.0;
}

int Speech_Timestamp_compare(Speech_Timestamp* a, Speech_Timestamp* b)
{
    long long generation_comp = a->generation - b->generation;
    if (generation_comp == 0) {
        float64 time_comp = a->time - b->time;
        if (time_comp == 0.0) {
            return 0;
        } else if (time_comp < 0.0) {
            return -1;
        } else /* > 0.0 */ {
            return 1;
        }
    } else if (generation_comp < 0) {
        return -1;
    } else /* > 0 */ {
        return 1;
    }
}

void Speech_Timestamp_print(Speech_Timestamp* ts)
{
    MTT_print("sp_ts:[gen=%llu, t=%f t_elapsed=%f  t_elapsed_init_off=%f t_sys_diff=%f\n",
              ts->generation, ts->time, ts->time_elapsed, ts->time_elapsed_initial_offset, ts->system_time_diff);
}

extern_link_end()

