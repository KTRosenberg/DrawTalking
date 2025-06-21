//
//  video_recording.h
//  Make The Thing macos
//
//  Created by Toby Rosenberg on 7/28/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef video_recording_h
#define video_recording_h

struct SD_Image;

#ifdef __cplusplus
extern "C" {
#endif



typedef const void* MTT_Video_Recording_Context_Ref;


struct Video_Recording_Context_Info;
typedef void (*On_Data_Proc)(struct Video_Recording_Context_Info*);

typedef struct MTT_Video_Recording_Context_Args {
    void* image;
    void* images;
    void* user_data;
    void* renderer;
    On_Data_Proc on_data;
    
} MTT_Video_Recording_Context_Args;



void MTT_Video_Recording_init();

bool MTT_Video_Recording_is_init(void);



MTT_Video_Recording_Context_Ref MTT_Video_Recording_Context_make(MTT_Video_Recording_Context_Args*);

void MTT_Video_Recording_Context_destroy(MTT_Video_Recording_Context_Ref);

void MTT_Video_Recording_discover_devices(MTT_Video_Recording_Context_Ref);

void MTT_Video_Recording_start(MTT_Video_Recording_Context_Ref);
void MTT_Video_Recording_stop(MTT_Video_Recording_Context_Ref);

typedef struct Video_Recording_Context_Info {
    void* image;
    void* images;
    void* user_data;
    void* renderer;
    On_Data_Proc on_data;
} Video_Recording_Context_Info;

#if defined(__OBJC__)
@interface MTT_Video_Recording_Context : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate> {
    @public Video_Recording_Context_Info info;
}
@end
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace sd {
struct Image;
}

namespace mtt {
typedef MTT_Video_Recording_Context_Args Video_Recording_Context_Args;

void Video_Recording_init(void);
bool Video_Recording_is_init(void);
void Video_Recording_discover_devices(MTT_Video_Recording_Context_Ref ctx);
MTT_Video_Recording_Context_Ref Video_Recording_Context_make(MTT_Video_Recording_Context_Args*);

void Video_Recording_Context_destroy(MTT_Video_Recording_Context_Ref);


sd::Image* Video_Recording_get_image(MTT_Video_Recording_Context_Ref);

bool Video_Recording_has_device(MTT_Video_Recording_Context_Ref ctx);
void Video_Recording_start(MTT_Video_Recording_Context_Ref);
void Video_Recording_stop(MTT_Video_Recording_Context_Ref);

}
#endif

#endif /* video_recording_h */
