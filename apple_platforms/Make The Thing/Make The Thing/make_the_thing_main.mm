//
//  make_the_thing_main.m
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/11/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//


#include "make_the_thing_main.h"
#include "Make_The_Thing-Swift.h"
//#define SOKOL_METAL
//#define SOLOL_IMPL
//#include "sokol_gfx.h"

#define MTT_IMPLEMENTATION
#include "make_the_thing.h"

//#include "stratadraw_platform.h"

//#include "c_library.h"
//#define DR_WAV_IMPLEMENTATION

//#include "audio/miniaudio/extras/dr_wav.h"
//#include "audio/miniaudio/extras/dr_flac.h"
//#include "audio/miniaudio/extras/dr_mp3.h"

#include "audio/miniaudio/miniaudio.h"


#include "mtt_core_platform.h"
#include "mtt_time.h"

#include "speech_info_platform.hpp"

#include "memory.hpp"

#include "file_system_platform.hpp"

#include "drawtalk.hpp"





typedef BOOL (^Speech_BlockCallbackType)(NSString* _Nonnull, const void*);

//speechCallbackBlockType block = receive_speech;

Speech_BlockCallbackType speech_recognition_callback = ^BOOL(NSString* _Nonnull msg, const void* user_data) {
    
    @autoreleasepool {
        //NSLog(@"%@", msg);
    }
    return true;
};


// MARK:- speech handlers
bool handle_speech_hypothesis(void* handler, void* data, usize record_id, usize modification_count, float64 time_seconds, float64 time_difference);
void handle_speech_result(void* handler, void* data, usize record_id, usize modification_count, float64 time_seconds);
// TODO: this is a global scope hack
TCP_Client* client;
TCP_Client* client_msgs;

// MARK: - speech to NLP callbacks
const NSString* const MESSAGE_SPEECH_HYPOTHESIS = @"SPHY";
const NSString* const MESSAGE_SPEECH_RESULT     = @"SPRE";

NSTimer* speech_timer = nil;

float32 max_time_delta = 0.0f;
float32 time_prev      = 0.0f;

//#define SPEECH_HANDLER_DEBUG
#ifdef SPEECH_HANDLER_DEBUG
#define SPEECH_DEBUG_PRINT(...) MTT_print(__VA_ARGS__)
#else
#define SPEECH_DEBUG_PRINT(...)
#endif

const static float32 SILENCE_DELAY = 0.7f//0.61f
;



