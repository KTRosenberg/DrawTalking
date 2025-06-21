//
//  linked_list.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/23/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef linked_list_hpp
#define linked_list_hpp

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MTT_List MTT_List;

typedef struct MTT_List_Node {
    void* data;
    struct MTT_List_Node* next;
    struct MTT_List_Node* prev;
    MTT_List* list;
} MTT_List_Node;

typedef struct MTT_List {
    MTT_List_Node head;
    MTT_List_Node tail;
    mem_Allocator allocator;
    usize count;
} MTT_List;

typedef struct MTT_List_Iterator {
    MTT_List* list;
    MTT_List_Node* current;
} MTT_List_Iterator;



void MTT_List_init(MTT_List* list);
void MTT_List_init_with_allocator(MTT_List* list, mem_Allocator* allocator);





MTT_List_Node* MTT_List_Node_make(MTT_List* list, void* data);
void* MTT_List_Node_destroy(MTT_List* list, MTT_List_Node* node);

MTT_List_Node* MTT_List_insert_node(MTT_List_Node* node, MTT_List_Node* where);

MTT_List_Node* MTT_List_prepend_node(MTT_List* list, MTT_List_Node* node);

MTT_List_Node* MTT_List_append_node(MTT_List* list, MTT_List_Node* node);

void MTT_List_append(MTT_List* list, void* data);
void MTT_List_prepend(MTT_List* list, void* data);
void MTT_List_remove_node(MTT_List_Node* node);

inline static MTT_List_Node* MTT_List_begin(MTT_List* list)
{
    return list->head.next;
}

inline static void* MTT_List_data(MTT_List_Node* node)
{
    return node->data;
}

inline static MTT_List_Iterator MTT_List_iterator_begin(MTT_List* list)
{
    MTT_List_Node* node = MTT_List_begin(list);
    MTT_List_Iterator iterator;
    iterator.list = list;
    iterator.current = node;
    return iterator;
}

inline static void MTT_List_advance(MTT_List_Node** node)
{
    *node = (*node)->next;
}

inline static void MTT_List_iterator_advance(MTT_List_Iterator* iter)
{
     MTT_List_advance(&iter->current);
}

inline static void MTT_List_iterator_set(MTT_List_Iterator* iter, MTT_List_Node* node)
{
    iter->current = node;
}

inline static MTT_List_Node* MTT_List_end(MTT_List* list)
{
    return &list->tail;
}

inline static bool MTT_List_iterator_has_next(MTT_List_Iterator* iter)
{
    return (iter->current != MTT_List_end(iter->list));
}

inline static bool MTT_List_is_empty(MTT_List* list)
{
    return (MTT_List_begin(list) == MTT_List_end(list));
}


inline static MTT_List_Node* MTT_List_first(MTT_List* list)
{
    return list->head.next;
}

inline static MTT_List_Node* MTT_List_last(MTT_List* list)
{
    return list->tail.prev;
}

void MTT_List_destroy(MTT_List* list);


typedef struct MTT_List_Auto_Append {
    MTT_List list;
    MTT_List_Iterator iterator;
} MTT_List_Auto_Append;

inline static void* MTT_List_Auto_Append_replace_or_append(MTT_List_Auto_Append* list, void* data)
{
    if (MTT_List_iterator_has_next(&list->iterator)) {
        void* old_data = list->iterator.current->data;
        list->iterator.current->data = data;
        MTT_List_iterator_advance(&list->iterator);
        return old_data;
    }
 
    MTT_List_append(&list->list, data);
    MTT_List_iterator_advance(&list->iterator);
    return NULL;
}

inline static void MTT_List_Auto_Append_reset(MTT_List_Auto_Append* list)
{
    list->iterator = MTT_List_iterator_begin(&list->list);
}

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#include "array.hpp"

template <typename T>
inline static T* MTT_List_begin(mtt::Array_Slice<T>* arr)
{
    return arr->begin();
}
template <typename T>
inline static T* MTT_List_end(mtt::Array_Slice<T>* arr)
{
    return arr->end();
}
template <typename T>
inline static T* MTT_List_advance(T** el_ptr)
{
    return (*el_ptr) += 1;
}

template <typename T>
inline static T MTT_List_data(T* data)
{
    return *data;
}

template <typename T>
inline static T MTT_List_data_ptr(MTT_List* list, T* data)
{
    return *data;
}

template <typename T>
inline static T* MTT_List_data_ptr(mtt::Array_Slice<T>* arr, T* data)
{
    return data;
}




#endif


//struct Node {
//    struct Node* next;
//    struct Node* prev;
//};
//
//struct List {
//    struct Node head;
//    struct Node tail;
//};
//
//void list_init(struct List* list)
//{
//    list->head.prev = 0;
//    list->head.next = &list->tail;
//    list->tail.prev = &list->head;
//    list->tail.next = 0;
//}
//
//void list_append(struct Node* node, struct Node* where)
//{
//    node->prev = where;
//    node->next = where->next;
//    where->next->prev = node;
//    where->next = node;
//}
//
//void list_remove(struct Node* node)
//{
//    node->prev->next = node->next;
//    node->next->prev = node->prev;
//}
//
//#define LIST(list, n) (struct Node* n = (list).head.next; n != &(list).tail; n=n->next)
//
//struct Data
//{
//    struct Node node;
//    int data;
//};
//
//#include <stdio.h>
//
//int main()
//{
//    struct List list;
//    list_init(&list);
//
//    struct Data d1; d1.data = 11;
//    struct Data d2; d2.data = 22;
//    struct Data d3; d3.data = 33;
//
//    // 11 -> 33 -> 22
//    list_append(&d1.node, &list.head);
//    list_append(&d3.node, &d1.node);
//    list_append(&d2.node, &d3.node);
//
//    for LIST(list, n)
//    {
//        struct Data* d = (struct Data*)n;
//        printf("%d\n", d->data);
//    }
//}



#endif /* linked_list_hpp */
