//
//  tree.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/24/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef tree_hpp
#define tree_hpp

#include <stdlib.h>

struct MTT_Tree_Node;

typedef struct MTT_Tree_Node_List {
    struct MTT_Tree_Node* head;
    struct MTT_Tree_Node* tail;
    usize count;
} MTT_Tree_Node_List;

typedef struct MTT_Tree_Node {
    void* data;
    
    // siblings
    struct MTT_Tree_Node* next;
    struct MTT_Tree_Node* prev;
    
    // child list
    struct MTT_Tree_Node_List child_list;
    
    struct MTT_Tree_Node* parent;
    
    uint64 flags;
} MTT_Tree_Node;

typedef enum MTT_Tree_NODE_UPDATE_TYPE {
    MTT_Tree_NODE_UPDATE_TYPE_MAKE,
    MTT_Tree_NODE_UPDATE_TYPE_ATTACHED,
    MTT_Tree_NODE_UPDATE_TYPE_DETACHED,
    MTT_Tree_NODE_UPDATE_TYPE_WILL_DESTROY,
    MTT_Tree_NODE_UPDATE_TYPE_COPY,
} MTT_Tree_NODE_UPDATE_TYPE;

typedef struct MTT_Tree {
    MTT_Tree_Node* root;
    mem_Allocator allocator;
    void* user_data;
    void (*on_update)(MTT_Tree*, MTT_Tree_Node*, void*, uint64);
} MTT_Tree;


void MTT_Tree_init(MTT_Tree* tree);
void MTT_Tree_init_with_allocator(MTT_Tree* tree, mem_Allocator* allocator);
void MTT_Tree_init_with_allocator_and_callback(MTT_Tree* tree, mem_Allocator* allocator, void (*on_update)(MTT_Tree*, MTT_Tree_Node*, void*, uint64), void* user_data);
void MTT_Tree_child_list_init(MTT_Tree* tree, MTT_Tree_Node_List* list);

MTT_Tree_Node* MTT_Tree_insert_node(MTT_Tree* tree, MTT_Tree_Node* node, MTT_Tree_Node* where);

MTT_Tree_Node* MTT_Tree_insert_new_node(MTT_Tree* tree, MTT_Tree_Node* where, void* data);

MTT_Tree_Node* MTT_Tree_insert_new_root(MTT_Tree* tree, void* data);

MTT_Tree_Node* MTT_Tree_insert_child_at_position(MTT_Tree* tree, MTT_Tree_Node* parent, MTT_Tree_Node* node, unsigned long long idx);

MTT_Tree_Node* MTT_Tree_prepend_child_node(MTT_Tree* tree, MTT_Tree_Node* parent, MTT_Tree_Node* node);

MTT_Tree_Node* MTT_Tree_append_child_node(MTT_Tree* tree, MTT_Tree_Node* parent, MTT_Tree_Node* node);

void MTT_Tree_remove_node(MTT_Tree* tree, MTT_Tree_Node* node);
void MTT_Tree_remove_subtree(MTT_Tree* tree, MTT_Tree_Node* node);
void MTT_Tree_transfer_subtree(MTT_Tree* tree, MTT_Tree_Node* node_dst, MTT_Tree_Node* node_src);
/*
void transplant(Node u, Node v) {
    if (u.parent == sentinel)     // was u the root?
        root = v;                   // if so, now v is the root
    else if (u == u.parent.left)  // otherwise adjust the child of u's parent
        u.parent.left = v;
    else
        u.parent.right = v;
    
    if (v != sentinel)      // if v wasn't the sentinel ...
        v.parent = u.parent;  // ... update its parent
}
*/

inline MTT_Tree_Node* MTT_Tree_child_list_begin(MTT_Tree_Node* node)
{
    if (node == NULL) {
        return NULL;
    }
    
    return node->child_list.head;
}

inline void MTT_Tree_child_list_advance(MTT_Tree_Node** node)
{
    *node = (*node)->next;
}

