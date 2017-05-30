//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "mip_tknzr.h"

#include <sstream>

#include <iomanip>
#include <iostream>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

bool tknzr_t::_is_prefix_ch(const char_t ch, const std::set<string_t> & tknset)
{
    for (const auto & item : tknset) {
        if (!item.empty() && item[0] == ch) {
            return true;
        }
    }

    return false;
}


/* -------------------------------------------------------------------------- */

std::set<string_t>::const_reverse_iterator tknzr_t::_search_token(
    const std::set<string_t>& tknset,
    string_t & line)
{
    auto it = tknset.rbegin();

    for (; it != tknset.rend(); ++it) {
        auto & tkn = *it;

        if (tkn.size() <= line.size()) {
            if (line.substr(0, tkn.size()) == tkn) {
                return it;
            }
        }
    }

    return it;
}


/* -------------------------------------------------------------------------- */

string_t tknzr_t::_extract_token(
    const string_t& set_tkn,
    string_t & line,
    get_t cut_type)
{
    string_t token;

    if (cut_type == get_t::WHOLE_LN) {
        token = line;
        line.clear();
    }
    else {
        token = set_tkn;
        line = line.substr(set_tkn.size(), line.size() - set_tkn.size());
    }

    return token;
}


/* -------------------------------------------------------------------------- */

bool tknzr_t::_getline(
    _istream & is, string_t & line, string_t& eol_s, bool & eof)
{
    const bool cr = _eoldef.find(base_tknzr_t::eol_t::CR) != _eoldef.end();
    const bool lf = _eoldef.find(base_tknzr_t::eol_t::LF) != _eoldef.end();

    _stringstream ss;

    eof = is.eof();

    while (!eof) {

        if (is.bad()) {
            return false;
        }

        char_t ch = 0;
        is >> ch;

        eof = is.eof() || ch == 0;

        if (eof) {
            line = ss.str();
            eol_s = _T("");
            return true;
        }

        if (cr && ch == _T('\r')) {
            line = ss.str();
            eol_s = _T("\r");
            return true;
        }

        if (lf && ch == _T('\n')) {
            line = ss.str();
            eol_s = _T("\n");
            return true;
        }

        ss << ch;
    }

    return false;
}


/* -------------------------------------------------------------------------- */

void tknzr_t::_reset()
{
    _textline.clear();
    _eol_seq.clear();
    _offset = 0;
    _line_number = 0;
    _eof = false;
}


/* -------------------------------------------------------------------------- */

std::unique_ptr<token_t> tknzr_t::_search_eof()
{
    if (_eof) {
        auto token_obj = new token_t(
            token_t::tcl_t::END_OF_FILE,
            _eol_seq,
            _line_number,
            _offset);

        return std::unique_ptr<token_t>(token_obj);
    }

    return nullptr;
}


/* -------------------------------------------------------------------------- */

std::unique_ptr<token_t> tknzr_t::_search_eol()
{
    if (!_eol_seq.empty()) {
        auto token_obj = new token_t(
            token_t::tcl_t::END_OF_LINE,
            _eol_seq,
            _line_number,
            _offset);

        ++_line_number;
        _offset = 0;
        _eol_seq.clear();

        return std::unique_ptr<token_t>(token_obj);
    }

    return nullptr;
}


/* -------------------------------------------------------------------------- */

std::unique_ptr<token_t> tknzr_t::_search_other_tkn()
{
    if (!_other_token.empty())
    {
        auto token_obj = new token_t(
            token_t::tcl_t::OTHER,
            _other_token,
            _line_number,
            _offset);

        _offset += _other_token.size();
        _other_token.clear();

        return std::unique_ptr<token_t>(token_obj);
    }

    return nullptr;
}


/* -------------------------------------------------------------------------- */

std::unique_ptr<token_t> tknzr_t::_get_tkn(
    const std::set<string_t> & tknset,
    token_t::tcl_t tkncl,
    get_t cut_type)
{
    if (_is_prefix_ch(_textline[0], tknset)) {

        auto it = _search_token(tknset, _textline);

        if (it != tknset.rend()) {

            auto tkn = _search_other_tkn();
            if (tkn) {
                return tkn;
            }

            auto token = *it;

            if (cut_type == get_t::WHOLE_LN) {
                token = _textline;
                _textline.clear();
            }
            else {
                _textline = 
                    _textline.substr(
                        token.size(), 
                        _textline.size() - token.size());
            }

            auto token_obj = new token_t(
                tkncl,
                token,
                _line_number,
                _offset);

            _offset += token.size();

            return std::unique_ptr<token_t>(token_obj);
        }
    }

    return nullptr;
}


/* -------------------------------------------------------------------------- */

bool tknzr_t::_ml_comment_begin( 
    const std::set<ml_commdef_t> & tknset,
    string_t& end_comment)
{
    for (const auto & item : tknset) {
        
        const auto& prefix = item.first;

        if (prefix.size() <= _textline.size() &&
            _textline.substr(0, prefix.size()) == prefix) 
        {
            end_comment = item.second;
            return true;
        }
    }

    return false;
}


/* -------------------------------------------------------------------------- */

