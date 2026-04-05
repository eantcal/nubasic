//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
//

#include "nu_examples_paths.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <set>
#include <sstream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#include <filesystem>

namespace fs = std::filesystem;

namespace nu {
namespace {

    static void trim_inplace(std::string& s)
    {
        while (!s.empty() && (unsigned char)s.front() <= ' ')
            s.erase(s.begin());
        while (!s.empty() && (unsigned char)s.back() <= ' ')
            s.pop_back();
    }

    static std::string to_lower(std::string s)
    {
        for (char& c : s)
            c = (char)std::tolower((unsigned char)c);
        return s;
    }

#ifdef _WIN32
    static std::string reg_query_string(HKEY root, const char* value_name)
    {
        HKEY h = nullptr;
        if (RegOpenKeyExA(root, "Software\\nuBASIC", 0, KEY_READ, &h)
            != ERROR_SUCCESS)
            return {};

        char buf[MAX_PATH * 4];
        DWORD sz = sizeof(buf);
        DWORD type = 0;
        LONG r = RegQueryValueExA(
            h, value_name, nullptr, &type, reinterpret_cast<LPBYTE>(buf), &sz);
        RegCloseKey(h);

        if (r != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ))
            return {};

        if (type == REG_EXPAND_SZ) {
            char expanded[MAX_PATH * 4];
            DWORD ex
                = ExpandEnvironmentStringsA(buf, expanded, sizeof(expanded));
            if (ex > 0 && ex < sizeof(expanded))
                return std::string(expanded);
        }

        return std::string(buf);
    }
#endif

    static std::string getenv_str(const char* name)
    {
        const char* v = std::getenv(name);
        return v ? std::string(v) : std::string{};
    }

    static std::string exe_directory()
    {
#ifdef _WIN32
        char buf[MAX_PATH];
        DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
        if (n == 0 || n >= MAX_PATH)
            return {};
        fs::path p(buf);
        return p.parent_path().string();
#elif defined(__linux__)
        char buf[8192];
        const ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (n <= 0)
            return {};
        buf[n] = '\0';
        return fs::path(buf).parent_path().string();
#else
        return {};
#endif
    }

    static bool is_dir(const std::string& p)
    {
        std::error_code ec;
        return fs::is_directory(fs::path(p), ec);
    }

    static std::string try_examples_next_to_exe()
    {
        const std::string ed = exe_directory();
        if (ed.empty())
            return {};

        const fs::path base(ed);
        const fs::path candidates[] = {
            base / "examples",
            base / ".." / "examples",
            base / ".." / ".." / "examples",
            base / ".." / ".." / ".." / "examples",
        };

        for (const auto& c : candidates) {
            std::error_code ec;
            const fs::path canon = fs::weakly_canonical(c, ec);
            if (!ec && fs::is_directory(canon))
                return canon.string();
        }

        return {};
    }

} // namespace

std::string examples_directory()
{
#ifdef _WIN32
    std::string r = reg_query_string(HKEY_CURRENT_USER, "ExamplesDir");
    if (!r.empty() && is_dir(r))
        return r;
    r = reg_query_string(HKEY_LOCAL_MACHINE, "ExamplesDir");
    if (!r.empty() && is_dir(r))
        return r;
#endif

    std::string env = getenv_str("NUBASIC_EXAMPLES");
    if (!env.empty() && is_dir(env))
        return env;

    std::string guess = try_examples_next_to_exe();
    if (!guess.empty())
        return guess;

    return {};
}

std::string examples_install_root()
{
#ifdef _WIN32
    std::string r = reg_query_string(HKEY_CURRENT_USER, "InstallDir");
    if (!r.empty() && is_dir(r))
        return r;
    r = reg_query_string(HKEY_LOCAL_MACHINE, "InstallDir");
    if (!r.empty() && is_dir(r))
        return r;
#endif

    const std::string ex = examples_directory();
    if (ex.empty())
        return {};

    fs::path p(ex);
    std::error_code ec;
    p = fs::weakly_canonical(p, ec);
    if (ec)
        p = fs::path(ex);
    fs::path parent = p.parent_path();
    if (parent.empty())
        return {};
    return parent.string();
}

static void parse_manifest_line(
    const std::string& line, std::vector<example_entry_t>& out)
{
    std::string t = line;
    trim_inplace(t);
    if (t.empty() || t[0] == '#')
        return;

    const auto pipe = t.find('|');
    example_entry_t e;
    if (pipe == std::string::npos) {
        e.filename = t;
        e.description = "Sample program";
    } else {
        e.filename = t.substr(0, pipe);
        e.description = t.substr(pipe + 1);
        trim_inplace(e.filename);
        trim_inplace(e.description);
    }

    if (e.filename.empty())
        return;

    out.push_back(std::move(e));
}

static std::vector<example_entry_t> load_manifest_ordered(
    const std::string& exdir)
{
    std::vector<example_entry_t> v;
    if (exdir.empty())
        return v;

    const fs::path mf = fs::path(exdir) / "manifest.txt";
    std::ifstream in(mf.string());
    if (!in)
        return v;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        parse_manifest_line(line, v);
    }
    return v;
}

std::vector<example_entry_t> list_example_entries()
{
    const std::string exdir = examples_directory();
    std::vector<example_entry_t> v = load_manifest_ordered(exdir);

    std::vector<example_entry_t> merged;
    std::set<std::string> seen;
    for (const auto& e : v) {
        const fs::path fp = fs::path(exdir) / e.filename;
        std::error_code ec;
        if (fs::is_regular_file(fp, ec)) {
            merged.push_back(e);
            seen.insert(to_lower(e.filename));
        }
    }

    std::vector<example_entry_t> extras;
    if (!exdir.empty()) {
        std::error_code ec;
        for (const auto& ent : fs::directory_iterator(fs::path(exdir), ec)) {
            if (ec)
                break;
            if (!ent.is_regular_file())
                continue;
            const std::string ext = to_lower(ent.path().extension().string());
            if (ext != ".bas")
                continue;
            const std::string fn = ent.path().filename().string();
            if (seen.count(to_lower(fn)))
                continue;
            example_entry_t e;
            e.filename = fn;
            e.description = "Sample program";
            extras.push_back(std::move(e));
            seen.insert(to_lower(fn));
        }
    }

    std::sort(extras.begin(), extras.end(),
        [](const example_entry_t& a, const example_entry_t& b) {
            return to_lower(a.filename) < to_lower(b.filename);
        });
    merged.insert(merged.end(), extras.begin(), extras.end());

    return merged;
}

std::string examples_help_text()
{
    std::ostringstream os;
    os << "nuBASIC example programs\n"
          "------------------------\n"
          "Working directory: use  cd examples  to set the current directory "
          "to the "
          "installation root (parent of the examples folder), so you can run:\n"
          "  load examples\\<file.bas>   (Windows)   or   load "
          "examples/<file.bas>\n"
          "  run\n"
          "You can also  load <name.bas>  or  load <name>  if the file is in "
          "the examples folder.\n\n";

    const auto entries = list_example_entries();
    if (entries.empty()) {
        os << "No examples directory found. Set registry "
              "HKCU\\Software\\nuBASIC\\ExamplesDir "
              "or environment NUBASIC_EXAMPLES to the folder that contains "
              "manifest.txt and the .bas files.\n";
        return os.str();
    }

    os << "Examples (from " << examples_directory() << "):\n\n";

    for (const auto& e : entries) {
        os << "  " << e.filename << "\n";
        os << "    " << e.description << "\n";
        os << "    load " << e.filename << "\n";
        os << "    run\n\n";
    }

    return os.str();
}

} // namespace nu
