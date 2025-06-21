//
//  time_common.h
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/19/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef time_common_h
#define time_common_h

extern_link_begin()



float64 system_timestamp_to_mtt_seconds(float64 system_timestamp, float64 mtt_time_seconds, float64 target_system_timestamp);

struct MTT_Core;
float64 system_timestamp_to_mtt_seconds_with_core(struct MTT_Core* core, float64 target_system_timestamp);

float64 mtt_time_seconds(void);

uint64 mtt_time_nanoseconds(void);
uint64 mtt_time_delta_nanoseconds(const uint64 t_start, const uint64 t_end);

float64 mtt_ns_to_ms(uint64 ns);
float64 mtt_ns_to_s(uint64 ns);
uint64 mtt_s_to_ns(float64 s);

typedef struct MTT_Date_Time {
    usize year;
    usize month;
    usize day;
    usize hour;
    usize minute;
    usize second;
} MTT_Date_Time;

void mtt_date_time_current(MTT_Date_Time* date_time_out);


extern_link_end()

#endif /* time_common_h */
