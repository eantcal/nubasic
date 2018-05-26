//  
// This file is part of MipJson Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_EXCEPTION_H__
#define __MIP_EXCEPTION_H__


/* -------------------------------------------------------------------------- */

#include <exception>
#include <string>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

class exception_t : public std::exception {
public:
    exception_t() = delete;
    exception_t(const exception_t&) = default;
    exception_t& operator=(const exception_t&) = default;

    /** ctor
      *  @param message C-style string error message.
      */
    explicit exception_t(const char* message)
        : _msg(message)
    {
    }

    //! move ctor
    exception_t(exception_t&& e) { _msg = std::move(e._msg); }

    //! move assign operator
    exception_t& operator=(exception_t&& e)
    {
        if (&e != this)
            _msg = std::move(e._msg);

        return *this;
    }

    /** ctor
     *  @param message The error message.
     */
    explicit exception_t(const std::string& message)
        : _msg(message)
    {
    }

    /** dtor
     * Virtual to allow for subclassing.
     */
    virtual ~exception_t() throw() {}

    /** Returns a pointer to the (constant) error description.
     *  @return A pointer to a const char*.
     *          The underlying memory is in posession of the exception_t
     *          object. Callers must not free the memory.
     */
    virtual const char* what() const throw() override { return _msg.c_str(); }

protected:
    /** Error message.
     */
    std::string _msg;
};


/* -------------------------------------------------------------------------- */

}


#endif // __MIP_EXCEPTION_H__
