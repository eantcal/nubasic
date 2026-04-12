//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
//

#pragma once

#include <string>
#include <vector>

namespace nu {

/** One row from examples/manifest.txt (filename|description). */
struct example_entry_t {
    std::string filename;
    std::string description;
};

/**
 * Install root directory (parent of the `examples` folder for a typical
 * layout). Windows: HKCU/HKLM Software\\nuBASIC\\InstallDir, else derived from
 * examples path.
 */
std::string examples_install_root();

/**
 * Directory containing sample .bas files (trailing separator not required).
 * Windows: registry Software\\nuBASIC\\ExamplesDir, env NUBASIC_EXAMPLES, or
 * heuristics next to the running executable.
 */
std::string examples_directory();

/**
 * Text for `help examples`: manifest entries + how to run (load / run, cd
 * examples).
 */
std::string examples_help_text();

/**
 * Parsed manifest + any extra .bas in the directory (for IDE menu).
 */
std::vector<example_entry_t> list_example_entries();

} // namespace nu
