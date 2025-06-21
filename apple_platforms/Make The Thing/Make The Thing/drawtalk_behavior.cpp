//
//  drawtalk_behavior.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/18/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//


#include "drawtalk_behavior.hpp"
#include "drawtalk.hpp"

namespace dt {

void Behavior_Block_Type_Info_init(DrawTalk* dt, Behavior_Block_Type_Info* block_type_info)
{
    auto& type_info_list = block_type_info->list;
    type_info_list.reserve(dt::BUILTIN_BLOCK_TYPE_COUNT);
    for (usize i = 0; i < dt::BUILTIN_BLOCK_TYPE_COUNT; i += 1) {
        auto* info = &type_info_list.emplace_back(dt::builtin_block_type_info[i]);
        
        // FIXME: pointers will be invalidated if the list grows, need to remember to update pointers if the list grows
        
        Pool_Allocation_init(&info->allocation, dt->core->allocator, 128, info->size, 16);
    }
    
    // TODO : // custom types
    

}

void Behavior_Catalogue_init(DrawTalk* dt, Behavior_Catalogue* bc)
{
    Behavior_Block_Type_Info_init(dt, &bc->type_info);
}

Behavior_Block* Block_make(DrawTalk* dt, dt::Behavior_Catalogue* bc, usize type)
{
    if (type >= bc->type_info.list.size()) {
        return nullptr;
    }
    
    auto* type_info = &bc->type_info.list[type];
    auto* bb = (Behavior_Block*)type_info->allocation.allocator.allocate(&type_info->allocation.allocator, type_info->size);
    
    if (bb == nullptr) {
        return nullptr;
    }
    
    bb->type    = (BLOCK_TYPE)type;
    bb->derived = bb;
    bb->label = "[label]";
    
    bb->reference_id = Behavior_Block::next_available_id();
    
    return bb;
}

Behavior_Block* Block_get(MTT_Tree_Node* node)
{
    return static_cast<Behavior_Block*>(node->data);
}

void Block_destroy(DrawTalk* dt, dt::Behavior_Catalogue* bc, usize type, Behavior_Block* block)
{
    auto* type_info = &bc->type_info.list[type];
    
    type_info->allocation.allocator.deallocate(&type_info->allocation.allocator, block->derived, type_info->size);
}

void Block_destroy_cases(DrawTalk* dt, Behavior_Block* block_base)
{
    dt::Behavior_Catalogue* bc = &dt->lang_ctx.behaviors;
    
    const BLOCK_TYPE builtin_block_type = static_cast<BLOCK_TYPE>(block_base->type);
    switch (builtin_block_type) {
#define DESTROY_CASE(pre_action__, typename__) case typename__ :: block_type() : {{ pre_action__ } Block_destroy< typename__ >(dt, bc, ( typename__ * )block_base->derived); break; }
        DESTROY_CASE({}, dt::None_Block)
        DESTROY_CASE({}, dt::Root_Block)
        DESTROY_CASE({}, dt::Scope_Block)
        DESTROY_CASE({}, dt::Procedure_Block)
        DESTROY_CASE({}, dt::Procedure_Parameter_List_Block)
        DESTROY_CASE({}, dt::Procedure_Parameter_Block)
        DESTROY_CASE({}, dt::Goto_Block)
        DESTROY_CASE({}, dt::Generator_Block)
        DESTROY_CASE({}, dt::Vector_Block)
        DESTROY_CASE({}, dt::Point_Block)
        DESTROY_CASE({}, dt::Mover_Block)
        DESTROY_CASE({}, dt::Follower_Block)
        DESTROY_CASE({}, dt::When_Block)
        DESTROY_CASE({}, dt::Curve_Block)
        DESTROY_CASE({}, dt::Namespace_Block)
        DESTROY_CASE({}, dt::Option_Block)
        DESTROY_CASE({}, dt::Number_Block)
        DESTROY_CASE({}, dt::Value_Block)
        DESTROY_CASE({}, dt::Constructor_Block)
        DESTROY_CASE({}, dt::Procedure_Call_Block)
        DESTROY_CASE({}, dt::Property_Set_Block)
        DESTROY_CASE({}, dt::Expression_Block)
        DESTROY_CASE({}, dt::Connection_Block)
        DESTROY_CASE({}, dt::Jump_Block)
        DESTROY_CASE({}, dt::End_Block)
        DESTROY_CASE({}, dt::Parent_Connection_Block)
        DESTROY_CASE({}, dt::Representation_Block)
        DESTROY_CASE({}, dt::Phrase_Block)
#undef DESTROY_CASE
    default: {
        Block_destroy(dt, bc, block_base->type, block_base);
    }
    }

}

void copy(Behavior_Block* to, Behavior_Block* from)
{
    switch (from->type) {
        
    }
}

static dt::Behavior_Catalogue* active_behavior_catalogue = nullptr;

void set_active_behavior_catalogue(dt::Behavior_Catalogue* bc)
{
    active_behavior_catalogue = bc;
}


dt::Behavior_Catalogue* get_active_behavior_catalogue(void)
{
    return active_behavior_catalogue;
}

MTT_Tree_Node* mk_searchable(MTT_Tree_Node* node)
{
    //node->data
    
    //auto* dt = (DrawTalk*)tree->user_data;
    auto* behavior = (dt::Behavior_Block*)node->data;
    
    std::cout << "{ " << behavior->label << std::endl;
    for (auto* it = MTT_Tree_child_list_begin(node);
         it != MTT_Tree_child_list_end(node); MTT_Tree_child_list_advance(&it)) {
        std::cout << "id=[" << static_cast<dt::Behavior_Block*>(it->data)->reference_id << "] " << static_cast<dt::Behavior_Block*>(it->data)->label << " ";
        behavior->label_to_node.emplace(static_cast<dt::Behavior_Block*>(it->data)->label, it);
    }
    //std::cout << std::endl << "}" << std::endl;
    
    return node;
};

void Block_print(dt::Behavior_Block* block)
{
    MTT_print("Block: id=[%llu][%s]\n", block->reference_id, get_active_behavior_catalogue()->type_info.list[block->type].name.c_str());
}
void Block_print(void* block)
{
    Block_print((dt::Behavior_Block*)block);
}

///

void Program_Sequence_Node::init()
{
    
}

void del(mtt::World* world, mtt::Thing_ID id)
{
    mtt::Thing* thing = nullptr;
    if (world->Thing_try_get(id, &thing)) {
        while (thing->child_id_set.size() != 0) {
            del(world, *(thing->child_id_set.begin()));
        }
        mtt::Thing_destroy(world, id);
    }
}

void Program_Sequence_Node::deinit()
{
    
    mtt::World* world = dt::DrawTalk::ctx()->mtt;

    MTT_print("ROOT ID=%llu\n", this->id);
    
    
    
    del(world, this->id);
    //mtt::Thing_destroy_self_and_connected(world, this->id);
    
    return;
    for (auto it = this->identifiers.map.begin(); it != this->identifiers.map.end(); ++it) {
        mtt::Any& any = it->second;
        if (it->second.type == mtt::MTT_LIST) {
            void* ptr = (void*)(any.List);
            switch (any.contained_type) {
            case mtt::MTT_ANY: {
                // FIXME: need infinite nesting ability
                
                auto& actual = *static_cast<Dynamic_Array<mtt::Any>*>(ptr);
                for (usize i = 0; i < actual.size(); i += 1) {
                    if (actual[i].type == mtt::MTT_THING  &&
                        actual[i].thing_id != mtt::Thing_ID_INVALID) {
                        mtt::Thing_destroy(world, actual[i].thing_id);
                    }
                }
                delete &actual;
                
                break;
            }
            case mtt::MTT_THING: {
                auto& actual = *static_cast<Dynamic_Array<mtt::Thing_ID>*>(ptr);
                for (usize i = 0; i < actual.size(); i += 1) {
                    if (actual[i] != mtt::Thing_ID_INVALID) {
                        mtt::Thing_destroy(world, actual[i]);
                    }
                }
                
                delete &actual;
                
                break;
            }
            }
            
            continue;
        }
        if (any.type == mtt::MTT_THING &&
            any.thing_id != mtt::Thing_ID_INVALID) {
                
            mtt::Thing_destroy(dt::DrawTalk::ctx()->mtt, it->second.thing_id);
        }
    }
}

void Program_Sequence::init()
{
    MTT_Tree_init_with_allocator_and_callback(&this->tree, &dt::DrawTalk::ctx()->tree_node_allocation.allocator, [](MTT_Tree* tree, MTT_Tree_Node* node, void* data, uint64 type) {
        
        MTT_Tree_NODE_UPDATE_TYPE mode = (MTT_Tree_NODE_UPDATE_TYPE)type;
        
        switch (mode) {
        case MTT_Tree_NODE_UPDATE_TYPE_MAKE: {
            auto* prog_node = new Program_Sequence_Node();
            prog_node->init();
            prog_node->node = node;
            
            node->data = (void*)(prog_node);
            
            prog_node->id = mtt::Thing_ID_INVALID;
            
            break;
        }
        case MTT_Tree_NODE_UPDATE_TYPE_WILL_DESTROY: {
            Program_Sequence_Node* node_data = static_cast<Program_Sequence_Node*>(node->data);
            
            node_data->deinit();
            
            delete node_data;
            
            break;
        }
        case MTT_Tree_NODE_UPDATE_TYPE_ATTACHED: {
            if (node->parent != NULL && node != MTT_Tree_root(tree)) {
                
                auto* parent_program = Program_Sequence_Node::get(node->parent);
                parent_program->unfinished_child_count += 1;
            }
            break;
        }
        default: {
            ASSERT_MSG(false, "UNHANDLED!\n");
            break;
        }
        }
        
    }, (void*)this);
    
    MTT_Tree_Node* root = MTT_Tree_insert_new_root(&this->tree, NULL);
    auto* prog = Program_Sequence_Node::get(root);

    

    
    ASSERT_MSG(MTT_Tree_root(&this->tree) != NULL, "root should not be null!\n");
}


Program_Sequence_Node* Program_Sequence_Node::get(MTT_Tree_Node* node)
{
    if (node == nullptr) {
        return nullptr;
    }
    
    return static_cast<Program_Sequence_Node*>(node->data);
}

void Program_Sequence::run_begin()
{
    this->active_programs.clear();
    
    
    MTT_Tree_find_leaves(&this->tree, this->tree.root, [](MTT_Tree* tree, MTT_Tree_Node* node, void* user_data) -> void {
        
        if (node == MTT_Tree_root(tree)) {
            return;
        }
        
        Program_Sequence* progs = static_cast<Program_Sequence*>(user_data);
        
        progs->active_programs.emplace_back(static_cast<Program_Sequence_Node*>(node->data));
        
    }, (void*)this);
    
    ASSERT_MSG(this->active_programs.size() != 0, "Number of programs should not be 0!\n");
    
    for (usize i = 0; i < this->active_programs.size(); i += 1) {
        mtt::thing_group_set_active_state(dt::DrawTalk::ctx()->mtt, this->active_programs[i]->id, true);
    }
}

void Program_Sequence::run_end()
{
    for (usize i = 0; i < this->active_programs.size(); i += 1) {
        mtt::thing_group_set_active_state(dt::DrawTalk::ctx()->mtt, this->active_programs[i]->id, false);
    }
    // TODO: decide whether or not to destroy the tree?
    //MTT_Tree_destroy(&this->tree);
}
void Program_Sequence::deinit()
{
    MTT_Tree_destroy(&this->tree);
}

void System_Evaluation_Context::eval_history_append(Evaluation_Context* eval_ctx)
{
    this->eval_history.push_back(eval_ctx);
}
void Evaluation_Context::run_begin()
{
    dt::DrawTalk::ctx()->sys_eval_ctx.eval_history_append(this);
    this->programs.run_begin();
}
void Evaluation_Context::run_end()
{
    this->programs.run_end();
}

void Evaluation_Context::activate()
{
    auto* program = dt::Program_Sequence_Node::get(MTT_Tree_root(&this->programs.tree));
    if (!mtt::thing_group_set_active_state(dt::DrawTalk::ctx()->mtt, program->id, true)) {
        ASSERT_MSG(false, "Missing thing!\n");
    }
    this->is_active = true;
}
void Evaluation_Context::deactivate()
{
    auto* program = dt::Program_Sequence_Node::get(MTT_Tree_root(&this->programs.tree));
    if (!mtt::thing_group_set_active_state(dt::DrawTalk::ctx()->mtt, program->id, false)) {
        ASSERT_MSG(false, "Missing thing!\n");
    }
    this->is_active = false;
}

////

Word_Dictionary_Entry* Word_Dictionary::entry_make(void)
{
    return mem::alloc_init<Word_Dictionary_Entry>(&dt::DrawTalk::ctx()->lang_ctx.dictionary.alloc_word_entry_pool.allocator);
}

void Word_Dictionary::entry_destroy(Word_Dictionary_Entry* entry)
{
    mem::deallocate<Word_Dictionary_Entry>(&entry->dict->alloc_word_entry_pool.allocator, entry);
}


Word_Dictionary_Entry* verb_add(const mtt::String& key, const mtt::String& key_secondary)
{
    auto* ctx = DrawTalk::ctx();
    
    auto* ecs_w = &(ctx->mtt->ecs_world);
    
    auto* dict = &(ctx->lang_ctx.dictionary);
    
    
    Word_Dictionary_Entry** word_ref;
    if (mtt::map_try_get(&dict->verb_map, key, &word_ref)) {
        return *word_ref;
    }
    
    Word_Dictionary_Entry* entry = Word_Dictionary::entry_make();
    {
        entry->type = WORD_DICTIONARY_ENTRY_TYPE::VERB;
        entry->name = key;
        entry->name_secondary = key_secondary;
        entry->typename_desc = flecs::entity(*ecs_w, key.c_str());
        Word_Dictionary_Entry_verb_event_define((void*)ecs_w, entry, VERB_EVENT_BEGIN, key);
        Word_Dictionary_Entry_verb_event_define((void*)ecs_w, entry, VERB_EVENT_END, key);
        entry->template_desc = flecs::entity(*ecs_w, "NULL");
        entry->dict = dict;
    }
    mtt::map_set(&dict->verb_map, key, entry, &word_ref);
    
    (*word_ref)->verb_class = VERB_CLASS_ACTION;

    
    mtt::insert_sorted(dict->verbs, *word_ref, Word_Dictionary_Entry_Compare_Routine_increasing());

    return *word_ref;
}

Word_Dictionary_Entry* verb_lookup(const mtt::String& key, const mtt::String& key_secondary)
{
    auto* ctx = &(DrawTalk::ctx()->lang_ctx);
    
    auto result = ctx->dictionary.verb_map.find(key);
    if (result == ctx->dictionary.verb_map.end()) {
        return nullptr;
    }
    
    return result->second;
}


inline Word_Dictionary_Entry* noun_add_internal(const mtt::String& key, const mtt::String& key_secondary)
{
    auto* ctx = DrawTalk::ctx();
    
    auto* ecs_w = &(ctx->mtt->ecs_world);
    
    auto* dict = &(ctx->lang_ctx.dictionary);
    
    Word_Dictionary_Entry** word_ref;
    if (mtt::map_try_get(&dict->noun_map, key, &word_ref)) {
        return *word_ref;
    }
    
    Word_Dictionary_Entry& entry = *Word_Dictionary::entry_make();
    entry.name = key;
    entry.name_secondary = key_secondary;
    entry.type = WORD_DICTIONARY_ENTRY_TYPE::NOUN;
    
    std::string type_name =  (key);
    cstring type_name_as_cstring = type_name.c_str();
    entry.typename_desc = flecs::entity(*ecs_w, type_name_as_cstring);
    entry.dict = dict;
    //MTT_print("added typename[%s]\n", entry.typename_desc.name().c_str());
    std::string temp_name = (key);
    cstring temp_name_as_cstring = temp_name.c_str();
    //entry.template_desc = flecs::prefab(*ecs_w, temp_name_as_cstring);
    entry.template_desc = ecs_w->prefab(temp_name_as_cstring);
    //MTT_print("added template[%s]\n", entry.template_desc.name().c_str());
    entry.template_desc.add(entry.typename_desc);
    
    //noun_make_equivalent_to(&entry, &entry);
    
    mtt::map_set(&dict->noun_map, key, &entry, &word_ref);
    mtt::insert_sorted(dict->nouns, *word_ref, Word_Dictionary_Entry_Compare_Routine_increasing());
    
    return *word_ref;
}

Word_Dictionary_Entry* noun_add(const mtt::String& key, const mtt::String& key_secondary)
{
    mtt::String key_modified           = mtt::space2underscore(key);
    mtt::String key_secondary_modified = mtt::space2underscore(key_secondary);
    
    return noun_add_internal(key_modified, key_secondary_modified);
}

Word_Dictionary_Entry* noun_lookup(const mtt::String& key, const mtt::String& key_secondary)
{
    auto* ctx = &(DrawTalk::ctx()->lang_ctx);
    
    mtt::String key_modified = mtt::space2underscore(key);

    auto result = ctx->dictionary.noun_map.find(key_modified);
    if (result == ctx->dictionary.noun_map.end()) {
        return nullptr;
    }
    
    return result->second;
}



Word_Dictionary_Entry* noun_derive_from(Word_Dictionary_Entry* derived, Word_Dictionary_Entry* base)
{
    
    derived->typename_desc.add(
                                     flecs::IsA,
                                     base->typename_desc);
    
    derived->template_desc.add(
                                     flecs::IsA,
                                     base->template_desc);
    
//    MTT_print("derived: %30s, [%s]\n",            derived->typename_desc.path().c_str(), derived->typename_desc.type().str().c_str());
//    MTT_print("derived: %30s, [%s] (template)\n", derived->template_desc.path().c_str(), derived->template_desc.type().str().c_str());
//
//    MTT_print("derived: %30s, [%s]\n",            base->typename_desc.path().c_str(), base->typename_desc.type().str().c_str());
//    MTT_print("derived: %30s, [%s] (template)\n", base->template_desc.path().c_str(), base->template_desc.type().str().c_str());
    
    return derived;
}

Word_Dictionary_Entry* noun_make_equivalent_to(Word_Dictionary_Entry* equivalent, Word_Dictionary_Entry* base)
{
    auto* equiv = verb_lookup("equivalent_to");
    
    
    base->typename_desc.add(
                                     equiv->typename_desc,
                                     equivalent->typename_desc);
    
    base->template_desc.add(
                                     equiv->typename_desc,
                                     equivalent->template_desc);
    
    return equivalent;
}


void attribute_derive_from(Word_Dictionary_Entry* derived, Word_Dictionary_Entry* base)
{
    derived->typename_desc.add(
                               flecs::IsA,
                               base->typename_desc);
}

void verb_make_equivalent_to(Word_Dictionary_Entry* derived, Word_Dictionary_Entry* base)
{
    derived->equivalent_to = base;
}

Word_Dictionary_Entry* verb_root_equivalence(Word_Dictionary_Entry* entry)
{
    Word_Dictionary_Entry* parent = entry->equivalent_to;
    while (parent != nullptr) {
        entry = parent;
        parent = entry->equivalent_to;
    }
    
    return entry;
}

bool is_derived_from(mtt::Thing* thing, Word_Dictionary_Entry* entry)
{
    auto* dict = entry->dict;
    
    Visualizable_Word* inst;
    return mtt::map_try_get(&entry->things, thing->id, &inst);
}
bool is_derived_from(mtt::Thing_ID thing_id, Word_Dictionary_Entry* entry)
{
    auto* dict = entry->dict;
    
    Visualizable_Word* inst;
    return mtt::map_try_get(&entry->things, thing_id, &inst);
}

Visualizable_Word* vis_word_derive_from(mtt::Thing* thing, Word_Dictionary_Entry* entry)
{
    
//    ASSERT_MSG(mtt::is_main_thread(), "Should only access in main thread\n");
//    ASSERT_MSG(entry != nullptr, "An entry should never be null\n");
//    ASSERT_MSG(thing != nullptr, "A thing should never be null\n");
    auto* dict = entry->dict;
//    ASSERT_MSG(entry->typename_desc.is_valid(), "HUH");
    {
        auto find_entry = entry->things.find(thing->id);
        if (find_entry != entry->things.end()) {
            return &find_entry->second;
        }
    }
    
    Visualizable_Word* inst = nullptr;
    
    {
        mtt::Set_Stable<Word_Dictionary_Entry*>* words = nullptr;
        
        auto find_words = dict->thing_to_word.find(thing->id);
        if (find_words == dict->thing_to_word.end()) {
            dict->thing_to_word.insert({thing->id, mtt::Set_Stable<Word_Dictionary_Entry*>()});
            words = &dict->thing_to_word[thing->id];
        } else {
            words = &find_words->second;
            for (auto it = words->begin(); it != words->end(); ++it) {
                if ((*it)->typename_desc.has(flecs::IsA, entry->typename_desc)) {
                    DT_print("Do not add this word since it's already covered by a subtype\n");
                    return nullptr;
                }
            }
        }
        
        assert(words != nullptr);
        entry->things.insert({thing->id, Visualizable_Word()});
        inst = &entry->things[thing->id];
        
        inst->id = thing->id;
        if (words->find(entry) != words->end()) {
            DT_print("WARNING: %llu already has this dictionary entry!\n", thing->id);
        } else {
            words->insert(entry);
        }
    }
    thing->ecs_entity.add(flecs::IsA, entry->template_desc);
    
    // FIXME: remove this line after fix
    thing->ecs_entity.add(entry->typename_desc);
    //MTT_print("instance type = [%s]", thing->ecs_entity.type().str().c_str());
    //thing->attributes.
    
    
    //dt::DrawTalk::ctx()->sketch_library.add(entry, thing);
    
    return inst;
}

void vis_word_underive_from(mtt::Thing* thing, Word_Dictionary_Entry* entry)
{

    
    auto* dict = entry->dict;
    ASSERT_MSG(entry->typename_desc.is_valid(), "HUH");
    
        Visualizable_Word* inst = nullptr;
        if (!mtt::map_try_get(&entry->things, thing->id, &inst)) {
            MTT_error("%s", "WARNING, should exist!\n");
            return;
        }
    
    {
        mtt::Set_Stable<Word_Dictionary_Entry*>* words = nullptr;
        if (!mtt::map_try_get(&dict->thing_to_word, thing->id, &words)) {
            MTT_error("%s", "SHOULD EXIST!\n");
            return;
        }
        
       

        {
            auto find = words->find(entry);
            if (words->find(entry) != words->end()) {
                words->erase(find);
            }
        }
        
        
        {
            auto find = entry->things.find(thing->id);
            if (find != entry->things.end()) {
                entry->things.erase(find);
            }
        }
    }
    thing->ecs_entity.remove(flecs::IsA, entry->template_desc);
    
    // FIXME: remove this line after fix
    thing->ecs_entity.remove(entry->typename_desc);
    //MTT_print("instance type = [%s]", thing->ecs_entity.type().str().c_str());
    //thing->attributes.
    
    dt::DrawTalk::ctx()->sketch_library.remove(entry, thing);
    
    return;
}

void Thing_add_IsDoingAction(mtt::Thing* thing, Word_Dictionary_Entry* entry)
{
    auto  rel = thing->world()->ecs_world.lookup("IsDoingAction");
    ASSERT_MSG(rel.is_valid(), "should exist");
    thing->ecs_entity.add(rel, entry->typename_desc);
}

void Thing_remove_IsDoingAction(mtt::Thing* thing, Word_Dictionary_Entry* entry)
{
    auto  rel = thing->world()->ecs_world.lookup("IsDoingAction");
    ASSERT_MSG(rel.is_valid(), "should exist");
    thing->ecs_entity.remove(rel, entry->typename_desc);
}

void Thing_add_IsReceivingAction(mtt::Thing* thing, Word_Dictionary_Entry* entry)
{
    auto  rel = thing->world()->ecs_world.lookup("IsReceivingAction");
    ASSERT_MSG(rel.is_valid(), "should exist");
    thing->ecs_entity.add(rel, entry->typename_desc);
}

void Thing_remove_IsReceivingAction(mtt::Thing* thing, Word_Dictionary_Entry* entry)
{
    auto  rel = thing->world()->ecs_world.lookup("IsReceivingAction");
    ASSERT_MSG(rel.is_valid(), "should exist");
    thing->ecs_entity.remove(rel, entry->typename_desc);
}


Word_Dictionary_Entry* attribute_add(const mtt::String& key, const mtt::String& key_secondary)
{
    auto* ctx = DrawTalk::ctx();
    
    auto* ecs_w = &(ctx->mtt->ecs_world);
    
    auto* dict = &(ctx->lang_ctx.dictionary);
    
    
    Word_Dictionary_Entry** word_ref;
    if (mtt::map_try_get(&dict->attribute_map, key + NAMESPACE_SEPARATOR + key_secondary, &word_ref)) {
        return *word_ref;
    }
    
    Word_Dictionary_Entry& entry = *Word_Dictionary::entry_make();
    {
        entry.type = WORD_DICTIONARY_ENTRY_TYPE::ATTRIBUTE;
        if (key_secondary.size() == 0) {
            entry.name = ATTRIBUTE_PREFIX + NAMESPACE_SEPARATOR + key;
            entry.name_secondary = "";
            entry.typename_desc = flecs::entity(*ecs_w, entry.name.c_str());
            entry.typename_desc.add(ctx->mtt->ecs_world.lookup("attribute"));
            entry.template_desc = flecs::entity(*ecs_w, "NULL");
            entry.dict = dict;
        } else {
            entry.name = ATTRIBUTE_PREFIX + NAMESPACE_SEPARATOR + key;
            entry.name_secondary = ATTRIBUTE_PREFIX + NAMESPACE_SEPARATOR + key_secondary;
            entry.typename_desc = flecs::entity(*ecs_w, entry.name.c_str());
            entry.typename_desc.add(ctx->mtt->ecs_world.lookup("attribute"));
            entry.template_desc = flecs::entity(*ecs_w, "NULL");
            entry.dict = dict;
        }
    };
    
    mtt::map_set(&dict->attribute_map, key + NAMESPACE_SEPARATOR + key_secondary, &entry, &word_ref);
    mtt::insert_sorted(dict->attributes, *word_ref, Word_Dictionary_Entry_Compare_Routine_increasing());
    
    return *word_ref;
}
Word_Dictionary_Entry* attribute_lookup(const mtt::String& key, const mtt::String& key_secondary)
{
    auto* ctx = &(DrawTalk::ctx()->lang_ctx);
    
    auto result = ctx->dictionary.attribute_map.find(key + NAMESPACE_SEPARATOR + key_secondary);
    if (result == ctx->dictionary.attribute_map.end()) {
        return nullptr;
    }
    
    return result->second;
}


Word_Dictionary_Entry* noun_add_attribute(Word_Dictionary_Entry* word, const mtt::String& attribute)
{
    //auto* ecs_w = &(DrawTalk::ctx()->mtt->ecs_world);
    
    //cstring cstr = attribute.c_str();
    
    
    Word_Dictionary_Entry* entry = attribute_lookup(attribute);
    
    if (entry == nullptr) {
        entry = attribute_add(attribute);
    } else if (word->typename_desc.has(flecs::IsA, entry->typename_desc)) {
        return entry;
    }
    
    //word->template_desc.add(attrib);
    word->typename_desc.add(flecs::IsA, entry->typename_desc);
    //word->typename_desc.add(attrib);
    
    return entry;
}
void noun_remove_attribute(Word_Dictionary_Entry* word, const mtt::String& attribute)
{
    //auto* ecs_w = &(DrawTalk::ctx()->mtt->ecs_world);
    
    //cstring cstr = attribute.c_str();
    // TODO: re-enable
    return;
    
    Word_Dictionary_Entry* entry = attribute_lookup(attribute);
    if (entry == nullptr) {
        return;
    }
    
    if (!word->typename_desc.has(flecs::IsA, entry->typename_desc)) {
        return;
    }
    
    word->typename_desc.remove(flecs::IsA, entry->typename_desc);
}

Relation_Entry noun_add_relational_attribute(Word_Dictionary_Entry* word, const mtt::String& relation, const mtt::String& attribute, bool is_transitive)
{
    //auto* ecs_w = &(DrawTalk::ctx()->mtt->ecs_world);
    
    //cstring cstr_rel    = relation.c_str();
    //cstring cstr_attrib = attribute.c_str();
    
    auto* rel_entry = attribute_lookup(relation);//ecs_w->lookup(cstr_rel);
    if (rel_entry == nullptr) {
        rel_entry = attribute_add(relation);
        if (is_transitive) {
            rel_entry->typename_desc.add(flecs::Transitive);
        }
    }
    
    auto* attrib_entry = attribute_lookup(attribute);
    if (attrib_entry == nullptr) {
        attrib_entry = attribute_add(attribute);
    }
    
    word->template_desc.add(rel_entry->typename_desc, attrib_entry->typename_desc);
    
    Relation_Entry pair;
    pair.rel = rel_entry;
    pair.arg = attrib_entry;
    
    return pair;
}

void noun_remove_relational_attribute(Word_Dictionary_Entry* word, const mtt::String& relation, const mtt::String& attribute)
{
    //auto* ecs_w = &(DrawTalk::ctx()->mtt->ecs_world);
    
    //cstring cstr_rel    = relation.c_str();
    //cstring cstr_attrib = attribute.c_str();
        
    
    auto* rel = attribute_lookup(relation);
    if (rel == nullptr) {
        return;
    }
    
    auto* attrib = attribute_lookup(attribute);
    if (attrib == nullptr) {
        return;
    }
    
    if (!word->template_desc.has(rel->typename_desc, attrib->typename_desc)) {
        return;
    }
    word->template_desc.remove(rel->typename_desc, attrib->typename_desc);
}

void Thing_add_relation(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target)
{
    if (target == nullptr) {
        if (src->ecs_entity.has(rel->typename_desc)) {
            return;
        }
        
        auto& map = word_dict()->source_to_target_relation;
        
        auto new_entry = (Word_Dictionary::Active_Relation){
            .source = src->id,
            .target = mtt::Thing_ID_INVALID,
            .dict_entry = rel
        };
        map[src->id][rel].push_back(new_entry);
    
        src->ecs_entity.add(rel->typename_desc);
    } else {
        if (src->ecs_entity.has(rel->typename_desc, target->ecs_entity)) {
            return;
        }
        
        auto& map = word_dict()->source_to_target_relation;
        
        auto new_entry = (Word_Dictionary::Active_Relation){
            .source = src->id,
            .target = target->id,
            .dict_entry = rel
        };
        map[src->id][rel].push_back(new_entry);
        
        auto& reverse_map = word_dict()->target_to_source_relation;
        reverse_map[target->id][rel].push_back(new_entry);
        
        src->ecs_entity.add(rel->typename_desc, target->ecs_entity);
    }
}

// TODO ..



bool Thing_has_relation(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target)
{
    if (target == nullptr) {
        if (!src->ecs_entity.has(rel->typename_desc)) {
            return false;
        }
    } else {
        if (!src->ecs_entity.has(rel->typename_desc, target->ecs_entity)) {
            return false;
        }
    }
    
    return true;
}

void Thing_remove_relation(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target)
{
    if (target == nullptr) {
        if (!src->ecs_entity.has(rel->typename_desc)) {
            return;
        }
        
        auto& map = word_dict()->source_to_target_relation;
        
        auto& list = map[src->id][rel];

        bool found = false;
        for (usize i = 0; i < list.size(); i += 1) {
            if (list[i].target == mtt::Thing_ID_INVALID) {
                list.erase(list.begin() + i);
                found = true;
                break;
            }
        }
        ASSERT_MSG(found, "Should've existed!\n");
    
        src->ecs_entity.remove(rel->typename_desc);
    } else {
        if (!src->ecs_entity.has(rel->typename_desc, target->ecs_entity)) {
            return;
        }

        
        auto& map = word_dict()->source_to_target_relation;
        
        {
            auto& list = map[src->id][rel];
            mtt::Thing_ID target_id = target->id;
            bool found = false;
            for (usize i = 0; i < list.size(); i += 1) {
                if (list[i].target == target_id) {
                    list.erase(list.begin() + i);
                    found = true;
                    break;
                }
            }
            ASSERT_MSG(found, "Should've existed!\n");
        }
        
        auto& reverse_map = word_dict()->target_to_source_relation;
        
        {
            auto& list = reverse_map[target->id][rel];
            bool found = false;
            for (usize i = 0; i < list.size(); i += 1) {
                if (list[i].source == src->id) {
                    list.erase(list.begin() + i);
                    found = true;
                    break;
                }
            }
            ASSERT_MSG(found, "Should've existed!\n");
        }
    
        src->ecs_entity.remove(rel->typename_desc, target->ecs_entity);
    }
}

struct Thing_Action_Args {
    mtt::World* world = nullptr;
    mtt::Thing_ID thing_id_src = mtt::Thing_ID_INVALID;
    mtt::Thing_ID thing_id_tgt = mtt::Thing_ID_INVALID;
    flecs::entity ev;
    Word_Dictionary_Entry* rel = nullptr;
};

void Thing_add_action_event_instantaneous(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target, VERB_EVENT event_id)
{
    auto ev = Word_Dictionary_Entry_event_for_verb(rel, event_id);
    
    auto* world = mtt::world(src);
    auto& allocator = *mtt::buckets_allocator(world);
    auto* args = mem::alloc_init<Thing_Action_Args>(&allocator);
    args->rel = rel;
    
    if (target == nullptr) {
        //assert(!src->ecs_entity.has(ev));
        src->ecs_entity.add(ev);
        args->thing_id_src = mtt::thing_id(src);
        args->thing_id_tgt = mtt::Thing_ID_INVALID;
    } else {
        //assert(!src->ecs_entity.has(ev, target->ecs_entity));
        src->ecs_entity.add(ev, target->ecs_entity);
        args->thing_id_src = mtt::thing_id(src);
        args->thing_id_tgt = mtt::thing_id(target);
    }
    args->world = world;
    args->ev = ev;
    

    mtt::send_system_message_deferred_before_script_eval(&world->message_passer, mtt::MTT_NONE, src->id, args, mtt::Procedure_make([](void* state, mtt::Procedure_Input_Output* io) {
        
        Thing_Action_Args* args = static_cast<Thing_Action_Args*>(mtt::Message_contents(mtt::Message_from(io)));
        
        mtt::World* world = args->world;
        
        if (!args->ev.is_valid()) {
            mem::deinit_deallocate<Thing_Action_Args>(mtt::buckets_allocator(world), args);
            return mtt::Procedure_Return_Type();
        }
        
        
        mtt::Thing* t_src = mtt::Thing_try_get(world, args->thing_id_src);
        
//        if (args->thing_id_tgt == mtt::Thing_ID_INVALID) {
//            //t_src->ecs_entity.remove(args->ev);
//        } else {
        
        if (!mtt::Thing_ID_is_valid(args->thing_id_tgt)) {
            if (t_src != nullptr) {
                t_src->ecs_entity.remove(args->ev);
            }
        } else {
            if (t_src != nullptr) {
                mtt::Thing* t_tgt = mtt::Thing_try_get(world, args->thing_id_tgt);
                if (t_tgt != nullptr) {
                    
                    t_src->ecs_entity.remove(args->ev, t_tgt->ecs_entity);
                }
            }
        }
        
        
        // tmp allocator - does not need deallocation explicitly
        mem::deinit_deallocate<Thing_Action_Args>(mtt::buckets_allocator(world), args);
        
        
        return mtt::Procedure_Return_Type();
    }));
}

struct Thing_Attribute_Args {
    mtt::World* world = nullptr;
    mtt::Thing_ID thing_id_src = mtt::Thing_ID_INVALID;
    //mtt::Thing_ID thing_id_tgt = mtt::Thing_ID_INVALID;
    flecs::entity ev;
    Word_Dictionary_Entry* rel = nullptr;
    Word_Dictionary_Entry* attrib = nullptr;
};

void Thing_add_attribute_event_instantaneous(mtt::Thing* src, Word_Dictionary_Entry* rel, Word_Dictionary_Entry* attrib, VERB_EVENT event_id)
{
    auto ev = Word_Dictionary_Entry_event_for_verb(rel, event_id);
    
    auto* world = mtt::world(src);
    auto& allocator = *mtt::buckets_allocator(world);
    auto* args = mem::alloc_init<Thing_Attribute_Args>(&allocator);
    args->rel = rel;
    
    args->world = world;
    args->thing_id_src = mtt::thing_id(src);
    args->ev = ev;
    args->attrib = attrib;
    
    src->ecs_entity.add(ev, attrib->typename_desc);
//    src->ecs_entity.add(rel->typename_desc, attrib->typename_desc);
    
    
    
    
    mtt::send_system_message_deferred_before_script_eval(&world->message_passer, mtt::MTT_NONE, src->id, args, mtt::Procedure_make([](void* state, mtt::Procedure_Input_Output* io) {
        
        Thing_Attribute_Args* args = static_cast<Thing_Attribute_Args*>(mtt::Message_contents(mtt::Message_from(io)));
        mtt::World* world = args->world;
        
        if (!args->ev.is_valid()) {
            mem::deinit_deallocate<Thing_Attribute_Args>(mtt::buckets_allocator(world), args);
            return mtt::Procedure_Return_Type();
        }
        
        
        mtt::Thing* t_src = mtt::Thing_try_get(world, args->thing_id_src);

        
        if (t_src != nullptr) {
            t_src->ecs_entity.remove(args->ev, args->attrib->typename_desc);
            //t_src->ecs_entity.remove(args->rel->typename_desc, args->attrib->typename_desc);
        }
        
        mem::deinit_deallocate<Thing_Attribute_Args>(mtt::buckets_allocator(world), args);
        
        
        return mtt::Procedure_Return_Type();
    }));
    
}

void Thing_add_action(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target)
{
    Thing_add_relation(src, rel, target);
    Thing_add_action_event_instantaneous(src, rel, target, VERB_EVENT_BEGIN);
}
void Thing_remove_action(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target)
{
    Thing_remove_relation(src, rel, target);
    Thing_add_action_event_instantaneous(src, rel, target, VERB_EVENT_END);
}


void Thing_add_action(mtt::Thing* src, Word_Dictionary_Entry* rel)
{
    Thing_add_relation(src, rel, nullptr);
    Thing_add_action_event_instantaneous(src, rel, nullptr, VERB_EVENT_BEGIN);
}
void Thing_remove_action(mtt::Thing* src, Word_Dictionary_Entry* rel)
{
    Thing_remove_relation(src, rel, nullptr);
    Thing_add_action_event_instantaneous(src, rel, nullptr, VERB_EVENT_END);
}


//void Thing_add_attribute(mtt::Thing* src, const mtt::String& A, const mtt::String& B, mtt::Any data);
//void Thing_add_attribute(mtt::Thing* src, Word_Dictionary_Entry* A, Word_Dictionary_Entry* B, mtt::Any data);
//mtt::Any* Thing_get_attribute_value(mtt::Thing* src, Word_Dictionary_Entry* relation, Word_Dictionary_Entry* attribute);
//mtt::Any* Thing_get_attribute_value(mtt::Thing* src, const mtt::String& relation, const mtt::String& attribute);
//void Thing_remove_attribute_property(mtt::Thing* src, Word_Dictionary_Entry* relation, Word_Dictionary_Entry* attribute);
//void Thing_remove_attribute_property(mtt::Thing* src, const mtt::String& relation, const mtt::String& attribute);

void Thing_add_attribute(mtt::Thing* src, Word_Dictionary_Entry* attrib, mtt::Any data)
{
    auto& record = attrib->dict->thing_to_own_attribute[src->id];
    
    auto attrib_find = record.find(attrib);
    if (attrib_find == record.end()) {
        if (!attrib->on_attribute_add(attrib, src)) {
            return;
        }
        record.insert(attrib);
        src->ecs_entity.add(flecs::IsA, attrib->typename_desc);
        src->ecs_entity.set<mtt::Any>(attrib->typename_desc, data);
    }
    
}



bool Thing_has_attribute(mtt::Thing* src, Word_Dictionary_Entry* attrib)
{
    auto& record = attrib->dict->thing_to_own_attribute[src->id];
    
    auto attrib_find = record.find(attrib);
    if (attrib_find == record.end()) {
        return false;
    }
    
    return true;
}

void Thing_remove_attribute(mtt::Thing* src, Word_Dictionary_Entry* attrib)
{
    auto& record = attrib->dict->thing_to_own_attribute[src->id];
    
    auto attrib_find = record.find(attrib);
    attrib->on_attribute_remove(attrib, src);
    if (attrib_find != record.end()) {
        
        src->ecs_entity.remove<mtt::Any>(attrib->typename_desc);
        src->ecs_entity.remove(flecs::IsA, attrib->typename_desc);
        auto prop_find = attrib->properties.find(src->id);
        if (prop_find != attrib->properties.end()) {
            for (dt::Word_Dictionary_Entry* prop_entry : prop_find->second) {
                src->ecs_entity.remove(attrib->typename_desc, prop_entry->typename_desc);
            }
            
            attrib->properties.erase(prop_find);
        }
        
        record.erase(attrib_find);
    }
}

void Thing_add_attribute(mtt::Thing* src, Word_Dictionary_Entry* A, Word_Dictionary_Entry* B, mtt::Any data)
{
    src->ecs_entity.add(flecs::IsA, A->typename_desc);
    src->ecs_entity.add(A->typename_desc, B->typename_desc);
    src->ecs_entity.set<mtt::Any>(A->typename_desc, data);
    
    auto& record = A->dict->thing_to_own_attribute[src->id];
    auto find = record.find(A);
    if (find == record.end()) {
        record.insert(A);
    }
    
    auto p_record_find = A->properties.find(src->id);
    if (p_record_find == A->properties.end()) {
        A->properties[src->id].insert(B);
    } else {
        auto& p_record = p_record_find->second;
        // TODO: support more than one
        if (p_record.size() > 0) {
//            src->ecs_entity.remove(A->typename_desc, (*(p_record.begin()))->typename_desc);
//            p_record.clear();
        } else {
            p_record.insert(B);
        }
    }
}
void Thing_add_attribute(mtt::Thing* src, const mtt::String& A, const mtt::String& B, mtt::Any data)
{
    auto* entry_A = attribute_add(A);
    auto* entry_B = attribute_add(B);
    
    Thing_add_attribute(src, entry_A, entry_B, data);
}

mtt::Any* Thing_get_attribute_value(mtt::Thing* src, Word_Dictionary_Entry* A)
{
    return src->ecs_entity.get_mut<mtt::Any>(A->typename_desc);
}

mtt::Any* Thing_get_attribute_value(mtt::Thing* src, const mtt::String& key, const mtt::String& key_secondary)
{
    auto* entry = attribute_lookup(key, key_secondary);
    if (entry == nullptr) {
        return nullptr;
    }
    
    return Thing_get_attribute_value(src, entry);
}



bool Thing_remove_attribute_property(mtt::Thing* src, Word_Dictionary_Entry* A, Word_Dictionary_Entry* B)
{
    auto& record = A->dict->thing_to_own_attribute[src->id];
        
    auto attrib_find = record.find(A);
    
    if (attrib_find != record.end()) {
        auto& p_record = A->properties[src->id];
        // TODO: support more than one
        if (p_record.size() > 0) {
            auto* other = (*(p_record.begin()));
            if (other == B) {
                src->ecs_entity.remove(A->typename_desc, (*(p_record.begin()))->typename_desc);
                src->ecs_entity.remove<mtt::Any>(A->typename_desc);
                p_record.clear();
                
                return true;
            }
        }
    }
    
    return false;
}

bool Thing_remove_attribute_property(mtt::Thing* src, const mtt::String& A, const mtt::String& B)
{
    auto* entry_A = attribute_lookup(A);
    if (entry_A == nullptr) {
        return false;
    }
    auto* entry_B = attribute_lookup(B);
    if (entry_B == nullptr) {
        return false;
    }
    
    return Thing_remove_attribute_property(src, entry_A, entry_B);
}

void Thing_remove_all_own_attributes(mtt::Thing* thing)
{
    auto* dict = &dt::DrawTalk::ctx()->lang_ctx.dictionary;

    auto find = dict->thing_to_own_attribute.find(thing->id);
    if (find == dict->thing_to_own_attribute.end()) {
        return;
    }
    
    auto& attribs = find->second;
    while (!attribs.empty()) {
        Thing_remove_attribute(thing, (*attribs.begin()));
    }
}

mtt::Set_Stable<Word_Dictionary_Entry*>* Thing_get_own_attributes(mtt::Thing* src)
{
    auto* dict = &dt::DrawTalk::ctx()->lang_ctx.dictionary;
    return &dict->thing_to_own_attribute[src->id];
}

mtt::Any* attribute_value(mtt::Thing* thing, Word_Dictionary_Entry* entry)
{
    return thing->ecs_entity.get_mut<mtt::Any>(entry->typename_desc);
}

void Thing_copy_own_attributes(mtt::Thing* src, mtt::Thing* dst)
{
    auto* dict = &dt::DrawTalk::ctx()->lang_ctx.dictionary;
    auto attrib_it = dict->thing_to_own_attribute.find(src->id);
    if (attrib_it == dict->thing_to_own_attribute.end()) {
        return;
    }
    
    auto& attribs = attrib_it->second;
    
    for (const auto& entry : attribs) {
        auto* val = src->ecs_entity.get<mtt::Any>(entry->typename_desc);
        auto properties_for_src = entry->properties.find(src->id);
        if (properties_for_src != entry->properties.end()) {
            for (const auto& prop : properties_for_src->second) {
                Thing_add_attribute(dst, entry, prop, *val);
            }
        } else {
            Thing_add_attribute(dst, entry, *val);
        }
    }
}

void Word_Dictionary_init(Word_Dictionary* dict)
{
    mem::Pool_Allocation_init(&dict->alloc_word_entry_pool, DrawTalk::ctx()->mtt->allocator, 1024, sizeof(Word_Dictionary_Entry), 16);
}


Word_Dictionary* word_dict(void)
{
    return &(DrawTalk::ctx()->lang_ctx.dictionary);
}

DT_Rule_Query* DT_Rule_Clause_make(mtt::World* world, mtt::String& rule_string)
{
    flecs::world& ecs_world = world->ecs_world;
    cstring rule_cstring = rule_string.c_str();
    ecs_rule_t* rule = mtt_ecs_rule_new(ecs_world.c_ptr(), rule_cstring);
    if (rule == NULL) {
        return NULL;
    }
    
    DT_Rule_Query& rc = *mem::alloc_init<DT_Rule_Query>(&world->allocator);
    rc.rule_ptr = rule;
    rc.rule_string = rule_string;
    rc.world = world;
    rc.is_owned_memory = true;
    rc.is_init = true;
    
    return &rc;
}

DT_Rule_Query* DT_Rule_Query_make(mtt::World* world, mtt::String& rule_string, DT_Rule_Query* rc)
{
    flecs::world& ecs_world = world->ecs_world;
    cstring rule_cstring = rule_string.c_str();
    ecs_rule_t* rule = mtt_ecs_rule_new(ecs_world.c_ptr(), rule_cstring);
    if (rule == NULL) {
        return NULL;
    }
    
    
    rc->rule_ptr = rule;
    rc->rule_string = rule_string;
    rc->world = world;
    rc->is_owned_memory = false;
    rc->is_init = true;
    
    return rc;
}

DT_Rule_Query* DT_Rule_Clause_make_from_builder(mtt::World* world, DT_Rule_Query_Builder& builder)
{
    mtt::String str = builder.to_string();
    return DT_Rule_Clause_make(world, str);
}

DT_Rule_Query* DT_Rule_Clause_make_from_builder(mtt::World* world, DT_Rule_Query_Builder& builder, DT_Rule_Query* rc)
{
    mtt::String str = builder.to_string();
    return DT_Rule_Query_make(world, str, rc);
}
void DT_Rule_Clause_destroy(DT_Rule_Query* clause)
{
    if (!clause->is_init || clause->rule_ptr == nullptr) {
        return;
    }
    
//    mtt::job_dispatch_to_main((void*)clause->rule_ptr, [](void* j_ctx) {
//        ecs_rule_t* rule_ptr = (ecs_rule_t*)j_ctx;
//        mtt_ecs_rule_free(rule_ptr);
//    });
    
    if (clause->is_original) {
        // TODO: fix leak
        //mtt_ecs_rule_free(clause->rule_ptr);
        if (clause->is_owned_memory) {
            mem::deallocate<DT_Rule_Query>(&clause->world->allocator, clause);
        }
    }
    clause->rule_ptr = nullptr;
    clause->is_init = false;
    
}

void init_rules(std::vector<Rule_Test>& rule_list)
{
    for (auto it = rule_list.begin(); it != rule_list.end(); ++it) {
        it->init();
    }
}

void print_rule_results_raw(mtt::World* world, ecs_rule_t* rule)
{
    ecs_world_t* ecs_world = world->ecs_world.c_ptr();
    ecs_iter_t it = ecs_rule_iter(ecs_world, rule);
    MTT_print("%s", "{\n");
    while (ecs_rule_next(&it)) {
        if (ecs_rule_var_count(rule) == 0) {
            continue;
        }
        
        bool something= false;
        
        for (int i = 0; i < ecs_rule_var_count(rule); i ++) {
            if (!ecs_rule_var_is_entity(rule, i)) {
                continue; // table variables
            }
            const char *var_name = ecs_rule_var_name(rule, i);
            if (var_name[0] == MTT_Q_VAR_PREFIX_CH) { // anonymous variables generated by query parser
                continue;
            }
            
            ecs_entity_t var_result = ecs_iter_get_var(&it, i);
            auto ent_cpp = flecs::entity(it.world, var_result);
            if (ent_cpp.has(flecs::Prefab)) {
                continue;
            }
            
            something = true;
            break;
        }
        if (!something) {
            continue;
        }
        MTT_print("%s", "\t{\n");
        for (int i = 0; i < ecs_rule_var_count(rule); i ++) {
            if (!ecs_rule_var_is_entity(rule, i)) {
                continue; // table variables
            }
            const char *var_name = ecs_rule_var_name(rule, i);
            if (var_name[0] == MTT_Q_VAR_PREFIX_CH) { // anonymous variables generated by query parser
                continue;
            }
            
            ecs_entity_t var_result = ecs_iter_get_var(&it, i);
            auto ent_cpp = flecs::entity(it.world, var_result);
            if (ent_cpp.has(flecs::Prefab)) {
                continue;
            }
            
            MTT_print("\t\t%s: [%s]\n", var_name, ent_cpp.path().c_str());
            
            if (ent_cpp.has<mtt::Thing_Info>()) {
                mtt::Thing* thing = nullptr;
                world->Thing_get(ent_cpp.get<mtt::Thing_Info>()->thing_id, &thing);
                MTT_print("\t\tname: [%s]\n", MTT_string_ref_to_cstring(thing->label));
            }
        }
        MTT_print("%s", "\t}\n");
    }
    MTT_print("%s", "}\n");
}
void print_rule_results(mtt::World* world, std::vector<Rule_Test>& rule_list)
{
    ecs_world_t* ecs_world = world->ecs_world.c_ptr();
    for (auto rit = rule_list.begin(); rit != rule_list.end(); ++rit) {
        if (!rit->is_valid) {
            continue;
        }
        
        ecs_iter_t it = ecs_rule_iter(ecs_world, rit->rule);
        MTT_print("%s {\n", rit->rule_str.c_str());
        MTT_print("\t msg: [%s]\n", rit->msg.c_str());
        while (ecs_rule_next(&it)) {
            if (ecs_rule_var_count(rit->rule) == 0) {
                continue;
            }
            
            bool something= false;
            
            for (int i = 0; i < ecs_rule_var_count(rit->rule); i ++) {
                if (!ecs_rule_var_is_entity(rit->rule, i)) {
                    continue; // table variables
                }
                const char *var_name = ecs_rule_var_name(rit->rule, i);
                if (var_name[0] == MTT_Q_VAR_PREFIX_CH) { // anonymous variables generated by query parser
                    continue;
                }
                
                ecs_entity_t var_result = ecs_iter_get_var(&it, i);
                auto ent_cpp = flecs::entity(it.world, var_result);
                if (ent_cpp.has(flecs::Prefab)) {
                    continue;
                }
                
                something = true;
                break;
            }
            if (!something) {
                continue;
            }
            MTT_print("%s", "\t{\n");
            for (int i = 0; i < ecs_rule_var_count(rit->rule); i ++) {
                if (!ecs_rule_var_is_entity(rit->rule, i)) {
                    continue; // table variables
                }
                const char *var_name = ecs_rule_var_name(rit->rule, i);
                if (var_name[0] == MTT_Q_VAR_PREFIX_CH) { // anonymous variables generated by query parser
                    continue;
                }
                
                ecs_entity_t var_result = ecs_iter_get_var(&it, i);
                auto ent_cpp = flecs::entity(it.world, var_result);
                if (ent_cpp.has(flecs::Prefab)) {
                    continue;
                }
                
                MTT_print("\t\t%s: [%s]\n", var_name, ent_cpp.path().c_str());
                
                if (ent_cpp.has<mtt::Thing_Info>()) {
                    mtt::Thing* thing = nullptr;
                    world->Thing_get(ent_cpp.get<mtt::Thing_Info>()->thing_id, &thing);
                    MTT_print("\t\tname: [%s]\n", MTT_string_ref_to_cstring(thing->label));
                }
            }
            MTT_print("%s", "\t}\n");
        }
        MTT_print("%s", "}\n");
        
    }
    for (auto rit = rule_list.begin(); rit != rule_list.end(); ++rit) {
        if (rit->is_valid) {
            mtt_ecs_rule_free(rit->rule);
            rit->rule = NULL;
            rit->is_valid = false;
        }
    }
}

void print_rule_results_no_free(mtt::World* world, std::vector<Rule_Test>& rule_list)
{
    ecs_world_t* ecs_world = world->ecs_world.c_ptr();
    for (auto rit = rule_list.begin(); rit != rule_list.end(); ++rit) {
        if (!rit->is_valid) {
            continue;
        }
        
        ecs_iter_t it = ecs_rule_iter(ecs_world, rit->rule);
        MTT_print("%s {\n", rit->rule_str.c_str());
        MTT_print("\t msg: [%s]\n", rit->msg.c_str());
        while (ecs_rule_next(&it)) {
            if (ecs_rule_var_count(rit->rule) == 0) {
                continue;
            }
            
            bool something= false;
            
            for (int i = 0; i < ecs_rule_var_count(rit->rule); i ++) {
                if (!ecs_rule_var_is_entity(rit->rule, i)) {
                    continue; // table variables
                }
                const char *var_name = ecs_rule_var_name(rit->rule, i);
                if (var_name[0] == MTT_Q_VAR_PREFIX_CH) { // anonymous variables generated by query parser
                    continue;
                }
                
                ecs_entity_t var_result = ecs_iter_get_var(&it, i);
                auto ent_cpp = flecs::entity(it.world, var_result);
                if (ent_cpp.has(flecs::Prefab)) {
                    continue;
                }
                
                something = true;
                break;
            }
            if (!something) {
                continue;
            }
            MTT_print("%s", "\t{\n");
            for (int i = 0; i < ecs_rule_var_count(rit->rule); i ++) {
                if (!ecs_rule_var_is_entity(rit->rule, i)) {
                    continue; // table variables
                }
                const char *var_name = ecs_rule_var_name(rit->rule, i);
                if (var_name[0] == MTT_Q_VAR_PREFIX_CH) { // anonymous variables generated by query parser
                    continue;
                }
                
                ecs_entity_t var_result = ecs_iter_get_var(&it, i);
                auto ent_cpp = flecs::entity(it.world, var_result);
                if (ent_cpp.has(flecs::Prefab)) {
                    continue;
                }
                
                MTT_print("\t\t%s: [%s]\n", var_name, ent_cpp.path().c_str());
                
                if (ent_cpp.has<mtt::Thing_Info>()) {
                    mtt::Thing* thing = nullptr;
                    world->Thing_get(ent_cpp.get<mtt::Thing_Info>()->thing_id, &thing);
                    MTT_print("\t\tname: [%s]\n", MTT_string_ref_to_cstring(thing->label));
                }
            }
            MTT_print("%s", "\t}\n");
        }
        MTT_print("%s", "}\n");
        
    }
}

//enum struct ROLE {
//    AGENT = (1 << 0),
//    PATIENT = (1 << 1),
//    DIRECT_OBJECT = (1 << 2),
//    INDIRECT_OBJECT_OR_OBJECT_OF_PREPOSITION = (1 << 3),
//
//    TEMPORAL = (1 << 4),
//
//    FROM_SOURCE = (1 << 5),
//    TO_DESTINATION = (1 << 6),
//
//    SPATIAL_RELATION = (1 << 7),
//};


const mtt::Map<mtt::String, Speech_Property::ROLE> Speech_Property::STRING_TO_ROLE = {
    {"AGENT", Speech_Property::ROLE::AGENT},
    {"PATIENT", Speech_Property::ROLE::PATIENT},
    {"DIRECT_OBJECT", Speech_Property::ROLE::DIRECT_OBJECT},
    {"INDIRECT_OBJECT_OR_OBJECT_OF_PREPOSITION", Speech_Property::ROLE::INDIRECT_OBJECT_OR_OBJECT_OF_PREPOSITION},
    {"TEMPORAL", Speech_Property::ROLE::TEMPORAL},
    {"FROM_SOURCE", Speech_Property::ROLE::FROM_SOURCE},
    {"TO_DESTINATION", Speech_Property::ROLE::TO_DESTINATION},
    {"SPATIAL_RELATION", Speech_Property::ROLE::SPATIAL_RELATION},
};
mtt::Map<uint64, mtt::String> Speech_Property::ROLE_TO_STRING = {
    {(uint64)Speech_Property::ROLE::AGENT,         "AGENT"},
    {(uint64)Speech_Property::ROLE::PATIENT,       "PATIENT"},
    {(uint64)Speech_Property::ROLE::DIRECT_OBJECT, "DIRECT_OBJECT"},
    {(uint64)Speech_Property::ROLE::INDIRECT_OBJECT_OR_OBJECT_OF_PREPOSITION, "INDIRECT_OBJECT_OR_OBJECT_OF_PREPOSITION"},
    {(uint64)Speech_Property::ROLE::TEMPORAL,          "TEMPORAL"},
    {(uint64)Speech_Property::ROLE::FROM_SOURCE,       "FROM_SOURCE"},
    {(uint64)Speech_Property::ROLE::TO_DESTINATION,    "TO_DESTINATION"},
    {(uint64)Speech_Property::ROLE::SPATIAL_RELATION,  "SPATIAL_RELATION"},
};


const mtt::Map<mtt::String, Speech_Property::ATTRIBUTE_FLAG> Speech_Property::STRING_TO_ATTRIBUTE_FLAG = {
    {"UNKNOWN", Speech_Property::ATTRIBUTE_FLAG::UNKNOWN},
    {"COLOR", Speech_Property::ATTRIBUTE_FLAG::COLOR},
    {"SPEED", Speech_Property::ATTRIBUTE_FLAG::SPEED},
    {"MAGNITUDE_OR_VALUE", Speech_Property::ATTRIBUTE_FLAG::MAGNITUDE_OR_VALUE},
    {"DIRECTION", Speech_Property::ATTRIBUTE_FLAG::DIRECTION},
    {"TIMING", Speech_Property::ATTRIBUTE_FLAG::TIMING},
    {"QUANTITY", Speech_Property::ATTRIBUTE_FLAG::QUANTITY},
    {"RANDOMNESS", Speech_Property::ATTRIBUTE_FLAG::RANDOMNESS},
    {"PROPERTY_OF", Speech_Property::ATTRIBUTE_FLAG::PROPERTY_OF},
    {"END_CONDITION", Speech_Property::ATTRIBUTE_FLAG::END_CONDITION},
    {"SINGULAR_OR_PLURAL", Speech_Property::ATTRIBUTE_FLAG::SINGULAR_OR_PLURAL},
};

mtt::Map<uint64, mtt::String> Speech_Property::ATTRIBUTE_FLAG_TO_STRING = {
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::UNKNOWN, "UNKNOWN"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::COLOR, "COLOR"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::SPEED, "SPEED"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::MAGNITUDE_OR_VALUE, "MAGNITUDE_OR_VALUE"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::DIRECTION, "DIRECTION"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::TIMING, "TIMING"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::QUANTITY, "QUANTITY"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::RANDOMNESS, "RANDOMNESS"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::PROPERTY_OF, "PROPERTY_OF"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::END_CONDITION, "END_CONDITION"},
    {(uint64)Speech_Property::ATTRIBUTE_FLAG::SINGULAR_OR_PLURAL, "SINGULAR_OR_PLURAL"},
};

