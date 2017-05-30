//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_TKNZR_H__
#define __NU_EXPR_TKNZR_H__


/* -------------------------------------------------------------------------- */

#include "nu_base_tknzr.h"

#include "nu_icstring.h"
#include "nu_lxa.h"
#include "nu_token.h"
#include "nu_token_list.h"

#include <deque>
#include <ostream>
#include <set>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

/**
 * This class implements a tokenizer which provides a container view interface
 * of a series of tokens contained in a given expression string
 */
class expr_tknzr_t : public base_tknzr_t {
public:
    expr_tknzr_t() = delete;
    expr_tknzr_t(const expr_tknzr_t&) = delete;
    expr_tknzr_t& operator=(const expr_tknzr_t&) = delete;
    virtual ~expr_tknzr_t() {}

    expr_tknzr_t(const std::string& data, // expression string data
        size_t pos, // original position of expression
        // used only to generate syntax error info
        // Each token position marker is the result of the sum
        // between this value and the token position
        // in the expression
        const std::string& blanks, // Blanks symbols
        const std::string& newlines, // New line symbols
        const std::string& operators, // Single char operators
        const std::set<std::string>& str_op, // Word operators
        const char subexp_bsymb, // Begin sub-expression symbol
        const char subexp_esymb, // End sub-expression symbol
        const std::string& string_bsymb, // Begin string marker
        const std::string& string_esymb, // End string marker
        const char string_escape, // Escape string symbol
        const std::set<std::string>& line_comment
        );


    //! Get a token and advance to the next one (if any)
    virtual token_t next() override;


    //! Split expression in token and copy them into a token list
    virtual void get_tknlst(
        token_list_t& tl, bool strip_comment = true) override;


    using typed_token_id_t = std::pair<std::string, tkncl_t>;
    using typed_token_set_t = std::set<typed_token_id_t>;

    //! Remove comment lines and comments at end of token list
    static void strip_comment_line(
        token_list_t& tl, const typed_token_set_t& comment_id_set);


    //! Get begin sub-expression symbol
    std::string subexp_begin_symbol() const noexcept
    {
        return _subexp_begin_symb;
    }

    //! Get end sub-expression symbol
    std::string subexp_end_symbol() const noexcept { return _subexp_end_symb; }


    //! Return expression position in the source line
    size_t get_exp_pos() const noexcept { return _pos; }


protected:
    //! Get a token and advance to the next one (if any)
    token_t _next();

    size_t _pos = 0;
    std::string _subexp_begin_symb;
    std::string _subexp_end_symb;
    std::set<std::string> _str_op; // mod, div, ...
    lxa_symb_t _blank, _op, _newl;
    lxa_word_t _word_op;
    lxa_str_t _strtk;

    std::set<std::string> _line_comment;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_TKNZR_H__
