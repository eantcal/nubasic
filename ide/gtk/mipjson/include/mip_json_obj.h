//  
// This file is part of MipJson Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_JSON_OBJ_H__
#define __MIP_JSON_OBJ_H__


/* -------------------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <cassert>
#include <type_traits>

#include "mip_unicode.h"
#include "mip_exception.h"


/* -------------------------------------------------------------------------- */

namespace mip
{


/* -------------------------------------------------------------------------- */

/**
 * @brief json_obj_t class represents a variant JSON object value.
 *
 * It represents null, boolean, floating point, integer, unsigned integer,
 * (unicode) strings, array of objects, collection of zero or more 
 * string/object pairs (object members) JSON types
 */
class json_obj_t
{
public:
    // Typedefs
    enum class type_t : int {
        NIL,       ///< JSON null
        BOOLEAN,   ///< JSON boolean 
        FLOATING,  ///< JSON floating point 
        I64,       ///< JSON signed integer
        U64,       ///< JSON unsigned integer
        STRING,    ///< JSON string 
        ARRAY,     ///< JSON array 
        OBJECT     ///< JSON list of pairs (name, obj)
    };

    //! json_obj_t value or pointer to complex object instances
    //! (e.g. JSON objects, strings and arrays)
    union value_t {
        uint64_t u64;
        int64_t  i64;
        bool     bval;
        void *   ptr;
        double   d64;
    };

    //! Represents a std::unique_ptr of JSON object 
    //! (instance of json_obj_t)
    using handle_t = std::unique_ptr< json_obj_t >;

    //! Represents a JSON object member (as name / value pair)
    using member_t = std::pair<string_t, handle_t>;

    //! Represents an object value which is an unordered collection of 
    //! zero or more handles of JSON object members 
    using data_t = std::list< member_t >;

    //! Represents a JSON array of zero or more handles of 
    //! JSON objects (instances of json_obj_t)
    using array_t = std::vector< handle_t >;


    // Ctors

    //! Default ctor
    json_obj_t() noexcept : _type(type_t::NIL) {}

    //! Copy ctor
    json_obj_t(const json_obj_t& other);


    //! Move ctor
    json_obj_t(json_obj_t&& other) noexcept {
        _value = other._value;
        _type = other._type;
        other._type = type_t::NIL;
    }

    /** This function returns a pointer to the 
     * managed object and releases the ownership
     * Object's type is set to type_t::NIL.
     * @return pointer to the managed object or nullptr
     * if the object is a plain type object
     */
    void* release() noexcept {
        if (is_plain()) {
            return nullptr;
        }

        auto ret = _value.ptr;
        _value.ptr = nullptr;
        _type = type_t::NIL;

        return ret;
    }


    /** Constructs a json object from a data pointer (managed object)
     * Object will be freed by the dtor, unless relese() method is invoked
     * @param value is a pointer to an data_t instance or nullptr
     * @note value is expected to be allocated by global new operator
     * @note if value is null an empty object will be automatically created
     */
    explicit json_obj_t(data_t* value) noexcept : _type(type_t::OBJECT) {

        if (!value) {
            value = new data_t;
            assert(value);
        }

        _value.ptr = value;
    }

    //! Constructs a json object from an object value
    explicit json_obj_t(data_t&& value) noexcept : _type(type_t::OBJECT) {
        auto obj_ptr = new data_t;
        assert(obj_ptr);

        _value.ptr = obj_ptr;
    }

    /** Constructs a json object from an array pointer
     * Array will be freed by the dtor
     * Value is expected to be allocated by global new operator
     */
    explicit json_obj_t(array_t* value) noexcept : _type(type_t::ARRAY) {
        if (!value) {
            value = new array_t;
            assert(value);
        }
        _value.ptr = value;
    }

    //! Constructs a json object from an array object
    explicit json_obj_t(array_t&& value) noexcept : _type(type_t::ARRAY) {
        auto obj_ptr = new array_t(std::move(value));
        assert(obj_ptr);

        _value.ptr = obj_ptr;
    }

