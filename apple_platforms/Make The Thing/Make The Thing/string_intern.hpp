//
//  string_intern.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 9/28/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef string_intern_hpp
#define string_intern_hpp


#include "string_intern.h"


namespace mtt {

#define STRING_REF_DEBUG (false)

struct MTT_String_Ref_Modify {
    cstring read()
    {
        return MTT_string_ref_to_cstring(this->ref);
    }
    void write(cstring to_write)
    {
        MTT_String_Ref old = this->ref;
        MTT_string_add(0, to_write);
        if (MTT_string_ref_is_valid(old)) {
            MTT_string_ref_release(&old);
        }
    }
    MTT_String_Ref ref;
};

static inline bool operator==(const MTT_String_Ref& left, const MTT_String_Ref& right)
{
    return left.id == right.id;
}

//struct String_Ref {
//    MTT_String_Ref ref;
//
//    static void set_string_pool(MTT_String_Pool* pool, usize id)
//    {
//        MTT_set_string_pool(pool, id);
//    }
//    
//    static MTT_String_Pool* get_string_pool(usize id)
//    {
//        return MTT_get_string_pool(id);
//    }
//    
//    void operator=(const String_Ref& ref)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        if (*this == ref) {
//            return;
//        }
//        
//        this->free();
//        this->ref = MTT_string_ref_retain(ref.ref);
//    }
//    
//    void operator=(cstring& cstr)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        this->free();
//        this->ref = MTT_string_add(0, cstr);
//    }
//    void operator=(MTT_String_Ref& str_ref)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        this->free();
//        this->ref = MTT_string_ref_retain(str_ref);
//    }
//    
//    void operator=(MTT_String_Ref str_ref)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        
//        this->free();
//        this->ref = MTT_string_ref_retain(str_ref);
//    }
//    
//    void operator=(const robin_hood::pair<usize, cstring>& cstr_pool_id_pair)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        this->free();
//        
//        usize pool_id = cstr_pool_id_pair.first;
//        cstring str = cstr_pool_id_pair.second;
//        this->ref = MTT_string_add(pool_id, str);
//    }
//    
////    operator MTT_String_Ref() {
////        if constexpr (STRING_REF_DEBUG) {
////            MTT_print("%s\n", __PRETTY_FUNCTION__);
////        }
////        this->inc_ref_count();
////        
////        return this->ref;
////    }
//    operator cstring() {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        return MTT_string_ref_to_cstring(this->ref);
//    }
//    
//    cstring ptr() { return MTT_string_ref_to_cstring(this->ref); }
//    cstring c_str() { return MTT_string_ref_to_cstring(this->ref); }
//    
//    String_Ref() = default;
//    
//    
//    String_Ref(cstring cstr)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        this->ref = MTT_string_add(0, cstr);
//    }
//    
//    String_Ref(MTT_String_Ref ref)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        this->ref = ref;
//        MTT_string_ref_retain(ref);
//    }
//    
//    String_Ref(const String_Ref& ref)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//            int COPY = 0;
//        }
//        
//        *this = ref;
//    }
//    String_Ref(const String_Ref*& ref)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//            int COPY = 0;
//        }
//        
//        *this = *ref;
//    }
//    
//    String_Ref(String_Ref&& other)
//    : ref(other.ref)
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//            int MOVE = 0;
//        }
//        
//    }
//    
//    int inc_ref_count()
//    {
//        const int count = MTT_string_ref_inc_ref_count(this->ref);
//        return count;
//    }
//    
//    int dec_ref_count()
//    {
//        const int count = MTT_string_ref_dec_ref_count(this->ref);
//        return count;
//    }
//    
//    int get_ref_count()
//    {
//        const int count = MTT_string_ref_get_ref_count(this->ref);
//        return count;
//    }
//    
//    void free()
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        MTT_string_ref_release(&this->ref);
//    }
//    
//    ~String_Ref()
//    {
//        if constexpr (STRING_REF_DEBUG) {
//            MTT_print("%s\n", __PRETTY_FUNCTION__);
//        }
//        this->free();
//    }
//    
//    bool operator==(String_Ref& other) {
//        return MTT_string_ref_is_equal(this->ref, other.ref);
//    }
//    bool operator!=(String_Ref& other) {
//        return !MTT_string_ref_is_equal(this->ref, other.ref);
//    }
//    
//    bool operator==(MTT_String_Ref& other) {
//        return MTT_string_ref_is_equal(this->ref, other);
//    }
//    bool operator!=(MTT_String_Ref& other) {
//        return !MTT_string_ref_is_equal(this->ref, ref);
//    }
//    
//    bool operator==(const String_Ref& other) {
//        return MTT_string_ref_is_equal(this->ref, other.ref);
//    }
//    bool operator!=(const String_Ref& other) {
//        return !MTT_string_ref_is_equal(this->ref, other.ref);
//    }
//    
//    bool is_valid() { return MTT_string_ref_is_valid(this->ref); }
//};
//
//static inline String_Ref string_ref(MTT_String_Ref ref)
//{
//    return ref;
//}

//static_assert(std::is_standard_layout<String_Ref>::value && std::is_trivially_copyable<String_Ref>::value, "check that String_Ref has standard layout and is trivially copiable");

}

#endif /* string_intern_hpp */
