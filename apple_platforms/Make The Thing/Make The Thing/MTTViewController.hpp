//
//  MTTViewController.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/12/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//




#import <simd/simd.h>
#import <ModelIO/ModelIO.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#import <Speech/Speech.h>
#import <Foundation/Foundation.h>
#include "types_common.h"
#import "camera_types.h"
#import <Photos/Photos.h>
#import <PhotosUI/PhotosUI.h>
#import <WebKit/WebKit.h>
#import <SafariServices/SafariServices.h>

#if !TARGET_OS_OSX
#import <UIKit/UIKit.h>
#import <ARKit/ARKit.h>
#else
#import <AppKit/AppKit.h>
#endif

NS_ASSUME_NONNULL_BEGIN

#if !TARGET_OS_OSX

@interface MTTViewController :
UIViewController <ARSessionDelegate, UIPencilInteractionDelegate> {
    @public UIImageView* image_view;
    @public UIImagePickerController* picker;
    @public SFSafariViewController* web_browser;
    @public NSMutableArray* main_gesture_recognizers;
}

@property(strong,nonatomic) WKWebView* web_view;
@property(strong,nonatomic) UIView* web_view_container;
@property(strong,nonatomic) UIView* header_view;
@property(nonatomic) BOOL is_init;
@property(nonatomic) UITextView* debug_console_view;
@property(nonatomic) UIFont* debug_console_view_default_font;


- (BOOL) prefersStatusBarHidden;
- (BOOL) prefersHomeIndicatorAutoHidden;

-(nonnull instancetype)init;

- (void) objc_on_frame;

- (void) post_init;

- (void) deinit;

- (void) initializeImagePicker:(UIImagePickerController*)picker;

- (void) exitWebBrowser:(id)sender;

- (void) presentWebBrowser:(NSURL*)url;

- (ARSession*) ARSession_get;

- (BOOL) ARKit_load;
- (void) ARKit_unload;


//- (NSString*) get_debug_visualization;
- (void) share:(id)sender;


//- (void) session:(ARSession *)session didUpdateAnchors:(NSArray<__kindof ARAnchor *> *)anchors;

- (void) did_enter_background;
- (void) will_enter_foreground;


@end


#else


@interface MTTViewController :
NSViewController {
    @public NSImageView* image_view;
    @public NSMutableArray* main_gesture_recognizers;
}

@property(strong,nonatomic) WKWebView* web_view;
@property(strong,nonatomic) NSView* web_view_container;
@property(strong,nonatomic) NSView* header_view;
@property(nonatomic) BOOL is_init;
@property(nonatomic) NSTextView* debug_console_view;
@property(nonatomic) NSFont* debug_console_view_default_font;



- (BOOL) prefersStatusBarHidden;
- (BOOL) prefersHomeIndicatorAutoHidden;

-(nonnull instancetype)init;

- (void) objc_on_frame;

- (void) post_init;

- (void) deinit;

- (void) exitWebBrowser:(id)sender;

- (void) presentWebBrowser:(NSURL*)url;

//- (NSString*) get_debug_visualization;
- (void) share:(id)sender;

@end



#endif


NS_ASSUME_NONNULL_END

#if TARGET_OS_OSX
static bool find_host_window_bounds(CGRect* _Nonnull screenspace_bounds);
#endif

