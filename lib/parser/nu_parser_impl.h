//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

// Internal header — included only by nu_parser_*.cc files.
// Aggregates all dependencies needed by the statement parser translation units.
// Not part of the public API.

/* -------------------------------------------------------------------------- */

#pragma once

#include "nu_basic_defs.h"
#include "nu_global_function_tbl.h"
#include "nu_label_tbl.h"
#include "nu_token_list.h"

#include "nu_stmt_beep.h"
#include "nu_stmt_block.h"
#include "nu_stmt_call.h"
#include "nu_stmt_case.h"
#include "nu_stmt_class.h"
#include "nu_stmt_close.h"
#include "nu_stmt_cls.h"
#include "nu_stmt_const.h"
#include "nu_stmt_data.h"
#include "nu_stmt_delay.h"
#include "nu_stmt_dim.h"
#include "nu_stmt_do.h"
#include "nu_stmt_else.h"
#include "nu_stmt_end.h"
#include "nu_stmt_end_select.h"
#include "nu_stmt_endclass.h"
#include "nu_stmt_endfunction.h"
#include "nu_stmt_endif.h"
#include "nu_stmt_endstruct.h"
#include "nu_stmt_endsub.h"
#include "nu_stmt_exit_do.h"
#include "nu_stmt_exit_for.h"
#include "nu_stmt_exit_sub.h"
#include "nu_stmt_exit_while.h"
#include "nu_stmt_expr.h"
#include "nu_stmt_for_to_step.h"
#include "nu_stmt_function.h"
#include "nu_stmt_gosub.h"
#include "nu_stmt_goto.h"
#include "nu_stmt_if_then_else.h"
#include "nu_stmt_inherits.h"
#include "nu_stmt_input.h"
#include "nu_stmt_input_file.h"
#include "nu_stmt_let.h"
#include "nu_stmt_locate.h"
#include "nu_stmt_loop_while.h"
#include "nu_stmt_method_call.h"
#include "nu_stmt_next.h"
#include "nu_stmt_on_goto.h"
#include "nu_stmt_open.h"
#include "nu_stmt_print.h"
#include "nu_stmt_randomize.h"
#include "nu_stmt_read.h"
#include "nu_stmt_read_file.h"
#include "nu_stmt_redim.h"
#include "nu_stmt_return.h"
#include "nu_stmt_screen.h"
#include "nu_stmt_select_case.h"
#include "nu_stmt_stop.h"
#include "nu_stmt_struct.h"
#include "nu_stmt_struct_element.h"
#include "nu_stmt_sub.h"
#include "nu_stmt_syntax.h"
#include "nu_stmt_using.h"
#include "nu_stmt_wend.h"
#include "nu_stmt_while.h"

#include "nu_expr_parser.h"
#include "nu_expr_tknzr.h"
#include "nu_statement_parser.h"

#include "nu_stdlib.h"
#include "nu_stmt_geninstr.h"

#ifndef TINY_NUBASIC_VER
#include "nu_os_gdi.h"
#endif

#include <set>
#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu {

using stmt_shell_t = stmt_geninstr_t<os_shell_t, variant_t, 1>;
using stmt_chdir_t = stmt_geninstr_t<os_chdir_t, variant_t, 1>;

using stmt_fopen_t = stmt_geninstr_t<os_fopen_t, variant_t, 3>;
using stmt_fseek_t = stmt_geninstr_t<os_fseek_t, variant_t, 3>;
using stmt_flush_t = stmt_geninstr_t<os_fflush_t, variant_t, 1>;

#ifndef TINY_NUBASIC_VER
using stmt_textout_t = stmt_geninstr_t<os_textout_t, gdi_varg_t, 4>;
using stmt_plotimage_t = stmt_geninstr_t<os_plotimage_t, gdi_varg_t, 3>;
using stmt_line_t = stmt_geninstr_t<os_line_t, gdi_iarg_t, 5>;
using stmt_set_pixel_t = stmt_geninstr_t<os_set_pixel_t, gdi_iarg_t, 3>;
using stmt_rect_t = stmt_geninstr_t<os_rect_t, gdi_iarg_t, 5>;
using stmt_fillrect_t = stmt_geninstr_t<os_fillrect_t, gdi_iarg_t, 5>;
using stmt_ellipse_t = stmt_geninstr_t<os_ellipse_t, gdi_iarg_t, 5>;
using stmt_fillellipse_t = stmt_geninstr_t<os_fillellipse_t, gdi_iarg_t, 5>;
#endif // TINY_NUBASIC_VER

} // namespace nu


/* -------------------------------------------------------------------------- */
