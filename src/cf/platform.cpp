// Crimson Fields -- a game of tactical warfare
// Copyright (C) 2000-2007 Jens Granseuer
// ported to Windows Mobile 2006 by Silvio Iaccarino (silvio@iaccarino.de)
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
// platform.cpp -- platform-specific functions
////////////////////////////////////////////////////////////////////////

#include "platform.h"

#ifdef _WIN32_WCE

#include <aygshell.h>
#include <winuser.h>
#include <notify.h>

#include "game.h"

static HWND GetMainWnd( void ) {
  return FindWindow( _T("SDL_app"), _T(PROGRAMNAME) );
}

// a dirty hack to keep phone editions working
static void ShowTaskBar( bool bShow ) {
  HWND hWndTaskBar = FindWindow( TEXT("HHTaskBar"), NULL );
  if (!hWndTaskBar)
    return;

  if (bShow) {
    if (!IsWindowVisible( hWndTaskBar ))
      ShowWindow( hWndTaskBar, SW_SHOWNORMAL );
    if (!IsWindowEnabled( hWndTaskBar ))
      EnableWindow( hWndTaskBar, TRUE );
    InvalidateRect( hWndTaskBar, NULL, TRUE );
    UpdateWindow( hWndTaskBar );
  } else if (IsWindowVisible( hWndTaskBar ))
    ShowWindow( hWndTaskBar, SW_HIDE );
}

// the old SDL window proc
static WNDPROC g_oldWindowProc = NULL;

static LRESULT crimsonWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
  View *theView;

  switch (msg) {
  case WM_SHOWWINDOW:
    if (wParam) {
      SHFullScreen( hWnd, SHFS_HIDESIPBUTTON|SHFS_HIDETASKBAR|SHFS_HIDESTARTICON );
      ShowTaskBar( false );
      theView = (View *)GetWindowLong( hWnd, GWL_USERDATA );
      if (theView) {
        theView->EnableUpdates();
        // screen needs update
        theView->Refresh();
      }
    } else {
      SHFullScreen( hWnd, SHFS_SHOWSIPBUTTON|SHFS_SHOWTASKBAR|SHFS_SHOWSTARTICON );
      ShowTaskBar( true );
      theView = (View *)GetWindowLong( hWnd, GWL_USERDATA );
      if (theView)
        theView->DisableUpdates();
    }
    break;
  case WM_ACTIVATE:
    // get the stored reference to the view
    theView = (View *)GetWindowLong( hWnd, GWL_USERDATA );

    if (theView) { // initialized yet
      WORD fActive = LOWORD(wParam);

      if (fActive) {
        SetForegroundWindow(hWnd); // I'm the one and only
        // hide the SIP panel to disable the virtual keyboard
        SHFullScreen( hWnd, SHFS_HIDESIPBUTTON|SHFS_HIDETASKBAR|SHFS_HIDESTARTICON );
        ShowTaskBar( false );
        theView->EnableUpdates();
        // screen needs update
        theView->Refresh();
      } else {
        // restore the SIP panel to enable the virtual keyboard
        SHFullScreen( hWnd, SHFS_SHOWSIPBUTTON|SHFS_SHOWTASKBAR|SHFS_SHOWSTARTICON );
        ShowTaskBar( true );
        theView->DisableUpdates();
      }
    }
    break;
  case WM_KEYDOWN:
  case WM_KEYUP: {
    UINT mappedKey = 0;
    switch (wParam) {
    case VK_LWIN:
      SDL_WM_IconifyWindow();
      return TRUE;
    case VK_ESCAPE: // WinCE devices don't have an ESC key
      mappedKey=VK_BACK;
      break;
    case VK_VOLUME_UP:
      mappedKey = VK_F6;
      break;
    case VK_NUMPAD7: // Volume Down
    case VK_VOLUME_DOWN:
      mappedKey = VK_F7;
      break;
    case VK_NUMPAD8: // '*'
      mappedKey = '.';
      break;
    case VK_NUMPAD9: // '#'
      mappedKey = ','; // VK_F9;
      break;
    case VK_MULTIPLY: // camera key
      mappedKey = VK_F10;
      break;
    }
    if (mappedKey) {
      // don't route message to SDL, simulate a key instead
      keybd_event( mappedKey, MapVirtualKey(mappedKey,0),
                   msg == WM_KEYUP ? KEYEVENTF_KEYUP : 0, 0 );
      return TRUE;
    }
    }
    break;
  }

  // now let SDL do the rest
  return CallWindowProc( g_oldWindowProc, hWnd, msg, wParam, lParam );
}

bool platform_init( GUIOptions &opts ) {
  HWND hwnd = FindWindow( _T("SDL_app"), _T(PROGRAMNAME) );
  if ( hwnd ) {
    SetForegroundWindow( hwnd );
    ShowWindow( hwnd, SW_SHOW );
    return false;
  }

  unsigned short maxw = GetSystemMetrics(SM_CXSCREEN);
  unsigned short maxh = GetSystemMetrics(SM_CYSCREEN);

  if( opts.px_width > maxw ) opts.px_width = maxw;
  if( opts.px_height > maxh ) opts.px_height = maxh;

  opts.sdl_flags |= SDL_FULLSCREEN;
  return true;
}

bool platform_setup( View *view ) {
  // prepare the SDL window for the PocketPC environment
  HWND hwnd = GetMainWnd();

  if (!hwnd) // no hook no fun!
    return false;

  // hide the SIP panel to disable the virtual keyboard
  SHFullScreen( hwnd, SHFS_HIDESIPBUTTON|SHFS_HIDETASKBAR|SHFS_HIDESTARTICON );
  SHSipPreference( hwnd, SIP_FORCEDOWN );

  // hide taskbar notifications without disabling phone
  ShowTaskBar( false );

  // store a reference to the main view
  SetWindowLong( hwnd, GWL_USERDATA, (LONG)view );

  // Subclass the windows proc
  WNDPROC theWindowProc = (WNDPROC)GetWindowLong( hwnd, GWL_WNDPROC );
  if (theWindowProc != NULL && theWindowProc != crimsonWndProc) {
    g_oldWindowProc = theWindowProc;
    SetWindowLong( hwnd, GWL_WNDPROC, (LONG)crimsonWndProc );
  }

  return true;
}

void platform_dispose( void ) {
  HWND hwnd = GetMainWnd();
  if (hwnd) {
    SetWindowLong( hwnd, GWL_WNDPROC, (LONG)g_oldWindowProc );
    SetWindowLong( hwnd, GWL_USERDATA, 0 );
    SHFullScreen( hwnd, SHFS_SHOWSIPBUTTON|SHFS_SHOWTASKBAR|SHFS_SHOWSTARTICON );
  }
  // restore previous system state
  ShowTaskBar( true );
}

void platform_shutdown( void ) {
  HWND hWndTaskBar = FindWindow( TEXT("HHTaskBar"), NULL );
  if (hWndTaskBar) {
    InvalidateRect( hWndTaskBar, NULL, TRUE );
    UpdateWindow( hWndTaskBar );
  }

  // sometimes closing SDL windows will leave some rubbish on screen
  HWND hwnd = ::GetDesktopWindow();
  if (IsWindowVisible(hwnd)) {
    InvalidateRect( hwnd, NULL, TRUE );
    UpdateWindow( hwnd );
  }
}

#else

bool platform_init( GUIOptions &opts ) { return true; }
bool platform_setup( View *view ) { return true; }
void platform_dispose( void ) {}
void platform_shutdown( void ) {}

#endif  // _WIN32_WCE
