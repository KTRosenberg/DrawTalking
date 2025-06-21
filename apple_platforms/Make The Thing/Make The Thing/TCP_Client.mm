//
//  TCP_Client.mm
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/28/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TCP_Client.h"
#import "extension_server.h"
#import "Make_The_Thing-Swift.h"
#include "mtt_core_platform.h"

#include "misc_platform.hpp"

struct Message_Subscriber_Info {
    void (*handler)(void*, void*);
    void* ctx;
};

//enum MSG_RECV_TYPE {
//    MSG_RECV_TYPE_UNKNOWN = 0,
//    MSG_RECV_TYPE_NLF = 1,
//    MSG_RECV_TYPE_NLH = 2,
//    MSG_RECV_TYPE_SYN = 3,
//    MSG_RECV_TYPE_LDF = 4,
//    MSG_RECV_TYPE_DYL = 5,
//};
//
//struct Compare_NSString {
//    bool operator()(NSString* lhs, NSString* rhs) const {
//        if (rhs != nil)
//            return (lhs == nil) || ([lhs compare: rhs] == NSOrderedAscending);
//        else
//            return false;
//    }
//    
//    bool operator()(NSString* lhs, NSString* rhs) {
//        if (rhs != nil)
//            return (lhs == nil) || ([lhs compare: rhs] == NSOrderedAscending);
//        else
//            return false;
//    }
//};
//
//static const robin_hood::unordered_flat_map<NSString*, MSG_RECV_TYPE, robin_hood::hash<NSString*>, Compare_NSString> msg_map = {
//    {@"nlf:", MSG_RECV_TYPE_NLF},
//    {@"nlh:", MSG_RECV_TYPE_NLH},
//    {@"syn:", MSG_RECV_TYPE_SYN},
//    {@"ldf:", MSG_RECV_TYPE_LDF},
//    {@"dyl:", MSG_RECV_TYPE_DYL}
//};
//
//MSG_RECV_TYPE msg_match(NSString* msg)
//{
//    auto find = msg_map.find(msg);
//    if (find == msg_map.end()) {
//        return MSG_RECV_TYPE_UNKNOWN;
//    } else {
//        return find->second;
//    }
//}

static inline MTT_Logger tcp_client_logger = {.logger = os_log_create("mtt.main", "net"), .buf = new mtt::String()};

@interface TCP_Client()
{
    NSTimer*  timer;
    BOOL first_launch;
    robin_hood::unordered_set<Data_Handler*> subscribers;
    robin_hood::unordered_map<mtt::String, std::vector<Message_Subscriber_Info>> message_subscribers;
    usize msg_id;
    uint64 self_id;
}
@end

@implementation TCP_Client

- (void)load:(NSString* _Nonnull) message {
    NSData* data_send = [[@"LOAD" stringByAppendingString: message] dataUsingEncoding:NSUTF8StringEncoding];
    [self->async_socket writeData:data_send withTimeout:-1 tag:0];
}

- (void)query:(NSString* _Nonnull) message {
    
    NSData* data_send = [message dataUsingEncoding:NSUTF8StringEncoding];
    [self->async_socket writeData:data_send withTimeout:-1 tag:msg_id];
    msg_id += 1;
}

- (void)sendMessage:(NSString* _Nonnull) message {
    NSData* data_send = [message dataUsingEncoding:NSUTF8StringEncoding];
    [self->async_socket writeData:data_send withTimeout:-1 tag:msg_id];
    msg_id += 1;
}

