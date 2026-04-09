//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

// I/O statement parsing:
//   parse_print, parse_data, parse_var_arg, parse_read,
//   parse_input, parse_input_con, parse_input_file, parse_fd_args,
//   parse_read_file, parse_open, parse_close

/* -------------------------------------------------------------------------- */

#include "nu_parser_impl.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_print(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    // expression — safe with Locate / scatter plots. "Write#" is unchanged.
    const bool is_write
        = token.identifier() == "write" || token.identifier() == "write#";

    // print# filenum  (also handle "print #n" with a space before #)
    bool is_file
        = token.identifier() == "print#" || token.identifier() == "write#";

    if (!is_file && !tl.empty()) {
        const std::string& peek = tl.begin()->identifier();
        if (peek.size() > 1 && peek.c_str()[0] == '#') {
            const std::string fds = peek.substr(1);
            if (variant_t::is_integer(fds))
                is_file = true;
        }
    }

    int fd = 0;
    if (is_file) {
        // print #filenum
        remove_blank(tl);

        syntax_error_if(tl.empty(), token.expression(), token.position());

        token = *tl.begin();

        std::string fds = token.identifier();

        if (fds.size() > 1 && fds.c_str()[0] == '#')
            fds = fds.substr(1, fds.size() - 1);

        syntax_error_if(!variant_t::is_integer(fds), token.expression(),
            token.position(),
            "print# expected to be passed a valid filenumber");

        fd = nu::stoi(fds);

        --tl;
        remove_blank(tl);

        if (!tl.empty()) {
            token = *tl.begin();

            syntax_error_if(
                token.type() != tkncl_t::OPERATOR || token.identifier() != ",",
                token.expression(), token.position());

            --tl;
            remove_blank(tl);
        }
    }

    return parse_arg_list<stmt_print_t, 0>(
        ctx, token, tl,
        [](const token_t& t) {
            return t.type() == tkncl_t::OPERATOR
                && (t.identifier() == "," || t.identifier() == ";");
        },
        ctx, is_write, fd);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_data(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());


    if (token.type() == tkncl_t::STRING_LITERAL && tl.size() == 1) {
        --tl;

        // Create statement to print out a literal string
        return stmt_t::handle_t(
            std::make_shared<stmt_data_t>(ctx, token.identifier()));
    }

    return parse_arg_list<stmt_data_t, 0>(
        ctx, token, tl,
        [](const token_t& t) {
            return t.type() == tkncl_t::OPERATOR
                && (t.identifier() == "," || t.identifier() == ";");
        },
        ctx);
}


/* -------------------------------------------------------------------------- */

var_arg_t statement_parser_t::parse_var_arg(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    token = *tl.begin();

    remove_blank(tl);

    syntax_error_if(tl.empty() || token.type() != tkncl_t::IDENTIFIER,
        token.expression(), token.position());

    std::string variable_name = token.identifier();

    --tl;
    remove_blank(tl);

    expr_any_t::handle_t variable_vector_index = nullptr;

    if (!tl.empty()) {
        token = *tl.begin();
        token_list_t vect_etl;
        variable_vector_index = parse_sub_expr(ctx, token, tl, vect_etl);
    }

    return std::make_pair(variable_name, variable_vector_index);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_read(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    var_list_t var_list;

    while (!tl.empty()
        && (token.type() != tkncl_t::OPERATOR || token.identifier() != ":")) {

        const auto var = parse_var_arg(ctx, token, tl);
        var_list.emplace_back(var);

        if (!tl.empty()) {
            token = *tl.begin();

            syntax_error_if(token.type() != tkncl_t::OPERATOR
                    || (token.identifier() != "," && token.identifier() != ":"),
                token.expression(), token.position());

            if (token.identifier() == ":"
                && token.type() == tkncl_t::OPERATOR) {
                break;
            }

            extract_next_token(tl, token);

            token = *tl.begin();
        }
    }

    syntax_error_if(var_list.empty(), token.expression(), token.position());

    return stmt_t::handle_t(std::make_shared<stmt_read_t>(ctx, var_list));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_input(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    // input #<filenumber>
    if (tl.begin()->type() == tkncl_t::IDENTIFIER) {
        std::string id = tl.begin()->identifier();

        // if token begins with # it should be
        // an filenumber (which should be an integer)
        if (id.size() > 1 && id.c_str()[0] == '#') {
            // Check if it is a integer
            id = id.substr(1, id.size() - 1);

            syntax_error_if(!variant_t::is_integer(id), token.expression(),
                token.position(),
                "input# expected to be passed a valid filenumber");

            token.set_identifier(id, token_t::case_t::LOWER);
            token.set_type(tkncl_t::INTEGRAL);

            // Replace old token with new one
            --tl;
            tl.data().push_front(token);

            return parse_input_file(ctx, token, tl);
        }
    }

    return parse_input_con(ctx, token, tl);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_input_con(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    std::string prompt;

    if (token.type() == tkncl_t::STRING_LITERAL) {
        prompt = token.identifier();

        extract_next_token(tl, token);

        syntax_error_if(
            (token.type() != tkncl_t::OPERATOR
                || (token.identifier() != ";" && token.identifier() != ",")),
            token.expression(), token.position());

        // The semicolon will print a question mark at
        // the end of the prompt string
        if (token.identifier() == ";") {
            prompt += "?";
        }

        extract_next_token(tl, token);
    }

    var_list_t var_list;

    while (!tl.empty()
        && (token.type() != tkncl_t::OPERATOR || token.identifier() != ":")) {

        auto var = parse_var_arg(ctx, token, tl);
        var_list.push_back(var);

        if (!tl.empty()) {
            token = *tl.begin();

            syntax_error_if(token.type() != tkncl_t::OPERATOR
                    || (token.identifier() != "," && token.identifier() != ":"),
                token.expression(), token.position());

            if (token.identifier() == ":"
                && token.type() == tkncl_t::OPERATOR) {
                break;
            }

            extract_next_token(tl, token);

            token = *tl.begin();
        }
    }

    syntax_error_if(var_list.empty(), token.expression(), token.position());

    return stmt_t::handle_t(
        std::make_shared<stmt_input_t>(ctx, prompt, var_list));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_input_file(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;

    int fd = 0;
    var_list_t vlist;

    parse_fd_args(ctx, token, tl, fd, vlist);

    return stmt_t::handle_t(
        std::make_shared<stmt_input_file_t>(ctx, fd, vlist));
}


/* -------------------------------------------------------------------------- */

void statement_parser_t::parse_fd_args(prog_ctx_t& ctx, token_t token,
    nu::token_list_t& tl, int& fd, var_list_t& var_list)
{
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    syntax_error_if(token.type() != tkncl_t::INTEGRAL, token.expression(),
        token.position());

    try {
        fd = nu::stoi(token.identifier());
    } catch (...) {
        syntax_error(token.expression(), token.position());
    }

    --tl;
    remove_blank(tl);

    if (!tl.empty()) { // must be a comma separator

        token = *tl.begin();
        syntax_error_if(
            token.type() != tkncl_t::OPERATOR || token.identifier() != ",",
            token.expression(), token.position());
        --tl; // remove comma from tl
    }

    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    while (!tl.empty()
        && (token.type() != tkncl_t::OPERATOR || token.identifier() != ":")) {

        const auto var = parse_var_arg(ctx, token, tl);
        var_list.emplace_back(var);

        if (!tl.empty()) {
            token = *tl.begin();

            syntax_error_if(
                token.type() != tkncl_t::OPERATOR || token.identifier() != ",",
                token.expression(), token.position());

            extract_next_token(tl, token);

            token = *tl.begin();
        }
    }

    syntax_error_if(var_list.empty(), token.expression(), token.position());
}


/* -------------------------------------------------------------------------- */

// parse statement 'READ filenumber, variable, sizeexpr'
stmt_t::handle_t statement_parser_t::parse_read_file(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;

    int fd = 0;

    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    syntax_error_if(token.type() != tkncl_t::INTEGRAL, token.expression(),
        token.position());

    try {
        fd = nu::stoi(token.identifier());
    } catch (...) {
        syntax_error(token.expression(), token.position());
    }

    --tl;
    remove_blank(tl);

    if (!tl.empty()) { // must be a comma separator

        token = *tl.begin();
        syntax_error_if(
            token.type() != tkncl_t::OPERATOR || token.identifier() != ",",
            token.expression(), token.position());
        --tl; // remove comma from tl
    }

    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    const var_arg_t var = parse_var_arg(ctx, token, tl);

    token = *tl.begin();

    syntax_error_if(
        token.type() != tkncl_t::OPERATOR || token.identifier() != ",",
        token.expression(), token.position());

    extract_next_token(tl, token);

    token = *tl.begin();

    syntax_error_if(tl.empty(), token.expression(), token.position());

    remove_blank(tl);

    return parse_arg_list<stmt_read_file_t, 1>(
        ctx, token, tl,
        [](const token_t& t) {
            return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
        },
        ctx, fd, var);
}


/* -------------------------------------------------------------------------- */

// Open pathname For mode [Access access] As [#]filenumber
stmt_t::handle_t statement_parser_t::parse_open(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());

    expr_parser_t ep;
    token_list_t etl;

    move_sub_expression(tl, etl, "for", tkncl_t::IDENTIFIER);

    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());

    expr_any_t::handle_t filename = ep.compile(etl, token.position() + 1);

    // skip "for"
    extract_next_token(tl, token);

    syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
        token.position());

    std::string mode = token.identifier();

    syntax_error_if(mode != "input" && mode != "output" && mode != "random"
            && mode != "append",
        token.expression(), token.position());

    // skip mode
    extract_next_token(tl, token);
    std::string access = "";

    if (token.type() == tkncl_t::IDENTIFIER && token.identifier() == "access") {
        extract_next_token(tl, token);

        access = token.identifier();

        syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
            token.position());

        syntax_error_if(access != "read" && access != "write",
            token.expression(), token.position());

        extract_next_token(tl, token);
    }

    if (access == "read") {
        syntax_error_if(token.type() != tkncl_t::IDENTIFIER
                || (token.identifier() != "write"
                    && token.identifier() != "as"),
            token.expression(), token.position());

        if (token.identifier() == "write") {
            access += " write";
            extract_next_token(tl, token);
        }
    }

    syntax_error_if(
        token.type() != tkncl_t::IDENTIFIER || token.identifier() != "as",
        token.expression(), token.position());

    // skip "as"
    extract_next_token(tl, token);

    --tl; // remove #<filenumber> token from tl

    std::string fds = token.identifier();

    if (fds.size() > 1 && fds.c_str()[0] == '#')
        fds = fds.substr(1, fds.size() - 1);

    int fd = 0;

    try {
        fd = nu::stoi(fds);
    } catch (...) {
        syntax_error(token.expression(), token.position());
    }

    return stmt_t::handle_t(
        std::make_shared<stmt_open_t>(ctx, filename, mode, access, fd));
}


/* -------------------------------------------------------------------------- */

// close [#]filenumber
stmt_t::handle_t statement_parser_t::parse_close(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    extract_next_token(tl, token);

    --tl; // remove #<filenumber> token from tl

    std::string fds = token.identifier();

    if (fds.size() > 1 && fds.c_str()[0] == '#')
        fds = fds.substr(1, fds.size() - 1);

    int fd = 0;

    try {
        fd = nu::stoi(fds);
    } catch (...) {
        syntax_error(token.expression(), token.position());
    }

    return stmt_t::handle_t(std::make_shared<stmt_close_t>(ctx, fd));
}


/* -------------------------------------------------------------------------- */

} // namespace nu
