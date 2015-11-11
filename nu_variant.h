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
*  Author: <antonino.calderone@ericsson.com>, <acaldmail@gmail.com>
*
*/

#ifndef __NU_VARIANT_H__
#define __NU_VARIANT_H__


/* -------------------------------------------------------------------------- */

#include "nu_stdtype.h"
#include "nu_variable.h"
#include "nu_exception.h"
#include "nu_error_codes.h"

#include <memory>
#include <string>
#include <cassert>
#include <map>
#include <iostream>
#include <vector>
#include <functional>
#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu
{

//! Structure type
struct struct_variant_t
{
   explicit struct_variant_t(const std::string& prototype_name) NU_NOEXCEPT
      : _prototype_name(prototype_name) {}

   const std::string& get() const NU_NOEXCEPT { return _prototype_name; }
private:
   std::string _prototype_name;
};


/* -------------------------------------------------------------------------- */

/**
 * A variant is a special data type that can contain any kind of typed data
 * Numeric data can be any integer (32 bit) or real number value (float range)
 * Variant data type is used to work with data in a more flexible way.
 * This approach simplifies data manipulation logic of the interpreter.
 */
class variant_t
{
protected:
   void _resize(size_t size)
   {
      if (is_struct())
      {
         _struct_data.resize(size);
      }
      else if (is_number())
      {
         if (is_integral())
            _i_data.resize(size);
         else
            _f_data.resize(size);
      }
      else
         _s_data.resize(size);
   }


   template <class T, class DT=T>
   void _set(const T& value, std::vector<DT>& data, variable_t::type_t t)
   {
      _type = t;

      if (data.empty())
         data.resize(1, value);
      else
         data[0] = value;
   }


   template <class T, class DT = T>
   void _set(const T& value, std::vector<DT>& data, variable_t::type_t t, size_t idx)
   {
      _type = t;

      if (idx>=_vect_size)
         _vect_size = idx+1;

      if (data.empty())
         data.resize(_vect_size, value);
      else
         data[idx] = value;
   }
     
   
public:
   using handle_t = std::shared_ptr<variant_t>;
   using type_t = variable_t::type_t;

   static bool is_integer(const std::string& value);
   static bool is_real(const std::string& value);

   variant_t(const struct_variant_t& value, size_t vect_size = 0) :
      variant_t(value.get(), type_t::STRUCT, vect_size) {}

protected:
   using struct_data_t = std::map<std::string, variant_t::handle_t>;

   variant_t(const std::string& name, const struct_data_t& value) :
      _type(type_t::STRUCT),
      _struct_data_type_name(name)
   {
      _struct_data.resize(1);
      _struct_data[0] = value;
   }

public:
   variant_t(const string_t& value, type_t t, size_t vect_size = 0);
   variant_t(const char* value, type_t t, size_t vect_size = 0);
   variant_t(const string_t& value, size_t vect_size = 0);
   variant_t(const char* value, size_t vect_size = 0);
   variant_t(const real_t& value, size_t vect_size = 0);
   variant_t(const double_t& value, size_t vect_size = 0);
   variant_t(const integer_t& value, size_t vect_size = 0);
   variant_t(const bool_t& value, size_t vect_size = 0);
   variant_t(const long64_t& value, size_t vect_size = 0);

   variant_t(const std::vector<byte_t>& value);

   variant_t() = default;

   variant_t(const variant_t& v) = default;
   variant_t& operator =(const variant_t& v);

   variant_t(variant_t&& v);
   variant_t& operator=(variant_t&& v);

   // Struct
   void define_struct_member(
      const std::string& field_name, 
      const variant_t& value);

   handle_t struct_member(
      const std::string& field_name, 
      size_t vector_idx = 0);
   
   const std::string& struct_type_name() const NU_NOEXCEPT
   {
      return _struct_data_type_name;
   }

   void describe_type(std::stringstream & ss) const NU_NOEXCEPT
   {
      if (is_struct())
      {
         ss << "\t\tSTRUCT " << struct_type_name() << "\n";
         ss << "\t\t{\n";

         if (!_struct_data.empty()) for (auto & s : _struct_data[0])
         {
            ss << "\t\t\t" << s.first << " As ";

            if (s.second)
               s.second->describe_type(ss);
            else
               ss << "undefined";

            ss << "\n";
         }
         ss << "\t\t}";
      }
      else
      {
         ss << get_type_desc(get_type());
      }
   }


   bool is_const() const NU_NOEXCEPT
   {
      return _constant;
   }

   bool is_vector() const NU_NOEXCEPT
   {
      return _vector_type;
   }

   bool is_struct() const NU_NOEXCEPT
   {
      return _type == type_t::STRUCT;
   }

   size_t vector_size() const NU_NOEXCEPT
   {
      return _vect_size;
   }

   size_t item_size() const NU_NOEXCEPT;
   size_t size() const NU_NOEXCEPT;

   real_t to_real(size_t idx = 0) const
   {
      return real_t(to_double(idx));
   }

   double_t to_double(size_t idx = 0) const
   {
      rt_error_code_t::get_instance().throw_if(
         _type == type_t::STRUCT,
         0,
         rt_error_code_t::E_TYPE_ILLEGAL,
         "");

      if (is_number())
         return is_integral() ? double_t(_at_i(idx)) : _at_f(idx);

      return nu::stod(_at_s(idx));
   }

   integer_t to_int(size_t idx = 0) const
   {
      return integer_t(to_long64(idx));
   }

   long64_t to_long64(size_t idx = 0) const
   {
      rt_error_code_t::get_instance().throw_if(
         _type == type_t::STRUCT,
         0,
         rt_error_code_t::E_TYPE_ILLEGAL,
         "");

      if (is_number())
         return is_integral() ? _at_i(idx) : long64_t(_at_f(idx));

      return nu::stoll(_at_s(idx));
   }

   bool to_bool(size_t idx = 0) const
   {
      return to_long64(idx) != 0;
   }

   type_t get_type() const NU_NOEXCEPT
   {
      return _type;
   }

   bool is_number() const NU_NOEXCEPT
   {
      return variable_t::is_number(get_type());
   }

   bool is_integral() const NU_NOEXCEPT
   {
      return variable_t::is_integral(get_type());
   }

   bool is_float() const NU_NOEXCEPT
   {
      return variable_t::is_float(get_type());
   }

   static const char* get_type_desc(const type_t& type) NU_NOEXCEPT;

   explicit operator real_t() const
   {
      return to_real();
   }

   explicit operator double_t() const
   {
      return to_double();
   }

   explicit operator integer_t() const
   {
      return to_int();
   }

   explicit operator unsigned int() const
   {
      return to_int();
   }

   explicit operator long() const
   {
      return to_int();
   }

   explicit operator unsigned long() const
   {
      return to_int();
   }

   const string_t& to_str(size_t idx = 0) const
   {
      rt_error_code_t::get_instance().throw_if(
         _type == type_t::STRUCT,
         0,
         rt_error_code_t::E_TYPE_ILLEGAL,
         "");
      
      if (is_number())
      {
         _s_data.resize(idx + 1);

         _s_data[idx] = is_integral() ?
            std::to_string(_at_i(idx)) :
            std::to_string(_at_f(idx));
      }

      return _at_s(idx);
   }

   explicit operator string_t() const
   {
      return to_str();
   }

   explicit operator bool() const
   {
      return to_int() != 0;
   }

   void set_str(const string_t& value)
   {
      _set(value, _s_data, type_t::STRING);
   }

   void set_str(const char* value)
   {
      _set<string_t>(value, _s_data, type_t::STRING);
   }

   void set_int(const integer_t& value)
   {
      _set(value, _i_data, type_t::INTEGER);
   }

   void set_real(real_t value)
   {
      _set(value, _f_data, type_t::FLOAT);
   }

   void set_double(double_t value)
   {
      _set(value, _f_data, type_t::DOUBLE);
   }

   void set_bvect(integer_t value)
   {
      _set(value, _i_data, type_t::BYTEVECTOR);
   }

   void set_bool(bool_t value)
   {
      _set(value, _i_data, type_t::BOOLEAN);
   }

   void set_long64(long64_t value)
   {
      _set(value, _i_data, type_t::LONG64);
   }

   void set_str(const string_t& value, size_t idx)
   {
      _set(value, _s_data, type_t::STRING, idx);
   }

   void set_str(const char* value, size_t idx)
   {
      _set<string_t>(value, _s_data, type_t::STRING, idx);
   }

   void set_int(const integer_t& value, size_t idx)
   {
      _set(value, _i_data, type_t::INTEGER, idx);
   }

   void set_real(real_t value, size_t idx)
   {
      _set(value, _f_data, type_t::FLOAT, idx);
   }

   void set_double(double_t value, size_t idx)
   {
      _set(value, _f_data, type_t::DOUBLE);
   }

   void set_bvect(integer_t value, size_t idx)
   {
      _set(value, _i_data, type_t::BYTEVECTOR, idx);
   }

   void set_bool(bool_t value, size_t idx)
   {
      _set(value, _i_data, type_t::BOOLEAN, idx);
   }

   void set_long64(long64_t value, size_t idx)
   {
      _set(value, _i_data, type_t::LONG64, idx);
   }

   void set_const( bool enable ) NU_NOEXCEPT
   {
      _constant = enable;
   }

   variant_t operator[](size_t idx) const
   {
      if (is_struct())
         return variant_t(struct_type_name(), _struct_data[idx]);
      else
         return is_number() ?
            (is_integral() ? 
               variant_t(_at_i(idx)) : variant_t(_at_f(idx))) : 
            variant_t(_at_s(idx));
   }

   void set_bvect(const std::vector<byte_t>& value);


   friend variant_t operator +(const variant_t& a, const variant_t& b);
   friend variant_t operator -(const variant_t& a, const variant_t& b);
   friend variant_t operator *(const variant_t& a, const variant_t& b);
   friend variant_t operator /(const variant_t& a, const variant_t& b);


   friend variant_t operator <=(const variant_t& a, const variant_t& b);
   friend variant_t operator >=(const variant_t& a, const variant_t& b);
   friend variant_t operator ==(const variant_t& a, const variant_t& b);
   friend variant_t operator !=(const variant_t& a, const variant_t& b);
   friend variant_t operator <(const variant_t& a, const variant_t& b);
   friend variant_t operator >(const variant_t& a, const variant_t& b);

   variant_t int_div(const variant_t& b) const;
   variant_t int_mod(const variant_t& b) const;
   variant_t power(const variant_t&b) const;

   variant_t increment();
   variant_t decrement();

   variant_t& operator+=(const variant_t&b);
   variant_t& operator-=(const variant_t&b);

   friend std::ostream& operator << (std::ostream& os, const variant_t& val);

protected:
   type_t _type = type_t::UNDEFINED;
   bool _constant = false;
   bool _vector_type = false;
   size_t _vect_size = 0;

   mutable std::vector<string_t> _s_data;
   std::vector<long64_t>         _i_data;
   std::vector<double_t>         _f_data;

   std::vector<struct_data_t> _struct_data;
   std::string _struct_data_type_name;

   const std::string& _at_s(size_t idx) const
   {
      rt_error_code_t::get_instance().throw_if(
         idx >= _s_data.size(),
         0,
         rt_error_code_t::E_VAL_OUT_OF_RANGE,
         "");

      return _s_data[idx];
   }

   std::string& _at_s(size_t idx)
   {
      rt_error_code_t::get_instance().throw_if(
         idx >= _s_data.size(),
         0,
         rt_error_code_t::E_VAL_OUT_OF_RANGE,
         "");

      return _s_data[idx];
   }

   const long64_t& _at_i(size_t idx) const
   {
      rt_error_code_t::get_instance().throw_if(
         idx >= _i_data.size(),
         0,
         rt_error_code_t::E_VAL_OUT_OF_RANGE,
         "");

      return _i_data[idx];
   }

   long64_t& _at_i(size_t idx)
   {
      rt_error_code_t::get_instance().throw_if(
         idx >= _i_data.size(),
         0,
         rt_error_code_t::E_VAL_OUT_OF_RANGE,
         "");

      return _i_data[idx];
   }

   const double_t& _at_f(size_t idx) const
   {
      rt_error_code_t::get_instance().throw_if(
         idx >= _f_data.size(),
         0,
         rt_error_code_t::E_VAL_OUT_OF_RANGE,
         "");

      return _f_data[idx];
   }

   double_t& _at_f(size_t idx)
   {
      rt_error_code_t::get_instance().throw_if(
         idx >= _f_data.size(),
         0,
         rt_error_code_t::E_VAL_OUT_OF_RANGE,
         "");

      return _f_data[idx];
   }
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_VARIANT_H__
