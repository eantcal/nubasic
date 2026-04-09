# Interpreter Internals

← [History and Evolution](History-and-Evolution) | Next: [Building from Source](Building-from-Source)

---

This chapter describes the internal architecture of the nuBASIC interpreter. It is aimed at
developers who want to understand how the interpreter works, contribute to the codebase, or
build upon it for their own language experiments. All classes and files mentioned are in the
`nu` namespace; header files are in `include/` and implementations in `lib/`.

## Contents

- [Main Components](#main-components)
- [A Line-Oriented Interpreter](#a-line-oriented-interpreter)
- [Tokens and the Tokenizer](#tokens-and-the-tokenizer)
- [Token List Container](#token-list-container)
- [Parsing the Code](#parsing-the-code)
- [Expression Parsing in Detail](#expression-parsing-in-detail)
- [The Variant Type](#the-variant-type)
- [Tracing Execution of a Simple Program](#tracing-execution-of-a-simple-program)
- [Extending the Built-in Function Set](#extending-the-built-in-function-set)

---

## Main Components

The interpreter follows the classical pipeline: source text → tokens → abstract syntax tree → execution.

| Component | Class / Type | Key file |
|-----------|-------------|----------|
| **Tokenizer** | `tokenizer_t` | `include/nu_tokenizer.h` |
| **Token** | `token_t` | `include/nu_token.h` |
| **Token List** | `token_list_t` | `include/nu_token_list.h` |
| **Expression Parser** | `expr_parser_t` | `include/nu_expr_parser.h` |
| **Expression node base** | `expr_any_t` | `include/nu_expr_any.h` |
| **Statement Parser** | `statement_parser_t` | `include/parser/nu_statement_parser.h` |
| **Statement base** | `stmt_t` | `include/nu_stmt.h` |
| **Static Program Context** | `prog_ctx_t` | `include/nu_prog_ctx.h` |
| **Runtime Program Context** | `rt_prog_ctx_t` | `include/nu_rt_prog_ctx.h` |
| **Program line map** | `prog_line_t` | `include/nu_program.h` |
| **Interpreter** | `interpreter_t` | `include/nu_interpreter.h` |
| **Variant value** | `variant_t` | `include/nu_variant.h` |
| **Built-in help** | `builtin_help_t` | `include/nu_builtin_help.h` |

- **Tokenizer** — breaks a source line into a flat sequence of typed `token_t` objects.
- **Expression Parser** — reads a token sequence representing an expression and returns an `expr_any_t::handle_t` that can be evaluated on demand.
- **Statement Parser** — builds an executable statement object tree from a complete source line.
- **Static Program Context** (`prog_ctx_t`) — accumulated during the build phase; holds procedure prototypes, structure definitions, label tables, and multi-line construct metadata.
- **Runtime Program Context** (`rt_prog_ctx_t`) — extends `prog_ctx_t` with variable values, the call stack, loop runtime data, file descriptors, the program counter, and function return registers.
- **Interpreter** (`interpreter_t`) — owns the source lines, the parsed program, and implements the CLI command loop, `rebuild()`, `run()`, and the debugger.

---

## A Line-Oriented Interpreter

BASIC is a line-oriented language: the position of a hard line break is syntactically
significant. nuBASIC preserves this throughout the entire pipeline.

Source text is stored in two parallel maps inside `interpreter_t`:

- `_source_line` — maps a `line_num_t` to the raw source string.
- `_prog_line` — maps line numbers to compiled statement objects plus debug information.

During `rebuild()`, each source line is handed to `stmt_parser_t::compile_line()`. The static
context (`prog_ctx_t`) is built at the same time, stitching together multi-line constructs
(`For`/`Next`, `While`/`Wend`, `If`/`ElseIf`/`EndIf`, `Sub`/`Function`, etc.).

At execution time, `run()` iterates over `_prog_line` in key order, calling the `run()` method
of each block statement object. Jump statements (`GoTo`, `GoSub`, `Return`, procedure calls)
modify the runtime program counter directly to redirect execution.

---

## Tokens and the Tokenizer

Each `token_t` carries:

| Attribute | Member | Description |
|-----------|--------|-------------|
| Original text | `_org_id` | Preserves the original capitalisation |
| Normalised text | `_identifier` | Lowercase version, used for keyword matching |
| Token class | `_type` (`tkncl_t` enum) | Category of the token |
| Source position | `_position` | Byte offset within the source line |

The `tkncl_t` enumeration:

```cpp
enum class tkncl_t {
    UNDEFINED,
    BLANK,
    NEWLINE,
    IDENTIFIER,    // keywords and variable names
    INTEGRAL,      // integer literals
    REAL,          // floating-point literals
    OPERATOR,      // +  -  *  /  ^  =  <>  <  <=  >  >=  \  &
    SUBEXP_BEGIN,  // (
    SUBEXP_END,    // )
    STRING_LITERAL,
    STRING_COMMENT,
    SUBSCR_BEGIN,  // [
    SUBSCR_END,    // ]
    LINE_COMMENT   // '  or  Rem
};
```

The Statement Parser uses `_type` to make fast branching decisions — an `IDENTIFIER` token
whose `_identifier` is `"print"` immediately routes to `parse_print()`.

---

## Token List Container

`token_list_t` wraps a `std::deque<token_t>` and adds:

- Efficient removal from either end (front-popping as tokens are consumed)
- Marker insertion: `SUBEXP_BEGIN`/`SUBEXP_END` tokens delimit sub-expressions after operator-precedence reordering
- Helpers to peek at the front token, check emptiness, and skip blanks

---

## Parsing the Code

**Expression Parser (`expr_parser_t`)** — analyses a single expression and returns an
`expr_any_t::handle_t`:

```cpp
expr_any_t::handle_t compile(expr_tknzr_t& tknzr);
expr_any_t::handle_t compile(token_list_t tl, size_t expr_pos);
```

**Statement Parser (`stmt_parser_t`)** — analyses one complete source line:

```cpp
stmt_t::handle_t compile_line(prog_ctx_t& ctx, const std::string& source_line);
stmt_t::handle_t parse_block(prog_ctx_t& ctx, token_list_t& tl, ...);
stmt_t::handle_t parse_stmt (prog_ctx_t& ctx, token_list_t& tl);
```

`parse_block()` loops while the token list is non-empty, calling `parse_stmt()` on each
iteration. `parse_stmt()` inspects the first token and dispatches to a dedicated
`parse_xxx()` method for each statement keyword.

**CLI Parser (inside `interpreter_t`)** — handles interpreter-specific commands (`List`,
`Run`, `Load`, `Save`, `New`, `Help`, …) via `interpreter_t::exec_command()`.

---

## Expression Parsing in Detail

The central challenge is operator precedence: `2 + 4 * 17` must evaluate to
`2 + (4 * 17) = 70`, not `(2 + 4) * 17 = 102`.

The Expression Parser resolves this in three steps.

**Step 1 — Tokenise.** Feed the expression string to a `tokenizer_t`.

**Step 2 — Reorder by precedence.** Insert `SUBEXP_BEGIN`/`SUBEXP_END` markers to encode
the following precedence order (highest to lowest):

1. Unary identity and negation (`+`, `-`)
2. Exponentiation (`^`)
3. Multiplication and floating-point division (`*`, `/`)
4. Integer division (`\`, `Div`)
5. Modulus (`Mod`)
6. Addition and subtraction (`+`, `-`)
7. Comparison (`=`, `<>`, `<`, `<=`, `>`, `>=`)
8. Logical and bitwise (`And`, `Or`, `Xor`, and bitwise variants)

**Step 3 — Build the syntax tree.** `expr_parser_t::parse()` recursively constructs a tree
of `expr_any_t` nodes. For `2 + 4 * 17`:

```
        binary_expression(+)
              /          \
        literal          binary_expression(*)
        (integer, 2)           /        \
                          literal      literal
                          (int, 4)     (int, 17)
```

The `expr_any_t` hierarchy:

- **Binary expression** — one operator, two child nodes
- **Unary expression** — one operator, one child
- **Function call** — built-in or user-defined, with an argument list of `expr_any_t` nodes
- **Variable** — name looked up in the runtime context at `eval()` time
- **Literal constant** — integer, floating-point, or string value embedded at compile time

The base class interface:

```cpp
class expr_any_t {
public:
    using handle_t    = std::shared_ptr<expr_any_t>;
    using func_args_t = std::vector<expr_any_t::handle_t>;

    virtual variant_t eval(rt_prog_ctx_t& ctx) const = 0;
    virtual bool      empty()  const noexcept = 0;
    virtual std::string name() const noexcept = 0;
    virtual ~expr_any_t() {}
};
```

---

## The Variant Type

`variant_t` (`include/nu_variant.h`) is the universal value type. Every expression evaluates
to a `variant_t`; every variable stores one.

| Enumerator | Meaning |
|------------|---------|
| `UNDEFINED` | Uninitialised / not-set |
| `INTEGER` | 32-bit signed integer |
| `DOUBLE` | Double-precision floating point |
| `STRING` | Text string |
| `BYTEVECTOR` | Raw byte array |
| `BOOLEAN` | Boolean value |
| `STRUCT` | User-defined structure |
| `OBJECT` | Object handle (for GUI/external objects) |
| `ANY` | Wildcard (used in procedure signatures) |

Internally, scalar values are stored in a
`std::vector<std::variant<string_t, integer_t, double_t>>`. The vector has size 1 for
scalars and size *n* for array variables. Structure instances carry field data in a separate
`std::vector<struct_data_t>`.

---

## Tracing Execution of a Simple Program

The complete path from keyboard input to printed output for:

```basic
10 Print 2 + 4 * 17
```

**1. Entry.** The user types `RUN`. `interpreter_t::exec_command("RUN")` is called.

**2. Build phase.** `rebuild()` iterates over `_source_line`. For line 10 it calls
`stmt_parser_t::compile_line(ctx, "10 PRINT 2+4*17")`. The tokenizer produces:

```
{ ("print", IDENTIFIER), (" ", BLANK),
  ("2", INTEGRAL), ("+", OPERATOR),
  ("4", INTEGRAL), ("*", OPERATOR), ("17", INTEGRAL) }
```

`parse_stmt()` sees `"print"` → dispatches to `parse_print()`.

**3. Building the Print statement.** `parse_print()` calls the Expression Parser on
`2 + 4 * 17`. The parser builds the tree:

```
binary_expression(+)
 ├── literal(integer, 2)
 └── binary_expression(*)
      ├── literal(integer, 4)
      └── literal(integer, 17)
```

The root handle is wrapped in a `stmt_print_t` object stored in `_prog_line[10]`.

**4. Run phase.** `run()` calls `stmt_print_t::run(ctx)` for line 10.

**5. Evaluation.** `stmt_print_t::run()` calls `node->eval(ctx)` on the root:
- `literal(2)` returns `variant_t(INTEGER, 2)`
- `binary_expression(*)` evaluates to `variant_t(INTEGER, 68)` (4 × 17)
- `binary_expression(+)` adds them: `variant_t(INTEGER, 70)`

**6. Output.** The variant is converted to `"70"` and written to the output stream.

---

## Extending the Built-in Function Set

Adding a new built-in function requires touching exactly three source files. The example
is a 1-D convolution function `Conv`.

| File | Change |
|------|--------|
| `lib/nu_global_function_tbl.cc` | 1. Add the C++ algorithm. 2. Add the functor. 3. Register with `fmap["conv"] = conv_functor;` |
| `lib/nu_builtin_help.cc` | Add one `help_content_t` entry to `_help_content[]` |
| `lib/nu_reserved_keywords.cc` | Add `"conv"` to the `list()` set |

### Step 1 — Implement the C++ Algorithm

```cpp
template <typename T>
std::vector<T> conv(const std::vector<T>& v1, const std::vector<T>& v2)
{
    const int n = int(v1.size());
    const int m = int(v2.size());
    const int k = n + m - 1;
    std::vector<T> w(k, T());

    for (int i = 0; i < k; ++i) {
        const int jmn = (i >= m - 1) ? i - (m - 1) : 0;
        const int jmx = (i < n - 1)  ? i            : n - 1;
        for (int j = jmn; j <= jmx; ++j)
            w[i] += v1[j] * v2[i - j];
    }
    return w;
}
```

### Step 2 — Write the Functor

Every built-in function is registered as a C++ callable with the signature:

```cpp
variant_t functor_name(
    rt_prog_ctx_t&        ctx,
    const std::string&    name,
    const nu::func_args_t& args
);
```

Arguments are evaluated in order (`args[0]`, `args[1]`, …); each `eval()` call may update
variables or advance I/O state, so the order is semantically significant.

### Step 3 — Register the Functor

Inside `global_function_tbl_t::get_instance()`:

```cpp
fmap["conv"] = conv_functor;
```

The key is a case-insensitive string (`icstring_t`), so users may write `Conv`, `conv`,
or `CONV` interchangeably.

### Step 4 — Add Inline Help

In `lib/nu_builtin_help.cc`, add to `_help_content[]`:

```cpp
{ lang_item_t::FUNCTION, "conv",
    "Returns a vector of Double as result of convolution of 2 given vectors of numbers",
    "Conv( v1, v2 [, count1, count2 ] )" },
```

### Step 5 — Add to the Reserved Keyword List

In `lib/nu_reserved_keywords.cc`, add `"conv"` (lowercase) to the set returned by
`reserved_keywords_t::list()`. This enables syntax highlighting and F12 auto-completion
in the IDE.

---

← [History and Evolution](History-and-Evolution) | Next: [Building from Source](Building-from-Source)
