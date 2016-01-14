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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_LIBSDL2
#  include <SDL2/SDL.h>
#else
#  include <stdlib.h>
#endif


/* -------------------------------------------------------------------------- */

#include <thread>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

#if HAVE_LIBSDL2


/* -------------------------------------------------------------------------- */

struct sound_player_t
{
   static bool init_lib;

   Uint8 *audio_pos = nullptr; // pointer to the audio buffer to be played
   Uint32 audio_len = 0; // remaining length of the sample we have to play
   Uint32 wav_length; // length of our sample
   Uint8 *wav_buffer; // buffer containing our audio file
   SDL_AudioSpec wav_spec; // the specs of our piece of music

   // audio callback function
   // here you have to copy the data of your audio buffer into the
   // requesting audio buffer (stream)
   // you should only copy as much as the requested length (len)

   static void audio_callback(void *userdata, Uint8 *stream, int len)
   {
      sound_player_t* this_ptr = reinterpret_cast<sound_player_t*>(userdata);

      assert( this_ptr );

      if (this_ptr->audio_len == 0)
         return;

      len = ( len > int(this_ptr->audio_len) ? int(this_ptr->audio_len) : len );
      SDL_memcpy (stream, this_ptr->audio_pos, len);

      this_ptr->audio_pos += len;
      this_ptr->audio_len -= len;
   }


/* -------------------------------------------------------------------------- */

   sound_player_t()
   {
      if ( ! init_lib )
      {
         // Initialize SDL.
         if (SDL_Init(SDL_INIT_AUDIO) >= 0)
         {
            init_lib = true;
         }
      }
   }


/* -------------------------------------------------------------------------- */

   bool play(const char* path, bool sync_mode)
   {
      // Load the WAV
      if( SDL_LoadWAV(path, &wav_spec, &wav_buffer, &wav_length) == NULL )
      {
         return false;
      }

      // set the callback function
      wav_spec.callback = audio_callback;
      wav_spec.userdata = this;

      audio_pos = wav_buffer; // copy sound buffer
      audio_len = wav_length; // copy file length

      if ( SDL_OpenAudio(&wav_spec, NULL) < 0 )
      {
         return false;
      }

      SDL_PauseAudio(0);

      auto player_fnc = [](sound_player_t* this_ptr)
      {
         while ( this_ptr->audio_len > 0 )
         {
            SDL_Delay(1000);
         }

         // shut everything down
         SDL_CloseAudio();
         SDL_FreeWAV(this_ptr->wav_buffer);

         delete this_ptr;
      };

      if ( sync_mode )
      {
         player_fnc(this);
      }

      else
      {
         std::thread async( player_fnc, this );
         async.detach();
      }

      return true;
   }

};


/* -------------------------------------------------------------------------- */

bool sound_player_t::init_lib = false;


/* -------------------------------------------------------------------------- */

#else // This implementation relies on "aplay" external player


/* -------------------------------------------------------------------------- */

struct sound_player_t
{

/* -------------------------------------------------------------------------- */

   sound_player_t() = default;


/* -------------------------------------------------------------------------- */

   bool play(const char* path, bool sync_mode)
   {
      assert( path );

      std::string cmd = "aplay \"";
      cmd += path;
      cmd += "\" >/dev/null 2>/dev/null";

      if ( sync_mode )
      {
         return 0 == ::system( cmd.c_str() );
      }
      else
      {
         std::thread async( [&]()
         {
            int res = ::system( cmd.c_str() );
            (void) res;
         } );
         async.detach();
      }

      return true;
   }

};

#endif


/* -------------------------------------------------------------------------- */

bool play_sound(const char* path, bool sync_mode)
{
   auto p = new sound_player_t();

   assert( p );

   return p->play( path, sync_mode );
}


/* -------------------------------------------------------------------------- */

}

