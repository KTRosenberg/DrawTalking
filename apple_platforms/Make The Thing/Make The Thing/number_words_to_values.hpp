#ifndef number_words_to_values_hpp
#define number_words_to_values_hpp
/*
 # This library is a simple implementation of a function to convert textual
 # numbers written in English into their integer representations.
 #
 # This code is open source according to the MIT License as follows.
 #
 # Copyright (c) 2008 Greg Hewgill
 #
 # Permission is hereby granted, free of charge, to any person obtaining a copy
 # of this software and associated documentation files (the "Software"), to deal
 # in the Software without restriction, including without limitation the rights
 # to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 # copies of the Software, and to permit persons to whom the Software is
 # furnished to do so, subject to the following conditions:
 #
 # The above copyright notice and this permission notice shall be included in
 # all copies or substantial portions of the Software.
 #
 # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 # AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 # LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 # OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 # THE SOFTWARE.

 import re

 Small = {
     'zero': 0,
     'one': 1,
     'two': 2,
     'three': 3,
     'four': 4,
     'five': 5,
     'six': 6,
     'seven': 7,
     'eight': 8,
     'nine': 9,
     'ten': 10,
     'eleven': 11,
     'twelve': 12,
     'thirteen': 13,
     'fourteen': 14,
     'fifteen': 15,
     'sixteen': 16,
     'seventeen': 17,
     'eighteen': 18,
     'nineteen': 19,
     'twenty': 20,
     'thirty': 30,
     'forty': 40,
     'fifty': 50,
     'sixty': 60,
     'seventy': 70,
     'eighty': 80,
     'ninety': 90
 }

 Magnitude = {
     'thousand':     1000,
     'million':      1000000,
     'billion':      1000000000,
     'trillion':     1000000000000,
     'quadrillion':  1000000000000000,
     'quintillion':  1000000000000000000,
     'sextillion':   1000000000000000000000,
     'septillion':   1000000000000000000000000,
     'octillion':    1000000000000000000000000000,
     'nonillion':    1000000000000000000000000000000,
     'decillion':    1000000000000000000000000000000000,
 }

 class NumberException(Exception):
     def __init__(self, msg):
         Exception.__init__(self, msg)

 def text2num(s):
     a = re.split(r"[\s-]+", s)
     n = 0
     g = 0
     for w in a:
         x = Small.get(w, None)
         if x is not None:
             g += x
         elif w == "hundred" and g != 0:
             g *= 100
         else:
             x = Magnitude.get(w, None)
             if x is not None:
                 n += g * x
                 g = 0
             else:
                 raise NumberException("Unknown number: "+w)
     return n + g
     
 if __name__ == "__main__":
     assert 1 == text2num("one")
     assert 12 == text2num("twelve")
     assert 72 == text2num("seventy two")
     assert 300 == text2num("three hundred")
     assert 1200 == text2num("twelve hundred")
     assert 12304 == text2num("twelve thousand three hundred four")
     assert 6000000 == text2num("six million")
     assert 6400005 == text2num("six million four hundred thousand five")
     assert 123456789012 == text2num("one hundred twenty three billion four hundred fifty six million seven hundred eighty nine thousand twelve")
     assert 4000000000000000000000000000000000 == text2num("four decillion")
 */

//
//  number_words_to_values.hpp
//  Make The Thing
//
//  C++ adaptation Created by Karl Rosenberg on 8/8/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//



namespace dt {

static const mtt::Map<mtt::String, isize> Small = {
    {"zero", 0},
    {"one", 1},
    {"two", 2},
    {"three", 3},
    {"four", 4},
    {"five", 5},
    {"six", 6},
    {"seven", 7},
    {"eight", 8},
    {"nine", 9},
    {"ten", 10},
    {"eleven", 11},
    {"twelve", 12},
    {"thirteen", 13},
    {"fourteen", 14},
    {"fifteen", 15},
    {"sixteen", 16},
    {"seventeen", 17},
    {"eighteen", 18},
    {"nineteen", 19},
    {"twenty", 20},
    {"thirty", 30},
    {"forty", 40},
    {"fifty", 50},
    {"sixty", 60},
    {"seventy", 70},
    {"eighty", 80},
    {"ninety", 90}
};

static const mtt::Map<mtt::String, isize> Magnitude = {
    {"hundred",      100},
    {"thousand",     1000},
    {"million",      1000000},
    {"billion",      1000000000},
    {"trillion",     1000000000000},
    {"quadrillion",  1000000000000000},
    {"quintillion",  1000000000000000000},
//    {"sextillion",   1000000000000000000000},
//    {"septillion",   1000000000000000000000000},
//    {"octillion",    1000000000000000000000000000},
//    {"nonillion",    1000000000000000000000000000000},
//    {"decillion",    1000000000000000000000000000000000},
};

static inline bool text2num(dt::Dynamic_Array<mtt::String> a, isize* out)
{
    //a = re.split(r"[\s-]+", s)
    isize n = 0;
    isize g = 0;
    for (usize i_w = 0; i_w < a.size(); i_w += 1) {
        mtt::String& w = a[i_w];
        
        auto find = Small.find(w);
        if (find != Small.end()) {
            isize x = (*find).second;
            g += x;
        } else if (w == "hundred" && g != 0) {
            g *= 100;
        } else {
            auto find_mag = Magnitude.find(w);
            if (find_mag != Magnitude.end()) {
                isize x = (*find_mag).second;
                n += ((g == 0) ? 1 : g) * x;
                g = 0;
            } else {
                return false;
            }
        }
    }
        
    *out = n + g;
    return true;

}

static inline bool numstr2num(mtt::String& num_str_mtt, float64* out)
{
    {
        char* src = &num_str_mtt[0];
        char* dst = &num_str_mtt[0];
        while (*src) { if (*src != ',') *dst++ = *src; src++; }
        num_str_mtt.resize(dst - &num_str_mtt[0]);
        // or
        // str.erase(std::remove_if(str.begin(), str.end(), [](char ch) { return ch == ','; }), str.end());
    }
    cstring num_str = num_str_mtt.c_str();
    
    char* end;
    auto result = strtod(num_str, &end);
    usize len = strlen(num_str);
    if ((end != num_str + len) || (result == 0 && num_str == end) ||
        result == HUGE_VAL || result == -HUGE_VAL ||
        result == HUGE_VALF || result == -HUGE_VALF ||
        result == HUGE_VALL || result == -HUGE_VALL) {
        return false;
    }
    
    *out = result;
    return true;
}
    
}


#endif /* number_words_to_values_hpp */
