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
// ai.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_AI_H
#define _INCLUDE_AI_H

#include "mission.h"
#include "path.h"
#include "extwindow.h"

class AI {
public:
  AI( Mission &mission );

  void Play( void );

private:
  class AIObj : public Node {
  public:
    AIObj( void ) : needed_ground(0), needed_ship(0), needed_air(0),
                    requested_ground(0), requested_ship(0), requested_air(0) {}
    void AssignUnit( Unit *unit, unsigned short str );
    void ReleaseUnits( void );
    void ReleaseUnit( Unit *u ) const;
    bool UnitAssigned( const Unit *u ) const;

    Point pos;
    unsigned short type;
    unsigned short flags;
    unsigned short needed_ground;
    unsigned short needed_ship;
    unsigned short needed_air;
    bool requested_ground;
    bool requested_ship;
    bool requested_air;
    unsigned char priority;
    List alloc_units;

    class AIAllocNode : public Node {
    public:
      AIAllocNode( Unit *u ) { unit = u; }

      Unit *unit;
    };
  };

  void IdentifyObjectives( void );
  void AssignObjectives( void );
  void BuildReinforcements( void ) const;
  void ProcessObjective( AIObj &obj );
  void AddObjective( AIObj *obj );
  AIObj *GetObjectiveForUnit( const Unit *u ) const;

  void CommandUnit( Unit *u, AIObj &obj );
  void CommandUnitDefend( Unit *u, AIObj &obj );
  void CommandUnitConquer( Unit *u, AIObj &obj );
  void CommandUnitAttack( Unit *u, AIObj &obj );
  bool CommandUnitRepair( Unit *u );
  void CommandUnitReturnToBase( Unit *u );
  void CommandUnitTransport( Unit *u, AIObj &obj );

  void UnitPresence( Player *owner, AI::AIObj *obj,
                     short radius = -1 ) const;
  unsigned short UnitStrength( Unit *u ) const;
  bool UnitCanReach( const Unit *u, const Point &pos, unsigned short dist );
  bool UnitGoTo( Unit *u, const Point &dest, unsigned short dist );
  Unit *ClosestUnit( Player *owner, const Point &p,
       unsigned long uflags, unsigned long nuflags, const Unit *last = NULL ) const;
  Building *ClosestBuilding( Player *owner, const Point &p,
                             const Building *last = NULL ) const;

  Point FollowPath( const Unit *u, const Path &path, unsigned short dist = 0 ) const;
  Point FindBestHex( const Unit *u, const Unit *enemy ) const;
  Unit *FindBestTarget( const Unit *u );
  Transport *FindTransport( const Unit *u, const Point &dest,
             unsigned short dist, bool forreal );
  bool SameDirection( const Point &pos, const Point &dest1, const Point &dest2 ) const;

  Player *player;
  List objectives;
  Mission &mission;
  Map *map;
  ProgressWindow *progress;
};


#define AI_OBJ_DEFEND	0x0001
#define AI_OBJ_CONQUER	0x0002
#define AI_OBJ_ATTACK	0x0004
#define AI_OBJ_TRANSPORT 0x0008

#define AI_PRI_TRANSPORT 0
#define AI_PRI_LOW	20
#define AI_PRI_MEDIUM	40
#define AI_PRI_HIGH	60
#define AI_PRI_CRITICAL	80
#define AI_PRI_MAX	255

#define AI_ATTENTION_RADIUS	10     // the higher this value, the more defensive
                                       // computer player will act

#endif	/* _INCLUDE_AI_H */

