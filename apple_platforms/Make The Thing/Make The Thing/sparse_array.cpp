//
//  sparse_array.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/7/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

namespace mtt {

template <typename T_Index, typename T_Dense, typename T_Data>
void init(mtt::Sparse_Array<T_Index, T_Dense, T_Data> * s_a, Sparse_Array_Descriptor* desc)
{
    mtt::init(&s_a->sparse, desc->allocator,      1, desc->initial_capacity);
    mtt::init(&s_a->dense,  desc->allocator,      1, desc->initial_capacity);
    mtt::init(&s_a->data,   desc->data_allocator, 1, desc->initial_capacity);
    
    s_a->max_resident = desc->max_resident;
}

template <typename T_Index, typename T_Dense, typename T_Data>
const T_Data* lookup(mtt::Sparse_Array<T_Index, T_Dense, T_Data>* s_a, T_Index entry_ID)
{
    if (s_a->sparse.count < entry_ID || entry_ID == 0
        ) {
        
        return nullptr;
    }
    
    auto* sparse_record = &s_a->sparse[entry_ID];
    
    auto dense_index = sparse_record->dense_index;
    
    // does not exist
    if (dense_index == 0) {
        return nullptr;
    }
    
    return &s_a->dense[dense_index].data;
}

template <typename T_Index, typename T_Dense, typename T_Data>
bool is_resident(mtt::Sparse_Array<T_Index, T_Dense, T_Data>* s_a, T_Index ent)
{
    
}

template <typename T_Index, typename T_Dense, typename T_Data>
const T_Data* insert(mtt::Sparse_Array<T_Index, T_Dense, T_Data>* s_a, T_Data* data)
{
    Sparse_Array_Dense_Record<T_Index, T_Dense>* last = s_a->dense.last_ptr();
    T_Data* last_data      = s_a->data.last_ptr();
    
    Sparse_Array_Sparse_Record<T_Index>* to_use = &s_a->sparse[last->sparse_index];
    
    to_use->dense_index = s_a->dense.count;
    
    // TODO
    
    return nullptr;
}

template <typename T_Index, typename T_Dense, typename T_Data>
void remove(mtt::Sparse_Array<T_Index, T_Dense, T_Data>* s_a, T_Index entry_ID)
{
    // TODO
}

}
