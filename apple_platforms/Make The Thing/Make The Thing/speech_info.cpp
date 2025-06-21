//
//  speech_info.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/29/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "speech_info.hpp"
#include "speech_info_platform.hpp"



//#define DEBUG_NL_INFO

void Speech_Info_reset(Speech_Info* info)
{
    info->event_queue.clear();
    info->deferred_event_queue.clear();
    info->speech_event_queue.clear();
}


bool Natural_Language_Info_update_with_JSON(Natural_Language_Info* info, void* data, usize byte_length, float64 time_seconds)
{
    
#ifdef DEBUG_NL_INFO
    //MTT_print("update nl info %s %llu %f\n", (char*)data, byte_length, time_seconds);
#endif
    usize doc_byte_size = (64 * byte_length < 4096) ? 4096 : 64 * byte_length;

    if (info->modification_count == 0) {
#ifdef DEBUG_NL_INFO
        MTT_print("%s  %p\n", "ALLOCATING NL_INFO!", info);
#endif
    
        info->doc = mem::alloc_init<Natural_Language_Data>(&Natural_Language_Info::nl_data_pool.allocator);
    } else {
        //ASSERT_MSG(!info->doc->is_in_use, "%s\n", "data race for overwriting?");
        if (info->doc != nullptr && info->doc->is_in_use) {
            MTT_log_error("%s\n", "data race for overwriting?");
        }
        
    }
    auto* doc_queue = &info->doc->doc_queue;
    doc_queue->emplace_back(doc_byte_size);
    auto& doc = doc_queue->back();
    do {
        DeserializationError deserialization_error = ArduinoJson::deserializeJson(doc, (const char*)data);
    
        if (deserialization_error) {
            MTT_error("deserializeJson() failed: %s\n", deserialization_error.c_str());
            if (strcmp(deserialization_error.c_str(), "NoMemory") != 0) {
                return false;
            } else {
                doc.clear();
                doc_byte_size *= 2;
            }
        } else {
#ifdef DEBUG_NL_INFO
            MTT_print("received NLP data\nBEGIN{\n");
            std::cout << doc << std::endl <<
            doc.memoryUsage() << " bytes" << std::endl;
            MTT_print("\n}END\n");
#endif
            break;
        }
    } while (true);
    
    info->is_valid = true;
    MTT_print("VALID AFTER:%d\n", info->is_valid);
    return true;
}

#undef DEBUG_NL_INFO

void Speech_Info_push_record(Speech_Info* speech_info, usize id, float64 timestamp_created)
{
    MTT_print("%s\n", "calling Speech_Info_push_record");
    speech_info->text_info.emplace_back();
    Text_Info* text_info = &speech_info->text_info.back();
    text_info->ID = id;
    text_info->marked_final = false;
    text_info->is_changed   = true;
    text_info->timestamp_created = timestamp_created;
    text_info->timestamp_changed = timestamp_created;
    
    text_info->modification_count = 0;
    
    
    
    speech_info->nl_info.emplace_back();
    Natural_Language_Info* nl_info = &speech_info->nl_info.back();
    nl_info->ID                        = id;
    nl_info->is_changed                = false;
    nl_info->marked_final = false;
    nl_info->timestamp_created = -1;
    nl_info->timestamp_changed = -1;
    nl_info->is_valid = false;
    nl_info->is_question = false;
    
    nl_info->modification_count = 0;
    
    speech_info->most_recent_id = id;
    speech_info->count += 1;
    
    std::sort(speech_info->nl_info.begin(), speech_info->nl_info.end(), less_than_key<Natural_Language_Info>());
}

usize Speech_Info_most_recent_record_id(Speech_Info* info)
{
    return info->most_recent_id;
}
usize Speech_Info_least_recent_record_id(Speech_Info* info)
{
    return info->least_recent_id;
}

