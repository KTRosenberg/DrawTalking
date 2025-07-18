//
//  job_dispatch.hpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 7/19/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef job_dispatch_hpp
#define job_dispatch_hpp

namespace mtt {

void job_dispatch_serial(void* ctx, void (*procedure)(void*), void (*callback)(void*));
void job_dispatch_serial_sync(void* ctx, void (*procedure)(void*));
void job_dispatch_to_main(void* ctx, void (*procedure)(void*));

bool is_main_thread(void);

}

#endif /* job_dispatch_hpp */
