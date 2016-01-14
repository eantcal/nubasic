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

#ifndef __NU_SIGNAL_HANDLING_H__
#define __NU_SIGNAL_HANDLING_H__


/* -------------------------------------------------------------------------- */

#include <memory>
#include <map>
#include <cassert>
#include <set>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

struct signal_handler_t
{
   using handle_t = signal_handler_t*;

   // Right now we've got just an event
   enum class event_t
   {
      BREAK
   };

   virtual bool notify(const event_t& ev) = 0;
   virtual ~signal_handler_t() {}
};


/* -------------------------------------------------------------------------- */

class signal_mgr_t
{
private:
   //This is a singleton
   static signal_mgr_t * _instance;

   signal_mgr_t() = default;

   //Copy not allowed
   signal_mgr_t(const signal_mgr_t&) = delete;
   signal_mgr_t& operator = ( const signal_mgr_t& ) = delete;

public:
   static signal_mgr_t& instance();


   void register_handler(
      const signal_handler_t::event_t& ev,
      signal_handler_t::handle_t handle)
   {
      _handlers[ev].insert(handle);
   }


   void unregister_handler(
      const signal_handler_t::event_t& ev,
      signal_handler_t::handle_t handle)
   {
      auto i = _handlers.find(ev);

      if ( i != _handlers.end() )
         i->second.erase(handle);
   }

   
   void disable_notifications() throw()
   {
      _disable_notifications = true;
   }


   void enable_notifications() throw()
   {
      _disable_notifications = false;
   }


protected:
   using hlist_t = std::set < signal_handler_t::handle_t > ;

   std::map< signal_handler_t::event_t, hlist_t > _handlers;

   bool dispatch(signal_handler_t::event_t ev);

   friend bool _ev_dispatcher(signal_handler_t::event_t ev);

   bool _disable_notifications = false;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_SIGNAL_HANDLING_H__
