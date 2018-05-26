//  
// This file is part of MipJson Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __MIP_JSON_PARSER_H__
#define __MIP_JSON_PARSER_H__


/* -------------------------------------------------------------------------- */

#include <memory>

#include "mip_unicode.h"
#include "mip_tknzr_bldr.h"
#include "mip_esc_cnvrtr.h"
#include "mip_json_obj.h"


/* -------------------------------------------------------------------------- */

namespace mip {

class json_parser_t {
public:
    using result_t = std::pair<bool, json_obj_t::handle_t>;

    json_parser_t(_ostream * logger = nullptr, size_t indent_spc = 2);

    result_t parse(_istream & is, _ostream & err);

protected:

    std::unique_ptr<token_t> get_token(_istream & is);

    result_t parse_number(const token_t & tkn, _istream & is, _ostream & err);
    result_t parse_object(_istream & is, _ostream & err);
    result_t parse_array(_istream & is, _ostream & err);

    result_t _parse_value(_istream & is, _ostream & err, bool array);

    result_t parse_obj_value(_istream & is, _ostream & err) {
        return _parse_value(is, err, false);
    }

    result_t parse_array_value(_istream & is, _ostream & err) {
        return _parse_value(is, err, true);
    }

private:
    int _level = 0;
    _ostream * _logger = nullptr;
    size_t _indent_spc = 2;

    struct with {
        std::unique_ptr<token_t>& _tkn;

        with(std::unique_ptr<token_t>& tkn) noexcept : _tkn(tkn) {}

        friend _ostream & operator<<(_ostream &os, const with & obj) {
            if (obj._tkn) {
                os << _T(" at ") << obj._tkn->line() + 1 << _T(":") << obj._tkn->offset() + 1;
            }
            return os;
        }
    };

    static constexpr const char_t* err_reading_stream() { return _T("reading input stream"); }
    static constexpr const char_t* err_val_expected() { return _T("value expected"); }
    static constexpr const char_t* err_inv_value() { return _T("invalid value"); }
    static constexpr const char_t* err_str_expected() { return _T("string expected"); }
    static constexpr const char_t* err_colon_expected() { return _T("':' expected"); }
    static constexpr const char_t* err_unmatching_curly_brace() { return _T("'}' or ',' expected"); }
    static constexpr const char_t* err_curly_brace_expected() { return _T("'{' expected"); }

private:
    std::unique_ptr< base_tknzr_t > _tknzr;
};


/* -------------------------------------------------------------------------- */

}


#endif // __MIP_JSON_PARSER_H__

