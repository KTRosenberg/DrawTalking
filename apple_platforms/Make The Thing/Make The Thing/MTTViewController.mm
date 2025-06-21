//
//  MTTViewController.m
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/12/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#import "MTTViewController.hpp"
#include "make_the_thing_main.h"
#ifdef __cplusplus
#include "make_the_thing.h"
#endif
#import "Make_The_Thing-Swift.h"

#define SD_IMPLEMENTATION_IOS
#include "stratadraw_platform_apple.hpp"

#import "sd_metal_renderer.h"
#define MTT_CORE_PLATFORM_IMPLEMENTATION
#include "mtt_core_platform.h"

#include "keys_platform.hpp"
#include "input.hpp"

#include "web_browser_support.hpp"


#import <Photos/Photos.h>
#import <PhotosUI/PhotosUI.h>

#include "IP_Address.h"

#include "file_system_platform.hpp"
#include "file_system_platform_private.hpp"
#import "video_recording.h"
#include "augmented_reality_platform.hpp"

#include "configuration.hpp"


static bool pencil_simulation_mode_on = false;
static bool pencil_simulation_mode_current = false;
static inline bool pencil_simulation_mode_is_on(void)
{
    return pencil_simulation_mode_on;
}
//@interface WKWebViewWindowed : WKWebView
//{
//
//}
//
//-viewDid
//
//-(nonnull instancetype)initWithFrame:(CGRect)rect configuration:(nonnull WKWebViewConfiguration*)config;
//
//@end
//
//@interface WKWebViewWindowed ()
//{
//
//
//}
//@end
//
//@implementation WKWebViewWindowed
//{
//
//
//}
//@end

//Core_OBJC core_objc;

//Application_Launch_Configuration* applicationConfig;


typedef MTTViewController_Swift* const MTTVCSwiftRef;

@interface MTTViewController () <
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
    UIGestureRecognizerDelegate,
    UINavigationControllerDelegate,
    PHPickerViewControllerDelegate,
    UIPointerInteractionDelegate,
#else
    NSGestureRecognizerDelegate,
#endif
    
    WKNavigationDelegate

    //UIImagePickerControllerDelegate,
    
>
@end


@implementation MTTViewController {
    MTKView* _mtk_view;
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    NSArray<id<MTLDevice>> *deviceList;
    NSMutableArray<id<MTLDevice>> *externalGPUs;
    NSMutableArray<id<MTLDevice>> *integratedGPUs;
    NSMutableArray<id<MTLDevice>> *discreteGPUs;
    id <NSObject> deviceObserver;
    id <NSObject> window_move_event_observer_for_external_app_following;
    id <NSObject> window_resize_event_observer_for_external_app_following;
#endif
    
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
    ARSession* _session;

    SHCamera   _cam;
    BOOL       ARKitIsSupported;
    
    UIPointerInteraction* pointer_interaction;
    
    PHPickerViewController* picker_view_controller;
#endif
    
    SD_Renderer_Metal_Backend* _renderer;
    MTT_Core_Platform          _core_platform;
    
    usize _userID;
}

- (void)windowDidMove:(NSNotification *)notification {
    int BP = 0;
}
- (void)windowDidResize:(NSNotification *)notification {
    
}

- (BOOL)prefersStatusBarHidden{
    return YES;
}

- (BOOL) prefersHomeIndicatorAutoHidden{
    return YES;
}

- (BOOL) shouldAutorotate {
    return NO;
}

#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
-(NSUInteger)supportedInterfaceOrientations {
    return UIInterfaceOrientationLandscapeRight | UIInterfaceOrientationLandscapeLeft;
}
#endif

-(nonnull instancetype)init {
    self = [super init];
    if (self) {
        self->_is_init = NO;
    }
    
    return self;
}


#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
- (ARSession*) ARSession_get {
    return self->_session;
}

- (BOOL) ARKit_load {

    self->_session = [ARSession new];
    if (self->_session != nil) {
        self->_session.delegate = self;
        [self->_renderer ARSession_set:self->_session];
        return YES;
    }
    
    return NO;
}

- (void) ARKit_unload {
    self->_session.delegate = nil;
    self->_session = nil;
    [self->_renderer ARSession_set:nil];
}

#endif

#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear: animated];
}
#else
- (void)viewDidAppear  {
    [super viewDidAppear];
}
#endif

- (void)viewDidLoad {
    [super viewDidLoad];
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
    [self setNeedsStatusBarAppearanceUpdate];
#else
    
#endif
    _userID = 0;
}

#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
#else
- (void)viewWillAppear  {
    [super viewWillAppear];
}
#endif


#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    mtt::Augmented_Reality_pause_platform(&_core_platform.core.ar_ctx);
}
#else
- (void)viewWillDisappear  {
    [super viewWillDisappear];
}
#endif




- (MTTVCSwiftRef)swift_vc {
    return (MTTVCSwiftRef)self;
}


-(void)objc_on_frame {
}

// MARK: post-initialization setup

#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
-(void)didRotateDeviceChangeNotification:(NSNotification *)notification
{
    UIDevice* ui_dev = notification.object;
    
    
    UIInterfaceOrientation newOrientation = [((UIWindowScene*)[[[UIApplication sharedApplication] connectedScenes] anyObject]) interfaceOrientation];
     
    if ((newOrientation == UIInterfaceOrientationLandscapeLeft || newOrientation == UIInterfaceOrientationLandscapeRight))
     {

     }
}
#endif

-(void)post_init {
    @autoreleasepool {

        // Do any additional setup after loading the view.
        
        //NSLog(@"initializing objc++ entry");
        
        //NSLog(@"Testing NDJson deserialization");
        
        //#define JSON_TEST_1
    #if JSON_TEST_1
        
        ArduinoJson::DynamicJsonDocument doc(200);
        
        char json[] =
        "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, json);
        // Test if parsing succeeds.
        if (error) {
            printf("deserializeJson() failed: %s\n", error.c_str());
            goto cleanup;
        }
        
        {
            // Fetch values.
            //
            // Most of the time, you can rely on the implicit casts.
            // In other case, you can do doc["time"].as<long>();
            const char* sensor = doc["sensor"];
            long time = doc["time"];
            double latitude = doc["data"][0];
            double longitude = doc["data"][1];
            
            // Print values.
            printf("JSON values : sensor=[%s], time=[%ld], latitute=[%f], longitude=[%f]\n", sensor, time, latitude, longitude);
        }
        
        MTT_main(&_core_platform);
        
        return;
        
    cleanup:
        doc.clear();
    #else
    #if (0)
        
        //    char json[] =
        //        "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
        //
        //    NSURLSessionConfiguration* defaultSessionConfiguration = [NSURLSessionConfiguration defaultSessionConfiguration];
        //    NSURLSession* defaultSession = [NSURLSession sessionWithConfiguration:defaultSessionConfiguration];
        
        NSString* sketch_request_path_str = [[NSString alloc] initWithFormat:@"%@/%@.ndjson", @"https://storage.googleapis.com/quickdraw_dataset/full/simplified", @"cat"];
        
        NSString* sketch_request_path_bin = [[NSString alloc] initWithFormat:@"%@/%@.bin", @"https://storage.googleapis.com/quickdraw_dataset/full/binary", @"cat"];
        
    https://storage.cloud.google.com/quickdraw_dataset/full/binary/cat.bin
        NSMutableURLRequest *request = [[NSMutableURLRequest alloc] init];
        [request setURL:[NSURL URLWithString:sketch_request_path_str]];
        [request setHTTPMethod:@"GET"];
        
        NSURLSession *session = [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration]];
        [[session dataTaskWithRequest:request completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
            NSLog(@"Received reply\n");
            
            if (error) {
                NSLog(@" error=[%@]\n", [error localizedDescription]);
                return;
            }
            NSString *requestReply = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            const size_t len = [data length]; //[requestReply lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
            const char* as_cstr = [requestReply UTF8String];
            ArduinoJson::DynamicJsonDocument doc(len);
            //std::cout << "LEN: " << strlen(as_cstr) << std::endl;
            {
                DeserializationError deserialization_error = ArduinoJson::deserializeJson(doc, as_cstr);
                if (deserialization_error) {
                    printf("deserializeJson() failed: %s\n", deserialization_error.c_str());
                }
                
                JsonArray drawing = doc["drawing"].as<JsonArray>();
                
                {
                    [[self core_objc] tag_to_thing][@"WEE"] = [[Multicurve_OBJC alloc]initWithName:@"WEE"];
                    CGMutablePathRef path = ((Multicurve_OBJC*)[[self core_objc] tag_to_thing][@"WEE"]).strokes;
                    
                    usize i = 0;
                    for (auto stroke_it = drawing.begin();
                         stroke_it != drawing.end();
                         stroke_it += 1, i += 1) {
                        
                        JsonArray stroke = stroke_it->as<JsonArray>();
                        std::cout << "stroke " << i << " =" << stroke << std::endl;
                        
                        JsonArray stroke_x = (*stroke.begin()).as<JsonArray>();
                        JsonArray stroke_y = (*(++stroke.begin())).as<JsonArray>();
                        std::cout <<
                        "\txcomp" << stroke_x << std::endl <<
                        "\tycomp" << stroke_y << std::endl;
                        
                        auto it_x = stroke_x.begin();
                        auto end_x = stroke_x.end();
                        auto it_y = stroke_y.begin();
                        auto end_y = stroke_y.end();
                        usize i = 0;
                        
                        //                    CGPathMoveToPoint(
                        //                    path, nil, (it_x)->as<unsigned long long>(), (it_y)->as<unsigned long long>()
                        //                    );
                        UIBezierPath* subpath = [[UIBezierPath alloc] init];
                        [subpath moveToPoint:CGPointMake((it_x)->as<unsigned long long>(), (it_y)->as<unsigned long long>())];
                        
                        i += 1;
                        it_x += 1;
                        it_y += 1;
                        for (;
                             it_x != end_x && it_y != end_y;
                             i += 1, it_x += 1, it_y += 1) {
                            
                            //                        CGPathAddLineToPoint(
                            //                            path, nil, (it_x)->as<unsigned long long>(), (it_y)->as<unsigned long long>()
                            //                        );
                            [subpath addLineToPoint:CGPointMake((it_x)->as<unsigned long long>(), (it_y)->as<unsigned long long>())];
                        }
                        
                        CGPathAddPath(path, nil, [subpath CGPath]);
                        
                        
                    }
                    
                }
            }
            doc.clear();
        }] resume];
    #endif
        
        //    // Deserialize the JSON document
        //    DeserializationError error = deserializeJson(doc, json);
        //    // Test if parsing succeeds.
        //    if (error) {
        //        printf("deserializeJson() failed: %s\n", error.c_str());
        //        goto cleanup;
        //    }
        //
        //    {
        //        // Fetch values.
        //        //
        //        // Most of the time, you can rely on the implicit casts.
        //        // In other case, you can do doc["time"].as<long>();
        //        const char* sensor = doc["sensor"];
        //        long time = doc["time"];
        //        double latitude = doc["data"][0];
        //        double longitude = doc["data"][1];
        //
        //        // Print values.
        //        printf("JSON values : sensor=[%s], time=[%ld], latitute=[%f], longitude=[%f]\n", sensor, time, latitude, longitude);
        //    }
        
        // MARK: - init view
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP

        {
            [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];

                [[NSNotificationCenter defaultCenter] addObserver:self
                                                         selector:@selector(didRotateDeviceChangeNotification:)
                                                             name:UIDeviceOrientationDidChangeNotification
                                                           object:nil];
        }
#endif
        
        _mtk_view = [[MTKView alloc] initWithFrame:self.view.frame];
        _mtk_view.framebufferOnly = false;
        
        


        _mtk_view.device = MTLCreateSystemDefaultDevice();
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP

        _mtk_view.backgroundColor = UIColor.clearColor;
#else
        
#if 0
        if (_mtk_view.device.removable)
        {
            NSLog(@"removable default GPU");
        }
        else if (_mtk_view.device.lowPower)
        {
            NSLog(@"low power default GPU");
        }
        else
        {
            NSLog(@"discrete default GPU");
        }
        
        deviceList = MTLCopyAllDevicesWithObserver(&deviceObserver,
                                                   ^(id<MTLDevice> device, MTLDeviceNotificationName name) {
            //[self handleExternalGPUEventsForDevice:device notification:name];
        });
        
        externalGPUs = [[NSMutableArray alloc] init];
        integratedGPUs = [[NSMutableArray alloc] init];
        discreteGPUs = [[NSMutableArray alloc] init];
        
        for (id <MTLDevice> device in deviceList) {
            if (device.removable)
            {
                [externalGPUs addObject:device];
            }
            else if (device.lowPower)
            {
                [integratedGPUs addObject:device];
            }
            else
            {
                [discreteGPUs addObject:device];
            }
        }
#endif
        
#endif
        
        
        
        [[self view] addSubview:_mtk_view];
        [_mtk_view layer].zPosition = -10;
        //[[self view] bringSubviewToBack:_mtk_view];
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
        [_mtk_view setUserInteractionEnabled:YES];
        [_mtk_view setMultipleTouchEnabled:YES];
#else
        self->window_move_event_observer_for_external_app_following = nil;
        self->window_resize_event_observer_for_external_app_following = nil;
#endif
        //    [[self view] setUserInteractionEnabled:YES];
        //    [[self view] setMultipleTouchEnabled:YES];
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
        _mtk_view.preferredFramesPerSecond = 120;
        
        {
            [self setDebug_console_view:[[UITextView alloc] init]];
            UITextView* tv = [self debug_console_view];
            tv.layer.zPosition = -4;
            
            [[self view] addSubview: tv];
            [tv setTextColor: [UIColor whiteColor]];
            [tv setBackgroundColor:[UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.4]];
            [self setDebug_console_view_default_font: [UIFont systemFontOfSize:18]];
            [tv setFont:self->_debug_console_view_default_font];
            tv.translatesAutoresizingMaskIntoConstraints = NO;
            {
                tv.editable = NO;
                tv.selectable = NO;
                tv.userInteractionEnabled = NO;
            }
            [[tv topAnchor] constraintEqualToAnchor:[_mtk_view topAnchor]].active = YES;
            [[tv leftAnchor] constraintEqualToAnchor:[_mtk_view leftAnchor]].active = YES;
            [[tv rightAnchor] constraintEqualToAnchor:[_mtk_view rightAnchor]].active = YES;
            tv.scrollEnabled = NO;
            
            
            
            [tv setHidden:YES];
            _core_platform.core.set_command_line_text = [](MTT_Core* core, cstring text, usize length, uint64 flags) {
                @autoreleasepool {
                    UITextView* tv = ((MTT_Core_Platform*)core->core_platform)->vc->_debug_console_view;
                    
                    if (flags == SELECT_ALL_MODE_ON) {
                        NSString* text_storage = [NSString stringWithUTF8String:text];
                        [tv setSelectedRange:NSMakeRange(0, text_storage.length + 3)];
                        return;
                    } else if (flags == SELECT_ALL_MODE_OFF) {
                        [tv setSelectedRange:NSMakeRange(0, 0)];
                        return;
                    }
//                } else if (tv.hidden) {
//                    [UIView transitionWithView:tv
//                                      duration:0.2
//                                       options:UIViewAnimationOptionTransitionCrossDissolve
//                                    animations:^{
//                        [tv setHidden:NO];
//                    }
//                                    completion:NULL];
//                }
                    if (strlen(text) == 0) {
                        [tv setHidden:YES];
                        MTT_print("%s\n", "Invalid text length of 0");
                        return;
                    }
                    if (length == 0) {
                        [tv setHidden:YES];
                        return;
                    } else if (tv.hidden) {
                        [tv setHidden:NO];
                    }
                    
                    NSString* text_storage = [NSString stringWithUTF8String:text];
                    if (text_storage == nil) {
                        return;
                    }
                    [[tv textStorage] setAttributedString:[[NSAttributedString alloc] initWithString:text_storage attributes:@{NSForegroundColorAttributeName : [UIColor whiteColor], NSBackgroundColorAttributeName : [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.4], NSFontAttributeName : ((MTT_Core_Platform*)core->core_platform)->vc->_debug_console_view_default_font
                    }]];
                    
//                    [tv.layoutManager ensureLayoutForTextContainer:tv.textContainer];
//
//                    tv.frame = [tv.layoutManager usedRectForTextContainer:tv.textContainer];
                    [tv sizeToFit];
                }
            };
        }
#else
        _mtk_view.preferredFramesPerSecond = 120;

        {
            [self setDebug_console_view:[[NSTextView alloc] init]];
            NSTextView* tv = [self debug_console_view];
            
            
            [[self view] addSubview: tv];
            [tv setTextColor: [NSColor whiteColor]];
            [tv setBackgroundColor:[NSColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.4]];
            
            [self setDebug_console_view_default_font: [NSFont systemFontOfSize:18]];
            [tv setFont:self->_debug_console_view_default_font];
            tv.translatesAutoresizingMaskIntoConstraints = NO;
            {
                tv.editable = NO;
                tv.selectable = NO;
            }
            [[tv topAnchor] constraintEqualToAnchor:[_mtk_view topAnchor]].active = YES;
            [[tv leftAnchor] constraintEqualToAnchor:[_mtk_view leftAnchor]].active = YES;
            [[tv rightAnchor] constraintEqualToAnchor:[_mtk_view rightAnchor]].active = YES;
            
            
            [tv setHidden:YES];
            _core_platform.core.set_command_line_text = [](MTT_Core* core, cstring text, usize length, uint64 flags) {
                @autoreleasepool {
                    NSTextView* tv = ((MTT_Core_Platform*)core->core_platform)->vc->_debug_console_view;
                    
                    if (flags == SELECT_ALL_MODE_ON) {
                        NSString* text_storage = [NSString stringWithUTF8String:text];
                        [tv setSelectedRange:NSMakeRange(0, text_storage.length + 3)];
                        return;
                    } else if (flags == SELECT_ALL_MODE_OFF) {                        
                        [tv setSelectedRange:NSMakeRange(0, 0)];
                        return;
                    }
                    
                    if (length == 0) {
                        [tv setHidden:YES];
                        return;
                    } else if (tv.hidden) {
                        [tv setHidden:NO];
                    }
                    
                    NSString* text_storage = [NSString stringWithUTF8String:text];
                    [[tv textStorage] setAttributedString:[[NSAttributedString alloc] initWithString:text_storage attributes:@{NSForegroundColorAttributeName : [NSColor whiteColor], NSBackgroundColorAttributeName : [NSColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.4], NSFontAttributeName : ((MTT_Core_Platform*)core->core_platform)->vc->_debug_console_view_default_font
                    }]];
                    
                    [tv.layoutManager ensureLayoutForTextContainer:tv.textContainer];
                    
                    tv.frame = [tv.layoutManager usedRectForTextContainer:tv.textContainer];
                }
            };
        }
#endif
        


//        NotificationCenter.default.addObserver(self, selector: #selector(self.windowIsKey(_:)), name: .NSWindowDidBecomeKey, object: win)
        


        
        if (!_mtk_view.device) {
            NSLog(@"Metal is not supported on this device");
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
            self.view = [[UIView alloc] initWithFrame:self.view.frame];
#else
            self.view = [[NSView alloc] initWithFrame:self.view.frame];
#endif
            return;
        }
        
      
        //[[NSPasteboard generalPasteboard] setString: as_platform_string forType:NSPasteboardTypeString];
        {
            //[[NSPasteboard generalPasteboard] clearContents];
//            mtt::String S = "This is a testssss";
//            NSString* as_platform_string = [NSString stringWithUTF8String:S.c_str()];
//            [[NSPasteboard generalPasteboard] setString: as_platform_string forType:NSPasteboardTypeString];
//            {
//                auto bla = [[[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString] UTF8String];
//                int x = 0;
//            }
        }

        
        mtt_time_init();
        
        // MARK: - init rendering
        _core_platform.vc = self;
        mem::Allocator allocator = mem::Heap_Allocator();
        MTT_Core_Platform_init(&_core_platform, &allocator);
        
        mtt::File_System_init_platform();
        
        
        // MARK: - init AR

        mtt::Augmented_Reality_init_platform(&_core_platform.core.ar_ctx, (__bridge void*)self);
        
        MTT_Video_Recording_init();

//        if constexpr ((false)) {
//        struct DT_Node {
//            unsigned long long a;
//            DT_Node* rest[3];
//        };
//
//
//
//
//        MTT_print("size of node: %lu\n", sizeof(CT_Node));
//
//        Memory_Pool_Fixed_init(&_core_platform.core.memory_pool, _core_platform.core.allocator, 4, sizeof(CT_Node), 16);
//
//        _core_platform.core.memory_pool_allocator = mem::Memory_Pool_Fixed_Allocator(&_core_platform.core.memory_pool);
//        MTT_print("Memory pool init\n");
//
//        // TEST
//        mem::Allocator& palloc = _core_platform.core.memory_pool_allocator;
//        mem::Memory_Pool_Fixed& mem_pool = _core_platform.core.memory_pool;
//        constexpr const usize COUNT = 256;
//        DT_Node* all[COUNT];
//        for (usize i = 0; i < COUNT; i += 1) {
//            all[i] = (DT_Node*)palloc.allocate(&palloc, sizeof(CT_Node));
//            all[i]->a = i + 1;
//        }
//
//        for (usize i = 0; i < COUNT; i += 1) {
//            MTT_print("node: %llu %lu", all[i]->a, (uintptr)all[i]);
//            if (i > 0) {
//                MTT_print(" off: %lu", (uintptr)all[i] - (uintptr)all[i - 1]);
//            }
//            MTT_print("\n");
//        }
//
//        for (usize i = 0; i < COUNT; i += 2) {
//            palloc.deallocate(&palloc, all[i], 1);
//        }
//        for (usize i = 0; i < COUNT; i += 2) {
//            all[i] = (CT_Node*)palloc.allocate(&palloc, sizeof(CT_Node));
//            all[i]->a = (i + 1) * 10;
//        }
//
//        for (usize i = 0; i < COUNT; i += 1) {
//            MTT_print("node: %llu\n", all[i]->a);
//        }
//        mem::Memory_Pool_Fixed_deinit(&mem_pool);
//        }
        
        
        
        
        
        
        

        
        
        
        
        

        
        // MARK: gesture recognizers
#if  !TARGET_OS_OSX
    #define GESTURE_RECOGNIZERS
        pointer_interaction = [[UIPointerInteraction alloc] initWithDelegate:self];
        [self.view addInteraction:pointer_interaction];
#elif TARGET_OS_OSX
    #define GESTURE_RECOGNIZERS_MACOS_DESKTOP
#endif
    
        main_gesture_recognizers = [[NSMutableArray alloc] init];
        
