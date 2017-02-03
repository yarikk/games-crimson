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
// gamedefs.h - general game definitions
// This file can be included by both C and C++ code.
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_GAMEDEFS_H
#define _INCLUDE_GAMEDEFS_H

/* game info flags */
#define GI_SAVEFILE      0x0001   /* for saved games */
#define GI_PBEM          0x0002   /* email game */
#define GI_CAMPAIGN      0x0004   /* campaign map */
#define GI_AI            0x0008   /* intended as a single-player map */
#define GI_SKIRMISH      0x0010   /* map is suitable for skirmish play */
#define GI_NETWORK       0x0020   /* network game */
#define GI_GAME_OVER     0x8000

#define TURN_START		0
#define TURN_IN_PROGRESS	1

#define HANDICAP_NONE	0x01
#define HANDICAP_P1	0x02
#define HANDICAP_P2	0x04

#define PLAYER_ONE	0
#define PLAYER_TWO	1
#define PLAYER_NONE	2

#define HUMAN           0
#define COMPUTER        1


/* event types */
#define EVENT_MESSAGE           1  /* just show a message */
#define EVENT_MINING            2  /* set crystals in a building */
#define EVENT_SCORE		3  /* raise the success rate of a player */
#define EVENT_CONFIGURE		4  /* change various internal settings */
#define EVENT_CREATE_UNIT	5  /* create a unit */
#define EVENT_MANIPULATE_EVENT	6  /* set or unset flags for another event */
#define EVENT_RESEARCH		7  /* enable new units in factories */
#define EVENT_SET_HEX		8  /* change a map tile */
#define EVENT_SET_TIMER		9  /* adjust the timer for another event */
#define EVENT_DESTROY_UNIT	10 /* remove unit from board */

/* event triggers */
#define ETRIGGER_TIMER          0
#define ETRIGGER_UNIT_DESTROYED 1
#define ETRIGGER_HAVE_BUILDING  2
#define ETRIGGER_HAVE_UNIT      3
#define ETRIGGER_UNIT_POSITION  4
#define ETRIGGER_HANDICAP	5
#define ETRIGGER_HAVE_CRYSTALS	6

/* event flags */
#define EFLAG_DISABLED		0x0001
#define EFLAG_DISCARDED		0x8000

/* terrain definitions */
#define TT_ROAD			0x0001
#define TT_PLAINS		0x0002
#define TT_FOREST		0x0004
#define TT_SWAMP		0x0008
#define TT_MOUNTAINS		0x0010
#define TT_WATER_SHALLOW	0x0020
#define TT_WATER_DEEP		0x0040
#define TT_BARRICADES		0x0080
#define TT_RAILS		0x0100
#define TT_WATER		0x0200
#define TT_RESTRICTED           0x0400  /* for the default set, this means Infantry and aircraft only */
#define TT_TRENCHES             0x0800
#define TT_ENTRANCE		0x4000	/* entrance to a building */

/* unit definitions */
#define U_GROUND	0x00000001	/* a unit can only have one of U_GROUND, U_SHIP */
#define U_SHIP		0x00000002	/* and U_AIR set; for amphibian units you must */
#define U_AIR		0x00000004	/* always set U_GROUND! */

#define U_CONQUER	0x00000010	/* unit can take over enemy buildings */
#define U_SLOW		0x00000020	/* can only fight OR move on one turn */
#define U_TRANSPORT	0x00000040	/* can carry other units, */
                                        /* note: transporters may NEVER have U_CONQUER set! */
#define U_MINE		0x00000080	/* can be cleared/reused by minesweepers */
#define U_MEDIC		0x00000100	/* can repair other units */
#define U_MINESWEEPER	0x00000400

#define U_FLOATING	0x01000000	/* for units which can move on both land and water */
					/* (i.e. U_SHIP|U_GROUND) this flag must be set */
					/* when the unit moves on water; always true for ships */
#define U_DUMMY		0x02000000	/* used for units generated for replays */
#define U_BUSY		0x04000000	/* used by computer player */
#define U_DONE		0x08000000
#define U_MOVED		0x10000000	/* unit moved this turn */
#define U_ATTACKED	0x20000000	/* unit initiated combat this turn */
#define U_SHELTERED	0x40000000	/* unit is currently inside a building/transporter */
#define U_DESTROYED	0x80000000

#define UT_NO_SOUND		255

#define MAX_GROUP_SIZE		6
#define XP_MAX_LEVEL		6
#define XP_PER_LEVEL		3
#define MCOST_MIN		1  /* cost to cross a road hex for ground units
				      or any hex type for airborne units */

#define ICON_WIDTH	32
#define ICON_HEIGHT	32
#define XP_ICON_WIDTH	12
#define XP_ICON_HEIGHT	14

#define ICON_XP_BASE	0	// XP icons are 0 through 6
#define NUM_IMAGES	7

/* building definitions */
#define BLD_WORKSHOP	0x0001    /* units can be repaired */
#define BLD_FACTORY	0x0002    /* units can be produced */

#endif   /* _INCLUDE_GAMEDEFS_H */

