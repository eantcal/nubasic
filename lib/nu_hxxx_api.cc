//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_hxxx_api.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variant_t hash_set(
    rt_prog_ctx_t& ctx,
    const std::string& fname,
    const func_args_t& args)
{
    (void)fname;

    const auto n_of_p = args.size();

    if (n_of_p != 3 && n_of_p != 2) {
        throw exception_t(
            std::string("Syntax error: expected 2 or 3 parameters: hash, key[, value=false]"));
    }

    const auto& hash_name = args[0]->eval(ctx);
    const auto& hash_key = args[1]->eval(ctx);
    
    if (n_of_p == 2) {
        ctx.hash_tbls[hash_name.to_str()][hash_key.to_str()] = false;
    }
    else {
        const auto& hash_value = args[2]->eval(ctx);
        ctx.hash_tbls[hash_name.to_str()][hash_key.to_str()] = hash_value;
        return hash_value;
    }
    
    return false;
}


/* -------------------------------------------------------------------------- */

variant_t hash_get(
    rt_prog_ctx_t& ctx,
    const std::string& fname,
    const func_args_t& args)
{
    (void)fname;

    if (args.size() != 2) {
        throw exception_t(
            std::string("Syntax error: expected 2 parameters: hash, key"));
    }

    const auto& hash_name = args[0]->eval(ctx);
    const auto& hash_key = args[1]->eval(ctx);

    auto tbl_it = ctx.hash_tbls.find(hash_name.to_str());

    if (tbl_it == ctx.hash_tbls.end()) {
        return variant_t(false);
    }

    auto it = tbl_it->second.find(hash_key.to_str());

    if (it == tbl_it->second.end()) {
        return variant_t(false);
    }

    return it->second;
}


/* -------------------------------------------------------------------------- */

variant_t hash_chk(
    rt_prog_ctx_t& ctx,
    const std::string& fname,
    const func_args_t& args)
{
    (void)fname;

    const auto n_of_p = args.size();

    if (n_of_p > 2 || n_of_p < 1) {
        throw exception_t(
            std::string("Syntax error: expected 1 or 2 parameters: hash[, key]"));
    }

    const auto& hash_name = args[0]->eval(ctx);
    
    auto tbl_it = ctx.hash_tbls.find(hash_name.to_str());

    if (tbl_it == ctx.hash_tbls.end()) {
        return false;
    }

    if (n_of_p == 1) {
        return true;
    }

    const auto& hash_key = args[1]->eval(ctx);

    auto it = tbl_it->second.find(hash_key.to_str());

    if (it == tbl_it->second.end()) {
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

variant_t hash_cnt(
    rt_prog_ctx_t& ctx,
    const std::string& fname,
    const func_args_t& args)
{
    (void)fname;

    const auto n_of_p = args.size();

    if (n_of_p != 1) {
        throw exception_t(
            std::string("Syntax error: expected parameter: hash"));
    }

    const auto& hash_name = args[0]->eval(ctx);

    auto tbl_it = ctx.hash_tbls.find(hash_name.to_str());

    if (tbl_it == ctx.hash_tbls.end()) {
        return variant_t(0);
    }
    
    const auto & tbl = tbl_it->second;

    return variant_t(long64_t(tbl.size()));
}


/* -------------------------------------------------------------------------- */

variant_t hash_del(
    rt_prog_ctx_t& ctx,
    const std::string& fname,
    const func_args_t& args)
{
    (void)fname;

    const auto n_of_p = args.size();
    
    if (args.size() != 1 && args.size() != 2) {
        throw exception_t(
            std::string("Syntax error: expected 1 or 2 parameters: hash[, key]"));
    }

    const auto& hash_name = args[0]->eval(ctx);

    const auto sHashName = hash_name.to_str();
    auto & hash_tbl = ctx.hash_tbls[sHashName];

    if (n_of_p > 1) {
        const auto& hash_key = args[1]->eval(ctx);

        hash_tbl.erase(hash_key.to_str());

        if (hash_tbl.empty()) {
            ctx.hash_tbls.erase(sHashName);
        }
    }
    else {
        ctx.hash_tbls.erase(sHashName);
    }

    return true;
}


/* -------------------------------------------------------------------------- */

}

