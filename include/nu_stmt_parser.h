//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_PARSER_H__
#define __NU_STMT_PARSER_H__

#include "nu_var_scope.h"

#include "nu_expr_literal.h"
#include "nu_expr_parser.h"
#include "nu_expr_tknzr.h"
#include "nu_global_function_tbl.h"
#include "nu_stmt.h"
#include "nu_stmt_empty.h"
#include "nu_stmt_on_goto.h"

#include <set>


/* -------------------------------------------------------------------------- */

namespace nu {
class stmt_parser_t {
protected:
    prog_pointer_t::line_number_t _parsing_line = 0;
    prog_pointer_t::stmt_number_t _parsing_stmt = 0;

public:
    static void remove_blank(nu::token_list_t& tl);

    static void extract_next_token(token_list_t& tl, token_t& token,
        std::function<bool(const token_list_t& tl, const token_t& token)> check
        = [](const token_list_t& tl, const token_t& token) {
              return tl.empty();
          });


    static void move_sub_expression(token_list_t& source_tl,
        token_list_t& dst_tl, const std::string& id, tkncl_t idtype);

protected:
    //! parse single stamtement
    stmt_t::handle_t parse_stmt(prog_ctx_t& ctx, nu::token_list_t& tl);


    //! parse an expression
    stmt_t::handle_t parse_expr(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl);


    //! parse a block of statement
    stmt_t::handle_t parse_block(prog_ctx_t& ctx, nu::token_list_t& tl,
        const std::string& end_block_id = "",
        tkncl_t end_block_id_cl = tkncl_t::UNDEFINED);


