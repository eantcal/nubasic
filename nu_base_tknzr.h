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

#ifndef __NU_BASE_TKNZR_H__
#define __NU_BASE_TKNZR_H__


/* -------------------------------------------------------------------------- */

#include "nu_token.h"
#include "nu_token_list.h"

#include <string>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu
{

/* -------------------------------------------------------------------------- */

/**
 * This is abstract tokenizer which provides a container view interface
 * of a series of tokens contained in a data text buffer
 */
class base_tknzr_t
{
public:
   /** ctor
    * \param data: input buffer
    * \param pos: original position of input data
    */
   base_tknzr_t(const std::string& data) :
      _data(std::make_shared< std::string >(data)),
      _data_len(data.size())
   {
      assert(_data);
   }


   base_tknzr_t() = delete;
   base_tknzr_t(const base_tknzr_t&) = delete;
   base_tknzr_t& operator=( const base_tknzr_t& ) = delete;

   //! dtor
   virtual ~base_tknzr_t() {}

   //! Get a token and advance to the next one (if any)
   virtual token_t next() = 0;


   //! Get a token list
   virtual void get_tknlst(token_list_t& tl, bool strip_comment) = 0;


   //! Get buffer pointer (cptr)
   size_t tell() const
   {
      return _cptr;
   }


   //! Get size of the input buffer
   size_t size() const
   {
      return _data_len;
   }


   //! Return true if current position >= size()
   bool eol() const
   {
      return tell() >= size();
   }


   //! Reset buffer pointer to zero
   void reset() NU_NOEXCEPT
   {
      _rst_cptr();
   }


   //! Create a token object
   static token_t make_token(
      size_t pos,
      token_t::data_ptr_t data_ptr)
   {
      return token_t(pos, data_ptr);
   }


protected:
   //! Get current pointed character within the buffer
   char get_symbol() const NU_NOEXCEPT
   {
      return !eol() ? ( *_data )[tell()] : 0;
   }


   //! If not eof, move buffer pointer to next symbol
   void seek_next() NU_NOEXCEPT
   {
      if ( tell() < size() ) _inc_cptr();
   }


   //! Assign a new value to cptr
   void set_cptr(size_t cptr)
   {
      _cptr = cptr;
   }


   //! If not begin of buffer, move cptr to the previous symbol
   void seek_prev() NU_NOEXCEPT
   {
      if ( tell() ) _dec_cptr();
   }


   //! Return a shared_ptr to internal data
   token_t::data_ptr_t data() const NU_NOEXCEPT
   {
      return _data;
   }


private:
   void _inc_cptr() NU_NOEXCEPT
   {
      ++_cptr;
   }

   void _dec_cptr() NU_NOEXCEPT
   {
      --_cptr;
   }

   void _rst_cptr() NU_NOEXCEPT
   {
      _cptr = 0;
   }

   std::shared_ptr< std::string > _data;
   size_t _data_len = 0;
   size_t _cptr = 0;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_BASE_TKNZR_H__
