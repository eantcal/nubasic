//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_SYNTAX_TREE_H__
#define __NU_EXPR_SYNTAX_TREE_H__


/* -------------------------------------------------------------------------- */

#include "nu_exception.h"
#include "nu_expr_parser.h"
#include "nu_var_scope.h"
#include "nu_variable.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

/**
* Transform a given token list in its intermediate representation for
* the expression parser. The token list is transformed in a abstract syntax
* tree where each node denotes a "well-formed" expression.
*/
struct expr_syntax_tree_t {
    //! ctors
    expr_syntax_tree_t();
    expr_syntax_tree_t(expr_syntax_tree_t&) = default;
    expr_syntax_tree_t& operator=(expr_syntax_tree_t&) = default;

protected:
    std::string explicit_left_marker = "(";
    std::string explicit_right_marker = ")";
    std::string implicit_left_marker = "";
    std::string implicit_right_marker = "";

    std::set<std::string> processing_ops;
    std::set<std::string> ignoring_ops;

    enum class op_preced_t {
        STRUCT_ACCESS,
        RELATIONAL_OP,
        RELATIONAL_ANDOR_OP,
        MATH_MUL_DIV,
        MATH_SUM,
        BITWISE_OP
    };

    void change_operator_precedence(op_preced_t precedence);
    void swap_lr_markers();

    token_list_t remove_blank(token_list_t tl);
    void skip_processed(token_list_t& tl, token_list_t::data_t::iterator& i);
    void rework_subexp(token_list_t& tl, token_list_t::data_t::iterator i);
    void rework_operator(token_list_t& tl, token_list_t::data_t::iterator i);
    void rework_identifier(token_list_t& tl, token_list_t::data_t::iterator i);
    token_list_t rework_token_list(token_list_t tl);
    void rework_minus_operator(token_list_t& tl, token_list_t::data_t::iterator i);
    void rework_unary_operator(token_list_t& tl, token_list_t::data_t::iterator i);

public:
    //! Converts a given token list into syntax-oriented
    //! and well-formed token list
    token_list_t operator()(token_list_t tl);
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_EXPR_SYNTAX_TREE_H__
