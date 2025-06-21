//
//  extension_server.m
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/25/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#import "extension_server.h"

#define WELCOME_MSG  0
#define ECHO_MSG     1
#define WARNING_MSG  2
#define DATA_SEND_MSG 3
#define READ_TIMEOUT -1
#define READ_TIMEOUT_EXTENSION 10.0

#define FORMAT(format, ...) [NSString stringWithFormat:(format), ##__VA_ARGS__]

@interface Extension_Server()
@end

@implementation Extension_Server

-(nonnull instancetype)init;
{
    if ((self = [super init])) {
        
        socketQueue = dispatch_queue_create("socketQueue", NULL);
        
        listenSocket = [[GCDAsyncSocket alloc] initWithDelegate:self delegateQueue:socketQueue];
        
        port = 0;
        
        is_running = NO;
        
        connectedSockets = [[NSMutableArray alloc] initWithCapacity:1];
    }
    return self;
}


- (BOOL)start:(UInt16)port {
    if (self->is_running) {
        return NO;
    }
    self->port = port;
    
    NSError * error = nil;
    if (![listenSocket acceptOnPort:port error:&error]) {
        NSLog(@"%@", FORMAT(@"Error starting server: %@", error));
        return NO;
    }
    
    self->is_running = YES;
    
    NSLog(@"%@", FORMAT(@"extension server started on port %hu", [listenSocket localPort]));
    
    self->frontend->on_start();

    return YES;
}
- (void)stop {
    if (!is_running) {
        return;
    }
    
    [listenSocket disconnect];
    
    // Stop any client connections
    @synchronized(connectedSockets)
    {
        NSUInteger i;
        const auto count = [connectedSockets count];
        for (i = 0; i < count; i++) {
            // Call disconnect on the socket,
            // which will invoke the socketDidDisconnect: method,
            // which will remove the socket from the list.
            [[connectedSockets objectAtIndex:i] disconnect];
        }
    }
    
    NSLog(@"%@", @"Stopped Echo server");
    is_running = NO;
}

- (void)socket:(GCDAsyncSocket *)sock didAcceptNewSocket:(GCDAsyncSocket *)newSocket
{
    // This method is executed on the socketQueue (not the main thread)
    
    @synchronized(connectedSockets)
    {
        [connectedSockets addObject:newSocket];
    }
    
    NSString *host = [newSocket connectedHost];
    UInt16 port = [newSocket connectedPort];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        @autoreleasepool {
            
            NSLog(@"%@", FORMAT(@"Accepted client %@:%hu", host, port));
            
        }
    });
    
    NSString *welcomeMsg = @"{ \"type\": \"init\", \"value\":\"Make-The-Thing\" }\r\n";
    NSData *welcomeData = [welcomeMsg dataUsingEncoding:NSUTF8StringEncoding];
    
    [newSocket writeData:welcomeData withTimeout:-1 tag:WELCOME_MSG];
    
    [newSocket readDataToData:[GCDAsyncSocket CRLFData] withTimeout:READ_TIMEOUT tag:0];
    
    self->frontend->on_connect();
}

- (void)socket:(GCDAsyncSocket *)sock didWriteDataWithTag:(long)tag
{
    // This method is executed on the socketQueue (not the main thread)
    
    //if (tag == DATA_SEND_MSG)
    {
        [sock readDataToData:[GCDAsyncSocket CRLFData] withTimeout:READ_TIMEOUT tag:0];
    }
}

- (void)socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag
{
    // This method is executed on the socketQueue (not the main thread)
    
    dispatch_async(dispatch_get_main_queue(), ^{
        @autoreleasepool {
            
            NSData *strData = [data subdataWithRange:NSMakeRange(0, [data length] - 2)];
            NSString *msg = [[NSString alloc] initWithData:strData encoding:NSUTF8StringEncoding];
            if (msg) {
                //NSLog(@"%@", msg);
                cstring c_msg = [msg cStringUsingEncoding:NSUTF8StringEncoding];
                
                self->frontend->on_read(c_msg, [msg lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
            } else {
                NSLog(@"%@", @"Error converting received data into UTF-8 String");
            }
            
        }
    });
    
    @autoreleasepool {
        //// Echo message back to client
        NSData* data_send = [@"{ \"type\": \"MTT_PING\"}\r\n" dataUsingEncoding:NSUTF8StringEncoding];
        [sock writeData:data_send withTimeout:-1 tag:ECHO_MSG];
    }
}

/**
 * This method is called if a read has timed out.
 * It allows us to optionally extend the timeout.
 * We use this method to issue a warning to the user prior to disconnecting them.
 **/
- (NSTimeInterval)socket:(GCDAsyncSocket *)sock shouldTimeoutReadWithTag:(long)tag
                 elapsed:(NSTimeInterval)elapsed
               bytesDone:(NSUInteger)length
{
//    if (elapsed <= READ_TIMEOUT)
//    {
//        NSString *warningMsg = @"Are you still there?\r\n";
//        NSData *warningData = [warningMsg dataUsingEncoding:NSUTF8StringEncoding];
//
//        [sock writeData:warningData withTimeout:-1 tag:WARNING_MSG];
//
//        return READ_TIMEOUT_EXTENSION;
//    }
//
//    return 0.0;
    return INFINITY;
}

- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *)err
{
    if (sock != listenSocket)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            @autoreleasepool {
                
                NSLog(@"%@", FORMAT(@"Client Disconnected"));
                
            }
        });
        
        @synchronized(connectedSockets)
        {
            [connectedSockets removeObject:sock];
        }
    }
}


- (void)write:(NSData*)data withTag:(long)tag {
    //NSLog(@"writing!!!!!!!!!!!\n");
    @synchronized(connectedSockets)
    {
        for (GCDAsyncSocket* connectedSocket in connectedSockets) {
            [connectedSocket writeData:data withTimeout:-1 tag:tag];
        }
    }
}

@end

void Extension_Server_init_platform(Extension_Server_Frontend* serv)
{
    
    Extension_Server* server = [[Extension_Server alloc] init];
    const void* ID = CFBridgingRetain(server);
    serv->backend = ID;
    
    server->frontend = serv;
}
bool Extension_Server_start_with_port_platform(Extension_Server_Frontend* serv, uint16 port)
{
    Extension_Server* server = (__bridge Extension_Server*)serv->backend;
    return [server start:port];
}
void Extension_Server_stop_platform(Extension_Server_Frontend* serv)
{
    Extension_Server* server = (__bridge Extension_Server*)serv->backend;
    [server stop];
}
void Extension_Server_deinit_platform(Extension_Server_Frontend* serv)
{
    @autoreleasepool {
        CFBridgingRelease(serv->backend);
        serv->backend = NULL;
    }
}
void Extension_Server_write_platform(Extension_Server_Frontend* serv, cstring msg, usize length)
{
    Extension_Server* server = (__bridge Extension_Server*)serv->backend;
    @autoreleasepool {
        NSData *data = [NSData dataWithBytes:msg length:length];
        [server write:data withTag:DATA_SEND_MSG];
        data = nil;
    }
}

#undef FORMAT
