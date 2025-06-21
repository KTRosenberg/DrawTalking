//
//  job_dispatch.m
//  Make The Thing
//
//  Created by Karl Rosenberg on 7/19/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "job_dispatch_platform.hpp"
#include "mtt_core_platform.h"

namespace mtt {

#define MTT_ENABLE_JOBS (1)

void job_dispatch_serial_platform(void* ctx, void (*procedure)(void*), void (*callback)(void*))
{
#if MTT_ENABLE_JOBS
    __block void* ctx_for_block = ctx;
    dispatch_async(mtt_core_platform_ctx()->bg_queue, ^{
       procedure(ctx_for_block);
   
        dispatch_sync(dispatch_get_main_queue(), ^{
            callback(ctx_for_block);
        });
   });
#else
    procedure(ctx);
    callback(ctx);
#endif
}

void job_dispatch_serial_sync_platform(void* ctx, void (*procedure)(void*))
{
#if MTT_ENABLE_JOBS
    __block void* ctx_for_block = ctx;
    dispatch_sync(mtt_core_platform_ctx()->bg_queue, ^{
        procedure(ctx_for_block);
    });
#else
    procedure(ctx);
#endif
}

void job_dispatch_to_main_platform(void* ctx, void (*procedure)(void*))
{
#if MTT_ENABLE_JOBS
    __block void* ctx_for_block = ctx;
    dispatch_async(dispatch_get_main_queue(), ^{
        procedure(ctx_for_block);
    });
#else
    procedure(ctx);
#endif
}


bool is_main_thread_platform(void)
{
    return [NSThread isMainThread];
}

#undef MTT_ENABLE_JOBS

}
