//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_interpreter.h"
#include "nu_about.h"
#include "nu_builtin_help.h"
#include "nu_builtin_registry.h"
#include "nu_examples_paths.h"
#include "nu_expr_literal.h"
#include "nu_os_console.h"
#include "nu_os_std.h"
#include "nu_program.h"
#include "nu_stdlib.h"
#include "nu_stmt_block.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdarg>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <string.h>
#include <string>

#ifdef _WIN32
#include "nu_winconsole_api.h"
#endif

/* -------------------------------------------------------------------------- */

namespace nu {

namespace {

    namespace fs = std::filesystem;

    static std::string to_lower_ascii(std::string s)
    {
        for (char& c : s)
            c = char(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    static std::string trim_trailing_path_separators(std::string s)
    {
        while (s.size() > 1 && (s.back() == '/' || s.back() == '\\'))
            s.pop_back();
        return s;
    }

    static bool can_open_program_file(const fs::path& path)
    {
        FILE* f = fopen(path.string().c_str(), "r");
        if (!f)
            return false;

        fclose(f);
        return true;
    }

    /** Try arg; then examples_dir/name and name.bas. */
    static std::string resolve_program_file(const std::string& arg)
    {
        if (can_open_program_file(arg))
            return arg;

        const std::string ex = examples_directory();
        if (ex.empty())
            return "";

        fs::path p = fs::path(ex) / arg;
        if (can_open_program_file(p))
            return p.string();

        if (arg.find('.') == std::string::npos) {
            p = fs::path(ex) / (arg + ".bas");
            if (can_open_program_file(p))
                return p.string();
        }

        return "";
    }

    static bool source_line_has_blocking_input(std::string line)
    {
        bool in_string = false;
        std::string code;
        code.reserve(line.size());

        for (const auto ch : line) {
            if (ch == '"') {
                in_string = !in_string;
            } else if (ch == '\'' && !in_string) {
                break;
            }

            code += static_cast<char>(
                std::tolower(static_cast<unsigned char>(ch)));
        }

        const auto skip_spaces = [&](size_t pos) {
            while (pos < code.size()
                && std::isspace(static_cast<unsigned char>(code[pos]))) {
                ++pos;
            }
            return pos;
        };

        size_t pos = skip_spaces(0);
        while (pos < code.size()
            && std::isdigit(static_cast<unsigned char>(code[pos]))) {
            ++pos;
        }
        pos = skip_spaces(pos);

        if (code.compare(pos, 5, "input") == 0) {
            const auto next = pos + 5;
            return next >= code.size()
                || std::isspace(static_cast<unsigned char>(code[next]))
                || code[next] == '$';
        }

        return code.find("input$") != std::string::npos;
    }

    static std::string normalize_source_name(const std::string& source_name)
    {
        if (source_name.empty())
            return "<anonymous>";

        if (!source_name.empty() && source_name.front() == '<'
            && source_name.back() == '>') {
            return source_name;
        }

        std::error_code ec;
        const auto canon = fs::weakly_canonical(fs::path(source_name), ec);
        return ec ? source_name : canon.string();
    }

    static std::string trim_left_ascii(std::string s)
    {
        const auto pos = s.find_first_not_of(" \t");
        return pos == std::string::npos ? std::string() : s.substr(pos);
    }

    static void trim_right_ascii(std::string& s)
    {
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) {
            s.pop_back();
        }
    }

    static bool strip_line_continuation(
        const std::string& line, std::string& stripped)
    {
        bool in_string = false;
        size_t code_end = line.size();

        for (size_t idx = 0; idx < line.size(); ++idx) {
            if (line[idx] == '"') {
                in_string = !in_string;
            } else if (line[idx] == '\'' && !in_string) {
                code_end = idx;
                break;
            }
        }

        size_t end = code_end;
        while (end > 0 && (line[end - 1] == ' ' || line[end - 1] == '\t')) {
            --end;
        }

        if (end == 0 || line[end - 1] != '_') {
            stripped = line;
            return false;
        }

        stripped = line.substr(0, end - 1);
        trim_right_ascii(stripped);
        return true;
    }

    static void append_logical_source_part(
        std::string& logical_line, const std::string& part)
    {
        if (logical_line.empty()) {
            logical_line = part;
            return;
        }

        logical_line += " ";
        logical_line += trim_left_ascii(part);
    }

} // namespace


/* -------------------------------------------------------------------------- */

source_id_t source_registry_t::intern(const std::string& source_name)
{
    const auto it = _ids_by_name.find(source_name);
    if (it != _ids_by_name.end())
        return it->second;

    const auto next_id = source_id_t(_names_by_id.size() + 1);
    _ids_by_name.insert(std::make_pair(source_name, next_id));
    _names_by_id.push_back(source_name);
    return next_id;
}


/* -------------------------------------------------------------------------- */

source_id_t source_registry_t::find(
    const std::string& source_name) const noexcept
{
    const auto it = _ids_by_name.find(source_name);
    return it == _ids_by_name.end() ? 0 : it->second;
}


/* -------------------------------------------------------------------------- */

const std::string& source_registry_t::lookup(
    source_id_t source_id) const noexcept
{
    static const std::string empty;

    if (source_id == 0 || source_id > _names_by_id.size())
        return empty;

    return _names_by_id[source_id - 1];
}


/* -------------------------------------------------------------------------- */

void source_registry_t::clear()
{
    _ids_by_name.clear();
    _names_by_id.clear();
}

// fprintf wrapper: routes to GDI console when active, otherwise normal fprintf
static void console_fprintf(FILE* fp, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    if (nu_winconsole_is_active() && !nu::_os_get_hybrid_stdio()) {
        char buf[8192];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        nu_winconsole_write(buf);
        return;
    }
#endif
    vfprintf(fp, fmt, args);
    va_end(args);
}

static void quote_string(
    std::string& str, const char qt = '\"', const std::string escape_s = "\\")
{
    const std::string s = str;
    str.clear();

    for (auto it = s.cbegin(); it != s.cend(); ++it) {
        if (*it == qt)
            str += escape_s;

        str += *it;
    }
}


/* -------------------------------------------------------------------------- */

interpreter_t::interpreter_t(FILE* std_output, FILE* std_input)
    : _prog(new program_t(_prog_line, _prog_ctx, true))
    , _prog_ctx(*_prog, std_output, std_input, _source_line)
    , _parser()
    , _stdout_ptr(std_output)
    , _stdin_ptr(std_input)
{
    signal_mgr_t::instance().register_handler(event_t::BREAK, this);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::notify(const event_t& ev)
{
    _break_event = ev == event_t::BREAK;

    return _ignore_break_event ? false : _break_event;
}


/* -------------------------------------------------------------------------- */

interpreter_t::~interpreter_t()
{
    signal_mgr_t::instance().unregister_handler(event_t::BREAK, this);
    delete _prog;
}


/* -------------------------------------------------------------------------- */

std::string interpreter_t::skip_blank(expr_tknzr_t& tz, nu::token_t& t)
{
    std::string blank;

    while (!tz.eol() && t.type() == tkncl_t::BLANK) {
        if (t.type() == tkncl_t::BLANK) {
            blank += t.org_id();
        }

        t = tz.next();
    }

    if (t.type() == tkncl_t::BLANK) {
        blank += t.org_id();
    }

    return blank;
}


/* -------------------------------------------------------------------------- */

std::string interpreter_t::version()
{
    using namespace about;

    char buf[4096] = { 0 };

    snprintf(buf, sizeof(buf) - 1,
        "%s - %s\n" // prog ver
        "(c) %s <%s>" //(c) author <mail>
        " - %s\n" // - <year> <CR>
        "%s\n" // <homepage>  <CR>
        "%s\n", // <descr>  <CR>
        progname, about::version, author, contacts, copyright, homepage,
        description);

    return buf;
}


/* -------------------------------------------------------------------------- */

void interpreter_t::clear_rtdata() { get_rt_ctx().clear_rtdata(); }


/* -------------------------------------------------------------------------- */

void interpreter_t::clear_all()
{
    cleanup_builtin_modules_runtime_state();

    _prog_line.clear();
    _source_line.clear();
    _source_locations.clear();
    _source_registry.clear();
    _included_files.clear();
    get_rt_ctx().clear_rtdata();
    get_rt_ctx().clear_metadata();
    _breakpoints.clear();
}


/* -------------------------------------------------------------------------- */

void interpreter_t::rebuild(runnable_t::line_num_t to_num)
{
    cleanup_builtin_modules_runtime_state();

    // Clear precompiled program
    _prog_line.clear();

    // Clear all labels, program contexts including get, array, etc...
    get_rt_ctx().clear_rtdata();
    get_rt_ctx().clear_metadata();

    // Re-build program
    for (const auto& sl : _source_line) {
        if (to_num <= 0 || sl.first <= to_num)
            update_program(sl.second, sl.first);
    }
}


/* -------------------------------------------------------------------------- */

void interpreter_t::erase_line(runnable_t::line_num_t num)
{
    _prog_line.erase(num);
    _source_line.erase(num);
    _source_locations.erase(num);
}


//----------------------------------------------------------------------

void interpreter_t::renum_line(std::string& line, const renum_tbl_t& renum_tbl)
{
    std::string new_line;
    tokenizer_t tknzr(line);

    enum class st_t {
        EXPECTED_GOXX,
        EXPECTED_TO_OR_SUB,
        EXPECTED_LINENUM,
    } state
        = st_t::EXPECTED_GOXX;

    while (!tknzr.eol()) {
        token_t t = tknzr.next();

        if (t.type() == tkncl_t::BLANK) {
            new_line += t.org_id();
            continue;
        }

        if (t.type() == tkncl_t::STRING_COMMENT) {
            new_line += t.org_id();

            while (!tknzr.eol()) {
                token_t t = tknzr.next();
                new_line += t.org_id();
            }

            break;
        }

        if (t.type() == tkncl_t::STRING_LITERAL) {

            auto id = t.org_id();
            quote_string(id);
            new_line += "\"" + id + "\"";

            continue;
        }

        switch (state) {
        case st_t::EXPECTED_GOXX:
            if (t.type() == tkncl_t::IDENTIFIER) {

                if (t.identifier() == "goto") {
                    state = st_t::EXPECTED_LINENUM;
                } else if (t.identifier() == "gosub") {
                    state = st_t::EXPECTED_LINENUM;
                } else if (t.identifier() == "go") {
                    state = st_t::EXPECTED_TO_OR_SUB;
                } else {
                    state = st_t::EXPECTED_GOXX;
                }
            }

            new_line += t.org_id();

            break;

        case st_t::EXPECTED_TO_OR_SUB:
            state = t.identifier() == "to" || t.identifier() == "sub"
                ? st_t::EXPECTED_LINENUM
                : st_t::EXPECTED_GOXX;

            new_line += t.org_id();

            break;

        case st_t::EXPECTED_LINENUM:
            state = st_t::EXPECTED_GOXX;

            if (t.type() == tkncl_t::INTEGRAL) {
                prog_pointer_t::line_number_t ln = 0;

                try {
                    ln = nu::stoi(t.org_id());
                } catch (...) {
                    ln = 0;
                }

                auto i = renum_tbl.find(ln);

                new_line += i != renum_tbl.end() ? nu::to_string(i->second)
                                                 : t.org_id();
            } else {
                new_line += t.org_id();
            }

            break;

        default:
            new_line += t.org_id();
            break;
        }
    }

    line = std::move(new_line);
}


//----------------------------------------------------------------------

void interpreter_t::renum_prog(runnable_t::line_num_t step)
{
    if (step < 1)
        step = 10;

    runnable_t::line_num_t ln = step;

    source_line_t renumered_source;
    std::map<runnable_t::line_num_t, source_location_t> renumbered_locations;
    renum_tbl_t renum_tbl;

    for (auto line : _source_line) {
        renumered_source.insert(std::make_pair(ln, line.second));
        auto loc = _source_locations.find(line.first);
        if (loc != _source_locations.end()) {
            renumbered_locations.insert(std::make_pair(ln, loc->second));
        }
        renum_tbl.insert(std::make_pair(line.first, ln));
        ln += step;
    }

    for (auto i = renumered_source.begin(); i != renumered_source.end(); ++i) {
        renum_line(i->second, renum_tbl);
    }

    // Replace renumbered breakpoints
    breakpoint_tbl_t breakpoints;

    for (const auto& rl : renum_tbl) {
        auto i = _breakpoints.find(rl.first);

        if (i != _breakpoints.end()) {
            breakpoints.insert(std::make_pair(rl.second, i->second));
        }
    }

    _breakpoints = breakpoints;

    _source_line = std::move(renumered_source);
    _source_locations = std::move(renumbered_locations);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::update_program(const std::string& line, int ln)
{
    tokenizer_t ltknzr(line);

    bool empty_line = ltknzr.eol();

    if (ln < 1 && empty_line) {
        return false;
    }

    nu::token_t token(ltknzr.next());
    skip_blank(ltknzr, token);

    runnable_t::line_num_t line_num = 0;
    int pos = 0;

    if (ln > 0) {
        line_num = ln;
    } else {
        try {
            line_num = nu::stoi(token.identifier());
        } catch (std::exception&) {
            return false;
        }

        pos = int(ltknzr.tell());
        skip_blank(ltknzr, token);

        // If you just typed the line number...
        if (ltknzr.eol()) {
            // It means "remove that line"
            erase_line(line_num);
            rebuild();

            return true;
        }
    }

    std::string code = line.substr(pos, line.size() - pos);

    get_rt_ctx().compiletime_pc.set(line_num, 0);

    // compile may throw exceptions....
    tokenizer_t tknzr(code, pos);
    auto h = _parser.compile_line(tknzr, get_rt_ctx());

    _prog_line[line_num].first = h;

    if (ln == 0 && !code.empty() && code.c_str()[0] == ' ') {
        code = code.substr(1, code.size() - 1);
    }

    _source_line[line_num] = code;

    // Set breakpoint if any
    auto it = _breakpoints.find(line_num);

    if (it != _breakpoints.end()) {
        _prog_line[line_num].second.break_point = true;
        _prog_line[line_num].second.condition_stmt = it->second.condition_stmt;
        _prog_line[line_num].second.condition_str = it->second.condition_str;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

source_id_t interpreter_t::intern_source_name(const std::string& source_name)
{
    return _source_registry.intern(normalize_source_name(source_name));
}


/* -------------------------------------------------------------------------- */

void interpreter_t::set_source_location(runnable_t::line_num_t program_line,
    source_id_t source_id, runnable_t::line_num_t source_line) noexcept
{
    if (source_id == 0 || source_line <= 0) {
        _source_locations.erase(program_line);
        return;
    }

    _source_locations[program_line] = { source_id, source_line };
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::try_get_source_location(runnable_t::line_num_t program_line,
    source_location_t& location) const noexcept
{
    const auto it = _source_locations.find(program_line);
    if (it == _source_locations.end())
        return false;

    location = it->second;
    return true;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::try_get_program_line(const std::string& source_name,
    runnable_t::line_num_t source_line,
    runnable_t::line_num_t& program_line) const noexcept
{
    const auto source_id
        = _source_registry.find(normalize_source_name(source_name));

    if (source_id == 0 || source_line <= 0)
        return false;

    for (const auto& entry : _source_locations) {
        if (entry.second.source_id == source_id
            && entry.second.source_line == source_line) {
            program_line = entry.first;
            return true;
        }
    }

    return false;
}


/* -------------------------------------------------------------------------- */

source_location_t interpreter_t::get_cur_source_location() const noexcept
{
    source_location_t location;
    const auto line = get_cur_line_n();
    try_get_source_location(line, location);
    return location;
}


/* -------------------------------------------------------------------------- */

const std::string& interpreter_t::lookup_source_name(
    source_id_t source_id) const noexcept
{
    return _source_registry.lookup(source_id);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::save(const std::string& filepath)
{
    FILE* f = fopen(filepath.c_str(), "w+b");

    if (!f) {
        return false;
    }

    bool ret = true;

    for (const auto& line : _source_line) {
        // If program contains explicit reference to a line number (e.g. goto
        // or gosub statement has a line number parameter instead of label)
        // line numbers will be saved into the program file
        if (get_rt_ctx().prog_label.get_explicit_line_reference_mode()
            && 0 > fprintf(f, "%u ", line.first)) {
            ret = false;
            break;
        }

        if (0 > int(fwrite(line.second.c_str(), line.second.size(), 1, f))) {
            ret = false;
            break;
        }

        if (0 > int(fwrite("\n", 1, 1, f))) {
            ret = false;
            break;
        }
    }

    fclose(f);

    return ret;
}


/* -------------------------------------------------------------------------- */

std::string interpreter_t::read_line(FILE* f)
{
    std::string line;

    while (ferror(f) == 0) {
        int c = fgetc(f);

        if (c == EOF)
            break;

        if (c == '\n')
            break;

        // Cast to unsigned char so that UTF-8 bytes (>= 0x80) are not
        // misinterpreted as negative values by the signed-char comparison,
        // which previously caused them to be silently dropped.
        if (static_cast<unsigned char>(c) >= 32)
            line.push_back(static_cast<char>(c));
    }

    return line;
}


/* -------------------------------------------------------------------------- */

std::string interpreter_t::read_line(std::stringstream& ss)
{
    std::string line;

    while (!ss.bad()) {
        char c(0);

        ss >> std::noskipws >> c;

        if (ss.fail())
            break;

        if (c == '\n')
            break;

        // Cast to unsigned char so that UTF-8 bytes (>= 0x80) are not
        // misinterpreted as negative values by the signed-char comparison.
        if (static_cast<unsigned char>(c) >= 32)
            line.push_back(c);
    }

    return line;
}


/* -------------------------------------------------------------------------- */

// Parse an Include directive from a source line.
// Accepts:  Include "path"   or  #Include "path"  (case-insensitive).
// Returns the quoted path, or empty string if the line is not an include.
static std::string parse_include_directive(const std::string& line)
{
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos)
        return "";

    std::string s = line.substr(start);
    if (s.empty())
        return "";

    // Strip optional leading '#'
    if (s[0] == '#')
        s = s.substr(1);

    // Case-insensitive "include" check
    if (s.size() < 7)
        return "";
    std::string kw = s.substr(0, 7);
    std::transform(kw.begin(), kw.end(), kw.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    if (kw != "include")
        return "";

    s = s.substr(7);
    start = s.find_first_not_of(" \t");
    if (start == std::string::npos || s[start] != '"')
        return "";

    s = s.substr(start + 1);
    size_t end = s.find('"');
    if (end == std::string::npos)
        return "";

    return s.substr(0, end);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::load_with_includes(FILE* f, int& ln,
    const std::string& base_dir, const std::string& source_name, int depth)
{
    if (depth > 64)
        return false; // guard against infinite include recursion

    const auto source_id = intern_source_name(source_name);
    int source_line = 0;

    bool old_format = false;

    // Detect format from the first non-shebang line
    {
        std::string first = read_line(f);
        ++source_line;
        if (!first.empty() && first.size() >= 2 && first.substr(0, 2) == "#!") {
            first = read_line(f);
            ++source_line;
        }

        if (!first.empty()) {
            tokenizer_t ltknzr(first);
            if (!ltknzr.eol()) {
                nu::token_t token(ltknzr.next());
                skip_blank(ltknzr, token);
                if (token.type() == tkncl_t::INTEGRAL)
                    old_format = true;
            }
        }
    }

    fseek(f, 0, SEEK_SET);
    source_line = 0;
    std::string pending_logical_line;
    int pending_source_line = 0;

    while (!feof(f) && ferror(f) == 0) {
        std::string line = read_line(f);
        ++source_line;
        int logical_source_line = source_line;

        if (!line.empty() && ln == 0) {
            // skip shebang on very first line of the top-level file
            if (line.size() > 1 && line.substr(0, 2) == "#!") {
                line = read_line(f);
                ++source_line;
            }
        }

        if (line.empty() && feof(f))
            break;

        if (line.empty() && ferror(f))
            return false;

        std::string continued_part;
        const bool continued
            = !old_format && strip_line_continuation(line, continued_part);

        if (continued || pending_source_line != 0) {
            if (pending_source_line == 0)
                pending_source_line = source_line;

            append_logical_source_part(
                pending_logical_line, continued ? continued_part : line);

            if (continued)
                continue;

            line = pending_logical_line;
            pending_logical_line.clear();
            logical_source_line = pending_source_line;
            pending_source_line = 0;
        }

        // Only process Include directives in new-format files
        if (!old_format) {
            std::string inc_path = parse_include_directive(line);
            if (!inc_path.empty()) {
                if (find_builtin_module(inc_path) != nullptr) {
                    if (!update_program("Using " + inc_path, ++ln))
                        return false;

                    _source_line[ln] = line;
                    set_source_location(get_rt_ctx().compiletime_pc.get_line(),
                        source_id, logical_source_line);
                    continue;
                }

                // Resolve relative to the including file's directory
                if (!base_dir.empty() && !fs::path(inc_path).is_absolute()) {
                    inc_path = (fs::path(base_dir) / inc_path).string();
                }

                // Skip files that have already been included (include guard).
                if (!_included_files.insert(normalize_source_name(inc_path))
                        .second)
                    continue;

                FILE* inc_f = fopen(inc_path.c_str(), "r");
                if (!inc_f)
                    return false;

                std::string inc_base
                    = fs::path(inc_path).parent_path().string();
                const bool saved_in_include = _prog_ctx.in_include_file;
                _prog_ctx.in_include_file = true;
                bool ok = load_with_includes(
                    inc_f, ln, inc_base, inc_path, depth + 1);
                _prog_ctx.in_include_file = saved_in_include;
                fclose(inc_f);
                if (!ok)
                    return false;

                continue;
            }
        }

        if ((!old_format || !line.empty())
            && !update_program(line, old_format ? 0 : ++ln))
            return false;

        if (!old_format || !line.empty()) {
            set_source_location(get_rt_ctx().compiletime_pc.get_line(),
                source_id, logical_source_line);
        }
    }

    if (pending_source_line != 0
        && !update_program(pending_logical_line, ++ln)) {
        return false;
    }

    if (pending_source_line != 0) {
        set_source_location(get_rt_ctx().compiletime_pc.get_line(), source_id,
            pending_source_line);
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::load_with_includes(std::stringstream& source, int& ln,
    const std::string& base_dir, const std::string& source_name, int depth)
{
    if (depth > 64)
        return false; // guard against infinite include recursion

    const auto source_id = intern_source_name(source_name);
    int source_line = 0;

    bool old_format = false;

    // Detect format from the first non-shebang line
    {
        std::string first = read_line(source);
        ++source_line;
        if (!first.empty() && first.size() >= 2 && first.substr(0, 2) == "#!") {
            first = read_line(source);
            ++source_line;
        }

        if (!first.empty()) {
            tokenizer_t ltknzr(first);
            if (!ltknzr.eol()) {
                nu::token_t token(ltknzr.next());
                skip_blank(ltknzr, token);
                if (token.type() == tkncl_t::INTEGRAL)
                    old_format = true;
            }
        }
    }

    source.clear();
    source.seekg(0, std::ios::beg);
    source_line = 0;
    std::string pending_logical_line;
    int pending_source_line = 0;

    while (!source.eof() && !source.bad()) {
        std::string line = read_line(source);
        ++source_line;
        int logical_source_line = source_line;

        if (!line.empty() && ln == 0) {
            // skip shebang on very first line of the top-level source
            if (line.size() > 1 && line.substr(0, 2) == "#!") {
                line = read_line(source);
                ++source_line;
            }
        }

        if (line.empty() && source.eof())
            break;

        if (line.empty() && source.bad())
            return false;

        std::string continued_part;
        const bool continued
            = !old_format && strip_line_continuation(line, continued_part);

        if (continued || pending_source_line != 0) {
            if (pending_source_line == 0)
                pending_source_line = source_line;

            append_logical_source_part(
                pending_logical_line, continued ? continued_part : line);

            if (continued)
                continue;

            line = pending_logical_line;
            pending_logical_line.clear();
            logical_source_line = pending_source_line;
            pending_source_line = 0;
        }

        // Only process Include directives in new-format sources
        if (!old_format) {
            std::string inc_path = parse_include_directive(line);
            if (!inc_path.empty()) {
                if (find_builtin_module(inc_path) != nullptr) {
                    if (!update_program("Using " + inc_path, ++ln))
                        return false;

                    _source_line[ln] = line;
                    set_source_location(get_rt_ctx().compiletime_pc.get_line(),
                        source_id, logical_source_line);
                    continue;
                }

                // Resolve relative to the including source's directory
                if (!base_dir.empty() && !fs::path(inc_path).is_absolute()) {
                    inc_path = (fs::path(base_dir) / inc_path).string();
                }

                // Skip files that have already been included (include guard).
                if (!_included_files.insert(normalize_source_name(inc_path))
                        .second)
                    continue;

                FILE* inc_f = fopen(inc_path.c_str(), "r");
                if (!inc_f)
                    return false;

                std::string inc_base
                    = fs::path(inc_path).parent_path().string();
                const bool saved_in_include = _prog_ctx.in_include_file;
                _prog_ctx.in_include_file = true;
                bool ok = load_with_includes(
                    inc_f, ln, inc_base, inc_path, depth + 1);
                _prog_ctx.in_include_file = saved_in_include;
                fclose(inc_f);
                if (!ok)
                    return false;

                continue;
            }
        }

        if ((!old_format || !line.empty())
            && !update_program(line, old_format ? 0 : ++ln))
            return false;

        if (!old_format || !line.empty()) {
            set_source_location(get_rt_ctx().compiletime_pc.get_line(),
                source_id, logical_source_line);
        }
    }

    if (pending_source_line != 0
        && !update_program(pending_logical_line, ++ln)) {
        return false;
    }

    if (pending_source_line != 0) {
        set_source_location(get_rt_ctx().compiletime_pc.get_line(), source_id,
            pending_source_line);
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::load(const std::string& filepath)
{
    FILE* f = fopen(filepath.c_str(), "r");
    if (!f)
        return false;

    _included_files.clear();
    const auto normalized = normalize_source_name(filepath);
    std::string base_dir = fs::path(normalized).parent_path().string();
    int ln = 0;
    bool ok = load_with_includes(f, ln, base_dir, normalized);
    fclose(f);
    return ok;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::load(std::stringstream& source, const std::string& base_dir,
    const std::string& source_name)
{
    int ln = 0;
    return load_with_includes(source, ln, base_dir, source_name);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::load(FILE* f)
{
    int ln = 0;
    bool ok = load_with_includes(f, ln, "", "<stdin>");
    fclose(f);
    return ok;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::append(std::stringstream& is, int& n_of_lines)
{
    while (!is.eof() && !is.bad()) {
        std::string line = read_line(is);

        if (line.empty() && is.eof()) {
            break;
        }

        if (line.empty() && is.bad()) {
            return false;
        }

        if ((!line.empty()) && !update_program(line, ++n_of_lines)) {
            return false;
        } else if (line.empty()) {
            ++n_of_lines;
        }

#ifdef _DEBUG
        printf("%5i %s\n", n_of_lines, line.c_str());
#endif
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::list(runnable_t::line_num_t from, runnable_t::line_num_t to,
    const std::string grep_filter)
{

    for (const auto& line : _source_line) {
        bool show[2] = { true, true };

        if (from) {
            show[0] = line.first >= from;
        }

        if (to) {
            show[1] = line.first <= to;
        }

        if (show[0] && show[1]) {
            bool bshow = true;

            if (!grep_filter.empty()) {
                std::string s = line.second;
                std::string key = grep_filter;
                std::transform(s.begin(), s.end(), s.begin(), tolower);
                std::transform(key.begin(), key.end(), key.begin(), tolower);

                bshow = s.find(key) != std::string::npos;
            }

            if (bshow) {
                dbginfo_t dbg;
                program_t* ptr = dynamic_cast<program_t*>(_prog);

                if (ptr) {
                    ptr->get_dbg_info(line.first, dbg);
                }

                console_fprintf(get_rt_ctx().get_stdout_ptr(), "%c%6u %s\n",
                    dbg.break_point ? '*' : ' ', line.first,
                    line.second.c_str());

                if (!dbg.condition_str.empty()) {
                    console_fprintf(get_rt_ctx().get_stdout_ptr(),
                        "Breakpoint at %i %s\n", line.first,
                        dbg.condition_str.c_str());
                }
            }
        }
    }

    return true;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::set_breakpoint(
    runnable_t::line_num_t line, interpreter_t::breakpoint_cond_t&& bp)
{
    dbginfo_t dbg;
    program_t* ptr = dynamic_cast<program_t*>(_prog);

    if (ptr) {
        bool res = ptr->get_dbg_info(line, dbg);

        if (!res) {
            return exec_res_t::RT_ERROR;
        }

        dbg.break_point = true;
        dbg.condition_stmt = bp.condition_stmt;
        dbg.condition_str = bp.condition_str;

        if (ptr->set_dbg_info(line, dbg)) {
            set_debug_mode(true);
            _breakpoints.insert(std::make_pair(line, bp));
            return exec_res_t::CMD_EXEC;
        }

        return exec_res_t::RT_ERROR;
    }

    return exec_res_t::SYNTAX_ERROR;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::continue_afterbrk(runnable_t::line_num_t line)
{
    dbginfo_t dbg;
    program_t* ptr = dynamic_cast<program_t*>(_prog);

    if (!ptr) {
        return false;
    }

    if (!ptr->get_dbg_info(line, dbg)) {
        return false;
    }

    if (dbg.break_point) {
        dbg.break_point = false;
        dbg.continue_after_break = true;

        return ptr->set_dbg_info(line, dbg);
    }

    return true;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::erase_breakpoint(
    runnable_t::line_num_t line)
{
    dbginfo_t dbg;
    program_t* ptr = dynamic_cast<program_t*>(_prog);

    if (!ptr) {
        return exec_res_t::SYNTAX_ERROR;
    }

    if (!ptr->get_dbg_info(line, dbg)) {
        return exec_res_t::RT_ERROR;
    }

    dbg.break_point = false;

    if (ptr->set_dbg_info(line, dbg)) {
        _breakpoints.erase(line);
        return exec_res_t::CMD_EXEC;
    }

    return exec_res_t::RT_ERROR;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::break_if(
    prog_pointer_t::line_number_t line, token_list_t& tl)
{
    if (tl.empty() || tl.begin()->type() != tkncl_t::IDENTIFIER
        || tl.begin()->identifier() != "if") {
        return exec_res_t::SYNTAX_ERROR;
    }

    std::string condition_str;

    for (const auto& t : tl) {
        if (t.type() == tkncl_t::STRING_LITERAL) {
            auto id = t.org_id();
            quote_string(id);
            condition_str += "\"" + id + "\"";
        } else {
            condition_str += t.org_id();
        }
    }

    auto token = *tl.begin();

    std::string id = "then";
    token_t tk_then(
        id, tkncl_t::IDENTIFIER, token.position(), token.expression_ptr());

    tl.data().push_back(tk_then);

    id = " ";
    token_t tk_blank(
        id, tkncl_t::BLANK, token.position(), token.expression_ptr());

    tl.data().push_back(tk_blank);

    id = "end";
    token_t tk_end(
        id, tkncl_t::IDENTIFIER, token.position(), token.expression_ptr());

    tl.data().push_back(tk_end);

    nu::stmt_t::handle_t cond_stmt(_parser.compile_line(tl, get_rt_ctx()));

    return set_breakpoint(line, breakpoint_cond_t(condition_str, cond_stmt));
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::set_step_mode(bool on)
{
    auto& ctx = get_rt_ctx();
    ctx.step_mode_active = on;
    const bool resumable_pause = ctx.last_break_line > 0 || is_stop_stmt_line();
    ctx.step_break_on_entry_pending = on && !resumable_pause;
    return exec_res_t::CMD_EXEC;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::debug_exec(
    const debug_exec_request_t& request)
{
    set_debug_mode(true);

    auto& ctx = get_rt_ctx();
    ctx.active_debug_exec = request;
    ctx.active_debug_exec.start_pc = ctx.runtime_pc;
    if (ctx.last_break_line > 0) {
        ctx.active_debug_exec.start_pc.set(ctx.last_break_line, 0);
    }
    ctx.active_debug_exec.start_stack_depth = ctx.call_stack.size();
    if (request.action == debug_exec_action_t::StepOut
        && !ctx.call_stack.empty()) {
        ctx.active_debug_exec.target_pc.set(
            ctx.call_stack.back().call_site_line, 0);
        ctx.active_debug_exec.has_target_pc = true;
    }
    ctx.last_debug_stop_reason = debug_stop_reason_t::None;

    const auto debug_start_line = ctx.active_debug_exec.start_pc.get_line();
    const auto debug_start_stack_depth
        = ctx.active_debug_exec.start_stack_depth;
    const bool debug_has_target_pc = ctx.active_debug_exec.has_target_pc;
    const auto debug_target_line = ctx.active_debug_exec.target_pc.get_line();

    auto check_break_and_stop = [&]() {
        auto runtime_pc_is_debug_steppable = [&]() {
            const auto line = ctx.runtime_pc.get_line();
            const auto it = _prog_line.find(line);
            return it != _prog_line.end()
                && it->second.first->is_debug_steppable();
        };

        auto pending_expression_step_is_debug_steppable = [&]() {
            if (ctx.runtime_pc.get_line() > 0) {
                return false;
            }

            const auto line = get_cur_line_n();
            return line > 0 && has_runnable_stmt(line);
        };

        if (ctx.flag[rt_prog_ctx_t::FLG_STOP_REQUEST]) {
            ctx.flag.set(rt_prog_ctx_t::FLG_STOP_REQUEST, false);
            ctx.last_stop_was_step = false;
            ctx.last_debug_stop_reason = debug_stop_reason_t::Pause;
            return exec_res_t::STOP_REQ;
        }

        if (is_stop_stmt_line()) {
            ctx.last_stop_was_step = false;
            ctx.last_debug_stop_reason = debug_stop_reason_t::StopStatement;
            return exec_res_t::STOP_REQ;
        }

        if (is_breakpoint_active()) {
            ctx.last_stop_was_step = false;
            ctx.last_debug_stop_reason = debug_stop_reason_t::Breakpoint;
            return exec_res_t::BREAKPOINT;
        }

        if (ctx.step_mode_active
            && (ctx.last_break_line > 0 || runtime_pc_is_debug_steppable()
                || pending_expression_step_is_debug_steppable())) {
            ctx.last_stop_was_step = true;
            ctx.last_debug_stop_reason = debug_stop_reason_t::Step;
            return exec_res_t::STOP_REQ;
        }

        ctx.last_stop_was_step = false;
        ctx.last_break_line = 0;
        ctx.call_stack.clear();
        if (ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST])
            ctx.last_debug_stop_reason = debug_stop_reason_t::ProgramEnd;
        return exec_res_t::CMD_EXEC;
    };

    auto continue_from_current_pc = [&]() {
        const auto line = ctx.runtime_pc.get_line();
        const auto stmt_id = ctx.runtime_pc.get_stmt_pos();

        if (line == 0 || continue_afterbrk(line)) {
            if (!cont(line, stmt_id))
                rebuild();
        }
    };

    auto execute_step_into_once = [&]() {
        const auto prev_step_mode = ctx.step_mode_active;
        const auto prev_entry_step = ctx.step_break_on_entry_pending;

        set_step_mode(true);

        struct step_state_guard_t {
            rt_prog_ctx_t& ctx;
            bool prev_step_mode = false;
            bool prev_entry_step = false;

            ~step_state_guard_t()
            {
                ctx.step_mode_active = prev_step_mode;
                ctx.step_break_on_entry_pending = prev_entry_step;
            }
        } guard{ ctx, prev_step_mode, prev_entry_step };

        const bool resumable_pause
            = ctx.last_debug_stop_reason != debug_stop_reason_t::ProgramEnd
            && (ctx.last_break_line > 0 || is_stop_stmt_line());

        if (!resumable_pause) {
            rebuild();
            ctx.step_break_on_entry_pending = true;
            run(0);
        } else {
            continue_from_current_pc();

            while (!ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST]
                && !ctx.debug_pending_returns.empty() && !is_breakpoint_active()
                && !is_stop_stmt_line()) {
                continue_from_current_pc();
            }
        }

        return check_break_and_stop();
    };

    auto should_keep_stepping_over = [&]() {
        return get_last_debug_stop_reason() == debug_stop_reason_t::Step
            && ctx.call_stack.size() > debug_start_stack_depth;
    };

    auto should_resume_expression_function = [&]() {
        return get_last_debug_stop_reason() == debug_stop_reason_t::Step
            && !ctx.debug_function_checkpoints.empty()
            && ctx.debug_function_checkpoints.front().expression_call;
    };

    auto should_complete_pending_expression_return = [&]() {
        return !ctx.debug_pending_returns.empty();
    };

    auto restore_pending_expression_call_site = [&]() {
        if (ctx.runtime_pc.get_line() > 0 || ctx.debug_pending_returns.empty())
            return;

        const auto call_site_line
            = ctx.debug_pending_returns.front().call_site_line;
        if (call_site_line < 1)
            return;

        ctx.runtime_pc.set(call_site_line, 0);
        ctx.last_break_line = call_site_line;
        ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, false);
    };

    auto should_keep_stepping_out = [&]() {
        if (debug_start_stack_depth == 0)
            return false;

        return get_last_debug_stop_reason() == debug_stop_reason_t::Step
            && ctx.call_stack.size() >= debug_start_stack_depth;
    };

    auto stopped_again_on_start_breakpoint = [&](exec_res_t res) {
        return res == exec_res_t::BREAKPOINT
            && ctx.runtime_pc.get_line() == debug_start_line;
    };

    auto stopped_on_procedure_boundary = [&](exec_res_t res) {
        if (res != exec_res_t::STOP_REQ
            || get_last_debug_stop_reason() != debug_stop_reason_t::Step) {
            return false;
        }

        const auto line = ctx.last_break_line > 0 ? ctx.last_break_line
                                                  : ctx.runtime_pc.get_line();
        const auto it = _prog_line.find(line);
        return it != _prog_line.end()
            && it->second.first->get_cl() == stmt_t::stmt_cl_t::SUB_END;
    };

    auto stopped_on_stepout_call_site = [&](exec_res_t res) {
        return res == exec_res_t::STOP_REQ
            && get_last_debug_stop_reason() == debug_stop_reason_t::Step
            && debug_has_target_pc
            && ctx.runtime_pc.get_line() == debug_target_line;
    };

    auto stopped_on_blocking_input_line = [&](exec_res_t res) {
        if (res != exec_res_t::STOP_REQ
            || get_last_debug_stop_reason() != debug_stop_reason_t::Step) {
            return false;
        }

        const auto line = get_cur_line_n();
        const auto it = _source_line.find(line);
        return it != _source_line.end()
            && source_line_has_blocking_input(it->second);
    };

    auto current_line_has_blocking_input = [&]() {
        const auto line = get_cur_line_n();
        const auto it = _source_line.find(line);
        return it != _source_line.end()
            && source_line_has_blocking_input(it->second);
    };

    auto normalize_expression_function_step_stop
        = [&](exec_res_t res, prog_pointer_t::line_number_t call_site_line) {
              const bool step_stop = res == exec_res_t::STOP_REQ
                  && get_last_debug_stop_reason() == debug_stop_reason_t::Step;
              const bool completed_without_stop = res == exec_res_t::CMD_EXEC;

              if ((!step_stop && !completed_without_stop)
                  || call_site_line < 1 || ctx.runtime_pc.get_line() > 0
                  || ctx.last_break_line > 0
                  || !ctx.debug_function_checkpoints.empty()
                  || !ctx.debug_pending_returns.empty()) {
                  return res;
              }

              for (auto it = _prog_line.upper_bound(call_site_line);
                  it != _prog_line.end(); ++it) {
                  if (it->second.first->is_debug_steppable()) {
                      ctx.runtime_pc.set(it->first, 0);
                      ctx.last_break_line = it->first;
                      ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, false);
                      ctx.last_stop_was_step = true;
                      ctx.last_debug_stop_reason = debug_stop_reason_t::Step;
                      return exec_res_t::STOP_REQ;
                  }
              }

              return res;
          };

    switch (request.action) {
    case debug_exec_action_t::Continue:
        continue_from_current_pc();

        while (!ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST]
            && !ctx.debug_pending_returns.empty() && !is_breakpoint_active()
            && !is_stop_stmt_line()) {
            continue_from_current_pc();
        }

        return check_break_and_stop();

    case debug_exec_action_t::StepInto:
        return execute_step_into_once();

    case debug_exec_action_t::StepOver: {
        auto res = execute_step_into_once();

        while (
            (res == exec_res_t::STOP_REQ && !stopped_on_blocking_input_line(res)
                && (should_keep_stepping_over()
                    || should_resume_expression_function()))
            || should_complete_pending_expression_return()
            || stopped_on_procedure_boundary(res)
            || stopped_again_on_start_breakpoint(res)) {
            restore_pending_expression_call_site();
            res = execute_step_into_once();
        }

        res = normalize_expression_function_step_stop(res, debug_start_line);

        return res;
    }

    case debug_exec_action_t::StepOut: {
        if (debug_start_stack_depth == 0)
            return debug_continue();

        if (current_line_has_blocking_input()) {
            ctx.last_break_line = get_cur_line_n();
            ctx.last_stop_was_step = true;
            ctx.last_debug_stop_reason = debug_stop_reason_t::Step;
            ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, false);
            return exec_res_t::STOP_REQ;
        }

        auto res = execute_step_into_once();

        while (
            (res == exec_res_t::STOP_REQ && !stopped_on_blocking_input_line(res)
                && (should_keep_stepping_out()
                    || should_resume_expression_function()))
            || should_complete_pending_expression_return()
            || stopped_again_on_start_breakpoint(res)
            || stopped_on_procedure_boundary(res)
            || stopped_on_stepout_call_site(res)) {
            restore_pending_expression_call_site();
            res = execute_step_into_once();
        }

        res = normalize_expression_function_step_stop(
            res, debug_has_target_pc ? debug_target_line : debug_start_line);

        return res;
    }

    case debug_exec_action_t::RunToCursor:
        return exec_res_t::SYNTAX_ERROR;

    case debug_exec_action_t::Pause:
        ctx.flag.set(rt_prog_ctx_t::FLG_STOP_REQUEST, true);
        ctx.last_debug_stop_reason = debug_stop_reason_t::Pause;
        return exec_res_t::CMD_EXEC;
    }

    return exec_res_t::RT_ERROR;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::debug_continue()
{
    return debug_exec(debug_exec_request_t{ debug_exec_action_t::Continue });
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::debug_step_into()
{
    return debug_exec(debug_exec_request_t{ debug_exec_action_t::StepInto });
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::debug_step_over()
{
    return debug_exec(debug_exec_request_t{ debug_exec_action_t::StepOver });
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::debug_step_out()
{
    return debug_exec(debug_exec_request_t{ debug_exec_action_t::StepOut });
}


/* -------------------------------------------------------------------------- */

debug_stop_reason_t interpreter_t::get_last_debug_stop_reason() const noexcept
{
    return _prog_ctx.last_debug_stop_reason;
}


/* -------------------------------------------------------------------------- */

void interpreter_t::set_debug_mode(bool on) noexcept
{
    auto& ctx = get_rt_ctx();
    ctx.debug_mode = on;
    if (!on) {
        ctx.call_stack.clear();
        ctx.debug_function_checkpoints.clear();
        ctx.debug_pending_returns.clear();
        ctx.last_break_line = 0;
        ctx.step_break_on_entry_pending = false;
        ctx.last_stop_was_step = false;
        ctx.active_debug_exec = debug_exec_request_t{};
        ctx.last_debug_stop_reason = debug_stop_reason_t::None;
    }
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::get_debug_mode() const noexcept
{
    return _prog_ctx.debug_mode;
}


/* -------------------------------------------------------------------------- */

const std::vector<rt_prog_ctx_t::call_frame_t>&
interpreter_t::get_call_stack() const noexcept
{
    return _prog_ctx.call_stack;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::get_fileparameter(tokenizer_t& tknzr, std::string& filename)
{
    auto token = tknzr.next();

    if (tknzr.eol()) {
        return false;
    }

    skip_blank(tknzr, token);

    filename = token.identifier();

    while (!tknzr.eol()) {
        token = tknzr.next();
        filename += token.identifier();
    }

    // Remove trailing spaces
    while (!filename.empty() && *filename.rbegin() == ' ') {
        filename = filename.substr(0, filename.size() - 1);
    }

    return true;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::exec_command(const std::string& cmd)
{
    std::string command = cmd;
    tokenizer_t tknzr(command, 0);

    if (tknzr.eol()) {
        return exec_res_t::NOP;
    }

    nu::token_t token(tknzr.next());
    skip_blank(tknzr, token);

    bool add_ok = false;

    //? -> PRINT ;-)
    if (token.identifier() == "?") {
        auto i = command.find("?");
        command = command.substr(0, i) + "print "
            + command.substr(i + 1, command.size() - i - 1);
        token.set_identifier("print", token_t::case_t::NOCHANGE);
        token.set_type(tkncl_t::IDENTIFIER);
    } else if (token.identifier() == "!") {
        auto i = command.find("!");

        command = command.substr(i + 1, command.size() - i - 1);

        os_shell_t::exec(command);

        return exec_res_t::CMD_EXEC;
    } else if (token.type() == tkncl_t::INTEGRAL) {
        unsigned int line = std::stoi(token.identifier());

        if (line > 0) {
            rebuild(line - 1);
        }

        add_ok = update_program(command);
    }

    if (add_ok) {
        return exec_res_t::UPDATE_PROG;
    }

    if (token.type() == tkncl_t::IDENTIFIER) {
        std::string cmd = token.identifier();

        if (cmd == "exit") {
            exit(0);
            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "cd") {
            token = tknzr.next();
            skip_blank(tknzr, token);

            std::string arg = token.identifier();

            while (!tknzr.eol()) {
                token = tknzr.next();
                arg += token.identifier();
            }

            const auto normalized_arg
                = to_lower_ascii(trim_trailing_path_separators(arg));

            // Treat "cd examples" like a normal shell command and enter the
            // examples directory itself. The historical install-root special
            // case was surprising because "dir" right after "cd examples"
            // still listed the parent directory.
            if (normalized_arg == "examples") {
                const std::string examples_dir = examples_directory();
                if (!examples_dir.empty() && _os_change_dir(examples_dir))
                    return exec_res_t::CMD_EXEC;
            }

            return _os_change_dir(arg) ? exec_res_t::CMD_EXEC
                                       : exec_res_t::IO_ERROR;
        }

        if (cmd == "pwd") {
            std::string wd = _os_get_working_dir();
            console_fprintf(get_stdout_ptr(), "%s\n", wd.c_str());

            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "ls" || cmd == "dir") {
            token = tknzr.next();
            skip_blank(tknzr, token);

            std::string arg
                = token.type() != tkncl_t::UNDEFINED ? token.identifier() : ".";

            while (!tknzr.eol()) {
                token = tknzr.next();
                arg += token.identifier();
            }

            namespace fs = std::filesystem;

            std::error_code ec;
            fs::path target(arg);

            if (!fs::exists(target, ec) || !fs::is_directory(target, ec)) {
                console_fprintf(get_stdout_ptr(),
                    "ls: cannot access '%s': No such directory\n", arg.c_str());
                return exec_res_t::IO_ERROR;
            }

            std::vector<fs::directory_entry> entries;
            for (const auto& e : fs::directory_iterator(target, ec))
                entries.push_back(e);

            std::sort(entries.begin(), entries.end(),
                [](const fs::directory_entry& a, const fs::directory_entry& b) {
                    return a.path().filename() < b.path().filename();
                });

            for (const auto& e : entries) {
                const bool is_dir = e.is_directory(ec);
                const std::string name = e.path().filename().string();
                if (is_dir) {
                    console_fprintf(get_stdout_ptr(), "%s/\n", name.c_str());
                } else {
                    const auto sz = e.file_size(ec);
                    console_fprintf(get_stdout_ptr(), "%-36s %10llu\n",
                        name.c_str(), static_cast<unsigned long long>(sz));
                }
            }

            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "help") {
            token = tknzr.next();
            skip_blank(tknzr, token);

            const std::string topic
                = token.type() != tkncl_t::UNDEFINED ? token.identifier() : "";

            if (to_lower_ascii(topic) == "examples") {
                const std::string text = examples_help_text();
                console_fprintf(get_stdout_ptr(), "%s\n", text.c_str());
                return exec_res_t::CMD_EXEC;
            }

            const std::string help_content
                = builtin_help_t::get_instance().help(topic);

            console_fprintf(get_stdout_ptr(), "%s\n", help_content.c_str());

            return exec_res_t::CMD_EXEC;
        }


        if (cmd == "apropos") {
            std::string help_content;

            token = tknzr.next();
            skip_blank(tknzr, token);

            help_content = builtin_help_t::get_instance().apropos(
                token.type() != tkncl_t::UNDEFINED ? token.identifier() : "");

            console_fprintf(get_stdout_ptr(), "%s\n", help_content.c_str());

            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "ver") {
            auto ver = version();
            console_fprintf(get_stdout_ptr(), "%s\n", ver.c_str());

            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "new") {
            clear_all();
            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "clr") {
            clear_rtdata();
            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "tron") {
            get_rt_ctx().tracing_on = true;
            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "troff") {
            get_rt_ctx().tracing_on = false;
            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "load") {
            std::string arg;

            if (!get_fileparameter(tknzr, arg)) {
                return exec_res_t::SYNTAX_ERROR;
            }

            // Resolve the file path (try current dir, then examples dir)
            std::string resolved;
            {
                FILE* probe = fopen(arg.c_str(), "r");
                if (probe) {
                    fclose(probe);
                    resolved = arg;
                } else {
                    const std::string ex = examples_directory();
                    if (!ex.empty()) {
                        fs::path p = fs::path(ex) / arg;
                        probe = fopen(p.string().c_str(), "r");
                        if (probe) {
                            fclose(probe);
                            resolved = p.string();
                        } else if (arg.find('.') == std::string::npos) {
                            p = fs::path(ex) / (arg + ".bas");
                            probe = fopen(p.string().c_str(), "r");
                            if (probe) {
                                fclose(probe);
                                resolved = p.string();
                            }
                        }
                    }
                }
            }

            if (resolved.empty()) {
                return exec_res_t::IO_ERROR;
            }

            clear_all();

            return load(resolved) ? exec_res_t::CMD_EXEC : exec_res_t::IO_ERROR;
        }

        if (cmd == "exec") {
            std::string arg;

            if (!get_fileparameter(tknzr, arg)) {
                return exec_res_t::SYNTAX_ERROR;
            }

            const std::string resolved = resolve_program_file(arg);

            if (resolved.empty()) {
                return exec_res_t::IO_ERROR;
            }

            clear_all();

            if (!load(resolved)) {
                return exec_res_t::IO_ERROR;
            }

            run_main_or_default();

            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "save") {
            std::string arg;

            if (!get_fileparameter(tknzr, arg)) {
                return exec_res_t::SYNTAX_ERROR;
            }

            return save(arg) ? exec_res_t::CMD_EXEC : exec_res_t::IO_ERROR;
        }


        if (cmd == "build") {
            skip_blank(tknzr, token);

            if (!tknzr.eol()) {
                return exec_res_t::SYNTAX_ERROR;
            }

            rebuild();

            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "resume") {
            if (!run_next(get_rt_ctx().runtime_pc.get_line()))
                rebuild();

            return exec_res_t::CMD_EXEC;
        }

        auto check_break_and_stop = [&]() {
            auto runtime_pc_is_debug_steppable = [&]() {
                const auto line = get_rt_ctx().runtime_pc.get_line();
                const auto it = _prog_line.find(line);
                return it != _prog_line.end()
                    && it->second.first->is_debug_steppable();
            };

            auto pending_expression_step_is_debug_steppable = [&]() {
                if (get_rt_ctx().runtime_pc.get_line() > 0) {
                    return false;
                }

                const auto line = get_cur_line_n();
                return line > 0 && has_runnable_stmt(line);
            };

            if (get_rt_ctx().flag[rt_prog_ctx_t::FLG_STOP_REQUEST]) {
                get_rt_ctx().flag.set(rt_prog_ctx_t::FLG_STOP_REQUEST, false);
                get_rt_ctx().last_stop_was_step = false;
                return exec_res_t::STOP_REQ;
            }

            if (is_stop_stmt_line()) {
                get_rt_ctx().last_stop_was_step = false;
                return exec_res_t::STOP_REQ;
            }

            if (is_breakpoint_active()) {
                get_rt_ctx().last_stop_was_step = false;
                return exec_res_t::BREAKPOINT;
            }

            if (get_rt_ctx().step_mode_active
                && (get_rt_ctx().last_break_line > 0
                    || runtime_pc_is_debug_steppable()
                    || pending_expression_step_is_debug_steppable())) {
                get_rt_ctx().last_stop_was_step = true;
                return exec_res_t::STOP_REQ;
            }

            get_rt_ctx().last_stop_was_step = false;
            return exec_res_t::CMD_EXEC;
        };

        if (cmd == "step" || cmd == "stepinto") {
            return debug_step_into();
        }

        if (cmd == "stepover" || cmd == "next") {
            return debug_step_over();
        }

        if (cmd == "stepout") {
            return debug_step_out();
        }

        if (cmd == "cont") {
            return debug_continue();
        }

        if (cmd == "run") {
            token = tknzr.next();

            if (tknzr.eol()) {
                rebuild();
                run_main_or_default();
                return check_break_and_stop();
            }

            skip_blank(tknzr, token);

            if (token.type() == tkncl_t::INTEGRAL) {
                // Command "RUN <num>" does not clear the context
                run(nu::stoi(token.identifier()));

                return check_break_and_stop();
            }

            return exec_res_t::SYNTAX_ERROR;
        }

        if (cmd == "ston") {
            // set step mode off
            set_step_mode(true);
            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "stoff") {
            // set step mode off
            set_step_mode(false);
            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "break") {
            token_list_t tl;
            tknzr.get_tknlst(tl);
            statement_parser_t::remove_blank(tl);

            if (tl.empty()) {
                for (auto& b : _breakpoints) {
                    list(b.first, b.first);
                }

                return exec_res_t::CMD_EXEC;
            }

            token_t token = *tl.begin();

            prog_pointer_t::line_number_t line = 0;

            if (token.type() == tkncl_t::INTEGRAL) {
                line = nu::stoi(token.identifier());
                --tl;
                statement_parser_t::remove_blank(tl);

                if (tl.empty()) {
                    return set_breakpoint(line, breakpoint_cond_t("", nullptr));
                }
            } else {
                return exec_res_t::SYNTAX_ERROR;
            }

            return break_if(line, tl);
        }

        if (cmd == "rmbrk") {
            token = tknzr.next();

            if (tknzr.eol()) {
                return exec_res_t::SYNTAX_ERROR;
            }

            skip_blank(tknzr, token);

            if (token.type() == tkncl_t::INTEGRAL) {
                const auto line = nu::stoi(token.identifier());
                return erase_breakpoint(line);
            }

            return exec_res_t::SYNTAX_ERROR;
        }

        if (cmd == "clrbrk") {
            auto breakpoints = _breakpoints;

            for (const auto& line : breakpoints) {
                erase_breakpoint(line.first);
            }

            return exec_res_t::CMD_EXEC;
        }

        if (cmd == "vars") {
            std::stringstream ss;
            get_rt_ctx().trace_rtdata(ss);
            console_fprintf(
                get_rt_ctx().get_stdout_ptr(), "%s\n", ss.str().c_str());

            return exec_res_t::CMD_EXEC;
        }


        if (cmd == "meta") {
            std::stringstream ss;
            get_rt_ctx().trace_metadata(ss);
            console_fprintf(
                get_rt_ctx().get_stdout_ptr(), "%s\n", ss.str().c_str());

            return exec_res_t::CMD_EXEC;
        }


        if (cmd == "renum") {
            token = tknzr.next();

            if (tknzr.eol()) {
                renum_prog(0);
                rebuild();
                return exec_res_t::CMD_EXEC;
            }

            skip_blank(tknzr, token);

            if (token.type() == tkncl_t::INTEGRAL) {
                renum_prog(nu::stoi(token.identifier()));
                rebuild();
                return exec_res_t::CMD_EXEC;
            }

            return exec_res_t::SYNTAX_ERROR;
        }


        if (cmd == "list") {
            token = tknzr.next();

            if (tknzr.eol()) {
                return list() ? exec_res_t::CMD_EXEC : exec_res_t::SYNTAX_ERROR;
            }

            skip_blank(tknzr, token);

            runnable_t::line_num_t from_line = 0;
            runnable_t::line_num_t to_line = 0;

            bool parse_minus = true;

            if (token.type() == tkncl_t::INTEGRAL) {
                to_line = from_line = nu::stoi(token.identifier());
            } else {
                if (token.type() == tkncl_t::OPERATOR
                    && token.identifier() == "-") {
                    from_line = 0; // from begin
                    parse_minus = false;
                } else {
                    return exec_res_t::SYNTAX_ERROR;
                }
            }

            token = tknzr.next();
            skip_blank(tknzr, token);

            if (parse_minus) {
                if (token.type() == tkncl_t::OPERATOR
                    && token.identifier() == "-") {
                    to_line = 0; // until the end
                }

                if (tknzr.eol()) {
                    return list(from_line, to_line) ? exec_res_t::CMD_EXEC
                                                    : exec_res_t::SYNTAX_ERROR;
                }

                token = tknzr.next();
                skip_blank(tknzr, token);
            }

            if (token.type() == tkncl_t::INTEGRAL) {
                to_line = nu::stoi(token.identifier());
            } else {
                return exec_res_t::SYNTAX_ERROR;
            }

            return list(from_line, to_line) ? exec_res_t::CMD_EXEC
                                            : exec_res_t::SYNTAX_ERROR;
        }

        if (cmd == "grep") {
            token = tknzr.next();

            if (tknzr.eol()) {
                return exec_res_t::SYNTAX_ERROR;
            }

            skip_blank(tknzr, token);

            return list(0, 0, token.identifier()) ? exec_res_t::CMD_EXEC
                                                  : exec_res_t::SYNTAX_ERROR;
        }
    }

    // Remove trailing spaces
    while (!command.empty() && *command.rbegin() == ' ') {
        command = command.substr(0, command.size() - 1);
    }

    // Try to load and exec filename
    if (can_open_program_file(command)) {
        clear_all();

        if (load(command)) {
            set_ignore_break_event(true);
            signal_mgr_t::instance().disable_notifications();
            bool res = run_main_or_default();
            exit(res ? 0 : 1);

            return exec_res_t::CMD_EXEC;
        }
    }

    // Run BASIC in-line instruction
    tokenizer_t inlinetknzr(command, 0);
    nu::stmt_t::handle_t h(_parser.compile_line(inlinetknzr, get_rt_ctx()));

    NU_TRACE_CTX(get_rt_ctx());

    h->run(get_rt_ctx());

    NU_TRACE_CTX(get_rt_ctx());

    return exec_res_t::CMD_EXEC;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::run(runnable_t::line_num_t line)
{
    struct _guard_t {
        _guard_t() { _os_config_term(false); }
        ~_guard_t() { _os_config_term(true); }
    } _guard;

    if (line == 0) {
        const bool restore_entry_step = _prog_ctx.debug_mode
            && (_prog_ctx.step_mode_active
                || _prog_ctx.step_break_on_entry_pending);
        const auto saved_step_mode = _prog_ctx.step_mode_active;
        const auto saved_entry_step = _prog_ctx.step_break_on_entry_pending;
        const auto saved_debug_exec = _prog_ctx.active_debug_exec;
        const auto saved_stop_reason = _prog_ctx.last_debug_stop_reason;

        _prog_ctx.clear_rtdata();

        if (restore_entry_step) {
            _prog_ctx.step_mode_active = saved_step_mode;
            _prog_ctx.step_break_on_entry_pending = saved_entry_step;
            _prog_ctx.active_debug_exec = saved_debug_exec;
            _prog_ctx.last_debug_stop_reason = saved_stop_reason;
        }
    }

    _prog_ctx.call_stack.clear();
    _prog_ctx.last_break_line = 0;

    program_t prog(_prog_line, _prog_ctx, false);

    if (_yield_cbk) {
        prog.set_yield_cbk(_yield_cbk, _yield_data);
    }

    return prog.run(line);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::run_main_or_default()
{
    if (_prog_ctx.get_syntax_mode() != prog_ctx_t::syntax_mode_t::MODERN) {
        return run(0);
    }

    // If the loaded program defines a top-level "main" function/sub,
    // use it as the entry point and pass the stored CLI arguments.
    auto main_it = _prog_ctx.proc_prototypes.data.end();
    std::string main_name;
    for (auto it = _prog_ctx.proc_prototypes.data.begin();
         it != _prog_ctx.proc_prototypes.data.end(); ++it) {
        if (to_lower_ascii(it->first) == "main") {
            main_it = it;
            main_name = it->first;
            break;
        }
    }

    if (main_it != _prog_ctx.proc_prototypes.data.end()) {
        const auto& params = main_it->second.second.parameters;
        const size_t n = _cli_args.size();

        std::vector<expr_any_t::handle_t> args;

        if (params.size() >= 1) {
            // First parameter: argc As Integer
            args.push_back(
                std::make_shared<expr_literal_t>(variant_t(integer_t(n))));
        }

        if (params.size() >= 2) {
            // Second parameter: argv() As String
            variant_t argv_val("", n > 0 ? n : 1);
            for (size_t k = 0; k < n; ++k)
                argv_val.set_str(_cli_args[k], k);
            args.push_back(std::make_shared<expr_literal_t>(argv_val));
        }

        struct _guard_t {
            _guard_t() { _os_config_term(false); }
            ~_guard_t() { _os_config_term(true); }
        } _guard;

        _prog_ctx.clear_rtdata();

        // Use checkpoint mode (true) so that End Function / End Sub
        // can cleanly return to line 0 without triggering a runtime error.
        program_t prog(_prog_line, _prog_ctx, true);

        if (_yield_cbk)
            prog.set_yield_cbk(_yield_cbk, _yield_data);

        return prog.run(main_name, args);
    }

    return run(0);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::cont(
    runnable_t::line_num_t line, runnable_t::stmt_num_t stmtid)
{
    struct _guard_t {
        _guard_t() { _os_config_term(false); }
        ~_guard_t() { _os_config_term(true); }
    } _guard;

    const bool resume_function_call
        = _prog_ctx.debug_mode && !_prog_ctx.debug_function_checkpoints.empty();

    _prog_ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, false);

    program_t prog(_prog_line, _prog_ctx, resume_function_call);

    if (_yield_cbk) {
        prog.set_yield_cbk(_yield_cbk, _yield_data);
    }

    return prog.cont(line, stmtid);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::run_next(runnable_t::line_num_t line)
{
    struct _guard_t {
        _guard_t() { _os_config_term(false); }
        ~_guard_t() { _os_config_term(true); }
    } _guard;

    program_t prog(_prog_line, _prog_ctx, false);

    if (_yield_cbk) {
        prog.set_yield_cbk(_yield_cbk, _yield_data);
    }

    return prog.run_next(line);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::has_runnable_stmt(int line) const noexcept
{
    if (line < 0 || _prog_line.empty()) {
        return false;
    }

    prog_line_t::const_iterator it = _prog_line.find(line);

    if (it == _prog_line.end()) {
        return false;
    }

    return it->second.first->is_debug_steppable();
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::get_global_var(const std::string& name, nu::variant_t& var)
{
    auto& ctx = get_rt_ctx();
    auto scope = ctx.proc_scope.get_global();

    if (!scope || !scope->is_defined(name)) {
        return false;
    }

    var = (*scope)[name].first;

    return true;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::set_global_var(
    const std::string& name, const nu::variant_t& value)
{
    auto scope = get_rt_ctx().proc_scope.get_global();

    if (!scope) {
        return false;
    }

    scope->define(name, nu::var_value_t(value, VAR_ACCESS_RW));

    return true;
}


/* -------------------------------------------------------------------------- */

prog_pointer_t::line_number_t interpreter_t::get_cur_line_n() const noexcept
{
    auto next_runnable_line_or_self = [&](prog_pointer_t::line_number_t line) {
        if (line < 1)
            return line;

        if (has_runnable_stmt(line))
            return line;

        for (auto it = _prog_line.upper_bound(line); it != _prog_line.end();
            ++it) {
            if (it->second.first->is_debug_steppable())
                return it->first;
        }

        return line;
    };

    // When a breakpoint fired inside an expression-called function the
    // checkpoint mechanism restores runtime_pc to the call site; use the
    // recorded break line instead so the IDE highlights the right source line.
    if (_prog_ctx.last_break_line > 0)
        return next_runnable_line_or_self(_prog_ctx.last_break_line);

    auto line = _prog_ctx.runtime_pc.get_line();

    if (line < 1) {
        line = _prog_ctx.compiletime_pc.get_line();
    }

    return next_runnable_line_or_self(line);
}


/* -------------------------------------------------------------------------- */

prog_pointer_t::line_number_t interpreter_t::get_last_line_n() const noexcept
{
    auto line = _prog_ctx.runtime_pc.get_last_line();

    if (line < 1) {
        line = _prog_ctx.compiletime_pc.get_last_line();
    }

    return line;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
