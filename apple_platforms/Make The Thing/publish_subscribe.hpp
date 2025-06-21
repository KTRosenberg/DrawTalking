//
//  publish_subscribe.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 2/2/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef publish_subscribe_hpp
#define publish_subscribe_hpp

namespace mtt {

struct Subscription {
    mtt::Procedure_With_Any proc = {};
    uint64 id = 0;
};

struct Publish_Subscribe {
    mtt::Map<mtt::String, dt::Dynamic_Array<Subscription>> type_to_receivers;
    
    void subscribe(const mtt::String& type, mtt::Procedure_With_Any* proc, uint64 id)
    {
        auto& receivers = type_to_receivers[type];
        receivers.push_back({*proc, id});
    }
    
    void unsubscribe(const mtt::String& type, uint64 id)
    {
        auto find_type = type_to_receivers.find(type);
        if (find_type != type_to_receivers.end()) {
            auto& receivers = find_type->second;
            for (auto it = receivers.begin(); it != receivers.end();) {
                if (it->id == id) {
                    it = receivers.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
};

}

#endif /* publish_subscribe_hpp */
