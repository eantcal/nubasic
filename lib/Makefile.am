lib_LIBRARIES = libnubasic.a

libnubasic_a_SOURCES = $(top_srcdir)/config.h \
   nu_global_function_tbl.cc \
   nu_error_codes.cc \
   nu_hxxx_api.cc \
   nu_eval_expr.cc \
   nu_expr_function.cc \
   nu_expr_unary_op.cc \
   nu_expr_parser.cc \
   nu_expr_syntax_tree.cc \
   nu_expr_subscrop.cc \
   nu_expr_tknzr.cc \
   nu_expr_var.cc \
   nu_icstring.cc \
   nu_instrblock_metadata.cc \
   nu_interpreter.cc \
   nu_program.cc \
   nu_lxa.cc \
   nu_builtin_help.cc \
   nu_os_console.cc \
   nu_os_std.cc \
   nu_label_tbl.cc \
   nu_prog_ctx.cc \
   nu_stmt_block.cc \
   nu_stmt_call.cc \
   nu_stmt_delay.cc \
   nu_stmt_const.cc \
   nu_stmt_dim.cc \
   nu_stmt_do.cc \
   nu_stmt_endsub.cc \
   nu_stmt_endfunction.cc \
   nu_stmt_endstruct.cc \
   nu_stmt_else.cc \
   nu_stmt_for_to_step.cc \
   nu_stmt_exit_do.cc \
   nu_stmt_exit_for.cc \
   nu_stmt_exit_sub.cc \
   nu_stmt_gosub.cc \
   nu_stmt_endif.cc \
   nu_stmt.cc \
   nu_stmt_if_then_else.cc \
   nu_stmt_input.cc \
   nu_stmt_input_file.cc \
   nu_stmt_let.cc \
   nu_stmt_locate.cc \
   nu_stmt_next.cc \
   nu_stmt_on_goto.cc \
   nu_stmt_open.cc \
   nu_stmt_parser.cc \
   nu_stmt_print.cc \
   nu_stmt_sub.cc \
   nu_stmt_expr.cc \
   nu_stmt_function.cc \
   nu_stmt_struct.cc \
   nu_stmt_struct_element.cc \
   nu_terminal.cc \
   nu_token.cc \
   nu_token_list.cc \
   nu_variable.cc \
   nu_terminal_frame.cc \
   nu_signal_handling.cc \
   nu_stdlib.cc \
   nu_stmt_redim.cc \
   nu_stmt_read.cc \
   nu_stmt_exit_while.cc \
   nu_stmt_while.cc \
   nu_stmt_loop_while.cc \
   nu_stmt_wend.cc \
   nu_file_dscrptr_tbl.cc \
   nu_for_loop_rtdata.cc \
   nu_proc_scope.cc \
   nu_rt_prog_ctx.cc \
   nu_stmt_beep.cc \
   nu_stmt_close.cc \
   nu_stmt_cls.cc \
   nu_stmt_empty.cc \
   nu_stmt_end.cc \
   nu_stmt_goto.cc \
   nu_reserved_keywords.cc \
   nu_var_scope.cc \
   nu_variant.cc 


    AM_CXXFLAGS = $(INTI_CFLAGS) -pthread -std=c++11 -I$(top_srcdir)/include -DTINY_NUBASIC_VER

libnubasic_a_LIBADD = $(INTI_LIBS) 

