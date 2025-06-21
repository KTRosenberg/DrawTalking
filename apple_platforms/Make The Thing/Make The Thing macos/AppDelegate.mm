//
//  AppDelegate.m
//  Make The Thing macos
//
//  Created by Toby Rosenberg on 11/7/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#import "AppDelegate.h"



#import "MTTViewController.hpp"
#import "sd_metal_renderer.h"


@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    
    
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
//    NSWindow
//    SD_Renderer_Metal_Backend* renderer = (SD_Renderer_Metal_Backend*)[[self window] delegate];
//    MTTViewController* mtt_vc = renderer->_core_platform->vc;
//    [mtt_vc deinit];

    
    NSArray<NSWindow*>* windows = [[NSApplication sharedApplication] windows];
    for (NSWindow* window in windows) {
        //window.delegate = self;
        if (window.delegate != nil && ([window.delegate isKindOfClass:[SD_Renderer_Metal_Backend class]])) {
            [((SD_Renderer_Metal_Backend*)window.delegate)->_core_platform->vc deinit];
        }
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    //NSLog(@"Application did become active");

    NSArray<NSWindow*>* windows = [[NSApplication sharedApplication] windows];
    for (NSWindow* window in windows) {
        //window.delegate = self;
        if (window.delegate != nil && ([window.delegate isKindOfClass:[SD_Renderer_Metal_Backend class]])) {
            MTT_Core* core = (MTT_Core*)&(((SD_Renderer_Metal_Backend*)window.delegate)->_core_platform->core);
            if (core->on_application_focus) {
                core->on_application_focus(core);
                break;
            }
        }
    }

}

@end
