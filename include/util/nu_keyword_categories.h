//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

// Shared keyword catalog grouped by category. Consumers can reuse the
// categories directly (for example graphics-only heuristics) or aggregate them
// into a full reserved-keyword list.
// clang-format off

#define NUBASIC_KEYWORDS_CONTROL(X)                                             \
    X("as") X("beep") X("call") X("case") X("chdir") X("close") X("cls")       \
    X("const") X("data") X("delay") X("dim") X("do") X("elif") X("else")       \
    X("elseif") X("end") X("erase") X("exit") X("for") X("fopen")              \
    X("function") X("gosub") X("goto") X("if") X("include") X("input")         \
    X("let") X("loop") X("mdelay") X("modern") X("next") X("on")               \
    X("open") X("print") X("print#") X("quit") X("randomize") X("read")        \
    X("redim") X("rem") X("restore") X("return") X("shell") X("sleep")         \
    X("static") X("step") X("stop") X("sub") X("syntax") X("then")             \
    X("using") X("while") X("wend") X("write") X("write#")

#define NUBASIC_KEYWORDS_OOP(X)                                                 \
    X("class") X("inherits") X("me") X("mybase") X("new") X("object")          \
    X("overridable") X("overrides") X("private") X("protected")                \
    X("public") X("struct") X("type")

#define NUBASIC_KEYWORDS_TYPES(X)                                               \
    X("any") X("boolean") X("byref") X("byval") X("double") X("float")         \
    X("integer") X("long64") X("string")

#define NUBASIC_KEYWORDS_OPERATORS(X)                                           \
    X("and") X("band") X("bnot") X("bor") X("bshl") X("bshr") X("bxor")        \
    X("div") X("mod") X("not") X("or") X("xor")

#define NUBASIC_KEYWORDS_CONSTANTS(X) X("false") X("true")

#define NUBASIC_KEYWORDS_BUILTIN_CORE(X)                                        \
    X("abs") X("acos") X("asc") X("asin") X("atan") X("chr") X("conv")         \
    X("cos") X("cosh") X("eof") X("errno") X("eval") X("exp") X("ferror")      \
    X("ftell") X("fsize") X("getapppath") X("getenv") X("getmousebtn")         \
    X("getmousex") X("getmousey") X("getplatid") X("getsheight")                \
    X("getswidth") X("getvkey") X("getwindowdx") X("getwindowdy")               \
    X("getwindowx") X("getwindowy") X("hchk") X("hcnt") X("hdel") X("hex")     \
    X("hget") X("hset") X("inkey") X("instr") X("instrcs") X("int")            \
    X("lcase") X("left") X("len") X("locate") X("log") X("log10")              \
    X("max") X("mid") X("min") X("mkdir") X("msgbox") X("pi") X("playsound")   \
    X("pow") X("pstr") X("pwd") X("rgb") X("right") X("rmdir") X("rnd")        \
    X("setenv") X("settopmost") X("sign") X("sin") X("sinh") X("sizeof")       \
    X("space") X("spc") X("sqr") X("sqrt") X("str") X("substr") X("sysday")    \
    X("syshour") X("sysmin") X("sysmonth") X("syssec") X("systime")             \
    X("syswday") X("sysyday") X("sysyear") X("tan") X("tanh") X("time")        \
    X("truncf") X("ucase") X("unsetenv") X("val") X("ver")

#define NUBASIC_KEYWORDS_GRAPHICS(X)                                            \
    X("ellipse") X("fillellipse") X("fillrect") X("getpixel") X("line")        \
    X("movewindow") X("plotimage") X("rect") X("refresh") X("screen")          \
    X("screenlock") X("screenunlock") X("setpixel") X("textout")

#define NUBASIC_KEYWORDS_ALL(X)                                                 \
    NUBASIC_KEYWORDS_CONTROL(X)                                                 \
    NUBASIC_KEYWORDS_OOP(X)                                                     \
    NUBASIC_KEYWORDS_TYPES(X)                                                   \
    NUBASIC_KEYWORDS_OPERATORS(X)                                               \
    NUBASIC_KEYWORDS_CONSTANTS(X)                                               \
    NUBASIC_KEYWORDS_BUILTIN_CORE(X)                                            \
    NUBASIC_KEYWORDS_GRAPHICS(X)

// clang-format on
/* -------------------------------------------------------------------------- */
