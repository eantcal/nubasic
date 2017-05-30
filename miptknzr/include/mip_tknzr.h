//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_TKNZR_H__
#define __MIP_TKNZR_H__


/* -------------------------------------------------------------------------- */

#include "mip_token.h"
#include "mip_base_tknzr.h"
#include "mip_base_esc_cnvrtr.h"

#include <memory>
#include <istream>
#include <set>
#include <map>
#include <string>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

class tknzr_t : public base_tknzr_t
{
    friend class tknzr_bldr_t;

public:
    using ml_commdef_t = std::pair<string_t, string_t>;

    //! Return next token found in a given input stream
    std::unique_ptr<token_t> next(_istream & is) override;

    //! Return true if there is no more data to process
    bool eos(_istream & is) override;

    //! dtor
    virtual ~tknzr_t();

    
private:
    tknzr_t() noexcept {}
    tknzr_t(const tknzr_t&) = delete;
    tknzr_t& operator=(const tknzr_t&) = delete;

    enum class get_t {
        JUST_TKN,
        WHOLE_LN
    };

    size_t _offset = 0;
    size_t _line_number = 0;

    string_t _textline;
    string_t _eol_seq;
    string_t _other_token;

    bool _eof = false;

    static bool _is_prefix_ch(
        const char_t ch, 
        const std::set<string_t> & tknset);

    static std::set<string_t>::const_reverse_iterator _search_token(
        const std::set<string_t>& tknset,
        string_t & line);

    static string_t _extract_token(
        const string_t& set_tkn,
        string_t & line,
        get_t cut_type);

    bool _ml_comment_begin(
        const std::set<ml_commdef_t> & tknset,
        string_t & end_comment);

    void _reset();

    bool _getline(
        _istream & is, 
        string_t & line, 
        string_t& eol_s, 
        bool & eof);

    std::unique_ptr<token_t> _extract_comment(
        string_t & comment,
        size_t end_comment_offset,
        string_t& end_comment,
        size_t line_number,
        size_t offset);

    std::unique_ptr<token_t> _search_eof();
    std::unique_ptr<token_t> _search_eol();
    std::unique_ptr<token_t> _search_other_tkn();
    std::unique_ptr<token_t> _get_comment(_istream & is);

    std::unique_ptr<token_t> _get_tkn(
        const std::set<string_t> & tknset,
        token_t::tcl_t tkncl,
        get_t cut_type);

    std::unique_ptr<token_t> _get_string();

    std::set<string_t> _blkdef;
    std::set<string_t> _atomdef;
    std::set<base_tknzr_t::eol_t> _eoldef;
    std::set<string_t> _sl_comdef;
    std::set<ml_commdef_t> _ml_comdef;
    std::map< char_t /*quote*/, std::shared_ptr<base_esc_cnvrtr_t > > _strdef;
};


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */

#endif // __MIP_TKNZR_H__
