//
//  word_info.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/2/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef word_info_hpp
#define word_info_hpp

#include "drawtalk_shared_types.hpp"

#include "thing.hpp"

#include <unordered_map>

namespace dt {


struct Speech_Token;

typedef enum WORD_INFO_TYPE {
    WORD_INFO_TYPE_NOMINAL,
    WORD_INFO_TYPE_VERBAL,
    WORD_INFO_TYPE_DETERMINER,
    WORD_INFO_TYPE_RULE,
    WORD_INFO_TYPE_PREPOSITION,
    WORD_INFO_TYPE_CONJUNCTION,
    
    WORD_INFO_TYPE_WHADVERB,
    WORD_INFO_TYPE_ADVERB,
    WORD_INFO_TYPE_ADJECTIVE,
    WORD_INFO_TYPE_PRONOUN,
    
    WORD_INFO_TYPE_IGNORE,
    WORD_INFO_TYPE_COUNT
} WORD_INFO_TYPE;

struct Word_Info;

typedef enum CONJUNCTION_RELATIONSHIP {
    CONJUNCTION_RELATIONSHIP_UNKNOWN,
    CONJUNCTION_RELATIONSHIP_CONSEQUENCE, // Consequence - so
    CONJUNCTION_RELATIONSHIP_BUT, // contrast/exception
    CONJUNCTION_RELATIONSHIP_YET, // contrast/exception
    CONJUNCTION_RELATIONSHIP_AND, // non-contrasting
    CONJUNCTION_RELATIONSHIP_OR, // alternative
    CONJUNCTION_RELATIONSHIP_FOR, // rationale
    
} CONJUNCTION_RELATIONSHIP;

typedef enum VERB_CLASS {
    VERB_CLASS_UNKNOWN,
    VERB_CLASS_STATE_CHANGE,
    VERB_CLASS_STATE_CONTINUOUS,
    VERB_CLASS_ACTION,
} VERB_CLASS;

const char *const VERB_CLASS_STRINGS[] = {
    "VERB_CLASS_UNKNOWN",
    "VERB_CLASS_STATE_CHANGE",
    "VERB_CLASS_STATE_CONTINUOUS",
    "VERB_CLASS_ACTION",
};

struct Pronoun_Info;

struct Nominal_Info {
    Speech_Token* token;
    bool is_plural;
    bool is_mass;
    bool should_likely_represent;
    bool is_main_actor_or_agent;
    bool is_acted_upon_or_patient;
    bool is_direct_object;
    bool is_indirect_object;
    bool is_object_of_preposition;
    
    usize number;
    bool specific_entity;
    
    
    Dynamic_Array<Nominal_Info*> associated;
    
    bool is_unresolved;
    bool is_pronoun;
    
    Pronoun_Info* unresolved_pronoun;
    
    Word_Info* action_ref;
    Word_Info* acted_on_by_ref;
    
    mtt::Thing_ID thing_id;
    
    bool is_time_expression;
    
    Nominal_Info() : token(nullptr), is_plural(false), is_mass(false), should_likely_represent(false),
    is_main_actor_or_agent(false), is_acted_upon_or_patient(false), is_direct_object(false), is_indirect_object(false), is_object_of_preposition(false), number(0), specific_entity(false), is_unresolved(false), is_pronoun(false), unresolved_pronoun(nullptr), action_ref(nullptr), acted_on_by_ref(nullptr), thing_id(mtt::Thing_ID_INVALID), is_time_expression(false) {}
};

typedef enum VERBAL_TENSE {
    VERBAL_TENSE_PRESENT,
    VERBAL_TENSE_PAST,
    VERBAL_TENSE_FUTURE,
} VERBAL_TENSE;


enum struct WORD_DICTIONARY_ENTRY_TYPE {
    UNKNOWN,
    VERB,
    NOUN,
    ATTRIBUTE,
};

static const mtt::String TYPE_PREFIX = "type";
static const mtt::String ACTION_PREFIX = "action";
static const mtt::String ATTRIBUTE_PREFIX = "attribute";
static const mtt::String NAMESPACE_SEPARATOR = "::";
static const mtt::String TEMPLATE_PREFIX = "template";

struct Word_Dictionary;
struct Word_Dictionary_Entry;

struct Visualizable_Word {
    Word_Dictionary_Entry* dict_entry;
    mtt::Thing_ID id;
};


Word_Dictionary* word_dict(void);

typedef enum VERB_EVENT {
    VERB_EVENT_BEGIN = 0,
    VERB_EVENT_END = 1,
    VERB_EVENT_CONTINUOUS = 2,
    VERB_EVENT_COUNT__
} VERB_EVENT;

struct alignas(16) Word_Dictionary_Entry {
    WORD_DICTIONARY_ENTRY_TYPE type;
    mtt::String name;
    mtt::String name_secondary;
    
