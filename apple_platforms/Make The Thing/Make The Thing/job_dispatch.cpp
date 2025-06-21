//
//  job_dispatch.cpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 7/19/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "job_dispatch.hpp"
#include "job_dispatch_platform.hpp"

namespace mtt {


void job_dispatch_serial(void* ctx, void (*procedure)(void*), void (*callback)(void*))
{
    job_dispatch_serial_platform(ctx, procedure, callback);
}

void job_dispatch_serial_sync(void* ctx, void (*procedure)(void*))
{
    job_dispatch_serial_sync_platform(ctx, procedure);
}

void job_dispatch_to_main(void* ctx, void (*procedure)(void*))
{
    job_dispatch_to_main_platform(ctx, procedure);
}


bool is_main_thread(void)
{
    return is_main_thread_platform();
}

}