#if TARGET_OS_OSX
        

        
        
        NSPanGestureRecognizer *panGestureRecognizer = [[NSPanGestureRecognizer alloc] initWithTarget:self
                                                                                               action:@selector(panGestureWasRecognized:)];
//        panGestureRecognizer.cancelsTouchesInView = NO;
//        panGestureRecognizer.delaysTouchesBegan = NO;
//        panGestureRecognizer.delaysTouchesEnded = NO;
        panGestureRecognizer.delaysSecondaryMouseButtonEvents = NO;
        panGestureRecognizer.buttonMask = (1 << 2);
        
        //panGestureRecognizer.delegate = self;
//        [self.view addGestureRecognizer:panGestureRecognizer];
//        [main_gesture_recognizers addObject:panGestureRecognizer];
        
        NSPressGestureRecognizer * pressGestureRecognizer = [[NSPressGestureRecognizer alloc] initWithTarget:self action:@selector(pressGestureWasRecognized:)];
        pressGestureRecognizer.delaysPrimaryMouseButtonEvents = NO;
        
        pressGestureRecognizer.buttonMask = 0x2;
        pressGestureRecognizer.delaysSecondaryMouseButtonEvents = NO;
        pressGestureRecognizer.minimumPressDuration = 0.4;
        pressGestureRecognizer.delaysRotationEvents = NO;
        [self.view addGestureRecognizer:pressGestureRecognizer];
        [main_gesture_recognizers addObject:pressGestureRecognizer];
        
        
        
#endif
#if !TARGET_OS_OSX
        
        NSArray* allowed = @[[NSNumber numberWithInt : UITouchTypeDirect]];

    
        UIPanGestureRecognizer *panGestureRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self
                                                                                               action:@selector(panGestureWasRecognized:)];
        panGestureRecognizer.cancelsTouchesInView = NO;
        panGestureRecognizer.delaysTouchesBegan = NO;
        panGestureRecognizer.delaysTouchesEnded = NO;
        panGestureRecognizer.allowedTouchTypes = allowed;
        [panGestureRecognizer setMinimumNumberOfTouches:2];
        [panGestureRecognizer setMaximumNumberOfTouches:2];
        //[panGestureRecognizer requireGestureRecognizerToFail:swipeGestureRecognizer];

        panGestureRecognizer.delegate = self;
        [self.view addGestureRecognizer:panGestureRecognizer];
        [main_gesture_recognizers addObject:panGestureRecognizer];
        
#define ENABLE_SWIPE (1)
        
#if ENABLE_SWIPE
        UISwipeGestureRecognizer* swipeGestureRecognizerLeft = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeGestureRecognition:)];
        
        swipeGestureRecognizerLeft.delegate = self;
        swipeGestureRecognizerLeft.direction = UISwipeGestureRecognizerDirectionLeft;
        swipeGestureRecognizerLeft.cancelsTouchesInView = NO;
        swipeGestureRecognizerLeft.numberOfTouchesRequired = 3;
        swipeGestureRecognizerLeft.allowedTouchTypes = allowed;
        [swipeGestureRecognizerLeft requireGestureRecognizerToFail:panGestureRecognizer];
        [self.view addGestureRecognizer:swipeGestureRecognizerLeft];
        [main_gesture_recognizers addObject:swipeGestureRecognizerLeft];
        
        UISwipeGestureRecognizer* swipeGestureRecognizerRight = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeGestureRecognition:)];
        swipeGestureRecognizerRight.delegate = self;
        swipeGestureRecognizerRight.direction = UISwipeGestureRecognizerDirectionRight;
        swipeGestureRecognizerRight.cancelsTouchesInView = NO;
        swipeGestureRecognizerRight.numberOfTouchesRequired = 3;
        swipeGestureRecognizerRight.allowedTouchTypes = allowed;
        [swipeGestureRecognizerRight requireGestureRecognizerToFail:panGestureRecognizer];
        [self.view addGestureRecognizer:swipeGestureRecognizerRight];
        [main_gesture_recognizers addObject:swipeGestureRecognizerRight];
#endif
        
        UIPinchGestureRecognizer* pinchGestureRecognizer = [[UIPinchGestureRecognizer alloc] initWithTarget:self
                                                                                                     action:@selector(pinchGestureWasRecognized:)];
        pinchGestureRecognizer.cancelsTouchesInView = NO;
        pinchGestureRecognizer.delaysTouchesBegan = NO;
        pinchGestureRecognizer.delaysTouchesEnded = NO;
        pinchGestureRecognizer.allowedTouchTypes = allowed;
#if ENABLE_SWIPE
        [pinchGestureRecognizer requireGestureRecognizerToFail:swipeGestureRecognizerLeft];
        [pinchGestureRecognizer requireGestureRecognizerToFail:swipeGestureRecognizerRight];
#endif
        pinchGestureRecognizer.delegate = self;
        [self.view addGestureRecognizer:pinchGestureRecognizer];
        [main_gesture_recognizers addObject:pinchGestureRecognizer];
        

        
        
        

        UIRotationGestureRecognizer* rotationGestureRecognizer = [[UIRotationGestureRecognizer alloc] initWithTarget:self
                                                                                                              action:@selector (rotationGestureWasRecognized:)];
        rotationGestureRecognizer.cancelsTouchesInView = NO;
        rotationGestureRecognizer.delaysTouchesBegan = NO;
        rotationGestureRecognizer.delaysTouchesEnded = NO;
        rotationGestureRecognizer.allowedTouchTypes = allowed;
#if ENABLE_SWIPE
        [rotationGestureRecognizer requireGestureRecognizerToFail:swipeGestureRecognizerLeft];
        [rotationGestureRecognizer requireGestureRecognizerToFail:swipeGestureRecognizerRight];
#endif
        rotationGestureRecognizer.delegate = self;
        [self.view addGestureRecognizer:rotationGestureRecognizer];
        [main_gesture_recognizers addObject:rotationGestureRecognizer];

        
        UILongPressGestureRecognizer* longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector (longPressGestureWasRecognized:)];
        longPressGestureRecognizer.cancelsTouchesInView = NO;
        longPressGestureRecognizer.delaysTouchesBegan = NO;
        longPressGestureRecognizer.delaysTouchesEnded = NO;
        longPressGestureRecognizer.allowedTouchTypes = allowed;
        longPressGestureRecognizer.delegate = self;
        [self.view addGestureRecognizer:longPressGestureRecognizer];
        [main_gesture_recognizers addObject:longPressGestureRecognizer];



        {
            UIScreenEdgePanGestureRecognizer* edgePan = [[UIScreenEdgePanGestureRecognizer alloc] initWithTarget:self action:@selector(leftScreenEdgeSwiped:)];

            [edgePan setEdges:UIRectEdgeLeft];
            edgePan.cancelsTouchesInView = NO;
            edgePan.delaysTouchesBegan = NO;
            edgePan.delaysTouchesEnded = NO;
            edgePan.allowedTouchTypes = allowed;

            edgePan.delegate = self;

            [self.view addGestureRecognizer:edgePan];
            [main_gesture_recognizers addObject:edgePan];
        }
        {
            UIScreenEdgePanGestureRecognizer* edgePan = [[UIScreenEdgePanGestureRecognizer alloc] initWithTarget:self action:@selector(rightScreenEdgeSwiped:)];

            [edgePan setEdges:UIRectEdgeRight];
            edgePan.cancelsTouchesInView = NO;
            edgePan.delaysTouchesBegan = NO;
            edgePan.delaysTouchesEnded = NO;
            edgePan.allowedTouchTypes = allowed;

            edgePan.delegate = self;

            [self.view addGestureRecognizer:edgePan];
            [main_gesture_recognizers addObject:edgePan];
        }
        
        {
            UITapGestureRecognizer* doubleTap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(doubleTapGestureRecognition:)];
            
            doubleTap .cancelsTouchesInView = NO;
            doubleTap .delaysTouchesBegan = NO;
            doubleTap .delaysTouchesEnded = NO;
            doubleTap .allowedTouchTypes = allowed;
            
            doubleTap.numberOfTapsRequired = 2;
            
            doubleTap.delegate = self;
            [self.view addGestureRecognizer:doubleTap ];
            [main_gesture_recognizers addObject:doubleTap];
        }
        
        
        
        UIPencilInteraction* pencilInteraction = [[UIPencilInteraction alloc] init];
        pencilInteraction.delegate = self;
        [self.view addInteraction:pencilInteraction];
        
        {
            UIHoverGestureRecognizer* hover = [[UIHoverGestureRecognizer alloc] initWithTarget:self action:@selector(hoverGestureWasRecognized:)];
            hover.delegate = self;
            
            [self.view addGestureRecognizer:hover];
            [main_gesture_recognizers addObject:hover];
        }
        
    #endif
        
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
        image_view = [[UIImageView alloc] initWithFrame:self.view.frame];
#else
        image_view = [[NSImageView alloc] initWithFrame:self.view.frame];
#endif
        
        image_view.layer.zPosition = -100000;
        
        [self.view addSubview:image_view];
        
        /*
        picker = [[UIImagePickerController alloc] init];
        picker.delegate = self;
        
        picker.modalPresentationStyle = UIModalPresentationPopover;
        picker.allowsEditing = NO;
        
       // _picker.mediaTypes = @[@"public.image", @"public.movie"];
        [self initializeImagePicker:picker];
        */
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
        
        
//        PHPickerConfiguration* config = [[PHPickerConfiguration alloc] init];
//        config.selectionLimit = 0;
//        config.filter = [PHPickerFilter imagesFilter];
//        config.preferredAssetRepresentationMode = PHPickerConfigurationAssetRepresentationModeCurrent;
//        
//        picker_view_controller = [[PHPickerViewController alloc] initWithConfiguration:config];
//        picker_view_controller.delegate = self;
        
        
        //[self presentViewController:picker_view_controller animated:YES completion:nil];
#endif
        
        if constexpr ((false)) {
        

            // MARK: initialize web view
            {
                CGRect container_rect = self.view.frame;
                container_rect = CGRectMake(container_rect.origin.x, container_rect.origin.y, container_rect.size.width, container_rect.size.height);
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP

                _web_view_container = [[UIView alloc] initWithFrame:container_rect];
#else
                _web_view_container = [[NSView alloc] initWithFrame:container_rect];
#endif
                _web_view_container.accessibilityIdentifier = @"web view container";
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                _web_view_container.backgroundColor = [UIColor whiteColor];
#endif
            }
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
            _web_view_container.userInteractionEnabled = YES;
#endif
            _web_view_container.autoresizesSubviews = YES;
            //_web_view_container.translatesAutoresizingMaskIntoConstraints = NO;
            [self.view addSubview:_web_view_container];
            [_web_view_container.bottomAnchor constraintLessThanOrEqualToAnchor:self.view.bottomAnchor].active = YES;


            
            // MARK: header view for web
            {
                //header_frame.size.height = 50;
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                _header_view = [[UIView alloc] init];
#else
                _header_view = [[NSView alloc] init];
#endif
                _header_view.accessibilityIdentifier = @"header view";
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                _header_view.userInteractionEnabled = YES;
                _header_view.backgroundColor = [UIColor systemBlueColor];
#endif
                _header_view.translatesAutoresizingMaskIntoConstraints = NO;
                [_web_view_container addSubview:_header_view];
                
                [_header_view.widthAnchor constraintEqualToAnchor:_web_view_container.widthAnchor].active = YES;
                [_header_view.heightAnchor constraintEqualToConstant:50].active = YES;
                [_header_view.topAnchor constraintEqualToAnchor:_web_view_container.topAnchor].active = YES;
                [_header_view.bottomAnchor constraintEqualToAnchor:_header_view.topAnchor constant:50].active = YES;


                // MARK: - button
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                UIButton* exit_button = [UIButton buttonWithType:UIButtonTypeClose];
#else
                NSButton* exit_button = [[NSButton alloc] init];
#endif
                exit_button.accessibilityIdentifier = @"exit button";
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                exit_button.userInteractionEnabled = YES;
#endif
                exit_button.translatesAutoresizingMaskIntoConstraints = NO;
                exit_button.frame = CGRectMake(0, 0, 100, 50);
                
                
                [_header_view addSubview:exit_button];

                [exit_button.centerXAnchor constraintEqualToAnchor:_header_view.leadingAnchor constant:exit_button.frame.size.width / 2].active = YES;
                [exit_button.centerYAnchor constraintEqualToAnchor:_header_view.centerYAnchor].active = YES;

                //exit_button.layer.zPosition = 9000000;
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                [exit_button addTarget:self action:@selector(exitWebBrowser:) forControlEvents:UIControlEventTouchDown];
#else
                [exit_button setAction:@selector(exitWebBrowser:)];
#endif
                
                {
                    WKWebViewConfiguration* web_view_config =
                    [[WKWebViewConfiguration alloc] init];
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                    [web_view_config setAllowsInlineMediaPlayback:YES];
                    [web_view_config setAllowsPictureInPictureMediaPlayback:YES];
#endif
                    [web_view_config setApplicationNameForUserAgent:@"Version/1.0 Make-The-Thing-Browser/1.0"];
                    
                    
                    _web_view = [[WKWebView alloc] initWithFrame:_web_view_container.frame
                                                   configuration:web_view_config];
                    
                    _web_view.accessibilityIdentifier = @"web view";

                    _web_view.translatesAutoresizingMaskIntoConstraints = NO;
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                    _web_view.navigationDelegate = self;
#endif

                    [_web_view setAllowsBackForwardNavigationGestures:YES];
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
                    [_web_view.scrollView setShowsVerticalScrollIndicator:NO];
#endif
                    
                    [_web_view_container addSubview:_web_view];

                    [_web_view.topAnchor constraintEqualToAnchor:_header_view.bottomAnchor].active = YES;
                    [_web_view.bottomAnchor constraintEqualToAnchor:_web_view_container.bottomAnchor].active = YES;
                    [_web_view.widthAnchor constraintEqualToAnchor:_header_view.widthAnchor].active = YES;
                }
                
                
//                [_web_view.heightAnchor constraintGreaterThanOrEqualToAnchor:_web_view_container.heightAnchor].active = YES;
//                [_web_view.leftAnchor constraintEqualToAnchor:_web_view_container.leftAnchor].active = YES;
//                [_web_view.rightAnchor constraintEqualToAnchor:_web_view_container.rightAnchor].active = YES;
            }
            //[self.view addSubview:_header_view];
            

            //[_web_view.widthAnchor constraintEqualToAnchor:_header_view.widthAnchor].active = YES;
            //[_web_view.topAnchor constraintEqualToAnchor:_header_view.bottomAnchor].active = YES;
            _web_view_container.layer.zPosition = 9000000;
            [_web_view_container setHidden:YES];
            //[_header_view setHidden:YES];
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
            [_web_view_container layoutSubviews];
#else
            NSArray* windows = [NSApp windows];
            [windows[0] makeFirstResponder:[self view]];
            
#endif
            //translatesAutoresizingMaskIntoConstraints
        }
        
        
    #endif
        
        
        Application_Launch_Configuration* app_config = [[self swift_vc] get_application_launch_configuration];
        
        MTT_main(&self->_core_platform, (__bridge const void*)app_config);
        
        __block auto start = ^() {
            self->_renderer = [[SD_Renderer_Metal_Backend alloc] initWithMetalKitView:self->_mtk_view andCorePlatform:&self->_core_platform];
            
            
            CGSize mult2 = self->_mtk_view.bounds.size;
            float64 scale = sd::get_native_display_scale();
            mult2.width = mult2.width * scale;
            mult2.height = mult2.height * scale;
            [self->_renderer drawRectResized:mult2];
            
            self->_mtk_view.delegate = self->_renderer;
            //[_renderer drawRectResized:_mtk_view.bounds.size];
            
            NSLog(@"renderer backend initialized");
            
            // world set-up
            mtt::setup(&self->_core_platform.core);
            
            self->_is_init = YES;
            
            return true;
        };
        
        if (app_config.loadConfigOnStart) {
            mtt::Text_File_Load_Descriptor desc = {};
            desc.paths.push_back({});
            desc.paths.back().path = "config/init";
            desc.paths.back().extension = "json";
            
            mtt::Text_File_load_async_platform_with_block(&desc, ^(mtt::Text_File_Load_Result result, bool status) {
                if (status == true) {
                    mtt::Application_Configuration_init(&self->_core_platform.core.application_configuration, (void*)&result);
                }
                
                if (self->_core_platform.core.application_configuration.enable_speech) {
                    [SpeechRecognition requestAuthorizationWithCallback:^{
                        start();
                    }];
                } else {
                    start();
                }
                
                return true;
            });

        } else {
            if (self->_core_platform.core.application_configuration.enable_speech) {
                [SpeechRecognition requestAuthorizationWithCallback:^{
                    start();
                }];
            } else {
                start();
            }
        }
    }
}

// MARK: touch controls

#if !TARGET_OS_OSX

inline static void begin_touch_state()
{
#if TARGET_OS_MACCATALYST
    pencil_simulation_mode_current = pencil_simulation_mode_is_on();
#endif
}
inline static void end_touch_state()
{
#if TARGET_OS_MACCATALYST
    pencil_simulation_mode_current = false;
#endif
}

inline static void check_set_modifier(UIPressesEvent* event, UIPress* press)
{
#if TARGET_OS_MACCATALYST
    switch (press.key.keyCode) {
    case UIKeyboardHIDUsageKeyboardLeftShift:
    case UIKeyboardHIDUsageKeyboardRightShift: {
        pencil_simulation_mode_on = true;
        break;
    }
    default: { break; }
    }
#endif
}

inline static void check_unset_modifier(UIPressesEvent* event, UIPress* press)
{
#if TARGET_OS_MACCATALYST
    switch (press.key.keyCode) {
    case UIKeyboardHIDUsageKeyboardLeftShift:
    case UIKeyboardHIDUsageKeyboardRightShift: {
        pencil_simulation_mode_on = false;
    }
    default: { break; }
    }
#endif
}

inline static UITouchType type_switch(UITouch* touch)
{
#if TARGET_OS_MACCATALYST
    return (pencil_simulation_mode_current) ? UITouchTypePencil : touch.type;
#else
    return touch.type;
#endif
}

#elif TARGET_OS_OSX

static bool right_mouse_down = false;
static bool right_mouse_down_state = false;

- (NSPoint)mouseLocationInView:(NSEvent *)event {
    MTTViewController_Swift* vc_swift = [self swift_vc];
    auto* view = [vc_swift get_main_view];
    NSPoint locationInView = [view convertPoint:[event locationInWindow]
                                       fromView:nil];
    return locationInView;
}
- (NSPoint)mouseLocationInViewWithPoint:(NSPoint)point {
    MTTViewController_Swift* vc_swift = [self swift_vc];
    auto* view = [vc_swift get_main_view];
    NSPoint locationInView = [view convertPoint:point
                                       fromView:nil];
    return locationInView;
}

inline bool is_in_alternate_mode(void)
{
    return ([NSEvent modifierFlags] & NSEventModifierFlagShift);
}

uint32 ui_event_flags;

#define LINK_OP_IN_MOUSE_CB (true)

- (void)mouseDown:(NSEvent*)event {
    if (!self->_is_init) { return; }
#if LINK_OP_IN_MOUSE_CB
    if ((event.modifierFlags & NSEventModifierFlagShift) == NSEventModifierFlagShift) {
        if (!right_mouse_down_state) {
            [self rightMouseDown:event];
            return;
        }
    }
#endif
    Input* const input = &_core_platform.core.input;
    
    [[NSCursor crosshairCursor] set];
    
    ui_event_flags = 0;//is_in_alternate_mode() ? UI_EVENT_FLAG_CONTEXT_MODE : 0;
    
    @autoreleasepool {
        
        right_mouse_down_state = right_mouse_down;
        
        NSTimeInterval system_uptime = 0;
        
        Input_Record* input_record = &input->users[_userID];
        usize seq = input_record->pointer_sequence_number;
        input_record->pointer_sequence_number += 1;
        
        ((input_record->pointer_map))[(uintptr_t)0] = UI_Touch();
        UI_Touch* in_touch = &((input_record->pointer_map))[(uintptr_t)0];
        in_touch->state = UI_TOUCH_STATE_BEGAN;
        in_touch->sequence_number = seq;
        
        in_touch->pointer.prev_position = vec2(0.0f);
        
        in_touch->pointer.count = 0;
        
        in_touch->pointer.position_delta = vec2(0.0f);
        
        NSPoint pos = [self mouseLocationInView:event];
        
        in_touch->pointer.positions[in_touch->pointer.count] = vec2(pos.x, pos.y);
        
        in_touch->pointer.azimuth_unit_vector = vec2_64(0, 0);
        
        in_touch->type = UI_TOUCH_TYPE_POINTER;
        in_touch->key = (uintptr_t)0;
        
        in_touch->pointer.state = UI_TOUCH_STATE_BEGAN;
        in_touch->pointer.count = 1;
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_BEGAN,
            .input_type = UI_TOUCH_TYPE_POINTER,
            .key = in_touch->key,
            .timestamp = event.timestamp - system_uptime,
            .flags = ui_event_flags,
        });
    }
}
- (void)mouseDragged:(NSEvent*)event {
    if (!self->_is_init) { return; }
#if LINK_OP_IN_MOUSE_CB
    if ((event.modifierFlags & NSEventModifierFlagShift) == NSEventModifierFlagShift) {
        if (!right_mouse_down_state) {
            [self rightMouseDragged:event];
            return;
        }
    }
#endif
    
    Input* const input = &_core_platform.core.input;
    
    @autoreleasepool {
        
        NSTimeInterval system_uptime = 0;
        
        Input_Record* input_record = &input->users[_userID];
            
        //((input_record->pointer_map))[(uintptr_t)0] = UI_Touch();
        UI_Touch* in_touch = &((input_record->pointer_map))[(uintptr_t)0];
        in_touch->state = UI_TOUCH_STATE_MOVED;
        
        if (in_touch->pointer.count != 0) {
            in_touch->pointer.prev_position = in_touch->pointer.positions[in_touch->pointer.count - 1];
        } else {
            in_touch->pointer.prev_position = vec2(0.0f);
        }
        
        in_touch->pointer.count = 0;
        
        in_touch->pointer.position_delta = vec2(event.deltaX, -event.deltaY);
        
        
        NSPoint pos = [self mouseLocationInView:event];

        
        in_touch->pointer.positions[in_touch->pointer.count] = vec2(pos.x, pos.y);
        
        in_touch->pointer.azimuth = m::atan2pos_64(-event.deltaY, -event.deltaX);
        in_touch->pointer.azimuth_unit_vector = m::normalize(vec2_64(-event.deltaX, -event.deltaY));
        
        in_touch->type = UI_TOUCH_TYPE_POINTER;
        in_touch->key = (uintptr_t)0;
        
        in_touch->pointer.state = UI_TOUCH_STATE_MOVED;
        
        in_touch->pointer.count = 1;
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_MOVED,
            .input_type = UI_TOUCH_TYPE_POINTER,
            .key = in_touch->key,
            .timestamp = event.timestamp - system_uptime,
            .flags = ui_event_flags,
        });

        
    }
}
- (void)mouseUp:(NSEvent*)event {
    if (!self->_is_init) { return; }
#if LINK_OP_IN_MOUSE_CB
    if ((event.modifierFlags & NSEventModifierFlagShift) == NSEventModifierFlagShift) {
        if (!right_mouse_down_state) {
            [self rightMouseUp:event];
            return;
        }
    }
#endif
    
    Input* const input = &_core_platform.core.input;
    
    [[NSCursor arrowCursor] set];
    
    @autoreleasepool {

        NSTimeInterval system_uptime = 0;
        
        Input_Record* input_record = &input->users[_userID];
        
        //((input_record->pointer_map))[(uintptr_t)0] = UI_Touch();
        UI_Touch* in_touch = &((input_record->pointer_map))[(uintptr_t)0];
        in_touch->state = UI_TOUCH_STATE_ENDED;
        
        if (in_touch->pointer.count != 0) {
            in_touch->pointer.prev_position = in_touch->pointer.positions[in_touch->pointer.count - 1];
        } else {
            in_touch->pointer.prev_position = vec2(0.0f);
        }
        
        in_touch->pointer.count = 0;
        
        in_touch->pointer.position_delta = vec2(event.deltaX, -event.deltaY);
        
        NSPoint pos = [self mouseLocationInView:event];

        
        in_touch->pointer.positions[in_touch->pointer.count] = vec2(pos.x, pos.y);
        
        in_touch->pointer.azimuth_unit_vector = m::normalize(vec2_64(-event.deltaX, -event.deltaY));
        
        in_touch->type = UI_TOUCH_TYPE_POINTER;
        in_touch->key = (uintptr_t)0;
        
        in_touch->pointer.state = UI_TOUCH_STATE_ENDED;
        
        in_touch->pointer.count = 1;
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_ENDED,
            .input_type = UI_TOUCH_TYPE_POINTER,
            .key = in_touch->key,
            .timestamp = event.timestamp - system_uptime,
            .flags = ui_event_flags,
        });
        
    }
}
- (void)mouseMoved:(NSEvent*)event {
    if (!self->_is_init) { return; }
    
    @autoreleasepool {
        
        NSTimeInterval system_uptime = 0;
        (void)system_uptime;
        
    }
}
- (void)mouseEntered:(NSEvent*)event {
    if (!self->_is_init) { return; }
    
    @autoreleasepool {
        
        NSTimeInterval system_uptime = 0;
        (void)system_uptime;
        
    }
}
- (void)mouseExited:(NSEvent*)event {
    if (!self->_is_init) { return; }
    
    @autoreleasepool {
        
        NSTimeInterval system_uptime = 0;
        (void)system_uptime;
        
    }
}