bool handle_speech_hypothesis(void* handler, void* data, usize record_id, usize modification_count, float64 time_seconds, float64 time_difference, Handler_Args args)
{
    Speech_Recognition_Handler* sr_handler = (Speech_Recognition_Handler*)handler;
    
    SpeechRecognition* speech = (__bridge SpeechRecognition*)sr_handler->speech_system->backend;


    SPEECH_DEBUG_PRINT("HYPOTHESIS IN_FLIGHT\n");
    
    usize active_id = Speech_Info_most_recent_record_id(sr_handler->speech_info);
    if (active_id == record_id) {
        SPEECH_DEBUG_PRINT("%s id=[%llu]\n", "updating record:", record_id);
        
        SPEECH_DEBUG_PRINT("\t TIME DIFF: %f, time_prev %f\n", time_seconds - time_prev, time_prev);
        

        Speech_Info_View info = Speech_Info_most_recent_record(sr_handler->speech_info);
        NSString* text_data = ((__bridge NSString*)(data));
        //SPEECH_DEBUG_PRINT("%s [%s]\n", __func__, [text_data cStringUsingEncoding:NSUTF8StringEncoding]);
        if (info.text_info->marked_final) {
            return true;
        }
        
        {
            info.text_info->marked_final = false;
            info.text_info->is_changed   = true;
            info.text_info->text = mtt::String([text_data cStringUsingEncoding:NSUTF8StringEncoding]);
            
            
//            ASSERT_MSG(info.nl_info != nullptr, "This should never be null");
            
            text_data = preprocess_local_transcription(text_data);
            
            info.text_info->modification_count += 1;
            
            Speech_Event ev;
            ev.event_type = SPEECH_EVENT_TYPE_NEW_TRANSCRIPTION_HYPOTHESIS;
            ev.ID = record_id;
            ev.timestamp = time_seconds;
            
            Speech_push_event(sr_handler->speech_info, &ev);
            

            NSString* text_msg = [MESSAGE_SPEECH_HYPOTHESIS stringByAppendingFormat:@"%llu&%llu:%@|", record_id, modification_count, text_data];
            [client write:[text_msg dataUsingEncoding:NSUTF8StringEncoding] withTag:record_id];
        }
    } else {
        // FIXME: this should really not happen
        if (active_id > record_id) {
            MTT_print("%s", "WARNING: old record arrived! ignoring old record\n");
            Speech_recognition_set_ID(sr_handler->speech_info, active_id + 1);
        }
        // less than
        {
            auto& text_panel = dt::DrawTalk::ctx()->ui.margin_panels[0].text;
            for (auto it = text_panel.text.begin(); it != text_panel.text.end(); ++it) {
                (*it).is_selected = false;
                (*it).update.mapping_count = 0;
                (*it).update.mappings.clear();
            }
        }
        
        NSString* text_data = ((__bridge NSString*)(data));

        SPEECH_DEBUG_PRINT("%s [%s]\n", __func__, [text_data cStringUsingEncoding:NSUTF8StringEncoding]);
        
        SPEECH_DEBUG_PRINT("%s id=[%llu]\n", "new record:", active_id);
        
        Speech_Info_push_record(sr_handler->speech_info, record_id, time_seconds);
        
        Speech_Info_View info = Speech_Info_most_recent_record(sr_handler->speech_info);
        if (!info.is_valid || info.nl_info == nullptr || info.text_info == nullptr) {
            return true;
        }
        
        info.text_info->marked_final = false;
        info.text_info->is_changed   = true;
        //info.text_info->text = std::string([text_data cStringUsingEncoding:NSUTF8StringEncoding]);
        info.text_info->timestamp_created = time_seconds;
        info.text_info->timestamp_changed = time_seconds;
        info.nl_info->is_valid   = false;
        info.nl_info->is_changed = false;
        
        
        text_data = preprocess_local_transcription(text_data);
        
        
        info.text_info->modification_count += 1;
        
        Speech_Event ev;
        ev.event_type = SPEECH_EVENT_TYPE_NEW_TRANSCRIPTION_HYPOTHESIS;
        ev.ID = record_id;
        ev.timestamp = time_seconds;
        
        Speech_push_event(sr_handler->speech_info, &ev);
        
        {
            NSString* text_msg = [MESSAGE_SPEECH_HYPOTHESIS stringByAppendingFormat:@"%llu&%llu:%@|", record_id, modification_count, text_data];
            [client write:[text_msg dataUsingEncoding:NSUTF8StringEncoding] withTag:record_id];
        }
    }
    
    
    SPEECH_DEBUG_PRINT("\tspeech record count=[%llu]\n", Speech_Info_record_count(sr_handler->speech_info));
    
    return true;
}

void handle_speech_result_manual(mtt::String msg, Speech_Recognition_Handler* sr_handler)
{
    ASSERT_MSG(false, "Do not use this!\n");
//    SpeechRecognition* speech = (__bridge SpeechRecognition*)sr_handler->speech_system->backend;
//    //[speech set_speech_state:SPEECH_STATE_TALKING_LIKELY];
//
//    usize record_id = Speech_Info_most_recent_record_id(sr_handler->speech_info);
//
//    {
//        SPEECH_DEBUG_PRINT("%s id=[%llu]\n", "final record:", active_id);
//
//        Speech_Info_View info = Speech_Info_most_recent_record(sr_handler->speech_info);
//        if (info.text_info->marked_final) {
//            SPEECH_DEBUG_PRINT("%s id=[%llu]\n", "record is already final", active_id);
//            return;
//        }
//
//
//
//        //info.text_info->marked_final = true;
//        info.text_info->is_changed = true;
//        info.text_info->text = msg;
//        info.text_info->timestamp_changed = 0;
//
//        preprocess_local_transcription(info);
//
//        info.text_info->modification_count += 1;
//
//        Speech_Event ev;
//        ev.event_type = SPEECH_EVENT_TYPE_NEW_TRANSCRIPTION_FINAL;
//        ev.ID = record_id;
//        ev.timestamp = 0;
//
//        Speech_push_event(sr_handler->speech_info, &ev);
//
//
//        NSString* text_msg = [MESSAGE_SPEECH_RESULT stringByAppendingFormat:@"%llu&%llu:%@|", record_id, info.text_info->modification_count, [NSString stringWithUTF8String:msg.c_str()]];
//        [client write:[text_msg dataUsingEncoding:NSUTF8StringEncoding] withTag:record_id];
//
//        SPEECH_DEBUG_PRINT("%s [%s]\n", __func__, [text_data cStringUsingEncoding:NSUTF8StringEncoding]);
//    }
//
//    SPEECH_DEBUG_PRINT("\n");
    //Speech_Info_print(sr_handler->speech_info);
}

