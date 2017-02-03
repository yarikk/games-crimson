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

/////////////////////////////////////////////////////////////////////
// main.cpp -- CoMET - The Crimson Fields Map Editing Tool
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#ifndef _WIN32_WCE
# include <time.h>
#endif

#include "edwindow.h"
#include "main.h"
#include "mission.h"
#include "color.h"
#include "fileio.h"
#include "sound.h"
#include "globals.h"

// local function prototypes
static void parse_options( int argc, char **argv, EdOptions &opts );
static void print_usage( char *prog );
static GUI_Status event_filter( SDL_Event &event, Window *window );
static void load_settings( EdOptions &opts );
static void save_settings( View *display );

// Start of program functions //////////////////////////////////////////

#ifdef _MSC_VER
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR szCmdLine, int sw) {
  int argc = __argc;
  char **argv = __argv;
#else
int main( int argc, char **argv ) {
#endif
  struct EdOptions options;

  srand( time(0) ); // init random number generator

  parse_options( argc, argv, options );

  // create options, save games, and maps directories if possible
  create_config_dir();
  string mapdir = get_home_levels_dir();
  if ( mapdir.length() == 0 ) mapdir = get_levels_dir();
  else if ( !File::Exists( mapdir ) ) make_dir( mapdir.c_str() );

  Editor ed( mapdir.c_str(), options );
  View *view = ed.GetView();

  if ( view ) {
    GUI_Status status;
    do {
      status = view->HandleEvents();
    } while ( status != GUI_QUIT );

  }

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : parse_options
// DESCRIPTION: Process any options given to the program on the command
//              line.
// PARAMETERS : argc - argument count
//              argv - pointer to array of arguments
//              opts - buffer to store options in
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

// parse command line arguments
void parse_options( int argc, char **argv, EdOptions &opts ) {
  // initialize with some default values
  opts.width = 800;
  opts.height = 600;
  opts.bpp = DISPLAY_BPP;
  opts.sound = true;
  opts.flags = SDL_HWSURFACE;
  opts.level = NULL;

  load_settings( opts );

  while ( argc > 1 ) {
    --argc;

    if (strcmp(argv[argc-1], "--width") == 0) {
      opts.width = atoi(argv[argc]);
    } else if (strcmp(argv[argc-1], "--height") == 0) {
      opts.height = atoi(argv[argc]);
    } else if (strcmp(argv[argc-1], "--level") == 0) {
      opts.level = argv[argc];
    } else if (strcmp(argv[argc-1], "--fullscreen") == 0) {
      if ( atoi( argv[argc] ) ) opts.flags |= SDL_FULLSCREEN;
      else opts.flags &= ~SDL_FULLSCREEN;
    } else if (strcmp(argv[argc-1], "--sound") == 0) {
      if ( atoi( argv[argc] ) ) opts.sound = true;
      else opts.sound = false;
    } else {
      if (strcmp(argv[argc], "--version") == 0)
        fprintf( stdout, PROGRAMNAME" "VERSION"\n" );
      else print_usage( argv[0] );
      exit ( 0 );
    }
    --argc;
  }
  if ( opts.width < MIN_XRES ) opts.width = MIN_XRES;
  if ( opts.height < MIN_YRES ) opts.height = MIN_YRES;
}

////////////////////////////////////////////////////////////////////////
// NAME       : print_usage
// DESCRIPTION: Print a usage message to stdout.
// PARAMETERS : prog - program name as given on the command line
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void print_usage( char *prog ) {
  fprintf( stdout, "Usage: %s [options]\n\n"
                   "Available options:\n"
                   "  --level <level>      load level or save file\n"
                   "  --width <width>      set screen width\n"
                   "  --height <height>    set screen height\n"
                   "  --fullscreen <1|0>   enable/disable fullscreen mode\n"
#ifndef DISABLE_SOUND
                   "  --sound <1|0>        enable/disable sound\n"
#endif
                   "  --help               display this help and exit\n"
                   "  --version            output version information and exit\n",
        prog );
}


////////////////////////////////////////////////////////////////////////
// NAME       : load_settings
// DESCRIPTION: Read default display settings from the crimsonrc file.
// PARAMETERS : opts - buffer to store the settings. These should
//                     already be initialized with some defaults in
//                     case the rc file doesn't exist or this function
//                     fails.
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void load_settings( EdOptions &opts ) {
  string cfedrc( get_config_dir() );
  cfedrc.append( CFEDRC );

  FILE *fp = fopen( cfedrc.c_str(), "r" );
  if ( fp ) {
    char linebuf[256], *val;
    unsigned short linecnt = 0;

    while ( fgets( linebuf, 255, fp ) ) {
      ++linecnt;
      if ( (linebuf[0] != '#') && (linebuf[0] != '\n') ) {
        val = strchr( linebuf, ' ' );
        if ( val ) {
          while ( *val == ' ' ) ++val;

          if ( !strncmp( linebuf, "width", 5 ) ) opts.width = atoi( val );
          else if ( !strncmp( linebuf, "height", 6 ) ) opts.height = atoi( val );
          else if ( !strncmp( linebuf, "fullscreen", 10 ) ) {
            if ( atoi( val ) != 0 ) opts.flags |= SDL_FULLSCREEN;
          } else if ( !strncmp( linebuf, "sound", 5 ) ) {
            if ( atoi( val ) == 0 ) opts.sound = 0;
            else opts.sound = 1;
          } else fprintf( stderr, "warning: unrecognized config option in line %d\n", linecnt );
        }
      }
    }
    fclose( fp );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : save_settings
// DESCRIPTION: Save current display settings to the crimsonrc file.
// PARAMETERS : display - pointer to display
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void save_settings( View *display ) {
  string cfedrc( get_config_dir() );
  cfedrc.append( CFEDRC );

  FILE *fp = fopen( cfedrc.c_str(), "w" );
  if ( fp ) {
    fprintf( fp, "width %d\n", display->Width() );
    fprintf( fp, "height %d\n", display->Height() );
    fprintf( fp, "fullscreen %d\n", display->IsFullScreen() ? 1 : 0 );
    fprintf( fp, "sound %d\n", Audio::GetSfxState() );
    fclose( fp );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : event_filter
// DESCRIPTION: This is the global event filter function. It is hooked
//              to the display and called everytime the event handler
//              receives an event.
// PARAMETERS : event  - event received by the event handler
//              window - pointer to the currently active window
// RETURNS    : GUI_Status; if the filter returns GUI_NONE the event
//              handler will not pass the event to its windows, but
//              silently drop it.
////////////////////////////////////////////////////////////////////////

GUI_Status event_filter( SDL_Event &event, Window *window ) {
  GUI_Status rc = GUI_OK;

  if ( event.type == SDL_KEYDOWN ) {
    rc = GUI_NONE;

    switch ( event.key.keysym.sym ) {
    case SDLK_F11:            // toggle SFX
      Audio::ToggleSfxState();
      break;
    default:
      rc = GUI_OK;            // send to windows
    }
  } else if ( event.type == SDL_QUIT ) rc = GUI_QUIT;

  return rc;
}


////////////////////////////////////////////////////////////////////////
// NAME       : Editor::Editor
// DESCRIPTION: Create a new editor instance and initialize screen
//              estate and general data. If the constructor fails,
//              Editor::GetView() will return NULL.
// PARAMETERS : mapdir - directory containing the maps
//              opts   - editor options
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Editor::Editor( const char *mapdir, const EdOptions &opts ) {
  view = NULL;

  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    fprintf( stderr, "Couldn't initialize: %s\n", SDL_GetError() );
    return;
  }

  bool err = false;

  // set main window title
  SDL_WM_SetCaption( PROGRAMNAME, PROGRAMNAME );
  SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );

  view = new View( opts.width, opts.height, opts.bpp, opts.flags );
  if ( !view->s_surface ) {
    fprintf( stderr, "Couldn't set video mode: %s\n", SDL_GetError() );
    err = true;
  } else if ( !Init(opts) ) {
    view->SetEventFilter( event_filter );

    EdWindow *ew = new EdWindow(mapdir, view);
    if ( ew ) {
      if ( opts.level ) ew->LoadMission(opts.level, false);
      else {
        ew->Draw();
        ew->Show();
      }
    } else err = true;
  } else err = true;

  if ( err ) {
    delete view;
    view = NULL;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Editor::~Editor
// DESCRIPTION: Close the editor.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Editor::~Editor( void ) {
  Audio::ShutdownSfx();
  Audio::ShutdownMusic();

  if ( view ) {
    save_settings( view );
    delete view;
  }

  SDL_Quit();
}


////////////////////////////////////////////////////////////////////////
// NAME       : Editor::Init
// DESCRIPTION: Initialize global data structures.
// PARAMETERS : opts - editor options
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Editor::Init( const EdOptions &opts ) {
  string datpath( get_data_dir() );
  datpath.append( CF_DATFILE );
  File datfile( datpath );
  if ( !datfile.Open( "rb" ) ) {
    cerr << "Error: Couldn't open '" << datpath << "'" << endl;
    return -1;
  }

  Surface *icons = new Surface;            // load icons surface
  if ( icons->LoadImageData( datfile ) ) {
    cerr << "Error reading data file" << endl;
    delete icons;
    return -1;
  }
  datfile.Close();

  datpath = get_data_dir() + CF_FONT;  // load fonts
  Font *f1 = new Font();
  Font *f2 = new Font();
  if ( TTF_Init() ||
       f1->Load( datpath.c_str(), CF_FONT_SMALL ) ||
       f2->Load( datpath.c_str(), CF_FONT_LARGE ) ) {
    cerr << "Error loading fonts" << endl;
    delete icons;
    delete f1;
    delete f2;
    return -1;
  }

  Audio::InitSfx( opts.sound, MIX_MAX_VOLUME );      // load sound effects
  Audio::InitMusic( false, MIX_MAX_VOLUME );

  view->SetFGPen( Color(0x00d87c00) );
  view->SetBGPen( Color(CF_COLOR_BLACK) );
  view->SetSmallFont( f1 );
  view->SetLargeFont( f2 );
  icons->DisplayFormat();
  view->SetSystemIcons( icons );
  return 0;
}

