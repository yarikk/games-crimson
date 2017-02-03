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
// listselect.cpp
////////////////////////////////////////////////////////////////////////

#include "listselect.h"

////////////////////////////////////////////////////////////////////////
// NAME       : ListWidget::ListWidget
// DESCRIPTION: Create a new list widget. All subclasses of ListWidget
//              must call ListWidget::Update() when initializing.
// PARAMETERS : id       - widget identifier
//              x        - left edge of widget
//              y        - top edge of widget
//              w        - widget width
//              h        - widget height
//              list     - list nodes
//              selected - node highlighted by default (-1 == none)
//              flags    - widget flags (see widget.h for details)
//              title    - widget title (currently unused)
//              window   - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

ListWidget::ListWidget( short id, short x, short y, unsigned short w,
    unsigned short h, List *list, short selected, unsigned short flags,
    const char *title, Window *window ) :
    CompositeWidget( id, x, y, w, h, flags, title, window ) {
  this->list = list;
  current = selected;
  listboxw = w;
  spacing = 2;
  toprow = 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListWidget::Draw
// DESCRIPTION: Draw the list widget.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ListWidget::Draw( void ) {
  CompositeWidget::Draw();
  PrintTitle( surface->GetFGPen() );
  surface->DrawBox( Rect(x,y,listboxw,h), BOX_RECESSED );
  DrawNodes();
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListWidget::Update
// DESCRIPTION: Update the internal variables if the list was
//              modified (added/removed nodes, changed names, or even
//              put in an entirely different list).
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ListWidget::Update( void ) {
  nodes = list->CountNodes();
  if ( current >= nodes ) {
    current = nodes - 1;
    if ( hook ) hook->WidgetActivated( this, surface );
  }

  rows = ItemHeight() * nodes + 2 * spacing + 2;
  visrows = h - 2 * spacing - 2;
  if ( visrows > rows ) {
    visrows = rows;
    toprow = 0;
  } else if ( visrows > rows - toprow ) {
  	toprow = rows - visrows;
  } else if ( current != -1 ) {
    short selrow = 1 + spacing + current * ItemHeight();
    if ( selrow < toprow ) toprow = selrow;
    else if ( selrow + ItemHeight() >= toprow + visrows )
      toprow = selrow - visrows + ItemHeight();
  }

  SliderWidget *slider = static_cast<SliderWidget *>( GetWidget(0) );
  if ( rows > visrows ) {
    if ( slider ) slider->Adjust( 0, rows - visrows, visrows );
    else {
      // create the corresponding slider widget
      slider = new SliderWidget( 0, x + w - DEFAULT_SLIDER_SIZE, y,
         DEFAULT_SLIDER_SIZE, h, 0, rows - visrows, toprow, visrows,
         WIDGET_VSCROLL | WIDGET_COMPOSITE |
         (flags & WIDGET_HSCROLLKEY ? 0 : WIDGET_HSCROLLKEY) |
         (flags & WIDGET_VSCROLLKEY ? 0 : WIDGET_VSCROLLKEY),
         NULL, surface );
      slider->SetHook( this );
      listboxw = w - DEFAULT_SLIDER_SIZE;   // adjust list box size
      AddWidget( slider );
    }
  } else if ( slider ) {  // delete the slider; we don't need it any more
    listboxw += slider->Width();   // adjust list box size
    RemoveWidget( slider );
    delete slider;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListWidget::SwitchList
// DESCRIPTION: Replace the current list by another one to be displayed.
// PARAMETERS : newlist - pointer to the list to replace the current
//              select  - number of the node to select initially for the
//                        new list
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ListWidget::SwitchList( List *newlist, short select ) {
  list = newlist;
  current = select;
  toprow = 0;
  Update();
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListWidget::Select
// DESCRIPTION: Adjust the list view so that the selected item is
//              visible and highlight it.
// PARAMETERS : item - number of the item to select
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ListWidget::Select( short item ) {
  if ( item < 0 ) item = -1;
  else if ( item >= nodes ) item = nodes - 1;

  if ( item != current ) {
    short newtop = toprow;
    current = item;

    if ( current >= 0 ) {
      short selrow = current * ItemHeight();
      if ( selrow < toprow ) newtop = selrow;
      else if ( selrow + ItemHeight() >= newtop + visrows )
        newtop = selrow - visrows + ItemHeight();
    }

    if ( toprow != newtop ) {
      static_cast<SliderWidget *>( GetWidget(0) )->ScrollTo( newtop );
    } else {
      DrawNodes();
      Show();
    }
  }
  if ( hook ) return hook->WidgetActivated( this, surface );
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListWidget::Set
// DESCRIPTION: Adjust the list view so that row is the top row.
// PARAMETERS : row - first visible row
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ListWidget::Set( short row ) {
  toprow = row;
  DrawNodes();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListWidget::WidgetActivated
// DESCRIPTION: The slider has been moved. Update the widgeti
//              accordingly.
// PARAMETERS : widget - calling widget (slider)
//              win    - window the widget is attached to
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status ListWidget::WidgetActivated( Widget *widget, Window *win ) {
  if ( widget->ID() == 0 )
    Set( static_cast<SliderWidget *>(widget)->Level() );
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListWidget::MouseDown
// DESCRIPTION: Select a node.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ListWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  SDL_MouseButtonEvent mybutton = button;
  if ( ((button.button == SDL_BUTTON_WHEELUP) ||
       (button.button == SDL_BUTTON_WHEELDOWN)) &&
       Contains( button.x - surface->LeftEdge(), button.y - surface->TopEdge() ) ) {
    // reroute all wheel events to the scroller
    SliderWidget *s = static_cast<SliderWidget *>( GetWidget(0) );
    if ( s ) {
      mybutton.x = s->LeftEdge() + surface->LeftEdge();
      mybutton.y = s->TopEdge() + surface->TopEdge();
    }
  }

  GUI_Status rc = CompositeWidget::MouseDown( mybutton );

  if ( (rc == GUI_OK) && (button.button == SDL_BUTTON_LEFT) ) {
    short mx = button.x - surface->LeftEdge();
    short my = button.y - surface->TopEdge();
    Rect sensitive( x + 2, y + spacing, listboxw - 4, h - 2 * spacing );

    if ( sensitive.Contains( mx, my ) ) {
      short item = (my - y - spacing - 1 + toprow) / ItemHeight();
      if ( (item < nodes) ) return Select( item );
    }
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListWidget::KeyDown
// DESCRIPTION: If the widget has WIDGET_[HV]SCROLLKEY set, the user can
//              shuffle through the items with the cursor keys.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ListWidget::KeyDown( const SDL_keysym &key ) {
  GUI_Status rc = CompositeWidget::KeyDown( key );

  if ( rc != GUI_OK ) return rc;

  if ( key.sym == this->key ) rc = Select( current );

  else if ( flags & (WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY) ) {
    switch ( key.sym ) {
    case SDLK_UP:
      if ( (current > 0) && (flags & WIDGET_VSCROLLKEY) )
        return Select( current - 1 );
    case SDLK_LEFT:
      if ( (current > 0) && (flags & WIDGET_HSCROLLKEY) )
        return Select( current - 1 );
      break;
    case SDLK_DOWN:
      if ( (current < nodes - 1) && (flags & WIDGET_VSCROLLKEY) )
        return Select( current + 1 );
    case SDLK_RIGHT:
      if ( (current < nodes - 1) && (flags & WIDGET_HSCROLLKEY) )
        return Select( current + 1 );
      break;
#ifdef _WIN32_WCE
    case SDLK_RETURN:
      return Select( current );
#endif
    default:
      break;
    }
  }

  return rc;
}