void handle_speech_result(void* handler, void* data, usize record_id, usize modification_count, float64 time_seconds, Handler_Args args)
{
    if (speech_timer != nil && [speech_timer isValid]) {
        [speech_timer invalidate];
    }
    
    Speech_Recognition_Handler* sr_handler = (Speech_Recognition_Handler*)handler;
    
    SpeechRecognition* speech = (__bridge SpeechRecognition*)sr_handler->speech_system->backend;
    //[speech set_speech_state:SPEECH_STATE_TALKING_LIKELY];


    
    usize active_id = Speech_Info_most_recent_record_id(sr_handler->speech_info);
    if (active_id == record_id) {
        SPEECH_DEBUG_PRINT("%s id=[%llu]\n", "final record:", active_id);
        
        Speech_Info_View info = Speech_Info_most_recent_record(sr_handler->speech_info);
        if (info.text_info->marked_final) {
            SPEECH_DEBUG_PRINT("%s id=[%llu]\n", "record is already final", active_id);
            return;
        }
        


        //info.text_info->marked_final = true;
        info.text_info->is_changed = true;
        NSString* text_data = ((__bridge NSString*)(data));
        text_data = preprocess_local_transcription(text_data);
        
        //info.text_info->text = mtt::String([text_data cStringUsingEncoding:NSUTF8StringEncoding]);
        
        info.text_info->timestamp_changed = time_seconds;
            
        
        
        info.text_info->modification_count += 1;
        
        Speech_Event ev;
        ev.event_type = SPEECH_EVENT_TYPE_NEW_TRANSCRIPTION_FINAL;
        ev.ID = record_id;
        ev.timestamp = time_seconds;
        
        Speech_push_event(sr_handler->speech_info, &ev);
        

        NSString* text_msg = [MESSAGE_SPEECH_RESULT stringByAppendingFormat:@"%llu&%llu:%@|", record_id, modification_count, text_data];
        [client write:[text_msg dataUsingEncoding:NSUTF8StringEncoding] withTag:record_id];

        SPEECH_DEBUG_PRINT("%s [%s]\n", __func__, [text_data cStringUsingEncoding:NSUTF8StringEncoding]);
                
    } else {
        SPEECH_DEBUG_PRINT("%s id=[%llu]\n", "ERROR: should be final record, but considered new:", active_id);
    }
    
    
    SPEECH_DEBUG_PRINT("\n");
    //Speech_Info_print(sr_handler->speech_info);
}
void MAIN_process_text_as_input(Speech_Info* speech_info, const mtt::String& text, uint64 flags)
{
    Speech_Recognition_Handler* sr_handler = &speech_info->system_callbacks;

    @autoreleasepool {
        NSString* text_data = [NSString stringWithUTF8String:text.c_str()];
        text_data = preprocess_local_transcription(text_data);
        
        //Speech_send_override(speech_info, mtt::String(cs), 1);
        

        
        auto record_id = 0llu;
        Speech_Info_push_record(sr_handler->speech_info, record_id, 0);
        constexpr const usize modification_count = ~(0);

        Speech_Event ev;
        ev.event_type = SPEECH_EVENT_TYPE_NEW_TRANSCRIPTION_FINAL;
        ev.ID = record_id;
        ev.timestamp = 0;

        Speech_push_event(sr_handler->speech_info, &ev);

        NSString* text_msg = [MESSAGE_SPEECH_RESULT stringByAppendingFormat:@"%llu&%llu:%@|", record_id, modification_count, text_data];
        
        [client write:[text_msg dataUsingEncoding:NSUTF8StringEncoding] withTag:record_id];
        
        Speech_discard(speech_info);
        //Speech_reset(speech_info);
        Speech_recognition_increment_ID(speech_info);
        //Speech_recognition_reset_state(speech_info);
    }
}