Speech_Info_View Speech_Info_most_recent_record(Speech_Info* info)
{
    if (info->text_info.size() == 0) {
        return {
            .is_valid = false
        };
    }
    Text_Info*             t_info  = &info->text_info.back();
    Natural_Language_Info* nl_info = &info->nl_info.back();
    
    return {
        .ID = info->most_recent_id,
        .text_info = t_info,
        .nl_info = nl_info,
        .is_valid = true
    };
}

Speech_Info_View Speech_Info_get_record(Speech_Info* info, usize ID)
{
    if (info->text_info.empty()) {
        return {
            .is_valid = false
        };
    }
    
    Text_Info*             t_info  = &*(info->text_info.begin() + ID - 1);
    Natural_Language_Info* nl_info = &*(info->nl_info.begin() + ID - 1);
    
    return {
        .ID = info->most_recent_id,
        .text_info = t_info,
        .nl_info = nl_info,
        .is_valid = true
    };
}

Speech_Info_View Speech_Info_least_recent_record(Speech_Info* info)
{
    Text_Info*             t_info  = &info->text_info.front();
    Natural_Language_Info* nl_info = &info->nl_info.front();
    
    return {
        .ID = info->least_recent_id,
        .text_info = t_info,
        .nl_info = nl_info,
        .is_valid = true
    };
}

usize Speech_Info_record_count(Speech_Info* info) {
    return info->count;
}
Text_Info override_text_info;
Natural_Language_Info override_nl_info;
Speech_Info_View Speech_Info_ID_to_record(Speech_Info* info, usize id)
{
    if (id == 0) {
        {
            override_text_info.modification_count = 0;
            override_text_info.ID = 0;
            override_text_info.marked_final = false;
            override_text_info.is_changed = true;
        }
        {
            override_nl_info.modification_count = 0;
            override_nl_info.ID = 0;
            override_nl_info.marked_final = false;
            override_nl_info.is_changed = true;
        }
        
        return {
            .ID = id,
            .text_info = &override_text_info,
            .nl_info = &override_nl_info,
            .is_valid = true,
            .flags = SPEECH_INFO_FLAG_PROCESS_UNCONDITIONALLY,
        };
    }
    
    isize idx = ((isize)id) - ((isize)info->least_recent_id) - 1;
    if (idx < 0) {
        MTT_error("%s", "ERROR: speech record discarded, id look-up canceled\n");
        return {
            .is_valid = false
        };
    } else if (idx >= info->text_info.size()) {
        MTT_error("ERROR: out of bounds, idx=[%lld]\n", idx);
        return {
            .is_valid = false
        };
    }
    
    
    MTT_print("NLP this is the id %lld and the idx %llu\n", id, idx);
    
    Text_Info*             t_info  = &(info->text_info[idx]);
    Natural_Language_Info* nl_info = &(info->nl_info[idx]);
    
    return {
        .ID = id,
        .text_info = t_info,
        .nl_info = nl_info,
        .is_valid = true
    };
}


void Speech_Info_reduce_history_to(Speech_Info* speech_info, isize new_size)
{
    return;
//    const isize old_size = speech_info->count;
//    if (new_size >= old_size) {
//        return;
//    }
//
//
//
//    const isize diff = old_size - new_size;
//
//    for (isize i = 0; i < diff; i += 1) {
//        speech_info->text_info.pop_front();
//    }
//    for (isize i = 0; i < diff; i += 1) {
//        delete speech_info->nl_info.front().doc;
//        speech_info->nl_info.pop_front();
//    }
//
//
//    speech_info->count = new_size;
//
//    speech_info->least_recent_id = (&speech_info->text_info.front())->ID;
}





Speech_Info_View Speech_Info_nearest_to_time(Speech_Info* info, float64 lower, float64 upper, bool* valid)
{
    // TODO: -
    
    Text_Info*             t_info  = &info->text_info.front();
    Natural_Language_Info* nl_info = &info->nl_info.front();
    
    *valid = false;

    return {
    };
    
}

Speech_Recognition_Handler* Speech_Info_init(Speech_Info* speech_info, Speech_Recognition_Handler handler, mem::Allocator* allocator)
{
    speech_info->least_recent_id = 0;
    speech_info->most_recent_id  = 0;
    speech_info->first_unfinished_id = 1;
    speech_info->system_callbacks = handler;
    
    mem::Pool_Allocation_init(&Natural_Language_Info::nl_data_pool, *allocator, 64, sizeof(Natural_Language_Data), 16);
    
    return &speech_info->system_callbacks;
}




void Text_Info_print(Text_Info* info)
{
    return;
    MTT_print("Text_Info{\n\t"
                "ID=[%llu],\n\t"
                "is_changed=[%d],\n\t"
                "timestamp_created=[%f],\n\t"
                "timestamp_changed=[%f],\n\t"
                "marked_final=[%d],\n\t"
                "text=[%s],\n\t"
                "modified_text=[%s],\n\t"
                "modification_count=[%llu]\n}\n",
                info->ID, info->is_changed, info->timestamp_created,
                info->timestamp_changed, info->marked_final,
                info->text.c_str(),
                info->modified_text.c_str(),
                info->modification_count
    );
}
//void Natural_Language_Info_print(Natural_Language_Info*);
void Speech_Info_print(Speech_Info* info)
{
    MTT_print("%s", "Speech_Info{\n");
    if (info->count > 0) {
        Text_Info_print(&info->text_info[info->count - 1]);
    }
    MTT_print("%s", "}\n");
}

void Speech_Info_print_all(Speech_Info* info)
{
    MTT_print("%s", "Speech_Info{\n");
    for (usize i = 0; i < info->count; i += 1) {
        Text_Info_print(&info->text_info[i]);
    }
    MTT_print("%s", "}\n");
}

void Natural_Language_Info_print(Natural_Language_Info* nl_info)
{
    MTT_error("%s", "ERROR: not implemented\n");
//    MTT_print("Natural_Language_Info{\n");
//    if (nl_info->doc->data != nullptr) {
//        std::cout << nl_info->doc->data;
//    }
//    MTT_print("}\n");

}

void Natural_Language_push_deferred_event(Speech_Info* info, Natural_Language_Event* event)
{
    info->deferred_event_queue.push_back(*event);
    
    //    std::sort(info->deferred_event_queue.begin(), info->deferred_event_queue.end(), less_than_key<Natural_Language_Event>());
}

void Natural_Language_push_all_finished_deferred_events(Speech_Info* info)
{
    std::deque<Natural_Language_Event>* q_deferred = &info->deferred_event_queue;
    std::deque<Natural_Language_Event>* q = &info->event_queue;
    
    if (q_deferred->empty()) {
        return;
    }
    
    std::sort(q_deferred->begin(), q_deferred->end(), less_than_key<Natural_Language_Event>());
    
    usize* expected = &info->first_unfinished_id;
    
    Natural_Language_Event* ev = nullptr;
    do {
        ev = &q_deferred->front();
        
        if (ev->ID <= *expected) {
            q->push_back(*ev);
            q_deferred->pop_front();
            
            *expected += 1;
        } else {
            break;
        }
        
    } while (!q_deferred->empty());
}

void Natural_Language_push_finished_event(Speech_Info* info, Natural_Language_Event* event)
{
    std::deque<Natural_Language_Event>* finalized = &info->event_queue;
    
    finalized->push_back(*event);
    
    info->first_unfinished_id += 1;
}

void Natural_Language_push_deferred_event(Speech_System* system, Natural_Language_Event* event, usize id)
{
    Speech_Info* info = &system->info_list[id];
    info->deferred_event_queue.push_back(*event);
    
    
}



void Natural_Language_push_event(Speech_Info* info, Natural_Language_Event event, usize id)
{
    info->event_queue.push_back(event);
}

void Natural_Language_push_event(Speech_System* system, Natural_Language_Event event, usize id)
{
    system->info_list[id].event_queue.push_back(event);
}

