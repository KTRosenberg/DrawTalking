import signal
import os
import sys
import time
import numpy as np
import json
import re

import spacy
import spacy.symbols
from   spacy import displacy
from   spacy.matcher import Matcher
from   spacy.util import filter_spans



import nltk
from nltk.corpus import wordnet as wn
from nltk.corpus import propbank


from collections import deque

from pathlib import Path

import importlib


def signal_handler(sig, frame):
    sys.exit(0)

LINGUA_INITIALIZE = 0

def get_all_hyponyms(synset):

    visited = set()
    Q = [{"set" : synset, "level": 0}]
    full = {"v":[], "n":[]}
    while len(Q) > 0:
        el = Q[0]
        Q.pop(0)
        sset = el["set"]
        next_level = el["level"] + 1
        if next_level > 2:
            continue

        next_hyponyms = sset.hyponyms()
        for hypo in next_hyponyms:
            name = hypo.name()
            if name not in visited:
                visited.add(name)
                next_el = {"set" : hypo, "level" : next_level}
                name_entry = name.split(".")
                full[name_entry[len(name_entry) - 2]].append(name_entry)
                Q.append(next_el)
    return full

def get_all_hypernyms(synset):
    visited = set()
    Q = [{"set" : synset, "level": 0}]
    full = {"v":[], "n":[]}
    while len(Q) > 0:
        el = Q[0]
        Q.pop(0)
        sset = el["set"]
        next_hypernyms = sset.hypernyms()
        next_level = el["level"] + 1
        for hyper in next_hypernyms:
            name = hyper.name()
            if name not in visited:
                next_el = {"set" : hyper, "level" : next_level}
                name_entry = name.split(".")
                full[name_entry[len(name_entry) - 2]].append(name_entry)
                Q.append(next_el)
    return full

def coref_resolution(doc):
    
    resolved_out = {}

    for i in range(0, len(doc)):
        token = doc[i]
        resolved = doc._.coref_chains.resolve(token)
        if resolved == None:
            continue

        res_list = []
        for entry in resolved:
            res_list.append(entry.i)
        resolved_out[token.i] = res_list

    return {"readable_rep" : doc._.coref_chains.pretty_representation, "resolved" : resolved_out, "full_len" : len(doc)}

# determines maximum number of uterrances for looking into the past
HISTORY_SIZE = 3