void handle_nlp_result(void* handler, void* data, usize length, float64 time_seconds, usize ID, usize modification_count, usize mode, Handler_Args args)
{
    // TODO: - if final result, enqueue into a "to-process" queue for the main loop to poll
    SPEECH_DEBUG_PRINT("NLP RESULT ARRIVED\n");
    Speech_Recognition_Handler* sr_handler = (Speech_Recognition_Handler*)(((Data_Handler*)handler)->data);
    SpeechRecognition* speech = (__bridge SpeechRecognition*)sr_handler->speech_system->backend;
    if ([speech IDsAreStaleDiscarded:ID with_modification_count:modification_count]) {
        
        MTT_error("discarded ID, nlp result should be ignored: ID=[%llu] mod_count=[%llu]\n", ID, modification_count);
        return;
    }
    
    
    Speech_Info_View info = Speech_Info_ID_to_record(sr_handler->speech_info, ID);
    if (!info.is_valid) {
        MTT_error("Something is wrong: requested ID is:[%llu], but size is [%lu]\n", ID, sr_handler->speech_info->text_info.size());

        return;
    }
//    if (info.nl_info->modification_count > modification_count) {
//        MTT_error("ERROR: NLP update aborted due to stale modification count\n");
//        return;
//    }
    
    if (info.nl_info->marked_final) {
        MTT_error("NL Info [%llu %llu] already marked final", ID, modification_count);
        return;
    }
    
    if (mode == 0) { // hypothesis
        NSData* nlp_data = (__bridge NSData*)data;
        const char* as_str = [[[NSString alloc] initWithData:nlp_data encoding:NSUTF8StringEncoding] cStringUsingEncoding:NSUTF8StringEncoding];
        bool ok = Natural_Language_Info_update_with_JSON(
            (Natural_Language_Info*)info.nl_info,
            (void*)as_str,
            (usize)nlp_data.length,
            (float64)time_seconds
        );
        if (!ok) {
            MTT_error("%s", "ERROR: NLP update unsuccessful\n");
            return;
        }
        
        info.nl_info->is_changed = true;
        info.nl_info->is_valid = true;
        info.nl_info->modification_count = modification_count;
        
        if (ID <= sr_handler->speech_info->first_unfinished_id) {
            Natural_Language_Event ev;
            ev.event_type = NATURAL_LANGUAGE_EVENT_TYPE_NEW_TRANSCRIPTION_FINAL;
            ev.ID = ID;
            ev.timestamp = time_seconds;
            ev.idx = info.nl_info->doc->doc_queue.size() - 1;
            
            Natural_Language_push_finished_event(sr_handler->speech_info, &ev);
            Natural_Language_push_all_finished_deferred_events(sr_handler->speech_info);
        } else {
            MTT_error("%s\n", "So this case does happen somehow");
            Natural_Language_Event ev;
            ev.event_type = NATURAL_LANGUAGE_EVENT_TYPE_NEW_TRANSCRIPTION_FINAL;
            ev.ID = ID;
            ev.timestamp = time_seconds;
            ev.idx = info.nl_info->doc->doc_queue.size() - 1;
            
            Natural_Language_push_deferred_event(sr_handler->speech_info, &ev);
        }
        
    } else if (mode == 1) { // finished
        NSData* nlp_data = (__bridge NSData*)data;
        const char* as_str = [[[NSString alloc] initWithData:nlp_data encoding:NSUTF8StringEncoding] cStringUsingEncoding:NSUTF8StringEncoding];
        bool ok = Natural_Language_Info_update_with_JSON(
            (Natural_Language_Info*)info.nl_info,
            (void*)as_str,
            (usize)nlp_data.length,
            (float64)time_seconds
        );
        if (!ok) {
            MTT_error("%s", "ERROR: NLP update unsuccessful\n");
            return;
        }
        
        info.nl_info->is_changed = true;
        info.nl_info->is_valid = true;
        info.nl_info->modification_count = modification_count;
        info.nl_info->marked_final = true;
        info.text_info->marked_final = true;
    
        
        SPEECH_DEBUG_PRINT("FIRST_UNFINISHED_ID [%llu]\n", sr_handler->speech_info->first_unfinished_id);
        
        if (ID <= sr_handler->speech_info->first_unfinished_id || info.flags == SPEECH_INFO_FLAG_PROCESS_UNCONDITIONALLY) {
            Natural_Language_Event ev;
            ev.event_type = NATURAL_LANGUAGE_EVENT_TYPE_NEW_TRANSCRIPTION_FINAL;
            ev.ID = ID;
            ev.timestamp = time_seconds;
            ev.idx = info.nl_info->doc->doc_queue.size() - 1;
            
            Natural_Language_push_finished_event(sr_handler->speech_info, &ev);
            Natural_Language_push_all_finished_deferred_events(sr_handler->speech_info);
        } else {
            MTT_error("%s\n", "So this case does happen somehow");
            Natural_Language_Event ev;
            ev.event_type = NATURAL_LANGUAGE_EVENT_TYPE_NEW_TRANSCRIPTION_FINAL;
            ev.ID = ID;
            ev.timestamp = time_seconds;
            ev.idx = info.nl_info->doc->doc_queue.size() - 1;
            
            Natural_Language_push_deferred_event(sr_handler->speech_info, &ev);
        }
    } else {
        MTT_error("%s", "ERROR: impossible case\n");
    }
}

