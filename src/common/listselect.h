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
// listselect.h - list widget class
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_LISTSELECT_H
#define _INCLUDE_LISTSELECT_H

#include "list.h"
#include "slider.h"

class ListWidget : public CompositeWidget, public WidgetHook {
public:
  ListWidget( short id, short x, short y, unsigned short w,
    unsigned short h, List *list, short selected,
    unsigned short flags, const char *title, Window *window );

  virtual void Draw( void );
  virtual void DrawNodes( void ) = 0;
  void Update( void );			// call after modifying the list

  GUI_Status Select( short item );	// highlight a list item
  void SwitchList( List *newlist, short select );

  virtual GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
  virtual GUI_Status KeyDown( const SDL_keysym &key );

  Node *Selected( void ) const { return list->GetNode(current); }
  virtual unsigned short ItemHeight( void ) const = 0;


protected:
  virtual GUI_Status WidgetActivated( Widget *widget, Window *win );
  virtual void Set( short row );	// make row the top row

  List *list;
  unsigned short nodes;
  short current;           // selected node
  unsigned short rows;
  unsigned short toprow;
  unsigned short visrows;
  short spacing;
  unsigned short listboxw;
};

#endif	/* _INCLUDE_LISTSELECT_H */

