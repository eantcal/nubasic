//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_reserved_keywords.h"


/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */
    std::set<std::string>& reserved_keywords_t::list()
    {
        static
            std::set<std::string> list = { "any", "and", "as", "band",
            "bor", "bshl", "bshr", "bxor", "chdir", "div", "mod", "or", "xor ", "beep",
            "close", "cls", "conv", "const", "data", "delay", "dim", "do", "ellipse", "else", "elif",
            "exit", "end", "fillellipse", "fillrect", "fopen", "for", "function",
            "gosub", "goto", "if", "input", "let", "line", "playsound", "locate",
            "mdelay", "movewindow", "next", "object", "on", "open", "print", "read", "redim",
            "rect", "return", "shell", "sleep", "step", "textout", "then", "write",
            "while", "wend", "plotimage", "randomize", "rem", "loop", "abs", "acos",
            "asc", "asin", "atan", "bnot", "chr", "cos", "cosh", "eval", "eof", "errno",
            "getapppath", "getmousebtn", "getmousex", "getmousey", "exp", "ferror",
            "ftell", "fsize", "hset", "hget","hcnt","hchk","hdel","restore",
            "hex", "getsheight", "getswidth", "getplatid", "getwindowx", "getwindowy", "getvkey",
            "getwindowdx", "getenv", "setenv", "unsetenv", "mkdir", "rmdir", "erase",
            "true", "false", "getwindowdy", "inkey", "input", "instr", "instrcs", "rgb",
            "int", "lcase", "left", "len", "log", "log10", "max", "mid", "mid", "min",
            "msgbox", "not", "pi", "pow", "pwd", "pstr", "right", "rnd", "sign", "sin",
            "sinh", "sizeof", "sub", "space", "spc", "sqr", "sqrt", "str", "strp",
            "substr", "tan", "tanh", "time", "truncf", "ucase", "val", "ver", "systime",
            "sysday", "syshour", "sysmin", "syssec", "sysyday", "syswday", "sysyear",
            "sysmonth", "boolean", "integer", "double", "float", "string", "type",
            "elseif", "setpixel", "getpixel", "struct", "integer", "double", "float",
             "boolean", "string", "long64", "quit", "settopmost", "stop" };

        return list;
    }

/* -------------------------------------------------------------------------- */

} // namespace nu
