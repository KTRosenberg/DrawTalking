//
//  speech_info_platform.mm
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/14/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//


#include "speech_info_platform.hpp"
#include "speech_recognition_objc.hpp"
#include "Make_The_Thing-Swift.h"

#include "make_the_thing_main.hpp"


void Speech_System_init(Speech_System* sys, void* backend)
{
    sys->info_count = 1;
    sys->backend = backend;
    
    //Speech_Info_init(&sys->info_list[0]);
}

bool Speech_split_platform(Speech_Info* speech_info)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
//    if (![speech_rec transcriptIsInProgress]) {
//        return false;
//    }
    
    if (![speech_rec is_running]) {
        return false;
    }
    
    if ([speech_rec get_speech_state] == SPEECH_STATE_TALKING_LIKELY) {
        return [speech_rec split];
        [speech_rec splitDeferred];
        return false;
    } else {
        
        return [speech_rec split];
        usize active_id = Speech_Info_most_recent_record_id(speech_info);
        auto view = Speech_Info_get_record(speech_info, active_id);
        
        if (view.nl_info == nullptr) {
            MTT_error("%s", "ERROR: some kind of error!\n");
            return true;
        }
        
        return [speech_rec split];
    }
}

bool Speech_discard_platform(Speech_Info* speech_info)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;

    return [speech_rec discard];
}

bool currently_talking_is_likely_platform(Speech_Info* speech_info)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    return ([speech_rec get_speech_state] == SPEECH_STATE_TALKING_LIKELY);
}


void Speech_set_active_state(Speech_Info* speech_info, bool state)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    if (state == false) {
        [speech_rec disable];
    } else {
        [speech_rec restart];
    }
}

bool Speech_get_active_state_platform(Speech_Info* speech_info)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    return [speech_rec is_running];
}

void Speech_reset(Speech_Info* speech_info)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    //[speech_rec setShouldReset:YES];
    [speech_rec restart];
}

void Speech_send_override_platform(Speech_Info* speech_info, mtt::String& msg, int mode)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;

    [speech_rec send_override:[NSString stringWithUTF8String:msg.c_str()] withMode:mode];
}

//void Speech_set_ignore_state_platform(Speech_Info* speech_info, bool do_ignore)
//{
//    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;    
//}


void Speech_register_event_callback_platform(Speech_Info* speech_info, SPEECH_SYSTEM_EVENT event, void (*callback)(SPEECH_SYSTEM_EVENT event, Speech_System_Event_Status status, void* data), void* data)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    
    [speech_rec registerCallback:callback forEvent:event withData:data];
    
}

void Speech_unregister_event_callback_platform(Speech_Info* speech_info, SPEECH_SYSTEM_EVENT event, void* data)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    
    
    [speech_rec unregisterCallback:event withData:data];
}

usize Speech_recognition_ID_platform(Speech_Info* speech_info)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    return [speech_rec recognitionID];
}

usize Speech_recognition_modification_count_platform(Speech_Info* speech_info)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    return [speech_rec modificationCount];
}


void Speech_process_text_as_input_platform(Speech_Info* speech_info, const mtt::String& text, uint64 flags)
{
    MAIN_process_text_as_input(speech_info, text, flags);
}

void Speech_recognition_increment_ID_platform(Speech_Info* speech_info)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    [speech_rec setRecognitionID:[speech_rec recognitionID] + 1];
}

void Speech_recognition_set_ID_platform(Speech_Info* speech_info, usize new_id)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    [speech_rec setRecognitionID:new_id];
}

NSString* preprocess_local_transcription(NSString* string)
{
    return [string stringByReplacingOccurrencesOfString:@" on to" withString:@" onto"];
    //return string;
}

void Speech_recognition_reset_state_platform(Speech_Info* speech_info)
{
    SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)speech_info->system_callbacks.speech_system->backend;
    [speech_rec resetState];
}

