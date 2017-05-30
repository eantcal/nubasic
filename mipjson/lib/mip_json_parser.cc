//  
// This file is part of MipJson Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "mip_json_parser.h"

#include <cassert>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

json_parser_t::json_parser_t(_ostream * logger, size_t indent_spc) : 
    _logger(logger),
    _indent_spc(indent_spc)
{
    tknzr_bldr_t tknzbldr;

    tknzbldr.def_atom(_T("["));
    tknzbldr.def_atom(_T("]"));
    tknzbldr.def_atom(_T("}"));
    tknzbldr.def_atom(_T("{"));
    tknzbldr.def_atom(_T(";"));
    tknzbldr.def_atom(_T(","));
    tknzbldr.def_atom(_T(":"));

    tknzbldr.def_blank(_T(" "));
    tknzbldr.def_blank(_T("\r")); // treat \r like a blank
    tknzbldr.def_blank(_T("\t"));

    tknzbldr.def_eol(mip::base_tknzr_t::eol_t::LF); // linefeed is end of line marker

    tknzbldr.def_string(_T('\"'), nullptr);

    _tknzr = tknzbldr.build();

    assert(_tknzr);
}


/* -------------------------------------------------------------------------- */

std::unique_ptr<token_t> json_parser_t::get_token(_istream & is) 
{
    while (!is.bad() && !_tknzr->eos(is)) {
        auto tkn = _tknzr->next(is);

        if (!tkn) {
            return nullptr;
        }

        const auto tkntype = tkn->type();

        assert(tkntype != token_t::tcl_t::COMMENT);

        // just ignore blanks, eol
        if (tkntype == token_t::tcl_t::BLANK ||
            tkntype == token_t::tcl_t::END_OF_LINE ||
            tkntype == token_t::tcl_t::COMMENT) 
        {
            continue;
        }

        return tkn;
    }

    return nullptr;
}


/* -------------------------------------------------------------------------- */

json_parser_t::result_t json_parser_t::parse_number(
    const token_t & tkn, 
    _istream & is, 
    _ostream & err)
{
    json_obj_t::handle_t ret;

    auto tt = tkn.type();

    switch (tt) {

        case token_t::tcl_t::ATOM:
        case token_t::tcl_t::STRING:
        case token_t::tcl_t::END_OF_FILE:

            err
                << err_val_expected()
                << _T(" at ") << tkn.line() + 1 
                << _T(":") << tkn.offset() + 1;

            return std::make_pair(false, nullptr);

        case token_t::tcl_t::OTHER:
        default:
            try {
                const auto value = tkn.value();

                if (value.size() > 1 && 
                    ( value[0] != '-' && value[0] != '.' && 
                        (value[0] < '0' || value[0] > '9'))) {

                    err
                        << err_inv_value()
                        << _T(" at ") << tkn.line() + 1 
                        << _T(":") << tkn.offset() + 1;

                    return std::make_pair(false, nullptr);
                }

                bool double_num =
                    (value.find('.') != std::string::npos) ||
                    (value.find('e') != std::string::npos) ||
                    (value.find('E') != std::string::npos);

                size_t idx = 0;

                if (double_num) {
                    const double dv = std::stod(tkn.value(), &idx);
                    if (_logger) {
                        *_logger << dv;
                    }
                    ret = std::make_unique<json_obj_t>(dv);
                }
                else {
                    if (value.size() > 1 && value[0] == '-') {
                        const int64_t v = std::stoll(tkn.value(), &idx);
                        ret = std::make_unique<json_obj_t>(v);
                        if (_logger) {
                            *_logger << v;
                        }
                    }
                    else {
                        const uint64_t v = std::stoull(tkn.value(), &idx);
                        ret = std::make_unique<json_obj_t>(v);
                        if (_logger) {
                            *_logger << v;
                        }
                    }
                }

                if (idx != value.size()) {
                    err
                        << err_inv_value()
                        << _T(" at ") 
                        << tkn.line() + 1 
                        << _T(":") 
                        << tkn.offset() + 1;

                    return std::make_pair(false, nullptr);
                }
            }
            catch (std::exception &) {
                err
                    << err_inv_value()
                    << _T(" at ") 
                    << tkn.line() + 1 
                    << _T(":") << tkn.offset() + 1;

                return std::make_pair(false, nullptr);
            }

            break;
    }

    return std::make_pair(true, std::move(ret));
}


/* -------------------------------------------------------------------------- */

json_parser_t::result_t json_parser_t::_parse_value( 
    _istream & is, _ostream & err, bool array) 
{
    json_obj_t::handle_t ret;

    auto tkn = get_token(is);

    if (!tkn) {
        err << err_reading_stream() << with(tkn);
        return std::make_pair(false, nullptr);
    }

    auto tt = tkn->type();

    switch (tt) {
        case token_t::tcl_t::END_OF_FILE:
            err << err_val_expected() << with(tkn);
            return std::make_pair(false, nullptr);

        case token_t::tcl_t::STRING:

            if (_logger) {
                *_logger << _T("\"") << tkn->value() << "\"";
            }

            ret = std::make_unique<json_obj_t>(tkn->value());
            break;

        case token_t::tcl_t::ATOM:
            if (tkn->value() == _T("{")) {
                return parse_object(is, err);
            }
            else if (tkn->value() == _T("[")) {
                return parse_array(is, err);
            }

            if (array && tkn->value() == _T("]")) {
                if (_logger) {
                    *_logger << _T("]");
                }
                return std::make_pair(true, json_obj_t::make_array());
            }

            return std::make_pair(false, nullptr);

        case token_t::tcl_t::OTHER:
        default:
            if (tkn->value() == _T("null")) {

                if (_logger) {
                    *_logger << _T("null");
                }

                ret = std::make_unique<json_obj_t>();
            }
            else if (tkn->value() == _T("true")) {

                if (_logger) {
                    *_logger << _T("true");
                }

                ret = std::make_unique<json_obj_t>(true);
            }
            else if (tkn->value() == _T("false")) {

                if (_logger) {
                    *_logger << _T("false");
                }

                ret = std::make_unique<json_obj_t>(false);
            }
            else {
                return parse_number(*tkn, is, err);
            }

            break;
    }

    return std::make_pair(true, std::move(ret));
}


