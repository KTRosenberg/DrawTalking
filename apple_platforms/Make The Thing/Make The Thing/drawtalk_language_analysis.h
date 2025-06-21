//
//  drawtalk_language_analysis.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 8/11/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_language_analysis_h
#define drawtalk_language_analysis_h

#include "drawtalk_shared_types.hpp"
#include "word_info.hpp"
//#include "drawtalk_behavior.hpp"
//#include "tree.hpp"
#include "recording.hpp"


//namespace mtt {

//typedef enum FUNCTION_ARGUMENT_TYPE : u64 {
//    FUNCTION_ARGUMENT_TYPE_TARGET,
//    
//    FUNCTION_ARGUMENT_TYPE_COUNT
//} FUNCTION_ARGUMENT_TYPE;
//
//typedef enum FUNCTION_TYPE : u64 {
//    FUNCTION_TYPE_SINGLE,
//    FUNCTION_TYPE_SEQUENCE,
//    
//    FUNCTION_TYPE_COUNT
//} FUNCTION_TYPE;
//
//struct Function_Argument {
//    uint64 type;
//    bool is_required;
//    mtt::String name;
//};
//
//// for non-flattened sequences
//struct Behavior_Function_Sequence_Descriptor {
//    std::vector<std::tuple<mtt::String, mtt::String, mtt::String>> action_group_sense_which;
//};
//
//struct Behavior_Function_Descriptor {
//    uint64 function_type;
//    std::vector<Function_Argument> args;
//    std::vector<Connection_Template> connection_templates;
//    
//    Behavior_Function_Sequence_Descriptor seq;
//};
//
//struct Behavior_Component {
//    MTT_Tree_Node* block_node;
//    mtt::String key;
//    
//    
//    // OLD
//    mtt::String name;
//    mtt::String sense_name;
//    
//    mtt::Map<mtt::String, Behavior_Function_Descriptor>
//    functions;
//    
//    //
//    
//    
//    
//    Behavior_Component() {}
//};
//
//struct Action {
//    // this is the flecs entity/prefab representing an action
//    mtt::Entity entity;
//    // table of behavior functions
//    Behavior_Component behavior;
//};
//
//struct Action_Group {
//    // name of the action group (regular name)
//    mtt::String name;
//    // all senses of the action (namespaces) - using an array instead of a map
//    // for now because the expected number of senses for the same action is very small
//    std::vector<Action> senses;
//};
//
//struct Action_Collection {
//    // map action name to group of actions (groups store all senses of the action)
//    mtt::Map<mtt::String, mtt::Action_Group> action_groups;
//};
//
//struct Property {
//    uint64 id;
//};
//
//
//struct Entity_Descriptor {
//    mtt::String name;
//    mtt::String sense_name;
//    
//    mtt::Entity entity_prefab;
//    flecs::type entity_type;
//};
//struct Entity_Group {
//    mtt::String name;
//    std::vector<Entity_Descriptor> senses;
//};
//struct Entity_Collection {
//    mtt::Map<mtt::String, mtt::Entity_Group> entity_groups;
//};
//
//
//}


