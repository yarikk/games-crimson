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
// widget.cpp
////////////////////////////////////////////////////////////////////////

#include <ctype.h>

#include "widget.h"
#include "sound.h"
#include "misc.h"
#include "strutil.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Widget::Widget
// DESCRIPTION: Initialize the widget object.
// PARAMETERS : id     - widget ID; by default, this ID is returned to
//                       the event handler upon widget activation
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - widget flags (see widget.h for details)
//              title  - widget title, may be NULL, underscore chooses
//                       keystroke
//              window - window to attach widget to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Widget::Widget( short id, short x, short y, unsigned short w, unsigned short h,
        unsigned short flags, const char *title, Window *window ) :
        Rect( x, y, w, h ), surface(window), key(0),
        font(NULL), hook(NULL), clicked(false) {
  this->id = id;
  this->flags = flags;
  SetTitle( title );

  window->AddWidget( this );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Widget::Show
// DESCRIPTION: Copy the widget to the display surface.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Widget::Show( void ) const {
  surface->Show( *this );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Widget::Push
// DESCRIPTION: Change the widget state to 'clicked', then redraw and
//              reblit it to reflect that change graphically.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Widget::Push( void ) {
  if ( !clicked ) {
    clicked = true;
    Draw();
    Show();
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Widget::Release
// DESCRIPTION: Change the widget state to 'released', then redraw and
//              reblit it to reflect that change graphically.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Widget::Release( void ) {
  clicked = false;
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Widget::HandleEvent
// DESCRIPTION: Distribute system events to the proper handling
//              functions.
// PARAMETERS : event - event received by the (window) event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status Widget::HandleEvent( const SDL_Event &event ) {

  switch ( event.type ) {
  case SDL_MOUSEBUTTONDOWN: return MouseDown( event.button ); break;
  case SDL_MOUSEBUTTONUP:   return MouseUp( event.button ); break;
  case SDL_KEYDOWN:         return KeyDown( event.key.keysym ); break;
  case SDL_KEYUP:           return KeyUp( event.key.keysym ); break;
  case SDL_MOUSEMOTION:     return MouseMove( event.motion ); break;
  default: return GUI_OK;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Widget::PrintTitle
// DESCRIPTION: Draw the widget title to the specified position.
// PARAMETERS : xoff - horizontal pixel position
//              yoff - vertical pixel position
//              hcol - highlight colour for the keyboard shortcut
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Widget::PrintTitle( short xoff, short yoff, const Color &hcol ) const {
  if ( !title.empty() ) {
    if ( Disabled() )
      font->Write( title.c_str(), surface, xoff, yoff, Color(CF_COLOR_GHOSTED) );
    else {
      font->Write( title.c_str(), surface, xoff, yoff );

      // highlight keyboard shortcut
      if ( key && (keypos != -1) ) {
        // don't worry about UTF-8 sequences with more than 2 bytes for now
        short keylen = (title[keypos] & 0x80) ? 2 : 1;
        xoff += font->TextWidth( title.substr( 0, keypos+keylen ).c_str() )
              - font->TextWidth( title.substr( keypos, keylen ).c_str() );
        font->Write( title.substr( keypos, keylen ).c_str(),
                     surface, xoff, yoff, hcol );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Widget::PrintTitle
// DESCRIPTION: Draw the widget title to the position requested by the
//              WIDGET_ALIGN_xxx flags and possibly highlight the
//              keyboard shortcut. Note: Some widgets may not support
//              all of the placement flags.
// PARAMETERS : hcol - highlight colour for the keyboard shortcut
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Widget::PrintTitle( const Color &hcol ) const {
  if ( !title.empty() ) {
    unsigned short xoff, yoff;
    unsigned short tlen = font->TextWidth(title.c_str());
    bool within = (flags & (WIDGET_ALIGN_CENTER|WIDGET_ALIGN_WITHIN)) != 0;

    if ( flags & WIDGET_ALIGN_TOP ) {
      yoff = y - font->Height() - 4;
    } else {
      yoff = y + (h - font->Height()) / 2;
    }

    if ( within ) {
      if ( flags & WIDGET_ALIGN_LEFT )
        xoff = x + 4;
      else if ( flags & WIDGET_ALIGN_RIGHT )
        xoff = x + (w - tlen) - 4;
      else
        xoff = x + (w - tlen) / 2;

      if ( Clicked() ) {
        ++xoff;            // slightly displace label
        ++yoff;
      }
    } else {
      if ( flags & WIDGET_ALIGN_LEFT )
        xoff = x - tlen - 4;
      else if ( flags & WIDGET_ALIGN_RIGHT )
        xoff = x + w + 4;
      else
        xoff = x + (w - tlen) / 2;

      surface->DrawBack( xoff, yoff, tlen, font->Height() );
    }

    PrintTitle( xoff, yoff, hcol );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Widget::SetTitle
// DESCRIPTION: Set the widget title. An underscore character in the
//              title string designates the following character as the
//              keyboard shortcut for this widget.
// PARAMETERS : str - widget title
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Widget::SetTitle( const char *str ) {
  title.erase();
  keypos = -1;

  if ( str ) {
    for (int i = 0; str[i] != '\0'; ++i) {
      if ( str[i] != '_' ) title += str[i];
      else if ( str[i+1] != '\0' ) {
        key = StringUtil::utf8chartoascii( &str[i+1] );
        keypos = i;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::CompositeWidget
// DESCRIPTION: Initialize the widget object.
// PARAMETERS : id     - widget ID; by default, this ID is returned to
//                       the event handler upon widget activation
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - widget flags (see widget.h for details)
//              title  - widget title, may be NULL, underscore chooses
//                       keystroke
//              window - window to attach widget to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

CompositeWidget::CompositeWidget( short id, short x, short y, unsigned short w, unsigned short h,
        unsigned short flags, const char *title, Window *window ) :
  Widget( id, x, y, w, h, flags, title, window ) {}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::~CompositeWidget
// DESCRIPTION: Destroy all components.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

CompositeWidget::~CompositeWidget( void ) {
  for ( vector<Widget *>::iterator i = components.begin();
        i != components.end(); ++i ) delete *i;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::Draw
// DESCRIPTION: Draw all components.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CompositeWidget::Draw( void ) {
  for ( vector<Widget *>::iterator i = components.begin();
        i != components.end(); ++i ) (*i)->Draw();
}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::MouseDown
// DESCRIPTION: Distribute mouse down events to all components.
// PARAMETERS : button - button event
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status CompositeWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  GUI_Status rc = GUI_OK;

  for ( vector<Widget *>::iterator i = components.begin();
        (i != components.end()) && (rc == GUI_OK); ++i ) {
    rc = (*i)->MouseDown( button );
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::MouseUp
// DESCRIPTION: Distribute mouse up events to all components.
// PARAMETERS : button - button event
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status CompositeWidget::MouseUp( const SDL_MouseButtonEvent &button ) {
  GUI_Status rc = GUI_OK;

  for ( vector<Widget *>::iterator i = components.begin();
        (i != components.end()) && (rc == GUI_OK); ++i ) {
    rc = (*i)->MouseUp( button );
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::KeyDown
// DESCRIPTION: Distribute key down events to all components.
// PARAMETERS : key - key descriptor
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status CompositeWidget::KeyDown( const SDL_keysym &key ) {
  GUI_Status rc = GUI_OK;

  for ( vector<Widget *>::iterator i = components.begin();
        (i != components.end()) && (rc == GUI_OK); ++i ) {
    rc = (*i)->KeyDown( key );
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::KeyUp
// DESCRIPTION: Distribute key up events to all components.
// PARAMETERS : key - key descriptor
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status CompositeWidget::KeyUp( const SDL_keysym &key ) {
  GUI_Status rc = GUI_OK;

  for ( vector<Widget *>::iterator i = components.begin();
        (i != components.end()) && (rc == GUI_OK); ++i ) {
    rc = (*i)->KeyUp( key );
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::MouseMove
// DESCRIPTION: Distribute mouse motion events to all components.
// PARAMETERS : motion - mouse motion event
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status CompositeWidget::MouseMove( const SDL_MouseMotionEvent &motion ) {
  GUI_Status rc = GUI_OK;

  for ( vector<Widget *>::iterator i = components.begin();
        (i != components.end()) && (rc == GUI_OK); ++i ) {
    rc = (*i)->MouseMove( motion );
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::GetWidget
// DESCRIPTION: Retrieve a component by its identifier.
// PARAMETERS : id - widget identifier
// RETURNS    : pointer to widget or NULL if not found
////////////////////////////////////////////////////////////////////////

Widget *CompositeWidget::GetWidget( short id ) const {
  for ( vector<Widget *>::const_iterator i = components.begin();
        i != components.end(); ++i )
    if ( (*i)->ID() == id ) return *i;

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CompositeWidget::RemoveWidget
// DESCRIPTION: Remove a component from the widget.
// PARAMETERS : w - widget
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CompositeWidget::RemoveWidget( Widget *w ) {
  for ( vector<Widget *>::iterator i = components.begin();
        i != components.end(); ++i ) {
    if ( (*i) == w ) {
      components.erase( i );
      return;
    }
  }
}

