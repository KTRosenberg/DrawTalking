//
//  tree.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/24/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "tree.hpp"

void MTT_Tree_init(MTT_Tree* tree)
{
    tree->root = NULL;
    tree->allocator = {};
    tree->on_update = NULL;
    tree->user_data = NULL;
}
void MTT_Tree_init_with_allocator(MTT_Tree* tree, mem_Allocator* allocator)
{
    MTT_Tree_init(tree);
    tree->allocator = *allocator;
}
void MTT_Tree_init_with_allocator_and_callback(MTT_Tree* tree, mem_Allocator* allocator, void (*on_update)(MTT_Tree*, MTT_Tree_Node*, void*, uint64), void* user_data)
{
    MTT_Tree_init_with_allocator(tree, allocator);
    tree->on_update = on_update;
    tree->user_data = user_data;
}
void MTT_Tree_child_list_init(MTT_Tree* tree, MTT_Tree_Node_List* list)
{
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}


MTT_Tree_Node* MTT_Tree_insert_node(MTT_Tree* tree, MTT_Tree_Node* node, MTT_Tree_Node* where)
{
    if (where == tree->root) {
        if (tree->root == NULL) {
            tree->root = node;
            tree->root->parent = NULL;
            tree->root->prev   = NULL;
            tree->root->next   = NULL;
            
            if (tree->on_update) {
                tree->on_update(tree, node, tree->user_data, MTT_Tree_NODE_UPDATE_TYPE_ATTACHED);
                
            }
            
            //MTT_Tree_Node_init(tree, node, NULL);
            
        } else {
            MTT_Tree_Node* old_root = tree->root;
            MTT_Tree_Node* new_root = node;
            new_root->parent = NULL;
            new_root->next = NULL;
            new_root->prev = NULL;
            tree->root = new_root;
            
            //MTT_Tree_Node_init(tree, node, NULL);
            
            MTT_Tree_append_child_node(tree, new_root, old_root);
            
            if (tree->on_update) {
                tree->on_update(tree, old_root, tree->user_data, MTT_Tree_NODE_UPDATE_TYPE_ATTACHED);
                
            }
        }
        

        
        return node;
    }
    
    //MTT_Tree_Node_init(tree, node, NULL);
    
    node->parent = where->parent;
    node->next = where->next;
    node->prev = where;
    node->parent->child_list.count += 1;
    
    if (where->next == NULL) {
        where->parent->child_list.tail = node;
    }
    
    where->next = node;
    
    if (tree->on_update) {
        tree->on_update(tree, node, tree->user_data, MTT_Tree_NODE_UPDATE_TYPE_ATTACHED);
        
    }
    
    return node;
}

MTT_Tree_Node* MTT_Tree_insert_new_node(MTT_Tree* tree, MTT_Tree_Node* where, void* data)
{
    return MTT_Tree_insert_node(tree, MTT_Tree_Node_make(tree, data), where);
}

MTT_Tree_Node* MTT_Tree_insert_new_root(MTT_Tree* tree, void* data)
{
    return MTT_Tree_insert_new_node(tree, MTT_Tree_root(tree), data);
}

MTT_Tree_Node* MTT_Tree_insert_child_at_position(MTT_Tree* tree, MTT_Tree_Node* parent, MTT_Tree_Node* node, unsigned long long idx)
{

    ASSERT_MSG(false, "TODO: implementation of %s\n", __PRETTY_FUNCTION__);
    // TODO:
    
    
//    node->parent = where->parent;
//    node->next = where->next;
//    node->prev = where;
//
//    if (where->next == NULL) {
//        where->parent->child_list.tail = node;
//    }
//
//    where->next = node;
//    return node;
    // TODO:
    return NULL;
}

MTT_Tree_Node* MTT_Tree_prepend_child_node(MTT_Tree* tree, MTT_Tree_Node* parent, MTT_Tree_Node* node)
{
    node->parent = parent;
    
    node->next = parent->child_list.head;
    node->prev = NULL;
    
    if (parent->child_list.head == NULL) {
        parent->child_list.tail = node;
    } else {
        parent->child_list.head->prev = node;
    }
    
    parent->child_list.head = node;
    
    parent->child_list.count += 1;
    
    if (tree->on_update) {
        tree->on_update(tree, node, tree->user_data, MTT_Tree_NODE_UPDATE_TYPE_ATTACHED);
    }
    
    return node;
}

MTT_Tree_Node* MTT_Tree_append_child_node(MTT_Tree* tree, MTT_Tree_Node* parent, MTT_Tree_Node* node)
{
    node->parent = parent;
    
    node->next = NULL;
    node->prev = parent->child_list.tail;
    
    if (parent->child_list.head == NULL) {
        parent->child_list.head = node;
    } else {
        parent->child_list.tail->next = node;
    }
    
    parent->child_list.tail = node;
    
    parent->child_list.count += 1;
    
    if (tree->on_update) {
        tree->on_update(tree, node, tree->user_data, MTT_Tree_NODE_UPDATE_TYPE_ATTACHED);
        
    }
    
    return node;
}

void MTT_Tree_remove_subtree(MTT_Tree* tree, MTT_Tree_Node* node);
void MTT_Tree_add_subtree(MTT_Tree* tree, MTT_Tree_Node* node);


MTT_Tree_Node* MTT_Tree_Node_init(MTT_Tree* tree, MTT_Tree_Node* node, void* data)
{
    *node = {};
    MTT_Tree_child_list_init(tree, &node->child_list);
    
    node->data = data;
    return node;
}
MTT_Tree_Node* MTT_Tree_Node_make(MTT_Tree* tree, void* data)
{
    MTT_Tree_Node* node = (MTT_Tree_Node*)tree->allocator.allocate(&tree->allocator, sizeof(MTT_Tree_Node));
    assert(node != NULL);
    
    MTT_Tree_Node_init(tree, node, data);
    
    if (tree->on_update) {
        tree->on_update(tree, node, tree->user_data, MTT_Tree_NODE_UPDATE_TYPE_MAKE);
    }
    
    return node;
}

void* MTT_Tree_Node_destroy(MTT_Tree* tree, MTT_Tree_Node* node)
{
    if (tree->on_update) {
        tree->on_update(tree, node, tree->user_data, MTT_Tree_NODE_UPDATE_TYPE_WILL_DESTROY);
    }
    
    if (node->parent != NULL) {
        node->parent->child_list.count -= 1;
    }
    
    void* data = node->data;
    tree->allocator.deallocate(&tree->allocator, (void*)node, sizeof(MTT_Tree_Node));
    

    return data;
}

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

MTT_Tree_Iterator iterator(MTT_Tree* tree, MTT_Tree_Node* subtree_node)
{
    MTT_Tree_Iterator iterator;
    
    if (subtree_node == NULL) {
        iterator.is_done = true;
        return iterator;
    }
    
    iterator.tree = tree;
    iterator.stack[0].parent = subtree_node;
    iterator.stack[0].child  = subtree_node->child_list.head;
    iterator.stack_top = 0;
    iterator.is_done = 0;
    
    return iterator;
}

void iterator_advance_preorder(MTT_Tree_Iterator* it)
{
    // TODO: //
    
    MTT_Tree_Iterator_Ref* current = &it->stack[it->stack_top];
    
    if (current->child != NULL) {
        it->stack_top += 1;
        assert(it->stack_top < MAX_TREE_DEPTH);
        
        it->stack[it->stack_top].parent = current->child;
        it->stack[it->stack_top].child  = current->child->child_list.head;
    } else {
        if (it->stack_top == 0) {
            it->is_done = 1;
            return;
        }
        it->stack_top -= 1;
        it->stack[it->stack_top].child = it->stack[it->stack_top].child->next;
    }
}

void iterator_advance_postorder(MTT_Tree_Iterator* it)
{
    // TODO: //
}

void  MTT_Tree_preorder_traversal(MTT_Tree* tree, MTT_Tree_Node* root,
                                  void (*procedure)(MTT_Tree* tree, MTT_Tree_Node* current, void* data), void* data)
{
    if (root == nullptr) {
        return;
    }
    procedure(tree, root, data);
    
    for (MTT_Tree_Node* it = MTT_Tree_child_list_begin(root);
         it != MTT_Tree_child_list_end(root); MTT_Tree_child_list_advance(&it)) {
        
        MTT_Tree_preorder_traversal(tree, it, procedure, data);
    }
}

void  MTT_Tree_postorder_traversal(MTT_Tree* tree, MTT_Tree_Node* root,
                                  void (*procedure)(MTT_Tree* tree, MTT_Tree_Node* current, void* data), void* data)
{
    if (root == nullptr) {
        return;
    }
    
    for (MTT_Tree_Node* it = MTT_Tree_child_list_begin(root);
         it != MTT_Tree_child_list_end(root); MTT_Tree_child_list_advance(&it)) {
        
        MTT_Tree_postorder_traversal(tree, it, procedure, data);
    }
    
    procedure(tree, root, data);

}

void  MTT_Tree_depth_traversal(MTT_Tree* tree, MTT_Tree_Node* root,
                               void (*procedure_preorder)(MTT_Tree* tree, MTT_Tree_Node* current, void* data),
                               void (*procedure_postorder)(MTT_Tree* tree, MTT_Tree_Node* current, void* data), void* data)
{
    if (root == nullptr) {
        return;
    }
    
    procedure_preorder(tree, root, data);
    
    for (MTT_Tree_Node* it = MTT_Tree_child_list_begin(root);
         it != MTT_Tree_child_list_end(root); MTT_Tree_child_list_advance(&it)) {
        
        MTT_Tree_depth_traversal(tree, it, procedure_preorder, procedure_postorder, data);
    }
    
    procedure_postorder(tree, root, data);
}

void MTT_Tree_find_leaves(MTT_Tree* tree, MTT_Tree_Node* root, void (*on_find)(MTT_Tree* tree, MTT_Tree_Node* node, void* user_data), void* user_data)
{
    if (MTT_Tree_child_list_begin(root) == MTT_Tree_child_list_end(root)) {
        // leaf found
        on_find(tree, root, user_data);
        
        return;
    }
    
    for (MTT_Tree_Node* it = MTT_Tree_child_list_begin(root);
         it != MTT_Tree_child_list_end(root); MTT_Tree_child_list_advance(&it)) {
        MTT_Tree_find_leaves(tree, it, on_find, user_data);
    }
}

void MTT_Tree_Node_destroy_internal(MTT_Tree* tree, MTT_Tree_Node* node, void* _)
{
    if (tree->on_update) {
        tree->on_update(tree, node, tree->user_data, MTT_Tree_NODE_UPDATE_TYPE_WILL_DESTROY);
    }
    
    tree->allocator.deallocate(&tree->allocator, (void*)node, sizeof(MTT_Tree_Node));
}

void MTT_Tree_destroy(MTT_Tree* tree)
{
    
    MTT_Tree_postorder_traversal(tree, tree->root, MTT_Tree_Node_destroy_internal, NULL);
    
    tree->root = NULL;
}


#ifdef __cplusplus
namespace mtt {

MTT_Tree* active_tree = NULL;

void set_active_tree(MTT_Tree* tree)
{
    active_tree = tree;
}
MTT_Tree* get_active_tree(void)
{
    return active_tree;
}

MTT_Tree_Node* tree_node_mk(void* data, std::initializer_list<MTT_Tree_Node*> children)
{
    MTT_Tree* tree = active_tree;
    MTT_Tree_Node* node = MTT_Tree_Node_make(tree, data);
    
    for (auto it = children.begin(); it != children.end(); ++it) {
        MTT_Tree_append_child_node(tree, node, *it);
    }
    
    return node;
}
MTT_Tree_Node* tree_node_mk(void* data, std::vector<MTT_Tree_Node*> children)
{
    MTT_Tree* tree = active_tree;
    MTT_Tree_Node* node = MTT_Tree_Node_make(tree, data);
    
    for (auto it = children.begin(); it != children.end(); ++it) {
        MTT_Tree_append_child_node(tree, node, *it);
    }
    
    return node;
}

MTT_Tree_Node* tree_node_mk(void* data, MTT_Tree_Node* (*callback)(MTT_Tree_Node*), std::initializer_list<MTT_Tree_Node*> children)
{
    return callback(tree_node_mk(data, children));
}
MTT_Tree_Node* tree_node_mk(void* data, MTT_Tree_Node* (*callback)(MTT_Tree_Node*), std::vector<MTT_Tree_Node*> children)
{
    return callback(tree_node_mk(data, children));
}

MTT_Tree_Node* tree_leaf_mk(void* data)
{
    MTT_Tree* tree = active_tree;
    MTT_Tree_Node* node = MTT_Tree_Node_make(tree, data);
    return node;
}

void tree_build(void* data, std::initializer_list<MTT_Tree_Node*> children)
{
    assert(active_tree != NULL);
    
    MTT_Tree* tree = active_tree;
    
    tree->root = MTT_Tree_Node_make(tree, data);
    
    for (auto it = children.begin(); it != children.end(); ++it) {
        MTT_Tree_append_child_node(tree, tree->root, *it);
    }
}
void tree_build(void* data, std::vector<MTT_Tree_Node*> children)
{
    assert(active_tree != NULL);
    
    MTT_Tree* tree = active_tree;
    
    tree->root = MTT_Tree_Node_make(tree, data);
    
    for (auto it = children.begin(); it != children.end(); ++it) {
        MTT_Tree_append_child_node(tree, tree->root, *it);
    }
}

void tree_build(MTT_Tree* tree, MTT_Tree_Node* root)
{
    tree->root = root;
}

void tree_build(void* data, MTT_Tree_Node* (*callback)(MTT_Tree_Node*), std::initializer_list<MTT_Tree_Node*> children)
{
    assert(active_tree != NULL);
    
    MTT_Tree* tree = active_tree;
    
    tree->root = MTT_Tree_Node_make(tree, data);
    
    for (auto it = children.begin(); it != children.end(); ++it) {
        MTT_Tree_append_child_node(tree, tree->root, *it);
    }
    
    callback(tree->root);
}

void tree_build(void* data, MTT_Tree_Node* (*callback)(MTT_Tree_Node*), std::vector<MTT_Tree_Node*> children)
{
    assert(active_tree != NULL);
    
    MTT_Tree* tree = active_tree;
    
    tree->root = MTT_Tree_Node_make(tree, data);
    
    for (auto it = children.begin(); it != children.end(); ++it) {
        MTT_Tree_append_child_node(tree, tree->root, *it);
    }
    
    callback(tree->root);
}


}
#endif
