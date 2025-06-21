//
//  drawtalk_definitions_tree.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/9/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_definitions_tree_hpp
#define drawtalk_definitions_tree_hpp

// UNUSED

namespace dt {

[[deprecated]]
struct System_Tree_Search {
    MTT_Tree_Node* result;
    
    System_Tree_Search search(const mtt::String& label);
    
    operator MTT_Tree_Node*()
    {
        return this->result;
    }
};

System_Tree_Search search(MTT_Tree_Node* root, const mtt::String& label);
System_Tree_Search search(MTT_Tree_Node* root, mtt::Array_Slice<mtt::String*>& labels);

void tree_init(dt::DrawTalk* dt);

}


#endif /* drawtalk_definitions_tree_hpp */
