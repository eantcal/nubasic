//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_print.h"
#include "nu_os_console.h"
#include "nu_rt_prog_ctx.h"
#include "nu_unicode.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_print_t::run(rt_prog_ctx_t& ctx)
{
    bool hide_cursor = true;
    FILE* sout = ctx.get_stdout_ptr();


    struct _hide_cursor_guard_t {
        bool _hide;

        _hide_cursor_guard_t(bool hide) noexcept : 
            _hide(hide)
        {
            if (_hide) {
                _os_cursor_visible(false);
            }
        }

        ~_hide_cursor_guard_t() noexcept {
            if (_hide) {
                _os_cursor_visible(true);
            }
        }
    };

    if (_fd >= 0) {
        sout = _fd == 0 ? stdout : ctx.file_tbl.resolve_fd(_fd);
        hide_cursor = false;
    }
    
    //if (sout == stdout && _unicode) {
    //   sout = stderr;
    //}

    if (sout == nullptr) {
        ctx.set_errno(EBADF);
        ctx.go_to_next();
        return;
    }

    int ret = 0;

    if (_args.empty()) {
        _hide_cursor_guard_t guard(hide_cursor);
        if (_unicode) {
           auto data = unicode_unescape(_data);
           if (sout == stdout) {
              data.push_back('\n');
              _os_u16write(data);
           }
           else {
              ret = fwprintf(sout, L"%ls\n", data.c_str());
           }
        }
        else {
           ret = ::fprintf(sout, "%s\n", _data.c_str());
        }
        ::fflush(sout);
    }

    else {
        _hide_cursor_guard_t guard(hide_cursor);

        for (auto arg : _args) {
            std::string separator;
            variant_t val;

            if (arg.first == nullptr) {
                val = variant_t("");
                separator = "";
            } 
            else {
                val = arg.first->eval(ctx);

                switch (arg.second) {
                case ',':
                    separator = "\t";
                    break;

                case ';':
                    separator = ""; // GW-BASIC syntax
                    break;

                case ' ':
                    separator = "";
                    break;

                default:
                    separator = "\n";
                    break;
                }
            }

            std::stringstream ss;

            if (sout != stdout && variable_t::is_number(val.get_type())) {
                // Implements write# semantic (see implementation of input#)
                switch (val.get_type()) {
                case variant_t::type_t::DOUBLE:
                    ret = ::fprintf(
                        sout, "%lf%s", val.to_double(), separator.c_str());
                    ::fflush(sout);
                    break;

                case variant_t::type_t::BOOLEAN:
                    ret = ::fprintf(sout, "%s%s",
                        val.to_bool() ? "true" : "false", separator.c_str());
                    ::fflush(sout);
                    break;

                case variant_t::type_t::INTEGER:
                    ret = ::fprintf(
                        sout, "%lli%s", val.to_int(), separator.c_str());
                    ::fflush(sout);
                    break;

                case variant_t::type_t::STRING:
                case variant_t::type_t::BYTEVECTOR:
                case variant_t::type_t::UNDEFINED:
                default:
                    assert(0);
                    break;
                }
            }

            else
                switch (val.get_type()) {
                case variant_t::type_t::BOOLEAN:
                    ret = ::fprintf(sout, "%s%s",
                        val.to_bool() ? "true" : "false", separator.c_str());
                    ::fflush(sout);
                    break;

                case variant_t::type_t::DOUBLE:
                    ss << val.to_double();

                    ret = ::fprintf(
                        sout, "%s%s", ss.str().c_str(), separator.c_str());
                    ::fflush(sout);
                    break;

                case variant_t::type_t::INTEGER:
                    ss << val.to_int();

                    ret = ::fprintf(
                        sout, "%s%s", ss.str().c_str(), separator.c_str());
                    ::fflush(sout);
                    break;

                case variant_t::type_t::STRING:
                case variant_t::type_t::BYTEVECTOR:
                case variant_t::type_t::UNDEFINED:
                default:
                    if (val.get_type() != variant_t::type_t::UNDEFINED) {
                       if (_unicode) {
                          auto s = val.to_str();
                          s += separator;
                          auto data = unicode_unescape(s);

                          if (sout == stdout) {
                             _os_u16write(data);
                          }
                          else {
                             ret = fwprintf(sout, L"%ls%s", 
                                data.c_str(), 
                                separator.c_str());
                          }
                       }
                       else {
                          ret = ::fprintf(sout, "%s%s", val.to_str().c_str(),
                             separator.c_str());
                       }
                        ::fflush(sout);
                    }

                    break;
                } // switch
        } // for

    } // else

    ctx.go_to_next();
    ctx.set_errno(ret != 0 ? errno : 0);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
