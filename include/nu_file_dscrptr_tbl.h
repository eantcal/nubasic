//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_FILE_DSCRPTR_TBL_H__
#define __NU_FILE_DSCRPTR_TBL_H__


/* -------------------------------------------------------------------------- */

#include "nu_flag_map.h"
#include "nu_for_loop_rtdata.h"
#include "nu_instrblock_metadata.h"
#include "nu_label_tbl.h"
#include "nu_proc_prototype_tbl.h"
#include "nu_proc_scope.h"
#include "nu_prog_pointer.h"
#include "nu_symbol_map.h"
#include "nu_var_scope.h"

#include <algorithm>
#include <deque>
#include <list>
#include <memory>
#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

struct file_dscrptr_t {
    friend struct file_dscrptr_tbl_t;

    using handle_t = std::shared_ptr<file_dscrptr_t>;

    file_dscrptr_t() = delete;
    FILE* data() const noexcept;

    ~file_dscrptr_t();

    file_dscrptr_t(FILE* fptr)
        : _fptr(fptr)
    {
    }

protected:
    FILE* _fptr = nullptr;
    bool close() noexcept;
};


/* -------------------------------------------------------------------------- */

struct file_dscrptr_tbl_t {
    bool open_fd(const std::string& fname, // file path
        const std::string& fmode, // see fopen modes
        unsigned int fd);

    FILE* resolve_fd(unsigned int fd);
    bool close_fd(unsigned int fd);
    bool seek_fd(int seek_ptr, int seek_origin, unsigned int fd);
    bool flush_fd(unsigned int fd);
    void trace(std::stringstream& ss);

    bool empty() const noexcept { 
        return _file_tbl.empty(); 
    }

    void clear() noexcept { 
        _file_tbl.clear(); 
    }


protected:
    // File Tbl: <Key:id, Value:file-ctx-handle>
    std::map<int, file_dscrptr_t::handle_t> _file_tbl;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_FILE_DSCRPTR_TBL_H__
