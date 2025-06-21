//
//  memory.mm
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/7/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "memory.hpp"

namespace mem {

void* objc_id_allocate(void* data, usize count)
{
    return calloc(1, count);
}

void objc_id_deallocate(void* data, void* memory, usize count)
{
    __strong id* id_ptr = (__strong id*)memory;
    for (usize i = 0; i < count / sizeof(id); i += 1) {
        id_ptr[i] = nil;
    }
    return mem::free(memory);
}


void* objc_id_resize(void* data, void* memory, usize count, usize old_count)
{
    __strong id* old_mem = (__strong id*)memory;
    __strong id* new_mem = (__strong id*)objc_id_allocate(data, count);
    for (usize i = 0; i < old_count / sizeof(id); i += 1) {
        new_mem[i] = old_mem[i];
        old_mem[i] = nil;
    }
    mem::free(old_mem);
    
    return (void*)new_mem;
}

}