inline MTT_Tree_Node* MTT_Tree_child_list_end(MTT_Tree_Node* node)
{
    return NULL;
}

inline MTT_Tree_Node_List* MTT_Tree_Node_children(MTT_Tree_Node* node)
{
    return &node->child_list;
}

MTT_Tree_Node* MTT_Tree_Node_init(MTT_Tree* tree, MTT_Tree_Node* node, void* data);
MTT_Tree_Node* MTT_Tree_Node_make(MTT_Tree* tree, void* data);


void MTT_Tree_find_leaves(MTT_Tree* tree, MTT_Tree_Node* root, void (*on_find)(MTT_Tree* tree, MTT_Tree_Node* node, void* user_data), void* user_data);
void* MTT_Tree_Node_destroy(MTT_Tree* tree, MTT_Tree_Node* node);


struct MTT_Tree_Iterator_Ref {
    MTT_Tree_Node* parent;
    MTT_Tree_Node* child;
};
struct MTT_Tree_Iterator {
    MTT_Tree* tree;
#define MAX_TREE_DEPTH (512)
    MTT_Tree_Iterator_Ref stack[MAX_TREE_DEPTH];
    size_t       stack_top;
    unsigned int is_done;
};
inline MTT_Tree_Node* it_value(MTT_Tree_Iterator* it)
{
    return it->stack[it->stack_top].parent;
}
inline unsigned int it_is_done(MTT_Tree_Iterator* it)
{
    return it->is_done;
}

MTT_Tree_Iterator iterator(MTT_Tree* tree, MTT_Tree_Node* subtree_node);
void iterator_advance_preorder(MTT_Tree_Iterator* it);
void iterator_advance_postorder(MTT_Tree_Iterator* it);

inline MTT_Tree_Node* MTT_Tree_root(MTT_Tree* tree)
{
    return tree->root;
}


void  MTT_Tree_preorder_traversal(MTT_Tree* tree, MTT_Tree_Node* root,
                                  void (*procedure)(MTT_Tree* tree, MTT_Tree_Node* current, void* data), void* data);

void  MTT_Tree_postorder_traversal(MTT_Tree* tree, MTT_Tree_Node* root,
                                   void (*procedure)(MTT_Tree* tree, MTT_Tree_Node* current, void* data), void* data);

void  MTT_Tree_depth_traversal(MTT_Tree* tree, MTT_Tree_Node* root,
                               void (*procedure_preorder)(MTT_Tree* tree, MTT_Tree_Node* current, void* data),
                               void (*procedure_postorder)(MTT_Tree* tree, MTT_Tree_Node* current, void* data), void* data);

void MTT_Tree_destroy(MTT_Tree* tree);

#ifdef __cplusplus
namespace mtt {

void set_active_tree(MTT_Tree* tree);
MTT_Tree* get_active_tree(void);

MTT_Tree_Node* tree_node_mk(void* data, std::initializer_list<MTT_Tree_Node*> children);
MTT_Tree_Node* tree_node_mk(void* data, std::vector<MTT_Tree_Node*> children);

MTT_Tree_Node* tree_node_mk(void* data, MTT_Tree_Node* (*callback)(MTT_Tree_Node*), std::initializer_list<MTT_Tree_Node*> children);
MTT_Tree_Node* tree_node_mk(void* data, MTT_Tree_Node* (*callback)(MTT_Tree_Node*), std::vector<MTT_Tree_Node*> children);


MTT_Tree_Node* tree_leaf_mk(void* data);

void tree_build(void* data, std::initializer_list<MTT_Tree_Node*> children);
void tree_build(void* data, std::vector<MTT_Tree_Node*> children);

void tree_build(void* data, MTT_Tree_Node* (*callback)(MTT_Tree_Node*), std::initializer_list<MTT_Tree_Node*> children);
void tree_build(void* data, MTT_Tree_Node* (*callback)(MTT_Tree_Node*), std::vector<MTT_Tree_Node*> children);
void tree_build(MTT_Tree* tree, MTT_Tree_Node* root);

}
#endif

#endif /* tree_hpp */
