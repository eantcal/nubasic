//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "nu_error_codes.h"
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

rt_error_code_t* rt_error_code_t::_instance_ptr = nullptr;


/* -------------------------------------------------------------------------- */

void rt_error_code_t::throw_exc(
    int line, value_t errcode, const std::string& stmt)
{
    class err_t : public error_code_t {
        const value_t _err = value_t::E_INTERNAL;

        public:
            err_t(value_t err)
                : _err(err)
            {
            }

            int get() const noexcept { 
                return int(_err); 
            }
    };

    std::string errmsg = stmt.empty() ? "" : stmt + " ";
    errmsg += rt_error_code_t::get_instance().message(errcode);

    const err_t err(errcode);

    runtime_error_t::error_if(true, runtime_error_t(line, err, errmsg));
}


/* -------------------------------------------------------------------------- */

std::string rt_error_code_t::message(value_t err_code)
{
    std::string ret;

    const auto err_it = _err_desc_tbl.find(err_code);

    if (err_it != _err_desc_tbl.end()) {
        ret = err_it->second;
    }

    return ret;
}


/* -------------------------------------------------------------------------- */

rt_error_code_t& rt_error_code_t::get_instance() noexcept
{
    if (!_instance_ptr) {
        _instance_ptr = new rt_error_code_t();
    }

    assert(_instance_ptr);

    return *_instance_ptr;
}


/* -------------------------------------------------------------------------- */

rt_error_code_t::rt_error_code_t()
{
    auto& e = _err_desc_tbl;
    e[value_t::E_INTERNAL] = "invalid metadata. Try to rebuild";
    e[value_t::E_DIV_BY_ZERO] = "division by zero";
    e[value_t::E_WRG_NUM_ARGS] = "wrong number of arguments";
    e[value_t::E_INVALID_ARGS] = "invalid arguments";
    e[value_t::E_SUB_UNDEF] = "procedure not defined";
    e[value_t::E_FUNC_UNDEF] = "function not defined";
    e[value_t::E_TYPE_MISMATCH] = "type mismatch";
    e[value_t::E_TYPE_ILLEGAL] = "type illegal";
    e[value_t::E_VAR_REDEF] = "variable already defined";
    e[value_t::E_VAR_UNDEF] = "variable not defined";
    e[value_t::E_INV_VECT_SIZE] = "invalid vector size";
    e[value_t::E_INV_IDENTIF] = "invalid identifier";
    e[value_t::E_ELSE_OUTSIDE_IFBLK] = "Else outside If-block";
    e[value_t::E_ELSE_DUP_IN_IFBLK] = "Else already defined in this If-block";
    e[value_t::E_NO_END_IF] = "missing 'End If' statement";
    e[value_t::E_NO_MATCH_IF] = "matching If statement not found";
    e[value_t::E_NO_MATCH_SUB] = "matching Sub statement not found";
    e[value_t::E_NO_MATCH_WHILE] = "matching While statement not found";
    e[value_t::E_NO_MATCH_DO] = "matching Do statement not found";
    e[value_t::E_NO_MATCH_FUNC] = "End Function without maching FUNCTION";
    e[value_t::E_NO_RET_VAL] = "missing function return value";
    e[value_t::E_EXIT_OUTSIDE_FORBLK] = "Exit For outside For-loop";
    e[value_t::E_NO_LABEL] = "missing label";
    e[value_t::E_LABEL_NOT_DEF] = "label not defined";
    e[value_t::E_IMPL_CNT_NOT_ALLOWED] = "implicit counter not allowed in nested loops";
    e[value_t::E_EXIT_SUB_OUTSIDE_SUB] = "Exit Sub/Function outside procedure";
    e[value_t::E_MISSING_END_SUB] = "missing End Sub/Function statement";
    e[value_t::E_EXIT_WHILE_OUT] = "Exit While outside While loop";
    e[value_t::E_EXIT_LOOP_OUT] = "Exit Do outside Do-Loop-While block";
    e[value_t::E_EXIT_FOR_OUT] = "Exit Fot outside For-Next block";
    e[value_t::E_NO_MATCH_WEND] = "matching Wend/End While not found";
    e[value_t::E_VEC_IDX_OUT_OF_RANGE] = "Array index out of range";
    e[value_t::E_NEXT_WITHOUT_FOR] = "Next without For";
    e[value_t::E_CANNOT_MOD_CONST] = "cannot modify a constant value";
    e[value_t::E_VAL_OUT_OF_RANGE] = "value out of range";
    e[value_t::E_NO_MATCH_FOR] = "matching For statement not found";
    e[value_t::E_STRUCT_UNDEF] = "Struct not defined";
    e[value_t::E_NO_MATCH_STRUCT] = "matching Struct statement not defined";
    e[value_t::E_INV_DATA_SEPARATOR] = "invalid separator";
}


/* -------------------------------------------------------------------------- */

void syntax_error(const std::string& expr, size_t pos, const std::string& msg)
{
    std::string err(msg.empty() ? "Syntax Error" : msg);
    err += " at (" + nu::to_string(pos + 1) + "):\n";

    if (expr.size() > pos) {
        err += expr + "\n";

        for (size_t i = 0; i < pos; ++i)
            err += " ";

        for (size_t i = 0; i < expr.size() - pos; ++i)
            err += "^";
    }

    throw exception_t(err);
}


/* -------------------------------------------------------------------------- */

void syntax_error_if(
    bool condition, const std::string& expr, size_t pos, const std::string& msg)
{
    if (condition) {
        syntax_error(expr, pos, msg);
    }
}


/* -------------------------------------------------------------------------- */

void syntax_error_if(bool condition, const std::string& msg)
{
    if (condition) {
        throw exception_t(msg);
    }
}


/* -------------------------------------------------------------------------- */

}
