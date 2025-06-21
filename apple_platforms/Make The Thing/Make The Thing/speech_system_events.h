//
//  speech_system_events.h
//  Make The Thing
//
//  Created by Karl Rosenberg on 8/5/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef speech_system_events_h
#define speech_system_events_h

extern_link_begin()

typedef enum SPEECH_SYSTEM_EVENT {
    SPEECH_SYSTEM_EVENT_CONFIRM,
    SPEECH_SYSTEM_EVENT_DISCARD,
    SPEECH_SYSTEM_EVENT_DISABLE,
    SPEECH_SYSTEM_EVENT_ENABLE,
} SPEECH_SYSTEM_EVENT;
static const uint64 SPEECH_SYSTEM_EVENT_COUNT = 4;

typedef struct Speech_System_Event_Status {
    uint64 mode;
} Speech_System_Event_Status;

typedef enum SPEECH_STATE {
    SPEECH_STATE_TALKING_UNLIKELY,
    SPEECH_STATE_TALKING_LIKELY,
} SPEECH_STATE;
static const uint64 SPEECH_STATE_COUNT = 2;

extern_link_end()


#endif /* speech_system_events_h */
