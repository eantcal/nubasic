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

#if !defined(__MINGW32__) && !defined(_WIN32)
#include "nu_terminal.h"

/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

//! Preserves terminal ios attributes 
struct termios_reset_t
{
private:
   struct termios _oldt;

public:
   inline termios_reset_t() throw()
   {
      tcgetattr( STDIN_FILENO, &_oldt );
   }

   inline ~termios_reset_t() throw()
   {
      tcsetattr( STDIN_FILENO, TCSANOW, &_oldt );
   }
};


/* -------------------------------------------------------------------------- */

//! Set terminal in raw mode
struct termios_makeraw_t : public termios_reset_t
{
public:
   inline termios_makeraw_t() throw()
   {
      struct termios new_termios;

      tcgetattr(0, &new_termios);

      cfmakeraw(&new_termios);
      tcsetattr(0, TCSANOW, &new_termios);
   }
};


/* -------------------------------------------------------------------------- */

//! Disable terminal echo
struct termios_disable_echo_t : public termios_reset_t
{
public:
   inline termios_disable_echo_t() throw()
   {
      struct termios newt;
      tcgetattr( STDIN_FILENO, &newt );
      newt.c_lflag &= ~( ICANON | ECHO );
      tcsetattr( STDIN_FILENO, TCSANOW, &newt );
   }
};


/* -------------------------------------------------------------------------- */

// class terminal_t

terminal_t :: terminal_t( terminal_input_t & term_input /* = 0*/ )
   : _term_input (term_input), _flag(0), _max_line_length(0),
     _insert_enabled(false)
{
}


/* -------------------------------------------------------------------------- */

terminal_t :: ~terminal_t()
{ }


/* -------------------------------------------------------------------------- */

int terminal_t :: getch() const throw()
{
   return _term_input.getch();
}


/* -------------------------------------------------------------------------- */

int terminal_t :: keybhit() const throw()
{
   return _term_input.keybhit();
}


/* -------------------------------------------------------------------------- */

char terminal_t :: getrawch() const throw()
{
   return _term_input.getrawch();
}


/* -------------------------------------------------------------------------- */

bool terminal_t :: end_of_line( int ch ) const throw()
{
   return _eol_code.find( ch ) != _eol_code.end();
}


/* -------------------------------------------------------------------------- */

