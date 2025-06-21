//
//  user_testing.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 8/22/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef user_testing_h
#define user_testing_h

#define MTT_USER_TEST_LOG_ENABLE (1)

enum MTT_USER_TEST_LOG_LABEL : uint64 {
    MTT_USER_TEST_LOG_LABEL_INPUT_ALWAYS = 0,
    MTT_USER_TEST_LOG_LABEL_INPUT = 1,
    MTT_USER_TEST_LOG_LABEL_INPUT_MOVE = 2,
};

static constexpr const bool MTT_USER_TEST_LOG_LABEL_IS_ENABLED[] = {
    [MTT_USER_TEST_LOG_LABEL_INPUT_ALWAYS] = false,
    [MTT_USER_TEST_LOG_LABEL_INPUT] = false,
    [MTT_USER_TEST_LOG_LABEL_INPUT_MOVE] = false,
};

bool MTT_user_test_log_init(const mtt::String& path);
void MTT_user_test_log_enqueue(const MTT_USER_TEST_LOG_LABEL label, const mtt::String& msg);
void MTT_user_test_log_enqueue(const MTT_USER_TEST_LOG_LABEL label, cstring msg, usize len);
void MTT_user_test_log_write_enqueued(void);
void MTT_user_test_log_flush(void);
void MTT_user_test_log_deinit(void);


#endif /* user_testing_h */
