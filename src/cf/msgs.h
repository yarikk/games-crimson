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
// msgs.h - message identifiers
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_MSGS_H
#define _INCLUDE_MSGS_H

#include "lang.h"
extern Language Lang;

#define MSG(m)      Lang.GetMsg(m)

enum {
  MSG_B_OK = 0,
  MSG_B_CANCEL,
  MSG_B_YES,
  MSG_B_NO,
  MSG_B_START,
  MSG_B_MAP,
  MSG_B_OBJECTIVES,
  MSG_B_LEVEL_INFO,
  MSG_B_LOAD,
  MSG_B_SAVE,
  MSG_B_SKIP,
  MSG_B_END_TURN,
  MSG_B_OPTIONS,
  MSG_B_OPT_GENERAL,
  MSG_B_OPT_VIDEO,
  MSG_B_OPT_AUDIO,
  MSG_B_OPT_FULLSCREEN,
  MSG_B_OPT_DAMAGE,
  MSG_B_OPT_REPLAYS,
  MSG_B_OPT_REPLAYS_QUICK,
  MSG_B_OPT_SFX,
  MSG_B_OPT_MUSIC,
  MSG_B_OPT_LANGUAGE,
  MSG_B_OPT_KEYBOARD,
  MSG_B_GAME_TYPE,
  MSG_B_MAP_TYPE,
  MSG_B_SAVE_GAME,
  MSG_B_MAIN_MENU,
  MSG_B_QUIT,
  MSG_B_EXIT,
  MSG_B_HANDICAP,
  MSG_B_REPAIR,
  MSG_B_UNIT_INFO,
  MSG_B_UNIT_CONTENT,
  MSG_B_UNIT_SWEEP,
  MSG_B_UNIT_UNDO,
  MSG_B_SERVER,
  MSG_B_PORT,
  MSG_GAME_HOT_SEAT,
  MSG_GAME_PBEM,
  MSG_GAME_AI,
  MSG_GAME_NETWORK_SERVER,
  MSG_GAME_NETWORK_CLIENT,
  MSG_MAPS_SINGLES,
  MSG_MAPS_CAMPAIGNS,
  MSG_MAPS_SAVES,
  MSG_HANDICAP_NONE,
  MSG_HANDICAP_P1,
  MSG_HANDICAP_P2,
  MSG_TAG_PBEM,
  MSG_TAG_NET,
  MSG_TURN,
  MSG_UNITS,
  MSG_SHOPS,
  MSG_PLAYER_SELECTION,
  MSG_DEBRIEFING,
  MSG_LVL_INFO,
  MSG_OPTIONS_GENERAL,
  MSG_OPTIONS_VIDEO,
  MSG_OPTIONS_AUDIO,
  MSG_OPTIONS_LANGUAGE,
  MSG_OPTIONS_KEYBOARD,
  MSG_OPT_KEY_MINIMIZE,
  MSG_OPT_KEY_END_TURN,
  MSG_OPT_KEY_MAP,
  MSG_OPT_KEY_GAME_MENU,
  MSG_OPT_KEY_UNIT_MENU,
  MSG_OPT_KEY_UNIT_CONTENT,
  MSG_OPT_KEY_UNIT_INFO,
  MSG_OPT_KEY_UNIT_NEXT,
  MSG_OPT_KEY_UNIT_SELECT,
  MSG_OPT_KEY_UNIT_UNDO,
  MSG_OPT_KEY_UNIT_SWEEP,
  MSG_PRESS_KEY,
  MSG_LOAD_GAME,
  MSG_SAVE_GAME,
  MSG_GAME_SAVED,
  MSG_GAME_SAVED_PBEM,
  MSG_ENTER_PASSWORD,
  MSG_CHOOSE_PASSWORD,
  MSG_CONFIRM_PASSWORD,
  MSG_NET_WAITING,
  MSG_NET_WAITING_CLIENT,
  MSG_NET_CONNECTING,
  MSG_NET_CONFIG_SERVER,
  MSG_NET_CONFIG_CLIENT,
  MSG_TRANSFER_UNITS,
  MSG_ASK_SIDE,
  MSG_ASK_REPAIR,
  MSG_ASK_OVERWRITE,
  MSG_ASK_ABORT,
  MSG_ASK_ABORT_PBEM,
  MSG_ASK_ABORT_NETWORK,
  MSG_ASK_QUIT,
  MSG_RESULT_DRAW,
  MSG_RESULT_VICTORY,
  MSG_RESULT_DEFEAT,
  MSG_MESSAGE,
  MSG_WARNING,
  MSG_ERROR,
  MSG_ERR_WRITE,
  MSG_ERR_SAVE,
  MSG_ERR_NETWORK,
  MSG_ERR_LOAD_MAP,
  MSG_ERR_NO_ACCESS,
  MSG_ERR_NO_BRIEFING,
  MSG_ERR_NO_LVL_INFO,
  MSG_ERR_MAP_NOT_FOUND,
  MSG_ERR_NO_MAP,
  MSG_ERR_SWEEPER_FULL,
  MSG_ERR_NO_CRYSTALS,
  MSG_ERR_NO_PRODUCTION,
  CF_MSGS
};

#endif   /* _INCLUDE_MSGS_H */

