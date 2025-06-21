//
//  make_the_thing_main.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/11/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef make_the_thing_main_h
#define make_the_thing_main_h

extern_link_begin()

#include <simd/simd.h>

#if defined(__OBJC__)

#import <Foundation/Foundation.h>
#if !TARGET_OS_OSX
#import <UIKit/UIKit.h>
#import <ARKit/ARKit.h>
#else
#import <AppKit/AppKit.h>
#endif
#import <ModelIO/ModelIO.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#import <Speech/Speech.h>
#import "TCP_Client.h"
#import "extension_server.h"
#include "IP_Address.h"

extern id<MTLDevice> mtl_device();
#endif

struct MTT_Core_Platform;

void MTT_main(MTT_Core_Platform* core_platform, const void* config);
void MTT_on_frame(MTT_Core_Platform* core_platform);
void MTT_on_exit(MTT_Core_Platform* core_platform);
void MTT_on_resize(MTT_Core_Platform* core_platform, vec2 new_size);

extern_link_end()

#endif /* make_the_thing_main_h */
