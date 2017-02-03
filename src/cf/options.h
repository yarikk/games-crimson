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
// options.h
// This is used as a persistent options container that holds
// relevant settings for longer than the actual Game object
// exists and can be used to properly reinitialize when a
// new game starts.
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_OPTIONS_H
#define _INCLUDE_OPTIONS_H

#include <algorithm>
#include <string>
#include <vector>
using namespace std;

#include "SDL.h"

enum KeyBinding {
  KEYBIND_MINIMIZE = 0,
  KEYBIND_END_TURN,
  KEYBIND_SHOW_MAP,
  KEYBIND_GAME_MENU,
  KEYBIND_UNIT_MENU,
  KEYBIND_UNIT_CONTENT,
  KEYBIND_UNIT_INFO,
  KEYBIND_UNIT_NEXT,
  KEYBIND_UNIT_SELECT,
  KEYBIND_UNIT_UNDO,
  KEYBIND_UNIT_SWEEP,
  KEYBIND_COUNT
};

enum GameType {
  GTYPE_AI = 0,
  GTYPE_HOTSEAT,
#ifndef DISABLE_NETWORK
  GTYPE_NET_SERVER,
  GTYPE_NET_CLIENT,
#endif
  GTYPE_PBEM,
  GTYPE_COUNT
#ifdef DISABLE_NETWORK
  , // ugh!
  GTYPE_NET_SERVER,
  GTYPE_NET_CLIENT
#endif
};

class Options {
public:
  Options( void );

  void SetDamageIndicator( bool di ) { show_damage = di; }
  void SetTurnReplay( bool rep ) { replay = rep; }
  void SetQuickReplay( bool rep ) { quick_replay = rep; }
  void SetCampaign( bool flag ) { campaign = flag; }
  void SetLanguage( const char *lang ) { language.assign(lang); }
  void SetGameType( GameType type ) { gametype = type; }

  bool GetDamageIndicator( void ) const { return show_damage; }
  bool GetTurnReplay( void ) const { return replay; }
  bool GetQuickReplay( void ) const { return quick_replay; }
  bool GetCampaign( void ) const { return campaign; }
  const char *GetLanguage( void ) const { return language.c_str(); }
  GameType GetGameType( void ) const { return gametype; }

  bool IsAI( void ) const
    { return (gametype == GTYPE_AI) || GetCampaign(); }
  bool IsPBEM( void ) const
    { return (gametype == GTYPE_PBEM) && !GetCampaign(); }
  bool IsNetwork( void ) const
    { return ((gametype == GTYPE_NET_SERVER) ||
              (gametype == GTYPE_NET_CLIENT)) && !GetCampaign(); }

  void SetRemoteName( const char *name );
  const char *GetRemoteName( void ) const;
  void SetRemotePort( unsigned short port ) { server_port = port; }
  unsigned short GetRemotePort( void ) const { return server_port; }
  void SetLocalPort( unsigned short port ) { local_port = port; }
  unsigned short GetLocalPort( void ) const { return local_port; }

  bool IsLocked( const string &map ) const
    { return find(unlocked_maps.begin(), unlocked_maps.end(), map)
             == unlocked_maps.end(); }
  void Unlock( const string &map )
    { if (IsLocked(map)) unlocked_maps.push_back( map ); }
  const vector<string> &GetUnlockedMaps( void ) const
    { return unlocked_maps; }

  const SDLKey *GetKeyBindings( void ) const { return keymap; }
  void SetKeyBinding( KeyBinding action, SDLKey key ) { keymap[action] = key; }

private:
  GameType gametype;  // hot seat, ai, pbem, or network
  bool show_damage;   // show damage indicator
  bool replay;        // show turn replays
  bool quick_replay;  // show only combat results
  bool campaign;      // playing a campaign
  string language;

  // network related settings
  string server;
  unsigned short server_port; // connecting as client
  unsigned short local_port;  // acting as server

  vector<string> unlocked_maps;
  SDLKey keymap[KEYBIND_COUNT];
};

#endif  /* _INCLUDE_OPTIONS_H */
