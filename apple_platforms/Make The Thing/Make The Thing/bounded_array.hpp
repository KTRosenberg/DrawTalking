//
//  bounded_array.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 8/13/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef bounded_array_h
#define bounded_array_h

template <typename T>
struct Bounded_Dynamic_Array {
    std::vector<T> list;
    usize count;
    
    Bounded_Dynamic_Array() : count(0) {}
    
    inline T& operator[](usize index)
    {
        return this->list[index];
    }

    inline const T& operator[](usize index) const
    {
        return this->list[index];
    }
    
    usize size() const
    {
        return this->count;
    }
    
    usize capacity() const
    {
        return this->list.size();
    }
    
    T* append(T* entry)
    {
        if (this->count >= this->capacity()) {
            this->list.emplace_back(*entry);
            this->count += 1;
        } else {
            this->list[this->count] = *entry;
            this->count += 1;
        }
        
        return &this->list[this->count - 1];
    }
    
    T* append(T entry)
    {
        if (this->count >= this->capacity()) {
            this->list.emplace_back(entry);
            this->count += 1;
        } else {
            this->list[this->count] = entry;
            this->count += 1;
        }
        
        return &this->list[this->count - 1];
    }
    
    void reset()
    {
        this->count = 0;
    }
    
    
};

template <typename T>
void fill_to_capacity(Bounded_Dynamic_Array<T>& arr, usize cap)
{
    while (arr.list.size() < cap) {
        arr.list.emplace_back(T());
    }
    arr.count = cap;
}

template <typename T>
void rewind(Bounded_Dynamic_Array<T>& arr)
{
    arr.count = 0;
}


#endif /* bounded_array_h */