std::unique_ptr<token_t> tknzr_t::_get_string()
{
    if (_textline.size() < 2) {
        return nullptr;
    }

    const auto quote_ch = _textline[0];
    auto quote_esc_it = _strdef.find(quote_ch);

    if (quote_esc_it == _strdef.end()) {
        return nullptr;
    }

    const auto esc_cnvt = quote_esc_it->second;
    const char_t esc_ch =
        esc_cnvt ? esc_cnvt->escape_char() : 0;

    if (_textline.size() == 2 && _textline[1] != quote_ch) {
        return nullptr;
    }

    _stringstream ss;

    size_t extra_ch_cnt = 2; /* quotes + any esc */

    for (size_t i = 1; i < _textline.size(); ++i) {
        char_t ch = _textline[i];

        if (esc_cnvt && ch == esc_ch) {
            size_t remove_cnt = 0;
            if (!esc_cnvt->convert(_textline.c_str() + i, remove_cnt, ch)) {
                return nullptr;
            }
            i += (remove_cnt - 1);
            extra_ch_cnt += (remove_cnt - 1);
        }
        else if (ch == quote_ch) {
            const auto str = ss.str();

            auto token_obj = new token_t(
                token_t::tcl_t::STRING,
                str,
                _line_number,
                _offset,
                quote_ch,
                esc_ch);

            const auto off = str.size() + extra_ch_cnt;

            _offset += off;

            if (_textline.size() <= off) {
                _textline.clear();
            }
            else {
                _textline = _textline.substr(off, _textline.size() - off);
            }

            return std::unique_ptr<token_t>(token_obj);
        }

        ss << ch;
    }


    return nullptr;
}


/* -------------------------------------------------------------------------- */

std::unique_ptr<token_t>
tknzr_t::_extract_comment(
    string_t & comment,
    size_t end_comment_offset,
    string_t& end_comment,
    size_t line_number,
    size_t offset)
{
    if (end_comment_offset != string_t::npos) {
        auto left_comment = _textline.substr(
            0, 
            end_comment_offset + end_comment.size());

        if (left_comment.size() >= _textline.size()) {
            _textline.clear();
        }
        else {
            _textline = _textline.c_str() + left_comment.size();
        }

        comment += left_comment;

        auto token_obj = new token_t(
            token_t::tcl_t::COMMENT,
            comment,
            line_number,
            offset);

        _offset += left_comment.size();
        comment.clear();

        return std::unique_ptr<token_t>(token_obj);
    }

    return nullptr;
}


/* -------------------------------------------------------------------------- */

std::unique_ptr<token_t> tknzr_t::_get_comment(_istream & is)
{
    string_t comment;
    string_t end_comment;

    const size_t comment_line = _line_number;
    const size_t comment_offset = _offset;

    if (_ml_comment_begin(_ml_comdef, end_comment)) {
        auto tkn = _search_other_tkn();

        if (tkn) {
            return tkn;
        }

        size_t end_comment_offset = _textline.find(end_comment);

        tkn = _extract_comment(
            comment, 
            end_comment_offset, 
            end_comment,
            comment_line,
            comment_offset);

        if (tkn) {
            return tkn;
        }

        bool eof = false;
        while (!eof && end_comment_offset == string_t::npos) {
            comment += _textline;
            comment += _eol_seq;
            
            ++_line_number;
            _offset = 0;
            _textline.clear();
            _eol_seq.clear();

            if (!_getline(is, _textline, _eol_seq, eof)) {
                return nullptr;
            }

            end_comment_offset = _textline.find(end_comment);

            tkn = _extract_comment(
                comment, 
                end_comment_offset, 
                end_comment,
                comment_line,
                comment_offset);

            if (tkn) {
                return tkn;
            }
        }
    }

    return nullptr;
}


/* -------------------------------------------------------------------------- */

std::unique_ptr<token_t> tknzr_t::next(_istream & is)
{
    is.unsetf(std::ios_base::skipws);

    std::unique_ptr<token_t> tkn;

    while (!tkn) {

        if (_textline.empty()) {

            // other token
            auto tkn = _search_other_tkn();

            if (tkn) {
                return tkn;
            }

            // end-of-line token
            tkn = _search_eol();
            if (tkn) {
                return tkn;
            }

            // end-of-file (virtual) token
            tkn = _search_eof();
            if (tkn) {
                return tkn;
            }

            // read a text line
            if (!_getline(is, _textline, _eol_seq, _eof)) {
                _reset();
                return nullptr;
            }
        }

        // multi-line commment
        auto tkn = _get_comment(is);
        if (tkn) {
            return tkn;
        }

        // blank
        tkn = _get_tkn(_blkdef, token_t::tcl_t::BLANK, get_t::JUST_TKN);

        if (tkn) {
            return tkn;
        }

        // single-line comment
        tkn = _get_tkn(_sl_comdef, token_t::tcl_t::COMMENT, get_t::WHOLE_LN);

        if (tkn) {
            return tkn;
        }

        // atomic token
        tkn = _get_tkn(_atomdef, token_t::tcl_t::ATOM, get_t::JUST_TKN);

        if (tkn) {
            return tkn;
        }

        // string
        tkn = _get_string();

        if (tkn) {
            return tkn;
        }

        // append to other token buffer 
        if (!_textline.empty()) {
            _other_token += _textline[0];

            _textline =
                _textline.size() == 1 ? _T("") :
                _textline.substr(1, _textline.size() - 1);
        }
    }

    _reset();
    return nullptr;
}


/* -------------------------------------------------------------------------- */

bool tknzr_t::eos(_istream & is)
{
    if (_textline.empty() && _other_token.empty() && _eol_seq.empty()) {
        return is.eof();
    }

    return false;
}


/* -------------------------------------------------------------------------- */

tknzr_t::~tknzr_t() 
{}


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */


