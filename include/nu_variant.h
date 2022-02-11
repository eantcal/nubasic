//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_VARIANT_H__
#define __NU_VARIANT_H__


/* -------------------------------------------------------------------------- */

#include "nu_error_codes.h"
#include "nu_exception.h"
#include "nu_stdtype.h"
#include "nu_variable.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <variant>
#include <cmath>


/* -------------------------------------------------------------------------- */

namespace nu {

//! Structure type
struct struct_variant_t {
   explicit struct_variant_t(const std::string& prototype_name) noexcept
      : _prototype_name(prototype_name)
   {
   }

   const std::string& get() const noexcept {
      return _prototype_name;
   }
private:
   std::string _prototype_name;
};


//! Structure type
struct any_variant_t {
   any_variant_t() = default;
};


//! Structure type
struct obj_variant_t {
   obj_variant_t(object_t::handle_t handle) :
      _handle(handle) {}

   object_t::handle_t _handle;
};


/* -------------------------------------------------------------------------- */

/**
 * A variant is a special data type that can contain any kind of typed data
 * Numeric data can be any integer (32 bit) or real number value (float range)
 * Variant data type is used to work with data in a more flexible way.
 * This approach simplifies data manipulation logic of the interpreter.
 */
class variant_t {
protected:
   void _resize(const size_t size);

   template <class T, class DT = T>
   void _set(const T& value, std::vector<DT>& data, variable_t::type_t t)
   {
      _type = t;

      if (data.empty())
         data.resize(1, value);
      else
         data[0] = value;
   }


   template <class T, class DT = T>
   void _set(
      const T& value, std::vector<DT>& data, variable_t::type_t t, const size_t idx)
   {
      _type = t;

      if (idx >= _vect_size)
         _vect_size = idx + 1;

      if (data.empty())
         data.resize(_vect_size, value);
      else
         data[idx] = value;
   }

   template<class sourceT, class destT>
   void init_variant_t(const std::vector<sourceT>& value, const variable_t::type_t TtypeId)
   {
      _type = TtypeId;
      _vect_size = value.size();
      _vector_type = _vect_size >= 1;

      if (_vect_size < 1) {
         _data.resize(1, destT());
      }
      else {
         _data.reserve(value.size());

         for (const auto e : value)
         {
            _data.emplace_back(destT(e));
         }
      }
   }

   template<class sourceT, class destT = sourceT>
   void init_variant_t(const sourceT& value, const variable_t::type_t TtypeId, const size_t vect_size = 0)
   {
      _type = TtypeId;
      _vect_size = vect_size;
      _vector_type = vect_size > 0;

      _data.resize(1 > vect_size ? 1 : vect_size, destT(value));
   }


public:
   using handle_t = std::shared_ptr<variant_t>;
   using type_t = variable_t::type_t;

   static bool is_integer(const std::string& value);
   static bool is_real(const std::string& value);

   explicit variant_t(const any_variant_t&) : _type(type_t::ANY)
   {}

   explicit variant_t(const obj_variant_t&) : _type(type_t::OBJECT)
   {}

   using struct_data_t = std::map<std::string, variant_t::handle_t>;

   explicit variant_t(const std::string& name, const struct_data_t& value, const size_t vect_size = 0)
      : _type(type_t::STRUCT)
      , _struct_data_type_name(name)
   {
      _struct_data.resize(1 > vect_size ? 1 : vect_size);
      _vector_type = vect_size > 0;

      _struct_data[0] = value;
   }

   explicit variant_t(const struct_variant_t& value, const size_t vect_size = 0)
      : variant_t(value.get(), {}, vect_size)
   {}

   static void copy_struct_data(struct_data_t& dst, const struct_data_t& src);

   variant_t(const string_t& value, const type_t t, const size_t vect_size = 0)
   {
      switch (t)
      {
      case type_t::STRING:
         init_variant_t<string_t>(value, t, vect_size);
         break;
      case type_t::INTEGER:
      case type_t::BOOLEAN:
      case type_t::BYTEVECTOR:
      {
         integer_t ivalue{ 0 };
         try {
            ivalue = nu::stoll(value);
         }
         catch (...) {
         }
         init_variant_t<integer_t, integer_t>(integer_t(ivalue), t, vect_size);
      }
      break;
      case type_t::DOUBLE:
      {
         double_t fvalue{ 0 };
         try {
            fvalue = stold(value);
         }
         catch (...) {
         }
         init_variant_t<double_t, double_t>(double_t(fvalue), t, vect_size);
      }
      break;
      default:
         assert(0);
         break;
      }
   }

   variant_t(const string_t& value, const size_t vect_size = 0)
   {
      init_variant_t<string_t>(value, type_t::STRING, vect_size);
   }

   variant_t(const char* value, const size_t vect_size = 0)
   {
      init_variant_t<string_t>(string_t(value), type_t::STRING, vect_size);
   }

   variant_t(const double_t value, const size_t vect_size = 0)
   {
      init_variant_t<double_t>(value, type_t::DOUBLE, vect_size);
   }

   variant_t(const std::vector<double_t>& value)
   {
      init_variant_t<double_t, double_t>(value, type_t::DOUBLE);
   }

   variant_t(const std::vector<byte_t>& value)
   {
      init_variant_t<byte_t, integer_t>(value, type_t::BYTEVECTOR);
   }

   variant_t(const integer_t value, const size_t vect_size = 0)
   {
      init_variant_t<integer_t>(integer_t(value), type_t::INTEGER, vect_size);
   }

   variant_t(const int value, const size_t vect_size = 0)
   {
      init_variant_t<integer_t>(integer_t(value), type_t::INTEGER, vect_size);
   }

   variant_t(const bool_t value, const size_t vect_size = 0)
   {
      init_variant_t<integer_t>(integer_t(value), type_t::BOOLEAN, vect_size);
   }

   variant_t() = default;

   variant_t(const variant_t& v);
   variant_t& operator=(const variant_t& v);

   variant_t(variant_t&& v);
   variant_t& operator=(variant_t&& v);

   // Struct
   void define_struct_member(
      const std::string& field_name, const variant_t& value);

   handle_t struct_member(
      const std::string& field_name, const size_t vector_idx = 0);

   void set_struct_value(const variant_t& v, const size_t vector_idx = 0)
   {
      rt_error_code_t::get_instance().throw_if(_type != type_t::STRUCT
         || v._type != type_t::STRUCT
         || _struct_data_type_name != v._struct_data_type_name
         || vector_idx >= _struct_data.size()
         || v._struct_data.size() != 1,
         0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

      _struct_data[vector_idx] = v._struct_data[0];
   }

   const std::string& struct_type_name() const noexcept {
      return _struct_data_type_name;
   }

   void describe_type(std::stringstream& ss) const noexcept;

   void resize(size_t size) {
      _vector_type = size > 0;
      _vect_size = size;
      _resize(size);
   }

   bool is_vector() const noexcept {
      return _vector_type;
   }

   bool is_struct() const noexcept {
      return _type == type_t::STRUCT;
   }

   size_t vector_size() const noexcept {
      return _vect_size;
   }

   bool copy_vector_content(std::vector<double>& dst) const {

      // Check if we are dealing with vectors
      if (!is_vector() || dst.size() < 1) {
         return false;
      }

      // Do not try to copy more elements of dst capacity
      size_t items_to_copy = vector_size();
      if (dst.size() < items_to_copy) {
         items_to_copy = dst.size();
      }

      for (size_t i = 0; i < items_to_copy; ++i) {
         dst[i] = to_double(i);
      }

      return true;
   }

   double_t to_double(const size_t idx = 0) const;
   integer_t to_int(const size_t idx = 0) const;
   bool to_bool(const size_t idx = 0) const { return to_int(idx) != 0; }

   type_t get_type() const noexcept { return _type; }

   bool is_number() const noexcept { return variable_t::is_number(get_type()); }
   bool is_integral() const noexcept { return variable_t::is_integral(get_type()); }
   bool is_float() const noexcept { return variable_t::is_float(get_type()); }
   bool is_string() const noexcept { return variable_t::is_string(get_type()); }

   static const char* get_type_desc(const type_t& type) noexcept;

   explicit operator float() const { return static_cast<float>(to_double()); }
   explicit operator double_t() const { return to_double(); }
   explicit operator integer_t() const { return to_int(); }
   explicit operator int() const { return static_cast<int>(to_int()); }
   explicit operator unsigned int() const { return static_cast<unsigned int>(to_int()); }
   explicit operator long() const { return static_cast<long>(to_int()); }
   explicit operator unsigned long() const { return static_cast<unsigned long>(to_int()); }
   explicit operator string_t() const { return to_str(); }
   explicit operator bool() const { return to_int() != 0; }

   const string_t to_str(const size_t idx = 0) const;

   void set_str(const string_t& value) { _set(value, _data, type_t::STRING); }
   void set_str(const char* value) { _set<string_t>(value, _data, type_t::STRING); }
   void set_int(const integer_t& value) { _set<integer_t>(value, _data, type_t::INTEGER); }
   void set_double(const double_t value) { _set<double_t>(value, _data, type_t::DOUBLE); }
   void set_bvect(const integer_t value) { _set<integer_t>(value, _data, type_t::BYTEVECTOR); }
   void set_bool(const bool_t value) { _set<integer_t>(value, _data, type_t::BOOLEAN); }
   void set_str(const string_t& value, const size_t idx) { _set<string_t>(value, _data, type_t::STRING, idx); }
   void set_str(const char* value, const size_t idx) { _set<string_t>(value, _data, type_t::STRING, idx); }
   void set_int(const integer_t& value, const size_t idx) { _set<integer_t>(value, _data, type_t::INTEGER, idx); }
   void set_double(const double_t value, const size_t idx) { _set<double_t>(value, _data, type_t::DOUBLE, idx); }
   void set_bvect(const integer_t value, const size_t idx) { _set<integer_t>(value, _data, type_t::BYTEVECTOR, idx); }
   void set_bool(const bool_t value, const size_t idx) { _set<integer_t>(value, _data, type_t::BOOLEAN, idx); }

   variant_t operator[](size_t idx) const;
   friend variant_t operator+(const variant_t& a, const variant_t& b);
   friend variant_t operator-(const variant_t& a, const variant_t& b);
   friend variant_t operator*(const variant_t& a, const variant_t& b);
   friend variant_t operator/(const variant_t& a, const variant_t& b);
   friend variant_t operator<=(const variant_t& a, const variant_t& b);
   friend variant_t operator>=(const variant_t& a, const variant_t& b);
   friend variant_t operator==(const variant_t& a, const variant_t& b);
   friend variant_t operator!=(const variant_t& a, const variant_t& b);
   friend variant_t operator<(const variant_t& a, const variant_t& b);
   friend variant_t operator>(const variant_t& a, const variant_t& b);
   variant_t int_div(const variant_t& b) const;
   variant_t int_mod(const variant_t& b) const;
   variant_t power(const variant_t& b) const;
   variant_t increment();
   variant_t decrement();
   variant_t& operator+=(const variant_t& b);
   variant_t& operator-=(const variant_t& b);

   friend std::ostream& operator<<(std::ostream& os, const variant_t& val);

protected:
   type_t _type = type_t::UNDEFINED;
   size_t _vect_size = 0;
   bool _vector_type = false;

   mutable std::vector<std::variant<string_t, integer_t, double_t>> _data;

   std::vector<struct_data_t> _struct_data;
   std::string _struct_data_type_name;

   template<class T>
   T _at(size_t idx) const {
      rt_error_code_t::get_instance().throw_if(
         idx >= _data.size(), 0, rt_error_code_t::value_t::E_VAL_OUT_OF_RANGE, "");

      const auto& data = _data[idx];

      rt_error_code_t::get_instance().throw_if(
         !std::holds_alternative<T>(data), 0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

      return std::get<T>(data);
   }
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_VARIANT_H__