    //! Implements both Goto and Gosub parsing...
    template <class T>
    stmt_t::handle_t parse_branch_instr(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse For-To-Step statement
    stmt_t::handle_t parse_for_to_step(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Next statement
    stmt_t::handle_t parse_next(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Exit <stmt-id> statement
    stmt_t::handle_t parse_exit(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse a procedure Sub/Function name([paramlist])
    template <class T>
    stmt_t::handle_t parse_procedure(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse a definition of structure
    stmt_t::handle_t parse_struct(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse a definition of structure element
    stmt_t::handle_t parse_struct_element(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse a label list
    stmt_on_goto_t::label_list_t parse_label_list(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);

    //! parse const statement
    stmt_t::handle_t parse_const(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse explicit typename
    bool search_for__as_type(const token_list_t& tl, std::string& tname);


    //! parse instruction parameter list
    template <class T, typename... E>
    stmt_t::handle_t parse_parameter_list(prog_ctx_t& ctx, token_t token,
        token_list_t& tl, const std::string& end_token, E&&... xprms)
    {
        --tl;

        remove_blank(tl);

        auto scan_token = [&](tkncl_t tc) {
            extract_next_token(tl, token);
            syntax_error_if(
                token.type() != tc, token.expression(), token.position());
        };

        auto get_type
            = [&](std::string& type, const std::string& variable_name) {
                  type = variable_t::typename_by_type(
                      variable_t::type_by_name(variable_name));

                  if (!tl.empty()) {
                      token = *tl.begin();

                      if (token.type() == tkncl_t::IDENTIFIER
                          && token.identifier() == "as") {
                          --tl;
                          remove_blank(tl);

                          syntax_error_if(
                              tl.empty(), token.expression(), token.position());

                          token = *tl.begin();

                          syntax_error_if(token.type() != tkncl_t::IDENTIFIER,
                              token.expression(), token.position());

                          type = token.identifier();

                          --tl;
                          remove_blank(tl);
                      }
                  }
              };

        stmt_t::handle_t handle(std::make_shared<T>(std::forward<E>(xprms)...));

        while (!tl.empty() && (tl.begin()->type() != tkncl_t::OPERATOR
                                  && tl.begin()->identifier() != end_token)) 
        {
            token = *tl.begin();

            syntax_error_if(token.type() != tkncl_t::IDENTIFIER,
                token.expression(), token.position());

            std::string variable_name = token.identifier();

            syntax_error_if(!variable_t::is_valid_name(variable_name, false),
                variable_name + " is an invalid identifier");

            extract_next_token(tl, token);

            if (token.type() == tkncl_t::SUBEXP_BEGIN) {
                extract_next_token(tl, token);

                syntax_error_if(token.type() != tkncl_t::INTEGRAL
                        && token.type() != tkncl_t::IDENTIFIER,
                    token.expression(), token.position());

                token_list_t etl;
                expr_parser_t ep;

                etl += token;

                T* ptr = dynamic_cast<T*>(handle.get());
                assert(ptr);

                scan_token(tkncl_t::SUBEXP_END);

                --tl;
                remove_blank(tl);

                std::string type;

                get_type(type, variable_name);

                ptr->define(variable_name, type,
                    ep.compile(etl, token.position()),
                    std::forward<E>(xprms)...);
            }
            else {

                T* ptr = dynamic_cast<T*>(handle.get());
                assert(ptr);

                std::string type;
                get_type(type, variable_name);

                ptr->define(variable_name, type,
                    std::make_shared<expr_literal_t>(0),
                    std::forward<E>(xprms)...);
            }

            if (tl.empty()) {
                break;
            }

            token = *tl.begin();

            syntax_error_if((token.type() != tkncl_t::OPERATOR
                                && token.type() != tkncl_t::SUBEXP_END)
                    || (token.identifier() != ","
                                && token.identifier() != end_token),
                token.expression(), token.position());

            if (token.identifier() == end_token) {
                if (end_token == ")") {
                    --tl;
                    remove_blank(tl);
                }

                break;
            }

            --tl;
            remove_blank(tl);
        }

        return handle;
    }


    //! parse Delay statement
    stmt_t::handle_t parse_delay(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl);


    //! parse MDelay statement
    stmt_t::handle_t parse_mdelay(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl);


    //! parse If .. Then .. Else  statement
    stmt_t::handle_t parse_if_then_else(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse ElIf  statement
    stmt_t::handle_t parse_elif_stmt(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse While-[Do]
    stmt_t::handle_t parse_while(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Loop-While ...
    stmt_t::handle_t parse_loop_while(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Read statement
    stmt_t::handle_t parse_read(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl);


    //! parse Input [#<file>] statement
    stmt_t::handle_t parse_input(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl);


    //! parse Input (from console) statement
    stmt_t::handle_t parse_input_con(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Input# (from file) statement
    stmt_t::handle_t parse_input_file(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse #fd, get, ...
    void parse_fd_args(prog_ctx_t& ctx, token_t token, token_list_t& tl,
        int& fd, var_list_t& var_list);


    //! parse var
    var_arg_t parse_var_arg(prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Read
    stmt_t::handle_t parse_read_file(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Let statement
    stmt_t::handle_t parse_let(prog_ctx_t& ctx, token_list_t& tl);


    //! parse Locate statement
    stmt_t::handle_t parse_locate(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl);


    //! parse Open  statement
    stmt_t::handle_t parse_open(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Close  statement
    stmt_t::handle_t parse_close(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Print statement
    stmt_t::handle_t parse_print(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);

    //! parse Data statement
    stmt_t::handle_t parse_data(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse label identifier
    stmt_t::handle_t parse_label(
        prog_ctx_t& ctx, token_t token, token_list_t& tl);


    //! parse Goto/Gosub statement
    stmt_t::handle_t parse_goto_gosub(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl);


    //! parse On-Goto statement
    stmt_t::handle_t parse_on_goto(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl);


    //! parse End [If|While|Sub|Function|Struct] statement
    stmt_t::handle_t parse_end(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl);


    //! parse instruction and its args
    template <class T, typename... E>
    stmt_t::handle_t parse_generic_instruction(
        prog_ctx_t& ctx, token_t token, nu::token_list_t& tl, E&&... xargs)
    {
        --tl;
        remove_blank(tl);

        syntax_error_if(tl.empty(), token.expression(), token.position());

        return parse_arg_list<T, 0>(ctx, token, tl,
            [](const token_t& t) {
                return t.type() == tkncl_t::OPERATOR && (t.identifier() == ",");
            },
            std::forward<E>(xargs)...);
    }

    //! parse argument list
    //  T is instruction statement class to instantiate
    //  NARGS is number of argument to check, 0 means "no-check"
    //  ...E is extra-argument type list (passed to T-class-ctor)
    template <class T, int NARGS, typename... E>
    stmt_t::handle_t parse_arg_list(prog_ctx_t& ctx, token_t token,
        token_list_t& tl, std::function<bool(const token_t&)> is_separator,
        E&&... xargs)
    {
        expr_parser_t ep;
        arg_list_t args;
        auto pos = token.position();
        int parenthesis_level = 0;

        int nargs = NARGS;
        bool check_args = nargs > 0;

        while (!tl.empty() && (!check_args || nargs > 0)) {
            token_list_t etl;

            remove_blank(tl);

            while (!tl.empty()) {
                const token_t token(*tl.begin());

                if (token.type() == tkncl_t::SUBEXP_BEGIN) {
                    ++parenthesis_level;
                }
                else if (token.type() == tkncl_t::SUBEXP_END) {
                    --parenthesis_level;
                }
                else if (token.type() == tkncl_t::OPERATOR) {
                    if (token.identifier() == ":") {
                        if (!etl.empty())
                            args.push_back(
                                std::make_pair(ep.compile(etl, pos), '\0'));

                        return stmt_t::handle_t(
                            new T(args, std::forward<E>(xargs)...));
                    }

                    if (parenthesis_level < 1 && is_separator(token)) {
                        expr_any_t::handle_t eh(ep.compile(etl, pos));

                        args.push_back(
                            std::make_pair(eh, token.identifier().c_str()[0]));

                        --nargs;

                        --tl;

                        remove_blank(tl);

                        if (tl.empty()) {
                            return stmt_t::handle_t(std::make_shared<T>(
                                args, std::forward<E>(xargs)...));
                        }

                        break;
                    }
                }

                etl += token;
                --tl;

                remove_blank(tl);
            }

            remove_blank(tl);

            if (tl.empty()) {
                if (!etl.empty())
                    args.push_back(std::make_pair(ep.compile(etl, pos), '\0'));

                return stmt_t::handle_t(
                    std::make_shared<T>(args, std::forward<E>(xargs)...));
            }
        }

        return stmt_t::handle_t(std::make_shared<T>(std::forward<E>(xargs)...));
    }


    //! parse a sub-expression
    expr_any_t::handle_t parse_sub_expr(prog_ctx_t& ctx, token_t token,
        token_list_t& tl, token_list_t& vect_etl);

public:
    stmt_parser_t() = default;
    stmt_parser_t(const stmt_parser_t&) = default;
    stmt_parser_t& operator=(const stmt_parser_t&) = default;

    stmt_t::handle_t compile_line(nu::expr_tknzr_t& st, prog_ctx_t& ctx);
    stmt_t::handle_t compile_line(nu::token_list_t& tl, prog_ctx_t& ctx);
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif
