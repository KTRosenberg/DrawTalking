//
//  speech_info_platform.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/14/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef speech_info_platform_hpp
#define speech_info_platform_hpp


#include "speech_info.hpp"
#include "speech_recognition_handler.hpp"
#include "speech_system_events.h"

void Speech_System_init(Speech_System* sys, void* backend);
bool Speech_split_platform(Speech_Info* speech_info);
bool Speech_discard_platform(Speech_Info* speech_info);
//void Speech_set_active_state_platform(Speech_Info* speech_info, bool state);
bool Speech_get_active_state_platform(Speech_Info* speech_info);
bool currently_talking_is_likely_platform(Speech_Info* speech_info);
//void Speech_reset_platform(Speech_Info* speech_info);
void Speech_send_override_platform(Speech_Info* speech_info, mtt::String& msg, int mode);


void Speech_set_ignore_state_platform(Speech_Info* speech_info, bool do_ignore);

void Speech_register_event_callback_platform(Speech_Info* speech_info, SPEECH_SYSTEM_EVENT event, void (*callback)(SPEECH_SYSTEM_EVENT event, Speech_System_Event_Status status, void* data), void* data);

void Speech_unregister_event_callback_platform(Speech_Info* speech_info, SPEECH_SYSTEM_EVENT event, void* data);

usize Speech_recognition_ID_platform(Speech_Info* speech_info);
usize Speech_recognition_modification_count_platform(Speech_Info* speech_info);

void Speech_process_text_as_input_platform(Speech_Info* speech_info, const mtt::String& text, uint64 flags);

void Speech_recognition_increment_ID_platform(Speech_Info* speech_info);

void Speech_recognition_set_ID_platform(Speech_Info* speech_info, usize new_id);

void Speech_recognition_reset_state_platform(Speech_Info* speech_info);

#if defined(__OBJC__)

#import <Foundation/Foundation.h>
#include "speech_recognition_objc.hpp"

NSString* preprocess_local_transcription(NSString* string);

static const float32 timer_waittime = 0.6f;

struct Speech_Input_Buffer;

struct Speech_Input_Buffer {
    NSString* message;
    NSString* message_plus_hypothesis;
    
    uint64 message_update_timestamp;
    uint64 message_plus_hypothesis_update_timestamp;
    
    NSTimer* timer;
    
    Speech_Recognition_ObjC* speech;
    
    NSString* compute_updated_hypothesis(NSString* input)
    {
        // NSLog(@"----HYPOTHESIS  %@", input);
        @autoreleasepool {
            
            [speech set_speech_state:SPEECH_STATE_TALKING_LIKELY];
            
            if (this->timer != nil && [this->timer isValid]) {
                [this->timer invalidate];
            }

            this->timer = [NSTimer scheduledTimerWithTimeInterval:timer_waittime
                                                           repeats:NO
                                                             block:^(NSTimer* timer) {
                @autoreleasepool {
                   
                    if ([this->timer isValid]) {
                        [this->timer invalidate];
                    }
    
                    
                    [speech set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];
                }

            }];
            
            if (this->message.length > 0) {
                this->message_plus_hypothesis = [this->message stringByAppendingString:[@" " stringByAppendingString:input]];
            } else {
                this->message_plus_hypothesis = input;
            }
            
            this->message_plus_hypothesis_update_timestamp = mtt_core_ctx()->time_nanoseconds;
            ASSERT_MSG(![this->message isEqualToString:@"."], "%s\n", "Something is wrong if the update is a single \".\"");
            return this->message_plus_hypothesis;
        }
    }
    
    NSString* update_message_with_result(NSString* input)
    {
        //NSLog(@"----RESULT  %@", input);
        @autoreleasepool {
            
            //[speech set_speech_state:SPEECH_STATE_TALKING_LIKELY];

//            if (this->timer != nil && [this->timer isValid]) {
//                [this->timer invalidate];
//            }
//            
//            this->timer = [NSTimer scheduledTimerWithTimeInterval:timer_waittime
//                                                           repeats:NO
//                                                             block:^(NSTimer* timer) {
//                @autoreleasepool {
//                   
//                    if ([this->timer isValid]) {
//                        [this->timer invalidate];
//                    }
//    
//                    
//                    [speech set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];
//                }
//
//            }];
            
            if (this->message.length > 0) {
                this->message = [this->message stringByAppendingString:[@" " stringByAppendingString:input]];
            } else {
                this->message = input;
            }
            
            this->message_update_timestamp = mtt_core_ctx()->time_nanoseconds;
            ASSERT_MSG(![this->message isEqualToString:@"."], "%s\n", "Something is wrong if the update is a single \".\"");
            return this->message;
        }
    }
    
    NSString* update_message_with_result_TMP(void)
    {
        @autoreleasepool {
            this->message = this->message_plus_hypothesis;
            
            return this->message;
        }
    }
    
    NSString* most_recent_message(void)
    {
        auto& ts_final = this->message_update_timestamp;
        auto& ts_hypo = this->message_plus_hypothesis_update_timestamp;
        auto& msg_final = this->message;
        auto& msg_hypo = this->message_plus_hypothesis;
        if (ts_final >= ts_hypo) {
            return msg_final;
        } else {
            return msg_hypo;
        }
//        return (this->message_update_timestamp > this->message_plus_hypothesis_update_timestamp) ? this->message : this->message_plus_hypothesis;
    }
    
    NSString* most_recent_message_TMP(void)
    {
        return nil;
    }
    
    cstring cstring_make(void)
    {
        return [this->message cStringUsingEncoding:NSUTF8StringEncoding];
    }
    
    void reset(void)
    {
        @autoreleasepool {
            this->message = @"";
            this->message_plus_hypothesis = @"";
            if (this->timer != nil && [this->timer isValid]) {
                [this->timer invalidate];
            }
        }
    }
};

static inline void Speech_Input_Buffer_init(Speech_Input_Buffer* sp_in, Speech_Recognition_ObjC* speech)
{
    sp_in->message = @"";
    sp_in->message_plus_hypothesis  = @"";
    sp_in->message_update_timestamp = 0;
    sp_in->message_plus_hypothesis_update_timestamp = 0;
    sp_in->timer = nil;
    sp_in->speech = speech;
}


#endif




#endif /* speech_info_platform_hpp */
