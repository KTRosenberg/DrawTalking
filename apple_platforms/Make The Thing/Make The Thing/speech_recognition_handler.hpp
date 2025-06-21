//
//  speech_recognition_handler.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/29/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef speech_recognition_handler_hpp
#define speech_recognition_handler_hpp

//#include "data_handler.h"

#include "speech_timestamp.h"

struct Speech_System;
struct Speech_Info;
struct MTT_Core;
struct MTT_Core_Platform;

typedef struct Handler_Args {
    void* args;
} Handler_Args;

struct Data_Handler {
    void* data;
    void (*handler)(void*, void*, usize, float64, usize, usize, usize, Handler_Args args);
    void (*on_reset)(void*);
};

struct Speech_Recognition_Handler {
    bool (*handle_hypothesis)(void*, void*, usize, usize, float64, float64, Handler_Args args);
    void (*handle_result)(void*, void*, usize, usize, float64, Handler_Args args);
    struct Data_Handler   nlp_handler;
    struct Speech_Info*   speech_info;
    struct Speech_System* speech_system;
    struct MTT_Core*      core;
    struct MTT_Core_Platform* core_platform;
    void (*handle_amplitude_change)(void* handler, float amplitude);
//    float32 prev_sample;
//    float32 latest_sample;
};


#endif /* speech_recognition_handler_hpp */
