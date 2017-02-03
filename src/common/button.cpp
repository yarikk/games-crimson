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
// button.cpp
////////////////////////////////////////////////////////////////////////

#include "button.h"
#include "misc.h"
#include "sound.h"

////////////////////////////////////////////////////////////////////////
// NAME       : ButtonWidget::ButtonWidget
// DESCRIPTION: Create a button widget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget relative to window border
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - see widget.h for details
//              title  - widget title (may be NULL)
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

ButtonWidget::ButtonWidget( short id, short x, short y, unsigned short w,
                       unsigned short h, unsigned short flags,
                       const char *title, Window *window ) :
     Widget( id, x, y, w, h, flags, title, window ) {
  if ( !(this->flags & (WIDGET_ALIGN_CENTER|WIDGET_ALIGN_LEFT|
                        WIDGET_ALIGN_RIGHT|WIDGET_ALIGN_TOP)) )
    this->flags |= WIDGET_ALIGN_CENTER;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ButtonWidget::Draw
// DESCRIPTION: Draw the button widget.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ButtonWidget::Draw( void ) {
  surface->DrawBack( *this );

  if ( (flags & WIDGET_STYLE_MENU) && Clicked() ) {
    surface->DrawBox( *this, BOX_RECESSED );
    surface->FillRectAlpha( x+1, y+1, w-2, h-2, Color(CF_COLOR_SHADOW) );
  } else if ( flags & WIDGET_STYLE_HIGHLIGHT ) {
    surface->FillRectAlpha( *this, surface->GetFGPen() );
  } else if ( !(flags & WIDGET_STYLE_NOBORDER) ) {
    surface->DrawBox( *this, BOX_RECESSED );
    surface->DrawBox( Rect(x+1,y+1,w-2,h-2), Clicked() ? BOX_RECESSED : BOX_RAISED );
    if ( Clicked() )
      surface->FillRectAlpha( x+2, y+2, w-4, h-4, Color(CF_COLOR_SHADOW) );
  }

  if ( flags & WIDGET_STYLE_GFX ) {
    Image &img = image[Clicked()];

    // the flags designate the label alignment; image alignment is opposite
    int xoff;
    if ( (flags & (WIDGET_ALIGN_LEFT|WIDGET_ALIGN_WITHIN)) ==
         (WIDGET_ALIGN_LEFT|WIDGET_ALIGN_WITHIN) ) {
      xoff = x + (w - img.Width()) - 2;
    } else if ( (flags & (WIDGET_ALIGN_RIGHT|WIDGET_ALIGN_WITHIN)) ==
         (WIDGET_ALIGN_RIGHT|WIDGET_ALIGN_WITHIN) ) {
      xoff = x + 2;
    } else {
      xoff = x + (w - img.Width()) / 2;
    }
    img.Draw( surface, xoff, y + (h - img.Height()) / 2 );
  }

  PrintTitle( ((flags & WIDGET_STYLE_HIGHLIGHT) && !Clicked()) ?
              surface->GetBGPen() : surface->GetFGPen() );
  if ( flags & WIDGET_STYLE_SUBMENU )
    image[0].Draw( surface, x + w - 3 - image[0].Width(),
                   y + (h - image[0].Height())/2 + (Clicked() ? 1 : 0) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : ButtonWidget::SetImage
// DESCRIPTION: For widgets which have the WIDGET_STYLE_GFX flag set,
//              define the images to use for displaying them.
// PARAMETERS : image  - the surface containing the images
//              state1 - rectangle containing the position of the image
//                       to show when the widget is not selected
//              state2 - rectangle containing the position of the image
//                       to show when the widget is selected
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ButtonWidget::SetImage( Surface *image,
                             const Rect &state1, const Rect &state2 ) {
  this->image[0] = Image( image, state1.x, state1.y, state1.w, state1.h );
  this->image[1] = Image( image, state2.x, state2.y, state2.w, state2.h );
}

////////////////////////////////////////////////////////////////////////
// NAME       : ButtonWidget::MouseDown
// DESCRIPTION: Show the widget in depressed state if it was selected.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status ButtonWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  if ( (button.button == SDL_BUTTON_LEFT) &&
       Contains( button.x - surface->LeftEdge(),
                 button.y - surface->TopEdge() ) ) Push();
  else if ( Clicked() ) Release();
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ButtonWidget::MouseUp
// DESCRIPTION: Release the button and activate it if the mouse pointer
//              is still over it.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ButtonWidget::MouseUp( const SDL_MouseButtonEvent &button ) {
  if ( Clicked() && (button.button == SDL_BUTTON_LEFT) ) {
    Release();
    if ( Contains( button.x - surface->LeftEdge(),
                   button.y - surface->TopEdge() ) ) return Activate();
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ButtonWidget::KeyDown
// DESCRIPTION: Depress button if the correct key was hit.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status ButtonWidget::KeyDown( const SDL_keysym &key ) {
  if ( (key.sym == this->key) ||
       ((flags & WIDGET_DEFAULT) && (key.sym == SDLK_RETURN)) ) Push();
  else if ( Clicked() ) Release();
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ButtonWidget::KeyUp
// DESCRIPTION: Activate widget if correct key was released.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ButtonWidget::KeyUp( const SDL_keysym &key ) {
  if ( Clicked() && ((key.sym == this->key) ||
       ((flags & WIDGET_DEFAULT) && (key.sym == SDLK_RETURN))) ) {
    Release();
    return Activate();
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ButtonWidget::Push
// DESCRIPTION: Change the widget state to 'clicked', redraw, and play
//              a sound effect.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ButtonWidget::Push( void ) {
  if ( !clicked ) {
    Audio::PlaySfx( Audio::SND_GUI_PRESSED, 0 );
    Widget::Push();
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : ButtonWidget::Activate
// DESCRIPTION: Activate widget, i.e. call the activation function of
//              the hook class.
// PARAMETERS : -
// RETURNS    : return code of the activation function
////////////////////////////////////////////////////////////////////////

GUI_Status ButtonWidget::Activate( void ) {
  if ( hook ) return hook->WidgetActivated( this, surface );
  return (GUI_Status)id;
}


////////////////////////////////////////////////////////////////////////
// NAME       : CheckboxWidget::CheckboxWidget
// DESCRIPTION: Create a checkbox widget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget relative to window border
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              state  - initial state (checked or unchecked)
//              flags  - see widget.h for details
//              title  - widget title (may be NULL)
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

#define GFX_CHECK_SIZE    15
#define GFX_CHECK_OFF_X   64
#define GFX_CHECK_OFF_Y   46
#define GFX_CHECK_ON_X    79
#define GFX_CHECK_ON_Y    46

CheckboxWidget::CheckboxWidget( short id, short x, short y, unsigned short w,
                       unsigned short h, bool state, unsigned short flags,
                       const char *title, Window *window ) :
     ButtonWidget( id, x, y, w, h, flags, title, window ) {
  clicked = state;

  // set default graphics
  if ( flags & WIDGET_STYLE_GFX ) {
    Surface *icons = surface->GetView()->GetSystemIcons();
    image[0] = Image( icons, GFX_CHECK_OFF_X, GFX_CHECK_OFF_Y,
                      GFX_CHECK_SIZE, GFX_CHECK_SIZE );
    image[1] = Image( icons, GFX_CHECK_ON_X, GFX_CHECK_ON_Y,
                      GFX_CHECK_SIZE, GFX_CHECK_SIZE );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : CheckboxWidget::MouseDown
// DESCRIPTION: Check or uncheck the widget, depending on its current
//              state.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status CheckboxWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  if ( (button.button == SDL_BUTTON_LEFT) &&
       Contains( button.x - surface->LeftEdge(),
                 button.y - surface->TopEdge() ) ) {
    if ( Clicked() ) {
      Audio::PlaySfx( Audio::SND_GUI_PRESSED, 0 );
      Release();
    } else Push();
    return Activate();
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CheckboxWidget::KeyDown
// DESCRIPTION: Toggle the widget status when the corresponding key was
//              hit.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status CheckboxWidget::KeyDown( const SDL_keysym &key ) {
  if ( (key.sym == this->key) ||
       ((flags & WIDGET_DEFAULT) && (key.sym == SDLK_RETURN)) ) {
    if ( Clicked() ) {
      Audio::PlaySfx( Audio::SND_GUI_PRESSED, 0 );
      Release();
    } else Push();
    return Activate();
  }
  return GUI_OK;
}


////////////////////////////////////////////////////////////////////////
// NAME       : MenuButtonWidget::MenuButtonWidget
// DESCRIPTION: Create a button widget for a MenuWindow.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget relative to window border
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - see widget.h for details
//              title  - widget title (may be NULL)
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

MenuButtonWidget::MenuButtonWidget( short id, short x, short y, unsigned short w,
                  unsigned short h, unsigned short flags,
                  const char *title, Window *window ) :
      ButtonWidget( id, x, y, w, h, flags, title, window ) {
  image[0] = Image( window->GetView()->GetSystemIcons(), 175, 46, 7, 11 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuButtonWidget::MouseMove
// DESCRIPTION: When the mouse is moved over the button, highlight it.
// PARAMETERS : motion - SDL_MouseMotionEvent received from the event
//                       handler
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status MenuButtonWidget::MouseMove( const SDL_MouseMotionEvent &motion ) {
  bool contain = Contains( motion.x - surface->LeftEdge(), motion.y - surface->TopEdge() );

  if ( ((flags & WIDGET_STYLE_HIGHLIGHT) && !contain) ||
       (!(flags & WIDGET_STYLE_HIGHLIGHT) && contain) ) {
    ToggleCurrent();
    Draw();
    Show();
  }

  return GUI_OK;
}


#define GFX_CYCLE_W 7
#define GFX_CYCLE_H 12
#define GFX_CYCLE_X 138
#define GFX_CYCLE_Y 46

////////////////////////////////////////////////////////////////////////
// NAME       : CycleWidget::CycleWidget
// DESCRIPTION: Create a new cycle widget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget relative to window border
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - see widget.h for details (default title
//                       placement is WIDGET_ALIGN_LEFT)
//              title  - widget title (may be NULL)
//              defval - default value, starting with 0
//              labels - array of available choices, terminated by a
//                       NULL-string
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

CycleWidget::CycleWidget( short id, short x, short y, unsigned short w,
             unsigned short h, unsigned short flags, const char *title,
             unsigned short defval, const char *labels[], Window *window ) :
     ButtonWidget( id, x, y, w, h, flags, title, window ),
     maxval(0) {

  if ( !(flags & (WIDGET_ALIGN_CENTER|WIDGET_ALIGN_LEFT|
                  WIDGET_ALIGN_RIGHT|WIDGET_ALIGN_TOP)) )
    // button widget defaults to center which we must revert
    this->flags ^= WIDGET_ALIGN_LEFT|WIDGET_ALIGN_CENTER;

  image[0] = Image( window->GetView()->GetSystemIcons(),
                    GFX_CYCLE_X, GFX_CYCLE_Y,
                    GFX_CYCLE_W, GFX_CYCLE_H );

  int i;
  for ( i = 1; labels[i]; ++i ) ++maxval;
  choices = new string[maxval+1];
  for ( i = 0; i <= maxval; ++i ) choices[i].assign( labels[i] );

  val = MIN( defval, maxval );
}

////////////////////////////////////////////////////////////////////////
// NAME       : CycleWidget::Draw
// DESCRIPTION: Draw the cycle widget.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CycleWidget::Draw( void ) {
  ButtonWidget::Draw();

  short off = (Clicked() ? 1 : 0);
  Color fcol;

  surface->FillRect( x + w - image[0].Width() - 8 + off, y + 4 + off, 1, h - 8, Color(CF_COLOR_SHADOW) );
  surface->FillRect( x + w - image[0].Width() - 7 + off, y + 4 + off, 1, h - 8, Color(CF_COLOR_HIGHLIGHT) );

  image[0].Draw( surface, x + w - image[0].Width() - 4 + off, y + (h - image[0].Height())/2 + off );

  if ( Disabled() ) {
    fcol = font->GetColor();
    font->SetColor( Color(CF_COLOR_GHOSTED) );
  }

  // draw current label
  font->Write( choices[val].c_str(), surface,
               x + (w - image[0].Width() - 10 - font->TextWidth(choices[val].c_str()))/2 + off,
               y + (h - font->Height()) / 2 + off, *this );

  if ( Disabled() ) font->SetColor( fcol );
}

////////////////////////////////////////////////////////////////////////
// NAME       : CycleWidget::MouseDown
// DESCRIPTION: Show the widget in depressed state if it was selected.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status CycleWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  if ( ((button.button == SDL_BUTTON_LEFT) ||
       (button.button == SDL_BUTTON_RIGHT)) &&
       Contains( button.x - surface->LeftEdge(),
                 button.y - surface->TopEdge() ) ) {
    up = (button.button == SDL_BUTTON_LEFT);
    Push();
  } else if ( Clicked() ) Release();
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CycleWidget::MouseUp
// DESCRIPTION: Release the button and activate it if the mouse pointer
//              is still over it.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status CycleWidget::MouseUp( const SDL_MouseButtonEvent &button ) {
  if ( Clicked() &&
       ((button.button == SDL_BUTTON_LEFT) ||
        (button.button == SDL_BUTTON_RIGHT)) ) {
    bool hit = Contains( button.x - surface->LeftEdge(),
                         button.y - surface->TopEdge() );
    if ( hit ) CycleValue();
    Release();
    if ( hit ) return Activate();
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CycleWidget::KeyUp
// DESCRIPTION: Activate widget if correct key was released.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status CycleWidget::KeyUp( const SDL_keysym &key ) {
  if ( Clicked() && ((key.sym == this->key) ||
       ((flags & WIDGET_DEFAULT) && (key.sym == SDLK_RETURN))) ) {
    up = ((key.mod & (KMOD_LSHIFT|KMOD_RSHIFT)) == 0);
    CycleValue();
    Release();
    return Activate();
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CycleWidget::CycleValue
// DESCRIPTION: Update the widget value according to the button pressed.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CycleWidget::CycleValue( void ) {
  if (up) val = (val + 1) % (maxval + 1);
  else if ( val == 0 ) val = maxval;
  else --val;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CycleWidget::SetValue
// DESCRIPTION: Set the cycle value.
// PARAMETERS : value - new value
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CycleWidget::SetValue( unsigned short value ) {
  val = value % (maxval + 1);
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : DropWidget::DropWidget
// DESCRIPTION: Create a dropd-own widget, which is a ButtonWidget with
//              a small arrow image.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget relative to window border
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - see widget.h for details
//              title  - widget title (may be NULL)
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

DropWidget::DropWidget( short id, short x, short y, unsigned short w,
             unsigned short h, unsigned short flags, const char *title,
             Window *window ) :
     ButtonWidget( id, x, y, w, h, flags, title, window ) {
  image[0] = Image( window->GetView()->GetSystemIcons(), 157, 53, 11, 7 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : DropWidget::Draw
// DESCRIPTION: Draw the drop-down widget.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void DropWidget::Draw( void ) {
  surface->DrawBack( *this );

  surface->DrawBox( *this, BOX_RECESSED );
  surface->DrawBox( Rect(x+1,y+1,w-2,h-2), Clicked() ? BOX_RECESSED : BOX_RAISED );
  if ( Clicked() )
    surface->FillRectAlpha( x+2, y+2, w-4, h-4, Color(CF_COLOR_SHADOW) );

  short off = (Clicked() ? 1 : 0);
  short xoff = x + (w - font->TextWidth(title.c_str()) - image[0].Width() - 8) / 2;
  short yoff = y + (h - font->Height()) / 2;
  PrintTitle( xoff + off, yoff + off, surface->GetFGPen() );

  surface->FillRect( x + w - image[0].Width() - 8 + off, y + 4 + off, 1, h - 8, Color(CF_COLOR_SHADOW) );
  surface->FillRect( x + w - image[0].Width() - 7 + off, y + 4 + off, 1, h - 8, Color(CF_COLOR_HIGHLIGHT) );

  image[0].Draw( surface, x + w - image[0].Width() - 4 + off, y + (h - image[0].Height())/2 + off );
}

