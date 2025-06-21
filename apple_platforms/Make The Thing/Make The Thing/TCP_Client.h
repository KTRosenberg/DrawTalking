//
//  TCP_Client.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/28/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef TCP_Client_h
#define TCP_Client_h

//#import "GCD/GCDAsyncSocket.h"
//#import "GCD/GCDAsyncUdpSocket.h"

#include "data_handler.h"



@class GCDAsyncSocket;
NS_ASSUME_NONNULL_BEGIN
@interface TCP_Client : NSObject <GCDAsyncSocketDelegate>
{
    @public GCDAsyncSocket* async_socket;
    @public NSString* host;
    @public UInt16    port;
    @public bool      is_disconnected;
    
    @public void (*on_load)(TCP_Client* _Nonnull client, void* _Nonnull path, void* _Nonnull data);
    @public void (*on_query_result)(TCP_Client* _Nonnull client, void* _Nonnull data, usize ID);
    @public void (*on_message)(void* _Nonnull data, usize length, void* _Nonnull ctx);
    @public void* on_message_ctx;
    
    @public void* user_data;
}

- (void)load:(NSString* _Nonnull) message;

- (void)query:(NSString* _Nonnull) message;

- (void)sendMessage:(NSString* _Nonnull) message;

- (void)socket:(GCDAsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port;

- (void)socket:(GCDAsyncSocket *)sock didReceiveTrust:(SecTrustRef)trust
completionHandler:(void (^)(BOOL shouldTrustPeer))completionHandler;

- (void)socketDidSecure:(GCDAsyncSocket *)sock;

- (void)socket:(GCDAsyncSocket *)sock didWriteDataWithTag:(long)tag;

- (void)socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag;

- (void)socket:(GCDAsyncSocket *)sock didReadPartialDataOfLength:(NSUInteger)partialLength tag:(long)tag;

- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *_Nullable)err;

- (void)write:(NSData*)data withTag:(long)tag;


- (void)subscribe:(Data_Handler*)handler;
- (void)subscribe_message:(const mtt::String&) message_name withHandler:(void (*)(void*, void*))handler andContext:(void*)ctx;
- (void)set_message_handler:(void(*)(void*, usize, void*)) handler withContext:(void*)ctx;

-(nonnull instancetype)initWithHost:(NSString*)host port:(UInt16)port selfID:(uint64)self_id;

@end
NS_ASSUME_NONNULL_END



#endif /* TCP_Client_h */