    flecs::entity typename_desc;
    flecs::entity events[VERB_EVENT_COUNT__];
    

    flecs::entity template_desc;
    
    VERB_CLASS verb_class;
    
    mtt::Map_Stable<mtt::Thing_ID, Visualizable_Word> things;
    
    Word_Dictionary_Entry* equivalent_to = nullptr;
    
    Word_Dictionary* dict;
    
    // should assume plural pronoun refers to both agent and patient when preposition is bidirectional
    bool must_apply_to_all_objects_in_group;
    
    bool verb_is_bidirectional_by_default;
    
    bool verb_should_check_all_equivalencies;
    
    bool should_not_visualize;
    
    bool (*on_attribute_add)(Word_Dictionary_Entry*, mtt::Thing* thing) = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool { return true; };
    bool (*on_attribute_remove)(Word_Dictionary_Entry*, mtt::Thing* thing) = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool { return true; };
    
    
    
    mtt::String get_query_name()
    {
        return mtt::String(this->typename_desc.name());
    }
    
    std::unordered_map<mtt::Thing_ID, std::unordered_set<Word_Dictionary_Entry*>> properties;
    
    
//    static void init(Word_Dictionary_Entry* entry) {
//        entry->type = WORD_DICTIONARY_ENTRY_TYPE::UNKNOWN;
//        entry->name = "";
//        entry->name_secondary = "";
//        entry->verb_class = VERB_CLASS_UNKNOWN;
//        entry->dict = nullptr;
//        entry->must_apply_to_all_objects_in_group = false;
//        entry->verb_is_bidirectional_by_default = false;
//        entry->verb_should_check_all_equivalencies = false;
//        entry->should_not_visualize = false;
//    }

    Word_Dictionary_Entry() :
    type(WORD_DICTIONARY_ENTRY_TYPE::UNKNOWN),
    name(""),
    name_secondary(""),
    verb_class(VERB_CLASS_UNKNOWN),
    dict(nullptr),
    must_apply_to_all_objects_in_group(false),
    verb_is_bidirectional_by_default(false),
    verb_should_check_all_equivalencies(false),
    should_not_visualize(false),
    properties({})
    {}
    
    
    
};

static mtt::String verb_event_suffix[] = {
    [VERB_EVENT_BEGIN] = "begin",
    [VERB_EVENT_END]   = "end",
    [VERB_EVENT_CONTINUOUS]   = ""
};
static inline auto Word_Dictionary_Entry_verb_event_define(void* ctx, Word_Dictionary_Entry* entry, VERB_EVENT event_id, const mtt::String& key)
{
    auto* ecs_world = (flecs::world*)ctx;
    auto* ev = &entry->events[event_id];
    *ev = flecs::entity(*ecs_world, (key + verb_event_suffix[event_id]).c_str());
    return *ev;
}
static inline auto Word_Dictionary_Entry_event_for_verb(Word_Dictionary_Entry* entry, VERB_EVENT event_id)
{
    return entry->events[event_id];
}

static inline mtt::String Word_Dictionary_Entry_name_for_verb_with_event(Word_Dictionary_Entry* entry, VERB_EVENT event_id)
{
    return (event_id != VERB_EVENT_CONTINUOUS) ? entry->name + verb_event_suffix[event_id] : entry->name;
}


mtt::String Word_Dictionary_Entry_name_for_verb_with_event(Word_Dictionary_Entry* entry, VERB_EVENT event_id);

struct Relation_Entry {
    Word_Dictionary_Entry* rel;
    Word_Dictionary_Entry* arg;
};

struct alignas(16) Word_Dictionary {
    mtt::Map_Stable<mtt::String, Word_Dictionary_Entry*> noun_map;
    std::vector<Word_Dictionary_Entry*> nouns;
    
    mtt::Map_Stable<mtt::String, Word_Dictionary_Entry*> verb_map;
    std::vector<Word_Dictionary_Entry*> verbs;
    
