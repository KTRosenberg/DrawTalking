//
//  speech_timestamp.h
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/18/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef speech_timestamp_h
#define speech_timestamp_h

extern_link_begin()

typedef struct Speech_Timestamp {
    float64 time;
    int64   generation;
    float64 time_elapsed;
    float64 time_elapsed_initial_offset;
    float64 system_time_diff;
    float64 raw_time;
} Speech_Timestamp;

void Speech_Timestamp_init(Speech_Timestamp* ts);
int Speech_Timestamp_compare(Speech_Timestamp* a, Speech_Timestamp* b);

void Speech_Timestamp_print(Speech_Timestamp* ts);




extern_link_end()

#endif /* speech_timestamp_h */
