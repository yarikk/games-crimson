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

/////////////////////////////////////////////////////////////////////////
// slider.cpp
////////////////////////////////////////////////////////////////////////

#include "slider.h"
#include "misc.h"

////////////////////////////////////////////////////////////////////////
// NAME       : SliderWidget::SliderWidget
// DESCRIPTION: Create a new slider widget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              vmin   - lowest slider level allowed
//              vmax   - highest slider level allowed
//              start  - default level
//              ksize  - size of the slider knob in items (if the slider
//                       has 10 possible positions and ksize is 2, the
//                       size will be claculated as if there were 12
//                       levels and two of them were represented by one
//                       slider position, so the knob would take up
//                       2/12th of the widget size)
//              flags  - widget flags (see widget.h for details)
//              title  - widget title (currently unused)
//              window - window to add this widget to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

SliderWidget::SliderWidget( short id, short x, short y, unsigned short w,
    unsigned short h, short vmin, short vmax, short start, short ksize,
    unsigned short flags, const char *title, Window *window ) :
    Widget( id, x, y, w, h, flags, title, window ) {
  mousehit = false;
  current = start;
  keystep = 1;

  Adjust( vmin, vmax, ksize );
}

////////////////////////////////////////////////////////////////////////
// NAME       : SliderWidget::Adjust
// DESCRIPTION: Set new minimum and maximum slider values.
// PARAMETERS : newmin  - new minimum level
//              newmax  - new maximum level
//              newsize - new knob size
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void SliderWidget::Adjust( short newmin, short newmax, short newsize ) {
  min_level = newmin;
  max_level = newmax;
  size = MAX( newsize, 1 );

  if ( current < min_level ) current = min_level;
  else if ( current > max_level ) current = max_level;

  short num = max_level - min_level;
  if ( flags & WIDGET_HSCROLL ) {
    knob.w = MAX( 2, (w - 2) * size / (size + num) );
    knob.h = h - 2;
    if ( max_level > min_level ) step = (float)(w - 2 - knob.w) / num;
    else step = 0;
    knob.x = (short)(x + 1 + (current - min_level) * step + 0.5);
    knob.y = y + 1;
  } else {
    knob.w = w - 2;
    knob.h = MAX( 2, (h - 2) * size / (size + num) );
    if ( max_level > min_level ) step = (float)(h - 2 - knob.h) / num;
    else step = 0;
    knob.x = x + 1;
    knob.y = (short)(y + 1 + (current - min_level) * step + 0.5);
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : SliderWidget::Draw
// DESCRIPTION: Draw slider widget and knob.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void SliderWidget::Draw( void ) {
  surface->DrawBack( *this );
  surface->DrawBox( *this, BOX_RECESSED );

  // draw knob
  surface->DrawBox( knob, Clicked() ? BOX_RECESSED : BOX_RAISED );
  if ( Clicked() )
    surface->FillRectAlpha( knob.x+1, knob.y+1, knob.w-2, knob.h-2, Color(CF_COLOR_SHADOW) );

  PrintTitle( surface->GetFGPen() );
}

////////////////////////////////////////////////////////////////////////
// NAME       : SliderWidget::ScrollTo
// DESCRIPTION: Set the slider to a new level.
// PARAMETERS : level - new slider position
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void SliderWidget::ScrollTo( short level ) {
  current = level;
  if ( current < min_level ) current = min_level;
  else if ( current > max_level ) current = max_level;

  if ( flags & WIDGET_HSCROLL )
    knob.x = (short)(x + 1 + (current - min_level) * step + 0.5);
  else knob.y = (short)(y + 1 + (current - min_level) * step + 0.5);

  if ( hook ) hook->WidgetActivated( this, surface );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : SliderWidget::MouseDown
// DESCRIPTION: Depress the slider or move the knob towards the mouse.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status SliderWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  short mx = button.x - surface->LeftEdge();
  short my = button.y - surface->TopEdge();

  if ( Contains( mx, my ) ) {
    if ( button.button == SDL_BUTTON_LEFT ) {
      if ( knob.Contains( mx, my ) ) {
        Push();
        mousehit = true;
      } else {
        short diff = MAX( 1, size/2 );
        if ( flags & WIDGET_HSCROLL ) {
          if ( mx < knob.x ) ScrollTo( current - diff );
          else if ( mx >= (knob.x + knob.w) ) ScrollTo( current + diff );
        } else {
          if ( my < knob.y ) ScrollTo( current - diff );
          else if ( my >= (knob.y + knob.h) ) ScrollTo( current + diff );
        }
      }
    } else if ( button.button == SDL_BUTTON_WHEELUP ) {
      ScrollTo( current - MAX(1, size/4) );
    } else if ( button.button == SDL_BUTTON_WHEELDOWN ) {
      ScrollTo( current + MAX(1, size/4) );
    }
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : SliderWidget::MouseUp
// DESCRIPTION: Release the knob.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status SliderWidget::MouseUp( const SDL_MouseButtonEvent &button ) {
  if ( Clicked() && (button.button == SDL_BUTTON_LEFT) ) {
    Release();
    mousehit = false;
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : SliderWidget::KeyDown
// DESCRIPTION: Depress knob and move slider if correct key was hit.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status SliderWidget::KeyDown( const SDL_keysym &key ) {
  if ( key.sym == this->key ) {
    Push();
    if ( key.mod & (KMOD_LSHIFT|KMOD_RSHIFT) ) {
      if ( current > min_level ) ScrollTo( current - keystep );
    } else if ( current < max_level ) ScrollTo( current + keystep );
  } else if ( flags & (WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY) ) {
    switch( key.sym ) {
    case SDLK_UP:
      if ( (flags & WIDGET_VSCROLLKEY) && (current > min_level) ) {
        Push();
        ScrollTo( current - keystep );
      }
      break;
    case SDLK_DOWN:
      if ( (flags & WIDGET_VSCROLLKEY) && (current < max_level) ) {
        Push();
        ScrollTo( current + keystep );
      }
      break;

    case SDLK_LEFT:
      if ( (flags & WIDGET_HSCROLLKEY) && (current > min_level) ) {
        Push();
        ScrollTo( current - keystep );
      }
      break;
    case SDLK_RIGHT:
      if ( (flags & WIDGET_HSCROLLKEY) && (current < max_level) ) {
        Push();
        ScrollTo( current + keystep );
      }
      break;
    default:
      break;
    }
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : SliderWidget::KeyUp
// DESCRIPTION: Release the knob if necessary.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status SliderWidget::KeyUp( const SDL_keysym &key ) {
  if ( Clicked() ) Release();
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : SliderWidget::MouseMove
// DESCRIPTION: Drag the knob after the mouse.
// PARAMETERS : motion - SDL_MouseMotionEvent from the event handler
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status SliderWidget::MouseMove( const SDL_MouseMotionEvent &motion ) {
  if ( mousehit ) {

    if ( max_level != min_level ) {
      short val;

      if ( flags & WIDGET_HSCROLL ) {
        short dx = knob.x + motion.xrel;
        knob.x = MIN( MAX( x + 1, dx ), x + w - 1 - knob.w );
        val = (short)((knob.x - 1 - x + step/2) / step) + min_level;
      } else {
        short dy = knob.y + motion.yrel;
        knob.y = MIN( MAX( y + 1, dy ), y + h - 1 - knob.h );
        val = (short)((knob.y - 1 - y + step/2) / step) + min_level;
      }

      Draw();
      Show();
      if ( val != current ) {
        current = MAX( min_level, MIN( val, max_level ) );
        if ( hook ) hook->WidgetActivated( this, surface );
      }
    }
  }
  return GUI_OK;
}


////////////////////////////////////////////////////////////////////////
// NAME       : ProgressWidget::ProgressWidget
// DESCRIPTION: Create a new progress bar. Initially, the new progress
//              bar will be empty.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              vmin   - value mapped to an empty bar
//              vmax   - value mapped to a completely filled bar
//              flags  - widget flags (see widget.h for details)
//              title  - widget title
//              window - window to add this widget to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

ProgressWidget::ProgressWidget( short id, short x, short y, unsigned short w,
    unsigned short h, short vmin, short vmax, unsigned short flags,
    const char *title, Window *window ) :
    Widget( id, x, y, w, h, flags, title, window ),
    level(0), min_level(vmin), max_level(vmax), col(CF_COLOR_HIGHLIGHT),
    title_x(0), title_y(0) {
  if ( title ) {
    title_x = x + (w - font->TextWidth( title )) / 2;
    title_y = y + (h - font->Height()) / 2;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : ProgressWidget::SetLevel
// DESCRIPTION: Fill the progress bar.
// PARAMETERS : lev - level up to which the bar should be filled. Must
//                    be between min and max. min will clear the bar,
//                    max will fill it completely.
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ProgressWidget::SetLevel( short lev ) {
  if ( lev < min_level ) lev = min_level;
  else if ( lev > max_level ) lev = max_level;

  level = lev - min_level;
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : ProgressWidget::Draw
// DESCRIPTION: Draw the progress bar.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ProgressWidget::Draw( void ) {
  surface->DrawBack( *this );
  surface->DrawBox( *this, BOX_RECESSED );

  if ( !Disabled() && (level > 0) ) {
    unsigned short levsize = level * (Width() - 2) / (max_level - min_level);
    surface->FillRectAlpha( x + 1, y + 1, levsize, h - 2, col );
  }

  PrintTitle( title_x, title_y, col );
}
