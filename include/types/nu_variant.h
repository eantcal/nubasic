//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#pragma once


/* -------------------------------------------------------------------------- */

#include "nu_error_codes.h"
#include "nu_exception.h"
#include "nu_stdtype.h"
#include "nu_variable.h"

#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {

//! Structure type
struct struct_variant_t {
    explicit struct_variant_t(
        const std::string& prototype_name, bool class_type = false) noexcept
        : _prototype_name(prototype_name)
        , _class_type(class_type)
    {
    }

    const std::string& get() const noexcept { return _prototype_name; }
    bool is_class_type() const noexcept { return _class_type; }

private:
    std::string _prototype_name;
    bool _class_type = false;
};


//! Structure type
struct any_variant_t {
    any_variant_t() = default;
};


//! Structure type
struct obj_variant_t {
    obj_variant_t(object_t::handle_t handle)
        : _handle(handle)
    {
    }

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

    // Materialise the inline scalar (if any) into _data[0] so that
    // subsequent vector-mode operations see a consistent backing store.
    // Cheap when there's no inline scalar (single bool check).
    void _demote_inline()
    {
        if (!_has_inline)
            return;
        _has_inline = false;
        if (_data.empty())
            _data.resize(1);
        if (_type == type_t::DOUBLE)
            _data[0] = _inline_double;
        else if (_type == type_t::STRING)
            _data[0] = std::move(_inline_string);
        else
            _data[0] = _inline_int; // INTEGER / BOOLEAN
        _inline_string.clear();
    }

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
    void _set(const T& value, std::vector<DT>& data, variable_t::type_t t,
        const size_t idx)
    {
        _type = t;

        if (idx >= _vect_size)
            _vect_size = idx + 1;

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

    explicit variant_t(const any_variant_t&)
        : _type(type_t::ANY)
    {
    }

    explicit variant_t(const obj_variant_t&)
        : _type(type_t::OBJECT)
    {
    }

    // Ordered field map.
    //
    // Pre-Phase 6 this was std::map<string, handle>: alphabetical by name,
    // which breaks the moment the FFI marshaller needs fields in their
    // declaration order (e.g. WIN32_FIND_DATA, where alphabetical != ABI
    // layout). Replaced with a small wrapper around vector<pair<>> that
    // preserves insertion order; lookup degrades from O(log N) to O(N)
    // but N is tiny in practice (typical struct = 2-20 fields).
    //
    // Public surface mirrors the std::map subset actually used by callers:
    //   - default construction, copy/move
    //   - find(key) / count(key) / empty() / size()
    //   - insert(make_pair(k, v)): appends if absent, replaces if present
    //   - range-for with structured binding [name, handle]
    //   - operator= for whole-map replacement (used by copy_struct_data)
    class struct_data_t {
    public:
        using value_type = std::pair<std::string, variant_t::handle_t>;
        using iterator = std::vector<value_type>::iterator;
        using const_iterator = std::vector<value_type>::const_iterator;

        struct_data_t() = default;

        iterator begin() noexcept { return _fields.begin(); }
        iterator end() noexcept { return _fields.end(); }
        const_iterator begin() const noexcept { return _fields.begin(); }
        const_iterator end() const noexcept { return _fields.end(); }

        bool empty() const noexcept { return _fields.empty(); }
        size_t size() const noexcept { return _fields.size(); }

        iterator find(const std::string& key) noexcept
        {
            for (auto it = _fields.begin(); it != _fields.end(); ++it)
                if (it->first == key)
                    return it;
            return _fields.end();
        }
        const_iterator find(const std::string& key) const noexcept
        {
            for (auto it = _fields.begin(); it != _fields.end(); ++it)
                if (it->first == key)
                    return it;
            return _fields.end();
        }
        size_t count(const std::string& key) const noexcept
        {
            return find(key) == _fields.end() ? 0 : 1;
        }

        // std::map-style insert: leaves an existing entry untouched and
        // appends a new one in declaration order. Returns
        // {iterator, inserted}. Most callers ignore the bool.
        std::pair<iterator, bool> insert(const value_type& v)
        {
            auto it = find(v.first);
            if (it != _fields.end())
                return { it, false };
            _fields.push_back(v);
            return { _fields.end() - 1, true };
        }

    private:
        std::vector<value_type> _fields;
    };

    explicit variant_t(const std::string& name, const struct_data_t& value,
        const size_t vect_size = 0)
        : _struct(std::make_shared<StructPayload>())
        , _type(type_t::STRUCT)
        , _vect_size(vect_size)
        , _vector_type(vect_size > 0)
    {
        _struct->type_name = name;
        _struct->struct_data.resize(1 > vect_size ? 1 : vect_size);
        _struct->struct_data[0] = value;
    }

    explicit variant_t(
        const struct_variant_t& value, const size_t vect_size = 0)
        : _struct(std::make_shared<StructPayload>())
        , _type(type_t::STRUCT)
        , _vect_size(vect_size)
        , _vector_type(vect_size > 0)
    {
        _struct->type_name = value.get();
        _struct->declared_class_type = value.is_class_type() ? value.get() : "";
        _struct->class_type = value.is_class_type();
        _struct->struct_data.resize(1 > vect_size ? 1 : vect_size);
    }

    static variant_t make_object_instance(
        const variant_t& prototype, const size_t vect_size = 0);
    static variant_t make_nothing(
        const std::string& type_name = "", const size_t vect_size = 0);

    static void copy_struct_data(struct_data_t& dst, const struct_data_t& src,
        bool instantiate_class_prototypes = false);

    variant_t(const string_t& value, const type_t t, const size_t vect_size = 0)
        : _type(t)
        , _vect_size(vect_size)
        , _vector_type(vect_size > 1)
    {
        // Scalar (vect_size == 0) types use inline storage to skip the
        // single-element _data allocation. BYTEVECTOR keeps using _data
        // because it is always a vector by definition.
        const bool inline_path = (vect_size == 0) && t != type_t::BYTEVECTOR;
        switch (t) {
        case type_t::STRING:
            if (inline_path) {
                _has_inline = true;
                _inline_string = value;
            } else {
                _data.resize(1 > vect_size ? 1 : vect_size, string_t(value));
            }
            break;
        case type_t::INTEGER:
        case type_t::BOOLEAN: {
            integer_t ivalue{ 0 };
            try {
                ivalue = nu::stoll(value);
            } catch (...) {
            }
            if (inline_path) {
                _has_inline = true;
                _inline_int = ivalue;
            } else {
                _data.resize(1 > vect_size ? 1 : vect_size, integer_t(ivalue));
            }
        } break;
        case type_t::BYTEVECTOR: {
            integer_t ivalue{ 0 };
            try {
                ivalue = nu::stoll(value);
            } catch (...) {
            }
            _data.resize(1 > vect_size ? 1 : vect_size, integer_t(ivalue));
        } break;
        case type_t::DOUBLE: {
            double_t fvalue{ 0 };
            try {
                fvalue = stold(value);
            } catch (...) {
            }
            if (inline_path) {
                _has_inline = true;
                _inline_double = fvalue;
            } else {
                _data.resize(1 > vect_size ? 1 : vect_size, double_t(fvalue));
            }
        } break;
        default:
            break;
        }
    }

    variant_t(const string_t& value, const size_t vect_size = 0)
        : _type(type_t::STRING)
        , _vect_size(vect_size)
        , _vector_type(vect_size > 1)
    {
        if (vect_size == 0) {
            _has_inline = true;
            _inline_string = value;
        } else {
            _data.resize(vect_size, string_t(value));
        }
    }

    variant_t(const char* value, const size_t vect_size = 0)
        : _type(type_t::STRING)
        , _vect_size(vect_size)
        , _vector_type(vect_size > 1)
    {
        if (vect_size == 0) {
            _has_inline = true;
            _inline_string = value;
        } else {
            _data.resize(vect_size, string_t(value));
        }
    }

    variant_t(const double_t value, const size_t vect_size = 0)
        : _type(type_t::DOUBLE)
        , _vect_size(vect_size)
        , _vector_type(vect_size > 1)
    {
        if (vect_size == 0) {
            _has_inline = true;
            _inline_double = value;
        } else {
            _data.resize(vect_size, double_t(value));
        }
    }

    variant_t(const std::vector<double_t>& value)
        : _type(type_t::DOUBLE)
        , _vect_size(value.size())
        , _vector_type(true)
    {
        if (_vect_size < 1) {
            _data.resize(1, double_t());
        } else {
            _data.reserve(value.size());

            for (const auto e : value) {
                _data.emplace_back(double_t(e));
            }
        }
    }

    variant_t(const std::vector<byte_t>& value)
        : _type(type_t::BYTEVECTOR)
        , _vect_size(value.size())
        , _vector_type(true)
    {
        if (_vect_size < 1) {
            _data.resize(1, integer_t());
        } else {
            _data.reserve(value.size());

            for (const auto e : value) {
                _data.emplace_back(integer_t(e));
            }
        }
    }

    variant_t(const integer_t value, const size_t vect_size = 0)
        : _type(type_t::INTEGER)
        , _vect_size(vect_size)
        , _vector_type(vect_size > 1)
    {
        if (vect_size == 0) {
            _has_inline = true;
            _inline_int = value;
        } else {
            _data.resize(vect_size, integer_t(value));
        }
    }

    variant_t(const int value, const size_t vect_size = 0)
        : _type(type_t::INTEGER)
        , _vect_size(vect_size)
        , _vector_type(vect_size > 1)
    {
        if (vect_size == 0) {
            _has_inline = true;
            _inline_int = value;
        } else {
            _data.resize(vect_size, integer_t(value));
        }
    }

    variant_t(const bool_t value, const size_t vect_size = 0)
        : _type(type_t::BOOLEAN)
        , _vect_size(vect_size)
        , _vector_type(vect_size > 1)
    {
        if (vect_size == 0) {
            _has_inline = true;
            _inline_int = value;
        } else {
            _data.resize(vect_size, integer_t(value));
        }
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
                || v._type != type_t::STRUCT || !_struct || !v._struct
                || _struct->type_name != v._struct->type_name
                || vector_idx >= _struct->struct_data.size()
                || v._struct->struct_data.size() != 1,
            0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

        // Phase 4: detach if shared before mutating.
        _struct_mut().struct_data[vector_idx] = v._struct->struct_data[0];
    }

    // Assign one slot of a class-instance array to a class instance.
    // Caller must validate base/derived compatibility via
    // prog_ctx_t::is_class_assignable; this method does not check type names
    // because polymorphism allows the source to be a derived class.
    void set_class_slot(const variant_t& v, const size_t vector_idx)
    {
        rt_error_code_t::get_instance().throw_if(!is_class_type()
                || !v.is_class_type()
                || vector_idx >= _struct->struct_data.size()
                || v._struct->struct_data.size() != 1,
            0, rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

        // For class arrays, mutation is still in-place: _struct_mut() will
        // skip the detach because class_type is true (aliasing is the
        // intended semantics for object references).
        auto& s = _struct_mut();
        s.struct_data[vector_idx] = v._struct->struct_data[0];
        if (s.object_ids.size() <= vector_idx)
            s.object_ids.resize(vector_idx + 1);
        s.object_ids[vector_idx] = v._struct->object_ids.empty()
            ? std::shared_ptr<size_t>()
            : v._struct->object_ids[0];
    }

    const std::string& struct_type_name() const noexcept
    {
        return _struct_ro().type_name;
    }

    const std::string& declared_class_type() const noexcept
    {
        return _struct_ro().declared_class_type;
    }

    void set_declared_class_type(const std::string& type_name)
    {
        _ensure_struct().declared_class_type = type_name;
    }

    bool is_class_type() const noexcept
    {
        return _type == type_t::STRUCT && _struct && _struct->class_type;
    }

    bool is_object_reference() const noexcept
    {
        return is_class_type() && !_struct->object_ids.empty();
    }

    bool is_nothing(const size_t idx = 0) const noexcept
    {
        return is_object_reference()
            && (idx >= _struct->object_ids.size() || !_struct->object_ids[idx]);
    }

    bool same_object_reference(const variant_t& other) const noexcept;

    // Read-only view of slot-0 field map; only meaningful when is_struct().
    const struct_data_t& struct_fields() const noexcept
    {
        static const struct_data_t _empty{};
        return (_struct && !_struct->struct_data.empty())
            ? _struct->struct_data[0]
            : _empty;
    }

    void describe_type(std::stringstream& ss) const noexcept;

    void resize(size_t size)
    {
        _vector_type = size > 0;
        _vect_size = size;
        _resize(size);
    }

    bool is_vector() const noexcept { return _vector_type; }

    bool is_struct() const noexcept { return _type == type_t::STRUCT; }

    size_t vector_size() const noexcept { return _vect_size; }

    bool copy_vector_content(std::vector<double>& dst) const
    {

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

    bool is_number() const noexcept
    {
        return variable_t::is_number(get_type());
    }
    bool is_integral() const noexcept
    {
        return variable_t::is_integral(get_type());
    }
    bool is_float() const noexcept { return variable_t::is_float(get_type()); }
    bool is_string() const noexcept
    {
        return variable_t::is_string(get_type());
    }

    static const char* get_type_desc(const type_t& type) noexcept;

    explicit operator float() const { return static_cast<float>(to_double()); }
    explicit operator double_t() const { return to_double(); }
    explicit operator integer_t() const { return to_int(); }
    explicit operator int() const { return static_cast<int>(to_int()); }
    explicit operator unsigned int() const
    {
        return static_cast<unsigned int>(to_int());
    }

    explicit operator unsigned long() const
    {
        return static_cast<unsigned long>(to_int());
    }
    explicit operator string_t() const { return to_str(); }
    explicit operator bool() const { return to_int() != 0; }

    const string_t to_str(const size_t idx = 0) const;

    void set_str(const string_t& value)
    {
        _type = type_t::STRING;
        _has_inline = true;
        _inline_string = value;
        _data.clear();
    }
    void set_str(const char* value)
    {
        _type = type_t::STRING;
        _has_inline = true;
        _inline_string = value;
        _data.clear();
    }
    void set_int(const integer_t& value)
    {
        _type = type_t::INTEGER;
        _has_inline = true;
        _inline_int = value;
        _data.clear();
    }
    void set_double(const double_t value)
    {
        _type = type_t::DOUBLE;
        _has_inline = true;
        _inline_double = value;
        _data.clear();
    }
    void set_bvect(const integer_t value)
    {
        _has_inline = false;
        _set<integer_t>(value, _data, type_t::BYTEVECTOR);
    }
    void set_bool(const bool_t value)
    {
        _type = type_t::BOOLEAN;
        _has_inline = true;
        _inline_int = value;
        _data.clear();
    }
    // Indexed setters always operate on the vector backing store. If the
    // variant was previously holding an inline scalar (Phase 1a), we must
    // demote it to the vector path before writing, otherwise _data would
    // be initialised with a default value instead of the inline one.
    void set_str(const string_t& value, const size_t idx)
    {
        _demote_inline();
        _set<string_t>(value, _data, type_t::STRING, idx);
    }
    void set_str(const char* value, const size_t idx)
    {
        _demote_inline();
        _set<string_t>(value, _data, type_t::STRING, idx);
    }
    void set_int(const integer_t& value, const size_t idx)
    {
        _demote_inline();
        _set<integer_t>(value, _data, type_t::INTEGER, idx);
    }
    void set_double(const double_t value, const size_t idx)
    {
        _demote_inline();
        _set<double_t>(value, _data, type_t::DOUBLE, idx);
    }
    void set_bvect(const integer_t value, const size_t idx)
    {
        _demote_inline();
        _set<integer_t>(value, _data, type_t::BYTEVECTOR, idx);
    }
    void set_bool(const bool_t value, const size_t idx)
    {
        _demote_inline();
        _set<integer_t>(value, _data, type_t::BOOLEAN, idx);
    }

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
    variant_t& operator*=(const variant_t& b);
    variant_t& operator/=(const variant_t& b);

    friend std::ostream& operator<<(std::ostream& os, const variant_t& val);

protected:
    // Phase 1c: struct/object metadata is boxed so that scalar variants
    // (the 99% case at runtime) do not pay its ~80 B of inline footprint.
    // _struct is nullptr for non-STRUCT variants.
    //
    // Phase 4: the payload is now ref-counted (shared_ptr) with
    // copy-on-write. A plain pass-by-value of a STRUCT variant is now an
    // O(1) refcount bump; the deep clone happens lazily when a mutating
    // operation finds use_count() > 1. Object-reference (class) variants
    // intentionally keep aliasing as the observable semantics; Phase 4
    // never detaches them.
    struct StructPayload {
        std::vector<struct_data_t> struct_data;
        std::string type_name;
        std::string declared_class_type;
        std::vector<std::shared_ptr<size_t>> object_ids;
        bool class_type = false;
    };
    std::shared_ptr<StructPayload> _struct;

    // Lazy COW detach: ensures the payload is uniquely owned before
    // returning a writable reference. For class/object-reference
    // variants, aliasing IS the intended semantics, so detach is
    // skipped; callers are responsible for not invoking _struct_mut()
    // on those (or accepting that any change is observed by all
        // references, which is what nuBASIC object semantics require).
    StructPayload& _struct_mut()
    {
        if (!_struct) {
            _struct = std::make_shared<StructPayload>();
            return *_struct;
        }
        if (_struct.use_count() > 1 && !_struct->class_type) {
            _struct
                = std::make_shared<StructPayload>(_struct_deep_clone(*_struct));
        }
        return *_struct;
    }

    // Original ensure (no detach). Kept for callsites where allocating
    // an empty payload is the goal and no concurrent observer exists
    // (e.g. inside copy/move ctor bodies that have just default-set
    // members and own the only refcount).
    StructPayload& _ensure_struct() { return _struct_mut(); }

    // Deep clone helper used by _struct_mut. Defined inline here so all
    // headers that need it see the body. Mirrors the previous behaviour
    // of the copy ctor; object_ids stays shared (identity preserved).
    static StructPayload _struct_deep_clone(const StructPayload& src)
    {
        StructPayload dst;
        dst.type_name = src.type_name;
        dst.declared_class_type = src.declared_class_type;
        dst.class_type = src.class_type;
        dst.object_ids = src.object_ids; // identity tokens stay shared
        dst.struct_data.resize(src.struct_data.size());
        for (size_t i = 0; i < src.struct_data.size(); ++i) {
            copy_struct_data(dst.struct_data[i], src.struct_data[i]);
        }
        return dst;
    }

    // Read-only accessor with a static empty fallback for non-STRUCT
    // variants; lets accessors like struct_type_name() stay noexcept.
    const StructPayload& _struct_ro() const noexcept
    {
        static const StructPayload kEmpty{};
        return _struct ? *_struct : kEmpty;
    }

    type_t _type = type_t::UNDEFINED;
    size_t _vect_size = 0;
    bool _vector_type = false;

    mutable std::vector<std::variant<string_t, integer_t, double_t>> _data;

    // Inline scalar storage (Phase 1a + 1b).
    // For non-vector INTEGER / DOUBLE / BOOLEAN / STRING scalars we keep
    // the value inline instead of materialising a single-element
    // std::vector. This eliminates the per-scalar heap allocation visible
    // by the upstream micro-benchmark (~40 B/op on x64 MSVC).
    // When _has_inline is true, _data is intentionally left empty and the
    // canonical value lives in _inline_int / _inline_double /
    // _inline_string according to _type. Setters that mutate a scalar
    // update the inline slot and clear _data; vector-mode setters
    // (with idx, or with vect_size > 0) take the legacy path and clear
    // _has_inline.
    //
    // Tradeoff: _inline_string adds ~32 B to sizeof(variant_t) on MSVC.
    // STRING is uncommon enough at runtime that paying this for every
    // variant is non-trivial but cheaper than the alternative: boxing it
    // (unique_ptr<string>) costs at least one allocation per STRING
    // scalar, defeating the point.
    bool _has_inline = false;
    integer_t _inline_int = 0;
    double_t _inline_double = 0.0;
    string_t _inline_string;

    template <class T> T _at(size_t idx) const
    {
        // Fast path: scalar reads from inline storage.
        if (_has_inline && idx == 0) {
            if constexpr (std::is_same_v<T, integer_t>) {
                rt_error_code_t::get_instance().throw_if(
                    _type != type_t::INTEGER && _type != type_t::BOOLEAN, 0,
                    rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");
                return _inline_int;
            } else if constexpr (std::is_same_v<T, double_t>) {
                rt_error_code_t::get_instance().throw_if(
                    _type != type_t::DOUBLE, 0,
                    rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");
                return _inline_double;
            } else if constexpr (std::is_same_v<T, string_t>) {
                rt_error_code_t::get_instance().throw_if(
                    _type != type_t::STRING, 0,
                    rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");
                return _inline_string;
            }
            // Any other T: fall through. Inline scalars never hold any
            // other alternative, so the bounds check below will report
            // E_VAL_OUT_OF_RANGE consistently with the legacy path.
        }

        rt_error_code_t::get_instance().throw_if(idx >= _data.size(), 0,
            rt_error_code_t::value_t::E_VAL_OUT_OF_RANGE, "");

        const auto& data = _data[idx];

        rt_error_code_t::get_instance().throw_if(
            !std::holds_alternative<T>(data), 0,
            rt_error_code_t::value_t::E_TYPE_ILLEGAL, "");

        return std::get<T>(data);
    }
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
