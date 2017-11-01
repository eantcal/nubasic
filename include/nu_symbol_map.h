//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_SYMBOL_MAP_H__
#define __NU_SYMBOL_MAP_H__


/* -------------------------------------------------------------------------- */

#include <map>
#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

template <typename Key, typename Symb> class symbol_map_t {
public:
    using key_t = Key;
    using symbol_t = Symb;

    symbol_map_t() = default;
    symbol_map_t(const symbol_map_t&) = default;
    symbol_map_t& operator=(const symbol_map_t&) = delete;
    virtual ~symbol_map_t() {}
    symbol_map_t(symbol_map_t&& obj) { _symbols = std::move(obj); }

    symbol_map_t& operator=(symbol_map_t&& obj) {
        if (this != &obj) {
            _symbols = std::move(obj);
        }

        return *this;
    }

    virtual bool define(const std::string& name, const Symb& value) {
        auto i = map().insert(std::make_pair(name, value));
        return i.second;
    }

    virtual void erase(const std::string& name) { 
        map().erase(name); 
    }

    bool is_defined(const std::string& name) const noexcept  {
        return _symbols.find(name) != _symbols.end();
    }

    Symb& operator[](const std::string& name) { return _symbols[name]; }

    const Symb& operator[](const std::string& name) const {
        auto i = map().find(name);

        if (i == map().end()) {
            std::string err;
            get_err_msg(name, err);
            throw exception_t(err);
        }

        return i->second;
    }

    bool empty() const noexcept { return _symbols.empty(); }
    size_t size() const noexcept { return _symbols.size(); }
    const std::map<Key, Symb>& map() const noexcept { return _symbols; }
    virtual void clear() { _symbols.clear(); }

    friend std::stringstream& operator<<(
        std::stringstream& ss, symbol_map_t<Key, Symb>& obj)
    {
        for (const auto& e : obj.map())
            ss << "\t" << e.first.str() << ": " << e.second << std::endl;

        return ss;
    }

protected:
    std::map<Key, Symb>& map() noexcept { return _symbols; }
    
    virtual void get_err_msg(
        const std::string& key, std::string& err) const = 0;

private:
    std::map<Key, Symb> _symbols;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif
