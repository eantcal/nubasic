bin_PROGRAMS = nubasic


nubasic_SOURCES = config.h \
   main.cc \
   nu_basic_defs.h \
   nu_about.h \
   nu_base_tknzr.h \
   nu_global_function_tbl.cc \
   nu_global_function_tbl.h \
   nu_string_tool.h \
   nu_error_codes.h \
   nu_error_codes.cc \
   nu_eval_expr.cc \
   nu_eval_expr.h \
   nu_exception.h \
   nu_expr_any.h \
   nu_expr_bin.h \
   nu_expr_empty.h \
   nu_expr_function.h \
   nu_expr_function.cc \
   nu_expr_unary_op.cc \
   nu_expr_unary_op.h \
   nu_expr_literal.h \
   nu_expr_parser.cc \
   nu_expr_parser.h \
   nu_expr_syntax_tree.cc \
   nu_expr_syntax_tree.h \
   nu_expr_subscrop.h \
   nu_expr_subscrop.cc \
   nu_expr_tknzr.cc \
   nu_expr_tknzr.h \
   nu_expr_var.h \
   nu_flag_map.h \
   nu_icstring.h \
   nu_icstring.cc \
   nu_instrblock_metadata.cc \
   nu_instrblock_metadata.h \
   nu_interpreter.cc \
   nu_interpreter.h \
   nu_runnable.h \
   nu_program.cc \
   nu_program.h \
   nu_lxa.cc \
   nu_lxa.h \
   nu_builtin_help.cc \
   nu_builtin_help.h \
   nu_os_console.cc \
   nu_os_console.h \
   nu_os_std.cc \
   nu_os_std.h \
   nu_label_tbl.cc \
   nu_label_tbl.h \
   nu_prog_pointer.h \
   nu_prog_ctx.cc \
   nu_prog_ctx.h \
   nu_stmt_beep.h \
   nu_stmt_block.cc \
   nu_stmt_block.h \
   nu_stmt_close.h \
   nu_stmt_call.h \
   nu_stmt_call.cc \
   nu_stmt_cls.h \
   nu_stmt_delay.cc \
   nu_stmt_delay.h \
   nu_stmt_dim.cc \
   nu_stmt_dim.h \
   nu_stmt_do.cc \
   nu_stmt_do.h \
   nu_stmt_empty.h \
   nu_stmt_end.h \
   nu_stmt_endsub.h \
   nu_stmt_endsub.cc \
   nu_stmt_endfunction.h \
   nu_stmt_endfunction.cc \
   nu_stmt_else.cc \
   nu_stmt_else.h \
   nu_stmt_for_to_step.cc \
   nu_stmt_for_to_step.h \
   nu_stmt_exit_do.cc \
   nu_stmt_exit_do.h \
   nu_stmt_exit_for.cc \
   nu_stmt_exit_for.h \
   nu_stmt_exit_sub.h \
   nu_stmt_exit_sub.cc \
   nu_stmt_geninstr.h \
   nu_stmt_gosub.h \
   nu_stmt_gosub.cc \
   nu_stmt_goto.h \
   nu_stmt_endif.cc \
   nu_stmt_endif.h \
   nu_stmt.h \
   nu_stmt.cc \
   nu_stmt_if_then_else.cc \
   nu_stmt_if_then_else.h \
   nu_stmt_input.cc \
   nu_stmt_input_file.cc \
   nu_stmt_input_file.h \
   nu_stmt_input.h \
   nu_stmt_let.cc \
   nu_stmt_let.h \
   nu_stmt_locate.cc \
   nu_stmt_locate.h \
   nu_stmt_next.cc \
   nu_stmt_next.h \
   nu_stmt_on_goto.h \
   nu_stmt_on_goto.cc \
   nu_stmt_open.cc \
   nu_stmt_open.h \
   nu_stmt_parser.cc \
   nu_stmt_parser.h \
   nu_stmt_print.cc \
   nu_stmt_print.h \
   nu_stmt_return.h \
   nu_stmt_randomize.h \
   nu_stmt_sub.h \
   nu_stmt_sub.cc \
   nu_stmt_expr.h \
   nu_stmt_expr.cc \
   nu_stmt_function.h \
   nu_stmt_function.cc \
   nu_symbol_map.h \
   nu_terminal.cc \
   nu_terminal.h \
   nu_token.cc \
   nu_token.h \
   nu_tokenizer.h \
   nu_token_list.cc \
   nu_token_list.h \
   nu_variable.cc \
   nu_variable.h \
   nu_terminal_frame.h \
   nu_terminal_frame.cc \
   nu_signal_handling.cc \
   nu_signal_handling.h \
   nu_generic_functor.h \
   nu_stdlib.cc \
   nu_stdlib.h \
   nu_stdtype.h \
   nu_stmt_redim.cc \
   nu_stmt_redim.h \
   nu_stmt_read.cc \
   nu_stmt_read.h \
   nu_stmt_exit_while.cc \
   nu_stmt_exit_while.h \
   nu_stmt_while.cc \
   nu_stmt_while.h \
   nu_stmt_loop_while.cc \
   nu_stmt_loop_while.h \
   nu_stmt_wend.cc \
   nu_stmt_wend.h \
   nu_file_dscrptr_tbl.cc \
   nu_file_dscrptr_tbl.h \
   nu_for_loop_rtdata.cc \
   nu_for_loop_rtdata.h \
   nu_proc_prototype_tbl.h \
   nu_proc_scope.cc \
   nu_proc_scope.h \
   nu_rt_prog_ctx.cc \
   nu_rt_prog_ctx.h \
   nu_stmt_beep.cc \
   nu_stmt_close.cc \
   nu_stmt_cls.cc \
   nu_stmt_empty.cc \
   nu_stmt_end.cc \
   nu_stmt_goto.cc \
   nu_reserved_keywords.cc \
   nu_reserved_keywords.h \
   nu_var_scope.cc \
   nu_var_scope.h \
   nu_variant.cc \
   nu_variant.h 

if COMPILE_TINYVER

AM_CXXFLAGS = $(INTI_CFLAGS) -std=c++11 -DTINY_NUBASIC_VER
nubasic_LDADD = $(INTI_LIBS) 

else

nubasic_SOURCES += \
   nu_stb_image.cc \
   nu_stb_image.h \
   stb_image.h \
   nu_os_gdi.cc \
   nu_os_gdi.h \
   nu_playsnd.h \
   nu_playsnd.cc

AM_CXXFLAGS = $(INTI_CFLAGS) -pthread -std=c++11 
nubasic_LDADD = -lpthread $(INTI_LIBS) 

endif 

EXTRA_DIST=examples nuBasicEditor
