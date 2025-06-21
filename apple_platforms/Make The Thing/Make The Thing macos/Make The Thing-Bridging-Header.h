//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//

#include "IP_Address.h"

#import "MTTViewController.hpp"

#include "types_common.h"
#include "c_common.h"

//#include "speech_recognition_handler.hpp"
#include "speech_timestamp.h"

#include "speech_system_events.h"

#import "speech_recognition_objc.hpp"

#include "time_common.h"


#include "mtt_bridge_api.h"



extern_link_begin()

#include "video_recording.h"

//#include "string_intern.h"

struct MTT_Core_Platform;
struct MTT_Core_Platform* mtt_core_platform_ctx(void);
struct MTT_Core;
struct MTT_Core* mtt_core_ctx(void);




extern_link_end()