void Speech_push_event(Speech_Info* info, Speech_Event* event)
{
    info->speech_event_queue.push_back(*event);
}

void Speech_push_event(Speech_System* system, Speech_Event* event, usize id)
{
    system->info_list[id].speech_event_queue.push_back(*event);
}



void Natural_Language_clear_events(Speech_Info* info)
{
//    for (auto it = info->event_queue.begin(); it != info->event_queue.end(); ++it) {
//        auto ID = it->ID;
//        Speech_Info_View view = Speech_Info_ID_to_record(info, ID);
//        if (!view.is_valid) {
//            continue;
//        }
//        //delete view.nl_info->doc;
//        //view.nl_info->doc = nullptr;
//    }
    info->event_queue.clear();
}

bool Natural_Language_poll_event(Speech_Info* info, Natural_Language_Event* event)
{
    std::deque<Natural_Language_Event>* q = &info->event_queue;
    if (q->empty()) {
        return false;
    }
    *event = q->front();
    q->pop_front();
    return true;
}

bool Speech_poll_event(Speech_Info* info, Speech_Event* event)
{
    std::deque<Speech_Event>* q = &info->speech_event_queue;
    if (q->empty()) {
        return false;
    }
    *event = q->front();
    q->pop_front();
    return true;
}

void Speech_clear_events(Speech_Info* info)
{
    info->speech_event_queue.clear();
}

bool Speech_split(Speech_Info* speech_info)
{
    return Speech_split_platform(speech_info);
}

bool Speech_discard(Speech_Info* speech_info)
{
    return Speech_discard_platform(speech_info);
}

//void Speech_set_active_state(Speech_Info* speech_info, bool state)
//{
//    Speech_set_active_state_platform(speech_info, state);
//}

bool Speech_get_active_state(Speech_Info* speech_info)
{
    return Speech_get_active_state_platform(speech_info);
}

bool currently_talking_is_likely(Speech_Info* speech_info)
{
    return currently_talking_is_likely_platform(speech_info);
}

//void Speech_reset(Speech_Info* speech_info)
//{
//    Speech_reset_platform(speech_info);
//}

void Speech_send_override(Speech_Info* speech_info, mtt::String& msg, int mode)
{
    Speech_send_override_platform(speech_info, msg, mode);
}

//void Speech_set_ignore_state(Speech_Info* speech_info, bool do_ignore)
//{
//    Speech_set_ignore_state_platform(speech_info, do_ignore);
//}

void Speech_register_event_callback(Speech_Info* speech_info, SPEECH_SYSTEM_EVENT event, void (*callback)(SPEECH_SYSTEM_EVENT event, Speech_System_Event_Status status, void* data), void* data)
{
    Speech_register_event_callback_platform(speech_info, event, callback, data);
}

void Speech_unregister_event_callback(Speech_Info* speech_info, SPEECH_SYSTEM_EVENT event, void* data)
{
    Speech_unregister_event_callback_platform(speech_info, event, data);
}

usize Speech_recognition_ID(Speech_Info* speech_info)
{
    return Speech_recognition_ID_platform(speech_info);
}

usize Speech_recognition_modification_count(Speech_Info* speech_info)
{
    return Speech_recognition_modification_count_platform(speech_info);
}

void Speech_process_text_as_input(Speech_Info* speech_info, const mtt::String& text, uint64 flags)
{
    Speech_process_text_as_input_platform(speech_info, text, flags);
}

void Speech_recognition_increment_ID(Speech_Info* speech_info)
{
    Speech_recognition_increment_ID_platform(speech_info);
}

void Speech_recognition_set_ID(Speech_Info* speech_info, usize new_id)
{
    Speech_recognition_set_ID_platform(speech_info, new_id);
}

void Speech_recognition_reset_state(Speech_Info* speech_info)
{
    ASSERT_MSG(false, "TODO");
    Speech_recognition_reset_state_platform(speech_info);
}

mem::Pool_Allocation Natural_Language_Info::nl_data_pool = (mem::Pool_Allocation){};