void speech_on_reset(void* handler)
{
    Speech_Recognition_Handler* sr_handler = (Speech_Recognition_Handler*)(((Data_Handler*)handler)->data);
    
    Speech_Info* info = sr_handler->speech_info;
    
    Speech_Info_reset(info);
    
    info->first_unfinished_id = Speech_Info_most_recent_record_id(info) + 1;
}
#undef SPEECH_DEBUG_PRINT

//#define AUDIO_TEST
#ifdef AUDIO_TEST
// audio test
#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_waveform* pSineWave;
    
    
    
    assert(pDevice->playback.channels == DEVICE_CHANNELS);
    
    pSineWave = (ma_waveform*)pDevice->pUserData;
    assert(pSineWave != NULL);
    
    ma_waveform_read_pcm_frames(pSineWave, pOutput, frameCount);
    
    (void)pInput;   /* Unused. */
}

void data_callback_playback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }
    
    f64 master_pan_value = 0.0;
    static f64 pan_range = 200.0;
    #define PAN_FOR_RIGHT(val) ((val / pan_range) + 0.5)
    #define PAN_FOR_LEFT(val) (1 - PAN_FOR_RIGHT(val))
    f64 pan[2] = {PAN_FOR_LEFT(master_pan_value), PAN_FOR_RIGHT(master_pan_value)};
    
    auto frames_read = ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);
    
    
    
    for (usize frame = 0; frame < frames_read; ++frame) {
        for (usize channel = 0; channel < 2; ++channel) {
            ((float*)pOutput)[(frame * 2) + channel] *= pan[channel];
        }
    }
    
    if (frames_read < frameCount) {
        ma_uint64 diff = frameCount - frames_read;
        ma_decoder_seek_to_pcm_frame(pDecoder, 0);
        
        for (usize frame = 0; frame < diff; ++frame) {
            for (usize channel = 0; channel < 2; ++channel) {
                ((float*)pOutput)[(frame * 2) + channel] *= pan[channel];
            }
        }

    }
    
    (void)pInput;
}

///
ma_waveform sineWave;
ma_device_config deviceConfig;
ma_device device;
ma_waveform_config sineWaveConfig;

ma_result result;
ma_decoder decoder;

float64 get_sample_rate(void)
{
    return 48000;
}

uint64 get_channel_count(void)
{
    return [[AVAudioSession sharedInstance] outputNumberOfChannels];
}

#endif

usize query_ID = 1;


inline static void main_print(void)
{
#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
    MTT_print("F=[%s] L=[%u]\n\n", __PRETTY_FUNCTION__, __LINE__);
#else
    MTT_print("F=[%s] L=[%u]\nmax_memory=[%zu]\n", __PRETTY_FUNCTION__, __LINE__, os_proc_available_memory());
#endif
}

