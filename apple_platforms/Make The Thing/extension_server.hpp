//
//  extension_server.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/25/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef extension_server_hpp
#define extension_server_hpp

struct Extension_Server_Frontend {
    const void* backend;
    void (*on_start     )(void) = []() {};
    void (*on_read      )(cstring msg, usize length) = [](cstring, usize) {};
    void (*on_write     )(void) = []() {};
    void (*on_connect   )(void) = []() {};
    void (*on_disconnect)(void) = []() {};
};

void Extension_Server_init(Extension_Server_Frontend* serv);
bool Extension_Server_start_with_port(Extension_Server_Frontend* serv, uint16 port);
void Extension_Server_stop(Extension_Server_Frontend* serv);
void Extension_Server_deinit(Extension_Server_Frontend* serv);

void Extension_Server_write(Extension_Server_Frontend* serv, cstring msg, usize length);

#endif /* extension_server_hpp */
