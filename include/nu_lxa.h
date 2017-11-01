//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_LXA_H__
#define __NU_LXA_H__


/* -------------------------------------------------------------------------- */

#include <iostream>
#include <memory>
#include <set>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class lxa_t {
public:
    virtual bool accept(char c) = 0;
    virtual const std::string& data() const = 0;
    virtual void reset() = 0;
};


/* -------------------------------------------------------------------------- */

template <class T> class lxa_atom_t : public lxa_t {
protected:
    using pmset_t = std::set<T>;
    pmset_t _plist;
    std::string _data;

public:
    lxa_atom_t() = default;
    lxa_atom_t(const lxa_atom_t&) = default;
    lxa_atom_t& operator=(const lxa_atom_t&) = default;

    void register_pattern(const T& pattern) { 
        _plist.insert(pattern); 
    }

    void remove_pattern(const T& pattern) { 
        _plist.erase(pattern); 
    }

    void reset() override { 
        _data.clear(); 
    }

    const std::string& data() const override { 
        return _data; 
    }
};


/* -------------------------------------------------------------------------- */

class lxa_word_t : public lxa_atom_t<std::string> {
public:
    bool keyword_matches = false;

    lxa_word_t() = default;
    lxa_word_t(const lxa_word_t&) = default;
    lxa_word_t& operator=(const lxa_word_t&) = default;

    bool accept(char c) override;

    void reset() override {
        lxa_atom_t<std::string>::reset();
        keyword_matches = false;
    }
};


/* -------------------------------------------------------------------------- */

class lxa_symb_t : public lxa_atom_t<char> {
public:
    lxa_symb_t() = default;
    lxa_symb_t(const lxa_symb_t&) = default;
    lxa_symb_t& operator=(const lxa_symb_t&) = default;

    bool accept(char c) override;
};


/* -------------------------------------------------------------------------- */

class lxa_str_t : public lxa_t {
private:
    std::string _begin_quote, _end_quote;
    std::string _begin_found, _end_found;
    char _escape_prefix = 0;
    char _escape_found = 0;
    size_t _bindex = 0, _eindex = 0;
    std::string _data;

public:
    lxa_str_t(const std::string& begin_quote, const std::string& end_quote,
        char escape_prefix = 0)
        : _begin_quote(begin_quote)
        , _end_quote(end_quote)
        , _escape_prefix(escape_prefix)
    {
    }

    bool accept(char c) override;
    void reset() override;
    const std::string& data() const override;

    bool string_complete() const { 
        return _end_found == _end_quote; 
    }
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_LXA_H__