    //! Constructs a json object from a string object
    json_obj_t(const string_t& value) noexcept : _type(type_t::STRING) {
        _value.ptr = new string_t(value);
        assert(_value.ptr);
    }

    //! Constructs a json object from a string object
    explicit json_obj_t(string_t* value) noexcept : _type(type_t::STRING) {
        assert(_value.ptr);
        _value.ptr = value;
    }

    //! Constructs a json object from a null-terminated string object
    explicit json_obj_t(const char_t* value) noexcept : _type(type_t::STRING) {
        if (!value) {
            _value.ptr = new string_t();
        }
        else {
            _value.ptr = new string_t(value);
        }
        assert(_value.ptr);
    }

    //! Constructs a json object from a boolean
    explicit json_obj_t(const bool& value) noexcept : _type(type_t::BOOLEAN) {
        _value.bval = value;
    }

    //! Constructs a json object from an unsigned integer
    json_obj_t(const uint64_t& value) noexcept : _type(type_t::U64) {
        _value.u64 = value;
    }

    //! Constructs a json object from an 32 bit unsigned integer
    explicit json_obj_t(const uint32_t& value) noexcept : 
        json_obj_t(uint64_t(value)) 
    {}

    //! Constructs a json object from an 16 bit unsigned integer
    explicit json_obj_t(const uint16_t& value) noexcept : 
        json_obj_t(uint64_t(value)) 
    {}

    //! Constructs a json object from an 8 bit unsigned integer
    explicit json_obj_t(const uint8_t& value) noexcept : 
        json_obj_t(uint64_t(value)) 
    {}

    //! Constructs a json object from an integer
    json_obj_t(const int64_t& value) noexcept : _type(type_t::I64) {
        _value.i64 = value;
    }
    
    //! Constructs a json object from an 32 bit signed integer
    explicit json_obj_t(const int32_t& value) noexcept : 
        json_obj_t(int64_t(value)) 
    {}

    //! Constructs a json object from an 16 bit signed integer
    explicit json_obj_t(const int16_t& value) noexcept : 
        json_obj_t(int64_t(value)) 
    {}

    //! Constructs a json object from an 8 bit signed integer
    explicit json_obj_t(const int8_t& value) noexcept : 
        json_obj_t(int64_t(value)) 
    {}

    //! Constructs a json object from a floating point (double)
    json_obj_t(const double& value) noexcept : _type(type_t::FLOATING) {
        _value.d64 = value;
    }

    //! Constructs a json object from a floating point (float)
    explicit json_obj_t(const float& value) noexcept : 
        json_obj_t(double(value)) 
    {}


    //! Dtor
    ~json_obj_t() noexcept;


    //! Assignment operator
    json_obj_t& operator=(const json_obj_t& other);


    //! Move-assignment operator
    json_obj_t& operator=(json_obj_t&& other) noexcept {

        if (this != &other) {
            _value = other._value;
            _type = other._type;
            other._type = type_t::NIL;
        }

        return *this;
    }


    //! Clones the entire vairant object hierarchy
    handle_t clone() const noexcept;

    //! Returns the managed object type
    const type_t& get_type() const noexcept {
        return _type;
    }

    //! Returns true if object represents null, false otherwise
    bool is_null() const noexcept {
        return get_type() == type_t::NIL;
    }

    //! Returns true if object represents a number, false otherwise
    bool is_number() const noexcept {
        const auto i_type = static_cast<int>(get_type());

        return
            (i_type >= static_cast<int>(type_t::FLOATING) &&
                i_type <= static_cast<int>(type_t::U64));
    }

    //! Returns true if object represents an structured object, false otherwise
    bool has_members() const noexcept {
        return (get_type() == type_t::OBJECT);
    }

