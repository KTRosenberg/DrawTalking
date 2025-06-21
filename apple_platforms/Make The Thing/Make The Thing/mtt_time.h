//
//  mtt_time.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/17/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef mtt_time_h
#define mtt_time_h

extern_link_begin()
typedef uint64 (*Time_MS_Proc)(void);

extern Time_MS_Proc time_millisecondsX;
extern_link_end()


#endif /* mtt_time_h */
