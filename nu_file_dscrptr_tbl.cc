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


/* -------------------------------------------------------------------------- */

#include "nu_file_dscrptr_tbl.h"
#include "nu_os_std.h"

/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void file_dscrptr_tbl_t::trace(std::stringstream & ss)
{
   ss << "Open FILEs tbl:\n";

   for (const auto & f : _file_tbl)
   {
      ss << "FILE [" << std::hex << f.first << "] = "
         << reinterpret_cast<size_t>(f.second->data()) << "\n";
   }
}


/* -------------------------------------------------------------------------- */

bool file_dscrptr_tbl_t::open_fd(
   const std::string& fname,
   const std::string& fmode,
   unsigned int fd)
{
   auto i = _file_tbl.find(fd);

   if (i != _file_tbl.end())
      return false;

   FILE* fptr = fopen(fname.c_str(), fmode.c_str());

   if (!fptr)
      return false;

   _file_tbl.insert(
      std::make_pair(fd, std::make_shared<file_dscrptr_t>(fptr)));

   return true;
}


/* -------------------------------------------------------------------------- */

bool file_dscrptr_tbl_t::seek_fd(
   int seek_ptr,
   int seek_origin,
   unsigned int fd)
{
   FILE* fptr = resolve_fd(fd);

   if (!fptr)
      return false;

   return fseek(fptr, seek_ptr, seek_origin) == 0;
}


/* -------------------------------------------------------------------------- */

bool file_dscrptr_tbl_t::flush_fd(unsigned int fd)
{
   FILE* fptr = resolve_fd(fd);

   if (!fptr)
      return false;

   return fflush(fptr) == 0;
}


/* -------------------------------------------------------------------------- */

FILE* file_dscrptr_tbl_t::resolve_fd(unsigned int fd)
{
   auto i = _file_tbl.find(fd);

   if (i == _file_tbl.end())
      return nullptr;

   return i->second->data();

}


/* -------------------------------------------------------------------------- */

bool file_dscrptr_tbl_t::close_fd(unsigned int fd)
{
   auto i = _file_tbl.find(fd);

   if (i == _file_tbl.end())
      return false;

   assert( i->second );

   const bool ret = i->second->close();

   _file_tbl.erase(fd);

   return ret;
}


/* -------------------------------------------------------------------------- */

FILE* file_dscrptr_t::data() const throw()
{
   return _fptr;
}


/* -------------------------------------------------------------------------- */

file_dscrptr_t::~file_dscrptr_t()
{
   if (_fptr)
      fclose(_fptr);
}


/* -------------------------------------------------------------------------- */

bool file_dscrptr_t::close() throw()
{
   bool ret = _fptr == nullptr ? false : fclose(_fptr) == 0;

   if (ret)
      _fptr = nullptr;

   return ret;
}


/* -------------------------------------------------------------------------- */

}