def run():

    signal.signal(signal.SIGINT, signal_handler)


    
    # add path to submodule for coreference
    path = os.getcwd()
    path = path + "/coreferee_container"
    sys.path.append(path)

    import coreferee


    nlp = None
    try:
        # This model has a maximum sequence length of 512 tokens
        nlp = spacy.load("en_core_web_trf")
        nlp.add_pipe('coreferee')
        sys.stderr.write("Loaded model successfully")
        sys.stderr.flush()
    except Exception as e:
        sys.stdout.write(json.dumps({"msg" : 'LINGUA_INITIALIZATION_FAILED'}))
        sys.stderr.flush()
        return

    sys.stdout.write(json.dumps({"msg" : 'LINGUA_INITIALIZED'}))
    sys.stdout.flush()


    text_history = {
        "text"   : deque(),
        "offset" : deque()
    }

    token_offset = 0

    doc = None

    wn.synsets("");

    if not os.path.isdir("./log"):
        os.mkdir("./log")
    if not os.path.isdir("./images"):
        os.mkdir("./images")

    output_path = Path("./", "images", "text_render.svg")
    # log_path    = Path("./", "log", "log.txt")


    while 1:
        line = sys.stdin.readline()

        sys.stdout.flush()
        while line:

            ID          = int(line.strip())
            mod_count   = int(sys.stdin.readline().strip())
            is_finished = int(sys.stdin.readline().strip())
            context     = sys.stdin.readline().strip()
            text        = sys.stdin.readline().strip().replace('anti-','anti').replace('right words', 'rightwards').replace('right word', 'rightward')


            if context == "reset":
                token_offset = 0
                text_history["text"]   = deque()
                text_history["offset"] = deque()
                line = sys.stdin.readline()

                # print("Lingua: reset!", file=sys.stderr)

                continue

            if ID == 1 and mod_count == 1:
                token_offset = 0
                text_history["text"]   = deque()
                text_history["offset"] = deque()
                # print("Lingua: reset!", file=sys.stderr)

            # if is_finished:
            #     print("=================\n", "ID", ID, "mod_count", mod_count, "is_finished", is_finished, "text", text, file=sys.stderr)
            # else:
            #     print("=================\n", "ID", ID, "mod_count", mod_count, "is_finished", is_finished, file=sys.stderr)

            if context == "synonym":
                try:
                    args = text.split(' ')
                    synsets = None
                    if len(args) == 1:
                        synsets = wn.synsets(args[0].strip())
                    else:
                        synsets = wn.synsets(args[0].strip(), args[1].strip())

                    data = {"entries" : []}
                    entries = data["entries"]
                    for synset in synsets:
                        entry = {
                            "lemmas" : [],
                            "synset" : synset.name().split("."),
                            "hyper"  : get_all_hypernyms(synset),
                            "hypo"   : get_all_hyponyms(synset)
                        }
                        
                        lemmas = entry["lemmas"]
                        for lemma in synset.lemmas():
                            lemmas.append(lemma.name())

                        entries.append(entry)    

                    data_to_send = json.dumps(data)
                    #data_length = len(data_to_send)

                    out_result = 'syn:' + str(ID) + '&' + str(mod_count) + '|' + data_to_send + '\r\n'

                    sys.stdout.write(out_result)
                except Exception as e:
                   sys.stderr.write("query failed\n")


                sys.stdout.flush()

                sys.stderr.flush()

                line = sys.stdin.readline()

                continue
            ###############################

            if len(text_history["text"]) == HISTORY_SIZE:
                to_remove = text_history["text"].popleft()
                offset = text_history["offset"].popleft()
                token_offset -= offset
            
            text_history["text"].append(text)
            doc = nlp(" ".join(text_history["text"]))
            # length of the new part of the document
            doc_len = len(doc) - token_offset
            text_history["offset"].append(doc_len)


            #print("=================", "ID", ID, "mod_count", mod_count, "is_finished", is_finished, "text", text , "text sub", doc, file=sys.stderr)
            #sys.stderr.flush()
                
                        
            



            as_json = {"tokens":[]}
            #tokens = as_json["tokens"]

            tokens = as_json["tokens"]
            for idx in range(token_offset, len(doc)):
                token = doc[idx]

                tokens.append({})
                token_record = tokens[len(tokens) - 1]

                token_record["text"] = str(token)
                token_record["pos"] = token.pos_
                token_record["tag"] = token.tag_
                token_record["dep"] = token.dep_
                token_record["lemma"] = token.lemma_
                token_record["head"] = token.head.i - token_offset
                
                token_record["i"]       = token.i - token_offset
                
                token_record["ent_iob"] = token.ent_iob
                token_record["ent_iob_str"] = token.ent_iob_
                token_record["ent_type_str"] = token.ent_type_
                token_record["norm"]    = token.norm_
                token_record["like_num"] = token.like_num
                token_record["left_edge"] = token.left_edge.i - token_offset
                token_record["right_edge"] = token.right_edge.i - token_offset

                ancestors = []
                token_record["ancestors"] = ancestors
                for ancestor in token.ancestors:
                    ancestors.append(ancestor.i - token_offset)

                children = []
                token_record["children"] = children
                for child in token.children:
                    children.append(child.i - token_offset)

                conjuncts = []
                token_record["conjuncts"] = conjuncts
                for conjunct in token.conjuncts:
                    conjuncts.append(conjunct.i - token_offset)

                lefts = []
                token_record["lefts"] = lefts
                for L in token.lefts:
                    lefts.append(L.i - token_offset)
                
                rights = []
                token_record["rights"] = rights
                for R in token.rights:
                    rights.append(R.i - token_offset)

                token_record["morph"] = token.morph.to_dict()

            # coreference resolution
            coref = coref_resolution(doc)
            #print(coref, file=sys.stderr)

            


            data_to_send = json.dumps({'ID': ID, 'mod_count' : mod_count, 'is_finished': is_finished, 
                'doc' : as_json, 'coref' : coref, 'count' : doc_len})
            #nlp_data_length = len(nlp_data)#len(nlp_data.encode('utf-8'))


            if is_finished:
                out_result = 'nlf:' + str(ID) + '&' + str(mod_count) + '|' + data_to_send + '\r\n' 
                sys.stdout.write(out_result)
                sys.stdout.flush()
                sys.stderr.flush()
                #
                token_offset += doc_len
            else:
                out_result = 'nlh:' + str(ID) + '&' + str(mod_count) + '|' + data_to_send + '\r\n'
                sys.stdout.write(out_result)
                sys.stdout.flush()
                sys.stderr.flush()
                #
                text_history["text"].pop()
                text_history["offset"].pop()                

            if False:

                #sys.stderr.write(text)
                #sys.stderr.flush()
                svg = displacy.render(doc, style="dep")

                if os.path.exists(output_path):
                    os.remove(output_path)
                # simplistic handling by writing and closing
                with output_path.open("w", encoding="utf-8") as vis_img_fd:
                    vis_img_fd.write(svg)



                # if os.path.exists(log_path):
                #     os.remove("./log/log.txt")
                # log_path.open("w", encoding="utf-8").write(data_to_send)

            line = sys.stdin.readline()

if __name__ == "__main__":
    run()