void MTT_main(MTT_Core_Platform* core_platform, const void* config)
{
    main_print();
    
    const auto* app_config = (__bridge const Application_Launch_Configuration*)config;
    
    mem::set_main_allocator(&core_platform->core.allocator);
    core_platform->core.page_size = get_page_size();
    
    core_platform->core.application_configuration.system_directory = [[app_config systemDirectory] UTF8String];

    // MARK: - set-up speech recognition
    // Toby TODO: - need to fix initialization when requesting for the first time -- try "asking permission" in step 1, then explicitly initialize in another step

    core_platform->speech = [[SpeechRecognition alloc] init];
    SpeechRecognition* sr = core_platform->speech;
    [sr setCallback: speech_recognition_callback: nullptr];
    [SpeechRecognition staticfuncWithSr:sr];
    

    // MARK: set-up speech and natural language handling
    Speech_System_init(&core_platform->core.speech_system, (__bridge void*)core_platform->speech);
    Speech_Recognition_Handler callback_settings = {
        .handle_hypothesis = handle_speech_hypothesis,
        .handle_result     = handle_speech_result,

        .speech_info   = &core_platform->core.speech_system.info_list[0],
        .speech_system = &core_platform->core.speech_system,
        .core          = &core_platform->core,
        .core_platform = core_platform,
        .handle_amplitude_change = [](void* handler, float amplitude) {
            Speech_Recognition_Handler* sr_handler = (Speech_Recognition_Handler*)handler;
            
            
            
            if (![NSThread isMainThread]) {
                __block float amplitude_val = amplitude;
                dispatch_async(dispatch_get_main_queue(), ^{
                    sr_handler->speech_system->amplitude = amplitude_val;
                });
            } else {
                //MTT_print("amplitude: %f\n", amplitude);
                sr_handler->speech_system->amplitude = amplitude;
            }
        }
    };
    Speech_Recognition_Handler* speech_handler = Speech_Info_init(&core_platform->core.speech_system.info_list[0], callback_settings, &core_platform->core.allocator);
    speech_handler->nlp_handler.handler = handle_nlp_result;
    speech_handler->nlp_handler.on_reset = speech_on_reset;
    speech_handler->nlp_handler.data = (void*)speech_handler;
    [core_platform->speech addSubscriber:speech_handler];
    
    [sr run];
    
#ifdef AUDIO_TEST
   do { // audio test
                
        NSString *resources_directory = [[NSBundle mainBundle] resourcePath];
        
        NSLog(@"%@\n", resources_directory);
        NSString *audio = [resources_directory stringByAppendingString:@"/rdb.wav"];
        const char *const audio_str = [audio cStringUsingEncoding:NSUTF8StringEncoding];

        ma_decoder_config conf;
        conf = ma_decoder_config_init(ma_format_f32, (uint32)get_channel_count(), get_sample_rate());
        
        result = ma_decoder_init_file(audio_str, &conf, &decoder);
       if (result != MA_SUCCESS) {
           break;
       }
        
        deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.playback.format   = decoder.outputFormat;
        deviceConfig.playback.channels = decoder.outputChannels;
        deviceConfig.sampleRate        = decoder.outputSampleRate;
        deviceConfig.dataCallback      = data_callback_playback;
        deviceConfig.pUserData         = &decoder;

        
       if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
           ma_decoder_uninit(&decoder);
           break;
       }
        
       if (ma_device_start(&device) != MA_SUCCESS) {
           ma_device_uninit(&device);
           ma_decoder_uninit(&decoder);
           
           break;
       }
        
   } while (0);
#endif
    
    core_platform->bg_queue = dispatch_queue_create("com.DrawTalk", DISPATCH_QUEUE_SERIAL);
    

    
    dispatch_queue_attr_t user_initiated_qos_attribute = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, 0);
    
    core_platform->bg_text_io_queue = dispatch_queue_create("com.Make_The_Thing.text", user_initiated_qos_attribute);
    core_platform->bg_image_io_queue = dispatch_queue_create("com.Make_The_Thing.image_asset", user_initiated_qos_attribute);
    
    dispatch_queue_attr_t queue_bg_attribute = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_BACKGROUND, 0);
    core_platform->bg_logging_queue = dispatch_queue_create("com.Make_The_Thing.test_logging", user_initiated_qos_attribute);
    
    
//    dispatch_queue_attr_t vis_qos_attribute = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_UTILITY, 0);
//    
//    
//    core_platform->bg_vis_queue = dispatch_queue_create("com.DrawTalk.vis", vis_qos_attribute);



    // MARK: - setup networking
    NSString* ip_address_host = [app_config serverIP];
    UInt16 port_host          = [[app_config serverPort] unsignedIntegerValue];
    
    
    

    //NSLog(@"Trying to connect to host=[%@] port=[%u]\n", ip_address_host, port_host);

    core_platform->net_client = [[TCP_Client alloc] initWithHost: ip_address_host port:port_host selfID:1];
    core_platform->net_client_msgs = [[TCP_Client alloc] initWithHost: ip_address_host port:port_host + 1 selfID:1];

    client = core_platform->net_client;
    [client subscribe:&speech_handler->nlp_handler];
    
    client_msgs = core_platform->net_client_msgs;
    

    TCP_Client* client = core_platform->net_client;
    client->async_socket = [[GCDAsyncSocket alloc] initWithDelegate:client delegateQueue:dispatch_get_main_queue()];
    [client->async_socket setIPv4PreferredOverIPv6:NO];
    [client->async_socket setIPv6Enabled:YES];
    client->user_data = core_platform;
    
    
