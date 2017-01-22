/*
*  This file is part of nuBASIC
*
*  nuBASIC is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  nuBASIC is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with nuBASIC; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
*
*  Author: Antonino Calderone <acaldmail@gmail.com>
*
*/


/* -------------------------------------------------------------------------- */

#include "nu_reserved_keywords.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

const std::set<std::string> reserved_keywords_t::list = { "any", "and", "as", "band",
    "bor", "bshl", "bshr", "bxor", "chdir", "div", "mod", "or", "xor ", "beep",
    "close", "cls", "const", "delay", "dim", "do", "ellipse", "else", "elif",
    "exit", "end", "fillellipse", "fillrect", "fopen", "for", "function",
    "gosub", "goto", "if", "input", "let", "line", "playsound", "locate",
    "mdelay", "movewindow", "next", "on", "open", "print", "read", "redim",
    "rect", "return", "shell", "sleep", "step", "textout", "then", "write",
    "while", "wend", "plotimage", "randomize", "rem", "loop", "abs", "acos",
    "asc", "asin", "atan", "bnot", "chr", "cos", "cosh", "eval", "eof", "errno",
    "getapppath", "getmousebtn", "getmousex", "getmousey", "exp", "ferror", 
    "ftell", "fsize", "hset", "hget","hcnt","hchk","hdel",
    "hex", "getsheight", "getswidth", "getplatid", "getwindowx", "getwindowy",
    "getwindowdx", "getenv", "setenv", "unsetenv", "mkdir", "rmdir", "erase",
    "true", "false", "getwindowdy", "inkey", "input", "instr", "instrcs", "rgb",
    "int", "lcase", "left", "len", "log", "log10", "max", "mid", "mid", "min",
    "msgbox", "not", "pi", "pow", "pwd", "pstr", "right", "rnd", "sign", "sin",
    "sinh", "sizeof", "sub", "space", "spc", "sqr", "sqrt", "str", "strp",
    "substr", "tan", "tanh", "time", "truncf", "ucase", "val", "ver", "systime",
    "sysday", "syshour", "sysmin", "syssec", "sysyday", "syswday", "sysyear",
    "sysmonth", "boolean", "integer", "double", "float", "string", "type",
    "elseif", "setpixel", "getpixel", "struct", "integer", "double", "float",
    "boolean", "string", "long64", "quit" };


/* -------------------------------------------------------------------------- */

} // namespace nu
