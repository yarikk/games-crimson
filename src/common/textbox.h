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

//////////////////////////////////////////////////////////////////////
// textbox.h - various widgets for texts and strings
//////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_TEXTBOX_H
#define _INCLUDE_TEXTBOX_H

#include <string>
using namespace std;

#include "widget.h"
#include "slider.h"
#include "listselect.h"

class TextWidget : public Widget {
public:
  TextWidget( short id, short x, short y, unsigned short w,
        unsigned short h, const char *str, unsigned short flags,
        const char *title, Window *window );
  ~TextWidget( void );

  virtual void SetText( const char *str );
  unsigned short SetRow( unsigned short top );
  unsigned short Rows( void ) const { return rows; }
  unsigned short RowsVisible( void ) const { return visrows; }

  void Draw( void );

private:
  Surface *textsurface;
  unsigned short rows;    // rows of pixels
  unsigned short visrows;  // rows visible at one time
  unsigned short toprow;  // first visible row
  unsigned short spacing;
};


class TextScrollWidget : public CompositeWidget, public WidgetHook {
public:
  TextScrollWidget( short id, short x, short y, unsigned short w,
         unsigned short h, const char *str, unsigned short flags,
         const char *title, Window *window );

  void SetText( const char *str );

private:
  GUI_Status WidgetActivated( Widget *widget, Window *win );
  GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
};


// node type used by the TextListWidget
class TLWNode : public Node {
public:
  TLWNode( const char *name );
  TLWNode( const char *name, void *data );
  TLWNode( const char *name, void *data, unsigned short id );

  const char *Name( void ) const { return name.c_str(); }
  void SetName( const string &name ) { this->name = name; }

  unsigned short ID( void ) const { return id; }
  void SetID( unsigned short id ) { this->id = id; }

  void *UserData( void ) const { return user_data; }
  void SetUserData( void *data ) { user_data = data; }

private:
  string name;
  unsigned short id;
  void *user_data;
};

class TLWList : public List {
public:
  TLWList( void ) {}

  void Sort( void );
  void InsertNodeSorted( TLWNode *n );
  TLWNode *GetNodeByID( unsigned short id ) const;
};


class TextListWidget : public ListWidget {
public:
  TextListWidget( short id, short x, short y, unsigned short w,
    unsigned short h, List *list, short selected,
    unsigned short flags, const char *title, Window *window ) :
    ListWidget( id, x, y, w, h, list, selected, flags, title, window )
             { itemh = font->Height() + spacing; Update(); }

  virtual void DrawNodes( void );
  virtual void PrintItem( const TLWNode *item, Surface *dest,
                          short x, short y, const Rect &clip ) const;
  virtual unsigned short ItemHeight( void ) const { return itemh; }

private:
  unsigned short itemh;
};


class InputValidator {
public:
  virtual ~InputValidator( void ) {}
  virtual bool ValidateKey( const char *str, unsigned short key,
                            unsigned short pos ) const = 0;
};

class StringWidget : public Widget {
public:
  StringWidget( short id, short x, short y, unsigned short w, unsigned short h,
                const char *str, unsigned short maxlen, unsigned short flags,
                const char *title, Window *window );
  virtual void Draw( void );

  const char *String( void ) const;
  void SetString( const char *newstr, bool upd = true );
  GUI_Status SetFocus( void );
  void SetValidator( InputValidator *val ) { validator = val; }

  GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
  GUI_Status KeyDown( const SDL_keysym &key );

private:
  GUI_Status InputLoop( void );
  void CharInput( short sym, unsigned short unicode );
  unsigned short CursorWidth( void ) const;
  short CursorPos( void ) const;
  unsigned short DispWidth( short ch ) const;

  string buffer;
  unsigned short cursor;
  unsigned short maxlen;
  short offset;
  Rect strbox;

  InputValidator *validator;
};

class NumberWidget : public StringWidget, public InputValidator {
public:
  NumberWidget( short id, short x, short y, unsigned short w, unsigned short h,
                long number, long min, long max, unsigned short flags,
                const char *title, Window *window );
  bool ValidateKey( const char *str, unsigned short key,
                    unsigned short pos ) const;
  long Number( void ) const { return num; }
  void SetNumber( long number, bool upd = true );
  void SetMin( long min );
  void SetMax( long max );
  void Release( void );

private:
  long num;
  long minval;
  long maxval;
  char numbuf[8];
  char numbuf2[8];
};

#endif  /* _INCLUDE_TEXTBOX_H */

