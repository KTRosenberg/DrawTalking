//
//  misc.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef misc_hpp
#define misc_hpp

namespace mtt {

struct Net_Connection_Info {
    mtt::String ip = {};
    uint16 port = 0;
};

}

void subscribe_message(const mtt::String& message_name, void (*handler)(void*, void*), void* ctx);

void set_net_message_handler(void (*handler)(void*, usize, void*), void* ctx);

void net_write(void* bytes, usize length);

#endif /* misc_hpp */
