//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_UNICODE_H__
#define __MIP_UNICODE_H__


/* -------------------------------------------------------------------------- */

#include <string>

#define _T(x) __T(x)


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

#ifdef _UNICODE
using char_t = wchar_t;
#define __T(x) L ## x
#define _to_string(x) std::to_wstring(x)
#define _cin wcin
#define _cout wcout
#define _cerr wcerr
#define _clog wclog
#else
using char_t = char;
#define __T(x) x
#define _to_string(x) std::to_string(x)
#define _cin cin
#define _cout cout
#define _cerr cerr
#define _clog clog
#endif

using _string = std::basic_string<char_t, std::char_traits<char_t> >;
using string_t = _string;
using _ios = std::basic_ios<char_t, std::char_traits<char_t> >;
using _streambuf = std::basic_streambuf<char_t, std::char_traits<char_t> >;
using _istream = std::basic_istream<char_t, std::char_traits<char_t> >;
using _ostream = std::basic_ostream<char_t, std::char_traits<char_t> >  ;
using _iostream = std::basic_iostream<char_t, std::char_traits<char_t> > ;
using _stringbuf = std::basic_stringbuf<char_t, std::char_traits<char_t>, std::allocator<char_t> > ;
using _istringstream = std::basic_istringstream<char_t, std::char_traits<char_t>, std::allocator<char_t> > ;
using _ostringstream = std::basic_ostringstream<char_t, std::char_traits<char_t>, std::allocator<char_t> > ;
using _stringstream = std::basic_stringstream<char_t, std::char_traits<char_t>, std::allocator<char_t> > ;
using _filebuf = std::basic_filebuf<char_t, std::char_traits<char_t> > ;
using _ifstream = std::basic_ifstream<char_t, std::char_traits<char_t> > ;
using _ofstream = std::basic_ofstream<char_t, std::char_traits<char_t> > ;
using _wfstream = std::basic_fstream<char_t, std::char_traits<char_t> > ;


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __MIP_UNICODE_H__