    mtt::Map_Stable<mtt::String, Word_Dictionary_Entry*> attribute_map;
    std::vector<Word_Dictionary_Entry*> attributes;
    
    std::unordered_map<mtt::Thing_ID, mtt::Set_Stable<Word_Dictionary_Entry*>> thing_to_word;
    std::unordered_map<mtt::Thing_ID, mtt::Set_Stable<Word_Dictionary_Entry*>> thing_to_own_attribute;
    
    mtt::Set_Stable<mtt::String> ignored_attribute_labels = {};
    
    inline void insert_ignored_attribute(const mtt::String& str)
    {
        this->ignored_attribute_labels.insert(str);
    }
    inline void remove_ignored_attribute(const mtt::String& str)
    {
        this->ignored_attribute_labels.erase(str);
    }
    bool is_ignored_attribute(const mtt::String& str)
    {
        return this->ignored_attribute_labels.contains(str);
    }
    
    struct Active_Relation {
        mtt::Thing_ID source = mtt::Thing_ID_INVALID;
        mtt::Thing_ID target = mtt::Thing_ID_INVALID;
        Word_Dictionary_Entry* dict_entry = nullptr;
        Word_Dictionary_Entry* dict_entry_target = nullptr;
    };
    
    
    typedef mtt::Map_Stable<Word_Dictionary_Entry*, dt::Dynamic_Array<Active_Relation>> Active_Relation_Map;
    
    std::unordered_map<mtt::Thing_ID, Active_Relation_Map> source_to_target_relation;
    std::unordered_map<mtt::Thing_ID, Active_Relation_Map> target_to_source_relation;

    mem::Pool_Allocation alloc_word_entry_pool;
    
    static Word_Dictionary_Entry* entry_make(void);
    
    static void entry_destroy(Word_Dictionary_Entry* entry);
};

void Word_Dictionary_init(Word_Dictionary* dict);



struct Word_Dictionary_Entry_Compare_Routine_increasing {
    bool operator()(const Word_Dictionary_Entry* a, const Word_Dictionary_Entry* b) const {
        if (a->name == b->name) {
            return a->name_secondary <= b->name_secondary;
        }
        
        return a->name < b->name;
    }
    bool operator()(Word_Dictionary_Entry* a, Word_Dictionary_Entry* b) const {
        if (a->name == b->name) {
            return a->name_secondary <= b->name_secondary;
        }
        
        return a->name < b->name;
    }

    bool operator()(Word_Dictionary_Entry a, Word_Dictionary_Entry b) const {
        if (a.name == b.name) {
            return a.name_secondary <= b.name_secondary;
        }
        
        return a.name < b.name;
    }
    
    
    bool operator()(Word_Dictionary_Entry& a, Word_Dictionary_Entry& b) const {
        if (a.name == b.name) {
            return a.name_secondary <= b.name_secondary;
        }
        
        return a.name < b.name;
    }
    
    bool operator()(const Word_Dictionary_Entry& a, const Word_Dictionary_Entry& b) const {
        if (a.name == b.name) {
            return a.name_secondary <= b.name_secondary;
        }
        
        return a.name < b.name;
    }
};



Word_Dictionary_Entry* noun_add(const mtt::String& key, const mtt::String& key_secondary="");
Word_Dictionary_Entry* noun_lookup(const mtt::String& key, const mtt::String& key_secondary="");
Word_Dictionary_Entry* verb_add(const mtt::String& key, const mtt::String& key_secondary="");
Word_Dictionary_Entry* verb_lookup(const mtt::String& key, const mtt::String& key_secondary="");
Word_Dictionary_Entry* attribute_add(const mtt::String& key, const mtt::String& key_secondary="");
Word_Dictionary_Entry* attribute_lookup(const mtt::String& key, const mtt::String& key_secondary="");

// FIXME: ...
Word_Dictionary_Entry* noun_add_attribute(Word_Dictionary_Entry* word, const mtt::String& attribute);
// FIXME: ...
void noun_remove_attribute(Word_Dictionary_Entry* word, const mtt::String& attribute);
Relation_Entry noun_add_relational_attribute(Word_Dictionary_Entry* word, const mtt::String& relation, const mtt::String& attribute, bool is_transitive);
void noun_remove_relational_attribute(Word_Dictionary_Entry* word, const mtt::String& relation, const mtt::String& attribute);
Word_Dictionary_Entry* noun_derive_from(Word_Dictionary_Entry* derived, Word_Dictionary_Entry* base);
Visualizable_Word* vis_word_derive_from(mtt::Thing* thing, Word_Dictionary_Entry* entry);
void vis_word_underive_from(mtt::Thing* thing, Word_Dictionary_Entry* entry);

Word_Dictionary_Entry* noun_make_equivalent_to(Word_Dictionary_Entry* equivalent, Word_Dictionary_Entry* base);

void attribute_derive_from(Word_Dictionary_Entry* derived, Word_Dictionary_Entry* base);
void verb_make_equivalent_to(Word_Dictionary_Entry* derived, Word_Dictionary_Entry* base);
Word_Dictionary_Entry* verb_root_equivalence(Word_Dictionary_Entry* entry);

void Thing_add_relation(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target);
bool Thing_has_relation(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target);
void Thing_remove_relation(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target);


template <typename PROC>
void Thing_get_relations(mtt::World* world, mtt::Thing_ID src_id, PROC&& proc)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, src_id);
    if (thing == nullptr) {
        return;
    }
    
    auto& map = word_dict()->source_to_target_relation;
    
    auto find = map.find(src_id);
    if (find != map.end()) {
        //  typedef mtt::Map_Stable<Word_Dictionary_Entry*, dt::Dynamic_Array<Active_Relation>> Active_Relation_Map;
        Word_Dictionary::Active_Relation_Map& target_rel = find->second;
        for (const auto& [key, value] : target_rel) {
            for (const auto& target : value) {
                proc(thing, &target);
            }
        }
    }
}

