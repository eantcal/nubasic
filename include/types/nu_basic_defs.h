//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_BASIC_DEFS_H__
#define __NU_BASIC_DEFS_H__


/* -------------------------------------------------------------------------- */

// Scanner config. data
#define NU_BASIC_BLANKS " \t\r"
#define NU_BASIC_NEWLINES "\n"

#define NU_BASIC_OP_INC "++"
#define NU_BASIC_OP_DEC "--"

#define NU_BASIC_BEGIN_SUBEXPR '('
#define NU_BASIC_END_SUBEXPR ')'
#define NU_BASIC_BEGIN_STRING "\""
#define NU_BASIC_END_STRING "\""
#define NU_BASIC_ESCAPE_CHAR '\\'
#define NU_BASIC_BEGIN_SUBSCR '['
#define NU_BASIC_END_SUBSCR ']'

#define NU_BASIC_PARAM_SEP ","

#define NU_BASIC_EXPONENT_SYMB 'E'

#define NU_BASIC_BEGIN_SUBEXPR_OP "("
#define NU_BASIC_END_SUBEXPR_OP ")"
#define NU_BASIC_BEGIN_SUBSCR_OP "["
#define NU_BASIC_END_SUBSCR_OP "]"

#define NU_BASIC_SINGLE_CHAR_OPS ".+-*/^,\\=\";:<>?'"
#define NU_BASIC_WORD_OPS                                                      \
    {                                                                          \
        "bxor", "band", "bshr", "bshl", "bor", NU_BASIC_OP_INC,                \
            NU_BASIC_OP_DEC, "mod", "div", "xor", "and", "or", "<>", "<=",     \
            ">="                                                               \
    }
#define NU_BASIC_BEGIN_SUBEXPR '('
#define NU_BASIC_END_SUBEXPR ')'
#define NU_BASIC_BEGIN_STRING "\""
#define NU_BASIC_END_STRING "\""
#define NU_BASIC_ESCAPE_CHAR '\\'

#define NU_BASIC_LINE_COMMENT { "'" }


/* -------------------------------------------------------------------------- */

// Error strings
#define NU_BASIC_ERROR_STR__SYNTAXERROR "Syntax Error"
#define NU_BASIC_ERROR_STR__ERRORLOADING "Error loading file or file not found"


/* -------------------------------------------------------------------------- */

// Console prompt
#define NU_BASIC_MSG_STR__READY "Ready."
#define NU_BASIC_PROMPT_STR "Ok."
#define NU_BASIC_PROMPT_NEWLINE "\n"


/* -------------------------------------------------------------------------- */

// Xterm config
#ifdef GNOME_TERMINAL
#define NU_BASIC_XTERM_EXECBIN "gnome-terminal"
#else
#define NU_BASIC_XTERM_EXECBIN "xterm"
#endif
#define NU_BASIC_XTERM_EXECOPT "-e"
#define NU_BASIC_HELP "-h"
#define NU_BASIC_XTERM_FRAME_SWITCH "-x"
#define NU_BASIC_XTERM_NOFRAME_SWITCH "-nx"


/* -------------------------------------------------------------------------- */

// Command line shortcut

#define NU_BASIC_HELP_MACRO 'h' // -h --> HELP "<argument>"
#define NU_BASIC_EXEC_MACRO 'e' // -e --> EXEC "<argument>"
#define NU_BASIC_LOAD_MACRO 'l' // -l --> LOAD "<argument>"


/* -------------------------------------------------------------------------- */

#endif
