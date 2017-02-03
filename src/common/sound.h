// Crimson Fields -- a game of tactical warfare
// Copyright (C) 2000-2007 Jens Granseuer
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

///////////////////////////////////////////////////////////////
// sound.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_SOUND_H
#define _INCLUDE_SOUND_H

#ifndef DISABLE_SOUND
# include <string>
using namespace std;

# include "SDL_mixer.h"

# define NUM_SFX 6  // system effects; they have nothing to do with
                    // the sounds defined by a mission set
#endif

#ifndef MIX_MAX_VOLUME
# define MIX_MAX_VOLUME 0
#endif

class SoundEffect {
public:
  SoundEffect( const char *file );
  ~SoundEffect( void );

  void Play( unsigned short flags );
  void Stop( void );

private:
#ifndef DISABLE_SOUND
  Mix_Chunk *sample;
  int channel;
#endif
};


class Audio {
public:
  // sound effect identifiers
  enum {
    SND_GUI_ERROR = 0,
    SND_GUI_PRESSED,
    SND_GUI_MENU_SHOW,
    SND_GUI_ASK,
    SND_GAM_SELECT,
    SND_GAM_REPAIR
  };

  // flags for PlaySfx()
  enum {
    SFX_LOOP = 0x0001
  };

  static int InitSfx( bool state, unsigned char vol );
  static int InitMusic( bool state, unsigned char vol );
  static void ShutdownSfx();
  static void ShutdownMusic();

  static SoundEffect *PlaySfx( unsigned short sfxid, unsigned short flags );
  static bool GetSfxState( void ) { return sfx_state; }
  static void ToggleSfxState( void );
  static int GetSfxVolume( void ) { return sfx_volume; }
  static void SetSfxVolume( int vol ) { sfx_volume = vol; }

  static void PlayMusic( const char *track );
  static void StopMusic( int ms );
  static bool GetMusicState( void ) { return music_state; }
  static void ToggleMusicState( void );
  static int GetMusicVolume( void ) { return music_volume; }
  static void SetMusicVolume( int vol );

private:
  static bool sfx_state;
  static int sfx_volume;
  static bool music_state;
  static int music_volume;

#ifndef DISABLE_SOUND
  static int InitBase( void );
  static void ShutdownBase( void );

  static bool init_base;
  static bool init_sfx;
  static bool init_music;
  static SoundEffect *sfx[NUM_SFX];
  static const char *sfx_files[NUM_SFX];
  static Mix_Music *music;
  static string music_name;
#endif
};

#endif	/* _INCLUDE_SOUND_H */

