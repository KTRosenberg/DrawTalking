//
//  string_intern.h
//  Make The Thing
//
//  Created by Karl Rosenberg on 9/3/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef string_intern_h
#define string_intern_h

extern_link_begin()

#include "strpool.h"


typedef STRPOOL_U64 MTT_String_Ref_ID;
typedef strpool_t MTT_String_Pool;
typedef uint64 MTT_String_Ref_Pool_ID;
typedef unsigned int MTT_String_Length;

typedef struct MTT_String_Ref {
    MTT_String_Ref_ID id;
    //MTT_String_Pool* pool;
    MTT_String_Ref_Pool_ID pool_id;
    MTT_String_Length length;
    
#ifdef __cplusplus
    operator cstring();
#endif
} MTT_String_Ref;

MTT_String_Ref MTT_string_add_with_length(usize pool_id, cstring string, MTT_String_Length length);

MTT_String_Ref MTT_string_add(usize pool_id, cstring string);
void MTT_string_delete(MTT_String_Ref* string_ref);

MTT_String_Ref MTT_string_ref_retain(MTT_String_Ref ref);

bool MTT_string_ref_release(MTT_String_Ref* string_ref);

int MTT_string_ref_dec_ref_count(MTT_String_Ref string_ref);
int MTT_string_ref_inc_ref_count(MTT_String_Ref string_ref);
int MTT_string_ref_get_ref_count(MTT_String_Ref string_ref);



cstring MTT_string_ref_to_cstring(MTT_String_Ref string_ref);
cstring MTT_string_ref_to_cstring_w_len(MTT_String_Ref string_ref, MTT_String_Length* len);
cstring MTT_string_ref_to_cstring_checked(MTT_String_Ref string_ref);

bool MTT_string_ref_is_valid(MTT_String_Ref string_ref);

bool MTT_string_ref_is_equal(MTT_String_Ref a, MTT_String_Ref b);

bool MTT_string_ref_is_equal(const MTT_String_Ref a, const MTT_String_Ref b);


void MTT_set_string_pool(MTT_String_Pool* pool, usize id);
MTT_String_Pool* MTT_get_string_pool(usize id);

bool MTT_string_ref_is_equal_cstring(MTT_String_Ref a, cstring b);

bool MTT_const_string_ref_is_equal_cstring(const MTT_String_Ref a, cstring b);

MTT_String_Ref MTT_String_Ref_get(cstring str, MTT_String_Ref_Pool_ID pool_id);

extern_link_end()

#endif /* string_intern_h */


#ifdef STRING_INTERN_IMPLEMENTATION
#undef STRING_INTERN_IMPLEMENTATION
#define STRPOOL_IMPLEMENTATION

extern_link_begin()

#include "strpool.h"



MTT_String_Pool* mtt_string_ref_pool[1];
void MTT_set_string_pool(MTT_String_Pool* pool, usize id)
{
    mtt_string_ref_pool[id] = pool;
}

MTT_String_Pool* MTT_get_string_pool(usize id)
{
    return mtt_string_ref_pool[id];
}

MTT_String_Ref MTT_string_add_with_length(usize pool_id, cstring string, MTT_String_Length length)
{
    MTT_String_Pool* pool = MTT_get_string_pool(pool_id);
    MTT_String_Ref_ID id = strpool_inject(pool, string, length);
    int count = strpool_incref(pool, id);
    (void)count;
    return (MTT_String_Ref){.id = id, .pool_id = pool_id, .length = length};
}

MTT_String_Ref MTT_string_add(usize pool_id, cstring string)
{
    return MTT_string_add_with_length(pool_id, string, (MTT_String_Length)strlen(string));
}

void MTT_string_delete(MTT_String_Ref* string_ref)
{
    MTT_String_Pool* pool = MTT_get_string_pool(string_ref->pool_id);
   
    strpool_discard(pool, string_ref->id);
    string_ref->id = 0;
}

MTT_String_Ref MTT_string_ref_retain(MTT_String_Ref ref)
{
    int count = strpool_incref(MTT_get_string_pool(ref.pool_id), ref.id);
    (void)count;
    return ref;
}

bool MTT_string_ref_release(MTT_String_Ref* string_ref)
{
    MTT_String_Pool* pool = MTT_get_string_pool(string_ref->pool_id);
    int ref_count = strpool_decref(pool, string_ref->id);
    
    if (ref_count == 0) {
        strpool_discard(pool, string_ref->id);
        //string_ref->id = 0;
        return true;
    }
    return false;
}

int MTT_string_ref_dec_ref_count(MTT_String_Ref string_ref)
{
    int ref_count = strpool_decref(MTT_get_string_pool(string_ref.pool_id), string_ref.id);
    return ref_count;
}
int MTT_string_ref_inc_ref_count(MTT_String_Ref string_ref)
{
    int ref_count = strpool_incref(MTT_get_string_pool(string_ref.pool_id), string_ref.id);
    return ref_count;
}
int MTT_string_ref_get_ref_count(MTT_String_Ref string_ref)
{
    int ref_count = strpool_getref(MTT_get_string_pool(string_ref.pool_id), string_ref.id);
    return ref_count;
}

cstring MTT_string_ref_to_cstring(MTT_String_Ref string_ref)
{
    return strpool_cstr(MTT_get_string_pool(string_ref.pool_id), string_ref.id);
}
cstring MTT_string_ref_to_cstring_checked(MTT_String_Ref string_ref)
{
    cstring str = MTT_string_ref_to_cstring(string_ref);
    if (str == NULL) {
        return "";
    }
    return str;
}
cstring MTT_string_ref_to_cstring_w_len(MTT_String_Ref string_ref, MTT_String_Length* len)
{
    *len = string_ref.length;
    return MTT_string_ref_to_cstring(string_ref);
}

bool MTT_string_ref_is_valid(MTT_String_Ref string_ref)
{
    return strpool_isvalid(MTT_get_string_pool(string_ref.pool_id), string_ref.id);
}

bool MTT_string_ref_is_equal(MTT_String_Ref a, MTT_String_Ref b)
{
    return a.id == b.id;
}

bool MTT_const_string_ref_is_equal(const MTT_String_Ref a, const MTT_String_Ref b)
{
    return a.id == b.id;
}


bool MTT_string_ref_is_equal_cstring(MTT_String_Ref a, cstring b)
{
    //MTT_String_Pool* pool = MTT_get_string_pool(a.pool_id);
    cstring c_a = MTT_string_ref_to_cstring(a);
    return strcmp(c_a, b) == 0;
}

bool MTT_const_string_ref_is_equal_cstring(const MTT_String_Ref a, cstring b)
{
    //MTT_String_Pool* pool = MTT_get_string_pool(a.pool_id);
    cstring c_a = MTT_string_ref_to_cstring(a);
    return strcmp(c_a, b) == 0;
}

MTT_String_Ref MTT_String_Ref_get(cstring str, MTT_String_Ref_Pool_ID pool_id)
{
    MTT_String_Pool* pool = MTT_get_string_pool(pool_id);
    usize len = strlen(str);
    MTT_String_Ref_ID id = strpool_inject(pool, str, (int)len);
    
    MTT_String_Ref ref;
    ref.id = id;
    ref.pool_id = pool_id;
    ref.length = (int)len;
    return ref;
}


extern_link_end()

#endif