// right

- (void)rightMouseDown:(NSEvent*)event {
    if (!self->_is_init) { return; }
    
    if (is_in_alternate_mode()) {
        [[NSCursor dragLinkCursor] set];
    } else {
        [[NSCursor closedHandCursor] set];
    }
    
    Input* const input = &_core_platform.core.input;
    @autoreleasepool {
        
        NSTimeInterval system_uptime = 0;
        right_mouse_down = true;
        
        Input_Record* input_record = &input->users[_userID];
        
        usize seq = input_record->direct_sequence_number;
        input_record->direct_sequence_number += 1;
        
        ((input_record->direct_map))[(uintptr_t)1] = UI_Touch();
        UI_Touch* in_touch = &((input_record->direct_map))[(uintptr_t)1];
        in_touch->state = UI_TOUCH_STATE_BEGAN;
        in_touch->sequence_number = seq;
        
        in_touch->direct.prev_position = vec2(0.0f);
        
        in_touch->direct.count = 0;
        
        in_touch->direct.position_delta = vec2(0.0f);
        
        NSPoint pos = [self mouseLocationInView:event];
        
        in_touch->direct.positions[in_touch->direct.count] = vec2(pos.x, pos.y);
        
        
        in_touch->type = UI_TOUCH_TYPE_DIRECT;
        in_touch->key = (uintptr_t)1;
        
        in_touch->direct.state = UI_TOUCH_STATE_BEGAN;
        in_touch->direct.count = 1;
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .key = in_touch->key,
            .timestamp = event.timestamp - system_uptime,
            .flags = 0,
        });
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)in_touch->direct.position_delta.x, (float32)-in_touch->direct.position_delta.y},
            .position = {(float32)pos.x, (float32)pos.y},
            .count = (usize)(~0llu)
        });
    }
}

- (void)rightMouseDragged:(NSEvent*)event {
    if (!self->_is_init) { return; }
    // FIXME: simulate a second touch event, temporary solution on devices with no multi-touch
    if (is_in_alternate_mode()) {
        [[NSCursor dragLinkCursor] set];
        return;
    } else {

        [[NSCursor closedHandCursor] set];
    }
    
    Input* const input = &_core_platform.core.input;
    @autoreleasepool {
        
        NSTimeInterval system_uptime = 0;
        
        Input_Record* input_record = &input->users[_userID];
        
        //((input_record->direct_map))[(uintptr_t)1] = UI_Touch();
        UI_Touch* in_touch = &((input_record->direct_map))[(uintptr_t)1];
        in_touch->state = UI_TOUCH_STATE_MOVED;
        
        if (in_touch->direct.count != 0) {
            in_touch->direct.prev_position = in_touch->direct.positions[in_touch->direct.count - 1];
        } else {
            in_touch->direct.prev_position = vec2(0.0f);
        }
        
        in_touch->direct.count = 0;
        
        in_touch->direct.position_delta = vec2(event.deltaX, -event.deltaY);
        
        
        NSPoint pos = [self mouseLocationInView:event];
        
        
        in_touch->direct.positions[in_touch->direct.count] = vec2(pos.x, pos.y);
        
        in_touch->type = UI_TOUCH_TYPE_DIRECT;
        in_touch->key = (uintptr_t)1;
        
        in_touch->direct.state = UI_TOUCH_STATE_MOVED;
        
        in_touch->direct.count = 1;
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_MOVED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .key = in_touch->key,
            .timestamp = event.timestamp - system_uptime,
            .flags = 0,
        });
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_CHANGED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)in_touch->direct.position_delta.x, (float32)-in_touch->direct.position_delta.y},
            .position = {(float32)pos.x, (float32)pos.y},
            .count = (usize)(~0llu),
            .flags = 0,
        });
        
    }
}

- (void)rightMouseUp:(NSEvent*)event {
    if (!self->_is_init) { return; }
    [[NSCursor arrowCursor] set];
    
    Input* const input = &_core_platform.core.input;
    @autoreleasepool {
        
        NSTimeInterval system_uptime = 0;
        
        Input_Record* input_record = &input->users[_userID];
        
        right_mouse_down = false;
        
        //((input_record->direct_map))[(uintptr_t)1] = UI_Touch();
        UI_Touch* in_touch = &((input_record->direct_map))[(uintptr_t)1];
        in_touch->state = UI_TOUCH_STATE_ENDED;
        
        if (in_touch->direct.count != 0) {
            in_touch->direct.prev_position = in_touch->direct.positions[in_touch->direct.count - 1];
        } else {
            in_touch->direct.prev_position = vec2(0.0f);
        }
        
        in_touch->direct.count = 0;
        
        in_touch->direct.position_delta = vec2(event.deltaX, -event.deltaY);
        
        NSPoint pos = [self mouseLocationInView:event];
        
        
        in_touch->direct.positions[in_touch->direct.count] = vec2(pos.x, pos.y);
        
        
        in_touch->type = UI_TOUCH_TYPE_DIRECT;
        in_touch->key = (uintptr_t)1;
        
        in_touch->direct.state = UI_TOUCH_STATE_ENDED;
        
        in_touch->direct.count = 1;
        
        // FIXME: simulate a second touch event, temporary solution on devices with no multi-touch
        if (is_in_alternate_mode()) {
            
            // simulated touch begin
            ((input_record->direct_map))[(uintptr_t)2] = UI_Touch();
            UI_Touch* in_touch = &((input_record->direct_map))[(uintptr_t)2];
            in_touch->direct.prev_position = in_touch->direct.prev_position;
            in_touch->direct.count = 0;
            in_touch->direct.position_delta = in_touch->direct.position_delta;
            in_touch->direct.positions[in_touch->direct.count] = vec2(pos.x, pos.y);
            
            in_touch->type = UI_TOUCH_TYPE_DIRECT;
            in_touch->key = (uintptr_t)2;
            
            in_touch->direct.state = UI_TOUCH_STATE_BEGAN;
            in_touch->direct.count = 1;
            
            Input_push_event(input_record, {
                .state_type = UI_TOUCH_STATE_BEGAN,
                .input_type = UI_TOUCH_TYPE_DIRECT,
                .key = 2,
                .timestamp = event.timestamp - system_uptime,
                .flags = UI_EVENT_FLAG_CONTEXT_MODE,
            });
            
            // simulated pen begin
            ((input_record->pointer_map))[(uintptr_t)2] = UI_Touch();
            UI_Touch* in_touch_pointer = &((input_record->pointer_map))[(uintptr_t)2];
            in_touch_pointer->pointer.prev_position = in_touch_pointer->pointer.prev_position;
            in_touch_pointer->pointer.count = 0;
            in_touch_pointer->pointer.position_delta = in_touch_pointer->pointer.position_delta;
            in_touch_pointer->pointer.positions[in_touch_pointer->pointer.count] = vec2(pos.x, pos.y);
            
            in_touch_pointer->pointer.state = UI_TOUCH_STATE_BEGAN;
            in_touch_pointer->pointer.count = 1;
            
            Input_push_event(input_record, {
                .state_type = UI_TOUCH_STATE_BEGAN,
                .input_type = UI_TOUCH_TYPE_POINTER,
                .key = 2,
                .timestamp = event.timestamp - system_uptime,
                .flags = UI_EVENT_FLAG_CONTEXT_MODE,
            });
            
            // simulated pen end
            in_touch_pointer->pointer.state = UI_TOUCH_STATE_ENDED;
            Input_push_event(input_record, {
                .state_type = UI_TOUCH_STATE_ENDED,
                .input_type = UI_TOUCH_TYPE_POINTER,
                .key = 2,
                .timestamp = event.timestamp - system_uptime,
                .flags = UI_EVENT_FLAG_CONTEXT_MODE,
            });
            
            // simulated touch end
            in_touch->direct.state = UI_TOUCH_STATE_ENDED;
            Input_push_event(input_record, {
                .state_type = UI_TOUCH_STATE_ENDED,
                .input_type = UI_TOUCH_TYPE_DIRECT,
                .key = 2,
                .timestamp = event.timestamp - system_uptime,
                .flags = UI_EVENT_FLAG_CONTEXT_MODE,
            });
            

        }
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .key = in_touch->key,
            .timestamp = event.timestamp - system_uptime,
            .flags = UI_EVENT_FLAG_CONTEXT_MODE,
        });
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)in_touch->direct.position_delta.x, (float32)-in_touch->direct.position_delta.y},
            .position = {(float32)pos.x, (float32)pos.y},
            .count = (usize)(~0llu),
            .flags = 0,
        });
        
        
        
    }
}


static const float64 DESKTOP_ZOOM_DELTA_FACTOR = (1.0 / 32.0);

- (void)scrollWheel:(NSEvent *)event {
    if (!self->_is_init) { return; }
    @autoreleasepool {
        
        NSTimeInterval system_uptime = 0;
        (void)system_uptime;
        //    [resultsField setStringValue:
        //     [NSString stringWithFormat:@"Rotation in degree is %f", [event rotation]]];
        //    [self setFrameCenterRotation:([self frameCenterRotation] + [event rotation])];
        
//        NSLog(@"Delta Translation (%f, %f)", event.scrollingDeltaX, event.scrollingDeltaY);
        
        
        if (right_mouse_down) {
            Input* const input = &_core_platform.core.input;
            Input_Record* input_record = &input->users[_userID];
            
            NSPoint gesture_position = [self mouseLocationInViewWithPoint:event.locationInWindow];
            
            auto rotation = (event.scrollingDeltaY * DESKTOP_ZOOM_DELTA_FACTOR);

            Input_push_event(input_record, {
                .state_type = UI_TOUCH_STATE_ROTATION_GESTURE_BEGAN,
                .input_type = UI_TOUCH_TYPE_DIRECT,
                .rotation = (float32)rotation,
                .position = {(float32)gesture_position.x, (float32)gesture_position.y},
                .count = 2
            });
        } else {
            Input* const input = &_core_platform.core.input;
            Input_Record* input_record = &input->users[_userID];
            NSPoint gesture_position = [self mouseLocationInViewWithPoint:event.locationInWindow];
            
            auto scale = (event.scrollingDeltaY * DESKTOP_ZOOM_DELTA_FACTOR) + 1.0f;
            
//            if ([event subtype] != NSEventSubtypeMouseEvent) {
//
//                Input_push_event(input_record, {
//                    .state_type = UI_TOUCH_STATE_PAN_GESTURE_BEGAN,
//                    .input_type = UI_TOUCH_TYPE_DIRECT,
//                    .translation = {(float32)-event.scrollingDeltaX, (float32)-event.scrollingDeltaY},
//                    .position = {(float32)gesture_position.x, (float32)gesture_position.y},
//                    .count = (~0llu)
//                });
//
//            } else
            {
                
            
                Input_push_event(input_record, {
                    .state_type = UI_TOUCH_STATE_PINCH_GESTURE_BEGAN,
                    .input_type = UI_TOUCH_TYPE_DIRECT,
                    .scale = {(float32)scale, (float32)scale},
                    .position = {(float32)gesture_position.x, (float32)gesture_position.y},
                    .count = 2
                });
            }
        }
        
    }
}



#endif

#if !TARGET_OS_OSX