//    core_platform->vis_client = [[TCP_Client alloc] initWith: ip_address_host andWith:port_host + 10];
//    TCP_Client* vis_client = core_platform->vis_client;
//    vis_client->async_socket = [[GCDAsyncSocket alloc] initWithDelegate:vis_client delegateQueue:core_platform->bg_vis_queue];
//    [vis_client->async_socket setIPv4PreferredOverIPv6:NO];
//    [vis_client->async_socket setIPv6Enabled:YES];
//    vis_client->user_data = core_platform;
    
//    core_platform->core.visualize = [](MTT_Core* core, const mtt::String& as_string) {
//        MTT_Core_Platform* core_platform = (MTT_Core_Platform*)core->core_platform;
//        TCP_Client* client = core_platform->vis_client;
//
//        static uint64 tag = 0;
//
//        NSString* msg = [NSString stringWithCString:as_string.c_str() encoding:[NSString defaultCStringEncoding]];
//        NSString* text_msg = [@"VISG" stringByAppendingFormat:@"%@", msg];
//        [client write:[text_msg dataUsingEncoding:NSUTF8StringEncoding] withTag:tag];
//
//        tag += 1;
//    };
    
    
    core_platform->core.load_remote_text = [](MTT_Core* core, mtt::String message) -> usize {
        MTT_Core_Platform* core_platform = (MTT_Core_Platform*)core->core_platform;
        TCP_Client* client = core_platform->net_client;
        
        static usize ID = 0;
        
        @autoreleasepool {
            NSString* path = [NSString stringWithCString:message.c_str() encoding:[NSString defaultCStringEncoding]];
            [client load:path];
        }
        
        ID += 1;
        
        return ID;
    };
    
    
    core_platform->core.query_for_data = [](MTT_Core* core, const mtt::String& message, void (*process_new_ID)(void*, usize), void* ctx) -> usize {
        MTT_Core_Platform* core_platform = (MTT_Core_Platform*)core->core_platform;
        TCP_Client* client = core_platform->net_client;
        //ASSERT_MSG(mtt::is_main_thread(), "Should be main thread\n");
        
        
        MTT_print("QUERY ID: %llu\n", query_ID);
        usize ID_to_send = query_ID;
        process_new_ID(ctx, ID_to_send);
        query_ID += 1;

        @autoreleasepool {
            
            NSString* plat_key = [NSString stringWithCString:message.c_str() encoding:[NSString defaultCStringEncoding]];
            NSString* msg = [@"SYNQ" stringByAppendingFormat:@"%llu&%@", ID_to_send, plat_key];
            [client query:msg];
        }
        
        
        
        return ID_to_send;

    };
    
    core_platform->core.send_message_drawtalk_server[DRAWTALK_SERVER_PARALLEL_COMMAND_INDEX_DEFAULT] = [](MTT_Core* core, const mtt::String& message_type, const mtt::String& message)
    {
        MTT_Core_Platform* core_platform = (MTT_Core_Platform*)core->core_platform;
        TCP_Client* client = core_platform->net_client;
        
        @autoreleasepool {
            NSString* msg = [@"MESG" stringByAppendingFormat:@"%s&%s", message_type.c_str(), message.c_str()];
            [client sendMessage: msg];
        }
    };
    
    client->on_query_result = [](TCP_Client* client, void* data, usize ID) {
        //ASSERT_MSG(mtt::is_main_thread(), "Should be main thread!");
        
        MTT_print("Query ID arrived %llu\n", ID);
        
        NSString* actual_data = (__bridge NSString*)data;
        const char* data_c_string = [actual_data cStringUsingEncoding: NSUTF8StringEncoding];
        mtt::String data_cpp_string = mtt::String(data_c_string);
        
        dt::on_query_result(data_cpp_string, ID);
    };
     
    client->on_load = [](TCP_Client* client, void* path, void* data) {
        
        NSString* actual_path = (__bridge NSString*)path;
        const char* path_c_string = [actual_path cStringUsingEncoding:NSUTF8StringEncoding];
        NSString* actual_data = (__bridge NSString*)data;
        const char* data_c_string = [actual_data cStringUsingEncoding: NSUTF8StringEncoding];
        
        mtt::String path_cpp_string = mtt::String(path_c_string);
        mtt::String data_cpp_string = mtt::String(data_c_string);
        
        //dt::on_load(path_cpp_string, data_cpp_string);
        auto* core = mtt_core_ctx();
        auto* conf = &core->application_configuration;
        Application_Configuration_init_from_string(conf, &data_cpp_string);
        Application_Configuration_reapply(conf, (void*)core);
    };
    
    {
        NSError *error = nil;

        [client->async_socket connectToHost:ip_address_host onPort:port_host error:&error];
        if (error != nil) {
            NSLog(@"error=[%@]\n", [error localizedDescription]);
        } else {
            NSLog(@"client (main) success!!!!!!!!!");
        }
    }
