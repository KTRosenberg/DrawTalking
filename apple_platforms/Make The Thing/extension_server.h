//
//  extension_server.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/25/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef extension_server_h
#define extension_server_h



#include "data_handler.h"

#include "extension_server_platform.h"

@class GCDAsyncSocket;
NS_ASSUME_NONNULL_BEGIN
@interface Extension_Server : NSObject <GCDAsyncSocketDelegate>
{
    @public dispatch_queue_t socketQueue;
    
    @public GCDAsyncSocket *listenSocket;
    @public NSMutableArray *connectedSockets;
    
    @public BOOL is_running;
    
    @public Extension_Server_Frontend* frontend;
    
    UInt16 port;
    

}

-(nonnull instancetype)init;
//- (void) register_callbacks:(Extension_Server_Frontend*)callbacks;

- (BOOL)start:(UInt16)port;
- (void)stop;

- (void)write:(NSData*)data withTag:(long)tag;
@end
NS_ASSUME_NONNULL_END



#endif /* extension_server_h */