static inline bool UI_primary_button_is_down(UIEvent* event)
{
    return (event.buttonMask & UIEventButtonMaskPrimary) != 0;
}
static inline bool UI_secondary_button_is_down(UIEvent* event)
{
    return (event.buttonMask & UIEventButtonMaskSecondary) != 0;
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    Input* const input = &_core_platform.core.input;
    
    begin_touch_state();
    
    @autoreleasepool {
        
        NSTimeInterval system_uptime = 0; // [[NSProcessInfo processInfo] systemUptime] - mtt_time_seconds();
        for (UITouch* touch in touches) {
            //auto loc = [touch preciseLocationInView:_mtk_view];
            //printf("OBJECTIVE C BEGIN [%f %f]\n", loc.x, loc.y);
            switch (type_switch(touch)) {
                case UITouchTypeIndirectPointer: {
                    if (UI_primary_button_is_down(event)) {
                        goto PENCIL_LABEL;
                    } else if (UI_secondary_button_is_down(event)) {
                        goto DIRECT_LABEL;
                    }
                    break;
                }
                case UITouchTypePencil: {
                PENCIL_LABEL:
                    Input_Record* input_record = &input->users[_userID];
                    
                    usize seq = input_record->pointer_sequence_number;
                    input_record->pointer_sequence_number += 1;
                    
                    // add to map
                    
                    input_record->pointer_input_map[(uintptr_t)touch] = seq;
                    ((input_record->pointer_map))[seq] = UI_Touch();
                    UI_Touch* in_touch = &((input_record->pointer_map))[(uintptr_t)seq];
                    
                    
                    in_touch->state = UI_TOUCH_STATE_BEGAN;
                    in_touch->sequence_number = seq;

                    in_touch->pointer.prev_position = vec2(0.0f);
                    
                    in_touch->pointer.count = 0;
                    
                    //MTT_print("%p %llu\n", touch, (uintptr_t)touch);
                    
                    // save previous frame snapshot
                    //in_touch->pointer.prev_position            = in_touch->pointer.positions.back();
                    //in_touch->pointer.prev_azimuth_unit_vector = in_touch->pointer.azimuth_unit_vectors.back();
                    //in_touch->pointer.prev_altitude_angle      = in_touch->pointer.altitude_angles.back();
                    //in_touch->pointer.prev_azimuth_unit_vector = in_touch->pointer.azimuth_unit_vectors.back();
                    
                    in_touch->pointer.average_force = touch.force;
                    
                    // update new in-between frame snapshots
                    CGPoint pos;
                    //CGVector azimuth_unit_vector;
                    NSArray* coalesced_touches = [event coalescedTouchesForTouch:touch];
                    usize count = [coalesced_touches count];
                    for (usize i = 0; i < count && i < (UI_TOUCH_MAX_ENTRIES - 1); i += 1) {
                        UITouch* coalesced = [coalesced_touches objectAtIndex:i];
                        pos = [coalesced preciseLocationInView:_mtk_view];
                        in_touch->pointer.positions[in_touch->pointer.count] = (vec2((float32)pos.x, (float32)pos.y));
                        in_touch->pointer.forces[in_touch->pointer.count] = coalesced.force;
                        in_touch->pointer.timestamps[in_touch->pointer.count] = coalesced.timestamp - system_uptime;
                        in_touch->pointer.altitude_angle_radians = coalesced.altitudeAngle;
                        in_touch->pointer.count += 1;
                        //in_touch->pointer.altitude_angles.push_back(coalesced.altitudeAngle);
                       // azimuth_unit_vector = [coalesced azimuthUnitVectorInView:_mtk_view];
                        //in_touch->pointer.azimuth_angles.push_back((float32)[coalesced azimuthAngleInView:_mtk_view]);
                        //in_touch->pointer.azimuth_unit_vectors.push_back(vec2((float32)azimuth_unit_vector.dx, (float32)azimuth_unit_vector.dy));
                    }
                    // current frame snapshots
//                    pos = [touch preciseLocationInView:_mtk_view];
//                    in_touch->pointer.positions[in_touch->pointer.count] = (vec2((float32)pos.x, (float32)pos.y));
//                    in_touch->pointer.timestamps[in_touch->pointer.count] = touch.timestamp - system_uptime;
//                    in_touch->pointer.count += 1;
                    //in_touch->pointer.positions.push_back(vec2(pos.x, pos.y));
                    
                    
                    
                    //in_touch->pointer.azimuth_angles.push_back([touch azimuthAngleInView:_mtk_view]);
                    
                    
                    in_touch->pointer.position_delta = vec2(0.0f);
                    
                    
                    in_touch->pointer.azimuth = [touch azimuthAngleInView:_mtk_view];
                    CGVector azimuth_unit_vector = [touch azimuthUnitVectorInView:_mtk_view];
                    in_touch->pointer.azimuth_unit_vector = vec2_64(azimuth_unit_vector.dx, azimuth_unit_vector.dy);
                    
                    
                    in_touch->type = UI_TOUCH_TYPE_POINTER;
                    in_touch->key = (uintptr_t)seq;

                    in_touch->pointer.state = UI_TOUCH_STATE_BEGAN;
                    
                    Input_push_event(input_record, { 
                        .state_type = UI_TOUCH_STATE_BEGAN,
                        .input_type = UI_TOUCH_TYPE_POINTER,
                        .key = in_touch->key,
                        .timestamp = touch.timestamp - system_uptime,
                    });
                    
                    
                    break;
                }
                case UITouchTypeDirect: {
                DIRECT_LABEL:
                    //MTT_print("BEGAN %lu\n", (uintptr_t)touch);
                    Input_Record* input_record = &input->users[_userID];
                    usize seq = input_record->direct_sequence_number;
                    input_record->direct_sequence_number += 1;
                    
                    
                    input_record->direct_input_map[(uintptr_t)touch] = seq;
                    ((input_record->direct_map))[seq] = UI_Touch();
                    UI_Touch* in_touch = &((input_record->direct_map))[(uintptr_t)seq];
                    in_touch->state = UI_TOUCH_STATE_BEGAN;
                    
                    in_touch->sequence_number = seq;
                    
                    in_touch->direct.prev_position = vec2(0.0f);
                    
                    in_touch->direct.count = 0;
                    // save previous frame snapshot
                    //in_touch->direct.prev_position = in_touch->direct.positions.back();
                    
                    // update new in-between frame snapshots
                    CGPoint pos;
                    NSArray* coalesced_touches = [event coalescedTouchesForTouch:touch];
                    usize count = [coalesced_touches count];
                    for (usize i = 0; i < count && i < (UI_TOUCH_MAX_ENTRIES - 1); i += 1) {
                        UITouch* coalesced = [coalesced_touches objectAtIndex:i];
                        pos = [coalesced preciseLocationInView:_mtk_view];
                        in_touch->direct.positions[in_touch->direct.count] = (vec2((float32)pos.x, (float32)pos.y));
                        in_touch->direct.timestamps[in_touch->direct.count] = coalesced.timestamp - system_uptime;
                        in_touch->direct.count += 1;
                        
                    }
                    
                    // current frame snapshots
//                    pos = [touch preciseLocationInView:_mtk_view];
//                    in_touch->direct.positions[in_touch->direct.count] = (vec2((float32)pos.x, (float32)pos.y));
//                    in_touch->direct.timestamps[in_touch->direct.count] = touch.timestamp - system_uptime;
//                    in_touch->direct.count += 1;

                    in_touch->type = UI_TOUCH_TYPE_DIRECT;
                    in_touch->key = (uintptr_t)seq;
                    
                    in_touch->direct.position_delta = vec2(0.0f);

                    
                    in_touch->direct.state = UI_TOUCH_STATE_BEGAN;
                    
                    Input_push_event(input_record, {
                        .state_type = UI_TOUCH_STATE_BEGAN,
                        .input_type = UI_TOUCH_TYPE_DIRECT,
                        .key = in_touch->key,
                        .timestamp = touch.timestamp - system_uptime,
                    });
                    
                    
                    break;
                }
                default: {
                    NSLog(@"unhandled input type\n");
                    break;
                }
            }
        }
    }
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    Input* const input = &_core_platform.core.input;
    
    @autoreleasepool {
        NSTimeInterval system_uptime = 0; // [[NSProcessInfo processInfo] systemUptime] - mtt_time_seconds();
        for (UITouch* touch in touches) {
            //auto loc = [touch preciseLocationInView:_mtk_view];
           // printf("OBJECTIVE C MOVE [%f %f]\n", loc.x, loc.y);
            
            switch (type_switch(touch)) {
                case UITouchTypeIndirectPointer: {
                    if (UI_primary_button_is_down(event)) {
                        goto PENCIL_LABEL;
                    } else if (UI_secondary_button_is_down(event)) {
                        goto DIRECT_LABEL;
                    }
                    break;
                }
                case UITouchTypePencil: {
                PENCIL_LABEL:
                    Input_Record* input_record = &input->users[_userID];
                    
                    UI_Touch* in_touch = &((input_record->pointer_map))[input_record->pointer_input_map[(uintptr_t)touch]];
                    in_touch->state = UI_TOUCH_STATE_MOVED;
                    if (in_touch->pointer.count != 0) {
                        in_touch->pointer.prev_position = in_touch->pointer.positions[in_touch->pointer.count - 1];
                    } else {
                        in_touch->pointer.prev_position = vec2(0.0f);
                    }
                    in_touch->pointer.count = 0;
                    
                    //MTT_print("%p %llu\n", touch, (uintptr_t)touch, (uintptr_t)in_touch->key);

                    
                    // save previous frame snapshot
                    //in_touch->pointer.prev_position            = in_touch->pointer.positions.back();
                    //in_touch->pointer.prev_azimuth_unit_vector = in_touch->pointer.azimuth_unit_vectors.back();
                    //in_touch->pointer.prev_altitude_angle      = in_touch->pointer.altitude_angles.back();
                    //in_touch->pointer.prev_azimuth_unit_vector = in_touch->pointer.azimuth_unit_vectors.back();
                    
                    in_touch->pointer.average_force = touch.force;
                    
                    // update new in-between frame snapshots
                    CGPoint pos;
                    //CGVector azimuth_unit_vector;
                    NSArray* coalesced_touches = [event coalescedTouchesForTouch:touch];
                    usize count = [coalesced_touches count];
                    for (usize i = 0; i < count && i < (UI_TOUCH_MAX_ENTRIES - 1); i += 1) {
                        UITouch* coalesced = [coalesced_touches objectAtIndex:i];
                        pos = [coalesced preciseLocationInView:_mtk_view];
                        in_touch->pointer.positions[in_touch->pointer.count] = (vec2((float32)pos.x, (float32)pos.y));
                        in_touch->pointer.forces[in_touch->pointer.count] = coalesced.force;
                        in_touch->pointer.timestamps[in_touch->pointer.count] = coalesced.timestamp - system_uptime;
                        in_touch->pointer.altitude_angle_radians = coalesced.altitudeAngle;
                        in_touch->pointer.count += 1;
                        //in_touch->pointer.altitude_angles.push_back(coalesced.altitudeAngle);
                        //azimuth_unit_vector = [coalesced azimuthUnitVectorInView:_mtk_view];
                        //in_touch->pointer.azimuth_angles.push_back((float32)[coalesced azimuthAngleInView:_mtk_view]);
                        //in_touch->pointer.azimuth_unit_vectors.push_back(vec2((float32)azimuth_unit_vector.dx, (float32)azimuth_unit_vector.dy));
                    }
                    // current frame snapshots
//                    pos = [touch preciseLocationInView:_mtk_view];
//                    in_touch->pointer.positions[in_touch->pointer.count] = (vec2((float32)pos.x, (float32)pos.y));
//                    in_touch->pointer.timestamps[in_touch->pointer.count] = touch.timestamp - system_uptime;
//                    in_touch->pointer.count += 1;
                    in_touch->pointer.azimuth = [touch azimuthAngleInView:_mtk_view];
                    CGVector azimuth_unit_vector = [touch azimuthUnitVectorInView:_mtk_view];
                    in_touch->pointer.azimuth_unit_vector = vec2_64(azimuth_unit_vector.dx, azimuth_unit_vector.dy);
                    
                    in_touch->pointer.position_delta = vec2(pos.x, pos.y) - in_touch->pointer.prev_position;
                    //in_touch->pointer.positions.push_back(vec2(pos.x, pos.y));
                    
                    //in_touch->pointer.azimuth_angles.push_back([touch azimuthAngleInView:_mtk_view]);
                    
                    //azimuth_unit_vector = [touch azimuthUnitVectorInView:_mtk_view];
                    //in_touch->pointer.azimuth_unit_vectors.push_back(vec2((float32)azimuth_unit_vector.dx, (float32)azimuth_unit_vector.dy));
                    
                    in_touch->pointer.state = UI_TOUCH_STATE_MOVED;
                    
                    Input_push_event(input_record, {
                        .state_type = UI_TOUCH_STATE_MOVED,
                        .input_type = UI_TOUCH_TYPE_POINTER,
                        .key = in_touch->key,
                        .timestamp = touch.timestamp - system_uptime,
                    });
                    
                    break;
                }
                case UITouchTypeDirect: {
                DIRECT_LABEL:
                    //MTT_print("MOVED%lu\n", (uintptr_t)touch);
                    Input_Record* input_record = &input->users[_userID];
                    
                    UI_Touch* in_touch = &((input_record->direct_map))[input_record->direct_input_map[(uintptr_t)touch]];
                    in_touch->state = UI_TOUCH_STATE_MOVED;
                    if (in_touch->direct.count != 0) {
                        in_touch->direct.prev_position = in_touch->direct.positions[in_touch->direct.count - 1];
                    } else {
                        in_touch->direct.prev_position = vec2(0.0f);
                    }
                    in_touch->direct.count = 0;
                    
                    // save previous frame snapshot
                    
                    
                    // update new in-between frame snapshots
                    CGPoint pos;
                    NSArray* coalesced_touches = [event coalescedTouchesForTouch:touch];
                    usize count = [coalesced_touches count];
                    for (usize i = 0; i < count && i < (UI_TOUCH_MAX_ENTRIES - 1); i += 1) {
                        UITouch* coalesced = [coalesced_touches objectAtIndex:i];
                        pos = [coalesced preciseLocationInView:_mtk_view];
                        in_touch->direct.positions[in_touch->direct.count] = (vec2((float32)pos.x, (float32)pos.y));
                        in_touch->direct.timestamps[in_touch->direct.count] = coalesced.timestamp - system_uptime;
                        in_touch->direct.count += 1;
                    }
                    
                    
                    // current frame snapshots
//                    pos = [touch preciseLocationInView:_mtk_view];
//                    in_touch->direct.positions[in_touch->direct.count] = (vec2((float32)pos.x, (float32)pos.y));
//                    in_touch->direct.timestamps[in_touch->direct.count] = touch.timestamp - system_uptime;
//                    in_touch->direct.count += 1;
                    
                    in_touch->direct.position_delta = vec2(pos.x, pos.y) - in_touch->direct.prev_position;

                    
                    in_touch->direct.state = UI_TOUCH_STATE_MOVED;
                    
                    Input_push_event(input_record, {
                        .state_type = UI_TOUCH_STATE_MOVED,
                        .input_type = UI_TOUCH_TYPE_DIRECT,
                        .key = in_touch->key,
                        .timestamp = touch.timestamp - system_uptime,
                    });
                    
                    break;
                }
                default: {
                    NSLog(@"unhandled input type\n");
                    break;
                }
                    
            }
        }
    }
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    Input* const input = &_core_platform.core.input;
    
    
    @autoreleasepool {
        NSTimeInterval system_uptime = 0; // [[NSProcessInfo processInfo] systemUptime] - mtt_time_seconds();
        for (UITouch* touch in touches) {
            //auto loc = [touch preciseLocationInView:_mtk_view];
            //printf("OBJECTIVE C CANCEL [%f %f]\n", loc.x, loc.y);
            
            switch (type_switch(touch)) {
                case UITouchTypeIndirectPointer: {
                    if (UI_primary_button_is_down(event)) {
                        goto PENCIL_LABEL;
                    } else if (UI_secondary_button_is_down(event)) {
                        goto DIRECT_LABEL;
                    }
                    break;
                }
                case UITouchTypePencil: {
                PENCIL_LABEL:
                    Input_Record* input_record = &input->users[_userID];
                    
                    auto find = input_record->pointer_input_map.find((uintptr_t)touch);
                    UI_Touch* in_touch = &((input_record->pointer_map))[find->second];
                    input_record->direct_input_map.erase(find);
                    
                    in_touch->state = UI_TOUCH_STATE_CANCELLED;
                    if (in_touch->pointer.count != 0) {
                        in_touch->pointer.prev_position = in_touch->pointer.positions[in_touch->pointer.count - 1];
                    } else {
                        in_touch->pointer.prev_position = vec2(0.0f);
                    }
                    
                    in_touch->pointer.count = 0;
                    // save previous frame snapshot
                    //in_touch->pointer.prev_position            = in_touch->pointer.positions.back();
                    //in_touch->pointer.prev_azimuth_unit_vector = in_touch->pointer.azimuth_unit_vectors.back();
                    //in_touch->pointer.prev_altitude_angle      = in_touch->pointer.altitude_angles.back();
                    //in_touch->pointer.prev_azimuth_unit_vector = in_touch->pointer.azimuth_unit_vectors.back();
                    
                    in_touch->pointer.average_force = touch.force;
                    
                    // update new in-between frame snapshots
                    CGPoint pos;
                    //CGVector azimuth_unit_vector;
                    NSArray* coalesced_touches = [event coalescedTouchesForTouch:touch];
                    usize count = [coalesced_touches count];
                    for (usize i = 0; i < count && i < (UI_TOUCH_MAX_ENTRIES - 1); i += 1) {
                        UITouch* coalesced = [coalesced_touches objectAtIndex:i];
                        pos = [coalesced preciseLocationInView:_mtk_view];
                        in_touch->pointer.positions[in_touch->pointer.count] = (vec2((float32)pos.x, (float32)pos.y));
                        in_touch->pointer.timestamps[in_touch->pointer.count] = coalesced.timestamp - system_uptime;
                        in_touch->pointer.forces[in_touch->pointer.count] = coalesced.force;
                        in_touch->pointer.altitude_angle_radians = coalesced.altitudeAngle;
                        in_touch->pointer.count += 1;
                        //in_touch->pointer.altitude_angles.push_back(coalesced.altitudeAngle);
                        //azimuth_unit_vector = [coalesced azimuthUnitVectorInView:_mtk_view];
                        //in_touch->pointer.azimuth_angles.push_back((float32)[coalesced azimuthAngleInView:_mtk_view]);
                        //in_touch->pointer.azimuth_unit_vectors.push_back(vec2((float32)azimuth_unit_vector.dx, (float32)azimuth_unit_vector.dy));
                    }
                    

                    // current frame snapshots
//                    pos = [touch preciseLocationInView:_mtk_view];
//                    in_touch->pointer.positions[in_touch->pointer.count] = (vec2((float32)pos.x, (float32)pos.y));
//                    in_touch->pointer.timestamps[in_touch->pointer.count] = touch.timestamp - system_uptime;
//                    in_touch->pointer.count += 1;

                    //in_touch->pointer.azimuth_angles.push_back([touch azimuthAngleInView:_mtk_view]);
                    
                    //azimuth_unit_vector = [touch azimuthUnitVectorInView:_mtk_view];
                    //in_touch->pointer.azimuth_unit_vectors.push_back(vec2((float32)azimuth_unit_vector.dx, (float32)azimuth_unit_vector.dy));
                        
                    in_touch->pointer.position_delta = vec2(pos.x, pos.y) - in_touch->pointer.prev_position;
                    
                    in_touch->pointer.azimuth = [touch azimuthAngleInView:_mtk_view];
                    CGVector azimuth_unit_vector = [touch azimuthUnitVectorInView:_mtk_view];
                    in_touch->pointer.azimuth_unit_vector = vec2_64(azimuth_unit_vector.dx, azimuth_unit_vector.dy);

                    
                    in_touch->pointer.state = UI_TOUCH_STATE_CANCELLED;
                    
                    Input_schedule_pointer_touch_removal(input, _userID, in_touch);
                    
                    Input_push_event(input_record, {
                        .state_type = UI_TOUCH_STATE_CANCELLED,
                        .input_type = UI_TOUCH_TYPE_POINTER,
                        .key = in_touch->key,
                        .timestamp = touch.timestamp - system_uptime,
                    });
                    
                    break;
                }
                case UITouchTypeDirect: {
                DIRECT_LABEL:
                    //MTT_print("CANCELLED%lu\n", (uintptr_t)touch);
                    Input_Record* input_record = &input->users[_userID];
                    
                    auto find = input_record->direct_input_map.find((uintptr_t)touch);
                    UI_Touch* in_touch = &((input_record->direct_map))[find->second];
                    input_record->direct_input_map.erase(find);
                    
                    in_touch->state = UI_TOUCH_STATE_CANCELLED;
                    if (in_touch->direct.count != 0) {
                        in_touch->direct.prev_position = in_touch->direct.positions[in_touch->direct.count - 1];
                    } else {
                        in_touch->direct.prev_position = vec2(0.0f);
                    }
                    in_touch->direct.count = 0;
                    // save previous frame snapshot
                    //in_touch->direct.prev_position = in_touch->direct.positions.back();
                    
                    // update new in-between frame snapshots
                    CGPoint pos;
                    NSArray* coalesced_touches = [event coalescedTouchesForTouch:touch];
                    usize count = [coalesced_touches count];
                    for (usize i = 0; i < count && i < (UI_TOUCH_MAX_ENTRIES - 1); i += 1) {
                        UITouch* coalesced = [coalesced_touches objectAtIndex:i];
                        pos = [coalesced preciseLocationInView:_mtk_view];
                        in_touch->direct.positions[in_touch->direct.count] = (vec2((float32)pos.x, (float32)pos.y));
                        in_touch->direct.timestamps[in_touch->direct.count] = coalesced.timestamp - system_uptime;
                        in_touch->direct.count += 1;
                    }
                    
                    

                    // current frame snapshots
//                    pos = [touch preciseLocationInView:_mtk_view];
//                    in_touch->direct.positions[in_touch->direct.count] = (vec2((float32)pos.x, (float32)pos.y));
//                    in_touch->direct.timestamps[in_touch->direct.count] = touch.timestamp - system_uptime;
//                    in_touch->direct.count += 1;
                    
                    in_touch->direct.position_delta = vec2(pos.x, pos.y) - in_touch->direct.prev_position;

                    
                    in_touch->direct.state = UI_TOUCH_STATE_CANCELLED;
                    
                    
                                    
                    Input_schedule_direct_touch_removal(input, _userID, in_touch);
                    
                    Input_push_event(input_record, {
                        .state_type = UI_TOUCH_STATE_CANCELLED,
                        .input_type = UI_TOUCH_TYPE_DIRECT,
                        .key = in_touch->key,
                        .timestamp = touch.timestamp - system_uptime,
                    });
                    
                    break;
                }
                default: {
                    NSLog(@"unhandled input type\n");
                    break;
                }
                    
            }
        }
    }
    
    end_touch_state();
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    Input* const input = &_core_platform.core.input;
    
    @autoreleasepool {
        NSTimeInterval system_uptime = 0; // [[NSProcessInfo processInfo] systemUptime] - mtt_time_seconds();
        for (UITouch* touch in touches) {
            //auto loc = [touch preciseLocationInView:_mtk_view];
            //printf("OBJECTIVE C END [%f %f]\n", loc.x, loc.y);
            
            switch (type_switch(touch)) {
                case UITouchTypeIndirectPointer: {
                    if (UI_primary_button_is_down(event)) {
                        goto PENCIL_LABEL;
                    } else if (UI_secondary_button_is_down(event)) {
                        goto DIRECT_LABEL;
                    }
                    break;
                }
                case UITouchTypePencil: {
                PENCIL_LABEL:
                    Input_Record* input_record = &input->users[_userID];
                    
                    auto find = input_record->pointer_input_map.find((uintptr_t)touch);
                    UI_Touch* in_touch = &((input_record->pointer_map))[find->second];
                    input_record->pointer_input_map.erase(find);
                    
                    in_touch->state = UI_TOUCH_STATE_ENDED;
                    if (in_touch->pointer.count != 0) {
                        in_touch->pointer.prev_position = in_touch->pointer.positions[in_touch->pointer.count - 1];
                    } else {
                        in_touch->pointer.prev_position = vec2(0.0f);
                    }
                    in_touch->pointer.count = 0;
                    //MTT_print("%p %llu\n", touch, (uintptr_t)touch);

                    
                    // save previous frame snapshot
                    //in_touch->pointer.prev_position            = in_touch->pointer.positions.back();
                    //in_touch->pointer.prev_azimuth_unit_vector = in_touch->pointer.azimuth_unit_vectors.back();
                    //in_touch->pointer.prev_altitude_angle      = in_touch->pointer.altitude_angles.back();
                    //in_touch->pointer.prev_azimuth_unit_vector = in_touch->pointer.azimuth_unit_vectors.back();
                    
                    // update new in-between frame snapshots
                    in_touch->pointer.average_force = touch.force;

                    
                    
                    CGPoint pos;
                    //CGVector azimuth_unit_vector;
                    NSArray* coalesced_touches = [event coalescedTouchesForTouch:touch];
                    usize count = [coalesced_touches count];
                    for (usize i = 0; i < count && i < (UI_TOUCH_MAX_ENTRIES - 1); i += 1) {
                        UITouch* coalesced = [coalesced_touches objectAtIndex:i];
                        pos = [coalesced preciseLocationInView:_mtk_view];
                        in_touch->pointer.positions[in_touch->pointer.count] = (vec2((float32)pos.x, (float32)pos.y));
                        in_touch->pointer.forces[in_touch->pointer.count] = coalesced.force;
                        in_touch->pointer.timestamps[in_touch->pointer.count] = coalesced.timestamp - system_uptime;
                        in_touch->pointer.altitude_angle_radians = coalesced.altitudeAngle;
                        in_touch->pointer.count += 1;
                        //in_touch->pointer.altitude_angles.push_back(coalesced.altitudeAngle);
                        //azimuth_unit_vector = [coalesced azimuthUnitVectorInView:_mtk_view];
                       // in_touch->pointer.azimuth_angles.push_back((float32)[coalesced azimuthAngleInView:_mtk_view]);
                        //in_touch->pointer.azimuth_unit_vectors.push_back(vec2((float32)azimuth_unit_vector.dx, (float32)azimuth_unit_vector.dy));
                    }

                    // current frame snapshots
//                    pos = [touch preciseLocationInView:_mtk_view];
//                    in_touch->pointer.positions[in_touch->pointer.count] = (vec2((float32)pos.x, (float32)pos.y));
//                    in_touch->pointer.timestamps[in_touch->pointer.count] = touch.timestamp - system_uptime;
//                    in_touch->pointer.count += 1;
                    
                    in_touch->pointer.azimuth = [touch azimuthAngleInView:_mtk_view];
                    CGVector azimuth_unit_vector = [touch azimuthUnitVectorInView:_mtk_view];
                    in_touch->pointer.azimuth_unit_vector = vec2_64(azimuth_unit_vector.dx, azimuth_unit_vector.dy);
                    
                    in_touch->pointer.position_delta = vec2(pos.x, pos.y) - in_touch->pointer.prev_position;

                    //in_touch->pointer.positions.push_back(vec2(pos.x, pos.y));
                    
                    //in_touch->pointer.azimuth_angles.push_back([touch azimuthAngleInView:_mtk_view]);
                    
                    //azimuth_unit_vector = [touch azimuthUnitVectorInView:_mtk_view];
                    //in_touch->pointer.azimuth_unit_vectors.push_back(vec2((float32)azimuth_unit_vector.dx, (float32)azimuth_unit_vector.dy));
                    
                    in_touch->pointer.state = UI_TOUCH_STATE_ENDED;
                    
                    Input_schedule_pointer_touch_removal(input, _userID, in_touch);
                    
                    Input_push_event(input_record, {
                        .state_type = UI_TOUCH_STATE_ENDED,
                        .input_type = UI_TOUCH_TYPE_POINTER,
                        .key = in_touch->key,
                        .timestamp = touch.timestamp - system_uptime,
                    });
                    
                    break;
                }
                case UITouchTypeDirect: {
                DIRECT_LABEL:
                    //MTT_print("ENDED%lu\n", (uintptr_t)touch);
                    Input_Record* input_record = &input->users[_userID];
                    
                    auto find = input_record->direct_input_map.find((uintptr_t)touch);
                    UI_Touch* in_touch = &((input_record->direct_map))[find->second];
                    input_record->direct_input_map.erase(find);
                    in_touch->state = UI_TOUCH_STATE_ENDED;
                    if (in_touch->direct.count != 0) {
                        in_touch->direct.prev_position = in_touch->direct.positions[in_touch->direct.count - 1];
                    } else {
                        in_touch->direct.prev_position = vec2(0.0f);
                    }
                    in_touch->direct.count = 0;
                    // save previous frame snapshot
                    //in_touch->direct.prev_position = in_touch->direct.positions.back();
                    
                    // update new in-between frame snapshots
                    CGPoint pos;
                    NSArray* coalesced_touches = [event coalescedTouchesForTouch:touch];
                    usize count = [coalesced_touches count];
                    for (usize i = 0; i < count && i < (UI_TOUCH_MAX_ENTRIES - 1); i += 1) {
                        UITouch* coalesced = [coalesced_touches objectAtIndex:i];
                        pos = [coalesced preciseLocationInView:_mtk_view];
                        in_touch->direct.positions[in_touch->direct.count] = (vec2((float32)pos.x, (float32)pos.y));
                        in_touch->direct.timestamps[in_touch->direct.count] = coalesced.timestamp - system_uptime;
                        in_touch->direct.count += 1;
                    }
                                        
                    // current frame snapshots
//                    pos = [touch preciseLocationInView:_mtk_view];
//                    in_touch->direct.positions[in_touch->direct.count] = (vec2((float32)pos.x, (float32)pos.y));
//                    in_touch->direct.timestamps[in_touch->direct.count] = touch.timestamp - system_uptime;
//                    in_touch->direct.count += 1;
                    
                    in_touch->direct.position_delta = vec2(pos.x, pos.y) - in_touch->direct.prev_position;


                    
                    in_touch->direct.state = UI_TOUCH_STATE_ENDED;
                    
                    
                    Input_schedule_direct_touch_removal(input, _userID, in_touch);
                    
                    Input_push_event(input_record, {
                        .state_type = UI_TOUCH_STATE_ENDED,
                        .input_type = UI_TOUCH_TYPE_DIRECT,
                        .key = in_touch->key,
                        .timestamp = touch.timestamp - system_uptime,
                    });

                    break;
                }
                default: {
                    NSLog(@"unhandled input type\n");
                    break;
                }
                    
            }
        }
    }
    
    end_touch_state();
}


#pragma mark - ARSessionDelegate

- (void)session:(ARSession *)session didFailWithError:(NSError *)error {
    // Present an error message to the user
    
    NSLog(@"ARSession failed %@", [error localizedDescription]);
    
}