mtt::String Speech_Instructions::indent = "";


const mtt::Map<mtt::String, Speech_Property::TYPE_FLAG> Speech_Property::STRING_TO_TYPE_FLAG = {
    {"UNKNOWN", Speech_Property::TYPE_FLAG::UNKNOWN},
    {"NOUN",    Speech_Property::TYPE_FLAG::NOUN},
    {"VERB",    Speech_Property::TYPE_FLAG::VERB},
    {"RULE",    Speech_Property::TYPE_FLAG::RULE},
    {"SEQUENCE", Speech_Property::TYPE_FLAG::SEQUENCE},
    {"SELECTION", Speech_Property::TYPE_FLAG::SELECTION},
    {"NUMBER",     Speech_Property::TYPE_FLAG::NUMBER},
    {"VALUE",      Speech_Property::TYPE_FLAG::VALUE},
    {"TRIGGER", Speech_Property::TYPE_FLAG::TRIGGER},
    {"ATTRIBUTE", Speech_Property::TYPE_FLAG::ATTRIBUTE},
    {"PRONOUN", Speech_Property::TYPE_FLAG::PRONOUN},
};

mtt::Map<uint64, mtt::String> Speech_Property::TYPE_FLAG_TO_STRING = {
    {(uint64)Speech_Property::TYPE_FLAG::UNKNOWN,   "UNKNOWN"},
    {(uint64)Speech_Property::TYPE_FLAG::NOUN,      "NOUN"},
    {(uint64)Speech_Property::TYPE_FLAG::VERB,      "VERB"},
    {(uint64)Speech_Property::TYPE_FLAG::RULE,      "RULE"},
    {(uint64)Speech_Property::TYPE_FLAG::SEQUENCE,  "SEQUENCE"},
    {(uint64)Speech_Property::TYPE_FLAG::SELECTION, "SELECTION"},
    {(uint64)Speech_Property::TYPE_FLAG::NUMBER,    "NUMBER"},
    {(uint64)Speech_Property::TYPE_FLAG::VALUE,     "VALUE"},
    {(uint64)Speech_Property::TYPE_FLAG::TRIGGER,   "TRIGGER"},
    {(uint64)Speech_Property::TYPE_FLAG::ATTRIBUTE, "ATTRIBUTE"},
    {(uint64)Speech_Property::TYPE_FLAG::PRONOUN,   "PRONOUN"},
    
};





