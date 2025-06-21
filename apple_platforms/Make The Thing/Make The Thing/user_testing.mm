//
//  user_testing.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 8/22/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#include "user_testing.h"

#include "mtt_core_platform.h"

#if MTT_USER_TEST_LOG_ENABLE

#define MTT_USER_TEST_LOG_USE_DISPATCH (0)

constexpr const usize max_pending_jobs = 16;
struct Log_State {
    mtt::String to_write_out = {};
    std::deque<mtt::String> to_write = {};
    usize to_write_count = 0;
    mtt::String path = {};
#if MTT_USER_TEST_LOG_USE_DISPATCH
    dispatch_semaphore_t sem = dispatch_semaphore_create(16);
#endif
} state = {};

void Log_State_reset(Log_State* state)
{
    state->to_write_out.clear();
    state->to_write.clear();
    state->to_write_count = 0;
}



bool MTT_user_test_log_init(const mtt::String& path)
{
    MTT_Core_Platform* core_platform = mtt_core_platform_ctx();
    core_platform->core.test_log_file = nullptr;
    core_platform->core.test_log_file = fopen(path.c_str(), "a+");
    state.path = path;
    return (core_platform->core.test_log_file != nullptr);
}

void MTT_user_test_log_enqueue(const MTT_USER_TEST_LOG_LABEL label, const mtt::String& msg)
{
    if (!MTT_USER_TEST_LOG_LABEL_IS_ENABLED[label]) {
        return;
    }
    //MTT_Core_Platform* core_platform = mtt_core_platform_ctx();
    
    state.to_write.push_back(msg);
    state.to_write_count += msg.size();
    
//    dispatch_async(core_platform->bg_logging_queue, ^{
//        cstring msg_c_str = msg_for_block.c_str();
//        fwrite(msg_c_str, sizeof(char), msg_for_block.size(), file);
//    });
}
void MTT_user_test_log_enqueue(const MTT_USER_TEST_LOG_LABEL label, cstring msg, usize len)
{
    if (!MTT_USER_TEST_LOG_LABEL_IS_ENABLED[label]) {
        return;
    }
    //MTT_Core_Platform* core_platform = mtt_core_platform_ctx();
    
//    __block FILE* file = core_platform->core.test_log_file;
//    __block usize len_for_block = len;
    
    state.to_write.push_back(mtt::String(msg));
    state.to_write_count += len;
    
//    dispatch_async(core_platform->bg_logging_queue, ^{
//        fwrite(msg, sizeof(char), len_for_block, file);
//    });
}

void MTT_user_test_log_write_enqueued(void)
{
    if (state.to_write_count == 0) {
        return;
    }
    
    MTT_Core_Platform* core_platform = mtt_core_platform_ctx();
    __block FILE* file = core_platform->core.test_log_file;
    
    state.to_write_out.reserve(state.to_write_count);
    
    for (auto it = state.to_write.begin(); it != state.to_write.end(); ++it) {
        state.to_write_out += *it;
    }
    
    fwrite(state.to_write_out.c_str(), sizeof(char), state.to_write_out.size(), file);
    
    Log_State_reset(&state);
//
//    usize cap = (to_write.size() < max_pending_jobs) ? to_write.size() : max_pending_jobs;
//    for (; cap != 0;) {
//        __block mtt::String msg = to_write.front();
//        to_write.pop_front();
//        
//        dispatch_async(core_platform->bg_logging_queue, ^{
//            fwrite(msg.c_str(), sizeof(char), msg.size(), file);
//        });
//        cap -= 1;
//    }
}

void MTT_user_test_log_flush(void)
{
    MTT_Core_Platform* core_platform = mtt_core_platform_ctx();
    
    __block FILE* file = core_platform->core.test_log_file;
    
    //dispatch_async(core_platform->bg_logging_queue, ^{
    if (fflush(file) != 0) {
        MTT_user_test_log_init(state.path);
    }
    //});
}

void MTT_user_test_log_deinit(void)
{
    MTT_Core_Platform* core_platform = mtt_core_platform_ctx();
    
    __block FILE* file = core_platform->core.test_log_file;
    if (file != nullptr) {
        MTT_user_test_log_write_enqueued();
        MTT_user_test_log_flush();
        //dispatch_sync(core_platform->bg_logging_queue, ^{
            fclose(file);
        //});
        core_platform->core.test_log_file = nullptr;
    }
}

#else

bool MTT_user_test_log_init(const mtt::String&)
{
    return true;
}

void MTT_user_test_log(mtt::String&)
{
    
}
void MTT_user_test_log(cstring msg, usize len)
{
    
}

void MTT_user_test_log_write_enqueued(void)
{
    
}

void MTT_user_test_log_flush(void)
{
    
}
void MTT_user_test_log_deinit(void)
{
    
}

#endif
