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

#include "nu_builtin_help.h"
#include "nu_about.h"
#include "nu_basic_defs.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

builtin_help_t* builtin_help_t::_instance = nullptr;


/* -------------------------------------------------------------------------- */

struct help_content_t {
    lang_item_t lang_item;
    const char* lang_id;
    const char* description;
    const char* signature;
};


/* -------------------------------------------------------------------------- */

static help_content_t _help_content[] = {

#ifndef _NUBASIC_IDE_
    { lang_item_t::COMMAND, "!", "Executes shell command",
        "! shell_command\n" },

    { lang_item_t::COMMAND, "?", "Executes Print instruction\n"
                                 "See also Print",
        "? [parameters...]" },

    { lang_item_t::COMMAND, "run", "Executes the program currently in memory",
        "Run [linenumber]" },

    { lang_item_t::COMMAND, "cont", "Continue interrupted program execution",
        "Cont" },

    { lang_item_t::COMMAND, "resume",
        "Resume program execution from next line of interrupted one",
        "Resume" },

    { lang_item_t::COMMAND, "break",
        "Set a breakpoint. If no parameters are specified,\n"
        "list lines with active breakpoints. Conditional breakpoint\n"
        "may be set using a boolean expression. Breakpoint is enabled\n"
        "if the expression becomes true during program execution",
        "Break [[linenum] [If expression]]" },

    { lang_item_t::COMMAND, "rmbrk", "Remove a breakpoint", "RmBrk linenum" },

    { lang_item_t::COMMAND, "clrbrk", "Remove all active breakpoints",
        "ClrBrk" },

    { lang_item_t::COMMAND, "load", "Loads a program file in memory",
        "Load filename\n" },

    { lang_item_t::COMMAND, "exec", "Loads and runs a program file",
        "Exec filename\n" },

    { lang_item_t::COMMAND, "save", "Saves current program to file",
        "Save filename" },

    { lang_item_t::COMMAND, "pwd", "Retrieves the current working directory",
        "Pwd" },

    { lang_item_t::COMMAND, "cd", "Changes the current working directory",
        "Cd new_working_dir" },

    { lang_item_t::COMMAND, "list", "Lists all or part of a program",
        "List [line number][-line number]\n"
        "List [line number-]" },

    { lang_item_t::COMMAND, "clr", "Clears all variables", "Clr" },

    { lang_item_t::COMMAND, "grep",
        "Lists lines containing a match to the given pattern", "Grep pattern" },

    { lang_item_t::COMMAND, "new", "Deletes the program currently "
                                   "in memory and clear all variables",
        "New" },

    { lang_item_t::COMMAND, "renum", "Renumbers program lines",
        "Renum [increment(default=10)]" },

    { lang_item_t::COMMAND, "tron", "Enables program tracing", "TrOn" },

    { lang_item_t::COMMAND, "troff", "Disables program tracing", "TrOff" },

    { lang_item_t::COMMAND, "ston", "Enables step mode", "StOn" },

    { lang_item_t::COMMAND, "stoff", "Disables step mode", "StOff" },

    { lang_item_t::COMMAND, "exit", "Quit the " NUBASIC_PROGRAM_NAME, "Exit" },

    { lang_item_t::COMMAND, "ver",
        "Prints out information about " NUBASIC_PROGRAM_NAME " version",
        "Ver" },

    { lang_item_t::COMMAND, "vars", "Show run-time data", "Vars" },

    { lang_item_t::COMMAND, "meta", "Show build-up metadata", "Meta" },

    { lang_item_t::COMMAND, "help",
        "Provides a description about keyword. In case keyword is not "
        "specified, shows the list of keywords, including commands, "
        "instructions, functions and operators.",
        "Help [keyword]" },

    { lang_item_t::COMMAND, "apropos",
        "Searches a help content containing short descriptions of commands "
        "for keywords and displays the result on the standard output.",
        "Apropos keyword" },
#endif // ..._NUBASIC_IDE_

    { lang_item_t::FUNCTION, "rgb",
        "Selects a red, green, blue (RGB) color based "
        "on the arguments supplied.",
        "rgb(red%, green%, blue%)" },

    { lang_item_t::FUNCTION, "eval",
        "Evaluates expression of a given string. "
        "The expression can include existing variables.",
        "Eval(expr$)" },

    { lang_item_t::FUNCTION, "hset", "Insert a pair key/value into a hash table",
        "hset(hash as String, key as String, value as Any)" },

    { lang_item_t::FUNCTION, "hget", "Returns a pair key/value from a hash table",
        "hget(hash as String, key as String) as Any" },

    { lang_item_t::FUNCTION, "hchk", "Returns true if hash/key exist",
        "hchk(hash as String, key as String) as Boolean" },

    { lang_item_t::FUNCTION, "hcnt", "Returns count of pairs hash/key of a hash table",
        "hcnt(hash as String) as Integer" },

    { lang_item_t::FUNCTION, "hdel", 
        "Erases the key/value pair from a hash table "
        "or a whole table in case the key is not specified",
        "hdel(hash as String [, key as String])" },

    { lang_item_t::FUNCTION, NU_BASIC_OP_INC, "Increment unary operator",
        "++variable" },

    { lang_item_t::FUNCTION, NU_BASIC_OP_DEC, "Decrement unary operator",
        "--variable" },

    { lang_item_t::FUNCTION, "pwd$", "Retrieves the current working directory",
        "Pwd$()" },

    { lang_item_t::FUNCTION, "min", "Returns the smallest of x and y",
        "Min(x,y)" },

    { lang_item_t::FUNCTION, "max", "Returns the largest of x and y",
        "Max(x,y)" },

    { lang_item_t::FUNCTION, "str",
        "Returns a string representation of the value of x", "Str[$](x)" },

    { lang_item_t::FUNCTION, "strp",
        "Returns a string representation of the value of x\n"
        "Precision% specifies the minimum number of digits to be written/\n"
        "If the value to be written is shorter than %precision, the result\n"
        "is padded with leading zeros",
        "StrP[$](x, precision%)" },

    { lang_item_t::FUNCTION, "hex",
        "Returns a hexadecimal representation (as string) of the value of x",
        "Hex[$](x)" },

    { lang_item_t::FUNCTION, "lcase", "Converts a string to lower-case.",
        "LCase[$](x)" },

    { lang_item_t::FUNCTION, "ucase", "Converts a string to upper-case.",
        "UCase[$](x)" },

    { lang_item_t::FUNCTION, "val",
        "Returns the numerical (val% also integral) value of string x$",
        "Val[%](x$)" },

    { lang_item_t::FUNCTION, "mid",
        "Returns a string of m characters from x$ beginning with the "
        "n-th character (first character has offset 1)",
        "Mid[$](x$,n,m)" },

    { lang_item_t::FUNCTION, "substr$",
        "Returns a string of m characters from x$ beginning with the "
        "n-th character (first character has offset 0)",
        "SubStr$(x$,n,m)" },

    { lang_item_t::FUNCTION, "pstr$",
        "Returns a string s$ with the pos-th character replaced "
        "by first character of c$, with pos >= 0 ",
        "PStr$(s$,pos,c$)" },

    { lang_item_t::FUNCTION, "right",
        "Returns a string that comprises the right-most n characters of x$",
        "Right[$](x$,n)" },

    { lang_item_t::FUNCTION, "left",
        "Returns a string that comprises the left-most n characters of x$",
        "Left[$](x$,n)" },

    { lang_item_t::FUNCTION, "chr",
        "Converts an ASCII code to its equivalent character", "Chr[$](x)" },


    { lang_item_t::FUNCTION, "spc", "Returns a string of x spaces",
        "Spc[CE$](x)" },

    { lang_item_t::FUNCTION, "asc",
        "Returns a numeric value that is the ASCII code "
        "for the first character of the string x$",
        "Asc[$](x$)" },

    { lang_item_t::FUNCTION, "inkey",
        "Reads a keyboard character from the standard input buffer"
        ", without echoing it to the screen. "
        "InKey[$] will return immediately, even if there are no character"
        " to retrieve",
        "InKey[$]()" },

    { lang_item_t::FUNCTION, "input$", "Returns a string of x characters read "
                                       "from the standard input",
        "Input$(x)" },

    { lang_item_t::FUNCTION, "len", "Returns the number of characters in x$",
        "Len(x$)" },

    { lang_item_t::FUNCTION, "not", "Returns 1 if x=0, otherwise 0", "Not(x)" },

    { lang_item_t::FUNCTION, "rnd",
        "Returns a random number between 0 and 1. If x<0 then "
        "the Random number generator is initialized",
        "Rnd(x)" },

    { lang_item_t::FUNCTION, "int", "Truncates an expression to a whole number",
        "Int(x)" },

    { lang_item_t::FUNCTION, "sign", "Returns the sign of x. If x=0 returns 0, "
                                     "if x<0 retunrs -1 else 1",
        "Sign(x)" },

    { lang_item_t::FUNCTION, "pow",
        "Returns base x raised to the power exponent y", "Pow(x,y)" },

    { lang_item_t::FUNCTION, "truncf",
        "Rounds x toward zero, returning the nearest integral value"
        " that is not larger in magnitude than x",
        "TruncF(x)" },

    { lang_item_t::FUNCTION, "sqr", "Returns the square root of x", "Sqr(x)" },

    { lang_item_t::FUNCTION, "sqrt", "Returns the square root of x",
        "Sqrt(x)" },

    { lang_item_t::FUNCTION, "sinh", "Returns the hyperbolic sine of x radians",
        "Sinh(x)" },

    { lang_item_t::FUNCTION, "tanh",
        "Returns the hyperbolic tangent of x radians", "Tanh(x)" },

    { lang_item_t::FUNCTION, "cosh",
        "Returns the hyperbolic cosine of x radians", "Cosh(x)" },

    { lang_item_t::FUNCTION, "asin",
        "Returns the principal value of the arc sine of x, "
        "expressed in radians",
        "Asin(x)" },

    { lang_item_t::FUNCTION, "acos",
        "Returns the principal value of the arc cosine of x, "
        "expressed in radians",
        "Acos(x)" },

    { lang_item_t::FUNCTION, "atan",
        "Returns the principal value of the arc tangent of x, "
        "expressed in radians",
        "Atan(x)" },

    { lang_item_t::FUNCTION, "sin",
        "Returns the trigonometric sine of x radians", "Sin(x)" },

    { lang_item_t::FUNCTION, "cos",
        "Returns the trigonometric cosine of x radians", "Cos(x)" },

    { lang_item_t::FUNCTION, "tan",
        "Returns the trigonometric tangent of x, in radians", "Tan(x)" },

    { lang_item_t::FUNCTION, "log", "Returns the natural logarithm of x",
        "Log(x)" },

    { lang_item_t::FUNCTION, "log10", "Returns common (base-10) logarithm of x",
        "Log10(x)\n" },

    { lang_item_t::FUNCTION, "exp",
        "Returns e (the base of natural logarithms) to the power of x",
        "Exp(x)" },

    { lang_item_t::FUNCTION, "abs",
        "Returns the absolute value of the expression x", "Abs(x)" },

    { lang_item_t::FUNCTION, "errno", "Returns error code of last operation",
        "Errno() as Integer" },

    { lang_item_t::FUNCTION, "getapppath", "Returns nuBASIC path",
        "GetAppPath() as String" },

    { lang_item_t::FUNCTION, "getenv", "Gets an environment variable",
        "GetEnv[$](var$)" },

    { lang_item_t::FUNCTION, "setenv",
        "Sets an environment variable to a given value", "SetEnv(var$, val$)" },

    { lang_item_t::FUNCTION, "unsetenv", "Unsets an environment variable",
        "UnsetEnv(var$)" },

    { lang_item_t::FUNCTION, "erase", "Deletes file from disk",
        "Delete(filename$)" },

    { lang_item_t::FUNCTION, "rmdir", "Deletes directory from disk",
        "RmDir(path$)" },

    { lang_item_t::FUNCTION, "mkdir", "Creates a new directory",
        "MkDir(path$)" },

    { lang_item_t::FUNCTION, "errno$",
        "Returns error string describing error errnum", "Errno$(errornum)" },

    { lang_item_t::FUNCTION, "ferror",
        "Tests for an error on a file.\n"
        "If no error has occurred on file, FError returns 0.\n"
        "If filenum is invalid, FError sets errno to 22 and returns -1.\n"
        "Otherwise, it returns a nonzero value\n"
        "See also Errno, Errno$, Open, Close, EOF",
        "FError(filenum)" },

    { lang_item_t::FUNCTION, "ftell",
        "Returns the current value of the position indicator of the file\n"
        "If no error has occurred on file, FTell returns the position\n"
        "If filenum is invalid, FTell sets errno to 22 and returns -1.\n"
        "\nSee also Errno, Errno$, Open, Close, EOF",
        "FTell(filenum)" },

    { lang_item_t::FUNCTION, "fsize",
        "Returns the current size in bytes of the file\n"
        "If no error has occurred on file, FSIZE returns the size\n"
        "If filenum is invalid, FSIZE sets errno to 22 and returns -1.\n"
        "\nSee also Errno, Errno$, Open, Close, EOF, FTell, Seek",
        "FSize(filenum)" },

    { lang_item_t::FUNCTION, "eof",
        "Checks the end-of-file indicator.\n"
        "If no error has occurred on filenum, "
        "EOF returns 1 if EOF reached else 0\n"
        "If filenum is invalid, EOF sets errno to 22 and returns -1."
        "Otherwise, it returns a nonzero value\n"
        "\nSee also Errno, Errno$, Open, Close, FError, FTell",
        "Eof(filenum)" },

    { lang_item_t::FUNCTION, "pi",
        "Returns the ratio of a circle's circumference "
        "to its diameter, approximately equal to 3.14159",
        "PI()" },

    { lang_item_t::FUNCTION, "ver$", "Returns nuBASIC version", "Ver$()" },

    { lang_item_t::FUNCTION, "instr",
        "Returns an integer specifying the start position\n"
        "of the first occurrence of one string within another.\n"
        "Returns -1 if searchStr$ is not a substring of muStr$\n"
        "Function INSTR is case insensitive.\n"
        "See also INSTRCS",
        "InStr(myStr$, searchStr$)\n" },

    { lang_item_t::FUNCTION, "instrcs",
        "Returns an integer specifying the start position\n"
        "of the first occurrence of one string within another.\n"
        "Returns -1 if searchStr$ is not a substring of muStr$\n"
        "Function INSTRCS is case sensitive.\n"
        "See also INSTR",
        "InStrCS(myStr$, searchStr$)" },

    { lang_item_t::FUNCTION, "bnot", "Inverts the bits of its operand",
        "bNot(x)" },

    { lang_item_t::FUNCTION, "sizeof",
        "Returns the size in bytes of the whole variable or array",
        "SizeOf(x)" },

    { lang_item_t::FUNCTION, "sizeof@",
        "Returns the number of items of the array x", "SizeOf@(x)" },

    { lang_item_t::FUNCTION, "sysmonth", "returns the month(from 0 to 11) for "
                                         "the system date, according to local "
                                         "time",
        "SysMonth()" },

    { lang_item_t::FUNCTION, "sysyear",
        "returns the year for the system date, according to local time",
        "SysYear()" },

    { lang_item_t::FUNCTION, "sysday",
        "returns the day for the system date, according to local time",
        "SysDay()" },

    { lang_item_t::FUNCTION, "syswday",
        "returns the week day for the system date, according to local time",
        "SysWDay()" },

    { lang_item_t::FUNCTION, "sysyday", "returns the year day (1-365) for the "
                                        "system date, according to local time",
        "SysYDay()" },

    { lang_item_t::FUNCTION, "syshour", "returns the hour of day",
        "SysHour()" },

    { lang_item_t::FUNCTION, "sysmin", "returns the minute of current hour",
        "SysMin()" },

    { lang_item_t::FUNCTION, "systime", "returns a local time and date in the "
                                        "format 'DDD MMM DN HH:MM:SS YYYY'",
        "SysTime[$]()" },


    { lang_item_t::FUNCTION, "time",
        "returns a local time in seconds since the Epoch", "Time()" },

    { lang_item_t::FUNCTION, "getplatid",
        "Returns 1 for Windows, 2 for other (Linux) platforms\n",
        "GetPlatId()" },

    { lang_item_t::OPERATOR, "xor",
        "Performs a logical exclusion on two Boolean expressions", "x Xor y" },

    { lang_item_t::OPERATOR, "and",
        "Performs a logical conjunction on two Boolean expressions",
        "x And y" },

    { lang_item_t::OPERATOR, "or",
        "Performs a logical conjunction on two Boolean expressions", "x Or y" },

    { lang_item_t::OPERATOR, "bxor",
        "Returns a one in each bit position for which the corresponding\n"
        "bits of either but not both operands are ones",
        "x bXor y" },

    { lang_item_t::OPERATOR, "band",
        "Returns a one in each bit position for which the corresponding\n"
        "bits of both operands are ones",
        "x bAnd y" },

    { lang_item_t::OPERATOR, "bor",
        "Returns a one in each bit position for which the corresponding\n"
        "bits of either or both operands are ones",
        "x bOR y" },

    { lang_item_t::OPERATOR, "bshl",
        "Shifts x in binary representation y bits to the left,\n"
        "shifting in zeros from the right",
        "x bShl y" },

    { lang_item_t::OPERATOR, "bshr",
        "Shifts x in binary representation y bits to the right,\n"
        "discarding bits shifted off",
        "x bShr y" },

    { lang_item_t::OPERATOR, "^",
        "Raises a number to the power of another number", "x ^ y" },

    { lang_item_t::OPERATOR, "/",
        "Divides two numbers and returns a floating-point result", "x / y" },

    { lang_item_t::OPERATOR, "*", "Multiplies two numbers", "x * y" },

    { lang_item_t::OPERATOR, "-",
        "Returns the difference between two numeric expressions "
        "or the negative value of a numeric expression",
        "[x] - y" },

    { lang_item_t::OPERATOR, "+",
        "Adds two numbers or returns the positive value of a "
        "numeric expression.\nCan also be used to concatenate two "
        "string expressions",
        "[x] + y" },

    { lang_item_t::OPERATOR, "mod",
        "Divides two numbers and returns only the remainder", "x MOD y" },

    { lang_item_t::OPERATOR, "div",
        "Divides two numbers and returns an integer result", "x DIV y" },

    { lang_item_t::OPERATOR, "\\",
        "Divides two numbers and returns an integer result", "x \\ y" },

    { lang_item_t::OPERATOR, "<>", "'Not equal to' operator", "x <> y" },

    { lang_item_t::OPERATOR, "=", "'Equal to' or assign operator "
                                  "(when x is a left-value variable)",
        "x = y" },

    { lang_item_t::OPERATOR, "<", "'Less than' operator", "x < y" },

    { lang_item_t::OPERATOR, ">", "'Greater than' operator", "x > y" },


    { lang_item_t::OPERATOR, "<=", "'Less than or equal to' operator",
        "x <= y" },

    { lang_item_t::OPERATOR, ">=", "'Greater than or equal to' operator",
        "x >= y" },

    { lang_item_t::INSTRUCTION, "chdir",
        "Changes the current working directory", "ChDir dir_string" },

    { lang_item_t::INSTRUCTION, "if",
        "Makes a decision regarding program flow based on "
        "the result returned by an expression\n"
        "If the result of expression is nonzero(logical true),"
        " the Then statement(s) is executed.\n"
        "If the result of expression is zero (false), the Then statement(s)"
        " is ignored and the Else statement(s), if present, is executed.\n"
        "Otherwise, execution continues with the next executable statement",
        "If expression Then statement(s) [Else statement(s)]" },

    { lang_item_t::INSTRUCTION, "elif",
        "Elif, as its name suggests, is a combination of If and Else.\n"
        "Like Else, it extends an If statement to execute a different \n"
        "statement in case the original If expression evaluates to False.\n"
        "However, unlike Else, it will execute that alternative expression\n"
        "only if the Elif conditional expression evaluates to True.\n",
        "If expression then\n"
        "  statement(s)\n"
        "[Elif expression then\n"
        "  statement(s)\n"
        "Elif ...\n"
        "  statement(s)\n"
        "Elif ...\n"
        "  statement(s)\n"
        "Else ...\n"
        "  statement(s)]\n"
        "End if\n" },

    { lang_item_t::INSTRUCTION, "for",
        "Executes a series of instructions a specified number "
        "of times in a loop.\n"
        "- variable is used as a counter\n"
        "- x,y, and z are numeric expressions\n"
        "- Step z specifies the counter increment for each loop."
        "If Step is not specified, the increment is assumed to be 1\n"
        "The first numeric expression (x) is the initial value of the "
        "counter.\n"
        "The second numeric expression (y) is the final value of the counter.\n"
        "Program lines following the For statement are executed until the Next"
        " statement is encountered. Then, the counter is incremented by the "
        "amount "
        "specified by Step",
        "  For variable=x TO y [Step z] [statement(s) [Next [variable]]]\n\n"
        "  For variable=x TO y [Step z] [statement(s)]\n"
        "  ... [statement(s)]\n"
        "      [Exit For]\n"
        "  ... [statement(s)]\n"
        "  Next [variables]\n" },

    { lang_item_t::INSTRUCTION, "on",
        "Send program flow to one of several possible destinations based\n"
        "on the value of a integer expression in the range of 0-2^31.\n",
        "On expression Goto label1[, label2[, ..., [labelN...]]]\n\n" },

    { lang_item_t::INSTRUCTION, "while",
        "Executes a series of statements as long as a "
        "given condition is not zero (True)\n"
        "While...Wend loops can be nested to any level.\n"
        "Each Wend matches the most recent While.\n"
        "The While...Wend statement syntax has these parts:\n"
        "- condition  :	 Numeric expression or string expression "
        "that evaluates to True or False.\n"
        "- statements : Optional. One or more statements executed while "
        "condition is True.\n"
        "- Exit While : Optional. Transfers control out of the While block.\n",
        "  While condition DO statement[:statements...]\n\n"
        "  While condition [DO]\n"
        "      [statement(s)]  \n"
        "      [Exit While]    \n"
        "      [statement(s)]  \n"
        "  Wend | End While " },

    // Do ... Loop While cond
    { lang_item_t::INSTRUCTION, "do",
        "Executes a series of statements as long as a "
        "given condition is not zero (True)\n"
        "Do...Loop While loops can be nested to any level.\n"
        "Each Loop While matches the most recent Do.\n"
        "The Do...Loop While statement syntax has these parts:\n"
        "- condition  :	 Numeric expression or string expression "
        "that evaluates to True or False.\n"
        "- statements : Optional. One or more statements executed while "
        "condition is True.\n"
        "- Exit Do : Optional. Transfers control out of the DO ... LOOP While "
        "block.\n",
        "  Do\n"
        "      [statement(s)]  \n"
        "      [Exit Do]       \n"
        "      [statement(s)]  \n"
        "  Loop While condition" },

    // Exit For [variable]
    { lang_item_t::INSTRUCTION, "exit_for",
        "Causes program execution to jump to the first statement"
        " following the next [variable] statement. ",
        "Exit For [variable]" },

    // Exit While
    { lang_item_t::INSTRUCTION, "exit_while",
        "Causes program execution to jump to the first statement"
        " following the Wend statement.",
        "Exit While" },

    { lang_item_t::INSTRUCTION, "goto",
        "Branches unconditionally out of the normal program sequence "
        "to a specified line number or label",
        "GoTo line-number | label" },

    { lang_item_t::INSTRUCTION, "gosub",
        "Branches unconditionally out of the normal program sequence "
        "to a specified subroutine at line number or label (See also Return)",
        "GoSub line-number | label" },

    { lang_item_t::INSTRUCTION, "return",
        "Returns from subroutine, branching back to the line "
        "following the most recent GoSub statement line (See also GoSub)",
        "Return" },

    { lang_item_t::INSTRUCTION, "locate",
        "Moves the cursor to the specified position on the active screen",
        "Locate row,col" },

    { lang_item_t::INSTRUCTION, "print",
        "Prints list of expressions to standard output\n"
        "If list of expressions is omitted, a blank line is displayed\n"
        "If list of expressions is included, the values of the expressions "
        "are displayed.\nExpressions in the list may be numeric and/or string "
        "expressions, separated by commas or semicolons.\n"
        "A question mark (?) may be used in place of the word Print when it is "
        "the first statement in the immediate-mode editing.\n"
        "Literal strings in the list must be enclosed in double quotation "
        "marks",
        "Print [list of expressions][;]" },

    { lang_item_t::INSTRUCTION, "print#",
        "Prints list of expressions to file (0-standard output)\n"
        "\nSee also Print, Write, Open, Close",
        "Write[#]|Print[#] filenumber, [list of expressions][;]" },

    { lang_item_t::INSTRUCTION, "input",
        "Prompts the user to input data that be will placed into a list of "
        "variables.\n"
        "Prompt-string and the list of variables must be separated by either a "
        "comma or a semicolon\n"
        "The semicolon will print a question mark at the end of the text.\n"
        "The prompt-string must be enclosed in double quotation marks",
        "Input [prompt-string[,|;]] variable1[, variable2, ...]" },

    { lang_item_t::INSTRUCTION, "input#",
        "Inputs data from file that will be placed into a list of variables.\n"
        "The list of variables must be separated by a comma",
        "Input# filenum, variable1[, variable2, ...]\n" },

    { lang_item_t::INSTRUCTION, "read",
        "Reads data from file that will be placed into a variable.\n"
        "Read statement overwrites the destination variable content with data "
        "content.\n"
        "If the variable does not exist, it will be created\n"
        "Using a byte vector variable or a string the size may be \n"
        "any positive integer value.\n"
        "Using plain type variable like integer or float the size \n"
        "must be equal to this variable size\n",
        "Read filenum, variable, size\n" },

    { lang_item_t::INSTRUCTION, "delay",
        "Suspends the execution of the next statement in the application "
        "program for the specified period of time.\n"
        "The expression is evaluated and converted to a number of seconds.\n"
        "\nSee also MDelay",
        "Delay|Sleep expression" },

    { lang_item_t::INSTRUCTION, "mdelay",
        "Suspends the execution of the next statement in the application "
        "program for the specified period of time.\n"
        "The expression is evaluated and converted to a number of "
        "milliseconds.\n"
        "\nSee also Delay",
        "MDelay expression" },

    { lang_item_t::INSTRUCTION, "cls", "Clears the screen", "Cls" },

    { lang_item_t::FUNCTION, "randomize",
        "Initializes the Random number generator with the current time",
        "Randomize" },

    { lang_item_t::INSTRUCTION, "dim",
        "Allocate storage space for variable and array variable\n"
        "An array must be declared in order to be used "
        "and once dimensioned, can be re-dimensioned by using ReDim\n"
        "The Dim statement sets all the elements of the specified "
        "arrays to an initial value of zero or empty string\n"
        "See also: ReDim",
        "Dim var1[(dim1)] [as Type], ..., varN[(dimN)] [As OtherType]\n" },

    { lang_item_t::INSTRUCTION, "redim",
        "Reallocate storage space for array variable\n"
        "An array must be declared in order to be used "
        "and re-dimensioned.\n"
        "The ReDim statement sets all the elements of the specified "
        "arrays to an initial value of zero or empty string\n"
        "See also: Dim",
        "ReDim var1(dim1), var2(dim2), ..., varN(dimN)\n" },

    { lang_item_t::INSTRUCTION, "sub",
        "Define a block of code that performs a single task\n"
        "The block is enclosed by a declaration statement.\n"
        "The procedure defines zero or more parameters, each of which\n"
        "represents a value it expects you to pass to it.\n"
        "See also: Function",
        "Sub subname ( [parameterlist] )\n"
        "   [statement(s)]\n"
        "   [Exit Sub]\n"
        "   [statement(s)]\n"
        "End Sub\n" },

    { lang_item_t::INSTRUCTION, "function",
        "Define a block of code that performs a single task\n"
        "The block is enclosed by a declaration statement.\n"
        "The procedure defines zero or more parameters, each of which\n"
        "represents a value it expects you to pass to it.\n"
        "A function returns a value. This value is assigned to its\n"
        "own function name variable in one or more statements "
        "of the procedure.\n"
        "See also: Sub",
        "Function function_name ( [parameterlist] ) [as Type]\n"
        "   [statement(s)]\n"
        "   [function_name = return-value\n"
        "   [Exit Function]]\n"
        "   [statement(s)]\n"
        "   function_name = return-value\n"
        "End Function\n" },

    { lang_item_t::INSTRUCTION, "let",
        "Assigns the value of an expression to a variable.\n"
        "The keyword Let is optional; that is, the equal sign is sufficient "
        "when assigning an expression to a variable name",
        "[Let] variable=expression\n" },

    { lang_item_t::INSTRUCTION, "beep",
        "Sounds a tone through the computer's speaker", "Beep" },

    { lang_item_t::INSTRUCTION, "open",
        "Establishes input/output (I/O) to a file.\n\n"
        " 'filename' is the name of the file to be opened\n"
        " 'mode'   determines the initial positioning within the file,\n"
        "          and the action to be taken if the file does not exist.\n"
        "  Input - Position to the beginning of the file.\n"
        "          A \"File not found\" error is given if the file does not "
        "exist.\n"
        "  Output - Position to the beginning of the file.\n"
        "           If the file does not exist, one is created.\n"
        "  Append - Position to the end of the file.\n"
        "           If the file does not exist, one is created.\n"
        "  Random - Specifies random input or output mode.\n\n"
        "'access' can be one of the following :\n"
        "  Read\n  Write\n  Read Write\n"
        "\nSee also FOpen, Close, Input#, Print#, Write#, "
        "Seek, FTell, FError, Errno, Errno$",
        "Open filename [For mode][ACCESS access] AS [#]file-number" },

    { lang_item_t::INSTRUCTION, "fopen",
        "Establishes input/output (I/O) to a file.\n\n"
        "'filename' is the name of the file to be opened\n"
        "'mode'     is a string that determines the initial\n"
        "           positioning within the file,\n"
        "           and the action to be taken if the file does not exist.\n\n"
        "Valid mode strings are the following:\n"
        "- \"r\" read: Open file for input operations. The file must exist.\n"
        "- \"w\" write: Create an empty file for output operations.\n"
        "  If a file with the same name already exists, its contents are "
        "discarded\n"
        "  and the file is treated as a new empty file.\n"
        "- \"a\" append: Open file for output at the end of a file.\n"
        "  Output operations always write data at the end of the file, "
        "expanding it.\n"
        "  Repositioning operations are ignored.\n"
        "  The file is created if it does not exist.\n"
        "- \"r+\" read/update: Open a file for update the file must exist.\n"
        "- \"w+\" write/update: Create an empty file and open it for update.\n"
        "  If a file with the same name already exists its contents are "
        "discarded\n"
        "  and the file is treated as a new empty file.\n"
        "- \"a+\" append/update. Open a file for update with all output \n"
        "  operations writing data at the end of the file.\n"
        "  Repositioning operations affects the next input operations,\n"
        "  but output operations move the position back to the end of file.\n"
        "  The file is created if it does not exist.\n"
        "\nSee also Open, Close, Input#, Print#, Write#, "
        "Seek, FTell, FError, Errno, Errno$",
        "FOpen filename, mode, file-number" },


    { lang_item_t::INSTRUCTION, "seek",
        "Moves the file pointer to a specified location\n"
        "position: specifies the number of bytes from initial position\n"
        "origin: specifies the initial position if is assumes one of the "
        "following values\n"
        "  0 - Current position\n"
        "  1 - End of file\n"
        "  2 - Beginning of file\n",
        "Seek filenumber, position, origin" },

    { lang_item_t::INSTRUCTION, "flush",
        "Shall cause any unwritten data for that file "
        "number to be written to the file\n",
        "Flush filenumber" },

    { lang_item_t::INSTRUCTION, "close",
        "Terminates input/output to a file.\n"
        "\nSee also Open, Read, Write, Errno, Errno$",
        "Close [#]file-number" },

    { lang_item_t::INSTRUCTION, "shell", "Executes a shell command\n"
                                         "\nSee also Errno, Errno$",
        "Shell \"command...\"" },

    { lang_item_t::INSTRUCTION, "struct",
        "A structure in nuBASIC is a composite data type\n"
        "consisting of a number elements of other types.\n"
        "\nSee also Dim",
        "Struct <struct_name>\n"
        "   field1[as FieldType1]\n"
        "   field2[as FieldType2]\n"
        "   ...\n"
        "   fieldn[as FieldTypeN]\n"
        "End Struct\n" },


#ifndef TINY_NUBASIC_VER
    { lang_item_t::FUNCTION, "getmousex",
        "Returns the current X position of the mouse. "
        "X is measured in pixels, starting at 0 for "
        "the left side of the working window.",
        "GetMouseX()" },

    { lang_item_t::FUNCTION, "getmousey",
        "Returns the current Y position of the mouse. "
        "Y is measured in pixels, starting at 0 for "
        "the top of the working window.",
        "GetMouseY()" },


    { lang_item_t::FUNCTION, "getmousebtn",
        "Returns whether the given mouse button is held down. "
        "Button values are 1 for left button, 4 for right button, "
        "2 for the middle button. ",
        "GetMouseBtn()" },

    { lang_item_t::FUNCTION, "getsheight",
        "Gets height in pixels of nuBASIC working window client area",
        "GetSHeight()" },

    { lang_item_t::FUNCTION, "getswidth",
        "Gets width in pixels of nuBASIC working window client area",
        "GetSWidth()" },

    { lang_item_t::FUNCTION, "getwindowx",
        "Gets x position of nuBASIC working window", "GetWindowX()" },

    { lang_item_t::FUNCTION, "getwindowy",
        "Gets y position of nuBASIC working window", "GetWindowY()" },

    { lang_item_t::FUNCTION, "getwindowdy",
        "Gets height in pixel of nuBASIC working window", "GetWindowDy()" },

    { lang_item_t::FUNCTION, "getwindowdx",
        "Gets width in pixel of nuBASIC working window", "GetWindowDx()" },

    { lang_item_t::FUNCTION, "msgbox",
        "Displays a modal dialog box containing given title and message\n"
        "Returns zero or positive value if function succeeded, -1 otherwise\n",
        "MsgBox(Title$, Message$)" },

    { lang_item_t::FUNCTION, "playsound",
        "Plays a sound (wav) specified by the given file name\n"
        "Returns zero or positive value if function succeeded, -1 otherwise\n"
        "If flg%=1 the sound is played asynchronously, synchronously "
        "otherwise\n",
        "PlaySound(filename$, flg%)" },

    { lang_item_t::FUNCTION, "movewindow",
        "Changes the position and dimensions of nuBASIC working window\n"
        "If the function succeeds, the return value is nonzero.\n"
        "If the function fails, the return value is zero.\n",
        "MoveWindow(x%,y%,width%,height%)" },

    { lang_item_t::INSTRUCTION, "line",
        "Draws a line on the screen from x1,y1 up to x2,y2.\n"
        "rgb_color may be written as either decimal or hexadecimal."
        "For example: &hff0000 is blue while &h00ff00 is green\n"
        "\nSee also Errno, Errno$",
        "Line x1, y1, x2, y2, rgb_color" },

    { lang_item_t::INSTRUCTION, "setpixel",
        "Draws a pixel on the screen at x,y.\n"
        "rgb_color may be written as either decimal or hexadecimal."
        "For example: &hff0000 is blue while &h00ff00 is green\n"
        "\nSee also Errno, Errno$",
        "SetPixel x, y, rgb_color" },

    { lang_item_t::INSTRUCTION, "getpixel",
        "Retrieves the red, green, blue (RGB) color value of the\n"
        "pixel at the specified coordinates.\n",
        "rgb_color% = GetPixel(x%, y%)" },

    { lang_item_t::INSTRUCTION, "rect",
        "Draws a rectangle frame on the screen.\n"
        "Point x1,y1 is upper-left corner and point x2,y2 is lower-right "
        "corner\n"
        "of the rectangle\n"
        "rgb_color may be written as either decimal or hexadecimal."
        "For example: &hff0000 is blue while &h00ff00 is green\n"
        "\nSee also FillRect",
        "Rect x1, y1, x2, y2, rgb_color" },

    { lang_item_t::INSTRUCTION, "fillrect",
        "Draws a filled rectangle on the screen.\n"
        "Point x1,y1 is upper-left corner and point x2,y2 is lower-right "
        "corner\n"
        "of the rectangle\n"
        "rgb_color may be written as either decimal or hexadecimal."
        "For example: &hff0000 is blue while &h00ff00 is green\n"
        "\nSee also Errno, Errno$",
        "FillRect x1, y1, x2, y2, rgb_color" },

    { lang_item_t::INSTRUCTION, "ellipse",
        "Draws an ellipse on the screen.\n"
        "The center of the ellipse is the center of the "
        "specified bounding rectangle.\n"
        "Point x1,y1 is upper-left corner and point x2,y2 is lower-right "
        "corner\n"
        "of the bounding rectangle.\n"
        "rgb_color may be written as either decimal or hexadecimal."
        "For example: &hff0000 is blue while &h00ff00 is green\n"
        "\nSee also FillEllipse",
        "Ellipse x1, y1, x2, y2, rgb_color" },

    { lang_item_t::INSTRUCTION, "fillellipse",
        "Draws a filled ellipse on the screen.\n"
        "The center of the ellipse is the center of the "
        "specified bounding rectangle.\n"
        "rgb_color may be written as either decimal or hexadecimal."
        "For example: &hff0000 is blue while &h00ff00 is green\n"
        "\nSee also Ellipse",
        "FillEllipse x1, y1, x2, y2, rgb_color" },

    { lang_item_t::INSTRUCTION, "textout",
        "Draws a text on the screen starting at coords x,y\n"
        "rgb_color may be written as either decimal or hexadecimal."
        "For example: &hff0000 is blue while &h00ff00 is green\n"
        "\nSee also Line, Rect, Ellipse, FillRect, FillEllipse",
        "TextOut x, y, string_msg, rgb_color" },

    { lang_item_t::INSTRUCTION, "plotimage", "Loads and draws a bitmap from "
                                             "file on the screen starting at "
                                             "coords x,y\n",
        "PlotImage bitmap$, x, y" },

#endif // ...TINY_NUBASIC_VER

    { lang_item_t::FUNCTION, "quit",
        "Quit nuBASIC interpreter\n",
        "Quit errorcode" }

};


