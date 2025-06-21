//
//  web_browser_support.mm
//  Make The Thing
//
//  Created by Toby Rosenberg on 10/16/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "web_browser_support.hpp"
#include "web_browser_support_platform.h"

namespace web {

void present_web_browser(std::string url)
{
#if !TARGET_OS_OSX
    MTTViewController* root_controller = (MTTViewController*)[[(AppDelegate*)
                                                            [[UIApplication sharedApplication]delegate] window] rootViewController];
    NSLog(@"{NSString URL as string: %@}", [NSString stringWithUTF8String:url.c_str()]);
    [root_controller presentWebBrowser:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]];
#else
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]];
#endif
}

}
