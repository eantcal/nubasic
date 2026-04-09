//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

// Control-flow statement parsing:
//   parse_block, parse_for_to_step, parse_next, parse_exit,
//   parse_if_then_else, parse_elif_stmt, parse_label_list,
//   parse_on_goto, parse_while, parse_loop_while, parse_goto_gosub

/* -------------------------------------------------------------------------- */

#include "nu_parser_impl.h"


/* -------------------------------------------------------------------------- */

namespace nu {

// Suppress implicit instantiation — definitions live in nu_statement_parser.cc.
extern template stmt_t::handle_t
statement_parser_t::parse_branch_instr<stmt_goto_t>(
    prog_ctx_t&, token_t, token_list_t&);
extern template stmt_t::handle_t
statement_parser_t::parse_branch_instr<stmt_gosub_t>(
    prog_ctx_t&, token_t, token_list_t&);


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_block(prog_ctx_t& ctx,
    nu::token_list_t& tl, const std::string& end_block_id,
    tkncl_t end_block_id_cl)
{
    remove_blank(tl);

    if (tl.empty()) {
        return stmt_empty_t::handle_t(std::make_shared<stmt_empty_t>(ctx));
    }

    stmt_block_t::handle_t hblock(std::make_shared<stmt_block_t>(ctx));

    while (!tl.empty()) {
        remove_blank(tl);
        (*hblock) += parse_stmt(ctx, tl);

        remove_blank(tl);

        if (tl.empty()) {
            break;
        }

        token_t token(*tl.begin());

        if (!end_block_id.empty() && end_block_id == token.identifier()
            && end_block_id_cl == token.type()) {
            --tl;
            break;
        }

        syntax_error_if(
            token.type() != tkncl_t::OPERATOR && token.identifier() != ":",
            token.expression(), token.position());

        --tl;
    }

    return hblock;
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_for_to_step(
    prog_ctx_t& ctx, nu::token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    size_t pos = token.position();
    std::string expr = token.expression();

    syntax_error_if(
        tl.empty() || token.type() != tkncl_t::IDENTIFIER, expr, pos);

    token = *tl.begin();
    --tl;
    remove_blank(tl);

    syntax_error_if(tl.empty(), expr, pos);

    std::string variable_name = token.identifier();

    syntax_error_if(!variable_t::is_valid_name(variable_name, false),
        variable_name + " is an invalid identifier");

    token = *tl.begin();

    syntax_error_if(
        token.type() != tkncl_t::OPERATOR || token.identifier() != "=",
        token.expression(), token.position());

    --tl;
    remove_blank(tl);

    expr_parser_t ep;
    pos = token.position();

    token_list_t etl;
    remove_blank(tl);

    move_sub_expression(tl, etl, "to", tkncl_t::IDENTIFIER);

    syntax_error_if(tl.empty(), expr, pos);

    extract_next_token(tl, token);

    auto from_expr = ep.compile(etl, pos);
    etl.clear();

    while (!tl.empty()
        && (tl.begin()->identifier() != "step"
            && tl.begin()->identifier() != ":")) {
        const token_t token(*tl.begin());
        etl += token;
        --tl;
        remove_blank(tl);
    }

    expr_any_t::handle_t to_expr = ep.compile(etl, pos);
    etl.clear();
    expr_any_t::handle_t step(std::make_shared<expr_literal_t>(1));

    if (!tl.empty() && tl.begin()->identifier() == "step") {
        pos = tl.begin()->position();
        --tl;
        remove_blank(tl);

        move_sub_expression(tl, etl, ":", tkncl_t::OPERATOR);

        step = ep.compile(etl, pos);
    }

    return stmt_t::handle_t(std::make_shared<stmt_for_to_step_t>(
        ctx, variable_name, from_expr, to_expr, step));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_next(
    prog_ctx_t& ctx, nu::token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    std::string variable_name;

    if (!tl.empty()) {
        token = *tl.begin();

        if (!(token.identifier() == ":" && token.type() == tkncl_t::OPERATOR)) {
            syntax_error_if(token.type() != tkncl_t::IDENTIFIER,
                token.expression(), token.position());

            variable_name = token.identifier();

            syntax_error_if(!variable_t::is_valid_name(variable_name, false),
                variable_name + " is an invalid identifier");

            --tl;
            remove_blank(tl);
        }
    }

    return stmt_t::handle_t(std::make_shared<stmt_next_t>(ctx, variable_name));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_exit(
    prog_ctx_t& ctx, nu::token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    syntax_error_if(token.type() != tkncl_t::IDENTIFIER
            || (token.identifier() != "for" && token.identifier() != "sub"
                && token.identifier() != "function"
                && token.identifier() != "while" && token.identifier() != "do"),
        token.expression(), token.position());

    --tl;
    remove_blank(tl);

    if (token.identifier() == "while") {
        return stmt_t::handle_t(std::make_shared<stmt_exit_while_t>(ctx));
    } else if (token.identifier() == "do") {
        return stmt_t::handle_t(std::make_shared<stmt_exit_do_t>(ctx));
    } else if (token.identifier() == "sub"
        || token.identifier() == "function") {
        return stmt_t::handle_t(std::make_shared<stmt_exit_sub_t>(ctx));
    } else if (token.identifier() == "for") {
        return stmt_t::handle_t(std::make_shared<stmt_exit_for_t>(ctx));
    }

    // throws an exception
    syntax_error_if(true, token.expression(), token.position());

    // never executed
    return nullptr;
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_if_then_else(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    expr_parser_t ep;
    token_list_t etl;

    // Move tokens from tl to etl until "then" is found
    move_sub_expression(tl, etl, "then", tkncl_t::IDENTIFIER);

    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());

    expr_any_t::handle_t condition = ep.compile(etl, token.position() + 1);

    // skip "then"
    --tl;
    remove_blank(tl);

    etl.clear();

    // Copy tokens from tl to etl until "else" is found
    move_sub_expression(tl, etl, "else", tkncl_t::IDENTIFIER);

    if (!tl.empty()) {
        // skip "else"
        --tl;
        remove_blank(tl);

        syntax_error_if(tl.empty(), "ELSE-statement expected");
    }

    auto amend_token_list = [](token_list_t& etl) {
        // amend the single 'if cond then line_num' inserting a goto statement
        // to allow to be interpreted as 'if cond then goto line_num'
        // this is provided to allow additional compatilibility with other
        // BASIC dialects
        if (etl.size() == 1 && etl[0].type() == tkncl_t::INTEGRAL) {
            token_t goto_stm(etl[0]);
            goto_stm.set_identifier("goto", token_t::case_t::NOCHANGE);
            goto_stm.set_type(tkncl_t::IDENTIFIER);

            token_t goto_line(etl[0]);

            etl[0] = std::move(goto_stm);
            etl += goto_line;
        }
    };

    amend_token_list(etl);
    stmt_t::handle_t then_stmt = parse_block(ctx, etl);

    remove_blank(tl);

    amend_token_list(tl);
    stmt_t::handle_t else_stmt = parse_block(ctx, tl);

    return stmt_t::handle_t(std::make_shared<stmt_if_then_else_t>(
        ctx, condition, then_stmt, else_stmt));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_elif_stmt(
    prog_ctx_t& ctx, token_t token, token_list_t& tl)
{
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.rbegin();
    tl--;
    remove_blank(tl);

    syntax_error_if(token.identifier() != "then"
            || token.type() != tkncl_t::IDENTIFIER || tl.empty(),
        token.expression(), token.position());

    expr_parser_t ep;
    expr_any_t::handle_t condition = ep.compile(tl, token.position() + 1);
    tl.clear();

    return stmt_t::handle_t(std::make_shared<stmt_elif_t>(ctx, condition));
}


/* -------------------------------------------------------------------------- */

stmt_on_goto_t::label_list_t statement_parser_t::parse_label_list(
    prog_ctx_t& ctx, nu::token_t token, nu::token_list_t& tl)
{
    (void)ctx;

    --tl;
    remove_blank(tl);

    stmt_on_goto_t::label_list_t lbl_list;

    while (!tl.empty()
        && (tl.begin()->type() != tkncl_t::OPERATOR
            && tl.begin()->identifier() != ":")) {
        token = *tl.begin();

        syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
            token.position());

        std::string label_name = token.identifier();

        lbl_list.push_back(label_name);

        --tl;
        remove_blank(tl);

        if (tl.empty()) {
            break;
        }

        token = *tl.begin();

        syntax_error_if((token.type() != tkncl_t::OPERATOR
                            && token.type() != tkncl_t::SUBEXP_END)
                || (token.identifier() != "," && token.identifier() != ":"),
            token.expression(), token.position());

        if (token.identifier() == ":") {
            break;
        }

        --tl;
        remove_blank(tl);
    }

    return lbl_list;
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_on_goto(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    expr_parser_t ep;
    token_list_t etl;

    move_sub_expression(tl, etl, "goto", tkncl_t::IDENTIFIER);

    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());

    expr_any_t::handle_t condition = ep.compile(etl, token.position() + 1);

    auto label_list = parse_label_list(ctx, token, tl);

    return stmt_t::handle_t(
        std::make_shared<stmt_on_goto_t>(ctx, condition, label_list));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_while(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    expr_parser_t ep;
    token_list_t etl;

    move_sub_expression(tl, etl, "do", tkncl_t::IDENTIFIER);

    remove_blank(tl);

    expr_any_t::handle_t condition = ep.compile(etl, token.position() + 1);

    if (!tl.empty()) {
        // skip "do"
        --tl;
        remove_blank(tl);

        if (!tl.empty()) {
            stmt_t::handle_t while_stmt = parse_block(ctx, tl);

            // while <condition> do <stmt>
            return stmt_t::handle_t(
                std::make_shared<stmt_while_t>(ctx, condition, while_stmt));
        }
    }

    // while <condition> [do]
    return stmt_t::handle_t(std::make_shared<stmt_while_t>(ctx, condition));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_loop_while(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();
    --tl;
    remove_blank(tl);

    syntax_error_if(token.identifier() != "while"
            || token.type() != tkncl_t::IDENTIFIER || tl.empty(),
        token.expression(), token.position());

    expr_parser_t ep;
    token_list_t etl;

    move_sub_expression(tl, etl, ":", tkncl_t::OPERATOR);

    remove_blank(tl);

    expr_any_t::handle_t condition = ep.compile(etl, token.position() + 1);

    // LOOP WHILE <condition>
    return stmt_t::handle_t(
        std::make_shared<stmt_loop_while_t>(ctx, condition));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_goto_gosub(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    std::string instruction = token.identifier();
    bool go_blank_to = instruction == "go";

    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    std::string identifier = token.identifier();

    syntax_error_if(go_blank_to && identifier != "to" && identifier != "sub",
        token.expression(), token.position());

    return ((instruction.find("to") != std::string::npos)
            ? parse_branch_instr<stmt_goto_t>(ctx, token, tl)
            : parse_branch_instr<stmt_gosub_t>(ctx, token, tl));
}


/* -------------------------------------------------------------------------- */

} // namespace nu