namespace dt {

static mtt::Set<mtt::String> deictic_words = {
    "this",
    "that",
    "those",
    "these",
    "here",
    "there",
};

static mtt::String preposition_list_str[] = {
    "in",
    "inside",
    "into"
    "out",
    "outside",
    "on",
    "onto",
    "above",
    "atop",
    "on top of",
    "over",
    "beneath",
    "below",
    "under",
    "underneath",
    "at",
    "next to",
    "beside",
    "above",
    "left of",
    "right of",
    "through",
    "with",
    "without",
    "between",
    "along with",
    "after",
    "away",
    "away from"
};
static constexpr const usize preposition_list_count = sizeof(preposition_list_str) / sizeof(mtt::String);



namespace spacy_nlp {

typedef enum LABEL {
    LABEL_COUNT
} LABEL;

static mtt::String tag_str[] = {
    "$",   //symbol, currency
    "``",    //PunctType=quot PunctSide=ini    opening quotation mark
    "''",    //PunctType=quot PunctSide=fin    closing quotation mark
    ",",    //PunctType=comm    punctuation mark, comma
    "-LRB-",    //PunctType=brck PunctSide=ini    left round bracket
    "-RRB-",    //PunctType=brck PunctSide=fin    right round bracket
    ".",    //PunctType=peri    punctuation mark, sentence closer
    ":",    //punctuation mark, colon or ellipsis
    "ADD",   //email
    "AFX",    //Hyph=yes    affix
    "CC",    //ConjType=comp    conjunction, coordinating
    "CD",    //NumType=card    cardinal number
    "DT",    //determiner
    "EX",    //AdvType=ex    existential there
    "FW",    //Foreign=yes    foreign word
    "GW",    //additional word in multi-word expression
    "HYPH",    //PunctType=dash    punctuation mark, hyphen
    "IN",    //conjunction, subordinating or preposition
    "JJ",    //Degree=pos    adjective
    "JJR",    //Degree=comp    adjective, comparative
    "JJS",    //Degree=sup    adjective, superlative
    "LS",    //NumType=ord    list item marker
    "MD",    //VerbType=mod    verb, modal auxiliary
    "NFP",    //superfluous punctuation
    "NIL",    //missing tag
    "NN",    //Number=sing    noun, singular or mass
    "NNP",    //NounType=prop Number=sing    noun, proper singular
    "NNPS",    //NounType=prop Number=plur    noun, proper plural
    "NNS",    //Number=plur    noun, plural
    "PDT",    //predeterminer
    "POS",    //Poss=yes    possessive ending
    "PRP",    //PronType=prs    pronoun, personal
    "PRP$",    //PronType=prs Poss=yes    pronoun, possessive
    "RB",    //Degree=pos    adverb
    "RBR",    //Degree=comp    adverb, comparative
    "RBS",    //Degree=sup    adverb, superlative
    "RP",    //adverb, particle
    "SP",   //space
    "SYM",    //symbol
    "TO",    //PartType=inf VerbForm=inf    infinitival “to”
    "UH",    //interjection
    "VB",    //VerbForm=inf    verb, base form
    "VBD",    //VerbForm=fin Tense=past    verb, past tense
    "VBG",    //VerbForm=part Tense=pres Aspect=prog    verb, gerund or present participle
    "VBN",    //VerbForm=part Tense=past Aspect=perf    verb, past participle
    "VBP",    //VerbForm=fin Tense=pres    verb, non-3rd person singular present
    "VBZ",   //VerbForm=fin Tense=pres Number=sing Person=three    verb, 3rd person singular present
    "WDT",    //wh-determiner
    "WP",    //wh-pronoun, personal
    "WP$",   // Poss=yes    wh-pronoun, possessive
    "WRB",   // wh-adverb
    "XX",    // unknown
    "_SP",
};

// TODO: //
static mtt::Map<mtt::String, usize> tag2idx = {
    {"$",0},   //symbol, currency
    {"``",1},    //PunctType=quot PunctSide=ini    opening quotation mark
    {"''",2},    //PunctType=quot PunctSide=fin    closing quotation mark
    {",",3},    //PunctType=comm    punctuation mark, comma
    {"-LRB-",4},    //PunctType=brck PunctSide=ini    left round bracket
    {"-RRB-",5},    //PunctType=brck PunctSide=fin    right round bracket
    {".",6},    //PunctType=peri    punctuation mark, sentence closer
    {":",7},    //punctuation mark, colon or ellipsis
    {"ADD",8},   //email
    {"AFX",9},    //Hyph=yes    affix
    {"CC",10},    //ConjType=comp    conjunction, coordinating
    {"CD",11},    //NumType=card    cardinal number
    {"DT",12},    //determiner
    {"EX",13},    //AdvType=ex    existential there
    {"FW",14},    //Foreign=yes    foreign word
    {"GW",15},    //additional word in multi-word expression
    {"HYPH",16},    //PunctType=dash    punctuation mark, hyphen
    {"IN",17},    //conjunction, subordinating or preposition
    {"JJ",18},    //Degree=pos    adjective
    {"JJR",19},    //Degree=comp    adjective, comparative
    {"JJS",20},    //Degree=sup    adjective, superlative
    {"LS",21},    //NumType=ord    list item marker
    {"MD",22},    //VerbType=mod    verb, modal auxiliary
    {"NFP",23},    //superfluous punctuation
    {"NIL",24},    //missing tag
    {"NN",25},    //Number=sing    noun, singular or mass
    {"NNP",26},    //NounType=prop Number=sing    noun, proper singular
    {"NNPS",27},    //NounType=prop Number=plur    noun, proper plural
    {"NNS",28},    //Number=plur    noun, plural
    {"PDT",29},    //predeterminer
    {"POS",30},    //Poss=yes    possessive ending
    {"PRP",31},    //PronType=prs    pronoun, personal
    {"PRP$",32},    //PronType=prs Poss=yes    pronoun, possessive
    {"RB",33},    //Degree=pos    adverb
    {"RBR",34},    //Degree=comp    adverb, comparative
    {"RBS",35},    //Degree=sup    adverb, superlative
    {"RP",36},    //adverb, particle
    {"SP",37},   //space
    {"SYM",38},    //symbol
    {"TO",39},    //PartType=inf VerbForm=inf    infinitival “to”
    {"UH",40},    //interjection
    {"VB",41},    //VerbForm=inf    verb, base form
    {"VBD",42},    //VerbForm=fin Tense=past    verb, past tense
    {"VBG",43},    //VerbForm=part Tense=pres Aspect=prog    verb, gerund or present participle
    {"VBN",44},    //VerbForm=part Tense=past Aspect=perf    verb, past participle
    {"VBP",45},    //VerbForm=fin Tense=pres    verb, non-3rd person singular present
    {"VBZ",46},   //VerbForm=fin Tense=pres Number=sing Person=three    verb, 3rd person singular present
    {"WDT",47},    //wh-determiner
    {"WP",48},    //wh-pronoun, personal
    {"WP$",49},   // Poss=yes    wh-pronoun, possessive
    {"WRB",50},   // wh-adverb
    {"XX",51},    // unknown
    {"_SP",52},
};

typedef enum TAG {
    TAG_$=0,   //symbol, currency
    TAG_OPEN_QUOTE=1,    //PunctType=quot PunctSide=ini    opening quotation mark
    TAG_CLOSE_QUOTE=2,    //PunctType=quot PunctSide=fin    closing quotation mark
    TAG_COMMA=3,    //PunctType=comm    punctuation mark, comma
    TAG_LEFT_ROUND_BRACKET=4,    //PunctType=brck PunctSide=ini    left round bracket
    TAG_RIGHT_ROUND_BRACKET=5,    //PunctType=brck PunctSide=fin    right round bracket
    TAG_SENTENCE_CLOSER=6,    //PunctType=peri    punctuation mark, sentence closer
    TAG_COLON_OR_ELLIPSIS=7,    //punctuation mark, colon or ellipsis
    TAG_ADD=8,   //email
    TAG_AFX=9,    //Hyph=yes    affix
    TAG_CC=10,    //ConjType=comp    conjunction, coordinating
    TAG_CD=11,    //NumType=card    cardinal number
    TAG_DT=12,    //determiner
    TAG_EX=13,    //AdvType=ex    existential there
    TAG_FW=14,    //Foreign=yes    foreign word
    TAG_GW=15,    //additional word in multi-word expression
    TAG_HYPH=16,    //PunctType=dash    punctuation mark, hyphen
    TAG_IN=17,    //conjunction, subordinating or preposition
    TAG_JJ=18,    //Degree=pos    adjective
    TAG_JJR=19,    //Degree=comp    adjective, comparative
    TAG_JJS=20,    //Degree=sup    adjective, superlative
    TAG_LS=21,    //NumType=ord    list item marker
    TAG_MD=22,    //VerbType=mod    verb, modal auxiliary
    TAG_NFP=23,    //superfluous punctuation
    TAG_NIL=24,    //missing tag
    TAG_NN=25,    //Number=sing    noun, singular or mass
    TAG_NNP=26,    //NounType=prop Number=sing    noun, proper singular
    TAG_NNPS=27,    //NounType=prop Number=plur    noun, proper plural
    TAG_NNS=28,    //Number=plur    noun, plural
    TAG_PDT=29,    //predeterminer
    TAG_POS=30,    //Poss=yes    possessive ending
    TAG_PRP=31,    //PronType=prs    pronoun, personal
    TAG_PRP$=32,    //PronType=prs Poss=yes    pronoun, possessive
    RB=33,    //Degree=pos    adverb
    TAG_RBR=34,    //Degree=comp    adverb, comparative
    TAG_RBS=35,    //Degree=sup    adverb, superlative
    TAG_RP=36,    //adverb, particle
    TAG_SP=37,   //space
    TAG_SYM=38,    //symbol
    TAG_TO=39,    //PartType=inf VerbForm=inf    infinitival “to”
    TAG_UH=40,    //interjection
    TAG_VB=41,    //VerbForm=inf    verb, base form
    TAG_VBD=42,    //VerbForm=fin Tense=past    verb, past tense
    TAG_VBG=43,    //VerbForm=part Tense=pres Aspect=prog    verb, gerund or present participle
    TAG_VBN=44,    //VerbForm=part Tense=past Aspect=perf    verb, past participle
    TAG_VBP=45,    //VerbForm=fin Tense=pres    verb, non-3rd person singular present
    TAG_VBZ=46,   //VerbForm=fin Tense=pres Number=sing Person=three    verb, 3rd person singular present
    TAG_WDT=47,    //wh-determiner
    TAG_WP=48,    //wh-pronoun, personal
    TAG_WP$=49,   // Poss=yes    wh-pronoun, possessive
    TAG_WRB=50,   // wh-adverb
    TAG_XX=51,    // unknown
    TAG__SP=52,
    
    TAG_COUNT
} TAG;

constexpr const usize PART_OF_SPEECH_NOMINAL_TAG_FIRST = 25;
constexpr const usize PART_OF_SPEECH_NOMINAL_TAG_LAST  = 28;
constexpr const usize PART_OF_SPEECH_GERUND_TAG        = 43;

constexpr const usize PART_OF_SPEECH_NOUN_SINGULAR_OR_MASS = 25;
constexpr const usize PART_OF_SPEECH_NOUN_PROPER_SINGULAR  = 26;
constexpr const usize PART_OF_SPEECH_NOUN_PROPER_PLURAL    = 27;
constexpr const usize PART_OF_SPEECH_NOUN_PLURAL           = 28;


bool pos_tag_is_noun_like(dt::ID tag);
bool pos_tag_noun_is_singular_or_mass(dt::ID tag);
bool pos_tag_noun_is_plural(dt::ID tag);
bool pos_tag_noun_is_proper(dt::ID tag);
bool pos_tag_is_gerund(dt::ID tag);

constexpr const usize PART_OF_SPEECH_VERB_TAG_FIRST = 41;
constexpr const usize PART_OF_SPEECH_VERB_TAG_LAST  = 46;

//static mtt::String tagidx2pos_str[] = {
//    "SYM",        //symbol, currency
//    "PUNCT",    //PunctType=quot PunctSide=ini    opening quotation mark
//    "PUNCT",    //PunctType=quot PunctSide=fin    closing quotation mark
//    "PUNCT",    //PunctType=comm    punctuation mark, comma
//    "PUNCT",    //PunctType=brck PunctSide=ini    left round bracket
//    "PUNCT",    //PunctType=brck PunctSide=fin    right round bracket
//    "PUNCT",    //PunctType=peri    punctuation mark, sentence closer
//    "PUNCT",        //punctuation mark, colon or ellipsis
//    "X",        //email
//    "ADJ",    //Hyph=yes    affix
//    "CCONJ",    //ConjType=comp    conjunction, coordinating
//    "NUM",    //NumType=card    cardinal number
//    "DET",        //determiner
//    "PRON",    //AdvType=ex    existential there
//    "X",    //Foreign=yes    foreign word
//    "X",        //additional word in multi-word expression
//    "PUNCT",    //PunctType=dash    punctuation mark, hyphen
//    "ADP",        //conjunction, subordinating or preposition
//    "ADJ",    //Degree=pos    adjective
//    "ADJ",    //Degree=comp    adjective, comparative
//    "ADJ",    //Degree=sup    adjective, superlative
//    "X",    //NumType=ord    list item marker
//    "VERB",    //VerbType=mod    verb, modal auxiliary
//    "PUNCT",        //superfluous punctuation
//    "X",        //missing tag
//    "NOUN",    //Number=sing    noun, singular or mass
//    "PROPN",    //NounType=prop Number=sing    noun, proper singular
//    "PROPN",    //NounType=prop Number=plur    noun, proper plural
//    "NOUN",    //Number=plur    noun, plural
//    "DET",        //predeterminer
//    "PART",    //Poss=yes    possessive ending
//    "PRON",    //PronType=prs    pronoun, personal
//    "DET",    //PronType=prs Poss=yes    pronoun, possessive
//    "ADV",    //Degree=pos    adverb
//    "ADV",    //Degree=comp    adverb, comparative
//    "ADV",    //Degree=sup    adverb, superlative
//    "ADP",        //adverb, particle
//    "SPACE",        //space
//    "SYM",        //symbol
//    "PART",    //PartType=inf VerbForm=inf    infinitival “to”
//    "INTJ",        //interjection
//    "VERB",    //VerbForm=inf    verb, base form
//    "VERB",    //VerbForm=fin Tense=past    verb, past tense
//    "VERB",    //VerbForm=part Tense=pres Aspect=prog    verb, gerund or present participle
//    "VERB",    //VerbForm=part Tense=past Aspect=perf    verb, past participle
//    "VERB",    //VerbForm=fin Tense=pres    verb, non-3rd person singular present
//    "VERB",    //VerbForm=fin Tense=pres Number=sing Person=three    verb, 3rd person singular present
//    "DET",        //wh-determiner
//    "PRON",        //wh-pronoun, personal
//    "DET",    //Poss=yes    wh-pronoun, possessive
//    "ADV",        //wh-adverb
//    "X",        //unknown
//    "SPACE",
//};


typedef enum POS {
    POS_SYM,
    POS_PUNCT,
    POS_X,
    POS_ADJ,
    POS_SCONJ,
    POS_NUM,
    POS_DET,
    POS_PRON,
    POS_ADP,
    POS_VERB,
    POS_NOUN,
    POS_PROPN,
    POS_PART,
    POS_ADV,
    POS_SPACE,
    POS_INTJ,
    POS_AUX,
    
    POS_COUNT
} POS;

static mtt::String pos_str[] = {
    [POS_SYM]="POS_SYM",
    [POS_PUNCT]="POS_PUNCT",
    [POS_X]="POS_X",
    [POS_ADJ]="POS_ADJ",
    [POS_SCONJ]="POS_SCONJ",
    [POS_NUM]="POS_NUM",
    [POS_DET]="POS_DET",
    [POS_PRON]="POS_PRON",
    [POS_ADP]="POS_ADP",
    [POS_VERB]="POS_VERB",
    [POS_NOUN]="POS_NOUN",
    [POS_PROPN]="POS_PROPN",
    [POS_PART]="POS_PART",
    [POS_ADV]="POS_ADV",
    [POS_SPACE]="POS_SPACE",
    [POS_INTJ]="POS_INTJ",
    [POS_AUX]="POS_AUX",
    
    
};

static mtt::Map<mtt::String, usize> pos2unique_idx = {
    {"SYM",POS_SYM},
    {"PUNCT",POS_PUNCT},
    {"X",POS_X},
    {"ADJ",POS_ADJ},
    {"SCONJ",POS_SCONJ},
    {"NUM",POS_NUM},
    {"DET",POS_DET},
    {"PRON",POS_PRON},
    {"X",POS_X},
    {"ADP",POS_ADP},
    {"VERB",POS_VERB},
    {"AUX",POS_AUX},
    {"NOUN",POS_NOUN},
    {"PROPN",POS_PROPN},
    {"PART",POS_PART},
    {"ADV",POS_ADV},
    {"SPACE",POS_SPACE},
    {"INTJ",POS_INTJ},
};



static mtt::Map<usize, mtt::String> tagidx2pos = {
    {0, "SYM"},        //symbol, currency
    {1, "PUNCT"},    //PunctType=quot PunctSide=ini    opening quotation mark
    {2, "PUNCT"},    //PunctType=quot PunctSide=fin    closing quotation mark
    {3, "PUNCT"},    //PunctType=comm    punctuation mark, comma
    {4, "PUNCT"},    //PunctType=brck PunctSide=ini    left round bracket
    {5, "PUNCT"},    //PunctType=brck PunctSide=fin    right round bracket
    {6, "PUNCT"},    //PunctType=peri    punctuation mark, sentence closer
    {7, "PUNCT"},        //punctuation mark, colon or ellipsis
    {8, "X"},        //email
    {9, "ADJ"},    //Hyph=yes    affix
    {10, "CCONJ"},    //ConjType=comp    conjunction, coordinating
    {11, "NUM"},    //NumType=card    cardinal number
    {12, "DET"},        //determiner
    {13, "PRON"},    //AdvType=ex    existential there
    {14, "X"},    //Foreign=yes    foreign word
    {15, "X"},        //additional word in multi-word expression
    {16, "PUNCT"},    //PunctType=dash    punctuation mark, hyphen
    {17, "ADP"},        //conjunction, subordinating or preposition
    {18, "ADJ"},    //Degree=pos    adjective
    {19, "ADJ"},    //Degree=comp    adjective, comparative
    {20, "ADJ"},    //Degree=sup    adjective, superlative
    {21, "X"},    //NumType=ord    list item marker
    {22, "AUX"},    //VerbType=mod    verb, modal auxiliary
    {23, "PUNCT"},        //superfluous punctuation
    {24, "X"},        //missing tag
    {25, "NOUN"},    //Number=sing    noun, singular or mass
    {26, "PROPN"},    //NounType=prop Number=sing    noun, proper singular
    {27, "PROPN"},    //NounType=prop Number=plur    noun, proper plural
    {28, "NOUN"},    //Number=plur    noun, plural
    {29, "DET"},        //predeterminer
    {30, "PART"},    //Poss=yes    possessive ending
    {31, "PRON"},    //PronType=prs    pronoun, personal
    {32, "DET"},    //PronType=prs Poss=yes    pronoun, possessive
    {33, "ADV"},    //Degree=pos    adverb
    {34, "ADV"},    //Degree=comp    adverb, comparative
    {35, "ADV"},    //Degree=sup    adverb, superlative
    {36, "ADP"},        //adverb, particle
    {37, "SPACE"},        //space
    {38, "SYM"},        //symbol
    {39, "PART"},    //PartType=inf VerbForm=inf    infinitival “to”
    {40, "INTJ"},        //interjection
    {41, "VERB"},    //VerbForm=inf    verb, base form
    {42, "VERB"},    //VerbForm=fin Tense=past    verb, past tense
    {43, "VERB"},    //VerbForm=part Tense=pres Aspect=prog    verb, gerund or present participle
    {44, "VERB"},    //VerbForm=part Tense=past Aspect=perf    verb, past participle
    {45, "VERB"},    //VerbForm=fin Tense=pres    verb, non-3rd person singular present
    {46, "VERB"},    //VerbForm=fin Tense=pres Number=sing Person=three    verb, 3rd person singular present
    {47, "DET"},        //wh-determiner
    {48, "PRON"},        //wh-pronoun, personal
    {49, "DET"},    //Poss=yes    wh-pronoun, possessive
    {50, "ADV"},        //wh-adverb
    {51, "X"},        //unknown
    {52, "SPACE"},
};

//static mtt::Map<usize, usize> tagidx2posidx = {
//    {0, POS_SYM},        //symbol, currency
//    {1, POS_PUNCT},    //PunctType=quot PunctSide=ini    opening quotation mark
//    {2, POS_PUNCT},    //PunctType=quot PunctSide=fin    closing quotation mark
//    {3, POS_PUNCT},    //PunctType=comm    punctuation mark, comma
//    {4, POS_PUNCT},    //PunctType=brck PunctSide=ini    left round bracket
//    {5, POS_PUNCT},    //PunctType=brck PunctSide=fin    right round bracket
//    {6, POS_PUNCT},    //PunctType=peri    punctuation mark, sentence closer
//    {7, POS_PUNCT},        //punctuation mark, colon or ellipsis
//    {8, POS_X},        //email
//    {9, POS_ADJ},    //Hyph=yes    affix
//    {10, POS_CCONJ},    //ConjType=comp    conjunction, coordinating
//    {11, POS_NUM},    //NumType=card    cardinal number
//    {12, POS_DET},        //determiner
//    {13, POS_ADV},    //AdvType=ex    existential there
//    {14, POS_X},    //Foreign=yes    foreign word
//    {15, POS_X},        //additional word in multi-word expression
//    {16, POS_PUNCT},    //PunctType=dash    punctuation mark, hyphen
//    {17, POS_ADP},        //conjunction, subordinating or preposition
//    {18, POS_ADJ},    //Degree=pos    adjective
//    {19, POS_ADJ},    //Degree=comp    adjective, comparative
//    {20, POS_ADJ},    //Degree=sup    adjective, superlative
//    {21, POS_X},    //NumType=ord    list item marker
//    {22, POS_VERB},    //VerbType=mod    verb, modal auxiliary
//    {23, POS_PUNCT},        //superfluous punctuation
//    {24, POS_X},        //missing tag
//    {25, POS_NOUN},    //Number=sing    noun, singular or mass
//    {26, POS_PROPN},    //NounType=prop Number=sing    noun, proper singular
//    {27, POS_PROPN},    //NounType=prop Number=plur    noun, proper plural
//    {28, POS_NOUN},    //Number=plur    noun, plural
//    {29, POS_DET},        //predeterminer
//    {30, POS_PART},    //Poss=yes    possessive ending
//    {31, POS_PRON},    //PronType=prs    pronoun, personal
//    {32, POS_PRON},    //PronType=prs Poss=yes    pronoun, possessive
//    {33, POS_ADV},    //Degree=pos    adverb
//    {34, POS_ADV},    //Degree=comp    adverb, comparative
//    {35, POS_ADV},    //Degree=sup    adverb, superlative
//    {36, POS_ADP},        //adverb, particle
//    {37, POS_SPACE},        //space
//    {38, POS_SYM},        //symbol
//    {39, POS_PART},    //PartType=inf VerbForm=inf    infinitival “to”
//    {40, POS_INTJ},        //interjection
//    {41, POS_VERB},    //VerbForm=inf    verb, base form
//    {42, POS_VERB},    //VerbForm=fin Tense=past    verb, past tense
//    {43, POS_VERB},    //VerbForm=part Tense=pres Aspect=prog    verb, gerund or present participle
//    {44, POS_VERB},    //VerbForm=part Tense=past Aspect=perf    verb, past participle
//    {45, POS_VERB},    //VerbForm=fin Tense=pres    verb, non-3rd person singular present
//    {46, POS_VERB},    //VerbForm=fin Tense=pres Number=sing Person=three    verb, 3rd person singular present
//    {47, POS_DET},        //wh-determiner
//    {48, POS_PRON},        //wh-pronoun, personal
//    {49, POS_DET},    //Poss=yes    wh-pronoun, possessive
//    {50, POS_ADV},        //wh-adverb
//    {51, POS_X},        //unknown
//    {52, POS_SPACE},
//};

typedef enum DEP {
    DEP_acl,    // clausal modifier of noun (adjectival clause)
    DEP_acomp,    // adjectival complement
    DEP_advcl,    // adverbial clause modifier
    DEP_advmod,    // adverbial modifier
    DEP_agent,    // agent
    DEP_amod,    // adjectival modifier
    DEP_appos,    // appositional modifier
    DEP_attr,    // attribute
    DEP_aux,    // auxiliary
    DEP_auxpass,    // auxiliary (passive)
    DEP_case,    // case marking
    DEP_cc,    // coordinating conjunction
    DEP_ccomp,    // clausal complement
    DEP_compound,    // compound
    DEP_conj,    // conjunct
    DEP_cop,    // copula
    DEP_csubj,    // clausal subject
    DEP_csubjpass,    // clausal subject (passive)
    DEP_dative,    // dative
    DEP_dep,    // unclassified dependent
    DEP_det,    // determiner
    DEP_dobj,    // direct object
    DEP_expl,    // expletive
    DEP_intj,    // interjection
    DEP_mark,    // marker
    DEP_meta,   // meta modifier
    DEP_neg,    // negation modifier
    DEP_nn,    // noun compound modifier
    DEP_nounmod,    // modifier of nominal
    DEP_npmod,    // noun phrase as adverbial modifier
    DEP_nsubj,    // nominal subject
    DEP_nsubjpass,    // nominal subject (passive)
    DEP_nummod,    // numeric modifier
    DEP_oprd,    // object predicate
    DEP_obj,    // object
    DEP_obl,    // oblique nominal
    DEP_parataxis,    // parataxis
    DEP_pcomp,    // complement of preposition
    DEP_pobj,    // object of preposition
    DEP_poss,    // possession modifier
    DEP_preconj,    // pre-correlative conjunction
    DEP_prep,    // prepositional modifier
    DEP_prt,    // particle
    DEP_punct,    // punctuation
    DEP_quantmod,    // modifier of quantifier
    DEP_relcl,    // relative clause modifier
    DEP_ROOT,    // root
    DEP_xcomp,    // open clausal complement
    DEP_npadvmod,
    
    DEP_COUNT
} DEP;

static mtt::String dep_str[] = {
    [DEP_acl]   = "acl",    // clausal modifier of noun (adjectival clause)
    [DEP_acomp] = "acomp",    // adjectival complement
    [DEP_advcl] = "advcl",    // adverbial clause modifier
    [DEP_advmod]="advmod",    // adverbial modifier
    [DEP_agent] = "agent",    // agent
    [DEP_amod] = "amod",    // adjectival modifier
    [DEP_appos] = "appos",    // appositional modifier
    [DEP_attr] = "attr",    // attribute
    [DEP_aux] = "aux",    // auxiliary
    [DEP_auxpass] = "auxpass",    // auxiliary (passive)
    [DEP_case] = "case",    // case marking
    [DEP_cc] = "cc",    // coordinating conjunction
    [DEP_ccomp] = "ccomp",    // clausal complement
    [DEP_compound] = "compound",    // compound
    [DEP_conj] = "conj",    // conjunct
    [DEP_cop] = "cop",    // copula
    [DEP_csubj] = "csubj",    // clausal subject
    [DEP_csubjpass] = "csubjpass",    // clausal subject (passive)
    [DEP_dative] = "dative",    // dative
    [DEP_dep] = "dep",    // unclassified dependent
    [DEP_det] = "det",    // determiner
    [DEP_dobj] = "dobj",    // direct object
    [DEP_expl] = "expl",    // expletive
    [DEP_intj] = "intj",    // interjection
    [DEP_mark] = "mark",    // marker
    [DEP_meta] = "meta",   // meta modifier
    [DEP_neg] = "neg",    // negation modifier
    [DEP_nn] = "nn",    // noun compound modifier
    [DEP_nounmod] = "nounmod",    // modifier of nominal
    [DEP_npmod] = "npmod",    // noun phrase as adverbial modifier
    [DEP_nsubj] ="nsubj",    // nominal subject
    [DEP_nsubjpass] = "nsubjpass",    // nominal subject (passive)
    [DEP_nummod] = "nummod",    // numeric modifier
    [DEP_oprd] = "oprd",    // object predicate
    [DEP_obj] = "obj",    // object
    [DEP_obl] = "obl",    // oblique nominal
    [DEP_parataxis] = "parataxis",    // parataxis
    [DEP_pcomp] = "pcomp",    // complement of preposition
    [DEP_pobj] = "pobj",    // object of preposition
    [DEP_poss] = "poss",    // possession modifier
    [DEP_preconj] = "preconj",    // pre-correlative conjunction
    [DEP_prep] = "prep",    // prepositional modifier
    [DEP_prt] = "prt",    // particle
    [DEP_punct] = "punct",    // punctuation
    [DEP_quantmod] = "quantmod",    // modifier of quantifier
    [DEP_relcl] = "relcl",    // relative clause modifier
    [DEP_ROOT] = "ROOT",    // root
    [DEP_xcomp] = "xcomp",    // open clausal complement
    [DEP_npadvmod] = "npadvmod",
};



static mtt::Map<mtt::String, usize> dep2idx = {
    {"acl", DEP_acl},    // clausal modifier of noun (adjectival clause)
    {"acomp",DEP_acomp},    // adjectival complement
    {"advcl",DEP_advcl},    // adverbial clause modifier
    {"advmod",DEP_advmod},    // adverbial modifier
    {"agent",DEP_agent},    // agent
    {"amod",DEP_amod},    // adjectival modifier
    {"appos",DEP_appos},    // appositional modifier
    {"attr",DEP_attr},    // attribute
    {"aux",DEP_aux},    // auxiliary
    {"auxpass",DEP_auxpass},    // auxiliary (passive)
    {"case",DEP_case},    // case marking
    {"cc",DEP_cc},    // coordinating conjunction
    {"ccomp",DEP_ccomp},    // clausal complement
    {"compound",DEP_compound},    // compound
    {"conj",DEP_conj},    // conjunct
    {"cop",DEP_cop},    // copula
    {"csubj",DEP_csubj},    // clausal subject
    {"csubjpass",DEP_csubjpass},    // clausal subject (passive)
    {"dative",DEP_dative},    // dative
    {"dep",DEP_dep},    // unclassified dependent
    {"det",DEP_det},    // determiner
    {"dobj",DEP_dobj},    // direct object
    {"expl",DEP_expl},    // expletive
    {"intj",DEP_intj},    // interjection
    {"mark",DEP_mark},    // marker
    {"meta",DEP_meta},   // meta modifier
    {"neg",DEP_neg},    // negation modifier
    {"nn",DEP_nn},    // noun compound modifier
    {"nounmod",DEP_nounmod},    // modifier of nominal
    {"npmod",DEP_npmod},    // noun phrase as adverbial modifier
    {"nsubj",DEP_nsubj},    // nominal subject
    {"nsubjpass",DEP_nsubjpass},    // nominal subject (passive)
    {"nummod",DEP_nummod},    // numeric modifier
    {"oprd",DEP_oprd},    // object predicate
    {"obj",DEP_obj},    // object
    {"obl",DEP_obl},    // oblique nominal
    {"parataxis",DEP_parataxis},    // parataxis
    {"pcomp",DEP_pcomp},    // complement of preposition
    {"pobj",DEP_pobj},    // object of preposition
    {"poss",DEP_poss},    // possession modifier
    {"preconj",DEP_preconj},    // pre-correlative conjunction
    {"prep",DEP_prep},    // prepositional modifier
    {"prt",DEP_prt},    // particle
    {"punct",DEP_punct},    // punctuation
    {"quantmod",DEP_quantmod},    // modifier of quantifier
    {"relcl",DEP_relcl},    // relative clause modifier
    {"ROOT",DEP_ROOT},    // root
    {"xcomp",DEP_xcomp},    // open clausal complement
    {"npadvmod", DEP_npadvmod}
};



}

namespace propbank_nlp {

typedef enum SRL_LABEL {
    SRL_LABEL_ARG0, // agent
    SRL_LABEL_ARG1, // patient
    SRL_LABEL_C_ARG1,
    SRL_LABEL_ARG2, // instrument, benefactive, attribute
    SRL_LABEL_ARG3, // starting point, benefactive, attribute
    SRL_LABEL_ARG4, // ending point
    
    SRL_LABEL_ARGA, // Secondary Agent
        
        
    SRL_LABEL_ARGM_COM, // Comitative
    SRL_LABEL_ARGM_LOC, // Locative
    SRL_LABEL_ARGM_DIR, // Directional
    SRL_LABEL_ARGM_GOL, // Goal
    SRL_LABEL_ARGM_MNR, // Manner
    SRL_LABEL_ARGM_TMP, // Temporal
    SRL_LABEL_ARGM_EXT, // Extent
    SRL_LABEL_ARGM_REC, // Reciprocals
    SRL_LABEL_ARGM_PRD, // Secondary Predication
    SRL_LABEL_ARGM_PRP, // Purpose
    SRL_LABEL_ARGM_CAU, // Cause
    SRL_LABEL_ARGM_DIS, // Discourse
    SRL_LABEL_ARGM_ADV, // Adverbials
    SRL_LABEL_ARGM_ADJ, // Adjectival
    SRL_LABEL_ARGM_MOD, // Modal
    SRL_LABEL_ARGM_NEG, // Negation
    SRL_LABEL_ARGM_DSP, // Direct Speech
    SRL_LABEL_ARGM_LVB, // Light Verb
    SRL_LABEL_ARGM_CXN, // Construction
    
    SRL_LABEL_REL,
    
    SRL_LABEL_ARGM_SLC, // ??? Old
    SRL_LABEL_ARGM_PCR, // ??? Old

    SRL_LABEL_COUNT
} SRL_LABEL;


static mtt::String label_str[] = {
    "ARG0", // agent
    "ARG1", // patient
    "C-ARG1",
    "ARG2", // instrument, benefactive, attribute
    "ARG3", // starting point, benefactive, attribute
    "ARG4", // ending point
    
    "ARGA", // secondary agent
    
    
    "ARGM-COM", // Comitative
    "ARGM-LOC", // Locative
    "ARGM-DIR", // Directional
    "ARGM-GOL", // Goal
    "ARGM-MNR", // Manner
    "ARGM-TMP", // Tempora;
    "ARGM-EXT", // Extent
    "ARGM-REC", // Reciprocals
    "ARGM-PRD", // Secondary Predication
    "ARGM-PRP", // Purpose
    "ARGM-CAU", // Cause
    "ARGM-DIS", // Discourse
    "ARGM-ADV", // Adverbials
    "ARGM-ADJ", // Adjectival
    "ARGM-MOD", // Modal
    "ARGM-NEG", // Negation
    "ARGM-DSP", // Direct Speech
    "ARGM-LVB", // Light Verb
    "ARGM-CXN", // Construction
    
    "V", // predicate / relation (main verb)
    
    "ARGM-SLC", // Relative Clause ???
    "ARGM-PCR", // ???
};

static mtt::Map<mtt::String, usize> label2idx = {
    {"ARG0",SRL_LABEL_ARG0}, // agent
    {"ARG1",SRL_LABEL_ARG1}, // patient
    {"C-ARG1", SRL_LABEL_C_ARG1},
    {"ARG2",SRL_LABEL_ARG2}, // instrument, benefactive, attribute
    {"ARG3",SRL_LABEL_ARG3}, // starting point, benefactive, attribute
    {"ARG4",SRL_LABEL_ARG4}, // ending point
    
    {"ARGA",SRL_LABEL_ARGA},
        
        
    {"ARGM-COM",SRL_LABEL_ARGM_COM}, // Comitative
    {"ARGM-LOC",SRL_LABEL_ARGM_LOC}, // Locative
    {"ARGM-DIR",SRL_LABEL_ARGM_DIR}, // Directional
    {"ARGM-GOL",SRL_LABEL_ARGM_GOL}, // Goal
    {"ARGM-MNR",SRL_LABEL_ARGM_MNR}, // Manner
    {"ARGM-TMP",SRL_LABEL_ARGM_TMP}, // Temporal
    {"ARGM-EXT",SRL_LABEL_ARGM_EXT}, // Extent
    {"ARGM-REC",SRL_LABEL_ARGM_REC}, // Reciprocals
    {"ARGM-PRD",SRL_LABEL_ARGM_PRD}, // Secondary Predication
    {"ARGM-PRP",SRL_LABEL_ARGM_PRP}, // Purpose
    {"ARGM-CAU",SRL_LABEL_ARGM_CAU}, // Cause
    {"ARGM-DIS",SRL_LABEL_ARGM_DIS}, // Discourse
    {"ARGM-ADV",SRL_LABEL_ARGM_ADV}, // Adverbials
    {"ARGM-ADJ",SRL_LABEL_ARGM_ADJ}, // Adjectival
    {"ARGM-MOD",SRL_LABEL_ARGM_MOD}, // Modal
    {"ARGM-NEG",SRL_LABEL_ARGM_NEG}, // Negation
    {"ARGM-DSP",SRL_LABEL_ARGM_DSP}, // Direct Speech
    {"ARGM-LVB",SRL_LABEL_ARGM_LVB}, // Light Verb
    {"ARGM-CXN",SRL_LABEL_ARGM_CXN}, // Construction
    
    {"V", SRL_LABEL_REL}, // predicate / relation (main verb)
    
    {"ARGM-SLC",SRL_LABEL_ARGM_SLC}, // ??? Old
    {"ARGM-PCR",SRL_LABEL_ARGM_PCR}, // ??? Old
};

struct SRL_Entry {
    dt::ID id;
    dt::ID label_id;
};

}

static mtt::Map<mtt::String, mtt::Map<mtt::String, mtt::String>> sense_to_roleset_id = {
    {"jump", {
        {"physical", "jump.03"},
        {"escape", "jump.06"},
    }},
};
static mtt::Map<mtt::String, mtt::Map<mtt::String, mtt::String>> roleset_id_to_sense = {
    {"jump", {
        {"jump.03", "physical"},
        {"jump.06", "escape"},
    }},
};

/*
namespace cog_comp_nlp {

typedef enum LABEL {
    LABEL_COUNT
} LABEL;

static mtt::String label_str[] = {
    "A0", // subject
    "A1", // object
    "A2", // indirect object
    "A3",
    "A4",
    "C-arg", // continuity of an argument/adjuct of type arg
    "R-arg", // reference to an actual argument/adjunct of type arg
    
    "AM-ADV", // adverbial
    "AM-DIR", // direction
    "AM-DIS", // discourse
    "AM-EXT", // extent
    "AM-LOC", // location
    "AM-MNR", // manner
    "AM-MOD", // general modification
    "AM-NEG", // negation
    "AM-PNC", // proper noun component
    "AM-PRD", // secondary predicate
    "AM-PRP", // purpose
    "AM-REC", // reciprocal
    "AM-TMP", // temporal
};

static mtt::Map<mtt::String, usize> label2idx = {
    {"A0",0}, // subject
    {"A1",1}, // object
    {"A2",2}, // indirect object
    {"A3",3},
    {"A4",4},
    {"C-arg",5}, // continuity of an argument/adjuct of type arg
    {"R-arg",6}, // reference to an actual argument/adjunct of type arg
        
    {"AM-ADV",7}, // adverbial
    {"AM-DIR",8}, // direction
    {"AM-DIS",9}, // discourse
    {"AM-EXT",10}, // extent
    {"AM-LOC",11}, // location
    {"AM-MNR",12}, // manner
    {"AM-MOD",13}, // general modification
    {"AM-NEG",14}, // negation
    {"AM-PNC",15}, // proper noun component
    {"AM-PRD",16}, // secondary predicate
    {"AM-PRP",17}, // purpose
    {"AM-REC",18}, // reciprocal
    {"AM-TMP",19}, // temporal
};

struct SRL_Entry {
    dt::ID id;
    dt::ID label_id;
};

}
*/

/*
namespace srl {

typedef enum LABEL {
    LABEL_A0,
    LABEL_A1,
    LABEL_A2,
    LABEL_A3,
    LABEL_A4,
    LABEL_COUNT
} LABEL;


static mtt::String label_str[] = {
    "LABEL_A0",
    "LABEL_A1",
    "LABEL_A2",
    "LABEL_A3",
    "LABEL_A4",
};

}
*/

template<typename T>
using Dynamic_Array = std::vector<T>;

// possibly update
struct Timing {
    float64 utterance_timestamp;
    Selection_Recording_State selection;
    Selection_Recording_State alt_selection;
    bool was_selected;
    bool was_selected_alt;
    
