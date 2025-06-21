//
//  expr_test.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/15/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "expr_test.hpp"


#ifndef TEST_EXPRTTK
#define TEST_EXPRTTK



void logic(void)
{
    typedef exprtk::symbol_table<float64> symbol_table_t;
    typedef exprtk::expression<float64>     expression_t;
    typedef exprtk::parser<float64>             parser_t;
    
    typedef exprtk::symbol_table<float32> symbol_table_f32_t;
    typedef exprtk::expression<float32>     expression_f32_t;
    typedef exprtk::parser<float32>             parser_f32_t;
    
    int x = 2;
    using T = float64;
    
    
    
    const std::string expression_string = "not(A and B) or C";
    
    symbol_table_t symbol_table;
    symbol_table.create_variable("A");
    symbol_table.create_variable("B");
    symbol_table.create_variable("C");
    
    expression_t expression;
    expression.register_symbol_table(symbol_table);
    
    parser_t parser;
    parser.compile(expression_string,expression);
    
    printf(" # | A | B | C | %s\n"
           "---+---+---+---+-%s\n",
           expression_string.c_str(),
           std::string(expression_string.size(),'-').c_str());
    
    for (int i = 0; i < 8; ++i)
    {
        symbol_table.get_variable("A")->ref() = T((i & 0x01) ? 1 : 0);
        symbol_table.get_variable("B")->ref() = T((i & 0x02) ? 1 : 0);
        symbol_table.get_variable("C")->ref() = T((i & 0x04) ? 1 : 0);
        
        const int result = static_cast<int>(expression.value());
        
        printf(" %d | %d | %d | %d | %d \n",
               i,
               static_cast<int>(symbol_table.get_variable("A")->value()),
               static_cast<int>(symbol_table.get_variable("B")->value()),
               static_cast<int>(symbol_table.get_variable("C")->value()),
               result);
    }
}

#endif