- (void)sessionWasInterrupted:(ARSession *)session {
    // Inform the user that the session has been interrupted, for example, by presenting an overlay
    
    NSLog(@"ARSession was interrupted");
    
}

- (void)sessionInterruptionEnded:(ARSession *)session {
    // Reset tracking and/or remove existing anchors if consistent tracking is required
    
    NSLog(@"ARSession interruption ended");
    
}






- (void)panGestureWasRecognized:(UIPanGestureRecognizer *)sender {
    CGPoint deltaTranslation = [sender translationInView:self.view];
    //usize touch_count = sender.numberOfTouches;
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    //sender.cancelsTouchesInView = false;
    if (sender.state == UIGestureRecognizerStateBegan) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)deltaTranslation.x, (float32)deltaTranslation.y},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    } else if (sender.state == UIGestureRecognizerStateChanged) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];

        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_CHANGED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)deltaTranslation.x, (float32)deltaTranslation.y},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    } else if (sender.state == UIGestureRecognizerStateEnded
               || sender.state == UIGestureRecognizerStateCancelled) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)deltaTranslation.x, (float32)deltaTranslation.y},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    }
    
    [sender setTranslation:CGPointZero inView:self.view];
}

- (void)pinchGestureWasRecognized:(UIPinchGestureRecognizer *)sender {
    //MTT_print("PINCH GESTURE RECOGNIZED!\n");
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    if (sender.state == UIGestureRecognizerStateBegan
        || sender.state == UIGestureRecognizerStateChanged) {
        //usize touch_count = sender.numberOfTouches;
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PINCH_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .scale = {(float32)sender.scale, (float32)sender.scale},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    } else if (sender.state == UIGestureRecognizerStateEnded
               || sender.state == UIGestureRecognizerStateCancelled) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PINCH_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .scale = {(float32)sender.scale, (float32)sender.scale},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    }
    
    sender.scale = 1.0;
}

- (void)rotationGestureWasRecognized:(UIRotationGestureRecognizer *)sender {
    //NSLog(@"rotation %f\n", sender.rotation);
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    if (sender.state == UIGestureRecognizerStateBegan
        || sender.state == UIGestureRecognizerStateChanged) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_ROTATION_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .rotation = (float32)sender.rotation,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    } else if (sender.state == UIGestureRecognizerStateEnded
               || sender.state == UIGestureRecognizerStateCancelled) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_ROTATION_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .rotation = (float32)sender.rotation,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    }
    
    sender.rotation = 0.0;
}

- (void)longPressGestureWasRecognized:(UILongPressGestureRecognizer *)sender {
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    if (sender.state == UIGestureRecognizerStateBegan
    /* || sender.state == UIGestureRecognizerStateChanged*/) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_LONG_PRESS_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
        
        //MTT_print("Long press gesture was recognized!\n");


    } else if (sender.state == UIGestureRecognizerStateEnded ||
               sender.state == UIGestureRecognizerStateCancelled) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_LONG_PRESS_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
        
        //MTT_print("Long press gesture ended!\n");
    }
}

- (void)leftScreenEdgeSwiped:(UIScreenEdgePanGestureRecognizer *)sender {
    if (sender.state == UIGestureRecognizerStateBegan) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
        });
    } else if (sender.state == UIGestureRecognizerStateChanged) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_CHANGED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
        });
    } else if (sender.state == UIGestureRecognizerStateCancelled) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_CANCELLED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
        });
    } else if (sender.state == UIGestureRecognizerStateEnded) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_BORDER_SWIPE_FROM_LEFT_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
        });
    }
}
- (void)rightScreenEdgeSwiped:(UIScreenEdgePanGestureRecognizer *)sender {
    
    if (sender.state == UIGestureRecognizerStateBegan) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
        });
    } else if (sender.state == UIGestureRecognizerStateChanged) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_CHANGED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
        });
    } else if (sender.state == UIGestureRecognizerStateCancelled) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_CANCELLED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
        });
    } else if (sender.state == UIGestureRecognizerStateEnded) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_BORDER_SWIPE_FROM_RIGHT_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
        });
    }
}

- (void)hoverGestureWasRecognized:(UIHoverGestureRecognizer *)sender {
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    //sender.cancelsTouchesInView = false;
    
    float64 azimuth = 0.0;
    vec2_64 azimuth_unit_vector = {};
    float64 altitude = 0.0;
    if (@available(iOS 16.4, *)) {
        azimuth = [sender azimuthAngleInView:_mtk_view];
        auto v = [sender azimuthUnitVectorInView:_mtk_view];
        azimuth_unit_vector = vec2_64(v.dx, v.dy);
        altitude = [sender altitudeAngle];
    }
    
#define HOVER_GESTURE_PRINT (0)
    
    if (sender.state == UIGestureRecognizerStateBegan) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
#if HOVER_GESTURE_PRINT
        MTT_print("Hover z_position began: [%f,%f,%f]\n", gesture_position.x, gesture_position.y, sender.zOffset);
#endif
        
        Input_push_event(input_record, {
            .state_type = UI_HOVER_STATE_BEGAN,
            .input_type = UI_TOUCH_TYPE_POINTER,
            .pointer_info = {
                .azimuth = azimuth,
                .altitude_angle_radians = altitude,
                .azimuth_unit_vector = azimuth_unit_vector
            },
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .z_position = sender.zOffset,
            .count = sender.numberOfTouches
        });
    } else if (sender.state == UIGestureRecognizerStateChanged) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
#if HOVER_GESTURE_PRINT
        MTT_print("Hover z_position changed: [%f,%f,%f]\n", gesture_position.x, gesture_position.y, sender.zOffset);
#endif

        Input_push_event(input_record, {
            .state_type = UI_HOVER_STATE_CHANGED,
            .input_type = UI_TOUCH_TYPE_POINTER,
            .pointer_info = {
                .azimuth = azimuth,
                .altitude_angle_radians = altitude,
                .azimuth_unit_vector = azimuth_unit_vector
            },
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .z_position = sender.zOffset,
            .count = sender.numberOfTouches
        });
    } else if (sender.state == UIGestureRecognizerStateEnded
               || sender.state == UIGestureRecognizerStateCancelled) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
#if HOVER_GESTURE_PRINT
        MTT_print("Hover z_position ended: [%f,%f,%f]\n", gesture_position.x, gesture_position.y, sender.zOffset);
#endif
        
        Input_push_event(input_record, {
            .state_type = UI_HOVER_STATE_ENDED,
            .input_type = UI_TOUCH_TYPE_POINTER,
            .pointer_info = {
                .azimuth = azimuth,
                .altitude_angle_radians = altitude,
                .azimuth_unit_vector = azimuth_unit_vector
            },
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .z_position = sender.zOffset,
            .count = sender.numberOfTouches
        });
    }
}


- (void)doubleTapGestureRecognition:(UITapGestureRecognizer *)sender {
    if (sender.state == UIGestureRecognizerStateRecognized) {
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_DOUBLE_TAP_NONPREFERRED,
            .input_type = UI_TOUCH_TYPE_DIRECT
        });
    }
}

- (void)swipeGestureRecognition:(UISwipeGestureRecognizer *)sender {
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    //sender.cancelsTouchesInView = false;
    
    vec2 direction;
    switch (sender.direction) {
        case UISwipeGestureRecognizerDirectionLeft: {
            direction = vec2(-1.0f, 0.0f);
            break;
        }
        case UISwipeGestureRecognizerDirectionRight: {
            direction = vec2(1.0f, 0.0f);
            break;
        }
        case UISwipeGestureRecognizerDirectionUp: {
            direction = vec2(0.0f, -1.0f);
            break;
        }
        case UISwipeGestureRecognizerDirectionDown: {
            direction = vec2(0.0f, 1.0f);
            break;
        }
        default: {
            break;
        }
    }
    
    if (sender.state == UIGestureRecognizerStateBegan) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_SWIPE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .scale = direction,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    } else if (sender.state == UIGestureRecognizerStateChanged) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];

        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_SWIPE_CHANGED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .scale = direction,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    } else if (sender.state == UIGestureRecognizerStateCancelled) {
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_SWIPE_CANCELLED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .scale = direction,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = sender.numberOfTouches
        });
    } else if (sender.state == UIGestureRecognizerStateEnded) {
          CGPoint gesture_position = [sender locationInView:_mtk_view];
          
          Input_push_event(input_record, {
              .state_type = UI_TOUCH_STATE_SWIPE_ENDED,
              .input_type = UI_TOUCH_TYPE_DIRECT,
              .scale = direction,
              .position = {(float32)gesture_position.x, (float32)gesture_position.y},
              .count = sender.numberOfTouches
          });
      }
    
}


- (void) pencilInteractionDidTap:(UIPencilInteraction *) interaction {
    switch (UIPencilInteraction.preferredTapAction) {
    default: {
        Input_push_event(&(_core_platform.core.input.users[_userID]), {
            .state_type = UI_TOUCH_STATE_DOUBLE_TAP_PREFERRED,
            .input_type = UI_TOUCH_TYPE_POINTER
        });
        break;
    }
//    case UIPencilPreferredActionIgnore:
//        break;
//    case UIPencilPreferredActionSwitchEraser:
//        break;
//    case UIPencilPreferredActionSwitchPrevious:
//        break;
//    case UIPencilPreferredActionShowColorPalette:
//        break;
    }
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer {
    
//    if ([gestureRecognizer isKindOfClass:[UILongPressGestureRecognizer class]] ||
//        [otherGestureRecognizer isKindOfClass:[UILongPressGestureRecognizer class]]) {
//        return NO;
//    }
    
    return YES;
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldBeRequiredToFailByGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer {

//    // If the gesture recognizer is a UITapGestureRecongizer, but the other
//    // gesture detected is a UILongPressGestureRecognizer, require the
//    // UITapGestureRecognizer to fail.
//    if ([otherGestureRecognizer isKindOfClass:[UILongPressGestureRecognizer class]] &&
//        [gestureRecognizer isKindOfClass:[UILongPressGestureRecognizer class]]) {
//
//       return YES;
//    } else {
//       return NO;
//    }
    
    return NO;
}
- (UIPointerRegion *)pointerInteraction:(UIPointerInteraction *)interaction regionForRequest:(UIPointerRegionRequest *)request defaultRegion:(UIPointerRegion *)defaultRegion {
    return [UIPointerRegion regionWithRect:self.view.bounds identifier:@"mtt_main"];
}
- (void)pointerInteraction:(UIPointerInteraction *)interaction willEnterRegion:(UIPointerRegion *)region animator:(id<UIPointerInteractionAnimating>)animator {

}
- (UIPointerStyle *)pointerInteraction:(UIPointerInteraction *)interaction styleForRegion:(UIPointerRegion *)region {

    if (![((NSString*)region.identifier) isEqualToString:@"mtt_main"]) {
        return nil;
    }
    return [UIPointerStyle systemPointerStyle];
}
- (void)pointerInteraction:(UIPointerInteraction *)interaction willExitRegion:(UIPointerRegion *)region animator:(id<UIPointerInteractionAnimating>)animator {

}

- (void)access_photo_library:(id)sender {

    
}

- (void) imagePickerController:(UIImagePickerController*)picker didFinishPickingMediaWithInfo:(NSDictionary<UIImagePickerControllerInfoKey,id> *)info {

    UIImage* chosen_image = info[UIImagePickerControllerOriginalImage];
  
    
    if (chosen_image == nil) {
        MTT_error("%s", "ERROR: image cannot be found\n");
        return;
    }
    
    NSLog(@"Loaded image %@\n", info[UIImagePickerControllerImageURL]);
    
    CGImageAlphaInfo alpha_info = CGImageGetAlphaInfo([chosen_image CGImage]);
    switch (alpha_info) {
    case kCGImageAlphaFirst:
        MTT_print("%s\n", "kCGImageAlphaFirst");
        break;
    case kCGImageAlphaLast:
        MTT_print("%s\n", "kCGImageAlphaLast");
        break;
    case kCGImageAlphaNone:
        MTT_print("%s\n", "kCGImageAlphaNone");
        break;
    case kCGImageAlphaNoneSkipFirst:
        MTT_print("%s\n", "kCGImageAlphaNoneSkipFirst");
        break;
    case kCGImageAlphaOnly:
        MTT_print("%s\n", "kCGImageAlphaOnly");
        break;
    case kCGImageAlphaNoneSkipLast:
        MTT_print("%s\n", "kCGImageAlphaNoneSkipLast");
        break;
    case kCGImageAlphaPremultipliedFirst:
        MTT_print("%s\n", "kCGImageAlphaPremultipliedFirst");
        break;
    case kCGImageAlphaPremultipliedLast:
        MTT_print("%s\n", "kCGImageAlphaPremultipliedLast");
        break;
    }



    [image_view setContentMode:UIViewContentModeCenter];
    image_view.layer.magnificationFilter = kCAFilterNearest;
    
    
    //image_view.layer.transform = CATransform3DTranslate(CATransform3DIdentity, 400, 200, 0);
    
    
    
    [image_view setImage:chosen_image];
//    image_view.backgroundColor =
//    [UIColor colorWithPatternImage:chosenImage];
    
    [picker dismissViewControllerAnimated:YES completion:^(void){
        [self initializeImagePicker:picker];
        
        
        [self presentViewController:picker animated:YES completion:nil];
    }];
}

- (void) initializeImagePicker:(UIImagePickerController*)picker {
    picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
    
    
    
    //_image_picker_view.layer.zPosition = 10000;
    
    
    picker.popoverPresentationController.sourceView = image_view;
    
    {
        
        CGFloat width = self.view.frame.size.width;
        CGFloat height = self.view.frame.size.height;
        //            CGFloat origin_x = self.view.frame.origin.x;
        //            CGFloat origin_y = self.view.frame.origin.y;
        
        CGRect popover_rect = CGRectMake(width, height, -width, height);
        
        picker.popoverPresentationController.sourceRect = popover_rect;
    }
}

- (void) imagePickerControllerDidCancel:(UIImagePickerController *)picker {
    [picker dismissViewControllerAnimated:YES completion:^(void){
    }];

}



/*
 #pragma mark - Navigation
 
 // In a storyboard-based application, you will often want to do a little preparation before navigation
 - (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
 // Get the new view controller using [segue destinationViewController].
 // Pass the selected object to the new view controller.
 }
 */



- (void) webView:(WKWebView *)webView decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction preferences:(WKWebpagePreferences *)preferences decisionHandler:(void (^)(WKNavigationActionPolicy, WKWebpagePreferences * _Nonnull))decisionHandler {
    MTT_print("%s\n", __PRETTY_FUNCTION__);
    
    preferences.preferredContentMode = WKContentModeMobile;
    
    decisionHandler(WKNavigationActionPolicyAllow, preferences);
}

- (void) webViewWebContentProcessDidTerminate:(WKWebView *)webView {
    MTT_print("%s\n", __PRETTY_FUNCTION__);
}

- (void) webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error {
    MTT_print("%s\n", __PRETTY_FUNCTION__);
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation withError:(NSError *)error {
    MTT_print("%s\n", __PRETTY_FUNCTION__);
}

- (void)webView:(WKWebView *)webView didCommitNavigation:(WKNavigation *)navigation {
    MTT_print("%s\n", __PRETTY_FUNCTION__);
    
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation {
    MTT_print("%s\n", __PRETTY_FUNCTION__);
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation {
    MTT_print("%s\n", __PRETTY_FUNCTION__);
}



- (void) presentWebBrowser:(NSURL*)url {
    @autoreleasepool {
    
        [_renderer pause];
    
        
        //[_header_view setHidden:NO];
        
        
        [self.view bringSubviewToFront:_web_view_container];
        
        //_web_view_container.userInteractionEnabled = YES;
        //_web_view.userInteractionEnabled           = YES;
//        self.view.userInteractionEnabled           = NO;
        
        for (UIGestureRecognizer* recognizer in main_gesture_recognizers) {
            [recognizer setEnabled:NO];
        }
        
        static NSURL* start_url = [NSURL URLWithString:@""];
        
        NSLog(@"%@\n", [url absoluteString]);
        if (!([start_url isEqual:url]) &&
            (([_web_view URL] == nil) || (![[_web_view URL] isEqual:url]))) {
            MTT_print("%s", "LOADING\n");
            NSURLRequest *request = [NSURLRequest requestWithURL:url];
            [_web_view loadRequest:request];
        }
        
        start_url = url;
        
//        [UIView animateWithDuration:0.0f animations:^{
//
//            [self->_web_view_container setAlpha:0.0f];
//
//
//        } completion:^(BOOL finished) {
//            [self->_web_view_container setHidden:NO];
//
//            [UIView animateWithDuration:0.0f animations:^{
//
//                [self->_web_view_container setAlpha:1.0f];
//
//            } completion:nil];
//
//        }];
        [_web_view_container setHidden:NO];
        //[self->_web_view_container setAlpha:0.0f];

    }
}

-(void)exitWebBrowser:(id)sender {
    //UIButton* exit_button = sender;
    
    @autoreleasepool {
        
        if ([[UIPasteboard generalPasteboard] hasImages]) {
            UIImage* image = [[UIPasteboard generalPasteboard] image];
            
            [image_view setContentMode:UIViewContentModeScaleAspectFit];
            image_view.layer.magnificationFilter = kCAFilterNearest;
            [image_view setImage:image];
            
        } else if ([[UIPasteboard generalPasteboard] hasURLs]) {
            NSURL* url = [[UIPasteboard generalPasteboard] URL];
            
            NSLog(@"URL copied: %@\n", url);
            
            NSData* data = [NSData dataWithContentsOfURL:url];
            if (data != nil) {
                UIImage* image = [UIImage imageWithData:data];
                
                [image_view setContentMode:UIViewContentModeScaleAspectFit];
                image_view.layer.magnificationFilter = kCAFilterNearest;
                [image_view setImage:image];
            }
        }
        
        UIPasteboard *pb = [UIPasteboard generalPasteboard];
        //////[pb declareTypes: [NSArray arrayWithObject:NSStringPboardType] owner: self];
        [pb setImages: nil];
        
        
        [_web_view_container setHidden:YES];
        
        //self.view.userInteractionEnabled           = YES;
        
        for (UIGestureRecognizer* recognizer in main_gesture_recognizers) {
            [recognizer setEnabled:YES];
        }
        
        [_renderer resume];
        
    }
}



- (void)picker:(nonnull PHPickerViewController *)picker didFinishPicking:(nonnull NSArray<PHPickerResult *> *)results {
   
    NSLog(@"-picker:%@ didFinishPicking:%@", picker, results);
    
    //[self clearImageViews];
    [picker dismissViewControllerAnimated:YES completion:nil];
    
    for (PHPickerResult *result in results) {
        NSLog(@"result: %@", result);
        
        NSLog(@"%@", result.assetIdentifier);
        NSLog(@"%@", result.itemProvider);
        
        NSLog(@"Is main thread=[%@]\n", ([NSThread isMainThread]) ? @"YES" : @"NO");
        
        NSMutableArray* images = [[NSMutableArray alloc] init];
        
        // Get UIImage objects
        [result.itemProvider loadObjectOfClass:[UIImage class] completionHandler:^(__kindof id<NSItemProviderReading>  _Nullable object, NSError * _Nullable error) {
            if (error != nil) {
                NSLog(@"object: %@, error: %@", object, error);
            } else {
                NSLog(@"object: %@", object);
            }
            
            if ([object isKindOfClass:[UIImage class]]) {
                UIImage* chosen_image = (UIImage*)object;
                [images addObject:chosen_image];
                
                CGImageAlphaInfo alpha_info = CGImageGetAlphaInfo([chosen_image CGImage]);
                
                switch (alpha_info) {
                case kCGImageAlphaFirst:
                    MTT_print("%s\n", "kCGImageAlphaFirst");
                    break;
                case kCGImageAlphaLast:
                    MTT_print("%s\n", "kCGImageAlphaLast");
                    break;
                case kCGImageAlphaNone:
                    MTT_print("%s\n", "kCGImageAlphaNone");
                    break;
                case kCGImageAlphaNoneSkipFirst:
                    MTT_print("%s\n", "kCGImageAlphaNoneSkipFirst");
                    break;
                case kCGImageAlphaOnly:
                    MTT_print("%s\n", "kCGImageAlphaOnly");
                    break;
                case kCGImageAlphaNoneSkipLast:
                    MTT_print("%s\n", "kCGImageAlphaNoneSkipLast");
                    break;
                case kCGImageAlphaPremultipliedFirst:
                    MTT_print("%s\n", "kCGImageAlphaPremultipliedFirst");
                    break;
                case kCGImageAlphaPremultipliedLast:
                    MTT_print("%s\n", "kCGImageAlphaPremultipliedLast");
                    break;
                }
                
                
                dispatch_async(dispatch_get_main_queue(), ^{
                    [self->image_view setContentMode:UIViewContentModeCenter];
                    self->image_view.layer.magnificationFilter = kCAFilterNearest;
                    [self->image_view setImage:chosen_image];
                });
            }
        }];
    }
}



- (void) share:(id)sender {
//    __block NSString* visualization = [self get_debug_visualization];
//    if (visualization == nil) {
//        return;
//    }
//
//    @autoreleasepool {
//        NSMutableArray* objects = [[NSMutableArray alloc] initWithArray:@[visualization]];
//
//        NSArray*objectsToShare = [[NSArray alloc] initWithArray:objects copyItems:YES];
//
//        UIActivityViewController *controller = [[UIActivityViewController alloc] initWithActivityItems:objectsToShare applicationActivities:nil];
//
//        // Exclude all activities except AirDrop and mail.
//        NSArray *excludedActivities = @[UIActivityTypePostToTwitter, UIActivityTypePostToFacebook,
//                                        UIActivityTypePostToWeibo,
//                                        UIActivityTypeMessage,
//                                        UIActivityTypePrint, UIActivityTypeCopyToPasteboard,
//                                        UIActivityTypeAssignToContact, UIActivityTypeSaveToCameraRoll,
//                                        UIActivityTypeAddToReadingList, UIActivityTypePostToFlickr,
//                                        UIActivityTypePostToVimeo, UIActivityTypePostToTencentWeibo];
//        controller.excludedActivityTypes = excludedActivities;
//
//        if (controller.popoverPresentationController != nil) {
//            controller.popoverPresentationController.sourceView = sender;
//        }
//
//
//
//        // Present the controller
//        [self presentViewController:controller animated:YES completion: ^{
//            [objects removeAllObjects];
//            visualization = nil;
//        }];
//
//    }
}

-(void) pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    //[super pressesBegan:presses withEvent:event];
    
    if (!_is_init) {
        return;
    }
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    
    @autoreleasepool {
    
        for (UIPress* press in event.allPresses) {
            check_set_modifier(event, press);
            UI_Event key_event = {};
            key_event.input_type = UI_TOUCH_TYPE_KEY;
            key_event.state_type = UI_TOUCH_STATE_KEY_BEGAN;
            key_event.key        = MTT_key_mapping[press.key.keyCode];
            key_event.key_sub    = ui_key_modifier_flags(press.key.modifierFlags);
            input_record->modifier_flags = ui_key_modifier_flags(press.key.modifierFlags);
            set_key_is_pressed(input_record, (UI_KEY_TYPE)key_event.key);
            Input_push_event(input_record, key_event);
            key_event.count = 0;
            if(![press.key.characters getCString:key_event.characters maxLength:sizeof(key_event.characters)/sizeof(*key_event.characters) encoding:NSUTF8StringEncoding]) {
                NSLog(@"Command buffer too small");
            } else {
                key_event.count = press.key.characters.length;
            }
            
            switch (press.key.keyCode) {

            case UIKeyboardHIDUsageKeyboardErrorRollOver:
                
                break;
            case UIKeyboardHIDUsageKeyboardPOSTFail:
                
                break;
            case UIKeyboardHIDUsageKeyboardErrorUndefined:
                
                break;
            case UIKeyboardHIDUsageKeyboardA:
                
                break;
            case UIKeyboardHIDUsageKeyboardB:
                
                break;
            case UIKeyboardHIDUsageKeyboardC:
                
                break;
            case UIKeyboardHIDUsageKeyboardD:
                
                break;
            case UIKeyboardHIDUsageKeyboardE:
                
                break;
            case UIKeyboardHIDUsageKeyboardF:
                
                break;
            case UIKeyboardHIDUsageKeyboardG:
                
                break;
            case UIKeyboardHIDUsageKeyboardH:
                
                break;
            case UIKeyboardHIDUsageKeyboardI:
                
                break;
            case UIKeyboardHIDUsageKeyboardJ:
                
                break;
            case UIKeyboardHIDUsageKeyboardK:
                
                break;
            case UIKeyboardHIDUsageKeyboardL:
                
                break;
            case UIKeyboardHIDUsageKeyboardM:
                
                break;
            case UIKeyboardHIDUsageKeyboardN:
                
                break;
            case UIKeyboardHIDUsageKeyboardO:
                
                break;
            case UIKeyboardHIDUsageKeyboardP:
                
                break;
            case UIKeyboardHIDUsageKeyboardQ:
                
                break;
            case UIKeyboardHIDUsageKeyboardR:
                
                break;
            case UIKeyboardHIDUsageKeyboardS:
                
                break;
            case UIKeyboardHIDUsageKeyboardT:
                
                break;
            case UIKeyboardHIDUsageKeyboardU:
                
                break;
            case UIKeyboardHIDUsageKeyboardV:
                
                break;
            case UIKeyboardHIDUsageKeyboardW:
                
                break;
            case UIKeyboardHIDUsageKeyboardX:
                
                break;
            case UIKeyboardHIDUsageKeyboardY:
                
                break;
            case UIKeyboardHIDUsageKeyboardZ:
                
                break;
            case UIKeyboardHIDUsageKeyboard1:
                
                break;
            case UIKeyboardHIDUsageKeyboard2:
                
                break;
            case UIKeyboardHIDUsageKeyboard3:
                
                break;
            case UIKeyboardHIDUsageKeyboard4:
                
                break;
            case UIKeyboardHIDUsageKeyboard5:
                
                break;
            case UIKeyboardHIDUsageKeyboard6:
                
                break;
            case UIKeyboardHIDUsageKeyboard7:
                
                break;
            case UIKeyboardHIDUsageKeyboard8:
                
                break;
            case UIKeyboardHIDUsageKeyboard9:
                
                break;
            case UIKeyboardHIDUsageKeyboard0:
                
                break;
            case UIKeyboardHIDUsageKeyboardReturnOrEnter:
                
                break;
            case UIKeyboardHIDUsageKeyboardEscape:
                
                break;
            case UIKeyboardHIDUsageKeyboardDeleteOrBackspace:
                
                break;
            case UIKeyboardHIDUsageKeyboardTab:
                
                break;
            case UIKeyboardHIDUsageKeyboardSpacebar:
                
                break;
            case UIKeyboardHIDUsageKeyboardHyphen:
                
                break;
            case UIKeyboardHIDUsageKeyboardEqualSign:
                
                break;
            case UIKeyboardHIDUsageKeyboardOpenBracket:
                
                break;
            case UIKeyboardHIDUsageKeyboardCloseBracket:
                
                break;
            case UIKeyboardHIDUsageKeyboardBackslash:
                
                break;
            case UIKeyboardHIDUsageKeyboardNonUSPound:
                
                break;
            case UIKeyboardHIDUsageKeyboardSemicolon:
                
                break;
            case UIKeyboardHIDUsageKeyboardQuote:
                
                break;
            case UIKeyboardHIDUsageKeyboardGraveAccentAndTilde:
                // TODO
                break;
            case UIKeyboardHIDUsageKeyboardComma:
                
                break;
            case UIKeyboardHIDUsageKeyboardPeriod:
                
                break;
            case UIKeyboardHIDUsageKeyboardSlash:
                
                break;
            case UIKeyboardHIDUsageKeyboardCapsLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardF1:
                
                break;
            case UIKeyboardHIDUsageKeyboardF2:
                
                break;
            case UIKeyboardHIDUsageKeyboardF3:
                
                break;
            case UIKeyboardHIDUsageKeyboardF4:
                
                break;
            case UIKeyboardHIDUsageKeyboardF5:
                
                break;
            case UIKeyboardHIDUsageKeyboardF6:
                
                break;
            case UIKeyboardHIDUsageKeyboardF7:
                
                break;
            case UIKeyboardHIDUsageKeyboardF8:
                
                break;
            case UIKeyboardHIDUsageKeyboardF9:
                
                break;
            case UIKeyboardHIDUsageKeyboardF10:
                
                break;
            case UIKeyboardHIDUsageKeyboardF11:
                
                break;
            case UIKeyboardHIDUsageKeyboardF12:
                
                break;
            case UIKeyboardHIDUsageKeyboardPrintScreen:
                
                break;
            case UIKeyboardHIDUsageKeyboardScrollLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardPause:
                
                break;
            case UIKeyboardHIDUsageKeyboardInsert:
                
                break;
            case UIKeyboardHIDUsageKeyboardHome:
                
                break;
            case UIKeyboardHIDUsageKeyboardPageUp:
                
                break;
            case UIKeyboardHIDUsageKeyboardDeleteForward:
                
                break;
            case UIKeyboardHIDUsageKeyboardEnd:
                
                break;
            case UIKeyboardHIDUsageKeyboardPageDown:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardDownArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardUpArrow:
                
                break;
            case UIKeyboardHIDUsageKeypadNumLock:
                
                break;
            case UIKeyboardHIDUsageKeypadSlash:
                
                break;
            case UIKeyboardHIDUsageKeypadAsterisk:
                
                break;
            case UIKeyboardHIDUsageKeypadHyphen:
                
                break;
            case UIKeyboardHIDUsageKeypadPlus:
                
                break;
            case UIKeyboardHIDUsageKeypadEnter:
                
                break;
            case UIKeyboardHIDUsageKeypad1:
                
                break;
            case UIKeyboardHIDUsageKeypad2:
                
                break;
            case UIKeyboardHIDUsageKeypad3:
                
                break;
            case UIKeyboardHIDUsageKeypad4:
                
                break;
            case UIKeyboardHIDUsageKeypad5:
                
                break;
            case UIKeyboardHIDUsageKeypad6:
                
                break;
            case UIKeyboardHIDUsageKeypad7:
                
                break;
            case UIKeyboardHIDUsageKeypad8:
                
                break;
            case UIKeyboardHIDUsageKeypad9:
                
                break;
            case UIKeyboardHIDUsageKeypad0:
                
                break;
            case UIKeyboardHIDUsageKeypadPeriod:
                
                break;
            case UIKeyboardHIDUsageKeyboardNonUSBackslash:
                
                break;
            case UIKeyboardHIDUsageKeyboardApplication:
                
                break;
            case UIKeyboardHIDUsageKeyboardPower:
                
                break;
            case UIKeyboardHIDUsageKeypadEqualSign:
                
                break;
            case UIKeyboardHIDUsageKeyboardF13:
                
                break;
            case UIKeyboardHIDUsageKeyboardF14:
                
                break;
            case UIKeyboardHIDUsageKeyboardF15:
                
                break;
            case UIKeyboardHIDUsageKeyboardF16:
                
                break;
            case UIKeyboardHIDUsageKeyboardF17:
                
                break;
            case UIKeyboardHIDUsageKeyboardF18:
                
                break;
            case UIKeyboardHIDUsageKeyboardF19:
                
                break;
            case UIKeyboardHIDUsageKeyboardF20:
                
                break;
            case UIKeyboardHIDUsageKeyboardF21:
                
                break;
            case UIKeyboardHIDUsageKeyboardF22:
                
                break;
            case UIKeyboardHIDUsageKeyboardF23:
                
                break;
            case UIKeyboardHIDUsageKeyboardF24:
                
                break;
            case UIKeyboardHIDUsageKeyboardExecute:
                
                break;
            case UIKeyboardHIDUsageKeyboardHelp:
                
                break;
            case UIKeyboardHIDUsageKeyboardMenu:
                
                break;
            case UIKeyboardHIDUsageKeyboardSelect:
                
                break;
            case UIKeyboardHIDUsageKeyboardStop:
                
                break;
            case UIKeyboardHIDUsageKeyboardAgain:
                
                break;
            case UIKeyboardHIDUsageKeyboardUndo:
                
                break;
            case UIKeyboardHIDUsageKeyboardCut:
                
                break;
            case UIKeyboardHIDUsageKeyboardCopy:
                
                break;
            case UIKeyboardHIDUsageKeyboardPaste:
                
                break;
            case UIKeyboardHIDUsageKeyboardFind:
                
                break;
            case UIKeyboardHIDUsageKeyboardMute:
                
                break;
            case UIKeyboardHIDUsageKeyboardVolumeUp:
                
                break;
            case UIKeyboardHIDUsageKeyboardVolumeDown:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingCapsLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingNumLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingScrollLock:
                
                break;
            case UIKeyboardHIDUsageKeypadComma:
                
                break;
            case UIKeyboardHIDUsageKeypadEqualSignAS400:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational1:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational2:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational3:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational4:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational5:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational6:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational7:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational8:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational9:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG1:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG2:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG3:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG4:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG5:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG6:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG7:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG8:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG9:
                
                break;
            case UIKeyboardHIDUsageKeyboardAlternateErase:
                
                break;
            case UIKeyboardHIDUsageKeyboardSysReqOrAttention:
                
                break;
            case UIKeyboardHIDUsageKeyboardCancel:
                
                break;
            case UIKeyboardHIDUsageKeyboardClear:
                
                break;
            case UIKeyboardHIDUsageKeyboardPrior:
                
                break;
            case UIKeyboardHIDUsageKeyboardReturn:
                
                break;
            case UIKeyboardHIDUsageKeyboardSeparator:
                
                break;
            case UIKeyboardHIDUsageKeyboardOut:
                
                break;
            case UIKeyboardHIDUsageKeyboardOper:
                
                break;
            case UIKeyboardHIDUsageKeyboardClearOrAgain:
                
                break;
            case UIKeyboardHIDUsageKeyboardCrSelOrProps:
                
                break;
            case UIKeyboardHIDUsageKeyboardExSel:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftControl:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftShift:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftAlt:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftGUI:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightControl:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightShift:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightAlt:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightGUI:
                
                break;
            case UIKeyboardHIDUsageKeyboard_Reserved:
                
                break;
            }
        }
        
    }
}

-(void) pressesChanged:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    //[super pressesChanged:presses withEvent:event];
    
    if (!_is_init) {
        return;
    }
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    
    @autoreleasepool {
    
        for (UIPress* press in event.allPresses) {
            UI_Event key_event = {};
            key_event.input_type = UI_TOUCH_TYPE_KEY;
            key_event.state_type = UI_TOUCH_STATE_KEY_CHANGED;
            key_event.key        = MTT_key_mapping[press.key.keyCode];
            key_event.key_sub    = ui_key_modifier_flags(press.key.modifierFlags);
            input_record->modifier_flags = ui_key_modifier_flags(press.key.modifierFlags);
            set_key_is_pressed(input_record, (UI_KEY_TYPE)key_event.key);
            key_event.count = 0;
            if(![press.key.characters getCString:key_event.characters maxLength:sizeof(key_event.characters)/sizeof(*key_event.characters) encoding:NSUTF8StringEncoding]) {
                NSLog(@"Command buffer too small");
            } else {
                key_event.count = press.key.characters.length;
            }
            
            Input_push_event(input_record, key_event);
            
            switch (press.key.keyCode) {

            case UIKeyboardHIDUsageKeyboardErrorRollOver:
                
                break;
            case UIKeyboardHIDUsageKeyboardPOSTFail:
                
                break;
            case UIKeyboardHIDUsageKeyboardErrorUndefined:
                
                break;
            case UIKeyboardHIDUsageKeyboardA:
                
                break;
            case UIKeyboardHIDUsageKeyboardB:
                
                break;
            case UIKeyboardHIDUsageKeyboardC:
                
                break;
            case UIKeyboardHIDUsageKeyboardD:
                
                break;
            case UIKeyboardHIDUsageKeyboardE:
                
                break;
            case UIKeyboardHIDUsageKeyboardF:
                
                break;
            case UIKeyboardHIDUsageKeyboardG:
                
                break;
            case UIKeyboardHIDUsageKeyboardH:
                
                break;
            case UIKeyboardHIDUsageKeyboardI:
                
                break;
            case UIKeyboardHIDUsageKeyboardJ:
                
                break;
            case UIKeyboardHIDUsageKeyboardK:
                
                break;
            case UIKeyboardHIDUsageKeyboardL:
                
                break;
            case UIKeyboardHIDUsageKeyboardM:
                
                break;
            case UIKeyboardHIDUsageKeyboardN:
                
                break;
            case UIKeyboardHIDUsageKeyboardO:
                
                break;
            case UIKeyboardHIDUsageKeyboardP:
                
                break;
            case UIKeyboardHIDUsageKeyboardQ:
                
                break;
            case UIKeyboardHIDUsageKeyboardR:
                
                break;
            case UIKeyboardHIDUsageKeyboardS:
                
                break;
            case UIKeyboardHIDUsageKeyboardT:
                
                break;
            case UIKeyboardHIDUsageKeyboardU:
                
                break;
            case UIKeyboardHIDUsageKeyboardV:
                
                break;
            case UIKeyboardHIDUsageKeyboardW:
                
                break;
            case UIKeyboardHIDUsageKeyboardX:
                
                break;
            case UIKeyboardHIDUsageKeyboardY:
                
                break;
            case UIKeyboardHIDUsageKeyboardZ:
                
                break;
            case UIKeyboardHIDUsageKeyboard1:
                
                break;
            case UIKeyboardHIDUsageKeyboard2:
                
                break;
            case UIKeyboardHIDUsageKeyboard3:
                
                break;
            case UIKeyboardHIDUsageKeyboard4:
                
                break;
            case UIKeyboardHIDUsageKeyboard5:
                
                break;
            case UIKeyboardHIDUsageKeyboard6:
                
                break;
            case UIKeyboardHIDUsageKeyboard7:
                
                break;
            case UIKeyboardHIDUsageKeyboard8:
                
                break;
            case UIKeyboardHIDUsageKeyboard9:
                
                break;
            case UIKeyboardHIDUsageKeyboard0:
                
                break;
            case UIKeyboardHIDUsageKeyboardReturnOrEnter:
                
                break;
            case UIKeyboardHIDUsageKeyboardEscape:
                
                break;
            case UIKeyboardHIDUsageKeyboardDeleteOrBackspace:
                
                break;
            case UIKeyboardHIDUsageKeyboardTab:
                
                break;
            case UIKeyboardHIDUsageKeyboardSpacebar:
                
                break;
            case UIKeyboardHIDUsageKeyboardHyphen:
                
                break;
            case UIKeyboardHIDUsageKeyboardEqualSign:
                
                break;
            case UIKeyboardHIDUsageKeyboardOpenBracket:
                
                break;
            case UIKeyboardHIDUsageKeyboardCloseBracket:
                
                break;
            case UIKeyboardHIDUsageKeyboardBackslash:
                
                break;
            case UIKeyboardHIDUsageKeyboardNonUSPound:
                
                break;
            case UIKeyboardHIDUsageKeyboardSemicolon:
                
                break;
            case UIKeyboardHIDUsageKeyboardQuote:
                
                break;
            case UIKeyboardHIDUsageKeyboardGraveAccentAndTilde:
                
                break;
            case UIKeyboardHIDUsageKeyboardComma:
                
                break;
            case UIKeyboardHIDUsageKeyboardPeriod:
                
                break;
            case UIKeyboardHIDUsageKeyboardSlash:
                
                break;
            case UIKeyboardHIDUsageKeyboardCapsLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardF1:
                
                break;
            case UIKeyboardHIDUsageKeyboardF2:
                
                break;
            case UIKeyboardHIDUsageKeyboardF3:
                
                break;
            case UIKeyboardHIDUsageKeyboardF4:
                
                break;
            case UIKeyboardHIDUsageKeyboardF5:
                
                break;
            case UIKeyboardHIDUsageKeyboardF6:
                
                break;
            case UIKeyboardHIDUsageKeyboardF7:
                
                break;
            case UIKeyboardHIDUsageKeyboardF8:
                
                break;
            case UIKeyboardHIDUsageKeyboardF9:
                
                break;
            case UIKeyboardHIDUsageKeyboardF10:
                
                break;
            case UIKeyboardHIDUsageKeyboardF11:
                
                break;
            case UIKeyboardHIDUsageKeyboardF12:
                
                break;
            case UIKeyboardHIDUsageKeyboardPrintScreen:
                
                break;
            case UIKeyboardHIDUsageKeyboardScrollLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardPause:
                
                break;
            case UIKeyboardHIDUsageKeyboardInsert:
                
                break;
            case UIKeyboardHIDUsageKeyboardHome:
                
                break;
            case UIKeyboardHIDUsageKeyboardPageUp:
                
                break;
            case UIKeyboardHIDUsageKeyboardDeleteForward:
                
                break;
            case UIKeyboardHIDUsageKeyboardEnd:
                
                break;
            case UIKeyboardHIDUsageKeyboardPageDown:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardDownArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardUpArrow:
                
                break;
            case UIKeyboardHIDUsageKeypadNumLock:
                
                break;
            case UIKeyboardHIDUsageKeypadSlash:
                
                break;
            case UIKeyboardHIDUsageKeypadAsterisk:
                
                break;
            case UIKeyboardHIDUsageKeypadHyphen:
                
                break;
            case UIKeyboardHIDUsageKeypadPlus:
                
                break;
            case UIKeyboardHIDUsageKeypadEnter:
                
                break;
            case UIKeyboardHIDUsageKeypad1:
                
                break;
            case UIKeyboardHIDUsageKeypad2:
                
                break;
            case UIKeyboardHIDUsageKeypad3:
                
                break;
            case UIKeyboardHIDUsageKeypad4:
                
                break;
            case UIKeyboardHIDUsageKeypad5:
                
                break;
            case UIKeyboardHIDUsageKeypad6:
                
                break;
            case UIKeyboardHIDUsageKeypad7:
                
                break;
            case UIKeyboardHIDUsageKeypad8:
                
                break;
            case UIKeyboardHIDUsageKeypad9:
                
                break;
            case UIKeyboardHIDUsageKeypad0:
                
                break;
            case UIKeyboardHIDUsageKeypadPeriod:
                
                break;
            case UIKeyboardHIDUsageKeyboardNonUSBackslash:
                
                break;
            case UIKeyboardHIDUsageKeyboardApplication:
                
                break;
            case UIKeyboardHIDUsageKeyboardPower:
                
                break;
            case UIKeyboardHIDUsageKeypadEqualSign:
                
                break;
            case UIKeyboardHIDUsageKeyboardF13:
                
                break;
            case UIKeyboardHIDUsageKeyboardF14:
                
                break;
            case UIKeyboardHIDUsageKeyboardF15:
                
                break;
            case UIKeyboardHIDUsageKeyboardF16:
                
                break;
            case UIKeyboardHIDUsageKeyboardF17:
                
                break;
            case UIKeyboardHIDUsageKeyboardF18:
                
                break;
            case UIKeyboardHIDUsageKeyboardF19:
                
                break;
            case UIKeyboardHIDUsageKeyboardF20:
                
                break;
            case UIKeyboardHIDUsageKeyboardF21:
                
                break;
            case UIKeyboardHIDUsageKeyboardF22:
                
                break;
            case UIKeyboardHIDUsageKeyboardF23:
                
                break;
            case UIKeyboardHIDUsageKeyboardF24:
                
                break;
            case UIKeyboardHIDUsageKeyboardExecute:
                
                break;
            case UIKeyboardHIDUsageKeyboardHelp:
                
                break;
            case UIKeyboardHIDUsageKeyboardMenu:
                
                break;
            case UIKeyboardHIDUsageKeyboardSelect:
                
                break;
            case UIKeyboardHIDUsageKeyboardStop:
                
                break;
            case UIKeyboardHIDUsageKeyboardAgain:
                
                break;
            case UIKeyboardHIDUsageKeyboardUndo:
                
                break;
            case UIKeyboardHIDUsageKeyboardCut:
                
                break;
            case UIKeyboardHIDUsageKeyboardCopy:
                
                break;
            case UIKeyboardHIDUsageKeyboardPaste:
                
                break;
            case UIKeyboardHIDUsageKeyboardFind:
                
                break;
            case UIKeyboardHIDUsageKeyboardMute:
                
                break;
            case UIKeyboardHIDUsageKeyboardVolumeUp:
                
                break;
            case UIKeyboardHIDUsageKeyboardVolumeDown:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingCapsLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingNumLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingScrollLock:
                
                break;
            case UIKeyboardHIDUsageKeypadComma:
                
                break;
            case UIKeyboardHIDUsageKeypadEqualSignAS400:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational1:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational2:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational3:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational4:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational5:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational6:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational7:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational8:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational9:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG1:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG2:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG3:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG4:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG5:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG6:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG7:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG8:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG9:
                
                break;
            case UIKeyboardHIDUsageKeyboardAlternateErase:
                
                break;
            case UIKeyboardHIDUsageKeyboardSysReqOrAttention:
                
                break;
            case UIKeyboardHIDUsageKeyboardCancel:
                
                break;
            case UIKeyboardHIDUsageKeyboardClear:
                
                break;
            case UIKeyboardHIDUsageKeyboardPrior:
                
                break;
            case UIKeyboardHIDUsageKeyboardReturn:
                
                break;
            case UIKeyboardHIDUsageKeyboardSeparator:
                
                break;
            case UIKeyboardHIDUsageKeyboardOut:
                
                break;
            case UIKeyboardHIDUsageKeyboardOper:
                
                break;
            case UIKeyboardHIDUsageKeyboardClearOrAgain:
                
                break;
            case UIKeyboardHIDUsageKeyboardCrSelOrProps:
                
                break;
            case UIKeyboardHIDUsageKeyboardExSel:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftControl:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftShift:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftAlt:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftGUI:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightControl:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightShift:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightAlt:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightGUI:
                
                break;
            case UIKeyboardHIDUsageKeyboard_Reserved:
                
                break;
            }
        }
        
    }
}

-(void) pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    //[super pressesCancelled:presses withEvent:event];
    
    if (!_is_init) {
        return;
    }
    
    
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    
    @autoreleasepool {
    
        for (UIPress* press in event.allPresses) {
            check_unset_modifier(event, press);
            UI_Event key_event = {};
            key_event.input_type = UI_TOUCH_TYPE_KEY;
            key_event.state_type = UI_TOUCH_STATE_KEY_CANCELLED;
            key_event.key        = MTT_key_mapping[press.key.keyCode];
            key_event.key_sub    = ui_key_modifier_flags(press.key.modifierFlags);
            input_record->modifier_flags = ui_key_modifier_flags(press.key.modifierFlags);
            unset_key_is_pressed(input_record, (UI_KEY_TYPE)key_event.key);
            key_event.count = 0;
            if(![press.key.characters getCString:key_event.characters maxLength:sizeof(key_event.characters)/sizeof(*key_event.characters) encoding:NSUTF8StringEncoding]) {
                NSLog(@"Command buffer too small");
            } else {
                key_event.count = press.key.characters.length;
            }
            
            Input_push_event(input_record, key_event);
            
            switch (press.key.keyCode) {

            case UIKeyboardHIDUsageKeyboardErrorRollOver:
                
                break;
            case UIKeyboardHIDUsageKeyboardPOSTFail:
                
                break;
            case UIKeyboardHIDUsageKeyboardErrorUndefined:
                
                break;
            case UIKeyboardHIDUsageKeyboardA:
                
                break;
            case UIKeyboardHIDUsageKeyboardB:
                
                break;
            case UIKeyboardHIDUsageKeyboardC:
                
                break;
            case UIKeyboardHIDUsageKeyboardD:
                
                break;
            case UIKeyboardHIDUsageKeyboardE:
                
                break;
            case UIKeyboardHIDUsageKeyboardF:
                
                break;
            case UIKeyboardHIDUsageKeyboardG:
                
                break;
            case UIKeyboardHIDUsageKeyboardH:
                
                break;
            case UIKeyboardHIDUsageKeyboardI:
                
                break;
            case UIKeyboardHIDUsageKeyboardJ:
                
                break;
            case UIKeyboardHIDUsageKeyboardK:
                
                break;
            case UIKeyboardHIDUsageKeyboardL:
                
                break;
            case UIKeyboardHIDUsageKeyboardM:
                
                break;
            case UIKeyboardHIDUsageKeyboardN:
                
                break;
            case UIKeyboardHIDUsageKeyboardO:
                
                break;
            case UIKeyboardHIDUsageKeyboardP:
                
                break;
            case UIKeyboardHIDUsageKeyboardQ:
                
                break;
            case UIKeyboardHIDUsageKeyboardR:
                
                break;
            case UIKeyboardHIDUsageKeyboardS:
                
                break;
            case UIKeyboardHIDUsageKeyboardT:
                
                break;
            case UIKeyboardHIDUsageKeyboardU:
                
                break;
            case UIKeyboardHIDUsageKeyboardV:
                
                break;
            case UIKeyboardHIDUsageKeyboardW:
                
                break;
            case UIKeyboardHIDUsageKeyboardX:
                
                break;
            case UIKeyboardHIDUsageKeyboardY:
                
                break;
            case UIKeyboardHIDUsageKeyboardZ:
                
                break;
            case UIKeyboardHIDUsageKeyboard1:
                
                break;
            case UIKeyboardHIDUsageKeyboard2:
                
                break;
            case UIKeyboardHIDUsageKeyboard3:
                
                break;
            case UIKeyboardHIDUsageKeyboard4:
                
                break;
            case UIKeyboardHIDUsageKeyboard5:
                
                break;
            case UIKeyboardHIDUsageKeyboard6:
                
                break;
            case UIKeyboardHIDUsageKeyboard7:
                
                break;
            case UIKeyboardHIDUsageKeyboard8:
                
                break;
            case UIKeyboardHIDUsageKeyboard9:
                
                break;
            case UIKeyboardHIDUsageKeyboard0:
                
                break;
            case UIKeyboardHIDUsageKeyboardReturnOrEnter:
                
                break;
            case UIKeyboardHIDUsageKeyboardEscape:
                
                break;
            case UIKeyboardHIDUsageKeyboardDeleteOrBackspace:
                
                break;
            case UIKeyboardHIDUsageKeyboardTab:
                
                break;
            case UIKeyboardHIDUsageKeyboardSpacebar:
                
                break;
            case UIKeyboardHIDUsageKeyboardHyphen:
                
                break;
            case UIKeyboardHIDUsageKeyboardEqualSign:
                
                break;
            case UIKeyboardHIDUsageKeyboardOpenBracket:
                
                break;
            case UIKeyboardHIDUsageKeyboardCloseBracket:
                
                break;
            case UIKeyboardHIDUsageKeyboardBackslash:
                
                break;
            case UIKeyboardHIDUsageKeyboardNonUSPound:
                
                break;
            case UIKeyboardHIDUsageKeyboardSemicolon:
                
                break;
            case UIKeyboardHIDUsageKeyboardQuote:
                
                break;
            case UIKeyboardHIDUsageKeyboardGraveAccentAndTilde:
                
                break;
            case UIKeyboardHIDUsageKeyboardComma:
                
                break;
            case UIKeyboardHIDUsageKeyboardPeriod:
                
                break;
            case UIKeyboardHIDUsageKeyboardSlash:
                
                break;
            case UIKeyboardHIDUsageKeyboardCapsLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardF1:
                
                break;
            case UIKeyboardHIDUsageKeyboardF2:
                
                break;
            case UIKeyboardHIDUsageKeyboardF3:
                
                break;
            case UIKeyboardHIDUsageKeyboardF4:
                
                break;
            case UIKeyboardHIDUsageKeyboardF5:
                
                break;
            case UIKeyboardHIDUsageKeyboardF6:
                
                break;
            case UIKeyboardHIDUsageKeyboardF7:
                
                break;
            case UIKeyboardHIDUsageKeyboardF8:
                
                break;
            case UIKeyboardHIDUsageKeyboardF9:
                
                break;
            case UIKeyboardHIDUsageKeyboardF10:
                
                break;
            case UIKeyboardHIDUsageKeyboardF11:
                
                break;
            case UIKeyboardHIDUsageKeyboardF12:
                
                break;
            case UIKeyboardHIDUsageKeyboardPrintScreen:
                
                break;
            case UIKeyboardHIDUsageKeyboardScrollLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardPause:
                
                break;
            case UIKeyboardHIDUsageKeyboardInsert:
                
                break;
            case UIKeyboardHIDUsageKeyboardHome:
                
                break;
            case UIKeyboardHIDUsageKeyboardPageUp:
                
                break;
            case UIKeyboardHIDUsageKeyboardDeleteForward:
                
                break;
            case UIKeyboardHIDUsageKeyboardEnd:
                
                break;
            case UIKeyboardHIDUsageKeyboardPageDown:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardDownArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardUpArrow:
                
                break;
            case UIKeyboardHIDUsageKeypadNumLock:
                
                break;
            case UIKeyboardHIDUsageKeypadSlash:
                
                break;
            case UIKeyboardHIDUsageKeypadAsterisk:
                
                break;
            case UIKeyboardHIDUsageKeypadHyphen:
                
                break;
            case UIKeyboardHIDUsageKeypadPlus:
                
                break;
            case UIKeyboardHIDUsageKeypadEnter:
                
                break;
            case UIKeyboardHIDUsageKeypad1:
                
                break;
            case UIKeyboardHIDUsageKeypad2:
                
                break;
            case UIKeyboardHIDUsageKeypad3:
                
                break;
            case UIKeyboardHIDUsageKeypad4:
                
                break;
            case UIKeyboardHIDUsageKeypad5:
                
                break;
            case UIKeyboardHIDUsageKeypad6:
                
                break;
            case UIKeyboardHIDUsageKeypad7:
                
                break;
            case UIKeyboardHIDUsageKeypad8:
                
                break;
            case UIKeyboardHIDUsageKeypad9:
                
                break;
            case UIKeyboardHIDUsageKeypad0:
                
                break;
            case UIKeyboardHIDUsageKeypadPeriod:
                
                break;
            case UIKeyboardHIDUsageKeyboardNonUSBackslash:
                
                break;
            case UIKeyboardHIDUsageKeyboardApplication:
                
                break;
            case UIKeyboardHIDUsageKeyboardPower:
                
                break;
            case UIKeyboardHIDUsageKeypadEqualSign:
                
                break;
            case UIKeyboardHIDUsageKeyboardF13:
                
                break;
            case UIKeyboardHIDUsageKeyboardF14:
                
                break;
            case UIKeyboardHIDUsageKeyboardF15:
                
                break;
            case UIKeyboardHIDUsageKeyboardF16:
                
                break;
            case UIKeyboardHIDUsageKeyboardF17:
                
                break;
            case UIKeyboardHIDUsageKeyboardF18:
                
                break;
            case UIKeyboardHIDUsageKeyboardF19:
                
                break;
            case UIKeyboardHIDUsageKeyboardF20:
                
                break;
            case UIKeyboardHIDUsageKeyboardF21:
                
                break;
            case UIKeyboardHIDUsageKeyboardF22:
                
                break;
            case UIKeyboardHIDUsageKeyboardF23:
                
                break;
            case UIKeyboardHIDUsageKeyboardF24:
                
                break;
            case UIKeyboardHIDUsageKeyboardExecute:
                
                break;
            case UIKeyboardHIDUsageKeyboardHelp:
                
                break;
            case UIKeyboardHIDUsageKeyboardMenu:
                
                break;
            case UIKeyboardHIDUsageKeyboardSelect:
                
                break;
            case UIKeyboardHIDUsageKeyboardStop:
                
                break;
            case UIKeyboardHIDUsageKeyboardAgain:
                
                break;
            case UIKeyboardHIDUsageKeyboardUndo:
                
                break;
            case UIKeyboardHIDUsageKeyboardCut:
                
                break;
            case UIKeyboardHIDUsageKeyboardCopy:
                
                break;
            case UIKeyboardHIDUsageKeyboardPaste:
                
                break;
            case UIKeyboardHIDUsageKeyboardFind:
                
                break;
            case UIKeyboardHIDUsageKeyboardMute:
                
                break;
            case UIKeyboardHIDUsageKeyboardVolumeUp:
                
                break;
            case UIKeyboardHIDUsageKeyboardVolumeDown:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingCapsLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingNumLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingScrollLock:
                
                break;
            case UIKeyboardHIDUsageKeypadComma:
                
                break;
            case UIKeyboardHIDUsageKeypadEqualSignAS400:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational1:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational2:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational3:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational4:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational5:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational6:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational7:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational8:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational9:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG1:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG2:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG3:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG4:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG5:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG6:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG7:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG8:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG9:
                
                break;
            case UIKeyboardHIDUsageKeyboardAlternateErase:
                
                break;
            case UIKeyboardHIDUsageKeyboardSysReqOrAttention:
                
                break;
            case UIKeyboardHIDUsageKeyboardCancel:
                
                break;
            case UIKeyboardHIDUsageKeyboardClear:
                
                break;
            case UIKeyboardHIDUsageKeyboardPrior:
                
                break;
            case UIKeyboardHIDUsageKeyboardReturn:
                
                break;
            case UIKeyboardHIDUsageKeyboardSeparator:
                
                break;
            case UIKeyboardHIDUsageKeyboardOut:
                
                break;
            case UIKeyboardHIDUsageKeyboardOper:
                
                break;
            case UIKeyboardHIDUsageKeyboardClearOrAgain:
                
                break;
            case UIKeyboardHIDUsageKeyboardCrSelOrProps:
                
                break;
            case UIKeyboardHIDUsageKeyboardExSel:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftControl:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftShift:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftAlt:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftGUI:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightControl:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightShift:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightAlt:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightGUI:
                
                break;
            case UIKeyboardHIDUsageKeyboard_Reserved:
                
                break;
            }
        }
        
    }
}

-(void) pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    //[super pressesEnded:presses withEvent:event];
    
    if (!_is_init) {
        return;
    }
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    
    @autoreleasepool {
    
        for (UIPress* press in event.allPresses) {
            check_unset_modifier(event, press);
            UI_Event key_event = {};
            key_event.input_type = UI_TOUCH_TYPE_KEY;
            key_event.state_type = UI_TOUCH_STATE_KEY_ENDED;
            key_event.key        = MTT_key_mapping[press.key.keyCode];
            key_event.key_sub    = ui_key_modifier_flags(press.key.modifierFlags);
            input_record->modifier_flags = ui_key_modifier_flags(press.key.modifierFlags);
            key_event.count = 0;
            if(![press.key.characters getCString:key_event.characters maxLength:sizeof(key_event.characters)/sizeof(*key_event.characters) encoding:NSUTF8StringEncoding]) {
                NSLog(@"Command buffer too small");
            } else {
                key_event.count = press.key.characters.length;
            }
            
            Input_push_event(input_record, key_event);
            
            switch (press.key.keyCode) {

            case UIKeyboardHIDUsageKeyboardErrorRollOver:
                
                break;
            case UIKeyboardHIDUsageKeyboardPOSTFail:
                
                break;
            case UIKeyboardHIDUsageKeyboardErrorUndefined:
                
                break;
            case UIKeyboardHIDUsageKeyboardA:
                
                break;
            case UIKeyboardHIDUsageKeyboardB:
                
                break;
            case UIKeyboardHIDUsageKeyboardC:
                
                break;
            case UIKeyboardHIDUsageKeyboardD:
                
                break;
            case UIKeyboardHIDUsageKeyboardE:
                
                break;
            case UIKeyboardHIDUsageKeyboardF:
                
                break;
            case UIKeyboardHIDUsageKeyboardG:
                
                break;
            case UIKeyboardHIDUsageKeyboardH:
                
                break;
            case UIKeyboardHIDUsageKeyboardI:
                
                break;
            case UIKeyboardHIDUsageKeyboardJ:
                
                break;
            case UIKeyboardHIDUsageKeyboardK:
                
                break;
            case UIKeyboardHIDUsageKeyboardL:
                
                break;
            case UIKeyboardHIDUsageKeyboardM:
                
                break;
            case UIKeyboardHIDUsageKeyboardN:
                
                break;
            case UIKeyboardHIDUsageKeyboardO:
                
                break;
            case UIKeyboardHIDUsageKeyboardP:
                
                break;
            case UIKeyboardHIDUsageKeyboardQ:
                
                break;
            case UIKeyboardHIDUsageKeyboardR:
                
                break;
            case UIKeyboardHIDUsageKeyboardS:
                
                break;
            case UIKeyboardHIDUsageKeyboardT:
                
                break;
            case UIKeyboardHIDUsageKeyboardU:
                
                break;
            case UIKeyboardHIDUsageKeyboardV:
                
                break;
            case UIKeyboardHIDUsageKeyboardW:
                
                break;
            case UIKeyboardHIDUsageKeyboardX:
                
                break;
            case UIKeyboardHIDUsageKeyboardY:
                
                break;
            case UIKeyboardHIDUsageKeyboardZ:
                
                break;
            case UIKeyboardHIDUsageKeyboard1:
                
                break;
            case UIKeyboardHIDUsageKeyboard2:
                
                break;
            case UIKeyboardHIDUsageKeyboard3:
                
                break;
            case UIKeyboardHIDUsageKeyboard4:
                
                break;
            case UIKeyboardHIDUsageKeyboard5:
                
                break;
            case UIKeyboardHIDUsageKeyboard6:
                
                break;
            case UIKeyboardHIDUsageKeyboard7:
                
                break;
            case UIKeyboardHIDUsageKeyboard8:
                
                break;
            case UIKeyboardHIDUsageKeyboard9:
                
                break;
            case UIKeyboardHIDUsageKeyboard0:
                
                break;
            case UIKeyboardHIDUsageKeyboardReturnOrEnter:
                
                break;
            case UIKeyboardHIDUsageKeyboardEscape:
                
                break;
            case UIKeyboardHIDUsageKeyboardDeleteOrBackspace:
                
                break;
            case UIKeyboardHIDUsageKeyboardTab:
                
                break;
            case UIKeyboardHIDUsageKeyboardSpacebar:
                
                break;
            case UIKeyboardHIDUsageKeyboardHyphen:
                
                break;
            case UIKeyboardHIDUsageKeyboardEqualSign:
                
                break;
            case UIKeyboardHIDUsageKeyboardOpenBracket:
                
                break;
            case UIKeyboardHIDUsageKeyboardCloseBracket:
                
                break;
            case UIKeyboardHIDUsageKeyboardBackslash:
                
                break;
            case UIKeyboardHIDUsageKeyboardNonUSPound:
                
                break;
            case UIKeyboardHIDUsageKeyboardSemicolon:
                
                break;
            case UIKeyboardHIDUsageKeyboardQuote:
                
                break;
            case UIKeyboardHIDUsageKeyboardGraveAccentAndTilde:
                
                break;
            case UIKeyboardHIDUsageKeyboardComma:
                
                break;
            case UIKeyboardHIDUsageKeyboardPeriod:
                
                break;
            case UIKeyboardHIDUsageKeyboardSlash:
                
                break;
            case UIKeyboardHIDUsageKeyboardCapsLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardF1:
                
                break;
            case UIKeyboardHIDUsageKeyboardF2:
                
                break;
            case UIKeyboardHIDUsageKeyboardF3:
                
                break;
            case UIKeyboardHIDUsageKeyboardF4:
                
                break;
            case UIKeyboardHIDUsageKeyboardF5:
                
                break;
            case UIKeyboardHIDUsageKeyboardF6:
                
                break;
            case UIKeyboardHIDUsageKeyboardF7:
                
                break;
            case UIKeyboardHIDUsageKeyboardF8:
                
                break;
            case UIKeyboardHIDUsageKeyboardF9:
                
                break;
            case UIKeyboardHIDUsageKeyboardF10:
                
                break;
            case UIKeyboardHIDUsageKeyboardF11:
                
                break;
            case UIKeyboardHIDUsageKeyboardF12:
                
                break;
            case UIKeyboardHIDUsageKeyboardPrintScreen:
                
                break;
            case UIKeyboardHIDUsageKeyboardScrollLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardPause:
                
                break;
            case UIKeyboardHIDUsageKeyboardInsert:
                
                break;
            case UIKeyboardHIDUsageKeyboardHome:
                
                break;
            case UIKeyboardHIDUsageKeyboardPageUp:
                
                break;
            case UIKeyboardHIDUsageKeyboardDeleteForward:
                
                break;
            case UIKeyboardHIDUsageKeyboardEnd:
                
                break;
            case UIKeyboardHIDUsageKeyboardPageDown:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardDownArrow:
                
                break;
            case UIKeyboardHIDUsageKeyboardUpArrow:
                
                break;
            case UIKeyboardHIDUsageKeypadNumLock:
                
                break;
            case UIKeyboardHIDUsageKeypadSlash:
                
                break;
            case UIKeyboardHIDUsageKeypadAsterisk:
                
                break;
            case UIKeyboardHIDUsageKeypadHyphen:
                
                break;
            case UIKeyboardHIDUsageKeypadPlus:
                
                break;
            case UIKeyboardHIDUsageKeypadEnter:
                
                break;
            case UIKeyboardHIDUsageKeypad1:
                
                break;
            case UIKeyboardHIDUsageKeypad2:
                
                break;
            case UIKeyboardHIDUsageKeypad3:
                
                break;
            case UIKeyboardHIDUsageKeypad4:
                
                break;
            case UIKeyboardHIDUsageKeypad5:
                
                break;
            case UIKeyboardHIDUsageKeypad6:
                
                break;
            case UIKeyboardHIDUsageKeypad7:
                
                break;
            case UIKeyboardHIDUsageKeypad8:
                
                break;
            case UIKeyboardHIDUsageKeypad9:
                
                break;
            case UIKeyboardHIDUsageKeypad0:
                
                break;
            case UIKeyboardHIDUsageKeypadPeriod:
                
                break;
            case UIKeyboardHIDUsageKeyboardNonUSBackslash:
                
                break;
            case UIKeyboardHIDUsageKeyboardApplication:
                
                break;
            case UIKeyboardHIDUsageKeyboardPower:
                
                break;
            case UIKeyboardHIDUsageKeypadEqualSign:
                
                break;
            case UIKeyboardHIDUsageKeyboardF13:
                
                break;
            case UIKeyboardHIDUsageKeyboardF14:
                
                break;
            case UIKeyboardHIDUsageKeyboardF15:
                
                break;
            case UIKeyboardHIDUsageKeyboardF16:
                
                break;
            case UIKeyboardHIDUsageKeyboardF17:
                
                break;
            case UIKeyboardHIDUsageKeyboardF18:
                
                break;
            case UIKeyboardHIDUsageKeyboardF19:
                
                break;
            case UIKeyboardHIDUsageKeyboardF20:
                
                break;
            case UIKeyboardHIDUsageKeyboardF21:
                
                break;
            case UIKeyboardHIDUsageKeyboardF22:
                
                break;
            case UIKeyboardHIDUsageKeyboardF23:
                
                break;
            case UIKeyboardHIDUsageKeyboardF24:
                
                break;
            case UIKeyboardHIDUsageKeyboardExecute:
                
                break;
            case UIKeyboardHIDUsageKeyboardHelp:
                
                break;
            case UIKeyboardHIDUsageKeyboardMenu:
                
                break;
            case UIKeyboardHIDUsageKeyboardSelect:
                
                break;
            case UIKeyboardHIDUsageKeyboardStop:
                
                break;
            case UIKeyboardHIDUsageKeyboardAgain:
                
                break;
            case UIKeyboardHIDUsageKeyboardUndo:
                
                break;
            case UIKeyboardHIDUsageKeyboardCut:
                
                break;
            case UIKeyboardHIDUsageKeyboardCopy:
                
                break;
            case UIKeyboardHIDUsageKeyboardPaste:
                
                break;
            case UIKeyboardHIDUsageKeyboardFind:
                
                break;
            case UIKeyboardHIDUsageKeyboardMute:
                
                break;
            case UIKeyboardHIDUsageKeyboardVolumeUp:
                
                break;
            case UIKeyboardHIDUsageKeyboardVolumeDown:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingCapsLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingNumLock:
                
                break;
            case UIKeyboardHIDUsageKeyboardLockingScrollLock:
                
                break;
            case UIKeyboardHIDUsageKeypadComma:
                
                break;
            case UIKeyboardHIDUsageKeypadEqualSignAS400:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational1:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational2:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational3:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational4:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational5:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational6:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational7:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational8:
                
                break;
            case UIKeyboardHIDUsageKeyboardInternational9:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG1:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG2:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG3:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG4:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG5:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG6:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG7:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG8:
                
                break;
            case UIKeyboardHIDUsageKeyboardLANG9:
                
                break;
            case UIKeyboardHIDUsageKeyboardAlternateErase:
                
                break;
            case UIKeyboardHIDUsageKeyboardSysReqOrAttention:
                
                break;
            case UIKeyboardHIDUsageKeyboardCancel:
                
                break;
            case UIKeyboardHIDUsageKeyboardClear:
                
                break;
            case UIKeyboardHIDUsageKeyboardPrior:
                
                break;
            case UIKeyboardHIDUsageKeyboardReturn:
                
                break;
            case UIKeyboardHIDUsageKeyboardSeparator:
                
                break;
            case UIKeyboardHIDUsageKeyboardOut:
                
                break;
            case UIKeyboardHIDUsageKeyboardOper:
                
                break;
            case UIKeyboardHIDUsageKeyboardClearOrAgain:
                
                break;
            case UIKeyboardHIDUsageKeyboardCrSelOrProps:
                
                break;
            case UIKeyboardHIDUsageKeyboardExSel:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftControl:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftShift:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftAlt:
                
                break;
            case UIKeyboardHIDUsageKeyboardLeftGUI:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightControl:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightShift:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightAlt:
                
                break;
            case UIKeyboardHIDUsageKeyboardRightGUI:
                
                break;
            case UIKeyboardHIDUsageKeyboard_Reserved:
                
                break;
            }
        }
        
    }
    

    
}

-(void) pencilInteraction:(UIPencilInteraction *)interaction didReceiveSqueeze:(UIPencilInteractionSqueeze *)squeeze {
    if (!_is_init) {
        return;
    }
    
    UI_TOUCH_STATE state = UI_TOUCH_STATE_NONE;
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    
    @autoreleasepool {
        switch (squeeze.phase) {
            case UIPencilInteractionPhaseBegan: {
                //MTT_print("%s\n", "squeeze began");
                state = UI_TOUCH_STATE_PRESSED_BUTTON_BEGAN;
                input_record->press_t_start = squeeze.timestamp;
                break;
            }
            case UIPencilInteractionPhaseChanged: {
                //MTT_print("%s\n", "squeeze changed");
                state = UI_TOUCH_STATE_PRESSED_BUTTON_CHANGED;
                break;
            }
            case UIPencilInteractionPhaseEnded: {
                //MTT_print("%s\n", "squeeze ended");
                state = UI_TOUCH_STATE_PRESSED_BUTTON_ENDED;
                input_record->press_t_end = squeeze.timestamp;
                break;
            }
            case UIPencilInteractionPhaseCancelled: {
                //MTT_print("%s\n", "squeeze cancelled");
                state = UI_TOUCH_STATE_PRESSED_BUTTON_CANCELLED;
                break;
            }
        }
        
        
        
    
        UIPencilHoverPose* pose = [squeeze hoverPose];
        uint32 flags = 0;
        vec2 pos = m::vec2_zero();
        if (pose == nil) {
            //MTT_print("%s\n", "pose is nil");
        } else {
            CGPoint pose_location = [pose location];
            pos.x = pose_location.x;
            pos.y = pose_location.y;
            //MTT_print("[%f, %f]\n", pose_location.x, pose_location.y);
            flags = 1;
        }
        switch (UIPencilInteraction.preferredTapAction) {
            default: {
                Input_push_event(&(_core_platform.core.input.users[_userID]), {
                    .state_type = state,
                    .input_type = UI_TOUCH_TYPE_POINTER,
                    .position = pos,
                    .timestamp = squeeze.timestamp,
                    .flags = flags,
                });
                break;
            }
        }
        
    }
}


-(void)deinit {
    NSLog(@"MTT VC deinit called");
    [_mtk_view setPaused:YES];
    [_renderer deinit];
    MTT_on_exit(&_core_platform);
}
#else
- (void)panGestureWasRecognized:(NSPanGestureRecognizer *)sender {
    // TODO do something else with this
    return;
    
    
    CGPoint deltaTranslation = [sender translationInView:self.view];
    //usize touch_count = sender.numberOfTouches;
    
    const usize count_flag = ~0llu;
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    //sender.cancelsTouchesInView = false;
    if (sender.state == NSGestureRecognizerStateBegan) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)deltaTranslation.x, (float32)deltaTranslation.y},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = count_flag
        });
    } else if (sender.state == NSGestureRecognizerStateChanged) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_CHANGED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)deltaTranslation.x, (float32)deltaTranslation.y},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = count_flag
        });
    } else if (sender.state == NSGestureRecognizerStateEnded
               || sender.state == NSGestureRecognizerStateCancelled) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)deltaTranslation.x, (float32)deltaTranslation.y},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = count_flag
        });
    }
    
    [sender setTranslation:CGPointZero inView:self.view];
}

