//
//  linked_list.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/23/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "linked_list.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void MTT_List_init(MTT_List* list)
{
    memset(list, 0, sizeof(*list));
    list->head.prev = nullptr;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = nullptr;
    list->count = 0;
}

void MTT_List_init_with_allocator(MTT_List* list, mem_Allocator* allocator)
{
    MTT_List_init(list);
    list->allocator = *allocator;
}



MTT_List_Node* MTT_List_Node_make(MTT_List* list, void* data)
{
    mem_Allocator* allocator = &list->allocator;
    
    MTT_List_Node* node = (MTT_List_Node*)allocator->allocate(allocator, sizeof(MTT_List_Node));
    if (node != NULL) {
        memset(node, 0, sizeof(MTT_List_Node));
        node->data = data;
    }
    
    node->list = list;
    
    return node;
}

void* MTT_List_Node_destroy(MTT_List* list, MTT_List_Node* node)
{
    mem_Allocator* allocator = &list->allocator;
    
    void* data = node->data;
    
    allocator->deallocate(allocator, node, sizeof(MTT_List_Node));
    
    return data;
}


MTT_List_Node* MTT_List_insert_node(MTT_List_Node* node, MTT_List_Node* where)
{
    node->prev = where;
    node->next = where->next;
    where->next->prev = node;
    where->next = node;
    node->list->count += 1;
    
    return node;
}

MTT_List_Node* MTT_List_prepend_node(MTT_List* list, MTT_List_Node* node)
{
    return MTT_List_insert_node(node, &list->head);
}

MTT_List_Node* MTT_List_append_node(MTT_List* list, MTT_List_Node* node)
{
    return MTT_List_insert_node(node, list->tail.prev);
}

void MTT_List_append(MTT_List* list, void* data)
{
    MTT_List_append_node(list, MTT_List_Node_make(list, data));
}

void MTT_List_prepend(MTT_List* list, void* data)
{
    MTT_List_prepend_node(list, MTT_List_Node_make(list, data));
}

void MTT_List_remove_node(MTT_List_Node* node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->list->count -= 1;
    node->list = NULL;
}

void MTT_List_destroy(MTT_List* list)
{
    while (!MTT_List_is_empty(list)) {
        MTT_List_Node* to_remove = MTT_List_first(list);
        MTT_List_remove_node(to_remove);
        MTT_List_Node_destroy(list, to_remove);
    }
    list->count = 0;
}

#ifdef __cplusplus
}
#endif
