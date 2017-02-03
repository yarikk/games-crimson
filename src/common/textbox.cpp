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
// textbox.cpp
////////////////////////////////////////////////////////////////////////

#include <cctype>
#include <string.h>
#include <stdlib.h>

#include "textbox.h"
#include "misc.h"

////////////////////////////////////////////////////////////////////////
// NAME       : TextWidget::TextWidget
// DESCRIPTION: Create a new TextWidget
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              str    - text to display (may be NULL)
//              flags  - widget flags (see widget.h for details)
//              title  - widget title (currently unused)
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

TextWidget::TextWidget( short id, short x, short y,
           unsigned short w, unsigned short h, const char *str,
           unsigned short flags, const char *title, Window *window ) :
    Widget( id, x, y, w, h, flags, title, window ) {
  textsurface = NULL;
  spacing = 2;
  SetText( str );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TextWidget::~TextWidget
// DESCRIPTION: Destroy a TextWidget
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

TextWidget::~TextWidget( void ) {
  delete textsurface;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TextWidget::SetText
// DESCRIPTION: Prepare the text for use and format it appropriately.
// PARAMETERS : str - string to display (may be NULL)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TextWidget::SetText( const char *str ) {
  delete textsurface;
  textsurface = NULL;
  rows = visrows = toprow = 0;

  if ( !str ) return;

  rows = font->TextHeight( str, w - 10, spacing );
  if ( rows > 0 ) {
    textsurface = new Surface();
    if ( textsurface->Create( w - 10, rows, surface->GetView()->ScreenBPP(), 0 ) ) {
      delete textsurface;
      textsurface = NULL;
      return;
    }

    textsurface->SetColorKey( Color(CF_COLOR_BLACK) );
    textsurface->Flood( Color(CF_COLOR_BLACK) );

    string full( str );
    int pos = 0, endpos = full.size(), rowcnt = 0;
    unsigned short xoff = 0;

    while ( pos < endpos ) {
      size_t linelen = font->FitText( &str[pos], w - 10, true );
      string sub( full.substr( pos, linelen ) );

      // remove newlines
      size_t rep = 0;
      while ( (rep = sub.find( '\n', rep )) != string::npos )
        sub.erase( rep, 1 );

      if ( flags & WIDGET_ALIGN_CENTER )
        xoff = (w - 10 - font->TextWidth( sub.c_str() )) / 2;
      font->Write( sub.c_str(), textsurface, xoff, rowcnt );

      rowcnt += font->Height() + spacing;
      pos += linelen;
    }

    visrows = h - 6;
    if ( visrows > rows ) visrows = rows;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : TextWidget::Draw
// DESCRIPTION: Draw the widget and render the text.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TextWidget::Draw( void ) {
  surface->DrawBack( *this );
  surface->DrawBox( *this, BOX_RECESSED );

  if ( textsurface )
    textsurface->Blit( surface, Rect(0,toprow,textsurface->Width(),visrows),
                       x + 5, y + 3 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TextWidget::SetRow
// DESCRIPTION: Set the first row visible in the widget textbox.
// PARAMETERS : top - first row to be visible; the widget automatically
//                    adjusts this value to fit as much text as possible
//                    into the box
// RETURNS    : new top row set (after optimization)
////////////////////////////////////////////////////////////////////////

unsigned short TextWidget::SetRow( unsigned short top ) {
  unsigned short maxrow = rows - visrows;
  if ( top <= maxrow ) toprow = top;
  else toprow = maxrow;
  Draw();
  Show();
  return toprow;
}


////////////////////////////////////////////////////////////////////////
// NAME       : TextScrollWidget::TextScrollWidget
// DESCRIPTION: Create a new TextScrollWidget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              str    - text to display (may be NULL)
//              flags  - widget flags (see widget.h for details)
//              title  - widget title (currently unused)
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

TextScrollWidget::TextScrollWidget( short id, short x, short y,
           unsigned short w, unsigned short h, const char *str,
           unsigned short flags, const char *title, Window *window ) :
    CompositeWidget( id, x, y, w, h, 0, 0, window ) {

  TextWidget *text = new TextWidget( 0, x, y, w, h, 0,
              flags|WIDGET_COMPOSITE, title, window );
  AddWidget( text );

  SetText( str );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TextWidget::WidgetActivated
// DESCRIPTION: Scroller has been used. Scroll the widget to the current
//              row.
// PARAMETERS : widget - calling widget (slider)
//              win    - window the widget belongs to
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status TextScrollWidget::WidgetActivated( Widget *widget, Window *win ) {
  TextWidget *t = static_cast<TextWidget *>( GetWidget( 0 ) );
  t->SetRow( static_cast<SliderWidget *>(widget)->Level() );
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TextScrollWidget::MouseDown
// DESCRIPTION: Distribute mouse down events to all components. Redirect
//              all scrollwheel events to the slider so that scrolling
//              works even if the pointer doesn't hover over the slider.
// PARAMETERS : button - button event
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status TextScrollWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  SDL_MouseButtonEvent mybutton = button;
  if ( ((button.button == SDL_BUTTON_WHEELUP) ||
       (button.button == SDL_BUTTON_WHEELDOWN)) &&
       Contains( button.x - surface->LeftEdge(), button.y - surface->TopEdge() ) ) {
    SliderWidget *s = static_cast<SliderWidget *>( GetWidget(1) );
    if ( s ) {
      // to address the slider widget we must adjust the click
      // coordinates to be inside the slider box
      mybutton.x = s->LeftEdge() + surface->LeftEdge();
      mybutton.y = s->TopEdge() + surface->TopEdge();
    }
  }

  return CompositeWidget::MouseDown( mybutton );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TextScrollWidget::SetText
// DESCRIPTION: Set a new text to be displayed in the widget.
// PARAMETERS : str - string to display (may be NULL)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TextScrollWidget::SetText( const char *str ) {
  TextWidget *t = static_cast<TextWidget *>( GetWidget(0) );
  SliderWidget *s = static_cast<SliderWidget *>( GetWidget(1) );

  bool needslider = str && (font->TextHeight( str, w - 10, 2 ) > h - 6);

  if ( needslider ) {
    if ( !s ) {
      s = new SliderWidget( 1, x + w - DEFAULT_SLIDER_SIZE, y,
          DEFAULT_SLIDER_SIZE, h, 0, 0, 0, 0,
          WIDGET_VSCROLL|WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY|WIDGET_COMPOSITE,
          NULL, surface );
      s->SetKeyStep( font->Height() + 2 );
      s->SetHook( this );
      AddWidget( s );

      t->SetSize( x, y, w - s->Width(), h );
    }
  } else if ( s ) {
    RemoveWidget( s );
    t->SetSize( x, y, w + s->Width(), h );
    delete s;
  }

  t->SetText( str );

  if ( needslider ) {
    s->Adjust( 0, t->Rows() - t->RowsVisible(), MAX(t->RowsVisible(),1) );
    s->ScrollTo( 0 );
  }
}


////////////////////////////////////////////////////////////////////////
// NAME       : TextListWidget::DrawNodes
// DESCRIPTION: Draw the list nodes.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TextListWidget::DrawNodes( void ) {
  Rect box( x + 4, y + 1 + spacing, listboxw - 8, h - 2 - 2 * spacing );
  Rect area( x + 1, y + 1, listboxw - 2, h - 2 );
  short num = toprow / ItemHeight();                        // number of top node
  TLWNode *n = static_cast<TLWNode *>(list->GetNode( num ));  // top node
  short xoff = box.x, yoff = box.y + (num * ItemHeight()) - toprow;
  Color fcol = font->GetColor();

  if ( Disabled() )
    font->SetColor( Color(CF_COLOR_GHOSTED) );

  surface->DrawBack( area );

  while ( n ) {
    if ( flags & WIDGET_ALIGN_CENTER )
      xoff = box.x + (box.w - font->TextWidth(n->Name())) / 2;

    if ( num == current ) {
      Rect hilite( x + 2, yoff, listboxw - 4, ItemHeight() );
      hilite.Clip( area );
      surface->FillRectAlpha( hilite, surface->GetFGPen() );
    }

    // print node name and clip to box
    PrintItem( n, surface, xoff, yoff + 1, box );

    yoff += ItemHeight();
    if ( yoff >= box.y + box.h ) break;

    ++num;
    n = static_cast<TLWNode *>( n->Next() );
  }

  font->SetColor( fcol );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TextListWidget::PrintItem
// DESCRIPTION: Print one item of the list widget. This can be used to
//              customize the look of the list, e.g. colour and font.
//              The item height must keep the same, though.
// PARAMETERS : item - node to print
//              dest - destination surface
//              x    - left edge of printing area
//              y    - top edge of printing area
//              clip - clipping rectangle
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TextListWidget::PrintItem( const TLWNode *item, Surface *dest,
                                short x, short y, const Rect &clip ) const {
    font->Write( item->Name(), dest, x, y, clip );
}


////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::StringWidget
// DESCRIPTION: Create a new StringWidget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              str    - string to display
//              maxlen - maximum length of string in characters (not
//                       including the trailing NUL-byte)
//              flags  - widget flags (see widget.h for details)
//              title  - widget title
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

StringWidget::StringWidget( short id, short x, short y, unsigned short w,
              unsigned short h, const char *str,
              unsigned short maxlen, unsigned short flags,
              const char *title, Window *window ) :
      Widget( id, x, y, w, h, flags, title, window ),
      cursor(0), maxlen(maxlen), offset(0),
      strbox(x+4,y+(h-font->Height())/2,w-8,font->Height()),
      validator(0) {
  SetString( str, false );
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::SetString
// DESCRIPTION: Set a new string to display in the widget.
// PARAMETERS : newstr - string to display (may be NULL)
//              upd    - whether to update the display (default value
//                       is "true")
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void StringWidget::SetString( const char *newstr, bool upd /* = true */ ) {
  if ( newstr ) buffer.assign( newstr );
  else buffer.erase();

  if ( upd ) {
    Draw();
    Show();
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::String
// DESCRIPTION: Get the current string.
// PARAMETERS : -
// RETURNS    : pointer to the string, or NULL if string is empty
////////////////////////////////////////////////////////////////////////

const char *StringWidget::String( void ) const {
  if ( !buffer.empty() ) return buffer.c_str();
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::Draw
// DESCRIPTION: Draw the widget.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void StringWidget::Draw( void ) {
  Rect clip;
  surface->DrawBack( *this );
  if ( !(flags & WIDGET_STYLE_NOBORDER ) )
    surface->DrawBox( *this, BOX_RECESSED );
  surface->GetClipRect( clip );
  surface->SetClipRect( strbox );

  if ( Clicked() )
    surface->FillRect( strbox.x + CursorPos(), strbox.y,
                       CursorWidth(), font->Height(), surface->GetFGPen() );

  short xoff = strbox.x - offset;
  if ( flags & WIDGET_STR_PASSWORD ) {
    unsigned short starw = font->CharWidth('*');
    unsigned short len = buffer.size();

    for ( int i = 0; i < len; ++i ) {
      if ( Clicked() && (cursor == i) )
        font->Write( '*', surface, xoff, strbox.y, surface->GetBGPen() );
      else font->Write( '*', surface, xoff, strbox.y );
      xoff += starw;
    }

  } else {
    font->Write( buffer.c_str(), surface, strbox.x - offset, strbox.y, strbox );
    if ( Clicked() && (cursor < buffer.size()) ) {
      font->Write( buffer[cursor], surface, strbox.x + CursorPos(), strbox.y, surface->GetBGPen() );
    }
  }

  surface->SetClipRect( clip );
  PrintTitle( surface->GetFGPen() );
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::MouseDown
// DESCRIPTION: React to mouse button presses.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status StringWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  if ( (button.button == SDL_BUTTON_LEFT) && !(flags & WIDGET_STR_CONST) &&
        Contains( button.x - surface->LeftEdge(),
                  button.y - surface->TopEdge() ) ) {
    short xoff = button.x - surface->LeftEdge() - x;
    unsigned short len = buffer.size();

    cursor = 0;
    while ( (cursor < len) && (CursorPos() + CursorWidth() < xoff) )
      ++cursor;

    short curpix = CursorPos();
    if ( curpix < 0 ) offset -= curpix;
    else if ( curpix + CursorWidth() >= strbox.w ) {
      offset += curpix + CursorWidth() - strbox.w;
    }

    return InputLoop();
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::KeyDown
// DESCRIPTION: React to key presses.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status StringWidget::KeyDown( const SDL_keysym &key ) {
  if ( (key.sym == this->key) && !(flags & WIDGET_STR_CONST) ) {
    unsigned short buflen = font->TextWidth( buffer.c_str() );

    cursor = buffer.size();
    offset = MAX( 0, buflen + CursorWidth() - strbox.w );
    return InputLoop();
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::InputLoop
// DESCRIPTION: After activation of the StringWidget, all events are
//              exclusively handled by this function, until the
//              widget is deselected at which point the control is given
//              back to the main event handler.
// PARAMETERS : -
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status StringWidget::InputLoop( void ) {
  SDL_Event event;
  GUI_Status rc = GUI_OK;
  int unicode;
  bool quit = false;

  Push();
  unicode = SDL_EnableUNICODE( 1 );

  do {
    rc = surface->GetView()->FetchEvent( event );
    if ( (rc == GUI_QUIT) || (rc == GUI_ERROR) ) quit = true;
    else {
      if ( ((event.type == SDL_MOUSEBUTTONDOWN) &&
             !Contains( event.button.x - x, event.button.y - y )) ||
           ((event.type == SDL_KEYUP) && ((event.key.keysym.sym == SDLK_RETURN) ||
                                          (event.key.keysym.sym == SDLK_TAB))) ) {
        quit = true;
      } else if ( event.type == SDL_KEYDOWN ) {
        CharInput( event.key.keysym.sym, event.key.keysym.unicode );
      }
    }
  } while ( !quit );

  cursor = offset = 0;

  SDL_EnableUNICODE( unicode );
  Release();
  if ( hook ) hook->WidgetActivated( this, surface );
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::CharInput
// DESCRIPTION: Insert a new character in the input buffer.
// PARAMETERS : sym     - ASCII symbol for printable characters
//              unicode - UNICODE representation for non-printable chars
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void StringWidget::CharInput( short sym, unsigned short unicode ) {
  bool changed = false;

  if ( unicode ) {
    switch ( unicode ) {
    case '\b':                // BACKSPACE - delete char at cursor-1
      if ( cursor > 0 ) {
        buffer.erase( --cursor, 1 );
        changed = true;
      }
      break;
    case 127:                 // DELETE - delete char at cursor
      if ( cursor < buffer.size() ) {
        buffer.erase( cursor, 1 );
        changed = true;
      }
      break;
    default:
      if ( buffer.size() < maxlen ) {   // insert char at cursor pos
        bool accept = (validator ?
                       validator->ValidateKey( buffer.c_str(), unicode, cursor ) :
                       isprint(unicode) != 0 );
        if ( accept ) {
          buffer.insert( cursor++, 1, unicode );
          changed = true;
        }
      }
    }
  } else switch ( sym ) {
    case SDLK_LEFT:                     // move cursor left
      if ( cursor > 0 ) {
        --cursor;
        changed = true;
      }
      break;
    case SDLK_RIGHT:
      if ( cursor < buffer.size() ) {
        ++cursor;
        changed = true;
      }
      break;
    default:
      break;
  }

  if ( changed ) {
    short curpix = CursorPos();
    if ( curpix < 0 ) offset += curpix;
    else if ( curpix + CursorWidth() >= strbox.w )
      offset += curpix + CursorWidth() - strbox.w;

    Draw();
    Show();
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::CursorWidth
// DESCRIPTION: Get width of cursor in pixels.
// PARAMETERS : -
// RETURNS    : cursor width
////////////////////////////////////////////////////////////////////////

unsigned short StringWidget::CursorWidth( void ) const {
  unsigned short curw;

  if ( flags & WIDGET_STR_PASSWORD ) curw = font->CharWidth('*');
  else if ( cursor < buffer.size() ) curw = font->CharWidth( buffer[cursor] );
  else curw = font->Width();

  return curw;
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::CursorPos
// DESCRIPTION: Get position of cursor on display in pixels.
// PARAMETERS : -
// RETURNS    : cursor position (- current offset)
////////////////////////////////////////////////////////////////////////

short StringWidget::CursorPos( void ) const {
  unsigned short cp;
  if ( flags & WIDGET_STR_PASSWORD )
    cp = cursor * font->CharWidth('*');
  else {
    cp = font->TextWidth(buffer.substr(0, cursor + 1).c_str());
    if ( buffer.size() > cursor ) cp -= CursorWidth();
  }
  return cp - offset;
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::DispWidth
// DESCRIPTION: Get width of a character in pixels.
// PARAMETERS : ch - glyph to measure
// RETURNS    : character width
////////////////////////////////////////////////////////////////////////

unsigned short StringWidget::DispWidth( short ch ) const {
  unsigned short cw;

  if ( flags & WIDGET_STR_PASSWORD ) cw = font->CharWidth('*');
  else cw = font->CharWidth( ch );

  return cw;
}

////////////////////////////////////////////////////////////////////////
// NAME       : StringWidget::SetFocus
// DESCRIPTION: Activate the widget, i.e. show the cursor and wait for
//              user input.
// PARAMETERS : -
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status StringWidget::SetFocus( void ) {
  cursor = 0;
  return InputLoop();
}


////////////////////////////////////////////////////////////////////////
// NAME       : NumberWidget::NumberWidget
// DESCRIPTION: Create a new NumberWidget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              number - initial value to display
//              min    - minimum allowed value
//              max    - maximum allowed value
//              flags  - widget flags (see widget.h for details)
//              title  - widget title
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

NumberWidget::NumberWidget( short id, short x, short y, unsigned short w, unsigned short h,
            long number, long min, long max, unsigned short flags,
            const char *title, Window *window ) :
    StringWidget( id, x, y, w, h, itoa(number, numbuf), strlen(itoa(max,numbuf2)),
                  flags, title, window ) {
  num = number;
  minval = min;
  maxval = max;
  SetValidator( this );
}

////////////////////////////////////////////////////////////////////////
// NAME       : NumberWidget::SetNumber
// DESCRIPTION: Fill the widget with a value.
// PARAMETERS : number - new widget value
//              upd    - whether to update the display (default is true)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void NumberWidget::SetNumber( long number, bool upd /* = true */ ) {
  num = MIN( MAX( minval, number ), maxval );

  SetString( itoa( num, numbuf ), upd );
}

////////////////////////////////////////////////////////////////////////
// NAME       : NumberWidget::ValidateKey
// DESCRIPTION: Only accept numbers for input.
// PARAMETERS : str - string currently entered in widget (not used)
//              key - char to be entered
//              pos - position at which to enter the char
// RETURNS    : TRUE if key is accepted, FALSE if refused
////////////////////////////////////////////////////////////////////////

bool NumberWidget::ValidateKey( const char *str, unsigned short key,
                                unsigned short pos ) const {
  return ((key >= '0') && (key <= '9')) || ((key == '-') && (pos == 0));
}

////////////////////////////////////////////////////////////////////////
// NAME       : NumberWidget::SetMin
// DESCRIPTION: Set a new minimum value.
// PARAMETERS : min - new minimum
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void NumberWidget::SetMin( long min ) {
  minval = min;

  if ( Number() < min ) SetNumber( min );
}

////////////////////////////////////////////////////////////////////////
// NAME       : NumberWidget::SetMax
// DESCRIPTION: Set a new maximum value.
// PARAMETERS : max - new maximum
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void NumberWidget::SetMax( long max ) {
  maxval = max;

  if ( Number() > max ) SetNumber( max );
}

////////////////////////////////////////////////////////////////////////
// NAME       : NumberWidget::Release
// DESCRIPTION: When the widget is released make sure the number which
//              was entered is inside the requested boundaries and
//              adjust it if necessary.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void NumberWidget::Release( void ) {
  const char *str = String();
  short n = (str ? atoi(str) : 0);
  SetNumber( n );
  StringWidget::Release();
}


////////////////////////////////////////////////////////////////////////
// NAME       : TLWNode::TLWNode
// DESCRIPTION: Create a new TextListWidget node.
// PARAMETERS : name - name to be displayed in TextListWidget
//              data - arbitrary data for private use by caller
//              id   - node identifier; this is not used internally
//                     either and may be used by the caller
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

TLWNode::TLWNode( const char *name, void *data, unsigned short id ) :
         name(name), id(id), user_data(data) {
}

TLWNode::TLWNode( const char *name, void *data ) :
         name(name), id(0), user_data(data) {
}

TLWNode::TLWNode( const char *name ) :
         name(name), id(0), user_data(0) {
}

////////////////////////////////////////////////////////////////////////
// NAME       : TLWList::Sort
// DESCRIPTION: Sort the list in ascending alphabetical order.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TLWList::Sort( void ) {
  TLWList s;

  while ( !IsEmpty() )
    s.AddHead( RemHead() );

  while ( !s.IsEmpty() )
    InsertNodeSorted( static_cast<TLWNode *>(s.RemTail()) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TLWList::InsertNodeSorted
// DESCRIPTION: Insert a node into the list according to its name.
//              Nodes will be sorted in ascending alphabetical order.
// PARAMETERS : n - TextListWidget node to be inserted
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TLWList::InsertNodeSorted( TLWNode *n ) {
  TLWNode *walk = static_cast<TLWNode *>( Tail() );

  while ( walk && (strcmp( n->Name(), walk->Name()) < 0) )
    walk = static_cast<TLWNode *>( walk->Prev() );

  InsertNode( n, walk );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TLWList::GetNodeByID
// DESCRIPTION: Retrieve the node with the given ID from the list.
// PARAMETERS : id - requested node identifier
// RETURNS    : pointer to the node, or NULL if not found
////////////////////////////////////////////////////////////////////////

TLWNode *TLWList::GetNodeByID( unsigned short id ) const {
  TLWNode *walk = static_cast<TLWNode *>( Head() );

  while ( walk && (walk->ID() != id) )
    walk = static_cast<TLWNode *>( walk->Next() );

  return walk;
}

