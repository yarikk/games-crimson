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
// filewindow.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_FILEWINDOW_H
#define _INCLUDE_FILEWINDOW_H

#include <string>

#include "window.h"
#include "button.h"
#include "textbox.h"

class FileWindow : public Window, public WidgetHook {
public:
  FileWindow( const char *path, const char *file,
              const char *suffix, unsigned short flags, View *view );
  ~FileWindow( void );

  const char *GetPath( void ) const { return path.c_str(); }
  string GetFile( void ) const;
  void Draw( void );

  GUI_Status WidgetActivated( Widget *widget, Window *win );

  static short CreateFilesList( const char *dir,
               const char *suffix, TLWList &list );

  ButtonWidget *ok;
  ButtonWidget *cancel;

private:
  short FindFileInList( const char *file, const TLWList &list ) const;

  TextListWidget *list;
  StringWidget *str;
  TLWList files;
  string path;
  string suffix;
  Node *last;
};

#endif	/* _INCLUDE_FILEWINDOW_H */

