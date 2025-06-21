//
//  video_recording.m
//  Make The Thing macos
//
//  Created by Toby Rosenberg on 7/28/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//




#include "video_recording_platform.hpp"
#include "stratadraw_platform_apple.hpp"
#import "sd_metal_renderer.h"



#import "Make_The_Thing-Swift.h"

CVMetalTextureRef create_texture_with_cache_and_pixel_buffer(CVMetalTextureCacheRef cache, CVPixelBufferRef pixel_buffer,
                                 MTLPixelFormat pixel_format)
{
    auto width = CVPixelBufferGetWidth(pixel_buffer);
    auto height = CVPixelBufferGetHeight(pixel_buffer);
    
    CVMetalTextureRef texture;
    auto status = CVMetalTextureCacheCreateTextureFromImage(nil,
                                                           cache,
                                                           pixel_buffer,
                                                           nil,
                                                           pixel_format,
                                                           width,
                                                           height,
                                                           0,
                                                            &texture);
    
    if (status != kCVReturnSuccess) MTT_UNLIKELY {
        CVBufferRelease(pixel_buffer);
        texture = nil;
    }
    
    return texture;
}

@interface MTT_Video_Recording_Context()
@property (nonatomic, strong) AVCaptureDevice* capture_device;
@property (nonatomic, strong) AVCaptureSession* capture_session;
@property (nonatomic, strong) AVCaptureDeviceInput* device_input;
@property (nonatomic, strong) AVCaptureVideoDataOutput* data_output;
@property (nonatomic, strong) id<MTLDevice> gpu_device;

@end

@implementation MTT_Video_Recording_Context {
    @public dispatch_queue_t queue;
    @public CVMetalTextureCacheRef cache;
    @public id<NSObject> device_connected_observer;
    @public id<NSObject> device_disconnected_observer;
    @public BOOL is_running;
}

-(nonnull instancetype)init;
{
    if ((self = [super init])) {
        dispatch_queue_attr_t queue_attribute = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, 0);
        self->queue = dispatch_queue_create("video_camera", queue_attribute);
        
        self->device_connected_observer = [[NSNotificationCenter defaultCenter] addObserverForName:AVCaptureDeviceWasConnectedNotification
                                                                         object:nil
                                                                          queue:[NSOperationQueue mainQueue]
                                                                     usingBlock:^(NSNotification *notification) {
            AVCaptureDevice* device = notification.object;
                            
            // TODO:
            NSLog(@"Device connected: name=%@, id=%@", device.localizedName, device.uniqueID);
        }];
        self->device_disconnected_observer = nil;
        self->_capture_session = [[AVCaptureSession alloc] init];
        self->is_running = NO;
    }
    
    return self;
}
// TODO: https://stackoverflow.com/a/37652756/7361580
- (void)start
{
    if (!mtt::Video_Recording_is_init() || self->is_running) {
        return;
    }
    
    @autoreleasepool {
        if (!self->_gpu_device) {
            auto* mtl_renderer = SD_Renderer_backend((sd::Renderer*)self->info.renderer);
            self->_gpu_device = mtl_renderer->_device;
            
            self->cache = nil;
            CVMetalTextureCacheCreate(nil, nil, self->_gpu_device, nil, &self->cache);
            if (self->cache == nil) {
                NSLog(@"Error: could not create texture cache");
                return;
            }
        }
        
            // Start session configuration
        [self->_capture_session beginConfiguration];
        
        NSError* error = nil;
//        [self->_capture_device lockForConfiguration:&error];
//        if (error != nil) {
//            NSLog(@"Error %@\n", [error localizedDescription]);
//            return;
//        }
//
//        AVCaptureDeviceFormat* bestFormat = nil;
//        AVFrameRateRange* bestFrameRateRange = nil;
//
//        for (AVCaptureDeviceFormat* format in self->_capture_device.formats) {
//            for (AVFrameRateRange* range in format.videoSupportedFrameRateRanges) {
//                if (range.maxFrameRate > ((bestFrameRateRange) ? bestFrameRateRange.maxFrameRate : 0)) {
//                    bestFormat = format;
//                    bestFrameRateRange = range;
//                }
//            }
//        }
//        if (bestFormat != nil) {
//            if (bestFrameRateRange != nil) {
//                if (error != nil) {
//                    NSLog(@"Error %@\n", [error localizedDescription]);
//                    return;
//                }
//
//                [self->_capture_device setActiveVideoMinFrameDuration: bestFrameRateRange.minFrameDuration];
//                [self->_capture_device setActiveVideoMaxFrameDuration: bestFrameRateRange.minFrameDuration];
//            }
//        }
//
//        [self->_capture_device unlockForConfiguration];
        
        error = nil;
        
        self->_device_input = [[AVCaptureDeviceInput alloc] initWithDevice:self->_capture_device error:&error];
        if (error != nil) {
            NSLog(@"Error %@\n", [error localizedDescription]);
            return;
        }
        
        self->_capture_session.sessionPreset = AVCaptureSessionPresetHigh;
        
        [self->_capture_session addInput:self->_device_input];
        
        self->_data_output = [[AVCaptureVideoDataOutput alloc] init];
        self->_data_output.alwaysDiscardsLateVideoFrames = YES;
        

        
        [self->_data_output setSampleBufferDelegate:self queue:self->queue];
        [self->_data_output setVideoSettings:@{
            (id)kCVPixelBufferPixelFormatTypeKey : [NSNumber numberWithInt: kCVPixelFormatType_32BGRA]
        }];
        
        [self->_capture_session addOutput:self->_data_output];
        
        [self->_capture_session commitConfiguration];
            
        self->is_running = YES;
        dispatch_async(self->queue, ^{
            [self->_capture_session startRunning];
        });
    }
}