const mtt::Map<mtt::String, Speech_Property::VALUE_KIND> Speech_Property::STRING_TO_VALUE_KIND = {
    {"NONE",              Speech_Property::VALUE_KIND::NONE},
    {"THING_TYPE",        Speech_Property::VALUE_KIND::THING_TYPE},
    {"THING_INSTANCE",    Speech_Property::VALUE_KIND::THING_INSTANCE},
    {"PROPERTY_OF_THING", Speech_Property::VALUE_KIND::PROPERTY_OF_THING},
    {"FUNCTION",       Speech_Property::VALUE_KIND::FUNCTION},
    {"TEXT",              Speech_Property::VALUE_KIND::TEXT},
    {"NUMERIC",           Speech_Property::VALUE_KIND::NUMERIC},
    {"FLAG",           Speech_Property::VALUE_KIND::FLAG},
    {"VECTOR",           Speech_Property::VALUE_KIND::VECTOR},
};


mtt::Map<uint64, mtt::String> VALUE_KIND_TO_STRING = {
    {(uint64)Speech_Property::VALUE_KIND::NONE, "NONE"},
    {(uint64)Speech_Property::VALUE_KIND::THING_TYPE, "THING_TYPE"},
    {(uint64)Speech_Property::VALUE_KIND::THING_INSTANCE, "THING_INSTANCE"},
    {(uint64)Speech_Property::VALUE_KIND::PROPERTY_OF_THING, "PROPERTY_OF_THING"},
    {(uint64)Speech_Property::VALUE_KIND::FUNCTION, "FUNCTION"},
    {(uint64)Speech_Property::VALUE_KIND::TEXT,    "TEXT"},
    {(uint64)Speech_Property::VALUE_KIND::NUMERIC, "NUMERIC"},
    {(uint64)Speech_Property::VALUE_KIND::FLAG,    "FLAG"},
    {(uint64)Speech_Property::VALUE_KIND::VECTOR,    "VECTOR"},
    
};

