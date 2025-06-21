//
//  video_recording.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/30/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//


#include "video_recording_platform.hpp"





#ifdef __cplusplus
extern "C" {
#endif

void MTT_Video_Recording_init()
{
    mtt::Video_Recording_init();
}

bool MTT_Video_Recording_is_init(void)
{
    return mtt::Video_Recording_is_init();
}

MTT_Video_Recording_Context_Ref MTT_Video_Recording_Context_make(MTT_Video_Recording_Context_Args* args)
{
    return mtt::Video_Recording_Context_make(args);
}

void Video_Recording_Context_destroy(MTT_Video_Recording_Context_Ref ctx)
{
    mtt::Video_Recording_Context_destroy(ctx);
}

void MTT_Video_Recording_discover_devices(MTT_Video_Recording_Context_Ref ctx)
{
    return mtt::Video_Recording_discover_devices(ctx);
}

void MTT_Video_Recording_start(MTT_Video_Recording_Context_Ref ctx)
{
    mtt::Video_Recording_start(ctx);
}
void MTT_Video_Recording_stop(MTT_Video_Recording_Context_Ref ctx)
{
    mtt::Video_Recording_stop(ctx);
}


#ifdef __cplusplus
}


#endif
