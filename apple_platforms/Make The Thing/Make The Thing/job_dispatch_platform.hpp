//
//  job_dispatch_platform.hpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 7/19/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef job_dispatch_platform_hpp
#define job_dispatch_platform_hpp

#include "job_dispatch_platform.hpp"

namespace mtt {

void job_dispatch_serial_platform(void* ctx, void (*procedure)(void*), void (*callback)(void*));

void job_dispatch_serial_sync_platform(void* ctx, void (*procedure)(void*));

void job_dispatch_to_main_platform(void* ctx, void (*procedure)(void*));


bool is_main_thread_platform(void);

}


#endif /* job_dispatch_platform_hpp */
