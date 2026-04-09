//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

// Struct and class statement parsing:
//   has_top_level_assign, parse_struct, parse_struct_element,
//   parse_method_call_stmt, parse_class, parse_class_member

/* -------------------------------------------------------------------------- */

#include "nu_parser_impl.h"


/* -------------------------------------------------------------------------- */

namespace nu {

// Suppress implicit instantiation — definitions live in nu_statement_parser.cc.
extern template stmt_t::handle_t
statement_parser_t::parse_procedure<stmt_sub_t>(
    prog_ctx_t&, token_t, token_list_t&);
extern template stmt_t::handle_t
statement_parser_t::parse_procedure<stmt_function_t>(
    prog_ctx_t&, token_t, token_list_t&);


/* -------------------------------------------------------------------------- */

bool statement_parser_t::has_top_level_assign(const token_list_t& tl)
{
    int depth = 0;
    for (const auto& tok : tl.data()) {
        if (tok.type() == tkncl_t::SUBEXP_BEGIN)
            ++depth;
        else if (tok.type() == tkncl_t::SUBEXP_END)
            --depth;
        else if (depth == 0 && tok.type() == tkncl_t::OPERATOR
            && tok.identifier() == "=")
            return true;
    }
    return false;
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_struct(
    prog_ctx_t& ctx, nu::token_t token, nu::token_list_t& tl)
{
    // Skip keyword STRUCT
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
        token.position());

    const std::string& id = token.identifier();

    syntax_error_if(!variable_t::is_valid_name(id, false), token.expression(),
        token.position(), "'" + id + "' is an invalid identifier");

    --tl;
    remove_blank(tl);
    syntax_error_if(!tl.empty(), token.expression(), token.position());

    ctx.compiling_struct_name = id;

    return stmt_t::handle_t(std::make_shared<stmt_struct_t>(ctx, id));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_struct_element(
    prog_ctx_t& ctx, nu::token_t token, nu::token_list_t& tl)
{
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
        token.position());

    std::string id = token.identifier();

    syntax_error_if(!variable_t::is_valid_name(id, false), token.expression(),
        token.position(), "'" + id + "' is an invalid identifier");

    --tl;
    remove_blank(tl);

    auto type = variable_t::typename_by_type(variable_t::type_by_name(id));

    size_t size = 0;

    if (tl.empty()) {
        return stmt_t::handle_t(
            std::make_shared<stmt_struct_element_t>(ctx, id, type, size));
    }

    token = *tl.begin();

    syntax_error_if(
        !((token.type() == tkncl_t::IDENTIFIER && token.identifier() == "as")
            || token.type() == tkncl_t::SUBEXP_BEGIN),
        token.expression(), token.position());

    if (token.type() == tkncl_t::SUBEXP_BEGIN) {
        --tl;
        remove_blank(tl);
        syntax_error_if(tl.empty(), token.expression(), token.position());

        token = *tl.begin();

        syntax_error_if(token.type() != tkncl_t::INTEGRAL, token.expression(),
            token.position());

        try {
            size = size_t(nu::stoll(token.identifier()));
        } catch (...) {
            syntax_error(token.expression(), token.position());
        }

        --tl;
        remove_blank(tl);
        syntax_error_if(tl.empty(), token.expression(), token.position());

        token = *tl.begin();

        syntax_error_if(token.type() != tkncl_t::SUBEXP_END, token.expression(),
            token.position(), "'" + id + "' is an invalid identifier");

        --tl;
        remove_blank(tl);
    }

    if (tl.empty()) {
        return stmt_t::handle_t(
            std::make_shared<stmt_struct_element_t>(ctx, id, type, size));
    }

    token = *tl.begin();

    if (token.type() == tkncl_t::IDENTIFIER && token.identifier() == "as") {
        --tl;
        remove_blank(tl);

        syntax_error_if(tl.empty(), token.expression(), token.position());

        token = *tl.begin();

        syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
            token.position());

        type = token.identifier();

        --tl;
        remove_blank(tl);
    }

    return stmt_t::handle_t(
        std::make_shared<stmt_struct_element_t>(ctx, id, type, size));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_method_call_stmt(prog_ctx_t& ctx,
    token_t token, token_list_t& tl, const std::string& obj_name,
    const std::string& method_name)
{
    // If the argument list is wrapped in parentheses, strip them
    if (!tl.empty() && tl.begin()->type() == tkncl_t::SUBEXP_BEGIN) {
        --tl; // consume "("
        remove_blank(tl);

        // Empty arg list: obj.Method()
        if (!tl.empty() && tl.begin()->type() == tkncl_t::SUBEXP_END) {
            --tl; // consume ")"
            return stmt_t::handle_t(std::make_shared<stmt_method_call_t>(
                obj_name, method_name, ctx));
        }

        // Strip the matching closing ")" from the end
        if (!tl.empty() && tl.rbegin()->type() == tkncl_t::SUBEXP_END)
            tl--; // postfix --: removes last token

        if (!tl.empty())
            token = *tl.begin();
    }

    if (tl.empty()) {
        return stmt_t::handle_t(
            std::make_shared<stmt_method_call_t>(obj_name, method_name, ctx));
    }

    return parse_arg_list<stmt_method_call_t, 0>(
        ctx, token, tl,
        [](const token_t& t) {
            return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
        },
        obj_name, method_name, ctx);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_class(
    prog_ctx_t& ctx, nu::token_t token, nu::token_list_t& tl)
{
    // Skip keyword CLASS
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
        token.position());

    const std::string id = token.identifier();

    syntax_error_if(
        !variable_t::is_valid_name(id, true), id + " is an invalid class name");

    --tl;
    remove_blank(tl);
    syntax_error_if(!tl.empty(), token.expression(), token.position());

    ctx.compiling_class_name = id;

    return stmt_t::handle_t(std::make_shared<stmt_class_t>(ctx, id));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_class_member(
    prog_ctx_t& ctx, nu::token_t token, nu::token_list_t& tl)
{
    // Consume optional Public/Private modifier
    bool is_public = true;

    if (token.identifier() == "public") {
        is_public = true;
        --tl;
        remove_blank(tl);
        syntax_error_if(tl.empty(), token.expression(), token.position());
        token = *tl.begin();
    } else if (token.identifier() == "private") {
        is_public = false;
        --tl;
        remove_blank(tl);
        syntax_error_if(tl.empty(), token.expression(), token.position());
        token = *tl.begin();
    }

    ctx.compiling_class_member_is_public = is_public;

    // Consume optional Overridable / Overrides keyword before Sub/Function
    if (token.identifier() == "overridable") {
        ctx.class_overridable_methods.insert(ctx.compiling_class_name + "."
            + ""); // placeholder; full name known after Sub
        --tl;
        remove_blank(tl);
        syntax_error_if(tl.empty(), token.expression(), token.position());
        token = *tl.begin();
    } else if (token.identifier() == "overrides") {
        --tl;
        remove_blank(tl);
        syntax_error_if(tl.empty(), token.expression(), token.position());
        token = *tl.begin();
    }

    const std::string id = token.identifier();

    // Inherits BaseClass
    if (id == "inherits") {
        --tl; // consume "inherits"
        remove_blank(tl);
        syntax_error_if(tl.empty(), token.expression(), token.position());
        token = *tl.begin();
        syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
            token.position());
        const std::string base_name = token.identifier();
        --tl;
        remove_blank(tl);
        return stmt_t::handle_t(
            std::make_shared<stmt_inherits_t>(ctx, base_name));
    }

    // Method declaration
    if (id == "sub") {
        return parse_procedure<stmt_sub_t>(ctx, token, tl);
    }

    if (id == "function") {
        return parse_procedure<stmt_function_t>(ctx, token, tl);
    }

    // Data member declaration: treated like a struct element
    const std::string field_name = token.identifier();

    const std::string saved_struct = ctx.compiling_struct_name;
    ctx.compiling_struct_name = ctx.compiling_class_name;
    auto handle = parse_struct_element(ctx, token, tl);
    ctx.compiling_struct_name = saved_struct;

    // Register access level for the field
    ctx.class_member_visibility[ctx.compiling_class_name + "." + field_name]
        = is_public;

    return handle;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
