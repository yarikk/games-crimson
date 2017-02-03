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

///////////////////////////////////////////////////////////////////
// filewindow.cpp
///////////////////////////////////////////////////////////////////

#include <string.h>

#include "filewindow.h"
#include "fileio.h"
#include "misc.h"
#include "msgs.h"
#include "globals.h"      // strcasecmp alternatives

////////////////////////////////////////////////////////////////////////
// NAME       : FileWindow::FileWindow
// DESCRIPTION: Create a new FileWindow instance. At the moment this
//              class is not usable as a generic file window, but only
//              for loading and saving Crimson Fields game files.
// PARAMETERS : path   - directory path to show files in
//              file   - default file name (may be NULL)
//              suffix - only files with this ending will be displayed;
//                       if the user enters a name the suffix will be
//                       appended if necessary (may be NULL)
//              flags  - window flags (see window.h for details)
//              view   - view to attach window to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

FileWindow::FileWindow( const char *path, const char *file,
            const char *suffix, unsigned short flags, View *view ) :
    Window( flags|WIN_CENTER, view ), path(path), suffix(suffix), last(0) {
  // calculate window dimensions
  const char *oklabel = (flags & WIN_FILE_SAVE) ? MSG(MSG_B_SAVE) : MSG(MSG_B_LOAD);
  const char *cancellabel = MSG(MSG_B_CANCEL);
  unsigned short oklen = sfont->TextWidth( oklabel );
  unsigned short cancellen = sfont->TextWidth( cancellabel );
  unsigned short labellen = MAX(oklen, cancellen) + 10;
  unsigned short winw = labellen + sfont->Width() * 30 + 20;
  unsigned short winh = sfont->Height() * 15 + lfont->Height() + 30;

  SetSize( MIN(winw, view->Width()), MIN(winh, view->Height()) );

  CreateFilesList( path, suffix, files );
  short def_node = FindFileInList( file, files );

  // create widgets
  list = new TextListWidget( 100, 5, lfont->Height() + 10, w - labellen - 20,
                 h - 24 - sfont->Height() - lfont->Height(), &files, def_node,
                 WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY, NULL, this );
  list->SetHook( this );
  str = new StringWidget( 101, 5, h - 13 - sfont->Height(),
                 list->Width(), sfont->Height() + 8, file,
                 22, 0, NULL, this );
  ok = new ButtonWidget( GUI_CLOSE, w - 5 - labellen,
                 h - 10 - 2 * (sfont->Height() + 8), labellen,
                 sfont->Height() + 8, WIDGET_DEFAULT, oklabel, this );
  cancel = new ButtonWidget( GUI_CLOSE, ok->LeftEdge(), h - 13 - sfont->Height(),
                 ok->Width(), ok->Height(), 0, cancellabel, this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : FileWindow::~FileWindow
// DESCRIPTION: Destroy a file window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

FileWindow::~FileWindow( void ) {
  // the list nodes would be cleaned automatically, but the space
  // allocated for the node names must be freed explicitly
  while ( !files.IsEmpty() ) {
    TLWNode *n = static_cast<TLWNode *>(files.RemHead());
    delete (string *)n->UserData();
    delete n;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : FileWindow::Draw
// DESCRIPTION: Draw the file window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void FileWindow::Draw( void ) {
  const char *title;
  if ( flags & WIN_FILE_SAVE ) title = MSG(MSG_SAVE_GAME);
  else if ( flags & WIN_FILE_LOAD ) title = MSG(MSG_LOAD_GAME);
  else title = "Select";
  Window::Draw();

  lfont->Write( title, this, (w - lfont->TextWidth(title))/2 + 3, 8, GetBGPen() );
  lfont->Write( title, this, (w - lfont->TextWidth(title))/2, 5, GetFGPen() );
}

////////////////////////////////////////////////////////////////////////
// NAME       : FileWindow::GetFile
// DESCRIPTION: Get the currently selected filename, including the path.
// PARAMETERS : -
// RETURNS    : pointer to a buffer containing the path and filename or
//              NULL
////////////////////////////////////////////////////////////////////////

string FileWindow::GetFile( void ) const {
  string fw_file;

  const char *file = str->String();
  if ( file ) {
    fw_file.append( path );
    append_path_delim( fw_file );
    fw_file.append( file );

    // append the suffix if necessary
    if ( suffix.length() > 0 ) {
      short suflen = suffix.length();
      string::size_type pos = fw_file.find( suffix.c_str(), fw_file.length() - suflen, suflen );
      if ( pos == static_cast<string::size_type>(-1) ) fw_file.append( suffix );
    }
  }

  return fw_file;
}

////////////////////////////////////////////////////////////////////////
// NAME       : FileWindow::FindFileInList
// DESCRIPTION: Check whether a file dwells in the list.
// PARAMETERS : file - file name to search for
//              list - list (of TLWNodes) to search in
// RETURNS    : index of the node containing the file, or -1 if not
//              found
////////////////////////////////////////////////////////////////////////

short FileWindow::FindFileInList( const char *file, const TLWList &list ) const {

  if ( file ) {
    short i = 0;
    for ( TLWNode *n = static_cast<TLWNode *>( list.Head() );
           n; n = static_cast<TLWNode *>( n->Next() ), ++i ) {
      if ( !strcmp( file, n->Name() ) ) return i;
    }
  }
  return -1;
}

////////////////////////////////////////////////////////////////////////
// NAME       : FileWindow::WidgetActivated
// DESCRIPTION: The is the list hook method. Whenever a file is selected
//              from the list, print its name in the string widget.
// PARAMETERS : widget - list widget
//              win    - pointer to file window
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status FileWindow::WidgetActivated( Widget *widget, Window *win ) {
  Node *node = static_cast<ListWidget *>(widget)->Selected();
  GUI_Status rc = GUI_OK;

  if ( node ) {
    if ( node == last ) rc = ok->Activate();
    else {
      str->SetString( static_cast<TLWNode *>(node)->Name() );
      last = node;
    }
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : FileWindow::CreateFilesList
// DESCRIPTION: Read the names of all files with a specified suffix in
//              a directory and put them into a list.
// PARAMETERS : dirname - name of the directory to go through
//              suffix  - suffix for the files to grab (may be NULL)
//              list    - list to append nodes to
// RETURNS    : number of files found or -1 on error; the nodes added
//              to the list are of the TLWNode class.
////////////////////////////////////////////////////////////////////////

short FileWindow::CreateFilesList( const char *dirname,
      const char *suffix, TLWList &list ) {
  TLWNode *node;
  string *full;
  short num = 0, suflen = 0;
  if ( suffix ) suflen = strlen( suffix );

  Directory dir( dirname );
  if ( dir.IsValid() ) {
    do {
      int len = dir.GetFileNameLen();

      if ( (suflen <= len) &&
           (strcasecmp( &(dir.GetFileName()[len-suflen]), suffix ) == 0) ) {

        node = new TLWNode( dir.GetFileName() );
        full = new string( dirname );
        if ( node && full ) {
          append_path_delim( *full );        // store full file path
          full->append( dir.GetFileName() );
          node->SetUserData( full );

          list.InsertNodeSorted( node );
          ++num;
        } else {
          delete node;
          delete full;
          num = -1;
          break;
        }
      }
    } while ( dir.NextFile() );
  }
  return num;
}

