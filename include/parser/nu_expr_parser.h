//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_PARSER_H__
#define __NU_EXPR_PARSER_H__


/* -------------------------------------------------------------------------- */

#include "nu_exception.h"
#include "nu_expr_any.h"
#include "nu_expr_tknzr.h"
#include "nu_proc_scope.h"
#include "nu_token_list.h"
#include "nu_var_scope.h"

#include <list>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class expr_parser_t {
public:
    //! ctors
    expr_parser_t() = default;
    expr_parser_t(const expr_parser_t&) = delete;
    expr_parser_t& operator=(const expr_parser_t&) = delete;

    //! Creates an expression using tokens got by a given tokenizer
    expr_any_t::handle_t compile(expr_tknzr_t& tknzr);

    //! Creates an expression using a given token-list
    expr_any_t::handle_t compile(token_list_t tl, size_t expr_pos);


protected:
    static variant_t::type_t get_type(const token_t& t);

    expr_any_t::handle_t parse_operand(token_list_t& tl);

    bool parse_operator(token_list_t& tl, std::string& value);

    expr_any_t::handle_t parse(
        token_list_t& tl, expr_any_t::handle_t first_param = nullptr);

    static void reduce_brackets(token_list_t& rtl);
    static void convert_subscription_brackets(token_list_t& rtl);
    static void fix_real_numbers(token_list_t& rtl);

    // There are expressions which begin with '-' operator (e.g. -<exp>)
    // which need to be transformed into 0-<exp> in order to 
    // be process correctly later
    static void fix_minus_prefixed_expressions(token_list_t& rtl);
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_PARSER_H__
