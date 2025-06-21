//
//  extension_server.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/25/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "extension_server.hpp"
#include "extension_server_platform.h"

void Extension_Server_init(Extension_Server_Frontend* serv)
{
    Extension_Server_init_platform(serv);
}
bool Extension_Server_start_with_port(Extension_Server_Frontend* serv, uint16 port)
{
    return Extension_Server_start_with_port_platform(serv, port);
}
void Extension_Server_stop(Extension_Server_Frontend* serv)
{
    Extension_Server_stop_platform(serv);
}
void Extension_Server_deinit(Extension_Server_Frontend* serv)
{
    Extension_Server_deinit_platform(serv);
}

void Extension_Server_write(Extension_Server_Frontend* serv, cstring msg, usize length)
{
    Extension_Server_write_platform(serv, msg, length);
}
