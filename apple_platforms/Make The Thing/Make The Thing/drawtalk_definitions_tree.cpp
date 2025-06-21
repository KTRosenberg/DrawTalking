//
//  drawtalk_definitions_tree.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/9/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "drawtalk_definitions_tree.hpp"

#include "drawtalk.hpp"
#include "drawtalk_behavior.hpp"



namespace dt {

System_Tree_Search search(MTT_Tree_Node* root, const mtt::String& label)
{
    Behavior_Block* b = static_cast<Behavior_Block*>(root->data);
    auto result = b->label_to_node.find(label);
    System_Tree_Search search_result = {};
    if (result == b->label_to_node.end()) {
        search_result.result = nullptr;
        return search_result;
    }
    
    search_result.result = result->second;
    
    return search_result;
}


System_Tree_Search System_Tree_Search::search(const mtt::String& label)
{
    auto* ctx = dt::DrawTalk::ctx();
    if (this->result == nullptr) {
        return *this;
    }
    
    return ::dt::search(this->result, label);
}

System_Tree_Search search(MTT_Tree_Node* root, mtt::Array_Slice<mtt::String*>& labels)
{
    auto it = labels.begin_ptr();
    System_Tree_Search search_result = {};
    do {
        auto out = search(root, **it);
        if (out.result != nullptr) {
            search_result = out;
            ++it;
        } else {
            break;
        }
    } while (it != labels.end_ptr());
    
    return search_result;
}

void tree_init(dt::DrawTalk* dt)
{
    mtt::set_active_tree(&dt->sys_tree);
    
    MTT_Tree_init_with_allocator_and_callback(&dt->sys_tree, &dt->tree_node_allocation.allocator, [](MTT_Tree* tree, MTT_Tree_Node* node, void* data, uint64 type) {
        
        auto* dt = static_cast<DrawTalk*>(data);
        
        switch (type) {
        case MTT_Tree_NODE_UPDATE_TYPE_MAKE: {
            ((Behavior_Block*)node->data)->node = node;
            break;
        }
        case MTT_Tree_NODE_UPDATE_TYPE_WILL_DESTROY: {
            Block_destroy_cases(ct, (Behavior_Block*)node->data);
            node->data = NULL;
            break;
        }
        case MTT_Tree_NODE_UPDATE_TYPE_COPY: {
            break;
        }
        case MTT_Tree_NODE_UPDATE_TYPE_ATTACHED: {
            break;
        }
        }
        
    }, dt);
    
    dt->core->load_remote_text(dt->core, "templates/defaults.json");
    
    mtt::tree_build(&dt->sys_tree,
        dt::Root_Block::node("DrawTalk", {}, {
        dt::Scope_Block::node("usr_env", {}, {
            // TODO: // interface visible in the world
        }),
        dt::Scope_Block::node("en", {}, {
            //dt::Namespace_Block::node(
            dt::Namespace_Block::node("system_commands", {}, {
                dt::Phrase_Block::node("let", {
                    .instructions = {
                        {"SELF_OR_AUDIENCE", "VERB", "CREATE_OR_DESTROY", "DO_ACTION"},
                        {"SELF_OR_AUDIENCE", "VERB", "SEE_OR_GOTO", "DO_TRANSITION"},
                        {"SELF_OR_AUDIENCE", "VERB", "UNDO", "DO_UNDO"},
                        {"SELF_OR_AUDIENCE", "VERB", "UNDO", "DO_REDO"}
                    }
                }, {}),
                dt::Phrase_Block::node("let's", {
                    .instructions = {
                        {"VERB", "CREATE_OR_DESTROY", "DO_ACTION"},
                        {"VERB", "SEE_OR_GOTO", "DO_TRANSITION"},
                        {"VERB", "UNDO", "DO_UNDO"},
                        {"VERB", "UNDO", "DO_REDO"}
                    }
                }, {}),
                dt::Phrase_Block::node("CONDITIONAL", {
                    .instructions = {
                        {"SELF_OR_AUDIENCE", "VERB", "CREATE_OR_DESTROY", "DO_ACTION"},
                        {"SELF_OR_AUDIENCE", "VERB", "SEE_OR_GOTO", "DO_TRANSITION"},
                        {"SELF_OR_AUDIENCE", "VERB", "UNDO", "DO_UNDO"},
                        {"SELF_OR_AUDIENCE", "VERB", "UNDO", "DO_REDO"}
                    }
                }, {}),
                dt::Phrase_Block::node("suppose", {
                    .instructions = {
                        {"that", "suffix", "=", "SELF_OR_AUDIENCE"},
                        {"suffix", "=", "SELF_OR_AUDIENCE"},
                        {"THING", "VERB", "TENSE", "PRESENT"},
                    }
                }, {}),
                dt::Phrase_Block::node("SELF_OR_AUDIENCE", {
                    .instructions = {
                        {"VERB", "CREATE_OR_DESTROY", "DO_ACTION"},
                        {"VERB", "SEE_OR_GOTO", "DO_TRANSITION"},
                        {"VERB", "UNDO", "DO_UNDO"},
                        {"VERB", "UNDO", "DO_REDO"}
                    }
                }, {}),
                dt::Phrase_Block::node("what if", {
                    .instructions = {
                        {"SELF_OR_AUDIENCE", "VERB", "CREATE_OR_DESTROY", "DO_ACTION"},
                        {"SELF_OR_AUDIENCE", "VERB", "SEE_OR_GOTO", "DO_TRANSITION"},
                        {"SELF_OR_AUDIENCE", "VERB", "UNDO", "DO_UNDO"},
                        {"SELF_OR_AUDIENCE", "VERB", "UNDO", "DO_REDO"}
                    }
                }, {}),
                dt::Phrase_Block::node("what happens", {
                    .instructions = {{"CONDITIONAL", "SELF_OR_AUDIENCE"},
                        {"TEMPORAL",  "SELF_OR_AUDIENCE"}
                    }
                }, {}),
                dt::Phrase_Block::node("whoops", {
                    
                }, {}),
                dt::Phrase_Block::node("actually", {
                    
                }, {}),
                dt::Phrase_Block::node("now", {
                    .instructions = {{"DO_IMMEDIATE"}},
                    .requirements = {"BEGINNING"}
                }, {})
            }),
            dt::Namespace_Block::node("destruct", {}, {}),
            dt::Namespace_Block::node("explode", {}, {
                
            }),
            dt::Namespace_Block::node("annihilate", {}, {
                dt::Goto_Block::node("alias", {.label = "explode"}, {})
            }),
            // jump namespace
            dt::Namespace_Block::node("jump", {}, {
                dt::Namespace_Block::node("jump.03", {}, {
                    
                    
                    dt::Procedure_Block::node("onto", {}, {
                        dt::Procedure_Parameter_List_Block::node("params", {}, {
                            dt::Procedure_Parameter_Block::node("source", {
                                .type_name = "Source_List",
                                .requirement = dt::PARAM_REQUIRED_YES,
                                .combine_mode = dt::PARAM_COMBINE_MODE_NO
                            }, {}),
                            dt::Procedure_Parameter_Block::node("object", {
                                .type_name = "Object_List",
                                .requirement = dt::PARAM_REQUIRED_YES,
                                .combine_mode = dt::PARAM_COMBINE_MODE_NO
                            }, {}),
                            dt::Procedure_Parameter_Block::node("height", {
                                .type_name = "Spatial_Constraint_Distance",
                                .requirement = dt::PARAM_REQUIRED_NO,
                                .combine_mode = dt::PARAM_COMBINE_MODE_NO,
                            }, {}),
                            dt::Procedure_Parameter_Block::node("ceiling", {
                                .type_name = "Spatial_Constraint_Ceiling",
                                .requirement = dt::PARAM_REQUIRED_NO,
                                .combine_mode = dt::PARAM_COMBINE_MODE_NO
                            }, {}),
                        }),
                        
                        // MARK: widget begin
                        dt::Constructor_Block::node("widget_group", {
                            .arch_id = mtt::ARCHETYPE_GROUP
                        }, {
                            dt::Value_Block::node("", {
                                
                            }, {}),
                            dt::Value_Block::node("", {
                                
                            }, {}),
                            dt::Value_Block::node("", {
                                
                            }, {})
                        }),
                        
                        // choosing
                        dt::Constructor_Block::node("distance", {
                            .arch_id = mtt::ARCHETYPE_DISTANCE
                        }, {}),
                        
                        //                            dt::Constructor_Block::node("anchor_a", {
                        //                                .arch_id = mtt::ARCHETYPE_FREEHAND_SKETCH,
                        //                                .is_visible = false,
                        //                                .is_user_destructible = false,
                        //                                .is_locked = true,
                        //                            }, {
                        //                                dt::Value_Block::node("position", {
                        //                                    .kind = VALUE_BLOCK_KIND_THING_SELECTOR,
                        //                                    .identifiers = {{"params", "Source_List"}},
                        //                                    .selectors = {{"center"}}
                        //                                }, {})
                        //                            }),
                        
                        dt::Constructor_Block::node("anchor", {
                            .arch_id = mtt::ARCHETYPE_FREEHAND_SKETCH,
                            .is_visible = true,
                            .is_user_destructible = false,
                            .is_locked = true
                        }, {
                            dt::Value_Block::node("position_offset_from", {
                                .kind = VALUE_BLOCK_KIND_THING_REF,
                                .identifiers = {{"params", "source"}}
                            }, {}),
                            dt::Value_Block::node("position_offset", {
                                .kind = VALUE_BLOCK_KIND_CONSTANT,
                                .default_value = mtt::Any::from_Vector3(Vector3(0.f, -240.0f, 0.0f)),
                            }, {})
                        }),
                        
                        dt::Parent_Connection_Block::node("_", {
                            .child_label = {"anchor"},
                            .parent_label = {"widget_group"}
                        }, {}),
                        
                        dt::Connection_Block::node("_", {
                            .connection = {
                                .src_label = {"params", "source"},
                                .dst_label = {"distance"},
                                
                                .header = {
                                    "center",
                                    "source"
                                }
                            }
                        }, {}),
                        dt::Connection_Block::node("_", {
                            .connection = {
                                .src_label = {"anchor"},
                                .dst_label = {"distance"},
                                
                                .header = {
                                    "center",
                                    "destination"
                                }
                            }
                        }, {}),
                        
                        // MARK: widget end
                        
                        // MARK: main group begin
                        dt::Constructor_Block::node("main_group", {
                            .arch_id = mtt::ARCHETYPE_GROUP
                        }, {
                            dt::Value_Block::node("", {
                                
                            }, {}),
                            dt::Value_Block::node("", {
                                
                            }, {}),
                            dt::Value_Block::node("", {
                                
                            }, {})
                        }),
                        
                        dt::Jump_Block::node("jump", {}, {
                            dt::Value_Block::node("source", {
                                .kind = VALUE_BLOCK_KIND_THING_SELECTOR,
                                .identifiers = {{"params", "source"}},
                                .selectors = {"center"},
                            }, {}),
                            dt::Value_Block::node("destination", {
                                .kind = VALUE_BLOCK_KIND_THING_SELECTOR,
                                .identifiers = {{"params", "object"}},
                                .selectors   = {"top_with_offset"},
                            }, {}),
                            dt::Value_Block::node("jump_height", {
                                .kind = VALUE_BLOCK_KIND_NUMBER,
                                .identifiers   = {{"params", "height"}},
                                .default_value = mtt::Any::from_Float(240.0f),
                            }, {}),
                            // TODO: temporarily don't worry about the ceiling
                            //                                dt::Value_Block::node("ceiling", {
                            //                                    .kind = VALUE_BLOCK_KIND_THING_SELECTOR,
                            //                                    .identifiers   = {{"params", "ceiling"}},
                            //                                    .default_value = mtt::Any::from_Float(0.0f),
                            //                                    .selectors = {"bottom"},
                            //                                    .use_widget = true,
                            //                                    .widget = dt::WIDGET_ABSOLUTE_POSITION
                            //
                            //                                }, {})
                        }),
                        
                        dt::Parent_Connection_Block::node("_", {
                            .child_label = {"jump"},
                            .parent_label = {"main_group"}
                        }, {}),
                        
                        
                        dt::Constructor_Block::node("destroyer", {
                            .arch_id = mtt::ARCHETYPE_DESTROYER
                        }, {}),
                        
                        dt::Parent_Connection_Block::node("_", {
                            .child_label = {"destroyer"},
                            .parent_label = {"main_group"}
                        }, {}),
                        
                        
                        
                        // MARK: main group end
                        
                        dt::End_Block::node("end", {
                            .kind = END_KIND_OUT_PORT,
                            .expected_value = mtt::Any::from_Boolean(true),
                            .sources = {{"jump", "is_done"}},
                        }, {}),
                    }),
                    
                }), // MARK: end: jump | physical | onto
                
            }),
        }),
        dt::Scope_Block::node("de", {}, {
            // TODO: German in the future?
        })
    })
    );
}


}
