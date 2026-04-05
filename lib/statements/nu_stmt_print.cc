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
#include <cstdarg>

#ifdef _WIN32
#include "nu_winconsole_api.h"
#endif


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

// Wrapper for fprintf that supports GDI console
static int _fprintf_console(FILE* stream, const char* format, ...)
{
#ifdef _WIN32
    if (stream == stdout && nu_winconsole_is_active()) {
        char buffer[4096];
        va_list args;
        va_start(args, format);
        int ret = vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        if (ret > 0) {
            nu_winconsole_write(buffer);
        }
        return ret;
    }
#endif

    va_list args;
    va_start(args, format);
    int ret = vfprintf(stream, format, args);
    va_end(args);
    return ret;
}


/* -------------------------------------------------------------------------- */

// Wrapper for fflush that supports GDI console
static void _fflush_console(FILE* stream)
{
#ifdef _WIN32
    if (stream == stdout && nu_winconsole_is_active()) {
        nu_winconsole_refresh();
        return;
    }
#endif

    fflush(stream);
}


/* -------------------------------------------------------------------------- */

void stmt_print_t::run(rt_prog_ctx_t& ctx)
{
    bool hide_cursor = true;
    FILE* sout = ctx.get_stdout_ptr();


    struct _hide_cursor_guard_t {
        bool _hide;

        _hide_cursor_guard_t(bool hide) noexcept
            : _hide(hide)
        {
            if (_hide) {
                _os_cursor_visible(false);
            }
        }

        ~_hide_cursor_guard_t() noexcept
        {
            if (_hide) {
                _os_cursor_visible(true);
            }
        }
    };

    if (_fd >= 0) {
        sout = _fd == 0 ? stdout : ctx.file_tbl.resolve_fd(_fd);
        hide_cursor = false;
    }

    // if (sout == stdout && _unicode) {
    //    sout = stderr;
    // }

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
                if (!_suppress_final_newline) {
                    data.push_back('\n');
                }
                _os_u16write(data);
            } else {
                if (_suppress_final_newline) {
                    ret = fwprintf(sout, L"%ls", (wchar_t*)data.c_str());
                } else {
                    ret = fwprintf(sout, L"%ls\n", (wchar_t*)data.c_str());
                }
            }
        } else {
            if (_suppress_final_newline) {
                ret = _fprintf_console(sout, "%s", _data.c_str());
            } else {
                ret = _fprintf_console(sout, "%s\n", _data.c_str());
            }
        }
        _fflush_console(sout);
    }

    else {
        _hide_cursor_guard_t guard(hide_cursor);

        for (auto it = _args.begin(); it != _args.end(); ++it) {
            const auto& arg = *it;
            auto next = it;
            ++next;
            const bool is_last = (next == _args.end());

            std::string separator;
            variant_t val;

            if (arg.first == nullptr) {
                val = variant_t("");
                separator = "";
            } else {
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
                    separator
                        = (is_last && _suppress_final_newline) ? "" : "\n";
                    break;
                }
            }

            std::stringstream ss;

            if (sout != stdout && variable_t::is_number(val.get_type())) {
                // Implements write# semantic (see implementation of input#)
                switch (val.get_type()) {
                case variant_t::type_t::DOUBLE:
                    ret = _fprintf_console(
                        sout, "%lf%s", val.to_double(), separator.c_str());
                    _fflush_console(sout);
                    break;

                case variant_t::type_t::BOOLEAN:
                    ret = _fprintf_console(sout, "%s%s",
                        val.to_bool() ? "true" : "false", separator.c_str());
                    _fflush_console(sout);
                    break;

                case variant_t::type_t::INTEGER:
                    ret = _fprintf_console(
                        sout, "%lli%s", val.to_int(), separator.c_str());
                    _fflush_console(sout);
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
                    ret = _fprintf_console(sout, "%s%s",
                        val.to_bool() ? "true" : "false", separator.c_str());
                    _fflush_console(sout);
                    break;

                case variant_t::type_t::DOUBLE:
                    ss << val.to_double();

                    ret = _fprintf_console(
                        sout, "%s%s", ss.str().c_str(), separator.c_str());
                    _fflush_console(sout);
                    break;

                case variant_t::type_t::INTEGER:
                    ss << val.to_int();

                    ret = _fprintf_console(
                        sout, "%s%s", ss.str().c_str(), separator.c_str());
                    _fflush_console(sout);
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
                            } else {
                                ret = fwprintf(sout, L"%ls",
                                    reinterpret_cast<const wchar_t*>(
                                        data.c_str()));
                            }
                        } else {
                            ret = _fprintf_console(sout, "%s%s",
                                val.to_str().c_str(), separator.c_str());
                        }
                        _fflush_console(sout);
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