/* -------------------------------------------------------------------------- */

builtin_help_t& builtin_help_t::get_instance()
{
    if (!_instance) {
        _instance = new builtin_help_t();
        assert(_instance);

        builtin_help_t& hmap = *_instance;

        for (const auto& help_item : _help_content) {
            hmap[{ help_item.lang_item, help_item.lang_id }]
                = std::make_pair(help_item.description, help_item.signature);
        }
    }

    return *_instance;
}


/* -------------------------------------------------------------------------- */

std::string builtin_help_t::help(const std::string& s_item)
{
    std::string result;

    if (s_item.empty()) {
        lang_item_t last_sel = lang_item_t::OPERATOR;

        for (const auto& h : *this) {
            if (last_sel != h.first.first) {
                last_sel = h.first.first;

                switch (h.first.first) {
                case lang_item_t::COMMAND:
                    result += "\nCOMMANDs\n";
                    break;

                case lang_item_t::FUNCTION:
                    result += "\n\nFUNCTIONs\n";
                    break;

                case lang_item_t::INSTRUCTION:
                    result += "\n\nINSTRUCTIONs\n";
                    break;

                case lang_item_t::OPERATOR:
                    result += "\n\nOPERATORs\n";
                    break;
                }
            }

            std::string keyword = h.first.second.str();
            enum { KEYWORDFORMATLEN = 12, MAXDESCLEN = 68 };

            while (keyword.size() < KEYWORDFORMATLEN)
                keyword += " ";

            std::string desc = h.second.first;
            int lfpoint = int(desc.find('\n'));
            std::replace(desc.begin(), desc.end(), '\n', ' ');
            bool ellipses = lfpoint > MAXDESCLEN
                || (lfpoint < 0 && desc.size() > MAXDESCLEN);

            if (lfpoint > MAXDESCLEN || lfpoint < 0) {
                lfpoint = MAXDESCLEN;
            }

            result += keyword + desc.substr(0, lfpoint + 1);
            result += ellipses ? "...\n" : "\n";
        }
    }

    else {
        for (int it = 0; it <= static_cast<int>(lang_item_t::OPERATOR); ++it) {
            help_item_t item({ static_cast<lang_item_t>(it), s_item });


            auto i = this->find(item);

            if (i != this->end()) {
                result += "\n" + (*this)[item].second;
                result += "\n\n" + (*this)[item].first + "\n";
            }
        }
    }

    return result;
}


/* -------------------------------------------------------------------------- */

std::string builtin_help_t::apropos(const std::string& s_item)
{
    std::string result;

    if (!s_item.empty()) {
        lang_item_t last_sel = lang_item_t::OPERATOR;

        for (const auto& h : *this) {
            if (last_sel != h.first.first) {
                last_sel = h.first.first;
            }

            std::string keyword = h.first.second.str();
            std::string desc = h.second.first;

            nu::icstring_t k = keyword;
            nu::icstring_t d = desc;

            size_t found_k = k.find(s_item);
            size_t found_d = d.find(s_item);

            if (found_k != std::string::npos || found_d != std::string::npos) {
                enum { KEYWORDFORMATLEN = 12, MAXDESCLEN = 68 };

                while (keyword.size() < KEYWORDFORMATLEN)
                    keyword += " ";

                int lfpoint = int(desc.find('\n'));
                std::replace(desc.begin(), desc.end(), '\n', ' ');
                bool ellipses = lfpoint > MAXDESCLEN
                    || (lfpoint < 0 && desc.size() > MAXDESCLEN);

                if (lfpoint > MAXDESCLEN || lfpoint < 0) {
                    lfpoint = MAXDESCLEN;
                }

                result += keyword + desc.substr(0, lfpoint + 1);
                result += ellipses ? "...\n" : "\n";
            }
        }
    }

    return result;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