const mtt::Map<mtt::String, Speech_Property::TIME_UNIT> Speech_Property::STRING_TO_TIME_UNIT = {
    {"second",        Speech_Property::TIME_UNIT::SECOND},
    {"minute",           Speech_Property::TIME_UNIT::MINUTE},
    {"hour",            Speech_Property::TIME_UNIT::HOUR},
    {"day",              Speech_Property::TIME_UNIT::DAY},
    {"week",              Speech_Property::TIME_UNIT::WEEK},
    {"month",           Speech_Property::TIME_UNIT::MONTH},
    {"year",           Speech_Property::TIME_UNIT::YEAR},
    {"decade",           Speech_Property::TIME_UNIT::DECADE},
    {"century",           Speech_Property::TIME_UNIT::CENTURY},
};

const mtt::Map<mtt::String, Speech_Property::TIME_EXPRESSION> Speech_Property::STRING_TO_TIME_EXPRESSION = {
    {"dawn", Speech_Property::TIME_EXPRESSION::DAWN},
    {"morning", Speech_Property::TIME_EXPRESSION::MORNING},
    {"noon", Speech_Property::TIME_EXPRESSION::NOON },
    {"afternoon", Speech_Property::TIME_EXPRESSION::AFTERNOON },
    {"evening", Speech_Property::TIME_EXPRESSION::EVENING},
    {"dusk", Speech_Property::TIME_EXPRESSION::DUSK},
    {"night", Speech_Property::TIME_EXPRESSION::NIGHT},
    {"nightfall", Speech_Property::TIME_EXPRESSION::NIGHTFALL},
    {"midnight", Speech_Property::TIME_EXPRESSION::MIDNIGHT},
    {"o'clock", Speech_Property::TIME_EXPRESSION::O_CLOCK},
    {"AM", Speech_Property::TIME_EXPRESSION::AM},
    {"PM", Speech_Property::TIME_EXPRESSION::PM},
};

