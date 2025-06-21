//
//  mtt_core_platform.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/17/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "mtt_core_platform.h"

uint64 time_milliseconds(void)
{
    uint64 milliseconds = 0;
#if (defined(__MACH__) && defined(__APPLE__))
      struct mach_timebase_info convfact;
      mach_timebase_info(&convfact); // get ticks->nanoseconds conversion factor
      // get time in nanoseconds since computer was booted
      // the measurement is different per core
      uint64_t tick = mach_absolute_time();
      milliseconds = (tick * convfact.numer) / (convfact.denom * 1000000);
#endif
    return milliseconds;
}
