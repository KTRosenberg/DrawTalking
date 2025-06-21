//
//  command_history.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/25/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef command_history_hpp
#define command_history_hpp

namespace mtt {

struct Command_Record;
struct Command_History;

typedef Command_Record Entry_Type;
typedef mtt::Dynamic_Array<Entry_Type> Command_Buffer;
typedef Command_Buffer::size_type size_type;

void handler_no_op(Command_Record*, void*);

typedef void(*Command_Handler)(Command_Record*, void*);

struct Command_Record {
    Command_Handler handler = handler_no_op;
    void* custom_data = nullptr;
};
void Command_Record_init(Command_Record* record, Command_Handler handler, void* custom_data);

// adapted from conversation with BYP on HMN discord
struct Command_History {
    size_type index        = 0;
    size_type size         = 0;
    size_type size_in_time = 0;
    
    Command_Buffer cmds = {};
    
    void* custom_data = nullptr;
};

void init(Command_History* self,
          void* custom_data,
          mem::Allocator* allocator,
          Command_Buffer::size_type initial_count);

void deinit(Command_History* self);

void push(Command_History* self, Entry_Type& new_record);

void apply_record(Entry_Type& entry);

bool undo(Command_History* self);


bool redo(Command_History* self);

}

#endif /* command_history_hpp */