const mtt::Map<mtt::String, Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG> Speech_Property::STRING_TO_DEFAULT_TIME_FREQUENCY_FLAG = {
    {"NEVER",  Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG::NEVER},
    {"RARELY", Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG::RARELY},
    {"REGULARLY", Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG::REGULARLY},
    {"OFTEN", Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG::OFTEN},
    {"ALWAYS", Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG::ALWAYS},
    {"SOMETIMES", Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG::SOMETIMES},
    {"FREQENTLY", Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG::FREQUENTLY},
    {"EVERY_FEW", Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG::EVERY_FEW},
    {"OCCASIONALLY", Speech_Property::DEFAULT_TIME_FREQUENCY_FLAG::OCCASIONALLY},
};

const mtt::Map<mtt::String, Speech_Property::SPATIAL_RELATION> Speech_Property::STRING_TO_SPATIAL_RELATION = {
    {"ABOVE", Speech_Property::SPATIAL_RELATION::ABOVE},
    {"BELOW", Speech_Property::SPATIAL_RELATION::BELOW},
    {"NEXT_TO_LEFT", Speech_Property::SPATIAL_RELATION::NEXT_TO_LEFT},
    {"NEXT_TO_RIGHT", Speech_Property::SPATIAL_RELATION::NEXT_TO_RIGHT},
    {"NEXT_TO_ANGLE", Speech_Property::SPATIAL_RELATION::NEXT_TO_ANGLE},
    {"OFF_OF", Speech_Property::SPATIAL_RELATION::OFF_OF},
    {"AROUND", Speech_Property::SPATIAL_RELATION::AROUND},
    {"BESIDE_LEFT", Speech_Property::SPATIAL_RELATION::BESIDE_LEFT},
    {"BESIDE_RIGHT", Speech_Property::SPATIAL_RELATION::BESIDE_RIGHT},
    {"ATOP", Speech_Property::SPATIAL_RELATION::ATOP},
    {"ON_TOP_OF", Speech_Property::SPATIAL_RELATION::ON_TOP_OF},
};

