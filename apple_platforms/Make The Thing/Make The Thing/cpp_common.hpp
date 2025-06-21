//
//  cpp_common.hpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/22/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef cpp_common_hpp
#define cpp_common_hpp

#define FOR_ITER(ITER_VAR__, DATA_STRUCTURE__, ACTION__) for (auto ITER_VAR__ = DATA_STRUCTURE__ . begin(); ITER_VAR__ != DATA_STRUCTURE__ . end(); ACTION__)

#define FOR_PTR_ITER(ITER_VAR__, DATA_STRUCTURE__, ACTION__) for (auto ITER_VAR__ = DATA_STRUCTURE__ -> begin(); ITER_VAR__ != DATA_STRUCTURE__ -> end(); ACTION__)

#define MTT_FALLTHROUGH [[fallthrough]];
#define MTT_NODISCARD [[nodiscard]]
#define MTT_LIKELY [[likely]]
#define MTT_UNLIKELY [[unlikely]]

#define MTT_UNUSED(something__) ((void)something__)

#define itv(iterator) (*iterator)
#define mapk(iterator) ((*iterator).first)
#define mapv(iterator) ((*iterator).second)


#include "stb_ds.h"
#include "stb_sprintf.h"

#include "array.hpp"

namespace mtt {

typedef std::string String;

template <typename K, typename V>
using Map = robin_hood::unordered_flat_map<K, V>;
template <typename K, typename V>
using Map_Stable = robin_hood::unordered_node_map<K, V>;

template <typename V>
using Set = robin_hood::unordered_flat_set<V>;
template <typename V>
using Set_Stable = robin_hood::unordered_node_set<V>;


template<typename T, typename R = bool>
struct Result {
    R status;
    T value;
};
template<typename T, typename R = bool>
inline Result<T, R> Result_make(R status, T value)
{
    return (Result<T, R>){.status = status, .value = value};
}

using Bool_Result = Result<void*, bool>;
inline Bool_Result Result_make(bool status)
{
    return (Bool_Result){.status = status, .value = nullptr};
}
inline Bool_Result Result_make(bool status, void* val)
{
    return (Bool_Result){.status = status, .value = val};
}

template<typename T, typename Compare>
typename std::vector<T>::iterator
insert_sorted( std::vector<T> & data, T const& item, Compare comp)
{
    return data.insert
        (
            std::upper_bound( data.begin(), data.end(), item, comp),
            item
        );
}

template<typename T, typename Compare>
typename std::vector<T>::iterator
remove_sorted(std::vector<T> & data, T const& item, Compare comp)
{
    auto it_find = std::binary_search( data.begin(), data.end(), item, comp);
    if (it_find == data.end()) {
        return;
    }
    
    data.erase(it_find);
}

inline void to_upper(std::string& data)
{
    using namespace std;
    std::for_each(data.begin(), data.end(), [](char & c){
        c = std::toupper(c);
    });
}

inline void to_lower(std::string& data)
{
    using namespace std;
    std::for_each(data.begin(), data.end(), [](char & c){
        c = std::tolower(c);
    });
}

inline bool str_eq(const std::string& source, usize index_first, usize index_last_exclusive, const std::string& against)
{
    return (source.compare(index_first, index_last_exclusive, against) == 0);
}

inline bool str_eq(const std::string& source, const std::string& against)
{
    return str_eq(source, 0, against.size(), against);
}

inline void join(const std::vector<std::string>& v, char c, std::string* s)
{
    for (auto p = v.cbegin();
         p != v.cend(); ++p) {
        
        *s += *p;
        if (p != v.cend() - 1) {
            *s += c;
        }
    }
}

inline std::string str_to_upper(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); }
                   );
    return s;
}

struct Unique_String_Identifier_Sequence {
    usize ident_counter = 0;
    
    std::string get_query_ident()
    {
        std::string str="" ;
        usize x=(this->ident_counter)++, y=1; do { str +='A' + x % ('Z'-'A'+1); x/=('Z'-'A'+1); y *= 26; } while (y < x);
        return str;
    }
};

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}





inline cstring c_str(const std::string& str)
{
    return str.c_str();
}

inline std::string space2underscore(std::string text)
{
    std::replace(text.begin(), text.end(), ' ', '_');
    return text;
}

template <typename T>
inline void insert(std::vector<T>& c, T& val)
{
    c.push_back(val);
}


struct Random_Suffix {
    usize ident_counter = 0;
    
    std::string get_query_ident()
    {
        std::string str="" ;
        usize x=(this->ident_counter)++, y=1; do { str +='A' + x % ('Z'-'A'+1); x/=('Z'-'A'+1); y *= 26; } while (y < x);
        return str;
    }
};

struct Token_Range {
    usize idx_start         = 0;
    usize idx_end_exclusive = 1;
};
inline void split_string(const std::string& str, const std::string& delimiters, mtt::Dynamic_Array<Token_Range>& token_ranges) {
    using _ssize = std::string::size_type;
    const _ssize str_ln = str.length();
    const _ssize delimiters_ln = delimiters.length();
    _ssize last_pos = 0;

    // container for the extracted tokens
    //std::vector<std::string> tokens;

    while (last_pos < str_ln) {
      // find the position of the next delimiter
        _ssize pos = str.find(delimiters, last_pos);
        
        if (pos != std::string::npos) {
            Token_Range r;
            r.idx_start = last_pos;
            r.idx_end_exclusive = pos;
            token_ranges.push_back(r);
        } else {
            Token_Range r;
            r.idx_start = last_pos;
            r.idx_end_exclusive = str_ln;
            token_ranges.push_back(r);
            break;
        }
        last_pos = pos += delimiters_ln;
    }
}

inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

// trim from start (copying)
inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

template <typename T>
inline void swap_and_pop(T& container, usize i)
{
    container[i] = std::move(container[container.size() - 1]);
    container.pop_back();
}


template <typename T>
inline mtt::Array_Slice<T> to_Array_Slice(std::vector<T>& self)
{
    return mtt::Array_Slice<T>::from_buffer(self.data(), self.size());
}


inline static mtt::String to_str(auto arg)
{
    return std::to_string(arg);
}

#define STRVAR(STR) "\"" + STR + "\""
#define STRMKPAIR(KEY, VAL) STRMK(KEY) ":" + VAL

#define STRV2(V) \
"[" + mtt::to_str(V.x) + "," + mtt::to_str(V.y) + "]"
#define STRV3(V) \
"[" + mtt::to_str(V.x) + "," + mtt::to_str(V.y) + "," + mtt::to_str(V.z) + "]"
#define STRV4(V) \
"[" + mtt::to_str(V.x) + "," + mtt::to_str(V.y) + "," + mtt::to_str(V.z) + "," + mtt::to_str(V.w) "]"


}







#endif /* cpp_common_hpp */
