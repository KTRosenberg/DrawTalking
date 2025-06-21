//
//  speech_recognition_objc.m
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/28/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "speech_recognition_objc.hpp"
#import "Make_The_Thing-Swift.h"
#include "speech_info_platform.hpp"
#include "mtt_core_platform.h"
#include "speech_timestamp.h"

#include "drawtalk.hpp"

#define DT__CONSIDER_DEFERRING_IF_TALKING_UNLIKELY (false)

#define set_most_recent_transcript_count(count) { MTT_print("{\nTranscript count old=[%llu], new=[%llu]\n}\n", self->most_recent_transcript_count, count); self->most_recent_transcript_count = count; }

cstring state_strings[] = {
    [SFSpeechRecognitionTaskStateStarting]  = "SpeechTaskStateStarting",
    [SFSpeechRecognitionTaskStateRunning]   = "SpeechTaskStateRunning",
    [SFSpeechRecognitionTaskStateCanceling] = "SpeechTaskStateCanceling",
    [SFSpeechRecognitionTaskStateFinishing] = "SpeechTaskStateFinishing",
    [SFSpeechRecognitionTaskStateCompleted] = "SpeechTaskStateCompleted",
};

#define DT_TASK_COUNT_CHECK (1)

void update_words_and_timesteps(NSArray* segments, usize offset)
{
    
}

 
cstring task_state_to_string(SFSpeechRecognitionTaskState state)
{
    return state_strings[state];
}



@interface Speech_Recognition_ObjC ()
{
    @public mtt::Set<Speech_Recognition_Handler*> subscribers;
    
    @public NSArray* transcriptWords;
    
    @public Speech_Input_Buffer speech_input;
    
    @public mtt::Map<void*, void(*)(SPEECH_SYSTEM_EVENT, Speech_System_Event_Status, void*)> event_callbacks[SPEECH_SYSTEM_EVENT_COUNT];
    
    
    //@public void (*on_amplitude_change)(float);
    @public TASK_COMPLETION_REASON task_completion_status;
    @public std::vector<Recognition_Info> discarded;
    @public std::deque<TASK_COMPLETION_REASON> queue_task_creation;
}

@end

@implementation Speech_Recognition_ObjC

- (instancetype)init {
    self = [super init];
    
    [self setSpeech_queue:[[NSOperationQueue alloc] init]];
    [[self speech_queue] setQualityOfService:NSQualityOfServiceUserInteractive];
    
    self->_recognitionID = 1;
    self->_modificationCount = 1;
//    self->_recognitionIDDiscarded = 0;
//    self->_modificationCountDiscarded = 0;
    //self->_isPlaying = NO;
    
    self->speech_state = SPEECH_STATE_TALKING_UNLIKELY;
    self->amplitude = 0.0;
    //self->on_amplitude_change = nil;
    
    for (usize i = 0; i < SPEECH_SYSTEM_EVENT_COUNT; i += 1) {
        self->event_callbacks[i] = mtt::Map<void*, void(*)(SPEECH_SYSTEM_EVENT, Speech_System_Event_Status, void*)>();
    }
    
    self->_reasonCount = 0;
    self->_reason = @"";
    
    Speech_Input_Buffer_init(&self->speech_input, self);
    
    self->_message_update = @"";
    
    self->task_completion_status = TASK_COMPLETION_REASON_NONE;
    
    self->discarded = {};
    self->discarded.reserve(16);
    
    self->queue_task_creation = {};
    
    self->_is_active = false;
    
    return self;
}

-(SPEECH_STATE) get_speech_state {
    return self->speech_state;
}

-(void) set_speech_state:(SPEECH_STATE) state {
    self->speech_state = state;
}

-(void) set_amplitude:(float) amplitude {
    self->amplitude = amplitude;
    for (auto sub : subscribers) {
        if (sub->handle_amplitude_change) {
            sub->handle_amplitude_change(sub, self->amplitude);
        }
    }
    
    //self->on_amplitude_change(self->amplitude);
}

//-(void) set_on_amplitude_change_callback:(void(*)(float))callback {
//    self->on_amplitude_change = callback;
//}



-(void) addSubscriber:(Speech_Recognition_Handler*)handler {
    subscribers.insert(handler);
}
-(void) removeSubscriber:(Speech_Recognition_Handler*)handler {
    subscribers.erase(handler);
}



