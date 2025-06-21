//
//  command_history.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/25/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#include "command_history.hpp"

namespace mtt {

void handler_no_op(Command_Record*, void*)
{
    return;
}

void Command_Record_init(Command_Record* record, Command_Handler handler, void* custom_data)
{
    record->handler = handler;
    record->custom_data = custom_data;
}


void init(Command_History* self, void* custom_data, mem::Allocator* allocator, Command_Buffer::size_type initial_count)
{
    mtt::init(&self->cmds, *allocator, initial_count);
    self->custom_data = custom_data;
}

void deinit(Command_History* self)
{
    mtt::deinit(&self->cmds);
}

//
void push(Command_History* self, Entry_Type& new_record)
{
    // will have to prune stack size here
    self->cmds.set_size(self->index);
    self->cmds.push_back(new_record);
    self->index += 1;
    
    //size += 1;
    self->size = self->size_in_time + 1;
    self->size_in_time = self->size;
}

void apply_record(Command_History* self, Entry_Type& entry) {
    entry.handler(&entry, self->custom_data);
};

bool undo(Command_History* self)
{
    if (self->index == 0) MTT_UNLIKELY {
        return false;
    }
    
    self->index -= 1;
    apply_record(self, self->cmds[self->index]);
    self->size_in_time -= 1;
    return true;
}


bool redo(Command_History* self)
{
    if (self->index == self->size) MTT_UNLIKELY {
        return false;
    }
    
    apply_record(self, self->cmds[self->index]);
    self->index += 1;
    self->size_in_time += 1;
    return true;
}


}
