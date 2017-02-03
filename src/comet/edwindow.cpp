// CoMET - The Crimson Fields Map Editing Tool
// Copyright (C) 2002-2007 Jens Granseuer
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
// edwindow.cpp
////////////////////////////////////////////////////////////////////////

#include "edwindow.h"
#include "extwindow.h"
#include "extwindow2.h"
#include "gamewindow.h"
#include "filewindow.h"
#include "gfxwidget.h"
#include "fileio.h"
#include "strutil.h"
#include "main.h"

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::EdWindow
// DESCRIPTION: Create the editing window.
// PARAMETERS : mapdir - directory containing the map files
//              view   - pointer to the view to use
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdWindow::EdWindow( const string &mapdir, View *view ) :
          Window( view->x, view->y, view->w, view->h, 0, view ),
          mission(NULL),
          panel( view->w - DEFAULT_TILE_WIDTH * 4 - DEFAULT_SLIDER_SIZE - 20,
                 view->y, DEFAULT_TILE_WIDTH * 4 + DEFAULT_SLIDER_SIZE + 20, view->h ),
          mv( this,
              Rect( DEFAULT_SLIDER_SIZE, 0,
              w - panel.w - DEFAULT_SLIDER_SIZE, h - DEFAULT_SLIDER_SIZE),
              MV_DISABLE|MV_DISABLE_FOG ),
          mode(ED_MODE_VIEW), selected(-1), mapdir(mapdir) {

  unsigned short height = MIN( DEFAULT_TILE_HEIGHT * 6 + 4, h / 3 - 10 );
  unit_wd = new UnitWidget( L_ID_UNITS, panel.x + 5,
                            panel.y + panel.h - 6 - height,
                            panel.w - 10, height, 0, NULL, this, NULL );
  unit_wd->SetHook( this );

  tile_wd = new TileWidget( L_ID_TILES, unit_wd->x,
                            panel.y + sfont->Height() + 15,
                            unit_wd->w, panel.h - height - sfont->Height() - 30,
                            0, NULL, this, NULL );
  tile_wd->SetHook( this );

  ud_wd = new SliderWidget( S_ID_VERT, 0, 0, DEFAULT_SLIDER_SIZE, mv.Height(),
                            0, 0, 0, 1, WIDGET_VSCROLL, NULL, this );
  ud_wd->SetHook( this );
  lr_wd = new SliderWidget( S_ID_HORIZ, DEFAULT_SLIDER_SIZE, h - DEFAULT_SLIDER_SIZE,
                            mv.Width(), DEFAULT_SLIDER_SIZE,
                            0, 0, 0, 1, WIDGET_HSCROLL, NULL, this );
  lr_wd->SetHook( this );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::~EdWindow
// DESCRIPTION: Delete the editing window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdWindow::~EdWindow( void ) {
  delete mission;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::Draw
// DESCRIPTION: Draw the editing window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::Draw( void ) {
  mv.Draw();
  DrawBack( panel );
  DrawBox( panel, BOX_RAISED );

  Widget *wd = widgets;
  while ( wd ) {
    if ( !wd->Hidden() ) wd->Draw();
    wd = wd->next;
  }

  PrintCursorPos();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::PrintCursorPos
// DESCRIPTION: Print current cursor position to the panel.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::PrintCursorPos( void ) {
  Rect coord( panel.x + 5, panel.y + 5, panel.w - 10, sfont->Height() );
  char posbuf[16];
  DrawBack( coord );

  if ( mission ) {
    sprintf( posbuf, "X/Y: %d/%d", mv.Cursor().x, mv.Cursor().y );
    sfont->Write( posbuf, this, coord.x, coord.y );

    MapObject *obj = mission->GetMap().GetMapObject( mv.Cursor() );
    if ( obj && obj->Name() ) {
      string name = StringUtil::strprintf( "(%s)", obj->Name() );
      sfont->WriteEllipsis( name.c_str(), this,
             coord.x + sfont->TextWidth( posbuf ) + 5, coord.y, coord );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::HandleEvent
// DESCRIPTION: React on user input.
// PARAMETERS : event - event received by the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdWindow::HandleEvent( const SDL_Event &event ) {

  if ( !mission ) {
    if ( event.type == SDL_KEYDOWN )
      ShowContextMenu( Point(-1, -1) );
    else if ( event.type == SDL_MOUSEBUTTONDOWN )
      ShowContextMenu( Point(event.button.x - x, event.button.y - y) );

    return GUI_OK;
  }

  GUI_Status rc = Window::HandleEvent( event );
  if ( rc == GUI_OK ) {

    // check for keyboard commands
    if ( event.type == SDL_KEYDOWN ) {

      switch ( event.key.keysym.sym ) {
      case SDLK_KP1: case SDLK_KP2: case SDLK_KP3:
      case SDLK_KP4: case SDLK_KP6: case SDLK_KP7:
      case SDLK_KP8: case SDLK_KP9:
      case SDLK_LEFT: case SDLK_RIGHT: case SDLK_UP: case SDLK_DOWN:
        MoveCursor( event.key.keysym.sym );
        break;
      case SDLK_SPACE:      // equivalent to left mouse button
        LeftMouseButton( mv.Cursor() );
        break;
      case SDLK_q:
        Quit();
        break;
      case SDLK_ESCAPE:
        tile_wd->Select( -1 );
        unit_wd->Select( -1 );
        break;
      case SDLK_RETURN:
        ShowContextMenu( mv.Cursor() );

      case SDLK_u: {    // edit unit
        Unit *u = mv.GetMap()->GetUnit( mv.Cursor() );
        if ( u ) {
          EdUnitWindow *euw = new EdUnitWindow( *u, *mission, view );
          euw->SetMapView( mv );
        }
        break; }
      case SDLK_b: {    // edit building
        Building *b = mv.GetMap()->GetBuilding( mv.Cursor() );
        if ( b ) new EdBuildingWindow( *b, *mission, view );
        break; }

      default:
        break;
      }

    } else if ( event.type == SDL_MOUSEBUTTONDOWN ) {
      Point pos;
      bool validhex = !mv.Pixel2Hex( event.button.x - x, event.button.y - y, pos );

      if ( event.button.button == SDL_BUTTON_LEFT ) {
        if ( validhex ) LeftMouseButton( pos );
      } else if ( event.button.button == SDL_BUTTON_RIGHT ) {
         ShowContextMenu( validhex ? pos : Point(-1,-1) );
      }

    } else if ( event.type == SDL_MOUSEMOTION ) {
      // handle just like a mouse button if LMB is down and
      // user is terraforming
      if ( (mode == ED_MODE_TERRAIN) &&
           (event.motion.state & SDL_BUTTON( SDL_BUTTON_LEFT )) &&
           !ud_wd->Clicked() && !lr_wd->Clicked() ) {
        Point pos;
        if ( !mv.Pixel2Hex( event.motion.x, event.motion.y, pos ) )
          LeftMouseButton( pos );
      }
    }

  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::MoveCursor
// DESCRIPTION: Move the cursor in reaction to a key event.
// PARAMETERS : key - the key code used to give the order
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::MoveCursor( int key ) {
  Direction dir;

  switch ( key ) {
  case SDLK_KP1:  dir = SOUTHWEST; break;
  case SDLK_DOWN:
  case SDLK_KP2:  dir = SOUTH;     break;
  case SDLK_KP3:  dir = SOUTHEAST; break;
  case SDLK_KP7:  dir = NORTHWEST; break;
  case SDLK_UP:
  case SDLK_KP8:  dir = NORTH;     break;
  case SDLK_KP9:  dir = NORTHEAST; break;
  case SDLK_LEFT:
  case SDLK_KP4:  dir = WEST;      break;
  case SDLK_RIGHT:
  case SDLK_KP6:  dir = EAST;      break;
  default: return;
  }

  Point dest;
  if ( !mv.GetMap()->Dir2Hex( mv.Cursor(), dir, dest ) ) SetCursor( dest );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::SetCursor
// DESCRIPTION: Set the cursor to a new hex on the map. Contrary to the
//              low-level function in MapView this updates the display
//              at the old and new position if necessary.
// PARAMETERS : cursor - new cursor position
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::SetCursor( const Point &cursor ) {
  Rect upd;

  if ( mv.CursorEnabled() ) {
    upd = mv.SetCursor( Point(-1,-1) );  // disable cursor for hex update
    Show( upd );                         // update previous cursor position
  }

  upd = mv.SetCursor( cursor );
  Show( upd );

  PrintCursorPos();
  Show( panel );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::LeftMouseButton
// DESCRIPTION: Left mouse button has been pressed. See what we can do.
// PARAMETERS : hex - hex the button was clicked over
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::LeftMouseButton( const Point &hex ) {
  Map *map = mv.GetMap();

  if ( mode == ED_MODE_TERRAIN ) {
    if ( map->IsBuilding( hex ) && map->GetBuilding(hex) &&
       !(map->GetTerrainSet()->GetTerrainInfo(selected)->tt_type & TT_ENTRANCE) )
      new NoteWindow( "Warning", "You have just removed a shop entrance. "
                      "The corresponding shop has NOT been deleted! "
                      "(to do so restore the entrance and delete the shop before "
                      "erasing the entrance)", 0, view );
    map->SetHexType( hex, selected );
  } else if ( mode == ED_MODE_UNIT ) {
    if ( map->GetMapObject(hex) ) new NoteWindow( "Error", "Hex occupied.", 0, view );
    else {
      UnitSet *us = map->GetUnitSet();
      const UnitType *ut = us->GetUnitInfo( selected < us->NumTiles() ?
                                            selected : selected - us->NumTiles() );
      mission->CreateUnit( ut, selected < us->NumTiles() ? PLAYER_ONE : PLAYER_TWO, hex );
    }
  }

  SetCursor( hex );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::LoadMission
// DESCRIPTION: Load a mission from file and attach it to the display.
// PARAMETERS : file    - name of mission data file
//              filereq - pop up file requester
// RETURNS    : pointer to Mission object on success, NULL on error
////////////////////////////////////////////////////////////////////////

Mission *EdWindow::LoadMission( const char *file, bool filereq ) {
  string fn;
  Mission *ms = NULL;

  if ( filereq )
    fn = GetFileName( file, ".lev", mapdir, WIN_FILE_LOAD );
  else fn = file;

  if ( fn.length() > 0 ) {
    ms = new Mission();
    if ( ms ) {
      int err = ms->Load( fn.c_str() );
      if ( err ) {
        delete ms;
        new NoteWindow( "Error", "Couldn't load map. This could be a file type mismatch.", 0, view );
      } else {
        SetNewMission( ms );
      }
    }
  }

  return ms;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::SaveMission
// DESCRIPTION: Save a mission to file.
// PARAMETERS : file    - name of mission data file
//              filereq - pop up file requester
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdWindow::SaveMission( const char *file, bool filereq ) {
  string fn;
  GUI_Status rc = GUI_OK;

  if ( filereq )
    fn = GetFileName( file, ".lev", mapdir, WIN_FILE_SAVE );
  else fn = file;

  if ( fn.length() > 0 ) {
    if ( mission->Save( fn.c_str() ) != 0 ) {
      new NoteWindow( "Error", "Could not save mission!", WIN_CENTER, view );
    }
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::ExportMission
// DESCRIPTION: Save a mission to a plain text file.
// PARAMETERS : file    - default name of mission data file
//              filereq - pop up file requester
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdWindow::ExportMission( const char *file, bool filereq ) {
  string fn;
  GUI_Status rc = GUI_OK;

  if ( filereq )
    fn = GetFileName( file, ".src", mapdir, WIN_FILE_SAVE );
  else fn = file;

  if ( fn.length() > 0 ) mission->Export( fn.c_str() );
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::GetFileName
// DESCRIPTION: Get the name of a file to load or to save to. Pop up a
//              file requester.
// PARAMETERS : filename - default filename
//              suffix   - filename suffix
//              dirname  - name of directory to look in
//              flag     - WIN_FILE_LOAD or WIN_FILE_SAVE
// RETURNS    : file name or empty string on error
////////////////////////////////////////////////////////////////////////

string EdWindow::GetFileName( const char *filename, const char *suffix,
                              const string &dirname, int flag ) const {
  string file( filename );
  if ( file.length() > 0 ) file.append( suffix );

  bool filesel;
  do {
    GUI_Status rc;
    DialogWindow *dw;
    bool done = false;
    FileWindow *fw = new FileWindow( dirname.c_str(), file.c_str(),
                                     suffix, flag, view );
    fw->ok->SetID( 1 );
    fw->cancel->SetID( 0 );

    do {
      filesel = false;
      rc = fw->EventLoop();

      if ( rc == 1 ) {
        file = fw->GetFile();
        if ( file.length() != 0 ) {
          view->CloseWindow( fw );
          done = true;
        }
      } else if ( rc == 0 ) {
        view->CloseWindow( fw );
        file.assign( "" );
        done = true;
      }

    } while ( !done );

    if ( file.length() > 0 && (flag == WIN_FILE_SAVE) && File::Exists( file ) ) {
      // if file exists let user confirm the write
      char *conmsg = new char[ file.length() + 20 ];
      strcpy( conmsg, file.c_str() );
      strcat( conmsg, " exists. Overwrite?" );
      dw = new DialogWindow( NULL, conmsg, "_Yes|_No", 1, 0, view );
      dw->SetButtonID( 0, 1 );
      dw->SetButtonID( 1, 0 );
      rc = dw->EventLoop();
      view->CloseWindow( dw );
      if ( rc == 0 ) {
        filesel = true;
        file = file_part( file );
      }
      delete [] conmsg;
    }

  } while ( filesel );

  return file;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::WidgetActivated
// DESCRIPTION: This method is called when the user selects one of the
//              widgets associated with this window - the tiles or units
//              list, the map scrollers, or one of a number of buttons
//              including the main menu.
// PARAMETERS : widget - pointer to calling widget
//              win    - parent window of calling widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdWindow::WidgetActivated( Widget *widget, Window *win ) {
  GUI_Status rc = GUI_OK;

  if ( win == this ) {

    switch ( widget->ID() ) {
    case L_ID_UNITS:
      if ( unit_wd->Selected() == -1 ) {
        if ( tile_wd->Selected() == -1 ) {
          mode = ED_MODE_VIEW;
          selected = -1;
        }
      } else {
        mode = ED_MODE_UNIT;
        selected = unit_wd->Selected();
        tile_wd->Select( -1 );
      }
      break;
    case L_ID_TILES:
      if ( tile_wd->Selected() == -1 ) {
        if ( unit_wd->Selected() == -1 ) {
          mode = ED_MODE_VIEW;
          selected = -1;
        }
      } else {
        mode = ED_MODE_TERRAIN;
        selected = tile_wd->Selected();
        unit_wd->Select( -1 );
      }
      break;

    case S_ID_VERT: {
      Point offset = mv.GetOffsets();
      mv.Scroll( 0, ud_wd->Level() - offset.y );
      Show( mv );
      break; }
    case S_ID_HORIZ: {
      Point offset = mv.GetOffsets();
      mv.Scroll( lr_wd->Level() - offset.x, 0 );
      Show( mv );
      break; }
    }

  } else {

    Map *map = mv.GetMap();

    // context menu?
    if ( widget->ID() >= B_ID_TILE_GRAB )
      static_cast<MenuWindow *>(win)->CloseParent();
    view->CloseWindow(win);

    switch ( widget->ID() ) {
    case B_ID_NEW: {
      NewMissionWindow *nmw = new NewMissionWindow( view );
      nmw->SetHook( this, B_ID_NEW_MISSION_OK );
      break; }
    case B_ID_LOAD:
      LoadMission( "", true );
      break;
    case B_ID_SAVE:
      rc = SaveMission( mission->GetTitle().c_str(), true );
      break;
    case B_ID_EXPORT:
      rc = ExportMission( mission->GetTitle().c_str(), true );
      break;
    case B_ID_VALIDATE:
      ValidateMission();
      break;
    case B_ID_SETTINGS:
      new EdMissionSetupWindow( *mission, view );
      break;
    case B_ID_QUIT:
      Quit();
      break;

    case B_ID_TILE_GRAB:
      tile_wd->Select( map->HexTypeID(selected_hex) );
      break;
    case B_ID_TILE_SWAP:
      SwapTiles( map->HexTypeID(selected_hex), selected );
      break;

    case B_ID_UNIT_INFO:
      new UnitInfoWindow( map->GetUnit( selected_hex )->Type()->ID(), *map, view );
      break;
    case B_ID_UNIT_EDIT: {
      EdUnitWindow *euw = new EdUnitWindow( *(map->GetUnit( selected_hex )), *mission, view );
      euw->SetMapView( mv );
      break; }
    case B_ID_UNIT_DEL:
      mission->DeleteUnit( map->GetUnit(selected_hex) );
      Show( mv.UpdateHex( selected_hex ) );
      PrintCursorPos();
      Show( panel );
      break;

    case B_ID_BLD_CREATE:
      mission->CreateBuilding( PLAYER_ONE, selected_hex );  // fall through
    case B_ID_BLD_EDIT:
      new EdBuildingWindow( *(map->GetBuilding( selected_hex )),
                          *mission, view );
      break;
    case B_ID_BLD_DEL:
      mission->DeleteBuilding( map->GetBuilding( selected_hex ) );
      PrintCursorPos();
      Show( panel );
      break;

    case B_ID_EVENTS:
      new EdEventsWindow( *mission, view );
      break;

    case B_ID_MESSAGES:
      new EdMsgWindow( mission->GetMessages(), view );
      break;

    case B_ID_NEW_MISSION_OK: {
      NewMissionWindow *nmw = static_cast<NewMissionWindow *>(win);
      if ( nmw->GetMission() )
        SetNewMission( nmw->GetMission() );
      break; }
    }

  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::Quit
// DESCRIPTION: Ask user for confirmation and leave the editor.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::Quit( void ) const {
  DialogWindow *req = new DialogWindow( NULL, "Do you really want to quit?",
                                        "_Yes|_No", 1, 0, view );
  req->SetButtonID( 0, GUI_QUIT );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::ShowContextMenu
// DESCRIPTION: Open a pop-up menu with with context-dependent options.
// PARAMETERS : clicked - hex coordinates of the mouse click or -1,-1 if
//                        no specific hex was selected
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::ShowContextMenu( const Point &clicked ) {
  MenuWindow *menu = new MenuWindow( PROGRAMNAME, this, view );

  if ( mission ) {
    if ( clicked != Point(-1,-1) ) {
      Map *map = mv.GetMap();
      Unit *u = map->GetUnit( clicked );
      selected_hex = clicked;

      // tile sub-menu
      menu->AddMenu( 0, 0, "_Tile" );
      menu->AddItem( 1, B_ID_TILE_GRAB, 0, "_Grab" );
      menu->AddItem( 1, B_ID_TILE_SWAP,
                     (mode == ED_MODE_TERRAIN ? 0 : WIDGET_DISABLED),
                     "_Swap" );

      if ( u ) {  // unit sub-menu
        menu->AddMenu( 0, 0, "_Unit" );
        menu->AddItem( 1, B_ID_UNIT_INFO, 0, "_Info" );
        menu->AddItem( 1, B_ID_UNIT_EDIT, 0, "_Edit..." );
        menu->AddItem( 1, B_ID_UNIT_DEL, 0, "_Delete" );
      }

      if ( map->IsBuilding( clicked ) ) {
        Building *b = map->GetBuilding( clicked );

        menu->AddMenu( 0, 0, "_Building" );
        if ( b ) {
          menu->AddItem( 1, B_ID_BLD_EDIT,  0, "_Edit..." );
          menu->AddItem( 1, B_ID_BLD_DEL,   0, "_Delete" );
        } else menu->AddItem( 1, B_ID_BLD_CREATE, 0, "_Create..." );
      }
    }

    menu->AddItem( 0, B_ID_EVENTS, 0, "_Events..." );
    menu->AddItem( 0, B_ID_MESSAGES, 0, "_Messages..." );
    menu->AddBar( 0 );
  }

  // file ops
  menu->AddMenu( 0, 0, "_Project" );
  menu->AddItem( 1, B_ID_NEW, 0, "_New..." );
  menu->AddItem( 1, B_ID_LOAD, 0, "_Load..." );
  menu->AddItem( 1, B_ID_SAVE, (mission ? 0 : WIDGET_DISABLED),
                 "_Save..." );
  menu->AddItem( 1, B_ID_EXPORT, (mission ? 0 : WIDGET_DISABLED),
                 "_Export..." );
  menu->AddItem( 1, B_ID_VALIDATE, (mission ? 0 : WIDGET_DISABLED),
                 "_Validate" );
  menu->AddItem( 1, B_ID_SETTINGS, (mission ? 0 : WIDGET_DISABLED),
                 "Se_ttings" );
  menu->AddItem( 1, B_ID_QUIT, 0, "_Quit..." );

  menu->Layout();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::SwapTiles
// DESCRIPTION: Replace each occurence of one tile on the map with
//              another tile.
// PARAMETERS : t1 - identifier of the tiles to be replaced
//              t2 - identifier of the tiles to replace t1
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::SwapTiles( unsigned short t1, unsigned short t2 ) {
  Map *map = mv.GetMap();
  Point p;

  for ( p.y = 0; p.y < map->Height(); ++p.y ) {
    for ( p.x = 0; p.x < map->Width(); ++p.x ) {
      if ( map->HexTypeID(p) == t1 ) map->SetHexType( p, t2 );
    }
  }

  mv.Draw();
  Show( mv );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::ValidateMission
// DESCRIPTION: Check mission integrity.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::ValidateMission( void ) const {
  stringstream msg;

  if ( mission->Validate( msg ) > 0 ) {
    new MessageWindow( "Validation Results", msg.str().c_str(), view );
  } else {
     new NoteWindow( "Validation successful",
         "No errors or warnings have been detected.", 0, view );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdWindow::SetNewMission
// DESCRIPTION: Change the current mission.
// PARAMETERS : ms - new mission
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdWindow::SetNewMission( Mission *ms ) {
  delete mission;

  mv.Disable();
  mode = ED_MODE_VIEW;

  mission = ms;
  Map &map = mission->GetMap();
  mv.SetMap( &map );
  mv.SetCursor( Point(0,0) );

  tile_wd->SetTerrainSet( map.GetTerrainSet() );
  unit_wd->SetUnitSet( map.GetUnitSet() );

  short max = MAX( map.Height() * mv.TileHeight() +
                   mv.TileShiftY() - mv.Height(), 0 );
  ud_wd->Adjust( 0, max, mv.Height() );
  ud_wd->ScrollTo( 0 );

  max = MAX( map.Width() * (mv.TileWidth() - mv.TileShiftX()) +
             mv.TileShiftX() - mv.Width(), 0 );
  lr_wd->Adjust( 0, max, mv.Width() );
  lr_wd->ScrollTo( 0 );

  mv.Enable();
  Draw();
  Show();
}