/* -------------------------------------------------------------------------- */

json_parser_t::result_t json_parser_t::parse_object(_istream & is, _ostream & err) 
{
    ++_level;

    if (_logger) {
        *_logger 
            << std::endl 
            << string_t(_level * _indent_spc, ' ') 
            << _T("{");
    }

    json_obj_t::handle_t obj = json_obj_t::make_object();

    do {
        auto tkn = get_token(is);

        if (!tkn) {
            err << err_reading_stream() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        const auto tt = tkn->type();

        if (tt == token_t::tcl_t::END_OF_FILE) {
            break;
        }     

        if (tt == token_t::tcl_t::ATOM && tkn->value() == _T("}")) {
            --_level;

            if (_logger) {
                *_logger << _T("}");
            }

            break;
        }

        if (tt != token_t::tcl_t::STRING) {
            err << err_str_expected() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        const auto item_name = tkn->value();

        if (_logger) {
            *_logger 
                << _T("\"") 
                << item_name 
                << _T("\":");
        }

        tkn = get_token(is);

        if (!tkn) {
            err << err_reading_stream() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        if (tkn->type() == token_t::tcl_t::END_OF_FILE) {
            err << err_val_expected() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        if (tkn->type() != token_t::tcl_t::ATOM || tkn->value() != _T(":")) {
            err << err_colon_expected() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        auto ret = parse_obj_value(is, err);

        if (!ret.first) {
            return std::make_pair(false, nullptr);
        }

        obj->add_member(item_name, std::move(ret.second));

        tkn = get_token(is);

        if (!tkn) {
            err << err_reading_stream() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        if (tkn->type() == token_t::tcl_t::END_OF_FILE) {
            err << err_val_expected() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        if (tkn->type() != token_t::tcl_t::ATOM ||
                (tkn->value() != _T("}") && (tkn->value() != _T(","))))
        {
            err << err_unmatching_curly_brace() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        if (tkn->value() == _T("}")) {
            --_level;

            if (_logger) {
                *_logger << _T("}");
            }

            break;
        }

        if (_logger && tkn->value() == _T(",")) {
           *_logger 
                << _T(",")
                << std::endl 
                << string_t(_level * _indent_spc, ' '); 
        }

    } while (1);

    return std::make_pair(true, std::move(obj));
}


/* -------------------------------------------------------------------------- */

json_parser_t::result_t json_parser_t::parse_array(_istream & is, _ostream & err) 
{
    ++ _level;

    if (_logger) {
        *_logger 
            << std::endl 
            << string_t(_level * _indent_spc, ' ') 
            << _T("[");
    }

    json_obj_t::handle_t obj = json_obj_t::make_array();

    do {
        auto ret = parse_array_value(is, err);

        if (!ret.first) {
            return std::make_pair(false, nullptr);
        }

        obj->push_back(std::move(ret.second));

        auto tkn = get_token(is);

        if (!tkn) {
            err << err_reading_stream() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        if (tkn->type() == token_t::tcl_t::END_OF_FILE) {
            err << err_val_expected() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        if (tkn->type() != token_t::tcl_t::ATOM) {
            err << err_unmatching_curly_brace() << with(tkn);
            return std::make_pair(false, nullptr);
        }

        if (tkn->value() == _T("]")) {
            --_level;

            if (_logger) {
                *_logger << _T("]");
            }

            break;
        }
        else if (tkn->value() != _T(",")) {
            err << err_unmatching_curly_brace() << with(tkn);
            return std::make_pair(false, nullptr);
        }
        else if (_logger) {
           *_logger << _T(",")
            << std::endl 
            << string_t(_level * _indent_spc, ' '); 
        }


    } while (true);

    return std::make_pair(true, std::move(obj));
}


/* -------------------------------------------------------------------------- */

json_parser_t::result_t json_parser_t::parse(_istream & is, _ostream & err) 
{

    auto tkn = get_token(is);

    if (!tkn) {
        err << err_reading_stream() << with(tkn);
        return std::make_pair(false, nullptr);
    }

    if (tkn->type() == token_t::tcl_t::END_OF_FILE) {
        return std::make_pair(true, nullptr);
    }


    if (tkn->type() != token_t::tcl_t::ATOM ||
            (tkn->value() != _T("{") && (tkn->value() != _T("["))))
    {
        err << err_curly_brace_expected() << with(tkn);
        return std::make_pair(false, nullptr);
    }

    bool obj = tkn->value() == _T("{");

    return obj ?
        parse_object(is, err) :
        parse_array(is, err);
}


/* -------------------------------------------------------------------------- */

}