usize Speech_Instructions::next_sequence_id = 0;

Speech_Instructions* Speech_Instruction_Evaluation::current_instruction = nullptr;
mem::Allocator* Speech_Instruction_Evaluation::allocator(void)
{
    return &dt::DrawTalk::ctx()->instructions.allocator;
}


inline bool flag_contains(Speech_Property::TYPE_FLAG in_flag, Speech_Property::TYPE_FLAG flag)
{
    return ((uint64)in_flag | (uint64)flag) != 0llu;
}

void Speech_Property::print_list(std::vector<Value>& list)
{
    DT_print("LIST\n");
    DT_scope_open();
    for (auto v_it = list.begin(); v_it != list.end(); ++v_it) {
        if (v_it->kind_string == "THING_TYPE") {
            DT_print("THING_TYPE=[%s],\n", v_it->text.c_str());
        } else if (v_it->kind_string == "THING_INSTANCE") {
            DT_print("THING_INSTANCE=[%llu],\n", v_it->thing);
        } else if (v_it->kind_string == "PROPERTY_OF_THING") {
            DT_print("PROPERTY_OF_THING=[%s],\n", v_it->text.c_str());
        } else if (v_it->kind_string == "FUNCTION") {
            DT_print("FUNCTION=[%s],\n", v_it->text.c_str());
        } else if (v_it->kind_string == "TEXT") {
            DT_print("TEXT=[%s],\n", v_it->text.c_str());
        } else if (v_it->kind_string == "NUMERIC") {
            DT_print("NUMERIC=[%f],\n", v_it->numeric);
        } else if (v_it->kind_string == "FLAG") {
            DT_print("FLAG=[%s],\n", bool_str(v_it->flag));
        } else if (v_it->kind_string == "VECTOR") {
            DT_print("VECTOR=[<%f,%f,%f,%f>]", v_it->vector[0], v_it->vector[1], v_it->vector[2], v_it->vector[3]);
        } else if (v_it->kind_string == "LIST") {
            Speech_Property::print_list(v_it->list);
        }
    }
    DT_scope_close();
}

void Speech_Property::print(void)
{
#ifdef NDEBUG
//    static_assert(false, "what");
    return;
#endif
    DT_scope_open();
    {
        if (this->annotation == "") {
            DT_print("label=[%s], tag=[%s], type=[%s], kind=[%s], key=[%s], idx=[%llu] id=[%llu]\n", this->label.c_str(), this->tag_str.c_str(), this->type_str.c_str(), this->kind_str.c_str(), this->key.c_str(), this->idx, this->ID);
        } else {
            DT_print("label=[%s], tag=[%s], type=[%s], kind=[%s], key=[%s], idx=[%llu], @=[%s] id=[%llu]\n", this->label.c_str(), this->tag_str.c_str(), this->type_str.c_str(), this->kind_str.c_str(), this->key.c_str(), this->idx, this->annotation.c_str(), this->ID);
        }
//        if (this->referenced_by != nullptr) {
//            DT_print("<is referenced>\n");
//        }
        if (this->refers_to != nullptr) {
            DT_print("<coreference substitution> id=[%llu]\n", this->refers_to->ID);
        }
        
//        NONE,
//        THING_TYPE,
//        THING_INSTANCE,
//        PROPERTY_OF_THING,
//        FUNCTION,
//        TEXT,
//        NUMERIC,
//        FLAG,
//        VECTOR,
        if (this->value.kind_string != "NONE") {
            DT_print("value={\n");
            {
                DT_scope_opennb();
                
                if (this->value.kind_string == "THING_TYPE") {
                    DT_print("THING_TYPE=[%s]", this->value.text.c_str());
                } else if (this->value.kind_string == "THING_INSTANCE") {
                    DT_print("THING_INSTANCE=[%llu]", this->value.thing);
                } else if (this->value.kind_string == "PROPERTY_OF_THING") {
                    DT_print("PROPERTY_OF_THING=[%s]", this->value.text.c_str());
                } else if (this->value.kind_string == "FUNCTION") {
                    DT_print("FUNCTION=[%s]", this->value.text.c_str());
                } else if (this->value.kind_string == "TEXT") {
                    DT_print("TEXT=[%s]", this->value.text.c_str());
                } else if (this->value.kind_string == "NUMERIC") {
                    DT_print("NUMERIC=[%f]", this->value.numeric);
                } else if (this->value.kind_string == "FLAG") {
                    DT_print("FLAG=[%s]", bool_str(this->value.flag));
                } else if (this->value.kind_string == "VECTOR") {
                    DT_print("VECTOR=[<%f,%f,%f,%f>]", this->value.vector[0], this->value.vector[1], this->value.vector[2], this->value.vector[3]);
                } else if (this->value.kind_string == "LIST") {
                    Speech_Property::print_list(this->value.list);
                } else if (this->value.kind_string == "REFERENCE") {
                    DT_print("REFERENCE");
                    Speech_Property* ref = (Speech_Property*)this->value.reference;
                    if (ref != nullptr) {
                        ref->print();
                    }
                } else {
                    ASSERT_MSG(false, "Speech_Instruction::Value should always have a type flag!\n");
                }
                DT_print("\n");
                
                DT_scope_close();
            }
        }
        
        for (auto it = this->properties.begin(); it != this->properties.end(); ++it) {
            cstring key = it->first.c_str();
            DT_print("[%s] = [\n", key);
            auto& prop_list = it->second;
            for (auto p_it = prop_list.begin(); p_it != prop_list.end(); ++p_it) {
                dt::Speech_Property* instruction = *p_it;
                instruction->print();
                DT_print(",\n");
            }
            DT_print("]\n");
        }
    }
    DT_scope_close();
    
//    return;
//    using TYPE_FLAG = Speech_Property::TYPE_FLAG;
//    auto flag = static_cast<TYPE_FLAG>(this->flags);
//    if (flag_contains(flag, TYPE_FLAG::ATTRIBUTE)) {
//        //this->Attribute_print();
//    } else if (flag_contains(flag, TYPE_FLAG::NOUN)) {
//        //Speech_Instruction::Noun_print(this);
//    } else if (flag_contains(flag, TYPE_FLAG::PRONOUN)) {
//        //Speech_Instruction::Pronoun_print(this);
//    }
}


Speech_Property* Speech_Property::get_root(void)
{
    auto* parent = this->get_active_parent();
    if (parent == nullptr) {
        return this;
    }
    
    return parent->get_root();
}

void Speech_Property::print_from_root(void)
{
    this->get_root()->print();
}

