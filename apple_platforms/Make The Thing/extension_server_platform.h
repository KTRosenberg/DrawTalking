//
//  extension_server_platform.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/25/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef extension_server_platform_h
#define extension_server_platform_h

#include "extension_server.hpp"

void Extension_Server_init_platform(Extension_Server_Frontend* serv);
bool Extension_Server_start_with_port_platform(Extension_Server_Frontend* serv, uint16 port);
void Extension_Server_stop_platform(Extension_Server_Frontend* serv);

void Extension_Server_deinit_platform(Extension_Server_Frontend* serv);

void Extension_Server_write_platform(Extension_Server_Frontend* serv, cstring msg, usize length);

#endif /* extension_server_platform_h */
