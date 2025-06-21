//
//  clipboard_platform.m
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/23/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "clipboard_platform.hpp"

void Clipboard_paste_platform(const mtt::String& string)
{
    @autoreleasepool {
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        [[NSPasteboard generalPasteboard] clearContents];
        NSString* as_platform_string = [NSString stringWithUTF8String:string.c_str()];
        [[NSPasteboard generalPasteboard] setString: as_platform_string forType:NSPasteboardTypeString];
#elif MTT_PLATFORM == MTT_PLATFORM_TYPE_IOS
        NSString* as_platform_string = [NSString stringWithUTF8String:string.c_str()];
        [[UIPasteboard generalPasteboard] setString:as_platform_string];
#endif
    }
}


mtt::String Clipboard_copy_platform(void)
{
    @autoreleasepool {
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        
        NSString* out_val = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
        if (out_val == nil) {
            return "";
        } else {
           return [out_val UTF8String];
        }
#elif MTT_PLATFORM == MTT_PLATFORM_TYPE_IOS
        if ([[UIPasteboard generalPasteboard] hasStrings]) {
            auto * out_str = [[UIPasteboard generalPasteboard] string];
            if (out_str) {
                return [out_str UTF8String];
            } else {
                return "";
            }
        } else {
            return "";
        }
#endif
    return "";
    }
}