namespace rules {

MTT_NODISCARD Rule* Rule::make()
{
    auto* rule = mem::alloc_init<Rule>(&dt::DrawTalk::ctx()->rules.allocator);
    Rule::init(rule);
    return rule;
}

void Rule::init(Rule* rule)
{
    Rule::next_id += 1;
    rule->id = Rule::next_id;
    
    Rule::rules.insert({rule->id, rule});
}

Rule* Rule::lookup(Rule_ID id)
{
    auto find_it = Rule::rules.find(id);
    if (find_it == Rule::rules.end()) {
        return nullptr;
    }
    
    return find_it->second;
}

Rule_ID Rule::next_id= Rule_ID_INVALID;
mtt::Map_Stable<Rule_ID, Rule*> Rule::rules = {};


void Rule::destroy(Rule* ref)
{
    ref = Rule::lookup(ref->id);
    if (ref == nullptr) {
        return;
    }
    Rule::rules.erase(ref->id);
    mem::deallocate<Rule>(&dt::DrawTalk::ctx()->rules.allocator, ref);
}

void Rule::destroy(Rule_ID id)
{
    Rule* rule = Rule::lookup(id);
    if (rule == nullptr) {
        return;
    }
    Rule::rules.erase(id);
    mem::deallocate<Rule>(&dt::DrawTalk::ctx()->rules.allocator, rule);
}

MTT_NODISCARD Trigger* Trigger::make()
{
    return mem::alloc_init<Trigger>(&dt::DrawTalk::ctx()->triggers.allocator);
}
void Trigger::destroy(Trigger* ref)
{
    mem::deallocate<Trigger>(&dt::DrawTalk::ctx()->triggers.allocator, ref);
}

MTT_NODISCARD Trigger::Clause* Trigger::Clause::make()
{
    return mem::alloc_init<Trigger::Clause>(&dt::DrawTalk::ctx()->trigger_clauses.allocator);
}
void Trigger::Clause::destroy(Trigger::Clause* ref)
{
    mem::deallocate<Trigger::Clause>(&dt::DrawTalk::ctx()->trigger_clauses.allocator, ref);
}

MTT_NODISCARD Response* Response::make()
{
    return mem::alloc_init<Response>(&dt::DrawTalk::ctx()->responses.allocator);
}
void Response::destroy(Response* ref)
{
    mem::deallocate<Response>(&dt::DrawTalk::ctx()->responses.allocator, ref);
}


MTT_NODISCARD Response::Clause* Response::Clause::make()
{
    return mem::alloc_init<Response::Clause>(&dt::DrawTalk::ctx()->response_clauses.allocator);
}
void Response::Clause::destroy(Response::Clause* ref)
{
    mem::deallocate<Response::Clause>(&dt::DrawTalk::ctx()->response_clauses.allocator, ref);
}

MTT_NODISCARD Rule_Variable* Rule_Variable::make()
{
    return mem::alloc_init<Rule_Variable>(&dt::DrawTalk::ctx()->rule_variables.allocator);
}
void Rule_Variable::destroy(Rule_Variable* ref)
{
    mem::deallocate<Rule_Variable>(&dt::DrawTalk::ctx()->rule_variables.allocator, ref);
}


void group_related_vars(Condition_Builder& b, mtt::Map<mtt::String, dt::Dynamic_Array<uint64>>& var_to_idx, mtt::Map<uint64, dt::Dynamic_Array<mtt::String>>& idx_to_var, dt::Dynamic_Array<dt::Dynamic_Array<mtt::String>>& conn_components, mtt::String& key, mtt::Set<mtt::String>& visited)
{
    
    auto& idx = var_to_idx[key];
    for (auto idx_it = idx.begin(); idx_it != idx.end(); ++idx_it) {
        auto& key_list = idx_to_var[*idx_it];
        for (auto key_it = key_list.begin(); key_it != key_list.end(); ++key_it) {
            auto& key = (*key_it);
            if (visited.find(key) != visited.end()) {
                continue;
            }
            visited.insert(key);
            conn_components.back().push_back(key);
            
            group_related_vars(b, var_to_idx, idx_to_var, conn_components, key, visited);
        }
        
    }
}





void group_related_vars(Condition_Builder& b) {
    dt::Dynamic_Array<dt::Dynamic_Array<mtt::String>> conn_components;
    mtt::Set<mtt::String> visited;
    
    mtt::Map<mtt::String, dt::Dynamic_Array<uint64>>& var_to_idx = b.var_to_clause_indices;
    usize var_to_idx_size = var_to_idx.size();
    mtt::Map<uint64, dt::Dynamic_Array<mtt::String>>& idx_to_var = b.clause_indices_to_var;
    usize idx_to_var_size = idx_to_var.size();
    
    for (auto it = var_to_idx.begin(); it != var_to_idx.end(); ++it) {
        auto& key = it->first;
        auto& idx = it->second;
        
        if (visited.find(key) != visited.end()) {
            continue;
        }
        visited.insert(key);
        conn_components.push_back({});
        conn_components.back().push_back(key);
        
        for (auto idx_it = idx.begin(); idx_it != idx.end(); ++idx_it) {
            auto& key_list = idx_to_var[*idx_it];
            for (auto key_it = key_list.begin(); key_it != key_list.end(); ++key_it) {
                auto& key = (*key_it);
                if (visited.find(key) != visited.end()) {
                    continue;
                }
                visited.insert(key);
                conn_components.back().push_back(key);
                
                group_related_vars(b, var_to_idx, idx_to_var, conn_components, key, visited);
            }
            
        }
    }
    
    b.conn_components = conn_components;
    
    dt::Dynamic_Array<mtt::Set_Stable<usize>> clauses_to_combine = {};
    for (usize i = 0; i < conn_components.size(); i += 1) {
        MTT_print("%s", "{\n");
        clauses_to_combine.push_back({});
        for (usize j = 0; j < conn_components[i].size(); j += 1) {
            MTT_print("%s, ", conn_components[i][j].c_str());
            for (usize idx : var_to_idx[conn_components[i][j]]) {
                clauses_to_combine.back().insert(idx);
            }
        }
        MTT_print("%s", "\n}\n");
    }
    MTT_BP();
    std::vector<mtt::String> terms = {};


    for (usize i = 0; i < clauses_to_combine.size(); i += 1) {
        std::vector<Condition_Builder::Condition_Builder_Element> next_term_list = {};
        usize to_reserve = 0;
        for (auto idx : clauses_to_combine[i]) {
            auto& entry = b.clauses[idx];
            {
                usize len = entry.size();
                
                next_term_list.push_back((Condition_Builder::Condition_Builder_Element){});
                Condition_Builder::Condition_Builder_Element& el = next_term_list.back();
                mtt::String& as_string = el.term_str;
                uint64& key = el.key;
                
                switch (len) {
                case 3: {
                    as_string = (entry[0].is_negated ? "!" : "") +
                                mtt::String( (entry[0].is_variable  ? MTT_Q_VAR_PREFIX : "") + entry[0].name) + "(" +
                                mtt::String( (entry[1].is_variable  ? MTT_Q_VAR_PREFIX : "") + entry[1].name) + ", " +
                                mtt::String( (entry[2].is_variable  ? MTT_Q_VAR_PREFIX : "") + entry[2].name) + ")";
                    to_reserve += as_string.size() + 2;
                    key += (entry[0].is_negated) ? (1llu << 32) : 0;
                    key += (entry[0].is_variable ) ? 1 : 0;
                    key += (entry[1].is_variable ) ? 1 : 0;
                    key += (entry[2].is_variable ) ? 1 : 0;
                    
                    //ecs_rule_find_var(rule, name)
                    break;
                }
                case 2: {
                    as_string = (entry[0].is_negated ? "!" : "") +
                        mtt::String( (entry[0].is_variable  ? MTT_Q_VAR_PREFIX : "") + entry[0].name) + "(" +
                        mtt::String( (entry[1].is_variable  ? MTT_Q_VAR_PREFIX : "") + entry[1].name) + ")";
                    to_reserve += as_string.size() + 2;
                    key += (entry[0].is_negated) ? (1llu << 32) : 0;
                    key += (entry[0].is_variable ) ? 1 : 0;
                    key += (entry[1].is_variable ) ? 1 : 0;
                    
                    break;
                }
                }
                
                
            }
        }
        
        std::stable_sort(next_term_list.begin(), next_term_list.end(), [](const Condition_Builder::Condition_Builder_Element& a, const Condition_Builder::Condition_Builder_Element& b) {
                return (a.key < b.key);
        });
        
        mtt::String clause;
        clause.reserve(to_reserve);
        b.combined.push_back(next_term_list);
        for (isize t = 0; t < next_term_list.size() - 1; t += 1) {
            clause += next_term_list[t].term_str + ", ";
        }
        clause += next_term_list[next_term_list.size() - 1].term_str;
        terms.push_back(clause);
    }
    b.terms = terms;
    
    //mtt::String& combined_string = b.combined_string;
    

    MTT_print("%s", "{\n");
    mtt::World* world = mtt::ctx();
    for (usize i = 0; i < terms.size(); i += 1) {
        MTT_print("    %s\n", terms[i].c_str());
        
        mtt::Query_Rule rule = Query_Rule_make(world, terms[i]);
        if (mtt::Query_Rule_is_valid(&rule)) {
            usize idx = b.rules.size();
            b.vars_for_rule_idx.emplace_back();
            auto& vars_for_rule = b.vars_for_rule_idx.back();
            for (usize v_i = 0; v_i < b.vars_to_find.size();) {
                auto& var_key = b.vars_to_find[v_i];
                auto v = ecs_rule_find_var(rule.rule, var_key.c_str());
                if (v != mtt::Rule_Var_Handle_INVALID) {
                    mtt::Rule_Var_Record_One_Result record = {
                        .var =  v,
                        .idx = idx,
                    };
                    b.var_lookup.insert({var_key, record});
                    
                    mtt::Rule_Var_Record record_val_list = {
                        .var =  v,
                        .idx = idx,
                    };
                    vars_for_rule.push_back(record_val_list);
                    
                    std::swap(b.vars_to_find[v_i], b.vars_to_find[b.vars_to_find.size() - 1]);
                    b.vars_to_find.pop_back();
                } else {
                    v_i += 1;
                }
            }
            
            
            b.rules.push_back(rule);
            //b.rule_iterators.push_back({});
        } else {
            MTT_BP();
            MTT_error("%s\n", "rule failed to create");
        }
        
    }
    MTT_print("%s", "}\n");
    MTT_BP();
    
    
}
void Condition_Builder::build()
{

    
//    for (auto it = var_to_role.begin(); it != var_to_role.end(); ++it) {
//        dependency_graph[it->first] = {};
//    }
//#ifndef NDEBUG
//    MTT_print("%s", "Clause Indices to Var:{\n");
//    for (auto it = clause_indices_to_var.begin(); it != clause_indices_to_var.end(); ++it) {
//        MTT_print("%llu:\n", it->first);
//        for (auto it_sub = it->second.begin(); it_sub != it->second.end(); ++it_sub) {
//            MTT_print("    %s\n", (*it_sub).c_str());
//        }
//    }
//    MTT_print("%s", "}\n");
//    MTT_print("%s", "Var to Clause Indices:{\n");
//    for (auto it = var_to_clause_indices.begin(); it != var_to_clause_indices.end(); ++it) {
//        MTT_print("%s:\n", it->first.c_str());
//        for (auto it_sub = it->second.begin(); it_sub != it->second.end(); ++it_sub) {
//            MTT_print("    %llu\n", (*it_sub));
//        }
//    }
//    MTT_print("%s", "}\n");
//#endif
    
    group_related_vars(*this);
}



}

void get_prop_or_coref_list_(Speech_Property::Active_Prop_List& pl, Speech_Property::Prop_List* valid_list)
{
    for (auto _ : pl) {
        if (_->type_str == "pronoun") {
            Speech_Property::Prop_List* plist = nullptr;
            if (_->try_get_prop("COREFERENCE", &plist)) {
                for (auto* coref : *plist) {
                    if (coref->value.kind_string == "REFERENCE") {
                        Speech_Property* ref = (Speech_Property*)coref->value.reference;
                        valid_list->push_back(ref);
                    }
                    
                }
            }
            continue;
        }
        valid_list->push_back(_);
    }
}



void Sketch_Library::add(Word_Dictionary_Entry* word_entry, mtt::Thing* thing)
{
    auto* copy = mtt::Thing_copy(thing);
    copy->is_user_destructible = false;
    copy->is_visible = false;
    
    mtt::Thing_set_position(thing, vec3(0.0f, 0.0f, 0.0f));
    
    mtt::Rep* rep = mtt::rep(copy);
    
    for (auto& drawable : rep->render_data.drawable_info_list) {
        drawable->is_enabled = true;
    }
    
    
    this->examples[word_entry][thing->id].insert(copy->id);
}
void Sketch_Library::remove(Word_Dictionary_Entry* word_entry, mtt::Thing* thing)
{
    auto find_word = this->examples.find(word_entry);
    if (find_word == this->examples.end()) {
        return;
    }
    
    auto find_src = find_word->second.find(thing->id);
    if (find_src == find_word->second.end()) {
        return;
    }
    
    for (mtt::Thing_ID id : find_src->second) {
        mtt::Thing_destroy(mtt::world(thing), id);
    }
    find_word->second.erase(thing->id);
}

}