void Thing_add_action(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target);
void Thing_remove_action(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target);

void Thing_add_action(mtt::Thing* src, Word_Dictionary_Entry* rel);
void Thing_remove_action(mtt::Thing* src, Word_Dictionary_Entry* rel);

void Thing_add_action_event_instantaneous(mtt::Thing* src, Word_Dictionary_Entry* rel, mtt::Thing* target, VERB_EVENT event_id);

void Thing_add_attribute_event_instantaneous(mtt::Thing* src, Word_Dictionary_Entry* rel, Word_Dictionary_Entry* attrib, VERB_EVENT event_id);

void Thing_add_IsDoingAction(mtt::Thing* thing, Word_Dictionary_Entry* entry);
void Thing_remove_IsDoingAction(mtt::Thing* thing, Word_Dictionary_Entry* entry);

void Thing_add_IsReceivingAction(mtt::Thing* thing, Word_Dictionary_Entry* entry);
void Thing_remove_IsReceivingAction(mtt::Thing* thing, Word_Dictionary_Entry* entry);

void Thing_add_attribute(mtt::Thing* src, Word_Dictionary_Entry* attrib, mtt::Any data = {});

bool Thing_has_attribute(mtt::Thing* src, Word_Dictionary_Entry* attrib);
void Thing_remove_attribute(mtt::Thing* src, Word_Dictionary_Entry* attrib);
void Thing_copy_own_attributes(mtt::Thing* src, mtt::Thing* dst);
void Thing_add_relation(mtt::Thing* src, const mtt::String& relation, const mtt::String& attribute, bool is_transitive);

void Thing_add_attribute(mtt::Thing* src, const mtt::String& A, const mtt::String& B, mtt::Any data = {});
void Thing_add_attribute(mtt::Thing* src, Word_Dictionary_Entry* A, Word_Dictionary_Entry* B, mtt::Any data = {});
mtt::Any* Thing_get_attribute_value(mtt::Thing* src, Word_Dictionary_Entry* A);
mtt::Any* Thing_get_attribute_value(mtt::Thing* src, const mtt::String& key, const mtt::String& key_secondary="");
bool Thing_remove_attribute_property(mtt::Thing* src, Word_Dictionary_Entry* A, Word_Dictionary_Entry* B);
bool Thing_remove_attribute_property(mtt::Thing* src, const mtt::String& A, const mtt::String& B);

void Thing_remove_all_own_attributes(mtt::Thing* thing);

mtt::Set_Stable<Word_Dictionary_Entry*>* Thing_get_own_attributes(mtt::Thing* src);


struct Verbal_Info {
    Speech_Token* token;
    bool should_merge_with_parent;
    bool is_negated;
    bool should_treat_as_trigger;
    bool should_treat_as_trigger_response;
    bool should_treat_as_ability;
    VERBAL_TENSE tense;
    VERB_CLASS verb_class;
    