- (void)socket:(GCDAsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port {
    
    MTT_log_debugl(&tcp_client_logger, "socket:didConnectToHost:%{public}@ port:%hu", host, port);
    
#if MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP
    [sock performBlock:^{
        if ([sock enableBackgroundingOnSocket]) {
            MTT_log_debugl(&tcp_client_logger, "%{public}s\n", "Enabled backgrounding on socket");
        } else {
            MTT_log_debugl(&tcp_client_logger, "%{public}s\n", "Enabling backgrounding failed!");
        }
    }];
#endif
    
    self->host = host;
    self->port = port;
    self->is_disconnected = false;
    
    if (!self->first_launch) {
        for (auto i = subscribers.begin(); i != subscribers.end(); ++i) {
            Data_Handler* handler = (*i);

            handler->on_reset((void*)handler);
        }
    } else {
        self->first_launch = NO;
    }
    
//    NSData* data = [@"Hello from the client!\n" dataUsingEncoding:NSUTF8StringEncoding];
    [sock readDataToData:[GCDAsyncSocket CRLFData] withTimeout:-1 tag:1];
}

- (void)socket:(GCDAsyncSocket *)sock didReceiveTrust:(SecTrustRef)trust
    completionHandler:(void (^)(BOOL shouldTrustPeer))completionHandler {
    
    MTT_log_debugl(&tcp_client_logger, "%{public}s\n", "socket:shouldTrustPeer:");
}

- (void)socketDidSecure:(GCDAsyncSocket *)sock {
    MTT_log_debugl(&tcp_client_logger, "%{public}s\n", "socketDidSecure:");
}

- (void)socket:(GCDAsyncSocket *)sock didWriteDataWithTag:(long)tag {
    //NSLog(@"socket:didWriteDataWithTag:");
    [sock readDataToData:[GCDAsyncSocket CRLFData] withTimeout:-1 tag:1];
}

- (void)socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag {
    @autoreleasepool {
        NSString* cmd = [[NSString alloc] initWithBytes:data.bytes length:4 encoding:NSUTF8StringEncoding];
        
        //NSLog(@"socket:didReadData:withTag: command: %@\n", cmd);
        NSData* data_send = [[@"PING" stringByAppendingFormat:@"%llu", self->self_id] dataUsingEncoding:NSUTF8StringEncoding];
        [sock writeData:data_send withTimeout:-1 tag:0];
        
        
        if ([cmd isEqualToString:@"nlf:"]) {
            //MTT_print("%s\n", "received nlp command");
            
            NSString* from_data = [[NSString alloc] initWithBytes:data.bytes length:data.length encoding:NSUTF8StringEncoding];
            
            
            NSRange ID_range = [from_data rangeOfString:@"&"];
            //            if (ID_range.location == NSNotFound) {
            //                NSLog(@"string was not found\n");
            //            } else {
            //                NSLog(@"position %lu length %lu\n", (unsigned long)ID_range.location, (unsigned long)ID_range.length);
            //            }
            
            NSRange modification_count_range = [from_data rangeOfString:@"|"];
            //            if (modification_count_range.location == NSNotFound) {
            //                NSLog(@"string was not found\n");
            //            } else {
            //                NSLog(@"position %lu length %lu\n", (unsigned long)modification_count_range.location, (unsigned long)modification_count_range.length);
            //            }
            
            usize ID                 = [[from_data substringWithRange:NSMakeRange(4, ID_range.location - 1)] intValue];
            usize modification_count = [[from_data substringWithRange:NSMakeRange(ID_range.location + 1, modification_count_range.location - ID_range.location - 1)] intValue];
            NSString* msg = [from_data substringWithRange:NSMakeRange(modification_count_range.location + 1, from_data.length - modification_count_range.location - 1)];
            
            NSData* final_data = [msg dataUsingEncoding:NSUTF8StringEncoding];
            
            
            //            NSLog(@"{\n\t[%@][%@] [%llu] [%llu]\n}\n", from_data, msg, ID, modification_count);
            //            return;
            
            for (auto i = subscribers.begin(); i != subscribers.end(); ++i) {
                Data_Handler* handler = (*i);
                
                handler->handler((void*)handler, (__bridge void*)final_data, final_data.length, mtt_time_seconds(), ID, modification_count, 1, {});
            }
        }
        else if ([cmd isEqualToString:@"nlh:"]) {
            //MTT_print("%s\n", "received nlp command");
            
            NSString* from_data = [[NSString alloc] initWithBytes:data.bytes length:data.length encoding:NSUTF8StringEncoding];
            
            
            NSRange ID_range = [from_data rangeOfString:@"&"];
            //            if (ID_range.location == NSNotFound) {
            //                NSLog(@"string was not found\n");
            //            } else {
            //                NSLog(@"position %lu length %lu\n", (unsigned long)ID_range.location, (unsigned long)ID_range.length);
            //            }
            
            NSRange modification_count_range = [from_data rangeOfString:@"|"];
            //            if (modification_count_range.location == NSNotFound) {
            //                NSLog(@"string was not found\n");
            //            } else {
            //                NSLog(@"position %lu length %lu\n", (unsigned long)modification_count_range.location, (unsigned long)modification_count_range.length);
            //            }
            
            usize ID                 = [[from_data substringWithRange:NSMakeRange(4, ID_range.location - 1)] intValue];
            usize modification_count = [[from_data substringWithRange:NSMakeRange(ID_range.location + 1, modification_count_range.location - ID_range.location - 1)] intValue];
            NSString* msg = [from_data substringWithRange:NSMakeRange(modification_count_range.location + 1, from_data.length - modification_count_range.location - 1)];
            
            NSData* final_data = [msg dataUsingEncoding:NSUTF8StringEncoding];
            
            
            //            NSLog(@"{\n\t[%@][%@] [%llu] [%llu]\n}\n", from_data, msg, ID, modification_count);
            //            return;
            
            
            for (auto i = subscribers.begin(); i != subscribers.end(); ++i) {
                Data_Handler* handler = (*i);
                
                handler->handler((void*)handler, (__bridge void*)final_data, final_data.length, mtt_time_seconds(), ID, modification_count, 0, {});
            }
        }
        else if ([cmd isEqualToString:@"syn:"]) {
            NSString* from_data = [[NSString alloc] initWithBytes:data.bytes length:data.length encoding:NSUTF8StringEncoding];
            
            
            NSRange ID_range = [from_data rangeOfString:@"&"];
            if (ID_range.location == NSNotFound) {
                MTT_log_debugl(&tcp_client_logger, "%{public}s\n", "string was not found\n");
            } else {
                MTT_log_debugl(&tcp_client_logger, "position %lu length %lu\n", (unsigned long)ID_range.location, (unsigned long)ID_range.length);
            }
            
            NSRange modification_count_range = [from_data rangeOfString:@"|"];
            //            if (modification_count_range.location == NSNotFound) {
            //                NSLog(@"string was not found\n");
            //            } else {
            //                NSLog(@"position %lu length %lu\n", (unsigned long)modification_count_range.location, (unsigned long)modification_count_range.length);
            //            }
            
            usize ID                 = [[from_data substringWithRange:NSMakeRange(4, ID_range.location - 1)] intValue];
            //        usize
            [[from_data substringWithRange:NSMakeRange(ID_range.location + 1, modification_count_range.location - ID_range.location - 1)] intValue];
            NSString* msg = [from_data substringWithRange:NSMakeRange(modification_count_range.location + 1, from_data.length - modification_count_range.location - 1)];
            
            if (self->on_query_result != nil) {
                self->on_query_result(self, (__bridge void*)msg, ID);
            }
        }
        else if ([cmd isEqualToString:@"ldf:"]) {
            NSString* from_data = [[NSString alloc] initWithBytes:data.bytes length:data.length encoding:NSUTF8StringEncoding];
            
            NSRange path_range = [from_data rangeOfString:@"|"];
            NSString* path = [from_data substringWithRange:NSMakeRange(4, path_range.location - 4)];
            NSString* msg = [from_data substringFromIndex:path_range.location + 1];
            //
            //        NSString* msg = [from_data substringWithRange:NSMakeRange(modification_count_range.location + 1, from_data.length - modification_count_range.location - 1)];
            
            //        NSString* msg = [from_data substringWithRange:NSMakeRange(path_range.location + 1, [from_data length] - path_range.location - 3)];
            
            // NSLog(@"Received path=%@ msg=%@", path, msg);
            
            if (on_load != nil) {
                self->on_load(self, (__bridge void*)path, (__bridge void*)msg);
            }
            
        }
        else if ([cmd isEqualToString:@"dyl:"]) {
            NSString* from_data = [[NSString alloc] initWithBytes:data.bytes length:data.length encoding:NSUTF8StringEncoding];
            NSString* msg = [from_data substringFromIndex:4];
            mtt::String msg_str = mtt::String([msg cStringUsingEncoding:NSUTF8StringEncoding]);
            msg_str = msg_str.substr(0, msg_str.size() - 2);
            auto& subs = message_subscribers["dyl"];
            for (auto i = subs.begin(); i != subs.end(); ++i) {
                Message_Subscriber_Info* info = &(*i);
                
                (info->handler)(info->ctx, (void*)&msg_str);
            }
        }
        else if ([cmd isEqualToString:@"EXFR"]) {
            NSString* from_data = [[NSString alloc] initWithBytes:data.bytes length:data.length encoding:NSUTF8StringEncoding];
            //NSLog(@"EXFR: %@\n", from_data);
            NSString* msg = [from_data substringFromIndex:4];
            mtt::String msg_str = mtt::String([msg cStringUsingEncoding:NSUTF8StringEncoding]);
            
            self->on_message((void*)&msg_str, msg_str.size(), (void*)self->on_message_ctx);
        }
    }
}

- (void)socket:(GCDAsyncSocket *)sock didReadPartialDataOfLength:(NSUInteger)partialLength tag:(long)tag {
//    NSLog(@"READ\n");
////    [async_socket readDataToLength:partialLength withTimeout:-1 tag:0];
//    NSData* data = [@"Hello from the client!\n" dataUsingEncoding:NSUTF8StringEncoding];
//    [sock writeData:data withTimeout:-1 tag:0];
    
    //NSLog(@"Read partial data...\n");

}

-(void)reconnect:(NSTimer *)timer {
    NSError *error = nil;
    
    //NSLog(@"Trying to connect to host=[%@]\n", self->host);
    [async_socket connectToHost:self->host onPort:self->port error:&error];
    if (error != nil) {
        if ([self->timer isValid]) {
            [self->timer invalidate];
        }
        self->timer = nil;
        
        [self socketDidDisconnect:self->async_socket withError:error];
    } else {
        
        if ([self->timer isValid]) {
            [self->timer invalidate];
        }
        self->timer = nil;
    }
}

- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *_Nullable)err {
    if (err != nil) {
        MTT_log_debugl(&tcp_client_logger, "socketDidDisconnect:withError: \"%{public}@\"", err);
    }
    
    self->is_disconnected = true;
    
    if (!self->timer) {
        self->timer = [NSTimer scheduledTimerWithTimeInterval:4.0f
                                                       target:self
                                                     selector:@selector(reconnect:)
                                                     userInfo:nil
                                                      repeats:YES];
    }
}