    Timing() :
    utterance_timestamp(0),
    selection({}),
    alt_selection({}),
    was_selected(false),
    was_selected_alt(false)
    {}
};


struct Speech_Token;

struct Coreference {
    // direct pointers to tokens
    dt::Dynamic_Array<Speech_Token*> token_ref_list;
    
    Coreference() {}
};

struct Parse;
    
struct Speech_Token {
    dt::ID id;
    dt::ID i;
    spacy_nlp::POS pos;
    spacy_nlp::TAG tag;
    spacy_nlp::DEP dep;
    dt::ID head;
    mtt::String text;
    mtt::String lemma;
    
    Dynamic_Array<dt::ID> ancestors;
    Dynamic_Array<dt::ID> left_children;
    Dynamic_Array<dt::ID> right_children;

    Coreference coreference;
    
    mtt::Map<mtt::String, mtt::String> morph;
    
    Word_Info info;
    
    Timing timing;
    bool timing_taken_from_reference;
    
    bool is_blocked;
    
    bool force_not_a_type = false;
    
    Speech_Property* prop_ref = nullptr;
    
    dt::Parse* parse;
    
    bool mark_for_deletion;
    
    mtt::Set_Stable<mtt::Thing_ID> labeled_things;
    
    
    Speech_Token() :
    id(0), i(0), pos((spacy_nlp::POS)0), tag((spacy_nlp::TAG)0),
    dep((spacy_nlp::DEP)0), head(0), text(""), lemma(""),
    coreference(Coreference()), info({}), timing({}),
    timing_taken_from_reference(false), is_blocked(false), parse(nullptr), mark_for_deletion(false)
    {}
};

Speech_Property* Speech_Token_get_prop(Speech_Token* token);
void Speech_Token_set_prop(Speech_Token* token, Speech_Property* prop);

typedef enum BIO_TAG {
    BIO_TAG_B,
    BIO_TAG_I,
    BIO_TAG_O,
    
    BIO_TAG_COUNT
} BIO_TAG;

static const char* const bio_str[] = {
    "B",
    "I",
    "O"
};

struct Parse;

namespace srl {

struct Tag_Entry {
    dt::ID id;
    dt::ID label_id;
    uint32 bio_tag;
};


void Tag_Entry_print(Parse* parse, srl::Tag_Entry* entry);

struct Role_Entry {
    dt::ID id;
    dt::ID label_id;
    std::vector<Tag_Entry> tags;
    //std::vector<dt::ID> roles_range;
    
    Role_Entry* next;
    
    void* user_data;
    uint64 flags;
};

void Role_Entry_print(Parse* parse, srl::Role_Entry* entry);

struct Relation {
    dt::ID id; // index
    
    //std::vector<srl::Tag_Entry>  tags;
    std::vector<srl::Role_Entry> roles;
    std::vector<srl::Role_Entry> sorted_roles;
    srl::Role_Entry* first_role;
    
    dt::ID verb_id;
    srl::Role_Entry* verb_role;
    
    mtt::String lemma;
    mtt::String frame;
    
    Relation* next;
};
void Relation_print(Parse* parse, Relation* rel);

}

#define IF_COND(condition) (condition) ? (
#define ELSEIF_COND(condition) ) : (condition) ? (
#define ELSE_COND ) : (
#define ENDIF_COND )

struct verb_sort
{
    verb_sort(const std::vector<Speech_Token>& tokens, const dt::ID nsubj_dep_id) :
        tokens(tokens),
        nsubj_dep_id(nsubj_dep_id)
    {}
    
    inline bool operator() (const dt::ID& a, const dt::ID& b)
    {
        const Speech_Token* const t_a = &tokens[a];
        const Speech_Token* const t_b = &tokens[b];
        
        
//        return
//        IF_COND(t_a->dep == nsubj_dep_id)
//            IF_COND(t_b->dep == nsubj_dep_id)
//                (t_a->id <= t_b->id)
//            ELSE_COND
//                (true)
//            ENDIF_COND
//        ELSEIF_COND(t_b->dep == nsubj_dep_id)
//            (false)
//        ELSE_COND
//            (t_a->id <= t_b->id)
//        ENDIF_COND;
        
        return
        (t_a->dep == nsubj_dep_id) ? (
            (t_b->dep == nsubj_dep_id) ?
                (t_a->i <= t_b->i)
            :
                (true)
        ) : ((t_b->dep == nsubj_dep_id) ?
                (false)
            :
                (t_a->i <= t_b->i)
        );
    }
    
private:
    const std::vector<Speech_Token>& tokens;
    const dt::ID nsubj_dep_id;
};

struct Nominal_Entity_Record {
    dt::ID token_idx;
    dt::ID list_idx;
    bool should_visualize;
};
struct Nominal_Entity_Collection {
    dt::Dynamic_Array<Nominal_Entity_Record> noms;
    mtt::Map<dt::ID, dt::ID> token_idx_to_nominal_idx;
};

struct Parse_Node {
    uint64 temp;
};





void Token_print(Parse* parse, Speech_Token* token);




struct Coref {
    
};

struct Parse {
    dt::Dynamic_Array<Speech_Token*> token_list;
    usize root_dependency_token_ID = 0;
    