-(void) speechRecognitionTask:(SFSpeechRecognitionTask *)task didHypothesizeTranscription:(SFTranscription*)transcription {
    
    MTT_print("SPEECH_TASK: %s\n", task_state_to_string(task.state));
    
    auto completion_status = [self completionReason];
    //ASSERT_MSG(count == 1, "%s\n", "map should have only one thing");
    @autoreleasepool {
        {
            auto& text_panel = dt::DrawTalk::ctx()->ui.margin_panels[0].text;
            if (text_panel.is_overriding_speech()) {
                [self set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];
                return;
            }
        }
        if (completion_status == TASK_COMPLETION_REASON_DISCARD || !self->_is_active) {
            [self set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];
            self->_message_update = @"";
            self->speech_input.reset();
//            [self removeTaskFromMap:task];
            return;
        } else if (completion_status == TASK_COMPLETION_REASON_COMMAND_COMPLETE) {
            return;
        }
        
        NSString* hypo = self->speech_input.compute_updated_hypothesis(self->_message_update);
        MTT_NSLog(@"HYPOTHESIS: [%@] id=[%llu] mod_count=[%llu]\n", hypo, self->_recognitionID, self->_modificationCount);
        
        if (hypo.length == 0) {
            return;
        }
        
        const usize ID                 = self->_recognitionID;
        const usize modification_count = self->_modificationCount;
        void* data = (__bridge void*)hypo;
//        NSMutableString* msg_out = nil;
//        {
//            auto& text_out = dt::ctx()->ui.margin_panels[0].text.text;
//            usize reserved_size = 0;
//            for (usize i = 0; i < text_out.size(); i += 1) {
//                reserved_size += 1 + text_out[i].text.size();
//            }
//            msg_out = [[NSMutableString alloc] initWithCapacity:reserved_size];
//            usize word_count = 0;
//            {
//                usize i = 0;
//                for (;i < text_out.size(); i += 1) {
//                    auto& text = text_out[i];
//                    if (!text.is_selected) {
//                        continue;
//                    }
//                    word_count += 1;
//                    cstring str = text_out[i].text.c_str();
//                    [msg_out appendFormat:@"%s ", str];
//                }
//            }
//            
//            if (word_count != 0) {
//                [msg_out deleteCharactersInRange:NSMakeRange([msg_out length]-1, 1)];
//                
//                data = (__bridge void*)((NSString*)msg_out);
//            }
//        }
        
        [self set_speech_state:SPEECH_STATE_TALKING_LIKELY];
        
        for (auto i = subscribers.begin(); i != subscribers.end(); ++i) {
            Speech_Recognition_Handler* handler = (*i);
            bool _ = handler->handle_hypothesis(handler, data, ID, modification_count, mtt_time_seconds(),
                                       0, {.args = (__bridge void*)self }
                                       );
        }
        data = nullptr;
        
    }
}

-(void) speechRecognitionTask:(SFSpeechRecognitionTask *)task didFinishRecognition:(SFSpeechRecognitionResult *)recognitionResult {
    
    auto completion_status = [self completionReason];
    
    MTT_print("SPEECH_TASK: %p, %s, reason: %s \n", (__bridge void*)task, task_state_to_string(task.state), (task_completion_reason_str(completion_status)));
    
    //ASSERT_MSG(count == 1, "%s\n", "map should have only one thing");
    if (completion_status != TASK_COMPLETION_REASON_INCOMPLETE) {
        
    }
    
    @autoreleasepool {
        {
            auto& text_panel = dt::DrawTalk::ctx()->ui.margin_panels[0].text;
            if (text_panel.is_overriding_speech()) {
                [self set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];
                return;
            }
        }
        if (completion_status == TASK_COMPLETION_REASON_DISCARD) {
            [self set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];
            self->_message_update = @"";
            self->speech_input.reset();
            return;
        } else if (completion_status == TASK_COMPLETION_REASON_COMMAND_COMPLETE) {
            return;
        }
        //self->speech_input.compute_updated_hypothesis(self->_message_update);
        NSString* result = //self->speech_input.compute_updated_hypothesis(self->_message_update);
        self->speech_input.update_message_with_result(self->_message_update);
        MTT_NSLog(@"RESULT: [%@] id=[%llu] mod_count=[%llu]\n", result, self->_recognitionID, self->_modificationCount);
        
        void* data = (__bridge void*)result;
//        NSMutableString* msg_out = nil;
//        {
//            auto& text_out = dt::ctx()->ui.margin_panels[0].text.text;
//            usize reserved_size = 0;
//            for (usize i = 0; i < text_out.size(); i += 1) {
//                reserved_size += 1 + text_out[i].text.size();
//            }
//            msg_out = [[NSMutableString alloc] initWithCapacity:reserved_size];
//            usize word_count = 0;
//            {
//                usize i = 0;
//                for (;i < text_out.size(); i += 1) {
//                    auto& text = text_out[i];
//                    if (!text.is_selected) {
//                        continue;
//                    }
//                    word_count += 1;
//                    cstring str = text_out[i].text.c_str();
//                    [msg_out appendFormat:@"%s ", str];
//                }
//            }
//            
//            if (word_count != 0) {
//                [msg_out deleteCharactersInRange:NSMakeRange([msg_out length]-1, 1)];
//                
//                data = (__bridge void*)((NSString*)msg_out);
//            }
//        }
//
        const usize ID                 = self->_recognitionID;
        const usize modification_count = self->_modificationCount;
        
        for (auto i = subscribers.begin(); i != subscribers.end(); ++i) {
            Speech_Recognition_Handler* handler = (*i);
            bool _ = handler->handle_hypothesis(handler, data, ID, modification_count, mtt_time_seconds(),
                                       0, {.args = (__bridge void*)self }
                                       );
        }
        
        data = nullptr;

    }
}

-(void) speechRecognitionTask:(SFSpeechRecognitionTask *)task didFinishSuccessfully:(BOOL)successfully {

    @autoreleasepool {
        ASSERT_MSG(mtt::is_main_thread(), "%s\n", "should be in main thread");
        MTT_print("SPEECH_TASK: %s\n", task_state_to_string(task.state));
        
        auto completion_status = [self completionReason];
        
        //ASSERT_MSG(count == 1, "%s\n", "map should have only one thing");
    
        if (!successfully) {
            ASSERT_MSG(completion_status != TASK_COMPLETION_REASON_NONE, "%s\n", "task should still exist");
#ifndef NDEBUG
            NSLog(@"Error: {%@} %s %s %d\n", [task.error localizedDescription], __FILE__, __PRETTY_FUNCTION__, __LINE__);
//            switch (completion_status) {
//                case TASK_COMPLETION_REASON_ENABLE: {
//                    ASSERT_MSG([self is_running], "%s\n", "should be enabled!");
//                    break;
//                }
//                case TASK_COMPLETION_REASON_DISABLE: {
//                    ASSERT_MSG(![self is_running], "%s\n", "should be disabled!");
//                    break;
//                }
//                case TASK_COMPLETION_REASON_DISCARD: {
//                    break;
//                }
//                default: {
//                    MTT_println_s("?????");
//                    break;
//                }
//            }
            MTT_print("%s\n", task_completion_reason_str(completion_status));
            //[self removeTask:task];
#endif
        } else {

            MTT_NSLog(@"Success: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
            //[self removeTask:task];
        }
        if ([self is_running]) {
            [self enqueue_complete_task_event:completion_status];
        }
    }
    
    return;
}

-(BOOL) split {
    if (![self is_running] ||
        (![self audioEngine].isRunning)) {
        return NO;
    }
    
    @autoreleasepool {

        
#if DT__CONSIDER_DEFERRING_IF_TALKING_UNLIKELY
        
        if ([self get_speech_state] == SPEECH_STATE_TALKING_LIKELY) {
            return NO;
        }
        
#endif
        
        {
            NSString* message = self->speech_input.most_recent_message();
            
            MTT_NSLog(@"Split: [%@] id=[%llu]\n", message, self->_recognitionID);
            if (message.length == 0) {
                MTT_error("%s\n", "Empty message for some reason");
                self->speech_input.reset();
                
                [self set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];

                //[((SpeechRecognition*)self) restartForCommand];
                return NO;
            }
// FIXME: temp way to send sub-selections of text for just the final result for now, need to try and do this for intermediate results
#if (0)
            void* data = (__bridge void*)message;
            
            for (auto i = subscribers.begin(); i != subscribers.end(); ++i) {
                Speech_Recognition_Handler* handler = (*i);
                
                handler->handle_result(handler, data, self->_recognitionID, self->_modificationCount, mtt_time_seconds(), {.args = nullptr });
            }
#else
            auto& text_out = dt::ctx()->ui.margin_panels[0].text.text;
            if (text_out.empty()) {
                MTT_error("%s\n", "Empty message for some reason");
                self->speech_input.reset();
                
                [self set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];

                [((SpeechRecognition*)self) restartForCommand];
                return NO;
            }
            
            usize reserved_size = 0;
            for (usize i = 0; i < text_out.size(); i += 1) {
                reserved_size += 1 + text_out[i].text.size();
            }
            NSMutableString* msg_out = [[NSMutableString alloc] initWithCapacity:reserved_size];
            usize word_count = 0;
            {
                usize i = 0;
                for (;i < text_out.size(); i += 1) {
                    auto& text = text_out[i];
                    if (!text.is_selected) {
                        continue;
                    }
                    word_count += 1;
                    cstring str = text_out[i].text.c_str();
                    [msg_out appendFormat:@"%s ", str];
                }
            }
            
            void* data = nullptr;
            if (word_count == 0) {
                data = (__bridge void*)message;
            } else {
                [msg_out deleteCharactersInRange:NSMakeRange([msg_out length]-1, 1)];
                
                data = (__bridge void*)((NSString*)msg_out);
            }
            
            for (auto i = subscribers.begin(); i != subscribers.end(); ++i) {
                Speech_Recognition_Handler* handler = (*i);
                
                handler->handle_result(handler, data, self->_recognitionID, self->_modificationCount, mtt_time_seconds(), {.args = nullptr });
            }
#endif
        }
        
        
        self->speech_input.reset();
        
        [self set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];

        
        [((SpeechRecognition*)self) restartForCommand];
        
        
        self->_recognitionID += 1;
        self->_modificationCount = 1;
        
        return YES;
    }

}

-(BOOL) discard {
    //ASSERT_MSG(count == 1, "%s\n", "map should have only one thing");
    @autoreleasepool {
#if DT__CONSIDER_DEFERRING_IF_TALKING_UNLIKELY
        if ([self get_speech_state] == SPEECH_STATE_TALKING_LIKELY) {
            return NO;
        }
#endif
//        self->_recognitionIDDiscarded = _recognitionID;
//        self->_modificationCountDiscarded = _modificationCount;
        self->discarded.push_back({
            .recognition_ID = self->_recognitionID,
            .modification_count = self->_modificationCount
        });
        self->speech_input.reset();
                
        [self set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];
        
            //[((SpeechRecognition*)self) restart];
        [((SpeechRecognition*)self) restartForDiscard];
        
        
        Speech_System_Event_Status status = {};
        [self invokeCallbacks:SPEECH_SYSTEM_EVENT_DISCARD withStatus:status];
        
        //[self increment_ID];

        //self->_modificationCount = 1;
        self->_modificationCount += 1;
    }
    
    return YES;
}

-(BOOL) send_override:(NSString*)msg withMode:(int)mode {
    
    MTT_print("send_override mode=%d\n", mode);
    
    @autoreleasepool {
        
        const usize ID                 = self->_recognitionID;
        const usize modification_count = self->_modificationCount;
        void* data = (__bridge void*)msg;
    
        switch (mode) {
        // hypothesis
        case 0: {
            for (auto i = subscribers.begin(); i != subscribers.end(); ++i) {
                Speech_Recognition_Handler* handler = (*i);
                
                handler->handle_hypothesis(handler, data, ID, modification_count, mtt_time_seconds(), 0,
                                           {.args = (void*)nullptr }
                                           );
            }
            break;
        }
        // result
        case 1: {
            
            for (auto i = subscribers.begin(); i != subscribers.end(); ++i) {
                Speech_Recognition_Handler* handler = (*i);
                handler->handle_result(handler, data, ID, modification_count, mtt_time_seconds(), {.args = (__bridge void*)self});
                
            }
            
            self->speech_input.reset();
                    
            [self set_speech_state:SPEECH_STATE_TALKING_UNLIKELY];

            [((SpeechRecognition*)self) restart];
            
            break;
        }
        }
    }
    
    return YES;
}



-(void) speechRecognitionTaskWasCancelled:(SFSpeechRecognitionTask *)task
{
    MTT_print("SPEECH_TASK: %s\n", task_state_to_string(task.state));
}

-(void) registerCallback:(void(*)(SPEECH_SYSTEM_EVENT, Speech_System_Event_Status, void*)) callback forEvent:(SPEECH_SYSTEM_EVENT)event withData:(void*) data
{
    self->event_callbacks[event][data] = callback;
}

-(void) unregisterCallback:(SPEECH_SYSTEM_EVENT)forEvent withData:(void*) data
{
    auto callbacks = self->event_callbacks[forEvent];
    auto found = callbacks.find(data);
    if (found != callbacks.end()) {
        callbacks.erase(found);
    }
}

-(void) invokeCallbacks:(SPEECH_SYSTEM_EVENT)event withStatus:(Speech_System_Event_Status) status
{
    auto callbacks = self->event_callbacks[event];
    for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
        (*it).second(event, status, (*it).first);
    }
}

