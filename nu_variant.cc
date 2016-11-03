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

#include "nu_variant.h"
#include "nu_basic_defs.h"
#include "nu_error_codes.h"
#include "nu_exception.h"
#include "nu_os_std.h"

#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const char* s_value, type_t t, size_t vect_size)
    : _type(t),
      _vect_size(vect_size),
      _vector_type(vect_size > 0)
{
    if (vect_size < 1)
        vect_size = 1;

    if (is_struct()) {
        _struct_data_type_name = s_value;
    } else if (is_number()) {
        if (s_value[0] == '\0')
            s_value = "0";

        if (is_integral())
            _i_data.resize(vect_size, std::stoll(s_value));
        else
            _f_data.resize(vect_size, std::stod(s_value));
    } else {
        _s_data.resize(vect_size, s_value);
    }
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const string_t& value, type_t t, size_t vect_size)
    : variant_t(value.c_str(), t, vect_size)
{
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const string_t& value, size_t vect_size)
    : variant_t(value, type_t::STRING, vect_size)
{
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const char* value, size_t vect_size)
    : variant_t(value, type_t::STRING, vect_size)
{
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const real_t& value, size_t vect_size)
    : _type(type_t::FLOAT)
    , _vect_size(vect_size)
    , _vector_type(vect_size >= 1)
{
    if (vect_size < 1)
        vect_size = 1;

    _f_data.resize(vect_size, value);
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const double_t& value, size_t vect_size)
    : _type(type_t::DOUBLE)
    , _vect_size(vect_size)
    , _vector_type(vect_size >= 1)
{
    if (vect_size < 1)
        vect_size = 1;

    _f_data.resize(vect_size, value);
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const integer_t& value, size_t vect_size)
    : _type(type_t::INTEGER)
    , _vect_size(vect_size)
    , _vector_type(vect_size >= 1)
{
    if (vect_size < 1)
        vect_size = 1;

    _i_data.resize(vect_size, value);
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const bool_t& value, size_t vect_size)
    : _type(type_t::BOOLEAN)
    , _vect_size(vect_size)
    , _vector_type(vect_size >= 1)
{
    if (vect_size < 1)
        vect_size = 1;

    _i_data.resize(vect_size, value);
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const long64_t& value, size_t vect_size)
    : _type(type_t::LONG64)
    , _vect_size(vect_size)
    , _vector_type(vect_size >= 1)
{
    if (vect_size < 1)
        vect_size = 1;

    _i_data.resize(vect_size, value);
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(const std::vector<byte_t>& value)
    : variant_t(0, type_t::BYTEVECTOR, value.size())
{
    for (const auto& e : value)
        _s_data[0].push_back(e);
}


/* -------------------------------------------------------------------------- */

const char* variant_t::get_type_desc(const type_t& type) noexcept
{
    switch (type) {
    case type_t::BYTEVECTOR:
        return "bytevector";

    case type_t::FLOAT:
        return "float";

    case type_t::DOUBLE:
        return "double";

    case type_t::INTEGER:
        return "integer";

    case type_t::BOOLEAN:
        return "boolean";

    case type_t::LONG64:
        return "long64";

    case type_t::STRING:
        return "string";

    case type_t::STRUCT:
        return "struct";

    case type_t::UNDEFINED:
        break;

    default:
        assert(0);
    }

    return "undef";
}


/* -------------------------------------------------------------------------- */

variant_t& variant_t::operator+=(const variant_t& b)
{
    *this = *this + b;
    return *this;
}


/* -------------------------------------------------------------------------- */

variant_t& variant_t::operator-=(const variant_t& b)
{
    *this = *this - b;
    return *this;
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::power(const variant_t& b) const
{
    if (get_type() == variant_t::type_t::DOUBLE
        || b.get_type() == variant_t::type_t::DOUBLE) {
        return variant_t(double_t(pow(to_double(), b.to_double())));
    }

    if (get_type() == variant_t::type_t::FLOAT
        || b.get_type() == variant_t::type_t::FLOAT) {
        return variant_t(real_t(pow(to_real(), b.to_real())));
    }

    if (get_type() == variant_t::type_t::LONG64
        || b.get_type() == variant_t::type_t::LONG64) {
        return variant_t(long64_t(0.5F + ::pow(to_double(), b.to_double())));
    }

    if (get_type() == variant_t::type_t::INTEGER) {
        return variant_t(integer_t(0.5F + ::pow(to_double(), b.to_double())));
    }

    rt_error_code_t::get_instance().throw_if(
        true, 0, rt_error_code_t::E_TYPE_MISMATCH, "");

    return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::increment()
{
    if (is_vector()) {
        rt_error_code_t::get_instance().throw_if(
            is_vector(), 0, rt_error_code_t::E_TYPE_ILLEGAL, "");
    }

    switch (get_type()) {
    case variant_t::type_t::FLOAT:
    case variant_t::type_t::DOUBLE: {
        ++_f_data[0];
        return *this;
    }

    case variant_t::type_t::INTEGER:
    case variant_t::type_t::LONG64: {
        ++_i_data[0];
        return *this;
    }

    case variant_t::type_t::BOOLEAN:
    case variant_t::type_t::STRING:
    case variant_t::type_t::STRUCT:
    case variant_t::type_t::BYTEVECTOR:
    case variant_t::type_t::UNDEFINED:
    default:
        rt_error_code_t::get_instance().throw_if(
            true, 0, rt_error_code_t::E_TYPE_MISMATCH, "");
    }


    return *this;
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::decrement()
{
    if (is_vector()) {
        rt_error_code_t::get_instance().throw_if(
            is_vector(), 0, rt_error_code_t::E_TYPE_ILLEGAL, "");
    }

    switch (get_type()) {
    case variant_t::type_t::FLOAT:
    case variant_t::type_t::DOUBLE: {
        --_f_data[0];
        return *this;
    }

    case variant_t::type_t::INTEGER:
    case variant_t::type_t::LONG64: {
        --_i_data[0];
        return *this;
    }

    case variant_t::type_t::BOOLEAN:
    case variant_t::type_t::STRING:
    case variant_t::type_t::STRUCT:
    case variant_t::type_t::BYTEVECTOR:
    case variant_t::type_t::UNDEFINED:
    default:
        rt_error_code_t::get_instance().throw_if(
            true, 0, rt_error_code_t::E_TYPE_MISMATCH, "");
    }

    return variant_t();
}


/* -------------------------------------------------------------------------- */

template <class T>
static variant_t binary_operation(std::function<T(long64_t, long64_t)> fint,
    std::function<T(double_t, double_t)> ffloat,
    std::function<T(const string_t&, const string_t&)> fstr, const variant_t& a,
    const variant_t& b)
{
    if (a.get_type() == variant_t::type_t::STRING
        && b.get_type() == variant_t::type_t::STRING) {
        return T(fstr(a.to_str(), b.to_str()));
    }

    if (a.get_type() == variant_t::type_t::DOUBLE
        || b.get_type() == variant_t::type_t::DOUBLE) {
        return T(ffloat(a.to_double(), b.to_double()));
    }

    if (a.get_type() == variant_t::type_t::FLOAT
        || b.get_type() == variant_t::type_t::FLOAT) {
        return T(ffloat((double_t)a.to_real(), (double_t)b.to_real()));
    }

    if (a.get_type() == variant_t::type_t::LONG64
        || b.get_type() == variant_t::type_t::LONG64) {
        return T(fint(a.to_long64(), b.to_long64()));
    }

    if (a.get_type() == variant_t::type_t::INTEGER
        || b.get_type() == variant_t::type_t::INTEGER) {
        return T(integer_t(fint(a.to_int(), b.to_int())));
    }

    rt_error_code_t::get_instance().throw_if(
        true, 0, rt_error_code_t::E_TYPE_MISMATCH, "");

    return variant_t(T());
}


/* -------------------------------------------------------------------------- */

template <class T> static inline bool gteq(T a, T b) noexcept { return a >= b; }


/* -------------------------------------------------------------------------- */

template <class T> static inline bool lteq(T a, T b) noexcept { return a <= b; }


/* -------------------------------------------------------------------------- */

template <class T> static inline bool neq(T a, T b) noexcept { return a != b; }


/* -------------------------------------------------------------------------- */

template <class T> static inline bool eq(T a, T b) noexcept { return a == b; }


/* -------------------------------------------------------------------------- */

template <class T> static inline bool gt(T a, T b) noexcept { return a > b; }

/* -------------------------------------------------------------------------- */

template <class T> static inline bool lt(T a, T b) noexcept { return a < b; }


/* -------------------------------------------------------------------------- */

template <class T> static inline T sum(T a, T b) noexcept { return a + b; }


/* -------------------------------------------------------------------------- */

variant_t operator>=(const variant_t& a, const variant_t& b)
{
    return binary_operation<bool_t>(
        gteq<long64_t>, gteq<double_t>, gteq<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator<=(const variant_t& a, const variant_t& b)
{
    return binary_operation<bool_t>(
        lteq<long64_t>, lteq<double_t>, lteq<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator<(const variant_t& a, const variant_t& b)
{
    return binary_operation<bool_t>(
        lt<long64_t>, lt<double_t>, lt<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator>(const variant_t& a, const variant_t& b)
{
    return binary_operation<bool_t>(
        gt<long64_t>, gt<double_t>, gt<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator==(const variant_t& a, const variant_t& b)
{
    if (a.get_type() == variable_t::type_t::BYTEVECTOR
        && b.get_type() == variable_t::type_t::BYTEVECTOR) {
        return a._s_data == b._s_data;
    }

    if (a.get_type() == variable_t::type_t::BOOLEAN
        || b.get_type() == variable_t::type_t::BOOLEAN) {
        return a.to_bool() == b.to_bool();
    }

    return binary_operation<bool_t>(
        eq<long64_t>, eq<double_t>, eq<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator!=(const variant_t& a, const variant_t& b)
{
    if (a.get_type() == variable_t::type_t::BYTEVECTOR
        && b.get_type() == variable_t::type_t::BYTEVECTOR) {
        return a._s_data != b._s_data;
    }

    if (a.get_type() == variable_t::type_t::BOOLEAN
        || b.get_type() == variable_t::type_t::BOOLEAN) {
        return a.to_bool() != b.to_bool();
    }

    return binary_operation<bool_t>(
        neq<long64_t>, neq<double_t>, neq<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator+(const variant_t& a, const variant_t& b)
{
    if (!a.is_number()) {
        rt_error_code_t::get_instance().throw_if(
            a.get_type() != variable_t::type_t::STRING
                || b.get_type() != variable_t::type_t::STRING,
            0, rt_error_code_t::E_TYPE_ILLEGAL, "");

        return variant_t(a._at_s(0) + b._at_s(0));
    }

    if (a.is_float()) {
        if (b.is_float())
            return variant_t(a._at_f(0) + b._at_f(0));
        else
            return variant_t(a._at_f(0) + b.to_double());
    } else if (b.is_float()) {
        return variant_t(a.to_double() + b._at_f(0));
    }

    return variant_t(a._at_i(0) + b._at_i(0));
}


/* -------------------------------------------------------------------------- */

variant_t operator-(const variant_t& a, const variant_t& b)
{
    if (!a.is_number() || !b.is_number()) {
        rt_error_code_t::get_instance().throw_if(
            true, 0, rt_error_code_t::E_TYPE_MISMATCH, "");
    }

    if (a.get_type() == variant_t::type_t::DOUBLE
        || b.get_type() == variant_t::type_t::DOUBLE) {
        return variant_t(double_t(a.to_double() - b.to_double()));
    }

    if (a.get_type() == variant_t::type_t::FLOAT
        || b.get_type() == variant_t::type_t::FLOAT) {
        return variant_t(real_t(a.to_real() - b.to_real()));
    }

    if (a.get_type() == variant_t::type_t::LONG64
        || b.get_type() == variant_t::type_t::LONG64) {
        return variant_t(long64_t(a.to_long64() - b.to_long64()));
    }

    if (a.get_type() == variant_t::type_t::INTEGER
        || b.get_type() == variant_t::type_t::INTEGER) {
        return variant_t(integer_t(a.to_int() - b.to_int()));
    }

    return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t operator/(const variant_t& a, const variant_t& b)
{
    if (variable_t::is_number(a.get_type())
        || variable_t::is_number(a.get_type())) {
        rt_error_code_t::get_instance().throw_if(
            b.to_double() == 0.0, 0, rt_error_code_t::E_DIV_BY_ZERO, "");

        return variant_t(double_t(a.to_double() / b.to_double()));
    }

    rt_error_code_t::get_instance().throw_if(
        true, 0, rt_error_code_t::E_TYPE_MISMATCH, "");

    return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::int_div(const variant_t& b) const
{
    if (!is_integral() || !b.is_integral()) {
        rt_error_code_t::get_instance().throw_if(
            true, 0, rt_error_code_t::E_TYPE_MISMATCH, "");
    }

    if (get_type() == variant_t::type_t::LONG64
        || b.get_type() == variant_t::type_t::LONG64) {
        rt_error_code_t::get_instance().throw_if(
            b.to_long64() == 0, 0, rt_error_code_t::E_DIV_BY_ZERO, "");

        return variant_t(long64_t(to_long64() / b.to_long64()));
    }

    if (get_type() == variant_t::type_t::INTEGER
        || b.get_type() == variant_t::type_t::INTEGER) {
        rt_error_code_t::get_instance().throw_if(
            b.to_int() == 0, 0, rt_error_code_t::E_DIV_BY_ZERO, "");

        return variant_t(integer_t(to_int() / b.to_int()));
    }

    return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::int_mod(const variant_t& b) const
{
    if (!is_integral() || !b.is_integral()) {
        rt_error_code_t::get_instance().throw_if(
            true, 0, rt_error_code_t::E_TYPE_MISMATCH, "");
    }

    if (b.to_long64() == 0) {
        rt_error_code_t::get_instance().throw_if(
            true, 0, rt_error_code_t::E_DIV_BY_ZERO, "");
    }

    if (get_type() == variant_t::type_t::LONG64
        || b.get_type() == variant_t::type_t::LONG64) {
        return variant_t(long64_t(to_long64() % b.to_long64()));
    }

    if (get_type() == variant_t::type_t::INTEGER
        || b.get_type() == variant_t::type_t::INTEGER) {
        return variant_t(integer_t(to_int() % b.to_int()));
    }

    return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t operator*(const variant_t& a, const variant_t& b)
{
    if (!a.is_number() || !b.is_number()) {
        rt_error_code_t::get_instance().throw_if(
            true, 0, rt_error_code_t::E_TYPE_MISMATCH, "");
    }

    if (a.is_float()) {
        return b.is_float() ? variant_t(a._at_f(0) * b._at_f(0))
                            : variant_t(a._at_f(0) * b.to_double());
    } else if (b.is_float()) {
        return variant_t(a.to_double() * b._at_f(0));
    }

    return variant_t(a._at_i(0) * b._at_i(0));
}


/* -------------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const variant_t& val)
{
    bool bvector = val.is_vector();
    size_t vect_size = bvector ? val.vector_size() : 1;
    bool bigvect = vect_size > 10;

    enum { VECT_LIMIT = 64, VECT_LINES = 4, VECT_COL = 16 };

    os << variant_t::get_type_desc(val.get_type()) << " ";

    if (val.is_vector())
        os << "[" << val.vector_size() << "] ";

    os << "=";

    if (val.get_type() == variant_t::type_t::BYTEVECTOR) {
        if (vect_size > VECT_LIMIT) {
            vect_size = VECT_LIMIT;
            bigvect = true;
        }

        else {
            bigvect = false;
        }

        bool break_loop = false;

        for (int j = 0; j < VECT_LINES; ++j) {
            for (size_t i = 0; i < VECT_COL; ++i) {
                if ((i & (VECT_COL - 1)) == 0)
                    os << "\n\t >";

                auto index = j * VECT_COL + i;

                if (index >= vect_size)
                    break;

                os << std::setw(2) << std::setfill('0') << std::hex
                   << val.to_int(index) << " ";
            }

            for (size_t i = 0; i < VECT_COL; ++i) {
                auto index = j * VECT_COL + i;

                if (index >= vect_size) {
                    break_loop = true;
                    break;
                }

                char c = val.to_int(index);
                os << (int(c) >= 0x20 && int(c) <= 127 ? c : '.');
            }

            if (break_loop)
                break;
        }

        os << "\n\t";
    } else
        for (size_t vidx = 0; vidx < vect_size; ++vidx) {
            if (bigvect)
                vect_size = 10;

            if (bvector)
                os << "[" << vidx << "]:";

            switch (val.get_type()) {
            case variant_t::type_t::DOUBLE:
                os << val.to_double(vidx);
                break;

            case variant_t::type_t::FLOAT:
                os << val.to_real(vidx);
                break;

            case variant_t::type_t::INTEGER:
                os << val.to_int(vidx);
                break;

            case variant_t::type_t::LONG64:
                os << val.to_long64(vidx);
                break;

            case variant_t::type_t::BOOLEAN:
                os << val.to_bool(vidx);
                break;

            case variant_t::type_t::STRING:
                os << "\"" << val.to_str(vidx) << "\"";
                break;

            case variant_t::type_t::STRUCT: {
                os << " " << val._struct_data_type_name << "\n";

                os << "\t\t{\n";

                if (val._struct_data.size() > vidx)
                    for (const auto& e : val._struct_data[vidx]) {
                        os << "\t\t  " << e.first << " : ";
                        if (e.second)
                            os << *e.second << "\n";
                    }

                os << "\t\t}";
                break;
            }

            default:
                os << val.to_str(vidx);
                break;
            }

            if (bvector && vidx < (vect_size - 1))
                os << ", ";

            else if (bvector)
                os << " ";
        }

    if (bigvect)
        os << "...";

    return os;
}


/* -------------------------------------------------------------------------- */

bool variant_t::is_integer(const std::string& value)
{
    if (value.empty())
        return false;

    auto is_intexpr = [](char c) { return (c >= '0' && c <= '9'); };

    char first_char = value.c_str()[0];

    if (!is_intexpr(first_char) && first_char != '-')
        return false;

    if (value.size() == 1)
        return first_char != '-';

    for (size_t i = 1; i < value.size(); ++i) {
        const char c = value.c_str()[i];

        if (!is_intexpr(c))
            return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool variant_t::is_real(const std::string& value)
{
    if (value.empty())
        return false;

    auto is_intexpr = [](char c) { return (c >= '0' && c <= '9'); };

    char first_char = value.c_str()[0];

    if (!is_intexpr(first_char) && first_char != '-' && first_char != '.') {
        return false;
    }

    if (value.size() == 1)
        return first_char != '-' && first_char != '.';

    char old_c = 0;
    int point_cnt = 0;
    int E_cnt = 0;

    for (size_t i = 0; i < value.size(); ++i) {
        const char c = value.c_str()[i];

        bool is_valid = (c == '-' && i == 0) || is_intexpr(c)
            || (c == '.' && point_cnt++ < 1)
            || (::toupper(c) == 'E' && E_cnt++ < 1 && is_intexpr(old_c))
            || ((c == '+' || c == '-') && (::toupper(old_c) == 'E'));

        if (!is_valid)
            return false;

        old_c = c;
    }

    // we accept also expression like 1E (it is incomplete floating expression
    // parser will verify if next token match with +/- and an integer exponent)
    if (!is_intexpr(old_c) && ::toupper(old_c) != 'E')
        return false;

    return true;
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(variant_t&& v)
    : _type(std::move(v._type))
    , _vect_size(std::move(v._vect_size))
    , _vector_type(std::move(v._vector_type))
    , _s_data(std::move(v._s_data))
    , _i_data(std::move(v._i_data))
    , _f_data(std::move(v._f_data))
    , _struct_data(std::move(v._struct_data))
    , _struct_data_type_name(std::move(v._struct_data_type_name))
{
}


variant_t::variant_t(const variant_t& v)
    : _type(v._type)
    , _vect_size(v._vect_size)
    , _vector_type(v._vector_type)
    , _s_data(v._s_data)
    , _i_data(v._i_data)
    , _f_data(v._f_data)
    , _struct_data_type_name(v._struct_data_type_name)
{
    if (_struct_data.size() != v._struct_data.size())
        _struct_data.resize(v._struct_data.size());

    for (size_t i = 0; i < v._struct_data.size(); ++i)
        copy_struct_data(_struct_data[i], v._struct_data[i]);
}


/* -------------------------------------------------------------------------- */

variant_t& variant_t::operator=(variant_t&& v)
{
    if (this != &v) {
        _i_data = std::move(v._i_data);
        _f_data = std::move(v._f_data);
        _s_data = std::move(v._s_data);
        _struct_data = std::move(v._struct_data);
        _struct_data_type_name = std::move(v._struct_data_type_name);

        _vector_type = std::move(v._vector_type);
        _vect_size = std::move(v._vect_size);
        _type = std::move(v._type);
    }

    return *this;
}


/* -------------------------------------------------------------------------- */

variant_t& variant_t::operator=(const variant_t& v)
{
    if (this != &v) {
        _i_data = v._i_data;
        _f_data = v._f_data;
        _s_data = v._s_data;

        if (_struct_data.size() != v._struct_data.size())
            _struct_data.resize(v._struct_data.size());

        for (size_t i = 0; i < v._struct_data.size(); ++i)
            copy_struct_data(_struct_data[i], v._struct_data[i]);

        _struct_data_type_name = v._struct_data_type_name;

        _vector_type = v._vector_type;
        _vect_size = v._vect_size;
        _type = v._type;
    }

    return *this;
}


/* -------------------------------------------------------------------------- */

void variant_t::define_struct_member(
    const std::string& field_name, const variant_t& value)
{
    rt_error_code_t::get_instance().throw_if(
        _type != type_t::STRUCT, 0, rt_error_code_t::E_TYPE_ILLEGAL, "");

    auto hvalue = std::make_shared<variant_t>(value);

    if (_struct_data.empty())
        _struct_data.resize(1);

    _struct_data[0].insert(std::make_pair(field_name, hvalue));
}


/* -------------------------------------------------------------------------- */

variant_t::handle_t variant_t::struct_member(
    const std::string& field_name, size_t vector_idx)
{
    rt_error_code_t::get_instance().throw_if(
        _type != type_t::STRUCT || vector_idx >= _struct_data.size(), 0,
        rt_error_code_t::E_TYPE_ILLEGAL, "");

    auto it = _struct_data[vector_idx].find(field_name);

    rt_error_code_t::get_instance().throw_if(
        it == _struct_data[vector_idx].end(), 0, rt_error_code_t::E_INV_IDENTIF,
        "");

    return it->second;
}


/* -------------------------------------------------------------------------- */

void variant_t::_resize(size_t size)
{
    if (is_struct()) {
        if (size != _struct_data.size()) {
            _struct_data.resize(size);

            if (size > 0) {
                auto it = _struct_data.begin();
                auto prototype = *it;
                ++it;
                for (; it != _struct_data.end(); ++it)
                    copy_struct_data(*it, prototype);
            }
        }
    } else if (is_number()) {
        if (is_integral())
            _i_data.resize(size);
        else
            _f_data.resize(size);
    } else
        _s_data.resize(size);
}


/* -------------------------------------------------------------------------- */

void variant_t::describe_type(std::stringstream& ss) const noexcept
{
    if (is_struct()) {
        ss << "\t\tStruct " << struct_type_name() << "\n";
        ss << "\t\t{\n";

        for (size_t i = 0; i < _struct_data.size(); ++i)
            if (!_struct_data.empty())
                for (auto& s : _struct_data[i]) {
                    ss << "\t\t  " << s.first << " As ";

                    if (s.second)
                        s.second->describe_type(ss);
                    else
                        ss << "undefined";

                    if (is_vector())
                        ss << "[" << i << "]";

                    ss << "\n";
                }
        ss << "\t\t}";
    } else {
        ss << get_type_desc(get_type());
        if (is_vector())
            ss << "[" << vector_size() << "]";
    }
}


/* -------------------------------------------------------------------------- */

double_t variant_t::to_double(size_t idx) const
{
    rt_error_code_t::get_instance().throw_if(
        _type == type_t::STRUCT, 0, rt_error_code_t::E_TYPE_ILLEGAL, "");

    if (is_number())
        return is_integral() ? double_t(_at_i(idx)) : _at_f(idx);

    return nu::stod(_at_s(idx));
}


/* -------------------------------------------------------------------------- */

long64_t variant_t::to_long64(size_t idx) const
{
    rt_error_code_t::get_instance().throw_if(
        _type == type_t::STRUCT, 0, rt_error_code_t::E_TYPE_ILLEGAL, "");

    if (is_number())
        return is_integral() ? _at_i(idx) : long64_t(_at_f(idx));

    return nu::stoll(_at_s(idx));
}


/* -------------------------------------------------------------------------- */

const string_t& variant_t::to_str(size_t idx) const
{
    rt_error_code_t::get_instance().throw_if(
        _type == type_t::STRUCT, 0, rt_error_code_t::E_TYPE_ILLEGAL, "");

    if (is_number()) {
        _s_data.resize(idx + 1);

        _s_data[idx] = is_integral() ? std::to_string(_at_i(idx))
            : std::to_string(_at_f(idx));
    }

    return _at_s(idx);
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::operator[](size_t idx) const
{
    if (is_struct())
        return variant_t(struct_type_name(), _struct_data[idx]);
    else
        return is_number() ? (is_integral() ? variant_t(_at_i(idx))
            : variant_t(_at_f(idx)))
        : variant_t(_at_s(idx));
}


/* -------------------------------------------------------------------------- */

void variant_t::copy_struct_data(struct_data_t& dst, const struct_data_t& src)
{
    dst = src;
    for (auto& e : dst) {
        if (e.second)
            e.second = std::make_shared<variant_t>(*e.second);
    }
}

/* -------------------------------------------------------------------------- */

} // namespace nu
