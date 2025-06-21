//
//  mtt_core_platform.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/17/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef mtt_core_platform_h
#define mtt_core_platform_h

#include "make_the_thing.h"
#if (defined(__MACH__) && defined(__APPLE__))
#include <mach/mach_time.h>
#endif

#include "input.hpp"
#include "stratadraw_platform_apple.hpp"

#import "TCP_Client.h"
#import "extension_server.h"

#include "IP_Address.h"

#include <TargetConditionals.h>

#include "Make_The_Thing-Swift.h"

extern_link_begin()

typedef struct MTT_Core_Platform {
    MTT_Core           core;
    SpeechRecognition* speech;
    sd::Renderer       renderer;
    TCP_Client*        net_client;
    TCP_Client*        net_client_msgs;
    TCP_Client*        vis_client;
    Extension_Server*  server;
    
    MTTViewController* vc;
    
    dispatch_queue_t bg_queue;
    dispatch_queue_t bg_text_io_queue;
    dispatch_queue_t bg_image_io_queue;
    //dispatch_queue_t bg_vis_queue;
    dispatch_queue_t bg_logging_queue;
    
    
} MTT_Core_Platform;

void MTT_core__platform_init(MTT_Core_Platform* core_platform);

void mtt_time_init(void);
void mtt_time_deinit(void);



void* default_heap_allocate(void* data, usize byte_count);

void default_heap_deallocate(void* data, void* memory, usize count);

//void default_heap_deallocate_all(void* data, usize count);

void* default_heap_resize(void* data, void* memory, usize byte_count, usize old_byte_count);



void* default_unaligned_heap_allocate(void* data, usize byte_count);

void default_unaligned_heap_deallocate(void* data, void* memory, usize count);

//void default_unaligned_heap_deallocate_all(void* data, usize count);

void* default_unaligned_heap_resize(void* data, void* memory, usize byte_count, usize old_byte_count);

typedef struct MTT_Main_Config {
    char*              ip_address_host;
    unsigned long long port_host;
} MTT_Main_Config;

inline static usize get_page_size(void)
{
    return getpagesize();
}

MTT_Core_Platform* mtt_core_platform_ctx(void);





extern_link_end()




void MTT_Core_Platform_init(MTT_Core_Platform* core_platform, mem::Allocator* allocator);

#endif /* mtt_core_platform_h */

#ifdef MTT_CORE_PLATFORM_IMPLEMENTATION
#undef MTT_CORE_PLATFORM_IMPLEMENTATION

MTT_Core_Platform* global_core_platform;

void MTT_Core_Platform_init(MTT_Core_Platform* core_platform, mem::Allocator* allocator)
{
    MTT_print("%s\n", "initializing core platform");
    

    MTT_Core_init(&core_platform->core, (void*)core_platform, allocator);
    Input_init(&core_platform->core.input);
    core_platform->core.renderer = &core_platform->renderer;
    
    MTT_Random_init(time(NULL));
    
    global_core_platform = core_platform;
}

MTT_Core_Platform* mtt_core_platform_ctx(void)
{
    return global_core_platform;
}

extern_link_begin()

static bool time_started = false;
extern uint64 mtt_t_start;

void mtt_time_init(void)
{
    time_started = true;
    mtt_t_start = mtt_time_nanoseconds();
}

void mtt_time_deinit(void)
{
    time_started = false;
}

static inline uint64 mtt_time_get_start_tick(void)
{
    return mtt_t_start;
}


float64 mtt_time_seconds(void)
{
    return mtt_time_nanoseconds() / ((float64)1e+9);
}

float64 mtt_ns_to_ms(uint64 ns)
{
    return ((float64)ns) / ((float64)1e+6);
}

float64 mtt_ns_to_s(uint64 ns)
{
    return ((float64)ns) / ((float64)1e+9);
}

uint64 mtt_s_to_ns(float64 s)
{
    return s * ((float64)1e+9);
}

uint64 mtt_time_nanoseconds(void)
{
    return clock_gettime_nsec_np(CLOCK_UPTIME_RAW) - mtt_time_get_start_tick();
}

uint64 mtt_time_delta_nanoseconds(const uint64 t_start, const uint64 t_end)
{
    uint64 elapsed = (t_end - t_start);
    return elapsed;
}

#define USE_ALIGNED_MEMORY_ALLOCATION
void* default_heap_allocate(void* data, usize byte_count)
{
#ifdef USE_ALIGNED_MEMORY_ALLOCATION
    return mem::aligned_alloc(16, align_up(byte_count, 16));
#else
    return mem::malloc(byte_count);
#endif
}

void default_heap_deallocate(void* data, void* memory, usize count)
{
#ifdef USE_ALIGNED_MEMORY_ALLOCATION
    return mem::aligned_free(memory);
#else
    return mem::free(memory);
#endif
}

//void default_heap_deallocate_all(void* data, usize count)
//{
//    return free(memory);
//}

void* default_heap_resize(void* data, void* memory, usize byte_count, usize old_byte_count)
{
#ifdef USE_ALIGNED_MEMORY_ALLOCATION
    return mem::aligned_realloc(data, byte_count, old_byte_count);
#else
    return mem::realloc(data, byte_count, old_byte_count);
#endif
}


void* default_unaligned_heap_allocate(void* data, usize byte_count)
{
    return mem::malloc(byte_count);
}

void default_unaligned_heap_deallocate(void* data, void* memory, usize count)
{
    return mem::free(memory);
}


void* default_unaligned_heap_resize(void* data, void* memory, usize byte_count, usize old_byte_count)
{
    return mem::realloc(data, byte_count, old_byte_count);
}

//NSString* device_name(void)
//{
//    return [[UIDevice currentDevice] name];
//}

void MTT_reset_defaults_platform(void);

void MTT_reset_defaults(void)
{
    MTT_reset_defaults_platform();
}

void mtt_date_time_current(MTT_Date_Time* date_time_out)
{
    @autoreleasepool {
        NSDate* current_date_time = [NSDate date];
        NSCalendar* calendar = [NSCalendar currentCalendar];
        NSDateComponents* components = [calendar components:(NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute | NSCalendarUnitSecond) fromDate:current_date_time];
        
        date_time_out->year = [components year];
        date_time_out->month = [components month];
        date_time_out->day = [components day];
        date_time_out->hour = [components hour];
        date_time_out->minute = [components minute];
        date_time_out->second = [components second];
    }
    
//let currentDateTime = Date()
//
//// get the user's calendar
//let userCalendar = Calendar.current
//
//// choose which date and time components are needed
//let requestedComponents: Set<Calendar.Component> = [
//    .year,
//    .month,
//    .day,
//    .hour,
//    .minute,
//    .second
//]
//
//// get the components
//let dateTimeComponents = userCalendar.dateComponents(requestedComponents, from: currentDateTime)

    
//    dateTimeComponents.year
//    dateTimeComponents.month
//    dateTimeComponents.day
//    dateTimeComponents.hour
//    dateTimeComponents.minute
//    dateTimeComponents.second
}

extern_link_end()

#endif
