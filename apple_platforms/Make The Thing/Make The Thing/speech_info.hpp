//
//  speech_info.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/29/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef speech_info_hpp
#define speech_info_hpp

#include "make_the_thing.h"

#include "speech_recognition_handler.hpp"

#include "speech_timestamp.h"

#include "speech_system_events.h"

//struct Transcript_Word_Record {
//    //NSString* text;
//    float64 timestamp;
//};

struct Speech_Segment {
    mtt::String text;
    Speech_Timestamp system_timestamp;
};



struct Natural_Language_Data {
    usize byte_count_;
    bool is_in_use = false;
    std::deque<ArduinoJson::DynamicJsonDocument> doc_queue;
    
    ~Natural_Language_Data(void) 
    {
        for (auto it = this->doc_queue.begin(); it != this->doc_queue.end(); ++it) {
            (*it).clear();
        }
        this->doc_queue.clear();
    }
};
struct Natural_Language_Info {
    usize   ID;
    float64 timestamp_created;
    float64 timestamp_changed;
    bool is_changed;
    bool marked_final;
    bool is_valid;
    bool is_question;
    
    bool should_mark_final;
    
    
    Natural_Language_Data* doc;
    usize modification_count;
    
    //bool word_timestamps_ready;
    //std::vector<Speech_Segment> speech_segments;
    //usize word_timestamp_count;
    //std::vector<mtt::String> words;
    
    static mem::Pool_Allocation nl_data_pool;
    
    
};

inline static void Natural_Language_Data_deinit(Natural_Language_Data** nl)
{
    if (nl == nullptr || (*nl == nullptr)) {
        return;
    }
    
    mem::deallocate<Natural_Language_Data>(&Natural_Language_Info::nl_data_pool.allocator, *nl);
    *nl = nullptr;
}

template<typename T>
struct less_than_key
{
    inline bool operator() (const T& struct1, const T& struct2)
    {
        return (struct1.ID < struct2.ID);
    }
};

bool Natural_Language_Info_update_with_JSON(Natural_Language_Info* info, void* data, usize byte_length, float64 time_seconds);

struct Text_Info {
    usize   ID;
    float64 timestamp_created;
    float64 timestamp_changed;
    bool is_changed;
    bool marked_final;
    
    std::string text;
    std::string modified_text;
    usize modification_count;
};

struct Speech_Info_View {
    usize                  ID;
    Text_Info*             text_info;
    Natural_Language_Info* nl_info;
    bool                   is_valid;
    uint64 flags = 0;
};

typedef enum SPEECH_EVENT_TYPE {
    SPEECH_EVENT_TYPE_NEW_TRANSCRIPTION_HYPOTHESIS,
    SPEECH_EVENT_TYPE_NEW_TRANSCRIPTION_FINAL,
    
    SPEECH_EVENT_TYPE_COUNT
} SPEECH_EVENT_TYPE;

typedef enum NATURAL_LANGUAGE_EVENT_TYPE {
    NATURAL_LANGUAGE_EVENT_TYPE_NEW_TRANSCRIPTION_HYPOTHESIS,
    NATURAL_LANGUAGE_EVENT_TYPE_NEW_TRANSCRIPTION_FINAL,
    
    NATURAL_LANGUAGE_EVENT_TYPE_COUNT
} NATURAL_LANGUAGE_EVENT_TYPE;

const uint64 SPEECH_INFO_FLAG_PROCESS_UNCONDITIONALLY = 0x1;

struct Speech_Event {
    SPEECH_EVENT_TYPE event_type;
    usize ID;
    float64 timestamp;
};

struct Natural_Language_Event {
    NATURAL_LANGUAGE_EVENT_TYPE event_type;
    usize ID;
    float64 timestamp;
    usize idx;
};

struct Speech_Info {
    std::deque<Text_Info>             text_info;
    std::deque<Natural_Language_Info> nl_info;
    struct Entry {
        Text_Info text_info = {};
        Natural_Language_Info nl_info = {};
    };
    mtt::Map<usize, Speech_Info::Entry> info_map = {};
    
    usize least_recent_id;
    usize most_recent_id;
    usize first_unfinished_id;
    