bool terminal_t :: is_printable( int ch ) const throw()
{
   return ch >= terminal_input_t::SPACE && ch < terminal_input_t::BACKSPACE;
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit_putc( int ch ) throw()
{
   bool no_echo = (_flag & ECHO_DIS) == ECHO_DIS;
   bool no_accept_more_ch = false;
   bool insert_occurred = false;

   if ( _cursor != int(_line.length()) )
   {
      if ( _insert_enabled )
      {
         _line.insert ( _cursor, 1, char(ch & 0xff) );
         insert_occurred = true;
      }

      else
      {
         _line[ _cursor ] = char(ch & 0xff);
      }
   }

   else
   {
      if ( !_max_line_length || int(_line.size()) < _max_line_length )
      {
         _line += char(ch & 0xff);
      }

      else
      {
         no_echo = true;
         no_accept_more_ch = true;
      }
   }

   if ( no_echo == false )
   {
      if ( insert_occurred )
      {
         std::string tail =
            _line.substr(_cursor, _line.length()-_cursor);

         //Remark: printf -> nu::printf
         printf("%s ", tail.c_str());
         int tail_len = int(tail.length());

         while( tail_len-- ) printf("\b");

         fflush(stdout);
      }

      else
      {
         printf("%c", char(ch & 0xff));
      }

      fflush(stdout);
   }

   if ( no_accept_more_ch == false )
   {
      ++ _cursor;
   }
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit_backspace() throw()
{
   if ( _cursor > 0 )
   {
      -- _cursor;
      _line.erase( _cursor, 1 );

      if ( _cursor < int(_line.length())  )
      {
         std::string tail =
            _line.substr(_cursor, _line.length()-_cursor);

         if ( (_flag & ECHO_DIS) == 0 )
         {
            printf("\b%s ", tail.c_str());
            int tail_len = tail.length() + 1;

            while( tail_len-- ) printf("\b");

            fflush(stdout);
         }
      }

      else
      {
         if ( (_flag & ECHO_DIS) == 0 )
         {
            printf("\b \b");
            fflush(stdout);
         }
      }

   }
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit_ctrl_h() throw()
{
   edit_backspace( );
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit_delete() throw()
{
   if ( _cursor >= 0 && _cursor < int(_line.length()) )
   {
      _line.erase( _cursor, 1 );

      if ( _cursor <= int(_line.length())  )
      {
         std::string tail =
            _line.substr(_cursor, _line.length()-_cursor);

         if ( (_flag & ECHO_DIS) == 0 )
         {
            printf("%s ", tail.c_str());
            int tail_len = tail.length() + 1;

            while( tail_len-- ) printf("\b");

            fflush(stdout);
         }
      }

      else
      {
         if ( (_flag & ECHO_DIS) == 0 )
         {
            printf("\b \b");
            fflush(stdout);
         }
      }

   }
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit_right() throw()
{
   if ( _cursor < int(_line.length()) )
   {

      if ( (_flag & ECHO_DIS) == 0 )
      {
         printf("%c", _line[_cursor]);
         fflush(stdout);
      }

      ++ _cursor;
   }
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit_left() throw()
{
   if ( _cursor > 0 )
   {
      -- _cursor;

      if ( (_flag & ECHO_DIS) == 0 )
      {
         printf("\b");
         fflush(stdout);
      }
   }
}


/* -------------------------------------------------------------------------- */

void terminal_t :: delete_line() throw()
{
   if ( (_flag & ECHO_DIS) == 0 )
   {
      int ch2remove = _cursor;

      while ( ch2remove-- > 0 )
         printf("\b \b");
   }

   _line.clear();
   _cursor = 0;
}


/* -------------------------------------------------------------------------- */

void terminal_t :: refresh_line() throw()
{
   if ( (_flag & ECHO_DIS) == 0 )
   {
      printf("%s", _line.c_str());
   }

   _cursor = _line.size();
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit_up() throw()
{
   if ( _history.is_empty() )
      return;

   delete_line();

   _history.move_cursor_prev();
   _line = _history.get_cur_item();

   refresh_line();
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit_down() throw()
{
   if ( _history.is_empty() )
      return;

   delete_line();

   _history.move_cursor_next();
   _line = _history.get_cur_item();

   refresh_line();
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit_insert () throw()
{
   _insert_enabled = !_insert_enabled;
}


/* -------------------------------------------------------------------------- */

void terminal_t :: edit( int ch ) throw()
{
   if ( is_printable( ch ) )
   {
      edit_putc( ch );
   }

   else
   {
      switch (ch)
      {
         case terminal_input_t::CTRL_H:
            edit_ctrl_h();
            break;

         case terminal_input_t::BACKSPACE:
            edit_backspace();
            break;

         case terminal_input_t::DELETE:
            edit_delete();
            break;

         case terminal_input_t::INSERT:
            edit_insert();
            break;

         case terminal_input_t::RIGHT:
            edit_right();
            break;

         case terminal_input_t::LEFT:
            edit_left();
            break;

         case terminal_input_t::UP:
            edit_up();
            break;

         case terminal_input_t::DOWN:
            edit_down();
            break;
      }
   }
}


/* -------------------------------------------------------------------------- */

int terminal_t :: get_line( std::string& line, bool return_on_len_max ) throw()
{
   if ((_flag & APPEND) == 0)
   {
      line.clear();
   }

   _line = line;
   _cursor = line.length();

   refresh_line();

   do
   {
      bool quit_function =
         (return_on_len_max && int(_line.length())>=_max_line_length);

      int ch = 0;

      if ( ! quit_function )
      {
         ch = getch();
      }

      if ( end_of_line( ch ) || quit_function )
      {
         line = _line;

         if ( ! _line.empty() )
         {
            _history.add_item( _line );
         }

         return ch;
      }

      else
      {
         edit( ch );
      }
   }
   while( true );

   return EOF;
}


/* -------------------------------------------------------------------------- */

// class terminal_input_t

terminal_input_t :: ~terminal_input_t() throw()
{
}


/* -------------------------------------------------------------------------- */

int terminal_input_t :: getch() const throw()
{
   int ch = 0;

   termios_disable_echo_t echo_off;

   do
   {
      ch = getchar();

      if (ch != EOF )
      {
         if (ch == ESCAPE )
         {
            ch = getchar();

            if ( ch == ESCAPE2 )
            {
               ch = getchar();

               int retval1 = ((ESCAPE<<16)|(ESCAPE2<<8)|(ch & 0xff));
               int retval2 = (retval1 << 8) | 0x7e;

               switch ( retval2 )
               {
                  case HOME:
                  case END :
                  case PGUP:
                  case PGDOWN:
                  case DELETE:
                  case INSERT:
                     ch = getchar();

                     if ( ch != EOF )
                     {
                        return retval2;
                     }

                     else
                     {
                        ungetc( ch, stdin );
                        return retval1;
                     }

                     break;
               }

               return retval1;
            }

            else
            {
               return ((ESCAPE<<8)|(ch & 0xff));
            }
         }

         return ch;
      }
   }
   while ( true );

   return EOF;
}


/* -------------------------------------------------------------------------- */

int terminal_input_t :: keybhit() const throw()
{
   termios_makeraw_t makeraw_now;

   struct timeval tv = { 0L, 0L };

   fd_set fds;
   FD_ZERO(&fds);
   FD_SET(0, &fds);

   int ret = ::select(1, &fds, NULL, NULL, &tv);

   return ret > 0 ? 1 : ret;
}


/* -------------------------------------------------------------------------- */

char terminal_input_t :: getrawch() const throw()
{
   if ( keybhit() )
   {
      termios_makeraw_t makeraw_now;

      char c = 0;

      if (::read(0, &c, sizeof(c)) < 0)
         return (char) -1; // EOF

      return c;
   }

   return 0;
}


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif

