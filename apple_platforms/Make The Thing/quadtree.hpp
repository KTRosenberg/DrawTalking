//
//  quadtree.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/14/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef quadtree_hpp
#define quadtree_hpp

#ifndef INT_LIST_H
#define INT_LIST_H

#ifdef __cplusplus
    #define IL_FUNC extern "C"
#else
    #define IL_FUNC
#endif

typedef struct IntList IntList;
enum {il_fixed_cap = 128};

struct IntList
{
    // Stores a fixed-size buffer in advance to avoid requiring
    // a heap allocation until we run out of space.
    int fixed[il_fixed_cap];

    // Points to the buffer used by the list. Initially this will
    // point to 'fixed'.
    int* data;

    // Stores how many integer fields each element has.
    int num_fields;

    // Stores the number of elements in the list.
    int num;

    // Stores the capacity of the array.
    int cap;

    // Stores an index to the free element or -1 if the free list
    // is empty.
    int free_element;
};

// ---------------------------------------------------------------------------------
// List Interface
// ---------------------------------------------------------------------------------
// Creates a new list of elements which each consist of integer fields.
// 'num_fields' specifies the number of integer fields each element has.
IL_FUNC void il_create(IntList* il, int num_fields);

// Destroys the specified list.
IL_FUNC void il_destroy(IntList* il);

// Returns the number of elements in the list.
IL_FUNC int il_size(const IntList* il);

// Returns the value of the specified field for the nth element.
IL_FUNC int il_get(const IntList* il, int n, int field);

// Sets the value of the specified field for the nth element.
IL_FUNC void il_set(IntList* il, int n, int field, int val);

// Clears the specified list, making it empty.
IL_FUNC void il_clear(IntList* il);

// ---------------------------------------------------------------------------------
// Stack Interface (do not mix with free list usage; use one or the other)
// ---------------------------------------------------------------------------------
// Inserts an element to the back of the list and returns an index to it.
IL_FUNC int il_push_back(IntList* il);

// Removes the element at the back of the list.
IL_FUNC void il_pop_back(IntList* il);

// ---------------------------------------------------------------------------------
// Free List Interface (do not mix with stack usage; use one or the other)
// ---------------------------------------------------------------------------------
// Inserts an element to a vacant position in the list and returns an index to it.
IL_FUNC int il_insert(IntList* il);

// Removes the nth element in the list.
IL_FUNC void il_erase(IntList* il, int n);

#endif

#ifndef QUADTREE_H
#define QUADTREE_H

//#include "IntList.h"

#ifdef __cplusplus
    #define QTREE_FUNC extern "C"
#else
    #define QTREE_FUNC
#endif

typedef struct Quadtree Quadtree;

struct Quadtree
{
    // Stores all the nodes in the quadtree. The first node in this
    // sequence is always the root.
    IntList nodes;

    // Stores all the elements in the quadtree.
    IntList elts;

    // Stores all the element nodes in the quadtree.
    IntList enodes;

    // Stores the quadtree extents.
    int root_mx, root_my, root_sx, root_sy;

    // Maximum allowed elements in a leaf before the leaf is subdivided/split unless
    // the leaf is at the maximum allowed tree depth.
    int max_elements;

    // Stores the maximum depth allowed for the quadtree.
    int max_depth;

    // Temporary buffer used for queries.
    char* temp;

    // Stores the size of the temporary buffer.
    int temp_size;
};

// Function signature used for traversing a tree node.
typedef void QtNodeFunc(Quadtree* qt, void* user_data, int node, int depth, int mx, int my, int sx, int sy);

// Creates a quadtree with the requested extents, maximum elements per leaf, and maximum tree depth.
QTREE_FUNC void qt_create(Quadtree* qt, int width, int height, int max_elements, int max_depth);

// Destroys the quadtree.
QTREE_FUNC void qt_destroy(Quadtree* qt);

// Inserts a new element to the tree.
// Returns an index to the new element.
QTREE_FUNC int qt_insert(Quadtree* qt, int id, float x1, float y1, float x2, float y2);

// Removes the specified element from the tree.
QTREE_FUNC void qt_remove(Quadtree* qt, int element);

// Cleans up the tree, removing empty leaves.
QTREE_FUNC void qt_cleanup(Quadtree* qt);

// Outputs a list of elements found in the specified rectangle.
QTREE_FUNC void qt_query(Quadtree* qt, IntList* out, float x1, float y1, float x2, float y2, int omit_element);

// Traverses all the nodes in the tree, calling 'branch' for branch nodes and 'leaf'
// for leaf nodes.
QTREE_FUNC void qt_traverse(Quadtree* qt, void* user_data, QtNodeFunc* branch, QtNodeFunc* leaf);

#endif

#endif /* quadtree_hpp */