    Word_Dictionary_Entry* dict_entry;
    // search the system tree with these keys
    Dynamic_Array<mtt::String> keys;
    Dynamic_Array<Verbal_Info*> associated;
    Dynamic_Array<Nominal_Info*> agents;
    Dynamic_Array<Nominal_Info*> objects;
    
    bool has_preposition;
    Speech_Token* preposition;
    
    bool is_passive;
    
    bool is_bidirectional_by_default;
    
    
    Word_Info* rule_ref;
    
    
    mtt::Thing_ID thing_id;
    
    Verbal_Info() : token(nullptr), should_merge_with_parent(false), is_negated(false), should_treat_as_trigger(false),
    should_treat_as_trigger_response(false), should_treat_as_ability(false), tense((VERBAL_TENSE)0),
    verb_class((VERB_CLASS)0), dict_entry(nullptr), has_preposition(false), preposition(nullptr), is_passive(false), is_bidirectional_by_default(false), rule_ref(nullptr), thing_id(mtt::Thing_ID_INVALID) {}
};

struct Preposition_Info {
    Speech_Token* token;
    bool is_preposition;
    Dynamic_Array<Nominal_Info*> objects_of_preposition;
    
    Preposition_Info() : token(nullptr), is_preposition(false) {}
};

struct Rule_Info {
    Speech_Token* token;
    Word_Info* trigger_info;
    Word_Info* response_info;
    
    
    mtt::Thing_ID thing_id;
    
    Rule_Info() : token(nullptr), trigger_info(nullptr), response_info(nullptr), thing_id(mtt::Thing_ID_INVALID) {}
};


struct Determiner_Info {
    Speech_Token* token;
    bool is_guaranteed_plural;
    bool is_guaranteed_singular;
    bool is_definite;
    
    Determiner_Info() : token(nullptr), is_guaranteed_plural(false), is_guaranteed_singular(false), is_definite(false) {}
};

struct Statement_Info {
    Speech_Token* token;
    // TODO: decide if needed
    
    Statement_Info() : token(nullptr) {}
};

struct Conditional_Info {
    Speech_Token* token;
    
    Conditional_Info() : token(nullptr) {}
};


struct Adverb_Info {
    Speech_Token* token;
    
    Adverb_Info() : token(nullptr) {}
};
struct Adjective_Info {
    Speech_Token* token;
    
    Adjective_Info() : token(nullptr) {}
};
struct WHAdverb_Info {
    Speech_Token* token;
    
    bool is_question;
    bool is_time_expression;
    bool should_treat_as_rule;
    
    WHAdverb_Info() : token(nullptr), is_question(false), is_time_expression(false), should_treat_as_rule(false) {}
};


typedef enum PERSON_TYPE {
    PERSON_TYPE_UNKNOWN,
    PERSON_TYPE_FIRST,
    PERSON_TYPE_SECOND,
    PERSON_TYPE_THIRD,
} PERSON_TYPE;

struct Pronoun_Info {
    Speech_Token* token;
    
    PERSON_TYPE person_type;
    
    bool is_plural;
    bool should_assume_refers_to_agents_and_patients;
    
    Nominal_Info corresponding_nom_info;
    
    Pronoun_Info() : token(nullptr), person_type((PERSON_TYPE)0), is_plural(false), should_assume_refers_to_agents_and_patients(false), corresponding_nom_info(Nominal_Info()) {}
};

struct Sequence_Info {
    Speech_Token* token;
    
    Sequence_Info() : token(nullptr) {}

};

struct Speech_Property;

struct Word_Info {
    WORD_INFO_TYPE type;
    
    Nominal_Info     nominal;
    Verbal_Info      action;
    Rule_Info        rule;
    Determiner_Info  determiner_info;
    Preposition_Info preposition_info;
    Statement_Info   statement_info;
    Conditional_Info cond_info;
    WHAdverb_Info    whadverb_info;
    Adverb_Info      adverb_info;
    Adjective_Info   adjective_info;
    Pronoun_Info     pronoun_info;
    Sequence_Info    sequence_info;
    
    Dynamic_Array<Word_Info*> children;
    
    Speech_Token* token;
    Word_Info* parent;
    
    std::vector<dt::Speech_Property*> prop_list;
    
    Word_Info() : type((WORD_INFO_TYPE)0), token(nullptr), parent(nullptr) {}

};







}

#endif /* word_info_hpp */