    usize count;
    Speech_Recognition_Handler system_callbacks;
    
    std::deque<Speech_Event> speech_event_queue;
    std::deque<Natural_Language_Event> event_queue;
    std::deque<Natural_Language_Event> deferred_event_queue;
};

void Text_Info_print(Text_Info*);
void Natural_Language_Info_print(Natural_Language_Info*);

void Speech_Info_reset(Speech_Info* info);
void Speech_Info_print(Speech_Info*);
void Speech_Info_push_record(Speech_Info*, usize, float64);

Speech_Info_View Speech_Info_get_record(Speech_Info*, usize);
usize Speech_Info_most_recent_record_id(Speech_Info*);
usize Speech_Info_least_recent_record_id(Speech_Info*);
Speech_Info_View Speech_Info_most_recent_record(Speech_Info*);
Speech_Info_View Speech_Info_least_recent_record(Speech_Info*);
Speech_Info_View Speech_Info_ID_to_record(Speech_Info* info, usize id);
usize Speech_Info_record_count(Speech_Info*);

void Speech_Info_reduce_history_to(Speech_Info*, isize);

Speech_Info_View Speech_Info_nearest_to_time(Speech_Info*, float64, float64, bool*);


Speech_Recognition_Handler* Speech_Info_init(Speech_Info*, Speech_Recognition_Handler, mem::Allocator*);


struct Speech_System {
    Speech_Info info_list[1];
    usize info_count = 0;
    
    void* backend = nullptr;
    
    float amplitude = 0.0;
};

void Speech_System_init(Speech_System*, void*);

void Natural_Language_push_deferred_event(Speech_Info* info, Natural_Language_Event* event);

void Natural_Language_push_all_finished_deferred_events(Speech_Info* info);

void Natural_Language_push_finished_event(Speech_Info* info, Natural_Language_Event* event);

void Natural_Language_push_deferred_event(Speech_System* system, Natural_Language_Event* event, usize id);


void Natural_Language_push_event(Speech_Info* info, Natural_Language_Event event, usize id);

void Natural_Language_push_event(Speech_System* system, Natural_Language_Event event, usize id);

void Speech_push_event(Speech_Info* info, Speech_Event* event);

void Speech_push_event(Speech_System* system, Speech_Event* event, usize id);


bool Natural_Language_poll_event(Speech_Info* info, Natural_Language_Event* event);

void Natural_Language_clear_events(Speech_Info* info);

bool Speech_poll_event(Speech_Info* info, Speech_Event* event);

void Speech_clear_events(Speech_Info* info);

bool Speech_split(Speech_Info* speech_info);
bool Speech_discard(Speech_Info* speech_info);
bool currently_talking_is_likely(Speech_Info* speech_info);
void Speech_set_active_state(Speech_Info* speech_info, bool state);
bool Speech_get_active_state(Speech_Info* speech_info);

void Speech_reset(Speech_Info* speech_info);

void handle_speech_result_manual(mtt::String msg, Speech_Recognition_Handler* sr_handler);
void handle_speech_hypothesis_manual(mtt::String msg, Speech_Recognition_Handler* sr_handler);

void Speech_send_override(Speech_Info* speech_info, mtt::String& msg, int mode);

//void Speech_set_ignore_state(Speech_Info* speech_info, bool do_ignore);

void Speech_register_event_callback(Speech_Info* speech_info, SPEECH_SYSTEM_EVENT event, void (*callback)(SPEECH_SYSTEM_EVENT event, Speech_System_Event_Status status, void* data), void* data);

void Speech_unregister_event_callback(Speech_Info* speech_info, SPEECH_SYSTEM_EVENT event, void* data);

usize Speech_recognition_ID(Speech_Info* speech_info);

usize Speech_recognition_modification_count(Speech_Info* speech_info);

void Speech_process_text_as_input(Speech_Info* speech_info, const mtt::String& text, uint64 flags);


void Speech_recognition_increment_ID(Speech_Info* speech_info);

void Speech_recognition_set_ID(Speech_Info* speech_info, usize new_id);

void Speech_recognition_reset_state(Speech_Info* speech_info);


#endif /* speech_info_hpp */
