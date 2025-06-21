//
//  sparse_array.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 10/27/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef sparse_array_hpp
#define sparse_array_hpp

//#include "array.hpp"

namespace mtt {



//template<typename T>
template <typename T_Index, typename T_Dense>
struct Sparse_Array_Dense_Record {
    T_Index sparse_index;
    
    T_Dense data;
};

template<typename T_Index>
struct Sparse_Array_Sparse_Record {
    T_Index dense_index;
};

template<typename T_Index, typename T_Dense, typename T_Data>
struct Sparse_Array {
    mtt::Dynamic_Array<Sparse_Array_Sparse_Record<T_Index>>         sparse;
    mtt::Dynamic_Array<Sparse_Array_Dense_Record<T_Index, T_Dense>> dense;
    mtt::Dynamic_Array<T_Data>                                      data;
    
    usize max_resident;
    
    void (*on_add)(void);
    void (*on_destroy)(void);
};

struct Sparse_Array_Descriptor {
    mem::Allocator allocator;
    mem::Allocator data_allocator;
    usize initial_capacity;
    usize active_entry_count;
    usize max_resident;
};

template <typename T_Index, typename T_Dense, typename T_Data>
void init(mtt::Sparse_Array<T_Index, T_Dense, T_Data> * s_a, Sparse_Array_Descriptor* desc);

template <typename T_Index, typename T_Dense, typename T_Data>
const T_Data* lookup(mtt::Sparse_Array<T_Index, T_Dense, T_Data>* s_a, T_Index entry_ID);

template <typename T_Index, typename T_Dense, typename T_Data>
bool is_resident(mtt::Sparse_Array<T_Index, T_Dense, T_Data>* s_a, T_Index ent);

template <typename T_Index, typename T_Dense, typename T_Data>
const T_Data* insert(mtt::Sparse_Array<T_Index, T_Dense, T_Data>* s_a, T_Data* data);

template <typename T_Index, typename T_Dense, typename T_Data>
void remove(mtt::Sparse_Array<T_Index, T_Dense, T_Data>* s_a, T_Index entry_ID);


}

#include "sparse_array.cpp"

#endif /* sparse_array_hpp */
