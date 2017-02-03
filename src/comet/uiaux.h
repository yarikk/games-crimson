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
// uiaux.h - auxiliary user interface classes
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_UIAUX_H
#define _INCLUDE_UIAUX_H

#include "textbox.h"
#include "mission.h"
#include "misc.h"

class TLWListBuilder {
public:
  static void BuildEventList( const List &events, TLWList &tlw );
  static void BuildUnitList( const List &units, TLWList &tlw );
  static void BuildUnitList( const List &units, TLWList &tlw, const Point &pos );
  static void BuildShopList( const List &shops, TLWList &tlw );
  static void BuildMsgList( Locale &msgs, TLWList &tlw );

  static void BuildUnitTypesList( const UnitSet &set, TLWList &tlw );
};

#endif	/* _INCLUDE_UIAUX_H */

