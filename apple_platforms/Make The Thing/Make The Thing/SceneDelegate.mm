//
//  SceneDelegate.m
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/6/25.
//  Copyright © 2025 Toby Rosenberg. All rights reserved.
//

#import "SceneDelegate.hpp"
#import "MTTViewController.hpp"
#import "AppDelegate.h"

@interface SceneDelegate ()

@end

@implementation SceneDelegate


- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
    // Use this method to optionally configure and attach the UIWindow `window` to the provided UIWindowScene `scene`.
    // If using a storyboard, the `window` property will automatically be initialized and attached to the scene.
    // This delegate does not imply the connecting scene or session are new (see `application:configurationForConnectingSceneSession` instead).
}


- (void)sceneDidDisconnect:(UIScene *)scene {
    // Called as the scene is being released by the system.
    // This occurs shortly after the scene enters the background, or when its session is discarded.
    // Release any resources associated with this scene that can be re-created the next time the scene connects.
    // The scene may re-connect later, as its session was not necessarily discarded (see `application:didDiscardSceneSessions` instead).
    
    
    UIViewController* vc = (UIViewController*)_window.rootViewController;
    if ([vc isKindOfClass:[MTTViewController class]]) {
        MTTViewController* mtt_vc = (MTTViewController*)vc;
        [mtt_vc deinit];
    }
    NSLog(@"sceneDidDisconnect");
    //NSLog(@"%@\n", @"Quitting");
}


- (void)sceneDidBecomeActive:(UIScene *)scene {
    // Called when the scene has moved from an inactive state to an active state.
    // Use this method to restart any tasks that were paused (or not yet started) when the scene was inactive.
}


- (void)sceneWillResignActive:(UIScene *)scene {
    // Called when the scene will move from an active state to an inactive state.
    // This may occur due to temporary interruptions (ex. an incoming phone call).
}


- (void)sceneWillEnterForeground:(UIScene *)scene {
    // Called as the scene transitions from the background to the foreground.
    // Use this method to undo the changes made on entering the background.
    
    UIViewController* vc = (UIViewController*)_window.rootViewController;
    if ([vc isKindOfClass:[MTTViewController class]]) {
        MTTViewController* mtt_vc = (MTTViewController*)vc;
        [mtt_vc will_enter_foreground];
    }
}


- (void)sceneDidEnterBackground:(UIScene *)scene {
    // Called as the scene transitions from the foreground to the background.
    // Use this method to save data, release shared resources, and store enough scene-specific state information
    // to restore the scene back to its current state.
    UIViewController* vc = (UIViewController*)_window.rootViewController;
    if ([vc isKindOfClass:[MTTViewController class]]) {
        MTTViewController* mtt_vc = (MTTViewController*)vc;
        [mtt_vc did_enter_background];
    }
}

//- (void)windowScene:(UIWindowScene *)windowScene didUpdateEffectiveGeometry:(UIWindowSceneGeometry *)previousEffectiveGeometry {
// enable in iPadOS 26
//}

@end