    //! Returns true if object represents an array, false otherwise
    bool is_array() const noexcept {
        return (get_type() == type_t::ARRAY);
    }

    //! Returns true if variant does not represent an object nor an array, 
    //! false otherwise
    bool is_plain() const noexcept {
        return !has_members() && !is_array();
    }

    //! Returns true if object represents a string, false otherwise
    bool is_string() const noexcept {
        return get_type() == type_t::STRING;
    }

    //! Returns true if object represents an integer, false otherwise
    bool is_integer() const noexcept {
        return get_type() == type_t::I64 || get_type() == type_t::U64;
    }

    //! Returns true if object represents an unsigned integer, false otherwise
    bool is_unsigned() const noexcept {
        return get_type() == type_t::U64;
    }

    //! Returns true if object represents a boolean value, false otherwise
    bool is_bool() const noexcept {
        return get_type() == type_t::BOOLEAN;
    }


    // Basic type conversion

    /** Returns an integer representation of an object
     * @param value is converted output 
     * @return true if conversion is perfomed, false otherwise
     */
    bool to_int64(int64_t & value) const noexcept;


    /** Returns an unsigned integer representation of an object
     * @param value is converted output 
     * @return true if conversion is perfomed, false otherwise
     */
    bool to_uint64(uint64_t & value) const noexcept;


    /** Returns a floating point representation of an object
     * @param value is converted output 
     * @return true if conversion is perfomed, false otherwise
     */
    bool to_float(double & value) const noexcept;


    /** Returns a T representation of an object
     * @param value is converted output 
     * @return true if conversion is perfomed, false otherwise
     */
    template<class T>
    bool to_this_type(T & value) {
        if (std::is_floating_point<T>::value) {
            double dvalue = 0;
            if (!to_float(dvalue)) {
                return false;
            }

            value = static_cast<T>(dvalue);
        }
    }

    /** Returns a boolean representation of an object
     * @param value is converted output 
     * @return true if conversion is perfomed, false otherwise
     */
    bool to_bool(bool & value) const noexcept;


    /** Returns a string representation of an object
     * @param value is converted output 
     * @return true if conversion is perfomed, false otherwise
     */
    bool to_string(string_t & value) const noexcept;


    // String methods

    /** Returns a pointer to the string value of an object.
     * This method returns nullptr if the operation cannot be 
     * performed (e.g. an object does not represent a string)
     * @return the pointer to the string or nullptr
     */
    string_t * get_string() const noexcept {
        return reinterpret_cast<string_t*>(_value.ptr);
    }

    /** Returns a const reference to the string value of an object.
     * This method throws a mip::exception_t if the operation cannot be 
     * performed (e.g. an object does not represent a string)
     * @return a const reference to the string
     */
    operator string_t &() const {
        if (_type != type_t::STRING || !_value.ptr) {
            throw(mip::exception_t("Type mismatch: type is not string"));
        }
        return *(reinterpret_cast<string_t*>(_value.ptr));
    }


    // Array methods

    /** Returns a pointer to the array value of an object.
     * This method returns nullptr if the operation cannot be 
     * performed (e.g. an object does not represent an array)
     * @return the pointer to the array or nullptr
     */
    array_t * get_array() const noexcept {
        return reinterpret_cast<array_t*>(_value.ptr);
    }


    /** Returns a const reference to the array value of an object.
     * This method throws a mip::exception_t if the operation cannot be 
     * performed (e.g. an object does not represent an array)
     * @return a const reference to the array instance
     */
    operator const array_t &() const {
        if (_type != type_t::ARRAY || !_value.ptr) {
            throw(mip::exception_t("Type mismatch: type is not array"));
        }
        return *(reinterpret_cast<array_t*>(_value.ptr));
    }

    /** If the object represents an array, add an item to it
     * @param item: a variant_t item instance
     * @return true if operation is performed, false otherwise
     */
    template<class T>
    bool push_back(const T& item) {
        auto obj_ptr = get_array();
        if (!obj_ptr) {
            return false;
        }

        auto item_handle = make(item);

        if (!item_handle) {
            return false;
        }

        obj_ptr->push_back(std::move(item_handle));

        return true;
    }


    /** If the object represents an array, add an item to it
     * @param item: an unique_ptr to the variant_t instance
     * @return true if operation is performed, false otherwise
     */
    bool push_back(json_obj_t::handle_t&& item);


    //! Returns the number of any array items or object members
    size_t size() const noexcept;


    // Object methods

    //! Gets a pointer to any embedded object or nullptr
    data_t * get_object() const noexcept {
        return reinterpret_cast<data_t*>(_value.ptr);
    }

    //! Get a const reference to any embedded object
    operator const data_t &() const {
        if (_type != type_t::OBJECT || !_value.ptr) {
            throw(mip::exception_t("Type mismatch: type is not object"));
        }
        return *(reinterpret_cast<data_t*>(_value.ptr));
    }


    //! Writes to a destination stream the json representation of
    //! the managed object
    friend _ostream & operator<< (_ostream& os, const json_obj_t &jobj);


    // Building helper functions

   /**
    * Makes an object representing a T class value
    * @return a std::unique_ptr to the object instance
    */
    template<class T>
    static handle_t make(const T& value) {
        return std::make_unique<json_obj_t>(value);
    }

    /** Makes an object representing a T class value
     * @return a std::unique_ptr to the object instance
     */
    template<class T>
    static handle_t make(T&& value) {
        return std::make_unique<json_obj_t>(std::move(value));
    }

    /** Makes an object representing an json-object instance
     * @return a std::unique_ptr to the object instance
     */
    static handle_t make_object() {
        return std::make_unique<json_obj_t>(new json_obj_t::data_t);
    }

    /** Makes an object representing an jason-array instance
     * @return a std::unique_ptr to the object instance
     */
    static handle_t make_array(size_t size = 0) {
        json_obj_t::array_t an_array(size);
        return make(an_array);
    }

    /** If the object represents an object, add a <key, value> pair 
     * as member of it. 
     * @param key: member identifier string
     * @param item: a variant_t item instance
     * @return true if operation is performed, false otherwise
     */
    template<class T>
    bool add_member(const string_t& key, const T& item) {
        auto obj_ptr = get_object();
        if (!obj_ptr) {
            return false;
        }

        auto item_handle = make(item);

        if (!item_handle) {
            return false;
        }

        obj_ptr->push_back(std::make_pair(key, std::move(item_handle)));

        return true;
    }


    /** If the object represents an object, add a <key, value> pair 
     * as member of it. 
     * @param key: member identifier string
     * @param item: a const pointer to a null-terminated c-string
     * @return true if operation is performed, false otherwise
     */
    bool add_member(const string_t& key, const char_t* item) {
        return add_member<string_t>(key, item);
    }

    /** Add new <key, value> pair member to the object
     * @param key: member identifier string
     * @param item: handle (unique_ptr) to json object instance
     * @return true if operation is performed, false otherwise
     */
    bool add_member(const string_t& key, json_obj_t::handle_t&& item);

    /** Converts the object value to numeric type T.
     * @return true if the operation is performed, false otherwise
     */
    template<class T>
    bool convert_to(T & value) {
        return _convert_to(value, std::is_floating_point<T>());
    }

private:
    template<class T>
    bool _convert_to(T & value, std::true_type) {
        double dvalue = 0;

        if (!to_float(dvalue)) {
            return false;
        }

        value = static_cast<T>(dvalue);
    }

    template<class T>
    bool _convert_to(T & value, std::false_type) {
        uint64_t ivalue = 0;

        if (!to_uint64(ivalue)) {
            return false;
        }

        value = static_cast<T>(ivalue);
    }

    type_t _type;
    value_t _value;
};


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */

#endif // __MIP_JSON_OBJ_H__

