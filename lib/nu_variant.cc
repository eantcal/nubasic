//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

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

const char* variant_t::get_type_desc(const type_t& type) noexcept
{
   switch (type) {
   case type_t::BYTEVECTOR:
      return "bytevector";

   case type_t::DOUBLE:
      return "double";

   case type_t::INTEGER:
      return "integer";

   case type_t::BOOLEAN:
      return "boolean";

   case type_t::STRING:
      return "string";

   case type_t::STRUCT:
      return "struct";

   case type_t::ANY:
      return "any";

   case type_t::OBJECT:
      return "object";

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
   if (get_type() == variant_t::type_t::DOUBLE) {
      return variant_t(double_t(pow(to_double(), b.to_double())));
   }

   if (get_type() == variant_t::type_t::INTEGER) {
      return variant_t(integer_t(0.5F + ::pow(to_double(), b.to_double())));
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");

   return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::increment()
{
   if (is_vector()) {
      rt_error_code_t::get_instance().throw_if(
         is_vector(), 0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");
   }

   switch (get_type()) {
   case variant_t::type_t::DOUBLE: {
      _data[0] = std::get<double_t>(_data[0]) + 1.;
      return *this;
   }

   case variant_t::type_t::INTEGER:
   {
      _data[0] = std::get<integer_t>(_data[0]) + integer_t(1);
      return *this;
   }

   case variant_t::type_t::BOOLEAN:
   case variant_t::type_t::STRING:
   case variant_t::type_t::STRUCT:
   case variant_t::type_t::BYTEVECTOR:
   case variant_t::type_t::UNDEFINED:
   default:
      rt_error_code_t::get_instance().throw_if(
         true, 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");
   }


   return *this;
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::decrement()
{
   if (is_vector()) {
      rt_error_code_t::get_instance().throw_if(
         is_vector(), 0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");
   }

   switch (get_type()) {
   case variant_t::type_t::DOUBLE: {
      _data[0] = std::get<double_t>(_data[0]) - 1.;
      return *this;
   }

   case variant_t::type_t::INTEGER:
   {
      _data[0] = std::get<integer_t>(_data[0]) - integer_t(1);
      return *this;
   }

   case variant_t::type_t::BOOLEAN:
   case variant_t::type_t::STRING:
   case variant_t::type_t::STRUCT:
   case variant_t::type_t::BYTEVECTOR:
   case variant_t::type_t::UNDEFINED:
   default:
      rt_error_code_t::get_instance().throw_if(
         true, 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");
   }

   return variant_t();
}


/* -------------------------------------------------------------------------- */

template <class T>
static variant_t binary_operation(std::function<T(integer_t, integer_t)> fint,
   std::function<T(double_t, double_t)> ffloat,
   std::function<T(const string_t&, const string_t&)> fstr, const variant_t& a,
   const variant_t& b)
{
   if (a.get_type() == variant_t::type_t::STRING
      && b.get_type() == variant_t::type_t::STRING) {
      return T(fstr(a.to_str(), b.to_str()));
   }

   if (a.get_type() == variant_t::type_t::DOUBLE) {
      return T(ffloat(a.to_double(), b.to_double()));
   }

   if (a.get_type() == variant_t::type_t::INTEGER) {
      return T(integer_t(fint(a.to_int(), b.to_int())));
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");

   return variant_t(T());
}


/* -------------------------------------------------------------------------- */

template <class T> static inline bool gteq(T a, T b) noexcept { return a >= b; }
template <class T> static inline bool lteq(T a, T b) noexcept { return a <= b; }
template <class T> static inline bool neq(T a, T b) noexcept { return a != b; }
template <class T> static inline bool eq(T a, T b) noexcept { return a == b; }
template <class T> static inline bool gt(T a, T b) noexcept { return a > b; }
template <class T> static inline bool lt(T a, T b) noexcept { return a < b; }
template <class T> static inline T sum(T a, T b) noexcept { return a + b; }


/* -------------------------------------------------------------------------- */

variant_t operator>=(const variant_t& a, const variant_t& b)
{
   return binary_operation<bool_t>(
      gteq<integer_t>, gteq<double_t>, gteq<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator<=(const variant_t& a, const variant_t& b)
{
   return binary_operation<bool_t>(
      lteq<integer_t>, lteq<double_t>, lteq<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator<(const variant_t& a, const variant_t& b)
{
   return binary_operation<bool_t>(
      lt<integer_t>, lt<double_t>, lt<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator>(const variant_t& a, const variant_t& b)
{
   return binary_operation<bool_t>(
      gt<integer_t>, gt<double_t>, gt<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator==(const variant_t& a, const variant_t& b)
{
   if (a.get_type() == variable_t::type_t::BYTEVECTOR
      && b.get_type() == variable_t::type_t::BYTEVECTOR) {
      return a._data == b._data;
   }

   if (a.get_type() == variable_t::type_t::BOOLEAN
      || b.get_type() == variable_t::type_t::BOOLEAN) {
      return a.to_bool() == b.to_bool();
   }

   return binary_operation<bool_t>(
      eq<integer_t>, eq<double_t>, eq<string_t>, a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator!=(const variant_t& a, const variant_t& b)
{
   return !operator==(a, b);
}


/* -------------------------------------------------------------------------- */

variant_t operator+(const variant_t& a, const variant_t& b)
{
   if (a.is_string())
   {
      return variant_t(a._at<string_t>(0) + b.to_str(0));
   }
   else if (a.is_float() || b.is_float())
   {
      return variant_t(double_t(a.to_double() + b.to_double()));
   }
   else if (a.is_integral())
   {
      return variant_t(integer_t(a.to_int() + b.to_int()));
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");

   return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t operator-(const variant_t& a, const variant_t& b)
{
   if (a.is_float() || b.is_float())
   {
      return variant_t(double_t(a.to_double() - b.to_double()));
   }
   else if (a.is_integral())
   {
      return variant_t(integer_t(a.to_int() - b.to_int()));
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");

   return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t operator*(const variant_t& a, const variant_t& b)
{
   if (a.is_float() || b.is_float())
   {
      return variant_t(double_t(a.to_double() * b.to_double()));
   }
   else if (a.is_integral())
   {
      return variant_t(integer_t(a.to_int() * b.to_int()));
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");

   return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t operator/(const variant_t& a, const variant_t& b)
{
   if (variable_t::is_number(a.get_type())
      || variable_t::is_number(b.get_type()))
   {
      rt_error_code_t::get_instance().throw_if(
         b.to_double() == 0.0, 0, rt_error_code_t::value_t::E_DIV_BY_ZERO, "");

      return variant_t(double_t(a.to_double() / b.to_double()));
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");

   return variant_t();
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::int_div(const variant_t& b) const
{
   rt_error_code_t::get_instance().throw_if(
      !is_number(), 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");

   const auto bvalue = b.to_int();

   if (!bvalue) {
      rt_error_code_t::get_instance().throw_if(
         true, 0, rt_error_code_t::value_t::E_DIV_BY_ZERO, "");
   }

   const auto result = to_int() / bvalue;

   return is_float() ?
      variant_t(double_t(result)) :
      variant_t(integer_t(result));
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::int_mod(const variant_t& b) const
{
   rt_error_code_t::get_instance().throw_if(
      !is_number(), 0, rt_error_code_t::value_t::E_TYPE_MISMATCH, "");

   const auto bvalue = b.to_int();

   if (!bvalue) {
      rt_error_code_t::get_instance().throw_if(
         true, 0, rt_error_code_t::value_t::E_DIV_BY_ZERO, "");
   }

   const auto result = to_int() % bvalue;

   return is_float() ?
      variant_t(double_t(result)) :
      variant_t(integer_t(result));
}


/* -------------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const variant_t& val)
{
   const bool bvector = val.is_vector();
   size_t vect_size = bvector ? val.vector_size() : 1;
   bool bigvect = vect_size > 10;

   enum { VECT_LIMIT = 64, VECT_LINES = 4, VECT_COL = 16 };

   os << variant_t::get_type_desc(val.get_type()) << " ";

   if (val.is_vector()) {
      os << "[" << val.vector_size() << "] ";
   }

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

            const auto index = j * VECT_COL + i;

            if (index >= vect_size)
               break;

            os << std::setw(2) << std::setfill('0') << std::hex
               << val.to_int(index) << " ";
         }

         for (size_t i = 0; i < VECT_COL; ++i) {
            const auto index = j * VECT_COL + i;

            if (index >= vect_size) {
               break_loop = true;
               break;
            }

            const auto c = static_cast<char>(val.to_int(index));
            os << (int(c) >= 0x20 /* && int(c) <= 127 (implicit) */ ? c : '.');
         }

         if (break_loop)
            break;
      }

      os << "\n\t";
   }
   else for (size_t vidx = 0; vidx < vect_size; ++vidx) {
      if (bigvect) {
         vect_size = 10;
      }

      if (bvector) {
         os << "[" << vidx << "]:";
      }

      switch (val.get_type()) {
      case variant_t::type_t::DOUBLE:
         os << val.to_double(vidx);
         break;

      case variant_t::type_t::INTEGER:
         os << val.to_int(vidx);
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

      if (bvector) {
         if (vidx < (vect_size - 1)) {
            os << ",";
         }
         os << " ";
      }
   }

   if (bigvect) {
      os << "...";
   }

   return os;
}


/* -------------------------------------------------------------------------- */

bool variant_t::is_integer(const std::string& value)
{
   if (value.empty())
      return false;

   auto is_intexpr = [](char c) {
      return (c >= '0' && c <= '9');
   };

   const char first_char = value.c_str()[0];

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
   if (value.empty()) {
      return false;
   }

   auto is_intexpr = [](char c) {
      return (c >= '0' && c <= '9');
   };

   const char first_char = value.c_str()[0];

   if (!is_intexpr(first_char) && first_char != '-' && first_char != '.') {
      return false;
   }

   if (value.size() == 1) {
      return first_char != '-' && first_char != '.';
   }

   char old_c = 0;
   int point_cnt = 0;
   int E_cnt = 0;

   for (size_t i = 0; i < value.size(); ++i) {
      const char c = value.c_str()[i];

      const bool is_valid = (c == '-' && i == 0) || is_intexpr(c)
         || (c == '.' && point_cnt++ < 1)
         || (::toupper(c) == 'E' && E_cnt++ < 1 && is_intexpr(old_c))
         || ((c == '+' || c == '-') && (::toupper(old_c) == 'E'));

      if (!is_valid) {
         return false;
      }

      old_c = c;
   }

   // we accept also expression like 1E (it is incomplete floating expression
   // parser will verify if next token match with +/- and an integer exponent)
   if (!is_intexpr(old_c) && ::toupper(old_c) != 'E') {
      return false;
   }

   return true;
}


/* -------------------------------------------------------------------------- */

variant_t::variant_t(variant_t&& v)
   : _type(std::move(v._type))
   , _vect_size(std::move(v._vect_size))
   , _vector_type(std::move(v._vector_type))
   , _data(std::move(v._data))
   , _struct_data(std::move(v._struct_data))
   , _struct_data_type_name(std::move(v._struct_data_type_name))
{
}


variant_t::variant_t(const variant_t& v)
   : _type(v._type)
   , _vect_size(v._vect_size)
   , _vector_type(v._vector_type)
   , _data(v._data)
   , _struct_data_type_name(v._struct_data_type_name)
{
   if (_struct_data.size() != v._struct_data.size()) {
      _struct_data.resize(v._struct_data.size());
   }

   for (size_t i = 0; i < v._struct_data.size(); ++i) {
      copy_struct_data(_struct_data[i], v._struct_data[i]);
   }
}


/* -------------------------------------------------------------------------- */

variant_t& variant_t::operator=(variant_t&& v)
{
   if (this != &v) {
      _data = std::move(v._data);
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
      _data = v._data;

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
      _type != type_t::STRUCT, 0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

   const auto hvalue = std::make_shared<variant_t>(value);

   if (_struct_data.empty()) {
      _struct_data.resize(1);
   }

   _struct_data[0].insert(std::make_pair(field_name, hvalue));
}


/* -------------------------------------------------------------------------- */

variant_t::handle_t variant_t::struct_member(
   const std::string& field_name, const size_t vector_idx)
{
   rt_error_code_t::get_instance().throw_if(
      _type != type_t::STRUCT || vector_idx >= _struct_data.size(), 0,
      rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

   const auto it = _struct_data[vector_idx].find(field_name);

   rt_error_code_t::get_instance().throw_if(
      it == _struct_data[vector_idx].end(), 0, rt_error_code_t::value_t::E_INV_IDENTIF,
      "");

   return it->second;
}


/* -------------------------------------------------------------------------- */

void variant_t::_resize(const size_t size)
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
   }
   else
   {
      _data.resize(size);
   }
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
   }
   else {
      ss << get_type_desc(get_type());
      if (is_vector())
         ss << "[" << vector_size() << "]";
   }
}


/* -------------------------------------------------------------------------- */

double_t variant_t::to_double(const size_t idx) const
{
   if (is_number()) {
      return is_integral() ? double_t(_at<integer_t>(idx)) : _at<double_t>(idx);
   }
   else if (is_string())
   {
      return nu::stod(_at<string_t>(idx));
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

   return double_t();
}

/* -------------------------------------------------------------------------- */

integer_t variant_t::to_int(const size_t idx) const
{
   if (is_number())
   {
      return integer_t(is_integral() ? _at<integer_t>(idx) : _at<double_t>(idx));
   }
   else if (is_string())
   {
      return integer_t(nu::stoll(_at<string_t>(idx)));
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

   return integer_t();
}


/* -------------------------------------------------------------------------- */

const string_t variant_t::to_str(const size_t idx) const
{
   if (is_number())
   {
      return string_t(is_integral() ? std::to_string(_at<integer_t>(idx)) : std::to_string(_at<double_t>(idx)));
   }
   else if (is_string())
   {
      return _at<string_t>(idx);
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

   return string_t();
}


/* -------------------------------------------------------------------------- */

variant_t variant_t::operator[](const size_t idx) const
{
   if (is_struct())
   {
      return variant_t(struct_type_name(), _struct_data[idx]);
   }
   else
   {
      if (is_number())
      {
         return is_integral() ? variant_t(_at<integer_t>(idx)) : variant_t(_at<double_t>(idx));
      }

      if (is_string())
      {
         return variant_t(_at <string_t>(idx));
      }
   }

   rt_error_code_t::get_instance().throw_if(
      true, 0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

   return variant_t();
}


/* -------------------------------------------------------------------------- */

void variant_t::copy_struct_data(struct_data_t& dst, const struct_data_t& src)
{
   dst = src;
   for (auto& [key, value] : dst) {
      if (value) {
         value = std::make_shared<variant_t>(*value);
      }
   }
}

/* -------------------------------------------------------------------------- */

} // namespace nu
