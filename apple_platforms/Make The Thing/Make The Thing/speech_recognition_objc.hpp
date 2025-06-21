//
//  speech_recognition_objc.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/28/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef speech_recognition_objc_h
#define speech_recognition_objc_h

#include "speech_system_events.h"
#include "speech_timestamp.h"

//#include "speech_recognition_handler.hpp"
struct Speech_Recognition_Handler;

cstring __nonnull task_state_to_string(SFSpeechRecognitionTaskState state);


typedef enum TASK_COMPLETION_REASON {
    TASK_COMPLETION_REASON_NONE,
    TASK_COMPLETION_REASON_INCOMPLETE,
    TASK_COMPLETION_REASON_DISCARD,
    TASK_COMPLETION_REASON_COMMAND_COMPLETE,
    TASK_COMPLETION_REASON_ENABLE,
    TASK_COMPLETION_REASON_DISABLE,
} TASK_COMPLETION_REASON;

static cstring __nonnull task_completion_reason_strings[] = {
    [TASK_COMPLETION_REASON_NONE]  = "TASK_COMPLETION_REASON_NONE",
    [TASK_COMPLETION_REASON_INCOMPLETE]  = "TASK_COMPLETION_REASON_INCOMPLETE",
    [TASK_COMPLETION_REASON_DISCARD]   = "TASK_COMPLETION_REASON_DISCARD",
    [TASK_COMPLETION_REASON_COMMAND_COMPLETE] = "TASK_COMPLETION_REASON_COMMAND_COMPLETE",
    [TASK_COMPLETION_REASON_ENABLE] = "TASK_COMPLETION_REASON_ENABLE",
    [TASK_COMPLETION_REASON_DISABLE] = "TASK_COMPLETION_REASON_DISABLE",
};

static inline cstring task_completion_reason_str(TASK_COMPLETION_REASON reason)
{
    return task_completion_reason_strings[reason];
}

typedef struct Task_Completion_Status {
    bool is_valid;
    TASK_COMPLETION_REASON reason;
} Task_Completion_Status;

typedef struct Recognition_Info {
    usize recognition_ID;
    usize modification_count;
} Recognition_Info;

@interface Speech_Recognition_ObjC : NSObject <SFSpeechRecognizerDelegate, SFSpeechRecognitionTaskDelegate>
{
    @public SPEECH_STATE speech_state;
    @public float amplitude;
}

@property NSOperationQueue* speech_queue;
@property usize recognitionID;
@property usize modificationCount;
//@property usize recognitionIDDiscarded;
//@property usize modificationCountDiscarded;

//@property BOOL isPlaying;

@property NSTimer* blocked_timer;

@property BOOL shouldReset;

@property NSString* reason;
@property NSInteger reasonCount;

@property NSString* message_update;

@property NSLocale* locale;

@property SFSpeechRecognizer* speechRecognizer;
@property SFSpeechAudioBufferRecognitionRequest* recognitionRequest;
@property SFSpeechRecognitionTask* recognitionTask;
@property AVAudioEngine* audioEngine;

@property BOOL is_active;

- (instancetype)init;

-(SPEECH_STATE) get_speech_state;
-(void) set_speech_state:(SPEECH_STATE) state;

-(void) set_amplitude:(float) amplitude;
//-(void) set_on_amplitude_change_callback:(void(*)(float))callback;

-(void) addSubscriber:(struct Speech_Recognition_Handler *) handler;

-(void) removeSubscriber:(struct Speech_Recognition_Handler *) handler;


//-(void) speechRecognitionTask:(SFSpeechRecognitionTask *)task didHypothesizeTranscription:(SFTranscription*)transcription;

//-(void) speechRecognitionTask:(SFSpeechRecognitionTask *)task didFinishRecognition:(SFSpeechRecognitionResult *)recognitionResult;

//-(void) speechRecognitionTask:(SFSpeechRecognitionTask *)task didFinishSuccessfully:(BOOL)successfully;


-(BOOL) split;

-(BOOL) discard;


-(BOOL) send_override:(NSString*)msg withMode:(int)mode;

//public override func speechRecognitionTaskWasCancelled(_ task: SFSpeechRecognitionTask) {
//-(void) speechRecognitionTaskWasCancelled:(SFSpeechRecognitionTask *)task;


NS_ASSUME_NONNULL_BEGIN
-(void) registerCallback:(void(*)(SPEECH_SYSTEM_EVENT, Speech_System_Event_Status, void*)) callback forEvent:(SPEECH_SYSTEM_EVENT)event withData:(void*) data;

-(void) unregisterCallback:(SPEECH_SYSTEM_EVENT)forEvent withData:(void*) data;

-(void) invokeCallbacks:(SPEECH_SYSTEM_EVENT)event withStatus:(Speech_System_Event_Status) status;
-(void) reset_speech_input;

-(void) increment_ID;

-(void) set_ID:(usize) newID;

-(void) resetState;

- (SFSpeechRecognitionTask *__nonnull)recognitionTaskWithRecognizer:(SFSpeechRecognizer * __nonnull)recognizer withRequest:(SFSpeechRecognitionRequest * __nonnull)request
delegate:(id <SFSpeechRecognitionTaskDelegate>__nonnull)delegate;

-(BOOL) setCompletionReasonForTask:(SFSpeechRecognitionTask* __nonnull)task withReason:(TASK_COMPLETION_REASON)reason;
-(void) removeTask:(SFSpeechRecognitionTask* __nonnull)task;
-(TASK_COMPLETION_REASON) completionReason;

-(BOOL) IDsAreStaleDiscarded:(usize)recognition_ID with_modification_count:(usize)modification_count;

- (void) enqueue_complete_task_event:(TASK_COMPLETION_REASON) task_completion_reason;
- (void) clear_complete_task_events;
- (void) on_update;

- (BOOL) is_running;

- (void) stop;

@end
NS_ASSUME_NONNULL_END
#endif /* speech_recognition_objc_h */
 
