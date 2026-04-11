//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

// Core statement parser:
//   utilities (remove_blank, extract_next_token, move_sub_expression),
//   parse_expr, parse_sub_expr, parse_let,
//   parse_locate, parse_delay, parse_mdelay,
//   parse_const, parse_label, parse_end, parse_stop,
//   template definitions for parse_procedure and parse_branch_instr,
//   parse_stmt (main keyword dispatcher),
//   compile_line

/* -------------------------------------------------------------------------- */

#include "nu_parser_impl.h"


/* -------------------------------------------------------------------------- */

namespace nu {

namespace {

    bool consume_qualified_callable_name(token_list_t& tl, std::string& name,
        const std::string& expr, size_t pos)
    {
        if (tl.empty()) {
            return false;
        }

        auto token = *tl.begin();

        if (token.type() != tkncl_t::OPERATOR || token.identifier() != "::") {
            return false;
        }

        --tl;
        statement_parser_t::remove_blank(tl);
        syntax_error_if(tl.empty(), expr, pos);

        token = *tl.begin();
        syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
            token.position());

        name += "::";
        name += token.identifier();

        --tl;
        statement_parser_t::remove_blank(tl);

        return true;
    }

} // namespace


/* -------------------------------------------------------------------------- */

void statement_parser_t::remove_blank(nu::token_list_t& tl)
{
    while (!tl.empty()
        && (tl.begin()->type() == tkncl_t::BLANK
            || tl.begin()->type() == tkncl_t::LINE_COMMENT
            || tl.begin()->type() == tkncl_t::NEWLINE)) {
        --tl;
    }

    while (!tl.empty()
        && (tl.rbegin()->type() == tkncl_t::BLANK
            || tl.begin()->type() == tkncl_t::LINE_COMMENT)) {
        tl--;
    }
}


/* -------------------------------------------------------------------------- */

void statement_parser_t::extract_next_token(token_list_t& tl, token_t& token,
    std::function<bool(const token_list_t& tl, const token_t& token)> check)
{
    --tl;
    remove_blank(tl);
    syntax_error_if(check(tl, token), token.expression(), token.position());
    token = *tl.begin();
}


/* -------------------------------------------------------------------------- */

void statement_parser_t::move_sub_expression(token_list_t& source_tl,
    token_list_t& dst_tl, const std::string& id, tkncl_t idtype)
{
    while (!source_tl.empty()
        && (!(source_tl.begin()->type() == idtype
            && source_tl.begin()->identifier() == id))) {
        const token_t token(*source_tl.begin());
        dst_tl += token;
        --source_tl;
        remove_blank(source_tl);
    }
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_locate(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    return parse_arg_list<stmt_locate_t, 0>(
        ctx, token, tl,
        [](const token_t& t) {
            return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
        },
        ctx);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_delay(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    return parse_arg_list<stmt_delay_t, 0>(
        ctx, token, tl,
        [](const token_t& t) {
            return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
        },
        ctx);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_mdelay(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    return parse_arg_list<stmt_mdelay_t, 0>(
        ctx, token, tl,
        [](const token_t& t) {
            return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
        },
        ctx);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_using(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());
    token = *tl.begin();
    syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
        token.position());

    const std::string module_name = token.identifier();

    --tl;
    remove_blank(tl);
    syntax_error_if(!tl.empty(), token.expression(), token.position());

    return stmt_t::handle_t(std::make_shared<stmt_using_t>(ctx, module_name));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_include(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());
    token = *tl.begin();
    syntax_error_if(token.type() != tkncl_t::STRING_LITERAL, token.expression(),
        token.position());

    const std::string module_name = token.identifier();

    --tl;
    remove_blank(tl);
    syntax_error_if(!tl.empty(), token.expression(), token.position());

    return stmt_t::handle_t(std::make_shared<stmt_using_t>(ctx, module_name));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_syntax(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    --tl;
    remove_blank(tl);

    syntax_error_if(tl.empty(), token.expression(), token.position());
    token = *tl.begin();
    syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
        token.position());

    prog_ctx_t::syntax_mode_t mode;

    if (token.identifier() == "legacy") {
        mode = prog_ctx_t::syntax_mode_t::LEGACY;
    } else if (token.identifier() == "modern") {
        mode = prog_ctx_t::syntax_mode_t::MODERN;
    } else {
        syntax_error(token.expression(), token.position());
    }

    --tl;
    remove_blank(tl);
    syntax_error_if(!tl.empty(), token.expression(), token.position());

    return stmt_t::handle_t(std::make_shared<stmt_syntax_t>(ctx, mode));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_expr(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    if (token.type() == tkncl_t::STRING_LITERAL && tl.size() == 1) {
        --tl;

        // Create statement to print out a literal string
        return stmt_t::handle_t(
            std::make_shared<stmt_expr_t>(ctx, token.identifier()));
    }

    return parse_arg_list<stmt_expr_t, 0>(
        ctx, token, tl,
        [](const token_t& t) {
            return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
        },
        ctx);
}


/* -------------------------------------------------------------------------- */

expr_any_t::handle_t statement_parser_t::parse_sub_expr(
    prog_ctx_t& ctx, token_t token, token_list_t& tl, token_list_t& etl)
{
    (void)ctx;

    expr_any_t::handle_t sub_exp = nullptr;

    if (token.type() == tkncl_t::SUBEXP_BEGIN) {
        int parenthesis_level = 1;

        extract_next_token(tl, token);

        while (!tl.empty()) {
            token = *tl.begin();
            --tl;
            remove_blank(tl);

            if (token.type() == tkncl_t::SUBEXP_BEGIN) {
                ++parenthesis_level;
            } else if (token.type() == tkncl_t::SUBEXP_END) {
                --parenthesis_level;

                if (parenthesis_level < 1) {
                    break;
                }
            }

            etl += token;
        }

        syntax_error_if(parenthesis_level != 0 || etl.empty(),
            token.expression(), token.position());
    }

    if (!etl.empty()) {
        expr_parser_t ep;
        sub_exp = ep.compile(etl, token.position());
    }

    return sub_exp;
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_let(
    prog_ctx_t& ctx, nu::token_list_t& tl)
{
    remove_blank(tl);

    if (tl.empty()) {
        return stmt_t::handle_t(std::make_shared<stmt_empty_t>(ctx));
    }

    token_t token(*tl.begin());
    size_t pos = token.position();
    std::string expr = token.expression();

    syntax_error_if(token.type() != tkncl_t::IDENTIFIER, expr, pos);

    std::string identifier = token.identifier();

    --tl;
    remove_blank(tl);

    if (consume_qualified_callable_name(tl, identifier, expr, pos)) {
        if (tl.empty()) {
            return stmt_t::handle_t(
                std::make_shared<stmt_call_t>(identifier, ctx));
        }

        token = *tl.begin();

        if (token.type() == tkncl_t::SUBEXP_BEGIN) {
            --tl;
            remove_blank(tl);

            if (!tl.empty() && tl.begin()->type() == tkncl_t::SUBEXP_END) {
                --tl;
                return std::make_shared<stmt_call_t>(identifier, ctx);
            }

            if (!tl.empty() && tl.rbegin()->type() == tkncl_t::SUBEXP_END) {
                tl--;
            }

            token = *tl.begin();
        }

        return parse_arg_list<stmt_call_t, 0>(
            ctx, token, tl,
            [](const token_t& t) {
                return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
            },
            identifier, ctx);
    }

    if (tl.empty()) {
        return stmt_t::handle_t(std::make_shared<stmt_call_t>(identifier, ctx));
    }

    token = *tl.begin();

    if (token.type() != tkncl_t::OPERATOR
        && token.type() != tkncl_t::SUBEXP_BEGIN) {
        return parse_arg_list<stmt_call_t, 0>(
            ctx, token, tl,
            [](const token_t& t) {
                return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
            },
            identifier, ctx);
    }

    token_list_t vect_etl;
    expr_any_t::handle_t variable_vector_index
        = parse_sub_expr(ctx, token, tl, vect_etl);

    remove_blank(tl);
    syntax_error_if(tl.empty(), expr, pos);
    token = *tl.begin();

    bool struct_member_id = false;

    nu::expr_any_t::handle_t struct_member_vector_index;

    // Consume all dot-separated member names: var.a.b.c ...
    while (token.type() == tkncl_t::OPERATOR && token.identifier() == ".") {
        --tl;
        remove_blank(tl);
        syntax_error_if(tl.empty(), expr, pos);

        token = *tl.begin();

        syntax_error_if(token.type() != tkncl_t::IDENTIFIER, expr, pos);

        identifier += ".";
        identifier += token.identifier();

        --tl;
        remove_blank(tl);
        syntax_error_if(tl.empty(), expr, pos);

        token = *tl.begin();

        // Optional vector index on the last member OR method call with args
        if (token.type() == tkncl_t::SUBEXP_BEGIN) {
            // If no '=' follows at top level, this is a method call:
            // obj.Method(args)
            if (!has_top_level_assign(tl)) {
                auto dot_pos = identifier.rfind('.');
                std::string obj_name = identifier.substr(0, dot_pos);
                std::string method_name = identifier.substr(dot_pos + 1);
                return parse_method_call_stmt(
                    ctx, token, tl, obj_name, method_name);
            }

            // Otherwise it's an array member subscript: obj.arr(i) = ...
            token_list_t etl;
            move_sub_expression(tl, etl, "=", tkncl_t::OPERATOR);

            expr_parser_t ep;
            struct_member_vector_index = ep.compile(etl, pos);

            syntax_error_if(tl.empty(), token.expression(), token.position());

            token = *tl.begin();
        }

        struct_member_id = true;

        // If more dots follow, continue looping; otherwise stop
        if (!(token.type() == tkncl_t::OPERATOR && token.identifier() == "."))
            break;
    }

    // If we built a dotted name but the next token is not '=', this is a
    // method call without parentheses (e.g. "obj.Method" or "obj.Method arg")
    if (struct_member_id
        && (token.type() != tkncl_t::OPERATOR || token.identifier() != "=")) {
        auto dot_pos = identifier.rfind('.');
        std::string obj_name = identifier.substr(0, dot_pos);
        std::string method_name = identifier.substr(dot_pos + 1);
        return parse_method_call_stmt(ctx, token, tl, obj_name, method_name);
    }

    syntax_error_if(
        token.type() != tkncl_t::OPERATOR || token.identifier() != "=", expr,
        pos);

    --tl;
    remove_blank(tl);

    expr_parser_t ep;
    pos = token.position();

    token_list_t etl;
    remove_blank(tl);

    move_sub_expression(tl, etl, ":", tkncl_t::OPERATOR);

    syntax_error_if(!struct_member_id && identifier != "me"
            && !variable_t::is_valid_name(identifier, false),
        identifier + " is an invalid identifier");

    return stmt_t::handle_t(std::make_shared<stmt_let_t>(ctx, identifier,
        ep.compile(etl, pos), variable_vector_index, struct_member_vector_index,
        struct_member_id));
}


/* -------------------------------------------------------------------------- */

// parse 'label_identifier:'
stmt_t::handle_t statement_parser_t::parse_label(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    // Check for new label
    if (tl.size() > 1 && (tl.begin() + 1)->type() == tkncl_t::OPERATOR
        && (tl.begin() + 1)->identifier() == ":") {
        label_tbl_t& prog_label = ctx.prog_label;

        const std::string& label = token.identifier();

        if (prog_label.is_defined(label)) {
            syntax_error_if(int(prog_label[label]) != _parsing_line,
                "Label '" + token.identifier() + "' alrady defined at "
                    + nu::to_string(_parsing_line));
        } else {
            prog_label[label] = _parsing_line;
        }

        --tl; // consume label name
        --tl; // consume ':'
        remove_blank(tl);

        // If more statements follow on the same logical line (e.g. "lbl: x=1"),
        // parse them now so parse_block doesn't require an extra ':' separator.
        if (!tl.empty())
            return parse_block(ctx, tl);

        return stmt_t::handle_t(std::make_shared<stmt_empty_t>(ctx));
    }

    return stmt_t::handle_t();
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_const(
    prog_ctx_t& ctx, token_t token, token_list_t& tl)
{
    auto get_type = [&](std::string& type, const std::string& variable_name) {
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

    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
        token.position());

    std::string variable_name = token.identifier();

    syntax_error_if(!variable_t::is_valid_name(variable_name, false),
        variable_name + " is an invalid identifier");

    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());
    token = *tl.begin();

    std::string type;
    get_type(type, variable_name);

    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    syntax_error_if(
        token.type() != tkncl_t::OPERATOR || token.identifier() != "=",
        token.expression(), token.position());

    --tl;
    remove_blank(tl);

    expr_parser_t ep;
    auto pos = token.position();

    token_list_t etl;
    remove_blank(tl);

    move_sub_expression(tl, etl, ":", tkncl_t::OPERATOR);

    syntax_error_if(!variable_t::is_valid_name(variable_name, false),
        variable_name + " is an invalid identifier");

    return std::make_shared<stmt_const_t>(
        ctx, variable_name, type, ep.compile(etl, pos));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_end(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    (void)token;
    --tl;
    remove_blank(tl);

    if (!tl.empty()) {
        token_t token = *tl.begin();

        syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
            token.position());

        const auto& id = token.identifier();

        if (id == "while") {
            --tl;
            return stmt_t::handle_t(std::make_shared<stmt_wend_t>(ctx));
        } else if (id == "if") {
            --tl;
            return stmt_t::handle_t(std::make_shared<stmt_endif_t>(ctx));
        } else if (id == "sub") {
            --tl;
            return stmt_t::handle_t(std::make_shared<stmt_endsub_t>(ctx));
        } else if (id == "function") {
            --tl;
            return stmt_t::handle_t(std::make_shared<stmt_endfunction_t>(ctx));
        } else if (id == "struct") {
            ctx.compiling_struct_name.clear();
            --tl;
            return stmt_t::handle_t(std::make_shared<stmt_endstruct_t>(ctx));
        } else if (id == "class") {
            --tl;
            auto h = stmt_t::handle_t(std::make_shared<stmt_endclass_t>(ctx));
            ctx.compiling_class_name.clear();
            return h;
        } else if (id == "select") {
            --tl;
            return stmt_t::handle_t(std::make_shared<stmt_end_select_t>(ctx));
        } else {
            syntax_error(token.expression(), token.position());
        }
    }

    return stmt_t::handle_t(std::make_shared<stmt_end_t>(ctx));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_stop(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    (void)token;

    --tl;
    remove_blank(tl);

    if (!tl.empty()) {
        token_t token = *tl.begin();
        syntax_error(token.expression(), token.position());
    }

    return stmt_t::handle_t(std::make_shared<stmt_stop_t>(ctx));
}


/* -------------------------------------------------------------------------- */
// Template definitions
// Explicit instantiations are provided below so that the definitions do not
// need to be repeated in every translation unit that calls these methods.
/* -------------------------------------------------------------------------- */

template <class T>
stmt_t::handle_t statement_parser_t::parse_branch_instr(
    prog_ctx_t& ctx, token_t token, nu::token_list_t& tl)
{
    remove_blank(tl);

    if (!tl.empty()
        && (tl.begin()->identifier() == "to"
            || tl.begin()->identifier() == "sub")) {
        --tl;
        remove_blank(tl);

        if (!tl.empty()) {
            token = *tl.begin();
        }
    }

    syntax_error_if(tl.empty()
            || (token.type() != tkncl_t::INTEGRAL
                && token.type() != tkncl_t::IDENTIFIER),
        token.expression(), token.position());

    const auto& label = token.identifier();

    --tl;

    if (variant_t::is_integer(label)) {
        prog_pointer_t::line_number_t ln = nu::stoi(label);
        syntax_error_if(ln <= 0, token.expression(), token.position());

        ctx.prog_label.set_explicit_line_reference_mode();

        return stmt_t::handle_t(std::make_shared<T>(ctx, ln));
    }

    return stmt_t::handle_t(std::make_shared<T>(ctx, label));
}

template stmt_t::handle_t statement_parser_t::parse_branch_instr<stmt_goto_t>(
    prog_ctx_t&, token_t, token_list_t&);
template stmt_t::handle_t statement_parser_t::parse_branch_instr<stmt_gosub_t>(
    prog_ctx_t&, token_t, token_list_t&);


/* -------------------------------------------------------------------------- */

template <class T>
stmt_t::handle_t statement_parser_t::parse_procedure(
    prog_ctx_t& ctx, nu::token_t token, nu::token_list_t& tl)
{
    // Skip keyword SUB / FUNCTION
    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    token = *tl.begin();

    syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
        token.position());

    --tl;
    remove_blank(tl);
    syntax_error_if(tl.empty(), token.expression(), token.position());

    std::string id = token.identifier();

    // "New" is reserved globally but is a valid constructor name inside a
    // class.
    const bool is_class_new
        = (id == "new" && !ctx.compiling_class_name.empty());

    syntax_error_if(!is_class_new && !variable_t::is_valid_name(id, false),
        id + " is an invalid identifier");

    // When inside a class body, mangle the name as "ClassName.MethodName"
    if (!ctx.compiling_class_name.empty()) {
        id = ctx.compiling_class_name + "." + id;
    }

    token = *tl.begin();

    syntax_error_if(token.type() != tkncl_t::SUBEXP_BEGIN, token.expression(),
        token.position());

    --tl;
    remove_blank(tl);

    token = *tl.begin();

    stmt_t::handle_t stmt_handle;

    if (token.type() == tkncl_t::SUBEXP_END) {
        --tl;
        remove_blank(tl);

        stmt_handle = stmt_t::handle_t(std::make_shared<T>(ctx, id));
    } else {
        // reset '(' before other tokens
        tl.data().push_front(token);
        stmt_handle = parse_parameter_list<T>(ctx, token, tl, ")", ctx, id);
    }

    if (stmt_handle && !tl.empty() && tl.begin()->type() == tkncl_t::IDENTIFIER
        && tl.begin()->identifier() == "as") {
        --tl;
        remove_blank(tl);

        syntax_error_if(tl.empty() || tl.begin()->type() != tkncl_t::IDENTIFIER,
            token.expression(), token.position());

        const auto ret_type = tl.begin()->identifier();

        stmt_function_t* ptr
            = dynamic_cast<stmt_function_t*>(stmt_handle.get());

        syntax_error_if(ptr == nullptr, token.expression(), token.position());

        --tl;
        remove_blank(tl);

        size_t array_size = 0;

        if (tl.size() > 1) {
            syntax_error_if(tl.begin()->type() != tkncl_t::SUBEXP_BEGIN,
                token.expression(), token.position());

            --tl;
            remove_blank(tl);

            syntax_error_if(
                tl.size() < 2 || tl.begin()->type() != tkncl_t::INTEGRAL,
                token.expression(), token.position());

            try {
                const auto val = std::stol(tl.begin()->identifier());
                array_size = val < 0 ? 0 : val;
            } catch (...) {
            }

            --tl;
            remove_blank(tl);

            syntax_error_if(array_size < 1 || tl.size() < 1
                    || tl.begin()->type() != tkncl_t::SUBEXP_END,
                token.expression(), token.position());

            --tl;
            remove_blank(tl);
        }

        ptr->define_ret_type(ret_type, ctx, array_size);
    }

    // Register visibility and static flag when inside a class body
    if (!ctx.compiling_class_name.empty()) {
        ctx.class_member_visibility[id] = ctx.compiling_class_member_is_public;

        if (ctx.compiling_class_member_is_static) {
            ctx.class_static_methods.insert(id);
            ctx.compiling_class_member_is_static = false;
        }
    }

    return stmt_handle;
}

template stmt_t::handle_t statement_parser_t::parse_procedure<stmt_sub_t>(
    prog_ctx_t&, token_t, token_list_t&);
template stmt_t::handle_t statement_parser_t::parse_procedure<stmt_function_t>(
    prog_ctx_t&, token_t, token_list_t&);


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::parse_stmt(
    prog_ctx_t& ctx, nu::token_list_t& tl)
{
    remove_blank(tl);

    if (tl.empty()) {
        return stmt_t::handle_t(std::make_shared<stmt_empty_t>(ctx));
    }

    token_t token(*tl.begin());

    if (token.type() == tkncl_t::NEWLINE) {
        return std::make_shared<stmt_empty_t>(ctx);
    }

    const std::string& identifier = token.identifier();

    if (!ctx.compiling_struct_name.empty()) {
        if (!((token.identifier() == "end" || token.identifier() == "rem")
                && token.type() == tkncl_t::IDENTIFIER)) {
            return parse_struct_element(ctx, token, tl);
        }
    }

    // Inside a class body (but not inside a method body): redirect to class
    // member parsing.  procedure_metadata.is_building() is true once "Sub" or
    // "Function" inside the class has been opened, so we stop redirecting for
    // the duration of that method body.
    if (!ctx.compiling_class_name.empty()
        && !ctx.procedure_metadata.is_building()) {
        if (!((identifier == "end" || identifier == "rem")
                && token.type() == tkncl_t::IDENTIFIER)) {
            return parse_class_member(ctx, token, tl);
        }
    }

    syntax_error_if((token.type() != tkncl_t::IDENTIFIER)
            && (token.type() == tkncl_t::OPERATOR
                && (token.identifier() != NU_BASIC_OP_INC
                    && (token.identifier() != NU_BASIC_OP_DEC))),
        token.expression(), token.position());

    if (identifier == NU_BASIC_OP_INC || identifier == NU_BASIC_OP_DEC) {
        return parse_expr(ctx, token, tl);
    }

#ifndef TINY_NUBASIC_VER

    if (identifier == "textout") {
        return parse_generic_instruction<stmt_textout_t>(ctx, token, tl, ctx);
    }

    if (identifier == "plotimage") {
        return parse_generic_instruction<stmt_plotimage_t>(ctx, token, tl, ctx);
    }

    if (identifier == "line") {
        return parse_generic_instruction<stmt_line_t>(ctx, token, tl, ctx);
    }

    if (identifier == "setpixel") {
        return parse_generic_instruction<stmt_set_pixel_t>(ctx, token, tl, ctx);
    }

    if (identifier == "rect") {
        return parse_generic_instruction<stmt_rect_t>(ctx, token, tl, ctx);
    }

    if (identifier == "fillrect") {
        return parse_generic_instruction<stmt_fillrect_t>(ctx, token, tl, ctx);
    }

    if (identifier == "ellipse") {
        return parse_generic_instruction<stmt_ellipse_t>(ctx, token, tl, ctx);
    }

    if (identifier == "fillellipse") {
        return parse_generic_instruction<stmt_fillellipse_t>(
            ctx, token, tl, ctx);
    }

#endif // TINY_NUBASIC_VER

    if (identifier == "print" || identifier == "write" || identifier == "print#"
        || identifier == "write#") {
        return parse_print(ctx, token, tl);
    }

    if (identifier == "data") {
        return parse_data(ctx, token, tl);
    }

    if (identifier == "locate") {
        return parse_locate(ctx, token, tl);
    }

    if (identifier == "delay" || identifier == "sleep") {
        return parse_delay(ctx, token, tl);
    }

    if (identifier == "mdelay") {
        return parse_mdelay(ctx, token, tl);
    }

    if (identifier == "using") {
        return parse_using(ctx, token, tl);
    }

    if (identifier == "include") {
        return parse_include(ctx, token, tl);
    }

    if (identifier == "syntax") {
        return parse_syntax(ctx, token, tl);
    }

    if (identifier == "input") {
        return parse_input(ctx, token, tl);
    }

    if (identifier == "input#") {
        return parse_input_file(ctx, token, tl);
    }

    if (identifier == "read#") {
        return parse_read_file(ctx, token, tl);
    }

    if (identifier == "read") {
        return parse_read(ctx, token, tl);
    }

    if (identifier == "on") {
        return parse_on_goto(ctx, token, tl);
    }

    if (identifier == "go" || identifier == "goto" || identifier == "gosub") {
        return parse_goto_gosub(ctx, token, tl);
    }

    if (identifier == "rem") {
        tl.clear();
        return stmt_t::handle_t(std::make_shared<stmt_empty_t>(ctx));
    }

    if (identifier == "end") {
        return parse_end(ctx, token, tl);
    }

    if (identifier == "stop") {
        return parse_stop(ctx, token, tl);
    }

    if (identifier == "wend") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_wend_t>(ctx));
    }

    if (identifier == "do") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_do_t>(ctx));
    }

    if (identifier == "else") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_else_t>(ctx));
    }

    if (identifier == "return") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_return_t>(ctx));
    }

    if (identifier == "if") {
        return parse_if_then_else(ctx, token, tl);
    }

    if (identifier == "elif" || identifier == "elseif") {
        return parse_elif_stmt(ctx, token, tl);
    }

    if (identifier == "select") {
        return parse_select_case(ctx, token, tl);
    }

    if (identifier == "case") {
        return parse_case_stmt(ctx, token, tl);
    }

    if (identifier == "for") {
        return parse_for_to_step(ctx, token, tl);
    }

    if (identifier == "next") {
        return parse_next(ctx, token, tl);
    }

    if (identifier == "exit") {
        return parse_exit(ctx, token, tl);
    }

    if (identifier == "beep") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_beep_t>(ctx));
    }

    if (identifier == "cls") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_cls_t>(ctx));
    }

    if (identifier == "screen") {
        --tl;
        remove_blank(tl);
        return parse_arg_list<stmt_screen_t, 0>(
            ctx, token, tl,
            [](const token_t& t) {
                return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
            },
            ctx);
    }

    if (identifier == "screenlock") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_screenlock_t>(ctx));
    }

    if (identifier == "screenunlock") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_screenunlock_t>(ctx));
    }

    if (identifier == "refresh") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_refresh_t>(ctx));
    }

    if (identifier == "randomize") {
        --tl;
        return stmt_t::handle_t(std::make_shared<stmt_randomize_t>(ctx));
    }

    if (identifier == "const") {
        return parse_const(ctx, token, tl);
    }

    if (identifier == "dim") {
        return parse_parameter_list<stmt_dim_t>(ctx, token, tl, ":", ctx);
    }

    if (identifier == "redim") {
        return parse_parameter_list<stmt_redim_t>(ctx, token, tl, ":", ctx);
    }

    if (identifier == "open") {
        return parse_open(ctx, token, tl);
    }

    if (identifier == "close") {
        return parse_close(ctx, token, tl);
    }

    if (identifier == "while") {
        return parse_while(ctx, token, tl);
    }

    if (identifier == "loop") {
        return parse_loop_while(ctx, token, tl);
    }

    if (identifier == "shell") {
        return parse_generic_instruction<stmt_shell_t>(ctx, token, tl, ctx);
    }

    if (identifier == "chdir") {
        return parse_generic_instruction<stmt_chdir_t>(ctx, token, tl, ctx);
    }

    if (identifier == "fopen") {
        return parse_generic_instruction<stmt_fopen_t>(ctx, token, tl, ctx);
    }

    if (identifier == "seek") {
        return parse_generic_instruction<stmt_fseek_t>(ctx, token, tl, ctx);
    }

    if (identifier == "flush") {
        return parse_generic_instruction<stmt_flush_t>(ctx, token, tl, ctx);
    }

    if (identifier == "call") {
        // Call ProcName(arg1, arg2, ...) or Call ProcName arg1, arg2, ...
        --tl; // consume "call"
        remove_blank(tl);
        syntax_error_if(tl.empty(), token.expression(), token.position());

        token = *tl.begin();
        syntax_error_if(token.type() != tkncl_t::IDENTIFIER, token.expression(),
            token.position());

        std::string proc_name = token.identifier();
        --tl; // consume procedure name
        remove_blank(tl);

        consume_qualified_callable_name(
            tl, proc_name, token.expression(), token.position());

        if (tl.empty())
            return std::make_shared<stmt_call_t>(proc_name, ctx);

        token = *tl.begin();

        if (token.type() == tkncl_t::SUBEXP_BEGIN) {
            --tl; // consume "("
            remove_blank(tl);

            // Empty arg list: Call ProcName()
            if (!tl.empty() && tl.begin()->type() == tkncl_t::SUBEXP_END) {
                --tl; // consume ")"
                return std::make_shared<stmt_call_t>(proc_name, ctx);
            }

            // Strip the matching closing ")" from the end of the token list
            if (!tl.empty() && tl.rbegin()->type() == tkncl_t::SUBEXP_END)
                tl--; // postfix --: removes last token

            token = *tl.begin();
        }

        return parse_arg_list<stmt_call_t, 0>(
            ctx, token, tl,
            [](const token_t& t) {
                return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
            },
            proc_name, ctx);
    }

    if (identifier == "sub") {
        return parse_procedure<stmt_sub_t>(ctx, token, tl);
    }

    if (identifier == "struct") {
        return parse_struct(ctx, token, tl);
    }

    if (identifier == "class") {
        return parse_class(ctx, token, tl);
    }

    if (identifier == "function") {
        return parse_procedure<stmt_function_t>(ctx, token, tl);
    } else {
        if (identifier == "let") {
            --tl;
        } else {
            auto h = parse_label(ctx, token, tl);

            if (h)
                return h;
        }

        return parse_let(ctx, tl);
    }

    throw exception_t("Syntax error");
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::compile_line(
    nu::token_list_t& tl, prog_ctx_t& ctx)
{
    _parsing_line = ctx.compiletime_pc.get_line();
    _parsing_stmt = ctx.compiletime_pc.get_stmt_pos();

    if (ctx.tracing_on) {
        std::stringstream log;

        for (auto t : tl) {
            log << token_t::description_of(t.type()) << "[" << t.identifier()
                << "] ";
        }

        fprintf(ctx.get_stdout_ptr(), "%s\n", log.str().c_str());
    }

    remove_blank(tl);

    if (!tl.empty()) {
        token_t token(*tl.begin());

        // syntax check
        switch (token.type()) {
        case tkncl_t::IDENTIFIER:
            break;

        case tkncl_t::OPERATOR:
            syntax_error_if(token.identifier() != NU_BASIC_OP_INC
                    && token.identifier() != NU_BASIC_OP_DEC,
                token.expression(), token.position());
            break;

        default:
            syntax_error(token.expression(), token.position());
        }

        return parse_block(ctx, tl);
    }

    return std::make_shared<stmt_empty_t>(ctx);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t statement_parser_t::compile_line(
    nu::expr_tknzr_t& st, prog_ctx_t& ctx)
{
    nu::token_list_t tl;
    st.get_tknlst(tl);
    return compile_line(tl, ctx);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
