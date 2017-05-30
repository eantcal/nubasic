//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_expr_syntax_tree.h"
#include "nu_basic_defs.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

expr_syntax_tree_t::expr_syntax_tree_t()
{
    change_operator_precedence(op_preced_t::STRUCT_ACCESS);
}


/* -------------------------------------------------------------------------- */

void expr_syntax_tree_t::change_operator_precedence(op_preced_t precedence)
{
    processing_ops.clear();
    ignoring_ops.clear();

    switch (precedence) {
    case op_preced_t::STRUCT_ACCESS:
        processing_ops.insert(".");

        ignoring_ops.insert("*");
        ignoring_ops.insert("/");
        ignoring_ops.insert("^");
        ignoring_ops.insert("\\");
        ignoring_ops.insert("mod");
        ignoring_ops.insert("div");

        ignoring_ops.insert("+");
        ignoring_ops.insert("-");

        ignoring_ops.insert("=");
        ignoring_ops.insert("<");
        ignoring_ops.insert(">");
        ignoring_ops.insert(">=");
        ignoring_ops.insert("<>");
        ignoring_ops.insert("<=");

        ignoring_ops.insert("and");
        ignoring_ops.insert("or");
        ignoring_ops.insert("xor");

        ignoring_ops.insert("band");
        ignoring_ops.insert("bor");
        ignoring_ops.insert("bxor");
        ignoring_ops.insert("bshl");
        ignoring_ops.insert("bshr");
        break;

    case op_preced_t::RELATIONAL_OP:
        ignoring_ops.insert(".");

        ignoring_ops.insert("*");
        ignoring_ops.insert("/");
        ignoring_ops.insert("^");
        ignoring_ops.insert("\\");
        ignoring_ops.insert("mod");
        ignoring_ops.insert("div");

        ignoring_ops.insert("+");
        ignoring_ops.insert("-");

        processing_ops.insert("=");
        processing_ops.insert("<>");
        processing_ops.insert("<");
        processing_ops.insert(">");
        processing_ops.insert(">=");
        processing_ops.insert("<=");

        ignoring_ops.insert("and");
        ignoring_ops.insert("or");
        ignoring_ops.insert("xor");

        ignoring_ops.insert("band");
        ignoring_ops.insert("bor");
        ignoring_ops.insert("bxor");
        ignoring_ops.insert("bshl");
        ignoring_ops.insert("bshr");
        break;

    case op_preced_t::RELATIONAL_ANDOR_OP:
        ignoring_ops.insert(".");

        ignoring_ops.insert("*");
        ignoring_ops.insert("/");
        ignoring_ops.insert("^");
        ignoring_ops.insert("\\");
        ignoring_ops.insert("mod");
        ignoring_ops.insert("div");

        ignoring_ops.insert("+");
        ignoring_ops.insert("-");

        ignoring_ops.insert("=");
        ignoring_ops.insert("<>");

        ignoring_ops.insert("<");
        ignoring_ops.insert(">");
        ignoring_ops.insert(">=");
        ignoring_ops.insert("<=");

        processing_ops.insert("and");
        processing_ops.insert("or");
        processing_ops.insert("xor");

        ignoring_ops.insert("band");
        ignoring_ops.insert("bor");
        ignoring_ops.insert("bxor");
        ignoring_ops.insert("bshl");
        ignoring_ops.insert("bshr");
        break;


    case op_preced_t::MATH_MUL_DIV:
        ignoring_ops.insert(".");

        processing_ops.insert("*");
        processing_ops.insert("/");
        processing_ops.insert("^");
        processing_ops.insert("\\");
        processing_ops.insert("mod");
        processing_ops.insert("div");

        ignoring_ops.insert("+");
        ignoring_ops.insert("-");

        ignoring_ops.insert("=");
        ignoring_ops.insert(">=");

        ignoring_ops.insert("<");
        ignoring_ops.insert(">");
        ignoring_ops.insert("<>");
        ignoring_ops.insert("<=");

        ignoring_ops.insert("and");
        ignoring_ops.insert("or");
        ignoring_ops.insert("xor");

        ignoring_ops.insert("band");
        ignoring_ops.insert("bor");
        ignoring_ops.insert("bxor");
        ignoring_ops.insert("bshl");
        ignoring_ops.insert("bshr");
        break;

    case op_preced_t::MATH_SUM:
        ignoring_ops.insert(".");

        ignoring_ops.insert("*");
        ignoring_ops.insert("/");
        ignoring_ops.insert("^");
        ignoring_ops.insert("\\");
        ignoring_ops.insert("mod");
        ignoring_ops.insert("div");

        processing_ops.insert("+");
        processing_ops.insert("-");

        ignoring_ops.insert("=");
        ignoring_ops.insert("<>");

        ignoring_ops.insert("<");
        ignoring_ops.insert(">");
        ignoring_ops.insert(">=");
        ignoring_ops.insert("<=");

        ignoring_ops.insert("and");
        ignoring_ops.insert("or");
        ignoring_ops.insert("xor");

        ignoring_ops.insert("band");
        ignoring_ops.insert("bor");
        ignoring_ops.insert("bxor");
        ignoring_ops.insert("bshl");
        ignoring_ops.insert("bshr");
        break;


    case op_preced_t::BITWISE_OP:
        ignoring_ops.insert(".");

        ignoring_ops.insert("*");
        ignoring_ops.insert("/");
        ignoring_ops.insert("^");
        ignoring_ops.insert("\\");
        ignoring_ops.insert("mod");
        ignoring_ops.insert("div");

        ignoring_ops.insert("+");
        ignoring_ops.insert("-");

        ignoring_ops.insert("=");
        ignoring_ops.insert("<>");

        ignoring_ops.insert("<");
        ignoring_ops.insert(">");
        ignoring_ops.insert(">=");
        ignoring_ops.insert("<=");

        ignoring_ops.insert("and");
        ignoring_ops.insert("or");
        ignoring_ops.insert("xor");

        processing_ops.insert("band");
        processing_ops.insert("bor");
        processing_ops.insert("bxor");
        processing_ops.insert("bshl");
        processing_ops.insert("bshr");
        break;

    default:
        assert(0);
        break;
    }
}


/* -------------------------------------------------------------------------- */

void expr_syntax_tree_t::swap_lr_markers()
{
    explicit_left_marker.swap(implicit_left_marker);
    explicit_right_marker.swap(implicit_right_marker);
    // processing_ops.swap(ignoring_ops);
}


/* -------------------------------------------------------------------------- */

token_list_t expr_syntax_tree_t::remove_blank(token_list_t tl)
{
    auto is_blank = [](
        tkncl_t cl) { return cl == tkncl_t::BLANK || cl == tkncl_t::NEWLINE; };

    tkncl_t old_token_cl(tkncl_t::UNDEFINED);
    std::string old_token_id;

    auto i = tl.begin();
    token_list_t ret_tl;

    while (i != tl.end()) {
        if (!is_blank(i->type())) {
            if (i->type() == old_token_cl && i->identifier() == old_token_id) {
                if (i->type() != tkncl_t::SUBEXP_BEGIN
                    && i->type() != tkncl_t::SUBEXP_END) {
                    syntax_error(i->expression(), i->position());
                }
            }

            if (i->type() == tkncl_t::OPERATOR) {
                if (old_token_cl == tkncl_t::SUBEXP_BEGIN
                    && (i->identifier() != "+" && i->identifier() != "-"
                           && i->identifier() != NU_BASIC_OP_INC
                           && i->identifier() != NU_BASIC_OP_DEC)) {
                    syntax_error(i->expression(), i->position());
                }
            }

            if (i->type() == tkncl_t::SUBEXP_END
                && old_token_cl == tkncl_t::OPERATOR)
                syntax_error(i->expression(), i->position());

            ret_tl += *i;
            old_token_cl = i->type();
            old_token_id = i->identifier();
        }

        ++i;
    }

    return ret_tl;
}


/* -------------------------------------------------------------------------- */

void expr_syntax_tree_t::rework_subexp(
    token_list_t& tl, token_list_t::data_t::iterator i)
{
    if (i->type() != tkncl_t::SUBEXP_BEGIN
        || i->identifier() == implicit_left_marker) // (
    {
        syntax_error(i->expression(), i->position());
    }

    token_list_t::tkp_t begin
        = { explicit_left_marker, tkncl_t::SUBEXP_BEGIN }; // (
    token_list_t::tkp_t end
        = { explicit_right_marker, tkncl_t::SUBEXP_END }; // )

    auto pos = i->position();
    auto ptr = i->expression_ptr();

    token_list_t subexp = tl.sublist(begin, end, tl.iterator_to_pos(i));

    subexp--; // erase first and last token
    --subexp;

    subexp = rework_token_list(subexp); // rework sub-expression

    // mark as already precessed
    subexp.data().push_front(
        token_t(implicit_left_marker, tkncl_t::SUBEXP_BEGIN, pos, ptr));
    subexp.data().push_back(
        token_t(implicit_right_marker, tkncl_t::SUBEXP_END, pos, ptr));

    // replace old-subexp with reworked one
    tl = rework_token_list(
        tl.replace_sublist(begin, end, tl.iterator_to_pos(i), subexp));
}


/* -------------------------------------------------------------------------- */

void expr_syntax_tree_t::rework_operator(
    token_list_t& tl, token_list_t::data_t::iterator i)
{

    if (i == tl.end() || tl.size() < 3 || (i - 1) == tl.end()
        || (i + 1) == tl.end()) {
        const size_t pos = i != tl.end() ? i->position() : 0;
        const std::string expr
            = i != tl.end() ? i->expression() : "Internal error 14";

        syntax_error(expr, pos);
    }

    size_t pos = i->position();
    auto expr_ptr = i->expression_ptr();

    std::set<std::string> ops = processing_ops;

    if (ops.find(i->identifier()) != ops.end()) {
        if ((i + 1) == tl.end() || i == tl.begin()
            || (i - 1)->type() == tkncl_t::OPERATOR) {
            syntax_error(*expr_ptr, pos);
        }

        token_list_t::tkp_t begin
            = { implicit_left_marker, tkncl_t::SUBEXP_BEGIN };

        token_list_t::tkp_t end
            = { implicit_right_marker, tkncl_t::SUBEXP_END };

        bool left_sub = (i - 1)->type() == tkncl_t::SUBEXP_END;
        bool left_explicit = (i - 1)->identifier() == explicit_right_marker;

        if (left_explicit) {
            begin.first = explicit_left_marker;
            end.first = explicit_right_marker;
        }

        bool right_identifier = (i + 1)->type() == tkncl_t::IDENTIFIER;
        bool right_sub = (i + 1)->type() == tkncl_t::SUBEXP_BEGIN;
        bool right_explicit = (i + 1)->identifier() == explicit_left_marker;

        if (right_identifier && (i + 2) != tl.end()) {
            right_sub = (i + 2)->type() == tkncl_t::SUBEXP_BEGIN;
            right_explicit = (i + 2)->identifier() == explicit_left_marker;
        }

        if (right_explicit) {
            begin.first = explicit_left_marker;
            end.first = explicit_right_marker;
        }

        size_t left_size = 1;
        size_t right_size = 1;

        auto pos = i->position();
        auto ptr = i->expression_ptr();

        token_list_t left_list;

        if (left_sub) {
            auto& end_it = i;
            auto begin_it = tl.reverse_to_forward(
                tl.skip_left(tl.forward_to_reverse(i), begin, end));
            left_size = end_it - begin_it;

            left_list
                = tl.sublist(tl.iterator_to_pos(i) - left_size, left_size);

            left_list = rework_token_list(left_list);
        } else {
            left_list += *(i - 1);
        }

        token_list_t right_list;

        if (right_sub) {
            auto begin_it = right_identifier ? i + 1 : i;
            auto end_it = tl.skip_right(begin_it, begin, end);
            right_size = end_it - begin_it - 1;

            if (right_identifier)
                ++right_size;

            right_list = tl.sublist(tl.iterator_to_pos(i) + 1, right_size);

            right_list = rework_token_list(right_list);
        } else {
            right_list += *(i + 1);
        }

        token_list_t subexp = left_list + *i + right_list;

        subexp.data().push_front(
            token_t(implicit_left_marker, tkncl_t::SUBEXP_BEGIN, pos, ptr));

        subexp.data().push_back(
            token_t(implicit_right_marker, tkncl_t::SUBEXP_END, pos, ptr));

        // replace old-subexp with reworked one
        token_list_t ret_tl
            = tl.replace_sublist(tl.iterator_to_pos(i - left_size),
                tl.iterator_to_pos(i + right_size), subexp);

        tl = ret_tl;
    } else {
        syntax_error(*expr_ptr, pos);
    }
}


/* -------------------------------------------------------------------------- */

void expr_syntax_tree_t::rework_minus_operator(
    token_list_t& tl, token_list_t::data_t::iterator i)
{
    assert(!(i == tl.end() || (i + 1) == tl.end()));

    size_t pos = i->position();
    auto expr_ptr = i->expression_ptr();

    std::set<std::string> ops = { "-" };

    if (ops.find(i->identifier()) != ops.end()) {
        if ((i + 1) == tl.end())
            syntax_error(*expr_ptr, pos);

        token_list_t::tkp_t begin
            = { implicit_left_marker, tkncl_t::SUBEXP_BEGIN };

        token_list_t::tkp_t end
            = { implicit_right_marker, tkncl_t::SUBEXP_END };

        bool right_identifier = (i + 1)->type() == tkncl_t::IDENTIFIER;

        bool right_sub = (i + 1)->type() == tkncl_t::SUBEXP_BEGIN;

        bool right_explicit = (i + 1)->identifier() == explicit_left_marker;

        if (right_identifier) {
            if ((i + 2) != tl.end()) {
                right_sub = (i + 2)->type() == tkncl_t::SUBEXP_BEGIN;
                right_explicit = (i + 2)->identifier() == explicit_left_marker;
            }
        }

        if (right_explicit) {
            begin.first = explicit_left_marker;
            end.first = explicit_right_marker;
        }

        size_t right_size = 1;

        auto pos = i->position();
        auto ptr = i->expression_ptr();

        token_list_t right_list;

        if (right_sub) {
            auto begin_it = right_identifier ? i + 1 : i;
            auto end_it = tl.skip_right(begin_it, begin, end);
            right_size = end_it - begin_it - 1;

            if (right_identifier)
                ++right_size;

            right_list = tl.sublist(tl.iterator_to_pos(i) + 1, right_size);

            right_list = rework_token_list(right_list);
        } else
            right_list += *(i + 1);

        token_list_t subexp;
        subexp += *i;
        subexp += right_list;

        subexp.data().push_front(token_t(
            "0", tkncl_t::INTEGRAL, i->position(), i->expression_ptr()));

        subexp.data().push_front(
            token_t(implicit_left_marker, tkncl_t::SUBEXP_BEGIN, pos, ptr));

        subexp.data().push_back(
            token_t(implicit_right_marker, tkncl_t::SUBEXP_END, pos, ptr));

        // replace old-subexp with reworked one
        token_list_t ret_tl = tl.replace_sublist(
            tl.iterator_to_pos(i), tl.iterator_to_pos(i + right_size), subexp);

        tl = ret_tl;
    }
}


/* -------------------------------------------------------------------------- */

void expr_syntax_tree_t::rework_identifier(
    token_list_t& tl, token_list_t::data_t::iterator i)
{
    size_t pos = i->position();
    auto expr_ptr = i->expression_ptr();

    if ((i + 1) == tl.end())
        return;

    token_list_t::tkp_t begin = { implicit_left_marker, tkncl_t::SUBEXP_BEGIN };

    token_list_t::tkp_t end = { implicit_right_marker, tkncl_t::SUBEXP_END };

    bool right_sub = (i + 1)->type() == tkncl_t::SUBEXP_BEGIN;
    bool right_explicit = (i + 1)->identifier() == explicit_left_marker;

    if (right_explicit) {
        begin.first = explicit_left_marker;
        end.first = explicit_right_marker;
    }

    size_t right_size = 1;

    token_list_t right_list;

    if (right_sub) {
        auto begin_it = i;
        auto end_it = tl.skip_right(i, begin, end);
        right_size = end_it - begin_it - 1;

        right_list = tl.sublist(tl.iterator_to_pos(i) + 1, right_size);

        right_list = rework_token_list(right_list);

    } else {
        right_list += *(i + 1);
    }

    token_list_t subexp;
    subexp += *i;
    subexp += right_list;

    subexp.data().push_front(
        token_t(implicit_left_marker, tkncl_t::SUBEXP_BEGIN, pos, expr_ptr));
    subexp.data().push_back(
        token_t(implicit_right_marker, tkncl_t::SUBEXP_END, pos, expr_ptr));

    // replace old-subexp with reworked one
    token_list_t ret_tl = tl.replace_sublist(
        tl.iterator_to_pos(i), tl.iterator_to_pos(i + right_size), subexp);

    tl = ret_tl;
}


/* -------------------------------------------------------------------------- */

void expr_syntax_tree_t::skip_processed(
    token_list_t& tl, token_list_t::data_t::iterator& i)
{
    while (i != tl.end()) {
        if (i->type() == tkncl_t::IDENTIFIER
            && ((i + 1) == tl.end()
                   || ((i + 1)->type() == tkncl_t::OPERATOR
                          || (i + 1)->type() == tkncl_t::SUBEXP_END))) {
            ++i;
            continue;
        }

        if (i->type() == tkncl_t::SUBEXP_BEGIN
            && i->identifier() == implicit_left_marker) {
            token_list_t::tkp_t begin
                = { implicit_left_marker, tkncl_t::SUBEXP_BEGIN };

            token_list_t::tkp_t end
                = { implicit_right_marker, tkncl_t::SUBEXP_END };

            token_list_t subexp = tl.sublist(begin, end, tl.iterator_to_pos(i));

            i += subexp.size();

            if (i == tl.end())
                break;

            continue;
        }

        else if ((i->type() != tkncl_t::SUBEXP_BEGIN
                     && i->type() != tkncl_t::OPERATOR
                     && i->type() != tkncl_t::IDENTIFIER)
            || ignoring_ops.find(i->identifier()) != ignoring_ops.end()) {
            ++i;
        } else {
            break;
        }
    }
}


/* -------------------------------------------------------------------------- */

token_list_t expr_syntax_tree_t::rework_token_list(token_list_t tl)
{
    auto i = tl.begin();

    // If an expression begins with "+"
    // just ignore it
    if (i != tl.end()
        && (i->identifier() == "+" && i->type() == tkncl_t::OPERATOR)) {
        tl.data().erase(i);
        i = tl.begin();
    }

    // If an expression begins with "-"...
    if (i != tl.end()
        && (i->identifier() == "-" && i->type() == tkncl_t::OPERATOR)) {
        rework_minus_operator(tl, i);
        i = tl.begin();
    }

    // If an expression begins with increment or decrement unary operator...
    if (i != tl.end() && (i->type() == tkncl_t::OPERATOR
                             && (i->identifier() == NU_BASIC_OP_INC
                                    || i->identifier() == NU_BASIC_OP_DEC))) {
        rework_unary_operator(tl, i);
        i = tl.begin();
    }

    skip_processed(tl, i);

    // Do we have terminate?
    if (i == tl.end() || ((tl.begin()->type() == tkncl_t::SUBEXP_BEGIN
                             && i->identifier() == implicit_left_marker
                             && (tl.end() - 1)->type() == tkncl_t::SUBEXP_END
                             && i->identifier() == implicit_right_marker))) {
        return tl;
    }

    if (i->type() == tkncl_t::IDENTIFIER) {
        rework_identifier(tl, i);
        return rework_token_list(tl);
    }

    if (i->type() == tkncl_t::SUBEXP_BEGIN
        && i->identifier() == explicit_left_marker) {
        rework_subexp(tl, i);
        return rework_token_list(tl);
    }

    if (i->type() == tkncl_t::OPERATOR && i->identifier() != ",") {
        rework_operator(tl, i);
        return rework_token_list(tl);
    }

    return tl;
}


/* -------------------------------------------------------------------------- */

token_list_t expr_syntax_tree_t::operator()(token_list_t tl)
{
    tl = rework_token_list(remove_blank(tl));

    swap_lr_markers();
    change_operator_precedence(op_preced_t::STRUCT_ACCESS);
    tl = rework_token_list(tl);

    swap_lr_markers();
    change_operator_precedence(op_preced_t::MATH_MUL_DIV);
    tl = rework_token_list(tl);

    swap_lr_markers();
    change_operator_precedence(op_preced_t::MATH_SUM);
    tl = rework_token_list(tl);

    swap_lr_markers();
    change_operator_precedence(op_preced_t::RELATIONAL_OP);
    tl = rework_token_list(tl);

    swap_lr_markers();
    change_operator_precedence(op_preced_t::RELATIONAL_ANDOR_OP);
    tl = rework_token_list(tl);

    swap_lr_markers();
    change_operator_precedence(op_preced_t::BITWISE_OP);
    tl = rework_token_list(tl);

    swap_lr_markers();
    tl = rework_token_list(tl);

    return tl;
}


/* -------------------------------------------------------------------------- */

void expr_syntax_tree_t::rework_unary_operator(
    token_list_t& tl, token_list_t::data_t::iterator i)
{
    assert(!(i == tl.end() || (i + 1) == tl.end()));

    size_t pos = i->position();
    auto expr_ptr = i->expression_ptr();

    std::set<std::string> ops = { NU_BASIC_OP_INC, NU_BASIC_OP_DEC };

    if (ops.find(i->identifier()) != ops.end()) {
        if ((i + 1) == tl.end())
            syntax_error(*expr_ptr, pos);

        token_list_t::tkp_t begin
            = { implicit_left_marker, tkncl_t::SUBEXP_BEGIN };

        token_list_t::tkp_t end
            = { implicit_right_marker, tkncl_t::SUBEXP_END };

        bool right_identifier = (i + 1)->type() == tkncl_t::IDENTIFIER;

        bool right_sub = (i + 1)->type() == tkncl_t::SUBEXP_BEGIN;

        bool right_explicit = (i + 1)->identifier() == explicit_left_marker;

        if (right_identifier && (i + 2) != tl.end()) {
            right_sub = (i + 2)->type() == tkncl_t::SUBEXP_BEGIN;
            right_explicit = (i + 2)->identifier() == explicit_left_marker;
        }

        if (right_explicit) {
            begin.first = explicit_left_marker;
            end.first = explicit_right_marker;
        }

        size_t right_size = 1;

        auto pos = i->position();
        auto ptr = i->expression_ptr();

        token_list_t right_list;

        if (right_sub) {
            auto begin_it = right_identifier ? i + 1 : i;
            auto end_it = tl.skip_right(begin_it, begin, end);
            right_size = end_it - begin_it - 1;

            if (right_identifier)
                ++right_size;

            right_list = tl.sublist(tl.iterator_to_pos(i) + 1, right_size);

            right_list = rework_token_list(right_list);
        } else {
            right_list += *(i + 1);
        }

        token_list_t subexp;
        subexp += *i;
        subexp += right_list;

        subexp.data().push_front(
            token_t(implicit_left_marker, tkncl_t::SUBEXP_BEGIN, pos, ptr));

        subexp.data().push_back(
            token_t(implicit_right_marker, tkncl_t::SUBEXP_END, pos, ptr));

        // replace old-subexp with reworked one
        token_list_t ret_tl = tl.replace_sublist(
            tl.iterator_to_pos(i), tl.iterator_to_pos(i + right_size), subexp);

        tl = ret_tl;
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
