//
//  recording.hpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/22/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef recording_hpp
#define recording_hpp

#include "thing_shared_types.hpp"

namespace dt {

struct Recording_State {
    uint64 t_start;
    uint64 t_start_event_offset;
    //uint64 t_end;
    
    mtt::Transform transform;
    mtt::Transform absolute_transform;
    vec2           velocity;
};
struct Recording_Event {
    uint64 t_start;
    uint64 t_end;
    vec3 offset;
    vec3 first_position;
    mtt::Transform transform;
    bool is_ready;
    std::vector<Recording_State> states;
    
    void normalize()
    {
        
    }
};
struct Recording_Info {
    std::vector<std::vector<Recording_Event>> event_tracks;
};

struct Selection_Recording_State {
    mtt::Thing_ID ID;
    uint64        t_start;
    float64       t_start_timestamp;
    UI_Event      event;
    bool is_duplicate;
};

struct Location_Recording_State {
    vec3 world_position = vec3(0.0f);
    vec2 canvas_position = vec2(0.0f);
    uint64        t_start;
    float64       t_start_timestamp;
};

void Selection_Recording_State_print(Selection_Recording_State* st);


typedef std::deque<Selection_Recording_State> Selection_Recording_State_Queue;
typedef std::deque<Location_Recording_State> Location_Recording_State_Queue;
struct Selection_Recording {
    Selection_Recording_State_Queue selections;
    usize idx_playhead;
    bool enabled = true;
    Location_Recording_State_Queue location_selections;
};
struct Recorder {
    mtt::Map<mtt::Thing_ID, Recording_Info> thing_records;
    Selection_Recording selection_recording;
    Selection_Recording saved_selection_recording;
};

void Selection_Recorder_clear_selections(Selection_Recording* s);
void Selection_Recorder_clear_selections_except_for_thing(Selection_Recording* s, mtt::Thing_ID thing_id);
void Selection_Recorder_clear_location_selections(Selection_Recording* s);
void Selection_Recorder_clear_all_selection_types(Selection_Recording* s);

mtt::Thing* Selection_Recorder_get_next_unlabeled_Thing(Selection_Recording* s, usize* idx);
mtt::Thing* Selection_Recorder_get_next_labeled_Thing(Selection_Recording* s, usize* idx);
mtt::Thing* Selection_Recorder_get_next_Thing(Selection_Recording* s, usize* idx);

mtt::Thing* Selection_Recorder_get_last_labeled_Thing(Selection_Recording* s);
mtt::Thing* Selection_Recorder_get_last_unlabeled_Thing(Selection_Recording* s);

bool Selection_Recorder_get_all_unlabeled_Things(Selection_Recording* s, std::vector<mtt::Thing*>& list);
bool Selection_Recorder_get_all_labeled_Things(Selection_Recording* s, std::vector<mtt::Thing*>& list);

template<typename T>
struct time_start_less_than
{
    inline bool operator() (const T& struct1, const T& struct2)
    {
        return (struct1.t_start <= struct2.t_start);
    }
};

struct Recording_Status {
    Recording_Event* event;
    Recording_State* state;
};

Recording_Status  Recorder_begin(Recorder* recorder, mtt::Thing_ID id, uint64 t_start);
Recording_Status  Recorder_changed(Recorder* recorder, mtt::Thing_ID id, uint64 t_now);

Recording_Status  Recorder_end(Recorder* recorder, mtt::Thing_ID id, uint64 t_end);

struct Selection {
    mtt::Thing_ID      thing;
    mtt::Collider*     selection_region;
    mtt::COLLIDER_TYPE selection_region_type;
    vec2 offset;
    usize count;
    uint64 time;
};


struct DrawTalk;


void Recording_Event_print(Recording_Event* event);

struct Recording_Event_List {
    usize count;
    Recording_Event* events;
};

Recording_Event_List Recording_Events_for_thing(Recorder* recorder, mtt::Thing_ID thing_id);
Recording_Event* Recording_Event_latest(Recorder* recorder, mtt::Thing_ID thing_id);

void push_speech_event(MTT_Core* core, mtt::World* world, DrawTalk* dt, Speech_Event* ev, Speech_Info_View* sp_view);

void push_language_event(MTT_Core* core, mtt::World* world, DrawTalk* dt, Natural_Language_Event* ev, Speech_Info_View* sp_view);




void record_selection(DrawTalk* dt, mtt::Thing_ID thing_id, uint64 mtt_time, float64 sys_timestamp, UI_Event* event);

void record_location_selection(DrawTalk* dt, vec3 world_position, vec2 canvas_position, uint64 mtt_time, float64 sys_timestamp);

void record_selection_if_unlabeled(DrawTalk* dt, mtt::Thing_ID thing_id, uint64 mtt_time, float64 sys_timestamp, UI_Event* event);

void erase_selection(DrawTalk* dt, mtt::Thing_ID thing_id);

isize find_selection_position(DrawTalk* dt, mtt::Thing_ID thing);


void record_thing_creation(DrawTalk* dt, mtt::Thing_ID thing_id, uint64 event_time);
void record_thing_deletion(DrawTalk* dt, mtt::Thing_ID thing_id, uint64 event_time);

bool selection_is_recorded(mtt::Thing* thing);

void Selection_Recording_enable(void);
void Selection_Recording_disable(void);
bool Selection_Recording_is_enabled(void);

void* selections(void);

void print_thing_selections(DrawTalk* c);



}

#endif /* recording_hpp */
