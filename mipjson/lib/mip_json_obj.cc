//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "mip_json_obj.h"
#include "mip_exception.h"

#include <string.h>


/* -------------------------------------------------------------------------- */

namespace mip
{


/* -------------------------------------------------------------------------- */

json_obj_t::json_obj_t(const json_obj_t& other) : _type(other._type) 
{
    auto ptr = other.clone();

    if (!ptr) {
        throw mip::exception_t("Not enough memory");
    }

    memcpy(&_value, &ptr->_value, sizeof(_value));
    ptr.release();
}


/* -------------------------------------------------------------------------- */

json_obj_t::~json_obj_t() noexcept 
{
    switch (_type) {
    case type_t::STRING:
        delete (reinterpret_cast<string_t*>(_value.ptr));
        break;

    case type_t::OBJECT:
        delete (reinterpret_cast<data_t*>(_value.ptr));
        break;

    case type_t::ARRAY:
        delete (reinterpret_cast<array_t*>(_value.ptr));
        break;

    case type_t::NIL:
    case type_t::I64:
    case type_t::U64:
    case type_t::FLOATING:
    case type_t::BOOLEAN:
    default:
        break;
    }

}


/* -------------------------------------------------------------------------- */

//! Assignment operator
json_obj_t& json_obj_t::operator=(const json_obj_t& other) 
{
    if (this != &other) {
        auto ptr = other.clone();

        if (!ptr) {
            throw mip::exception_t("Not enough memory");
        }

        memcpy(&_value, &ptr->_value, sizeof(_value));

        ptr.release();
    }

    return *this;
}


/* -------------------------------------------------------------------------- */

//! Clones the entire vairant object hierarchy
json_obj_t::handle_t json_obj_t::clone() const noexcept
{
    switch (get_type()) {

    case type_t::NIL:
        return std::move(std::make_unique<json_obj_t>());

    case type_t::BOOLEAN:
        return std::move(std::make_unique<json_obj_t>(_value.bval));

    case type_t::FLOATING:
        return std::move(std::make_unique<json_obj_t>(_value.d64));

    case type_t::I64:
        return std::move(std::make_unique<json_obj_t>(_value.i64));

    case type_t::U64:
        return std::move(std::make_unique<json_obj_t>(_value.u64));

    case type_t::STRING: {
        const string_t * value = get_string();
        assert(value);
        return std::move(std::make_unique<json_obj_t>(*value));
    }

    case type_t::OBJECT: {
        auto an_obj = get_object();
        assert(an_obj);

        auto new_obj = new data_t;
        assert(an_obj);

        for (auto it = an_obj->cbegin(); it != an_obj->cend(); ++it) {
            const auto & key = it->first;

            assert(it->second);
            auto new_item = it->second->clone();

            new_obj->push_back(std::make_pair(key, std::move(new_item)));
        }

        return std::make_unique<json_obj_t>(std::move(new_obj));
    }

    case type_t::ARRAY: {
        auto an_array = get_array();
        assert(an_array);

        const auto size = an_array->size();

        array_t new_array(size);
        array_t::const_iterator it = an_array->cbegin();

        for (size_t i = 0; it != an_array->cend(); ++it, ++i) {
            const auto objptr = it->get();
            assert(objptr);
            new_array[i] = objptr->clone();
        }
        break;
    }
    default:
        break;
    }

    return nullptr;
}


/* -------------------------------------------------------------------------- */

bool json_obj_t::to_int64(int64_t & value) const noexcept 
{
    switch (get_type()) {
    case type_t::NIL:
        value = 0;
        break;

    case type_t::BOOLEAN:
        value = _value.bval ? 1 : 0;
        break;

    case type_t::FLOATING:
        value = static_cast<int64_t>(_value.d64);
        break;

    case type_t::I64:
        value = _value.i64;
        break;

    case type_t::U64:
        value = static_cast<int64_t>(_value.u64);
        break;

    case type_t::STRING:
    {
        try {
            assert(_value.ptr);

            if (!_value.ptr) {
                return false;
            }

            string_t * str_ptr =
                reinterpret_cast<string_t*>(_value.ptr);

            value = std::stoll(*str_ptr);
        }
        catch (std::exception&)
        {
            return false;
        }
        break;
    }

    default:
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool json_obj_t::to_uint64(uint64_t & value) const noexcept
{
    switch (get_type()) {
    case type_t::NIL:
        value = 0;
        break;

    case type_t::BOOLEAN:
        value = _value.bval ? 1 : 0;
        break;

    case type_t::FLOATING:
        value = static_cast<uint64_t>(_value.d64);
        break;

    case type_t::I64:
        value = static_cast<uint64_t>(_value.i64);
        break;

    case type_t::U64:
        value = _value.u64;
        break;

    case type_t::STRING:
    {
        try {
            assert(_value.ptr);

            if (!_value.ptr) {
                return false;
            }

            string_t * str_ptr =
                reinterpret_cast<string_t*>(_value.ptr);

            value = std::stoull(*str_ptr);
        }
        catch (std::exception&)
        {
            return false;
        }
        break;
    }

    default:
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool json_obj_t::to_float(double & value) const noexcept
{
    switch (get_type()) {
    case type_t::NIL:
        value = 0;
        break;

    case type_t::BOOLEAN:
        value = _value.bval ? 1.0 : 0.0;
        break;

    case type_t::FLOATING:
        value = _value.d64;
        break;

    case type_t::I64:
        value = static_cast<double>(_value.i64);
        break;

    case type_t::U64:
        value = static_cast<double>(_value.u64);
        break;

    case type_t::STRING:
    {
        try {
            assert(_value.ptr);

            if (!_value.ptr) {
                return false;
            }

            string_t * str_ptr =
                reinterpret_cast<string_t*>(_value.ptr);

            value = std::stod(*str_ptr);
            break;
        }
        catch (std::exception&) {
            return false;
        }
        break;
    }

    default:
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool json_obj_t::to_bool(bool & value) const noexcept
{
    switch (get_type()) {
    case type_t::NIL:
        value = false;
        break;

    case type_t::BOOLEAN:
        value = _value.bval;
        break;

    case type_t::FLOATING:
        value = _value.d64 != 0;
        break;

    case type_t::I64:
    case type_t::U64:
        value = _value.i64 != 0;
        break;

    case type_t::STRING: {
        assert(_value.ptr);

        if (!_value.ptr) {
            return false;
        }

        string_t * str_ptr =
            reinterpret_cast<string_t*>(_value.ptr);

        auto & str = *str_ptr;

        if (str == _T("true")) {
            value = true;
        }
        else if (str == _T("false")) {
            value = false;
        }
        else {
            try {
                value = std::stoi(str) != 0;
            }
            catch (std::exception&) {
                return false;
            }
        }
        break;

    }

    default:
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool json_obj_t::to_string(string_t & value) const noexcept
{
    switch (get_type()) {
    case type_t::NIL:
        value = _T("null");
        break;

    case type_t::BOOLEAN:
        value = _value.bval ? _T("true") : _T("false");
        break;

    case type_t::FLOATING:
        value = _to_string(_value.d64);
        break;

    case type_t::I64:
        value = _to_string(_value.i64);
        break;

    case type_t::U64:
        value = _to_string(_value.u64);
        break;

    case type_t::STRING:
    {
        assert(_value.ptr);

        if (!_value.ptr) {
            return false;
        }

        string_t * str_ptr =
            reinterpret_cast<string_t*>(_value.ptr);

        value = *str_ptr;

        break;
    }

    default:
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool json_obj_t::push_back(json_obj_t::handle_t&& item) {
    if (!item) {
        return false;
    }

    auto obj_ptr = get_array();
    if (!obj_ptr) {
        return false;
    }

    obj_ptr->push_back(std::move(item));

    return true;
}


/* -------------------------------------------------------------------------- */

size_t json_obj_t::size() const noexcept {
    switch (get_type()) {

    case type_t::ARRAY: {
        const auto ptr = get_array();
        assert(ptr);
        return ptr->size();
    }

    case type_t::OBJECT: {
        const auto ptr = get_object();
        assert(ptr);
        return ptr->size();
    }

    case type_t::BOOLEAN:
    case type_t::NIL:
    case type_t::I64:
    case type_t::U64:
    case type_t::FLOATING:
    case type_t::STRING:
    default:
        break;
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

_ostream & operator<< (_ostream& os, const json_obj_t &jobj) 
{
    const auto& objtype = jobj.get_type();
    using type_t = json_obj_t::type_t;

    switch (objtype) {
    case type_t::NIL:
    case type_t::BOOLEAN:
    case type_t::FLOATING:
    case type_t::I64:
    case type_t::U64: {
        string_t value;
        if (jobj.to_string(value)) {
            os << value;
        }
        break;
    }

    case type_t::STRING: {
        string_t value;
        if (jobj.to_string(value)) {
            os << _T("\"") << value << _T("\"");
        }
        break;
    }

    case type_t::OBJECT: {
        os << _T("{ ");
        auto an_obj = jobj.get_object();
        assert(an_obj);

        if (an_obj) {
            const auto size = an_obj->size();
            json_obj_t::data_t::const_iterator it = an_obj->cbegin();

            for (size_t i = 0; it != an_obj->cend(); ++it, ++i) {
                os << _T("\"") << it->first << _T("\" : ") << *(it->second.get());

                if (size > 1 && i < (size - 1)) {
                    os << _T(", ");
                }
            }
        }

        os << _T(" } ");
        break;
    }

    case type_t::ARRAY: {
        os << _T("[ ");
        auto an_array = jobj.get_array();
        assert(an_array);

        if (an_array) {
            const auto size = an_array->size();
            json_obj_t::array_t::const_iterator it = an_array->cbegin();

            for (size_t i = 0; it != an_array->cend(); ++it, ++i) {
                os << *(it->get());

                if (size > 1 && i < (size - 1)) {
                    os << _T(", ");
                }
            }
        }
        os << _T(" ] ");
        break;
    }
    default:
        break;
    }

    return os;
}


/* -------------------------------------------------------------------------- */

bool json_obj_t::add_member(const string_t& key, json_obj_t::handle_t&& item) 
{
    auto obj_ptr = get_object();
    if (!obj_ptr) {
        return false;
    }

    if (!item) {
        return false;
    }

    obj_ptr->push_back(std::make_pair(key, std::move(item)));

    return true;
}


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */

