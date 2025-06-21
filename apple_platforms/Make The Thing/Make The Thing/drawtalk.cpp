//
//  drawtalk.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/9/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "drawtalk.hpp"

#include "render_layer_labels.hpp"

//#include "nanovg.h"
//#include "nanovg_test_drawing.hpp"

#include "drawtalk_world.hpp"

#include "job_dispatch_platform.hpp"

//#include "cpp_common.hpp"

#define DT_OLD_NESTING_WITH_SELECT (false)

#include "number_words_to_values.hpp"

#include "drawtalk_run.hpp"
#include "drawtalk_run_new.hpp"

//#include "standard_behaviors.hpp"

#include "standard_actions.hpp"

#include "string_intern.hpp"

#include "regular_expr.hpp"

#define DT_CLEAR_SELECTIONS_ON_BUTTON (0)


namespace dt {


std::vector<robin_hood::pair<mtt::Regex_Record, mtt::String>> string_replacements = {
    {mtt::regex_make("×"), " * "},
    {mtt::regex_make(","), ""},
};

void replace_patterns_default(mtt::String& input)
{
    mtt::replace_patterns_in_place(input, string_replacements);
}

MTT_String_Ref dt_Instruction_label;

usize Instruction::count = 0;

// TODO: this may be temporary until new features can interpret these words and phrases in a useful way
namespace block_or_replace {

mtt::Map<mtt::String, mtt::Set<spacy_nlp::POS>> lemma_to_pos_set;

std::vector<
std::vector<
robin_hood::pair<mtt::String, spacy_nlp::POS>
>
> blocked_phrases = {
    {
        {"by",  spacy_nlp::POS_ADP}, {"the", spacy_nlp::POS_DET}, {"way", spacy_nlp::POS_NOUN }
    },
};


bool is_accepted_token(mtt::String& lemma, spacy_nlp::POS pos)
{
    
    auto it_lemma = lemma_to_pos_set.find(lemma);
    if (it_lemma == lemma_to_pos_set.end()) {
        return true;
    }
    
    auto it_pos = it_lemma->second.find(pos);
    if (it_pos == it_lemma->second.end()) {
        return true;
    }
    
    return false;
}


void mark_unaccepted_subsequences(std::vector<dt::Speech_Token*>& tokens)
{
    
    struct Match {
        usize i;
        usize j_exclusive;
    };
    for (usize b = 0; b < blocked_phrases.size(); b += 1) {
        auto& phrase = blocked_phrases[b];
        if (tokens.size() - 1 < phrase.size()) {
            continue;
        }
        
        Match match = {0, 0};
        for (usize i = 0; i < tokens.size() - 1; i += 1) {
            if (i + phrase.size() > tokens.size() - 1) {
                break;
            }
            
            bool had_match = true;;
            
            usize btok_i = 0;
            for (; btok_i < phrase.size(); btok_i += 1) {
                auto& btok = phrase[btok_i];
                if (btok.first == tokens[i + btok_i]->lemma && btok.second == tokens[i + btok_i]->pos) {
                    continue;
                }
                
                had_match = false;
                break;
            }
            if (!had_match) {
                match.i += 1;
            } else {
                match.j_exclusive = match.i + btok_i;
                
                for (usize ti = match.i; ti < match.j_exclusive; ti += 1) {
                    tokens[ti]->is_blocked = true;
                }
                
                match.i += 1;
            }
        }
    }
}
};

namespace fmt {

Formatter* ctx;

}


template <typename K, typename V>
mtt::Result<V> map_find(mtt::Map<K, V>* map, const K& key)
{
    auto res = map->find(key);
    if (res != map->end()) {
        return {
            .status = true,
            .value = res->second
        };
    }
    
    return {
        .status = false
    };
}

template <typename V>
mtt::Result<V> map_find(mtt::Map<std::string, V>* map, std::string_view& key)
{
    auto res = map->find(std::string(key));
    if (res != map->end()) {
        return {
            .status = true,
            .value = res->second
        };
    }
    
    return {
        .status = false
    };
}


template <typename T>
void copy_from_JSON_Array(Dynamic_Array<T>& arr, JsonArrayConst& json)
{
    usize in_count = 0;
    for (auto it = json.begin(); it != json.end(); ++it) {
        in_count += 1;
    }
    arr.resize(in_count);
    
    usize i = 0;
    for (auto it = json.begin(); it != json.end(); ++it, i += 1) {
        arr[i] = it->as<T>();
    }
}

template <typename T>
void copy_from_JSON_Array(Dynamic_Array<T>& arr, JsonArray& json)
{
    usize in_count = 0;
    for (auto it = json.begin(); it != json.end(); ++it) {
        in_count += 1;
    }
    arr.resize(in_count);
    
    usize i = 0;
    for (auto it = json.begin(); it != json.end(); ++it, i += 1) {
        arr[i] = it->as<T>();
    }
}

void Instruction_add_child(Instruction* self, Instruction* child, Instruction_CHILD_SIDE_TYPE ch_type)
{
    self->children[ch_type].push_back(child);
    child->child_side_type = ch_type;
    child->parent = self;
}

mtt::Thing* Instruction_get_proxy_for_thing(Instruction* ins, mtt::World* world, mtt::Thing_ID thing_id)
{
    auto& map = ins->thing_proxies;
    mtt::Thing_Proxy_ID* proxy_id = nullptr;
    if (mtt::map_try_get(&map, thing_id, &proxy_id)) {
        mtt::Thing* proxy_out = mtt::Thing_try_get(world, *proxy_id);
        return proxy_out;
    }
    return nullptr;
}

void Instruction_set_proxy_for_thing(Instruction* ins, mtt::World* world, mtt::Thing_ID thing_id, mtt::Thing_ID proxy_id)
{
    mtt::map_set(ins->thing_proxies, thing_id, proxy_id);
}

void Instruction_destroy_proxy_for_thing(Instruction* ins, mtt::World* world, mtt::Thing_ID thing_id)
{
    auto& map = ins->thing_proxies;
    mtt::Thing_Proxy_ID* proxy_id_ptr = nullptr;
    if (mtt::map_try_get(&map, thing_id, &proxy_id_ptr)) {
        mtt::Thing_Proxy_ID proxy_id = *proxy_id_ptr;
        mtt::map_erase(map, thing_id);
        mtt::Thing_destroy(world, proxy_id);
    }
    
}


void push_speech_event(MTT_Core* core, mtt::World* world, DrawTalk* dt, Speech_Event* ev, Speech_Info_View* sp_view)
{
    
    Speech_System* sys = &core->speech_system;
    
//    if (dt->dt_text_view.main_display != nullptr) {
//        if (sp_view->text_info->text.size() == 0) {
//            mtt::text_set(dt->dt_text_view.main_display, "   ");
//        } else {
//            mtt::text_set(dt->dt_text_view.main_display, sp_view->text_info->text.c_str());
//        }
//    }
    
}

void text_view_clear(DrawTalk* dt)
{
    dt->ui.margin_panels[0].text.clear();
    //dt->ui.margin_panels[0].pen_selections.clear();
    dt->ui.margin_panels[0].is_selected_by_touch = false;
    dt->ui.margin_panels[0].is_selected_by_pen = false;
    dt->ui.margin_panels[0].is_selected_by_touch_prev = false;
    dt->ui.margin_panels[0].is_selected_by_pen_prev = false;
    for (auto it = dt->ui.margin_panels[0].text.text.begin(); it != dt->ui.margin_panels[0].text.text.end(); ++it) {
        (*it).is_selected = false;
    }
    
    
    dt->ui.margin_panels[0].text.set_is_overriding_speech(false);
    dt->ui.margin_panels[0].text.reset();
    
    
    
    
    debug_msg.clear();
}



void push_language_event(MTT_Core* core, mtt::World* world, DrawTalk* dt, Natural_Language_Event* ev, Speech_Info_View* sp_view)
{
    Speech_System* sys = &core->speech_system;
    
    //ASSERT_MSG(mtt::is_main_thread(), "Should be on main thread\n");
    //std::cout << std::endl << sp_view->nl_info->doc->data << std::endl << std::endl;
    
//    mtt::String to_print = "";
//    serializeJsonPretty(sp_view->nl_info->doc->data, to_print);
//    std::cout << to_print << std::endl;
    
    
    //#define HARD_CODED_TEST
    
    
    auto* bg_data_context = mem::alloc_init<BG_Data_Context>(&dt->bg_ctx_data.allocator);
    bg_data_context->dt = dt;
    //bg_data_context->sp_view = *sp_view;
    //bg_data_context->world = world;
    bg_data_context->selection_recording = dt->recorder.selection_recording;
    bg_data_context->parse(mem::alloc_init<Parse>(&dt->parse_allocation.allocator));
    bg_data_context->nl_data(sp_view->nl_info->doc);
    bg_data_context->nl_data_idx(ev->idx);
    //std::cout << bg_data_context->nl_data->data << std::endl;


    auto dispatch_proc = [](void* j_ctx) {
        
        auto* context = (BG_Data_Context*)j_ctx;
        //auto* sp_view = &context->sp_view;
        auto* dt = context->dt;
        //auto* world = context->world;
        auto& in = *(context->nl_data()->doc_queue.begin() + (context->nl_data_idx_));
        
//        constexpr const bool do_print = true;
//        constexpr const bool get_srl_from_data = false;
        
        dt::ID ID = in["ID"].as<dt::ID>();
        auto doc  = in["doc"];
        
        
        
        auto tokens = doc["tokens"].as<JsonArrayConst>();
        usize token_count = 0;
        for (auto it = tokens.begin(); it != tokens.end(); ++it) {
            token_count += 1;
        }
        Language_Context& ctx = dt->lang_ctx;
        //auto& parse = *(ctx.parse_q.emplace_back(mem::alloc_init<Parse>(&world->allocator)));
        auto& parse = *(context->parse());
        parse.token_list.resize(token_count);
        
        parse.text = mtt::String(doc["text"]);
//        std::cout << " <<<<<<<<<<<<<<<<<<<<<<<<< " << parse.text << std::endl;
        
        parse.ID = ID;
        
        
        for (usize i = 0; i < parse.token_list.size(); i += 1) {
            parse.token_list[i] = mem::alloc_init<Speech_Token>(&dt->token_allocation.allocator);
            parse.token_list[i]->parse = &parse;
        }
                
        {
            usize i = 0;
            for (auto it = tokens.begin(); it != tokens.end(); ++it, i += 1) {
                auto tok = it->as<JsonObjectConst>();
                
                Speech_Token& token = *parse.token_list[i];
                
                token.id = tok["id"].as<dt::ID>();
                token.i = tok["i"].as<dt::ID>();
                
                token.tag = (spacy_nlp::TAG)map_find<std::string, dt::ID>(&spacy_nlp::tag2idx,  tok["tag"]).value;
                token.pos = (spacy_nlp::POS)map_find<std::string, dt::ID>(&spacy_nlp::pos2unique_idx, tok["pos"]).value;
                token.dep = (spacy_nlp::DEP)map_find<std::string, dt::ID>(&spacy_nlp::dep2idx,  tok["dep"]).value;
                
                //            if (token.dep == spacy_nlp::DEP_ROOT) {
                //                parse.root_dependency_token_ID = i;
                //            }
                
                token.head = tok["head"].as<uint64>();
                token.text = std::string(tok["text"]);
                token.lemma = std::string(tok["lemma"]);
                
                {
                    auto edges = tok["ancestors"].as<JsonArrayConst>();
                    
                    copy_from_JSON_Array(token.ancestors, edges);
                }
                {
                    auto edges = tok["lefts"].as<JsonArrayConst>();
                    
                    copy_from_JSON_Array(token.left_children, edges);
                }
                {
                    auto edges = tok["rights"].as<JsonArrayConst>();
                    
                    copy_from_JSON_Array(token.right_children, edges);
                }
                {
                    //MTT_print("(Token_Morph = %s){\n", token.text.c_str());
                    if (tok.containsKey("morph")) {
                        auto morph = tok["morph"].as<JsonObjectConst>();

    //                    for (auto m_it = morph.begin(); m_it != morph.end(); ++m_it) {
    //                        std::cout << m_it
    //                        if (m_it->value().is<const char*>()) {
    //                            int BP = 0;
    //                        } else if (m_it->value().is<int>()) {
    //                            int BP = 0;
    //                        }
    //                        //
    //                    }
                        for (JsonObjectConst::iterator m_it = morph.begin(); m_it != morph.end(); ++m_it) {
                            //std::cout << m_it->key().c_str() << ":" << mtt::String(m_it->value()) << std::endl;
                            token.morph.emplace(mtt::String(m_it->key().c_str()), mtt::String(m_it->value()));
                        }
                        
                        
                    }
                    //MTT_print("}\n");
                    
                }
            }
        }
        
        // FIXME: need to change to support the coreference buffering
        
        
        DT_DEBUG_print("%s\n", "coreference{");
        auto coref = in["coref"].as<JsonObjectConst>();

        cstring pretty_representation = coref["readable_rep"];
        
        DT_DEBUG_print("%s\n", pretty_representation);
        
        parse.pretty_coref_representation = pretty_representation;
        auto resolved = coref["resolved"].as<JsonObjectConst>();
        
        int full_len = coref["full_len"].as<int>();
        DT_DEBUG_print("full len =[%d]\n", full_len);
        
        
        DT_DEBUG_print("%s\n", "(TOKEN_BUFFER_BEFORE_FILL){");
        for (auto IT = dt->lang_ctx.token_buffer.token_q.begin();
             IT != dt->lang_ctx.token_buffer.token_q.end(); ++IT) {
            DT_DEBUG_print("(Token){ %s } \n", (*IT)->text.c_str());
        }
        DT_DEBUG_print("%s\n", "}");
        
         
        dt->lang_ctx.token_buffer.fill_with(parse.token_list);
        
        DT_DEBUG_print("%s\n", "(TOKEN_BUFFER_AFTER_FILL){");
        for (auto IT = dt->lang_ctx.token_buffer.token_q.begin();
             IT != dt->lang_ctx.token_buffer.token_q.end(); ++IT) {
            DT_DEBUG_print("(Token){ %s } \n", (*IT)->text.c_str());
        }
        DT_DEBUG_print("%s\n", "}");
        
        
        

        for (auto it_coref = resolved.begin(); it_coref != resolved.end(); ++ it_coref) {
            cstring key = it_coref->key().c_str();
            JsonArrayConst value = it_coref->value().as<JsonArrayConst>();
            usize key_as_int = atoi(key);
            //std::cout << key << ":" << key_as_int << ":" << value << std::endl;
            
            Speech_Token* token = *((dt->lang_ctx.token_buffer.token_q.end() - full_len) + key_as_int);//parse.token_list[key_as_int];
            
            Dynamic_Array<Speech_Token*>& ref_list = token->coreference.token_ref_list;
            
            
            for (auto it_entry = value.begin(); it_entry != value.end(); ++it_entry) {
                dt::sID idx = it_entry->as<dt::sID>();
                // TODO
                //ref_list.push_back(parse.token_list[idx]);
                ref_list.push_back(*((dt->lang_ctx.token_buffer.token_q.end() - full_len) + idx));
            }
            
        }
        DT_DEBUG_print("%s\n", "}");
        
        const auto is_finished = in["is_finished"].as<int>();
        if (!is_finished) {
            dt->lang_ctx.token_buffer.remove_back(parse.token_list.size());
        } else {

        }
        DT_DEBUG_print("%s\n", "(TOKEN_BUFFER_AFTER_ALL){");
        for (auto IT = dt->lang_ctx.token_buffer.token_q.begin();
             IT != dt->lang_ctx.token_buffer.token_q.end(); ++IT) {
            DT_DEBUG_print("(Token){ %s } \n",(*IT)->text.c_str());
        }
        DT_DEBUG_print("%s\n", "}");

        
        block_or_replace::mark_unaccepted_subsequences(parse.token_list);
        

        Speech_Property* cmds = dt::lang_compile(dt, &parse);
        if (cmds != nullptr) {
            context->result.props = cmds;
        }
        

        if (is_finished == 1) {
            parse.is_finished(true);
        } else {
            parse.is_finished(false);
        }
        
        in.clear();
    };
    auto* nl_data = bg_data_context->nl_data();
    if (nl_data == nullptr) {
        return;
    }
    nl_data->is_in_use = true;
    mtt::job_dispatch_serial(static_cast<void*>(bg_data_context), dispatch_proc, [](void* j_ctx) {
        auto* context = (BG_Data_Context*)j_ctx;
        //auto* sp_view = &context->sp_view;
        auto* dt = context->dt;
        
        
        auto& parse = *(context->parse());
        
        dt->lang_ctx.set_current_id(parse.ID);
        context->nl_data()->is_in_use = false;
        
        if (parse.is_finished()) {
            Natural_Language_Data_deinit(context->nl_data_ptr());
        }
        
        while (!dt->lang_ctx.parse_q().empty()) {
            dt::Parse* p = *(dt->lang_ctx.parse_q().begin());
            ASSERT_MSG(!p->is_finished(), "Should be impossible!\n");
            
            mtt::job_dispatch_serial(static_cast<void*>(p), [](void* ctx) {
                dt::Parse* p = static_cast<dt::Parse*>(ctx);
                auto* dt = dt::DrawTalk::ctx();
                for (usize tok_i = 0; tok_i < p->token_list.size(); tok_i += 1) {
                    mem::deallocate<Speech_Token>(&dt->token_allocation.allocator, p->token_list[tok_i]);
                }
            }, [](void* j_ctx) {
                dt::Parse* p = static_cast<dt::Parse*>(j_ctx);
                mem::deallocate<dt::Parse>(&dt::DrawTalk::ctx()->parse_allocation.allocator, p);
            });
            
            dt->lang_ctx.parse_q().erase(dt->lang_ctx.parse_q().begin());
        }
        

        dt->lang_ctx.parse_q().emplace_back(context->parse());
        
        mem::deallocate<BG_Data_Context>(&dt->bg_ctx_data.allocator, context);
        
        // push completed work to be done in evaluate_completed_events(...) at a specific part of the frame execution
    });
}

inline static mtt::Set<mtt::String> contractions = {
    "'ll", "'m", "'d", "'ve", "'t", "n't"
};

void generate_text_view_from_Parse(dt::DrawTalk* dt, dt::Parse& parse)
{
    Panel& panel = dt->ui.margin_panels[0];
    Text_Panel& text_panel = panel.text;
    text_panel.reset();
    text_panel.text.resize(parse.token_list.size());
    
    mtt::String message = "";
    
    // set text and colors for display
    for (usize i = 0; i < parse.token_list.size(); i += 1) {
        auto* tok = parse.token_list[i];
        auto* text_itm = &text_panel.text[i];
        text_itm->part_of_contraction = false;
        text_itm->matching_contraction_idx = -1ull;
        
        if (text_itm->text != tok->text) {
            text_itm->is_selected = false;
        }
        text_itm->text = tok->text;
        text_itm->token = tok;
        
        if (text_itm->update.mapping_count > 0) {
            for (auto map_it = text_itm->update.mappings.begin(); map_it != text_itm->update.mappings.end();) {
                if ((*map_it).second.first == false) {
                    ++map_it;
                    continue;
                }
                
                mtt::Thing_ID thing_id = (*map_it).first;
                mtt::Thing* thing = dt->mtt->Thing_try_get(thing_id);
                if (thing == nullptr) {
                    map_it = text_itm->update.mappings.erase(map_it);
                } else {
                    ++map_it;
                    if (spacy_nlp::pos_tag_is_noun_like(tok->tag)) {
                        dt::vis_word_derive_from(thing, dt::noun_add(tok->lemma));
                        tok->labeled_things.insert(thing_id);
                    }
                }
            }
        }
        
        if (contractions.find(text_itm->text) != contractions.end()) {
            text_itm->part_of_contraction = true;
            if (i > 0) {
                text_itm->matching_contraction_idx = i - 1;
            }
        }
        
        if (i < text_panel.text.size() - 1 && text_panel.text[i + 1].part_of_contraction) {
            message += tok->text;
        } else {
            message += tok->text + " ";
        }
        
        
        vec4 color = {};
        
        if (tok->prop_ref == nullptr) {
            color = dt::LABEL_COLOR_DEFAULT;
        } else {
            if (tok->prop_ref->type_str == "pronoun" ||
                tok->prop_ref->value.is_reference) {
                color = dt::LABEL_COLOR_PRONOUN;
            } else if (spacy_nlp::pos_tag_is_noun_like(tok->tag) || (tok->prop_ref->kind_str == "THING_INSTANCE")) {
                color = dt::LABEL_COLOR_THING_INSTANCE;
            } else if (tok->prop_ref->kind_str == "THING_TYPE") {
                color = dt::LABEL_COLOR_THING_TYPE;
            } else if (tok->prop_ref->kind_str == "ACTION" || tok->prop_ref->kind_str == "EXISTENTIAL") {
                if (tok->prop_ref->type_str == "TRIGGER") {
                    color = dt::LABEL_COLOR_TRIGGER;
                } else if (tok->prop_ref->type_str == "RESPONSE") {
                    color = dt::LABEL_COLOR_RESPONSE;
                } else {
                    color = dt::LABEL_COLOR_ACTION;
                }
            } else if (tok->prop_ref->type_str == "ACTION") {
                color = dt::LABEL_COLOR_ACTION;
            } else if (tok->pos == spacy_nlp::POS_ADJ) {
                color = dt::LABEL_COLOR_ADJECTIVE;
            } else if (tok->pos == spacy_nlp::POS_ADV) {
                color = dt::LABEL_COLOR_ADVERB;
            } else {
                color = dt::LABEL_COLOR_DEFAULT;
            }
            
            tok->prop_ref->candidate_selections = &text_itm->update;
        }
        
        text_itm->color = color;
        
    }
    
    // precompute bounding boxes and heights
    {
        auto* vg = nvgGetGlobalContext();
        
        nvgSave(vg);
        
        nvgFontSize(vg, text_panel.font_size);
        nvgFontFace(vg, text_panel.font_face);
        nvgTextAlign(vg, text_panel.align);
        float lineh = 0;
        nvgTextMetrics(vg, NULL, NULL, &lineh);
        
        float width = panel.bounds.dimensions.x - text_panel.offset.x;
        float x = text_panel.offset.x;
        float y = text_panel.offset.y;
        


        if (!text_panel.text.empty()) {
            text_panel.row_count = 1;
        }
        
        
        double out_of_bounds_y = (panel.bounds.tl.y - text_panel.offset.y + panel.bounds.dimensions.y);
        for (usize i = 0; i < text_panel.text.size(); i += 1) {
            auto& word = text_panel.text[i];
            cstring CS = word.text.c_str();
            
            float bounds[4];
            double advance = (i < text_panel.text.size() - 1 && text_panel.text[i + 1].part_of_contraction) ? 0 : nvgTextBounds(vg, 0, 0, " ", NULL, bounds);
            
            double text_advance = nvgTextBounds(vg, 0, 0, CS, NULL, bounds);
            
            if (x + advance + text_advance >= width) {
                x = text_panel.offset.x;
                y += lineh * text_panel.row_sep_factor;

                if (y >= out_of_bounds_y)
                {
                    text_panel.invisible_row_count += 1;
                }
                
                text_panel.row_count += 1;
                
            }
            
            word.line_idx = text_panel.row_count - 1;
            
            word.bounds[0] = x;
            word.bounds[1] = y;
            word.bounds[2] = x + text_advance;
            word.bounds[3] = y + (lineh * text_panel.row_sep_factor);
            
            x += text_advance + advance;
            
        }
        
//        if (!text_panel.text.empty()) {
//            auto& last_word = text_panel.text.back();
//            if (last_word.bounds[3] > panel.bounds.dimensions.y - (text_panel.offset.y * 2.0)) {
//                text_panel.row_offset = last_word.bounds[3] - (panel.bounds.dimensions.y - (text_panel.offset.y * 2.0));
//            } else {
//                text_panel.row_offset = 0.0;
//            }
//        }

        text_panel.row_offset = (text_panel.invisible_row_count) * lineh * text_panel.row_sep_factor;
        
        nvgRestore(vg);
    }
    if (!message.empty()) {
        message.pop_back();
    }
    text_panel.generated_message = message;
};


void regenerate_text_view(dt::DrawTalk* dt, dt::Parse& parse, Regenerate_Operation& op, bool compute_semantics)
{
    Panel& panel = dt->ui.margin_panels[0];
    Text_Panel& text_panel = panel.text;
    text_panel.reset();
    
    



//    for (usize i = 0; i < op.token.size(); i += 1) {
//        auto id = op.token[i]->id;
//        op.token[i]->mark_for_deletion = true;
//        parse.token_list.erase(parse.token_list.begin() + i);
//
//    }
    
    if (op.type == Regenerate_Operation::TYPE::DELETE) {
        // FIXME: leak - UNUSED for now
        for (auto it = parse.token_list.begin(); it != parse.token_list.end();) {
            auto find_it = op.token.find(*it);
            if (find_it != op.token.end()) {
                (*it)->mark_for_deletion = true;
                it = parse.token_list.erase(it);
            } else {
                ++it;
            }
        }
        text_panel.text.resize(parse.token_list.size());
    }
    
    
    
    mtt::String message = "";
    
    // set text and colors for display
    for (usize i = 0; i < parse.token_list.size(); i += 1) {
        auto* tok = parse.token_list[i];
        auto* text_itm = &text_panel.text[i];
        text_itm->text = tok->text;
        text_itm->token = tok;
        
        text_itm->part_of_contraction = false;
        text_itm->matching_contraction_idx = -1ull;
        
        if (text_itm->text != tok->text) {
            text_itm->is_selected = false;
        }
        text_itm->text = tok->text;
        text_itm->token = tok;
        
        if (op.type == Regenerate_Operation::TYPE::RENAME) {
            auto* noun_entry = dt::noun_lookup(text_itm->token->lemma);
            text_itm->token->lemma = text_panel.text[i].text;
            text_itm->token->text = text_panel.text[i].text;
            
//            auto* new_entry = dt::noun_add(text_itm->token->lemma);
//            for (auto thing_id : text_itm->update.mappings) {
//                mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), thing_id.first);
//                if (thing == nullptr) {
//                    continue;
//                }
//                dt::vis_word_underive_from(thing, noun_entry);
//                dt::vis_word_derive_from(thing, new_entry);
//            }
        }
        
        if (i < text_panel.text.size() - 1 && text_panel.text[i + 1].part_of_contraction) {
            message += tok->text;
        } else {
            message += tok->text + " ";
        }
        
        vec4 color = {};
        
        if (tok->prop_ref == nullptr) {
            color = dt::LABEL_COLOR_DEFAULT;
        } else if (tok->prop_ref->type_str == "pronoun" ||
                   tok->prop_ref->value.is_reference) {
            color = dt::LABEL_COLOR_PRONOUN;
        } else if (spacy_nlp::pos_tag_is_noun_like(tok->tag) || (tok->prop_ref->kind_str == "THING_INSTANCE")) {
            color = dt::LABEL_COLOR_THING_INSTANCE;
        } else if (tok->prop_ref->kind_str == "THING_TYPE") {
            color = dt::LABEL_COLOR_THING_TYPE;
        } else if (tok->prop_ref->kind_str == "ACTION" || tok->prop_ref->kind_str == "EXISTENTIAL") {
            if (tok->prop_ref->type_str == "TRIGGER") {
                color = dt::LABEL_COLOR_TRIGGER;
            } else if (tok->prop_ref->type_str == "RESPONSE") {
                color = dt::LABEL_COLOR_RESPONSE;
            } else {
                color = dt::LABEL_COLOR_ACTION;
            }
        } else if (tok->prop_ref->type_str == "ACTION") {
            color = dt::LABEL_COLOR_ACTION;
        } else if (tok->pos == spacy_nlp::POS_ADJ) {
            color = dt::LABEL_COLOR_ADJECTIVE;
        } else if (tok->pos == spacy_nlp::POS_ADV) {
            color = dt::LABEL_COLOR_ADVERB;
        } else {
            color = dt::LABEL_COLOR_DEFAULT;
        }
        
        text_itm->color = color;
        
    }
    
    if (op.type == Regenerate_Operation::TYPE::DELETE) {
        Speech_Info* speech_info = &dt->core->speech_system.info_list[0];
        Speech_send_override(speech_info, message, 0);
    } else if (op.type == Regenerate_Operation::TYPE::RENAME) {
        if (compute_semantics) {
            Speech_Info* speech_info = &dt->core->speech_system.info_list[0];
            Speech_send_override(speech_info, message, 0);
        }
    } else if (op.type == Regenerate_Operation::TYPE::CONFIRM) {
        if (compute_semantics) {
            Speech_Info* speech_info = &dt->core->speech_system.info_list[0];
            Speech_send_override(speech_info, message, 1);
            text_in_buf.clear();
            text_in_selected_all = false;
            text_in_cursor_idx = false;
            for (usize i = 0; i < parse.token_list.size(); i += 1) {
                auto* tok = parse.token_list[i];
                auto* text_itm = &text_panel.text[i];
                text_itm->is_selected = false;
                //text_itm->update.thing_id = mtt::Thing_ID_INVALID;
            }
        }
    }
    
    
    
    // precompute bounding boxes and heights
    {
        auto* vg = nvgGetGlobalContext();
        
        nvgSave(vg);
        
        nvgFontSize(vg, text_panel.font_size);
        nvgFontFace(vg, text_panel.font_face);
        nvgTextAlign(vg, text_panel.align);
        float lineh = 0;
        nvgTextMetrics(vg, NULL, NULL, &lineh);
        
        float width = panel.bounds.dimensions.x - text_panel.offset.x;
        float x = text_panel.offset.x;
        float y = text_panel.offset.y;
        


        if (!text_panel.text.empty()) {
            text_panel.row_count = 1;
        }
        
        double out_of_bounds_y = (panel.bounds.tl.y - text_panel.offset.y + panel.bounds.dimensions.y);
        for (usize i = 0; i < text_panel.text.size(); i += 1) {
            auto& word = text_panel.text[i];
            cstring CS = word.text.c_str();
            
            float bounds[4];
            double advance = (i < text_panel.text.size() - 1 && text_panel.text[i + 1].part_of_contraction) ? 0 : nvgTextBounds(vg, 0, 0, " ", NULL, bounds);
            double text_advance = nvgTextBounds(vg, 0, 0, CS, NULL, bounds);
            
            if (x + advance + text_advance >= width) {
                x = text_panel.offset.x;
                y += lineh * text_panel.row_sep_factor;

                if (y >= out_of_bounds_y) {
                    text_panel.invisible_row_count += 1;
                }
                
                text_panel.row_count += 1;
                
            }
            
            word.line_idx = text_panel.row_count - 1;
            
            word.bounds[0] = x;
            word.bounds[1] = y;
            word.bounds[2] = x + text_advance;
            word.bounds[3] = y + (lineh * text_panel.row_sep_factor);
            
            x += text_advance + advance;
            
        }
        
        if (!text_panel.text.empty()) {
            auto& last_word = text_panel.text.back();
            if (last_word.bounds[3] > panel.bounds.dimensions.y - (text_panel.offset.y * 2.0)) {
                text_panel.row_offset = last_word.bounds[3] - (panel.bounds.dimensions.y - (text_panel.offset.y * 2.0));
            } else {
                text_panel.row_offset = 0.0;
            }
        }

        text_panel.row_offset = (text_panel.invisible_row_count) * lineh * text_panel.row_sep_factor;
        
        nvgRestore(vg);
    }
    if (!message.empty()) {
        message.pop_back();
    }
    text_panel.generated_message = message;
};


#define DT_CONSIDER_THE_AS_DEIXIS (false)
#define DT_CONSIDER_IT_AS_DEIXIS (true)

bool deixis_is_plural(Speech_Property* prop)
{
    Speech_Property* d = nullptr;
    if (prop->try_get_only_prop("DEIXIS", &d)) {
        const mtt::String& val = d->label;
        return ((val == "these") || (val == "those"));
    }
    return false;
}
bool deixis_is_plural(const mtt::String& val)
{
    return ((val == "these") || (val == "those"));
}
std::vector<mtt::Thing*> things_to_process_deixis = {};

void try_labeling_via_deixis_match(dt::DrawTalk* dt)
{
//    if (!currently_talking_is_likely(&mtt_core_ctx()->speech_system.info_list[0])) {
//        return;
//    }
    
    if (dt->lang_ctx.parse_q().empty()) {
        return;
    }
    
    things_to_process_deixis.clear();
    
    Parse& parse = *(dt->lang_ctx.parse_q().back());
    if (parse.is_discarded() || parse.cmds == nullptr) {
        return;
    }
    if (!parse.token_list.empty()) {
        auto& tok_lem = parse.token_list.back()->lemma;
          
        if (tok_lem == "be" ||
            tok_lem == "a"     ||
            tok_lem == "an"    ||
            tok_lem == "this"  ||
            tok_lem == "that"  ||
            tok_lem == "those" ||
            tok_lem == "these" ||
            tok_lem == "and") {
                return;
        }
    }
    
    auto& cmds = parse.cmds;
    dt::Dynamic_Array<dt::Speech_Property*>& deixis = dt->lang_ctx.deixis;
    deixis.clear();
    cmds->traverse_preorder([dt](Speech_Property* prop) {
        if (prop->has_prop("DEIXIS")) {
            dt->lang_ctx.deixis_push(prop);
        } else if constexpr (DT_CONSIDER_THE_AS_DEIXIS) {
            Speech_Property* spec_prop = nullptr;
            MTT_UNUSED(spec_prop);
            if (prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
                if (spec_prop->value.flag == true) {
                    dt->lang_ctx.deixis_push(prop);
                }
            }
        }
        return true;
    });
    dt->lang_ctx.deixis_sort();
//    MTT_print("DEIXIS BEGIN================================\n");
    usize D_I = dt->lang_ctx.deixis_resolve_offset_i;
    usize deixis_len = deixis.size();
    bool deixis_len_changed = deixis_len != dt->lang_ctx.prev_deixis_len;
    //MTT_print("DEIXIS_LEN: prev:%llu curr:%llu\n", dt->lang_ctx.prev_deixis_len, deixis_len);
    dt->lang_ctx.prev_deixis_len = deixis_len;
    usize s_idx = 0;
    //cmds->print();
    // TODO
    
    if (/*!deixis_len_changed && */deixis_len > 0 && D_I >= deixis_len) {
        Speech_Property* prop = deixis.back();
        if (prop == nullptr || prop->token == nullptr || (prop->token-> pos != spacy_nlp::POS_PROPN  && prop->token->pos != spacy_nlp::POS_NOUN) || prop->token->lemma == "") {
            // no-op
            goto SKIP_DEIXIS_UPDATE;
        } else {
            bool is_negation = false;
            
            Speech_Property* parent = prop->get_active_parent();
            if (parent != nullptr) {
                Speech_Property* negation_prop = nullptr;
                if (parent->try_get_only_prop("NEGATED", &negation_prop)) {
                    is_negation = negation_prop->value.flag;
                }
            }
            
            
            if (!is_negation) {
                mtt::Thing* thing = nullptr;
                things_to_process_deixis.clear();
                bool is_plural = deixis_is_plural(prop);
                if (is_plural) {
                    Selection_Recorder_get_all_labeled_Things(&dt->recorder.selection_recording, things_to_process_deixis);
                } else {
                    thing = Selection_Recorder_get_last_labeled_Thing(&dt->recorder.selection_recording);
                    if (thing != nullptr) {
                        things_to_process_deixis.push_back(thing);
                    }
                }
                if (things_to_process_deixis.size() == 0) {
                    goto SKIP_DEIXIS_UPDATE;
                }
                
                for (usize t_i = 0; t_i < things_to_process_deixis.size(); t_i += 1) {
                    thing = things_to_process_deixis[t_i];
                    Speech_Property::Prop_List* trait = nullptr;
                    if (prop->try_get_prop("PROPERTY", &trait)) {
                        for (auto it_t = trait->begin(); it_t != trait->end(); ++it_t) {
                            
                            if ((*it_t)->tag_str == "ADJ") {
                                
                                bool used_modifier = false;
                                {
                                    dt::Speech_Property::Prop_List* modifiers = (*it_t)->try_get_prop("PROPERTY");
                                    if (modifiers != nullptr) {
                                        if ((*it_t)->kind_str != "NEGATED") {
                                            const Speech_Property* prev = nullptr;
                                            for (const Speech_Property* mod : *modifiers) {
                                                if (mod->label != "modifier") {
                                                    continue;
                                                }
                                                
                                                float32* value = nullptr;
                                                
                                                if (!mtt::map_try_get(&dt::magnitude_words, mod->value.text, &value)) {
                                                    Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, {});
                                                } else {
                                                    if (prev != nullptr && prev->value.text == mod->value.text) {
                                                        Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(accumulate_word_values((*value))));
                                                    } else {
                                                        Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(*value));
                                                    }
                                                }
                                                
                                                used_modifier = true;
                                                prev = mod;
                                            }
                                        } else {
                                            for (const Speech_Property* mod : *modifiers) {
                                                if (mod->label != "modifier") {
                                                    continue;
                                                }
                                                
                                                Thing_remove_attribute_property(thing, (*it_t)->value.text, mod->value.text);
                                                
                                                used_modifier = true;
                                            }
                                        }
                                    }
                                }
                                
                                
                                
                                if (!used_modifier) {
                                    Word_Dictionary_Entry* attrib = dt::attribute_add((*it_t)->value.text);
                                    if ((*it_t)->kind_str != "NEGATED") {
                                        Thing_add_attribute(thing, attrib);
                                        MTT_print("ADDING %s\n", attrib->name.c_str());
                                    } else {
                                        Thing_remove_attribute(thing, attrib);
                                    }
                                }
                            }
                            else {
                                auto* info = *it_t;
                                if (dt::naming_words.contains(info->label)) {
                                    Word_Dictionary_Entry* attrib = dt::noun_add(info->value.text);
                                    if ((*it_t)->kind_str != "NEGATED") {
                                        //Thing_add_attribute(thing, attrib);
                                        dt::vis_word_derive_from(thing, attrib);
                                        
                                    } else {
                                        //Thing_remove_attribute(thing, attrib);
                                        dt::vis_word_underive_from(thing, attrib);
                                    }
                                }
                            }
                        }
                    } else {
                        if (dt::naming_words.contains(parent->label)) {
                            Speech_Property* p__ = parent->try_get_only_prop("PROPERTY");
                            if (p__ != nullptr) {
                                Word_Dictionary_Entry* attrib = dt::noun_add(p__->label);
                                if (p__->kind_str != "NEGATED") {
                                    //Thing_add_attribute(thing, attrib);
                                    dt::vis_word_derive_from(thing, attrib);
                                    MTT_BP();
                                } else {
                                    dt::vis_word_underive_from(thing, attrib);
                                    //Thing_remove_attribute(thing, attrib);
                                }
                            }
                        }
                    }
                    
                }
                
            } else {
                
            }
        }
        
    }
    SKIP_DEIXIS_UPDATE:
    for (auto it = deixis.begin() + dt->lang_ctx.deixis_resolve_offset_i; D_I < deixis_len; ++it) {
        D_I += 1;
        if ((*it) == nullptr || (*it)->token == nullptr || ((*it)->token-> pos != spacy_nlp::POS_PROPN  && (*it)->token->pos != spacy_nlp::POS_NOUN)) {
            continue;
        }
        //MTT_print("DEIXIS COUNT: %lu, IDX: [%llu] offset: %llu", deixis.size(), D_I-1, dt->lang_ctx.deixis_resolve_offset_i);
        Speech_Property* prop = *it;
        
        mtt::String& label = prop->label;//prop->token->lemma;
//        if (label == "") {
//            continue;
//        }
        MTT_print("LABEL: %s\n", label.c_str());
        bool is_negation = false;
        bool is_end_of_s = false;
        
        Speech_Property* parent = prop->get_active_parent();
        Speech_Property* negation_prop = nullptr;
        if (parent != nullptr) {
            if (parent->try_get_only_prop("NEGATED", &negation_prop)) {
                is_negation = negation_prop->value.flag;
            }
        }
        
        
        if (!is_negation) {
            // FIXME: update label if the recognition changes it
            usize before = s_idx;
            mtt::Thing* thing = nullptr;
            
            things_to_process_deixis.clear();
            bool is_plural = deixis_is_plural(prop);
            if (is_plural) {
#define PLURAL_ALLOW_MULTIPLE_FIX (1)
                do {
#if PLURAL_ALLOW_MULTIPLE_FIX
                    thing = Selection_Recorder_get_next_Thing(&dt->recorder.selection_recording, &s_idx);
#else
                    thing = Selection_Recorder_get_next_unlabeled_Thing(&dt->recorder.selection_recording, &s_idx);
#endif
                    if (thing != nullptr) {
                        things_to_process_deixis.push_back(thing);
                    }
                    s_idx += 1;
                } while (thing != nullptr);
            } else {
                auto prev_sidx = s_idx;
                thing = Selection_Recorder_get_next_unlabeled_Thing(&dt->recorder.selection_recording, &s_idx);
                auto next_sidx = s_idx;
                if (thing != nullptr) {
                    things_to_process_deixis.push_back(thing);
                } else {
                    s_idx = prev_sidx;
                    thing = Selection_Recorder_get_next_labeled_Thing(&dt->recorder.selection_recording, &s_idx);
                    if (thing != nullptr) {
                        things_to_process_deixis.push_back(thing);
                    } else {
                        s_idx = next_sidx;
                    }
                }
            }
            
            if (things_to_process_deixis.size() == 0) {
                {
                    {
                        Speech_Property::Prop_List* trait = nullptr;
                        if (prop->try_get_prop("PROPERTY", &trait)) {
                            if ((*trait->begin())->tag_str == "ADJ") {
                                s_idx = before;
                                
                                
                                MTT_print("%s", "---------------------\n");
                                MTT_print("resolve_offset=[%llu] trait:%s\n", dt->lang_ctx.deixis_resolve_offset_i, (*trait->begin())->value.text.c_str());
                                print_thing_selections(dt);
                                MTT_print("%s", "---------------------\n");
                                
                                if (is_plural) {
                                    Selection_Recorder_get_all_labeled_Things(&dt->recorder.selection_recording, things_to_process_deixis);
                                } else {
                                    thing = Selection_Recorder_get_next_labeled_Thing(&dt->recorder.selection_recording, &s_idx);
                                    if (thing != nullptr) {
                                        things_to_process_deixis.push_back(thing);
                                    }
                                }
                                for (usize t_i = 0; t_i < things_to_process_deixis.size(); t_i += 1) {
                                    thing = things_to_process_deixis[t_i];
                                    
                                    if (dt::can_label_thing(thing)) {
                                        for (auto it_t = trait->begin(); it_t != trait->end(); ++it_t) {
                                            
                                            bool used_modifier = false;
                                            {
                                                dt::Speech_Property::Prop_List* modifiers = (*it_t)->try_get_prop("PROPERTY");
                                                if (modifiers != nullptr) {
                                                    if ((*it_t)->kind_str != "NEGATED") {
                                                        const Speech_Property* prev = nullptr;
                                                        for (const Speech_Property* mod : *modifiers) {
                                                            if (mod->label != "modifier") {
                                                                continue;
                                                            }
                                                            
                                                            float32* value = nullptr;
                                                            
                                                            if (!mtt::map_try_get(&dt::magnitude_words, mod->value.text, &value)) {
                                                                Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, {});
                                                            } else {
                                                                if (prev != nullptr && prev->value.text == mod->value.text) {
                                                                    Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(accumulate_word_values((*value))));
                                                                } else {
                                                                    Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(*value));
                                                                }
                                                            }
                                                            
                                                            prev = mod;
                                                            
                                                            used_modifier = true;
                                                        }
                                                    } else {
                                                        for (const Speech_Property* mod : *modifiers) {
                                                            if (mod->label != "modifier") {
                                                                continue;
                                                            }
                                                            
                                                            Thing_remove_attribute_property(thing, (*it_t)->value.text, mod->value.text);
                                                            
                                                            used_modifier = true;
                                                        }
                                                    }
                                                }
                                            }
                                            
                                            if (!used_modifier) {
                                                Word_Dictionary_Entry* attrib = dt::attribute_add((*it_t)->value.text);
                                                if ((*it_t)->kind_str != "NEGATED") {
                                                    Thing_add_attribute(thing, attrib);
                                                } else {
                                                    Thing_remove_attribute(thing, attrib);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        } else {
//                            MTT_BP();
//                            s_idx = before;
//
//                            thing = Selection_Recorder_get_next_labeled_Thing(&dt->recorder.selection_recording, &s_idx);
                        }

                    }
                }
                //break;
            } else {
                for (usize t_i = 0; t_i < things_to_process_deixis.size(); t_i += 1) {
                    thing = things_to_process_deixis[t_i];
                    if ((!dt::can_label_thing(thing))) {
                        
                        continue;
                    } else {
                        
                        if (label != "") {
                            auto* noun_entry = dt::noun_lookup(label);
                            if (!noun_entry || label == "I") {
                                noun_entry = dt::noun_add(label);
                            }
                            dt::vis_word_derive_from(thing, noun_entry);
                        }
                        
                        
                        {
                            Speech_Property::Prop_List* trait = nullptr;
                            prop->print();
                            if (prop->try_get_prop("PROPERTY", &trait)) {
                                for (auto it_t = trait->begin(); it_t != trait->end(); ++it_t) {
                                    
                                    if ((*it_t)->tag_str == "ADJ") {
                                        
                                        bool used_modifier = false;
                                        {
                                            dt::Speech_Property::Prop_List* modifiers = (*it_t)->try_get_prop("PROPERTY");
                                            if (modifiers != nullptr) {
                                                if ((*it_t)->kind_str != "NEGATED") {
                                                    const Speech_Property* prev = nullptr;
                                                    for (const Speech_Property* mod : *modifiers) {
                                                        if (mod->label != "modifier") {
                                                            continue;
                                                        }
                                                        
                                                        float32* value = nullptr;
                                                        
                                                        if (!mtt::map_try_get(&dt::magnitude_words, mod->value.text, &value)) {
                                                            Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, {});
                                                        } else {
                                                            if (prev != nullptr && prev->value.text == mod->value.text) {
                                                                Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(accumulate_word_values((*value))));
                                                            } else {
                                                                Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(*value));
                                                            }
                                                        }
                                                        
                                                        used_modifier = true;
                                                    }
                                                } else {
                                                    for (const Speech_Property* mod : *modifiers) {
                                                        if (mod->label != "modifier") {
                                                            continue;
                                                        }
                                                        
                                                        Thing_remove_attribute_property(thing, (*it_t)->value.text, mod->value.text);
                                                        
                                                        used_modifier = true;
                                                    }
                                                }
                                            }
                                        }
                                        
                                        
                                        if (!used_modifier) {
                                            // TODO: check if the remove attribute / negation case are necessary
                                            Word_Dictionary_Entry* attrib = dt::attribute_add((*it_t)->value.text);
                                            if ((*it_t)->kind_str != "NEGATED") {
                                                Thing_add_attribute(thing, attrib);
                                            } else {
                                                Thing_remove_attribute(thing, attrib);
                                            }
                                        }
                                    } else {
                                        auto* info = *it_t;
                                        if (dt::naming_words.contains(info->label)) {
                                            Word_Dictionary_Entry* attrib = dt::noun_add(info->value.text);
                                            if ((*it_t)->kind_str != "NEGATED") {
                                                //Thing_add_attribute(thing, attrib);
                                                dt::vis_word_derive_from(thing, attrib);
                                            } else {
                                                //Thing_remove_attribute(thing, attrib);
                                                dt::vis_word_underive_from(thing, attrib);
                                            }
                                        }
                                    }
                                }
                            }
                            
                        }
                        
                    }
                }
                dt->lang_ctx.deixis_resolve_offset_i += 1;

            }

        } else {
            
            mtt::Thing* thing = nullptr;
            things_to_process_deixis.clear();
            
            bool is_plural = deixis_is_plural(prop);
            
            if (is_plural) {
                do {
#if PLURAL_ALLOW_MULTIPLE_FIX
                    thing = Selection_Recorder_get_next_Thing(&dt->recorder.selection_recording, &s_idx);
#else
                    thing = Selection_Recorder_get_next_labeled_Thing(&dt->recorder.selection_recording, &s_idx);
#endif
                    if (thing != nullptr) {
                        things_to_process_deixis.push_back(thing);
                    }
                    s_idx += 1;
                } while (thing != nullptr);
            } else {
                thing = Selection_Recorder_get_next_labeled_Thing(&dt->recorder.selection_recording, &s_idx);
                
                if (thing != nullptr) {
                    things_to_process_deixis.push_back(thing);
                }
            }
                
            {
                if (things_to_process_deixis.size() == 0) {
                    break;
                }
                for (usize t_i = 0; t_i < things_to_process_deixis.size(); t_i += 1) {
                    thing = things_to_process_deixis[t_i];
                    if ((!dt::can_label_thing(thing))) {
                        continue;
                    } else {
                        auto* noun_entry = dt::noun_lookup(label);
                        if (!noun_entry) {
                            continue;
                        }
                        
                        {
                            
                            Speech_Property::Prop_List* trait = nullptr;
                            if (prop->try_get_prop("PROPERTY", &trait)) {
                                bool removed_attribute = false;
                                for (auto it_t = trait->begin(); it_t != trait->end(); ++it_t) {
                                    if ((*it_t)->tag_str == "ADJ") {
                                        
                                        bool used_modifier = false;
                                        {
                                            dt::Speech_Property::Prop_List* modifiers = (*it_t)->try_get_prop("PROPERTY");
                                            if (modifiers != nullptr) {
                                                if ((*it_t)->kind_str != "NEGATED") {
                                                    const Speech_Property* prev = nullptr;
                                                    for (const Speech_Property* mod : *modifiers) {
                                                        if (mod->label != "modifier") {
                                                            continue;
                                                        }
                                                        
                                                        float32* value = nullptr;
                                                        
                                                        if (!mtt::map_try_get(&dt::magnitude_words, mod->value.text, &value)) {
                                                            Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, {});
                                                        } else {
                                                            if (prev != nullptr && prev->value.text == mod->value.text) {
                                                                Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(accumulate_word_values((*value))));
                                                            } else {
                                                                Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(*value));
                                                            }
                                                        }
                                                        
                                                        used_modifier = true;
                                                        removed_attribute = true;
                                                        prev = mod;
                                                    }
                                                } else {
                                                    for (const Speech_Property* mod : *modifiers) {
                                                        if (mod->label != "modifier") {
                                                            continue;
                                                        }
                                                        
                                                        Thing_remove_attribute_property(thing, (*it_t)->value.text, mod->value.text);
                                                        
                                                        used_modifier = true;
                                                        removed_attribute = true;
                                                    }
                                                }
                                            }
                                        }
                                        
                                        
                                        
                                        if (!used_modifier) {
                                            Word_Dictionary_Entry* attrib = dt::attribute_lookup((*it_t)->value.text);
                                            if (attrib != nullptr) {
                                                Thing_remove_attribute(thing, attrib);
                                                removed_attribute = true;
                                            }
                                        }
                                    }
                                }
                                if (!removed_attribute) {
                                    dt::vis_word_underive_from(thing, noun_entry);
                                }
                            } else {
                                Speech_Property* verb_root = prop->get_active_parent();
                                if (true || verb_root == nullptr) {
                                    // defer because the sentence does not provide enough information
                                    // and is likely to refer to a property being negated, not the type being unassigned
                                    // X is not <?>
                                    if (negation_prop != nullptr && negation_prop->kind_str != "NEGATED_RIGHT") {
                                    } else {
                                        dt::vis_word_underive_from(thing, noun_entry);
                                    }
                                    
                                    if (verb_root != nullptr) {
                                        Speech_Property::Prop_List* objects = nullptr;
                                        if (verb_root->try_get_prop("OBJECT", &objects)) {
                                            if (objects->size() == 2) {
                                                if ((*objects)[0]->label == label) {
                                                    auto* other_noun_entry = dt::noun_add((*objects)[1]->label);
                                                    dt::vis_word_derive_from(thing, other_noun_entry);
                                                    Speech_Property* other = (*objects)[1];
                                                    Speech_Property::Prop_List* other_props;
                                                    if (other->try_get_prop("PROPERTY", &other_props)) {
                                                        for (auto it_t = other_props->begin(); it_t != other_props->end(); ++it_t) {
                                                            if ((*it_t)->tag_str == "ADJ") {
                                                                
                                                                bool used_modifier = false;
                                                                {
                                                                    dt::Speech_Property::Prop_List* modifiers = (*it_t)->try_get_prop("PROPERTY");
                                                                    if (modifiers != nullptr) {
                                                                        const Speech_Property* prev = nullptr;
                                                                        for (const Speech_Property* mod : *modifiers) {
                                                                            if (mod->label != "modifier") {
                                                                                continue;
                                                                            }
                                                                            
                                                                            float32* value = nullptr;
                                                                            
                                                                            if (!mtt::map_try_get(&dt::magnitude_words, mod->value.text, &value)) {
                                                                                Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, {});
                                                                            } else {
                                                                                if (prev != nullptr && prev->value.text == mod->value.text) {
                                                                                    Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(accumulate_word_values((*value))));
                                                                                } else {
                                                                                    Thing_add_attribute(thing, (*it_t)->value.text, mod->value.text, mtt::Any::from_Float(*value));
                                                                                }
                                                                            }
                                                                            
                                                                            used_modifier = true;
                                                                            prev = mod;
                                                                        }
                                                                    }
                                                                }
                                                                
                                                                
                                                                if (!used_modifier) {
                                                                    Word_Dictionary_Entry* attrib = dt::attribute_add((*it_t)->value.text);
                                                                    Thing_add_attribute(thing, attrib);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                } else {
                                    //                            Speech_Property::Prop_List* agent = nullptr;
                                    //                            if (verb_root->try_get_prop("AGENT", &agent)) {
                                    //                                for (auto a_it = agent->begin(); a_it != agent->end(); ++a_it) {
                                    //                                    Speech_Property* a_prop = *a_it;
                                    //                                    if (a_prop->label == noun_entry->name) {
                                    //                                        dt::vis_word_underive_from(thing, noun_entry);
                                    //                                        break;
                                    //                                    }
                                    //                                }
                                    //                            } else {
                                    //                                dt::vis_word_underive_from(thing, noun_entry);
                                    //                            }
                                }
                                
                            }
                            
                        }
                        
                        
                    }
                }
                dt->lang_ctx.deixis_resolve_offset_i += 1;
            }
        }
    }
    
}
void evaluate_completed_speech_events(dt::DrawTalk* dt, MTT_Core* core, mtt::World* world)
{
    
    //assert(mtt::is_main_thread());
    
    {
        //ASSERT_MSG(dt->lang_ctx.parse_q.size() == 1, "ore than one parse ready??\n");
        
        //MTT_print("Parse q size %lu\n", dt->lang_ctx.parse_q.size());
        if (!dt->lang_ctx.parse_q().empty()) {
            Parse& parse = *(dt->lang_ctx.parse_q().back());
            if (parse.was_processed) {
                return;
            }
            parse.was_processed = true;
            
            DT_print("%llu\n", parse.ID);
            

            if (parse.cmds != nullptr) {
                MTT_print("%s", "{\n========\n");
                parse.cmds->print();
                MTT_print("%s", "========\n}\n");

                MTT_BP();
                

                try_labeling_via_deixis_match(dt);
            }
            


//            for (usize i = 0; i < parse.token_count(); i += 1) {
//                auto& token = *parse.token_list[i];
//                if (token.timing.was_selected) {
//                    dt::Token_print(&parse, &token);
//                }
//            }

            ////////
            {
                generate_text_view_from_Parse(dt, parse);
            }
            
            lang_eval_compiled(dt, &parse, parse.cmds);
            
//            Label_Assignment_Guess* guess;
//            if (!dt->lang_ctx.type_label_guessing.find_or_make(parse.ID, &guess)) {
//                // is new
//                int BP = 0;
//            } else {
//                int BP = 0;
//                // is existing
//            }
//
//            auto& labels = dt->lang_ctx.dictionary.thing_to_word;
//            {
//                usize idx = 0;
//                for (auto t_it = dt->recorder.selection_recording.selections.begin();
//                     t_it != dt->recorder.selection_recording.selections.end(); ++t_it) {
//                    idx += 1;
//                }
//            }

            //////


            if (parse.is_finished()) {
                dt->lang_ctx.parse_q().pop_back();

                {
                    
                    Selection_Recorder_clear_selections_except_for_thing(&dt->recorder.selection_recording, dt->scn_ctx.thing_selected_drawable);
                    dt::Selection_Recorder_clear_location_selections(&dt->recorder.selection_recording);
                    //dt->scn_ctx.thing_selected_drawable = mtt::Thing_ID_INVALID;
                }
                
                //dt::DrawTalk::ctx().ui.margin_panels[0].text.reset();
                if (parse.cmds != nullptr) {
                    MTT_print("%s", "{\n========\n");
                    parse.cmds->print();
                    MTT_BP();
                }

                
                dt->lang_ctx.parse_history().push_back(&parse);
                
                // we want to have a small buffer
                
                // TODO: Very important resource-clearing here that should be enabled. Re-check for correctness, memory leaks, etc. or local/remote data de-sync. 
                if constexpr ((true)) {
                    {
                        for (auto it = dt->lang_ctx.parse_history().begin();
                             it != dt->lang_ctx.parse_history().end() &&
                             dt->lang_ctx.parse_history().size() > 2 * (4) * dt::parse_history_size;) {
                            
                            dt::Parse* p = (*it);
                            it = dt->lang_ctx.parse_history().erase(it);
                            
                            mtt::job_dispatch_serial(static_cast<void*>(p), [](void* ctx) {
                                dt::Parse* p = static_cast<dt::Parse*>(ctx);
                                auto* dt = dt::DrawTalk::ctx();
                                for (usize tok_i = 0; tok_i < p->token_list.size(); tok_i += 1) {
                                    mem::deallocate<dt::Speech_Token>(&dt->token_allocation.allocator, p->token_list[tok_i]);
                                }
                                
                                p->token_list.clear();
                                
                                dt->lang_ctx.token_buffer.remove_back(p->token_list.size());
                                Speech_Property::destroy_recursive(p->cmds);
                            }, [](void* j_ctx) {
                                dt::Parse* p = static_cast<dt::Parse*>(j_ctx);
                                auto* dt = dt::DrawTalk::ctx();
                                mem::deallocate<dt::Parse>(&dt->parse_allocation.allocator, p);
                            });
                            
                            
                            

                            
                        }
                    }
                }
            }
        }
            
    }

}

typedef enum INTERP_STATUS {
    INTERP_STATUS_OK,
    INTERP_STATUS_PARTIAL_INCOMPLETE,
    INTERP_STATUS_ALL_INCOMPLETE,
    
    INTERP_STATUS_COUNT
} INTERP_STATUS;

struct Interp_Status {
    INTERP_STATUS type;
};

using Interp_Result = mtt::Result<Word_Info*, Interp_Status>;
using SI = Speech_Property;



Interp_Result handle_IN(dt::Lang_Eval_Args* args, dt::Speech_Token& token);
Interp_Result handle_WHRB(dt::Lang_Eval_Args* args, dt::Speech_Token& token);
Interp_Result handle_RB(dt::Lang_Eval_Args* args, dt::Speech_Token& token);
Interp_Result handle_noun(dt::Lang_Eval_Args* args, dt::Speech_Token& token);
Interp_Result handle_det(dt::Lang_Eval_Args* args, dt::Speech_Token& token);
Interp_Result handle_aux(dt::Lang_Eval_Args* args, dt::Speech_Token& token);
Interp_Result handle_verb(dt::Lang_Eval_Args* args, dt::Speech_Token& token);
Interp_Result handle_pronoun(dt::Lang_Eval_Args* args, dt::Speech_Token& token);
Interp_Result handle_adj(dt::Lang_Eval_Args* args, dt::Speech_Token& token);
//Interp_Result handle_verb_root(dt::Lang_Eval_Args* args, dt::Dependency_Token& token);

MTT_NOINLINE void debug_break()
{
}

#define UNHANDLED() DT_scope_open(); DT_print("WARNING: Unhandled case!"); debug_break(); DT_scope_close()

Interp_Result handle_pronoun(dt::Lang_Eval_Args* args, dt::Speech_Token& token)
{
    args->curr_dep_token = &token;
    DT_print("%s, lemma=[%s] pos=[%s] dep=[%s]\n", token.text.c_str(), token.lemma.c_str(), dt::spacy_nlp::pos_str[token.pos].c_str(), dt::spacy_nlp::dep_str[token.dep].c_str());
    namespace nlp = dt::spacy_nlp;
    
    auto* dt    = args->dt;
    auto* parse = args->parse;
    
    DT_scope_open();
    
    auto& left_children  = token.left_children;
    auto& right_children = token.right_children;
    
    token.info.type = WORD_INFO_TYPE_PRONOUN;
    token.info.token = &token;
    
    dt::Speech_Property& prp = *dt::Speech_Property::make();
    prp.token = &token;
    token.info.prop_list.push_back(&prp);
    prp.type_str = "pronoun";
    prp.label = token.lemma;
    Speech_Token_set_prop(&token, &prp);
    
    Pronoun_Info& info = token.info.pronoun_info;
    info.token = &token;
    
    info.person_type = PERSON_TYPE_UNKNOWN;
    //    "morph": {
    //        "Case": "Nom",
    //        "Number": "Plur",
    //        "Person": "3",
    //        "PronType": "Prs"
    //    },
    {
        mtt::String* val = nullptr;
        for (auto en = token.morph.begin(); en != token.morph.end(); ++en) {
            DT_print("%s:%s\n", en->first.c_str(), en->second.c_str());
        }
        if (mtt::map_try_get(&token.morph, "Number", &val)) {
            info.is_plural = (*val == "Plur");
        } else {
            // assume singular
            info.is_plural = false;
        }
    }
    {
//        mtt::String* val = nullptr;
//        if (mtt::map_try_get(&token.morph, "Person", &val)) {
//            if (*val == "1") {
//                info.person_type = PERSON_TYPE_FIRST;
//                //UNHANDLED();
//            } else if (*val == "2") {
//                info.person_type = PERSON_TYPE_SECOND;
//                UNHANDLED();
//            } else if (*val == "3") {
//                info.person_type = PERSON_TYPE_THIRD;
//            }
//
//            auto& prop = *prp.push_prop("PROPERTY");
//            prop.type_str = "PERSON";
//            prop.value.kind_string = "TEXT";
//            prop.value.text = *val;
//        } else if (token.morph.find("Person_three") != token.morph.end()) {
//            info.person_type = PERSON_TYPE_THIRD;
//            auto& prop = *prp.push_prop("PROPERTY");
//            prop.type_str = "PERSON";
//            prop.value.kind_string = "TEXT";
//            prop.value.text = *val;
//        } else if (token.morph.find("Person_two") != token.morph.end()) {
//            info.person_type = PERSON_TYPE_SECOND;
//            auto& prop = *prp.push_prop("PROPERTY");
//            prop.type_str = "PERSON";
//            prop.value.kind_string = "TEXT";
//            prop.value.text = *val;
//        } else if (token.morph.find("Person_one") != token.morph.end()) {
//            info.person_type = PERSON_TYPE_FIRST;
//            auto& prop = *prp.push_prop("PROPERTY");
//            prop.type_str = "PERSON";
//            prop.value.kind_string = "TEXT";
//            prop.value.text = *val;
//        } else {
//            DT_print("Somehow there is an unknown pronoun\n");
//            UNHANDLED();
//        }
    }
    
    
    
    Nominal_Info& n_info = info.corresponding_nom_info;
    n_info.token         = &token;
    n_info.is_pronoun    = true;
    n_info.is_unresolved = true;
    n_info.is_plural = info.is_plural;
    
    switch (token.dep) {
    default: {
        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
        break;
    }
    case spacy_nlp::DEP_dobj:
        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
        break;
    case spacy_nlp::DEP_nsubj:
        n_info.is_main_actor_or_agent = true;
        n_info.should_likely_represent = true;
        break;
    case spacy_nlp::DEP_nsubjpass:
        n_info.is_acted_upon_or_patient = true;
        n_info.should_likely_represent = true;
        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
        break;
    case spacy_nlp::DEP_pobj:
        n_info.is_object_of_preposition = true;
        n_info.should_likely_represent = true;
        n_info.is_acted_upon_or_patient = true;
        break;
    }
    // this is a possessive pronoun
    if (token.dep == nlp::DEP_poss) {
        prp.type_str = "RELATION_POSSESSED_BY";
    }
    
    if (!info.is_plural) {
        prp.kind_str = "THING_INSTANCE";
        prp.value.kind_string = "THING_INSTANCE";
        prp.value.thing = mtt::Thing_ID_INVALID;
        auto& count_prop = *prp.push_prop("COUNT");
        count_prop.type_str = "VALUE";
        auto& val = count_prop.value;
        val.kind_string = "NUMERIC";
        val.numeric = 1.0f;
        
        
        auto& plural_prop = *prp.push_prop("PLURAL");

        plural_prop.type_str = "VALUE";
        plural_prop.value.kind_string = "FLAG";
        plural_prop.value.flag = false;

    } else {
        prp.kind_str = "THING_INSTANCE";
        prp.value.kind_string = "LIST";
        
        auto& plural_prop = *prp.push_prop("PLURAL");
        plural_prop.type_str = "VALUE";
        plural_prop.value.kind_string = "FLAG";
        plural_prop.value.flag = true;
    }
    
    auto* spec = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
    spec->value.kind_string = "FLAG";
    spec->value.flag = true;
    
    dt->sys_eval_ctx.noun_list.emplace_back(&token.info);
    
    
    DT_scope_close();
    
    return {Interp_Status{.type = INTERP_STATUS_OK}, &token.info};
}


Interp_Result handle_WHRB(dt::Lang_Eval_Args* args, dt::Speech_Token& token)
{
    args->curr_dep_token = &token;
    DT_print("%s, lemma=[%s] pos=[%s] dep=[%s]\n", token.text.c_str(), token.lemma.c_str(), dt::spacy_nlp::pos_str[token.pos].c_str(), dt::spacy_nlp::dep_str[token.dep].c_str());
    namespace nlp = dt::spacy_nlp;
    
    auto* dt    = args->dt;
    auto* parse = args->parse;
    
    DT_scope_open();
    
    auto& left_children  = token.left_children;
    auto& right_children = token.right_children;
    
    token.info.type = WORD_INFO_TYPE_WHADVERB;
    token.info.token = &token;
    
    
    WHAdverb_Info& info = token.info.whadverb_info;
    info.token = &token;
    //ASSERT_MSG((info.is_question == false && info.is_time_expression == false), "Need to do initialization!\n");
    
    // look-ahead to last token to check if a question
    // TODO: maybe just do this once on start
    if (parse->token_list.back()->text.compare("?") == 0) {
        info.is_question = true;
    } else {
        info.should_treat_as_rule = true;
    }
    
    DT_scope_close();
    
    return {Interp_Status{.type = INTERP_STATUS_OK}, &token.info};
}

Interp_Result handle_RB(dt::Lang_Eval_Args* args, dt::Speech_Token& token)
{
    args->curr_dep_token = &token;
    DT_print("%s, lemma=[%s] pos=[%s] dep=[%s]\n", token.text.c_str(), token.lemma.c_str(), dt::spacy_nlp::pos_str[token.pos].c_str(), dt::spacy_nlp::dep_str[token.dep].c_str());
    namespace nlp = dt::spacy_nlp;
    
    auto* dt    = args->dt;
    auto* parse = args->parse;
    
    auto& left_children  = token.left_children;
    auto& right_children = token.right_children;
    
    DT_scope_open();
    
    token.info.type = WORD_INFO_TYPE_ADVERB;
    token.info.token = &token;
    
    dt::Speech_Property& prp = *dt::Speech_Property::make();
    prp.token = &token;
    token.info.prop_list.push_back(&prp);
    Speech_Token_set_prop(&token, &prp);
    
    prp.label = "modifier";
    prp.type_str = "PROPERTY";
    prp.tag_str = "ADV";
    prp.value.kind_string = "TEXT";
    prp.value.text = token.lemma;
    
    {
        
        usize L = 0;
        for (;
             L < left_children.size();
             L += 1) {
            
            Speech_Token& child = *parse->token_list[left_children[L]];
            if (child.is_blocked) {
                continue;
            }
            
            switch (child.pos) {
            default: {
                MTT_print("Unhandled case: FUNC=%s line=%d pos=[%s] dep=[%s]\n", __PRETTY_FUNCTION__, __LINE__, spacy_nlp::tag_str[child.tag].c_str(), spacy_nlp::dep_str[child.dep].c_str());
                UNHANDLED();
                break;
            }
            case spacy_nlp::POS_ADJ: {
                UNHANDLED();
                break;
            }
            case spacy_nlp::POS_ADV: {
                auto result = handle_RB(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    auto& props = result.value->prop_list;
                    for (auto it = props.begin(); it != props.end(); ++it) {
                        prp.push_prop("PROPERTY", *it);
                    }
                    break;
                }
                }
                break;
            }
            }
        }
        
        usize R = 0;
        for (;
             R < right_children.size();
             R += 1) {
            
            Speech_Token& child = *parse->token_list[right_children[R]];
            if (child.is_blocked) {
                continue;
            }
            
            switch (child.pos) {
            default: {
                MTT_print("Unhandled case: FUNC=%s line=%d pos=[%s] dep=[%s]\n", __PRETTY_FUNCTION__, __LINE__, spacy_nlp::tag_str[child.tag].c_str(), spacy_nlp::dep_str[child.dep].c_str());
                UNHANDLED();
                break;
            }
            case spacy_nlp::POS_ADJ: {
                UNHANDLED();
                break;
            }
            case spacy_nlp::POS_ADV: {
                auto result = handle_RB(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    auto& props = result.value->prop_list;
                    for (auto it = props.begin(); it != props.end(); ++it) {
                        token.info.prop_list.push_back(*it);
                    }
                    break;
                }
                }
                break;
            }
            }
        }
        
    }
    
    DT_scope_close();
    
    return {Interp_Status{.type = INTERP_STATUS_OK}, &token.info};
}

Interp_Result handle_adj(dt::Lang_Eval_Args* args, dt::Speech_Token& token)
{
    args->curr_dep_token = &token;
    DT_print("%s, lemma=[%s] pos=[%s] dep=[%s]\n", token.text.c_str(), token.lemma.c_str(), dt::spacy_nlp::pos_str[token.pos].c_str(), dt::spacy_nlp::dep_str[token.dep].c_str());
    namespace nlp = dt::spacy_nlp;
    
    auto* dt    = args->dt;
    auto* parse = args->parse;
    
    auto& left_children  = token.left_children;
    auto& right_children = token.right_children;
    
    DT_scope_open();
    
    token.info.type = WORD_INFO_TYPE_ADJECTIVE;
    token.info.token = &token;
    
    dt::Speech_Property& prp = *dt::Speech_Property::make();
    prp.token = &token;
    token.info.prop_list.push_back(&prp);
    Speech_Token_set_prop(&token, &prp);

    
    prp.label = "trait";
    prp.type_str = "PROPERTY";
    prp.tag_str = "ADJ";
    prp.value.kind_string = "TEXT";
    prp.value.text = token.lemma;
    
    
    
    bool is_negated = false;
    
    {
        
        usize L = 0;
        for (;
             L < left_children.size();
             L += 1) {
            
            Speech_Token& child = *parse->token_list[left_children[L]];
            if (child.is_blocked) {
                continue;
            }
            
            switch (child.pos) {
            default: {
                MTT_print("Unhandled case: FUNC=%s line=%d pos=[%s] dep=[%s]\n", __PRETTY_FUNCTION__, __LINE__, spacy_nlp::tag_str[child.tag].c_str(), spacy_nlp::dep_str[child.dep].c_str());
                UNHANDLED();
                break;
            }
            case spacy_nlp::POS_ADJ: {
                UNHANDLED();
                break;
            }
            case spacy_nlp::POS_ADV: {
                auto result = handle_RB(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    auto& props = result.value->prop_list;
                    for (auto it = props.begin(); it != props.end(); ++it) {
                        prp.push_prop("PROPERTY", *it);
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_PART: {
                if (child.dep == nlp::DEP_neg) {
                    std::vector<Speech_Property*>* props = nullptr;
                    if (prp.try_get_prop("NEGATED", &props)) {
                        is_negated = !is_negated;
                        props->front()->value.flag = is_negated;
                    } else {
                        is_negated = true;
                        auto& negated_prop = *prp.push_prop("NEGATED");
                        negated_prop.value.kind_string = "FLAG";
                        negated_prop.value.flag = true;
                    }
                }
                break;
            }
            case nlp::POS_NUM: {
                // TODO
                // TODO handle separately
                Speech_Property* count_prop = nullptr;
                if (!prp.try_get_only_prop("COUNT", &count_prop)) {
                    count_prop = prp.push_prop("COUNT");
                }
                
                count_prop->type_str = "VALUE";
                count_prop->value.kind_string = "NUMERIC";
                
                mtt::String text = child.lemma;
                replace_patterns_in_place(text, string_replacements);
                
                cstring num_str = text.c_str();
                char* end;
                auto result = strtod(num_str, &end);
                usize len = strlen(num_str);
                if ((end != num_str + len) || (result == 0 && num_str == end) ||
                    result == HUGE_VAL || result == -HUGE_VAL ||
                    result == HUGE_VALF || result == -HUGE_VALF ||
                    result == HUGE_VALL || result == -HUGE_VALL) {
                    
                    count_prop->value.numeric = 1.0;
                    
                    {
                        dt::Dynamic_Array<mtt::String> words;
                        words.push_back(child.lemma);
                        if (!child.left_children.empty()) {
                            Speech_Token* next_child = parse->token_list[child.left_children.back()];
                            do {
                                if (next_child->dep == nlp::DEP_compound && next_child->pos == nlp::POS_NUM) {
                                    words.push_back(next_child->lemma);
                                    if (next_child->left_children.empty()) {
                                        break;
                                    }
                                    next_child = parse->token_list[next_child->left_children.back()];
                                } else {
                                    break;
                                }
                            } while (1);
                        }
                        
                        for (usize i = 0; i < words.size(); i += 1) {
                            replace_patterns_in_place(words[i], string_replacements);
                        }
                        std::reverse(words.begin(), words.end());
                        
                        isize value = 1;
                        if (!dt::text2num(words, &value)) {
                            MTT_error("%s", "Number invalid\n");
                        }
                        
                        count_prop->value.numeric = value;
                    }
                } else {
                    count_prop->value.numeric = result;
                }
                if (!child.right_children.empty()) {
                    Speech_Token& r_child = *parse->token_list[child.right_children[0]];
                    if (r_child.dep == spacy_nlp::DEP_quantmod && r_child.text == "times") {
                        count_prop->kind_str = "multiplier";
                    }
                }
                break;
            }
            }
        }
        
        usize R = 0;
        for (;
             R < right_children.size();
             R += 1) {
            
            Speech_Token& child = *parse->token_list[right_children[R]];
            if (child.is_blocked) {
                continue;
            }
            
            switch (child.pos) {
            default: {
                MTT_print("Unhandled case: FUNC=%s line=%d pos=[%s] dep=[%s]\n", __PRETTY_FUNCTION__, __LINE__, spacy_nlp::tag_str[child.tag].c_str(), spacy_nlp::dep_str[child.dep].c_str());
                UNHANDLED();
                break;
            }
            case nlp::POS_SCONJ: {
                DT_print("WARNING MAY HAVE TO CHANGE");
                
                // comparison
                if (child.lemma == "than") {
                    prp.label = "comparison";
                    for (usize R_c = 0; R_c < child.right_children.size(); R_c += 1) {
                        auto& rchild = *parse->token_list[child.right_children[R_c]];
                        switch (rchild.pos) {
                        default: {
                            UNHANDLED();
                            break;
                        }
                        case nlp::POS_PRON: {
                            auto result = handle_pronoun(args, rchild);
                            switch (result.status.type) {
                            default: {
                                UNHANDLED();
                                break;
                            }
                            case INTERP_STATUS_OK: {
                                auto& props = result.value->prop_list;
                                for (auto it = props.begin(); it != props.end(); ++it) {
                                    token.info.prop_list.push_back(*it);
                                }
                                break;
                            }
                            }
                            break;
                        }
                        case nlp::POS_NUM: {
                            // TODO
                            // TODO handle separately
                            Speech_Property* count_prop = nullptr;
                            if (!prp.try_get_only_prop("COUNT", &count_prop)) {
                                count_prop = prp.push_prop("COUNT");
                            }
                            
                            count_prop->type_str = "VALUE";
                            count_prop->value.kind_string = "NUMERIC";
                            
                            count_prop->token = &rchild;
                            rchild.prop_ref = count_prop;
                            
                            Speech_Property* spec = nullptr;
                            if (!prp.try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec)) {
                                spec = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                                spec->value.kind_string = "FLAG";
                                spec->value.flag = false;
                            } else {
                                spec->value.kind_string = "FLAG";
                                spec->value.flag = true;
                            }
                            

                            mtt::String text = rchild.lemma;
                            replace_patterns_in_place(text, string_replacements);
                            
                            cstring num_str = text.c_str();
                            char* end;
                            auto result = strtod(num_str, &end);
                            usize len = strlen(num_str);
                            if ((end != num_str + len) || result == 0 ||
                                result == HUGE_VAL || result == -HUGE_VAL ||
                                result == HUGE_VALF || result == -HUGE_VALF ||
                                result == HUGE_VALL || result == -HUGE_VALL) {
                                
                                count_prop->value.numeric = 1.0;
                                
                                {
                                    dt::Dynamic_Array<mtt::String> words;
                                    words.push_back(child.lemma);
                                    if (!child.left_children.empty()) {
                                        Speech_Token* next_child = parse->token_list[child.left_children.back()];
                                        do {
                                            if (next_child->dep == nlp::DEP_compound && next_child->pos == nlp::POS_NUM) {
                                                words.push_back(next_child->lemma);
                                                if (next_child->left_children.empty()) {
                                                    break;
                                                }
                                                next_child = parse->token_list[next_child->left_children.back()];
                                            } else {
                                                break;
                                            }
                                        } while (1);
                                    }
                                    
                                    for (usize i = 0; i < words.size(); i += 1) {
                                        replace_patterns_in_place(words[i], string_replacements);
                                    }
                                    std::reverse(words.begin(), words.end());
                                    
                                    isize value = 1;
                                    if (!dt::text2num(words, &value)) {
                                        MTT_error("%s", "Number invalid\n");
                                    }
                                    
                                    count_prop->value.numeric = value;
                                }
                            } else {
                                count_prop->value.numeric = result;
                            }

                            break;
                        }
                        case nlp::POS_PROPN:
                            // fallthrough
                        case nlp::POS_NOUN: {
                            auto result = handle_noun(args, rchild);
                            switch (result.status.type) {
                            default: {
                                UNHANDLED();
                                break;
                            }
                            case INTERP_STATUS_OK: {
                                auto& props = result.value->prop_list;
                                for (auto it = props.begin(); it != props.end(); ++it) {
                                    prp.push_prop("ARG", *it);
                                    //token.info.prop_list.push_back(*it);
                                }
                                break;
                            }
                            }
                            
                            break;
                        }
                        }
                    }
                }
                
                break;
            }
            case spacy_nlp::POS_ADJ: {
                
                auto result = handle_adj(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    auto& props = result.value->prop_list;
                    for (auto it = props.begin(); it != props.end(); ++it) {
                        token.info.prop_list.push_back(*it);
                    }
                    break;
                }
                }
                
                
                break;
            }
            case spacy_nlp::POS_ADV: {
                UNHANDLED();
                break;
            }
            }
        }
    }
    
    DT_scope_close();
    
    return {Interp_Status{.type = INTERP_STATUS_OK}, &token.info};
    
}

Interp_Result handle_noun(dt::Lang_Eval_Args* args, dt::Speech_Token& token)
{
    args->curr_dep_token = &token;
    DT_print("%s, lemma=[%s] pos=[%s] dep=[%s] found selection=[%s]\n", token.text.c_str(), token.lemma.c_str(), dt::spacy_nlp::pos_str[token.pos].c_str(), dt::spacy_nlp::dep_str[token.dep].c_str(), bool_str(token.timing.was_selected));
    
    namespace nlp = dt::spacy_nlp;
    if (token.text == "an") {
        token.pos = nlp::POS_DET;
    }
    
    auto* dt      = args->dt;
    auto* parse   = args->parse;
    auto* sys_ctx = &dt->sys_eval_ctx;
    DT_scope_open();
    
    auto& left_children  = token.left_children;
    auto& right_children = token.right_children;
    DT_print("l-child count=[%lu], r-child count=[%lu]\n", left_children.size(), right_children.size());
    
    token.info.type = WORD_INFO_TYPE_NOMINAL;
    token.info.token = &token;
    
    dt::Speech_Property& prp = *dt::Speech_Property::make();
    prp.token = &token;
    token.info.prop_list.push_back(&prp);
    Speech_Token_set_prop(&token, &prp);
    
    
    prp.tag_str = (token.tag == spacy_nlp::TAG_VBG) ? "ACTION_AS_NOUN" : "NOUN";
    
    
    Nominal_Info& info = token.info.nominal;
    info.token = &token;
    

    
    for (auto it = token.morph.begin(); it != token.morph.end(); ++it) {
        DT_print("morph: %s %s\n", it->first.c_str(), it->second.c_str());
    }
    // initially is singular unless it's a mass ... will make best guess
    // based on whether there is a prepositional phrase
    // suggesting that this is actually a group/mass
    bool plural_or_singular_is_known = false;
    
    auto time_found = dt::Speech_Property::STRING_TO_TIME_UNIT.find(token.lemma);
    if (time_found != dt::Speech_Property::STRING_TO_TIME_UNIT.end()) {
        info.is_time_expression = true;
        prp.tag_str = "TIME";
    } else if (dt::Speech_Property::STRING_TO_TIME_EXPRESSION.find(token.lemma) !=
               dt::Speech_Property::STRING_TO_TIME_EXPRESSION.end()) {
        info.is_time_expression = true;
        prp.tag_str = "TIME";
    } else {
        auto found = token.morph.find("Number");
        if (found != token.morph.end()) {
            if (found->second == "Sing") {
                info.is_plural = false;
                auto& plural_prop = *prp.push_prop("PLURAL");
                //plural_prop.tag_str = "__SYSTEM__";
                plural_prop.type_str = "VALUE";
                plural_prop.value.kind_string = "FLAG";
                plural_prop.value.flag = false;
                
            } else {
                info.is_plural = true;
                auto& plural_prop = *prp.push_prop("PLURAL");
                //plural_prop.tag_str = "__SYSTEM__";
                plural_prop.type_str = "VALUE";
                plural_prop.value.kind_string = "FLAG";
                plural_prop.value.flag = true;
                if (token.lemma == "people") {
                    token.lemma = "person";
                }
            }
            
            plural_or_singular_is_known = true;
            
        } else {
            info.is_plural = !spacy_nlp::pos_tag_noun_is_singular_or_mass(token.tag);
            
            if (!info.is_plural) {
                info.is_plural = false;
                auto& plural_prop = *prp.push_prop("PLURAL");
                //plural_prop.tag_str = "__SYSTEM__";
                plural_prop.type_str = "VALUE";
                plural_prop.value.kind_string = "FLAG";
                plural_prop.value.flag = false;
            } else {
                info.is_plural = true;
                auto& plural_prop = *prp.push_prop("PLURAL");
                //plural_prop.tag_str = "__SYSTEM__";
                plural_prop.type_str = "VALUE";
                plural_prop.value.kind_string = "FLAG";
                plural_prop.value.flag = true;
                if (token.lemma == "people") {
                    token.lemma = "person";
                }
            }
            
            
            DT_print("Warning, may not be good to assume this!\n");
            plural_or_singular_is_known = false;
        }
        
        
        
        
    }
    
    //    {
    //        mtt::String* val = nullptr;
    //        auto& person_prop = *ins.push_prop("PERSON");
    //        person_prop.type_str = "VALUE";
    //        person_prop.value.kind_string = "TEXT";
    //
    //        if (mtt::map_try_get(&token.morph, "Person", &val)) {
    //            if (*val == "1") {
    //                person_prop.value.text = "PERSON_TYPE_FIRST";
    //            } else if (*val == "2") {
    //                person_prop.value.text = "PERSON_TYPE_SECOND";
    //            } else if (*val == "3") {
    //                person_prop.value.text = "PERSON_TYPE_THIRD";
    //            } else {
    //                person_prop.value.text = "PERSON_TYPE_UNKNOWN";
    //            }
    //        } else {
    //            person_prop.value.text = "PERSON_TYPE_UNKNOWN";
    //        }
    //    }
    
    
    switch (token.dep) {
    default: {
        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
        break;
    }
    case spacy_nlp::DEP_compound: {
        info.should_likely_represent = false;
        break;
    }
    case spacy_nlp::DEP_dobj:
        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
        break;
    case spacy_nlp::DEP_nsubj:
        info.is_main_actor_or_agent = true;
        info.should_likely_represent = true;
        break;
    case spacy_nlp::DEP_nsubjpass:
        info.is_acted_upon_or_patient = true;
        info.should_likely_represent = true;
        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
        break;
    case spacy_nlp::DEP_pobj:
        info.is_object_of_preposition = true;
        info.should_likely_represent = true;
        info.is_acted_upon_or_patient = true;
        break;
    }
    
    bool is_possessed = false;
    mtt::String last_lemma = token.lemma;
    bool force_to_be_a_type = false;
    {
        usize L = 0;
        DT_print("L:\n");
        for (;
             L < left_children.size();
             L += 1) {
            
            Speech_Token& child = *parse->token_list[left_children[L]];
            if (child.is_blocked) {
                continue;
            }
            
            switch (child.pos) {
            default: {
                MTT_print("Unhandled case: FUNC=%s line=%d pos=[%s] dep=[%s]\n", __PRETTY_FUNCTION__, __LINE__, spacy_nlp::tag_str[child.tag].c_str(), spacy_nlp::dep_str[child.dep].c_str());
                UNHANDLED();
                break;
            }
            case nlp::POS_VERB: {
                auto result = handle_verb(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    if (child.tag == nlp::TAG_VB || child.dep == nlp::DEP_amod) {
                        for (Speech_Property* ch_prop : result.value->prop_list) {
                            auto* p = prp.push_prop("PROPERTY", ch_prop);
                            if (ch_prop->label == "name" || ch_prop->label == "call") {
                                p->type_str = "PROPERTY";
                                p->value.kind_string = "TEXT";
                                p->value.text = ch_prop->label;
                                p->label = "name";
                                p->tag_str = "";
                            } else {
                                p->type_str = "PROPERTY";
                                p->value.kind_string = "TEXT";
                                p->value.text = ch_prop->label;
                                p->label = "trait";
                                p->tag_str = "ADJ";
                            }
                        }
                    }
                    break;
                }
                }
            }
            case nlp::POS_PRON: {
                auto result = handle_pronoun(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                    switch (child.tag) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                        // this is a possessive pronoun
                    case nlp::TAG_PRP$: {
                        //UNHANDLED();
                        is_possessed = true;
                        DT_print("NUM: %lu\n", child.info.prop_list.size());
                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                            auto* prop = *it;
                            prp.push_prop("RELATION", prop);
                        }
                        
                        std::vector<Speech_Property*>* props = nullptr;
                        if (!prp.try_get_prop("SPECIFIC_OR_UNSPECIFIC", &props)) {
                            auto* spec = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                            spec->value.kind_string = "FLAG";
                            spec->value.flag = true;
                        } else {
                            for (auto s_it = props->begin(); s_it != props->end(); ++s_it) {
                                (*s_it)->value.flag = true;
                            }
                        }
                        
                        
                        
                        prp.value.kind_string = "THING_INSTANCE";
                        prp.value.thing = mtt::Thing_ID_INVALID;
                        break;
                    }
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_PROPN: {
                // fallthrough
            }
            case nlp::POS_NOUN: {
                if (child.dep == spacy_nlp::DEP_compound) {
                    // should merge info because we're treating compound nouns as one entity.
                    if (!token.timing.was_selected && child.timing.was_selected) {
                        token.timing = child.timing;
                    }
                    
                    prp.label += child.lemma + " ";
                    break;
                }
                
                auto result = handle_noun(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    if (!result.value->nominal.should_likely_represent) {
                        if (child.dep == spacy_nlp::DEP_compound) {
                            // TODO
                        }
                    }
                    
                    
                    switch (child.dep) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case nlp::DEP_poss: {
                        //UNHANDLED();
                        is_possessed = true;
                        
                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                            auto* prop = *it;
                            prp.push_prop("RELATION", prop);
                            prop->type_str = "RELATION_POSSESSED_BY";
                            prop->label = (*it)->label;
                        }
                        
                        std::vector<Speech_Property*>* props = nullptr;
                        if (!prp.try_get_prop("SPECIFIC_OR_UNSPECIFIC", &props)) {
                            auto* spec = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                            spec->value.kind_string = "FLAG";
                            spec->value.flag = true;
                        } else {
                            for (auto s_it = props->begin(); s_it != props->end(); ++s_it) {
                                (*s_it)->value.flag = true;
                            }
                        }
                        
                        
                        prp.value.kind_string = "THING_INSTANCE";
                        prp.value.thing = mtt::Thing_ID_INVALID;
                        break;
                    }
                    }
                    
                    break;
                }
                }
                break;
            }
            case nlp::POS_NUM: {
                if (child.dep == spacy_nlp::DEP_nummod) {
                    float64 val_out = 1.0;
                    Speech_Property* count_prop = nullptr;
                    if (!prp.try_get_only_prop("COUNT", &count_prop)) {
                        count_prop = prp.push_prop("COUNT");
                    }
                    
                    count_prop->type_str = "VALUE";
                    count_prop->value.kind_string = "NUMERIC";
                    
                    count_prop->token = &child;
                    child.prop_ref = count_prop;
                    
                    Speech_Property* spec = nullptr;
                    if (!prp.try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec)) {
                        spec = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                        spec->value.kind_string = "FLAG";
                        spec->value.flag = false;
                    } else {
                        spec->value.kind_string = "FLAG";
                        spec->value.flag = true;
                    }
                    
                    if (child.lemma == "to") {
                        child.lemma = "two";
                    } else if (child.lemma == "for") {
                        child.lemma = "four";
                    }
                    mtt::String text = child.lemma;
                    replace_patterns_in_place(text, string_replacements);
                    
                    cstring num_str = text.c_str();
                    char* end;
                    auto result = strtod(num_str, &end);
                    usize len = strlen(num_str);
                    if ((end != num_str + len) || (result == 0 && num_str == end) ||
                        result == HUGE_VAL || result == -HUGE_VAL ||
                        result == HUGE_VALF || result == -HUGE_VALF ||
                        result == HUGE_VALL || result == -HUGE_VALL) {
                        
                        count_prop->value.numeric = 1.0;
                        
                        {
                            dt::Dynamic_Array<mtt::String> words;
                            words.push_back(child.lemma);
                            if (!child.left_children.empty()) {
                                Speech_Token* next_child = parse->token_list[child.left_children.back()];
                                do {
                                    if (next_child->dep == nlp::DEP_compound && next_child->pos == nlp::POS_NUM) {
                                        words.push_back(next_child->lemma);
                                        if (next_child->left_children.empty()) {
                                            break;
                                        }
                                        next_child = parse->token_list[next_child->left_children.back()];
                                    } else {
                                        break;
                                    }
                                } while (1);
                            }
                            
                            for (usize i = 0; i < words.size(); i += 1) {
                                replace_patterns_in_place(words[i], string_replacements);
                            }
                            std::reverse(words.begin(), words.end());
                            

                            
                            isize value = 1;
                            if (!dt::text2num(words, &value)) {
                                //MTT_error("%s", "Number invalid\n");
//                                auto& SELF_TOKE = token
//                                ;
//                                for (usize L_ = 0; L_ < left_children.size(); L_ += 1) {
//                                    Speech_Token& TOK = *parse->token_list[left_children[L_]];
//                                    MTT_print("%s\n", TOK.lemma.c_str());
//                                }
//                                for (usize L_ = 0; L_ < parse->token_list.size(); L_ += 1) {
//                                    Speech_Token& TOK = *parse->token_list[L_];
//                                    MTT_print("%s\n", TOK.lemma.c_str());
//                                }
                                usize inc_by = 0;
                                if (left_children.size() >= 2) {
                                    
                                    if (left_children.size() == 2 && L <= left_children.size() - 2) {
                                        
                                        auto pt_idx = left_children[L] + 1;
                                        if (pt_idx < parse->token_list.size()) {
                                            Speech_Token& POINT = *parse->token_list[pt_idx];
                                            if (POINT.lemma == ".") {
                                                Speech_Token& NUM = *parse->token_list[left_children[L+1]];
                                                
                                                
                                                if (NUM.lemma.size() > 0 && dt::text2num({NUM.lemma}, &value)) {
                                                    float64 log10_part = log10(value) + 1;
                                                    float64 pow_part = ((float64)pow(10, (int(log10_part))));
                                                    val_out = ((float64)value) / pow_part;
                                                    
                                                    inc_by = 1;
                                                }
                                            }
                                        }
                                    } else if (left_children.size() == 3 && L <= left_children.size() - 3) {
                                        
                                        auto pt_idx = left_children[L+1];
                                        if (pt_idx < parse->token_list.size()) {
                                            Speech_Token& POINT = *parse->token_list[pt_idx];
                                            if (POINT.lemma == ".") {
                                                Speech_Token& NUM = *parse->token_list[left_children[L+2]];
                                                
                                                
                                                if (NUM.lemma.size() > 0 && dt::text2num({NUM.lemma}, &value)) {
                                                    float64 log10_part = log10(value) + 1;
                                                    float64 pow_part = ((float64)pow(10, (int(log10_part))));
                                                    val_out = ((float64)value) / pow_part;
                                                    
                                                    inc_by = 2;
                                                }
                                            }
                                        }
                                    }
                                }
                                L += inc_by;
                                count_prop->value.numeric = val_out;
                            } else {
                                count_prop->value.numeric = value;
                            }
                            
                            
                        }
                    } else {
                         
                        usize inc_by = 0;
                        
                        if (L + 2 < parse->token_list.size()) {
                            float64 val_out = 0.0;
                            isize value = 0;
                            if (left_children.size() == 2 && L <= left_children.size() - 2) {
                                
                                auto pt_idx = left_children[L] + 1;
                                if (pt_idx < parse->token_list.size()) {
                                    Speech_Token& POINT = *parse->token_list[pt_idx];
                                    if (POINT.lemma == ".") {
                                        Speech_Token& NUM = *parse->token_list[left_children[L+1]];
                                        
                                        
                                        if (NUM.lemma.size() > 0 && dt::text2num({NUM.lemma}, &value)) {
                                            float64 log10_part = log10(value) + 1;
                                            float64 pow_part = ((float64)pow(10, (int(log10_part))));
                                            val_out = ((float64)value) / pow_part;
                                            
                                            inc_by = 1;
                                        }
                                    }
                                }
                            } else if (left_children.size() == 3 && L <= left_children.size() - 3) {
                                
                                auto pt_idx = left_children[L+1];
                                if (pt_idx < parse->token_list.size()) {
                                    Speech_Token& POINT = *parse->token_list[pt_idx];
                                    if (POINT.lemma == ".") {
                                        Speech_Token& NUM = *parse->token_list[left_children[L+2]];
                                        
                                        
                                        if (NUM.lemma.size() > 0 && dt::text2num({NUM.lemma}, &value)) {
                                            float64 log10_part = log10(value) + 1;
                                            float64 pow_part = ((float64)pow(10, (int(log10_part))));
                                            val_out = ((float64)value) / pow_part;
                                            
                                            inc_by = 2;
                                        }
                                    }
                                }
                            }
                            count_prop->value.numeric = result + val_out;
                            L += inc_by;
                        } else {
                            count_prop->value.numeric = result;
                        }
                    }
                    
                    if (!child.right_children.empty()) {
                        Speech_Token& r_child = *parse->token_list[child.right_children[0]];
                        if (r_child.dep == spacy_nlp::DEP_quantmod && r_child.text == "times") {
                            count_prop->kind_str = "multiplier";
                        }
                    }
                    //prp.print();
//#ifndef NDEBUG
//                    if (count_prop != nullptr) {
//                        auto VAL = count_prop->value.numeric;
//                    }
//#endif
                }
                break;
            }
            case nlp::POS_DET: {
                auto result = handle_det(args, child);
                
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                    if (info.is_time_expression) {
                        if (result.value->token->lemma == "every" || result.value->token->lemma == "each") {
                            prp.type_str = "INTERVAL";
                        }
                        break;
                    } else if (result.value->token->lemma == "every" || result.value->token->lemma == "each" || result.value->token->lemma == "all") {
                        prp.kind_str = "THING_TYPE";
                        force_to_be_a_type = true;
                    }
                    
                    SI& c_ins = *result.value->prop_list.front();
                    DT_print("%s\n", c_ins.kind_str.c_str());
                    
                    // we need to figure out if queries are necessary or if we can just select specific values
                    if (c_ins.kind_str == "SPECIFIC") {
                        //if (token.timing.was_selected) {
                        prp.kind_str = "THING_INSTANCE";
                        
                        if (!prp.has_prop("COUNT")) {
                            auto& val_inst = *prp.push_prop("COUNT");
                            //val_inst.label = "__SYSTEM__";
                            val_inst.type_str = "VALUE";
                            auto& val = val_inst.value;
                            val.kind_string = "NUMERIC";
                            if (c_ins.label == "twice") {
                                // patch
                                val.numeric = 2.0f;
                            } else {
                                val.numeric = 1.0f;
                            }
                        }
                        
                        if (!info.is_plural) {
                            prp.value.kind_string = "THING_INSTANCE";
                            //ins.value.thing = token.timing.selection.ID;
                        } else {
                            prp.value.kind_string = "LIST";
                        }
                        
                    } else if (c_ins.kind_str == "UNSPECIFIC") {
                        if (token.timing.was_selected) {
                            prp.kind_str = "THING_INSTANCE";
                            
                            if (!prp.has_prop("COUNT")) {
                                auto& val_inst = *prp.push_prop("COUNT");
                                //val_inst.label = "__SYSTEM__";
                                auto& val = val_inst.value;
                                val.kind_string = "NUMERIC";
                                val.numeric = 1.0f;
                            }
                            
                            if (!info.is_plural) {
                                prp.value.kind_string = "THING_INSTANCE";
                            } else {
                                DT_print("Warning: unspecific, but plural unhandled\n");
                                continue;
                            }
                        } else {
                            // patch
                            if (c_ins.label == "twice") {
                                if (!prp.has_prop("COUNT")) {
                                    auto& val_inst = *prp.push_prop("COUNT");
                                    //val_inst.label = "__SYSTEM__";
                                    auto& val = val_inst.value;
                                    val.kind_string = "NUMERIC";
                                    val.numeric = 2.0f;
                                } else {
                                    auto& val_inst = *prp.try_get_only_prop("COUNT");
                                    auto& val = val_inst.value;
                                    val.kind_string = "NUMERIC";
                                    val.numeric = 2.0f;
                                }
                            } else if (c_ins.label == "both") {
                                if (!prp.has_prop("COUNT")) {
                                    auto& val_inst = *prp.push_prop("COUNT");
                                    //val_inst.label = "__SYSTEM__";
                                    auto& val = val_inst.value;
                                    val.kind_string = "NUMERIC";
                                    val.numeric = 2.0f;
                                } else {
                                    auto& val_inst = *prp.try_get_only_prop("COUNT");
                                    auto& val = val_inst.value;
                                    val.kind_string = "NUMERIC";
                                    val.numeric = 2.0f;
                                }
                                
                                if (!prp.has_prop("SPECIFIC_OR_UNSPECIFIC")) {
                                    auto& spec = *prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                                    spec.value.kind_string = "FLAG";
                                    spec.value.flag = true;
                                } else {
                                    auto& spec = *prp.try_get_only_prop("SPECIFIC_OR_UNSPECIFIC");
                                    spec.value.kind_string = "FLAG";
                                    spec.value.flag = true;
                                }
                                
                                
                            } else if (c_ins.label == "a" || c_ins.label == "an") {
                                if (!prp.has_prop("COUNT")) {
                                    auto& val_inst = *prp.push_prop("COUNT");
                                    //val_inst.label = "__SYSTEM__";
                                    auto& val = val_inst.value;
                                    val.kind_string = "NUMERIC";
                                    val.numeric = 1.0f;
                                    if (!prp.has_prop("SPECIFIC_OR_UNSPECIFIC")) {
                                        auto& spec = *prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                                        spec.value.kind_string = "FLAG";
                                        spec.value.flag = false;
                                    }
                                } else {
                                    auto& val_inst = *prp.try_get_only_prop("COUNT");
                                    auto& val = val_inst.value;
                                    val.kind_string = "NUMERIC";
                                    val.numeric = 1.0f;
                                }
                            }
                            
                        }
                    }
                    
                    if (!prp.has_prop("SPECIFIC_OR_UNSPECIFIC")) {
                        prp.push_prop("SPECIFIC_OR_UNSPECIFIC", &c_ins);
                    }
                    
                    
                    
                    // TODO: //
                    //                    if (result.value->determiner_info.is_guaranteed_plural) {
                    //                        // is definitely multiple entities
                    //                        // TODO: ...
                    //                        info.is_plural = true;
                    //                        int BREAK = 0;
                    //                    } else {
                    //                        // could be a group or a singular entity (unknown)
                    //                        // TODO: ...
                    //
                    //
                    //                    }
                    //
                    mtt::String* det_text = &result.value->token->lemma;
                    //                    if (*det_text == "the" ||
                    //                        *det_text == "The") {
                    //                        info.specific_entity = true;
                    //                        int BREAK = 1;
                    //                    }
                    
                    //if constexpr ((false))
                    {
                        auto dx_it = dt::deictic_words.find(*det_text);
                        if (dx_it != dt::deictic_words.end()) {
                            if (!result.value->prop_list.empty()) {
                            
                                Speech_Property* d_prop = *(result.value->prop_list.begin());
                                MTT_BP();
                                
                                auto* deixis     = prp.push_prop("DEIXIS");
                                deixis->label    = *dx_it;
                                deixis->type_str = "DEIXIS";
                            }
//                            if (!info.token->timing.was_selected) {
//                                info.token->timing.selection           = result.value->token->timing.selection;
//                                info.token->timing.utterance_timestamp = result.value->token->timing.utterance_timestamp;
//                                info.token->timing_taken_from_reference = true;
//                                info.specific_entity                    = true;
//                            }
                        }
                    }
                    
                    
                    
                    break;
                }
                }
                break;
            }
            case nlp::POS_ADJ: {
                auto result = handle_adj(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    auto& props = result.value->prop_list;
                    for (auto it = props.begin(); it != props.end(); ++it) {
                        if ((*it)->token->lemma == "new") {
                            prp.treat_as_unique = true;
                            continue;
                        }
                        else if ((*it)->token->lemma == "different") {
                            prp.treat_as_different_next_in_sequence = true;
                            prp.treat_as_unique = true;
                            continue;
                        } else if ((*it)->token->lemma == "unique") {
                            prp.treat_as_unique = true;
                            continue;
                        }
                        prp.push_prop("PROPERTY", *it);
                        (*it)->key = "PROPERTY";
                    }
                    break;
                }
                }
                
                break;
            }
            }
            
            
        }
        usize R = 0;
        usize R_COUNT = right_children.size();
        DT_print("R:\n");
        for (;
             R < right_children.size();
             R += 1) {
            
            Speech_Token& child = *parse->token_list[right_children[R]];
            if (child.is_blocked) {
                continue;
            }
            
            switch (child.pos) {
            default: {
                MTT_print("Unhandled case: FUNC=%s line=%d pos=[%s] dep=[%s]\n", __PRETTY_FUNCTION__, __LINE__, spacy_nlp::tag_str[child.tag].c_str(), spacy_nlp::dep_str[child.dep].c_str());
                UNHANDLED();
                break;
            }
            case nlp::POS_PROPN: {
                // fallthrough
            }
            case nlp::POS_NOUN: {
                auto result = handle_noun(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    //                    auto& props = result.value->instruction;
                    //                    for (auto it = props.begin(); it != props.end(); ++it) {
                    //                        ins.push_prop("PROPERTY", *it);
                    //                        (*it)->key = "PROPERTY";
                    //                    }
                    
                    
                    if (child.dep == nlp::DEP_appos) {
                        
                        if (!prp.has_prop("SPECIFIC_OR_UNSPECIFIC")) {
                            auto* spec = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                            spec->value.kind_string = "FLAG";
                            spec->value.flag = true;
                        }
                        
                        if (is_possessed) {
                            auto* type_of_rel = prp.push_prop("PROPERTY");
                            type_of_rel->type_str = "PROPERTY";
                            type_of_rel->tag_str = "APPOS";
                            type_of_rel->label = "name";
                            type_of_rel->kind_str = "name";
                            type_of_rel->value.kind_string = "TEXT";
                            if (!result.value->prop_list.empty()) {
                                type_of_rel->value.text = result.value->prop_list.front()->label;
                            } else {
                                type_of_rel->value.text = "?";
                            }
                        }
                        else if (!child.info.prop_list.empty()) {
                            
                            std::vector<Speech_Property*>* relations = nullptr;
                            if (child.info.prop_list.front()->try_get_prop("RELATION", &relations)) {
                                FOR_PTR_ITER(rel_it, relations, ++rel_it) {
                                    if ((*rel_it)->type_str == "RELATION_POSSESSED_BY") {
                                        //                                        auto* type_of_rel = ins.push_prop("RELATION");
                                        //                                        type_of_rel->type_str = "RELATION_TYPE_OF";
                                        //                                        type_of_rel->tag_str = "APPOS";
                                        //                                        type_of_rel->value.kind_string = "TEXT";
                                        //                                        type_of_rel->value.text = child.lemma;
                                        auto* type_of_rel = prp.push_prop("PROPERTY");
                                        type_of_rel->type_str = "PROPERTY";
                                        type_of_rel->tag_str = "APPOS";
                                        type_of_rel->kind_str = "name";
                                        type_of_rel->label = "name";
                                        type_of_rel->value.kind_string = "TEXT";
                                        type_of_rel->value.text = token.text;
                                        
                                        last_lemma = child.info.prop_list.front()->label;
                                        
                                        break;
                                    }
                                }
                            }
                            
                            
                        }
                        
                        for (auto p_it = child.info.prop_list.begin(); p_it != child.info.prop_list.end(); ++p_it) {
                            
                            
                            for (auto sub_it = (*p_it)->properties.begin(); sub_it != (*p_it)->properties.end(); ++sub_it) {
                                if (sub_it->first == "RELATION" || sub_it->first == "PROPERTY" || sub_it->first == "SELECT") {
                                    for (auto merge_it = sub_it->second.begin(); merge_it != sub_it->second.end(); ++merge_it) {
                                        prp.push_prop(sub_it->first, *merge_it);
                                    }
                                    
                                }
                            }
                        }
                        for (auto c_it = child.info.prop_list.begin(); c_it != child.info.prop_list.end(); ++c_it) {
                            Speech_Property::destroy(*c_it);
                        }
                        
                        if (!prp.token->timing.was_selected && child.info.token->timing.was_selected) {
                            prp.token->timing = child.info.token->timing;
                        }
                    } else if (child.dep == nlp::DEP_conj) {
                        std::vector<Speech_Property*>* spec = nullptr;
                        bool has_spec = false;
                        if (prp.try_get_prop("SPECIFIC_OR_UNSPECIFIC", &spec)) {
                            has_spec = true;
                        }
                        for (auto p_it = child.info.prop_list.begin(); p_it != child.info.prop_list.end(); ++p_it) {
                            token.info.prop_list.push_back(*p_it);
                            if (!(*p_it)->has_prop("SPECIFIC_OR_UNSPECIFIC") && has_spec) {
                                (*p_it)->kind_str = prp.kind_str;
                                auto& prop = *((*p_it)->push_prop("SPECIFIC_OR_UNSPECIFIC"));
                                prop.value.kind_string = "FLAG";
                                prop.value.flag = spec->front()->value.flag;
                            }
                        }
                    } else if (child.dep == nlp::DEP_npadvmod) {
                        for (auto p_it = child.info.prop_list.begin(); p_it != child.info.prop_list.end(); ++p_it) {
                            prp.push_prop("PROPERTY", *p_it);
                        }
                    }
                    
                    
                    
                    break;
                }
                }
                
                break;
            }
            case nlp::POS_VERB: {
                auto result = handle_verb(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    auto& ch_props = result.value->prop_list;
                    if (ch_props.empty()) {
                        break;
                    }
                    
                    
                    
                    
                    if (child.dep == nlp::DEP_relcl) {
                        
                        result.value->prop_list.front()->print();
                        
                        for (auto ch_it = ch_props.begin(); ch_it != ch_props.end(); ++ch_it) {
                            auto& ch_prop = **ch_it;
                            prp.key = "AGENT";
                            //ch_prop.key = "ALSO_DIRECT_OBJECT";
                            //ch_prop.push_prop("AGENT", &prp);
                            //prp.key = "DIRECT_OBJECT";
                            
                            

//                                auto& sub = *Speech_Property::make();
//                                sub.label = "EXTRA_INFO";
//                                prp.push_prop("EXTRA_INFO", &sub);
//                                sub.push_prop(ch_prop.type_str, &ch_prop);
                            

                            
                            prp.push_prop(ch_prop.type_str, &ch_prop);
                            // ????
                            // FIXME: why is it necessary to do this?
                            

                            Speech_Property::Prop_List* properties = nullptr;
                            if (ch_prop.try_get_prop("PROPERTY", &properties)) {

                                for (auto p_it = properties->begin(); p_it != properties->end(); ++p_it) {
                                    auto* sub = prp.push_prop("PROPERTY");
                                    *sub = **p_it;
                                }

                                //ch_prop.remove_prop("PROPERTY");
                            }
                            
                            
                        }
                        

                        result.value->prop_list.front()->print();

                        MTT_BP();
                        prp.print();
                        MTT_BP();
                    } else {
                        for (auto ch_it = ch_props.begin(); ch_it != ch_props.end(); ++ch_it) {
                            auto& ch_prop = **ch_it;
                            ch_prop.key = ch_prop.type_str;
                            prp.push_prop(ch_props.front()->type_str, &ch_prop);
                        }
                        
                        prp.print();
                        
                    }

                    break;
                }
                }
                
                break;
            }
            case nlp::POS_ADJ: {
                auto result = handle_adj(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    auto& props = result.value->prop_list;
                    for (auto it = props.begin(); it != props.end(); ++it) {
                        prp.push_prop("PROPERTY", *it);
                        (*it)->key = "PROPERTY";
                    }
                    break;
                }
                }
                
                break;
            }
            case nlp::POS_ADP: {
                auto result = handle_IN(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    auto& props = result.value->prop_list;
                    for (auto it = props.begin(); it != props.end(); ++it) {
                        prp.push_prop("RELATION", *it);
                        (*it)->key = "RELATION";
//                        if ((*it)->annotation == "DO_NOT_REQUIRE_SELECTION") {
//                            prp.annotation = "DO_NOT_REQUIRE_SELECTION";
//                        }
                        if ((*it)->ignore_selections) {
                            prp.ignore_selections = true;
                        }
                    }
                    break;
                }
                }
                
                break;
            }
            case nlp::POS_NUM: {
                {
                    if (child.dep == spacy_nlp::DEP_nummod || child.dep == spacy_nlp::DEP_appos) {
                        
                        std::deque<Speech_Token*> numbers;
                        

                        
                        if (child.dep == spacy_nlp::DEP_appos) {
                            // check left
                            for (usize L_idx = 0; L_idx < child.left_children.size(); L_idx += 1) {
                                Speech_Token& child_n_l = *parse->token_list[child.left_children[L_idx]];
                                if (child_n_l.text == "," || child_n_l.pos == spacy_nlp::POS_PUNCT) {
                                    continue;
                                } else if (child_n_l.pos == spacy_nlp::POS_NUM) {
                                    numbers.push_back(&child_n_l);
                                } else {
                                    break;
                                }
                            }
                            if (numbers.size() != 0) {
                                numbers.push_back(&child);
                            }
                        }
                        if (numbers.size() == 0 && (child.dep == spacy_nlp::DEP_nummod || child.dep == spacy_nlp::DEP_appos)) {
                            usize offset = 0;
                            for (usize R_idx = 0; R_idx < child.right_children.size(); R_idx += 1) {
                                Speech_Token& child_n_r = *parse->token_list[child.right_children[R_idx]];
                                if (child_n_r.text == "," || child_n_r.pos == spacy_nlp::POS_PUNCT) {
                                    continue;
                                } else if (child_n_r.pos == spacy_nlp::POS_NUM) {
                                    numbers.push_back(&child_n_r);
                                } else {
                                    break;
                                }
                                
                            }
                            if (numbers.size() != 0) {
                                numbers.push_front(&child);
                            }
                        }
                        
                        if (numbers.size() == 0 && (child.dep == spacy_nlp::DEP_nummod || child.dep == spacy_nlp::DEP_appos)) {
                            numbers.push_back(&child);
                            usize r_count = 0;
                            bool num_found = false;
                            for (usize R_idx = R + 1; R_idx < token.right_children.size(); R_idx += 1) {
                                Speech_Token& child_n_r = *parse->token_list[token.right_children[R_idx]];
                                if (child_n_r.text == "," || child_n_r.pos == spacy_nlp::POS_PUNCT) {
                                    r_count += 1;
                                } else if (child_n_r.pos == spacy_nlp::POS_NUM) {
                                    num_found = true;
                                    numbers.push_back(&child_n_r);
                                    r_count += 1;
                                } else {
                                    break;
                                }
                            }
                            if (num_found) {
                                R += r_count;
                            }
                        }

                        if (numbers.size() == 0) {
                            numbers.push_back(&child);
                        }
                        
//                        for (auto& val : numbers) {
//                            Token_print(parse, val);
//                        }
                        
                        if (numbers.size() == 1) {
                        
                            Speech_Property* count_prop = nullptr;
                            if (!prp.try_get_only_prop("COUNT", &count_prop)) {
                                count_prop = prp.push_prop("COUNT");
                            }
                            
                            count_prop->type_str = "VALUE";
                            count_prop->value.kind_string = "NUMERIC";
                            prp.ignore_selections = true;
                            
                            count_prop->token = &child;
                            child.prop_ref = count_prop;
                            
                            Speech_Property* spec = nullptr;
                            if (!prp.try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec)) {
                                spec = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                                spec->value.kind_string = "FLAG";
                                spec->value.flag = false;
                            } else {
                                spec->value.kind_string = "FLAG";
                                spec->value.flag = true;
                            }
                            
                            
                            mtt::String text = child.lemma;
                            replace_patterns_in_place(text, string_replacements);
                            
                            cstring num_str = text.c_str();
                            char* end;
                            auto result = strtod(num_str, &end);
                            usize len = strlen(num_str);
                            if ((end != num_str + len) || result == 0 ||
                                result == HUGE_VAL || result == -HUGE_VAL ||
                                result == HUGE_VALF || result == -HUGE_VALF ||
                                result == HUGE_VALL || result == -HUGE_VALL) {
                                
                                count_prop->value.numeric = 1.0;
                                
                                {
                                    dt::Dynamic_Array<mtt::String> words;
                                    words.push_back(child.lemma);
                                    if (!child.left_children.empty()) {
                                        Speech_Token* next_child = parse->token_list[child.left_children.back()];
                                        do {
                                            if (next_child->dep == nlp::DEP_compound && next_child->pos == nlp::POS_NUM) {
                                                words.push_back(next_child->lemma);
                                                if (next_child->left_children.empty()) {
                                                    break;
                                                }
                                                next_child = parse->token_list[next_child->left_children.back()];
                                            } else {
                                                break;
                                            }
                                        } while (1);
                                    }
                                    
                                    for (usize i = 0; i < words.size(); i += 1) {
                                        replace_patterns_in_place(words[i], string_replacements);
                                    }
                                    std::reverse(words.begin(), words.end());
                                    
                                    isize value = 1;
                                    if (!dt::text2num(words, &value)) {
                                        MTT_error("%s", "Number invalid\n");
                                    }
                                    
                                    count_prop->value.numeric = value;
                                }
                            } else {
                                count_prop->value.numeric = result;
                            }
                            
                            if (!child.right_children.empty()) {
                                Speech_Token& r_child = *parse->token_list[child.right_children[0]];
                                if (r_child.dep == spacy_nlp::DEP_quantmod && r_child.text == "times") {
                                    count_prop->kind_str = "multiplier";
                                }
                            }
                        } else {
                            Speech_Property* count_prop = nullptr;
                            if (!prp.try_get_only_prop("COUNT", &count_prop)) {
                                count_prop = prp.push_prop("COUNT");
                            }
                            
                            count_prop->type_str = "VALUE";
                            count_prop->value.kind_string = "VECTOR";
                            count_prop->value.vector = vec4(0.0f);
                            //prp.kind_str = "VECTOR";
                            //prp.annotation = "DO_NOT_REQUIRE_SELECTION";
                            prp.ignore_selections = true;
                            count_prop->token = &child;
                            child.prop_ref = count_prop;
                            
                            Speech_Property* spec = nullptr;
                            if (!prp.try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec)) {
                                spec = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                                spec->value.kind_string = "FLAG";
                                spec->value.flag = false;
                            } else {
                                spec->value.kind_string = "FLAG";
                                spec->value.flag = true;
                            }
                            
                            
                            for (usize v_idx = 0; v_idx < numbers.size() && v_idx < 4; v_idx += 1) {
                                Speech_Token* num_tok = numbers[v_idx];
                                mtt::String text = num_tok->text;
                                if (text.empty()) {
                                    continue;
                                } else if (text.back() == ',') {
                                    text.pop_back();
                                }
                                
                                replace_patterns_in_place(text, string_replacements);
                                
                                cstring num_str = text.c_str();
                                char* end;
                                auto result = strtod(num_str, &end);
                                usize len = strlen(num_str);

                                if ((end != num_str + len) || result == 0 ||
                                    result == HUGE_VAL || result == -HUGE_VAL ||
                                    result == HUGE_VALF || result == -HUGE_VALF ||
                                    result == HUGE_VALL || result == -HUGE_VALL) {
                                    
                                    isize out = 0.0f;
                                    if (dt::text2num({text}, &out)) {
                                        count_prop->value.vector[(int)v_idx] = (float32)(out);
                                    } else {
                                        count_prop->value.vector[(int)v_idx] = 0.0;
                                    }
                                } else {
                                    count_prop->value.vector[(int)v_idx] = result;
                                }
                                
                                
                            }
                            
                            
//                            if (!child.right_children.empty()) {
//                                Speech_Token& r_child = *parse->token_list[child.right_children[0]];
//                                if (r_child.dep == spacy_nlp::DEP_quantmod && r_child.text == "times") {
//                                    count_prop->kind_str = "multiplier";
//                                }
//                            }

                        }
                        
                    }

                    break;
                }
            }
            }
            
        }
    }
    
    // add the original label at the end in case there were compound nouns added
    prp.label += last_lemma;
    if (prp.kind_str == "THING_TYPE") {
        prp.value.text = prp.label;
        

    }
    if (info.should_likely_represent) {
        sys_ctx->noun_list.emplace_back(&token.info);
    }
    

    
    // since if this flag does not exist, we're likely referring to a type of thing, not an instance of a thing
    DT_print("checking specific or unspecific\n");
    std::vector<Speech_Property*>* spec_or_unspec = nullptr;
    if (!prp.try_get_prop("SPECIFIC_OR_UNSPECIFIC", &spec_or_unspec)) {
        DT_print("not specific or unspecific\n");
        
        if (is_possessed) {
            
        } else if (info.is_time_expression) {
        } else if (token.pos == nlp::POS_PROPN) {
            prp.kind_str = "THING_INSTANCE";
        } else {
            if (prp.kind_str != "THING_TYPE") {
                // so we overwrite
                prp.kind_str = "THING_TYPE";
            }
            

            
            
        }
    } else {
        DT_print("is specific or unspecific\n");
        // is specific
        {
            DT_print("is specific\n");
            // TODO: make sure this doesn't break anything
            Speech_Property::Prop_List* plural_prop = nullptr;
            if (prp.try_get_prop("PLURAL", &plural_prop) /* && !force_to_be_a_type*/) {
                // is it plural?
                if (plural_prop->front()->value.flag == true) {
                    prp.value.kind_string = "LIST";
                    prp.kind_str = "THING_INSTANCE";
                    if (auto* c_p = prp.try_get_only_prop("COUNT"); c_p != nullptr && force_to_be_a_type) {
                        c_p->value.numeric = 1;
                    } else {
                        auto& count_prop = *prp.push_prop("COUNT");
                        count_prop.type_str = "VALUE";
                        auto& val = count_prop.value;
                        val.kind_string = "NUMERIC";
                        val.numeric = 1;
                    }
                    if (auto* s_p = prp.try_get_only_prop("SPECIFIC_OR_UNSPECIFIC"); s_p != nullptr && force_to_be_a_type) {
                        s_p->value.flag = true;
                    } else {
                        auto* spec_p = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
                        spec_p->value.kind_string = "FLAG";
                        spec_p->value.flag = true;
                    }
                    //std::vector<Speech_Property*>* count_prop = nullptr;
                } else {
                    prp.value.kind_string = "THING_INSTANCE";
                    prp.kind_str = "THING_INSTANCE";
                }
            }
            // is unspecific
        }
        /*
         ins.value.thing = token.timing.selection.ID;
         ins.value.list.push_back(token.timing.selection.ID);
         ins.value.list.push_back((Speech_Instruction::Value){.kind_str = "THING_INSTANCE", .thing = token.timing.selection.ID});
         */
        //= ins.prop
        
    }

    //prp.print();
    
    //debug_break();
    
    if (prp.label == "") {
        prp.label = token.lemma;
    }
    
    if (token.force_not_a_type) {
        prp.kind_str = "THING_INSTANCE";
        Speech_Property* spec = nullptr;
        if (!prp.try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec)) {
            spec = prp.push_prop("SPECIFIC_OR_UNSPECIFIC");
            spec->value.kind_string = "FLAG";
            spec->value.flag = true;
        } else {
            spec->value.kind_string = "FLAG";
            spec->value.flag = true;
        }
    }
    if (spec_or_unspec == nullptr && prp.kind_str == "THING_TYPE") {
        prp.tag_str = "ACTION_AS_NOUN";
    }
    DT_scope_close();
    return {Interp_Status{.type = INTERP_STATUS_OK}, &token.info};
}

Interp_Result handle_det(dt::Lang_Eval_Args* args, dt::Speech_Token& token)
{
    args->curr_dep_token = &token;
    DT_print("%s, lemma=[%s] pos=[%s] dep=[%s]\n", token.text.c_str(), token.lemma.c_str(), dt::spacy_nlp::pos_str[token.pos].c_str(), dt::spacy_nlp::dep_str[token.dep].c_str());
    namespace nlp = dt::spacy_nlp;
    
    auto* dt = args->dt;
    DT_scope_open();
    
    
    
    Word_Info& info = token.info;
    dt::Speech_Property* prp = dt::Speech_Property::make();
    prp->token = info.token;
    info.prop_list.push_back(prp);
    Determiner_Info& det_info = info.determiner_info;
    Speech_Token_set_prop(&token, prp);

    
    token.info.type = WORD_INFO_TYPE_DETERMINER;
    token.info.token = &token;
    info.determiner_info.token = &token;
    
    
    prp->type_flag = (uint64)SI::TYPE_FLAG::ATTRIBUTE;
    prp->flags = (uint64)SI::ATTRIBUTE_FLAG::SINGULAR_OR_PLURAL;
    //ins->label = SI::ATTRIBUTE_FLAG_TO_STRING[ins->flags];
    //    mtt::String key;      // lookup
    //    mtt::String tag_str;  // system tagging e.g. noun
    //    mtt::String type_str; // type of node
    //
    //    mtt::String kind_str; // modifier based on the type
    //    mtt::String label;    // textual label
    
    
    mtt::String* number_val;
    bool has_number_val = false;
    if (mtt::map_try_get(&token.morph, "Number", &number_val)) {
        has_number_val = true;
        DT_print("%s\n", number_val->c_str());
    }
    mtt::String* definite_val;
    bool has_definite_val = false;
    if (mtt::map_try_get(&token.morph, "Definite", &definite_val)) {
        has_definite_val = true;
        DT_print("%s\n",definite_val->c_str());
    }
    mtt::String* pronoun_type_val;
    bool has_pronoun_type_val = false;
    if (mtt::map_try_get(&token.morph, "PronType", &pronoun_type_val)) {
        has_pronoun_type_val = true;
        DT_print("%s\n", pronoun_type_val->c_str());
    }
    //    FOR_ITER(map_it, token.morph, ++map_it) {
    //        std::cout << map_it->second << std::endl;
    //    }
    
    
    info.determiner_info.is_guaranteed_plural = false;
    info.determiner_info.is_guaranteed_singular = false;
    info.determiner_info.is_definite = false;
    
    if (number_val != nullptr) {
        if (*number_val == "Plur") {
            info.determiner_info.is_guaranteed_plural = true;
            info.determiner_info.is_guaranteed_singular = false;
        }
        else {
            info.determiner_info.is_guaranteed_plural   = false;
            info.determiner_info.is_guaranteed_singular = true;
        }
    } else if (token.lemma == "a" || token.lemma == "an") {
        info.determiner_info.is_guaranteed_plural   = false;
        info.determiner_info.is_guaranteed_singular = true;
    }
    
    if (definite_val != nullptr) {
        if (*definite_val == "Def") {
            info.determiner_info.is_definite = true;
        }
    } else if (pronoun_type_val != nullptr) {
        if (*pronoun_type_val == "Dem") {
            info.determiner_info.is_definite = true;
        }
    }
    
    prp->label    = info.token->lemma;
    prp->tag_str  = "DET";
    prp->key      = "";
    prp->type_str = "VALUE";
    prp->kind_str = det_info.is_definite ? "SPECIFIC" : "UNSPECIFIC";
    prp->value.kind_string = "FLAG";
    prp->value.flag = det_info.is_definite;
    
    //    ins->value.kind_string = "TEXT";
    //    if (det_info.is_guaranteed_plural) {
    //        ins->value.text = "PLURAL";
    //    } else if (det_info.is_guaranteed_singular) {
    //        ins->value.text  = "SINGULAR";
    //    } else {
    //        ins->value.text = "UNKNOWN";
    //    }
    //    ins->label    = info.token->lemma;
    //    ins->print();
    
    
    
    DT_scope_close();
    
    return {Interp_Status{.type = INTERP_STATUS_OK}, &info};
}

// MARK: temp .. should use the tree to look-up

inline static mtt::Set<mtt::String> stateful_verbs = {
    "collide",
    "intersect",
    "overlap",
    "touch",
    "exist",
    "know",
    "live",
    "die",
};
bool is_stateful_verb(const mtt::String& text)
{
    auto* result = verb_lookup(text);
    if (result != nullptr) {
        return result->verb_class == VERB_CLASS_STATE_CHANGE;
    }
    
    if (stateful_verbs.find(text) != stateful_verbs.end()) {
        return true;
    }
    
    return false;
}

void verb_handle_noun_on_left(Parse* parse, Speech_Token& token,  Speech_Token& child, Interp_Result& result, Speech_Property* prp, Verbal_Info& info);
void verb_handle_noun_on_left(Parse* parse, Speech_Token& token,  Speech_Token& child, Interp_Result& result, Speech_Property* prp, Verbal_Info& info) {
    if (child.dep == spacy_nlp::DEP_npadvmod) {
       if (result.value->nominal.is_time_expression) {
           for (auto it_sub = child.info.prop_list.begin(); it_sub != child.info.prop_list.end(); ++it_sub) {
               prp->push_prop("TIME", *it_sub);
           }
           return;
       }
   }
    if (prp->type_str == "RESPONSE") {
        for (auto c_it = child.info.prop_list.begin(); c_it != child.info.prop_list.end(); ++c_it) {
            Speech_Property* child_prop = *c_it;
            Speech_Property* spec_prop = nullptr;
            if (child_prop->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop) && spec_prop->label != "the") {
                child_prop->kind_str = "THING_TYPE";
                child_prop->ignore_selections = true;
            }
        }
    }
    
    bool treat_as_agent                 = false;
    bool treat_as_patient               = false;
    bool treat_as_direct_object         = false;
    bool treat_as_indirect_object       = false;
    bool treat_as_object_of_preposition = false;
    mtt::String key;
    
    auto& ch_props = result.value->prop_list;
    
    switch (child.dep) {
    case spacy_nlp::DEP_dobj:
        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
        treat_as_patient = true;
        treat_as_direct_object = true;
        key = "DIRECT_OBJECT";
        break;
    case spacy_nlp::DEP_nsubj:
        treat_as_agent = true;
        key = "AGENT";
        break;
    case spacy_nlp::DEP_nsubjpass:
        treat_as_direct_object = true;
        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
        key = "DIRECT_OBJECT";
        break;
    case spacy_nlp::DEP_pobj:
        treat_as_object_of_preposition = true;
        key = "OBJECT_OF_PREPOSITION";
        break;
    case spacy_nlp::DEP_npadvmod:
        
        bool has_intj = false;
        if (child.i > 0) {
            if (parse->token_list[child.i - 1]->pos == spacy_nlp::POS_INTJ) {
                has_intj = true;
            }
        }
        if (child.i == 0 || has_intj) {
            key = "AGENT";
            Speech_Property* spec = nullptr;
            if (!child.prop_ref->try_get_only_prop("SPECIFIC_OR_UNSPECIFIC", &spec)) {
                spec = child.prop_ref->push_prop("SPECIFIC_OR_UNSPECIFIC");
                spec->value.kind_string = "FLAG";
                spec->value.flag = true;
            } else {
                spec->value.kind_string = "FLAG";
                spec->value.flag = true;
            }
        } else {
            key = "PROPERTY";
        }
        break;
    }
    
    DT_print("Verb treat noun %s as [agent : %s, patient : %s, direct_object : %s, indirect_object : %s, object_of_preposition : %s\n",
             child.info.prop_list.front()->label.c_str(),
             bool_str(treat_as_agent),
             bool_str(treat_as_patient),
             bool_str(treat_as_direct_object),
             bool_str(treat_as_indirect_object),
             bool_str(treat_as_object_of_preposition));
    
    if constexpr ((DT_OLD_NESTING_WITH_SELECT))
    {
        auto& semantic_prop = *prp->push_prop(key);
        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
            semantic_prop.push_prop("SELECT", *it);
        }
    } else {
        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
            if (token.lemma == "exist") {
                (*it)->kind_str = "THING_TYPE";
            }
            prp->push_prop(key, *it);
        }
    }
    
    // TODO: //
    auto& nom = result.value->nominal;
    if (nom.is_main_actor_or_agent) {
        nom.action_ref = &token.info;
        info.agents.push_back(&nom);
        // TODO: Do I need this sort of "associated" array? Probably for merging
        for (usize nidx = 0; nidx < nom.associated.size(); nidx += 1) {
            info.agents.push_back(nom.associated[nidx]);
        }
    } else {
        UNHANDLED();
        return;
    }
}

Interp_Result handle_aux(dt::Lang_Eval_Args* args, dt::Speech_Token& token)
{
    args->curr_dep_token = &token;
    DT_print("%s, lemma=[%s] pos=[%s] dep=[%s]\n", token.text.c_str(), token.lemma.c_str(), dt::spacy_nlp::pos_str[token.pos].c_str(), dt::spacy_nlp::dep_str[token.dep].c_str());
    namespace nlp = dt::spacy_nlp;
    
    dt::System_Evaluation_Context& ctx = args->dt->sys_eval_ctx;
    auto& token_list = args->parse->token_list;
    auto* parse = args->parse;
    
    auto* dt    = args->dt;
    
    
    auto& left_children  = token.left_children;
    auto& right_children = token.right_children;
    
    DT_scope_open();
    
    token.info.type = WORD_INFO_TYPE_VERBAL;
    token.info.token = &token;
    
    Verbal_Info& info = token.info.action;
    info.keys.push_back(token.lemma);
    info.token = &token;
    
    dt::Speech_Property* prp = dt::Speech_Property::make();
    prp->token = &token;
    token.info.prop_list.push_back(prp);
    Speech_Token_set_prop(&token, prp);

    prp->tag_str = "be";
    prp->label = token.lemma;
    
    
    prp->label = "be";
    prp->type_str = "ACTION";
    prp->kind_str = "EXISTENTIAL";
    prp->tag_str = "be";
    prp->value.kind_string = "TEXT";
    prp->value.text = token.lemma;
    bool has_then  = false;
    bool has_after = false;
    
    bool is_negated = false;
    
    
    Speech_Token* nsubj = nullptr;
    Speech_Token* nsubjpass = nullptr;
    Speech_Token* csubj = nullptr;
    Speech_Token* csubjpass = nullptr;
    
    std::vector<dt::Speech_Property*> prop_swap;

    
    {
        
        usize L = 0;
        for (;
             L < left_children.size();
             L += 1) {
            
            Speech_Token& child = *parse->token_list[left_children[L]];
            if (child.is_blocked) {
                continue;
            }
            
            if (child.dep == nlp::DEP_nsubj) {
                nsubj = &child;
            } else if ( child.dep == nlp::DEP_csubj) {
                csubj = &child;
            } else if ( child.dep == nlp::DEP_nsubjpass) {
                nsubjpass = &child;
            } else if ( child.dep == nlp::DEP_csubjpass) {
                csubjpass = &child;
            }
            
            switch (child.pos) {
            default: {
                UNHANDLED();
                break;
            }
            case nlp::POS_DET: {
                auto result = handle_det(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK:
                    // TODO: find most recent selections
                    {
                        if (!result.value->prop_list.empty()) {
                            auto* d_prop = *result.value->prop_list.begin();
                            
                        }
                    }
                    break;
                }
                break;
            }
            case nlp::POS_PROPN: {
                // fallthrough
            }
            case nlp::POS_NOUN: {
                auto result = handle_noun(args, child);
                
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                    if (result.value->nominal.is_time_expression) {
                        for (auto it_sub = child.info.prop_list.begin(); it_sub != child.info.prop_list.end(); ++it_sub) {
                            prp->push_prop("TIME", *it_sub);
                        }
                        break;
                    }
                    
                    bool treat_as_agent                 = false;
                    bool treat_as_patient               = false;
                    bool treat_as_direct_object         = false;
                    bool treat_as_indirect_object       = false;
                    bool treat_as_object_of_preposition = false;
                    mtt::String key;
                    
                    auto& ch_props = result.value->prop_list;
                    
                    switch (child.dep) {
                    case spacy_nlp::DEP_dobj:
                        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
                        treat_as_patient = true;
                        treat_as_direct_object = true;
                        key = "DIRECT_OBJECT";
                        break;
                    case spacy_nlp::DEP_nsubj:
                        treat_as_agent = true;
                        key = "AGENT";
                        break;
                    case spacy_nlp::DEP_nsubjpass:
                        treat_as_direct_object = true;
                        DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
                        key = "DIRECT_OBJECT";
                        break;
                    case spacy_nlp::DEP_pobj:
                        treat_as_object_of_preposition = true;
                        key = "OBJECT_OF_PREPOSITION";
                        break;
                    }
                    DT_print("Verb treat noun %s as [agent : %s, patient : %s, direct_object : %s, indirect_object : %s, object_of_preposition : %s\n",
                             child.info.prop_list.front()->label.c_str(),
                             bool_str(treat_as_agent),
                             bool_str(treat_as_patient),
                             bool_str(treat_as_direct_object),
                             bool_str(treat_as_indirect_object),
                             bool_str(treat_as_object_of_preposition));
                    
                    if constexpr ((DT_OLD_NESTING_WITH_SELECT))
                    {
                        auto& semantic_prop = *prp->push_prop(key);
                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                            semantic_prop.push_prop("SELECT", *it);
                        }
                    } else {
                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                            prp->push_prop(key, *it);
                        }
                    }
                    
                    // TODO: //
                    auto& nom = result.value->nominal;
                    if (nom.is_main_actor_or_agent) {
                        nom.action_ref = &token.info;
                        info.agents.push_back(&nom);
                        // TODO: Do I need this sort of "associated" array? Probably for merging
                        for (usize nidx = 0; nidx < nom.associated.size(); nidx += 1) {
                            info.agents.push_back(nom.associated[nidx]);
                        }
                    } else {
                        UNHANDLED();
                        break;
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_ADP: {
                if (child.lemma == "until") {
                    prp->type_str = "END";
                    prp->annotation = "END_CONDITION";
                }
                break;
            }
            case nlp::POS_ADV: {
                switch (child.tag) {
                default: {
                    UNHANDLED();
                    break;
                }
                case nlp::TAG::RB: {
                    auto result = handle_RB(args, child);
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        if (child.lemma == "then") {
                            has_then = true;
                            break;
                        }
//                        else if (child.lemma == "after") {
//                            has_after = true;
//                            break;
//                        }
                        
                        
                        auto& props = result.value->prop_list;
                        for (auto it = props.begin(); it != props.end(); ++it) {
                            prp->push_prop("PROPERTY", *it);
                        }
                        
                        
                        break;
                    }
                    }
                    
                    break;
                }
                case nlp::TAG_WRB: {
                    auto result = handle_WHRB(args, child);
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        
                        
                        auto& wrb_info = result.value->whadverb_info;
                        if (wrb_info.is_question) {
                            UNHANDLED();
                            break;
                        } else if (wrb_info.is_time_expression) {
                            UNHANDLED();
                            break;
                        } else {
                            info.should_treat_as_trigger = true;
                        }
                        
                        switch (child.dep) {
                        default: {
                            UNHANDLED();
                            break;
                        }
                        case nlp::DEP_advmod: {
                            // TODO: other cases?
                            break;
                        }
                        }
                        
                        break;
                    }
                    }
                    
                    
                    
                    break;
                }
                }
                break;
            }
            case nlp::POS_PRON: {
                auto result = handle_pronoun(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    break;
                }
                }
                
                switch (child.tag) {
                default: {
                    UNHANDLED();
                    break;
                }
                case nlp::TAG_PRP: {
                    // unresolved nouns
                    
                    auto& pron_info = result.value->pronoun_info;
                    pron_info.corresponding_nom_info.action_ref = &token.info;
                    if (pron_info.is_plural) {
                        //info.
                    } else {
                        
                    }
                    info.agents.push_back(&pron_info.corresponding_nom_info);
                    
                    
                    
                    {
                        bool treat_as_agent                 = false;
                        bool treat_as_patient               = false;
                        bool treat_as_direct_object         = false;
                        bool treat_as_indirect_object       = false;
                        bool treat_as_object_of_preposition = false;
                        mtt::String key;
                        
                        auto& ch_props = result.value->prop_list;
                        
                        switch (child.dep) {
                        case spacy_nlp::DEP_dobj:
                            DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
                            treat_as_patient = true;
                            treat_as_direct_object = true;
                            key = "DIRECT_OBJECT";
                            break;
                        case spacy_nlp::DEP_nsubj:
                            treat_as_agent = true;
                            key = "AGENT";
                            break;
                        case spacy_nlp::DEP_dative:
                            // fallthrough
                        case spacy_nlp::DEP_nsubjpass:
                            treat_as_indirect_object = true;
                            DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
                            key = "INDIRECT_OBJECT";
                            break;
                        case spacy_nlp::DEP_pobj:
                            treat_as_object_of_preposition = true;
                            key = "OBJECT_OF_PREPOSITION";
                            break;
                        }
                        DT_print("Verb treat noun %s as [agent : %s, patient : %s, direct_object : %s, indirect_object : %s, object_of_preposition : %s\n",
                                 child.info.prop_list.front()->label.c_str(),
                                 bool_str(treat_as_agent),
                                 bool_str(treat_as_patient),
                                 bool_str(treat_as_direct_object),
                                 bool_str(treat_as_indirect_object),
                                 bool_str(treat_as_object_of_preposition));
                        
                        if constexpr ((DT_OLD_NESTING_WITH_SELECT))
                        {
                            auto& semantic_prop = *prp->push_prop(key);
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                semantic_prop.push_prop("SELECT", *it);
                            }
                        } else {
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                prp->push_prop(key, *it);
                            }
                        }
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_VERB: {
                auto result = handle_verb(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                
                    if (has_then || (!(child.right_children.empty()) && token_list[child.right_children[0]]->lemma == "then")) {
                        has_then = true;
                        if (!result.value->prop_list.empty()) {
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                dt::Speech_Property* child = *it_ch;
                                child->push_prop("SEQUENCE_THEN", prp)->sequence_type_str = "SEQUENCE_THEN";
                                prop_swap.push_back(child);
                                
                            }
                        }
                        break;
                        // current action happens after another
                    } else if (has_after) {
                        ASSERT_MSG(false, "Unhandled\n");
                        break;
                    }
                
                    switch (child.dep) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                        // probably a condition
                    case nlp::DEP_advcl: {
                        for (auto ch_it = child.info.prop_list.begin();
                             ch_it != child.info.prop_list.end(); ++ch_it) {
                            if ((*ch_it)->type_str == "TRIGGER") {
                                prp->type_str = "RESPONSE";
                                //prop_swap.push_back(*ch_it);
                                prp->push_prop("TRIGGER", *ch_it);
                                (*ch_it)->type_str = "TRIGGER";
                            
                                
                            }
                        }
    //                    if (child->type_str == "if") {
    //                        ins->type_str = "TRIGGER";
    //                    }
                        auto& v_info = result.value->action;
                        if (v_info.should_treat_as_trigger) {
                            info.should_treat_as_trigger_response = true;
                            info.associated.emplace_back(&v_info);
                            
                            token.info.type = WORD_INFO_TYPE_RULE;
                            token.info.rule.token = &token;
                            
                            token.info.rule.trigger_info = result.value;
                            token.info.rule.response_info = &token.info;
                            
                            token.info.action.rule_ref = &token.info;
                            child.info.action.rule_ref = &token.info;
                            
                        } else {
                            UNHANDLED();
                            break;
                        }
                        break;
                    }
                    case nlp::DEP_ccomp: {
                        FOR_ITER(it, result.value->prop_list, ++it) {
                            Speech_Property* f = *it;
                            //prp->push_prop("PREV_SENTENCE", (*it));
                            ctx.fragments.push_back(f);
                        }
                        break;
                    }
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_AUX: {
                auto result = handle_aux(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                    switch (child.dep) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case nlp::DEP_ccomp: {
                        FOR_ITER(it, result.value->prop_list, ++it) {
                            Speech_Property* f = *it;
                            //prp->push_prop("PREV_SENTENCE", (*it));
                            ctx.fragments.push_back(f);
                            DT_print("%s\n", f->label.c_str());
                        }
                        break;
                    }
                    }
                    break;
                }
                }
                break;
            }
                
                
            }
            
            
        }
        
        usize R = 0;
        for (;
             R < right_children.size();
             R += 1) {
            Speech_Token& child = *parse->token_list[right_children[R]];
            if (child.is_blocked) {
                continue;
            }
            
            if (child.dep == nlp::DEP_nsubj) {
                nsubj = &child;
            } else if ( child.dep == nlp::DEP_csubj) {
                csubj = &child;
            } else if ( child.dep == nlp::DEP_nsubjpass) {
                nsubjpass = &child;
            } else if ( child.dep == nlp::DEP_csubjpass) {
                csubjpass = &child;
            }
            
            switch (child.pos) {
            default: {
                UNHANDLED();
                break;
            }
            case nlp::POS_PUNCT: {
                //DT_print("WARNING: am not yet handling end of sentence\n");
                break;
            }
            case nlp::POS_PART: {
                if (child.dep == nlp::DEP_neg) {
                    std::vector<Speech_Property*>* props = nullptr;
                    if (prp->try_get_prop("NEGATED", &props)) {
                        is_negated = !is_negated;
                        props->front()->value.flag = is_negated;
                        if (nsubj != nullptr) {
                            if (nsubj->pos == nlp::POS_DET) {
                                props->front()->kind_str = "NEGATED_RIGHT";
                            } else {
                                props->front()->kind_str = "NEGATED_LEFT";
                            }
                        }
                        
                    } else {
                        is_negated = true;
                        auto& negated_prop = *prp->push_prop("NEGATED");
                        negated_prop.value.kind_string = "FLAG";
                        negated_prop.value.flag = true;
                        if (nsubj != nullptr) {
                            if (nsubj->pos == nlp::POS_DET) {
                                negated_prop.kind_str = "NEGATED_RIGHT";
                            } else {
                                negated_prop.kind_str = "NEGATED_LEFT";
                            }
                        }
                    }
                    
//                    Speech_Property* p_n = nullptr;
//                    if (prp->try_get_only_prop("PROPERTY_NEGATION", &p_n)) {
//                        auto* p_n = prp->push_prop("PROPERTY_NEGATION");
//                        p_n->value.kind_string = "FLAG";
//                        p_n->value.flag = true;
//                    }
                }
                break;
            }
            case spacy_nlp::POS_ADP: {
                switch (child.tag) {
                default: {
                    UNHANDLED();
                    break;
                }
                case spacy_nlp::TAG_IN: {
                    auto result = handle_IN(args, child);
                    
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        
                        Preposition_Info& prep_info = result.value->preposition_info;
                        if (child.dep == nlp::DEP_prep || child.dep == nlp::DEP_dative) {
                            
                            // Merge
                            info.has_preposition = true;
                            info.preposition = &child;
                            info.keys.push_back(child.lemma);
                            for (usize pidx = 0; pidx < prep_info.objects_of_preposition.size(); pidx += 1) {
                                info.objects.push_back(prep_info.objects_of_preposition[pidx]);
                                prep_info.objects_of_preposition[pidx]->acted_on_by_ref = &token.info;
                            }
                            
                            
                            Speech_Property* local_root = nullptr;
                            if (has_then) {
                                local_root = prp->push_prop("SEQUENCE_THEN");
                                local_root->sequence_type_str = "SEQUENCE_THEN";
                            } else {
                                local_root = prp;
                            }
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                local_root->push_prop("PREPOSITION", *it);
                            }
                            
                        } else if (child.dep == spacy_nlp::DEP_agent) {
                            for (usize pidx = 0; pidx < prep_info.objects_of_preposition.size(); pidx += 1) {
                                info.objects.push_back(prep_info.objects_of_preposition[pidx]);
                                prep_info.objects_of_preposition[pidx]->is_main_actor_or_agent = true;
                                prep_info.objects_of_preposition[pidx]->is_acted_upon_or_patient = false;
                                prep_info.objects_of_preposition[pidx]->should_likely_represent = true;
                                prep_info.objects_of_preposition[pidx]->action_ref = &token.info;
                            }
                            

                            auto& semantic_prop = *prp;//->push_prop("AGENT");
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                semantic_prop.push_prop("AGENT", *it);
                            }
                        }
                        break;
                    }
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_PROPN: {
                // fallthrough
            }
            case nlp::POS_NOUN: {
                auto result = handle_noun(args, child);
                
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {

                    if (result.value->nominal.is_time_expression) {
                        for (auto it_sub = child.info.prop_list.begin(); it_sub != child.info.prop_list.end(); ++it_sub) {
                            prp->push_prop("TIME", *it_sub);
                        }
                        break;
                    }
                    if (token.dep == spacy_nlp::DEP_acl) {
                        switch (child.dep) {
                        case spacy_nlp::DEP_oprd:
                            // so the verb is really a property like "was named X"
                            prp->type_str = "PROPERTY";
                            prp->tag_str = "";
                            prp->kind_str = token.lemma;
                            prp->value.kind_string = "TEXT";
                            prp->value.text = child.lemma;
                            break;
                        }
                        break;
                    } else if (child.dep == spacy_nlp::DEP_npadvmod) {
                        if (result.value->nominal.is_time_expression) {
                            for (auto it_sub = child.info.prop_list.begin(); it_sub != child.info.prop_list.end(); ++it_sub) {
                                prp->push_prop("TIME", *it_sub);
                            }
                            break;
                        }
                    } else if (child.dep == spacy_nlp::DEP_attr) {
                        if (nsubj != nullptr) {
                            Speech_Token* nsubj_token = nsubj;
                            

                            
                            {
                                auto dx_it = dt::deictic_words.find(nsubj_token->text);
                                if (dx_it != dt::deictic_words.end()) {

                                    //                            if (!info.token->timing.was_selected) {
                                    //                                info.token->timing.selection           = result.value->token->timing.selection;
                                    //                                info.token->timing.utterance_timestamp = result.value->token->timing.utterance_timestamp;
                                    //                                info.token->timing_taken_from_reference = true;
                                    //                                info.specific_entity                    = true;
                                    //                            }
                                    FOR_ITER(ins_, nsubj_token->info.prop_list, ++ins_) {
                                        auto& prop = *(*ins_);
                                        
                                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                            auto* obj = prp->push_prop("OBJECT", *it);
                                            auto* deixis = obj->push_prop("DEIXIS");
                                            deixis->label = *dx_it;
                                            deixis->type_str = "DEIXIS";
                                        }
                                    }
                                } else {
                                    FOR_ITER(ins_, nsubj_token->info.prop_list, ++ins_) {
                                        auto& prop = *(*ins_);
                                        
                                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                            prp->push_prop("OBJECT", *it);
                                        }
                                    }
                                }
                            }
                            
                            break;
                        }
                        

                    }
                    
                    bool treat_as_agent                 = false;
                    bool treat_as_patient               = false;
                    bool treat_as_direct_object         = false;
                    bool treat_as_indirect_object       = false;
                    bool treat_as_object_of_preposition = false;
                    mtt::String key;
                    
                    auto& ch_props = result.value->prop_list;
                    
                    switch (child.dep) {
                    case spacy_nlp::DEP_dobj:
                        treat_as_patient = true;
                        treat_as_direct_object = true;
                        key = "DIRECT_OBJECT";
                        break;
                    case spacy_nlp::DEP_attr:
                        // fallthrough
                    case spacy_nlp::DEP_nsubj:
                        treat_as_agent = true;
                        key = "AGENT";
                        break;
                    case spacy_nlp::DEP_dative:
                        // fallthrough
                    case spacy_nlp::DEP_nsubjpass:
                        treat_as_indirect_object = true;
                        key = "INDIRECT_OBJECT";
                        break;
                    case spacy_nlp::DEP_pobj:
                        treat_as_object_of_preposition = true;
                        key = "OBJECT_OF_PREPOSITION";
                        break;
                    }
                    
                    DT_print("Verb treat noun %s as [agent : %s, patient : %s, direct_object : %s, indirect_object : %s, object_of_preposition : %s\n",
                             child.info.prop_list.front()->label.c_str(),
                             bool_str(treat_as_agent),
                             bool_str(treat_as_patient),
                             bool_str(treat_as_direct_object),
                             bool_str(treat_as_indirect_object),
                             bool_str(treat_as_object_of_preposition));
                    
                    if constexpr ((DT_OLD_NESTING_WITH_SELECT)) {
                        auto& semantic_prop = *prp->push_prop(key);
                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                            semantic_prop.push_prop("SELECT", *it);
                        }
                    } else {
                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                            auto* parent = (*it)->parent;
                            
                            if (parent != nullptr &&
                                parent->token != nullptr && parent->token->dep == nlp::DEP_relcl &&
                                parent->token->pos == nlp::POS_VERB) {
                                *prp = *parent;
                                Speech_Property::destroy(parent);
                                
                            } else {
                                prp->push_prop(key, *it);
                            }
                        }
                    }
                    
                    // TODO: //
                    auto& nom = result.value->nominal;
                    if (nom.is_main_actor_or_agent) {
                        nom.action_ref = &token.info;
                        info.agents.push_back(&nom);
                        // TODO: Do I need this sort of "associated" array? Probably for merging
                        for (usize nidx = 0; nidx < nom.associated.size(); nidx += 1) {
                            info.agents.push_back(nom.associated[nidx]);
                        }
                    } else {
                        UNHANDLED();
                        break;
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_ADV: {
                switch (child.tag) {
                default: {
                    UNHANDLED();
                    break;
                }
                case nlp::TAG::RB: {
                    auto result = handle_RB(args, child);
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        
                        if (child.lemma == "then") {
                            has_then = true;
                            break;
                        }
//                        else if (child.lemma == "after") {
//                            has_after = true;
//                            break;
//                        }
                        
                        auto& props = result.value->prop_list;
                        for (auto it = props.begin(); it != props.end(); ++it) {
                            prp->push_prop("PROPERTY", *it);
                        }
                        
                        break;
                    }
                    }
                    
                    break;
                }
                }
                break;
            }
            case nlp::POS_VERB: {
                
                auto result = handle_verb(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    if (child.dep == nlp::DEP_conj) {
                        if (prp->type_str == "TRIGGER") {
                            prp->type_str = "TRIGGER";
                            bool found_trigger = false;
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                token.info.prop_list.push_back(*it_ch);
                                (*it_ch)->type_str = "TRIGGER";
                                found_trigger = true;
                            }
                            
                            if (found_trigger) {
                                break;
                            }
                            
                        } else if (prp->type_str == "RESPONSE") {
                            bool has_then_local = false;
                            if (!has_then) {
                                for (auto lc = child.left_children.begin(); lc != child.left_children.end(); ++lc) {
                                    auto* tok = token_list[*lc];
                                    if (tok->lemma == "then") {
                                        has_then_local = true;
                                        break;
                                    }
                                }
                            }
                            bool found_response = false;
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                if ((*it_ch)->has_prop("TRIGGER")) {
                                    //prp->push_prop("TRIGGER_RESPONSE", *it_ch);
                                    token.info.prop_list.push_back(*it_ch);
                                } else {
                                    if (has_then || has_then_local) {
                                        prp->push_prop("ANOTHER_RESPONSE_CLAUSE_THEN", *it_ch);
                                    } else {
                                        prp->push_prop("ANOTHER_RESPONSE_CLAUSE_AND", *it_ch);
                                    }
                                }
                                //token.info.prop_list.push_back(*it_ch);
                                (*it_ch)->type_str = "RESPONSE";
                                found_response = true;
                            }
                            
                            if (found_response) {
                                break;
                            }
                            
                        } else {
#define DT_CHAINED_AND_THEN_FIX (1)
                            bool has_then_local = false;
#if !DT_CHAINED_AND_THEN_FIX
                            if (!has_then)
#endif
                            {
                                for (auto lc = child.left_children.begin(); lc != child.left_children.end(); ++lc) {
                                    auto* tok = token_list[*lc];
                                    if (tok->lemma == "then") {
                                        has_then_local = true;
                                        break;
                                    }
                                }
                            }
                            
                            bool found = false;
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                if ((*it_ch)->type_str == "RESPONSE") {
                                    //token.info.prop_list.push_back(*it_ch);
                                    if ((*it_ch)->has_prop("TRIGGER")) {
                                        //prp->push_prop("TRIGGER_RESPONSE", *it_ch);
                                        token.info.prop_list.push_back(*it_ch);
                                    } else {
                                        if (has_then || has_then_local) {
                                            prp->push_prop("ANOTHER_RESPONSE_CLAUSE_THEN", *it_ch);
                                        } else {
                                            prp->push_prop("ANOTHER_RESPONSE_CLAUSE_AND", *it_ch);
                                        }
                                    }

                                    prp->type_str = "RESPONSE";
                                    found = true;
                                    
                                }
                            }
                            if (found) {
                                break;
                            }
                            
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                if ((*it_ch)->type_str == "TRIGGER") {
                                    token.info.prop_list.push_back(*it_ch);
                                    prp->type_str = "TRIGGER";
                                    found = true;
                                    
                                }
                            }
                            if (found) {
                                break;
                            }
                            

                            
                            if (has_then_local || has_then) {
                                if (!result.value->prop_list.empty()) {
                                    for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                        prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";
                                        std::vector<Speech_Property*>* sub = nullptr;
                                        if ((*it_ch)->try_get_prop("SEQUENCE_THEN", &sub)) {
                                            for (auto sub_it = sub->begin(); sub_it != sub->end(); ++sub_it) {
                                                prp->push_prop("SEQUENCE_THEN", *sub_it)->sequence_type_str = "SEQUENCE_THEN";
                                            }
                                            (*it_ch)->remove_prop("SEQUENCE_THEN");
                                        } else {
                                            prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";
                                        }
                                    }
                                }
                                break;
                            }
                            

                            for (auto prop_it = result.value->prop_list.begin(); prop_it != result.value->prop_list.end(); ++prop_it) {
                                if ((*prop_it)->annotation == "END_CONDITION") {
                                    prp->push_prop("END_CONDITION", *prop_it);
                                } else {
                                    (*prop_it)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
                                    token.info.prop_list.push_back(*prop_it);
                                }

                            }
                            break;
                        }
                        
                    } else if (child.dep == nlp::DEP_advcl) {
                        bool found_response = false;
                        for (auto ch_it = child.info.prop_list.begin();
                             ch_it != child.info.prop_list.end(); ++ch_it) {
                            if ((*ch_it)->type_str == "RESPONSE") {
                                prp->type_str = "TRIGGER";
                            
                                prp->push_prop("RESPONSE", *ch_it);
                                found_response = true;
                            }
                            
                        }
                        if (found_response) {
                            break;
                        }
                        
                        for (auto r_it = result.value->prop_list.begin(); r_it != result.value->prop_list.end(); ++r_it) {
                            if ((*r_it)->type_str == "TRIGGER") {
                                prp->type_str = "RESPONSE";
                            
                                prp->push_prop("TRIGGER", *r_it);
                                found_response = true;
                            }
                        }
                    } else {
                        bool found = false;
                        for (auto ch_it = child.info.prop_list.begin();
                             ch_it != child.info.prop_list.end(); ++ch_it) {
                            if ((*ch_it)->type_str == "TRIGGER") {
                                prp->type_str = "RESPONSE";
                            
                                prp->push_prop("TRIGGER", *ch_it);
                                found = true;
                            }
                            
                        }
                        if (found) {
                            break;
                        }
                        
                        for (auto r_it = result.value->prop_list.begin(); r_it != result.value->prop_list.end(); ++r_it) {
                            if ((*r_it)->type_str == "TRIGGER") {
                                prp->type_str = "RESPONSE";
                            
                                prp->push_prop("TRIGGER", *r_it);
                                found = true;
                            }
                        }
                        
                        if (found) {
                            break;
                        }
                        
                    }
                    
                    if (child.dep == nlp::DEP_xcomp) {
                        // modifier (e.g. "stop" doing something or "let" something happen)
                        if (true) {
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                prp->push_prop("MODIFIED", *it_ch);
                            }
                        }

                        break;
                    } else if (child.dep == nlp::DEP_ccomp) {
                        for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                            prp->push_prop("MODIFIED", *it_ch);
                        }
                        break;
                    }
                    //                    auto& props = result.value->instruction;
                    //                    for (auto it = props.begin(); it != props.end(); ++it) {
                    //                        ins->push_prop("PROPERTY", *it);
                    //                    }
                    if (has_then) {
                        if (!result.value->prop_list.empty()) {
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";
                                std::vector<Speech_Property*>* sub = nullptr;
                                if ((*it_ch)->try_get_prop("SEQUENCE_THEN", &sub)) {
                                    for (auto sub_it = sub->begin(); sub_it != sub->end(); ++sub_it) {
                                        prp->push_prop("SEQUENCE_THEN", *sub_it)->sequence_type_str = "SEQUENCE_THEN";
                                    }
                                    (*it_ch)->remove_prop("SEQUENCE_THEN");
                                } else {
                                    prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";
                                }
                            }
                        }
                        break;
                        // current action happens after another
                    } else if (has_after) {
                        ASSERT_MSG(false, "Unhandled\n");
                        break;
                    }
                    
                    bool found_AND = false;
                    bool found_BUT_or_HOWEVER_or_SO_or_THUS_or_THEREFORE = false;
                    for (usize R_i = 0; R_i < token.right_children.size(); R_i += 1) {
                        auto* right = parse->token_list[token.right_children[R_i]];
                        if (right->dep == nlp::DEP_cc && right->lemma == "and") {
                            found_AND = true;
                            break;
                        }
                        if (right->dep == nlp::DEP_cc && (right->lemma == "but" || right->lemma == "however" || right->lemma == "so" || right->lemma == "thus" || right->lemma == "therefore")) {
                            found_BUT_or_HOWEVER_or_SO_or_THUS_or_THEREFORE = true;
                            break;
                        }
                    }
                    
                    
                    if (!found_AND && !found_BUT_or_HOWEVER_or_SO_or_THUS_or_THEREFORE) {
                        if (/*!(child.dep == nlp::DEP_advcl && !child.left_children.empty() && */ !child.left_children.empty() && (token_list[child.left_children[0]]->lemma == "while")) {
                            
                            if (!result.value->prop_list.empty()) {
                                auto* seq_prop = prp;//ins->push_prop("SEQUENCE");
                                for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {

//                                    (*it_ch)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
//                                    token.info.prop_list.push_back(*it_ch);
                                }
                            }
                            
                            break;
                        }
                    }
                    //auto* ch___ = &child;
                    //int __BP = 0;
                    
                    bool found = false;
                    for (usize lci = 0; lci < child.left_children.size(); lci += 1) {
                        if (token_list[child.left_children[lci]]->dep == nlp::DEP_nsubj) {
                            
                            if (!result.value->prop_list.empty()) {
                                auto* seq_prop = prp;//ins->push_prop("SEQUENCE");
                                for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {

                                    (*it_ch)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
                                    token.info.prop_list.push_back(*it_ch);
                                }
                            }
                            
                            found = true;
                            break;
                        } else{
                            if (!result.value->prop_list.empty()) {
                                auto* seq_prop = prp;
                                for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {

                                    
                                    (*it_ch)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
                                    token.info.prop_list.push_back(*it_ch);
                                }
                            }
                        }
                    }
                    if (found) {
                        break;
                    } else {
                        if (!result.value->prop_list.empty()) {
                            
                            bool do_exit = false;
                            for (usize L_i = 0; L_i < child.left_children.size(); L_i += 1) {
                                auto* left = parse->token_list[child.left_children[L_i]];
                                if (left->lemma == "then") {
                                    do_exit = true;
                                    break;
                                }
                                
                            }
                            if (do_exit) {
                                break;
                            }
                            
                            if (child.dep == nlp::DEP_conj) {
                                for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                    //ins->push_prop("ACTION", *it_ch)->kind_str = "MUST_FILL_IN_AGENT";
                                    token.info.prop_list.push_back(*it_ch);

                                    
                                    
                                }
                            } else {
                                for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                    prp->push_prop("ACTION", *it_ch)->kind_str = "MUST_FILL_IN_AGENT";
                                    std::vector<Speech_Property*>* sub = nullptr;
                                    if ((*it_ch)->try_get_prop("ACTION", &sub)) {
                                        for (auto sub_it = sub->begin(); sub_it != sub->end(); ++sub_it) {
                                            prp->push_prop("ACTION", *sub_it)->kind_str = "MUST_FILL_IN_AGENT";;
                                        }
                                        (*it_ch)->remove_prop("ACTION");
                                    } else {
                                        prp->push_prop("ACTION", *it_ch)->kind_str = "MUST_FILL_IN_AGENT";;
                                    }
                                    
                                }
                            }
                            
                        }
                        break;
                    }
                    

                    
                    break;
                }
                }
                
                break;
            }
            case nlp::POS_ADJ: {
                auto result = handle_adj(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    auto& p_list = result.value->prop_list;
                    // failure_case, ignore
                    
                    if (p_list.size() == 1 && p_list[0]->tag_str == "ADJ" && p_list[0]->value.kind_string == "TEXT" && dt::magnitude_words.find(p_list[0]->value.text) != dt::magnitude_words.end()) {
                        return {Interp_Status{.type = INTERP_STATUS_ALL_INCOMPLETE}, &token.info};
                    }
                    
                    if (nsubj != nullptr) {
                        auto& src = nsubj->info.prop_list;
                        auto& props = result.value->prop_list;
                        
                        Speech_Property* p_n = nullptr;
                        mtt::String kind = "DEFAULT";
                        if (prp->try_get_only_prop("NEGATED", &p_n)) {
                            if (p_n->value.flag == true) {
                            //auto* p_n = prp->push_prop("PROPERTY_NEGATION");
                            //p_n->value.kind_string = "FLAG";
                            //p_n->value.flag = true;
                                kind = "NEGATED";
                                prp->remove_prop("NEGATED");
                            }
                        }
                        
                        for (auto src_it = src.begin(); src_it != src.end(); ++src_it) {
                            for (auto it = props.begin(); it != props.end(); ++it) {
                                
                                auto* sub_prop = ((*src_it)->push_prop(
                                                                       (*it)->label == "comparison" ? "COMPARISON" : "PROPERTY",
                                                                       *it));//->kind_str = kind;
                                Speech_Property* sub_negated = nullptr;
                                bool sub_negated_value = false;
                                if (sub_prop->try_get_only_prop("NEGATED", &sub_negated)) {
                                    sub_negated_value = sub_negated->value.flag;
                                }
                                
                                // negated
                                if (sub_negated_value == true) {
                                    if (kind == "NEGATED") {
                                        sub_prop->kind_str = "DEFAULT";
                                    } else {
                                        sub_prop->kind_str = "NEGATED";
                                    }
                                } else {
                                    sub_prop->kind_str = ((*it)->label != "comparison") ? kind : "COMPARISON";
                                }
                            }
                        }
                    } else {
                        auto& props = result.value->prop_list;
                        for (auto it = props.begin(); it != props.end(); ++it) {
                            token.info.prop_list.push_back(*it);
                        }
                    }
                    break;
                }
                }
            }
            }
            
            
        }
    }
    
//    std::vector<Speech_Property*>* actions = nullptr;
//    std::vector<Speech_Property*>* agents = nullptr;
//    if (prp->try_get_prop("AGENT", &agents) && prp->try_get_prop("ACTION", &actions)) {
//        for (auto it_action = actions->begin(); it_action != actions->end(); ++it_action) {
////            if ((*it_action)->kind_str == "MUST_FILL_IN_AGENT") {
////                (*it_action)->kind_str = "";
////                (*it_action)->push_prop_list("AGENT", *agents);
////            }
//        }
//    }
    
     if (prop_swap.empty()) {
        if (token.info.prop_list.empty() || *token.info.prop_list.begin() != prp) {
            token.info.prop_list.insert(token.info.prop_list.begin(), prp);
        }
    } else {
        token.info.prop_list.insert(token.info.prop_list.begin(), prop_swap.begin(), prop_swap.end());
    }
    
    
    DT_scope_close();
    
    return {Interp_Status{.type = INTERP_STATUS_OK}, &token.info};
}


inline static mtt::Set<mtt::String> existential_verbs = {
    "be",
    "exist",
    "happen",
    "occur",
};
Interp_Result handle_verb(dt::Lang_Eval_Args* args, dt::Speech_Token& token)
{
    args->curr_dep_token = &token;
    DT_print("%s, lemma=[%s] pos=[%s] dep=[%s]\n", token.text.c_str(), token.lemma.c_str(), dt::spacy_nlp::pos_str[token.pos].c_str(), dt::spacy_nlp::dep_str[token.dep].c_str());
    namespace nlp = dt::spacy_nlp;
    
    dt::System_Evaluation_Context& ctx = args->dt->sys_eval_ctx;
    auto& token_list = args->parse->token_list;
    auto* parse = args->parse;
    
    auto* dt = args->dt;
    DT_scope_open();
    
    token.info.type = WORD_INFO_TYPE_VERBAL;
    token.info.token = &token;
    
    Verbal_Info& info = token.info.action;
    info.keys.push_back(token.lemma);
    info.token = &token;
    
    
    // TODO: get tense info from spacy
    //
    info.tense = VERBAL_TENSE_PRESENT;
    
    info.verb_class = VERB_CLASS_UNKNOWN;
    if (is_stateful_verb(info.token->lemma)) {
        info.verb_class = VERB_CLASS_STATE_CHANGE;
    } else {
        info.verb_class = VERB_CLASS_ACTION;
    }
    
    auto* word_dict_entry = verb_lookup(token.text);
    if (word_dict_entry) {
        
    }
    info.is_bidirectional_by_default = (word_dict_entry != nullptr) ? word_dict_entry->verb_is_bidirectional_by_default : false;
    
    dt::Speech_Property* prp = dt::Speech_Property::make();
    prp->token = &token;
    prp->type_str = "ACTION";
    prp->kind_str = (existential_verbs.find(token.lemma) != existential_verbs.end()) ? "EXISTENTIAL" : "ACTION";
    prp->tag_str = "VERB";
    prp->label = token.lemma;
    Speech_Token_set_prop(&token, prp);
    

    auto& left_children  = token.left_children;
    auto& right_children = token.right_children;
    
    
    bool has_then = false;
    bool has_after = false;
    
    bool is_negated = false;
#ifndef NDEBUG
    mtt::String& LABEL = token.lemma;
#endif
    std::vector<dt::Speech_Property*> prop_swap;
    
    Speech_Token* l_nsubj_token_immediate = nullptr;
    Speech_Token* l_nsubjpass_token_immediate = nullptr;
    switch (token.pos) {
    case nlp::POS_VERB: {
        usize L = 0;
        for (;
             L < left_children.size();
             L += 1) {
            
            Speech_Token& child = *parse->token_list[left_children[L]];
            if (child.is_blocked) {
                continue;
            }
            
            if (child.dep == spacy_nlp::DEP_nsubj) {
                l_nsubj_token_immediate = &child;
            }
            if (child.dep == spacy_nlp::DEP_nsubjpass) {
                l_nsubjpass_token_immediate = &child;
            }
            
            switch (child.pos) {
            default: {
                UNHANDLED();
                break;
            }
            case nlp::POS_PART: {
                if (child.dep == nlp::DEP_neg) {
                    std::vector<Speech_Property*>* props = nullptr;
                    if (prp->try_get_prop("NEGATED", &props)) {
                        is_negated = !is_negated;
                        props->front()->value.flag = is_negated;
                    } else {
                        is_negated = true;
                        auto& negated_prop = *prp->push_prop("NEGATED");
                        negated_prop.value.kind_string = "FLAG";
                        negated_prop.value.flag = true;
                    }
                } else if (child.dep == nlp::DEP_aux && child.lemma == "to") {
                    Speech_Property* inf = nullptr;
                    if (!prp->try_get_only_prop("INFINITIVE", &inf)) {
                        inf = prp->push_prop("INFINITIVE");
                        inf->value.kind_string = "FLAG";
                        inf->value.flag = true;
                    }
                }
                break;
            }
            case nlp::POS_DET: {
                auto result = handle_det(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK:
                    // TODO: find most recent selections
                    DT_print("TODO\n");
                    break;
                }
                break;
            }
            case nlp::POS_PROPN: {
                // fallthrough
            }
            case nlp::POS_NOUN: {
                auto result = handle_noun(args, child);
                
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {

                    verb_handle_noun_on_left(parse, token,  child, result, prp, info);
                    break;
                }
                }
                break;
            }
            case nlp::POS_SCONJ: {
                if (child.lemma == "if") {
                    prp->type_str = "TRIGGER";
                }
//                else if (child.lemma == "while") {
//                   prp->type_str = "TRIGGER";
//                   prp->annotation = "CONTINUOUS";
//                }
                break;
            }
            case nlp::POS_ADP: {
                if (child.lemma == "after") {
                    prp->type_str = "TRIGGER";
                    prp->action_event = VERB_EVENT_END;
                } else if (child.lemma == "as") {
                    prp->type_str = "TRIGGER";
                    //prp->annotation = "RELATION";
                    prp->action_event = VERB_EVENT_CONTINUOUS;
                } else  if (child.lemma == "until") {
                    prp->type_str = "END";
                    prp->annotation = "END_CONDITION";
                } else {
                    switch (child.tag) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case spacy_nlp::TAG_IN: {
                        auto result = handle_IN(args, child);
                        
                        switch (result.status.type) {
                        default: {
                            UNHANDLED();
                            break;
                        }
                        case INTERP_STATUS_OK: {
                            
                            Preposition_Info& prep_info = result.value->preposition_info;
                            if (child.dep == nlp::DEP_prep || child.dep == nlp::DEP_dative) {
                                
                                // Merge
                                info.has_preposition = true;
                                info.preposition = &child;
                                info.keys.push_back(child.lemma);
                                for (usize pidx = 0; pidx < prep_info.objects_of_preposition.size(); pidx += 1) {
                                    info.objects.push_back(prep_info.objects_of_preposition[pidx]);
                                    prep_info.objects_of_preposition[pidx]->acted_on_by_ref = &token.info;
                                }
                                
                                
                                
                                Speech_Property* local_root = nullptr;
                                if (has_then && prop_swap.empty() && token.dep != nlp::DEP_conj) {
                                    local_root = prp->push_prop("SEQUENCE_THEN");
                                    local_root->sequence_type_str = "SEQUENCE_THEN";
                                    local_root->annotation = "MUST_FILL_IN_AGENT";
                                } else if (has_then && prop_swap.empty()){
                                    local_root = prp;
                                    local_root->annotation = "MUST_FILL_IN_AGENT";
                                    
                                } else {
                                    local_root = prp;
                                }
                                for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                    local_root->push_prop("PREPOSITION", *it);
                                }
                                
                                
                            } else if (child.dep == spacy_nlp::DEP_agent) {
                                for (usize pidx = 0; pidx < prep_info.objects_of_preposition.size(); pidx += 1) {
                                    info.objects.push_back(prep_info.objects_of_preposition[pidx]);
                                    prep_info.objects_of_preposition[pidx]->is_main_actor_or_agent = true;
                                    prep_info.objects_of_preposition[pidx]->is_acted_upon_or_patient = false;
                                    prep_info.objects_of_preposition[pidx]->should_likely_represent = true;
                                    prep_info.objects_of_preposition[pidx]->action_ref = &token.info;
                                    

                                }
                                
                                auto& semantic_prop = *prp;//->push_prop("AGENT");
                                for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                    semantic_prop.push_prop("AGENT", *it);
                                }
                            }
                            break;
                        }
                        }
                        break;
                    }
                    }
                    break;
                }
                break;
            }
            case nlp::POS_ADV: {
                switch (child.tag) {
                default: {
                    UNHANDLED();
                    break;
                }
                case nlp::TAG::RB: {
                    auto result = handle_RB(args, child);
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        if (child.lemma == "then") {
                            has_then = true;
                            break;
                        }
//                        else if (child.lemma == "after") {
//                            has_after = true;
//                            break;
//                        }
                        
                        
                        auto& props = result.value->prop_list;
                        for (auto it = props.begin(); it != props.end(); ++it) {
                            prp->push_prop("PROPERTY", *it);
                        }
                        
                        
                        break;
                    }
                    }
                    
                    break;
                }
                case nlp::TAG_WRB: {
                    auto result = handle_WHRB(args, child);
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        
                        
                        auto& wrb_info = result.value->whadverb_info;
                        if (wrb_info.is_question) {
                            UNHANDLED();
                            break;
                        } else if (wrb_info.is_time_expression) {
                            UNHANDLED();
                            break;
                        } else {
                            info.should_treat_as_trigger = true;
                            prp->type_str = "TRIGGER";
                        }
                        
                        switch (child.dep) {
                        default: {
                            UNHANDLED();
                            break;
                        }
                        case nlp::DEP_advmod: {
                            // TODO: other cases?
                            break;
                        }
                        }
                        
                        break;
                    }
                    }
                    
                    
                    
                    break;
                }
                }
                break;
            }
            case nlp::POS_PRON: {
                auto result = handle_pronoun(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    break;
                }
                }
                
                switch (child.tag) {
                default: {
                    UNHANDLED();
                    break;
                }
                case nlp::TAG_NN: {
                    // fallthrough
                    for (auto it_p = result.value->prop_list.begin(); it_p != result.value->prop_list.end(); ++it_p) {
                        if ((*it_p)->label == "something" || (*it_p)->label == "nothing") {
                            auto* spec = (*it_p)->push_prop("SPECIFIC_OR_UNSPECIFIC");
                            spec->value.kind_string = "FLAG";
                            spec->value.flag = false;
                        }
                    }
                }
                case nlp::TAG_PRP: {
                    // unresolved nouns
                    
                    auto& pron_info = result.value->pronoun_info;
                    pron_info.corresponding_nom_info.action_ref = &token.info;
                    if (pron_info.is_plural) {
                        //info.
                    } else {
                        
                    }
                    info.agents.push_back(&pron_info.corresponding_nom_info);
                    
                    
                    
                    {
                        bool treat_as_agent                 = false;
                        bool treat_as_patient               = false;
                        bool treat_as_direct_object         = false;
                        bool treat_as_indirect_object       = false;
                        bool treat_as_object_of_preposition = false;
                        mtt::String key;
                        
                        auto& ch_props = result.value->prop_list;
                        
                        switch (child.dep) {
                        case spacy_nlp::DEP_dobj:
                            DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
                            treat_as_patient = true;
                            treat_as_direct_object = true;
                            key = "DIRECT_OBJECT";
                            break;
                        case spacy_nlp::DEP_nsubj:
                            treat_as_agent = true;
                            key = "AGENT";
                            break;
                        case spacy_nlp::DEP_dative:
                            // fallthrough
                        case spacy_nlp::DEP_nsubjpass:
                            treat_as_indirect_object = true;
                            DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
                            key = "INDIRECT_OBJECT";
                            break;
                        case spacy_nlp::DEP_pobj:
                            treat_as_object_of_preposition = true;
                            key = "OBJECT_OF_PREPOSITION";
                            break;
                        }
                        DT_print("Verb treat noun %s as [agent : %s, patient : %s, direct_object : %s, indirect_object : %s, object_of_preposition : %s\n",
                                 child.info.prop_list.front()->label.c_str(),
                                 bool_str(treat_as_agent),
                                 bool_str(treat_as_patient),
                                 bool_str(treat_as_direct_object),
                                 bool_str(treat_as_indirect_object),
                                 bool_str(treat_as_object_of_preposition));
                        
                        if constexpr ((DT_OLD_NESTING_WITH_SELECT))
                        {
                            auto& semantic_prop = *prp->push_prop(key);
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                semantic_prop.push_prop("SELECT", *it);
                            }
                        } else {
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                prp->push_prop(key, *it);
                            }
                        }
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_AUX: {
                if (child.dep == nlp::DEP_auxpass) {
                    bool handled_noun = false;
                    for (auto it_L = child.left_children.begin(); it_L != child.left_children.end(); ++it_L) {
                        Speech_Token* sub_child = token_list[*it_L];
                        
                        if (sub_child->pos == nlp::POS_NOUN || sub_child->pos == nlp::POS_PROPN) {
                            auto result = handle_noun(args, *sub_child);
                            switch (result.status.type) {
                            default: {
                                UNHANDLED();
                                break;
                            }
                            case INTERP_STATUS_OK: {
                                verb_handle_noun_on_left(parse, token, *sub_child, result, prp, info);
                                handled_noun = true;
                                break;
                            }
                            }
                        }
                    }
                } else if (child.dep == spacy_nlp::DEP_ccomp) {
                    auto result = handle_aux(args, child);
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        FOR_ITER(it, result.value->prop_list, ++it) {
                            Speech_Property* f = *it;
                            //prp->push_prop("PREV_SENTENCE", (*it));
                            ctx.fragments.push_back(f);
                        }
                        break;
                    }
                    }
                }
                break;
            }
            case nlp::POS_VERB: {
                auto result = handle_verb(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                
                    if (has_then || (!(child.right_children.empty()) && token_list[child.right_children[0]]->lemma == "then")) {
                        has_then = true;
                        if (!result.value->prop_list.empty()) {
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                dt::Speech_Property* child = *it_ch;
                                child->push_prop("SEQUENCE_THEN", prp)->sequence_type_str = "SEQUENCE_THEN";;
                                prop_swap.push_back(child);
                                
                            }
                        }
                        break;
                        // current action happens after another
                    } else if (has_after) {
                        ASSERT_MSG(false, "Unhandled\n");
                        break;
                    }
                
                    switch (child.dep) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                        // probably a condition
                    case nlp::DEP_csubj: {
                        for (auto prop_it = result.value->prop_list.begin(); prop_it != result.value->prop_list.end(); ++prop_it) {
                            {
                                prp->push_prop("GOAL", *prop_it);
                            }
                        }
                        break;
                    }
                    case nlp::DEP_advcl: {
                        
                        bool something_found = false;
                        for (auto ch_it = child.info.prop_list.begin();
                             ch_it != child.info.prop_list.end(); ++ch_it) {
                            if ((*ch_it)->type_str == "TRIGGER") {
                                prp->type_str = "RESPONSE";
                                //prop_swap.push_back(*ch_it);
                                prp->push_prop("TRIGGER", *ch_it);
                                (*ch_it)->type_str = "TRIGGER";
                            
                                something_found = true;
                            }
                        }
                        
                        // TODO: check if any breakages
                        if (!something_found) {
                            for (auto prop_it = result.value->prop_list.begin(); prop_it != result.value->prop_list.end(); ++prop_it) {
                                
                                if ((*prop_it)->try_get_only_prop("INFINITIVE")) {
                                    prp->push_prop("GOAL", *prop_it);
                                } else {
                                    (*prop_it)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
                                    token.info.prop_list.push_back(*prop_it);
                                }

                            }
                        }
    //                    if (child->type_str == "if") {
    //                        ins->type_str = "TRIGGER";
    //                    }
                        auto& v_info = result.value->action;
                        if (v_info.should_treat_as_trigger) {
                            info.should_treat_as_trigger_response = true;
                            info.associated.emplace_back(&v_info);
                            
                            token.info.type = WORD_INFO_TYPE_RULE;
                            token.info.rule.token = &token;
                            
                            token.info.rule.trigger_info = result.value;
                            token.info.rule.response_info = &token.info;
                            
                            token.info.action.rule_ref = &token.info;
                            child.info.action.rule_ref = &token.info;
                            
                        } else {
                            UNHANDLED();
                            break;
                        }
                        break;
                    } case spacy_nlp::DEP_ccomp: {
                        FOR_ITER(it, result.value->prop_list, ++it) {
                            Speech_Property* f = *it;
                            //prp->push_prop("PREV_SENTENCE", (*it));
                            ctx.fragments.push_back(f);
                        }
                        break;
                    }
                    }
                    break;
                }
                }
                break;
            }
                
                
            }
            
            
        }
        usize R = 0;
        for (;
             R < right_children.size();
             R += 1) {
            Speech_Token& child = *parse->token_list[right_children[R]];
            if (child.is_blocked) {
                continue;
            }
            
            if (child.lemma  == "thrice") {
                child.pos = nlp::POS_ADV;
            }
            
            switch (child.pos) {
            default: {
                UNHANDLED();
                break;
            }
            case nlp::POS_AUX: {
                auto result = handle_aux(args, child);
                
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    if (child.dep == spacy_nlp::DEP_ccomp) {
                        FOR_ITER(it, result.value->prop_list, ++it) {
                            Speech_Property* f = *it;
                            //prp->push_prop("PREV_SENTENCE", (*it));
                            ctx.fragments.push_back(f);
                        }
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_PART: {
                if (child.dep == nlp::DEP_neg) {
                    std::vector<Speech_Property*>* props = nullptr;
                    if (prp->try_get_prop("NEGATED", &props)) {
                        is_negated = !is_negated;
                        props->front()->value.flag = is_negated;
                    } else {
                        is_negated = true;
                        auto& negated_prop = *prp->push_prop("NEGATED");
                        negated_prop.value.kind_string = "FLAG";
                        negated_prop.value.flag = true;
                    }
                }
                break;
            }
            case nlp::POS_PUNCT: {
                //DT_print("WARNING: am not yet handling end of sentence\n");
                break;
            }
            case spacy_nlp::POS_ADP: {
                switch (child.tag) {
                default: {
                    UNHANDLED();
                    break;
                }
                case spacy_nlp::TAG_RP:
                case spacy_nlp::TAG_IN: {
                    auto result = handle_IN(args, child);
                    
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        
                        Preposition_Info& prep_info = result.value->preposition_info;
                        
                        if (child.dep == nlp::DEP_prt) {
                            auto& sub_prp = *prp->push_prop("PROPERTY");
                            sub_prp.label = "trait";
                            sub_prp.type_str = "PROPERTY";
                            sub_prp.value.kind_string = "TEXT";
                            sub_prp.value.text = child.lemma;
                        } else if (child.dep == nlp::DEP_prep || child.dep == nlp::DEP_dative) {
                            
                            // Merge
                            info.has_preposition = true;
                            info.preposition = &child;
                            info.keys.push_back(child.lemma);
                            for (usize pidx = 0; pidx < prep_info.objects_of_preposition.size(); pidx += 1) {
                                info.objects.push_back(prep_info.objects_of_preposition[pidx]);
                                prep_info.objects_of_preposition[pidx]->acted_on_by_ref = &token.info;
                            }
                            
                            
                            
                            Speech_Property* local_root = nullptr;
                            if (has_then && prop_swap.empty() && token.dep != nlp::DEP_conj) {
                                local_root = prp->push_prop("SEQUENCE_THEN");
                                local_root->sequence_type_str = "SEQUENCE_THEN";
                                local_root->annotation = "MUST_FILL_IN_AGENT";
                            } else if (has_then && prop_swap.empty()){
                                local_root = prp;
                                local_root->annotation = "MUST_FILL_IN_AGENT";
                                
                            } else {
                                local_root = prp;
                            }
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                local_root->push_prop("PREPOSITION", *it);
                            }
                            
                            
                        } else if (child.dep == spacy_nlp::DEP_agent) {
                            for (usize pidx = 0; pidx < prep_info.objects_of_preposition.size(); pidx += 1) {
                                info.objects.push_back(prep_info.objects_of_preposition[pidx]);
                                prep_info.objects_of_preposition[pidx]->is_main_actor_or_agent = true;
                                prep_info.objects_of_preposition[pidx]->is_acted_upon_or_patient = false;
                                prep_info.objects_of_preposition[pidx]->should_likely_represent = true;
                                prep_info.objects_of_preposition[pidx]->action_ref = &token.info;
                                

                            }
                            
                            auto& semantic_prop = *prp;//->push_prop("AGENT");
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                semantic_prop.push_prop("AGENT", *it);
                            }
                        }
                        break;
                    }
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_PROPN: {
                // fallthrough
            }
            case nlp::POS_NOUN: {
                auto result = handle_noun(args, child);
                
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                    if (token.dep == spacy_nlp::DEP_acl) {
                        switch (child.dep) {
                        case spacy_nlp::DEP_oprd:
                            // so the verb is really a property like "was named X"
                            prp->type_str = "PROPERTY";
                            prp->tag_str = "";
                            prp->kind_str = token.lemma;
                            prp->value.kind_string = "TEXT";
                            prp->value.text = child.lemma;
                            break;
                        }
                        break;
                    } else if (child.dep == spacy_nlp::DEP_npadvmod) {
                        if (result.value->nominal.is_time_expression) {
                            for (auto it_sub = child.info.prop_list.begin(); it_sub != child.info.prop_list.end(); ++it_sub) {
                                prp->push_prop("TIME", *it_sub);
                            }
                            break;
                        }
                    }
                    
                    bool treat_as_agent                 = false;
                    bool treat_as_patient               = false;
                    bool treat_as_direct_object         = false;
                    bool treat_as_indirect_object       = false;
                    bool treat_as_object_of_preposition = false;
                    mtt::String key;
                    
                    auto& ch_props = result.value->prop_list;
                    switch (child.dep) {
                    case spacy_nlp::DEP_dobj:
                        treat_as_patient = true;
                        treat_as_direct_object = true;
                        key = "DIRECT_OBJECT";
                        break;
                    case spacy_nlp::DEP_attr:
                        if (token.lemma == "become" || token.lemma == "form") {
                            treat_as_patient = true;
                            treat_as_direct_object = true;
                            key = "DIRECT_OBJECT";
                            break;
                        } else {
                            MTT_FALLTHROUGH;
                        }
                    case spacy_nlp::DEP_nsubj:
                        treat_as_agent = true;
                        key = "AGENT";
                        break;
                    case spacy_nlp::DEP_dative:
                        MTT_FALLTHROUGH;
                    case spacy_nlp::DEP_nsubjpass:
                        treat_as_indirect_object = true;
                        key = "INDIRECT_OBJECT";
                        break;
                    case spacy_nlp::DEP_pobj:
                        treat_as_object_of_preposition = true;
                        key = "OBJECT_OF_PREPOSITION";
                        break;
                    case spacy_nlp::DEP_oprd:
                        MTT_FALLTHROUGH;
                    case spacy_nlp::DEP_npadvmod:
                        key = "PROPERTY";
                    }
                
                    
                    DT_print("Verb treat noun %s as [agent : %s, patient : %s, direct_object : %s, indirect_object : %s, object_of_preposition : %s\n",
                             child.info.prop_list.front()->label.c_str(),
                             bool_str(treat_as_agent),
                             bool_str(treat_as_patient),
                             bool_str(treat_as_direct_object),
                             bool_str(treat_as_indirect_object),
                             bool_str(treat_as_object_of_preposition));
                    
                    {
                        
                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                            if ((*it)->tag_str == "TIME") {
                                prp->push_prop("TIME", *it);
                            } else {
                                if constexpr ((DT_OLD_NESTING_WITH_SELECT)) {
                                    auto& semantic_prop = *prp->push_prop(key);
                                    for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                        semantic_prop.push_prop("SELECT", *it);
                                    }
                                } else {
                                    
                                    
                                    
                                    bool deixis_assumed = false;
                                    if (prp->kind_str == "EXISTENTIAL") {
                                        if (l_nsubj_token_immediate != nullptr) {
                                            auto dx_it = dt::deictic_words.find(l_nsubj_token_immediate->text);
                                            if (dx_it != dt::deictic_words.end()) {
                                                
                                                for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {

                                                    auto* sub = prp->push_prop(key, *it);
                                                    auto* deixis = sub->push_prop("DEIXIS");
                                                    deixis->label = *dx_it;
                                                    deixis->type_str = "DEIXIS";
                                                    deixis_assumed = true;
                                                }
                                            }
                                        }
                                    } else {

                                    }
                                    if (!deixis_assumed) {
                                        for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                            if (child.dep != spacy_nlp::DEP_oprd) {
                                                prp->push_prop(key, *it);
                                            } else {
                                                if (l_nsubjpass_token_immediate != nullptr && l_nsubjpass_token_immediate->prop_ref != nullptr) {

                                                    
                                                    Speech_Property* dobj = nullptr;
                                                    prp->try_get_only_prop("DIRECT_OBJECT", &dobj);
                                                    if (dobj == nullptr) {
                                                        dobj = prp->push_prop("DIRECT_OBJECT");
                                                        dobj->label = "thing";
                                                        dobj->tag_str = "NOUN";
                                                        dobj->kind_str = "THING_INSTANCE";
                                                        dobj->value.kind_string = "THING_INSTANCE";
                                                        dobj->value.thing = mtt::Thing_ID_INVALID;
                                                        dobj->token = nullptr;
                                                        auto* spec = dobj->push_prop("SPECIFIC_OR_UNSPECIFIC");
                                                        spec->label = l_nsubjpass_token_immediate->lemma;
                                                        spec->tag_str = "DET";
                                                        spec->type_str = "VALUE";
                                                        spec->kind_str = "SPECIFIC";
                                                        spec->value.kind_string = "FLAG";
                                                        spec->value.flag = true;
                                                        auto* deixis = dobj->push_prop("DEIXIS");
                                                        deixis->label = l_nsubjpass_token_immediate->lemma;
                                                        deixis->type_str = "DEIXIS";
                                                        auto* plural = dobj->push_prop("PLURAL");
                                                        plural->type_str = "VALUE";
                                                        plural->value.kind_string = "FLAG";
                                                        dobj->token = mem::alloc_init<Speech_Token>(&dt->token_allocation.allocator);
                                                        if (deixis->label == "this" || deixis->label == "that" ||
                                                            deixis->label == "these" || deixis->label == "those") {
                                                            plural->value.flag = false;
                                                            dobj->token->tag = spacy_nlp::TAG_NN;
                                                            dobj->token->pos = spacy_nlp::POS_NOUN;
                                                        } else {
                                                            plural->value.flag = true;
                                                            dobj->token->tag = spacy_nlp::TAG_NNS;
                                                            dobj->token->pos = spacy_nlp::POS_NOUN;
                                                        }
                                                        
                                                        auto* sub = dobj->push_prop(key, *it);
                                                        if (dt::naming_words.contains(token.lemma)) {
                                                            sub->label = token.lemma;
                                                        } else {
                                                            sub->label = "trait";
                                                        }
                                                        sub->type_str = "PROPERTY";
                                                        sub->kind_str = "DEFAULT";
                                                        sub->key = "PROPERTY";
                                                        sub->value.kind_string = "TEXT";
                                                        sub->value.text = child.lemma;
                                                        (void)sub;

                                                    } else {
                                                        auto* sub = l_nsubjpass_token_immediate->prop_ref->push_prop(key, *it);
                                                        if (dt::naming_words.contains(token.lemma)) {
                                                            sub->label = token.lemma;
                                                        } else {
                                                            sub->label = "trait";
                                                        }
                                                        sub->type_str = "PROPERTY";
                                                        sub->kind_str = "DEFAULT";
                                                        sub->key = "PROPERTY";
                                                        sub->value.kind_string = "TEXT";
                                                        sub->value.text = child.lemma;
                                                        (void)sub;
                                                    }
                                                } else {
                                                    prp->push_prop(key, *it);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    // TODO: //
                    auto& nom = result.value->nominal;
                    if (nom.is_main_actor_or_agent) {
                        nom.action_ref = &token.info;
                        info.agents.push_back(&nom);
                        // TODO: Do I need this sort of "associated" array? Probably for merging
                        for (usize nidx = 0; nidx < nom.associated.size(); nidx += 1) {
                            info.agents.push_back(nom.associated[nidx]);
                        }
                    } else {
                        //UNHANDLED();
                        break;
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_ADV: {
                switch (child.tag) {
                default: {
                    if (child.dep == nlp::DEP_advmod) {
                        MTT_FALLTHROUGH;
                    } else {
                        break;
                    }
                }
                case nlp::TAG::RB: {
                    auto result = handle_RB(args, child);
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        
                        if (child.lemma == "then") {
                            has_then = true;
                            break;
                        }
//                        else if (child.lemma == "after") {
//                            has_after = true;
//                            break;
//                        }
                        
                        auto& props = result.value->prop_list;
                        for (auto it = props.begin(); it != props.end(); ++it) {
                            prp->push_prop("PROPERTY", *it);
                        }
                        
                        
                        {
                            auto& current_child = child;
                            usize rb_R = 0;
                            for (;
                                 rb_R < child.right_children.size();
                                 rb_R += 1) {
                                Speech_Token& child = *parse->token_list[current_child.right_children[rb_R]];
                                if (child.is_blocked) {
                                    continue;
                                }
                                switch (child.tag) {
                                    case spacy_nlp::TAG_IN: {
                                        auto rb_result = handle_IN(args, child);
                                        
                                        switch (rb_result.status.type) {
                                            default: {
                                                UNHANDLED();
                                                break;
                                            }
                                            case INTERP_STATUS_OK: {
                                                
                                                Preposition_Info& prep_info = rb_result.value->preposition_info;
                                                if (child.dep == nlp::DEP_prep || child.dep == nlp::DEP_dative) {
                                                    
                                                    // Merge
                                                    info.has_preposition = true;
                                                    info.preposition = &child;
                                                    info.keys.push_back(child.lemma);
                                                    for (usize pidx = 0; pidx < prep_info.objects_of_preposition.size(); pidx += 1) {
                                                        info.objects.push_back(prep_info.objects_of_preposition[pidx]);
                                                        prep_info.objects_of_preposition[pidx]->acted_on_by_ref = &token.info;
                                                    }
                                                    
                                                    
                                                    
                                                    Speech_Property* local_root = nullptr;
                                                    if (has_then && prop_swap.empty() && token.dep != nlp::DEP_conj) {
                                                        local_root = prp->push_prop("SEQUENCE_THEN");
                                                        local_root->sequence_type_str = "SEQUENCE_THEN";
                                                        local_root->annotation = "MUST_FILL_IN_AGENT";
                                                    } else if (has_then && prop_swap.empty()){
                                                        local_root = prp;
                                                        local_root->annotation = "MUST_FILL_IN_AGENT";
                                                        
                                                    } else {
                                                        local_root = prp;
                                                    }
                                                    for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                                        local_root->push_prop("PREPOSITION", *it);
                                                    }
                                                    
                                                    
                                                } else if (child.dep == spacy_nlp::DEP_agent) {
                                                    for (usize pidx = 0; pidx < prep_info.objects_of_preposition.size(); pidx += 1) {
                                                        info.objects.push_back(prep_info.objects_of_preposition[pidx]);
                                                        prep_info.objects_of_preposition[pidx]->is_main_actor_or_agent = true;
                                                        prep_info.objects_of_preposition[pidx]->is_acted_upon_or_patient = false;
                                                        prep_info.objects_of_preposition[pidx]->should_likely_represent = true;
                                                        prep_info.objects_of_preposition[pidx]->action_ref = &token.info;
                                                        

                                                    }
                                                    
                                                    auto& semantic_prop = *prp;//->push_prop("AGENT");
                                                    for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                                        semantic_prop.push_prop("AGENT", *it);
                                                    }
                                                }
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                    default: {
                                        UNHANDLED();
                                        break;
                                    }
                                }
                            }
                            child = current_child;
                        }
                        
                        break;
                    }
                    }
                    
                    break;
                }
                case nlp::TAG_WRB: {
                    auto result = handle_WHRB(args, child);
                    switch (result.status.type) {
                    default: {
                        UNHANDLED();
                        break;
                    }
                    case INTERP_STATUS_OK: {
                        
                        
                        auto& wrb_info = result.value->whadverb_info;
                        if (wrb_info.is_question) {
                            UNHANDLED();
                            break;
                        } else if (wrb_info.is_time_expression) {
                            UNHANDLED();
                            break;
                        } else {
                            
                            info.should_treat_as_trigger_response = true;
                            prp->type_str = "RESPONSE";
                        }
                        
                        switch (child.dep) {
                        default: {
                            UNHANDLED();
                            break;
                        }
                        case nlp::DEP_advmod: {
                            // TODO: other cases?
                            break;
                        }
                        }
                        
                        break;
                    }
                    }
                    
                    
                    
                    break;
                }
                }
                break;
            }
            case nlp::POS_VERB: {
                
                auto result = handle_verb(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                    if (child.dep == nlp::DEP_conj) {
                        if (prp->type_str == "TRIGGER") {
                            prp->type_str = "TRIGGER";
                            bool found_trigger = false;
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                token.info.prop_list.push_back(*it_ch);
                                (*it_ch)->type_str = "TRIGGER";
                                found_trigger = true;
                            }
                            
                            if (found_trigger) {
                                break;
                            }
                            
                        } else if (prp->type_str == "RESPONSE") {
                            bool has_then_local = false;
                            if (!has_then) {
                                for (auto lc = child.left_children.begin(); lc != child.left_children.end(); ++lc) {
                                    auto* tok = token_list[*lc];
                                    if (tok->lemma == "then") {
                                        has_then_local = true;
                                        break;
                                    }
                                }
                            }
                            bool found_response = false;
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                if ((*it_ch)->has_prop("TRIGGER")) {
                                    //prp->push_prop("TRIGGER_RESPONSERESPONSE", *it_ch);
                                    token.info.prop_list.push_back(*it_ch);
                                } else {
                                    if (has_then || has_then_local) {
                                       // prp->push_prop("ANOTHER_RESPONSE_CLAUSE_THEN", *it_ch);
                                        {
                                            prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";;
                                            std::vector<Speech_Property*>* sub = nullptr;
                                            if ((*it_ch)->try_get_prop("SEQUENCE_THEN", &sub)) {
                                                for (auto sub_it = sub->begin(); sub_it != sub->end(); ++sub_it) {
                                                    prp->push_prop("SEQUENCE_THEN", *sub_it)->sequence_type_str = "SEQUENCE_THEN";;
                                                }
                                                (*it_ch)->remove_prop("SEQUENCE_THEN");
                                            } else {
                                                prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";;
                                            }
                                        }
                                    } else {
                                        prp->push_prop("ANOTHER_RESPONSE_CLAUSE_AND", *it_ch);
                                    }
                                }
                                //token.info.prop_list.push_back(*it_ch);
                                (*it_ch)->type_str = "RESPONSE";
                                found_response = true;
                            }
                            
                            if (found_response) {
                                break;
                            }
                            
                        } else {
                            bool has_then_local = false;
#if !DT_CHAINED_AND_THEN_FIX
                            if (!has_then)
#endif
                            {
                                for (auto lc = child.left_children.begin(); lc != child.left_children.end(); ++lc) {
                                    auto* tok = token_list[*lc];
                                    if (tok->lemma == "then") {
                                        has_then_local = true;
                                        break;
                                    }
                                }
                            }
                            bool found = false;
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                if ((*it_ch)->type_str == "RESPONSE") {
                                    //token.info.prop_list.push_back(*it_ch);
                                    if ((*it_ch)->has_prop("TRIGGER")) {
                                        //prp->push_prop("TRIGGER_RESPONSE", *it_ch);
                                        token.info.prop_list.push_back(*it_ch);
                                    } else {
                                        if (has_then || has_then_local) {
                                            //prp->push_prop("ANOTHER_RESPONSE_CLAUSE_THEN", *it_ch);
                                            
                                                {
                                                    prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";;
                                                    std::vector<Speech_Property*>* sub = nullptr;
                                                    if ((*it_ch)->try_get_prop("SEQUENCE_THEN", &sub)) {
                                                        for (auto sub_it = sub->begin(); sub_it != sub->end(); ++sub_it) {
                                                            prp->push_prop("SEQUENCE_THEN", *sub_it)->sequence_type_str = "SEQUENCE_THEN";;
                                                        }
                                                        (*it_ch)->remove_prop("SEQUENCE_THEN");
                                                    } else {
                                                        prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";;
                                                    }
                                                }
                                            
                                            continue;
                                        } else {
                                            prp->push_prop("ANOTHER_RESPONSE_CLAUSE_AND", *it_ch);
                                        }
                                    }

                                    prp->type_str = "RESPONSE";
                                    found = true;
                                    
                                }
                            }
                            if (found) {
                                break;
                            }
                            
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                if ((*it_ch)->type_str == "TRIGGER") {
                                    token.info.prop_list.push_back(*it_ch);
                                    prp->type_str = "TRIGGER";
                                    found = true;
                                    
                                }
                            }
                            if (found) {
                                break;
                            }
                        
                            
                            if (has_then_local /*|| has_then*/) {
                                if (!result.value->prop_list.empty()) {
                                    for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                        prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";;
                                        std::vector<Speech_Property*>* sub = nullptr;
                                        if ((*it_ch)->try_get_prop("SEQUENCE_THEN", &sub)) {
                                            for (auto sub_it = sub->begin(); sub_it != sub->end(); ++sub_it) {
                                                prp->push_prop("SEQUENCE_THEN", *sub_it)->sequence_type_str = "SEQUENCE_THEN";;
                                            }
                                            (*it_ch)->remove_prop("SEQUENCE_THEN");
                                        } else {
                                            prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";;
                                        }
                                    }
                                }
                                break;
                            }
                            
                            

                            for (auto prop_it = result.value->prop_list.begin(); prop_it != result.value->prop_list.end(); ++prop_it) {

                            
                                if ((*prop_it)->annotation == "END_CONDITION") {
                                    prp->push_prop("END_CONDITION", *prop_it);
                                } else {
                                    (*prop_it)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
                                    prp->push_prop("SEQUENCE_SIMULTANEOUS", *prop_it);
                                }
                                //token.info.prop_list.push_back(*prop_it);
                                

                            }
                            break;
                        }
                        
                    } else if (child.dep == nlp::DEP_advcl) {
                        bool found_response = false;
                        for (auto ch_it = child.info.prop_list.begin();
                             ch_it != child.info.prop_list.end(); ++ch_it) {
                            if ((*ch_it)->type_str == "RESPONSE") {
                                prp->type_str = "TRIGGER";
                            
                                prp->push_prop("RESPONSE", *ch_it);
                                found_response = true;
                            }
                            
                        }
                        if (found_response) {
                            break;
                        }
                        
                        bool found_trigger = false;
                        for (auto r_it = result.value->prop_list.begin(); r_it != result.value->prop_list.end(); ++r_it) {
                            if ((*r_it)->type_str == "TRIGGER") {
                                prp->type_str = "RESPONSE";
                            
                                prp->push_prop("TRIGGER", *r_it);
//                                found_response = true;
                                
                                found_trigger = true;
                            }
                        }
                        
                        // TODO: check if any breakages
                        if (!found_trigger) {
                            for (auto prop_it = result.value->prop_list.begin(); prop_it != result.value->prop_list.end(); ++prop_it) {
                                bool propagate_up = true;
                                if ((*prop_it)->label == "use") {
                                    prp->push_prop("SEQUENCE_SIMULTANEOUS", *prop_it)->kind_str = "MEANS" ;
                                } else {
                                //(*prop_it)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
                                    if ((*prop_it)->try_get_only_prop("INFINITIVE")) {
                                        prp->push_prop("GOAL", *prop_it);
                                        (*prop_it)->remove_prop("INFINITIVE");
                                        continue;
                                    } else {
                                        
                                        if ((*prop_it)->annotation == "END_CONDITION") {
                                            prp->push_prop("END_CONDITION", *prop_it);
                                        } else {
                                            prp->push_prop("SEQUENCE_SIMULTANEOUS", *prop_it);
                                            propagate_up = false;
                                        }
                                    }
                                    
                                }
                                if (propagate_up) {
                                    token.info.prop_list.push_back(*prop_it);
                                }

                            }
                        }
                        
                    } else {
                        bool found = false;
                        for (auto ch_it = child.info.prop_list.begin();
                             ch_it != child.info.prop_list.end(); ++ch_it) {
                            if ((*ch_it)->type_str == "TRIGGER") {
                                prp->type_str = "RESPONSE";
                            
                                prp->push_prop("TRIGGER", *ch_it);
                                found = true;
                            }
                            
                        }
                        if (found) {
                            break;
                        }
                        
                        for (auto r_it = result.value->prop_list.begin(); r_it != result.value->prop_list.end(); ++r_it) {
                            if ((*r_it)->type_str == "TRIGGER") {
                                prp->type_str = "RESPONSE";
                            
                                prp->push_prop("TRIGGER", *r_it);
                                found = true;
                            }
                        }
                        
                        if (found) {
                            break;
                        }

                        
                    }
                    
                    if (child.dep == nlp::DEP_xcomp) {
                        // modifier (e.g. "stop" doing something or "let" something happen)
                        for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                            auto* ch_prop = (*it_ch);
                            Speech_Property* inf = nullptr;
                            {
                                if (ch_prop->try_get_only_prop("INFINITIVE", &inf)) {
                                    
                                    if (inf->value.flag == true) {
                                        prp->push_prop("GOAL", *it_ch);
                                        ch_prop->remove_prop("INFINITIVE");
                                    } else {
                                        prp->push_prop("MODIFIED", *it_ch);
                                    }
                                } else {
                                    prp->push_prop("ACTION", *it_ch);
                                }
                            }
                            
                        }

                        break;
                    } else if (child.dep == nlp::DEP_ccomp) {
                        for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                            prp->push_prop("MODIFIED", *it_ch);
                        }
                        break;
                    }
                    //                    auto& props = result.value->instruction;
                    //                    for (auto it = props.begin(); it != props.end(); ++it) {
                    //                        ins->push_prop("PROPERTY", *it);
                    //                    }
                    if (has_then) {
                        if (!result.value->prop_list.empty()) {
                            for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";;
                                std::vector<Speech_Property*>* sub = nullptr;
                                if ((*it_ch)->try_get_prop("SEQUENCE_THEN", &sub)) {
                                    for (auto sub_it = sub->begin(); sub_it != sub->end(); ++sub_it) {
                                        prp->push_prop("SEQUENCE_THEN", *sub_it)->sequence_type_str = "SEQUENCE_THEN";;
                                    }
                                    (*it_ch)->remove_prop("SEQUENCE_THEN");
                                } else {
                                    prp->push_prop("SEQUENCE_THEN", *it_ch)->sequence_type_str = "SEQUENCE_THEN";
                                }
                            }
                        }
                        break;
                        // current action happens after another
                    } else if (has_after) {
                        ASSERT_MSG(false, "Unhandled\n");
                        break;
                    }
                    
                    bool found_AND = false;
                    bool found_BUT_or_HOWEVER_or_SO_or_THUS_or_THEREFORE = false;
                    for (usize R_i = 0; R_i < token.right_children.size(); R_i += 1) {
                        auto* right = parse->token_list[token.right_children[R_i]];
                        if (right->dep == nlp::DEP_cc && right->lemma == "and") {
                            found_AND = true;
                            break;
                        }
                        if (right->dep == nlp::DEP_cc && (right->lemma == "but" || right->lemma == "however" || right->lemma == "so" || right->lemma == "thus" || right->lemma == "therefore")) {
                            found_BUT_or_HOWEVER_or_SO_or_THUS_or_THEREFORE = true;
                            break;
                        }
                    }
                    
                    
                    if (!found_AND && !found_BUT_or_HOWEVER_or_SO_or_THUS_or_THEREFORE) {
                        if (/*!(child.dep == nlp::DEP_advcl && !child.left_children.empty() && */
                            
                            
                            !child.left_children.empty() && token_list.size() > child.left_children[0] &&
                            (token_list[child.left_children[0]]->lemma == "while")) {
                            
                            if (!result.value->prop_list.empty()) {
                                auto* seq_prop = prp;//ins->push_prop("SEQUENCE");
                                for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {

                                    //(*it_ch)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
                                    //token.info.prop_list.push_back(*it_ch);
                                }
                            }
                            
                            
                            break;
                        }
                    }
                    //auto* ch___ = &child;
                    //int __BP = 0;
                    
                    bool found = false;
                    bool found_when_or_after_on_left = false;
                    for (usize idx = 0; idx < token.i; idx += 1) {
                        if (token_list[idx]->lemma == "when" || token_list[idx]->lemma == "after") {
                           found_when_or_after_on_left = true;
                           break;
                        }
                    }
                    for (usize lci = 0; lci < child.left_children.size(); lci += 1) {
                        if (token_list[child.left_children[lci]]->dep == nlp::DEP_nsubj) {
                            
                            if (!result.value->prop_list.empty()) {
                                auto* seq_prop = prp;//ins->push_prop("SEQUENCE");
                                if (!prp->try_get_only_prop("END_CONDITION")) {
                                    for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                        if ((!(prp->type_str == "RESPONSE") || found_when_or_after_on_left)) {
                                            (*it_ch)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
                                            //token.info.prop_list.push_back(*it_ch);
                                            prp->push_prop("SEQUENCE_SIMULTANEOUS", *it_ch);
                                        }
                                    }
                                }
                            }
                            
                            found = true;
                            break;
                        } else{
                            if (!result.value->prop_list.empty()) {
                                auto* seq_prop = prp;
                                if (!prp->try_get_only_prop("END_CONDITION")) {
                                    for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                        
                                        if ((!(prp->type_str == "RESPONSE") || found_when_or_after_on_left)) {
                                            (*it_ch)->sequence_type_str = "SEQUENCE_SIMULTANEOUS";
                                            prp->push_prop("SEQUENCE_SIMULTANEOUS", *it_ch);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (found) {
                        break;
                    } else {
                        if (!result.value->prop_list.empty()) {
                            
                            bool do_exit = false;
                            for (usize L_i = 0; L_i < child.left_children.size(); L_i += 1) {
                                auto* left = parse->token_list[child.left_children[L_i]];
                                if (left->lemma == "then") {
                                    do_exit = true;
                                    break;
                                }
                                
                            }
                            if (do_exit) {
                                break;
                            }
                            
                            if (child.dep == nlp::DEP_conj) {
                                for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {

                                    token.info.prop_list.push_back(*it_ch);

                                    
                                    
                                }
                            } else {
                                for (auto it_ch = result.value->prop_list.begin(); it_ch != result.value->prop_list.end(); ++it_ch) {
                                    prp->push_prop("ACTION", *it_ch)->kind_str = "MUST_FILL_IN_AGENT";
                                    std::vector<Speech_Property*>* sub = nullptr;
                                    if ((*it_ch)->try_get_prop("ACTION", &sub)) {
                                        for (auto sub_it = sub->begin(); sub_it != sub->end(); ++sub_it) {
                                            prp->push_prop("ACTION", *sub_it)->kind_str = "MUST_FILL_IN_AGENT";;
                                        }
                                        (*it_ch)->remove_prop("ACTION");
                                    } else {
                                        prp->push_prop("ACTION", *it_ch)->kind_str = "MUST_FILL_IN_AGENT";;
                                    }
                                    
                                }
                            }
                            
                        }
                        break;
                    }
                    
                    break;
                }
                }
                
                break;
            }
            case nlp::POS_PRON: {
                auto result = handle_pronoun(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    break;
                }
                }
                
                switch (child.tag) {
                default: {
                    UNHANDLED();
                    break;
                }
                case nlp::TAG_PRP: {
                    // unresolved nouns
                    {
                        bool treat_as_agent                 = false;
                        bool treat_as_patient               = false;
                        bool treat_as_direct_object         = false;
                        bool treat_as_indirect_object       = false;
                        bool treat_as_object_of_preposition = false;
                        mtt::String key;
                        
                        auto& ch_props = result.value->prop_list;
                        
                        switch (child.dep) {
                        case spacy_nlp::DEP_dobj:
                            DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
                            treat_as_patient = true;
                            treat_as_direct_object = true;
                            key = "DIRECT_OBJECT";
                            break;
                        case spacy_nlp::DEP_nsubj:
                            treat_as_agent = true;
                            key = "AGENT";
                            break;
                        case spacy_nlp::DEP_dative:
                            // fallthrough
                        case spacy_nlp::DEP_nsubjpass:
                            treat_as_indirect_object = true;
                            DT_print("%s unhandled\n", spacy_nlp::dep_str[token.dep].c_str());
                            key = "INDIRECT_OBJECT";
                            break;
                        case spacy_nlp::DEP_pobj:
                            treat_as_object_of_preposition = true;
                            key = "OBJECT_OF_PREPOSITION";
                            break;
                        }
                        DT_print("Verb treat noun %s as [agent : %s, patient : %s, direct_object : %s, indirect_object : %s, object_of_preposition : %s\n",
                                 child.info.prop_list.front()->label.c_str(),
                                 bool_str(treat_as_agent),
                                 bool_str(treat_as_patient),
                                 bool_str(treat_as_direct_object),
                                 bool_str(treat_as_indirect_object),
                                 bool_str(treat_as_object_of_preposition));
                        
                        if constexpr ((DT_OLD_NESTING_WITH_SELECT)) {
                            auto& semantic_prop = *prp->push_prop(key);
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                semantic_prop.push_prop("SELECT", *it);
                            }
                        } else {
                            for (auto it = child.info.prop_list.begin(); it != child.info.prop_list.end(); ++it) {
                                prp->push_prop(key, *it);
                            }
                        }
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_ADJ: {
                auto result = handle_adj(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                    if (prp->kind_str != "EXISTENTIAL" || l_nsubj_token_immediate == nullptr) {
                        auto& props = result.value->prop_list;
                        for (auto it = props.begin(); it != props.end(); ++it) {
                            prp->push_prop("PROPERTY", *it);
                        }
                    } else {
                        // treat as AUX
                        auto& p_list = result.value->prop_list;
                        // failure_case, ignore
                        if (p_list.size() == 1 && p_list[0]->tag_str == "ADJ" && p_list[0]->value.kind_string == "TEXT" && dt::magnitude_words.find(p_list[0]->value.text) != dt::magnitude_words.end()) {
                            return {Interp_Status{.type = INTERP_STATUS_ALL_INCOMPLETE}, &token.info};
                        }
                        {
                            auto& src = l_nsubj_token_immediate->info.prop_list;
                            auto& props = result.value->prop_list;
                            
                            Speech_Property* p_n = nullptr;
                            mtt::String kind = "DEFAULT";
                            if (prp->try_get_only_prop("NEGATED", &p_n)) {
                                if (p_n->value.flag == true) {
                                    //auto* p_n = prp->push_prop("PROPERTY_NEGATION");
                                    //p_n->value.kind_string = "FLAG";
                                    //p_n->value.flag = true;
                                    kind = "NEGATED";
                                    prp->remove_prop("NEGATED");
                                }
                            }
                            
                            for (auto src_it = src.begin(); src_it != src.end(); ++src_it) {
                                for (auto it = props.begin(); it != props.end(); ++it) {
                                    auto* sub_prop = ((*src_it)->push_prop("PROPERTY", *it));//->kind_str = kind;
                                    Speech_Property* sub_negated = nullptr;
                                    bool sub_negated_value = false;
                                    if (sub_prop->try_get_only_prop("NEGATED", &sub_negated)) {
                                        sub_negated_value = sub_negated->value.flag;
                                    }
                                    
                                    // negated
                                    if (sub_negated_value == true) {
                                        if (kind == "NEGATED") {
                                            sub_prop->kind_str = "DEFAULT";
                                        } else {
                                            sub_prop->kind_str = "NEGATED";
                                        }
                                    } else {
                                        sub_prop->kind_str = kind;
                                    }
                                }
                            }
                        }
                        
                    }
                    break;
                }
                }
                break;
            }
            case nlp::POS_NUM: {
                // TODO
                // TODO handle separately
                bool is_found = false;
                Speech_Property* p_ref = nullptr;
                if (!token.left_children.empty()) {
                    for (usize i = 0; i < token.left_children.size(); i += 1) {
                        Speech_Token* next_child = parse->token_list[token.left_children[i]];
                        if (next_child->dep == spacy_nlp::DEP_nsubj) {
                            p_ref = next_child->prop_ref;
                            is_found = true;
                            break;
                        }
                    }
                }
                // FIXME: p_ref can be null, but should check why this case happens
                if (!is_found || p_ref == nullptr) {
                    break;
                }
                
                Speech_Property* count_prop = nullptr;
                if (!p_ref->try_get_only_prop("COUNT", &count_prop)) {
                    count_prop = p_ref->push_prop("COUNT");
                }
                
                count_prop->type_str = "VALUE";
                count_prop->value.kind_string = "NUMERIC";
                
                mtt::String text = child.lemma;
                replace_patterns_in_place(text, string_replacements);
                
                cstring num_str = text.c_str();
                char* end;
                auto result = strtod(num_str, &end);
                usize len = strlen(num_str);
                if ((end != num_str + len) || (result == 0 && num_str == end) ||
                    result == HUGE_VAL || result == -HUGE_VAL ||
                    result == HUGE_VALF || result == -HUGE_VALF ||
                    result == HUGE_VALL || result == -HUGE_VALL) {
                    
                    count_prop->value.numeric = 1.0;
                    
                    {
                        dt::Dynamic_Array<mtt::String> words;
                        words.push_back(child.lemma);
                        if (!child.left_children.empty()) {
                            Speech_Token* next_child = parse->token_list[child.left_children.back()];
                            do {
                                if (next_child->dep == nlp::DEP_compound && next_child->pos == nlp::POS_NUM) {
                                    words.push_back(next_child->lemma);
                                    if (next_child->left_children.empty()) {
                                        break;
                                    }
                                    next_child = parse->token_list[next_child->left_children.back()];
                                } else {
                                    break;
                                }
                            } while (1);
                        }
                        
                        for (usize i = 0; i < words.size(); i += 1) {
                            replace_patterns_in_place(words[i], string_replacements);
                        }
                        std::reverse(words.begin(), words.end());
                        
                        isize value = 1;
                        if (!dt::text2num(words, &value)) {
                            MTT_error("%s", "Number invalid\n");
                        }
                        
                        count_prop->value.numeric = value;
                    }
                } else {
                    count_prop->value.numeric = result;
                }
                if (!child.right_children.empty()) {
                    Speech_Token& r_child = *parse->token_list[child.right_children[0]];
                    if (r_child.dep == spacy_nlp::DEP_quantmod && r_child.text == "times") {
                        count_prop->kind_str = "multiplier";
                    }
                }
                break;
            }
            }
            
            
        }
        //
        break;
    }
    default:
        UNHANDLED();
        break;
    }
    
    //    DT_print("(Action_Info){\n");
    //    DT_print("action: ");
    //    for (usize i = 0; i < info.keys.size() - 1; i += 1) {
    //        DT_print("%s_", info.keys[i].c_str());
    //    }
    //    if (info.keys.size() > 0) {
    //        DT_print("%s\n", info.keys.back().c_str());
    //    }
    //    DT_print("agents: {\n\t\t");
    //    for (usize i = 0; i < info.agents.size(); i += 1) {
    //        Token_print(parse, info.agents[i]->token);
    //    }
    //    DT_print("objects: {\n\t\t");
    //    for (usize i = 0; i < info.objects.size(); i += 1) {
    //        Token_print(parse, info.objects[i]->token);
    //    }
    //    DT_print("}\n");
    
    int BREAK_ROOT_END = 0;
    
    args->dt->sys_eval_ctx.verb_list.emplace_back(&token.info);
    
    if (token.info.action.should_treat_as_trigger_response) {
        args->dt->sys_eval_ctx.rule_list.emplace_back(&token.info);
        info.rule_ref = &token.info;
        
        //card.info->rule.
    } else if (token.info.action.should_treat_as_trigger) {
        // TODO: ...
    } else {
        // TODO: ...
    }
    
    {
        mtt::String search_string = "";
        mtt::join(info.keys, '_', &search_string);
        if (is_stateful_verb(search_string)) {
            info.verb_class = VERB_CLASS_STATE_CHANGE;
        } else {
            info.verb_class = VERB_CLASS_ACTION;
        }
    }
    
    
    std::vector<Speech_Property*>* actions = nullptr;
    std::vector<Speech_Property*>* agents = nullptr;
    if (prp->try_get_prop("AGENT", &agents)) {
        if (prp->try_get_prop("ACTION", &actions)) {
//            for (auto it_action = actions->begin(); it_action != actions->end(); ++it_action) {
////                if ((*it_action)->kind_str == "MUST_FILL_IN_AGENT") {
////                    (*it_action)->kind_str = "";
////                    (*it_action)->push_prop_list("AGENT", *agents);
////                }
//            }
        }
    }

    if (prop_swap.empty()) {
        if (token.info.prop_list.empty() || *token.info.prop_list.begin() != prp) {
            token.info.prop_list.insert(token.info.prop_list.begin(), prp);
        }
    } else {
        token.info.prop_list.insert(token.info.prop_list.begin(), prop_swap.begin(), prop_swap.end());
    }
    
    
    DT_scope_close();
    return {Interp_Status{.type = INTERP_STATUS_OK}, &token.info};
}

Interp_Result handle_IN(dt::Lang_Eval_Args* args, dt::Speech_Token& token)
{
    DT_print("%s, lemma=[%s] pos=[%s] dep=[%s]\n", token.text.c_str(), token.lemma.c_str(), dt::spacy_nlp::pos_str[token.pos].c_str(), dt::spacy_nlp::dep_str[token.dep].c_str());
    namespace nlp = dt::spacy_nlp;
    
    auto* dt    = args->dt;
    auto* parse = args->parse;
    DT_scope_open();
    
    token.info.type = WORD_INFO_TYPE_PREPOSITION;
    token.info.token = &token;
    auto& info = token.info.preposition_info;
    info.token = &token;
    
    auto& left_children  = token.left_children;
    auto& right_children = token.right_children;
        
    switch (token.dep) {
    default: {
        UNHANDLED();
        break;
    }
    case spacy_nlp::DEP_agent: {
        usize L = 0;
        for (;
             L < left_children.size();
             L += 1) {
            Speech_Token& child = *parse->token_list[left_children[L]];
            if (child.is_blocked) {
                continue;
            }
            
            UNHANDLED();
        }
        
        usize R = 0;
        for (;
             R < right_children.size();
             R += 1) {
            Speech_Token& child = *parse->token_list[right_children[R]];
            if (child.is_blocked) {
                continue;
            }
            
            switch (child.pos) {
            default: {
                UNHANDLED();
                break;
            }
            case nlp::POS_PRON: {
                
                auto result = handle_pronoun(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    

                    FOR_ITER(ch, result.value->prop_list, ++ch) {
                        token.info.prop_list.push_back(*ch);
                    }

                    break;
                }
                }
                
                break;
            }
            case nlp::POS_PROPN:
                // fallthrough
            case nlp::POS_NOUN: {
                auto result = handle_noun(args, child);
                
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    
                    

                    FOR_ITER(ch, result.value->prop_list, ++ch) {
                        token.info.prop_list.push_back(*ch);
                    }

                    
                    // TODO: //
                    if (result.value->nominal.is_object_of_preposition) {
                        info.objects_of_preposition.push_back(&result.value->nominal);
                    } else {
                        UNHANDLED();
                        break;
                    }
                    
                    
                    break;
                }
                }
            }
            }
        }
        
        
        break;
    }
    case spacy_nlp::DEP_dative:
        // fallthrough for now
    case spacy_nlp::DEP_prep: {
        
        auto& prop = *Speech_Property::make();
        token.info.prop_list.push_back(&prop);
        prop.label = token.lemma;
        prop.type_str = prop.label;
        prop.token = &token;
        Speech_Token_set_prop(&token, &prop);
        
        if (left_children.size() > 0) {
            MTT_error("%s", "Does not handle left children of prepositions yet. Ignored!\n");
        }
        usize L = 0;
        for (;
             L < left_children.size();
             L += 1) {
            Speech_Token& child = *parse->token_list[left_children[L]];
            if (child.is_blocked) {
                continue;
            }

        }
        
        usize R = 0;
        for (;
             R < right_children.size();
             R += 1) {
            Speech_Token& child = *parse->token_list[right_children[R]];
            if (child.is_blocked) {
                continue;
            }
            
            switch (child.pos) {
            default: {
                UNHANDLED();
                break;
            }
            case nlp::POS_PRON: {
                
                auto result = handle_pronoun(args, child);
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    

                    FOR_ITER(ch, result.value->prop_list, ++ch) {
                        prop.push_prop("OBJECT", *ch);
                    }

                    break;
                }
                }
                
                break;
            }
            case nlp::POS_PROPN:
                // fallthrough
            case nlp::POS_NOUN: {
                auto result = handle_noun(args, child);
                
                switch (result.status.type) {
                default: {
                    UNHANDLED();
                    break;
                }
                case INTERP_STATUS_OK: {
                    

                    FOR_ITER(ch, result.value->prop_list, ++ch) {
                        if ((*ch)->tag_str == "TIME") {
                            prop.push_prop("TIME", *ch);
                            //(*ch)->print();
                        } else {
                            prop.push_prop("OBJECT", *ch);
//                            if ((*ch)->annotation == "DO_NOT_REQUIRE_SELECTION") {
//                                prop.annotation = "DO_NOT_REQUIRE_SELECTION";
//                            }
                            if ((*ch)->ignore_selections) {
                                prop.ignore_selections = true;
                            }
                        }
                    }

                    
                    // TODO: //
                    if (result.value->nominal.is_object_of_preposition) {
                        info.objects_of_preposition.push_back(&result.value->nominal);
                    } else {
                        UNHANDLED();
                        break;
                    }
                    
                    
                    break;
                }
                }
                break;
            }
            case nlp::POS_NUM: {
                auto* obj = Speech_Property::make();
                prop.push_prop("OBJECT", obj);
                obj->ignore_selections = true;
                obj->kind_str = "VALUE_TYPE";
                
                Speech_Property* count_prop = nullptr;
                //if (!obj.try_get_only_prop("COUNT", &count_prop)) {
                count_prop = obj->push_prop("COUNT");
                //}
                
                count_prop->type_str = "VALUE";
                count_prop->value.kind_string = "NUMERIC";
                
                mtt::String text = child.lemma;
                replace_patterns_in_place(text, string_replacements);
                
                cstring num_str = text.c_str();
                char* end;
                auto result = strtod(num_str, &end);
                usize len = strlen(num_str);
                if ((end != num_str + len) || (result == 0 && num_str == end) ||
                    result == HUGE_VAL || result == -HUGE_VAL ||
                    result == HUGE_VALF || result == -HUGE_VALF ||
                    result == HUGE_VALL || result == -HUGE_VALL) {
                    
                    count_prop->value.numeric = 1.0;
                    
                    {
                        dt::Dynamic_Array<mtt::String> words;
                        words.push_back(child.lemma);
                        if (!child.left_children.empty()) {
                            Speech_Token* next_child = parse->token_list[child.left_children.back()];
                            do {
                                if (next_child->dep == nlp::DEP_compound && next_child->pos == nlp::POS_NUM) {
                                    words.push_back(next_child->lemma);
                                    if (next_child->left_children.empty()) {
                                        break;
                                    }
                                    next_child = parse->token_list[next_child->left_children.back()];
                                } else {
                                    break;
                                }
                            } while (1);
                        }
                        
                        for (usize i = 0; i < words.size(); i += 1) {
                            replace_patterns_in_place(words[i], string_replacements);
                        }
                        std::reverse(words.begin(), words.end());
                        
                        isize value = 1;
                        if (!dt::text2num(words, &value)) {
                            MTT_error("%s", "Number invalid\n");
                        }
                        
                        count_prop->value.numeric = value;
                    }
                } else {
                    count_prop->value.numeric = result;
                }
                break;
            }
            }
        }
        
        
        break;
    }
    }
    
    DT_scope_close();
    return {Interp_Status{.type = INTERP_STATUS_OK}, &token.info};
}





inline static const mtt::String LANGUAGE_ENGLISH = "en";
inline static const mtt::String LANGUAGE_DEUTSCH = "de";

[[deprecated]]
inline static const mtt::String EVAL_ROOT_LABEL    = "__EVAL_ROOT__";
[[deprecated]]
inline static const mtt::String PROGRAM_ROOT_LABEL = "__Program_ROOT__";

[[deprecated]]
void generate_root_is_verb(dt::Lang_Eval_Args* args, Word_Info& base_info)
{
    //    Verbal_Info& info = base_info.action;
    //
    //    auto* dt = args->dt;
    //    auto* parse = args->parse;
    //    System_Evaluation_Context& sys_ctx = dt->sys_eval_ctx;
    //    Evaluation_Context* eval_ctx = nullptr;
    //    mtt::World* world = dt->mtt;
    //
    //    sys_ctx.create_evaluation_context(&eval_ctx);
    //    eval_ctx->init();
    //
    //    mtt::Thing* __root_thing__ = nullptr;
    //    mtt::Thing_ID __root_id__ = mtt::Thing_make(world, mtt::ARCHETYPE_GROUP, &__root_thing__);
    //    __root_thing__->label = dt::EVAL_ROOT_LABEL;
    //    mtt::thing_group_set_active_state(__root_thing__, false);
    //    Program_Sequence_Node* root_prog_seq_node = dt::Program_Sequence_Node::get(MTT_Tree_root(&eval_ctx->programs.tree));
    //    root_prog_seq_node->id = __root_id__;
    //    {
    //        mtt::Any* record;
    //        mtt::map_set(&root_prog_seq_node->identifiers.map, __root_thing__->label, mtt::Any::from_Thing_ID(__root_id__), &record);
    //    }
    //
    //    sys_ctx.set_active_context(eval_ctx);
    //
    //    //std::cout << Program_Sequence_Node::get(MTT_Tree_root(&eval_ctx->program.tree))->label << std::endl;
    //    auto root = search(MTT_Tree_root(&dt->sys_tree), LANGUAGE_ENGLISH);
    //
    //    // rest needs to happen per action
    //    auto curr = root;
    //    auto prev = curr;
    //    for (usize k = 0; k < info.keys.size() &&
    //         curr.result != nullptr; k += 1) {
    //
    //        prev = curr;
    //        curr = prev.search(info.keys[k]);
    //    }
    //
    //    if (curr.result == nullptr) {
    //        MTT_error("Whoops! Behavior not Found: [%s]\n");
    //        return;
    //    }
    //
    //    auto* block = static_cast<dt::Behavior_Block*>(curr.result->data);
    //    MTT_print("Behavior Found: [%s]\n", block->label.c_str());
    //    Block_print(block);
    //    if (block->type != BLOCK_TYPE_PROCEDURE) {
    //        MTT_error("EROR: does not handle non-procedure behaviors yet");
    //        return;
    //    }
    //
    //    MTT_print("Procedure\n");
    //
    //    auto& proc_block = *static_cast<Procedure_Block*>(block->derived);
    //
    //    // MARK: find entities
    //    dt::Recorder& recorder = dt->recorder;
    //    dt::Selection_Recording& rec_selection = recorder.selection_recording;
    //
    //    auto check_selections = [](dt::Selection_Recording& rec_selection, usize required_count) -> mtt::Result<Selection_Recording_State_Queue::iterator, usize> {
    //        if (rec_selection.idx_playhead + required_count > rec_selection.selections.size()) {
    //            return {0};
    //        }
    //        return {required_count, rec_selection.selections.begin() + rec_selection.idx_playhead};
    //    };
    //
    //
    //
    //    auto seek_selections_from_end = [](dt::Selection_Recording& rec_selection, usize count_from_end) -> mtt::Result<Selection_Recording_State_Queue::iterator, usize> {
    //        count_from_end = m::min<usize>(count_from_end, rec_selection.selections.size());
    //
    //        rec_selection.idx_playhead = rec_selection.selections.size() - count_from_end;
    //
    //        return {count_from_end, rec_selection.selections.begin() + rec_selection.idx_playhead};
    //    };
    //
    //    auto seek_selections_from_beginning = [](dt::Selection_Recording& rec_selection, usize count_from_start) -> mtt::Result<Selection_Recording_State_Queue::iterator, usize> {
    //        rec_selection.idx_playhead = 0;
    //
    //        if (rec_selection.idx_playhead >= rec_selection.selections.size()) {
    //            return {0};
    //        }
    //
    //        return {count_from_start, rec_selection.selections.begin()};
    //    };
    //
    //    auto get_selection = [](dt::Selection_Recording& rec_selection) -> mtt::Result<Selection_Recording_State_Queue::iterator> {
    //
    //        if (rec_selection.idx_playhead >= rec_selection.selections.size()) {
    //            return {false};
    //        }
    //
    //        auto it = rec_selection.selections.begin() + rec_selection.idx_playhead;
    //
    //
    //
    //        return {true, it};
    //
    //    };
    //    auto advance_selections = [](dt::Selection_Recording& rec_selection) -> bool {
    //        if (rec_selection.idx_playhead == rec_selection.selections.size()) {
    //            return false;
    //        }
    //
    //        rec_selection.idx_playhead += 1;
    //
    //        return true;
    //    };
    //
    //    usize candidate_required_entities_count = info.agents.size() + info.objects.size();
    //    usize match_count = 0;
    //
    //
    //    // MARK: check for types
    //    for (usize i = 0; i < info.agents.size(); i += 1) {
    //        mtt::String& ident = info.agents[i]->token->lemma;
    //        flecs::entity entity_type = dt->mtt->ecs_world.lookup(ident.c_str());
    //        if (entity_type == flecs::entity::null()) {
    //
    //            entity_type = flecs::entity(dt->mtt->ecs_world, ident.c_str());
    //            entity_type.add<mtt::Render_Data>();
    //            mtt::Render_Data* rd = entity_type.get_mut<mtt::Render_Data>();
    //
    //
    //        } else {
    //
    //            mtt::Render_Data* render_data = entity_type.get_mut<mtt::Render_Data>();
    //        }
    //    }
    //    for (usize i = 0; i < info.objects.size(); i += 1) {
    //        mtt::String& ident = info.objects[i]->token->lemma;
    //        flecs::entity entity_type = dt->mtt->ecs_world.lookup(ident.c_str());
    //        if (entity_type == flecs::entity::null()) {
    //
    //            entity_type = flecs::entity(dt->mtt->ecs_world, ident.c_str());
    //            entity_type.add<mtt::Render_Data>();
    //            mtt::Render_Data* rd = entity_type.get_mut<mtt::Render_Data>();
    //
    //        } else {
    //
    //            mtt::Render_Data* render_data = entity_type.get_mut<mtt::Render_Data>();
    //
    //
    //            flecs::query<> q(dt->mtt->ecs_world, entity_type.name().c_str());
    //            q.each([](flecs::entity ent) {
    //
    //            });
    //
    //
    //        }
    //    }
    //
    //    //auto result = seek_selections_from_beginning(rec_selection, candidate_required_entities_count);
    //    auto result = check_selections(rec_selection, candidate_required_entities_count);
    //    int ENTITY_SEARCH_BREAKPOINT = 0;
    //
    //
    //    if (result.status != candidate_required_entities_count
    //        ) {
    //
    //        MTT_print("WARNING: required entities mismatch with available recently selected entities - TODO implement other tests\n");
    //
    //        return;
    //    }
    //
    //    for (usize i_thing = 0; i_thing < info.agents.size(); i_thing += 1) {
    //        mtt::Thing_ID id = 0;
    //        bool more_to_check = true;
    //        bool found = false;
    //        while (more_to_check && !found) {
    //            auto out = get_selection(rec_selection);
    //            if (!out.status) {
    //                more_to_check = false;
    //                MTT_error("SOMETHING WENT WRONG ... not supporting background selections yet so maybe ran out of candiate things %s, %d\n", __PRETTY_FUNCTION__, __LINE__);
    //                continue;
    //            }
    //
    //            auto cand_id = out.value->ID;
    //            mtt::Thing* thing = nullptr;
    //            if (dt->mtt->Thing_try_get(cand_id, &thing)) {
    //                found = true;
    //                id = cand_id;
    //                match_count += 1;
    //            }
    //            more_to_check = advance_selections(rec_selection);
    //        }
    //        if (found) {
    //            info.agents[i_thing]->thing_id = id;
    //            mtt::Thing_print(dt->mtt, id);
    //        } else {
    //            info.agents[i_thing]->thing_id = 0;
    //            MTT_error("ERROR: aborting\n");
    //            return;
    //        }
    //    }
    //    for (usize i_thing = 0; i_thing < info.objects.size(); i_thing += 1) {
    //
    //        mtt::Thing* thing = nullptr;
    //        mtt::Thing_ID id = 0;
    //        bool more_to_check = true;
    //        bool found = false;
    //        while (more_to_check && !found) {
    //            auto out = get_selection(rec_selection);
    //            if (!out.status) {
    //                more_to_check = false;
    //                MTT_error("SOMETHING WENT WRONG ... not supporting background selections yet so maybe ran out of candiate things %s, %d\n", __PRETTY_FUNCTION__, __LINE__);
    //                continue;
    //            }
    //
    //            auto cand_id = out.value->ID;
    //
    //            if (dt->mtt->Thing_try_get(cand_id, &thing)) {
    //                found = true;
    //                id = cand_id;
    //                match_count += 1;
    //            }
    //
    //            more_to_check = advance_selections(rec_selection);
    //        }
    //        if (found) {
    //            info.objects[i_thing]->thing_id = id;
    //            mtt::Thing_print(dt->mtt, id);
    //        } else {
    //            info.objects[i_thing]->thing_id = 0;
    //            MTT_error("ERROR: aborting\n");
    //            return;
    //        }
    //    }
    //
    //    if (match_count != candidate_required_entities_count) {
    //        MTT_error("ERROR: NOT YET HANDLED CASE IN WHICH NOT ALL FOUND! :(\n");
    //        return;
    //    }
    //
    //    MTT_print("ALL FOUND!\n");
    ////    for (usize t = 0; t < info.agents.size(); t += 1) {
    ////        mtt::Thing* thing = nullptr;
    ////        if (info.agents[t]->thing == dt->scn_ctx.thing_selected_with_pen) {
    ////            dt->scn_ctx.thing_selected_with_pen = mtt::Thing_ID_INVALID;
    ////        }
    ////        if (world->Thing_try_get(info.agents[t]->thing, &thing)) {
    ////            if (auto select_it = dt->scn_ctx.selected_things.find(thing->id); select_it != dt->scn_ctx.selected_things.end()) {
    ////
    ////                dt->scn_ctx.selected_things.erase(select_it);
    ////                continue;
    ////            }
    ////        }
    ////    }
    ////    for (usize t = 0; t < info.objects.size(); t += 1) {
    ////        mtt::Thing* thing = nullptr;
    ////        if (info.objects[t]->thing == dt->scn_ctx.thing_selected_with_pen) {
    ////            dt->scn_ctx.thing_selected_with_pen = mtt::Thing_ID_INVALID;
    ////        }
    ////        if (world->Thing_try_get(info.objects[t]->thing, &thing)) {
    ////            if (auto select_it = dt->scn_ctx.selected_things.find(thing->id); select_it != dt->scn_ctx.selected_things.end()) {
    ////
    ////                dt->scn_ctx.selected_things.erase(select_it);
    ////                continue;
    ////            }
    ////        }
    ////    }
    //
    //    // MARK: objects found
    //
    //    // MARK: begin call
    //    // TODO: will need multiple coordinated calls
    //    eval_ctx->calls.push_back(Call());
    //    Call call = eval_ctx->calls.back();
    //
    //    {
    //        usize str_len = info.keys.size() - 1; // for '_' separators
    //        for (usize k_idx = 0; k_idx < info.keys.size(); k_idx += 1) {
    //            str_len += info.keys[k_idx].size();
    //        }
    //
    //        mtt::String key_concat;
    //        key_concat.reserve(str_len);
    //
    //        key_concat.append(info.keys[0]);
    //        for (usize k_idx = 1; k_idx < info.keys.size(); k_idx += 1) {
    //            key_concat.append("_");
    //            key_concat.append(info.keys[k_idx]);
    //        }
    //
    //        call.label = key_concat;
    //        call.info = &base_info;
    //
    //        {
    //            {
    //                auto& list = call.params["Source_List"];
    //                list.param.resize(info.agents.size());
    //                list.type_name = "Source_List";
    //                list.identifier = "source";
    //                for (usize a_i = 0; a_i < info.agents.size(); a_i += 1) {
    //                    list.param[a_i].type = mtt::MTT_THING;
    //                    list.param[a_i].thing_id = info.agents[a_i]->thing_id;
    //                }
    //            }
    //            {
    //                auto& list = call.params["Object_List"];
    //                list.identifier = "object";
    //                list.type_name = "Object_List";
    //                list.param.resize(info.objects.size());
    //                for (usize o_i = 0; o_i < info.objects.size(); o_i += 1) {
    //                    // TODO need to support locations as well as objects
    //                    list.param[o_i].type = mtt::MTT_THING;
    //                    list.param[o_i].thing_id = info.objects[o_i]->thing_id;
    //
    //                }
    //            }
    //            {
    //                auto& list = call.params["modifier"];
    //            }
    //        }
    //    }
    //    // MARK: generation
    //    {
    //        dt::Program_Sequence_Node* program = eval_ctx->programs.append_new_program();
    //        ASSERT_MSG(program->node != nullptr, "What\n");
    //        mtt::Thing* program_thing = nullptr;
    //        mtt::Thing_ID program_id = mtt::Thing_make(dt->mtt, mtt::ARCHETYPE_GROUP, &program_thing);
    //        program_thing->label = dt::PROGRAM_ROOT_LABEL;
    //        {
    //            dt::Program_Sequence_Node* program_parent = program->parent();
    //            ASSERT_MSG(program_parent != nullptr, "parent should not be null!\n");
    //        }
    //        {
    //            program->set_id(program_id);
    //            mtt::thing_group_set_active_state(program_thing, false);
    //
    //            mtt::Thing* root = nullptr;
    //            if (!program_thing->world->Thing_try_get(__root_id__, &root)) {
    //                ASSERT_MSG(false, "SHOULD EXIST!\n");
    //            }
    //            mtt::add_to_group(program_thing->world, root, program_thing);
    //
    //            mtt::Any* record;
    //            mtt::map_set(&program->identifiers.map, program_thing->label, mtt::Any::from_Thing_ID(program_id), &record);
    //        }
    //        program->init();
    //
    //
    //
    //
    //        // MARK: visit parameter list and map call args to identifiers in the parameter list
    //        Procedure_Parameter_List_Block& param_list = *(proc_block.get_param_list_block());
    //
    //        {
    //            auto* param_entry = param_list.begin();
    //            mtt::String& prefix = param_list.base.label;
    //            while (param_entry != param_list.end()) {
    //                dt::Block_print(static_cast<dt::Behavior_Block*>(&param_entry->base));
    //                // search by the typename
    //                // since args are grouped by type
    //                mtt::String& type_name = param_entry->type_name;
    //
    //                Call::Param* param_record = nullptr;
    //                if (!mtt::map_try_get(&call.params, type_name, &param_record)) {
    //                    // check if the parameter is optional
    //
    //                    if (param_entry->requirement == dt::PARAM_REQUIRED_YES) {
    //                        // FIXME: support missing required arguments
    //                        ASSERT_MSG(false, "ERROR: does not handle missing required arguments yet!\n");
    //                    } else {
    //                        MTT_print("Missing optional param type_name=[%s] label=[%s]\n", type_name.c_str(), param_entry->base.label.c_str());
    //                        // Leave empty?
    //                    }
    //                } else {
    //
    //                    Dynamic_Array<mtt::Any>* narr = new Dynamic_Array<mtt::Any>(param_record->param);
    //                    mtt::Any a;
    //                    a.type = mtt::MTT_LIST;
    //                    a.contained_type = mtt::MTT_ANY;
    //                    a.List = (uintptr)narr;
    //
    //                    MTT_print("Setting param type_name=[%s] key=[%s]\n", type_name.c_str(), (prefix + param_entry->base.label).c_str());
    //                    mtt::map_set(&program->identifiers.map, (prefix + ":" + param_entry->base.label), a);
    //                }
    //                param_list.advance(&param_entry);
    //            }
    //        }
    //
    //        // MARK: body
    //        {
    //            struct Tree_Arg {
    //                Evaluation_Context* ctx;
    //                Program_Sequence_Node* prog_node;
    //                mtt::Thing_ID program_root_id;
    //                mtt::World* world;
    //            } tree_arg = {eval_ctx, program, program_thing->id, world};
    //
    //
    //
    //            // MARK: Pre-instantiate entities
    //            for (Behavior_Block* it_body = proc_block.begin_body(); it_body != proc_block.end_body(); Behavior_Block::advance(&it_body)) {
    //
    //                MTT_Tree_postorder_traversal(&dt->sys_tree, it_body->node, [](MTT_Tree* tree, MTT_Tree_Node* node, void* data) {
    //
    //                    Tree_Arg* arg = static_cast<Tree_Arg*>(data);
    //                    Evaluation_Context* ctx = arg->ctx;
    //                    Program_Sequence_Node* prog = arg->prog_node;
    //
    //                    dt::Behavior_Block* base = dt::Block_get(node);
    //
    //                    mtt::World* world = dt::DrawTalk::ctx()->mtt;
    //
    //                    switch (base->type) {
    //
    //                    case BLOCK_TYPE_NONE: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_ROOT: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_SCOPE: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_PROCEDURE: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_PROCEDURE_PARAMETER_LIST: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_PROCEDURE_PARAMETER: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_GOTO: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_GENERATE: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_VECTOR: {
    //                        auto* block = base->get_concrete<Vector_Block>();
    //
    //                        mtt::Thing* thing = nullptr;
    //
    //
    //                        mtt::Thing_ID id = mtt::Thing_make(world, mtt::ARCHETYPE_VECTOR, &thing);
    //
    //                        mtt::Thing_set_label(thing, base->label);
    //
    //                        mtt::map_set(&prog->identifiers.map, base->label, mtt::Any::from_Thing_ID(id));
    //
    //                        mtt::Thing* program_root = nullptr;
    //                        if (!arg->world->Thing_try_get(arg->program_root_id, &program_root)) {
    //                            ASSERT_MSG(false, "could not find the root!\n");
    //                        }
    //
    //                        mtt::add_to_group(world, program_root, thing);
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_POINT: {
    //                        auto* block = base->get_concrete<Point_Block>();
    //
    //                        mtt::Thing* thing = nullptr;
    //
    //                        mtt::Thing_ID id = mtt::Thing_make(world, mtt::ARCHETYPE_VECTOR, &thing);
    //
    //                        thing->label = base->label;
    //
    //                        mtt::map_set(&prog->identifiers.map, base->label, mtt::Any::from_Thing_ID(id));
    //
    //                        mtt::Thing* program_root = nullptr;
    //                        if (!arg->world->Thing_try_get(arg->program_root_id, &program_root)) {
    //                            ASSERT_MSG(false, "could not find the root!\n");
    //                        }
    //
    //                        mtt::add_to_group(world, program_root, thing);
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_MOVE: {
    //                        auto* block = base->get_concrete<Mover_Block>();
    //
    //                        mtt::Thing* thing = nullptr;
    //
    //                        mtt::Thing_ID id = mtt::Thing_make(world, mtt::ARCHETYPE_MOVER, &thing);
    //
    //                        thing->label = base->label;
    //
    //
    //                        mtt::map_set(&prog->identifiers.map, base->label, mtt::Any::from_Thing_ID(id));
    //
    //                        mtt::Thing* program_root = nullptr;
    //                        if (!arg->world->Thing_try_get(arg->program_root_id, &program_root)) {
    //                            ASSERT_MSG(false, "could not find the root!\n");
    //                        }
    //
    //                        mtt::add_to_group(world, program_root, thing);
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_FOLLOW: {
    //                        auto* block = base->get_concrete<Follower_Block>();
    //
    //                        mtt::Thing* thing = nullptr;
    //
    //                        mtt::Thing_ID id = mtt::Thing_make(world, mtt::ARCHETYPE_FOLLOWER, &thing);
    //                        thing->label = base->label;
    //
    //
    //                        mtt::map_set(&prog->identifiers.map, base->label, mtt::Any::from_Thing_ID(id));
    //
    //                        mtt::Thing* program_root = nullptr;
    //                        if (!arg->world->Thing_try_get(arg->program_root_id, &program_root)) {
    //                            ASSERT_MSG(false, "could not find the root!\n");
    //                        }
    //
    //                        mtt::add_to_group(world, program_root, thing);
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_WHEN: {
    //                        auto* block = base->get_concrete<When_Block>();
    //
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_CURVE: {
    //                        auto* block = base->get_concrete<Curve_Block>();
    //
    //                        mtt::Thing* thing = nullptr;
    //
    //
    //                        mtt::Thing_ID id = mtt::Thing_make(world, mtt::ARCHETYPE_CONTROL_CURVE, &thing);
    //                        thing->label = base->label;
    //
    //                        mtt::map_set(&prog->identifiers.map, base->label, mtt::Any::from_Thing_ID(id));
    //
    //                        mtt::Thing* program_root = nullptr;
    //                        if (!arg->world->Thing_try_get(arg->program_root_id, &program_root)) {
    //                            ASSERT_MSG(false, "could not find the root!\n");
    //                        }
    //
    //                        mtt::add_to_group(world, program_root, thing);
    //
    //
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_NAMESPACE: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_OPTION: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_NUMBER: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_VALUE: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_CONSTRUCTOR: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_PROCEDURE_CALL: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_PROPERTY_SET: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_EXPRESSION: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_CONNECTION: {
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_JUMP: {
    //                        auto* block = base->get_concrete<Jump_Block>();
    //
    //                        mtt::Thing* thing = nullptr;
    //
    //                        mtt::Thing_ID id = mtt::Thing_make(world, mtt::ARCHETYPE_JUMP, &thing);
    //
    //                        thing->label = base->label;
    //
    //
    //                        mtt::map_set(&prog->identifiers.map, base->label, mtt::Any::from_Thing_ID(id));
    //
    //                        mtt::Thing* program_root = nullptr;
    //                        if (!arg->world->Thing_try_get(arg->program_root_id, &program_root)) {
    //                            ASSERT_MSG(false, "could not find the root!\n");
    //                        }
    //
    //                        mtt::add_to_group(world, program_root, thing);
    //
    //
    //                        break;
    //                    }
    //                    case BLOCK_TYPE_END: {
    //                        auto* block = base->get_concrete<End_Block>();
    //
    //                        mtt::Thing* thing = nullptr;
    //
    //                        mtt::Thing_ID id = mtt::Thing_make(world, mtt::ARCHETYPE_END, &thing);
    //
    //                        thing->label = base->label;
    //
    //                        mtt::map_set(&prog->identifiers.map, base->label, mtt::Any::from_Thing_ID(id));
    //
    //                        mtt::Thing* program_root = nullptr;
    //                        if (!arg->world->Thing_try_get(arg->program_root_id, &program_root)) {
    //                            ASSERT_MSG(false, "could not find the root!\n");
    //                        }
    //
    //                        mtt::add_to_group(world, program_root, thing);
    //
    //                        break;
    //                    }
    //                    default:
    //                        //ASSERT_MSG(false, "Unhandled! %d\n", __LINE__);
    //                        break;
    //                    }
    //
    //                }, (void*)&tree_arg);
    //            }
    //
    //            mtt::World* world = dt->mtt;
    //
    //
    //            eval_ctx->stack.clear();
    //            eval_ctx->instructions.clear();
    //
    //            Behavior_Block* it_body = proc_block.begin_body();
    //            for (;it_body != proc_block.end_body(); Behavior_Block::advance(&it_body)) {
    //                eval_ctx->it_body = it_body;
    //                // TODO: traverse body
    //                switch (it_body->type) {
    //                case BLOCK_TYPE_END: {
    //
    //                    auto& end = *it_body->get_concrete<dt::End_Block>();
    //                    const mtt::String& label = it_body->label;
    //                    mtt::Thing* end_thing = nullptr;
    //                    mtt::Any* data;
    //                    mtt::map_get(&program->identifiers.map, label, &data);
    //                    world->Thing_get(data->thing_id, &end_thing);
    //
    //                    // assume Boolean for now
    //                    *mtt::access<bool>(end_thing, "expected_end_value") = end.expected_value.Boolean;
    //
    //
    //                    if (end.sources.size() == 1) {
    //                        auto& keys = end.sources[0];
    //
    //
    //
    //
    //                        switch (end.kind) {
    //                        case END_KIND_OUT_PORT: {
    //                            ASSERT_MSG(keys.size() >= 2, "not enough args\n");
    //                            mtt::String key_path = "";
    //                            for (usize i = 1; i < keys.size() - 2; i += 1) {
    //                                key_path += keys[i] + ":";
    //                            }
    //                            key_path += keys[keys.size() - 2];
    //                            mtt::String port_name = keys.back();
    //
    //                            mtt::Any* data_ = nullptr;
    //                            if (!mtt::map_try_get(&program->identifiers.map, key_path, &data_)) {
    //                                MTT_error("ERROR: something is missing\n");
    //                                break;
    //                            }
    //
    //                            mtt::Thing* source_thing = nullptr;
    //                            ASSERT_MSG(world->Thing_try_get(data_->thing_id, &source_thing), "should exist!\n");
    //
    //                            ASSERT_MSG(mtt::add_connection(world, source_thing, port_name, end_thing, "is_done"), "Port should exist!\n");
    //
    //
    //                            auto* callback = mtt::access<mtt::Procedure>(end_thing, "on_end");
    //                            *callback = {};
    //
    //                            dt::Eval_End_Handle* end_event_handle = static_cast<Eval_End_Handle*>(world->allocator.do_allocate(sizeof(Eval_End_Handle)));
    //
    //                            end_event_handle->primary_id       = eval_ctx->id;
    //                            end_event_handle->secondary_id     = program->id;
    //                            end_event_handle->dealloc          = &world->allocator;
    //                            end_event_handle->world            = world;
    //
    //                            mtt::send_message_to_entity(&world->message_passer, mtt::MESSAGE_TYPE_FROM_ENTITY, mtt::Thing_ID_INVALID, end_thing->id, static_cast<void*>(end_event_handle), mtt::Procedure_make([](void* data, mtt::Procedure_Input_Output* args) -> mtt::Procedure_Return_Type {
    //
    //                                mtt::World* world = static_cast<mtt::World*>(data);
    //                                mtt::Message* msg = static_cast<mtt::Message*>(args->input);
    //
    //                                mtt::send_system_message(&world->message_passer, mtt::MESSAGE_TYPE_FROM_ENTITY, mtt::Thing_ID_INVALID, msg->contents,
    //
    //                                                         mtt::Procedure_make([](void* data, mtt::Procedure_Input_Output* args) -> mtt::Procedure_Return_Type {
    //
    //                                    mtt::Message* msg = static_cast<mtt::Message*>(args->input);
    //
    //                                    Eval_End_Handle* end_handle = static_cast<Eval_End_Handle*>(msg->contents);
    //
    //                                    uint64 eval_ctx_id = end_handle->primary_id;
    //                                    uint64 program_id  = end_handle->secondary_id;
    //
    //                                    dt::DrawTalk* dt = dt::DrawTalk::ctx();
    //
    //                                    dt::Evaluation_Context* eval_ctx = nullptr;
    //
    //                                    if (!dt->sys_eval_ctx.get_evaluation_context(eval_ctx_id, &eval_ctx)) {
    //                                        MTT_error("ERROR: missing evaluation context\n");
    //                                        return true;
    //                                    }
    //
    //                                    for (usize i = 0; i < eval_ctx->programs.active_programs.size(); i += 1) {
    //                                        auto* program = eval_ctx->programs.active_programs[i];
    //                                        if (program->id == program_id) {
    //
    //
    //
    //                                            mtt::thing_group_set_active_state(dt->mtt, program->id, false);
    //                                            eval_ctx->programs.active_programs.erase(eval_ctx->programs.active_programs.begin() + i);
    //
    //                                            auto* program_next = program->next();
    //                                            if (program_next == nullptr) {
    //                                                auto* parent = program->parent();
    //
    //                                                ASSERT_MSG(parent != nullptr, "Parent shouldn't be null!\n");
    //                                                parent->unfinished_child_count -= 1;
    //                                                if (parent->unfinished_child_count == 0) {
    //
    //                                                    if (parent->node == MTT_Tree_root(&eval_ctx->programs.tree)) {
    //                                                        eval_ctx->deactivate();
    //                                                        eval_ctx->programs.run_end();
    //                                                        eval_ctx->programs.deinit();
    //                                                    }
    //
    //                                                }
    //                                            } else {
    //                                                ASSERT_MSG(false, "Sequencing not yet implemented!\n");
    //                                            }
    //
    //                                        }
    //                                    }
    //
    //                                    end_handle->dealloc->do_deallocate(end_handle, 1);
    //
    //                                    return mtt::Procedure_Return_Type();
    //                                }, data));
    //
    //
    //                                return mtt::Procedure_Return_Type(true);
    //                            }, static_cast<void*>(world)));
    //
    //
    //
    //                            break;
    //                        }
    //                        default: {
    //                            ASSERT_MSG(false, "Only support end condition from another thing's output\n");
    //                        }
    //                        }
    ////                        case END_KIND_FIELD:
    ////                            <#code#>
    ////                            break;
    ////                        case END_KIND_CONDITION:
    ////                            <#code#>
    ////                            break;
    //
    //
    //                    } else {
    //                        //
    //                        ASSERT_MSG(false, "Must implement and gate thing to support multiple end sources");
    //                        break;
    //                    }
    //
    //
    //
    //
    //                    break;
    //                }
    //                case BLOCK_TYPE_JUMP: {
    //                    auto& cur_block = *it_body;
    //                    auto& block = *it_body->get_concrete<dt::Jump_Block>();
    //                    const mtt::String& label = it_body->label;
    //                    mtt::Thing* thing = nullptr;
    //                    mtt::Any* data;
    //                    mtt::map_get(&program->identifiers.map, label, &data);
    //                    world->Thing_get(data->thing_id, &thing);
    //
    //
    //
    //                    for (auto it_j = cur_block.begin_children(); it_j != cur_block.end_children(); cur_block.advance(&it_j)) {
    //
    //                        dt::Value_Block* val = it_j->get_concrete<dt::Value_Block>();
    //
    //                        switch (val->kind) {
    //                        case VALUE_BLOCK_KIND_THING_REF: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_CONSTANT: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_THING_FIELD: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //
    //
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_THING_IN_PORT: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_THING_SELECTOR: {
    //
    //                            auto& identifiers = val->identifiers;
    //
    //                            usize entry_idx = 0;
    //                            auto entry = identifiers.begin();
    //                            if (entry != identifiers.end()) {
    //
    //                                auto& keys = *entry;
    //                                mtt::String key_path = "";
    //                                for (usize i = 0; i < keys.size() - 1; i += 1) {
    //                                    key_path += keys[i] + ":";
    //                                }
    //                                if (keys.size() != 0) {
    //                                    key_path += keys[keys.size() - 1];
    //                                }
    //
    //                                // MTT_print("searching for %s\n", key_path.c_str());
    //                                mtt::Any* any = nullptr;
    //                                if (!mtt::map_try_get(&program->identifiers.map, key_path, &any)) {
    //                                    MTT_error("ERROR: something is missing\n");
    //                                    break;
    //                                }
    //                                ASSERT_MSG(any->type == mtt::MTT_LIST, "incorrect type");
    //                                ASSERT_MSG(any->contained_type == mtt::MTT_ANY, "incorrect contained type");
    //
    //                                auto* things = mtt::access<mtt::Thing_List>(thing, it_j->label);
    //                                if (things == nullptr) {
    //                                    ASSERT_MSG(false, "Missing data %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                                }
    //
    //                                auto* selectors = mtt::access<std::vector<mtt::String>>(thing, it_j->label + "_selectors");
    //
    //                                if (things == nullptr) {
    //                                    ASSERT_MSG(false, "Missing data %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                                }
    //
    //
    //                                for (auto it_selector = val->selectors.begin(); it_selector != val->selectors.end(); ++it_selector) {
    //                                    selectors->push_back(*it_selector);
    //                                }
    //
    //
    //                                {
    //                                    auto* contained = ((std::vector<mtt::Any>*)any->List);
    //
    //                                    vec3 pos_avg = {0.0, 0.0, 0.0};
    //                                    for (auto it_contained = contained->begin(); it_contained != contained->end(); ++it_contained) {
    //                                        mtt::Thing* thing_exists_check = nullptr;
    //
    //                                        ASSERT_MSG(it_contained->type == mtt::MTT_THING, "Only supports thing params for now\n");
    //                                        bool exists = world->Thing_try_get(it_contained->thing_id, &thing_exists_check);
    //                                        ASSERT_MSG(exists, "ERROR: thing should exist!");
    //                                        mtt::Thing_print(world, thing_exists_check);
    //
    //                                        if (val->base.label == "source") {
    //                                            thing_exists_check = mtt::get_root(thing_exists_check);
    //                                        }
    //
    //                                        for (auto it_selector = val->selectors.begin(); it_selector != val->selectors.end(); ++it_selector) {
    //                                            ASSERT_MSG(mtt::has_selector(thing_exists_check, *it_selector), "Missing selector %s\n", it_selector->c_str());
    //
    //
    //                                            if (val->base.label == "source") {
    //                                                int BP = 0;
    //
    //                                                mtt::Message msg;
    //                                                msg.sender   = mtt::Thing_ID_INVALID;
    //                                                msg.selector = *it_selector;
    //                                                selector_invoke(thing_exists_check, &msg);
    //                                                ASSERT_MSG(msg.output_value.type == mtt::MTT_VECTOR3, "position should be a vec3\n");
    //                                                vec3 pos = msg.output_value.Vector3;
    //                                                pos_avg += pos;
    //
    //                                            }
    //                                        }
    //
    //
    //                                        things->push_back(mtt::Thing_Ref(thing_exists_check->id, world));
    //
    //                                        int BP = 0;
    //                                    }
    //                                    pos_avg /= contained->size();
    //                                    if (val->base.label == "source") {
    //                                        vec3* initial_position = mtt::access<vec3>(thing, "initial_position");
    //                                        ASSERT_MSG(initial_position != nullptr, "Initial position is a required field on Jump!\n");
    //                                        *initial_position = pos_avg;
    //                                    }
    //
    //                                    int BP = 0;
    //
    //                                }
    //
    //                                entry_idx += 1;
    //                            }
    //
    //                            // MARK: implement first
    //                            int BP = 0;
    //
    //
    //
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_EXPRESSION: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_LABEL: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_PARAMETER: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_MULTIPLE: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_TYPE_NAME: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_IDENTIFIER: {
    //                            ASSERT_MSG(false, "TODO handle case %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    //                            break;
    //                        }
    //                        case VALUE_BLOCK_KIND_NUMBER: {
    //                            auto& identifiers = val->identifiers;
    //
    //                            auto entry = identifiers.begin();
    //                            if (entry != identifiers.end()) {
    //
    //                                auto& keys = *entry;
    //                                mtt::String key_path = "";
    //                                for (usize i = 0; i < keys.size() - 1; i += 1) {
    //                                    key_path += keys[i] + ":";
    //                                }
    //                                if (keys.size() != 0) {
    //                                    key_path += keys[keys.size() - 1];
    //                                }
    //
    //                                mtt::Any* any = nullptr;
    //                                float32 num_value = 0.0f;
    //                                if (!mtt::map_try_get(&program->identifiers.map, key_path, &any)) {
    //                                    MTT_print("parameter %s unspecified, using default %f\n", key_path.c_str(), val->default_value.Float);
    //                                    num_value = val->default_value.Float;
    //                                } else {
    //                                    num_value = any->Float;
    //                                }
    //
    //                                auto* num_val_data = mtt::access<float32>(thing, val->base.label);
    //                                ASSERT_MSG(num_val_data != nullptr, "Jump should always have this field property\n");
    //                                *num_val_data = num_value;
    //                            }
    //
    //                            int BP = 0;
    //
    //                            break;
    //                        }
    //                        }
    //                    }
    //
    //
    //
    //                    {
    ////                        auto search = dt::search(block.base.node
    ////                                                 , "option_flags");
    ////                        ASSERT_MSG(search.result != nullptr, "option_flags should exist!\n");
    ////
    ////                        auto* value = Block_get_concrete<dt::Value_Block>(search.result);
    ////
    ////                        jump_thing->logic.option_flags = value->
    //                    }
    //
    //
    //                    //                                                            for (Behavior_Block* it_j_args = it_body->begin_children(); it_j_args != it_body->end_children(); Behavior_Block::advance(&it_j_args)) {
    //                    //                                                                Block_print(it_j_args);
    //                    //                                                            }
    //                    break;
    //                }
    //                }
    //
    //            }
    //        }
    //
    //
    //    }
    //    int BP = 0;
    //    eval_ctx->run_begin();
    //    mtt::thing_group_set_active_state(world, __root_id__, true);
    //    int BP2 = 0;
}

















inline static std::vector<Speech_Property*> responses;

void transform_trigger_responses(Speech_Property* parent, Speech_Property* root, std::vector<Speech_Property*>& list);
void transform_trigger_responses(Speech_Property* parent, Speech_Property* root, std::vector<Speech_Property*>& list)
{
    if (root == nullptr) {
        return;
    }
    for (auto it = root->properties.begin(); it != root->properties.end(); ++it) {
        auto& child_list = it->second;
        for (auto c = child_list.begin(); c != child_list.end(); ++c) {
            transform_trigger_responses(root, *c, list);
        }
    }
    
    if (root->type_str == "RESPONSE" && root->has_prop("TRIGGER")) {
        auto* res = root->push_prop("RESPONSE");
        
        struct Move_Op {
            mtt::String key_from = {};
            mtt::String key_to = {};
        };
        std::vector<Move_Op> ops = {};
        for (auto it = root->properties.begin(); it != root->properties.end(); ++it) {
            const mtt::String& key = it->first;
//            std::vector<Speech_Property*>* props = &it->second;
            if (key == "TRIGGER" || key == "RESPONSE") {
                continue;
            }
            
            if (key == "ANOTHER_RESPONSE_CLAUSE_AND") {
                //root->move_prop(key, "SEQUENCE_SIMULTANEOUS", res);
                ops.push_back({key, "SEQUENCE_SIMULTANEOUS"});
            } else if (key == "ANOTHER_RESPONSE_CLAUSE_THEN") {
                //root->move_prop(key, "SEQUENCE_THEN", res);
                ops.push_back({key, "SEQUENCE_SIMULTANEOUS"});
            } else {
                //root->move_prop(key, key, res);
                ops.push_back({key, key});
            }
        }
        for (usize i = 0; i < ops.size(); i += 1) {
            auto& op = ops[i];
            root->move_prop(op.key_from, op.key_to, res);
        }
        
        
        res->label     = root->label;
        root->label    = "";
        root->annotation = "container";
        res->tag_str   = root->tag_str;
        res->kind_str  = root->kind_str;
        res->token     = root->token;
        res->token->prop_ref = res;
        root->token = nullptr;
        res->type_str  = "RESPONSE";
        root->type_str = "TRIGGER_RESPONSE";
    }
    
    
}

void xform_trig_res(Speech_Property* root);
void xform_trig_res(Speech_Property* root)
{
    
    //root->print();
    transform_trigger_responses(nullptr, root, responses);
    //root->print();
    
    return;
    //DT_scope_open();
//    DT_print("label=[%s], tag=[%s], type=[%s], kind=[%s], key=[%s]\n",   property->label.c_str(), property->tag_str.c_str(), property->type_str.c_str(), property->kind_str.c_str(), property->key.c_str());
    
//    {
//
//        std::vector<Speech_Property*>* simul_response_list_ptr = property->try_get_prop("SIMULTANEOUS_RESPONSE");
//
//
//        if (property->type_str == "RESPONSE" && property->has_prop("TRIGGER")) {
//
//            mtt::String key = property->key;
//
//            Speech_Property* parent = property->parent;
//
//            std::vector<Speech_Property*>* trigger_list_ptr = property->try_get_prop("TRIGGER");
//
//
//            if (trigger_list_ptr != nullptr) {
//                parent->remove_prop(property);
//
//                for (auto t_it = trigger_list_ptr->begin(); t_it != trigger_list_ptr->end(); ++t_it) {
//                    Speech_Property* trigger = *t_it;
//                    trigger->push_prop("RESPONSE", property);
//                }
//                if (simul_response_list_ptr != nullptr) {
//                    for (auto r_it = simul_response_list_ptr->begin(); r_it != simul_response_list_ptr->end(); ++r_it) {
//                        for (auto t_it = trigger_list_ptr->begin(); t_it != trigger_list_ptr->end(); ++t_it) {
//                            Speech_Property* trigger = *t_it;
//
//                            trigger->push_prop("RESPONSE", *r_it);
//                        }
//                    }
//                    property->remove_prop_no_parent_clear("SIMULTANEOUS_RESPONSE");
//                }
//                for (auto t_it = trigger_list_ptr->begin(); t_it != trigger_list_ptr->end(); ++t_it) {
//                    Speech_Property* trigger = *t_it;
//                    parent->push_prop(key, trigger);
//                }
//                property->remove_prop_no_parent_clear("TRIGGER");
//
//
//
//
//            }
//
//
//
//
//        }
//    }
//
//    usize i = 0;
//    for (auto it = property->properties.begin(); it != property->properties.end(); ++it) {
//        std::vector<Speech_Property*>& prop_list = it->second;
//        for (auto prop_it = prop_list.begin(); prop_it != prop_list.end(); ++prop_it) {
//            xform_trig_res(*prop_it);
//        }
//        i += 1;
//    }
}

Speech_Property* lang_compile(dt::DrawTalk* dt, dt::Parse* parse)
{
    // find root
    dt::System_Evaluation_Context& ctx = dt->sys_eval_ctx;
    ctx.noun_list.clear();
    ctx.verb_list.clear();
    ctx.rule_list.clear();
    ctx.card_list.clear();
    ctx.start_new_eval();
    
    
    dt::Lang_Eval_Args args;
    args.dt = dt;
    args.parse = parse;
    
    auto& token_list = parse->token_list;
//    for (usize i = 0; i < token_list.size(); i += 1) {
//        Token_print(parse, token_list[i]);
//    }
    
    dt::Speech_Instructions::init();
    
    Speech_Property* cmd_list = nullptr;
    
    
    for (usize i = 0; i < token_list.size(); i += 1) {
        if (token_list[i]->dep == spacy_nlp::DEP_ROOT) {
            parse->root_dependency_token_ID = i;
            if (token_list[i]->pos == spacy_nlp::POS_NOUN || token_list[i]->pos == spacy_nlp::POS_PROPN) {
                auto* tok = token_list[i];
                auto& children = tok->left_children;
                for (auto& l_child_idx : children) {

                    auto* l_child = token_list[l_child_idx];
                    if (l_child->pos == spacy_nlp::POS_NOUN || l_child->pos == spacy_nlp::POS_PROPN) {
                        token_list[i]->pos = spacy_nlp::POS_VERB;
                        l_child->dep = spacy_nlp::DEP_nsubj;
                        token_list[i]->tag = spacy_nlp::TAG_VB;
                        l_child->force_not_a_type = true;
                    }
                }
            }
            if (token_list[i]->pos == spacy_nlp::POS_VERB || token_list[i]->pos == spacy_nlp::POS_AUX) {
                ctx.dep_token_stack.push_back(token_list[i]);
                ctx.fragments.clear();
                auto result = (token_list[i]->pos == spacy_nlp::POS_VERB) ? handle_verb(&args, *token_list[i]) : handle_aux(&args, *token_list[i]);
                switch (result.status.type) {
                default: // fallthrough
                case INTERP_STATUS_OK: {
                    if (cmd_list == nullptr) {
                        cmd_list = Speech_Property::make();
                    }

                    for (auto it_f = ctx.fragments.begin(); it_f != ctx.fragments.end(); ++it_f) {
                        auto* cmd = Speech_Property::make();
                        cmd->type_str = "CMD";
                        cmd_list->push_prop("CMD_LIST", cmd);
                        cmd->push_prop((*it_f)->type_str, (*it_f));
//                        Speech_Property* fragment = *it_f;
//                        for (usize i_roots = 0; i_roots < result.value->prop_list.size(); i_roots += 1) {
//                            Speech_Property* root_prop = result.value->prop_list[i_roots];
//
//                            cmd->push_prop((root_prop->type_str == "RESPONSE") ? "TRIGGER_RESPONSE" : root_prop->type_str, root_prop);
//                        }
                    }
                    
                    auto* cmd = Speech_Property::make();
                    cmd->type_str = "CMD";
                    cmd_list->push_prop("CMD_LIST", cmd);
                    for (usize i_roots = 0; i_roots < result.value->prop_list.size(); i_roots += 1) {
                        Speech_Property* root_prop = result.value->prop_list[i_roots];
                        
                        auto* out = cmd->push_prop((root_prop->type_str == "RESPONSE") ? "TRIGGER_RESPONSE" : root_prop->type_str, root_prop);
                        
                    }
                    
//                    usize reserve_size = 0;
//                    for (usize t = 0; t < token_list.size(); t += 1) {
//                        if (token_list[t]->is_blocked) {
//                            continue;
//                        }
//
//                        reserve_size += token_list[t]->text.size() + 1;
//                    }
//                    reserve_size -= 1;
//                    mtt::String label;
//                    label.resize(reserve_size);
//                    {
//                        usize t = 0;
//                        for (; t < token_list.size() - 1; t += 1) {
//                            if (token_list[t]->is_blocked) {
//                                continue;
//                            }
//                            label += token_list[t]->text + " ";
//                        }
//                        for (; t < token_list.size(); t += 1) {
//                            if (token_list[t]->is_blocked) {
//                                continue;
//                            }
//                            label += token_list[t]->text;
//                        }
//                    }
//                    cmd->label = label;
                    
                    //cmd_list->print();
                    
                    //MTT_print("GOOD!\n");
                    
                    
                    
                    debug_break();
                    
                    
                    
                    
                    //MTT_print("///////////////////////////\n");
                    
                    //cmd_list->print();


                }
            }
            }
        }
    }
    
    parse->cmds = cmd_list;
    if (parse->cmds != nullptr) {
        xform_trig_res(parse->cmds);
        

        
    }
    return cmd_list;
}


namespace ex {

Exec_Result handle_action(Exec_Args& args);
Exec_Result handle_trigger_response(Exec_Args& args);
Exec_Result handle_thing_selection(Exec_Args& args);


const mtt::Set_Stable<mtt::String> action_treat_direct_objects_as_type = {
    "create",
    "make",
    "spawn",
    "copy",
    "clone",
    "duplicate",
    "transform",
    "become",
    "form",
};

Exec_Result handle_action(Exec_Args& args)
{
    args.prop->was_preprocessed = true;

    DT_scope_open();
    DT_print("%s", __PRETTY_FUNCTION__);
    
    mtt::String& label = args.prop->label;
    
    Instruction* ins = nullptr;
    args.prop->print();
    mtt::Map_Stable<mtt::String, DT_Behavior>* candidates = nullptr;
    if (args.lang_ctx->behavior_catalogue.lookup(label, &candidates)) {
//        for (auto it = candidates->begin(); it != candidates->end(); ++it) {
//            DT_print("KEY:[%s]VALUE:[%s]-[%s]\n", (*it).first.c_str(), (*it).second.key_primary.c_str(), (*it).second.key_secondary.c_str());
//        }
        if (candidates->size() > 1) {
            ins = Instruction::make();
            ins->type = "ACTION";
            ins->kind = args.prop->kind_str;
            ins->annotation = "choose_candidate";
            ins->prop.push_back(args.prop);
            ins->value.from_Reference_Type(static_cast<void*>(candidates));
            args.prop->instruction = ins;
            args.prop->sub_label = candidates->begin()->second.key_secondary;
            //args.lang_ctx->eval_ctx.instructions.push_back(ins);
        } else {
            ins = Instruction::make();
            ins->type = "ACTION";
            ins->kind = args.prop->kind_str;
            ins->annotation = args.prop->annotation != "" ? args.prop->annotation : "confirm";
            ins->prop.push_back(args.prop);
            ins->value.from_Reference_Type(static_cast<void*>(candidates));
            args.prop->instruction = ins;
            args.prop->sub_label = candidates->begin()->second.key_secondary;
            //args.lang_ctx->eval_ctx.instructions.push_back(ins);
        }
    } else {
        {
            ins = Instruction::make();
            ins->type = "ACTION";
            ins->kind = args.prop->kind_str;
            if (ins->kind == "EXISTENTIAL") {
                ins->annotation = args.prop->annotation != "" ? args.prop->annotation : "assigning_properties, e.g. to be";
                ins->prop.push_back(args.prop);
                ins->value.type = mtt::MTT_NONE;
                //args.lang_ctx->eval_ctx.instructions.push_back(ins);
            } else {
                ins->annotation = (args.prop->annotation != "" && args.prop->annotation != "END_CONDITION") ? args.prop->annotation : "unknown";
                ins->prop.push_back(args.prop);
                ins->value.type = mtt::MTT_NONE;
                //args.lang_ctx->eval_ctx.instructions.push_back(ins);
            }
            args.prop->instruction = ins;
            
            // TODO try checking arguments for context to find a possible default
        }
    }
    ins->should_descend = 0;
    
    std::vector<Speech_Property*>* preposition_list = nullptr;
    
    std::vector<Speech_Property*>* object_list = nullptr;
    std::vector<Speech_Property*>* ind_object_list = nullptr;
    std::vector<Speech_Property*>* misc_object_list = nullptr;
    std::vector<Speech_Property*>* agent_list = nullptr;
    
    
    if (args.prop->try_get_prop("AGENT", &agent_list)) {
        for (auto it = agent_list->begin(); it != agent_list->end(); ++it) {
            auto ch_args = args;
            ch_args.prop = *it;
        
            auto result = handle_thing_selection(ch_args);
            switch (result.status) {
            case Exec_Status::OK: {
                if (result.value.ins == nullptr) {
                    MTT_error("%s", "WARNING: instruction should exist!\n");
                    break;
                }
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::TO;
                result.value.ins->parent = ins;
                //ins->children.push_back(result.value.ins);
                if (ins->kind == "EXISTENTIAL") {
                    //Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                    // FIXME: maybe a different condition for the side?
                    Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_LEFT);
                } else {
                    Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_LEFT);
                }
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                if (!result.value.ins->siblings.empty()) {
                    for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
                        Instruction* s = *s_it;
                        s->link_type = Instruction::PARENT_LINK_TYPE::TO;
                        //ins->children.push_back(s);
                        Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_LEFT);
                        args.lang_ctx->eval_ctx.instructions().push_back(s);
                        s->parent = ins;
                    }
                }
                break;
            }
            case Exec_Status::FAILED: {
                if (ch_args.prop->tag_str == "TIME") {
                    auto* ch_ins = dt::Instruction::make();
                    ch_ins->parent = ins;
                    args.lang_ctx->eval_ctx.instructions().push_back(ch_ins);
                    //ins->children.push_back(ch_ins);
                    Instruction_add_child(ins, ch_ins, Instruction_CHILD_SIDE_TYPE_LEFT);
                    ch_ins->prop.push_back(ch_args.prop);
                    ch_ins->kind = ch_args.prop->label;
                    ch_ins->color_current = nvgRGBAf(color::WHITE[0],color::WHITE[1], color::WHITE[2], color::WHITE[3]);
                    ch_ins->color_selected = nvgRGBAf(color::WHITE[0],color::WHITE[1], color::WHITE[2], color::WHITE[3]);
                    ch_ins->link_type = Instruction::PARENT_LINK_TYPE::TO;
                    ch_ins->type = ch_args.prop->key;
                    ins->require_items_if_object = false;
                }
                break;
            }
            default: {
                MTT_BP();
                break;
            }
            }
        }
    }

    if (args.prop->try_get_prop("DIRECT_OBJECT", &object_list)) {
        for (auto it = object_list->begin(); it != object_list->end(); ++it) {
            auto ch_args = args;
            ch_args.prop = *it;
            
            bool treat_as_type_before = ch_args.treat_as_type;
            {
                
                if (action_treat_direct_objects_as_type.contains(args.prop->token->lemma)) {
                    ch_args.treat_as_type = true;
                }
            }
            
            auto result = handle_thing_selection(ch_args);
            ch_args.treat_as_type = treat_as_type_before;
            
            switch (result.status) {
            case Exec_Status::OK:
                if (result.value.ins == nullptr) {
                    MTT_error("%s", "WARNING: instruction should exist!\n");
                    break;
                }
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                result.value.ins->parent = ins;
                //result.value.ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                //ins->children.push_back(result.value.ins);
                Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                if (!result.value.ins->siblings.empty()) {
                    for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
                        Instruction* s = *s_it;
                        s->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                        //ins->children.push_back(s);
                        Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_RIGHT);
                        args.lang_ctx->eval_ctx.instructions().push_back(s);
                        s->parent = ins;
                    }
                }
                break;
            case Exec_Status::FAILED:
                break;
            }
        }
    }
    
    if (args.prop->try_get_prop("PREPOSITION", &preposition_list)) {
        for (auto it = preposition_list->begin(); it != preposition_list->end(); ++it) {
            Instruction* p_ins = Instruction::make();
            p_ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
            p_ins->parent = ins;
            //p_ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
            //ins->children.push_back(p_ins);
            Instruction_add_child(ins, p_ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
            p_ins->type = "PREPOSITION";
            p_ins->kind = (*it)->label;
            p_ins->prop.push_back(*it);
            p_ins->value.type = mtt::MTT_NONE;
            p_ins->should_descend = 1;
            args.lang_ctx->eval_ctx.instructions().push_back(p_ins);
            args.prop->instruction = ins;
            
            std::vector<Speech_Property*>* prep_object_list = nullptr;
            if ((*it)->try_get_prop("OBJECT", &prep_object_list)) {
                for (auto o_it = prep_object_list->begin(); o_it != prep_object_list->end(); ++o_it) {
                    auto ch_args = args;
                    ch_args.prop = *o_it;
                    
                    bool treat_as_type_before = ch_args.treat_as_type;
                    {
                        
                        if (action_treat_direct_objects_as_type.contains(args.prop->token->lemma)) {
                            ch_args.treat_as_type = true;
                        }
                    }
                    auto result = handle_thing_selection(ch_args);
                    ch_args.treat_as_type = treat_as_type_before;
                    switch (result.status) {
                    case Exec_Status::OK:
                        if (result.value.ins == nullptr) {
                            MTT_error("%s", "WARNING: instruction should exist!\n");
                            break;
                        }
                        result.value.ins->parent = p_ins;
                        //p_ins->children.push_back(result.value.ins);
                        result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                        //result.value.ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                        Instruction_add_child(p_ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                        args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                        if (!result.value.ins->siblings.empty()) {
                            for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
                                Instruction* s = *s_it;
                                s->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                                //s->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                                //ins->children.push_back(s);
                                Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_RIGHT);
                                args.lang_ctx->eval_ctx.instructions().push_back(s);
                                s->parent = ins;
                            }
                        }
                        break;
                    case Exec_Status::FAILED:
                        break;
                    default:
                        //ASSERT_MSG(false, "Should be impossible");
                        break;
                    }
                }
            }
            std::vector<Speech_Property*>* _list = nullptr;
            if ((*it)->try_get_prop("TIME", &_list)) {
                auto* P = _list->front();
                if (P->tag_str == "TIME") {
                    auto* ch_ins = dt::Instruction::make();
                    ch_ins->parent = ins;
                    args.lang_ctx->eval_ctx.instructions().push_back(ch_ins);
                    //ins->children.push_back(ch_ins);
                    Instruction_add_child(ins, ch_ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                    ch_ins->prop.push_back(P);
                    ch_ins->kind = "TIME";
                    // TODO: actual value source a little variable. This is good enough for now
                    
                    ch_ins->color_current = nvgRGBAf(color::WHITE[0],color::WHITE[1], color::WHITE[2], color::WHITE[3]);
                    ch_ins->color_selected = nvgRGBAf(color::WHITE[0],color::WHITE[1], color::WHITE[2], color::WHITE[3]);
                    ch_ins->link_type = Instruction::PARENT_LINK_TYPE::TO;
                    ch_ins->type = P->key;
                    ins->require_items_if_object = false;
                }
            }

        }
    }
    
    
    if (args.prop->try_get_prop("INDIRECT_OBJECT", &ind_object_list)) {
        for (auto it = ind_object_list->begin(); it != ind_object_list->end(); ++it) {
            auto ch_args = args;
            ch_args.prop = *it;
            bool treat_as_type_before = ch_args.treat_as_type;
            {
                
                if (action_treat_direct_objects_as_type.contains(args.prop->token->lemma)) {
                    ch_args.treat_as_type = true;
                }
            }
            auto result = handle_thing_selection(ch_args);
            ch_args.treat_as_type = treat_as_type_before;
            switch (result.status) {
            case Exec_Status::OK:
                if (result.value.ins == nullptr) {
                    MTT_error("%s", "WARNING: instruction should exist!\n");
                    break;
                }
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                result.value.ins->parent = ins;
                //result.value.ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                //ins->children.push_back(result.value.ins);
                Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                if (!result.value.ins->siblings.empty()) {
                    for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
                        Instruction* s = *s_it;
                        s->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                        //s->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                        //ins->children.push_back(s);
                        Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_RIGHT);
                        args.lang_ctx->eval_ctx.instructions().push_back(s);
                        s->parent = ins;
                    }
                }
                break;
            case Exec_Status::FAILED:
                break;
            }
        }
    }
    
    if (args.prop->try_get_prop("OBJECT", &misc_object_list)) {
        for (auto it = misc_object_list->begin(); it != misc_object_list->end(); ++it) {
            auto ch_args = args;
            ch_args.prop = *it;
            
            auto result = handle_thing_selection(ch_args);
            switch (result.status) {
            case Exec_Status::OK:
                if (result.value.ins == nullptr) {
                    MTT_error("%s", "WARNING: instruction should exist!\n");
                    break;
                }
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                result.value.ins->parent = ins;
                //result.value.ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                //ins->children.push_back(result.value.ins);
                Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                if (!result.value.ins->siblings.empty()) {
                    for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
                        Instruction* s = *s_it;
                        s->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                        //ins->children.push_back(s);
                        Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_RIGHT);
                        args.lang_ctx->eval_ctx.instructions().push_back(s);
                        s->parent = ins;
                    }
                }
                {
                    if (dt::Speech_Property::Prop_List* sub_action_prop = (*it)->try_get_prop("ACTION"); sub_action_prop != nullptr) {
                        for (Speech_Property* prop : *sub_action_prop) {
                            {
                                auto ch_args = args;
                                ch_args.prop =prop;
                                
                                
                                auto result = handle_action(ch_args);
                                switch (result.status) {
                                case Exec_Status::OK: {
                                    if (result.value.ins == nullptr) {
                                        MTT_error("%s", "WARNING: instruction should exist!\n");
                                        break;
                                    }
                                    result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                                    result.value.ins->parent = ins;
                                    //ins->children.push_back(result.value.ins);
                                    Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                                    args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                                    break;
                                }
                                case Exec_Status::FAILED: {
                                    break;
                                }
                                }
                            }
                        }
                    }
                }
                break;
            case Exec_Status::FAILED:
                break;
            }
        }
    }
    
    std::vector<Speech_Property*>* property_prop_list = nullptr;
    if (args.prop->try_get_prop("PROPERTY", &property_prop_list)) {
        std::vector<Speech_Property*>* count_list = nullptr;
        for (auto it = property_prop_list->begin(); it != property_prop_list->end(); ++it) {
            if ((*it)->try_get_prop("COUNT", &count_list)) {
                auto* P = count_list->front();
                {
                    auto* ch_ins = dt::Instruction::make();
                    ch_ins->parent = ins;
                    args.lang_ctx->eval_ctx.instructions().push_back(ch_ins);
                    //ins->children.push_back(ch_ins);
                    Instruction_add_child(ins, ch_ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                    ch_ins->prop.push_back(P);
                    // TODO add actual number for units
                    
                    const auto trim_trailing_zeros = [](const std::string& num) -> std::string {
                        if (num.find(".") == std::string::npos) {
                            return num;
                        }
                        auto j = num.length()-1;
                        while(j >= 0 && num[j]=='0')j--;
                        if(j<0)return "";
                        return num[j] == '.' ? num.substr(0, j) : num.substr(0,j+1);
                    };
                    ch_ins->kind = trim_trailing_zeros(std::to_string(P->value.numeric)) + "x";
                    
                    ch_ins->color_current = nvgRGBAf(color::WHITE[0],color::WHITE[1], color::WHITE[2], color::WHITE[3]);
                    ch_ins->color_selected = nvgRGBAf(color::WHITE[0],color::WHITE[1], color::WHITE[2], color::WHITE[3]);
                    ch_ins->link_type = Instruction::PARENT_LINK_TYPE::TO;
                    ch_ins->type = P->key;
                    ins->require_items_if_object = false;
                }
            }
        }
    }
            
    
    Speech_Property::Prop_List* seq_then = nullptr;
    if (args.prop->try_get_prop("SEQUENCE_THEN", &seq_then)) {
        for (auto it = seq_then->begin(); it != seq_then->end(); ++it) {
            auto ch_args = args;
            ch_args.prop = *it;
            
            
            Speech_Property::Prop_List* seq_sim_agent = nullptr;
            if (!ch_args.prop->try_get_prop("AGENT", &seq_sim_agent)) {
                if (agent_list != nullptr) {
                    
                    for (auto a_it = agent_list->begin(); a_it != agent_list->end(); ++a_it) {
                        auto* sub = ch_args.prop->push_prop("AGENT", (*a_it)->copy());
                        sub->annotation = "DO_NOT_SELECT_THING";
                    }
                }
            }
            
            auto* prev_already_chosen = ch_args.already_chosen;
            mtt::Set_Stable<mtt::Thing_ID> sub_already_chosen = {};
            ch_args.already_chosen = &sub_already_chosen;
            auto result = handle_action(ch_args);
            ch_args.already_chosen = prev_already_chosen;
            switch (result.status) {
            case Exec_Status::OK: {
                if (result.value.ins == nullptr) {
                    MTT_error("%s", "WARNING: instruction should exist!\n");
                    break;
                }
                
                Instruction* sub_ins = Instruction::make();
                //ins->children.push_back(sub_ins);
                Instruction_add_child(ins, sub_ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                sub_ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                //sub_ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                sub_ins->parent = ins;
                //sub_ins->children.push_back(result.value.ins);
                Instruction_add_child(sub_ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                sub_ins->type = "SEQUENCE_THEN";
                sub_ins->kind = "THEN";
                sub_ins->should_descend = 0;
                sub_ins->x_offset = 1;
                sub_ins->prop.push_back(*it);
                sub_ins->value.type = mtt::MTT_NONE;
                args.prop->instruction = ins;
                
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                //result.value.ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                result.value.ins->parent = sub_ins;
                
                args.lang_ctx->eval_ctx.instructions().push_back(sub_ins);
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                
                result.value.ins->link_label_type = Instruction::LINK_LABEL_TYPE::THEN;
                break;
            }
            case Exec_Status::FAILED: {
                break;
            }
            }
        }
    }
    
    Speech_Property::Prop_List* seq_sim = nullptr;
    if (args.prop->try_get_prop("SEQUENCE_SIMULTANEOUS", &seq_sim)) {
        for (auto it = seq_sim->begin(); it != seq_sim->end(); ++it) {
            auto ch_args = args;
            ch_args.prop = *it;
    
            Speech_Property::Prop_List* seq_sim_agent = nullptr;
            if (!ch_args.prop->try_get_prop("AGENT", &seq_sim_agent)) {
                if (agent_list != nullptr) {
                    for (auto a_it = agent_list->begin(); a_it != agent_list->end(); ++a_it) {
                        auto* sub = ch_args.prop->push_prop("AGENT", (*a_it)->copy());
                        sub->annotation = "DO_NOT_SELECT_THING";
                    }
                }
            }
            
            auto result = handle_action(ch_args);
            switch (result.status) {
            case Exec_Status::OK: {
                if (result.value.ins == nullptr) {
                    MTT_error("%s", "WARNING: instruction should exist!\n");
                    break;
                }
                Instruction* sub_ins = Instruction::make();
                //ins->children.push_back(sub_ins);
                Instruction_add_child(ins, sub_ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                sub_ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                //sub_ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                sub_ins->parent = ins;
                //sub_ins->children.push_back(result.value.ins);
                Instruction_add_child(sub_ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                sub_ins->type = "SEQUENCE_SIMULTANEOUS";
                sub_ins->kind = "AND";
                sub_ins->prop.push_back(*it);
                sub_ins->value.type = mtt::MTT_NONE;
                sub_ins->should_descend = 0;
                sub_ins->x_offset = 1;
                args.prop->instruction = ins;
                
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                result.value.ins->parent = sub_ins;
                //result.value.ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                
                args.lang_ctx->eval_ctx.instructions().push_back(sub_ins);
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                result.value.ins->link_label_type = Instruction::LINK_LABEL_TYPE::AND;
                break;
            }
            case Exec_Status::FAILED: {
                break;
            }
            }
        }
    }
    
    Speech_Property::Prop_List* goal = nullptr;
    if (args.prop->try_get_prop("GOAL", &goal)) {
        for (auto it = goal->begin(); it != goal->end(); ++it) {
            auto ch_args = args;
            ch_args.prop = *it;
            
            auto result = handle_action(ch_args);
            switch (result.status) {
            case Exec_Status::OK: {
                if (result.value.ins == nullptr) {
                    MTT_error("%s", "WARNING: instruction should exist!\n");
                    break;
                }
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                result.value.ins->parent = ins;
               // result.value.ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                //ins->children.push_back(result.value.ins);
                Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                break;
            }
            case Exec_Status::FAILED: {
                break;
            }
            }
        }
    }
    
    Speech_Property::Prop_List* end_condition = nullptr;
    if (args.prop->try_get_prop("END_CONDITION", &end_condition)) {
        for (auto it = end_condition->begin(); it != end_condition->end(); ++it) {
            auto ch_args = args;
            ch_args.prop = *it;
            
            auto result = handle_action(ch_args);
            switch (result.status) {
            case Exec_Status::OK: {
                if (result.value.ins == nullptr) {
                    MTT_error("%s", "WARNING: instruction should exist!\n");
                    break;
                }
                auto* until_ins = dt::Instruction::make();
                until_ins->type = "END_CONDITION";
                until_ins->kind = "UNTIL";
                until_ins->parent = ins;
                until_ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                //until_ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                //until_ins->children.push_back(result.value.ins);
                Instruction_add_child(until_ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                args.lang_ctx->eval_ctx.instructions().push_back(until_ins);
                //ins->children.push_back(until_ins);
                Instruction_add_child(ins, until_ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                result.value.ins->parent = until_ins;
                //result.value.ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                break;
            }
            case Exec_Status::FAILED: {
                break;
            }
            }
        }
    }
    
    Speech_Property::Prop_List* action = nullptr;
    if (args.prop->try_get_prop("ACTION", &action)) {
        for (auto it = action->begin(); it != action->end(); ++it) {
            auto ch_args = args;
            ch_args.prop = *it;
            
            auto result = handle_action(ch_args);
            switch (result.status) {
            case Exec_Status::OK: {
                if (result.value.ins == nullptr) {
                    MTT_error("%s", "WARNING: instruction should exist!\n");
                    break;
                }
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                result.value.ins->parent = ins;
               // result.value.ins->child_side_type = Instruction_CHILD_SIDE_TYPE_RIGHT;
                //ins->children.push_back(result.value.ins);
                Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                break;
            }
            case Exec_Status::FAILED: {
                break;
            }
            }
        }
    }
    

    
    DT_scope_close();
    
    return exec_ok(ins);
}

MTT_NODISCARD Exec_Result handle_trigger_response(Exec_Args& args)
{
    args.prop->was_preprocessed = true;

    using Prop_List = Speech_Property::Prop_List;
    DT_scope_open();
    DT_print("%s", __PRETTY_FUNCTION__);
    
    
    
    
    bool has_trigger  = false;
    bool has_response = false;
    
    Prop_List* trigger_list = nullptr;
    Prop_List* response_list = nullptr;
    if (!(args.prop->try_get_prop("TRIGGER", &trigger_list) &&
          args.prop->try_get_prop("RESPONSE", &response_list))) {
        return exec_failed();
    }
    
    Prop_List* end_condition_list = nullptr;
    args.prop->try_get_prop("END_CONDITION", &end_condition_list);
    
    auto* ins = Instruction::make();
    ins->type = "TRIGGER_RESPONSE";
    ins->prop.push_back(args.prop);
    args.prop->instruction = ins;
    
    auto handle_triggers = [&]() {

        for (usize i = 0; i < trigger_list->size(); i += 1) {
            Speech_Property& prop = *((*trigger_list)[i]);
            
            auto ch_args = args;
            ch_args.prop = &prop;
            auto result = handle_action(ch_args);
            
            switch (result.status) {
            case Exec_Status::OK:
                    if constexpr ((false)) {
                        args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                        result.value.ins->type = "TRIGGER";
                        result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::TO;
                        result.value.ins->parent = ins;
                        //ins->children.push_back(result.value.ins);
                        Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_LEFT);
                    } else {
                        
                        Instruction* sub_ins = Instruction::make();
                        Instruction_add_child(ins, sub_ins, Instruction_CHILD_SIDE_TYPE_LEFT);
                        sub_ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                        sub_ins->parent = ins;
                        sub_ins->type = "RULE_TYPE";
                        switch ((*(result.value.ins->prop.begin()))->action_event) {
                            case VERB_EVENT_BEGIN: {
                                sub_ins->kind = "WHEN";
                                break;
                            }
                            case VERB_EVENT_END: {
                                sub_ins->kind = "AFTER";
                                break;
                            }
                            case VERB_EVENT_CONTINUOUS: {
                                sub_ins->kind = "AS";
                                break;
                            }
                            default: {
                                sub_ins->kind = "";
                                break;
                            }
                        }
                        sub_ins->should_descend = 0;
                        sub_ins->x_offset = 0;
                        sub_ins->value.type = mtt::MTT_NONE;
                        args.prop->instruction = sub_ins;
                        args.lang_ctx->eval_ctx.instructions().push_back(sub_ins);
                        
                        
                        args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                        result.value.ins->type = "TRIGGER";
                        result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::TO;
                        result.value.ins->parent = ins;
                        //ins->children.push_back(result.value.ins);
                        Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_LEFT);
                    }
                
                break;
            case Exec_Status::FAILED:
                break;
            }
        }
    };
    
    auto handle_responses = [&]() {
        for (usize i = 0; i < response_list->size(); i += 1) {
            Speech_Property& prop = *((*response_list)[i]);
            
            auto ch_args = args;
            ch_args.prop = &prop;
            
            auto* prev_already_chosen = ch_args.already_chosen;
            mtt::Set_Stable<mtt::Thing_ID> sub_already_chosen = {};
            ch_args.already_chosen = &sub_already_chosen;
            auto result = handle_action(ch_args);
            ch_args.already_chosen = prev_already_chosen;
            
            switch (result.status) {
            case Exec_Status::OK:
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                result.value.ins->type = "RESPONSE";
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                result.value.ins->parent = ins;
                Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                //ins->children.push_back(result.value.ins);
                break;
            case Exec_Status::FAILED:
                break;
            }
        }
    };
    
    auto handle_end_conditions = [&]() {
        for (usize i = 0; i < end_condition_list->size(); i += 1) {
            Speech_Property& prop = *((*end_condition_list)[i]);
            
            auto ch_args = args;
            ch_args.prop = &prop;
            auto result = handle_action(ch_args);
            
            
            switch (result.status) {
            case Exec_Status::OK:
                args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                result.value.ins->type = "END_CONDITION";
                result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                result.value.ins->parent = ins;
                Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                //ins->children.push_back(result.value.ins);
                break;
            case Exec_Status::FAILED:
                break;
            }
        }
    };
    
    

    handle_triggers();
    handle_responses();
    if (end_condition_list != nullptr) {
        handle_end_conditions();
    }
    
    DT_scope_close();
    
    return exec_ok(ins);
}


Instruction* make_relation_instruction(Exec_Args& args, Instruction* ins, Speech_Property* prop)
{
    Instruction* rel_ins = Instruction::make();
    rel_ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
    Instruction_add_child(ins, rel_ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
    rel_ins->type = "RELATION";
    rel_ins->kind = prop->label;
    rel_ins->prop.push_back(prop);
    rel_ins->value.type = mtt::MTT_NONE;
    rel_ins->should_descend = 1;
    args.lang_ctx->eval_ctx.instructions().push_back(rel_ins);
    return rel_ins;
};

Exec_Result handle_thing_selection(Exec_Args& args)
{
    Word_Dictionary* dict = dt::word_dict();
    
    if (args.prop->annotation == "DO_NOT_SELECT_THING") {
        auto* ins = Instruction::make();
        ins->require_items_if_object = false;
        if (args.prop->kind_str == "THING_TYPE") {
            Instruction* ins = Instruction::make();
            ins->kind = "THING_TYPE_SELECTION";
            ins->type = args.prop->key;
            ins->annotation = "type=[" + args.prop->label + "]";
        } else if (args.prop->kind_str == "THING_INSTANCE") {
            ins->kind = "THING_INSTANCE_SELECTION";
            ins->type = args.prop->key;
        } else if (args.prop->kind_str == "VECTOR") {
            ins->kind = "VECTOR";
            ins->type = args.prop->key;
        } else if (args.prop->kind_str == "NUMERIC") {
            ins->kind = "NUMERIC";
            ins->type = args.prop->key;
        }else {
            ins->kind = "?";
            ins->type = args.prop->key;
        }
        
        for_each_value(*args.prop, [&](Speech_Property::Value& val) {
            for (const mtt::Thing_ID thing_id : ins->thing_id_list) {
                if (val.thing == thing_id) {
                    return;
                }
            }
            
            ins->thing_id_list.push_back(val.thing);
        });
        
        ins->prop.push_back(args.prop);
        return exec_ok(ins);
    }
    args.prop->was_preprocessed = true;
    
    
    using Prop_List = Speech_Property::Prop_List;
    

    DT_print("%s", __PRETTY_FUNCTION__);
     
    
    Prop_List*  count_prop   = nullptr;
    Prop_List*  plural_prop  = nullptr;
    Prop_List*  spec_prop    = nullptr;
    Prop_List*  property_prop = nullptr;
    usize       count_required = 1;
    bool        is_specific = false;
    bool        is_plural = false;
    
    usize found_count = 0;
    if (args.prop->try_get_prop("COUNT", &count_prop)) {
        found_count += 1;
        
        count_required = (usize)(((*count_prop)[0])->value.numeric);
    }
    if (args.prop->try_get_prop("SPECIFIC_OR_UNSPECIFIC", &spec_prop)) {
        
        is_specific = (bool)(((*spec_prop)[0])->value.flag);
        found_count += 1;
    } else if (args.prop->token && args.prop->token->pos == spacy_nlp::POS_PROPN) {
        is_specific = true;
        found_count += 1;
        count_required = 1;
    }

    if (args.prop->try_get_prop("PLURAL", &plural_prop)) {
        is_plural = (bool)(((*plural_prop)[0])->value.flag);
        
        found_count += 1;
    }
    
    Speech_Property* deixis_prop = nullptr;
    bool has_deixis = false;
    if (args.prop->try_get_only_prop("DEIXIS", &deixis_prop)) {
        has_deixis = true;
    }
    
    Speech_Property* negated_prop = nullptr;
    bool is_negated = false;
    if (args.prop->try_get_prop("PROPERTY", &property_prop)) {
        if (args.prop->parent != nullptr) {
            if (args.prop->parent->try_get_only_prop("NEGATED", &negated_prop)) {
                is_negated = negated_prop->value.flag;
            }
        }
    }
    
    Speech_Property::Prop_List* coref_prop_list = nullptr;
    if (args.prop->try_get_prop("COREFERENCE", &coref_prop_list) && !has_deixis) {
        dt::Dynamic_Array<dt::Instruction*> instructions;
        for (auto it = coref_prop_list->begin(); it != coref_prop_list->end(); ++it) {
            uintptr ref = (*it)->value.reference;
            if (ref == 0) {
                MTT_error("%s", "ref should not be 0!\n");
                continue;
            }
            
            Speech_Property* ref_prop = (Speech_Property*)ref;
            Instruction* ins = Instruction::make();
            instructions.push_back(ins);
            
            auto preprocess = [](Exec_Args::Deferred* args) -> bool {
                Speech_Property* ref_prop = args->ref_prop;
                
                mtt::World* mtt_world = mtt::ctx();

                if (!ref_prop->was_preprocessed) {
                    return false;
                }
                
                Instruction* ins = args->ins;
                
                Speech_Property* prop = args->prop;
                //prop->print();
                ins->instruction_ref = ref_prop->instruction;
                
                {
                    auto* rest_ins = ins->instruction_ref->referring_instruction_next;
                    ins->instruction_ref->referring_instruction_next = ins;
                    ins->referring_instruction_next = rest_ins;
                }
                
                if (ref_prop->kind_str == "THING_TYPE") {
                    ins->kind = "THING_TYPE_SELECTION";
                    args->prop->kind_str = "THING_TYPE";
                    args->prop->value.kind_string = "NONE";//"THING_TYPE";
                    //args->prop->value.text = ref_prop->label;
                    
                    ins->type = prop->key;
                    Prop_List* repl_prop;
                    //prop->try_get_prop("REPLACEMENT", &repl_prop);
                    Prop_List _ = Prop_List();
                    _.push_back(ref_prop);
                    repl_prop = &_;
                    ref_prop->instruction = ins;
                    ref_prop->print();
                    ins->prop.push_back(ref_prop);
                    return true;
                }
                
                ins->kind = "THING_INSTANCE_SELECTION";
                ins->type = prop->key;
                Prop_List* repl_prop;
                //prop->try_get_prop("REPLACEMENT", &repl_prop);
                Prop_List _ = Prop_List();
                _.push_back(ref_prop);
                repl_prop = &_;
                ref_prop->instruction = ins;
                //ref_prop->print();
                ins->prop.push_back(args->prop);
                args->prop->instruction = ins;
                
                
                for (auto r_it = repl_prop->begin(); r_it != repl_prop->end(); ++r_it) {
                    ins->prop.push_back(*r_it);
                    auto& val = (*r_it)->value;
                    mtt::String& kind_string = val.kind_string;
                    if (kind_string == "LIST") {
                        for (auto v_it = val.list.begin(); v_it != val.list.end(); ++v_it) {
                            auto& V = (*v_it);
                            mtt::Thing* t = mtt::Thing_try_get(mtt_world, V.thing);
                            if (t != nullptr && mtt::is_active(t)) {
                                ins->thing_id_list.push_back(V.thing);
                            }
                        }
                    } else if (kind_string == "THING_INSTANCE") {
                        mtt::Thing* t = mtt::Thing_try_get(mtt_world, val.thing);
                        if (t != nullptr && mtt::is_active(t)) {
                            ins->thing_id_list.push_back(val.thing);
                        }
                    } else {
                        MTT_error("SHOULD NOT BE ANYTHING ELSE NOW!, %s, %d\n", __FUNCTION__, __LINE__);
                    }
                    
                }
                
                if (prop->kind_str != "THING_INSTANCE") {
                    return true;
                }
                
                if (ins->thing_id_list.size() == 1) {
                    
                    auto* prop = args->prop;
                    
                    prop->value.kind_string = "THING_INSTANCE";
                    prop->value.thing = ins->thing_id_list.front();
                    
                } else if (ins->thing_id_list.size() > 1) {
                    
                    auto* prop = args->prop;
                    
                    prop->value.kind_string = "LIST";
                    prop->value.list.resize(ins->thing_id_list.size());
                    for (usize i = 0; i < ins->thing_id_list.size(); i += 1) {
                        prop->value.list[i] = Speech_Property::Value();
                        prop->value.list[i].thing = ins->thing_id_list[i];
                        prop->value.list[i].kind_string = "THING_INSTANCE";
                    }
                }
                
                
                
                return true;
            };
            
            Exec_Args::Deferred deferred;
            deferred.ins = ins;
            deferred.ref_prop = ref_prop;
            deferred.prop = args.prop;
            deferred.proc = preprocess;
            if (!ref_prop->was_preprocessed) {
                args.deferred->push_back(deferred);
            } else {
                preprocess(&deferred);
            }
        }
        
        if (instructions.empty()) {
            return exec_failed();
        } else if (instructions.size() == 1) {
            return exec_ok(instructions[0]);
        } else {
            for (usize i = 1; i < instructions.size(); i += 1) {
                instructions[0]->siblings.push_back(instructions[i]);
            }
            return exec_ok(instructions[0]);
        }
    }
    
    
    if (args.prop->kind_str == "THING_INSTANCE") {
    

        
        //ASSERT_MSG(found_count == 3, "All properties should exist for the simple examples\n");
                
        if (!args.dt->recorder.selection_recording.selections.empty() && (is_specific && has_deixis)) {
            mtt::String& label = args.prop->label;
            auto* word_entry = noun_lookup(label, "");
            if (word_entry != nullptr) {
                Instruction* ins = Instruction::make();
                ins->kind = "THING_INSTANCE_SELECTION";
                ins->type = args.prop->key;
                ins->prop.push_back(args.prop);
                args.prop->instruction = ins;
                
                ins->is_plural = deixis_is_plural(deixis_prop->label);
                ins->is_specific = true;

                usize deixis_i = *(args.deixis_i);
                //assert(args.dt->recorder.selection_recording.selections.size() != 0);
                deixis_i = (deixis_i > args.dt->recorder.selection_recording.selections.size()) ? args.dt->recorder.selection_recording.selections.size() : deixis_i;
                if (!args.dt->recorder.selection_recording.selections.empty()) {
                    auto it = args.dt->recorder.selection_recording.selections.begin() + deixis_i;
                    for (;it != args.dt->recorder.selection_recording.selections.end(); ++it) {
                        deixis_i += 1;
                        mtt::Thing* thing = nullptr;
                        if (!args.dt->mtt->Thing_try_get(it->ID, &thing)) {
                            continue;
                        }
                        if (mtt::Thing_is_proxy(thing)) {
                            thing = mtt::Thing_mapped_from_proxy(thing);
                            if (thing == nullptr) {
                                continue;
                            }
                        }
                        if (!is_derived_from(mtt::thing_id(thing), word_entry)) {
                            continue;
                        }
                        
                        
                        ins->thing_id_list.push_back(mtt::thing_id(thing));
                        
                        if (!ins->is_plural) {
                            break;
                        }
                        
                        if (found_count && ins->thing_id_list.size() == count_required && count_required > 1) {
                            break;
                        }
                    }
                }
                *(args.deixis_i) = deixis_i;
                if (ins->thing_id_list.empty()) {
                    Instruction::destroy(&ins);
                } else {
                    if (ins->thing_id_list.size() > 1) {
                        auto* world = args.dt->mtt;
                        args.prop->value.kind_string = "LIST";

                        args.prop->value.list.resize(ins->thing_id_list.size());
                        for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                            args.prop->value.list[val_i] = Speech_Property::Value();
                            args.prop->value.list[val_i].kind_string = "THING_INSTANCE";
                            args.prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                            args.already_chosen->insert(ins->thing_id_list[val_i]);
                        }
                        if (plural_prop != nullptr) {
                            ((*plural_prop)[0])->value.flag = true;
                        }
                    } else {
                        args.prop->value.kind_string = "THING_INSTANCE";
                        args.prop->value.thing = ins->thing_id_list.back();
                        args.already_chosen->insert(ins->thing_id_list.back());
                    }
                    if (count_prop != nullptr) {
                        (*count_prop->begin())->value.numeric = ins->thing_id_list.size();
                    }
                    args.prop->uses_deixis = true;
                    return exec_ok(ins);
                }
            }

        }
        
        if (true || is_specific || (args.prop->token != nullptr && args.prop->token->pos == spacy_nlp::POS_PROPN)) {
            // TODO: traverse for other relationships
            mtt::String& label = args.prop->label;
            
//            Instruction* ins = Instruction::make();
//            ins->kind = "THING_INSTANCE_SELECTION";
//            ins->type = args.prop->key;
//            ins->prop.push_back(args.prop);
//            args.prop->instruction = ins;

            {
                auto* mappings = &args.prop->candidate_selections->mappings;
                if (args.prop->candidate_selections != nullptr && args.prop->candidate_selections->mapping_count > 0) {
                    Instruction* ins = Instruction::make();
                    ins->kind = "THING_INSTANCE_SELECTION";
                    ins->type = args.prop->key;
                    ins->prop.push_back(args.prop);
                    args.prop->instruction = ins;
                    
                    ins->is_plural = is_plural;
                    ins->is_specific = is_specific;
                    
                    if (count_prop != nullptr) {
                        (*count_prop->begin())->value.numeric = args.prop->candidate_selections->mapping_count;
                    }
                    if (args.prop->candidate_selections->mapping_count > 1) {
                        if (plural_prop != nullptr) {
                            (*plural_prop->begin())->value.flag = true;
                        }
                        args.prop->value.kind_string = "LIST";
                        
                        
                        for (auto m_it = mappings->begin(); m_it != mappings->end(); ++m_it) {
                            if ((*m_it).second.first == false) {
                                continue;
                            }
                            ins->thing_id_list.push_back((*m_it).first);
                        }
                        
                        args.prop->value.list.resize(ins->thing_id_list.size());
                        for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                            args.prop->value.list[val_i] = Speech_Property::Value();
                            args.prop->value.list[val_i].kind_string = "THING_INSTANCE";
                            args.prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                        }
                        
                    } else {
                        for (auto m_it = mappings->begin(); m_it != mappings->end(); ++m_it) {
                            if ((*m_it).second.first == false) {
                                continue;
                            }
                            ins->thing_id_list.push_back((*m_it).first);
                        }
                        
                        if (ins->thing_id_list.size() == 1) {
                            args.prop->value.kind_string = "THING_INSTANCE";
                            args.prop->value.thing = ins->thing_id_list.back();
                        } else {
                            args.prop->value.kind_string = "LIST";
                            args.prop->value.list.resize(ins->thing_id_list.size());
                            for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                                args.prop->value.list[val_i] = Speech_Property::Value();
                                args.prop->value.list[val_i].kind_string = "THING_INSTANCE";
                                args.prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                            }
                            
                            if (plural_prop != nullptr) {
                                ((*plural_prop)[0])->value.flag = true;
                            }
                        }
                    }
                    

                    Speech_Property::Prop_List* relation_list = nullptr;
                    if (args.prop->try_get_prop("RELATION", &relation_list)) {
                        for (auto rel_it = relation_list->begin(); rel_it != relation_list->end(); ++rel_it) {
                            Speech_Property::Prop_List* object_list = nullptr;
                            if ((*rel_it)->try_get_prop("OBJECT", &object_list)) {
                                for (auto o_it = object_list->begin(); o_it != object_list->end(); ++o_it) {
                                    auto ch_args = args;
                                    ch_args.prop = *o_it;
                                    ch_args.is_relation = true;
                                    
                                    auto result = handle_thing_selection(ch_args);
                                    switch (result.status) {
                                    case Exec_Status::OK:
                                        if (result.value.ins == nullptr) {
                                            MTT_error("%s", "WARNING: instruction should exist!\n");
                                            break;
                                        }
                                        result.value.ins->parent = ins;
                                        //ins->children.push_back(result.value.ins);
                                        Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                                        result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                                        args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                                        if (!result.value.ins->siblings.empty()) {
                                            for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
                                                Instruction* s = *s_it;
                                                s->link_type = Instruction::PARENT_LINK_TYPE::TO;
                                                //ins->children.push_back(s);
                                                Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_LEFT);
                                                args.lang_ctx->eval_ctx.instructions().push_back(s);
                                                s->parent = ins;
                                            }
                                        }
                                        break;
                                    case Exec_Status::FAILED:
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    
                    
                    return exec_ok(ins);
                }
            }

            DT_Rule_Query search;
            
            auto* noun_entry = dt::noun_add(label);
            if (noun_entry != nullptr) {
                label = noun_entry->name;
                Instruction* ins = Instruction::make();
//                if (args.prop->annotation == "DO_NOT_REQUIRE_SELECTION") {
//                    ins->require_items_if_object = false;
//                }
                if (args.prop->ignore_selections) {
                    ins->require_items_if_object = false;
                }
                ins->prop.push_back(args.prop);
                args.prop->instruction = ins;
                
                ins->is_plural = is_plural;
                ins->is_specific = is_specific;
                
                bool property_of = false;
                bool property_of_ref = false;
                
                
                
                
                Speech_Property::Prop_List* relation_list = nullptr;
                if (args.prop->try_get_prop("RELATION", &relation_list)) {
                    for (auto rel_it = relation_list->begin(); rel_it != relation_list->end(); ++rel_it) {
                        //if ((*rel_it)->label == "of") {
                            property_of = true;
                        //}

//                        Instruction* rel_ins = make_relation_instruction(args, ins, *rel_it);
                        
                        
                        Speech_Property::Prop_List* coref_list = nullptr;
                        if ((*rel_it)->try_get_prop("COREFERENCE", &coref_list)) {
                            for (auto o_it = coref_list->begin(); o_it != coref_list->end(); ++o_it) {
                                
                                uintptr ref = (*o_it)->value.reference;
                                if (ref == 0) {
                                    MTT_error("%s", "ref should not be 0!\n");
                                    continue;
                                }
                                
                                property_of_ref = true;
                                args.is_relation = true;
                            }
                        }
                                
//                                auto result = handle_thing_selection(ch_args);
//                                switch (result.status) {
//                                    case Exec_Status::OK:
//                                        if (result.value.ins == nullptr) {
//                                            MTT_error("%s", "WARNING: instruction should exist!\n");
//                                            break;
//                                        }
//                                        //result.value.ins->parent = ins;
//                                        //ins->children.push_back(result.value.ins);
//                                        Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
//                                        result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
//                                        args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
//                                        if (!result.value.ins->siblings.empty()) {
//                                            for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
//                                                Instruction* s = *s_it;
//                                                s->link_type = Instruction::PARENT_LINK_TYPE::TO;
//                                                //ins->children.push_back(s);
//                                                Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_LEFT);
//                                                args.lang_ctx->eval_ctx.instructions().push_back(s);
//                                                s->parent = ins;
//                                            }
//                                        }
//                                        break;
//                                    case Exec_Status::FAILED:
//                                        break;
//                                }
                        
                        if (!property_of_ref) {
                            Speech_Property::Prop_List* object_list = nullptr;
                            if ((*rel_it)->try_get_prop("OBJECT", &object_list)) {
                                for (auto o_it = object_list->begin(); o_it != object_list->end(); ++o_it) {
                                    auto ch_args = args;
                                    ch_args.prop = *o_it;
                                    ch_args.is_relation = true;
                                    
                                    auto result = handle_thing_selection(ch_args);
                                    switch (result.status) {
                                        case Exec_Status::OK:
                                            if (result.value.ins == nullptr) {
                                                MTT_error("%s", "WARNING: instruction should exist!\n");
                                                break;
                                            }
                                            //result.value.ins->parent = ins;
                                            //ins->children.push_back(result.value.ins);
                                            Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                                            result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                                            args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                                            if (!result.value.ins->siblings.empty()) {
                                                for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
                                                    Instruction* s = *s_it;
                                                    s->link_type = Instruction::PARENT_LINK_TYPE::TO;
                                                    //ins->children.push_back(s);
                                                    Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_LEFT);
                                                    args.lang_ctx->eval_ctx.instructions().push_back(s);
                                                    s->parent = ins;
                                                }
                                            }
                                            break;
                                        case Exec_Status::FAILED:
                                            break;
                                    }
                                }
                            }
                        }
                    }
                }
                
                
                
                mtt::String searched_var =  args.get_query_ident();
                /*
                 Rule_Test(ecs_w->c_ptr(), "equivalent_to(EQUIVALENTTYPE, type.object), IsA(SUBTYPE, EQUIVALENTTYPE), SUBTYPE(INSTANCE)", ""),*/
                
                // try exact match
                //mtt::String query = "type." + label + "(X)";
                
                mtt::String query = "IsA(" MTT_Q_VAR_L("SUBTYPE")", " + ((label != "this") ? label : "thing") + "), " MTT_Q_VAR_L("SUBTYPE")"(" + MTT_Q_VAR_S(searched_var) + "), !Prefab(" + MTT_Q_VAR_S(searched_var) + ")";
                
                // mtt::String query = "!Prefab(_" + searched_var + "), " + label + "(_" + searched_var + ")";



                //mtt::String query = "IsA(_SUBTYPE, " + label + "), _SUBTYPE(" + "_" + searched_var + ")";
                
                if (property_prop != nullptr) {
                    for (auto it_prop = property_prop->begin(); it_prop != property_prop->end(); ++it_prop) {
                        auto* property = *it_prop;
                        if (property->label == "trait") {
                            if (!dict->is_ignored_attribute(property->value.text)) {
                                Speech_Property* n_prop = nullptr;
                                property->try_get_only_prop("NEGATED", &n_prop);
                                
                                dt::Speech_Property::Prop_List* modifiers = property->try_get_prop("PROPERTY");
#define DT_IGNORE_MODIFIERS (false)
                                if ((modifiers == nullptr || (DT_IGNORE_MODIFIERS))) {
                                    attribute_add(property->value.text);
                                    if (is_negated || (n_prop != nullptr && n_prop->value.flag == true)) {
                                        
                                        query += ", !IsA(" + MTT_Q_VAR_S(searched_var) + ", attribute." + property->value.text + ")";
                                        continue;
                                    }
                                    //auto* attrib = dt::attribute_add("type::" + property->value.text);
                                    query += ", IsA(" + MTT_Q_VAR_S(searched_var) + ", attribute." + property->value.text + ")";
                                    
                                } else {
                                    attribute_add(property->value.text);
                                    query ="IsA(" + MTT_Q_VAR_S(searched_var) + ", " + label + ")";
                                    if (is_negated || (n_prop != nullptr && n_prop->value.flag == true)) {
                                        //query += ", IsA(_" + searched_var + ", attribute." + property->value.text + ")";
                                        bool modifier_added = false;
                                        for (const Speech_Property* mod : *modifiers) {
                                            if (mod->label != "modifier") {
                                                continue;
                                            }
                                            
                                            modifier_added = true;
                                            
                                            query += ", !attribute." + property->value.text + "(" + MTT_Q_VAR_S(searched_var) + ", attribute." + mod->value.text + ")";
                                        }
                                        
                                        if (!modifier_added) {
                                            
                                            query += ", !IsA(" + MTT_Q_VAR_S(searched_var) + ", attribute." + property->value.text + ")";
                                        }
                                        
                                        continue;
                                    }
                                    
                                    //query += ", IsA(_" + searched_var + ", attribute." + property->value.text + ")";
                                    bool modifier_added = false;
                                    for (const Speech_Property* mod : *modifiers) {
                                        if (mod->label != "modifier") {
                                            continue;
                                        }
                                        
                                        modifier_added = true;
                                        
                                        query += ", attribute." + property->value.text + "(" + MTT_Q_VAR_S(searched_var) + ", attribute." + mod->value.text + ")";
                                    }
                                    
                                    if (!modifier_added) {
                                        query += ", IsA(" + MTT_Q_VAR_S(searched_var) + ", attribute." + property->value.text + ")";
                                    }
                                    
                                }
                            }
                        }
                    }
                }
                auto& query_ref = query;
                auto* out = DT_Rule_Query_make(args.dt->mtt, query, &search);

                if (out == nullptr) {
                    return exec_failed();
                }
                args.prop->search_rule = search;
                args.prop->search_rule.main_var = searched_var;
                args.prop->search_rule_builder.push_variable(MTT_Q_VAR_S(label));
                
                
                search.is_original = false;
                //args.prop->search_rule_builder.push_term({"_EQUIVALENTTYPE", "_" + label}, "equivalent_to");
                
                
                
                //print_rule_results_raw(args.dt->mtt, search.rule_ptr);
                {
                    ecs_world_t* ecs_world = args.dt->mtt->ecs_world.c_ptr();
                    cstring searched_var_cstring = searched_var.c_str();
                    bool it_done = false;
                    ecs_iter_t r_it = ecs_rule_iter(ecs_world, search.rule_ptr);
                    auto q_var = ecs_rule_find_var(search.rule_ptr, searched_var_cstring);
                    usize el_count = 0;
                    while (!it_done) {
                        if (!ecs_rule_next(&r_it)) {
                            it_done = true;
                            break;
                        }
                        
                        auto result_for_var = ecs_iter_get_var(&r_it, q_var);
                        auto ent = flecs::entity(r_it.world,  result_for_var);
                        if (ent.has(flecs::Prefab)) {
                            continue;
                        }
                        
                        if (ent.has<mtt::Thing_Info>()) {
                            mtt::Thing* thing = nullptr;
                            
                            if (!args.dt->mtt->Thing_try_get(ent.get<mtt::Thing_Info>()->thing_id, &thing)) {
                                continue;
                            }
                            MTT_print("ID=%llu\n", thing->id);
                            
                            if (args.already_chosen->find(thing->id) != args.already_chosen->end()) {
                                continue;
                            }
                            el_count += 1;
                            if (property_of)
                            if (property_of_ref) {
                                auto* world = args.dt->mtt;
                                auto& arrows = *mtt::arrow_links(world);
                                auto find_it = arrows.edges_reverse.find(thing->id);
                                mtt::Arrow_Link_List* arrow_list = nullptr;
                                if (find_it != arrows.edges_reverse.end()) {
                                    arrow_list = &find_it->second;
                                }
                                for (auto rel_it = relation_list->begin(); rel_it != relation_list->end(); ++rel_it) {
                                    Speech_Property::Prop_List* coref_list = nullptr;
                                    if ((*rel_it)->try_get_prop("COREFERENCE", &coref_list)) {
                                        for (auto o_it = coref_list->begin(); o_it != coref_list->end(); ++o_it) {
                                            
                                            uintptr ref = (*o_it)->value.reference;
                                            if (ref == 0) {
                                                MTT_error("%s", "ref should not be 0!\n");
                                                continue;
                                            }
                                            
                                            property_of_ref = true;
                                            
                                            Speech_Property* ref_prop = (Speech_Property*)ref;
                                            
                                            const Speech_Property::Value& value = ref_prop->value;
                                            const mtt::String& kind = value.kind_string;
                                            
                                            for_each_thing_instance_value(*ref_prop, [&](mtt::Thing* TH) {
                                                bool FOUND = false;
                                                if (mtt::exists_in_other_hierarchy(TH, thing)) {
                                                    FOR_ITER(BLA, ins->thing_id_list, ++BLA) {
                                                        if (*BLA == mtt::thing_id(thing)) {
                                                            FOUND = true;
                                                            return;
                                                        }
                                                    }
                                                    if (FOUND) {
                                                        return;
                                                    }
                                                    
                                                    ins->thing_id_list.push_back(thing->id);
                                                    //args.prop->search_rule = search;
                                                    //something_found = true;
                                                    
                                                    FOUND = true;
                                                }
                                                if (!FOUND) {
                                                    if (arrow_list != nullptr) {
                                                        for (usize arrow_e = 0; arrow_e < arrow_list->size(); arrow_e += 1) {
                                                            auto& arrow_el = (*arrow_list)[arrow_e];
                                                            if ((arrow_el.label == "have" || arrow_el.label == "own")) {
                                                                FOR_ITER(BLA, ins->thing_id_list, ++BLA) {
                                                                    if (*BLA == mtt::thing_id(thing)) {
                                                                        FOUND = true;
                                                                        break;
                                                                    }
                                                                }
                                                                if (FOUND == true) {
                                                                    return;
                                                                }
                                                                ins->thing_id_list.push_back(thing->id);
                                                                
                                                            }
                                                        }
                                                    }
                                                }
                                            });
                                        }
                                    }
                                }
                            } else {
                                bool something_found = false;
                                for (usize ch_list_i = 0; ch_list_i < ins->child_list_count(); ch_list_i += 1) {
                                    auto& ch_list = ins->child_list(ch_list_i);
                                    for (auto ch = ch_list.begin(); ch != ch_list.end(); ++ch) {
                                        auto* _ = *ch;
                                        for (auto sub = _->prop.begin(); sub != _->prop.end(); ++sub) {
                                            auto* __ = *sub;
                                            if (__->value.kind_string == "THING_INSTANCE") {
                                                mtt::Thing* TH = args.dt->mtt->Thing_try_get(__->value.thing);
                                                if (TH != nullptr) {
                                                    DT_print("%llu\n", TH->id);
                                                    if (mtt::is_ancestor_of(TH, thing)) {
                                                        //args.already_chosen->insert(thing->id);
                                                        bool FOUND = false;
                                                        FOR_ITER(BLA, ins->thing_id_list, ++BLA) {
                                                            if (*BLA == thing->id) {
                                                                FOUND = true;
                                                                break;
                                                            }
                                                        }
                                                        if (FOUND == true) {
                                                            continue;
                                                        }
                                                        ins->thing_id_list.push_back(thing->id);
                                                        args.prop->search_rule = search;
                                                        something_found = true;
                                                    }
                                                }
                                            } else if (__->value.kind_string == "LIST") {
                                                for (auto v_it = __->value.list.begin(); v_it != __->value.list.end(); ++v_it) {
                                                    
                                                    mtt::Thing* TH = args.dt->mtt->Thing_try_get((*v_it).thing);
                                                    if (TH != nullptr) {
                                                        if (mtt::is_ancestor_of(TH, thing)) {
                                                            bool FOUND = false;
                                                            FOR_ITER(BLA, ins->thing_id_list, ++BLA) {
                                                                if (*BLA == thing->id) {
                                                                    FOUND = true;
                                                                    break;
                                                                }
                                                            }
                                                            if (FOUND == true) {
                                                                continue;
                                                            }
                                                            args.already_chosen->insert(thing->id);
                                                            ins->thing_id_list.push_back(thing->id);
                                                            args.prop->search_rule = search;
                                                            something_found = true;
                                                        }
                                                    }
                                                }
                                            }
                                            
                                        }
                                    }
                                }
                                if constexpr ((false && !something_found)) {
                                    for (usize ch_list_i = 0; ch_list_i < ins->child_list_count(); ch_list_i += 1) {
                                        auto& ch_list = ins->child_list(ch_list_i);
                                        for (auto ch = ch_list.begin(); ch != ch_list.end(); ++ch) {
                                            auto* _ = *ch;
                                            for (auto sub = _->prop.begin(); sub != _->prop.end(); ++sub) {
                                                auto* __ = *sub;
                                                if (__->value.kind_string == "THING_INSTANCE") {
                                                    mtt::String& label = __->label;
                                                    {
                                                        auto& named_things = args.dt->lang_ctx.dictionary.thing_to_word;
                                                        if (mtt::true_for_some_ancestor(thing, [&](mtt::Thing* ancestor) -> bool {
                                                            auto it_find = named_things.find(ancestor->id);
                                                            if (it_find != named_things.end()) {
                                                                for (auto it_entry = it_find->second.begin(); it_entry != it_find->second.end(); ++it_entry) {
                                                                    auto* entry = (*it_entry);
                                                                    auto name = entry->name;
                                                                    //                                                                std::cout << thing->id << ":" << ancestor->id << ":" << name << std::endl;
                                                                    
                                                                    if ((*it_entry)->name == label) {
                                                                        return true;
                                                                    }
                                                                }
                                                            }
                                                            
                                                            return false;
                                                        })) {
                                                            bool FOUND = false;
                                                            FOR_ITER(BLA, ins->thing_id_list, ++BLA) {
                                                                if (*BLA == thing->id) {
                                                                    FOUND = true;
                                                                    break;
                                                                }
                                                            }
                                                            if (FOUND == true) {
                                                                continue;
                                                            }
                                                            //mtt::Thing_print(thing);
                                                            
                                                            args.already_chosen->insert(thing->id);
                                                            
                                                            ins->thing_id_list.push_back(thing->id);
                                                            args.prop->search_rule = search;
                                                            
                                                            
                                                        }
                                                    }
                                                    
                                                } else if (__->value.kind_string == "LIST") {
                                                    //for (auto v_it = __->value.list.begin(); v_it != __->value.list.end(); ++v_it) {
                                                    mtt::String& label = __->label;
                                                    {
                                                        auto& named_things = args.dt->lang_ctx.dictionary.thing_to_word;
                                                        if (mtt::true_for_some_ancestor(thing, [&](mtt::Thing* ancestor) -> bool {
                                                            auto it_find = named_things.find(ancestor->id);
                                                            if (it_find != named_things.end()) {
                                                                for (auto it_entry = it_find->second.begin(); it_entry != it_find->second.end(); ++it_entry) {
                                                                    if ((*it_entry)->name == label) {
                                                                        return true;
                                                                        
                                                                    }
                                                                }
                                                            }
                                                            
                                                            return false;
                                                        })) {
                                                            bool FOUND = false;
                                                            FOR_ITER(BLA, ins->thing_id_list, ++BLA) {
                                                                if (*BLA == thing->id) {
                                                                    FOUND = true;
                                                                    break;
                                                                }
                                                            }
                                                            if (FOUND == true) {
                                                                continue;
                                                            }
                                                            //mtt::Thing_print(thing);
                                                            
                                                            args.already_chosen->insert(thing->id);
                                                            
                                                            ins->thing_id_list.push_back(thing->id);
                                                            args.prop->search_rule = search;
                                                            
                                                            // break;
                                                        }
                                                    }
                                                    
                                                    //}
                                                }
                                                
                                            }
                                        }
                                    }
                                }
                            } else {
#ifndef NDEBUG
                                mtt::Thing_print(thing);
#endif
                                bool FOUND = false;
                                FOR_ITER(BLA, ins->thing_id_list, ++BLA) {
                                    if (*BLA == thing->id) {
                                        FOUND = true;
                                        break;
                                    }
                                }
                                if (FOUND == true) {
                                    continue;
                                }
                                args.already_chosen->insert(thing->id);
                                
                                ins->thing_id_list.push_back(thing->id);
                                args.prop->search_rule = search;
                                
                            }
                            
                            if (!args.is_relation) {
                                if (count_prop == nullptr && ins->thing_id_list.size() == 1 && !is_plural && is_specific) {
                                    break;
                                }
                                
                                if (is_specific) {
                                    if (is_plural && count_required == 1) {
                                        continue;
                                    } else if (count_required == ins->thing_id_list.size()) {
//                                        {
//                                            if (!ecs_rule_next(&r_it)) {
//                                                break;
//                                            }
//                                            
//                                            auto result_for_var = ecs_iter_get_var(&r_it, q_var);
//                                            auto ent = flecs::entity(r_it.world,  result_for_var);
//                                            if (!ent.has(flecs::Prefab) && ent.has<mtt::Thing_Info>()) {
//                                                mtt::Thing* thing = nullptr;
//                                                
//                                                if (args.dt->mtt->Thing_try_get(ent.get<mtt::Thing_Info>()->thing_id, &thing) &&
//                                                    args.already_chosen->find(thing->id) == args.already_chosen->end()) {
//                                                    ins->more_els_than_expected = true;
//                                                }
//                                            }
//                                            
//                                            
//                                        }
                                        break;
                                    }
                                }
                            }
                        }
                        
                    }
                }
                
                if (!args.treat_as_type && count_required > ins->thing_id_list.size() && count_prop != nullptr) {
                    int __TODO__ = 0;
                } else if (!is_specific) {
                    while (ins->thing_id_list.size() > count_required) {
                        auto to_remove_idx = MTT_Random_range(0, ins->thing_id_list.size());
                        ins->thing_id_list.erase(ins->thing_id_list.begin() + to_remove_idx);
                    }
                    
                    ins->refers_to_random = !(args.prop->token->pos == spacy_nlp::POS_PROPN);
                    args.prop->kind_str = "THING_TYPE";
                    ins->kind = "THING_TYPE_SELECTION";
                    ins->type = args.prop->key;
                    return exec_ok(ins);
                }

                
                if (ins->thing_id_list.empty()) {
                    ins->kind = "THING_INSTANCE_SELECTION";
                    ins->type = args.prop->key;
                } else if (count_required > ins->thing_id_list.size()) {
                    ins->kind = "THING_INSTANCE_SELECTION";
                    ins->type = args.prop->key;
                } else {
                    ins->kind = "THING_INSTANCE_SELECTION";
                    ins->type = args.prop->key;
                }
                
                if (ins->thing_id_list.empty()) {
                    args.prop->value.kind_string = "THING_INSTANCE";
                    args.prop->value.thing = mtt::Thing_ID_INVALID;
                } else if (ins->thing_id_list.size() == 1) {
                    args.prop->value.kind_string = "THING_INSTANCE";
                    args.prop->value.thing = ins->thing_id_list.front();
                } else {
                    args.prop->value.kind_string = "LIST";
                    args.prop->value.list.resize(ins->thing_id_list.size());
                    for (usize val_i = 0; val_i < ins->thing_id_list.size(); val_i += 1) {
                        args.prop->value.list[val_i] = Speech_Property::Value();
                        args.prop->value.list[val_i].kind_string = "THING_INSTANCE";
                        args.prop->value.list[val_i].thing = ins->thing_id_list[val_i];
                    }

                    if (plural_prop != nullptr) {
                        ((*plural_prop)[0])->value.flag = true;
                    }
                }
                

                return exec_ok(ins);
            } else {
                Instruction* ins = Instruction::make();
                ins->prop.push_back(args.prop);
                ins->kind = "THING_INSTANCE_SELECTION";
                ins->type = args.prop->key;
                args.prop->instruction = ins;
                
                ins->is_plural = is_plural;
                ins->is_specific = is_specific;
                
                Speech_Property::Prop_List* relation_list = nullptr;
                if (args.prop->try_get_prop("RELATION", &relation_list)) {
                    for (auto rel_it = relation_list->begin(); rel_it != relation_list->end(); ++rel_it) {
                        Speech_Property::Prop_List* object_list = nullptr;
                        if ((*rel_it)->try_get_prop("OBJECT", &object_list)) {
                            for (auto o_it = object_list->begin(); o_it != object_list->end(); ++o_it) {
                                auto ch_args = args;
                                ch_args.prop = *o_it;
                                
                                
                                auto result = handle_thing_selection(ch_args);
                                switch (result.status) {
                                case Exec_Status::OK:
                                    if (result.value.ins == nullptr) {
                                        MTT_error("%s", "WARNING: instruction should exist!\n");
                                        break;
                                    }
                                    result.value.ins->parent = ins;
                                    //ins->children.push_back(result.value.ins);
                                    Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                                    result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                                    args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                                    if (!result.value.ins->siblings.empty()) {
                                        for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
                                            Instruction* s = *s_it;
                                            s->link_type = Instruction::PARENT_LINK_TYPE::TO;
                                            //ins->children.push_back(s);
                                            Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_LEFT);
                                            args.lang_ctx->eval_ctx.instructions().push_back(s);
                                            s->parent = ins;
                                        }
                                    }
                                    break;
                                case Exec_Status::FAILED:
                                    break;
                                }
                            }
                        }
                    }
                }
                
                return exec_ok(ins);
            }
            

            
            // search
        } else {

            MTT_error("%s", "TODO unspecific things!\n");
            return exec_failed();
            
            
        }
        
    } else if (args.prop->kind_str == "THING_TYPE") {
        Instruction* ins = Instruction::make();
        ins->prop.push_back(args.prop);
        ins->kind = "THING_TYPE_SELECTION";
        ins->type = args.prop->key;
        ins->annotation = "type=[" + args.prop->label + "]";
        args.prop->instruction = ins;
        
        bool property_of = false;
        
        Speech_Property::Prop_List* relation_list = nullptr;
        if (args.prop->try_get_prop("RELATION", &relation_list)) {
            for (auto rel_it = relation_list->begin(); rel_it != relation_list->end(); ++rel_it) {
                //if ((*rel_it)->label == "of") {
                    property_of = true;
                //}
                
                Instruction* rel_ins = make_relation_instruction(args, ins, *rel_it);
                
                Speech_Property::Prop_List* object_list = nullptr;
                if ((*rel_it)->try_get_prop("OBJECT", &object_list)) {
                    for (auto o_it = object_list->begin(); o_it != object_list->end(); ++o_it) {
                        auto ch_args = args;
                        ch_args.prop = *o_it;
                        ch_args.is_relation = true;
                        
                        auto result = handle_thing_selection(ch_args);
                        switch (result.status) {
                        case Exec_Status::OK:
                            if (result.value.ins == nullptr) {
                                MTT_error("%s", "WARNING: instruction should exist!\n");
                                break;
                            }
                            result.value.ins->parent = ins;
                            //ins->children.push_back(result.value.ins);
                            Instruction_add_child(ins, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                            result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                            args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                            if (!result.value.ins->siblings.empty()) {
                                for (auto s_it = result.value.ins->siblings.begin(); s_it != result.value.ins->siblings.end(); ++s_it) {
                                    Instruction* s = *s_it;
                                    s->link_type = Instruction::PARENT_LINK_TYPE::TO;
                                    //ins->children.push_back(s);
                                    Instruction_add_child(ins, s, Instruction_CHILD_SIDE_TYPE_LEFT);
                                    args.lang_ctx->eval_ctx.instructions().push_back(s);
                                    s->parent = ins;
                                }
                            }
                            
                            break;
                        case Exec_Status::FAILED:
                            break;
                        }
                    }
                }
            }
        }
        
        
        
        
//        Prop_List* props = nullptr;
//        if (args.prop->try_get_prop("PROPERTY", &props)) {
//            for (auto p_it = props->begin(); p_it != props->end(); ++p_it) {
//                auto* prop = *p_it;
//                if (prop->kind_str == "THING_TYPE") {
//                    Instruction* ins = Instruction::make();
//                    ins->prop.push_back(prop);
//                    ins->kind = "THING_TYPE_SELECTION";
//                    ins->type = args.prop->key;
//                    ins->annotation = "type=[" + args.prop->label + "]";
//
//                    args.lang_ctx->eval_ctx.instructions.push_back(ins);
//                }
//            }
//        }
        
        return exec_ok(ins);
        //ASSERT_MSG(false, "Not handling types of things just yet!\n");
    } else {
        MTT_error("%s", "ERROR: unrecognized\n");
        return exec_failed();
        //ASSERT_MSG(false, "%s:%s", args.prop->type_str.c_str(), args.prop->kind_str.c_str());
    }
    
    

    
    return exec_ok(nullptr);
}



Exec_Result build_confirmation_instruction_list(Exec_Args& args)
{
    args.prop->was_preprocessed = true;

    std::vector<Speech_Property*>* cmd_list = nullptr;
    if (!args.root->try_get_prop("CMD_LIST", &cmd_list)) {
        return exec_failed();
    }
    
    for (usize c = 0; c < cmd_list->size(); c += 1) {
        Speech_Property* cmd = (*cmd_list)[c];
        args.root_cmd = cmd;
        
        std::vector<Speech_Property *>* action_list = nullptr;
        std::vector<Speech_Property *>* trigger_response_list = nullptr;
        args.already_chosen->clear();
        
        if (args.root_cmd->try_get_prop("ACTION", &action_list)) {
            for (usize i = 0; i < action_list->size(); i += 1) {
                Speech_Property& prop = *((*action_list)[i]);
                
                auto ch_args = args;
                ch_args.prop = &prop;
                auto result = handle_action(ch_args);
                switch (result.status) {
                    
                case Exec_Status::OK:
                    args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                    
                    if (result.value.ins->child_count() == 0 && i > 0) {
                        auto* other_instruction = (Instruction*)((*action_list)[i - 1]->instruction);
                        result.value.ins->parent = other_instruction;
                        //other_instruction->children.push_back(result.value.ins);
                        Instruction_add_child(other_instruction, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                        result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                    }
                    break;
                case Exec_Status::FAILED:
                    
                    break;
                }
            }
        }
        if (args.root_cmd->try_get_prop("TRIGGER_RESPONSE", &trigger_response_list)) {
            for (usize i = 0; i < trigger_response_list->size(); i += 1) {
                Speech_Property& prop = *((*trigger_response_list)[i]);
                
                auto ch_args = args;
                ch_args.prop = &prop;
                auto result = handle_trigger_response(ch_args);
                switch (result.status) {
                    
                case Exec_Status::OK:
                    args.lang_ctx->eval_ctx.instructions().push_back(result.value.ins);
                    
                    if (result.value.ins->child_count() == 0 && i > 0) {
                        auto* other_instruction = (Instruction*)((*action_list)[i - 1]->instruction);
                        result.value.ins->parent = other_instruction;
                        //other_instruction->children.push_back(result.value.ins);
                        Instruction_add_child(other_instruction, result.value.ins, Instruction_CHILD_SIDE_TYPE_RIGHT);
                        result.value.ins->link_type = Instruction::PARENT_LINK_TYPE::FROM;
                    }
                    break;
                case Exec_Status::FAILED:
                    
                    break;
                }
            }
        }
        
    }

    return exec_ok(nullptr);

}
}

mtt::String debug_msg = {};

void lang_eval_compiled(dt::DrawTalk* dt, dt::Parse* parse, Speech_Property* comp)
{
    if (comp == nullptr) {
        if (dt->is_waiting_on_results) {
            dt->is_waiting_on_results = false;
            auto* speech_info = &dt->core->speech_system.info_list[0];
            dt::text_view_clear(dt);
            dt::set_confirm_phase(dt::CONFIRM_PHASE_SPEECH);
            Speech_set_active_state(speech_info, true);
        }
        return;
    }
    
    usize counter = 0;
    usize deixis_i = 0;
    ex::Exec_Args& args = dt->lang_ctx.eval_ctx.args; {
        args.dt = dt;
        args.lang_ctx = &dt->lang_ctx;
        args.parse = parse;
        args.prop = comp;
        args.root = comp;
        args.root_cmd = nullptr;
        args.ident_counter = &counter;
        args.deixis_i = &deixis_i;
    }
    mtt::Set_Stable<mtt::Thing_ID> already_chosen;
    args.already_chosen = &already_chosen;
    
    dt::Dynamic_Array<ex::Exec_Args::Deferred> deferred;
    args.deferred = &deferred;
    
    auto resolve_coreferences = [&](Parse* parse) {
        auto& referrer_map = dt->lang_ctx.eval_ctx.referrer_map;
        referrer_map.clear();
        auto& referrer_map_reversed = dt->lang_ctx.eval_ctx.referrer_map_reversed;
        referrer_map_reversed.clear();
        MTT_print("Coreferences: %s\n", parse->pretty_coref_representation.c_str());
        
        for (usize i = 0; i < parse->token_count(); i += 1) {
            auto* tok = parse->token_list[i];
            
            if (tok->coreference.token_ref_list.empty()) {
                continue;
            }
            
            auto& coref_list = tok->coreference.token_ref_list;
            
            Speech_Property* referrer_prop = tok->prop_ref;
            if (referrer_prop == nullptr) {
                MTT_error("%s", "WARNING: token with coreference has no associated property!\n");
                continue;
            }
            
            for (usize cr = 0; cr < coref_list.size(); cr += 1) {
                Speech_Property* referenced_prop = coref_list[cr]->prop_ref;
                if (referenced_prop == nullptr ||
                    (referenced_prop->token != nullptr &&
                     referenced_prop->token->parse != nullptr &&
                     referenced_prop->token->parse->is_discarded())
                     ) {
                    MTT_error("%s", "WARNING: coreference has no associated property!\n");
                    continue;
                } else if ((referrer_prop->candidate_selections != nullptr && referrer_prop->candidate_selections->mapping_count > 0)) {
                    MTT_print("%s", "Ignoring coreference due to manual mapping!\n");
                    continue;
                } else {
                    
//                    if (referrer_prop->candidate_selections != nullptr && referrer_prop->candidate_selections->mapping_count > 0) {
//                        if (referenced_prop->value.kind_string == "THING_INSTANCE") {
//                            mtt::Thing_ID ref_id = referenced_prop->value.thing;
//                            if (dt->mtt->Thing_try_get(ref_id) != nullptr) {
//                                if (referrer_prop->candidate_selections->mappings.find(ref_id) == referrer_prop->candidate_selections->mappings.end()) {
//
//                                    // don't treat as a coreference
//                                    continue;
//                                }
//                            }
//                        } else if (referrer_prop->value.kind_string == "LIST") {
//                            bool anything_is_consistent = false;
//                            for (auto it_list = referenced_prop->value.list.begin(); it_list != referenced_prop->value.list.end(); ++it_list) {
//
//                            }
//                        }
//                    }
                    
                    
                    auto* coref_prop = referrer_prop->push_prop("COREFERENCE");
                    //referrer_prop->print();
                    std::vector<Speech_Property*>* corefs;
                    if (referenced_prop->try_get_prop("COREFERENCE", &corefs)) {
                        // disallow nested coreferences

                        Speech_Property* nested_coref = (Speech_Property*)((*corefs->begin())->value.reference);
                        //nested_coref->print();
                        //referenced_prop->print();
                        
                        Speech_Property* clone = nested_coref->copy();
                        clone->token = nullptr;
                        //coref_prop->value.reference = (uintptr)clone;
                        
                        
//                        referenced_prop->print();
//                        referrer_prop->print();
//                        coref_prop->print();
//                        clone->print();
                        
                        
                        referenced_prop = clone;
                        coref_list[cr]->prop_ref = clone;
                        //referenced_prop->push_prop("COREFERENCE", clone);
                        
                    }
                    //auto* copy = referenced_prop->copy();
                    //referrer_prop->annotation = "use_coreference";
                    //referrer_prop->push_prop("REPLACEMENT", copy);
                    
                    //auto* coref_prop = referrer_prop->push_prop("COREFERENCE", referenced_prop->copy());
                    coref_prop->annotation = "coreference";
                    coref_prop->value.kind_string = "REFERENCE";
                    coref_prop->value.is_reference = true;
                    referenced_prop->value.is_referenced = true;
                    //referenced_prop->referenced_by = referrer_prop;
                    coref_prop->value.reference = (uintptr)referenced_prop;
                    
                    {
                        auto find_it = referrer_map.find(referenced_prop);
                        if (find_it == referrer_map.end()) {
                            referrer_map.insert({referenced_prop, (std::vector<Speech_Property*>){coref_prop} });
                        } else {
                            find_it->second.push_back(coref_prop);
                        }
                    }
                    {
                        auto find_it = referrer_map_reversed.find(coref_prop);
                        if (find_it == referrer_map_reversed.end()) {
                            referrer_map_reversed.insert({coref_prop, (std::vector<Speech_Property*>){referenced_prop}});
                        } else {
                            find_it->second.push_back(referenced_prop);
                        }
                    }
                    
                    
                    

                    
//                    Instruction& ins = *Instruction::make();
//                    ins.type = "THING_INSTANCE_SELECTION";
//                    ins.kind = "CONFIRM_SELECTION";
//                    ins.annotation = "possible_coreference";
//                    
//                    ins.prop.push_back(referrer_prop);
//                    ins.value.type = mtt::MTT_NONE;
//                    args.lang_ctx->eval_ctx.instructions.push_back(&ins);
                }
            }
        }
        // TODO: remove print-out
        {
            for (const auto& [prop, prop_list] : referrer_map) {
                for (const auto& referrer : prop_list) {
                    MTT_print("%s %p -> %p \n", prop->label.c_str(), prop, (Speech_Property*)referrer->value.reference);
                }
            }
        }
    };

    //dt::debug_msg = "";
    if (parse->is_finished()) {
        
        for (usize i = 0; i < dt->lang_ctx.eval_ctx.instructions().size(); i += 1) {
            mtt::Thing_destroy(dt->mtt, dt->lang_ctx.eval_ctx.instructions()[i]->thing_id);
            dt::Instruction_on_destroy(dt, dt->lang_ctx.eval_ctx.instructions()[i]);
        }
        dt->lang_ctx.eval_ctx.instructions().clear();
        dt->lang_ctx.eval_ctx.instructions_to_disambiguate.clear();
        //MTT_BP();
        //parse->cmds->print();
        resolve_coreferences(parse);
        //parse->cmds->print();
        //MTT_BP();
        
        

        
        ex::Exec_Result res =
        ex::build_confirmation_instruction_list(args);
        


        switch (res.status) {
        case ex::Exec_Status::OK: {
            
            bool made_progress = false;
            while (!deferred.empty() && made_progress) {
                made_progress = false;
                for (auto it = deferred.begin(); it != deferred.end(); ) {
                    auto& deferred_process = *it;
                    bool status = deferred_process.proc(&deferred_process);
                    if (status == false) {
                        ++it;
                        continue;
                    }
                
                    it = deferred.erase(it);
                    made_progress = true;
                }
            }
            

            
            args.lang_ctx->eval_ctx.instructions_saved() = args.lang_ctx->eval_ctx.instructions();
            args.lang_ctx->eval_ctx.instructions().clear();
            
            usize feedback_instruction_count = args.lang_ctx->eval_ctx.instructions_saved().size();
            for (usize i = 0; i < feedback_instruction_count; i += 1) {
                //MTT_print("%s\n", args.lang_ctx->eval_ctx.instrucu8]tions[i]->to_string().c_str());
                dt::debug_msg += args.lang_ctx->eval_ctx.instructions_saved()[i]->to_string() + "\n";
            }
            
            if (parse->is_finished()) {
                mtt::Camera_reset(&dt->ui.top_panel.cam);
            }
            

            //assert(mtt::is_main_thread());
            if (feedback_instruction_count == 0) {
                
                set_confirm_phase(CONFIRM_PHASE_SPEECH);
                dt->is_waiting_on_results = false;
                MTT_error("%s", "Nothing?\n");
                break;
            }
            
            
            build_feedback_ui_from_instructions(dt, args.lang_ctx->eval_ctx);

            if (parse->is_finished()) {
                comp->print();
            }
            
            
            set_confirm_phase(CONFIRM_PHASE_ANIMATE);
            
            
            dt->is_waiting_on_results = false;
            
            //dt->deferred_confirm();
            
            
            
            
           
            
            break;
        }
        case ex::Exec_Status::FAILED: {
            set_confirm_phase(CONFIRM_PHASE_SPEECH);
            break;
        }
        }
        
        DT_print("Exec_Status: %s\n", ex::Exec_Status_Strings[(uint64)res.status]);
    }


    
}

vec2 update_selection_rectangles(vec2 canvas_pos, Panel& panel, Selection_Rectangle& rect) {
    canvas_pos = m::max(panel.bounds.tl, m::min(canvas_pos, panel.bounds.tl + panel.bounds.dimensions));
    
    rect.curr = canvas_pos;
    rect.end = canvas_pos;
    
    vec2 minimum = vec2(POSITIVE_INFINITY);
    vec2 maximum = vec2(NEGATIVE_INFINITY);
    using Length_Type = vec2::length_type;
    for (Length_Type i = 0; i < 2; i += 1) {
        if (rect.init_origin[i] >= rect.end[i]) {
            minimum[i] = rect.end[i];
            maximum[i] = rect.init_origin[i];
        } else {
            minimum[i] = rect.init_origin[i];
            maximum[i] = rect.end[i];
        }
    }
    
    rect.start = minimum;
    rect.end   = maximum;
    
    rect.extent = rect.end - rect.start;
    rect.area = rect.extent.x * rect.extent.y;
    
    return canvas_pos;
};


sd::Drawable_Info* filter = nullptr;

DrawTalk* DrawTalk::global_ctx = nullptr;
void DrawTalk_init(MTT_Core* core, mtt::World* world, DrawTalk* dt)
{
    DrawTalk::global_ctx = dt;
    
    dt->core = core;
    dt->mtt = world;
    dt->recorder.selection_recording.idx_playhead = 0;
    
    
    dt->mode = 0;
    
    // system interface
    
    Pool_Allocation_init(&dt->parse_allocation,         core->allocator, 4096, sizeof(dt::Parse), 16);
    Pool_Allocation_init(&dt->token_allocation,         core->allocator, 4096, sizeof(dt::Speech_Token), 16);
    Pool_Allocation_init(&dt->list_node_allocation,     core->allocator, 1024, sizeof(MTT_List_Node), 16);
    Pool_Allocation_init(&dt->tree_node_allocation,     core->allocator, 1024, sizeof(MTT_Tree_Node), 16);
    
    Pool_Allocation_init(&dt->instructions, core->allocator, 2048, sizeof(dt::Speech_Property), 16);
    
    Pool_Allocation_init(&dt->rules,        core->allocator, 512, sizeof(dt::rules::Rule), 16);
    Pool_Allocation_init(&dt->triggers,     core->allocator, 512, sizeof(dt::rules::Trigger), 16);
    Pool_Allocation_init(&dt->trigger_clauses,     core->allocator, 512, sizeof(dt::rules::Trigger::Clause), 16);
    Pool_Allocation_init(&dt->responses,    core->allocator, 512, sizeof(dt::rules::Response), 16);
    Pool_Allocation_init(&dt->response_clauses,     core->allocator, 512, sizeof(dt::rules::Response::Clause), 16);
    Pool_Allocation_init(&dt->rule_variables,     core->allocator, 512, sizeof(dt::rules::Rule_Variable), 16);
    
    Pool_Allocation_init(&dt->bg_ctx_data,  core->allocator, 64,   sizeof(dt::BG_Data_Context), 16);
    
    Pool_Allocation_init(&Instruction::pool, core->allocator, 64, sizeof(Instruction), 16);
    
    
    dt_Instruction_label = mtt::string("ct.Instruction");
    
    
    Pool_Allocation_init(&UI_Feedback_Node::pool, core->allocator, 1024, sizeof(dt::UI_Feedback_Node), 16);
    
    Pool_Allocation_init(&Command::pool, core->allocator, 4096, sizeof(dt::Command), 16);
    
    MTT_Logger_init(&dt->parse_logger, "drawtalk", "commands");
    
    Behavior_Catalogue_init(dt, &dt->lang_ctx.behaviors);
    set_active_behavior_catalogue(&dt->lang_ctx.behaviors);
    
    fmt::set_ctx(&dt->formatter);
    
    auto* dict = word_dict();
    dt::Word_Dictionary_init(dict);
    
    //dt::tree_init(ct);
    

    
    
    
#if 0
    if constexpr ((false))
    {
        MTT_Tree_Node* found = search(MTT_Tree_root(&dt->sys_tree), "en").search("jump").search(dt::sense_to_roleset_id["jump"]["physical"]).search("onto");
        assert(found != nullptr);
        MTT_print("%s\n", static_cast<dt::Behavior_Block*>(found->data)->label.c_str());
        
        struct Temp_Context {
            dt::DrawTalk* dt;
            usize tab_count;
        } temp_ctx = {dt, 0};
        MTT_Tree_depth_traversal(&dt->sys_tree, MTT_Tree_root(&dt->sys_tree), [](auto* tree, auto* node, auto* data) -> void {
            //uintv id = (uintptr)node->data;
            Temp_Context* ctx = (Temp_Context*)data;
            DrawTalk* dt = ctx->dt;
            Behavior_Block* block = (Behavior_Block*)node->data;
            
            const auto entab = (ctx->tab_count > 0) ? std::string(ctx->tab_count * 4, ' ') : std::string("");
            MTT_print("%stypename=[%s] label=[%s] child_count=[%llu] {\n", entab.c_str(), dt->lang_ctx.behaviors.type_info.list[block->type].name.c_str(),
                      block->label.c_str(), static_cast<MTT_Tree_Node*>(node)->child_list.count);
            
            ctx->tab_count += 1;
            
        },
                                 [](auto* tree, auto* node, auto* data) -> void {
            Temp_Context* ctx = (Temp_Context*)data;
            ctx->tab_count -= 1;
            const auto entab = (ctx->tab_count > 0) ? std::string(ctx->tab_count * 4, ' ') : std::string("");
            MTT_print("%s}\n", entab.c_str());
            
        }, (void*)&temp_ctx);
        //        MTT_Tree_postorder_traversal(&dt->sys_tree, MTT_Tree_root(&dt->sys_tree), [](auto* tree, auto* node, auto* data) -> void {
        //            //uintptr id = (uintptr)node->data;
        //
        //            //MTT_print("Tree Node postorder id=[%lu]\n", id);
        //        }, NULL);
        
        //MTT_Tree_destroy(&dt->sys_tree);
    }
#endif
    
    
    
    
    //    Memory_Pool_Fixed_init(&dt->node_allocation.node_pool, core->allocator, 512, sizeof(dt::Behavior_Block), 16);
    //    dt->node_allocation.node_allocator = mem::Memory_Pool_Fixed_Allocator(&dt->node_allocation.node_pool);
    //
    //
    //    auto& language_ctx = dt->lang_ctx;
    //
    //    auto* block = Behavior_Block_make(ct);
    //    assert(block != nullptr);
    //
    //    block->state = std::vector<std::vector<int>>();
    //    auto& out = std::get<std::vector<std::vector<int>>>(block->state);
    //    out.push_back({2});
    //    Behavior_Block_destroy(dt, block);
    //
    //    // tokens
    //    Memory_Pool_Fixed_init(&language_ctx.token_allocation.dep_token_pool, core->allocator, 512, sizeof(dt::Dependency_Token), 16);
    //    language_ctx.token_allocation.dep_token_allocator = mem::Memory_Pool_Fixed_Allocator(&language_ctx.token_allocation.dep_token_pool);
    //
    //    Dependency_Token* tok = mem::allocate<Dependency_Token>(&language_ctx.token_allocation.dep_token_allocator);
    //
    //    tok->id = 256;
    //
    //    mem::deallocate<Dependency_Token>(&language_ctx.token_allocation.dep_token_allocator, tok);
    
    flecs::entity IS_ATTRIBUTE_TAG = flecs::entity(world->ecs_world, "attribute");
    world->IS_ATTRIBUTE_TAG = IS_ATTRIBUTE_TAG;
    
    verb_add("become");
    
    Word_Dictionary_Entry* v_collide = verb_add("collide");
    v_collide->verb_is_bidirectional_by_default = true;
    v_collide->verb_class = VERB_CLASS_STATE_CHANGE;
    v_collide->verb_should_check_all_equivalencies = true;
    
    world->collide = v_collide;
    world->collide_tag = world->collide->typename_desc;
    world->collide_begin_tag = Word_Dictionary_Entry_event_for_verb(v_collide, VERB_EVENT_BEGIN);
    world->collide_end_tag = Word_Dictionary_Entry_event_for_verb(v_collide, VERB_EVENT_END);
    
    Word_Dictionary_Entry* v_collide_with = verb_add("collidewith");
    v_collide_with->verb_is_bidirectional_by_default = true;
    v_collide_with->verb_class = VERB_CLASS_STATE_CHANGE;
    v_collide_with->verb_should_check_all_equivalencies = true;
    
    Word_Dictionary_Entry* v_collide_against = verb_add("collideagainst");
    v_collide_against->verb_is_bidirectional_by_default = true;
    v_collide_against->verb_class = VERB_CLASS_STATE_CHANGE;
    v_collide_against->verb_should_check_all_equivalencies = true;
    
    Word_Dictionary_Entry* v_collide_between = verb_add("collidebetween");
    v_collide_between->verb_is_bidirectional_by_default = true;
    v_collide_between->verb_class = VERB_CLASS_STATE_CHANGE;
    v_collide_between->verb_should_check_all_equivalencies = true;
    
    world->collide = v_collide;
    
    verb_make_equivalent_to(v_collide_against, v_collide);
    
    world->select = verb_add("select");
    world->select_tag = world->select->typename_desc;
    world->select_begin_tag = Word_Dictionary_Entry_event_for_verb(world->select, VERB_EVENT_BEGIN);
    world->select_end_tag = Word_Dictionary_Entry_event_for_verb(world->select, VERB_EVENT_END);
    
    auto* v_press = verb_add("press");
    verb_make_equivalent_to(v_press, world->select);
    
    auto* v_deselect = verb_add("deselect");
    auto* v_unpress = verb_add("unpress");
    auto* v_release = verb_add("release");
    verb_make_equivalent_to(v_release, v_deselect);
    
    Word_Dictionary_Entry* v_overlap = verb_add("overlap");
    v_overlap->verb_is_bidirectional_by_default = true;
    v_overlap->verb_class = VERB_CLASS_STATE_CONTINUOUS;
    v_overlap->verb_should_check_all_equivalencies = true;
    
    world->overlap = v_overlap;
    world->overlap_tag = world->overlap->typename_desc;
    world->overlap_begin_tag = Word_Dictionary_Entry_event_for_verb(world->overlap, VERB_EVENT_BEGIN);
    world->overlap_end_tag = Word_Dictionary_Entry_event_for_verb(world->overlap, VERB_EVENT_END);
    
    Word_Dictionary_Entry* v_exit = verb_add("exit");
    v_overlap->verb_is_bidirectional_by_default = false;
    v_overlap->verb_class = VERB_CLASS_STATE_CHANGE;
    v_overlap->verb_should_check_all_equivalencies = true;
    
    Word_Dictionary_Entry* v_paint = verb_add("paint");
    
    //world->exit = v_exit;
    
    //auto* a_collision = dt::attribute_add("type::collide");
    //world->collision_between = dt::attribute_add("type::collision_between");
    //auto* a_collide_with    = dt::attribute_add("type::collide_with");
    //auto* a_collide_against = dt::attribute_add("type::collide_against");
    
    world->equivalent_to = verb_add("equivalent_to");
    world->equivalent_to->typename_desc.add(flecs::Transitive);
    world->equivalent_to->typename_desc.add(flecs::Reflexive);
    //    a_collision->typename_desc.add(equivalent_to, a_collide_with->typename_desc);
    //    a_collision->typename_desc.add(equivalent_to, a_collide_against->typename_desc);
    //    a_collision->typename_desc.add(equivalent_to, world->collision_between->typename_desc);
    
    
    //attribute_derive_from(world->collision_between, a_collision);
    verb_make_equivalent_to(v_collide_with, v_collide);
    verb_make_equivalent_to(v_collide_between, v_collide);
    verb_make_equivalent_to(v_collide_against, v_collide);
    
    
//    assert(v_collide_with->typename_desc.has(flecs::IsA, v_collide->typename_desc) &&
//           v_collide_against->typename_desc.has(flecs::IsA,
//                                                v_collide->typename_desc) &&
//           v_collide_between->typename_desc.has(flecs::IsA,
//                                                v_collide->typename_desc)
//           );
    
    auto* ecs_w = &dt->mtt->ecs_world;
    
    //auto* id_is = attribute_add("id_is");
    
    flecs::entity(*ecs_w, "IsDoingAction");
    flecs::entity(*ecs_w, "IsReceivingAction");
    
    Word_Dictionary_Entry* n_thing   = noun_add("thing");
    //n_thing->should_not_visualize  = true;
    Word_Dictionary_Entry* n_entity = noun_add("entity");
    //n_entity->should_not_visualize = true;
    //noun_make_equivalent_to(n_entity, n_thing);
    //noun_derive_from(n_entity, n_thing);
    
    Word_Dictionary_Entry* n_object = noun_add("object");
    //n_object->should_not_visualize = true;
    //noun_derive_from(n_object, n_entity);
    //noun_make_equivalent_to(n_object, n_thing);
    
//    Word_Dictionary_Entry* n_concept = noun_add("concept");
//    noun_derive_from(n_concept, n_thing);
//    
//    Word_Dictionary_Entry* n_art              = noun_add("art");
//    noun_derive_from(n_art, n_concept);
//    
//    Word_Dictionary_Entry* n_artwork          = noun_add("artwork");
//    noun_derive_from(n_artwork, n_art);
//    
//    Word_Dictionary_Entry* n_drawing          = noun_add("drawing");
//    noun_derive_from(n_drawing, n_artwork);
//    
//    Word_Dictionary_Entry* n_sketch           = noun_add("sketch");
//    noun_derive_from(n_sketch, n_drawing);
//    
//    Word_Dictionary_Entry* n_painting           = noun_add("painting");
//    noun_derive_from(n_painting, n_artwork);
//    
//    Word_Dictionary_Entry* n_motion_picture = noun_add("motion_picture");
//    noun_derive_from(n_motion_picture, n_artwork);
//    
//    Word_Dictionary_Entry* n_physics              = noun_add("physics");
//    noun_derive_from(n_physics, n_concept);
//    
//    Word_Dictionary_Entry* n_person = noun_add("person");
//    noun_derive_from(n_person, n_thing);
//    
//    Word_Dictionary_Entry* n_artist = noun_add("artist");
//    noun_derive_from(n_artist, n_person);
//    
//    
//    Word_Dictionary_Entry* n_matter = noun_add("matter");
//    noun_derive_from(n_matter, n_thing);
//    Word_Dictionary_Entry* n_antimatter = noun_add("anti_matter");
//    noun_derive_from(n_antimatter, n_thing);
//    Word_Dictionary_Entry* n_tree = noun_add("tree");
//    noun_derive_from(n_tree, n_thing);
//    
//    Word_Dictionary_Entry* n_explosion = noun_add("explosion");
//    noun_derive_from(n_explosion, n_thing);
//    
//    Word_Dictionary_Entry* n_fire = noun_add("fire");
//    noun_derive_from(n_fire, n_thing);
//    
//    
//    Word_Dictionary_Entry* n_dog = noun_add("dog");
//    Word_Dictionary_Entry* n_spaniel = noun_add("spaniel");
//    noun_derive_from(n_spaniel, n_dog);
//    
    
//    Word_Dictionary_Entry* n_number = noun_add("number");
//    noun_derive_from(n_number, n_thing);
//    
//    for (auto it = numeric_value_words.begin(); it != numeric_value_words.end(); ++it) {
//        Word_Dictionary_Entry* n_entry = noun_add(*it);
//        noun_derive_from(n_entry, n_number);
//    }
    

    
    auto& dock = dt->ui.dock;
    auto& viewport = core->viewport;
    
    
    
    // MARK: Behaviors
    
    // MARK: old...
    //load_standard_behaviors(ct);
    // MARK: new ...
    
    
    auto* visible = attribute_add("visible");
    
    visible->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        mtt::set_should_render(thing);
        return false;
    };
    visible->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        mtt::unset_should_render(thing);
        return false;
    };
    auto* invisible = attribute_add("invisible");
    invisible->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        mtt::unset_should_render(thing);
        return false;
    };
    invisible->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        mtt::set_should_render(thing);
        return false;
    };
    
    auto* new_attrib = attribute_add("new");
    new_attrib->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        //mtt::unset_should_render(thing);
        return false;
    };
    new_attrib->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        //mtt::set_should_render(thing);
        return false;
    };
    
    auto* different_attrib = attribute_add("different");
    different_attrib->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        //mtt::unset_should_render(thing);
        return false;
    };
    different_attrib->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        //mtt::set_should_render(thing);
        return false;
    };
    
    auto* unique_attrib = attribute_add("unique");
    unique_attrib->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        //mtt::unset_should_render(thing);
        return false;
    };
    unique_attrib->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        //mtt::set_should_render(thing);
        return false;
    };
    
    auto* other_attrib = attribute_add("other");
    other_attrib->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        //mtt::unset_should_render(thing);
        return false;
    };
    other_attrib->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        //mtt::set_should_render(thing);
        return false;
    };
    
    auto* static_attrib = attribute_add("static");
    const auto make_static = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        thing->lock_to_canvas = true;
        thing->is_user_movable = false;
        thing->is_user_drawable = false;
        thing->is_static = true;
        
        mtt::World* world = mtt::world(thing);
        
        auto* dt_world = DrawTalk_World_ctx();
        
        
//        mtt::Collision_System* tgt_canvas = &world->collision_system_canvas;
//        
        mtt::Rep& rep = *mtt::rep(thing);
        
        rep.hierarchy_model_transform = dt_world->cam.view_transform * rep.hierarchy_model_transform;
        
        vec3 d_scale;
        quat d_orientation;
        vec3 d_translation;
        vec3 d_skew;
        vec4 d_perspective;
        {
            
            m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
            
            
            mtt::Thing_set_position(thing, d_translation);
            
            mtt::set_pose_transform(thing, rep.pose_transform * m::scale(Mat4(1.0f), d_scale));
        }
//
//        for (usize i = 0; i < rep.colliders.size(); i += 1) {
//            mtt::Collider* c = rep.colliders[i];
//            if (c->system == tgt_canvas) {
//                continue;
//            }
//            
//            Collider_remove(c->system, 0, c);
//            push_AABB(tgt_canvas, c);
//        }
        
        return false;
    };
    const auto make_dynamic = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        thing->lock_to_canvas = false;
        thing->is_user_movable = true;
        thing->is_user_drawable = true;
        thing->is_static = false;
        
        mtt::World* world = mtt::world(thing);
        
        auto* dt_world = DrawTalk_World_ctx();
        
        mtt::Rep& rep = *mtt::rep(thing);
        
        rep.hierarchy_model_transform = dt_world->cam.view_transform * rep.hierarchy_model_transform;
        
        vec3 d_scale;
        quat d_orientation;
        vec3 d_translation;
        vec3 d_skew;
        vec4 d_perspective;
        {
            
            m::decompose(rep.hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
            
            
            mtt::Thing_set_position(thing, d_translation);
            
            mtt::set_pose_transform(thing, rep.pose_transform * m::scale(Mat4(1.0f), d_scale));
        }
        
//        mtt::Collision_System* tgt_canvas = &world->collision_system;
//        
//        mtt::Rep& rep = *mtt::rep(thing);
//        
//        for (usize i = 0; i < rep.colliders.size(); i += 1) {
//            mtt::Collider* c = rep.colliders[i];
//            if (c->system == tgt_canvas) {
//                continue;
//            }
//            
//            Collider_remove(c->system, 0, c);
//            push_AABB(tgt_canvas, c);
//        }
        
        return false;
    };
    static_attrib->on_attribute_add = make_static;
    static_attrib->on_attribute_remove = make_dynamic;
    
    auto* dynamic_attrib = attribute_add("dynamic");
    dynamic_attrib->on_attribute_add = make_dynamic;
    dynamic_attrib->on_attribute_remove = make_static;
    
    auto* finished_attrib = attribute_add("finished");
    finished_attrib->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        thing->is_user_drawable = false;
        //mtt::unset_should_render(thing);
        return true;
    };
    finished_attrib->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        thing->is_user_drawable = true;
        //mtt::set_should_render(thing);
        return true;
    };
    
    auto* unfinished_attrib = attribute_add("unfinished");
    unfinished_attrib->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        thing->is_user_drawable = true;
        //mtt::unset_should_render(thing);
        return false;
    };
    unfinished_attrib->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        thing->is_user_drawable = false;
        //mtt::set_should_render(thing);
        return false;
    };
    
    auto* positive_attrib = attribute_add("positive");
    auto* negative_attrib = attribute_add("negative");
    positive_attrib->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        if (mtt::thing_type_id(thing) == mtt::ARCHETYPE_NUMBER) {
            auto* polarity_field = mtt::access<float32>(thing, "polarity_constraint");
            *polarity_field = 1;
            auto* val_field = mtt::access<float32>(thing, "value");
            number_update_value(thing, *val_field);
        }
        
        return true;
    };
    positive_attrib->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        if (mtt::thing_type_id(thing) == mtt::ARCHETYPE_NUMBER) {
            auto* polarity_field = mtt::access<float32>(thing, "polarity_constraint");
            *polarity_field = 0;
            auto* val_field = mtt::access<float32>(thing, "value");
            number_update_value(thing, *val_field);
        }
        
        return true;
    };
    negative_attrib->on_attribute_add = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        if (mtt::thing_type_id(thing) == mtt::ARCHETYPE_NUMBER) {
            auto* polarity_field = mtt::access<float32>(thing, "polarity_constraint");
            *polarity_field = -1;
            auto* val_field = mtt::access<float32>(thing, "value");
            number_update_value(thing, *val_field);
        }
        
        return true;
    };
    negative_attrib->on_attribute_remove = [](Word_Dictionary_Entry* entry, mtt::Thing* thing) -> bool {
        if (mtt::thing_type_id(thing) == mtt::ARCHETYPE_NUMBER) {
            auto* polarity_field = mtt::access<float32>(thing, "polarity_constraint");
            *polarity_field = 0;
            auto* val_field = mtt::access<float32>(thing, "value");
            number_update_value(thing, *val_field);
        }
        
        return true;
    };
    
    {
        Word_Dictionary* dict = dt::word_dict();
        dict->insert_ignored_attribute("new");
        dict->insert_ignored_attribute("unique");
        dict->insert_ignored_attribute("different");
        dict->insert_ignored_attribute("other");
        dict->insert_ignored_attribute("another");
        dict->insert_ignored_attribute("some");
        dict->insert_ignored_attribute("any");
        dict->insert_ignored_attribute("miscellaneous");
        dict->insert_ignored_attribute("static");
        dict->insert_ignored_attribute("dynamic");
        dict->insert_ignored_attribute("finished");
        dict->insert_ignored_attribute("unfinished");
    }
    
    dt::load_standard_actions(dt);
    // MARK: User Interface
#define USE_CAMERA_AT_ALL (1)
    
#if USE_CAMERA_AT_ALL
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
#define CAMERA (0)
#else
#define CAMERA (1)
#endif
#else
#define CAMERA (0)
#endif

#define DT_INCLUDE_DELETE_BUTTON_FROM_V0 (1)


    dock.labels = {
        DT_SPEECH_COMMAND_BUTTON_NAME,
        "discard",
        "speech ignore",
        DT_CONTEXT_BUTTON_NAME,
        "copy",
        "copy attached",
#if DT_INCLUDE_DELETE_BUTTON_FROM_V0
        "delete",
#endif
        "delete all",
        "scale",
        "flip left / right",
        "attach / detach",
        //"draw arrows",
        "toggle show attached",
#if CAMERA
        "enable camera",
#endif
        "toggle labels",
        "toggle system labels",
        "pause",
        "save thing",
        "color",
        "select all",
        "selection clear",
        "selection invert",
        "label things",
    };
    auto& labels = dock.labels;
    
    //auto WH = sd::get_native_display_bounds();
    
    float32 b_actual_width = 1.0f;
    float32 b_actual_height = 1.0f;
    // top, bottom, left, right with respect to orientation
    vec4 view_offsets = sd::view_offset_for_safe_area(core->renderer);
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    MTT_UNUSED(view_offsets);
    float32 ui_width  = viewport.width;
    float32 ui_height = viewport.height;
    float32 b_width   = ui_width / 29.0f;
    float32 b_height = ui_height / 29.0f;
    b_actual_width  = b_width;
    b_actual_height = b_height;
    float32 default_b_width  = 1367.0f / 32.0f;
    float32 default_b_height = 979.0f / 32.0f;
    b_width  =  default_b_width;
    b_height =  default_b_height;
    
    float32 padding = b_width / 2.0f;
    float32 y_padding = b_height * 0.5f;
    
    float32 x_pos = padding;
    float32 y_pos = (b_height / 2.0f) + (viewport.height / 2.0f) - 0.5f * (((labels.size() * b_height) + ((labels.size()) * y_padding - 2)));
#else
    float32 ui_width  = 1.0f;
    float32 ui_height = 1.0f;
    float32 b_width = 1.0f;
    float32 b_height = 1.0f;
    
    float32 padding = 0.0f;
    float32 y_padding = 0.0f;
    float32 x_pos = 0.0f;
    float32 y_pos = 0.0f;
    
    

    //auto native_scale = sd::get_native_display_scale();
    if (viewport.width >= viewport.height) {
        ui_width = viewport.width - view_offsets[2] - view_offsets[3];
        ui_height = viewport.height - view_offsets[0] - view_offsets[1] - b_width;
        b_width = ui_width / 29.0f;
        b_height = ui_height / 29.0f;
        
        b_actual_width = b_width;
        b_actual_height = b_height;
        
        padding = b_width * 0.5f;
        y_padding = b_height * 0.5f;
        
        x_pos = padding + view_offsets[2];
        y_pos = ((b_height * 0.5f) + (ui_height * 0.5f) - 0.5f * (((labels.size() * b_height) + ((labels.size()) * y_padding - 2)))) + view_offsets[0];
    } else {
        ui_width = viewport.height - view_offsets[2] - view_offsets[3];
        ui_height = viewport.width - view_offsets[0] - view_offsets[1];
        b_width = ui_width / 29.0f;
        b_height = ui_height / 29.0f;
        
        b_actual_width  = b_height;
        b_actual_height = b_width;
        
        padding = b_width * 0.5f;
        y_padding = b_height * 0.5f;
        
        x_pos = padding + view_offsets[2];
        y_pos = (((b_width * 0.5f) + (ui_width * 0.5f) - 0.5f * (((labels.size() * b_height) + ((labels.size()) * y_padding - 2)))) + view_offsets[0]) - (ui_height * 0.25f) + view_offsets[0] + (b_width);
    }

    
    
    
    
#endif
    
    // panels
    {
        // MARK: text panel
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        float32 width = 1367.0f;
#else
        float32 width = viewport.width;
#endif
        
        UI& ui = dt->ui;
        // non-dominant - assume right-handed for now
        Panel& ndom_panel = ui.margin_panels[0];
        
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP || MTT_PLATFORM == MTT_PLATFORM_TYPE_IOS
        
#define USE_OLD_TEXT_PANEL_LOCATION (0)
#if USE_OLD_TEXT_PANEL_LOCATION
        ndom_panel.bounds.dimensions = vec2(width * ((600.0f + 400.0f) / (60.0f * 50.0f * 2.0f)), viewport.height);
        ndom_panel.bounds.tl = vec2(width - ndom_panel.bounds.dimensions.x, 0);
#else
        ndom_panel.bounds.dimensions = vec2((viewport.width / 2.0f), (viewport.height / 4.0f) * (1.0f));
        ndom_panel.bounds.tl = vec2(viewport.width / 2.0f, 0.0f);
#endif
        
        
        {
//            float32 y_offset = 16.0f;
//            ndom_panel.bounds.tl.y += y_offset;
//            if (ndom_panel.bounds.dimensions.y >= y_offset) {
//                ndom_panel.bounds.dimensions.y -= y_offset;
//            }
            
        }
        ndom_panel.is_active = true;
        ndom_panel.is_selected_by_touch = false;
        ndom_panel.is_selected_by_pen = false;
        ndom_panel.text.offset = vec2(b_width + padding + padding + padding, padding * 1.5);
        
        
        // MARK: init text panel
        ndom_panel.text.init();
        //ndom_panel.text.offset = vec2(b_width + padding + padding, 0.0f);
        ndom_panel.text.color = core->application_configuration.text_panel_color;

        mtt::Rep* rep = nullptr;
        mtt::Thing* ui_thing =
//        mtt::Thing_make_with_unit_collider(world, mtt::ARCHETYPE_UI_ELEMENT, ndom_panel.bounds.dimensions, &rep, mtt::COLLIDER_TYPE_AABB, vec3(-250.0f, 0.0f) + vec3(viewport.width / 2.0f, ndom_panel.bounds.tl.y + (ndom_panel.bounds.dimensions.y / 2.0f), 900.0f), false);
//
         mtt::Thing_make_with_aabb_dimensions(world, mtt::ARCHETYPE_UI_ELEMENT, vec2(1.0f),  &rep, vec3(ndom_panel.bounds.tl, 900.0f), ndom_panel.bounds.dimensions, false);
#endif
        ndom_panel.thing = ui_thing->id;
        *mtt::access_pointer_to_pointer<Panel*>(ui_thing, "ctx_ptr") = &ndom_panel;
        
        ui_thing->input_handlers.custom_data = mem::alloc_init<mtt::Map_Stable<uintptr, Selection_Rectangle>>(&dt->mtt->allocator);
        ui_thing->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            if (cancel_input_default(event)) {
                return false;
            }
            
            Panel& panel = **mtt::access_pointer_to_pointer<Panel*>(thing, "ctx_ptr");
            panel.most_recent_selected_canvas_position_touch = canvas_pos;
            panel.most_recent_selected_world_position_touch = world_pos;
            
            panel.touch_state = touch->direct.state;
            panel.touch_began_time = event->timestamp;
            
            panel.most_recent_touch_key = touch->key;
            
            {
                auto& text_panel = panel.text;
                
                auto scroll_bounds = panel.scroll_bounds();
                vec2 scroll_top = scroll_bounds.tl;
                vec2 dim = scroll_bounds.dimensions;
                
                
                vec2 pos = panel.most_recent_selected_canvas_position_touch;
                
                mtt::Circle c;
                c.radius = ((uint64)(mtt_core_ctx()->viewport.width / 298.0f)) * 2;
                c.center = pos;
                float bounds[4];
                bounds[0] = scroll_top[0];
                bounds[1] = scroll_top[1];
                bounds[2] = scroll_top[0] + dim[0];
                bounds[3] = scroll_top[1] + dim[1];
                if (mtt::Box_Circle_intersection(bounds, &c)) {
                    panel.slider_active = true;
                    panel.slider_prev_pos = c.center;
                    panel.slider_for_text.offset = 0;
                    panel.slider_input_key = panel.most_recent_touch_key;
                }
            }
            
            if (panel.slider_active && panel.slider_input_key == panel.most_recent_touch_key) {
                return true;
            }
            
            if (!panel.text.selection_rectangles.empty()) {
                return true;
            }
            
            
            Selection_Rectangle rect = {};
        
            
            rect.end = canvas_pos;
            rect.start = canvas_pos;
            rect.extent = m::vec2_zero();
            rect.area = 0.0f;
            rect.init_origin = canvas_pos;
            rect.curr = canvas_pos;
            
            panel.text.first_selected_i = ULLONG_MAX;
            panel.text.selection_rectangles.insert({(UI_Key)touch->key, rect});
            //panel.pen_selections.clear();
            
            
            
            return true;
        };
        
        
        ui_thing->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            if (cancel_input_default(event)) {
                return;
            }
            
            Panel& panel = **mtt::access_pointer_to_pointer<Panel*>(thing, "ctx_ptr");
            panel.most_recent_selected_canvas_position_touch = canvas_pos;
            panel.most_recent_selected_world_position_touch = world_pos;
            
            panel.touch_state = touch->direct.state;
            
            panel.most_recent_touch_key = touch->key;
            
            {
                if (panel.slider_active) {
                    Input* const input = &mtt_core_ctx()->input;
                    Input_Record* u_input = &input->users[0];
                    
                    auto result = u_input->direct_map.find(panel.slider_input_key);
                    if (result != u_input->direct_map.end()) {
                        const UI_Touch* in = &result->second;
                        const UI_Touch_Direct* direct = &in->direct;
                        
                        auto& text_panel = panel.text;
                        
                        if (text_panel.text.size() == 0) {
                            text_panel.vertical_scroll_offset = 0;
                        } else {
                            
                            vec2 scroll_top = panel.bounds.tl + vec2(text_panel.offset.x * 0.2, text_panel.offset.y);
                            vec2 dim = vec2(text_panel.offset.x * 0.3, panel.bounds.dimensions.y - text_panel.offset.y * 2);
#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
                            auto delta = -direct->position_delta.y;
#else
                            auto delta = direct->position_delta.y;
#endif
                            auto& word = text_panel.text.back();
                            float half_height = (word.bounds[3] - word.bounds[1]) * 0.5;
                            text_panel.vertical_scroll_offset -= delta;
                            text_panel.vertical_scroll_offset = m::min(m::max(-half_height, text_panel.vertical_scroll_offset), text_panel.row_offset + half_height);
                        }
                    }
                }
            }
            
            if (panel.slider_active && panel.slider_input_key == panel.most_recent_touch_key) {
                return;
            }
            
            Selection_Rectangle* sel_rect = nullptr;
            if (mtt::map_try_get(&panel.text.selection_rectangles, touch->key, &sel_rect)) {
                update_selection_rectangles(canvas_pos, panel, *sel_rect);
            }
        };
        ui_thing->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            Panel& panel = **mtt::access_pointer_to_pointer<Panel*>(thing, "ctx_ptr");
            

            panel.most_recent_touch_key = touch->key;
            
            
            if (panel.slider_input_key == touch->key) {
                {
                    panel.slider_active = false;
                    panel.slider_for_text.offset = 0;
                    panel.slider_input_key = UI_Key_INVALID;
                }
            } else {
                Selection_Rectangle* sel_rect = nullptr;
                if (mtt::map_try_get(&panel.text.selection_rectangles, touch->key, &sel_rect)) {
                    update_selection_rectangles(canvas_pos, panel, *sel_rect);
                    panel.text.selection_rectangles_to_handle.push_back(*sel_rect);
                    panel.text.selection_rectangles.erase(touch->key);
                }
            }
//            if (panel.text.selection_rectangles.size() > 0) {
//                update_selection_rectangles(canvas_pos, panel);
//                //panel.text.selection_rectangles_to_handle.push_back(panel.text.selection_rectangles[0]);
//                mtt:clear(&panel.text.selection_rectangles);
//            }
            
            if (cancel_input_default(event)) {
                return;
            }
            
            
            panel.most_recent_selected_canvas_position_touch = canvas_pos;
            panel.most_recent_selected_world_position_touch = world_pos;
            
            panel.touch_state = touch->direct.state;
        };
        
        ui_thing->input_handlers.on_pen_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            Panel& panel = **mtt::access_pointer_to_pointer<Panel*>(thing, "ctx_ptr");
            panel.most_recent_selected_canvas_position_pen = canvas_pos;
            panel.most_recent_selected_world_position_pen = world_pos;
            
            panel.pen_state = touch->pointer.state;
            panel.pen_began_time = event->timestamp;
            
            //dt::UI_draw();
            
            return true;
        };
        ui_thing->input_handlers.on_pen_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            Panel& panel = **mtt::access_pointer_to_pointer<Panel*>(thing, "ctx_ptr");
            panel.most_recent_selected_canvas_position_pen = canvas_pos;
            panel.most_recent_selected_world_position_pen = world_pos;
            
            panel.pen_state = touch->pointer.state;
            
            //dt::UI_draw();
        };
        ui_thing->input_handlers.on_pen_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            Panel& panel = **mtt::access_pointer_to_pointer<Panel*>(thing, "ctx_ptr");
            
            panel.most_recent_selected_canvas_position_pen = canvas_pos;
            panel.most_recent_selected_world_position_pen = world_pos;
            
            panel.pen_state = touch->pointer.state;
            
            dt::UI_draw();
        };
    }
    
    
    {
        UI& ui = dt->ui;
        Panel& ndom_panel = ui.margin_panels[0];
        
        auto& map_base = dt->ui.top_panel.base;
        map_base.bounds.dimensions = vec2(viewport.width / 2.0f, ndom_panel.bounds.dimensions.y);
        map_base.bounds.tl = vec2(0.0f, 0.0f);
        map_base.is_active = true;
        
        y_pos = (map_base.bounds.tl.y + map_base.bounds.dimensions.y) + (b_width);
        
        mtt::Camera_init(&dt->ui.top_panel.cam);
        // disallow rotation of the top panel view
        dt->ui.top_panel.cam.flags = (mtt::CAMERA_FLAGS)(mtt::CAMERA_FLAGS_ENABLE_TRANSLATION | mtt::CAMERA_FLAGS_ENABLE_SCALE);
        
//        {
//            sd::Renderer* renderer = core->renderer;
//            sd::save(renderer);
//            sd::set_render_layer(renderer, LAYER_LABEL_STATIC_CANVAS);
//            sd::begin_drawable(renderer);
//            sd::begin_polygon_no_new_drawable(renderer);
//            sd::set_color_rgba_v4(renderer, {1.0f, 0.0f, 0.0f, 0.5f});
//            sd::rectangle(renderer, map_base.bounds.tl, map_base.bounds.dimensions, 0.0f);
//            sd::end_polygon_no_new_drawable(renderer);
//            sd::end_drawable(renderer);
//            sd::restore(renderer);
//        }
        

        mtt::Rep* rep = nullptr;
        mtt::Thing* ui_thing = mtt::Thing_make_with_aabb_dimensions(world, mtt::ARCHETYPE_UI_ELEMENT, vec2(1.0f),  &rep, vec3(map_base.bounds.tl, 900.0f), map_base.bounds.dimensions, false);
        ui_thing->is_user_drawable     = false;
        ui_thing->is_user_destructible = false;
        ui_thing->is_user_movable      = false;
        
        map_base.thing = ui_thing->id;
        
        ui_thing->input_handlers.on_pen_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            
            
            MTT_print("%s\n", "DIAGRAM PANEL PEN BEGAN\n");
            return true;
            
            auto* el = *mtt::access_pointer_to_pointer<Top_Panel*>(thing, "ctx_ptr");
            if (el->is_selected_key == UI_Key_INVALID) {
                el->is_selected_key = touch->key;
                
                auto* const world = DrawTalk_World_ctx();
                
                cam_stack_push(world, &el->cam);
                
                return mtt::Input_Handler_Return_STATUS_DO_RECURSE;
            }
            
            return true;
        };
        ui_thing->input_handlers.on_pen_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            MTT_print("%s\n", "DIAGRAM PANEL PEN ENDED\n");
            return;
            
            auto* el = *mtt::access_pointer_to_pointer<Top_Panel*>(thing, "ctx_ptr");
            if (el->is_selected_key == UI_Key_INVALID) {
                return;
            } else {
                el->is_selected_key = UI_Key_INVALID;
                auto* core = mtt_core_ctx();
                Input* input = &core->input;
                auto* world = DrawTalk_World_ctx(core);
                
                cam_stack_pop(world);
                
//                Input_Deferred_push(input, (Input_Deferred_Record){(void*)world, [](Input* input, void* ctx) {
//                    auto* world = (DrawTalk_World*)ctx;
//                    cam_stack_pop(world);
//                }});
                
                return;
            }
        };
        ui_thing->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            // TOGO: not going to make this dynamic for now
            
            
            MTT_print("%s\n", "DIAGRAM PANEL TOUCH BEGAN\n");
            return true;
            
            
            
            auto* el = *mtt::access_pointer_to_pointer<Top_Panel*>(thing, "ctx_ptr");
            if (el->is_selected_key == UI_Key_INVALID) {
                el->is_selected_key = touch->key;
                
                auto* const world = DrawTalk_World_ctx();
                
                cam_stack_push(world, &el->cam);
                
                return mtt::Input_Handler_Return_STATUS_DO_RECURSE;
            }
            
            return true;
        };
        ui_thing->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            
            
            MTT_print("%s\n", "DIAGRAM PANEL TOUCH ENDED\n");
            return;
            
            
            auto* el = *mtt::access_pointer_to_pointer<Top_Panel*>(thing, "ctx_ptr");
            if (el->is_selected_key == UI_Key_INVALID) {
                return;
            } else {
                el->is_selected_key = UI_Key_INVALID;
                auto* core = mtt_core_ctx();
                Input* input = &core->input;
                auto* world = DrawTalk_World_ctx(core);
                
                cam_stack_pop(world);
                
//                Input_Deferred_push(input, (Input_Deferred_Record){(void*)world, [](Input* input, void* ctx) {
//                    auto* world = (DrawTalk_World*)ctx;
//                    cam_stack_pop(world);
//                }});
                
                return;
            }
        };
        
        Collision_System_init(&dt->ui.top_panel.collision_system_world, 0, core->allocator, world);
        Collision_System_layer_make(&dt->ui.top_panel.collision_system_world, 1 << 31);
        dt->ui.top_panel.collision_system_world.collision_handler_AABB_default = mtt::collision_handler_AABB_default;
        dt->ui.top_panel.collision_system_world.collision_handler_Circle_default = mtt::collision_handler_Circle_default;
        dt->ui.top_panel.collision_system_world.label = "world";
        
        
        Collision_System_init(&dt->ui.top_panel.collision_system_canvas, 0, core->allocator, world);
        Collision_System_layer_make(&dt->ui.top_panel.collision_system_canvas, 1 << 31);
        dt->ui.top_panel.collision_system_canvas.collision_handler_AABB_default = mtt::collision_handler_AABB_default;
        dt->ui.top_panel.collision_system_canvas.collision_handler_Circle_default = mtt::collision_handler_Circle_default;
        dt->ui.top_panel.collision_system_canvas.label = "canvas";
        
        *mtt::access_pointer_to_pointer<Top_Panel*>(ui_thing, "ctx_ptr") = &dt->ui.top_panel;

        
        
        

        
//        sd::set_render_layer(core->renderer, LAYER_LABEL_STATIC_CANVAS);
//        sd::set_color_rgba_v4(core->renderer, ndom_panel.text.color);
//        sd::begin_polygon(core->renderer);
//            auto* col = first_collider(rep);
//            auto& aabb = col->aabb;
//            vec2 offset = vec2(0.0f);
//            sd::rectangle(core->renderer, aabb.tl + offset, (aabb.br - aabb.tl) * vec2(4.0f, 1.0f), 0.0f);
//        auto* info = sd::end_polygon(core->renderer);
//        info->set_transform(rep->model_transform * rep->pose_transform);
//        rep->render_data.drawable_info_list.push_back(info);
        
        // active items
        
        auto& active_things_panel = dt->ui.active_things_panel;
        
        // create Thing
        
        Scroll_Panel_init(&active_things_panel.scroll_base, world);
        mtt::Array_init(&active_things_panel.things, core->allocator);
        mtt::Array_init(&active_things_panel.things, core->allocator);
    }
    
    if ((false)) {
        UI& ui = dt->ui;
        Panel& ndom_panel = ui.margin_panels[0];
        
        auto& base = dt->ui.bottom_panel.base;
        base.bounds.dimensions = vec2(viewport.width - ndom_panel.bounds.dimensions.x, (viewport.height * 1.75f * 1.2) / 8.0f);
        base.bounds.tl = vec2(0.0f, viewport.height - base.bounds.dimensions.y);
        base.is_active = true;
        
        mtt::Rep* rep = nullptr;
        vec3 POS = vec3((float32)viewport.width /2.0f, base.bounds.tl.y + (base.bounds.dimensions.y * 0.5f), 0.1f);
        mtt::Thing* ui_thing = mtt::Thing_make_with_unit_collider(world, mtt::ARCHETYPE_UI_ELEMENT, base.bounds.dimensions, &rep, mtt::COLLIDER_TYPE_AABB, POS, false);
        base.thing = ui_thing->id;
        *mtt::access_pointer_to_pointer<Bottom_Panel*>(ui_thing, "ctx_ptr") = &dt->ui.bottom_panel;
        
        
    }


//    float32 x_pos = (b_width / 2.0f) + (viewport.width) - (((labels.size() * b_width) + ((labels.size()) * padding - 2)));
//    UI& ui = dt->ui;
//    // non-dominant - assume right-handed for now
//    Panel& ndom_panel = ui.margin_panels[0];
//    ndom_panel.bounds.tl = vec2(0, 0);
//    ndom_panel.bounds.dimensions = vec2(width * (1.0f / 5.0f), viewport.height);
//    ndom_panel.is_active = true;
//    ndom_panel.is_selected_by_touch = false;
//    ndom_panel.is_selected_by_pen = false;
//    ndom_panel.offset = vec2(0, 0);
//    ndom_panel.text.offset = vec2(width / 150.0f, width / 150.0f);
//    float32 x_pos = padding;// + dt->ui.margin_panels[0].bounds.dimensions.x / 2 ;//dt->ui.margin_panels[0].bounds.tl.x + dt->ui.margin_panels[0].text.offset.x +  dt->ui.margin_panels[0].bounds.dimensions.x + (2*padding);
    //x_pos = padding;
//    float32 y_pos = (b_height / 2.0f) + (viewport.height / 2.0f) - 0.5f * (((labels.size() * b_height) + ((labels.size()) * y_padding - 2)));
    dock.buttons.resize(labels.size());
    
    mtt::Map<mtt::String, usize>& label_to_index = dock.label_to_index;
    for (usize i = 0; i < labels.size(); i += 1) {
        label_to_index[labels[i]] = i;
    }
#define USE_OLD_BUTTON_COLORS_UI (true)
    mtt::Map<mtt::String, vec4>& label_to_color = dock.label_to_color;
    label_to_color = {
        {DT_SPEECH_COMMAND_BUTTON_NAME, RGBint_toRGBfloat(vec4(0, 181, 204, 255/2))},
        {"discard", vec4(0.5f, 0.0f, 0.0f, 0.5f)},
        {"speech ignore", vec4(1.0f, 0.5f, 0.0f, 0.8f)},
        {DT_CONTEXT_BUTTON_NAME, RGBint_toRGBfloat(vec4(80, 220, 100, 255 / 2))},
        {"copy", RGBint_toRGBfloat(vec4(80, 220, 100, 255 / 4))},
        {"copy attached", RGBint_toRGBfloat(vec4(80, 220, 100, 255 / 4))},
#if DT_INCLUDE_DELETE_BUTTON_FROM_V0
        {"delete", vec4(1.0f, 0.0f, 0.0f, 0.25f)},
#endif
        {"delete all", vec4(1.0f, 0.0f, 0.0f, 0.25f)},
        {"scale", vec4(color::SYSTEM_BLUE[0], color::SYSTEM_BLUE[1], 1, 0.25f)},
        {"flip left / right", vec4(color::SYSTEM_BLUE[0], color::SYSTEM_BLUE[1],1, 0.25f)},
        {"attach / detach", vec4(color::SYSTEM_BLUE[0], color::SYSTEM_BLUE[1], 1, 0.25f)},
        //{"draw arrows", vec4(127.0f/255.0f,165.0f/255.0f,1.0f,0.5f)},
        
        {"toggle show attached", vec4(127.0f/255.0f,165.0f/255.0f,1.0f,0.5f)},
#if CAMERA
        {"enable camera", vec4(1.0f, 0.0f, 1.0f, 0.25f)},
#endif
        {"toggle labels", vec4(127.0f/255.0f,165.0f/255.0f,1.0f,0.5f)},
        {"toggle system labels", vec4(vec3(0.5f), 0.5f)},
        {"pause", vec4(vec3(0.5f), 0.5f)},
        {"save thing", vec4(vec3(0.6f), 0.5f)},
        {"color", vec4(0.0f, 0.0f, 1.0f, 1.0f)},
        {"select all", vec4(0.75, 0.75, 0.75, 1.0)},
        {"selection clear", vec4(0.75, 0.75, 0.75, 1.0)},
        {"selection invert", vec4(0.75, 0.75, 0.75, 1.0)},
        {"label things", vec4(0.75, 0.75, 0.75, 1.0)},
    };
    
    UI& ui = dt->ui;
    Panel& ndom_panel = ui.margin_panels[0];

    mtt::Map<mtt::String, UI_Dock::Entry_Config>& label_to_config = dock.label_to_config;
    
    float32 x_text_col = -padding - b_width;
#define DT_TEXT_COL_Y(i) (y_padding + (b_width * i) + (0.5f*y_padding*(i+1)))
    label_to_config = {
        {DT_SPEECH_COMMAND_BUTTON_NAME, (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                .hsv_color = false,
            .override_box = true,
#if USE_OLD_TEXT_PANEL_LOCATION
            .box = {.tl = ndom_panel.bounds.tl + vec2(padding, y_padding), .dimensions = vec2(b_width)}
#else
            .box = {.tl = ndom_panel.bounds.tl + vec2(ndom_panel.bounds.dimensions.x, 0.0f) + vec2(x_text_col, DT_TEXT_COL_Y(0)), .dimensions = vec2(b_width)}
#endif
        }},
        {"discard", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                .hsv_color = false,
            .override_box = true,
#if USE_OLD_TEXT_PANEL_LOCATION
            .box = {.tl = ndom_panel.bounds.tl + vec2(padding + b_width + padding, y_padding), .dimensions = vec2(b_width)}
#else
            .box = {.tl = ndom_panel.bounds.tl + vec2(ndom_panel.bounds.dimensions.x, 0.0f) + vec2(x_text_col, DT_TEXT_COL_Y(1)), .dimensions = vec2(b_width)}
#endif
        }},
        {"speech ignore", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                .hsv_color = false,
            .override_box = true,
#if USE_OLD_TEXT_PANEL_LOCATION
            .box = {.tl = ndom_panel.bounds.tl + vec2(padding + b_width + padding + b_width + padding, y_padding), .dimensions = vec2(b_width)}
#else
            .box = {.tl = ndom_panel.bounds.tl + vec2(ndom_panel.bounds.dimensions.x, 0.0f) + vec2(x_text_col, DT_TEXT_COL_Y(2)), .dimensions = vec2(b_width)}
#endif
        }},
        {DT_CONTEXT_BUTTON_NAME, (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                .hsv_color = false,
            .override_box = true,

            .box = {.tl = ndom_panel.bounds.tl + vec2(ndom_panel.bounds.dimensions.x, 0.0f) + vec2(x_text_col, DT_TEXT_COL_Y(3)), .dimensions = vec2(b_width)}
        }},
        {"copy", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        {"copy attached", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
#if DT_INCLUDE_DELETE_BUTTON_FROM_V0
        {"delete", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
#endif
        {"delete all", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        {"scale", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        {"flip left / right", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        {"attach / detach", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        //{"draw arrows", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        {"toggle show attached", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
#if CAMERA
        {"enable camera", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
#endif
        {"toggle labels", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        {"toggle system labels", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        {"pause", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        {"save thing", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true}},
        {"color",             (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS_HSV, .show_label = false, .hsv_color = true,
            .override_box = true, .box = {.tl = vec2(0.25f * m::min(b_actual_width, b_actual_height), viewport.height - (4 * b_actual_height * 1.1)), .dimensions = vec2(m::max(b_actual_width, b_actual_height) * 0.5, b_actual_height * 3*.9),  }, .ignore_graphics = true
        }},
        {"select all", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
            .hsv_color = false,
            .override_box = true,
            .box = {.tl = ndom_panel.bounds.tl + vec2(0.25 * padding, 0.0f) + vec2(0.0f, DT_TEXT_COL_Y(0)), .dimensions = vec2(b_width)}
            
        }},
        {"selection clear", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                .hsv_color = false,
            .override_box = true,
            .box = {.tl = ndom_panel.bounds.tl + vec2(0.25 * padding, 0.0f) + vec2(0.0f, DT_TEXT_COL_Y(1)), .dimensions = vec2(b_width)}
        }},
        {"selection invert", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                .hsv_color = false,
            .override_box = true,
            .box = {.tl = ndom_panel.bounds.tl + vec2(0.25 * padding, 0.0f) + vec2(0.0f, DT_TEXT_COL_Y(2)), .dimensions = vec2(b_width)}
        }},
        {"label things", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                .hsv_color = false,
            .override_box = true,
            .box = {.tl = ndom_panel.bounds.tl + vec2(0.25 * padding, 0.0f) + vec2(0.0f, DT_TEXT_COL_Y(3)), .dimensions = vec2(b_width)}
        }},
    };
    
    mtt::Thing* root_b = mtt::Thing_make(world, mtt::ARCHETYPE_UI_ELEMENT);
    
    for (usize i = 0; i < labels.size(); i += 1) {
        auto* button_main = mem::alloc_init<UI_Button>(&core->allocator);
        button_main->label = labels[i];
        button_main->name = labels[i];
        dock.buttons[i] = button_main;

        dock.label_to_button[labels[i]] = button_main;
        auto* conf = &label_to_config[button_main->label];
        
        button_main->show_label = conf->show_label;
        
        mtt::Thing* ui_button = nullptr;
        mtt::Rep* rep = nullptr;
        
        //dock.buttons[i]->saved_bounds.tl = info.transform * vec4(rep->colliders.back()->aabb.tl, 0.0f, 1.0f);
        //dock.buttons[i]->saved_bounds.br = info.transform * vec4(rep->colliders.back()->aabb.br, 0.0f, 1.0f);
        dock.buttons[i]->saved_center = vec2(x_pos, y_pos);
        dock.buttons[i]->saved_dimensions = vec2(b_width);
        
        
        if (conf->override_box) {
            ui_button = mtt::Thing_make_with_aabb_dimensions(world, mtt::ARCHETYPE_UI_ELEMENT, vec2(1.0f), &rep, vec3(conf->box.tl, 999.999f), conf->box.dimensions, false);
        } else {
            ui_button = mtt::Thing_make_with_unit_collider(world, mtt::ARCHETYPE_UI_ELEMENT, vec2(b_width), &rep, mtt::COLLIDER_TYPE_AABB, vec3(x_pos, y_pos, 999.999f), false);
        }

        
        dock.buttons[i]->thing = ui_button->id;
        
        
        *mtt::access_pointer_to_pointer<UI_Button*>(ui_button, "ctx_ptr") = button_main;
        
        
        
        
        if (!conf->ignore_graphics) {
            sd::set_render_layer(core->renderer, conf->render_layer);
            sd::begin_drawable(core->renderer);
            sd::begin_polygon_no_new_drawable(core->renderer);
            //rep->model_transform = m::translate(Mat4(1.0f), vec3(half_width, viewport.height - (b_width * 0.5f), 100.0f));
            //mtt::Collider_print(rep->colliders.back());
            //int BP = 0;
            
            vec2 offset = vec2(0.0f); // 8=conf->override_box ? -vec2((rep->colliders.back()->aabb.br - rep->colliders.back()->aabb.tl) / 2.0f) : vec2(0.0f);
            
            if (conf->hsv_color) {
                sd::set_color_hsva_v4(core->renderer, label_to_color.find(button_main->label)->second);
            } else {
                sd::set_color_rgba_v4(core->renderer, label_to_color.find(button_main->label)->second);
            }
            if (conf->override_box) {
                auto* aabb = &rep->colliders.back()->aabb;
                vec2 center = (aabb->tl + aabb->br) / 2.0f;
                vec2 tl = aabb->tl - center;
                vec2 br = aabb->br - center;
                sd::rectangle_rounded(core->renderer, tl, br - tl, 0.0f, 0.2f, 4);
            } else {
                sd::rectangle_rounded(core->renderer, rep->colliders.back()->aabb.tl + offset, rep->colliders.back()->aabb.br - rep->colliders.back()->aabb.tl, 0.0f, 0.2f, 4);
                y_pos += y_padding + b_height;
            }
            sd::end_polygon_no_new_drawable(core->renderer);
//            sd::begin_path_no_new_drawable(core->renderer);
//            sd::path_radius(core->renderer, 1.0f);
//            sd::path_vertex_v3(core->renderer, vec3(0.0f, 0.0f, 0.0f));
//            sd::path_vertex_v3(core->renderer, vec3(core->viewport.width, core->viewport.height, 0.0f));
//            sd::end_path_no_new_drawable(core->renderer);
            auto& info = *sd::end_drawable(core->renderer);
            {
                info.set_transform(rep->model_transform * rep->pose_transform);
            }
            rep->render_data.drawable_info_list.push_back(&info);
        }

        
    }
    
    auto* button  = dock.label_to_button["color"];
    auto* conf = &label_to_config[button->label];
    mtt::Thing* thing = world->Thing_get(button->thing);
    mtt::Rep* rep = mtt::rep(thing);
    
    sd::set_render_layer(core->renderer, conf->render_layer);
    
    
#if !USE_OLD_TEXT_PANEL_LOCATION
    set_pose_transform(thing, m::translate(mat4(1.0f), (vec3){35.0f, 50.0f, 0.0f}) * m::rotate(mat4(1.0f), (float32)M_PI_2, (vec3){0.0f, 0.0f, 1.0f}));
#endif
    
    for (usize i = 0; i < 2; i += 1){
        sd::begin_polygon(core->renderer);
        //rep->model_transform = m::translate(Mat4(1.0f), vec3(half_width, viewport.height - (b_width * 0.5f), 100.0f));
        //mtt::Collider_print(rep->colliders.back());
        //int BP = 0;
        
        vec2 offset = vec2(0.0f); // 8=conf->override_box ? -vec2((rep->colliders.back()->aabb.br - rep->colliders.back()->aabb.tl) / 2.0f) : vec2(0.0f);
        
        
        if (conf->override_box) {
            auto* aabb = &rep->colliders.back()->aabb;
            vec2 center = (aabb->tl + aabb->br) / 2.0f;
            vec2 tl = aabb->tl - center;
            vec2 br = aabb->br - center;
            
            
            vec2 vert_offset = vec2(0.0f, 10.0f);
            
            // using HSV color
            vec4 top_color;
            switch ((mtt::Color_Scheme_ID)i) {
                default: {
                    MTT_FALLTHROUGH;
                }
                case mtt::COLOR_SCHEME_LIGHT_MODE: {
                    top_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
                    break;
                }
                case mtt::COLOR_SCHEME_DARK_MODE: {
                    top_color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
                    break;
                }
            }
            
            sd::quad_color(core->renderer, tl, tl + vert_offset, vec2(br.x, tl.y + vert_offset.y), vec2(br.x, tl.y), 0.0f, top_color, top_color, top_color, top_color);
        
            
            sd::quad_color(core->renderer, tl + vert_offset, vec2(tl.x, br.y), br, vec2(br.x, tl.y) + vert_offset, 0.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        auto& info = *sd::end_polygon(core->renderer);
        {
            info.set_transform(rep->model_transform * rep->pose_transform);
        }
        rep->render_data.drawable_info_list.push_back(&info);
        info.is_enabled = false;
        
    }
    switch (mtt::color_scheme()) {
        default: {
            MTT_FALLTHROUGH;
        }
        case mtt::COLOR_SCHEME_DARK_MODE: {
            rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_DARK_MODE]->is_enabled = true;
            break;
        }
        case mtt::COLOR_SCHEME_LIGHT_MODE: {
            rep->render_data.drawable_info_list[mtt::COLOR_SCHEME_LIGHT_MODE]->is_enabled = true;
            break;
        }
    }
    for (auto* t_button : dock.buttons) {
        mtt::connect_parent_to_child(world, root_b, mtt::Thing_get(world, t_button->thing));
    }
    for (auto* t_button : dock.buttons) {
        t_button->position_is_modified = true;
    }
//    if (!Speech_split(&core->speech_system.info_list[0])) {
////                                    dt::Selection_Recorder_clear_selections(&world->ct.recorder.selection_recording);
//    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find(DT_SPEECH_COMMAND_BUTTON_NAME)->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            

            handle_speech_phase_change(*dt::DrawTalk::ctx(), thing);
            
            return true;
        };
        
        ui_button->input_handlers.on_pen_input_began = ui_button->input_handlers.on_touch_input_began;
        ui_button->input_handlers.on_pen_input_moved = ui_button->input_handlers.on_touch_input_moved;
        ui_button->input_handlers.on_pen_input_ended = ui_button->input_handlers.on_touch_input_ended;
        
        Speech_register_event_callback(&dt->core->speech_system.info_list[0], SPEECH_SYSTEM_EVENT_CONFIRM, [](SPEECH_SYSTEM_EVENT ev, Speech_System_Event_Status status, void* data) {
            MTT_print("%s", "SPEECH CONFIRM!!\n");
        }, ui_button);
        
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("discard")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print(">>>Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            
            dt_ctx.deferred_confirm = []() {};
            dt_ctx.is_waiting_on_results = false;
            
            if (dt_ctx.lang_ctx.eval_ctx.prompting_for_different_actions) {
                leave_prompt_for_different_actions(&dt_ctx);
                dt_ctx.lang_ctx.eval_ctx.instructions_to_disambiguate.clear();
            }
            
            if (get_confirm_phase() == CONFIRM_PHASE_ANIMATE) {
                dt::text_view_clear(&dt_ctx);
                {
//                    dt::Selection_Recorder_clear_selections_except_for_thing(&dt_ctx.recorder.selection_recording, dt_ctx.scn_ctx.thing_selected_drawable);
                    dt::Selection_Recorder_clear_location_selections(&dt_ctx.recorder.selection_recording);
                    //dt_ctx.scn_ctx.thing_selected_drawable = mtt::Thing_ID_INVALID;
                }
                
                for (usize i = 0; i < dt_ctx.lang_ctx.eval_ctx.instructions_saved().size(); i += 1) {
                    mtt::Thing_destroy(dt_ctx.mtt, dt_ctx.lang_ctx.eval_ctx.instructions_saved()[i]->thing_id);
                    dt_ctx.lang_ctx.eval_ctx.instructions_saved()[i]->thing_id = mtt::Thing_ID_INVALID;
                    dt::Instruction_on_destroy(&dt_ctx, dt_ctx.lang_ctx.eval_ctx.instructions_saved()[i]);
                }
                mtt::Thing_destroy(dt_ctx.mtt, dt_ctx.lang_ctx.eval_ctx.root_thing_id);
                dt_ctx.lang_ctx.eval_ctx.instructions_saved().clear();
                
                
//                {
//                    mtt::Thing* ui_button2 = Thing_get(dt_ctx.mtt, dt_ctx.ui.dock.buttons[dt_ctx.ui.dock.label_to_index.find("speech ignore")->second]->thing);
//                    
//                    auto* core_ctx = mtt_core_ctx();
//                    auto* speech = &core_ctx->speech_system;
//                    
//                    auto& dt_ctx = *ct::DrawTalk::ctx();
//                    
//                    auto* on_flag = &(mtt::access<mtt::Any>(ui_button2, "state0")->Boolean);
//                    if (*on_flag == true) {
//                        Speech_set_active_state(&dt_ctx.core->speech_system.info_list[0], true);
//                    }
//                }
                
                auto* button = dt_ctx.ui.dock.label_to_button[DT_SPEECH_COMMAND_BUTTON_NAME];
                button->label = DT_SPEECH_COMMAND_BUTTON_NAME;
                
                auto* dinfo = mtt::rep(dt_ctx.mtt->Thing_get(button->thing))->render_data.drawable_info_list[0];
                
                dinfo->set_color_factor(RGBint_toRGBfloat(vec4(0, 181, 204, 255)));
                
                button->alpha = 1.0f;
                
                text_in_buf.clear();
                text_in_selected_all = false;
                text_in_cursor_idx = false;
                
                dt_ctx.ui.margin_panels[0].text.set_is_overriding_speech(false);


                
                if (!dt_ctx.lang_ctx.parse_history().empty()) {
                    dt_ctx.lang_ctx.parse_history().back()->is_discarded(true);
                }
                
                {
                    auto* th = dt_ctx.mtt->Thing_get(dt_ctx.ui.dock.buttons[dt_ctx.ui.dock.label_to_index.find("speech ignore")->second]->thing);
                    
                    mtt::Any* on_flag = mtt::access<mtt::Any>(th, "state0");
                    
                    Speech_set_active_state(&dt_ctx.core->speech_system.info_list[0], on_flag->Boolean);
                }
                
                
            } else {
                Speech_discard(&dt_ctx.core->speech_system.info_list[0]);
                dt::text_view_clear(&dt_ctx);
                {
//                    dt::Selection_Recorder_clear_selections_except_for_thing(&dt_ctx.recorder.selection_recording, dt_ctx.scn_ctx.thing_selected_drawable);
                    dt::Selection_Recorder_clear_location_selections(&dt_ctx.recorder.selection_recording);
                    //dt_ctx.scn_ctx.thing_selected_drawable = mtt::Thing_ID_INVALID;
                }
#ifndef NDEBUG
                auto& INFO = dt_ctx.core->speech_system.info_list[0];
                
#endif
            }
            
            set_confirm_phase(CONFIRM_PHASE_SPEECH);
            
            dt_ctx.lang_ctx.deixis_reset();
            
            {
//                auto& q = dt_ctx.lang_ctx.parse_q;
//                for (auto it_q = q.begin(); it_q != q.end(); ++it_q) {
//                    (*it_q)->is_discarded = true;
//                }
                
                auto* dt = &dt_ctx;
                while (!dt->lang_ctx.parse_q().empty()) {
                    (*dt->lang_ctx.parse_q().begin())->is_discarded(true);
                    dt::Parse* p = *(dt->lang_ctx.parse_q().begin());
                    ASSERT_MSG(!p->is_finished(), "Should be impossible!\n");
                    
                    mtt::job_dispatch_serial(static_cast<void*>(p), [](void* ctx) {
                        dt::Parse* p = static_cast<dt::Parse*>(ctx);
                        auto* dt = dt::DrawTalk::ctx();
                        for (usize tok_i = 0; tok_i < p->token_list.size(); tok_i += 1) {
                            mem::deallocate<Speech_Token>(&dt->token_allocation.allocator, p->token_list[tok_i]);
                        }
                    }, [](void* j_ctx) {
                        dt::Parse* p = static_cast<dt::Parse*>(j_ctx);
                        mem::deallocate<dt::Parse>(&dt::DrawTalk::ctx()->parse_allocation.allocator, p);
                    });
                                        
                    dt->lang_ctx.parse_q().erase(dt->lang_ctx.parse_q().begin());
                }
                
                
            }
            
            send_message_drawtalk_server_quick_command(dt_ctx.core, "reset", "");
            
            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            destroy_all_proxies(dt_ctx.mtt);
            
            return true;
        };
        
        ui_button->input_handlers.on_pen_input_began = ui_button->input_handlers.on_touch_input_began;
        ui_button->input_handlers.on_pen_input_moved = ui_button->input_handlers.on_touch_input_moved;
        ui_button->input_handlers.on_pen_input_ended = ui_button->input_handlers.on_touch_input_ended;
        
        Speech_register_event_callback(&dt->core->speech_system.info_list[0], SPEECH_SYSTEM_EVENT_DISCARD, [](SPEECH_SYSTEM_EVENT ev, Speech_System_Event_Status status, void* data) {
        }, (void*)ui_button->id);
    }
    {
        mtt::Thing* ui_button2 = world->Thing_get(dock.buttons[label_to_index.find("speech ignore")->second]->thing);
        
        mtt::Any* on_flag = mtt::access<mtt::Any>(ui_button2, "state0");
        on_flag->type = mtt::MTT_BOOLEAN;
        on_flag->Boolean = true;

        ui_button2->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            //mtt_core_ctx()->tween_ctx
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            auto* core_ctx = mtt_core_ctx();
            auto* speech = &core_ctx->speech_system;
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            
            auto* on_flag = &(mtt::access<mtt::Any>(thing, "state0")->Boolean);
            auto* info = &speech->info_list[0];
            *on_flag = Speech_get_active_state(info);
            if (*on_flag == false) {
                *on_flag = true;
                
                ctx_ptr->label = "speech ignore";
                
                
                
                Speech_set_active_state(info, true);
//                auto record = Speech_Info_most_recent_record(info);
//                if (record.is_valid) {
//                    record.text_info->modification_count += 1;
//                }
                
                
                auto* twn_ctx = core_ctx->tween_ctx;
                twn_Opt opt = {};
                opt.time = 0.25f;
                opt.ease = TWN_QUADIN;
                for (uint32 comp = 0; comp < 3; comp += 1) {
                    twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 1.0, opt);
                }
                twn_add_f32(twn_ctx, &ctx_ptr->alpha, 1.0f, opt);
            } else {
                *on_flag = false;
                
                ctx_ptr->label = "speech enable";
                
                
                Speech_set_active_state(info, false);
                
                auto* twn_ctx = core_ctx->tween_ctx;
                twn_Opt opt = {};
                opt.time = 0.25f;
                opt.ease = TWN_QUADOUT;
                for (uint32 comp = 0; comp < 3; comp += 1) {
                    twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5, opt);
                }
                twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            }
            
            return true;
        };
        
        Speech_register_event_callback(&dt->core->speech_system.info_list[0], SPEECH_SYSTEM_EVENT_ENABLE, [](SPEECH_SYSTEM_EVENT ev, Speech_System_Event_Status status, void* data) {
            MTT_print("%s", "SPEECH ENABLED!!\n");
        }, ui_button2);
        
        Speech_register_event_callback(&dt->core->speech_system.info_list[0], SPEECH_SYSTEM_EVENT_DISABLE, [](SPEECH_SYSTEM_EVENT ev, Speech_System_Event_Status status, void* data) {
            MTT_print("%s", "SPEECH DISABLE!!\n");
        }, ui_button2);
        
        ui_button2->input_handlers.on_pen_input_began = ui_button2->input_handlers.on_touch_input_began;
        ui_button2->input_handlers.on_pen_input_moved = ui_button2->input_handlers.on_touch_input_moved;
        ui_button2->input_handlers.on_pen_input_ended = ui_button2->input_handlers.on_touch_input_ended;
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find(DT_CONTEXT_BUTTON_NAME)->second]->thing);
        
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print(">>>Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            if (ctx_ptr->flags == 1) {
                return true;
            }
            ctx_ptr->flags = 1;
            
            auto* dt = dt::ctx();
            
            
            mtt::World* world = dt->mtt;
            Context_View* ctx_view = &dt->ui.context_view;
            auto* thing_context_view = mtt::Thing_get(world, ctx_view->thing_id);
            if (!mtt::is_active(thing_context_view)) {
                bool status = Context_View_enable(dt);
                MTT_UNUSED(status);
            } else {
                Context_View_disable(dt);
            }
            
            
            
            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            ctx_ptr->flags = 0;
        };
        
        ui_button->input_handlers.on_pen_input_began = ui_button->input_handlers.on_touch_input_began;
        ui_button->input_handlers.on_pen_input_ended = ui_button->input_handlers.on_touch_input_ended;
        
        mtt::Rep* repr;
        float32 el_width = ((float32)viewport.width) * (0.25f);
        float32 el_height = (((float32)viewport.height) - (ui.top_panel.base.bounds.tl.y + ui.top_panel.base.bounds.dimensions.y + dt::bottom_panel_height));
        vec2 tl = vec2(((float32)viewport.width) - el_width, 0.0f) + vec2(0.0f, (ui.top_panel.base.bounds.tl.y + ui.top_panel.base.bounds.dimensions.y));
        mtt::Box box = {
            .tl = tl,
            .br = {tl.x + el_width, tl.y + el_height},
        };
        ui.context_view.box = box;
        ui.context_view.depth = 1.0f;
        
        mtt::Thing* thing_context_view = Thing_make_with_aabb_corners(world, mtt::ARCHETYPE_UI_ELEMENT, vec2(1.0f, 1.0f), &repr, box, 0.0f, false, world->collision_system_group.canvas);
        thing_context_view->is_user_destructible = false;
        ui.context_view.thing_id = thing_context_view->id;
        
        mtt::unset_is_active(thing_context_view);
        
        thing_context_view->input_handlers.on_pen_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            auto* dt = dt::DrawTalk::ctx();
            
            UI& ui = dt->ui;
            
            ui.context_view.selected_this_frame = true;
            
            return true;
        };
        thing_context_view->input_handlers.on_pen_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            auto* core = mtt_core_ctx();
            Input* const input = &core->input;
            Input_Record* u_input = &input->users[0];

            switch (pointer_op_current(u_input)) {
                case UI_POINTER_OP_ERASE: {
                    
                    auto* dt = dt::DrawTalk::ctx();
                    
                    UI& ui = dt->ui;
                    mtt::World* world = mtt::world(thing);
                    auto* proxy_scene = Thing_Proxy_Scene_for_idx(world, DT_THING_PROXY_SCENE_IDX_CONTEXT_PANEL_VIEW);
                    //auto* map = &proxy_scene->thing_to_proxy_map;
                    auto& things_to_display = ui.context_view.things_to_display;
                    for (usize i = 0; i < things_to_display.size(); i += 1) {
                        mtt::Thing* thing_to_destroy = mtt::Thing_try_get(world, things_to_display[i]);
                        if (!mtt::Thing_is_valid(thing_to_destroy)) {
                            continue;
                        }
                        
                        mtt::Thing_destroy(thing_to_destroy);
                    }
                    return;
                }
            }
            
            auto* dt = dt::DrawTalk::ctx();
            
            UI& ui = dt->ui;
            mtt::World* world = mtt::world(thing);
            auto* proxy_scene = Thing_Proxy_Scene_for_idx(world, DT_THING_PROXY_SCENE_IDX_CONTEXT_PANEL_VIEW);
            auto* map = &proxy_scene->thing_to_proxy_map;
            auto& things_to_display = ui.context_view.things_to_display;
            
            auto* cam = &DrawTalk_World_ctx()->cam;
            struct Info {
                mtt::Thing_ID id;
                vec2 dimensions;
            };
            std::vector<Info> final_to_copy = {};
            final_to_copy.reserve(things_to_display.size());
            
            //vec4 max_box = {POSITIVE_INFINITY, POSITIVE_INFINITY, NEGATIVE_INFINITY, NEGATIVE_INFINITY};
            vec2 dim = {0,0};
            for (usize i = 0; i < things_to_display.size(); i += 1) {
                mtt::Thing* t = mtt::Thing_try_get(world, things_to_display[i]);
                if (t == nullptr) {
                    continue;
                }
                
                //auto* box = &mtt::rep(t)->colliders[0]->aabb;
                auto* rep = mtt::rep(t);
                if (rep->colliders.empty()) {
                    continue;
                }
                auto* box = &rep->colliders[0]->aabb.saved_box;
//                max_box[0] = m::min(box->tl.x, max_box[0]);
//                max_box[1] = m::min(box->tl.y, max_box[1]);
//                max_box[2] = m::max(box->br.x, max_box[2]);
//                max_box[3] = m::max(box->br.y, max_box[3]);
                const vec2 dimensions = (box->br - box->tl);;
                dim += dimensions;
                final_to_copy.push_back({mtt::thing_id(t), dimensions});
            }
            float32 side = m::sqrt(dim.x * dim.y);
            
            Mat4 view = cam->view_transform;
            side = (view * vec4(side, 0.0f, 0.0f, 1.0f)).x;
            
            vec2 tl = vec2(world_pos);
            vec2 br = vec2(world_pos) + (side * 0.5f);
            vec2 pos = tl;
            float32 max_y = tl.y;
            float32 max_y_dimension_local_row = NEGATIVE_INFINITY;
            for (usize i = 0; i < things_to_display.size(); i += 1) {
                auto* info = &final_to_copy[i];
                mtt::Thing* t = mtt::Thing_get(world, info->id);

                auto* copy = mtt::Thing_copy(t);
                
                pos.x += info->dimensions.x * 0.5f;
                pos.y = max_y + info->dimensions.y * 0.5f;
                mtt::Thing_set_position(copy, vec3(pos, mtt::access<vec3>(t, "position")->z));
                max_y_dimension_local_row = m::max(info->dimensions.y, max_y_dimension_local_row);
                pos.x += info->dimensions.x;
                if (pos.x >= br.x) {
                    max_y += max_y_dimension_local_row + (max_y_dimension_local_row * 0.5f);
                    max_y_dimension_local_row = NEGATIVE_INFINITY;
                    pos.x = tl.x;
                }
            }
        };
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("selection clear")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print(">>>Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
                
            auto& dt = *dt::ctx();
            auto& text_panel = dt.ui.margin_panels[0].text;
            for (usize i = 0; i < text_panel.text.size(); i += 1) {
                text_panel.text[i].is_selected = false;
            }
            
            return true;
        };
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("selection invert")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print(">>>Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            auto& dt = *dt::ctx();
            auto& text_panel = dt.ui.margin_panels[0].text;
            for (usize i = 0; i < text_panel.text.size(); i += 1) {
                text_panel.text[i].is_selected = !text_panel.text[i].is_selected;
            }
            
            return true;
        };
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("label things")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print(">>>Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            auto& dt = *dt::ctx();
            auto& text_panel = dt.ui.margin_panels[0].text;
//            for (usize i = 0; i < text_panel.text.size(); i += 1) {
//                text_panel.text[i].is_selected = !text_panel.text[i].is_selected;
//            }
            auto& ins_list = dt.lang_ctx.eval_ctx.instructions_saved();
            for (usize i = 0; i < ins_list.size(); i += 1) {
                auto* ins = ins_list[i];
                if (ins->kind != "THING_INSTANCE_SELECTION") {
                    continue;
                }
                
                mtt::String& label = ins->display_label;
                Word_Dictionary_Entry* entry = dt::noun_lookup(label);
                if (entry == nullptr) {
                    continue;
                }
                
                auto& thing_id_list = ins->thing_id_list;
                for (usize t = 0; t < thing_id_list.size(); t += 1) {
                    mtt::Thing* ins_thing_el = mtt::Thing_try_get(dt.mtt, thing_id_list[t]);
                    vis_word_derive_from(ins_thing_el, entry);
                }
            }
            
            return true;
        };
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("select all")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print(">>>Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            auto& dt = *dt::ctx();
            auto& text_panel = dt.ui.margin_panels[0].text;
            for (usize i = 0; i < text_panel.text.size(); i += 1) {
                text_panel.text[i].is_selected = true;
            }
            
            return true;
        };
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("copy")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print(">>>Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            MTT_print(">>>Most recently selected in BUTTON HANDLER=[%llu]\n", dt_ctx.scn_ctx.thing_most_recently_selected_with_touch);
            if (dt_ctx.scn_ctx.thing_most_recently_selected_with_touch != mtt::Thing_ID_INVALID) {
                
                
                mtt::Thing* to_copy = thing->world()->Thing_try_get(dt_ctx.scn_ctx.thing_most_recently_selected_with_touch);
                if (to_copy != nullptr) {
                    bool is_proxy = mtt::Thing_is_proxy(to_copy);
                    if (is_proxy) {
                        to_copy = mtt::Thing_mapped_from_proxy(to_copy);
                    }
                    mtt::Thing* root_thing = mtt::get_root(to_copy);
                    //mtt::Thing_copy_recursively(root_thing);
                    mtt::Thing* copy = mtt::Thing_copy(to_copy);
                    // assume you want the position of
                    if (is_proxy) {
                        MTT_Core* core = mtt_core_ctx();
                        
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
                        mtt::Thing_set_position(copy, DrawTalk_World_ctx()->cam.view_transform * vec4((vec3){
                            core->viewport.width*0.5,
                            core->viewport.height*0.5,
                            mtt::access<vec3>(to_copy, "position")->z
                        }, 1.0f));
#else
                        Input* const input = &core->input;
                        Input_Record* u_input = &input->users[0];
                        vec3 point;
                        if (input->users[0].hover.state != UI_HOVER_STATE_CANCELLED && input->users[0].hover.state != UI_HOVER_STATE_ENDED) {
                            point = vec3(input->users[0].hover.pos, mtt::access<vec3>(to_copy, "position")->z);
                        } else {
                            point = (vec3){
                                core->viewport.width*0.5,
                                core->viewport.height*0.5,
                                mtt::access<vec3>(to_copy, "position")->z
                            };
                        }
                        
                        mtt::Thing_set_position(copy, DrawTalk_World_ctx()->cam.view_transform *
                        vec4(point, 1.0f));
#endif
                    }
                }
                

            } else {
                MTT_print("%s", "Nothing to copy\n");
            }
            
            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
    }
    
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("copy attached")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print(">>>Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            MTT_print(">>>Most recently selected in BUTTON HANDLER=[%llu]\n", dt_ctx.scn_ctx.thing_most_recently_selected_with_touch);
            if (dt_ctx.scn_ctx.thing_most_recently_selected_with_touch != mtt::Thing_ID_INVALID) {
                
                
                mtt::Thing* to_copy = thing->world()->Thing_try_get(dt_ctx.scn_ctx.thing_most_recently_selected_with_touch);
                if (to_copy != nullptr) {
                    bool is_proxy = mtt::Thing_is_proxy(to_copy);
                    if (is_proxy) {
                        to_copy = mtt::Thing_mapped_from_proxy(to_copy);
                    }
                    mtt::Thing* root_thing = mtt::get_root(to_copy);
                    //mtt::Thing_copy_recursively(root_thing);
                    mtt::Thing* copy = mtt::Thing_copy_recursively(to_copy);
                    // assume you want the position of
                    if (is_proxy) {
                        MTT_Core* core = mtt_core_ctx();
                        
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
                        mtt::Thing_set_position(copy, DrawTalk_World_ctx()->cam.view_transform * vec4((vec3){
                            core->viewport.width*0.5,
                            core->viewport.height*0.5,
                            mtt::access<vec3>(to_copy, "position")->z
                        }, 1.0f));
#else
                        Input* const input = &core->input;
                        Input_Record* u_input = &input->users[0];
                        vec3 point;
                        if (input->users[0].hover.state != UI_HOVER_STATE_CANCELLED && input->users[0].hover.state != UI_HOVER_STATE_ENDED) {
                            point = vec3(input->users[0].hover.pos, mtt::access<vec3>(to_copy, "position")->z);
                        } else {
                            point = (vec3){
                                core->viewport.width*0.5,
                                core->viewport.height*0.5,
                                mtt::access<vec3>(to_copy, "position")->z
                            };
                        }
                        
                        mtt::Thing_set_position(copy, DrawTalk_World_ctx()->cam.view_transform *
                        vec4(point, 1.0f));
#endif
                    }
                }
                

            } else {
                MTT_print("%s", "Nothing to copy\n");
            }
            
            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
    }
    
#if DT_INCLUDE_DELETE_BUTTON_FROM_V0
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("delete")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            if (!dt_ctx.scn_ctx.selected_things.empty()) {
                
                for (auto it = dt_ctx.scn_ctx.selected_things.begin(); it != dt_ctx.scn_ctx.selected_things.end(); ++it) {
                    mtt::Thing* to_destroy_thing = thing->world()->Thing_try_get(*it);
                    if (mtt::Thing_is_proxy(to_destroy_thing)) {
                        to_destroy_thing = Thing_mapped_from_proxy(to_destroy_thing);
                        if (to_destroy_thing == nullptr) {
                            continue;
                        }
                    }
                    if (!to_destroy_thing->is_user_destructible || to_destroy_thing->id == thing->id) {
                        continue;
                    }
                    
                    
                    
//                    mtt::Destroy_Command cmd;
//                    cmd.affects_connected = false;
//                    cmd.do_fade_animation = false;
//                    cmd.time_delay = 0.0f;
//                    cmd.time_remaining = 0.0f;
//                    cmd.thing_id = *(it);
//                    thing->world->to_destroy.push_back(cmd);
                    
                    
                    auto recorder_find = dt_ctx.recorder.thing_records.find(to_destroy_thing->id);
                    if (recorder_find != dt_ctx.recorder.thing_records.end()) {
                        dt_ctx.recorder.thing_records.erase(recorder_find);
                    }
                    
                    mtt::Thing_destroy(to_destroy_thing);
                    
                    dt_ctx.ui.changed = true;
                    

                }
            } else {
                
                
                
                MTT_print("%s\n", "Nothing to delete");
            }
            
            dt_ctx.ui.margin_panels[0].text.delete_mode_on = true;

            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
            
        };
    }
#endif
    
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("delete all")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            thing->world()->clear_all_of_type(mtt::ARCHETYPE_FREEHAND_SKETCH);
            thing->world()->clear_all_of_type(mtt::ARCHETYPE_NUMBER);
            thing->world()->clear_all_of_type(mtt::ARCHETYPE_TEXT);
            thing->world()->clear_all_of_type(mtt::ARCHETYPE_PARTICLE_SYSTEM);
            
            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
    }
    
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("scale")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            


            auto* dr = mtt::rep(thing)->render_data.drawable_info_list[0];
            dr->set_color_factor(vec4(0));
            
            if (ctx_ptr->flags == 0) {
                dr->set_color_addition(vec4(color::SYSTEM_BLUE[0], color::SYSTEM_BLUE[1], 1, 0.125f));
                ctx_ptr->label = "scale disable";
            } else {
                dr->set_color_addition(vec4(color::SYSTEM_BLUE[0], color::SYSTEM_BLUE[1], 1, 0.25f));
                ctx_ptr->label = "scale";
            }
            ctx_ptr->flags = !ctx_ptr->flags;
            
            
            return true;
        };
        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());


            auto* dr = mtt::rep(thing)->render_data.drawable_info_list[0];
            dr->set_color_factor(vec4(0));
            
        };
    }
    
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("flip left / right")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            if (!dt_ctx.scn_ctx.selected_things.empty()) {
                
                for (auto it = dt_ctx.scn_ctx.selected_things.begin(); it != dt_ctx.scn_ctx.selected_things.end(); ++it) {
                    mtt::Thing* sel_thing = thing->world()->Thing_try_get(*it);
                    if (sel_thing->archetype_id != mtt::ARCHETYPE_FREEHAND_SKETCH) {
                        continue;
                    }
                    
                    if (mtt::Thing_is_proxy(sel_thing)) {
                        sel_thing = mtt::Thing_mapped_from_proxy(sel_thing);
                    }
                    
                    mtt::flip_init_direction(sel_thing);
                    mtt::flip_left_right(sel_thing);
                    
                    break;

                }
            } else {
                MTT_print("%s", "Nothing to apply\n");
            }
            
            dt_ctx.ui.margin_panels[0].text.delete_mode_on = true;

            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
    }
    
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("attach / detach")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            if (!dt_ctx.scn_ctx.selected_things.empty() && !dt_ctx.recorder.selection_recording.selections.empty()) {
                
                // find parent
                {
                    auto it = dt_ctx.recorder.selection_recording.selections.end() - 1;
                    mtt::Thing* parent_thing = nullptr;

                    for (; ; --it) {
                        auto& record = *it;
                        if (!dt_ctx.scn_ctx.selected_things.contains(record.ID)) {
                            continue;
                        }
                        
                        mtt::Thing* sel_thing = thing->world()->Thing_try_get(record.ID);
                        if (sel_thing == nullptr || !dt::can_label_thing(sel_thing)) {
                            continue;
                        }
                        parent_thing = sel_thing;
                        if (mtt::Thing_is_proxy(parent_thing)) {
                            parent_thing = mtt::Thing_mapped_from_proxy(parent_thing);
                        }
                        break;
                    }
                    if (parent_thing != nullptr) {
                        if (dt_ctx.scn_ctx.selected_things.size() == 2) {
                            if (mtt::get_parent_ID(parent_thing) != mtt::Thing_ID_INVALID) {
                                
                                mtt::disconnect_child_from_parent(mtt::world(parent_thing), parent_thing);
                            }
                        } else {
                            for (auto ch_it = dt_ctx.recorder.selection_recording.selections.begin(); ch_it != it && ch_it != dt_ctx.recorder.selection_recording.selections.end(); ++ch_it) {
                                auto& record = *ch_it;
                                if (!dt_ctx.scn_ctx.selected_things.contains(record.ID)) {
                                    continue;
                                }
                                
                                mtt::Thing* sel_thing = thing->world()->Thing_try_get(record.ID);
                                if (!dt::can_label_thing(sel_thing)) {
                                    continue;
                                }
                                
                                mtt::Thing* child = sel_thing;
                                if (mtt::Thing_is_proxy(child)) {
                                    child = mtt::Thing_mapped_from_proxy(child);
                                }
                                
                                if (mtt::get_parent_ID(child) != mtt::Thing_ID_INVALID) {
                                    mtt::disconnect_child_from_parent(child->world(), child);
                                    continue;
                                }
                                
                                if (mtt::is_ancestor_of(child, parent_thing)) {
                                    continue;
                                }
                                
                                mtt::connect_parent_to_child(child->world(), parent_thing, child);
                            }
                        }
                    }
                }
#define OLD_ATTACH_SINGLE (0)
#if OLD_ATTACH_SINGLE
                for (auto it = dt_ctx.recorder.selection_recording.selections.begin(); it != dt_ctx.recorder.selection_recording.selections.end(); ++it) {
                    auto& record = *it;
                    if (!dt_ctx.scn_ctx.selected_things.contains(record.ID)) {
                        continue;
                    }
                    
                    mtt::Thing* sel_thing = thing->world()->Thing_try_get(it->ID);
                    if (!dt::can_label_thing(sel_thing)) {
                        continue;
                    }
                    
                    mtt::Thing* child = sel_thing;
                    
                    if (mtt::get_parent_ID(child) != mtt::Thing_ID_INVALID) {
                        mtt::disconnect_child_from_parent(child->world(), child);
                        break;
                    }
                    
                    for (auto it_sub = it + 1; it_sub != dt_ctx.recorder.selection_recording.selections.end(); ++it_sub) {
                    
                        auto& record_sub = *it_sub;
                        if (!dt_ctx.scn_ctx.selected_things.contains(record_sub.ID)) {
                            continue;
                        }
                        
                        mtt::Thing* parent_thing = thing->world()->Thing_try_get(it_sub->ID);
                        if (parent_thing == nullptr || !dt::can_label_thing(parent_thing)) {
                            continue;
                        }
                        
                        if (mtt::is_ancestor_of(child, parent_thing)) {
                            continue;
                        }
                        
                        mtt::connect_parent_to_child(child->world(), parent_thing, child);

                    }
                    break;
                    
                }
#endif
#define OLD_ATTACH (0)
#if OLD_ATTACH
                
                for (auto it = dt_ctx.scn_ctx.selected_things.begin(); it != dt_ctx.scn_ctx.selected_things.end(); ++it) {
                    mtt::Thing* sel_thing = thing->world()->Thing_try_get(*it);
                    if (!dt::can_label_thing(sel_thing)) {
                        continue;
                    }
                    
                    mtt::Thing* child = sel_thing;
                    
                    if (mtt::get_parent_ID(child) != mtt::Thing_ID_INVALID) {
                        mtt::disconnect_child_from_parent(child->world(), child);
                        break;
                    }
                    
                    
                    /*
                     //            flecs::entity REL_1 = flecs::entity(world->mtt_world.ecs_world, "REL_1");
                     //            //flecs::entity REL_2 = flecs::entity(world->mtt_world.ecs_world, "REL_2");
                     //            flecs::entity box__ = flecs::entity(world->mtt_world.ecs_world, "box__");
                     //            flecs::entity ball__ = flecs::entity(world->mtt_world.ecs_world, "ball__");
                     //            flecs::entity wall__ = flecs::entity(world->mtt_world.ecs_world, "wall__");
                     //
                     //            box__.add(REL_1, ball__);
                     //            ball__.add(REL_1, box__);
                     //
                     //            box__.add(REL_1, wall__);
                     //            wall__.add(REL_1, box__);
                     //
                     //            int BLABLA;
                     //
                     //            box__.each(REL_1, [&box__, &REL_1](flecs::entity obj) {
                     //                std::cout << "found: " << obj.name() << std::endl;
                     //                if (obj.has(REL_1, box__)) {
                     //                    std::cout << "removing: " << box__.name() << std::endl;
                     //                    obj.remove(REL_1, box__);
                     //                }
                     //            });
                     //
                     //            box__.destruct();
                     */
                    
//                    mtt::Rep* rep = mtt::rep(sel_thing);
////                    rep->forward_dir.x *= -1.0f;
////                    rep->pose_transform = m::scale(mat4(1.0f), {-1.0f, 1.0f, 1.0f}) * rep->pose_transform;
//
//                    mtt::Collider* c = rep->colliders.front();
//
//
//                    vec3 center = c->center_anchor;
//
////                    auto x = sel_thing->input_handlers.prev_positions[0];
////                    vec2 prev_world = x.prev_world_pos;
////
//                    mtt::Collider* c_against = nullptr;
//
//                    mtt::Hit hit = {};
//                    float32 min_area = MAXFLOAT;
                    
                    
                    auto overlap = thing->world()->overlap->typename_desc;
                    child->ecs_entity.each(overlap, [&child, &overlap](flecs::entity obj) {
                        if (obj.has(overlap, child->ecs_entity)) {
                            mtt::Thing_ID parent_id = obj.get<mtt::Thing_Info>()->thing_id;
                            if (parent_id == child->id) {
                                return;
                            }
                            mtt::Thing* parent_thing = nullptr;
                            if (!child->world()->Thing_try_get(parent_id, &parent_thing)) {
                                return;
                            }
                            
                            
                            if (mtt::is_ancestor_of(child, parent_thing)) {
                                return;
                            }
                            
                            mtt::connect_parent_to_child(child->world(), parent_thing, child);
                        }
                    });
                    
//
////                    if (!mtt::point_query_narrow_with_exclusion(c->system, prev_world, &c_against, hit, &min_area, &dt_ctx.scn_ctx.selected_things, sel_thing->id)) {
////                        continue;
////                    }
//                    mtt::AABB_AABB_intersection(c, <#AABB *b#>)
//
//                    mtt::Thing_ID thing_against = (mtt::Thing_ID)c_against->user_data
//
//                    mtt::Thing* parent = against;
//                    mtt::connect_parent_to_child(thing->world, parent, child);
                    
//                    auto* twn_ctx = mtt_core_ctx()->tween_ctx;
//                    twn_Opt opt = {};
//                    opt.time = 0.5f;
//                    opt.ease = TWN_QUADINOUT;
//                    opt.do_forwards_and_backwards = 0;
//                    opt.abort_if_in_progress = 1;
//
//
//                    struct Restore {
//                        mtt::World* world;
//                        mtt::Thing_ID id;
//                        vec4 color_factor;
//                        vec4 color_addition;
//                    };
//
//                    opt.callback = [](void* data) {
//                        Restore* restore = (Restore*)data;
//
//                        mtt::Thing* thing = restore->world->Thing_try_get(restore->id);
//                        if (thing != nullptr) {
//                            mtt::rep(thing)->render_data.drawable_info_list[0]->color_factor = restore->color_factor;
//                            mtt::rep(thing)->render_data.drawable_info_list[0]->color_addition = restore->color_addition;
//                        }
//                        delete restore;
//                    };
//                    auto* args = new Restore();
//                    args->world = child->world;
//                    args->id    = child->id;
//                    args->color_factor   = mtt::rep(child)->render_data.drawable_info_list[0]->color_factor;
//                    args->color_addition = mtt::rep(child)->render_data.drawable_info_list[0]->color_addition;
//                    opt.user_data = args;
//
//                    mtt::rep(child)->render_data.drawable_info_list[0]->color_factor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
//
//                    for (uint32 comp = 0; comp < 3; comp += 1) {
//                        twn_add_f32(twn_ctx, &(mtt::rep(child)->render_data.drawable_info_list[0]->color_addition[comp]), 0.5f, opt);
//                    }
//                    twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
                    
                    
                    break;

                }
#endif
            } else {
                MTT_print("%s", "Nothing to apply\n");
            }
            
            dt_ctx.ui.margin_panels[0].text.delete_mode_on = true;

            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
        ui_button->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
        };
        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            {
                static mtt::Collider_List out_canvas;
                out_canvas.clear();
                
                bool was_hit = false;
                
                auto* collision_system_canvas = &thing->world()->collision_system_canvas;
                
                vec2 pos = {};
                
                //bool broad_hit_canvas = mtt::point_query(collision_system_canvas, 0, canvas_pos, &out_canvas);
                
                //if (!broad_hit_canvas) {
                //    return;
                //}
                
                mtt::Collider* c_hit = nullptr;
                
                float32 min_area = POSITIVE_INFINITY;
                
                auto& dt_ctx = *dt::DrawTalk::ctx();
                
                mtt::Hit hit;
                was_hit = mtt::point_query_narrow(collision_system_canvas, canvas_pos, &c_hit, hit, &min_area, &dt_ctx.scn_ctx.selected_things);
                
                pos = canvas_pos;
                if (was_hit) {
                    if (thing->id != (mtt::Thing_ID)c_hit->user_data) {
                    }
                } else {

                    
                }
            }
        };
    }
    
    
//    {
//        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("draw arrows")->second]->thing);
//        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
//            
//            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
//            
//            dt::DrawTalk* dt_ctx = dt::DrawTalk::ctx();
//            
//            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
//            twn_Opt opt = {};
//            opt.time = 0.5f;
//            opt.ease = TWN_QUADINOUT;
//            opt.do_forwards_and_backwards = 0;
//            opt.abort_if_in_progress = 1;
//            
//            for (uint32 comp = 0; comp < 3; comp += 1) {
//                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
//            }
//            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
//            
//            return true;
//        };
//        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
//            
//            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
//            
//            dt::DrawTalk* dt_ctx = dt::DrawTalk::ctx();
//            
//            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
//            twn_Opt opt = {};
//            opt.time = 0.25f;
//            opt.ease = TWN_QUADIN;
//            for (uint32 comp = 0; comp < 3; comp += 1) {
//                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 1.0, opt);
//            }
//            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 1.0f, opt);
//        };
//    }
    
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("toggle labels")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            dt::DrawTalk* dt_ctx = dt::DrawTalk::ctx();
            mtt::toggle_verbose_display(dt_ctx->mtt);
            
            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
        ui_button->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
        };
        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            {
                static mtt::Collider_List out_canvas;
                out_canvas.clear();
                
                bool was_hit = false;
                
                auto* collision_system_canvas = &thing->world()->collision_system_canvas;
                
                vec2 pos = {};
                
                //bool broad_hit_canvas = mtt::point_query(collision_system_canvas, 0, canvas_pos, &out_canvas);
                
                //if (!broad_hit_canvas) {
                //    return;
                //}
                
                mtt::Collider* c_hit = nullptr;
                
                float32 min_area = POSITIVE_INFINITY;
                
                auto& dt_ctx = *dt::DrawTalk::ctx();
                
                mtt::Hit hit;
                was_hit = mtt::point_query_narrow(collision_system_canvas, canvas_pos, &c_hit, hit, &min_area, &dt_ctx.scn_ctx.selected_things);
                
                pos = canvas_pos;
                if (was_hit) {
                    if (thing->id != (mtt::Thing_ID)c_hit->user_data) {
                    }
                } else {
                    
                    
                }
            }
        };
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("toggle show attached")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            dt::DrawTalk* dt_ctx = dt::DrawTalk::ctx();
            mtt::toggle_attachment_links_display(dt_ctx->mtt);
            
            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
        ui_button->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
        };
        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            {
                static mtt::Collider_List out_canvas;
                out_canvas.clear();
                
                bool was_hit = false;
                
                auto* collision_system_canvas = &thing->world()->collision_system_canvas;
                
                vec2 pos = {};
                
                //bool broad_hit_canvas = mtt::point_query(collision_system_canvas, 0, canvas_pos, &out_canvas);
                
                //if (!broad_hit_canvas) {
                //    return;
                //}
                
                mtt::Collider* c_hit = nullptr;
                
                float32 min_area = POSITIVE_INFINITY;
                
                auto& dt_ctx = *dt::DrawTalk::ctx();
                
                mtt::Hit hit;
                was_hit = mtt::point_query_narrow(collision_system_canvas, canvas_pos, &c_hit, hit, &min_area, &dt_ctx.scn_ctx.selected_things);
                
                pos = canvas_pos;
                if (was_hit) {
                    if (thing->id != (mtt::Thing_ID)c_hit->user_data) {
                    }
                } else {
                    
                    
                }
            }
        };
    }
#if CAMERA
    {
        mtt::Thing* ui_button2 = world->Thing_get(dock.buttons[label_to_index.find("enable camera")->second]->thing);
        mtt::Any* ar_ctx = mtt::access<mtt::Any>(ui_button2, "state0");
        ar_ctx->type = mtt::MTT_POINTER;
        ar_ctx->Reference_Type = reinterpret_cast<uintptr>(&core->ar_ctx);
        
        mtt::Any* ar_on_flag = mtt::access<mtt::Any>(ui_button2, "state1");
        ar_on_flag->type = mtt::MTT_BOOLEAN;
        ar_on_flag->Boolean = false;
        
#if (USE_OLD_BUTTON_COLORS_UI)
        mtt::rep(ui_button2)->render_data.drawable_info_list[0]->set_color_factor(vec4(vec3(0.5f), 1.0f));
#endif
        
        sd::set_render_layer(world->renderer, LAYER_LABEL_STATIC_CANVAS);
        sd::begin_polygon(world->renderer);
        sd::set_color_rgba_v4(world->renderer, vec4(0.1f, 0.1f, 0.1f, 0.4f));
        //sd::rectangle_rounded(world->renderer, vec2(0.0f), vec2(core->viewport.width, core->viewport.height), -990.0f, 0.2f, 4);
        SD_vec2 depths = sd::depth_range_default(world->renderer);
        sd::rectangle(world->renderer, vec2(0.0f), vec2(core->viewport.width, core->viewport.height), depths[0]);
        filter = sd::end_polygon(world->renderer);
        filter->is_enabled = false;

        ui_button2->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            //mtt_core_ctx()->tween_ctx
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            //Augmented_Reality_pause(&core->ar_ctx);
            mtt::Augmented_Reality_Context* ar_ctx = reinterpret_cast<mtt::Augmented_Reality_Context*>(mtt::access<mtt::Any>(thing, "state0")->Reference_Type);
            auto* ar_on_flag = &(mtt::access<mtt::Any>(thing, "state1")->Boolean);
            if (*ar_on_flag == false) {
                Augmented_Reality_config(ar_ctx, mtt::AUGMENTED_REALITY_FLAG_FACE_TRACKING);
                if (!Augmented_Reality_run(ar_ctx)) {
                    ctx_ptr->label = "camera unavail.";
                    return true;
                }
                
                *ar_on_flag = true;

                ctx_ptr->label = "disable camera";
                
                //vec4(1.0f, 0.0f, 0.0f, 0.5f)
                //
                auto* twn_ctx = mtt_core_ctx()->tween_ctx;
                twn_Opt opt = {};
                opt.time = 0.25f;
                opt.ease = TWN_QUADIN;
                for (uint32 comp = 0; comp < 3; comp += 1) {
                    twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 1.0, opt);
                }
                twn_add_f32(twn_ctx, &ctx_ptr->alpha, 1.0f, opt);
                
                filter->is_enabled = true;
            } else {
                *ar_on_flag = false;
                Augmented_Reality_pause(ar_ctx);
                ctx_ptr->label = "enable camera";
                
                auto* twn_ctx = mtt_core_ctx()->tween_ctx;
                twn_Opt opt = {};
                opt.time = 0.25f;
                opt.ease = TWN_QUADOUT;
                for (uint32 comp = 0; comp < 3; comp += 1) {
                    twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5, opt);
                }
                twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
                
                filter->is_enabled = false;
            }
            
            return true;
        };
    }
#endif
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("toggle system labels")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            dt::DrawTalk* dt_ctx = dt::DrawTalk::ctx();
            mtt::toggle_debug_display(dt_ctx->mtt);
            
            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;
            
            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
        ui_button->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
        };
        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            {
                static mtt::Collider_List out_canvas;
                out_canvas.clear();
                
                bool was_hit = false;
                
                auto* collision_system_canvas = &thing->world()->collision_system_canvas;
                
                vec2 pos = {};
                
                //bool broad_hit_canvas = mtt::point_query(collision_system_canvas, 0, canvas_pos, &out_canvas);
                
                //if (!broad_hit_canvas) {
                //    return;
                //}
                
                mtt::Collider* c_hit = nullptr;
                
                float32 min_area = POSITIVE_INFINITY;
                
                auto& dt_ctx = *dt::DrawTalk::ctx();
                
                mtt::Hit hit;
                was_hit = mtt::point_query_narrow(collision_system_canvas, canvas_pos, &c_hit, hit, &min_area, &dt_ctx.scn_ctx.selected_things);
                
                pos = canvas_pos;
                if (was_hit) {
                    if (thing->id != (mtt::Thing_ID)c_hit->user_data) {
                    }
                } else {
                    
                    
                }
            }
        };
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("pause")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            //dt::DrawTalk* dt_ctx = dt::DrawTalk::ctx();
            if (MTT_Core_toggle_pause_and_check(mtt_core_ctx())) {
                // just paused
                ctx_ptr->label = "unpause";
            } else {
                // just unpaused
                ctx_ptr->label = "pause";
            }
            
//            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
//            twn_Opt opt = {};
//            opt.time = 0.5f;
//            opt.ease = TWN_QUADINOUT;
//            opt.do_forwards_and_backwards = 1;
//            opt.abort_if_in_progress = 1;
//            
//            for (uint32 comp = 0; comp < 3; comp += 1) {
//                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
//            }
//            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
        ui_button->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
        };
        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            {
                static mtt::Collider_List out_canvas;
                out_canvas.clear();
                
                bool was_hit = false;
                
                auto* collision_system_canvas = &thing->world()->collision_system_canvas;
                
                vec2 pos = {};
                
                //bool broad_hit_canvas = mtt::point_query(collision_system_canvas, 0, canvas_pos, &out_canvas);
                
                //if (!broad_hit_canvas) {
                //    return;
                //}
                
                mtt::Collider* c_hit = nullptr;
                
                float32 min_area = POSITIVE_INFINITY;
                
                auto& dt_ctx = *dt::DrawTalk::ctx();
                
                mtt::Hit hit;
                was_hit = mtt::point_query_narrow(collision_system_canvas, canvas_pos, &c_hit, hit, &min_area, &dt_ctx.scn_ctx.selected_things);
                
                pos = canvas_pos;
                if (was_hit) {
                    if (thing->id != (mtt::Thing_ID)c_hit->user_data) {
                    }
                } else {
                    
                    
                }
            }
        };
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("save thing")->second]->thing);
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            auto* mtt_world = dt_ctx.mtt;
            
            bool something_saved = false;
            for (auto& selection : dt_ctx.recorder.selection_recording.selections) {
                auto thing_id = selection.ID;
                mtt::Thing* thing = mtt::Thing_try_get(mtt_world, thing_id);
                if (thing == nullptr) {
                    continue;
                }
                
                bool status_ok = mtt::Thing_save_as_preset(thing);
                if (!status_ok) {
                    MTT_error("%s\n", "could not save thing as preset");
                    continue;
                }
                
                something_saved = true;
            }
            
            if (!something_saved || mtt::selection_count(thing) > 1) {
                return true;
            }
            
            auto* twn_ctx = mtt_core_ctx()->tween_ctx;
            twn_Opt opt = {};
            opt.time = 0.5f;
            opt.ease = TWN_QUADINOUT;
            opt.do_forwards_and_backwards = 1;
            opt.abort_if_in_progress = 1;

            for (uint32 comp = 0; comp < 3; comp += 1) {
                twn_add_f32(twn_ctx, &(mtt::rep(thing)->render_data.drawable_info_list[0]->get_color_factor()[comp]), 0.5f, opt);
            }
            twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
            
            return true;
        };
//        ui_button->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
//        };
//        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
//        };
    }
    {
        mtt::Thing* ui_button = world->Thing_get(dock.buttons[label_to_index.find("color")->second]->thing);
        mtt::Rep* rep = mtt::rep(ui_button);
        for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
            (*it)->set_color_factor(vec4(1.0f, 1.0f, 1.0f, 0.2f));
        }
        ui_button->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            
            mtt::Rep* rep = mtt::rep(thing);
            mtt::Collider* c = rep->colliders.front();
            
            float32 offset = 10.0f;
            MTT_UNUSED(offset);
            
            auto* box = &c->aabb;
            
            vec2 tl = c->aabb.saved_box.tl;
            vec2 br = c->aabb.saved_box.br;

#if USE_OLD_TEXT_PANEL_LOCATION

            
            if (canvas_pos.y + offset <= tl.y) {
                dt_ctx.pen.color = dt::pen_color_default();
            } else {
                float32 dist = (canvas_pos.y - tl.y) / (br.y - tl.y - offset);
                dist = 1.0 - m::clamp(dist, 0.0f, 1.0f);
                
                dt_ctx.pen.color = HSVtoRGBinplace((uint32)(dist * 359), 1.0, 1.0, 1.0);
            }
#else
//            auto* rend = mtt::ctx()->renderer;
//            sd::set_render_layer(rend, LAYER_LABEL_CANVAS_PER_FRAME);
//            sd::begin_polygon(rend);
//            sd::set_color_rgba_v4(rend, vec4(1.0f, 0.0f, 0.0f, 1.0f));
//            sd::circle(rend, 24.0f, vec3(tl, 999.0f));
//            sd::set_color_rgba_v4(rend, vec4(0.0f, 1.0f, 0.0f, 1.0f));
//            sd::circle(rend, 24.0f, vec3(br, 999.0f));
//            sd::end_polygon(rend);
            

            if (canvas_pos.x + offset >= br.x) {
                dt_ctx.pen.color = dt::pen_color_default();
            } else {
                float32 dist = (canvas_pos.x - tl.x) / (br.x - tl.x - offset);
                dist = m::clamp(dist, 0.0f, 1.0f);
                
                dt_ctx.pen.color = HSVtoRGBinplace((uint32)(dist * 359), 1.0, 1.0, 1.0);
            }
#endif
            for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
                (*it)->set_color_factor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
            
            return true;
        };
        
        ui_button->input_handlers.on_touch_input_moved = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            
            mtt::Rep* rep = mtt::rep(thing);
            mtt::Collider* c = rep->colliders.front();
            
            float32 offset = 10.0f;
            MTT_UNUSED(offset);
            
            auto* box = &c->aabb;
            
            vec2 tl = c->aabb.saved_box.tl;
            vec2 br = c->aabb.saved_box.br;

#if USE_OLD_TEXT_PANEL_LOCATION

            
            if (canvas_pos.y + offset <= tl.y) {
                dt_ctx.pen.color = dt::pen_color_default();
            } else {
                float32 dist = (canvas_pos.y - tl.y) / (br.y - tl.y - offset);
                dist = 1.0 - m::clamp(dist, 0.0f, 1.0f);
                
                dt_ctx.pen.color = HSVtoRGBinplace((uint32)(dist * 359), 1.0, 1.0, 1.0);
            }
#else
//            auto* rend = mtt::ctx()->renderer;
//            sd::set_render_layer(rend, LAYER_LABEL_CANVAS_PER_FRAME);
//            sd::begin_polygon(rend);
//            sd::set_color_rgba_v4(rend, vec4(1.0f, 0.0f, 0.0f, 1.0f));
//            sd::circle(rend, 24.0f, vec3(tl, 999.0f));
//            sd::set_color_rgba_v4(rend, vec4(0.0f, 1.0f, 0.0f, 1.0f));
//            sd::circle(rend, 24.0f, vec3(br, 999.0f));
//            sd::end_polygon(rend);
            

            if (canvas_pos.x + offset >= br.x) {
                dt_ctx.pen.color = dt::pen_color_default();
            } else {
                float32 dist = (canvas_pos.x - tl.x) / (br.x - tl.x - offset);
                dist = m::clamp(dist, 0.0f, 1.0f);
                
                dt_ctx.pen.color = HSVtoRGBinplace((uint32)(dist * 359), 1.0, 1.0, 1.0);
            }
#endif
            for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
                (*it)->set_color_factor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        };
        
        ui_button->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
            MTT_print("Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
        
            
            auto& dt_ctx = *dt::DrawTalk::ctx();
            
            mtt::Rep* rep = mtt::rep(thing);
            mtt::Collider* c = rep->colliders.front();
            
            float32 offset = 10.0f;
            
            auto* box = &c->aabb;
            
            vec2 tl = box->tl + vec2(0.0f, 10.0f);
            vec2 br = box->br;
            
            for (auto it = rep->render_data.drawable_info_list.begin(); it != rep->render_data.drawable_info_list.end(); ++it) {
                (*it)->set_color_factor(vec4(1.0f, 1.0f, 1.0f, 0.2f));
            }
        };
        
        ui_button->input_handlers.on_pen_input_began = ui_button->input_handlers.on_touch_input_began;
        ui_button->input_handlers.on_pen_input_moved = ui_button->input_handlers.on_touch_input_moved;
        ui_button->input_handlers.on_pen_input_ended = ui_button->input_handlers.on_touch_input_ended;
    }
    

    if (true)
    {
        mtt::Rep* rep = nullptr;
        mtt::Thing* substitution_menu = mtt::Thing_make_with_aabb_dimensions(world, mtt::ARCHETYPE_UI_ELEMENT, vec2(1.0f), &rep, vec3(0.0f, 0.0f, 990.0f), vec2(1.0f, 1.0f), false, &ui.top_panel.collision_system_canvas);
        substitution_menu->is_user_drawable     = false;
        substitution_menu->is_user_destructible = false;
        substitution_menu->is_user_movable      = false;
        
        substitution_menu->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing *thing, vec3 world_pos, vec2 canvas_pos, UI_Event *event, UI_Touch *touch, uint64 flags) -> mtt::Input_Handler_Return {
            auto& menu = dt::DrawTalk::ctx()->ui.element_menu;
            menu.touch_began = true;
            menu.touch_ended = false;
            
            return true;
        };
        
        substitution_menu->input_handlers.on_pen_input_began = [](mtt::Input_Handlers* in, mtt::Thing *thing, vec3 world_pos, vec2 canvas_pos, UI_Event *event, UI_Touch *touch, uint64 flags) -> mtt::Input_Handler_Return {
            dt::DrawTalk::ctx()->ui.element_menu.pen_ended = false;
            auto& menu = dt::DrawTalk::ctx()->ui.element_menu;
            menu.pen_began = true;
            menu.pen_ended = false;
            
            return true;
        };
        
        substitution_menu->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing *thing, vec3 world_pos, vec2 canvas_pos, UI_Event *event, UI_Touch *touch, uint64 flags) {
            auto& menu = dt::DrawTalk::ctx()->ui.element_menu;
            menu.touch_began = false;
            menu.touch_ended = true;
            menu.touch_end_canvas_pos = canvas_pos;
        };
        
        substitution_menu->input_handlers.on_pen_input_ended = [](mtt::Input_Handlers* in, mtt::Thing *thing, vec3 world_pos, vec2 canvas_pos, UI_Event *event, UI_Touch *touch, uint64 flags) {
            auto& menu = dt::DrawTalk::ctx()->ui.element_menu;
            menu.pen_began = false;
            menu.pen_ended = true;
            menu.pen_end_canvas_pos = canvas_pos;
        };
        
        dt->ui.element_menu.thing_id = substitution_menu->id;
        //dt->ui.element_menu.priority_layer = 1;
        mtt::Thing_set_position(substitution_menu, vec3(viewport.width * 0.5f, viewport.height * 0.5f, 990.0f));
//        for (auto c_it = rep->colliders.begin(); c_it != rep->colliders.end(); ++c_it) {
//            (*c_it)->priority_layer = dt->ui.element_menu.priority_layer;
//        }
        mtt::set_pose_transform(substitution_menu, m::scale(mat4(1.0f), vec3(viewport.width, viewport.height, 1.0f)));
        
        for (usize c_i = 0; c_i < rep->colliders.size(); c_i += 1) {
            mtt::Collider* c = rep->colliders[c_i];
            mtt::Thing_ID id = (mtt::Thing_ID)c->user_data;
            mtt::Collider_remove(nullptr, 0, rep->colliders[c_i]);
        }
        mtt::unset_is_active(substitution_menu);
    }
    
    
    // MARK: load predefined behaviors
    mtt::Thing_Archetype* arch = nullptr;
    Thing_Archetype_try_get(world, mtt::ARCHETYPE_FREEHAND_SKETCH, &arch);
    
    
    struct ARGS_Event {
        mtt::World* world = nullptr;
        mtt::Thing_ID thing_id_src = mtt::Thing_ID_INVALID;
        mtt::Thing_ID thing_id_tgt = mtt::Thing_ID_INVALID;
        Word_Dictionary_Entry* ev_add;
        Word_Dictionary_Entry* ev_remove;
    };
    arch->input_handlers.on_touch_input_began = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> mtt::Input_Handler_Return {
        
        mtt::World* world = mtt::world(thing);
        
        auto& allocator = *mtt::buckets_allocator(world);
        
        
        auto* args = mem::alloc_init<ARGS_Event>(&allocator);
        
        args->world = world;
        args->thing_id_src = DrawTalk_World_ctx()->system_thing_I;
        args->thing_id_tgt = mtt::thing_id(thing);
        args->ev_add = world->select;
        args->ev_remove = verb_add("deselect");
        
        mtt::send_system_message_deferred(&world->message_passer, mtt::MTT_NONE, args->thing_id_src, args, mtt::Procedure_make([](void* state, mtt::Procedure_Input_Output* io) {
            
            ARGS_Event* args = static_cast<ARGS_Event*>(mtt::Message_contents(mtt::Message_from(io)));
            
            mtt::World* world = args->world;
            auto& allocator = *mtt::buckets_allocator(world);
            
            mtt::Thing* t_src = mtt::Thing_get(world, args->thing_id_src);
            mtt::Thing* t_tgt = mtt::Thing_try_get(world, args->thing_id_tgt);
            if (t_tgt == nullptr) {
                
                mem::deinit_deallocate<ARGS_Event>(mtt::buckets_allocator(world), args);
                return mtt::Procedure_Return_Type();
            }
            
            
            Thing_add_action(t_src, args->ev_add, t_tgt);
            Thing_remove_action(t_src, args->ev_remove, t_tgt);
            
            mem::deinit_deallocate<ARGS_Event>(mtt::buckets_allocator(world), args);

            return mtt::Procedure_Return_Type();
        }));
        
        
        return true;
    };
    arch->input_handlers.on_touch_input_ended = [](mtt::Input_Handlers* in, mtt::Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> void {
        
        mtt::World* world = mtt::world(thing);
        
        auto& allocator = *mtt::buckets_allocator(world);
        
        auto* args = mem::alloc_init<ARGS_Event>(&allocator);
        
        args->world = world;
        args->thing_id_src = DrawTalk_World_ctx()->system_thing_I;
        args->thing_id_tgt = mtt::thing_id(thing);
        args->ev_remove = world->select;
        args->ev_add = verb_add("deselect");
        
        mtt::send_system_message_deferred(&world->message_passer, mtt::MTT_NONE, args->thing_id_src, args, mtt::Procedure_make([](void* state, mtt::Procedure_Input_Output* io) {
            
            ARGS_Event* args = static_cast<ARGS_Event*>(mtt::Message_contents(mtt::Message_from(io)));
            
            mtt::World* world = args->world;
            
            mtt::Thing* t_src = mtt::Thing_get(world, args->thing_id_src);
            mtt::Thing* t_tgt = mtt::Thing_try_get(world, args->thing_id_tgt);
            if (t_tgt == nullptr) {
                mem::deinit_deallocate<ARGS_Event>(mtt::buckets_allocator(world), args);
                return mtt::Procedure_Return_Type();
            }
            
            
            Thing_add_action(t_src, args->ev_add, t_tgt);
            Thing_remove_action(t_src, args->ev_remove, t_tgt);
            
            mem::deinit_deallocate<ARGS_Event>(mtt::buckets_allocator(world), args);

            return mtt::Procedure_Return_Type();
        }));
    };
    
}


void on_load(mtt::String& path, mtt::String& data)
{
    MTT_print("ct::on_load path=[%s]\n", path.c_str());
    
    usize size = 4096 * 2;
    ArduinoJson::DynamicJsonDocument doc(size);
    DeserializationError error = deserializeJson(doc, data);
    while (error) {
        if (strcmp(error.c_str(), "NoMemory") != 0) {
            MTT_error("%s", "JSON deserialization memory issue\n");
            break;
        } else {
            doc.clear();
        }
        
        size = size * 2;
        doc = ArduinoJson::DynamicJsonDocument(size);
        error = deserializeJson(doc, data);
    }
    
}



void on_query_result(mtt::String& data, usize ID)
{
    //ASSERT_MSG(mtt::is_main_thread(), "should be on main thread");
    MTT_print("Got query result ID=%llu\n", ID);
    dt::DrawTalk* ctx = dt::DrawTalk::ctx();
    //MTT_print("ct::on_query_result [%s]\n", data.c_str());
    
    ArduinoJson::DynamicJsonDocument doc(200000);
    DeserializationError error = deserializeJson(doc, data.c_str());
    
    // Test if parsing succeeds.
    if (error) {
        MTT_error("deserializeJson() failed: %s\n", error.c_str());
        auto it = ctx->word_data_query_map.find(ID);
        if (it != ctx->word_data_query_map.end()) {
            ctx->word_data_query_map.erase(it);
        }
        return;
    }
    
    
    
    
    //   usize ID = dt->core->query_for_data(dt->core, word + " " + part_of_speech);
    //  dt->word_data_query_map.insert({ID, proc});
    
    auto it = ctx->word_data_query_map.find(ID);
//    for (auto bla = ctx->word_data_query_map.begin(); bla != ctx->word_data_query_map.end(); ++bla) {
//        std::cout << bla->first << std::endl;
//    }
    if (it != ctx->word_data_query_map.end()) {
        auto& proc = it->second;
        proc(data, static_cast<void*>(&doc), ID);
        ctx->word_data_query_map.erase(it);
    } else {
        mtt::String debug_print = "";
        serializeJsonPretty(doc, debug_print);
        std::cout << debug_print << std::endl;
        std::cout << "BUT SOMETHING IS WRONG HERE" << std::endl;
    }
    doc.clear();
}


Language_Context::Language_Context()
{
    
}

mem::Pool_Allocation Instruction::pool = (mem::Pool_Allocation){};

mem::Pool_Allocation Command::pool = (mem::Pool_Allocation){};



void on_frame(void)
{
//    auto& to_remove = dt::DrawTalk::ctx()->relations_to_remove;
//    for (usize i = 0; i < to_remove.size(); i += 1) {
//        to_remove[i].procedure();
//    }
//    to_remove.clear();
    dt::UI_draw_pre();
    dt::UI_draw();
    dt::UI_draw_post();
}




CONFIRM_PHASE get_confirm_phase()
{
    return dt::DrawTalk::ctx()->lang_ctx.eval_ctx.phase;
}

void set_confirm_phase(CONFIRM_PHASE phase)
{
    dt::DrawTalk::ctx()->lang_ctx.eval_ctx.phase = phase;
}

Command_ID Command::next_id = Command_ID_INVALID;
mtt::Map_Stable<Command_ID, Command*> Command::commands = {};
std::unordered_map<mtt::Thing_ID, dt::Dynamic_Array<Command_ID>> Command::thing_agent_to_cmd = {};



std::vector<RegisteredFunc>& DT_Behavior_preloaded_behavior_configs(void)
{
    return DT_Behavior_Config::list();
}

void DT_Behavior_Catalogue_register_module(RegisteredFunc rf)
{
    DT_Behavior_Config::list().push_back(rf);
}


DT_Behavior::Args DT_Behavior::Args::make(dt::DrawTalk* dt, DT_Behavior_Catalogue* catalogue, DT_Behavior* behavior, Speech_Property* current, dt::Dynamic_Array<Speech_Property*>* siblings, Speech_Property* root, Command_List* cmd_list)
{
    DT_Behavior::Args args = DT_Behavior::Args();
    {
        args.world    = dt->mtt;
        args.dt       = dt;
        args.catalogue = catalogue;
        args.behavior = behavior;
        args.current  = current;
        args.siblings = siblings;
        args.root     = root;
        args.cmd_list = cmd_list;
        args.lang_ctx = &dt->lang_ctx;
        args.eval_ctx = &dt->lang_ctx.eval_ctx;
    }
    return args;
}


void handle_speech_phase_change(dt::DrawTalk& dt_ctx, mtt::Thing* thing)
{
    UI_Button* ctx_ptr = *mtt::access_pointer_to_pointer<UI_Button*>(thing, "ctx_ptr");
    MTT_print(">>>Pressed button with label=[%s]\n", ctx_ptr->label.c_str());
    
    mtt::set_should_render(thing);
    mtt::set_is_active(thing);
    
    static vec4 colors[] = {
        RGBint_toRGBfloat(vec4(0, 181, 204, 255/2)),
        RGBint_toRGBfloat(vec4(0, 181, 0, 255/2)),
        RGBint_toRGBfloat(vec4(0, 181, 204, 255/2)),
    };
    
    
    auto* dinfo = mtt::rep(thing)->render_data.drawable_info_list[0];
    
    
    auto handle_text_entity = [&](Speech_Info* speech_info) {
        auto* DT = &dt_ctx;
        for (auto it = DT->scn_ctx.selected_things.begin(); it != DT->scn_ctx.selected_things.end(); ++it) {
            mtt::Thing_ID other_id = *it;
            if (other_id == mtt::Thing_ID_INVALID || other_id == thing->id) {
                continue;
            }
            mtt::Thing* other_thing = thing->world()->Thing_try_get(other_id);
            if (other_thing == nullptr) {
                continue;
            }
            
            if (other_thing->archetype_id == mtt::ARCHETYPE_TEXT) {
                {
                    auto* val_field = mtt::access<MTT_String_Ref>(other_thing, "value");
                    if (val_field != nullptr) {
                        cstring in_cstr = MTT_string_ref_to_cstring(*val_field);
                        mtt::String in = mtt::String(in_cstr);
                        if (!in.empty() && strcmp(in_cstr, mtt::text_thing_empty_string) != 0) {
                            Speech_process_text_as_input(speech_info, in, 0);
                            return true;
                        }
                        
                    }
                }
                
            }
        }
        
        return false;
    };
    
    dt_ctx.lang_ctx.deixis_reset();
    auto& text_panel = dt_ctx.ui.margin_panels[0].text;
    if (text_panel.is_overriding_speech()) {
        if (!dt_ctx.lang_ctx.parse_q().empty()) {
            Regenerate_Operation op;
            op.do_send = true;
            op.type = Regenerate_Operation::TYPE::CONFIRM;
            dt::regenerate_text_view(&dt_ctx, *(dt_ctx.lang_ctx.parse_q().back()), op);
        }
        dt_ctx.ui.margin_panels[0].text.set_is_overriding_speech(false);
    } else {
        auto* speech_info = &dt_ctx.core->speech_system.info_list[0];
        if (get_confirm_phase() == CONFIRM_PHASE_SPEECH) {
            
            if (handle_text_entity(speech_info)) {
                auto* twn_ctx = mtt_core_ctx()->tween_ctx;
                twn_Opt opt = {};
                opt.time = 0.5f;
                opt.ease = TWN_QUADINOUT;
                opt.do_forwards_and_backwards = 1;
                opt.abort_if_in_progress = 1;
                for (uint32 comp = 0; comp < 3; comp += 1) {
                    twn_add_f32(twn_ctx, &(dinfo->get_color_factor()[comp]), 0.5f, opt);
                }
                twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
                //dt_ctx.is_waiting_on_results = true;
                return;
            } else if (Speech_split(speech_info)) {
                Speech_set_active_state(speech_info, false);
                
                
                //                        if (dt_ctx.is_waiting_on_results) {
                //                            ctx_ptr->label = "not ready";
                //                        }
                //set_confirm_phase(CONFIRM_PHASE_ANIMATE);
                
                
                
                dt_ctx.is_waiting_on_results = true;
                
                ctx_ptr->label = DT_SPEECH_COMMAND_BUTTON_NAME;
                ctx_ptr->alpha = 1.0f;
                
                dt::DrawTalk::ctx()->lang_ctx.deixis_reset();
            } else {
                //                        if (!dt_ctx.is_waiting_on_results) {
                //                            dt::run(&dt_ctx);
                //                        }
                //return;
            }
            
        } else if (get_confirm_phase() == CONFIRM_PHASE_ANIMATE && !dt_ctx.is_waiting_on_results) {
            
            
            //                    mtt::job_dispatch_serial((void*)ctx_ptr, [](void* ctx) {
            //                    }, [](void* ctx)
            {
                //MTT_print("%s", "running\n");
                auto& dt_ctx = *dt::DrawTalk::ctx();
                //auto* ctx_ptr = (UI_Button*)ctx;
                
                //ASSERT_MSG(mtt::is_main_thread(), "Should not happen in a separate thread");
                
                if (!dt_ctx.lang_ctx.eval_ctx.instructions_to_disambiguate.empty()) {
                    
                    Speech_set_active_state(speech_info, false);
                    
                    prompt_for_different_actions(&dt_ctx);
                    
//                    for (auto rit = dt_ctx.lang_ctx.eval_ctx.instructions_to_disambiguate.rbegin(); rit != dt_ctx.lang_ctx.eval_ctx.instructions_to_disambiguate.rend(); ++rit) {
//                        Instruction* ins = rit->ins;
//                        //ins->display_label += ":(?)";
//                    }
                    //rebuild_feedback_ui_from_modified_instructions(&dt_ctx, dt_ctx.lang_ctx.eval_ctx);
                    mtt::unset_should_render(thing);
                    mtt::unset_is_active(thing);
                } else {
                    for (usize i = 0; i < dt_ctx.lang_ctx.eval_ctx.instructions_saved().size(); i += 1) {
                        auto* ins = dt_ctx.lang_ctx.eval_ctx.instructions_saved()[i];

                        if (Instruction_requires_selection_of_things(ins)) {
                            mtt::unset_should_render(thing);
#if !DT_DEFINE_FLEXIBLE_IGNORE_SELECTIONS
                            mtt::unset_is_active(thing);

                            goto LABEL_CONFIRM_PHASE_ANIMATE_BLOCKED;
#endif
                        }

                    }
                    
                    dt::run(&dt_ctx);
                    
                    if (dt_ctx.lang_ctx.eval_ctx.prompting_for_different_actions) {
                        leave_prompt_for_different_actions(&dt_ctx);
                    }
                    
                    dt_ctx.lang_ctx.eval_ctx.instructions_to_disambiguate.clear();
                    
                    for (usize i = 0; i < dt_ctx.lang_ctx.eval_ctx.instructions_saved().size(); i += 1) {
                        mtt::Thing_destroy(dt_ctx.mtt, dt_ctx.lang_ctx.eval_ctx.instructions_saved()[i]->thing_id);
                        dt_ctx.lang_ctx.eval_ctx.instructions_saved()[i]->thing_id = mtt::Thing_ID_INVALID;
                        Instruction_on_destroy(&dt_ctx, dt_ctx.lang_ctx.eval_ctx.instructions_saved()[i]);
                    }
                    
                    mtt::Thing_destroy(dt_ctx.mtt, dt_ctx.lang_ctx.eval_ctx.root_thing_id);
                    dt_ctx.lang_ctx.eval_ctx.instructions_saved().clear();
                    dt_ctx.lang_ctx.eval_ctx.instructions().clear();
                    dt::text_view_clear(&dt_ctx);
                    {
//                        dt::Selection_Recorder_clear_selections_except_for_thing(&dt_ctx.recorder.selection_recording, dt_ctx.scn_ctx.thing_selected_drawable);
                        dt::Selection_Recorder_clear_location_selections(&dt_ctx.recorder.selection_recording);
                        //dt_ctx.scn_ctx.thing_selected_drawable = mtt::Thing_ID_INVALID;
                    }
                    
                    {
                        auto* th = dt_ctx.mtt->Thing_get(dt_ctx.ui.dock.buttons[dt_ctx.ui.dock.label_to_index.find("speech ignore")->second]->thing);
                        
                        mtt::Any* on_flag = mtt::access<mtt::Any>(th, "state0");
                        
                        Speech_set_active_state(&dt_ctx.core->speech_system.info_list[0], on_flag->Boolean);
                    }
                    
                    
                    set_confirm_phase(CONFIRM_PHASE_SPEECH);
                    
                    ctx_ptr->label = DT_SPEECH_COMMAND_BUTTON_NAME;
                    ctx_ptr->alpha = 1.0f;
                    
                    dt::DrawTalk::ctx()->lang_ctx.deixis_reset();
                    
                    
                    destroy_all_proxies(dt_ctx.mtt);
                }
            LABEL_CONFIRM_PHASE_ANIMATE_BLOCKED:;
            }
            //                    );
            
        } else if (dt_ctx.is_waiting_on_results) {
            //                    dt_ctx.deferred_confirm = [&]() {
            //                        dt::run(&dt_ctx);
            //
            //                        for (usize i = 0; i < dt_ctx.lang_ctx.eval_ctx.instructions_saved.size(); i += 1) {
            //                            mtt::Thing_destroy(dt_ctx.mtt, dt_ctx.lang_ctx.eval_ctx.instructions_saved[i]->thing_id);
            //                            dt_ctx.lang_ctx.eval_ctx.instructions_saved[i]->thing_id = mtt::Thing_ID_INVALID;
            //                            Instruction::destroy(&dt_ctx.lang_ctx.eval_ctx.instructions_saved[i]);
            //                        }
            //                        mtt::Thing_destroy(dt_ctx.mtt, dt_ctx.lang_ctx.eval_ctx.root_thing_id);
            //                        dt_ctx.lang_ctx.eval_ctx.instructions_saved.clear();
            //
            //                        dt::text_view_clear(&dt_ctx);
            //                        dt::Selection_Recorder_clear_selections(&dt_ctx.recorder.selection_recording);
            //
            //                        Speech_set_active_state(&dt_ctx.core->speech_system.info_list[0], true);
            //
            //                        ctx_ptr->label = DT_SPEECH_COMMAND_BUTTON_NAME;
            //                        set_confirm_phase(CONFIRM_PHASE_SPEECH);
            //
            //                        ctx_ptr->alpha = 1.0f;
            //
            //                        dt_ctx.deferred_confirm = []() {};
            //                    };
        }
        dinfo->set_color_factor(vec4(vec3(colors[get_confirm_phase()]), dinfo->get_color_factor()[3]));
        
        dt_ctx.ui.margin_panels[0].text.set_is_overriding_speech(false);
        
        
    }
    
    auto* twn_ctx = mtt_core_ctx()->tween_ctx;
    twn_Opt opt = {};
    opt.time = 0.5f;
    opt.ease = TWN_QUADINOUT;
    opt.do_forwards_and_backwards = 1;
    opt.abort_if_in_progress = 1;
    for (uint32 comp = 0; comp < 3; comp += 1) {
        twn_add_f32(twn_ctx, &(dinfo->get_color_factor()[comp]), 0.5f, opt);
    }
    twn_add_f32(twn_ctx, &ctx_ptr->alpha, 0.25f, opt);
    
    text_in_buf.clear();
    text_in_selected_all = false;
    text_in_cursor_idx = false;
    dt_ctx.ui.margin_panels[0].text.set_is_overriding_speech(false);
}

void handle_speech_phase_change(dt::DrawTalk& dt_ctx)
{
    dt::UI_Button* button = dt_ctx.ui.dock.label_to_button[DT_SPEECH_COMMAND_BUTTON_NAME];
    handle_speech_phase_change(dt_ctx, mtt::Thing_get(dt_ctx.mtt, button->thing));
}




mtt::Thing* word_multiply_procedure_make(dt::Word_Val_Pair words[], void* args)
{
    using namespace mtt;
    
    return code_procedure_make([](LOGIC_PROCEDURE_PARAM_LIST) -> Logic_Procedure_Return_Status {
        auto IN_arg     = get_in_port(thing, input, "in");
        auto IN_source  = get_in_port(thing, input, "in_source");
        auto OUT_arg    = get_out_port(thing, "out");
        
        
        
        
        if (IN_arg.status == PORT_STATUS_OK && IN_source.status == PORT_STATUS_OK) {
            mtt::Any& val = IN_arg.value.out;
            mtt::Any& src = IN_source.value.out;
            
            mtt::Thing_ID thing_src = src.thing_id;
            if (mtt::Thing* src = mtt::Thing_try_get(world, thing_src); src != nullptr) {
                
                
                dt::Word_Val_Pair* words = *(dt::Word_Val_Pair**)mtt::access<void*>(thing, "args");
                
                float32 attrib_val = val.Float;
                for (const dt::Word_Val_Pair* it = words; it->word != nullptr; it += 1) {
                    if (Thing_has_attribute(src, it->word)) {
                        auto* word = it->word;
                        
                        attrib_val *= it->value;
                        
                        auto* val = Thing_get_attribute_value(src, word);
                        if (val != nullptr && val->type != MTT_NONE) {
                            auto prev_attrib_val = attrib_val;
                            auto mult_val = val->Float;
                            if (mult_val == 0) {
                                attrib_val = 0;
                            } else {
                                const bool positive = (it->value >= 1);
                                attrib_val = attrib_val * ( (positive) ? mult_val : (1.0 / mult_val) );
                                if (isnan(attrib_val) || attrib_val == POSITIVE_INFINITY) {
                                    attrib_val = prev_attrib_val;
                                }
                            }
                        }
                    }
                }
                //MTT_print("initial val: %f, post: %f\n", val.Float, attrib_val);
                OUT_arg.value.out.set_Float(attrib_val);
                
                
                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
            }
        } else {
            OUT_arg.value.out = mtt::Any::from_Float(1.0f);
        }
        
        
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }, (void*)words);
}


DrawTalk* ctx(void)
{
    return DrawTalk::ctx();
}

MTT_Logger* logger(void)
{
    return &dt::ctx()->parse_logger;
}

bool Instruction_warn_multiple_for_specific_single(Instruction* ins)
{
    return ins->more_els_than_expected && !ins->is_plural && ins->is_specific && !ins->refers_to_random;
}

bool Instruction_requires_selection_of_things(Instruction* ins)
{
    return (ins->kind == "THING_INSTANCE_SELECTION" && ins->thing_id_list.empty() && ins->require_items_if_object);
}


bool handle_context_sensitive_command_clear(dt::DrawTalk* dt)
{
    dt::UI* ui = &dt->ui;
    auto* ctx_view = &ui->context_view;
    //DT_Rule_Query& search = ctx_view->search;
    
    mtt::String label = {};
    
    mtt::World* mtt = dt->mtt;
    
    mtt::Map<mtt::String, DT_Rule_Query>& queries = ctx_view->queries;
    
    for (auto& [label, query] : queries) {
        DT_Rule_Query_destroy(&query);
    }
    queries.clear();
    
    for (auto const& [thing_id, element] : ctx_view->thing_to_element) {
        mtt::Thing_destroy(mtt, element.proxy_id);
    }
    ctx_view->thing_to_element.clear();
    
    auto* proxy_scene = Thing_Proxy_Scene_for_idx(mtt, DT_THING_PROXY_SCENE_IDX_CONTEXT_PANEL_VIEW);
    
    proxy_scene->thing_to_proxy_map.clear();
    proxy_scene->proxy_aggregate.clear();
    
    
    return true;
}
bool handle_context_sensitive_command_rules(dt::DrawTalk* dt)
{
//    dt::UI* ui = &dt->ui;
//    auto* ctx_view = &ui->context_view;
//    
    return true;
}
bool handle_context_sensitive_command_actions(dt::DrawTalk* dt)
{
//    dt::UI* ui = &dt->ui;
//    auto* ctx_view = &ui->context_view;
//
    return true;
}

struct Find_Labels_Args_Do_Check {
    static constexpr const bool do_check = true;
};
struct Find_Labels_Args_Ignore_Check {
    static constexpr const bool do_check = false;
};
bool handle_context_sensitive_command(dt::DrawTalk* dt)
{
    dt::UI* ui = &dt->ui;
    auto* ctx_view = &ui->context_view;
    dt::Dynamic_Array<mtt::Thing_ID>& things_to_display = ctx_view->things_to_display;
    
    constexpr const float64 T_INTERVAL = CONTEXT_VIEW_T_INTERVAL;
    
    float64& time_elapsed = ctx_view->time_elapsed_since_update;
    float64 time_inc = dt->mtt->time_seconds - dt->mtt->time_seconds_prev;
    
    if (time_elapsed < T_INTERVAL) {
        time_elapsed += time_inc;
        return things_to_display.size() > 0;
    }
    do {
        time_elapsed -= T_INTERVAL;
    } while (time_elapsed >= T_INTERVAL);
    time_elapsed += time_inc;
    //ASSERT_MSG(false, "%s\n", "unfinished");
    
    //DT_Rule_Query& search = ctx_view->search;
    
    mtt::String label = {};
    
    mtt::World* mtt = dt->mtt;
    
    mtt::Map<mtt::String, DT_Rule_Query>& queries = ctx_view->queries;
    dt::Dynamic_Array<DT_Rule_Query*>& queries_this_frame = ctx_view->queries_this_frame;
    
    dt::Dynamic_Array<Context_View::Label_Record> labels = ctx_view->labels;
    labels.clear();
    mtt::Set<mtt::String> unique_labels = ctx_view->unique_labels;
    unique_labels.clear();
    
    queries_this_frame.clear();

    things_to_display.clear();
    
    ctx_view->found_set.clear();
    
    auto& active_elements = ctx_view->active_elements;
    active_elements.clear();
    
    Panel& panel = ui->margin_panels[0];
    Text_Panel& text_panel = panel.text;
    
//    auto is_valid_for_use = [](dt::Speech_Token* token) -> bool {
//        return (spacy_nlp::pos_tag_is_noun_like(token->tag) || token->pos == spacy_nlp::POS_PRON || token->pos == spacy_nlp::POS_PROPN);
//    }
    
    
    
    auto find_labels = [&]<typename ARGS>(void) {
        for (usize i = 0; i < text_panel.text.size(); i += 1) {
            auto& word = text_panel.text[i];
            
            if constexpr (ARGS::do_check) {
                if (!word.is_selected) {
                    continue;
                } else if (word.token->text == "rules") {
                    return CONTEXT_VIEW_MODE_RULES;
                } else if (word.token->text == "actions") {
                    return CONTEXT_VIEW_MODE_ACTIONS;
                }
            }
            
            auto* noun_entry = dt::noun_lookup(word.token->lemma);
            if (!noun_entry && word.token->lemma != "I") {
                continue;
                //noun_entry = dt::noun_add(word.token->lemma);
            }
            
            mtt::String& label = word.token->lemma;
            if (!unique_labels.contains(label)) {
                unique_labels.insert(label);
                labels.push_back({.label = label, .prop_ref = word.token->prop_ref});
            }
        }
        return CONTEXT_VIEW_MODE_DEFAULT;
    };
    
    auto status = find_labels.operator()<Find_Labels_Args_Do_Check>();
    ctx_view->mode = status;
    if (status == CONTEXT_VIEW_MODE_RULES) {
        labels.clear();
        unique_labels.clear();
        for (auto const& [thing_id, element] : ctx_view->thing_to_element) {
            Thing_destroy(mtt, element.proxy_id);
        }
        return handle_context_sensitive_command_rules(dt);
    } else if (status == CONTEXT_VIEW_MODE_ACTIONS) {
        labels.clear();
        unique_labels.clear();
        for (auto const& [thing_id, element] : ctx_view->thing_to_element) {
            Thing_destroy(mtt, element.proxy_id);
        }
        return handle_context_sensitive_command_actions(dt);
    }
    
    if (labels.size() == 0) {
        find_labels.operator()<Find_Labels_Args_Ignore_Check>();
        if (labels.size() == 0) {
            for (auto const& [thing_id, element] : ctx_view->thing_to_element) {
                Thing_destroy(mtt, element.proxy_id);
            }
            return false;
        }
    }
    
    
    mtt::String searched_var = "X";
    cstring searched_var_cstring = searched_var.c_str();
    
    
    std::stable_sort(labels.begin(), labels.end());
#ifndef NDEBUG
    for (usize i = 1; i < labels.size(); i += 1) {
        ASSERT_MSG(labels[i - 1].label <= labels[i].label, "should be sorted\n");
    }
#endif
    
    auto* dictionary = dt::word_dict();
    // create or reuse queries
    // TODO: remove all queries when closing this view
    for (usize label_i = 0; label_i < labels.size(); label_i += 1) {
        
        auto& label_record = labels[label_i];
        mtt::String& label = label_record.label;
        dt::Speech_Property* prop_ref = label_record.prop_ref;
        
        mtt::String query = "IsA(" MTT_Q_VAR_L("SUBTYPE") ", " + label + "), " MTT_Q_VAR_L("SUBTYPE") "(" + MTT_Q_VAR_S(searched_var) + "), !Prefab(" + MTT_Q_VAR_S(searched_var) + ")";
        
        using Prop_List = dt::Speech_Property::Prop_List;
        
        Prop_List* property_prop = nullptr;
        dt::Speech_Property* negated_prop = nullptr;
        bool is_negated = false;
        if (prop_ref != nullptr && prop_ref->try_get_prop("PROPERTY", &property_prop)) {
            if (prop_ref->parent != nullptr) {
                if (prop_ref->parent->try_get_only_prop("NEGATED", &negated_prop)) {
                    is_negated = negated_prop->value.flag;
                }
            }
            for (auto it_prop = property_prop->begin(); it_prop != property_prop->end(); ++it_prop) {
                auto* p = *it_prop;
                if (p->label != "trait") {
                    continue;
                }
                
                if (dictionary->is_ignored_attribute(p->value.text)) {
                    continue;
                }
                
                Speech_Property* n_prop = nullptr;
                p->try_get_only_prop("NEGATED", &n_prop);
                
                dt::Speech_Property::Prop_List* modifiers = p->try_get_prop("PROPERTY");
                
                if (modifiers == nullptr) {
                    if (is_negated || (n_prop != nullptr && n_prop->value.flag == true)) {
                        
                        query += ", !IsA(" + MTT_Q_VAR_S(searched_var) + ", attribute." + p->value.text + ")";
                    } else {
                        
                        query += ", IsA(" + MTT_Q_VAR_S(searched_var) + ", attribute." + p->value.text + ")";
                        
                    }
                } else {
                    query ="IsA(" + MTT_Q_VAR_S(searched_var) + ", " + label + ")";
                    if (is_negated || (n_prop != nullptr && n_prop->value.flag == true)) {
                        //query += ", IsA(_" + searched_var + ", attribute." + property->value.text + ")";
                        bool modifier_added = false;
                        for (const Speech_Property* mod : *modifiers) {
                            if (mod->label != "modifier") {
                                continue;
                            }
                            
                            modifier_added = true;
                            
                            query += ", !attribute." + p->value.text + "(" + MTT_Q_VAR_S(searched_var) + ", attribute." + mod->value.text + ")";
                        }
                        
                        if (!modifier_added) {
                            
                            query += ", !IsA(" + MTT_Q_VAR_S(searched_var) + ", attribute." + p->value.text + ")";
                        }
                    } else {
                        bool modifier_added = false;
                        for (const Speech_Property* mod : *modifiers) {
                            if (mod->label != "modifier") {
                                continue;
                            }
                            
                            modifier_added = true;
                            
                            query += ", attribute." + p->value.text + "(" + MTT_Q_VAR_S(searched_var) + ", attribute." + mod->value.text + ")";
                        }
                        
                        if (!modifier_added) {
                            query += ", IsA(" + MTT_Q_VAR_S(searched_var) + ", attribute." + p->value.text + ")";
                        }
                    }
                }
            }
        }
        
        {
            auto find_it = queries.find(query);
            if (find_it != queries.end()) {
                queries_this_frame.push_back(&find_it->second);
                label_record.query = &find_it->second;
                continue;
            }
        }
        
        
        //ctx_view->query_str = query;
        //        DT_Rule_Query_destroy(&search);
        DT_Rule_Query search;
        auto* status = DT_Rule_Query_make(mtt, query, &search);
        if (status == nullptr) {
                        //MTT_error("[%s] query creation failed [%s]\n", __PRETTY_FUNCTION__, query.c_str());
            continue;
        }
        queries.insert({query, search});
        {
            auto find_it = queries.find(query);
            auto* el = &find_it->second;
            queries_this_frame.push_back(el);
            label_record.query = el;
        }
    }
    
    
    
    

    for (usize i = 0; i < queries_this_frame.size(); i += 1) {
        auto& search = *queries_this_frame[i];
        ecs_world_t* ecs_world = mtt->ecs_world.c_ptr();
        //cstring searched_var_cstring = searched_var.c_str();
        bool it_done = false;
        ecs_iter_t r_it = ecs_rule_iter(ecs_world, search.rule_ptr);
        auto q_var = ecs_rule_find_var(search.rule_ptr, searched_var_cstring);
        
        while (!it_done) {
            if (!ecs_rule_next(&r_it)) {
                it_done = true;
                break;
            }
            
            auto result_for_var = ecs_iter_get_var(&r_it, q_var);
            auto ent = flecs::entity(r_it.world, result_for_var);
            if (ent.has(flecs::Prefab)) {
                continue;
            }
            
            if (ent.has<mtt::Thing_Info>()) {
                mtt::Thing* thing = nullptr;
                
                if (!Thing_try_get(mtt, ent.get<mtt::Thing_Info>()->thing_id, &thing)) {
                    continue;
                }
                if (thing->is_reserved) {
                    continue;
                }
            
                ctx_view->found_set.insert(mtt::thing_id(thing));
            }
        }
    }
    

    {
        
        if (!ctx_view->found_set.empty()) {
            things_to_display.resize(ctx_view->found_set.size());
        }
        
        // check for removal of stale entries
        struct To_Remove {
            mtt::Thing_ID thing_id = mtt::Thing_ID_INVALID;
            mtt::Thing_ID proxy_id = mtt::Thing_ID_INVALID;
        };
        static std::vector<To_Remove> to_remove = {};
        to_remove.clear();
        for (auto const& [thing_id, element] : ctx_view->thing_to_element) {
            // check if already exists
            
            
            // pre-existing element exists in to-display list, proxy should continue being displayed
            if (!ctx_view->found_set.contains(thing_id)) {
                // pre-existing element does not exist in to-display list, should remove element since proxy should not be displayed
                //mtt::Thing_destroy(mtt, element.proxy_id);
                to_remove.push_back({thing_id, element.proxy_id});;
            }
        }
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it) {
            ctx_view->thing_to_element.erase((mtt::Thing_ID)it->thing_id);
            mtt::Thing_destroy(mtt, it->proxy_id);
        }
    }
    if (ctx_view->found_set.size() > 0) {
        std::copy(ctx_view->found_set.begin(), ctx_view->found_set.end(), things_to_display.begin());
        std::stable_sort(things_to_display.begin(), things_to_display.end());
        
        
#define DT_DEBUG_THINGS_TO_PRINT_CONTEXT_VIEW (false)
#if DT_DEBUG_THINGS_TO_PRINT_CONTEXT_VIEW
        MTT_print("things found for context view size = %zu\n", ctx_view->found_set.size());
//        for (const mtt::Thing_ID thing_id : ctx_view->found_set) {
//            MTT_print("id=[%llu]\n", thing_id);
//        }
//        
//        for (usize i = 1; i < things_to_display.size(); i += 1) {
//            ASSERT_MSG(things_to_display[i - 1] <= things_to_display[i], "should be sorted\n");
//        }
#endif
#undef DT_DEBUG_THINGS_TO_PRINT_CONTEXT_VIEW
        return true;
    }
    
    return false;
        
        
}

void Instruction_on_destroy(dt::DrawTalk* dt, dt::Instruction* ins)
{
    if (ins == dt->lang_ctx.eval_ctx.instruction_selected_with_touch()) {
        dt->lang_ctx.eval_ctx.instruction_selected_with_touch() = nullptr;
    }
    if (ins == dt->lang_ctx.eval_ctx.instruction_selected_with_pen()) {
        dt->lang_ctx.eval_ctx.instruction_selected_with_pen() = nullptr;
    }
    Instruction::destroy(&ins);
}

void on_resize(MTT_Core* core, dt::DrawTalk* dt, vec2 new_size)
{
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    
    auto& dock = dt->ui.dock;
    auto& viewport = core->viewport;
    
    auto& labels = dock.labels;
    
    float32 b_actual_width = 1.0f;
    float32 b_actual_height = 1.0f;
    // top, bottom, left, right with respect to orientation
    vec4 view_offsets = sd::view_offset_for_safe_area(core->renderer);
    MTT_UNUSED(view_offsets);
    float32 ui_width  = viewport.width;
    float32 ui_height = viewport.height;
    float32 b_width   = ui_width / 29.0f;
    float32 b_height = ui_height / 29.0f;
    b_actual_width  = b_width;
    b_actual_height = b_height;
    float32 default_b_width  = 1367.0f / 32.0f;
    float32 default_b_height = 979.0f / 32.0f;
    b_width  =  default_b_width;
    b_height =  default_b_height;
    
    float32 padding = b_width / 2.0f;
    float32 y_padding = b_height * 0.5f;
    
    float32 x_pos = padding;
    float32 y_pos = (b_height / 2.0f) + (viewport.height / 2.0f) - 0.5f * (((labels.size() * b_height) + ((labels.size()) * y_padding - 2)));
    
    float32 width = 1367.0f;
    
    UI& ui = dt->ui;
    // non-dominant - assume right-handed for now
    Panel& ndom_panel = ui.margin_panels[0];

    ndom_panel.bounds.dimensions = vec2((viewport.width / 2.0f), (viewport.height / 4.0f) * (1.0f));
    ndom_panel.bounds.tl = vec2(viewport.width / 2.0f, 0.0f);
    
    ndom_panel.text.offset = vec2(b_width + padding + padding + padding, padding * 1.5);
    ndom_panel.text.row_count = 0;
    ndom_panel.text.invisible_row_count = 0;
    ndom_panel.text.vertical_scroll_offset = 0.0;
    
    auto* world = dt->mtt;
    
    // resize text panel
    {
        mtt::Thing* ui_thing = mtt::Thing_try_get(world, ndom_panel.thing);
        if (ui_thing == nullptr) {
            return;
        }
        
        mtt::Rep* rep = mtt::rep(ui_thing);
        
        mtt::Thing_set_aabb_dimensions(ui_thing, vec2(1.0f),  &rep, vec3(ndom_panel.bounds.tl, 900.0f), ndom_panel.bounds.dimensions);
    }
    
    // resize semantic diagram
    {
        UI& ui = dt->ui;
        Panel& ndom_panel = ui.margin_panels[0];
        
        auto& map_base = dt->ui.top_panel.base;
        map_base.bounds.dimensions = vec2(viewport.width / 2.0f, ndom_panel.bounds.dimensions.y);
        map_base.bounds.tl = vec2(0.0f, 0.0f);
        map_base.is_active = true;
        
        y_pos = (map_base.bounds.tl.y + map_base.bounds.dimensions.y) + (b_width);
        
        mtt::Thing* ui_thing = mtt::Thing_try_get(world, map_base.thing);
        if (ui_thing == nullptr) {
            return;
        }
        mtt::Rep* rep = mtt::rep(ui_thing);
        mtt::Thing_set_aabb_dimensions(ui_thing, vec2(1.0f), &rep, vec3(map_base.bounds.tl, 900.0f), map_base.bounds.dimensions);
        
        mtt::Map<mtt::String, usize>& label_to_index = dock.label_to_index;
        
        mtt::Map<mtt::String, UI_Dock::Entry_Config>& label_to_config = dock.label_to_config;
 
        float32 x_text_col = -padding - b_width;
    #define DT_TEXT_COL_Y(i) (y_padding + (b_width * i) + (0.5f*y_padding*(i+1)))
        label_to_config = {
            {DT_SPEECH_COMMAND_BUTTON_NAME, (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                    .hsv_color = false,
                .override_box = true,
    #if USE_OLD_TEXT_PANEL_LOCATION
                .box = {.tl = ndom_panel.bounds.tl + vec2(padding, y_padding), .dimensions = vec2(b_width)}
    #else
                .box = {.tl = ndom_panel.bounds.tl + vec2(ndom_panel.bounds.dimensions.x, 0.0f) + vec2(x_text_col, DT_TEXT_COL_Y(0)), .dimensions = vec2(b_width)}
    #endif
            }},
            {"discard", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                    .hsv_color = false,
                .override_box = true,
    #if USE_OLD_TEXT_PANEL_LOCATION
                .box = {.tl = ndom_panel.bounds.tl + vec2(padding + b_width + padding, y_padding), .dimensions = vec2(b_width)}
    #else
                .box = {.tl = ndom_panel.bounds.tl + vec2(ndom_panel.bounds.dimensions.x, 0.0f) + vec2(x_text_col, DT_TEXT_COL_Y(1)), .dimensions = vec2(b_width)}
    #endif
            }},
            {"speech ignore", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                    .hsv_color = false,
                .override_box = true,
    #if USE_OLD_TEXT_PANEL_LOCATION
                .box = {.tl = ndom_panel.bounds.tl + vec2(padding + b_width + padding + b_width + padding, y_padding), .dimensions = vec2(b_width)}
    #else
                .box = {.tl = ndom_panel.bounds.tl + vec2(ndom_panel.bounds.dimensions.x, 0.0f) + vec2(x_text_col, DT_TEXT_COL_Y(2)), .dimensions = vec2(b_width)}
    #endif
            }},
            {DT_CONTEXT_BUTTON_NAME, (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                    .hsv_color = false,
                .override_box = true,

                .box = {.tl = ndom_panel.bounds.tl + vec2(ndom_panel.bounds.dimensions.x, 0.0f) + vec2(x_text_col, DT_TEXT_COL_Y(3)), .dimensions = vec2(b_width)}
            }},
            {"color",             (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS_HSV, .show_label = false, .hsv_color = true,
                .override_box = true, .box = {.tl = vec2(0.25f * m::min(b_actual_width, b_actual_height), viewport.height - (4 * b_actual_height * 1.1)), .dimensions = vec2(m::max(b_actual_width, b_actual_height) * 0.5, b_actual_height * 3*.9),  }, .ignore_graphics = true
            }},
            {"select all", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                .hsv_color = false,
                .override_box = true,
                .box = {.tl = ndom_panel.bounds.tl + vec2(0.25 * padding, 0.0f) + vec2(0.0f, DT_TEXT_COL_Y(0)), .dimensions = vec2(b_width)}
                
            }},
            {"selection clear", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                    .hsv_color = false,
                .override_box = true,
                .box = {.tl = ndom_panel.bounds.tl + vec2(0.25 * padding, 0.0f) + vec2(0.0f, DT_TEXT_COL_Y(1)), .dimensions = vec2(b_width)}
            }},
            {"selection invert", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                    .hsv_color = false,
                .override_box = true,
                .box = {.tl = ndom_panel.bounds.tl + vec2(0.25 * padding, 0.0f) + vec2(0.0f, DT_TEXT_COL_Y(2)), .dimensions = vec2(b_width)}
            }},
            {"label things", (UI_Dock::Entry_Config){.render_layer = LAYER_LABEL_STATIC_CANVAS, .show_label = true,
                    .hsv_color = false,
                .override_box = true,
                .box = {.tl = ndom_panel.bounds.tl + vec2(0.25 * padding, 0.0f) + vec2(0.0f, DT_TEXT_COL_Y(3)), .dimensions = vec2(b_width)}
            }},
        };
        
        
        for (auto [label, conf] : label_to_config) {
            auto* button = dock.label_to_button[label];
            auto thing_id = button->thing;
            
            mtt::Thing* ui_button = mtt::Thing_try_get(world, thing_id);
            if (ui_button == nullptr) {
                return;
            }
            mtt::Rep* rep = mtt::rep(ui_button);
            
            mtt::Thing_set_aabb_dimensions(ui_button, vec2(1.0f), &rep, vec3(conf.box.tl, 999.999f), conf.box.dimensions);
            
            button->position_is_modified = true;
        }
        
        {
            auto* button  = dock.label_to_button["color"];
            auto* conf = &label_to_config[button->label];
            mtt::Thing* thing = world->Thing_get(button->thing);
            mtt::Rep* rep = mtt::rep(thing);
            
        #if !USE_OLD_TEXT_PANEL_LOCATION
            set_pose_transform(thing, m::translate(mat4(1.0f), (vec3){35.0f, 50.0f, 0.0f}) * m::rotate(mat4(1.0f), (float32)M_PI_2, (vec3){0.0f, 0.0f, 1.0f}));
        #endif
        }
        
        {
            float32 el_width = ((float32)viewport.width) * (0.25f);
            float32 el_height = (((float32)viewport.height) - (ui.top_panel.base.bounds.tl.y + ui.top_panel.base.bounds.dimensions.y + dt::bottom_panel_height));
            vec2 tl = vec2(((float32)viewport.width) - el_width, 0.0f) + vec2(0.0f, (ui.top_panel.base.bounds.tl.y + ui.top_panel.base.bounds.dimensions.y));
            mtt::Box box = {
                .tl = tl,
                .br = {tl.x + el_width, tl.y + el_height},
            };
            ui.context_view.box = box;
            ui.context_view.depth = 1.0f;
            
            mtt::Thing* thing_context_view = mtt::Thing_try_get(world, ui.context_view.thing_id);
            if (thing_context_view == nullptr) {
                return;
            }
            mtt::Rep* rep = mtt::rep(thing_context_view);
            auto& map_base = dt->ui.top_panel.base;
            mtt::Thing_set_aabb_dimensions(thing_context_view, vec2(1.0f, 1.0f), &rep, vec3(map_base.bounds.tl, 900.0f), map_base.bounds.dimensions);
            
            Context_View_disable(dt);
        }
        
        // precompute bounding boxes and heights for text view
        {
            Panel& panel = dt->ui.margin_panels[0];
            Text_Panel& text_panel = panel.text;
            
            auto* vg = nvgGetGlobalContext();
            
            nvgSave(vg);
            
            nvgFontSize(vg, text_panel.font_size);
            nvgFontFace(vg, text_panel.font_face);
            nvgTextAlign(vg, text_panel.align);
            float lineh = 0;
            nvgTextMetrics(vg, NULL, NULL, &lineh);
            
            float width = panel.bounds.dimensions.x - text_panel.offset.x;
            float x = text_panel.offset.x;
            float y = text_panel.offset.y;
            


            if (!text_panel.text.empty()) {
                text_panel.row_count = 1;
            }
            
            double out_of_bounds_y = (panel.bounds.tl.y - text_panel.offset.y + panel.bounds.dimensions.y);
            for (usize i = 0; i < text_panel.text.size(); i += 1) {
                auto& word = text_panel.text[i];
                cstring CS = word.text.c_str();
                
                float bounds[4];
                double advance = (i < text_panel.text.size() - 1 && text_panel.text[i + 1].part_of_contraction) ? 0 : nvgTextBounds(vg, 0, 0, " ", NULL, bounds);
                double text_advance = nvgTextBounds(vg, 0, 0, CS, NULL, bounds);
                
                if (x + advance + text_advance >= width) {
                    x = text_panel.offset.x;
                    y += lineh * text_panel.row_sep_factor;

                    if (y >= out_of_bounds_y) {
                        text_panel.invisible_row_count += 1;
                    }
                    
                    text_panel.row_count += 1;
                    
                }
                
                word.line_idx = text_panel.row_count - 1;
                
                word.bounds[0] = x;
                word.bounds[1] = y;
                word.bounds[2] = x + text_advance;
                word.bounds[3] = y + (lineh * text_panel.row_sep_factor);
                x += text_advance + advance;
                
            }
            
            if (!text_panel.text.empty()) {
                auto& last_word = text_panel.text.back();
                if (last_word.bounds[3] > panel.bounds.dimensions.y - (text_panel.offset.y * 2.0)) {
                    text_panel.row_offset = last_word.bounds[3] - (panel.bounds.dimensions.y - (text_panel.offset.y * 2.0));
                } else {
                    text_panel.row_offset = 0.0;
                }
            }

            text_panel.row_offset = (text_panel.invisible_row_count) * lineh * text_panel.row_sep_factor;
            nvgRestore(vg);
        }
        
    }
    
#endif
}

}

#ifdef CAMERA
#undef CAMERA
#endif

//#include "drawtalk_definitions_tree.cpp"