- (void)magnifyWithEvent:(NSEvent *)event {
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    NSPoint gesture_position = [self mouseLocationInViewWithPoint:event.locationInWindow];
    auto scale = [event magnification] + 1.0f;
    if (event.phase == NSEventPhaseBegan) {
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PINCH_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .scale = {(float32)scale, (float32)scale},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = 2
        });
    } else if (event.phase == NSEventPhaseChanged) {
            Input_push_event(input_record, {
                .state_type = UI_TOUCH_STATE_PINCH_GESTURE_BEGAN,
                .input_type = UI_TOUCH_TYPE_DIRECT,
                .scale = {(float32)scale, (float32)scale},
                .position = {(float32)gesture_position.x, (float32)gesture_position.y},
                .count = 2
            });
    } else if (event.phase == NSEventPhaseEnded
               || event.phase == NSEventPhaseCancelled) {
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PINCH_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .scale = {(float32)scale, (float32)scale},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = 2
        });
    }
}

- (void)rotateWithEvent:(NSEvent *)event {
//    [resultsField setStringValue:
//     [NSString stringWithFormat:@"Rotation in degree is %f", [event rotation]]];
//    [self setFrameCenterRotation:([self frameCenterRotation] + [event rotation])];
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    
    NSPoint gesture_position = [self mouseLocationInViewWithPoint:event.locationInWindow];
    
    auto rotation = [event rotation] * -MTT_PI / 180.0;
    
    if (event.phase == NSEventPhaseBegan
        || event.phase == NSEventPhaseChanged) {
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_ROTATION_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .rotation = (float32)rotation,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = 2
        });
    } else if (event.phase == NSEventPhaseEnded
               || event.phase == NSEventPhaseCancelled) {
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_ROTATION_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .rotation = (float32)rotation,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = 2
        });
    }
}

- (void)swipeWithEvent:(NSEvent *)event {
    CGFloat x = [event deltaX];
    CGFloat y = [event deltaY];
    
    CGPoint deltaTranslation = CGPointMake(x, y);
    //usize touch_count = sender.numberOfTouches;
    return;
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    
    NSPoint gesture_position = [self mouseLocationInViewWithPoint:event.locationInWindow];
    
    if (event.phase == NSEventPhaseBegan) {
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)deltaTranslation.x, (float32)deltaTranslation.y},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = 1
        });
    } else if (event.phase == NSEventPhaseChanged) {

        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_CHANGED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)deltaTranslation.x, (float32)deltaTranslation.y},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = 1
        });
    } else if (event.phase == NSEventPhaseEnded
               || event.phase == NSEventPhaseCancelled) {
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_PAN_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .translation = {(float32)deltaTranslation.x, (float32)deltaTranslation.y},
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = 1
            
        });
    }
//    if (x != 0) {
//        swipeColorValue = (x > 0)  ? SwipeLeftGreen : SwipeRightBlue;
//    }
//    if (y != 0) {
//        swipeColorValue = (y > 0)  ? SwipeUpRed : SwipeDownYellow;
//    }
//    NSString *direction;
//    switch (swipeColorValue) {
//        case SwipeLeftGreen:
//            direction = @"left";
//            break;
//        case SwipeRightBlue:
//            direction = @"right";
//            break;
//        case SwipeUpRed:
//            direction = @"up";
//            break;
//        case SwipeDownYellow:
//        default:
//            direction = @"down";
//            break;
//    }
}

- (void)pressGestureWasRecognized:(NSPressGestureRecognizer *)sender {
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    if (sender.state == NSGestureRecognizerStateBegan
        /* || sender.state == UIGestureRecognizerStateChanged*/) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        gesture_position = [self mouseLocationInViewWithPoint:gesture_position];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_LONG_PRESS_GESTURE_BEGAN,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = 1
        });
        
        //MTT_print("Long press gesture was recognized!\n");
        
        
    } else if (sender.state == NSGestureRecognizerStateEnded ||
               sender.state == NSGestureRecognizerStateCancelled) {
        
        CGPoint gesture_position = [sender locationInView:_mtk_view];
        gesture_position = [self mouseLocationInViewWithPoint:gesture_position];
        
        Input_push_event(input_record, {
            .state_type = UI_TOUCH_STATE_LONG_PRESS_GESTURE_ENDED,
            .input_type = UI_TOUCH_TYPE_DIRECT,
            .position = {(float32)gesture_position.x, (float32)gesture_position.y},
            .count = 1
        });
        
        //MTT_print("Long press gesture ended!\n");
    }
}

-(void)keyDown:(NSEvent*)event {
    if (!_is_init) {
        return;
    }
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    
    @autoreleasepool {
        UI_Event key_event = {};
        key_event.input_type = UI_TOUCH_TYPE_KEY;
        key_event.state_type = (event.ARepeat == NO) ? UI_TOUCH_STATE_KEY_BEGAN : UI_TOUCH_STATE_KEY_REPEATED;
        key_event.key        = MTT_key_mapping[event.keyCode];
        key_event.key_sub    = event.modifierFlags;
        key_event.timestamp  = event.timestamp;
        
        
        set_key_is_pressed(input_record, (UI_KEY_TYPE)key_event.key);
        key_event.count = 0;
        if(![event.characters getCString:key_event.characters maxLength:sizeof(key_event.characters)/sizeof(*key_event.characters) encoding:NSUTF8StringEncoding]) {
            NSLog(@"Command buffer too small");
        } else {
            key_event.count = event.characters.length;
        }
        
        Input_push_event(input_record, key_event);
    }
}
-(void)keyUp:(NSEvent*)event {
    if (!_is_init) {
        return;
    }
    
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    
    @autoreleasepool {
        UI_Event key_event = {};
        key_event.input_type = UI_TOUCH_TYPE_KEY;
        key_event.state_type = UI_TOUCH_STATE_KEY_ENDED;
        key_event.key        = MTT_key_mapping[event.keyCode];
        key_event.key_sub    = event.modifierFlags;
        key_event.timestamp  = event.timestamp;
        unset_key_is_pressed(input_record, (UI_KEY_TYPE)key_event.key);
        key_event.count = 0;
        if(![event.characters getCString:key_event.characters maxLength:sizeof(key_event.characters)/sizeof(*key_event.characters) encoding:NSUTF8StringEncoding]) {
            NSLog(@"Command buffer too small");
        } else {
            key_event.count = event.characters.length;
        }
        
        Input_push_event(input_record, key_event);
    }
}

- (void) flagsChanged:(NSEvent *) event {
    Input* const input = &_core_platform.core.input;
    Input_Record* input_record = &input->users[_userID];
    input_record->modifier_flags = ui_key_modifier_flags([event modifierFlags]);
}

-(void)deinit {
    NSLog(@"MTT VC deinit called");
    [_mtk_view setPaused:YES];
    //[_renderer deinit];
    MTT_on_exit(&_core_platform);
}

-(void)exitWebBrowser:(id)sender {
}
#endif

//- (NSString*) get_debug_visualization {
////    if (self->_core_platform.core.get_debug_visualization) {
////        char* const output = self->_core_platform.core.get_debug_visualization(&self->_core_platform.core);
////        
////        NSString* out_native = [NSString stringWithUTF8String:output];
////        
////        self->_core_platform.core.allocator.deallocate((void*) &self->_core_platform.core.allocator, output, strlen(output) + 1);
////        
////        return out_native;
////    }
//    
//    return @"";
//}



//- (void) session:(ARSession *)session didUpdateAnchors:(NSArray<__kindof ARAnchor *> *)anchors {
//    ARAnchor* anchor = anchors[0];
//    if ([anchor isKindOfClass:[ARFaceAnchor class]]) {
//        NSLog(@"WEE");
//
//        ARFaceAnchor* face_anchor = (ARFaceAnchor*)anchor;
//
//
//    }
//}

- (void) did_enter_background
{
    [self->_renderer pause];
}
- (void) will_enter_foreground
{
    [self->_renderer resume];
}

#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP

static bool find_host_window_bounds(CGRect* _Nonnull screenspace_bounds, NSString* identifier)
{
    @autoreleasepool {
        
        NSArray<NSRunningApplication *>* applications = [NSRunningApplication  runningApplicationsWithBundleIdentifier:@"com.apple.QuickTimePlayerX"];
        if (applications.count == 0) {
            return false;
        }
        
        NSRunningApplication* application = applications[0];
        
        CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
        NSArray* window_arr = CFBridgingRelease(window_list);
        for (NSMutableDictionary* entry in window_arr) {
            // Get window PID
            pid_t pid = [[entry objectForKey:(id)kCGWindowOwnerPID] intValue];
            if (pid == application.processIdentifier) {
                CGRect bounds = {};
                CGWindowID window_id = {};
                NSNumber *windowNumber = [entry objectForKey:(id)kCGWindowNumber];
                
                if (CGRectMakeWithDictionaryRepresentation((CFDictionaryRef)[entry objectForKey:(id)kCGWindowBounds], &bounds)) {
                    
                    *screenspace_bounds = bounds;
                    
                    {
//                        AXUIElementRef appRef = AXUIElementCreateApplication(pid);
//                        NSLog(@"Ref = %@",appRef);
//                        CFArrayRef names;
//                        if (AXUIElementCopyAttributeNames(appRef, &names) == kAXErrorSuccess) {
//                            usize count = CFArrayGetCount(names);
//                            for (usize i = 0; i < count; i += 1) {
//                                NSLog(@"%@", CFArrayGetValueAtIndex(names, i));
//                            }
//                            int BP = 0;
//                        } else {
//                            int BP = 0;
//                        }
//
//                        // Get the windows
//                        CFArrayRef windowList;
//                        AXUIElementCopyAttributeValue(appRef, kAXWindowsAttribute, (CFTypeRef *)&windowList);
//                        NSLog(@"WindowList = %@", windowList);
//                        if ((!windowList) || CFArrayGetCount(windowList)<1)
//                            continue;
//
//
//                        // get just the first window for now
//                        AXUIElementRef windowRef = (AXUIElementRef) CFArrayGetValueAtIndex( windowList, 0);
//                        CFTypeRef role;
//                        AXUIElementCopyAttributeValue(windowRef, kAXRoleAttribute, (CFTypeRef *)&role);
//                        AXValueRef position;
//                        CGPoint point;
//
//                        // Get the position attribute of the window (maybe something is wrong?)
//                        AXUIElementCopyAttributeValue(windowRef, kAXPositionAttribute, (CFTypeRef *)&position);
//                        AXValueGetValue(position, kAXValueTypeCGPoint, &point);
//                        // Debugging (always zeros?)
//                        NSLog(@"point=%f,%f", point.x,point.y);
//                        // Create a point
//                        CGPoint newPoint;
//                        newPoint.x = 0;
//                        newPoint.y = 0;
//                        NSLog(@"Create");
//                        position = (AXValueCreate(kAXValueTypeCGPoint, (void *)&newPoint));
//                        // Set the position attribute of the window (runtime error over here)
//                        NSLog(@"SetAttribute");
//                        AXUIElementSetAttributeValue(windowRef, kAXPositionAttribute, position);
                    }
                    return true;
                }
            }

        }
    }
    
    return false;
}

void start_follow_system_window(cstring identifier_cstr);

void start_follow_system_window(cstring identifier_cstr) {
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    @autoreleasepool {
    CGRect bounds = {};
        
    auto* identifier =  [NSString stringWithUTF8String:identifier_cstr];
    
    if (find_host_window_bounds(&bounds, identifier)) {
        //NSLog(@"%@", NSStringFromRect(bounds));
        NSWindow* first_window = [[NSApplication sharedApplication] mainWindow];
        //NSLog(@"Window frame: %@", NSStringFromRect(first_window.frame));
        //auto frame = first_window.frame;
        
        auto screen = get_main_screen();
        auto H = screen.frame.size.height;
        
        
        
        [first_window setBackgroundColor:[NSColor clearColor]];
        
        
//
//
        
        
        {
            CGSize resized = CGSizeMake(bounds.size.width, bounds.size.height);
            auto new_frame = first_window.frame;
            new_frame.size.width  = resized.width;
            new_frame.size.height = resized.height;
            [first_window setFrame:new_frame display:YES animate:NO];
            [first_window setFrameTopLeftPoint:CGPointMake(bounds.origin.x, H - bounds.origin.y)];
            
            [first_window orderFrontRegardless];
        
            if constexpr ((false)) {
                float32 native_scale = get_native_scale(screen);
                resized.width = resized.width * native_scale;
                resized.height = resized.height * native_scale;
                auto* renderer_backend = (__bridge SD_Renderer_Metal_Backend*)(mtt_core_ctx()->renderer->backend);
                [renderer_backend drawRectResized:resized];
            }
        }
        
        //[renderer_backend drawRectResized:resized];
        
        sd::set_see_through_background(mtt_core_ctx()->renderer, true);
        if (  ((MTT_Core_Platform*)mtt_core_ctx()->core_platform)->vc->window_move_event_observer_for_external_app_following != nil) {
            
            [[NSNotificationCenter defaultCenter] removeObserver:((MTT_Core_Platform*)mtt_core_ctx()->core_platform)->vc->window_move_event_observer_for_external_app_following];

        }
        if (  ((MTT_Core_Platform*)mtt_core_ctx()->core_platform)->vc->window_resize_event_observer_for_external_app_following != nil) {
            
            [[NSNotificationCenter defaultCenter] removeObserver:((MTT_Core_Platform*)mtt_core_ctx()->core_platform)->vc->window_resize_event_observer_for_external_app_following];

        }
        
        
        __block NSString* identifier_ = identifier;
        auto response_block = ^(NSNotification* notification) {
            
            NSWindow* window = notification.object;
            
            CGRect bounds = {};
            if (!find_host_window_bounds(&bounds, identifier_)) {
                sd::set_see_through_background(mtt_core_ctx()->renderer, false);
                [window setBackgroundColor:[NSColor whiteColor]];
                return;
            }
            sd::set_see_through_background(mtt_core_ctx()->renderer, true);
            [window setBackgroundColor:[NSColor clearColor]];
            
            auto screen = get_main_screen();
            auto H = screen.frame.size.height;
            
            
//            [window setFrameTopLeftPoint:CGPointMake(bounds.origin.x, H - bounds.origin.y)];
            
            {
                CGSize resized = CGSizeMake(bounds.size.width, bounds.size.height);
                auto new_frame = first_window.frame;
                new_frame.size.width  = resized.width;
                new_frame.size.height = resized.height;
                [first_window setFrame:new_frame display:YES animate:NO];
                [first_window setFrameTopLeftPoint:CGPointMake(bounds.origin.x, H - bounds.origin.y)];
                
                [first_window orderFrontRegardless];
                
                if constexpr ((false)) {
                    float32 native_scale = get_native_scale(screen);
                    resized.width = resized.width * native_scale;
                    resized.height = resized.height * native_scale;
                    auto* renderer_backend = (__bridge SD_Renderer_Metal_Backend*)(mtt_core_ctx()->renderer->backend);
                    [renderer_backend drawRectResized:resized];
                }
            }
            
        };
        ((MTT_Core_Platform*)mtt_core_ctx()->core_platform)->vc->window_move_event_observer_for_external_app_following = [[NSNotificationCenter defaultCenter] addObserverForName: NSWindowDidMoveNotification object:first_window queue:[NSOperationQueue currentQueue] usingBlock:response_block];
        
//        ((MTT_Core_Platform*)mtt_core_ctx()->core_platform)->vc->window_resize_event_observer_for_external_app_following = [[NSNotificationCenter defaultCenter] addObserverForName: NSWindowDidResizeNotification object:first_window queue:[NSOperationQueue currentQueue] usingBlock:response_block];
        
    }
    }
#endif
}

void stop_follow_system_window()
{
    
    if (  ((MTT_Core_Platform*)mtt_core_ctx()->core_platform)->vc->window_move_event_observer_for_external_app_following != nil) {
        @autoreleasepool {
        [[NSNotificationCenter defaultCenter] removeObserver:((MTT_Core_Platform*)mtt_core_ctx()->core_platform)->vc->window_move_event_observer_for_external_app_following];
            
            auto* window = [[NSApplication sharedApplication] mainWindow];
            [window setBackgroundColor:[NSColor whiteColor]];
            window.titleVisibility = NSWindowTitleVisible;
            window.titlebarAppearsTransparent = false;
        }
    }
}



#endif

@end

void MTT_follow_system_window(bool state)
{
#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
if (state) {
    start_follow_system_window("com.apple.QuickTimePlayerX");
} else {
    stop_follow_system_window();
}
#endif
    
}

void MTT_reset_defaults_platform(void)
{
    [[NSUserDefaults standardUserDefaults] removePersistentDomainForName:[[NSBundle mainBundle] bundleIdentifier]];
}
