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
// uiaux.cpp
////////////////////////////////////////////////////////////////////////

#include "uiaux.h"

////////////////////////////////////////////////////////////////////////
// NAME       : TLWListBuilder::BuildEventList
// DESCRIPTION: Use the events list to create a second list which can
//              be displayed by a TextListWidget.
// PARAMETERS : events - list of events
//              tlw    - list of TLWNodes to be filled
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TLWListBuilder::BuildEventList( const List &events, TLWList &tlw ) {
  for ( Event *e = static_cast<Event *>( events.Head() ); e;
        e = static_cast<Event *>(e->Next()) ) {
    char buf[64];
    sprintf( buf, "%s / %s (%d)", e->Name(), e->TriggerName(), e->ID() );
    tlw.InsertNodeSorted( new TLWNode(buf, e, e->ID()) );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : TLWListBuilder::BuildUnitList
// DESCRIPTION: Use the units list to create a second list which can
//              be displayed by a TextListWidget.
// PARAMETERS : units - list of units
//              tlw   - list of TLWNodes to be filled
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TLWListBuilder::BuildUnitList( const List &units, TLWList &tlw ) {
  for ( Unit *u = static_cast<Unit *>( units.Head() ); u;
        u = static_cast<Unit *>(u->Next()) ) {
    char buf[28];
    sprintf( buf, "%s (%d)", u->Name(), u->ID() );
    tlw.InsertNodeSorted( new TLWNode(buf, u, u->ID()) );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : TLWListBuilder::BuildUnitList
// DESCRIPTION: Use the units list to create a second list which can
//              be displayed by a TextListWidget. Only include units at
//              the given hex coordinates which are inside a unit
//              container.
// PARAMETERS : units - list of units
//              tlw   - list of TLWNodes to be filled
//              pos   - requested hex position
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TLWListBuilder::BuildUnitList( const List &units, TLWList &tlw,
                                    const Point &pos ) {
  for ( Unit *u = static_cast<Unit *>( units.Head() ); u;
        u = static_cast<Unit *>(u->Next()) ) {
    if ( (u->Position() == pos) && u->IsSheltered() ) {
      char buf[28];
      sprintf( buf, "%s (%d)", u->Name(), u->ID() );
      tlw.InsertNodeSorted( new TLWNode(buf, u, u->ID()) );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : TLWListBuilder::BuildShopList
// DESCRIPTION: Use the buildings list to create a second list which can
//              be displayed by a TextListWidget.
// PARAMETERS : shops - list of buildings
//              tlw   - list of TLWNodes to be filled
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TLWListBuilder::BuildShopList( const List &shops, TLWList &tlw ) {
  for ( Building *b = static_cast<Building *>( shops.Head() ); b;
        b = static_cast<Building *>(b->Next()) ) {
    char buf[40];
    sprintf( buf, "%s (%d)", b->Name(), b->ID() );
    tlw.InsertNodeSorted( new TLWNode(buf, b, b->ID()) );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : TLWListBuilder::BuildMsgList
// DESCRIPTION: Use the messages list to create a second list which can
//              be displayed by a TextListWidget.
// PARAMETERS : msgs - message catalog
//              tlw  - list of TLWNodes to be filled
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TLWListBuilder::BuildMsgList( Locale &msgs, TLWList &tlw ) {
  const char *m;

  for ( int i = 0; (m = msgs.GetMsg(i)) != 0; ++i ) {
    tlw.InsertNodeSorted( new TLWNode(string(m).substr(0, 30).c_str(), const_cast<char *>(m), i) );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : TLWListBuilder::BuildUnitTypesList
// DESCRIPTION: Create a list containing all unit types defined in a
//              given unit set.
// PARAMETERS : set - unit set to use
//              tlw - list of TLWNodes to be filled
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TLWListBuilder::BuildUnitTypesList( const UnitSet &set, TLWList &tlw ) {
  const UnitType *ut;

  for ( unsigned short i = 0; (ut = set.GetUnitInfo( i )) != NULL; ++i ) {
    tlw.InsertNodeSorted( new TLWNode(ut->Name(), NULL, i) );
  }
}