-(void) reset_speech_input
{
    self->speech_input.reset();
}

-(void) increment_ID
{
    self->_recognitionID += 1;
}
-(void) set_ID:(usize) newID
{
    self->_recognitionID = newID;
}

-(void) resetState {
    
}




- (BOOL) setCompletionReasonForTask:(SFSpeechRecognitionTask* __nonnull)task withReason:(TASK_COMPLETION_REASON)reason
{
    self->task_completion_status = reason;
    return YES;
}

- (SFSpeechRecognitionTask *__nonnull)recognitionTaskWithRecognizer:(SFSpeechRecognizer * __nonnull)recognizer withRequest:(SFSpeechRecognitionRequest *__nonnull)request
                                               delegate:(id <SFSpeechRecognitionTaskDelegate>__nonnull)delegate;
{
    
#if DT_TASK_COUNT_CHECK
    ASSERT_MSG([self recognitionTask] == nil, "%s\n", "Only one task should exist at once");
#endif
    SFSpeechRecognitionTask* task = [recognizer recognitionTaskWithRequest:request delegate:delegate];
    //tasks->insert({(__bridge void*)task, TASK_COMPLETION_REASON_INCOMPLETE});
    self->task_completion_status = TASK_COMPLETION_REASON_INCOMPLETE;
    //MTT_print("%s size is now=[%zu]\n", "successfully added task to map", tasks->size());
    return task;
}


- (TASK_COMPLETION_REASON) completionReason
{
    return self->task_completion_status;
}
- (void) removeTask:(SFSpeechRecognitionTask* __nonnull)task
{
#if DT_TASK_COUNT_CHECK
    ASSERT_MSG(self->task_completion_status != TASK_COMPLETION_REASON_NONE, "%s\n", "Task should not be at none state when removed");
#endif
    self->task_completion_status = TASK_COMPLETION_REASON_NONE;
}




-(BOOL) IDsAreStaleDiscarded:(usize)recognition_ID with_modification_count:(usize)modification_count
{
    auto& discarded = self->discarded;
    const usize count = discarded.size();
    for (usize i = 0; i < count; i += 1) {
        auto* el = &discarded[i];
        if (el->recognition_ID == recognition_ID
            && el->modification_count == modification_count) {
            return true;
        }
    }
    // clean-up
    // TODO: test
//    for (usize i = 0; i < count; i += 1) {
//        if (discarded[i].recognition_ID >= recognition_ID) {
////            for (usize idx = 0; idx < i; idx += 1) {
////                discarded[idx] = discarded[i + idx];
////            }
//            memmove(&discarded[0], &discarded[i], sizeof(discarded[0]) * (count - i));
//            discarded.resize(count - i);
//            break;
//        }
//    }
    return false;
}

- (void)speechRecognitionDidDetectSpeech:(SFSpeechRecognitionTask *)task
{
    @autoreleasepool {
//        usize count = [self taskMapSize];
//        ASSERT_MSG(count == 1, "%s\n", "map should have only one thing");
        
        
    }
}

- (void) enqueue_complete_task_event:(TASK_COMPLETION_REASON) task_completion_reason {
    self->queue_task_creation.push_back(task_completion_reason);
}

- (void) clear_complete_task_events {
    self->queue_task_creation.clear();
}


- (void) on_update {
    if (!(!self->_audioEngine.isRunning && self->_is_active)) {
        return;
    }
    
    if (!self->queue_task_creation.empty()) {
        // TODO: create new task and request
#ifndef NDEBUG
        if (self->queue_task_creation.size() > 1) MTT_UNLIKELY {
            MTT_println_s("WARNING: task creation queue should only have a size of 1");
        }
#endif
        [((SpeechRecognition*)self) doTaskNoExceptionWithSupportsOnDeviceRecognition:self.speechRecognizer.supportsOnDeviceRecognition];
        [self clear_complete_task_events];
    }
    
}

- (BOOL) is_running {
    //return self->_audioEngine.isRunning;
    return self->_is_active;
}

- (void) stop {
    [self->_audioEngine stop];
}
    
#undef DT__CONSIDER_DEFERRING_IF_TALKING_UNLIKELY

@end

bool Speech_check_if_IDs_stale_bc_discarded(Speech_Info* speech_info, usize recognition_id, usize modification_count)
{
    SpeechRecognition* speech = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    
    return ([speech IDsAreStaleDiscarded:recognition_id with_modification_count:modification_count] == YES);
}
