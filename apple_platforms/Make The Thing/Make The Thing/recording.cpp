//
//  recording.cpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/22/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "recording.hpp"
#include "drawtalk.hpp"

namespace dt {

Recording_Status Recorder_begin(Recorder* recorder, mtt::Thing_ID id, uint64 t_start)
{
    //MTT_print("begin recording t=%llu\n", t_start);
    auto result = recorder->thing_records.find(id);
    if (result == recorder->thing_records.end()) {
        auto info = recorder->thing_records.emplace(id, Recording_Info());
        //Recording_Info* r_info = info->sec
        Recording_Info* r_info = &info.first->second;
        
        r_info->event_tracks.emplace_back(std::vector<Recording_Event>());
        
        auto* track = &r_info->event_tracks.back();
        
        track->emplace_back(Recording_Event());
        
        auto* event = &track->back();
        
        event->t_start = t_start;
        event->states.emplace_back(Recording_State());
        event->is_ready = false;
        
        auto* state = &event->states.back();
        state->t_start = t_start;
        state->t_start_event_offset = 0;
        
        return {event, state};
    } else {
        Recording_Info* r_info = &result->second;
        
        auto* track = &r_info->event_tracks[0];
        
        track->emplace_back(Recording_Event());
        
        auto* event = &track->back();
        
        event->t_start = t_start;
        event->states.emplace_back(Recording_State());
        event->is_ready = false;
        
        auto* state = &event->states.back();
        state->t_start = t_start;
        state->t_start_event_offset = 0;
        
        return {event, state};
    }
}

Recording_Status Recorder_changed(Recorder* recorder, mtt::Thing_ID id, uint64 t_now)
{
    //MTT_print("changed recording t=%llu\n", t_now);
    auto result = recorder->thing_records.find(id);
    Recording_Info* r_info = &result->second;
    
    auto* track = &r_info->event_tracks[0];
    
    auto* event = &track->back();
    
    
    event->states.emplace_back(Recording_State());
    auto* state = &event->states.back();
    state->t_start = t_now;
    state->t_start_event_offset = state->t_start - event->t_start;
    
    
    return {event, state};
}

Recording_Status Recorder_end(Recorder* recorder, mtt::Thing_ID id, uint64 t_end)
{
    //MTT_print("ended recording t=%llu\n", t_end);
    
    auto result = recorder->thing_records.find(id);
    Recording_Info* r_info = &result->second;
    
    auto* track = &r_info->event_tracks[0];
    
    auto* event = &track->back();
    event->t_end = t_end;
    event->is_ready = true;
    
    event->states.emplace_back(Recording_State());
    auto* state = &event->states.back();
    state->t_start = t_end;
    state->t_start_event_offset = state->t_start - event->t_start;
    
    
    return {event, state};
}

void Selection_Recorder_clear_selections(Selection_Recording* s)
{
    s->idx_playhead = 0;
    s->selections.clear();
}

void Selection_Recorder_clear_selections_except_for_thing(Selection_Recording* s, mtt::Thing_ID thing_id)
{
    s->idx_playhead = 0;
    
    for (usize i = 0; i < s->selections.size();) {
        if ((s->selections.begin() + i)->ID == thing_id) {
            i += 1;
        } else {
            s->selections.erase(s->selections.begin() + i);
        }
    }
}


void Selection_Recorder_clear_all_selection_types(Selection_Recording* s)
{
    Selection_Recorder_clear_selections(s);
    Selection_Recorder_clear_location_selections(s);
}

mtt::Thing* Selection_Recorder_get_next_unlabeled_Thing(Selection_Recording* s, usize* idx)
{
    if (s->selections.empty() || *idx >= s->selections.size()) {
        return nullptr;
    }
    
    usize off = *idx;
    
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    mtt::World* mtt = dt->mtt;
    for (auto it = s->selections.begin() + (*idx); it != s->selections.end();) {
        mtt::Thing* thing = mtt->Thing_try_get(it->ID);
        if (thing == nullptr) {
            //it = s->selections.erase(it);
            ++it;
            off += 1;
        } else {
            {
                auto it_words = dt->lang_ctx.dictionary.thing_to_word.find(thing->id);
                if (it_words != dt->lang_ctx.dictionary.thing_to_word.end() && it_words->second.size() == 1) {
                    // found unlabeled
                    //s->selections.erase(it);
                    *idx = off;
                    return thing;
                } else if (thing->archetype_id == mtt::ARCHETYPE_NUMBER || thing->archetype_id == mtt::ARCHETYPE_TEXT) {
                    // found unlabeled
                    *idx = off;
                    return thing;
                }
            }
            ++it;
            off += 1;
        }
    }
    
    *idx = off;
    return nullptr;
}

mtt::Thing* Selection_Recorder_get_next_labeled_Thing(Selection_Recording* s, usize* idx)
{
    if (s->selections.empty() || *idx >= s->selections.size()) {
        return nullptr;
    }
    
    usize off = *idx;
    
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    mtt::World* mtt = dt->mtt;
    for (auto it = s->selections.begin() + (*idx); it != s->selections.end();) {
        mtt::Thing* thing = mtt->Thing_try_get(it->ID);
        if (thing == nullptr) {
            //it = s->selections.erase(it);
            ++it;
            off += 1;
        } else {
            {
                auto it_words = dt->lang_ctx.dictionary.thing_to_word.find(thing->id);
                if (it_words != dt->lang_ctx.dictionary.thing_to_word.end() && it_words->second.size() > 1) {
                    // found unlabeled
                    //s->selections.erase(it);
                    *idx = off;
                    return thing;
                }
            }
            ++it;
            off += 1;
        }
    }
    
    *idx = off;
    return nullptr;
}

mtt::Thing* Selection_Recorder_get_next_Thing(Selection_Recording* s, usize* idx)
{
    if (s->selections.empty() || *idx >= s->selections.size()) {
        return nullptr;
    }
    
    usize off = *idx;
    
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    mtt::World* mtt = dt->mtt;
    for (auto it = s->selections.begin() + (*idx); it != s->selections.end();) {
        mtt::Thing* thing = mtt->Thing_try_get(it->ID);
        if (thing == nullptr) {
            //it = s->selections.erase(it);
            ++it;
            off += 1;
        } else {
            {
                *idx = off;
                return thing;
            }
            ++it;
            off += 1;
        }
    }
    
    *idx = off;
    return nullptr;
}

mtt::Thing* Selection_Recorder_get_last_unlabeled_Thing(Selection_Recording* s)
{
    if (s->selections.empty()) {
        return nullptr;
    }
    
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    mtt::World* mtt = dt->mtt;
    for (auto it = s->selections.rbegin(); it != s->selections.rend();) {
        mtt::Thing* thing = mtt->Thing_try_get(it->ID);
        if (thing == nullptr) {
            //it = s->selections.erase(it);
            ++it;
            
        } else {
            {
                auto it_words = dt->lang_ctx.dictionary.thing_to_word.find(thing->id);
                if (it_words != dt->lang_ctx.dictionary.thing_to_word.end() && it_words->second.size() == 1) {
                    return thing;
                }
            }
            ++it;
        }
    }
    
    return nullptr;
}

bool Selection_Recorder_get_all_unlabeled_Things(Selection_Recording* s, std::vector<mtt::Thing*>& list)
{
    usize initial_size = list.size();
    if (s->selections.empty()) {
        return false;
    }
    
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    mtt::World* mtt = dt->mtt;
    for (auto it = s->selections.begin(); it != s->selections.end();) {
        mtt::Thing* thing = mtt->Thing_try_get(it->ID);
        if (thing == nullptr) {
            //it = s->selections.erase(it);
            ++it;
            
        } else {
            {
                auto it_words = dt->lang_ctx.dictionary.thing_to_word.find(thing->id);
                if (it_words != dt->lang_ctx.dictionary.thing_to_word.end() && it_words->second.size() == 1) {
                    list.push_back(thing);
                }
            }
            ++it;
        }
    }
    
    return list.size() != initial_size;
}

mtt::Thing* Selection_Recorder_get_last_labeled_Thing(Selection_Recording* s)
{
    if (s->selections.empty()) {
        return nullptr;
    }
    
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    mtt::World* mtt = dt->mtt;
    for (auto it = s->selections.rbegin(); it != s->selections.rend();) {
        mtt::Thing* thing = mtt->Thing_try_get(it->ID);
        if (thing == nullptr) {
            //it = s->selections.erase(it);
            ++it;
            
        } else {
            {
                auto it_words = dt->lang_ctx.dictionary.thing_to_word.find(thing->id);
                if (it_words != dt->lang_ctx.dictionary.thing_to_word.end() && it_words->second.size() > 1) {
                    return thing;
                }
            }
            ++it;
        }
    }
    
    return nullptr;
}

bool Selection_Recorder_get_all_labeled_Things(Selection_Recording* s, std::vector<mtt::Thing*>& list)
{
    if (s->selections.empty()) {
        return false;
    }
    
    usize initial_size = list.size();
    
    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    mtt::World* mtt = dt->mtt;
    for (auto it = s->selections.begin(); it != s->selections.end();) {
        mtt::Thing* thing = mtt->Thing_try_get(it->ID);
        if (thing == nullptr) {
            //it = s->selections.erase(it);
            ++it;
            
        } else {
            {
                auto it_words = dt->lang_ctx.dictionary.thing_to_word.find(thing->id);
                if (it_words != dt->lang_ctx.dictionary.thing_to_word.end() && it_words->second.size() > 1) {
                    list.push_back(thing);
                }
            }
            ++it;
        }
    }
    
    return initial_size != list.size();
}




void Recording_Event_print(Recording_Event* event)
{
    return;
    
    MTT_print("(Recording_Event) {\n\t"
              "tstart_sec=[%f]tend_sec=[%f]\n",
              (float32)event->t_start / 1000.0,
              (float32)event->t_end / 1000.0
              );
    
    for (usize i = 0; i < event->states.size(); i += 1) {
        
        MTT_print("\t(State) {\n\t\t"
                  "tstart_sec=[%f] tstart_off_sec=[%f]\n\t\t\t"
                  "rel_position=[%f,%f] abs_position=[%f,%f]\n\t\t}\n",
                  (float32)event->states[i].t_start / 1000.0,
                  (float32)event->states[i].t_start_event_offset / 1000.0,
                  event->states[i].transform.translation[0],
                  event->states[i].transform.translation[1],
                  event->states[i].absolute_transform.translation[0],
                  event->states[i].absolute_transform.translation[1]
                  );
    }
    MTT_print("%s", "}\n");
}

Recording_Event_List Recording_Events_for_thing(Recorder* recorder, mtt::Thing_ID thing_id)
{
    auto result = recorder->thing_records.find(thing_id);
    if (result != recorder->thing_records.end() ||
        result->second.event_tracks[0].size() == 0) {
        
        usize i = 0;
        for (; i < result->second.event_tracks[0].size(); i += 1) {
            if (!result->second.event_tracks[0][i].is_ready) {
                Recording_Event_List list = {
                    i,
                    &result->second.event_tracks[0][0]
                };
                return list;
            }
        }
        Recording_Event_List list = {
            i,
            &result->second.event_tracks[0][0]
        };
        return list;
    }
    
    return { .count = 0, .events = nullptr };
}

Recording_Event* Recording_Event_latest(Recorder* recorder, mtt::Thing_ID thing_id)
{
    auto result = recorder->thing_records.find(thing_id);
    if (result != recorder->thing_records.end()) {
        int64 i = result->second.event_tracks[0].size() - 1;
        Recording_Event* out = &result->second.event_tracks[0][i];
        for (; i >= 0 && !out->is_ready; i -= 1) {
            out = &result->second.event_tracks[0][i];
        }
        if (out->is_ready) {
            return out;
        }
    }
    
    return nullptr;
    
}


void Selection_Recording_State_print(Selection_Recording_State* st)
{
    MTT_print("(Selection_Recording_State){\n\t"
              "thing ID[%llu]\n\t"
              "t_ms    [%llu]\n"
              "}\n",
              st->ID, st->t_start);
}

void record_selection_if_unlabeled(DrawTalk* dt, mtt::Thing_ID thing_id, uint64 mtt_time, float64 sys_timestamp, UI_Event* event)
{
    auto it_words = dt->lang_ctx.dictionary.thing_to_word.find(thing_id);
    if (it_words != dt->lang_ctx.dictionary.thing_to_word.end() && it_words->second.size() == 1) {
        record_selection(dt, thing_id, mtt_time, sys_timestamp, event);
    }
}

void record_location_selection(DrawTalk* dt, vec3 world_position, vec2 canvas_position, uint64 mtt_time, float64 sys_timestamp)
{
    if (!dt::Selection_Recording_is_enabled()) {
        return;
    }
    
    Recorder& rc = dt->recorder;
    Location_Recording_State st; {
        st.world_position = world_position;
        st.canvas_position = canvas_position;
        st.t_start = mtt_time;
        st.t_start_timestamp = sys_timestamp;
    }
    rc.selection_recording.location_selections.push_back(st);
}

void Selection_Recorder_clear_location_selections(Selection_Recording* s)
{
    s->location_selections.clear();
}

void record_selection(DrawTalk* dt, mtt::Thing_ID thing_id, uint64 mtt_time, float64 sys_timestamp, UI_Event* event)
{
    if (!dt::Selection_Recording_is_enabled()) {
        return;
    }
        
        
        
    Recorder& rc = dt->recorder;
//    // always record background selection
//    if (thing_id == mtt::Thing_ID_INVALID) {
//        Selection_Recording_State st; {
//            st.ID = thing_id;
//            st.t_start = mtt_time;
//            st.t_start_timestamp = sys_timestamp;
//            st.event = *event;
//        }
//
//        Recorder& rc = dt->recorder;
//        rc.selection_recording.selections.emplace_back(st);
//
//        //Selection_Recording_State_print(&rc.selection_recording.selections.back());
//
//        return;
//    }
    
    if (dt->selection_map.find(thing_id) != dt->selection_map.end()) {
        return;
    }
    
    
    // allow duplicates?
        if (rc.selection_recording.selections.size() > 0 &&
            rc.selection_recording.selections.back().ID == thing_id) {
            return;
        }
    
    {

        bool is_duplicate = false;
        if (find_selection_position(dt, thing_id) != -1) {
            is_duplicate = true;
        }
        
        Selection_Recording_State st; {
            st.ID = thing_id;
            st.t_start = mtt_time;
            st.t_start_timestamp = sys_timestamp;
            st.event = *event;
            st.is_duplicate = is_duplicate;
        }
        rc.selection_recording.selections.emplace_back(st);
        
        //Selection_Recording_State_print(&rc.selection_recording.selections.back());
    }
    
}

void erase_selection(DrawTalk* dt, mtt::Thing_ID thing_id)
{
    Recorder& rc = dt->recorder;
    auto& selections = rc.selection_recording.selections;
    for (auto it = selections.begin(); it != selections.end();) {
        if (it->ID == thing_id) {
            it = rc.selection_recording.selections.erase(it);
        } else {
            ++it;
        }
    }
}

isize find_selection_position(DrawTalk* dt, mtt::Thing_ID thing)
{
    Recorder& rc = dt->recorder;
    auto& selections = rc.selection_recording.selections;
    isize idx = 0;
    for (auto it = selections.begin(); it != selections.end(); ++it, idx += 1) {
        if (it->ID == thing) {
            return idx;
        }
    }
    return -1;
}



void record_thing_creation(DrawTalk* dt, mtt::Thing_ID thing_id, uint64 event_time)
{
    // TODO: //
}
void record_thing_deletion(DrawTalk* dt, mtt::Thing_ID thing_id, uint64 event_time)
{
    // TODO: //
}


bool selection_is_recorded(mtt::Thing* thing)
{
    mtt::Thing_ID ID = thing->id;
    
    auto& selections = dt::DrawTalk::ctx()->recorder.selection_recording.selections;
    for (auto it = selections.begin(); it != selections.end(); ++it) {
        if (it->ID == ID) {
            return true;
        }
    }
    return false;
}

void Selection_Recording_enable(void)
{
    dt::DrawTalk::ctx()->recorder.selection_recording.enabled = true;
}
void Selection_Recording_disable(void)
{
    dt::DrawTalk::ctx()->recorder.selection_recording.enabled = false;
}
bool Selection_Recording_is_enabled(void)
{
    return dt::DrawTalk::ctx()->recorder.selection_recording.enabled; // && dt::get_confirm_phase() == dt::CONFIRM_PHASE_SPEECH;
}

void* selections(void)
{
    return &dt::DrawTalk::ctx()->recorder.selection_recording.selections;
}

void print_thing_selections(DrawTalk* c)
{
    Recorder& rc = c->recorder;
    for (const auto& selection : rc.selection_recording.selections) {
        MTT_print("SELECTIONS_ID=[%llu]\n", selection.ID);
    }
}



}