//    {
//        NSError *error = nil;
//
//        [vis_client->async_socket connectToHost:ip_address_host onPort:port_host + 10 error:&error];
//        if (error != nil) {
//            NSLog(@"error=[%@]\n", [error localizedDescription]);
//        } else {
//            NSLog(@"success!!!!!!!!!");
//        }
//    }
    
    // flecs threading api
    //posix_set_os_api();
    
    //core_platform->core.allocator.deallocate_all = default_heap_deallocate_all;
    
    core_platform->core.view_transform = mat4(1.0f);
    core_platform->core.view_position = vec3(0.0f, 0.0f, 0.0f);
    
    
    core_platform->core.application_configuration.host = {
        .ip = mtt::String([ip_address_host UTF8String]),
        .port = port_host
    };
    
    TCP_Client* net_client_msgs = core_platform->net_client_msgs;
    net_client_msgs->async_socket = [[GCDAsyncSocket alloc] initWithDelegate:net_client_msgs delegateQueue:dispatch_get_main_queue()];
    [net_client_msgs->async_socket setIPv4PreferredOverIPv6:NO];
    [net_client_msgs->async_socket setIPv6Enabled:YES];
    net_client_msgs->user_data = core_platform;
    
    core_platform->core.send_message_drawtalk_server[DRAWTALK_SERVER_PARALLEL_COMMAND_INDEX_QUICK_COMMAND] = [](MTT_Core* core, const mtt::String& message_type, const mtt::String& message)
    {
        MTT_Core_Platform* core_platform = (MTT_Core_Platform*)core->core_platform;
        TCP_Client* client = core_platform->net_client_msgs;
        
        @autoreleasepool {
            NSString* msg = [@"MESG" stringByAppendingFormat:@"%s&%s", message_type.c_str(), message.c_str()];
            [client sendMessage: msg];
        }
    };
    
    {
        NSError *error = nil;

        [net_client_msgs->async_socket connectToHost:ip_address_host onPort:(port_host + 1) error:&error];
        if (error != nil) {
            NSLog(@"error=[%@]\n", [error localizedDescription]);
        } else {
            NSLog(@"client (msgs) success!!!!!!!!!");
        }
    }
}

//bool WEE = false;
void MTT_on_frame(MTT_Core_Platform* core_platform)
{
    {
        SpeechRecognition* speech = core_platform->speech;
        [speech on_update];
    }
//    if (!WEE) {
//        [core_platform->vc presentViewController:core_platform->vc->picker animated:YES completion:nil];
//    }
    // called per world update
    mtt::on_frame(&core_platform->core);
}

void MTT_on_exit(MTT_Core_Platform* core_platform)
{
    MTT_println_s("MTT_on_exit called");
#ifdef AUDIO_TEST
    if (ma_device_stop(&device) != MA_SUCCESS) {
        MTT_println_s("Failed to stop playback device.");
    } else {
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
    }
#endif
    for (usize i = 0; i < core_platform->core.speech_system.info_count; i += 1) {
//        Speech_set_active_state(&core_platform->core.speech_system.info_list[i], false);
        SpeechRecognition* speech_rec = (__bridge SpeechRecognition*)(core_platform->core.speech_system.info_list[i].system_callbacks.speech_system->backend);
        [speech_rec stop];
    }
    mtt::on_exit(&core_platform->core);
    
    MTT_Core_deinit(&core_platform->core);
    
    mtt::File_System_deinit_platform();
    MTT_println_s("MTT_on_exit done");
}

void MTT_on_resize(MTT_Core_Platform* core_platform, vec2 new_size)
{
    mtt::on_resize(&core_platform->core, new_size);
}