- (void)write:(NSData*)data withTag:(long)tag {
    //NSLog(@"writing!!!!!!!!!!!\n");
    [async_socket writeData:data withTimeout:-1 tag:tag];
}

- (void)subscribe:(Data_Handler*)handler {
    subscribers.insert(handler);
    MTT_print("%s\n", "Subscribed to net client");
}
- (void)subscribe_message:(const mtt::String&) message_name withHandler:(void (*)(void*, void*))handler andContext:(void*)ctx {
    message_subscribers[message_name].push_back({handler, ctx});
}

- (void)set_message_handler:(void(*)(void*, usize, void*)) handler withContext:(void*)ctx {
    self->on_message = handler;
    self->on_message_ctx = ctx;
}

-(nonnull instancetype)initWithHost:(NSString*)host port:(UInt16)port selfID:(uint64)self_id {
    self = [super init];
    if (self) {
        MTT_print("%s\n", "initializing net client");
        
        self->host = host;
        self->port = port;
        self->first_launch = YES;
        self->is_disconnected = true;
        self->on_load = nil;
        self->user_data = NULL;
        self->msg_id = 0;
        self->self_id = self_id;
        self->on_message = [](void*, usize, void*) {};
        self->on_message_ctx = nullptr;
    }

    return self;
}


@end

void subscribe_message_platform(const mtt::String& message_name, void (*handler)(void*, void*), void* ctx)
{
    MTT_Core* core = (MTT_Core*)ctx;
    MTT_Core_Platform* core_platform = (MTT_Core_Platform*)core->core_platform;
    TCP_Client* client = core_platform->net_client;
    [client subscribe_message:message_name withHandler:handler andContext:ctx];
}

void set_net_message_handler(void (*handler)(void*, usize, void*), void* ctx)
{
    MTT_Core* core = (MTT_Core*)ctx;
    MTT_Core_Platform* core_platform = (MTT_Core_Platform*)core->core_platform;
    TCP_Client* client = core_platform->net_client_msgs;
    [client set_message_handler:handler withContext:ctx];
}

void net_write(void* bytes, usize length) 
{
    NSData* data = [NSData dataWithBytes:bytes length:(NSUInteger)length];
    MTT_Core* core = mtt_core_ctx();
    MTT_Core_Platform* core_platform = (MTT_Core_Platform*)core->core_platform;
    TCP_Client* client = core_platform->net_client_msgs;
    
    [client write:data withTag:-1];
}