    dt::Dynamic_Array<srl::Relation> srl;
    
    dt::Dynamic_Array<dt::ID> verbs;
    
    //Nominal_Entity_Collection nominals;
    
    inline usize token_count() const {
        return this->token_list.size();
    }
    
    dt::Speech_Token* dependency_tree = nullptr;
    
    mtt::String text = "";
    
    mtt::String pretty_coref_representation = "";
    
    usize ID = 0;
    bool is_finished_ = false;
    void is_finished(bool val)
    {
        is_finished_ = val;
    }
    bool is_finished(void)
    {
        return is_finished_;
    }
    
    dt::Speech_Property* cmds = nullptr;
    
    bool is_discarded_ = false;
    void is_discarded(bool val)
    {
        is_discarded_ = val;
    }
    bool is_discarded(void)
    {
        return is_discarded_;
    }
    
    
    Speech_Token& get_at(usize idx)
    {
        return *this->token_list[idx];
    }
    
    
    bool was_processed = false;
    //std::vector<Word_Info*> eval_list;
};

static inline void Parse_init_token_count(dt::Parse* parse, usize count)
{
    parse->token_list.resize(count);
}


static mtt::Set<mtt::String> naming_words = {"name", "call", "dub", "label", "title", "entitle", "term"};

static mtt::Map<mtt::String, float32> magnitude_words = {
    {"too",   3.0f},
    {"overly",3.0f},
    {"excessively",     3.0f},
    {"extraordinarily", 2.6f},
    {"exceedingly",     2.6f},
    {"contagiously",    2.2f},
    {"completely", 2.0f},
    {"absolutely", 2.0f},
    {"entirely",   2.0f},
    {"fiercely",  1.96f},
    {"extremely",  1.9f},
    {"super",      1.8f},
    {"really",     1.8f},
    {"very",       1.8f},
    {"abundantly", 1.8f},
    {"fairly",     1.7f},
    {"moderately", 1.0f},
    {"somewhat",   0.45f},
    {"slightly",   0.29f},
    {"barely",     0.09f},
    {"marginally", 0.09f},
    {"doubtfully", 0.01f},
    {"__DEFAULT__", 1.0f},
};

static mtt::Map<mtt::String, float32> speed_words {
    {"fiercely",      2.5f},
    {"strenuously",   2.4f},
    {"energetically", 2.0f},
    {"hyperactively", 2.0f},
    {"swiftly",       2.0f},
    {"spryly",        1.9f},
    {"excitedly",     1.9f},
    {"speedily",      1.5f},
    {"quickly",       1.5f},
    {"quick",         1.5f},
    {"fast",          1.5f},
    {"happily",       1.5f},
    {"slow",          0.5f},
    {"slowly",        0.5f},
    {"sluggishly",    0.5f},
    {"unhappily",     0.5f},
    {"sadly",         0.5f},
    {"sluggish",      0.5f},
    {"lethargically", 0.1f},
    {"tiredly",       0.1f},
    {"exhaustedly",   0.05f},
    {"laboriously",   0.05f},
    {"antigravitationally", 0.01f},
};
static mtt::Map<mtt::String, float32> size_words {
    {"gigantic",      2.0f},
    {"giant",         2.0f},
    {"large",         1.5f},
    {"big",           1.5f},
    {"small",         0.5f},
    {"tiny",          0.25f},
    {"miniscule",     0.25f},
};
static mtt::Map<mtt::String, float32> height_words {
    {"high", 2.0f},
    {"low",  0.25f},
};

static mtt::Map<mtt::String, float32> distance_words {
    {"far",    2.0f},
    {"away",   2.0f},
    {"near",   0.25f},
    {"nearby", 0.25f},
    {"close",  0.25f},
};

static mtt::Map<mtt::String, float32> accuracy_words {
    {"accurately", 1.0f},
    {"accurate", 1.0f},
    {"precisely", 1.0f},
    {"precise", 1.0f},
    {"inaccurately", 1.0f},
    {"inaccurate", 1.0f},
    {"imprecisely", 0.0f},
    {"imprecise", 0.0f},
};


static mtt::Map<mtt::String, mtt::Map<mtt::String, float32>*> key_to_type_words = {
    {"magnitude", &magnitude_words},
    {"speed", &speed_words},
    {"size", &size_words},
    {"height", &height_words},
    {"distance", &distance_words},
    {"accuracy", &accuracy_words},
};

static inline bool type_words_lookup(const mtt::String& key, const mtt::String& value_key, float32* out)
{
    auto find_it = key_to_type_words.find(key);
    if (find_it == key_to_type_words.end()) {
        return false;
    }
    auto& map = *find_it->second;
    auto find_sub = map.find(value_key);
    if (find_sub == map.end()) {
        return false;
    }
    
    *out = find_sub->second;
    
    return true;
}

static inline bool magnitude_values_lookup(const mtt::String& key, float32* out)
{
    auto find_it = magnitude_words.find(key);
    if (find_it == magnitude_words.end()) {
        return false;
    }
    
    *out = find_it->second;
    
    return true;
}
static inline bool size_values_lookup(const mtt::String& key, float32* out)
{
    auto find_it = size_words.find(key);
    if (find_it == size_words.end()) {
        return false;
    }
    
    *out = find_it->second;
    
    return true;
}
static inline bool speed_values_lookup(const mtt::String& key, float32* out)
{
    auto find_it = speed_words.find(key);
    if (find_it == speed_words.end()) {
        return false;
    }
    
    *out = find_it->second;
    
    return true;
}

static inline bool height_values_lookup(const mtt::String& key, float32* out)
{
    auto find_it = height_words.find(key);
    if (find_it == height_words.end()) {
        return false;
    }
    
    *out = find_it->second;
    
    return true;
}

static inline bool distance_values_lookup(const mtt::String& key, float32* out)
{
    auto find_it = distance_words.find(key);
    if (find_it == distance_words.end()) {
        return false;
    }
    
    *out = find_it->second;
    
    return true;
}

static inline float32 accumulate_word_values(float32 current_value)
{
    return current_value * current_value;
}

static inline bool accuracy_values_lookup(const mtt::String& key, float32* out)
{
    auto find_it = accuracy_words.find(key);
    if (find_it == accuracy_words.end()) {
        return false;
    }
    
    *out = find_it->second;
    
    return true;
}

}

#endif /* drawtalk_language_analysis_h */
