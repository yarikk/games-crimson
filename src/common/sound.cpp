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

////////////////////////////////////////////////////////////////////////
// sound.cpp
////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "SDL.h"

#include "sound.h"
#include "fileio.h"

// initialize static variables
bool Audio::sfx_state = false;
bool Audio::music_state = false;
int Audio::sfx_volume;
int Audio::music_volume;

#ifndef DISABLE_SOUND
bool Audio::init_base = false;
bool Audio::init_sfx = false;
bool Audio::init_music = false;
Mix_Music *Audio::music = 0;
string Audio::music_name;

SoundEffect *Audio::sfx[];
const char *Audio::sfx_files[] = {
      "error.wav", "clicked.wav", "menu.wav",
      "ask.wav", "select.wav", "repair.wav" };
#endif


////////////////////////////////////////////////////////////////////////
// NAME       : Audio::InitBase
// DESCRIPTION: Initialize the basic audio structures.
// PARAMETERS : -
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

#ifndef DISABLE_SOUND
int Audio::InitBase( void ) {
  int rc = 0;

  if ( !init_base ) {

    rc = SDL_InitSubSystem( SDL_INIT_AUDIO );
    if ( rc < 0 ) {
      cerr << "Couldn't initialize audio: " << SDL_GetError() << endl;
    } else {
      rc = Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, AUDIO_S16LSB, 
                          MIX_DEFAULT_CHANNELS, 1024 );
      if ( rc ) ShutdownBase();
      else init_base = true;
    }
  }
  return rc;
}
#endif

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::InitSfx
// DESCRIPTION: Load the sound effects and prepare the audio device if
//              necessary.
// PARAMETERS : state - initial state of sound effects (on/off)
//              vol   - volume (0 to MIX_MAX_VOLUME)
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Audio::InitSfx( bool state, unsigned char vol ) {
  int rc = 0;

#ifndef DISABLE_SOUND
  sfx_state = state;
  SetSfxVolume( vol );

  if ( sfx_state && !init_sfx ) {

    if ( !init_base ) rc = InitBase();

    if ( init_base ) {
      string sndd( get_sfx_dir() );
      for ( int i = 0; i < NUM_SFX; ++i ) {
        sfx[i] = new SoundEffect( (sndd + sfx_files[i]).c_str() );
      }

      init_sfx = true;
    }
  }
#endif

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::InitMusic
// DESCRIPTION: Load music and prepare the audio device if necessary.
// PARAMETERS : state - initial state of music (on/off)
//              vol   - volume (0 to MIX_MAX_VOLUME)
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Audio::InitMusic( bool state, unsigned char vol ) {
  int rc = 0;

#ifndef DISABLE_SOUND
  music_state = state;
  SetMusicVolume( vol );

  if ( music_state && !init_music ) {

    if ( !init_base ) rc = InitBase();

    init_music = init_base;
  }
#endif

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::ToggleSfx
// DESCRIPTION: Toggle SFX state (on/off).
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Audio::ToggleSfxState( void ) {
#ifndef DISABLE_SOUND
  sfx_state ^= 1;

  if ( sfx_state ) {
    if ( !init_sfx ) InitSfx( sfx_state, sfx_volume );
  } else if ( init_sfx ) ShutdownSfx();
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::ToggleMusicState
// DESCRIPTION: Toggle music state (on/off).
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Audio::ToggleMusicState( void ) {
#ifndef DISABLE_SOUND
  music_state ^= 1;

  if ( music_state ) {
    if ( !init_music ) InitMusic( music_state, music_volume );
    if ( init_music ) PlayMusic( 0 );
  } else ShutdownMusic();
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::PlaySfx
// DESCRIPTION: Play a sound effect by its identifier.
// PARAMETERS : sfxid - sound effect identifier
//              flags - see sound.h for details
// RETURNS    : pointer to the effect played or NULL if no effect played
////////////////////////////////////////////////////////////////////////

SoundEffect *Audio::PlaySfx( unsigned short sfxid, unsigned short flags ) {
  SoundEffect *se = NULL;

#ifndef DISABLE_SOUND
  if ( (sfxid < NUM_SFX) && sfx[sfxid] ) {
    se = sfx[sfxid];
    se->Play( flags );
  }
#endif

  return se;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::SetMusicVolume
// DESCRIPTION: Set the music volume.
// PARAMETERS : vol - new volume
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Audio::SetMusicVolume( int vol ) {
  music_volume = vol;

#ifndef DISABLE_SOUND
  Mix_VolumeMusic( vol );
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::PlayMusic
// DESCRIPTION: Play music by its identifier. This method is a noop if
//              another track is already being played at the moment.
// PARAMETERS : track - track name (without .ogg or .mid suffix). If
//                      NULL try to restart the previous track.
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Audio::PlayMusic( const char *track ) {
#ifndef DISABLE_SOUND
  if ( track ) music_name = track;

  if ( music_state ) {

    // if another track is being faded out, wait for it to end
    while ( Mix_FadingMusic() == MIX_FADING_OUT ) SDL_Delay( 100 );

    if ( !Mix_PlayingMusic() ) {
      // free previous track
      Mix_FreeMusic( music );

      string file( get_music_dir() + music_name + ".ogg" );  // try ogg version first
      music = Mix_LoadMUS( file.c_str() );

      if ( !music ) {
        file = get_music_dir() + music_name + ".mid";
        music = Mix_LoadMUS( file.c_str() );
      }

      if ( music ) Mix_PlayMusic( music, -1 );
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::StopMusic
// DESCRIPTION: Stop music, optionally fading it out.
// PARAMETERS : ms - fade time in milliseconds
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Audio::StopMusic( int ms ) {
#ifndef DISABLE_SOUND
  if ( music_state && Mix_PlayingMusic() ) {
    if ( ms <= 0 ) Mix_HaltMusic();
    else Mix_FadeOutMusic( ms );
  }
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::ShutdownSfx
// DESCRIPTION: Shutdown the SFX subsystem.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Audio::ShutdownSfx( void ) {
#ifndef DISABLE_SOUND
  if ( init_sfx ) {
    for ( int i = 0; i < NUM_SFX; ++i ) {
      delete sfx[i];
      sfx[i] = NULL;
    }
    init_sfx = false;
    sfx_state = false;
  }
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::ShutdownMusic
// DESCRIPTION: Shutdown the music subsystem.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Audio::ShutdownMusic( void ) {
#ifndef DISABLE_SOUND
  Mix_HaltMusic();
  Mix_FreeMusic( music );
  music = 0;
  init_music = false;
  music_state = false;
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Audio::ShutdownBase
// DESCRIPTION: Shutdown the base audio system.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

#ifndef DISABLE_SOUND
void Audio::ShutdownBase( void ) {
  if ( init_base ) {
    Mix_CloseAudio();
    SDL_QuitSubSystem( SDL_INIT_AUDIO );

    init_base = false;
  }
}
#endif


////////////////////////////////////////////////////////////////////////
// NAME       : SoundEffect::SoundEffect
// DESCRIPTION: Load a sound effect from a WAVE file.
// PARAMETERS : file - name of the WAVE file to load
//              dev  - audio spec of the (already open) audio device
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

SoundEffect::SoundEffect( const char *file ) {
#ifndef DISABLE_SOUND
  channel = -1;
  if ( !(sample = Mix_LoadWAV( file )) ) {
    cerr << "Error: Failed to load sfx " << file << " (" << SDL_GetError() << ')' << endl;
  }
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : SoundEffect::~SoundEffect
// DESCRIPTION: Free the audio data.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

SoundEffect::~SoundEffect( void ) {
#ifndef DISABLE_SOUND
  if ( sample ) Mix_FreeChunk( sample );
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : SoundEffect::Play
// DESCRIPTION: Play a sound effect.
// PARAMETERS : flags - see sound.h for details
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void SoundEffect::Play( unsigned short flags ) {
#ifndef DISABLE_SOUND
  if ( Audio::GetSfxState() && sample ) {
    sample->volume = Audio::GetSfxVolume();
    channel = Mix_PlayChannel( -1, sample, (flags & Audio::SFX_LOOP ? -1 : 0) );
  }
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : SoundEffect::Stop
// DESCRIPTION: Stop playing a sound effect.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void SoundEffect::Stop( void ) {
#ifndef DISABLE_SOUND
  if ( channel >= 0 ) {
    if ( Mix_Playing( channel ) ) Mix_HaltChannel( channel );
    channel = -1;
  }
#endif
}