-(void)stop {
    if (!mtt::Video_Recording_is_init() || !self->is_running) {
        return;
    }
    self->is_running = NO;
    
    dispatch_async(self->queue, ^{
        [self->_capture_session stopRunning];
        [self->_capture_session beginConfiguration];
        for (AVCaptureDeviceInput* input in self->_capture_session.inputs) {
            [self->_capture_session removeInput:input];
        }
        for (AVCaptureOutput* output in self->_capture_session.outputs) {
            [self->_capture_session removeOutput:output];
        }
        [self->_capture_session commitConfiguration];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            self->_capture_device = nil;
        });
    });
}

- (void)captureOutput:(AVCaptureOutput *)output didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
#ifndef NDEBUG
    NSLog(@"Dropped frame!");
#endif
}

- (void)captureOutput:(AVCaptureOutput *)output didOutputSampleBuffer:(CMSampleBufferRef)sample_buffer fromConnection:(AVCaptureConnection *)connection {
    
    @autoreleasepool {
        CVImageBufferRef image_buffer = CMSampleBufferGetImageBuffer(sample_buffer);
        if (image_buffer == nil) MTT_UNLIKELY {
            return;
        }
    
        CVMetalTextureRef cv_texture = create_texture_with_cache_and_pixel_buffer(self->cache, image_buffer, MTLPixelFormatBGRA8Unorm);
        if (cv_texture == nil) MTT_UNLIKELY {
            return;
        }
        
        __block id<MTLTexture> mtl_texture = CVMetalTextureGetTexture(cv_texture);
        CVBufferRelease(cv_texture);
        dispatch_async(dispatch_get_main_queue(), ^{
            sd::Image_replace_associated_texture((sd::Images*)self->info.images,
                                                 (sd::Image*)self->info.image,
                                                 mtl_texture);
        });
        
        sample_buffer = nil;
        image_buffer = nil;
    }
}

-(void)destroy {
    self->_capture_session = nil;
}

@end




namespace mtt {
    
    
    bool is_init_ = false;

    bool Video_Recording_is_init(void)
    {
        return is_init_;
    }

    bool Video_Recording_has_device(MTT_Video_Recording_Context_Ref ctx)
    {
        MTT_Video_Recording_Context* v_ctx = (__bridge MTT_Video_Recording_Context*)ctx;
        
        return v_ctx.capture_device != nil;
        
    }

    void Video_Recording_init(void)
    {
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_IOS
        // UNUSUED
        return;
#else
        @autoreleasepool {
            auto activate_external_connection = []() {
                dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_IOS
                    CMIOObjectPropertyAddress prop = {
                        .mSelector = CMIOObjectPropertySelector(kCMIOHardwarePropertyAllowScreenCaptureDevices),
                        .mScope = CMIOObjectPropertyScope(kCMIOObjectPropertyScopeGlobal),
                        .mElement = CMIOObjectPropertyElement(kCMIOObjectPropertyElementMain)
                    };
                    
                    UInt32 allow = 1;

                    CMIOObjectSetPropertyData(CMIOObjectID(kCMIOObjectSystemObject),
                                              &prop,
                                              0,
                                              NULL,
                                              sizeof(allow),
                                              &allow);
                    dispatch_async(dispatch_get_main_queue(), ^{
                        is_init_ = true;
                    });
#else
                    is_init_ = true;
#endif
                    
                    
                });
            };
            
            if ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo] != AVAuthorizationStatusAuthorized) {
                [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
                    if (granted) {
                        activate_external_connection();
                    } else {
                        is_init_ = false;
                    }
                }];
            } else {
                activate_external_connection();
            }
        }
#endif
    }

    void Video_Recording_discover_devices(MTT_Video_Recording_Context_Ref ctx)
    {
        __block MTT_Video_Recording_Context* v_ctx = (__bridge MTT_Video_Recording_Context*)ctx;
        @autoreleasepool {
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_IOS
            AVCaptureDeviceDiscoverySession* discovery_session = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeExternal]
                                                                   mediaType:AVMediaTypeMuxed
                                                                    position:AVCaptureDevicePositionUnspecified];
#else
            AVCaptureDeviceDiscoverySession* discovery_session = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeBuiltInDualCamera, AVCaptureDeviceTypeBuiltInTripleCamera, AVCaptureDeviceTypeBuiltInUltraWideCamera, AVCaptureDeviceTypeBuiltInTelephotoCamera, AVCaptureDeviceTypeBuiltInTrueDepthCamera]
                                                                   mediaType:AVMediaTypeVideo
                                                                    position:AVCaptureDevicePositionUnspecified];
#endif
            v_ctx.capture_device = nil;
            NSArray<AVCaptureDevice*>* devices = discovery_session.devices;
            for (AVCaptureDevice* device in devices) {
                v_ctx.capture_device = device;
                v_ctx->device_disconnected_observer = [[NSNotificationCenter defaultCenter] addObserverForName:AVCaptureDeviceWasDisconnectedNotification
                                                                                    object:nil
                                                                                     queue:[NSOperationQueue mainQueue]
                                                                                usingBlock:^(NSNotification *notification) {
                    AVCaptureDevice* device = notification.object;
                                       
                    // TODO:
                    NSLog(@"Device disconnected: name=%@, id=%@", device.localizedName, device.uniqueID);
                    
                    [v_ctx stop];
                }];
                break;
            }
            
            discovery_session = nil;
        }
        

    }

MTT_Video_Recording_Context_Ref Video_Recording_Context_make(MTT_Video_Recording_Context_Args* args)
{
    MTT_Video_Recording_Context* ctx = [[MTT_Video_Recording_Context alloc] init];
    
    const void* ID = CFBridgingRetain(ctx);
    ctx->info.image     = args->image;
    ctx->info.images    = args->images;
    ctx->info.user_data = args->user_data;
    ctx->info.renderer = args->renderer;
    ctx->info.on_data = args->on_data;
    
    return ID;
}

void Video_Recording_Context_destroy(MTT_Video_Recording_Context_Ref ctx)
{
    
    @autoreleasepool {
        
        MTT_Video_Recording_Context* plat_ctx = (__bridge MTT_Video_Recording_Context*)ctx;
        [plat_ctx destroy];
        
        CFBridgingRelease(ctx);
        ctx = nil;
    }
}

sd::Image* Video_Recording_get_image(MTT_Video_Recording_Context_Ref ctx)
{
    MTT_Video_Recording_Context* plat_ctx = (__bridge MTT_Video_Recording_Context*)ctx;
    
    return (sd::Image*)plat_ctx->info.image;
}

void Video_Recording_start(MTT_Video_Recording_Context_Ref ctx)
{
    MTT_Video_Recording_Context* plat_ctx = (__bridge MTT_Video_Recording_Context*)ctx;
    [plat_ctx start];
}

void Video_Recording_stop(MTT_Video_Recording_Context_Ref ctx)
{
    MTT_Video_Recording_Context* plat_ctx = (__bridge MTT_Video_Recording_Context*)ctx;
    [plat_ctx stop];
}


}
