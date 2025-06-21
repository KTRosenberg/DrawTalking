//
//  drawtalk_language_analysis.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/17/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "drawtalk.hpp"
#include "drawtalk_language_analysis.h"

namespace dt {

Speech_Property* Speech_Token_get_prop(Speech_Token* token)
{
    return token->prop_ref;
}


void Speech_Token_set_prop(Speech_Token* token, Speech_Property* prop)
{
    token->prop_ref = prop;
}

namespace spacy_nlp {

bool pos_tag_is_noun_like(dt::ID tag)
{
    return ((tag >= spacy_nlp::PART_OF_SPEECH_NOMINAL_TAG_FIRST &&
             tag <= spacy_nlp::PART_OF_SPEECH_NOMINAL_TAG_LAST) /* || tag == spacy_nlp::PART_OF_SPEECH_GERUND_TAG *//* VBG */);
}

bool pos_tag_noun_is_singular_or_mass(dt::ID tag)
{
    return
    (tag == PART_OF_SPEECH_NOUN_SINGULAR_OR_MASS) ||
    (tag == PART_OF_SPEECH_NOUN_PROPER_SINGULAR);
}

bool pos_tag_noun_is_plural(dt::ID tag)
{
    return
    (tag == PART_OF_SPEECH_NOUN_PLURAL) ||
    (tag == PART_OF_SPEECH_NOUN_PROPER_PLURAL);
}

bool pos_tag_noun_is_proper(dt::ID tag)
{
    return
    (tag == PART_OF_SPEECH_NOUN_PROPER_SINGULAR) ||
    (tag == PART_OF_SPEECH_NOUN_PROPER_PLURAL);
}

bool pos_tag_is_gerund(dt::ID tag)
{
    return (tag == PART_OF_SPEECH_GERUND_TAG);
}

}

namespace srl {

void Tag_Entry_print(Parse* parse, srl::Tag_Entry* entry)
{
    MTT_print("(Tag_Entry){\n\t"
              "      id:[%llu]:[%s]\n\t"
              "label_id:[%s]\n\t"
              " bio_tag:[%s]\n"
              "}\n",
              entry->id,
              parse->token_list[entry->id]->text.c_str(),
              propbank_nlp::label_str[entry->label_id].c_str(),
              bio_str[entry->bio_tag]
              );
}


void Role_Entry_print(Parse* parse, srl::Role_Entry* entry)
{
    MTT_print("(Role_Entry){\n\t"
              "      id:[%llu]\n\t"
              "label_id:[%s]\n\t",
              
              entry->id,
              propbank_nlp::label_str[entry->label_id].c_str()
              );
    
    MTT_print("%s", "[\n\t");
    auto& tags = entry->tags;
    for (usize i = 0; i < tags.size(); i += 1) {
        MTT_print("\t[%llu]:[%s]\n\t", tags[i].id, parse->token_list[entry->tags[i].id]->text.c_str());
    }
    MTT_print("%s", "]\n}\n");
}


void Relation_print(Parse* parse, srl::Relation* rel)
{
    MTT_print("(Relation){\n\t"
              "     id:[%llu]\n\t"
              "verb_id:[%llu]\n\t"
              "  lemma:[%s]\n\t"
              "  frame:[%s]\n\t"
              "}\n",
              rel->id,
              rel->verb_id,
              rel->lemma.c_str(),
              rel->frame.c_str()
              );
}

}

void Token_print(Parse* parse, Speech_Token* token)
{
    if (parse == nullptr) {
        MTT_print(
                  "(Token){\n\t"
                  "   id:[%llu]\n\t"
                  " text:[%s]\n\t"
                  "lemma:[%s]\n\t"
                  "  tag:[%s]\n\t"
                  "  pos:[%s]\n\t"
                  "  dep:[%s]\n\t"
                  " head:[%llu]\n"
                  "}\n",
                  token->id,
                  token->text.c_str(),
                  token->lemma.c_str(),
                  spacy_nlp::tag_str[token->tag].c_str(),
                  spacy_nlp::pos_str[token->pos].c_str(),
                  spacy_nlp::dep_str[token->dep].c_str(),
                  
                  token->head
                  );
    } else {
        MTT_print(
                  "(Token){\n\t"
                  "   id:[%llu]\n\t"
                  " text:[%s]\n\t"
                  "lemma:[%s]\n\t"
                  "  tag:[%s]\n\t"
                  "  pos:[%s]\n\t"
                  "  dep:[%s]\n\t"
                  " head:[%s : %llu]\n"
                  "}\n",
                  token->id,
                  token->text.c_str(),
                  token->lemma.c_str(),
                  spacy_nlp::tag_str[token->tag].c_str(),
                  spacy_nlp::pos_str[token->pos].c_str(),
                  spacy_nlp::dep_str[token->dep].c_str(),
                  parse->token_list[token->head]->text.c_str(),
                  token->head
                  );
    }
}


}
